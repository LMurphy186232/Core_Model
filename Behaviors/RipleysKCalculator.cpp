//---------------------------------------------------------------------------
// RipleysKCalculator.cpp
//---------------------------------------------------------------------------
#include "RipleysKCalculator.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "Constants.h"
#include "Plot.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clRipleysKCalculator::clRipleysKCalculator( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "RipleysK";
    m_sXMLRoot = "RipleysK";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    mp_fIncs = NULL;
    mp_iXCodes = NULL;
    mp_iYCodes = NULL;
    mp_fKValues = NULL;
    mp_bCellsSearched = NULL;
    mp_iGridCodes = NULL;

    m_fIncrement = 0;
    m_iNumXCells = 0;
    m_fMaxDistance = 0;
    m_iNumYCells = 0;
    mp_oGrid = NULL;
    m_iNumIncs = 0;
    m_iNumTotalSpecies = 0;
    m_iNumXToSearch = 0;
    m_iNumYToSearch = 0;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

  }
  catch ( modelErr & err )
  {
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
  	modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clRipleysKCalculator::clRipleysKCalculator" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clRipleysKCalculator::~clRipleysKCalculator()
{
  short int i, iNumTotCells = m_iNumXCells * m_iNumYCells;

  delete[] mp_fIncs;

  if ( mp_iXCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iXCodes[i];
  delete[] mp_iXCodes;

  if ( mp_iYCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iYCodes[i];
  delete[] mp_iYCodes;

  if (mp_fKValues)
    for (i = 0; i <= m_iNumTotalSpecies; i++)
      delete[] mp_fKValues[i];
  delete[] mp_fKValues;

  if (mp_bCellsSearched)
    for (i = 0; i < iNumTotCells; i++)
      delete[] mp_bCellsSearched[i];
  delete[] mp_bCellsSearched;

  if (mp_iGridCodes)
    for (i = 0; i <= m_iNumTotalSpecies; i++)
      delete[] mp_iGridCodes[i];
  delete[] mp_iGridCodes;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clRipleysKCalculator::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    m_iNumXCells = p_oPop->GetNumXCells();
    m_iNumYCells = p_oPop->GetNumYCells();

    GetParameterFileData(p_oDoc);
    GetTreeCodes(p_oPop);
    SetUpSearching(p_oPop);
    SetupGrid();

    //Calculate Ripley's K for the initial conditions
    Action();
  }
  catch ( modelErr & err )
  {
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clRipleysKCalculator::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
/////////////////////////////////////////////////////////////////////////////
void clRipleysKCalculator::GetParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

    //Get the max distance
    FillSingleValue( p_oElement, "an_RipleysKMaxDistance", &m_fMaxDistance, true );

    //Get the increment
    FillSingleValue( p_oElement, "an_RipleysKDistanceInc", &m_fIncrement, true );

    //Validate that the increment and max distance values are greater than 0
    if ( m_fMaxDistance <= 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clRipleysKCalculator::GetData" ;
      stcErr.sMoreInfo = "The max distance must be greater than zero.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
    if ( m_fIncrement <= 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clRipleysKCalculator::GetData" ;
      stcErr.sMoreInfo = "The distance increment must be greater than zero.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Verify that the increment is less than the max distance
    if (m_fMaxDistance <= m_fIncrement) {
      modelErr stcErr;
      stcErr.sFunction = "clRipleysKCalculator::GetData" ;
      stcErr.sMoreInfo = "The distance increment must be less than the max distance.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  }
  catch ( modelErr & err )
  {
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clRipleysKCalculator::GetParameterFileData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetTreeCodes()
/////////////////////////////////////////////////////////////////////////////
void clRipleysKCalculator::GetTreeCodes(clTreePopulation *p_oPop)
{
  int iNumTypes = 2,
      i, j;

  mp_iXCodes = new short int*[m_iNumTotalSpecies];
  mp_iYCodes = new short int*[m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_iXCodes[i] = new short int[iNumTypes];
    mp_iYCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iXCodes[i][j] = p_oPop->GetXCode(i, j + clTreePopulation::sapling);
      mp_iYCodes[i][j] = p_oPop->GetYCode(i, j + clTreePopulation::sapling);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clRipleysKCalculator::SetupGrid()
{
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  std::stringstream sLabel;
  short int i, j; //loop counters

  //Declare the arrays for our grid codes
  mp_iGridCodes = new short int*[m_iNumTotalSpecies + 1];
  for (i = 0; i < m_iNumTotalSpecies+1; i++) {
    mp_iGridCodes[i] = new short int[m_iNumIncs];
  }

  //Create the grid
  mp_oGrid = mp_oSimManager->CreateGrid( "Ripley's K",
                        0,                      //number of ints
                        (m_iNumIncs * (m_iNumTotalSpecies + 1)) + 2, //number of floats
                        0,                      //number of chars
                        0,                      //number of bools
                        p_oPlot->GetXPlotLength(),  //X cell length
                        p_oPlot->GetYPlotLength()); //Y cell length

  //Register increment and max distance
  mp_oGrid->RegisterFloat("inc");
  mp_oGrid->RegisterFloat("dist");

  //Register the data members
  for (i = 0; i < m_iNumTotalSpecies + 1; i++) {
    for (j = 0; j < m_iNumIncs; j++) {
      sLabel << j << "_" << i;
      mp_iGridCodes[i][j] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
  }

  //Set the value for max distance and increment
  mp_oGrid->SetValueOfCell(0, 0, mp_oGrid->GetFloatDataCode("inc"), (float)m_fIncrement);
  mp_oGrid->SetValueOfCell(0, 0, mp_oGrid->GetFloatDataCode("dist"), (float)m_fMaxDistance);
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clRipleysKCalculator::Action()
{
  float *p_fNumTrees = new float[m_iNumTotalSpecies+1];
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    clTree *p_oFirstTree; //looping variables
    float fPlotArea, //in square meters
          fValue; //grid values
    int iNumTotCells = m_iNumXCells * m_iNumYCells,
        iMinX, iMaxX, iMinY, iMaxY, //search limits
        iX, iY, iLoopX, iLoopY, iNeighX, iNeighY, //for looping through cells
        i, j;

    //Reset the values in the Ripley's K grid to 0
    fValue = 0;
    for (i = 0; i < m_iNumTotalSpecies + 1; i++) {
      for (j = 0; j < m_iNumIncs; j++) {
        mp_oGrid->SetValueOfCell(0, 0, mp_iGridCodes[i][j], fValue);
      }
    }

    //Initialize the Ripley's K calculation array
    for (i = 0; i < m_iNumTotalSpecies + 1; i++) {
      p_fNumTrees[i] = 0;
      for (j = 0; j < m_iNumIncs; j++)
        mp_fKValues[i][j] = 0;
    }

    //Reset the grid search array
    for (i = 0; i < iNumTotCells; i++) {
      for (j = 0; j < iNumTotCells; j++) {
        mp_bCellsSearched[i][j] = false;
      }
    }

    for (iX = 0; iX < m_iNumXCells; iX++) {
      for (iY = 0; iY < m_iNumYCells; iY++) {

        //Find our starting tree in this cell - we'll have to loop over
        //it several times
        p_oFirstTree = p_oPop->GetShortestTreeInCell(iX, iY);
        if (p_oFirstTree)
          while (p_oFirstTree != NULL &&
                 clTreePopulation::seedling == p_oFirstTree->GetType())
            p_oFirstTree = p_oFirstTree->GetTaller();
        if (p_oFirstTree) {

          //Get the extent of the area to search
          iMinX = iX - m_iNumXToSearch;
          iMaxX = iX + m_iNumXToSearch;
          iMinY = iY - m_iNumYToSearch;
          iMaxY = iY + m_iNumYToSearch;

          //Process this cell
          ProcessOwnCell(p_oFirstTree, p_fNumTrees, iX, iY);

          //Do the search
          for (iLoopX = iMinX; iLoopX <= iMaxX; iLoopX++) {
            for (iLoopY = iMinY; iLoopY <= iMaxY; iLoopY++) {

              //Get the neighbor cell indexes corrected for torus geometry
              iNeighX = iLoopX >= m_iNumXCells ? iLoopX - m_iNumXCells : iLoopX;
              iNeighX = iNeighX < 0 ? iNeighX + m_iNumXCells : iNeighX;
              iNeighY = iLoopY >= m_iNumYCells ? iLoopY - m_iNumYCells : iLoopY;
              iNeighY = iNeighY < 0 ? iNeighY + m_iNumYCells : iNeighY;

              //Continue if the cell pair has not already been searched and
              //is worth searching
              if (!mp_bCellsSearched[(iX * m_iNumYCells) + iY]
                                    [(iNeighX * m_iNumYCells) + iNeighY]) {
                ProcessCell(p_oPop, p_oFirstTree, iX, iY, iNeighX, iNeighY);
              }
            }
          }
        }
      }
    }

    //Calculate the K values
    fPlotArea = p_oPlot->GetPlotArea() * M_SQ_PER_HA; //get area in sq m
    for (i = 0; i <= m_iNumTotalSpecies; i++) {
      if (p_fNumTrees[i] > 0) {
        p_fNumTrees[i] *= p_fNumTrees[i]; //square this value
        for (j = 0; j < m_iNumIncs; j++) {
          mp_fKValues[i][j] = (mp_fKValues[i][j] * fPlotArea) / p_fNumTrees[i];
        }
      }
    }

    //Set the K values into the grid
    for (i = 0; i <= m_iNumTotalSpecies; i++) {
      for (j = 0; j < m_iNumIncs; j++) {
        mp_oGrid->SetValueOfCell(0, 0, mp_iGridCodes[i][j], mp_fKValues[i][j]);
      }
    }

    delete[] p_fNumTrees;
  }
  catch ( modelErr & err )
  {
    delete[] p_fNumTrees;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fNumTrees;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fNumTrees;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clRipleysKCalculator::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ProcessCell()
////////////////////////////////////////////////////////////////////////////
void clRipleysKCalculator::ProcessCell(clTreePopulation *p_oPop,
  clTree *p_oFirstTree, const int &iX, const int &iY, const int &iNeighX,
  const int &iNeighY)
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  clTree *p_oFirstNeighbor, *p_oNeighbor, *p_oTree;
  float fX, fY, fNeighX, fNeighY, fDistance;
  int iSp, iTp, iNeighSp, iNeighTp, i;

  //Find our starting tree in this cell - we'll have to loop over
  //it several times
  p_oFirstNeighbor = p_oPop->GetShortestTreeInCell(iNeighX, iNeighY);
    if (p_oFirstNeighbor)
      while (p_oFirstNeighbor != NULL &&
             clTreePopulation::seedling == p_oFirstNeighbor->GetType())
        p_oFirstNeighbor = p_oFirstNeighbor->GetTaller();

  if (p_oFirstNeighbor) {
    //Pair up each tree in the two cells
    p_oTree = p_oFirstTree;
    while (p_oTree) {
      iTp = p_oTree->GetType();
      if (clTreePopulation::sapling == iTp ||
          clTreePopulation::adult == iTp) {
        iSp = p_oTree->GetSpecies();
        iTp -= clTreePopulation::sapling;
        p_oTree->GetValue(mp_iXCodes[iSp][iTp], &fX);
        p_oTree->GetValue(mp_iYCodes[iSp][iTp], &fY);

        p_oNeighbor = p_oFirstNeighbor;
        while (p_oNeighbor) {
          iNeighTp = p_oNeighbor->GetType();
          if (clTreePopulation::sapling == iNeighTp ||
              clTreePopulation::adult == iNeighTp) {
            iNeighSp = p_oNeighbor->GetSpecies();
            iNeighTp -= clTreePopulation::sapling;
            p_oNeighbor->GetValue(mp_iXCodes[iNeighSp][iNeighTp], &fNeighX);
            p_oNeighbor->GetValue(mp_iYCodes[iNeighSp][iNeighTp], &fNeighY);

            fDistance = p_oPlot->GetDistance(fX, fY, fNeighX, fNeighY);

            //Count this tree pair into all of the appropriate buckets
            i = m_iNumIncs - 1;
            while (i >= 0 && mp_fIncs[i] >= fDistance) {
              mp_fKValues[m_iNumTotalSpecies][i]++;
              i--;
            }

            //If they are of the same species, do it again with their
            //species bucket
            if (iSp == iNeighSp) {
              i = m_iNumIncs - 1;
              while (i >= 0 && mp_fIncs[i] >= fDistance) {
                mp_fKValues[iSp][i]++;
                i--;
              }
            }
          }
          p_oNeighbor = p_oNeighbor->GetTaller();
        }
      }
      p_oTree = p_oTree->GetTaller();
    }
  }

  //Set the flags on the grid cell search matrix to true
  mp_bCellsSearched[(iX * m_iNumYCells) + iY][(iNeighX * m_iNumYCells) + iNeighY] = true;
  mp_bCellsSearched[(iNeighX * m_iNumYCells) + iNeighY][(iX * m_iNumYCells) + iY] = true;
}

////////////////////////////////////////////////////////////////////////////
// ProcessOwnCell()
////////////////////////////////////////////////////////////////////////////
void clRipleysKCalculator::ProcessOwnCell(clTree *p_oFirstTree,
float *p_fNumTrees, const int &iX, const int &iY)
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  clTree *p_oNeighbor, *p_oTree;
  float fX, fY, fNeighX, fNeighY, fDistance;
  int iSp, iTp, iNeighSp, iNeighTp, i;

  p_oTree = p_oFirstTree;
  while (p_oTree) {
    iTp = p_oTree->GetType();
    if (clTreePopulation::sapling == iTp ||
        clTreePopulation::adult == iTp) {

       iSp = p_oTree->GetSpecies();
       iTp -= clTreePopulation::sapling;
       p_oTree->GetValue(mp_iXCodes[iSp][iTp], &fX);
       p_oTree->GetValue(mp_iYCodes[iSp][iTp], &fY);

      //Count the trees in this cell
      p_fNumTrees[m_iNumTotalSpecies]++;
      p_fNumTrees[iSp]++;

      p_oNeighbor = p_oTree->GetTaller();
      while (p_oNeighbor) {
        iNeighTp = p_oNeighbor->GetType();
        if (clTreePopulation::sapling == iNeighTp ||
            clTreePopulation::adult == iNeighTp) {
          iNeighSp = p_oNeighbor->GetSpecies();
          iNeighTp -= clTreePopulation::sapling;
          p_oNeighbor->GetValue(mp_iXCodes[iNeighSp][iNeighTp], &fNeighX);
          p_oNeighbor->GetValue(mp_iYCodes[iNeighSp][iNeighTp], &fNeighY);

          fDistance = p_oPlot->GetDistance(fX, fY, fNeighX, fNeighY);

          //Count this tree pair into all of the appropriate buckets
          i = m_iNumIncs - 1;
          while (i >= 0 && mp_fIncs[i] >= fDistance) {
            mp_fKValues[m_iNumTotalSpecies][i]++;
            i--;
          }

          //If they are of the same species, do it again with their
          //species bucket
          if (iSp == iNeighSp) {
            i = m_iNumIncs - 1;
            while (i >= 0 && mp_fIncs[i] >= fDistance) {
              mp_fKValues[iSp][i]++;
              i--;
            }
          }
        }

        p_oNeighbor = p_oNeighbor->GetTaller();
      }
    }
    p_oTree = p_oTree->GetTaller();
  }

  mp_bCellsSearched[(iX * m_iNumYCells) + iY][(iX * m_iNumYCells) + iY] = true;
}

////////////////////////////////////////////////////////////////////////////
// SetUpSearching()
////////////////////////////////////////////////////////////////////////////
void clRipleysKCalculator::SetUpSearching(clTreePopulation *p_oPop)
{
  int iCellSize = p_oPop->GetGridCellSize(),
      iNumTotCells = m_iNumXCells * m_iNumYCells,
      i,
      iLimit,           //effective search limit
      iSearchDistance;  //search distance

  m_iNumIncs = (int)ceil(m_fMaxDistance / m_fIncrement);
  mp_fIncs = new float[m_iNumIncs];
  mp_fIncs[0] = m_fIncrement;
  for (i = 1; i < m_iNumIncs; i++) {
    mp_fIncs[i] = mp_fIncs[i-1] + m_fIncrement;
  }

  //Initialize the Ripley's K calculation array
  mp_fKValues = new float*[m_iNumTotalSpecies + 1];
  for (i = 0; i < m_iNumTotalSpecies + 1; i++) {
    mp_fKValues[i] = new float[m_iNumIncs];
  }

  mp_bCellsSearched = new bool*[iNumTotCells];
  for (i = 0; i < iNumTotCells; i++)
    mp_bCellsSearched[i] = new bool[iNumTotCells];

  //Find the search distance in number of cells
  iSearchDistance = (int)ceil(m_fMaxDistance / iCellSize);

  //Find the effective X axis search limit - half the number of plot cells
  iLimit = (int)ceil((float)(m_iNumXCells / 2));

  m_iNumXToSearch = iSearchDistance > iLimit ? iLimit : iSearchDistance;

  //Repeat with Y
  iLimit = (int)ceil((float)(m_iNumYCells / 2));
  m_iNumYToSearch = iSearchDistance > iLimit ? iLimit : iSearchDistance;
}
