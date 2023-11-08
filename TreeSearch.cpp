//---------------------------------------------------------------------------
#include "TreeSearch.h"
#include "Allometry.h"
#include "TreePopulation.h"
#include "Plot.h"
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clTreeSearch::clTreeSearch(clTreePopulation *p_oTreePop, clPlot *p_oPlot) {
  try {
    //Initialize variables
    mp_oTreePop = p_oTreePop;
    mp_oCurrentTree = NULL;
    function = NULL;

    //Initialize search flags
    m_bDistanceHeightUsed = false;
    m_bTypeUsed = false;
    m_bSpeciesUsed = false;
    m_bAllUsed = false;

    m_bSeeds = false;
    m_bSeedlings = false;
    m_bSaplings = false;
    m_bAdults = false;
    m_bSnags = false;
    m_bWoody_Debris = false;

    //Initialize type search variables
    m_iWhatTypes = 0;
    m_iCurrentXGrid = 0;
    m_iCurrentYGrid = 0;
    m_fHeightCutoff = 0;
    m_fFromX = 0;
    m_fFromY = 0;
    m_fDistanceCutoff = 0;

    //Create the array for which species will be used (if it's a species
    //search), and set all to false
    int iNumSpecies = p_oTreePop->GetNumberOfSpecies(), i;
    mp_bWhatSpecies = new bool[iNumSpecies];
    for (i = 0; i < iNumSpecies; i++) {
      mp_bWhatSpecies[i] = false;
    }

    //Set the plot object pointer
    mp_oPlot = p_oPlot;

    m_iMaxX = 0;
    m_iMaxY = 0;
    m_iMinX = 0;
    m_iMinY = 0;
    m_iHomeX = 0;
    m_iHomeY = 0;

    m_iWhatTypes = 0;
    m_iStartHeightDiv = 0;
    m_iEndHeightDiv = 0;

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreeSearch::clTreeSearch";
    throw(stcErr);
  }
}

clTreeSearch::~clTreeSearch() {
  delete[] mp_bWhatSpecies;
}


////////////////////////////////////////////////////////////////////////////
// Setup()
////////////////////////////////////////////////////////////////////////////
void clTreeSearch::Setup() {
  try {
    unsigned long int iBitMask = 1;

    //Validate the flag combinations - type alone, species alone, all alone, and
    //height/distance alone OK; type and species together OK; anything else not
    if(!((m_bAllUsed&& !m_bDistanceHeightUsed&& !m_bSpeciesUsed&& !m_bTypeUsed)
        || (!m_bAllUsed && m_bDistanceHeightUsed && !m_bSpeciesUsed && !m_bTypeUsed)
        || (!m_bAllUsed && !m_bDistanceHeightUsed && m_bSpeciesUsed && m_bTypeUsed)
        || (!m_bAllUsed && !m_bDistanceHeightUsed && !m_bSpeciesUsed
            && m_bTypeUsed))) {
      modelErr stcErr;
      stcErr.iErrorCode = ILLEGAL_OP;
      stcErr.sFunction = "clTreeSearch::Setup";
      stcErr.sMoreInfo = "Invalid search request.";
      throw(stcErr);
    }

    //Validate the data
    if (m_bDistanceHeightUsed) {
      //Make sure we have a positive number for distance and height
      if (m_fDistanceCutoff < 0 || m_fHeightCutoff < 0) { //nope - throw error
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreeSearch::Setup";
        stcErr.sMoreInfo = "Either distance or height search criterion is a negative number.";
        throw(stcErr);
      }
      //Make sure our "from" point is within the plot
      if (m_fFromX < 0 || m_fFromX >= mp_oTreePop->m_fPlotLengthX ||
          m_fFromY < 0 || m_fFromY >= mp_oTreePop->m_fPlotLengthY) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreeSearch::Setup";
        stcErr.sMoreInfo = "Either X or Y search point is outside plot.";
        throw(stcErr);
      }
    }  //end of if (m_bDistanceHeightUsed)

    if (m_bTypeUsed) {
      //Make sure we don't have invalid types
      if (m_iWhatTypes >= pow(2, mp_oTreePop->m_iNumTypes)) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreeSearch::Setup";
        stcErr.sMoreInfo = "Invalid types in tree search.";
        throw(stcErr);
      }

      //Extract the type information into boolean values to make it easier to
      //work with - we use a bit mask against the value (more on this elsewhere)
      if ((m_iWhatTypes >> clTreePopulation::seed) & iBitMask)
        m_bSeeds = true;
      if ((m_iWhatTypes >> clTreePopulation::seedling) & iBitMask)
        m_bSeedlings = true;
      if ((m_iWhatTypes >> clTreePopulation::sapling) & iBitMask)
        m_bSaplings = true;
      if ((m_iWhatTypes >> clTreePopulation::adult) & iBitMask)
        m_bAdults = true;
      if ((m_iWhatTypes >> clTreePopulation::snag) & iBitMask)
        m_bSnags = true;
      if ((m_iWhatTypes >> clTreePopulation::woody_debris) & iBitMask)
        m_bWoody_Debris = true;

      //Figure out the bracketing height divisions
      //if we're looking for seedlings or saplings, the beginning height div
      //is the first one
      if (m_bSeeds || m_bSeedlings || m_bSaplings)
        m_iStartHeightDiv = 0;
      else //start at the height div of the shortest adult height
        m_iStartHeightDiv = (int)
        floor(mp_oTreePop->m_fMinAdultHeight/mp_oTreePop->m_iSizeHeightDivs);
      //If there are no adults or saplings then the tallest height div is the
      //seedling height cutoff one
      if (!m_bAdults && !m_bSnags && !m_bWoody_Debris && !m_bSaplings) {
        float fMaxSeedlingHeight = 0;
        for (int m = 0; m < mp_oTreePop->GetNumberOfSpecies(); m++) {
          if (fMaxSeedlingHeight < mp_oTreePop->mp_fMaxSeedlingHeight[m])
            fMaxSeedlingHeight = mp_oTreePop->mp_fMaxSeedlingHeight[m];
        }
        m_iEndHeightDiv = (int)floor(fMaxSeedlingHeight/mp_oTreePop->m_iSizeHeightDivs);
      }
      //If there are saplings then the tallest height div is the sapling height
      //cutoff one
      else if (!m_bAdults && !m_bSnags && !m_bWoody_Debris)
        m_iEndHeightDiv = (int)
        floor(mp_oTreePop->m_fMaxSaplingHeight/mp_oTreePop->m_iSizeHeightDivs);
      //Otherwise it's the tallest one
      else m_iEndHeightDiv = mp_oTreePop->m_iNumHeightDivs - 1;
    } //end of if (m_bTypesUsed

    if (m_bAllUsed) {
      function = &clTreeSearch::FindNextAllTree;
      mp_oCurrentTree = FindFirstAllTree();
    } else if (m_bDistanceHeightUsed) {
      function = &clTreeSearch::FindNextDHTree;
      mp_oCurrentTree = FindFirstDHTree();
    } else if (m_bTypeUsed && !m_bSpeciesUsed) {
      function = &clTreeSearch::FindNextTTree;
      mp_oCurrentTree = FindFirstTTree();
    } else {
      function = &clTreeSearch::FindNextTSTree;
      mp_oCurrentTree = FindFirstTSTree();
    }
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreeSearch::Setup";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// StartOver()
////////////////////////////////////////////////////////////////////////////
void clTreeSearch::StartOver() {
  try {
    m_iCurrentXGrid = 0;
    m_iCurrentYGrid = 0;

    if (m_bAllUsed)
      mp_oCurrentTree = FindFirstAllTree();
    else if (m_bDistanceHeightUsed)
      mp_oCurrentTree = FindFirstDHTree();
    else if (m_bTypeUsed && !m_bSpeciesUsed)
      mp_oCurrentTree = FindFirstTTree();
    else
      mp_oCurrentTree = FindFirstTSTree();
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreeSearch::StartOver";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// FindNextAllTree
////////////////////////////////////////////////////////////////////////////
clTree* clTreeSearch::FindNextAllTree() {
  clTree *p_oTree = NULL; //the tree that this function will return
  unsigned short int iX, iY, iDiv; //loop counters

  //If the current tree is NULL, continue to return NULL
  if (NULL == mp_oCurrentTree) return NULL;

  p_oTree = mp_oCurrentTree->GetTaller();
  if (p_oTree)
    return p_oTree;
  else { //go hunting through the rest of the Y grid cells for this X grid
    iX = m_iCurrentXGrid;
    for (iY = m_iCurrentYGrid + 1; iY < mp_oTreePop->m_iNumYCells; iY++) {
      for (iDiv = 0; iDiv < mp_oTreePop->m_iNumHeightDivs; iDiv++) {
        p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][iDiv];
        if (p_oTree != NULL) break;
      }
      if (p_oTree != NULL) break;
    }

    //Still no tree?  Start the next X row and search everything else
    if (!p_oTree) {
      for (iX = m_iCurrentXGrid + 1; iX < mp_oTreePop->m_iNumXCells; iX++) {
        for (iY = 0; iY < mp_oTreePop->m_iNumYCells; iY++) {
          for (iDiv = 0; iDiv < mp_oTreePop->m_iNumHeightDivs; iDiv++) {
            p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][iDiv];
            if (p_oTree != NULL) break;
          }
          if (p_oTree != NULL) break;
        }
        if (p_oTree != NULL) break;
      }
    }
    //Set the current grid stuff accordingly
    m_iCurrentXGrid = iX;
    m_iCurrentYGrid = iY;

    //We should have a tree if there are any to be had.  Return it.
    return p_oTree;
  } //end of else
}

////////////////////////////////////////////////////////////////////////////
// FindNextTSTree()
////////////////////////////////////////////////////////////////////////////
clTree* clTreeSearch::FindNextTSTree() {
  float fHeight;
  unsigned long int iBitMask = 1;
  short int iX, iY,  //loop counters for moving through grids
  iTypeTest = 0;
  clTree *p_oNextTree = NULL;

  //If the current tree is NULL, continue to return NULL
  if (NULL == mp_oCurrentTree) return NULL;

  //Start with the next taller tree
  p_oNextTree = mp_oCurrentTree->GetTaller();
  //Check to see if this type is good - if so, return it
  while (p_oNextTree) {
    //test each tree to see if it's one of the ones we want - in order to test
    //whether type n is on our list, we start by shifting the type int
    //n bits to the right, which puts the nth bit as the rightmost one.  Then
    //by using a bitwise AND against iBitMask, we reduce the type list to
    //either 1 (type is on the list) or 0 (type not  on the list).
    iTypeTest = m_iWhatTypes >> p_oNextTree->GetType();
    if (mp_bWhatSpecies[p_oNextTree->GetSpecies()] && (iTypeTest & iBitMask))
      return p_oNextTree;
    else {
      p_oNextTree->GetValue(mp_oTreePop->mp_iHeightCode[
                                                        p_oNextTree->GetSpecies()][p_oNextTree->GetType()], &fHeight);
      //Check to see if we're taller than the cutoff height division
      if (floor(fHeight/mp_oTreePop->m_iSizeHeightDivs) > m_iEndHeightDiv)
        break;
    }
    p_oNextTree = p_oNextTree->GetTaller();
  } //end of while (p_oNextTree)

  //If we're still here, go through the rest of the Y grid cells for this
  //X grow
  for (iY = m_iCurrentYGrid + 1; iY < mp_oTreePop->m_iNumYCells; iY++) {
    //if our starting size division is empty find the first populated one
    //above it
    p_oNextTree =
        mp_oTreePop->mp_oTreeShortest[m_iCurrentXGrid][iY][m_iStartHeightDiv];
    if (p_oNextTree == NULL)
      for (int i = m_iStartHeightDiv + 1; i <= m_iEndHeightDiv; i++) {
        p_oNextTree = mp_oTreePop->mp_oTreeShortest[m_iCurrentXGrid][iY][i];
        if (p_oNextTree != NULL)
          break;
      }

    //loop through all the trees taller than our first in this grid cell
    while (p_oNextTree != NULL) {
      iTypeTest = m_iWhatTypes >> p_oNextTree->GetType();
      if (mp_bWhatSpecies[p_oNextTree->GetSpecies()] && (iTypeTest & iBitMask)) {
        //found a tree - mark our place and return the tree
        m_iCurrentYGrid = iY;
        return p_oNextTree;
      } else {
        p_oNextTree->GetValue(mp_oTreePop->mp_iHeightCode[
                                                          p_oNextTree->GetSpecies()][p_oNextTree->GetType()], &fHeight);
        //Check to see if we're taller than the cutoff height division
        if (floor(fHeight/mp_oTreePop->m_iSizeHeightDivs) > m_iEndHeightDiv)
          break;
      }
      p_oNextTree = p_oNextTree->GetTaller();
    } //end of while (p_oNextTree != NULL)
  } //end of for loops

  //Still here?  Start searching through the rest of the grid cells
  for (iX = m_iCurrentXGrid + 1; iX < mp_oTreePop->m_iNumXCells; iX++)
    for (iY = 0; iY < mp_oTreePop->m_iNumYCells; iY++) {

      //if our starting size division is empty find the first populated one
      //above it
      p_oNextTree = mp_oTreePop->mp_oTreeShortest[iX][iY][m_iStartHeightDiv];
      if (p_oNextTree == NULL)
        for (int i = m_iStartHeightDiv + 1; i <= m_iEndHeightDiv; i++) {
          p_oNextTree = mp_oTreePop->mp_oTreeShortest[iX][iY][i];
          if (p_oNextTree != NULL)
            break;
        }

      //loop through all the trees taller than our first in this grid cell
      while (p_oNextTree != NULL) {
        iTypeTest = m_iWhatTypes >> p_oNextTree->GetType();
        if (mp_bWhatSpecies[p_oNextTree->GetSpecies()] && (iTypeTest & iBitMask)) {
          //found a tree - mark our place and return the tree
          m_iCurrentXGrid = iX;
          m_iCurrentYGrid = iY;
          return p_oNextTree;
        } else {
          p_oNextTree->GetValue(mp_oTreePop->mp_iHeightCode[
                                                            p_oNextTree->GetSpecies()][p_oNextTree->GetType()], &fHeight);
          //Check to see if we're taller than the cutoff height division
          if (floor(fHeight/mp_oTreePop->m_iSizeHeightDivs) > m_iEndHeightDiv)
            break;
        }
        p_oNextTree = p_oNextTree->GetTaller();
      } //end of while (p_oNextTree != NULL)
    } //end of for loops

  //Still nothing?  No trees left.
  m_iCurrentXGrid = mp_oTreePop->m_iNumXCells;
  m_iCurrentYGrid = mp_oTreePop->m_iNumYCells;
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
// FindNextTTree()
////////////////////////////////////////////////////////////////////////////
clTree* clTreeSearch::FindNextTTree() {
  float fHeight;
  unsigned long int iBitMask = 1;
  short int iX, iY,  //loop counters for moving through grids
  iTypeTest = 0;
  clTree *p_oNextTree = NULL;

  //If the current tree is NULL, continue to return NULL
  if (NULL == mp_oCurrentTree) return NULL;

  //Start with the next taller tree
  p_oNextTree = mp_oCurrentTree->GetTaller();
  //Check to see if this type is good - if so, return it
  while (p_oNextTree) {
    iTypeTest = m_iWhatTypes >> p_oNextTree->GetType();
    if (iTypeTest & iBitMask)
      return p_oNextTree;
    else {
      p_oNextTree->GetValue(mp_oTreePop->mp_iHeightCode[
                                                        p_oNextTree->GetSpecies()][p_oNextTree->GetType()], &fHeight);
      //Check to see if we're taller than the cutoff height division
      if (floor(fHeight/mp_oTreePop->m_iSizeHeightDivs) > m_iEndHeightDiv)
        break;
    }
    p_oNextTree = p_oNextTree->GetTaller();
  } //end of while (p_oNextTree)

  //If we're still here, go through the rest of the Y grid cells for this
  //X grow
  for (iY = m_iCurrentYGrid + 1; iY < mp_oTreePop->m_iNumYCells; iY++) {   \
    //if our starting size division is empty find the first populated one
    //above it
    p_oNextTree =
        mp_oTreePop->mp_oTreeShortest[m_iCurrentXGrid][iY][m_iStartHeightDiv];
  if (p_oNextTree == NULL)
    for (int i = m_iStartHeightDiv + 1; i <= m_iEndHeightDiv; i++) {
      p_oNextTree = mp_oTreePop->mp_oTreeShortest[m_iCurrentXGrid][iY][i];
      if (p_oNextTree != NULL)
        break;
    }

  //loop through all the trees taller than our first in this grid cell
  while (p_oNextTree != NULL) {
    iTypeTest = m_iWhatTypes >> p_oNextTree->GetType();
    //test each tree to see if it's one of the ones we want
    if (iTypeTest & iBitMask) {
      //found a tree - mark our place and return the tree
      m_iCurrentYGrid = iY;
      return p_oNextTree;
    } else {
      p_oNextTree->GetValue(mp_oTreePop->mp_iHeightCode[
                                                        p_oNextTree->GetSpecies()][p_oNextTree->GetType()], &fHeight);
      //Check to see if we're taller than the cutoff height division
      if (floor(fHeight/mp_oTreePop->m_iSizeHeightDivs) > m_iEndHeightDiv)
        break;
    }
    p_oNextTree = p_oNextTree->GetTaller();
  } //end of while (p_oNextTree != NULL)
  } //end of for loops

  //Still here?  Start searching through the rest of the grid cells
  for (iX = m_iCurrentXGrid + 1; iX < mp_oTreePop->m_iNumXCells; iX++)
    for (iY = 0; iY < mp_oTreePop->m_iNumYCells; iY++) {

      //if our starting size division is empty find the first populated one
      //above it
      p_oNextTree = mp_oTreePop->mp_oTreeShortest[iX][iY][m_iStartHeightDiv];
      if (p_oNextTree == NULL)
        for (int i = m_iStartHeightDiv + 1; i <= m_iEndHeightDiv; i++) {
          p_oNextTree = mp_oTreePop->mp_oTreeShortest[iX][iY][i];
          if (p_oNextTree != NULL)
            break;
        }

      //loop through all the trees taller than our first in this grid cell
      while (p_oNextTree != NULL) {
        iTypeTest = m_iWhatTypes >> p_oNextTree->GetType();
        //test each tree to see if it's one of the ones we want
        if (iTypeTest & iBitMask) {
          //add a new record
          m_iCurrentXGrid = iX;
          m_iCurrentYGrid = iY;
          return p_oNextTree;
        } else {
          p_oNextTree->GetValue(mp_oTreePop->mp_iHeightCode[
                                                            p_oNextTree->GetSpecies()][p_oNextTree->GetType()], &fHeight);
          //Check to see if we're taller than the cutoff height division
          if (floor(fHeight/mp_oTreePop->m_iSizeHeightDivs) > m_iEndHeightDiv)
            break;
        }
        p_oNextTree = p_oNextTree->GetTaller();
      } //end of while (p_oNextTree != NULL)
    } //end of for loops

  //Still nothing?  No trees left.
  m_iCurrentXGrid = mp_oTreePop->m_iNumXCells;
  m_iCurrentYGrid = mp_oTreePop->m_iNumYCells;
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
// FindFirstAllTree()
////////////////////////////////////////////////////////////////////////////
clTree* clTreeSearch::FindFirstAllTree() {
  clTree *p_oTree = NULL; //tree object to assign to current tree variable
  int iX = 0, iY = 0, iDiv; //loop counters

  //Loop through grid cells and height divisions until we find a tree
  for (iX = 0; iX < mp_oTreePop->m_iNumXCells; iX++) {
    for (iY = 0; iY < mp_oTreePop->m_iNumYCells; iY++) {
      for (iDiv = 0; iDiv < mp_oTreePop->m_iNumHeightDivs; iDiv++) {
        p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][iDiv];
        if (p_oTree != NULL) break;
      }
      if (p_oTree != NULL) break;
    }
    if (p_oTree != NULL) break;
  }

  //Set the current grid stuff accordingly
  m_iCurrentXGrid = iX;
  m_iCurrentYGrid = iY;

  //We should have a tree if there are any to be had.  Return it.
  return p_oTree;
}

////////////////////////////////////////////////////////////////////////////
// FindFirstTTree()
////////////////////////////////////////////////////////////////////////////
clTree* clTreeSearch::FindFirstTTree() {
  float fHeight;
  unsigned long int iBitMask = 1;
  short int iX, iY,  //loop counters for moving through grids
  iTypeTest = 0;
  clTree *p_oTree;

  //Go through the hash table to see if we can find a candidate tree
  for (iX = 0; iX < mp_oTreePop->m_iNumXCells; iX++)
    for (iY = 0; iY < mp_oTreePop->m_iNumYCells; iY++) {
      //if our starting size division is empty find the first populated one
      //above it
      p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][m_iStartHeightDiv];
      if (p_oTree == NULL)
        for (int i = m_iStartHeightDiv + 1; i <= m_iEndHeightDiv; i++) {
          p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][i];
          if (p_oTree != NULL)
            break;
        }

      //loop through all the trees taller than our first in this grid cell
      while (p_oTree != NULL) {
        iTypeTest = m_iWhatTypes >> p_oTree->GetType();
        //test each tree to see if it's one of the ones we want
        if (iTypeTest & iBitMask) {
          //set this as the tree to return
          m_iCurrentXGrid = iX;
          m_iCurrentYGrid = iY;
          return p_oTree;
        } else {
          p_oTree->GetValue(mp_oTreePop->mp_iHeightCode[
                                                        p_oTree->GetSpecies()][p_oTree->GetType()], &fHeight);
          //Check to see if we're taller than the cutoff height division
          if (floor(fHeight/mp_oTreePop->m_iSizeHeightDivs) > m_iEndHeightDiv)
            break;
        }
        p_oTree = p_oTree->GetTaller();
      } //end of while (p_oTree != NULL)
    } //end of for loops
  //If we don't have anything by now, return NULL
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
// FindFirstTSTree()
////////////////////////////////////////////////////////////////////////////
clTree* clTreeSearch::FindFirstTSTree() {
  float fHeight;
  unsigned long int iBitMask = 1, iTypeTest = 0;
  unsigned short int iX, iY; //loop counters
  clTree *p_oTree;

  //Go through the hash table to see if we can find a candidate tree
  for (iX = 0; iX < mp_oTreePop->m_iNumXCells; iX++)
    for (iY = 0; iY < mp_oTreePop->m_iNumYCells; iY++) {
      //if our starting size division is empty find the first populated one
      //above it
      p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][m_iStartHeightDiv];
      if (p_oTree == NULL)
        for (int i = m_iStartHeightDiv + 1; i <= m_iEndHeightDiv; i++) {
          p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][i];
          if (p_oTree != NULL)
            break;
        }

      //loop through all the trees taller than our first in this grid cell
      while (p_oTree != NULL) {
        iTypeTest = m_iWhatTypes >> p_oTree->GetType();
        if (mp_bWhatSpecies[p_oTree->GetSpecies()] && (iTypeTest & iBitMask)) {
          //set this as the tree to return
          m_iCurrentXGrid = iX;
          m_iCurrentYGrid = iY;
          return p_oTree;
        } else {
          p_oTree->GetValue(mp_oTreePop->mp_iHeightCode[
                                                        p_oTree->GetSpecies()][p_oTree->GetType()], &fHeight);
          //Check to see if we're taller than the cutoff height division
          if (floor(fHeight/mp_oTreePop->m_iSizeHeightDivs) > m_iEndHeightDiv)
            break;
        }
        p_oTree = p_oTree->GetTaller();
      } //end of while (p_oTree != NULL)
    } //end of for loops
  //If we don't have anything by now, return NULL
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
// FindFirstDHTree()
////////////////////////////////////////////////////////////////////////////
clTree* clTreeSearch::FindFirstDHTree() {
  clTree *p_oTree;
  float fHeight, fX, fY;
  //fHalfGrLen; //half of the grid length
  short int iX, iY, //grid cell coords, corrected for torus
  iOldX, iOldY, iDiv, //loop counters
  iGrLen;  //tree population grid length

  iGrLen = mp_oTreePop->m_iLengthGrids;
  //fHalfGrLen = iGrLen / 2.0;

  //Get the home grid cell of the target point
  m_iHomeX = (int)floor(m_fFromX / iGrLen);
  m_iHomeY = (int)floor(m_fFromY / iGrLen);

  //Get the starting height division based on the target height
  m_iStartHeightDiv = (int)floor(m_fHeightCutoff / mp_oTreePop->m_iSizeHeightDivs);
  if (m_iStartHeightDiv >= mp_oTreePop->m_iNumHeightDivs)
    m_iStartHeightDiv = mp_oTreePop->m_iNumHeightDivs - 1;

  //Figure out our min. and max. Y search cells - for each Y cell we loop
  //through, we'll find the min. and max. X for that cell
  //Shortcut check:
  //If the distance cutoff is greater than 1/2 the Y plot length, only use
  //that value so that we don't wrap
  if (m_fDistanceCutoff >= mp_oTreePop->m_fPlotLengthY / 2) {
    m_iMaxY = (int)floor((m_fFromY + mp_oTreePop->m_fPlotLengthY/2) / iGrLen);
    m_iMinY = (int)floor((m_fFromY - mp_oTreePop->m_fPlotLengthY/2) / iGrLen) + 1;
  } else {
    m_iMaxY = (int)ceil((m_fFromY + m_fDistanceCutoff) / iGrLen);
    m_iMinY = (int)floor((m_fFromY - m_fDistanceCutoff) / iGrLen);
  }

  //Move from min to max Y until we find a tree
  for (iOldY = m_iMinY; iOldY <= m_iMaxY; iOldY++) {

    //"Fix" the Y grid cell for torus, if need be
    if (iOldY < 0) iY = iOldY + mp_oTreePop->m_iNumYCells;
    else if (iOldY >= mp_oTreePop->m_iNumYCells)
      iY = iOldY - mp_oTreePop->m_iNumYCells;
    else iY = iOldY;

    GetMinMaxX(iOldY, iGrLen);

    for (iOldX = m_iMinX; iOldX <= m_iMaxX; iOldX++) {
      //"Fix" the X grid cell for torus, if needed
      if (iOldX < 0) iX = iOldX + mp_oTreePop->m_iNumXCells;
      else if (iOldX >= mp_oTreePop->m_iNumXCells)
        iX = iOldX - mp_oTreePop->m_iNumXCells;
      else iX = iOldX;

      //Now see if there's anything actually IN that cell
      //find the first tree in our list that's above the height cutoff
      p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][m_iStartHeightDiv];
      //if this height div is empty find one that isn't
      if (p_oTree == NULL) {
        for (iDiv = m_iStartHeightDiv + 1;
            iDiv < mp_oTreePop->m_iNumHeightDivs; iDiv++) {
          p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][iDiv];
          if (p_oTree != NULL)
            break;
        }
      }
      while (p_oTree) {
        unsigned int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();
        p_oTree->GetValue(mp_oTreePop->mp_iHeightCode[iSp][iTp], &fHeight);
        if (fHeight > m_fHeightCutoff) {
          p_oTree->GetValue(mp_oTreePop->mp_iXCode[iSp][iTp], &fX);
          p_oTree->GetValue(mp_oTreePop->mp_iYCode[iSp][iTp], &fY);
          if (mp_oPlot->GetDistance(m_fFromX, m_fFromY, fX, fY)
              <= m_fDistanceCutoff) {
            m_iCurrentXGrid = iOldX; m_iCurrentYGrid = iOldY;
            return p_oTree;
          }
        }
        p_oTree = p_oTree->GetTaller();
      } //end of while (p_oTree)
    } //end of for (iX = iMinXGridCell; iX <= iMaxXGridCell; iX++)
  } //end of for (iY = iMinYGridCell; iY <= iMaxYGridCell; iY++)

  //If we got to here, it meant that there were no trees to return
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
// FindNextDHTree()
////////////////////////////////////////////////////////////////////////////
clTree* clTreeSearch::FindNextDHTree() {
  clTree *p_oTree;
  float fHeight, fX, fY;
  //fHalfGrLen; //half of the grid length
  short int iX, iY, //grid cell coords, corrected for torus
  iOldX, iOldY, iDiv, //loop counters
  iGrLen;  //tree population grid length


  //First see if the current tree has a taller tree that works
  if (!mp_oCurrentTree) return NULL;
  else p_oTree = mp_oCurrentTree->GetTaller();
  if (p_oTree) {
    unsigned int iSp, iTp;
    while (p_oTree) {
      iSp = p_oTree->GetSpecies(); iTp = p_oTree->GetType();
      p_oTree->GetValue(mp_oTreePop->mp_iXCode[iSp][iTp], &fX);
      p_oTree->GetValue(mp_oTreePop->mp_iYCode[iSp][iTp], &fY);
      if (mp_oPlot->GetDistance(m_fFromX, m_fFromY, fX, fY)
          <= m_fDistanceCutoff)
        return p_oTree;
      p_oTree = p_oTree->GetTaller();
    }
  }

  iGrLen = mp_oTreePop->m_iLengthGrids;
  //fHalfGrLen = iGrLen / 2.0;

  //If we're still here, it means that there was nothing else in this grid
  //cell.  Thus, look for the next one.  Start by checking the X grids.
  if (m_iCurrentXGrid < m_iMaxX)
    for (iOldX = m_iCurrentXGrid + 1; iOldX <= m_iMaxX; iOldX++) {
      //Correct for torus
      if (iOldX < 0) iX = iOldX + mp_oTreePop->m_iNumXCells;
      else if (iOldX >= mp_oTreePop->m_iNumXCells)
        iX = iOldX - mp_oTreePop->m_iNumXCells;
      else iX = iOldX;
      if (m_iCurrentYGrid < 0)
        iY = m_iCurrentYGrid + mp_oTreePop->m_iNumYCells;
      else if (m_iCurrentYGrid >= mp_oTreePop->m_iNumYCells)
        iY = m_iCurrentYGrid - mp_oTreePop->m_iNumYCells;
      else iY = m_iCurrentYGrid;

      p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][m_iStartHeightDiv];
      //if this height div is empty find one that isn't
      if (p_oTree == NULL) {
        for (iDiv = m_iStartHeightDiv + 1;
            iDiv < mp_oTreePop->m_iNumHeightDivs; iDiv++) {
          p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][iDiv];
          if (p_oTree != NULL)
            break;
        }
      }
      while (p_oTree) {
        unsigned int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();
        p_oTree->GetValue(mp_oTreePop->mp_iHeightCode[iSp][iTp], &fHeight);
        if (fHeight > m_fHeightCutoff) {
          p_oTree->GetValue(mp_oTreePop->mp_iXCode[iSp][iTp], &fX);
          p_oTree->GetValue(mp_oTreePop->mp_iYCode[iSp][iTp], &fY);
          if (mp_oPlot->GetDistance(m_fFromX, m_fFromY, fX, fY)
              <= m_fDistanceCutoff) {
            m_iCurrentXGrid = iOldX;
            return p_oTree;
          }
        }
        p_oTree = p_oTree->GetTaller();
      } //end of while (p_oTree)
    } //end of for (iOldX = m_iCurrentXGrid + 1; iOldX < m_iMaxX; iOldX++)

  //Still nothing - or it's time to search the next Y grid
  iGrLen = mp_oTreePop->m_iLengthGrids;

  //Move from min to max Y until we find a tree
  for (iOldY = m_iCurrentYGrid + 1; iOldY <= m_iMaxY; iOldY++) {

    //"Fix" the Y grid cell for torus, if need be
    if (iOldY < 0) iY = iOldY + mp_oTreePop->m_iNumYCells;
    else if (iOldY >= mp_oTreePop->m_iNumYCells)
      iY = iOldY - mp_oTreePop->m_iNumYCells;
    else iY = iOldY;

    GetMinMaxX(iOldY, iGrLen);

    for (iOldX = m_iMinX; iOldX <= m_iMaxX; iOldX++) {
      //"Fix" the X grid cell for torus, if needed
      if (iOldX < 0) iX = iOldX + mp_oTreePop->m_iNumXCells;
      else if (iOldX >= mp_oTreePop->m_iNumXCells)
        iX = iOldX - mp_oTreePop->m_iNumXCells;
      else iX = iOldX;

      //Now see if there's anything actually IN that cell
      //find the first tree in our list that's above the height cutoff
      p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][m_iStartHeightDiv];
      //if this height div is empty find one that isn't
      if (p_oTree == NULL) {
        for (iDiv = m_iStartHeightDiv + 1;
            iDiv < mp_oTreePop->m_iNumHeightDivs; iDiv++) {
          p_oTree = mp_oTreePop->mp_oTreeShortest[iX][iY][iDiv];
          if (p_oTree != NULL)
            break;
        }
      }
      while (p_oTree) {
        unsigned int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();
        p_oTree->GetValue(mp_oTreePop->mp_iHeightCode[iSp][iTp], &fHeight);
        if (fHeight > m_fHeightCutoff) {
          p_oTree->GetValue(mp_oTreePop->mp_iXCode[iSp][iTp], &fX);
          p_oTree->GetValue(mp_oTreePop->mp_iYCode[iSp][iTp], &fY);
          if (mp_oPlot->GetDistance(m_fFromX, m_fFromY, fX, fY)
              <= m_fDistanceCutoff) {
            m_iCurrentXGrid = iOldX; m_iCurrentYGrid = iOldY;
            return p_oTree;
          }
        }
        p_oTree = p_oTree->GetTaller();
      } //end of while (p_oTree)
    } //end of for (iX = iMinXGridCell; iX <= iMaxXGridCell; iX++)
  } //end of for (iY = iMinYGridCell; iY <= iMaxYGridCell; iY++)

  //If we got to here, it meant that there were no trees to return
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
// GetMinMaxX()
////////////////////////////////////////////////////////////////////////////
void clTreeSearch::GetMinMaxX(const int &iY, const int &iGrLen) {
  //we know XY of center, and Y of point on edge of circle - solve for X
  //X2 = X1 - square root(distance^2 - (Y1 - Y2)^2)
  //If this is the home Y cell, then the X dist = the radius

  float fXDist, fYCellEdge, fMinX, fMaxX;
  //If Y is the home row, the search radius equals the distance
  if (iY == m_iHomeY) {
    fXDist = m_fDistanceCutoff;
  } else {
    //When we are south of the target Y, add one to grid cell number so
    //we'll be calculating the northern edge
    if (iY < m_iHomeY)
      fYCellEdge = (iY + 1) * iGrLen;
    else
      fYCellEdge = iY * iGrLen;

    fXDist = pow(m_fDistanceCutoff,2) - pow(m_fFromY - fYCellEdge,2);
    if (fXDist > 0)
      fXDist = sqrt(fXDist);
    else fXDist = 0;
  }

  //If the distance is greater than 1/2 X plot length, we need to search
  //the whole row of X cells.
  if (fXDist >= (mp_oTreePop->m_fPlotLengthX / 2)) {
    m_iMinX = 0;
    m_iMaxX = mp_oTreePop->GetNumXCells() - 1;
    return;
  }

  //Use actual points to figure the distance - that way we won't be stymied
  //by short end cells
  fMinX = m_fFromX - fXDist;
  fMaxX = m_fFromX + fXDist;

  m_iMaxX = (int)(mp_oPlot->CorrectX(fMaxX) / iGrLen) + 1;
  m_iMinX = (int)(mp_oPlot->CorrectX(fMinX) / iGrLen);

  //DECORRECT for torus
  if (fMinX < 0) m_iMinX -= mp_oTreePop->GetNumXCells();
  if (fMinX >= mp_oPlot->GetXPlotLength()) m_iMinX += mp_oTreePop->GetNumXCells();
  if (fMaxX < 0) m_iMaxX -= mp_oTreePop->GetNumXCells();
  if (fMaxX >= mp_oPlot->GetXPlotLength()) m_iMaxX += mp_oTreePop->GetNumXCells();
  if ((m_iMaxX - m_iMinX) >= mp_oTreePop->GetNumXCells()) {
    //It's the whole row
    m_iMinX = 0;
    m_iMaxX = mp_oTreePop->GetNumXCells() - 1;
  }
}
