#include <stddef.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <string>
#include "TreePopulation.h"
#include "Plot.h"
#include "Allometry.h"
#include "Tree.h"
#include "SimManager.h"
#include "ModelMath.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "PlatformFuncs.h"
#include "GhostTreePopulation.h"

#define MINDIAM 0.001

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clTreePopulation::clTreePopulation(clSimManager * p_oSimManager) :
          clWorkerBase(p_oSimManager), clPopulationBase(p_oSimManager) {
  try
  {
    //This stuff can be set here because it won't change
    m_sNameString = "treepopulation";
    //         m_iNumHeightDivs = 10;
    m_iSizeHeightDivs = 4;
    m_iLengthGrids = 8;
    m_iNumTypes = 7;


    m_fNewSeedlingDiam10 = 0.1;
    m_fMinAdultHeight = 0;
    m_fMaxSaplingHeight = 0;
    m_fPlotLengthX = 0;
    m_fPlotLengthY = 0;
    m_iNumHeightDivs = 0;
    m_iNumSpecies = 0;
    m_iNumXCells = 0;
    m_iNumYCells = 0;
    m_iNumSizeClasses = 0;
    m_bMakeSnag = false;
    m_bDoUpdates = false;

    //Null our pointers
    mp_openSearches = NULL;
    mp_fMinAdultDbh = NULL;
    mp_fMaxSeedlingHeight = NULL;
    mp_fSizeClasses = NULL;
    mp_speciesCodes = NULL;
    mp_oAllom = NULL;
    mp_oTreeTallest = NULL;
    mp_oTreeShortest = NULL;
    mp_oStumps = NULL;
    mp_oGhosts = NULL;
    mp_bMakeStump = NULL;
    m_iNumXCells = 0;
    m_iNumYCells = 0;

    mp_iNumTreeIntVals = NULL;
    mp_iNumTreeFloatVals = NULL;
    mp_iNumTreeStringVals = NULL;
    mp_iNumTreeBoolVals = NULL;
    mp_sIntLabels = NULL;
    mp_sFloatLabels = NULL;
    mp_sStringLabels = NULL;
    mp_sBoolLabels = NULL;
    mp_iXCode = NULL;
    mp_iYCode = NULL;
    mp_iHeightCode = NULL;
    mp_iDiam10Code = NULL;
    mp_iDbhCode = NULL;
    mp_iCrownRadCode = NULL;
    mp_iCrownDepthCode = NULL;
    mp_iAgeCode = NULL;
    mp_iWhyDeadCode = NULL;

    mp_openSearches = NULL;
    stcOpenSearches * p_dummyRecord;
    //Make a dummy record for the first record in the tree results list
    //This way the record list is always accessed in the same way by everyone
    p_dummyRecord = new stcOpenSearches;
    p_dummyRecord->p_oSearch = NULL;
    p_dummyRecord->p_nextSearch = NULL;
    mp_openSearches = p_dummyRecord;

    //Allowed file types
    m_iNumAllowedTypes = 5;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = tree;
    mp_iAllowedFileTypes[2] = treemap;
    mp_iAllowedFileTypes[3] = detailed_output_timestep;
    mp_iAllowedFileTypes[4] = detailed_output;
  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::clTreePopulation" ;
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clTreePopulation::~clTreePopulation() {
  int i, j;

  TimestepCleanup();
  EmptyHashTable();

  //Delete the hash table, if it hasn't been already
  if (mp_oTreeShortest) {
    for (i = 0; i < m_iNumXCells; i++) {
      for (j = 0; j < m_iNumYCells; j++) {
        delete[] mp_oTreeTallest[i][j];
        mp_oTreeTallest[i][j] = NULL;
        delete[] mp_oTreeShortest[i][j];
        mp_oTreeShortest[i][j] = NULL;
      } //end of for (int j = 0; j < m_iNumYCells; j++)
    }
    for (i = 0; i < m_iNumXCells; i++) {
      delete[] mp_oTreeTallest[i];
      mp_oTreeTallest[i] = NULL;
      delete[] mp_oTreeShortest[i];
      mp_oTreeShortest[i] = NULL;
    } //end of for (int i = 0; i < m_iNumXCells; i++)
    delete[] mp_oTreeTallest;
    mp_oTreeTallest = NULL;
    delete[] mp_oTreeShortest;
    mp_oTreeShortest = NULL;
  }

  //Delete other arrays
  delete[] mp_fMinAdultDbh;
  mp_fMinAdultDbh = NULL;
  delete[] mp_fMaxSeedlingHeight;
  mp_fMaxSeedlingHeight = NULL;
  delete[] mp_speciesCodes;
  mp_speciesCodes = NULL;
  delete[] mp_fSizeClasses;
  mp_fSizeClasses = NULL;
  delete[] mp_bMakeStump;
  mp_bMakeStump = NULL;

  if (mp_sIntLabels) { //int labels
    for (i = 0; i < m_iNumSpecies; i++) {
      for (j = 0; j < m_iNumTypes; j++) {
        delete[] mp_sIntLabels[i][j];
        mp_sIntLabels[i][j] = NULL;
      }
      delete[] mp_sIntLabels[i];
      mp_sIntLabels[i] = NULL;
    }
    delete[] mp_sIntLabels;
    mp_sIntLabels = NULL;
  }
  if (mp_sFloatLabels) { //float labels
    for (i = 0; i < m_iNumSpecies; i++) {
      for (j = 0; j < m_iNumTypes; j++) {
        delete[] mp_sFloatLabels[i][j];
        mp_sFloatLabels[i][j] = NULL;
      }
      delete[] mp_sFloatLabels[i];
      mp_sFloatLabels[i] = NULL;
    }
    delete[] mp_sFloatLabels;
    mp_sFloatLabels = NULL;
  }
  if (mp_sStringLabels) { //char labels
    for (i = 0; i < m_iNumSpecies; i++) {
      for (j = 0; j < m_iNumTypes; j++) {
        delete[] mp_sStringLabels[i][j];
        mp_sStringLabels[i][j] = NULL;
      }
      delete[] mp_sStringLabels[i];
      mp_sStringLabels[i] = NULL;
    }
    delete[] mp_sStringLabels;
    mp_sStringLabels = NULL;
  }
  if (mp_sBoolLabels) { //bool labels
    for (i = 0; i < m_iNumSpecies; i++) {
      for (j = 0; j < m_iNumTypes; j++) {
        delete[] mp_sBoolLabels[i][j];
        mp_sBoolLabels[i][j] = NULL;
      }
      delete[] mp_sBoolLabels[i];
      mp_sBoolLabels[i] = NULL;
    }
    delete[] mp_sBoolLabels;
    mp_sBoolLabels = NULL;
  }

  //Tree data member arrays
  for (i = 0; i < m_iNumSpecies; i++) {
    if (mp_iNumTreeIntVals) {
      delete[] mp_iNumTreeIntVals[i];
      mp_iNumTreeIntVals[i] = NULL;
    }
    if (mp_iNumTreeFloatVals) {
      delete[] mp_iNumTreeFloatVals[i];
      mp_iNumTreeFloatVals[i] = NULL;
    }
    if (mp_iNumTreeStringVals) {
      delete[] mp_iNumTreeStringVals[i];
      mp_iNumTreeStringVals[i] = NULL;
    }
    if (mp_iNumTreeBoolVals) {
      delete[] mp_iNumTreeBoolVals[i];
      mp_iNumTreeBoolVals[i] = NULL;
    }
    if (mp_iXCode) {
      delete[] mp_iXCode[i];
      mp_iXCode[i] = NULL;
    }
    if (mp_iYCode) {
      delete[] mp_iYCode[i];
      mp_iYCode[i] = NULL;
    }
    if (mp_iHeightCode) {
      delete[] mp_iHeightCode[i];
      mp_iHeightCode[i] = NULL;
    }
    if (mp_iDiam10Code) {
      delete[] mp_iDiam10Code[i];
      mp_iDiam10Code[i] = NULL;
    }
    if (mp_iDbhCode) {
      delete[] mp_iDbhCode[i];
      mp_iDbhCode[i] = NULL;
    }
    if (mp_iCrownRadCode) {
      delete[] mp_iCrownRadCode[i];
      mp_iCrownRadCode[i] = NULL;
    }
    if (mp_iCrownDepthCode) {
      delete[] mp_iCrownDepthCode[i];
      mp_iCrownDepthCode[i] = NULL;
    }
  } //end of for (i = 0; i < m_iNumSpecies; i++)

  if (mp_iNumTreeIntVals) {
    delete[] mp_iNumTreeIntVals;
    mp_iNumTreeIntVals = NULL;
  }
  if (mp_iNumTreeFloatVals) {
    delete[] mp_iNumTreeFloatVals;
    mp_iNumTreeFloatVals = NULL;
  }
  if (mp_iNumTreeStringVals) {
    delete[] mp_iNumTreeStringVals;
    mp_iNumTreeStringVals = NULL;
  }
  if (mp_iNumTreeBoolVals) {
    delete[] mp_iNumTreeBoolVals;
    mp_iNumTreeBoolVals = NULL;
  }
  if (mp_iXCode) {
    delete[] mp_iXCode;
    mp_iXCode = NULL;
  }
  if (mp_iYCode) {
    delete[] mp_iYCode;
    mp_iYCode = NULL;
  }
  if (mp_iHeightCode) {
    delete[] mp_iHeightCode;
    mp_iHeightCode = NULL;
  }
  if (mp_iDiam10Code) {
    delete[] mp_iDiam10Code;
    mp_iDiam10Code = NULL;
  }
  if (mp_iDbhCode) {
    delete[] mp_iDbhCode;
    mp_iDbhCode = NULL;
  }
  if (mp_iCrownRadCode) {
    delete[] mp_iCrownRadCode;
    mp_iCrownRadCode = NULL;
  }
  if (mp_iCrownDepthCode) {
    delete[] mp_iCrownDepthCode;
    mp_iCrownDepthCode = NULL;
  }
  if (mp_iAgeCode) {
    delete[] mp_iAgeCode;
    mp_iAgeCode = NULL;
  }
  if (mp_iWhyDeadCode) {
    delete[] mp_iWhyDeadCode;
    mp_iWhyDeadCode = NULL;
  }

  //Delete the tree search results dummy (the records should have been deleted
  //during TimestepCleanup())
  delete mp_openSearches;
  mp_openSearches = NULL;
  delete mp_oAllom;
  mp_oAllom = NULL;

  //NULL out clTree's static tree population pointer
  clTree::mp_oTreePop = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// GetRandomDiam10Value
/////////////////////////////////////////////////////////////////////////////
float clTreePopulation::GetRandomDiam10Value(float fDiam10Seed) {
  float fRandom = clModelMath::GetRand();
  //I couldn't make the default assignment to m_fNewSeedlingDiam10 in the
  //arguments list without compiler errors so I'm doing it here.
  if (fDiam10Seed == 0)
    fDiam10Seed = m_fNewSeedlingDiam10;
  return (fDiam10Seed * (0.9995 + 0.001 * fRandom));
}

/////////////////////////////////////////////////////////////////////////////
// GetShortestTreeInCell
/////////////////////////////////////////////////////////////////////////////
clTree * clTreePopulation::GetShortestTreeInCell(int iX, int iY) {
  clTree * p_oTree = NULL;
  short int i;

  if (iX < 0 || iX >= m_iNumXCells || iY < 0 || iY >= m_iNumYCells)
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetShortestTreeInCell" ;
    std::stringstream s;
    s << "Invalid grid numbers.  X = " << iX << ", Y = " << iY;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }

  for (i = 0; i < m_iNumHeightDivs; i++)
  {
    p_oTree = mp_oTreeShortest[iX][iY][i];
    if (p_oTree) return p_oTree;
  }

  return p_oTree;
}

/////////////////////////////////////////////////////////////////////////////
// GetTallestTreeInCell
/////////////////////////////////////////////////////////////////////////////
clTree * clTreePopulation::GetTallestTreeInCell(int iX, int iY) {
  clTree * p_oTree = NULL;
  short int i;

  if (iX < 0 || iX >= m_iNumXCells || iY < 0 || iY >= m_iNumYCells)
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetTallestTreeInCell" ;
    std::stringstream s;
    s << "Invalid grid numbers.  X = " << iX << ", Y = " << iY;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }

  for (i = m_iNumHeightDivs - 1; i >= 0; i--)
  {
    p_oTree = mp_oTreeTallest[iX][iY][i];
    if (p_oTree) return p_oTree;
  }

  return p_oTree;
}


//////////////////////////////////////////////////////////////////////////////
// BarebonesDataSetup()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::BarebonesDataSetup(DOMDocument * p_oDoc) {
  try
  {
    DOMNodeList * p_oNodeList; //for retrieving tag name searches
    DOMNode * p_oDocNode; //for getting individual tag nodes
    DOMElement * p_oElement; //for casting to the DOMElement class
    XMLCh *sTag;
    char * cData;
    int iNumNodes, i;

    sTag = XMLString::transcode("tr_speciesList");
    p_oNodeList = p_oDoc->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    if (0 == p_oNodeList->getLength())
    {
      modelErr stcErr;
      stcErr.sFunction = "clTreePopulation::BarebonesDataSetup" ;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "tr_speciesList";
      throw(stcErr);
    }
    //get all the "species" elements in the list
    p_oDocNode = p_oNodeList->item(0);
    p_oElement = (DOMElement *) p_oDocNode; //cast to DOMElement object
    sTag = XMLString::transcode("tr_species");
    p_oNodeList = p_oElement->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes)
    {
      modelErr stcErr;
      stcErr.sFunction = "clTreePopulation::BarebonesDataSetup" ;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "tr_species";
      throw(stcErr);
    }

    //declare an array to hold the code structs.  The species codes will be
    //integers counting up from 0
    m_iNumSpecies = iNumNodes;

    mp_speciesCodes = new speciesCodes[iNumNodes];
    for (i = 0; i < iNumNodes; i++)
    {
      mp_speciesCodes[i].iCode = i;
      //Get the next item on the list and cast to an element object so we can
      //get its attribute
      p_oDocNode = p_oNodeList->item(i);
      p_oElement = (DOMElement *) p_oDocNode;
      //Get the attribute object - its value is the species string
      sTag = XMLString::transcode("speciesName");
      cData = XMLString::transcode(p_oElement->getAttributeNode(sTag)->getValue());
      XMLString::release(&sTag);
      mp_speciesCodes[i].sName = cData;
      delete[] cData;
    }

    //Get the ghost population link
    mp_oGhosts = (clGhostTreePopulation*)mp_oSimManager->GetPopulationObject("GhostTreePopulation");
  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::BarebonesDataSetup" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetPlotDimensions()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::GetPlotDimensions() {
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  //If unsuccessful - error
  if (!p_oPlot) {
    modelErr stcErr;
    stcErr.sFunction = "clTreePopulation::GetData";
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    stcErr.sMoreInfo = "clPlot";
    throw(stcErr);
  }
  m_fPlotLengthX = p_oPlot->GetXPlotLength();
  m_fPlotLengthY = p_oPlot->GetYPlotLength();
}

//////////////////////////////////////////////////////////////////////////////
// SetupCalculations()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::SetupCalculations() {
  float fMaxHeight, //maximum tree height
  fTemp; //for converting data to float
  int i;

  //Calculate the minimum possible adult height
  fTemp = mp_oAllom->CalcAdultHeight(mp_fMinAdultDbh[0], 0);
  m_fMinAdultHeight = fTemp;
  for (i = 1; i < m_iNumSpecies; i++) {
    fTemp = mp_oAllom->CalcAdultHeight(mp_fMinAdultDbh[i], i);
    if (fTemp < m_fMinAdultHeight)
      m_fMinAdultHeight = fTemp;
  }

  //Calculate the maximum possible sapling height
  fTemp = mp_oAllom->CalcSaplingHeight(mp_fMinAdultDbh[0], 0);
  m_fMaxSaplingHeight = fTemp;
  for (i = 1; i < m_iNumSpecies; i++) {
    fTemp = mp_oAllom->CalcSaplingHeight(mp_fMinAdultDbh[i], i);
    if (fTemp > m_fMaxSaplingHeight)
      m_fMaxSaplingHeight = fTemp;
  }

  //Calculate the number of height divisions
  //Get tallest possible tree
  fMaxHeight = mp_oAllom->GetMaxTreeHeight(0);
  for (i = 1; i < m_iNumSpecies; i++) {
    fTemp = mp_oAllom->GetMaxTreeHeight(i);
    if (fTemp > fMaxHeight)
      fMaxHeight = fTemp;
  }

  m_iNumHeightDivs = (int)ceil(fMaxHeight / m_iSizeHeightDivs);

}

//////////////////////////////////////////////////////////////////////////////
// CreateHashTable()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::CreateHashTable() {

  //Calculate how many grid cells we will have.  Then give each grid cell
  //the proper number of height subdivisions
  m_iNumXCells = (int)ceil(m_fPlotLengthX / m_iLengthGrids);
  m_iNumYCells = (int)ceil(m_fPlotLengthY / m_iLengthGrids);
  mp_oTreeTallest = new clTree * * * [m_iNumXCells];
  mp_oTreeShortest = new clTree * * * [m_iNumXCells];
  for (int i = 0; i < m_iNumXCells; i++) {
    mp_oTreeTallest[i] = new clTree * * [m_iNumYCells];
    mp_oTreeShortest[i] = new clTree * * [m_iNumYCells];
    for (int j = 0; j < m_iNumYCells; j++) {
      mp_oTreeTallest[i][j] = new clTree * [m_iNumHeightDivs];
      mp_oTreeShortest[i][j] = new clTree * [m_iNumHeightDivs];
      for (int k = 0; k < m_iNumHeightDivs; k++) {
        mp_oTreeTallest[i][j][k] = NULL;
        mp_oTreeShortest[i][j][k] = NULL;
      }
    }
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::GetData(DOMDocument * p_oDoc) {
  try
  {
    int iFileCode; //integer version of the file code

    //If this is a tree file, a detailed output file, or a tree map file, and a
    //parameter file has been loaded, jump to the tree creating part
    iFileCode = mp_oSimManager->GetFileType(p_oDoc);
    if (parfile == iFileCode || detailed_output == iFileCode)
    {
      mp_oAllom = new clAllometry(this);
      GetPlotDimensions();
      ReadParameters(p_oDoc);
      SetupCalculations();
      CreateHashTable();
      DoTreeDataStructureSetup();
      DataMemberRegistrations();
    }
    CreateTreesFromInitialDensities(p_oDoc);
    CreateTreesFromTreeMap(p_oDoc);
    CreateTreesFromTextTreeMap(p_oDoc);

    m_bDoUpdates = false;

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::GetData" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// ReadParameters()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::ReadParameters(DOMDocument * p_oDoc) {
  try
  {

    DOMNodeList * p_oNodeList; //for retrieving tag name searches
    DOMNode * p_oDocNode; //for getting individual tag nodes
    DOMElement * p_oElement; //for casting to the DOMElement class
    XMLCh *sVal;
    char *cData;
    std::string strData; //for extracting text data from XML nodes
    float fTemp; //for converting data to float
    int iNumNodes, //for counting the results of tag name searches
    i, j; //loop counters

    //Allocate memory for the species-specific arrays
    mp_fMinAdultDbh = new double[m_iNumSpecies];
    mp_fMaxSeedlingHeight = new double[m_iNumSpecies];

    //Get the values from the XML document
    //Get the parent node
    sVal = XMLString::transcode("trees");
    p_oNodeList = p_oDoc->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength())
    {
      modelErr stcErr;
      stcErr.sFunction = "clTreePopulation::GetData" ;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "trees";
      throw(stcErr);
    }
    p_oDocNode = p_oNodeList->item(0);
    p_oElement = (DOMElement *) p_oDocNode;
    FillSpeciesSpecificValue(p_oElement, "tr_minAdultDBH", "tr_madVal", mp_fMinAdultDbh, this, true);
    FillSpeciesSpecificValue(p_oElement, "tr_maxSeedlingHeight", "tr_mshVal", mp_fMaxSeedlingHeight, this, true);
    FillSingleValue(p_oElement, "tr_seedDiam10Cm", & m_fNewSeedlingDiam10, true);

    //Size classes - not required
    //Get number of size classes
    sVal = XMLString::transcode("tr_sizeClasses");
    p_oNodeList = p_oDoc->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength())
    { //i.e. there are size classes defined
      p_oDocNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oDocNode;
      sVal = XMLString::transcode("tr_sizeClass");
      p_oNodeList = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumNodes = p_oNodeList->getLength();
      m_iNumSizeClasses = iNumNodes;
      if (iNumNodes > 0)
      {
        //we need to pre-authenticate all the data so we size our array to
        //only include good data - i.e. numbers
        for (i = 0; i < iNumNodes; i++)
        {
          p_oDocNode = p_oNodeList->item(i);
          p_oElement = (DOMElement *) p_oDocNode;
          sVal = XMLString::transcode("sizeKey");
          cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
          strData = cData;
          XMLString::release(&sVal); delete[] cData; cData = NULL;
          if (strData.length() > 0)
          {
            //check to see if it's "Seed" or "Seedling" - if it is, it's OK
            if ("Seed" != strData && "Seedling" != strData)
            {
              //erase the first character-should be a letter
              strData.erase(0, 1);
              fTemp = atof(strData.c_str());
              if (0 == fTemp)
                m_iNumSizeClasses--;
            } //end of if (strData.length() > 0)
            else
              m_iNumSizeClasses--;
          } //end of if ("seed" != strData && "seedling" != strData)
        } //end of for (i = 0; i < iNumNodes; i++)

        //Size classes are authenticated - create and fill array
        if (m_iNumSizeClasses > 0)
        {
          mp_fSizeClasses = new float[m_iNumSizeClasses];
          j = 0;
          for (i = 0; i < iNumNodes; i++)
          {
            p_oDocNode = p_oNodeList->item(i);
            p_oElement = (DOMElement *) p_oDocNode;
            sVal = XMLString::transcode("sizeKey");
            cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
            strData = cData;
            XMLString::release(&sVal); delete[] cData; cData = NULL;
            if ("Seed" == strData || "Seedling" == strData) {;} //do nothing - don't add
            else if (strData.length() > 0)
            {
              //erase first character - should be a letter
              strData.erase(0, 1);
              fTemp = atof(strData.c_str());
              if (fTemp > 0)
              {
                mp_fSizeClasses[j] = fTemp;
                j++;
              }
            }
          } //end of for (i = 0; i < iNumNodes; i++)

          //Now sort the size classes into ascending order
          //p_fTempSizeClasses = new float[m_iNumSizeClasses];
          //for (i = 0; i <
          std::sort(mp_fSizeClasses, mp_fSizeClasses + m_iNumSizeClasses);
        } //end of if (m_iNumSizeClasses > 0)
      } //end of if (iNumNodes > 0)
    } //end of if (0 != p_oNodeList->getLength())

    mp_oAllom->GetData(p_oDoc, this);

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::ReadParameters" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// DoTreeDataStructureSetup()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::DoTreeDataStructureSetup() {
  try
  {
    clBehaviorBase * p_oBehavior;
    struct stcSpeciesTypeCombo combo; //one species/type combo
    short int i, j, //loop counters
    iNumBehaviors, //number of behaviors to loop through
    iNumCombos, //number type/species combos for a behavior
    iNewInts, //the number of new int values to add for a data type
    iNewFloats, //the number of new float values to add for a data type
    iNewChars, //the number of new char values to add for a data type
    iNewBools; //the number of new bool values to add for a data type

    m_bMakeSnag = false;
    //Declare the value arrays - size them species by type
    mp_bMakeStump = new bool[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++)
      mp_bMakeStump[i] = false;

    mp_iNumTreeIntVals = new short int*[m_iNumSpecies];
    mp_iNumTreeFloatVals = new short int*[m_iNumSpecies];
    mp_iNumTreeStringVals = new short int*[m_iNumSpecies];
    mp_iNumTreeBoolVals = new short int*[m_iNumSpecies];
    mp_iXCode = new short int*[m_iNumSpecies];
    mp_iYCode = new short int*[m_iNumSpecies];
    mp_iHeightCode = new short int*[m_iNumSpecies];
    mp_iDiam10Code = new short int*[m_iNumSpecies];
    mp_iDbhCode = new short int*[m_iNumSpecies];
    mp_iCrownRadCode = new short int*[m_iNumSpecies];
    mp_iCrownDepthCode = new short int*[m_iNumSpecies];
    mp_iAgeCode = new short int[m_iNumSpecies];
    mp_iWhyDeadCode = new short int[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++) {
      mp_iNumTreeIntVals[i] = new short int[m_iNumTypes];
      mp_iNumTreeFloatVals[i] = new short int[m_iNumTypes];
      mp_iNumTreeStringVals[i] = new short int[m_iNumTypes];
      mp_iNumTreeBoolVals[i] = new short int[m_iNumTypes];
      mp_iXCode[i] = new short int[m_iNumTypes];
      mp_iYCode[i] = new short int[m_iNumTypes];
      mp_iHeightCode[i] = new short int[m_iNumTypes];
      mp_iDiam10Code[i] = new short int[m_iNumTypes];
      mp_iDbhCode[i] = new short int[m_iNumTypes];
      mp_iCrownRadCode[i] = new short int[m_iNumTypes];
      mp_iCrownDepthCode[i] = new short int[m_iNumTypes];
      for (j = 0; j < m_iNumTypes; j++) {
        //Add the default values for each
        //Float - X, Y, diameter, height for seedling, sapling, adult, snag
        //X, Y, diameter for stump
        //Crown rad and height for saplings, adults, snag
        //Nothing for anything else
        if (j == seedling)
          mp_iNumTreeFloatVals[i][j] = 4;
        else if (j == sapling || j == adult || j == snag)
          mp_iNumTreeFloatVals[i][j] = 6;
        else if (j == stump)
          mp_iNumTreeFloatVals[i][j] = 3;
        else
          mp_iNumTreeFloatVals[i][j] = 0;
        //Int - age and why dead for snags
        if (j == snag) mp_iNumTreeIntVals[i][j] = 2;
        else
          mp_iNumTreeIntVals[i][j] = 0;
        //No chars or bools
        mp_iNumTreeStringVals[i][j] = 0;
        mp_iNumTreeBoolVals[i][j] = 0;

        mp_iXCode[i][j] = -1;
        mp_iYCode[i][j] = -1;
        mp_iHeightCode[i][j] = -1;
        mp_iDbhCode[i][j] = -1;
        mp_iDiam10Code[i][j] = -1;
        mp_iCrownDepthCode[i][j] = -1;
        mp_iCrownRadCode[i][j] = -1;
      } //end of for (j = 0; j < m_iNumTypes; j++)
      //For saplings of each species, add an extra because they have both dbh
      //and diam 10
      mp_iNumTreeFloatVals[i][sapling] ++;
    } //end of for (i = 0; i < m_iNumSpecies; i++)

    //Now query the behaviors about what they want to add and add them to the
    //appropriate type/species combos
    iNumBehaviors = mp_oSimManager->GetNumberOfBehaviors();
    for (i = 0; i < iNumBehaviors; i++) {
      p_oBehavior = mp_oSimManager->GetBehaviorObject(i);

      iNewInts = p_oBehavior->GetNewTreeInts();
      iNewFloats = p_oBehavior->GetNewTreeFloats();
      iNewChars = p_oBehavior->GetNewTreeChars();
      iNewBools = p_oBehavior->GetNewTreeBools();
      iNumCombos = p_oBehavior->GetNumSpeciesTypeCombos();

      for (j = 0; j < iNumCombos; j++) {
        combo = p_oBehavior->GetSpeciesTypeCombo(j);

        mp_iNumTreeIntVals[combo.iSpecies][combo.iType] += iNewInts;
        mp_iNumTreeBoolVals[combo.iSpecies][combo.iType] += iNewBools;
        mp_iNumTreeFloatVals[combo.iSpecies][combo.iType] += iNewFloats;
        mp_iNumTreeStringVals[combo.iSpecies][combo.iType] += iNewChars;

        //Check to see if any type is stump - if so, set the make stumps flag
        //for that species to true
        if (stump == combo.iType) mp_bMakeStump[combo.iSpecies] = true;

        //If any type is snag - make the snags flag true
        if (snag == combo.iType) m_bMakeSnag = true;

      } //end of for (j = 0; j < iNumCombos; j++)
    } //end of for (i = 0; i < iNumBehaviors; i++)

    //Declare the label arrays
    mp_sIntLabels = new string **[m_iNumSpecies];
    mp_sFloatLabels = new string **[m_iNumSpecies];
    mp_sStringLabels = new string **[m_iNumSpecies];
    mp_sBoolLabels = new string **[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++) {
      mp_sIntLabels[i] = new string *[m_iNumTypes];
      mp_sFloatLabels[i] = new string *[m_iNumTypes];
      mp_sStringLabels[i] = new string *[m_iNumTypes];
      mp_sBoolLabels[i] = new string *[m_iNumTypes];
      for (j = 0; j < m_iNumTypes; j++) {
        mp_sIntLabels[i][j] = new string [mp_iNumTreeIntVals[i][j]];
        mp_sFloatLabels[i][j] = new string [mp_iNumTreeFloatVals[i][j]];
        mp_sStringLabels[i][j] = new string [mp_iNumTreeStringVals[i][j]];
        mp_sBoolLabels[i][j] = new string [mp_iNumTreeBoolVals[i][j]];
      } //end of for (j = 0; j < m_iNumTypes; j++)
    } //end of for (i = 0; i < m_iNumSpecies; i++)

    //Register the tree population's values
    for (i = 0; i < m_iNumSpecies; i++) {
      for (j = 0; j < m_iNumTypes; j++)  {
        if (seed != j && woody_debris != j)
        {
          mp_iXCode[i][j] = RegisterFloat("X", i, j);
          mp_iYCode[i][j] = RegisterFloat("Y", i, j);
        }
      }
      //For seedlings and saplings - register diam10
      mp_iDiam10Code[i][sapling] = RegisterFloat("Diam10", i, sapling);
      mp_iDiam10Code[i][seedling] = RegisterFloat("Diam10", i, seedling);
      //For saplings, adults, and snags - register dbh
      mp_iDbhCode[i][sapling] = RegisterFloat("DBH", i, sapling);
      mp_iDbhCode[i][adult] = RegisterFloat("DBH", i, adult);
      mp_iDbhCode[i][snag] = RegisterFloat("DBH", i, snag);
      //For seedlings, saplings, adults, and snags - register height
      mp_iHeightCode[i][sapling] = RegisterFloat("Height", i, sapling);
      mp_iHeightCode[i][seedling] = RegisterFloat("Height", i, seedling);
      mp_iHeightCode[i][adult] = RegisterFloat("Height", i, adult);
      mp_iHeightCode[i][snag] = RegisterFloat("Height", i, snag);
      //For saplings, adults, and snags - register crown depth, crown rad
      mp_iCrownRadCode[i][sapling] = RegisterFloat("Crown Radius", i, sapling);
      mp_iCrownRadCode[i][adult] = RegisterFloat("Crown Radius", i, adult);
      mp_iCrownRadCode[i][snag] = RegisterFloat("Crown Radius", i, snag);
      mp_iCrownDepthCode[i][sapling] = RegisterFloat("Crown Depth", i, sapling);
      mp_iCrownDepthCode[i][adult] = RegisterFloat("Crown Depth", i, adult);
      mp_iCrownDepthCode[i][snag] = RegisterFloat("Crown Depth", i, snag);
      //If this is a stumping species, register dbh for stump
      if (mp_bMakeStump[i])
        mp_iDbhCode[i][stump] = RegisterFloat("DBH", i, stump);
      //Register age and dead code for snags
      mp_iAgeCode[i] = RegisterInt("Age", i, snag);
      mp_iWhyDeadCode[i] = RegisterInt("Why dead", i, snag);
    }
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::DoTreeDataStructureSetup" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// CreateTreesFromInitialDensities()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::CreateTreesFromInitialDensities(DOMDocument * p_oDoc) {
  try
  {
    using namespace std;
    DOMNodeList * p_oNodeList, * p_oInitDensities;
    DOMNode * p_oDocNode;
    DOMElement * p_oElement;
    XMLCh *sVal;
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject(); //for getting the area of the plot
    char *cData;
    double * p_fDensities = new double[m_iNumSpecies];
    double fSeedlingHeight1Upper = 0, //upper bound of seedling height class 1, in cm
    fSeedlingHeight2Upper = 0; //upper bound of seedling height class 2, in cm
    float fX, fY, //new tree coordinates
    fDiam, //new tree diameter - either dbh or diam10
    fPlotAreaInHec= p_oPlot->GetPlotArea(), //plot area in hectares
    fRand, //random number
    fTemp, //workhorse variable
    //fMinDiam = 0.001, //minimum possible diameter value
    fMinDiam = m_fNewSeedlingDiam10, //minimum possible diameter value
    fLowerLimit = 0, fUpperLimit = 0, //size class size limits
    fNumTrees, //number of trees to create
    k; //loop counter in case number of trees is large
    int iSpecies; //species code
    unsigned short int iNumSpecies, //for counting the number of species
    iNumClasses, //for counting one species's size classes
    i, j, //loop counters
    iType; //type code
    bool bFound; //for verifying size classes

    //Look for initial density data
    sVal = XMLString::transcode("tr_initialDensities");
    p_oNodeList = p_oDoc->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength()) goto seedlingHeightClasses; //nothing found - skip this part

    //Make sure there were size classes defined - if not, error out
    if (NULL == mp_fSizeClasses)
    {
      delete[] p_fDensities;
      modelErr stcErr;
      stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities" ;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Size classes must be defined to use initial densities.";
      throw(stcErr);
    }

    //Start parsin' - go though each species's data
    p_oDocNode = p_oNodeList->item(0);
    p_oElement = (DOMElement *) p_oDocNode;
    sVal = XMLString::transcode("tr_idVals");
    //one tr_idVal holds the initial density data for one species
    p_oNodeList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);

    iNumSpecies = p_oNodeList->getLength();
    if (0 == iNumSpecies) goto seedlingHeightClasses; //nothing found - skip this part

    //************************************************
    //cycle through each of the species that are present - a species is not
    //required to be present
    //************************************************
    for (i = 0; i < iNumSpecies; i++)
    {

      //Get the species code for this set of initial densities
      p_oDocNode = p_oNodeList->item(i);
      p_oElement = (DOMElement *) p_oDocNode;
      sVal = XMLString::transcode("whatSpecies");
      cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
      XMLString::release(&sVal);
      iSpecies = TranslateSpeciesNameToCode(cData);
      if (-1 == iSpecies)
      { //if it's not a species we recognize error out
        modelErr stcErr;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Unrecognized species in initial densities:  ";
        stcErr.sMoreInfo += cData;
        delete[] cData; cData = NULL;
        delete[] p_fDensities;
        throw(stcErr);
      }
      delete[] cData; cData = NULL;

      //*******************************************
      //Go through each of the size classes for this species
      //*******************************************
      sVal = XMLString::transcode("tr_initialDensity");
      p_oInitDensities = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumClasses = p_oInitDensities->getLength();
      for (j = 0; j < iNumClasses; j++)
      {

        //What size class is this? Parse out the text data
        p_oDocNode = p_oInitDensities->item(j);
        p_oElement = (DOMElement *) p_oDocNode;
        sVal = XMLString::transcode("sizeClass");
        cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
        XMLString::release(&sVal);
        if (strcmp("Seedling", cData) == 0)
        {
          //**************************************
          // Create seedlings
          //**************************************
          //Parse out the number of seedlings per hectare
          delete[] cData; cData = NULL;
          cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          fNumTrees = atof(cData);
          //If negative throw an error
          if (fNumTrees < 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
            stcErr.sMoreInfo = "Found negative tree density.";
            delete[] cData; cData = NULL;
            delete[] p_fDensities;
            throw(stcErr);
          }
          if (fNumTrees == 0) {
            //If zero, verify that it really is supposed to be zero and
            //the character string just couldn't be parsed
            if (cData[0] != '0') {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
              std::stringstream s;
              s << "Tree density is not a number: \"" << cData << "\".";
              stcErr.sMoreInfo = s.str();
              delete[] cData; cData = NULL;
              delete[] p_fDensities;
              throw(stcErr);
            }
          }
          delete[] cData; cData = NULL;

          //transform this to a number of total trees
          fNumTrees = clModelMath::Round(fNumTrees * fPlotAreaInHec, 0);

          //Create all seedlings
          for (k = 0; k < fNumTrees; k++)
          {

            //Get coordinates for this tree
            fRand = clModelMath::GetRand();
            fX = p_oPlot->CorrectX(fRand * m_fPlotLengthX);
            fRand = clModelMath::GetRand();
            fY = p_oPlot->CorrectY(fRand * m_fPlotLengthY);

            //Create tree with zero diameter - allow CreateTree to randomize
            //us up a value
            CreateTree(fX, fY, iSpecies, seedling, 0);
          } // end of for (k = 0; k < fNumTrees; k++)

        } //end of if ("Seedling" == strData)

        //***********************************************
        //Create saplings or adults
        //***********************************************
        else
        { //not the "seedling" class - creating saplings or adults

          //parse out the size data
          string strData = cData;
          delete[] cData; cData = NULL;
          strData.erase(0, 1);
          fUpperLimit = atof(strData.c_str());
          if (fUpperLimit != 0)
          {
            //Get the lower limit of this size class - making sure we recognize
            //this value
            bFound = false;
            fLowerLimit = 0;
            for (k = 0; k < m_iNumSizeClasses; k++) {
              if (fUpperLimit == mp_fSizeClasses[(int)k]) {
                bFound = true;
                if (0 == k)
                  fLowerLimit = fMinDiam;
                else
                  fLowerLimit = mp_fSizeClasses[(int)k - 1];
              }
            }
            //Make sure that we found the size class
            if (false == bFound) {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
              std::stringstream s;
              s << "Found unrecognized tree size class: \"" << fUpperLimit << "\".";
              stcErr.sMoreInfo = s.str();
              delete[] p_fDensities;
              throw(stcErr);
            }

            //parse out the number of trees per hectare
            cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
            fNumTrees = atof(cData);
            //If negative throw an error
            if (fNumTrees < 0) {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
              std::stringstream s;
              s << "Found negative tree density: \"" << fNumTrees << "\".";
              stcErr.sMoreInfo = s.str();
              delete[] cData; cData = NULL;
              delete[] p_fDensities;
              throw(stcErr);
            }
            if (fNumTrees == 0) {
              //If zero, verify that it really is supposed to be zero and
              //the character string just couldn't be parsed
              if (cData[0] != '0') {
                modelErr stcErr;
                stcErr.iErrorCode = BAD_DATA;
                stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
                std::stringstream s;
                s << "Tree density is not a number: \"" << cData << "\".";
                stcErr.sMoreInfo = s.str();
                delete[] cData; cData = NULL;
                delete[] p_fDensities;
                throw(stcErr);
              }
            }
            delete[] cData; cData = NULL;

            //transform this to a number of total trees
            fNumTrees = clModelMath::Round(fNumTrees * fPlotAreaInHec, 0);

            //Create all trees
            for (k = 0; k < fNumTrees; k++)
            {

              //Get a diameter for this tree
              fRand = clModelMath::GetRand();
              fDiam = fUpperLimit - (fRand * (fUpperLimit - fLowerLimit));

              //Get coordinates for this tree
              fRand = clModelMath::GetRand();
              fX = p_oPlot->CorrectX(fRand * m_fPlotLengthX);
              fRand = clModelMath::GetRand();
              fY = p_oPlot->CorrectY(fRand * m_fPlotLengthY);

              //Figure out whether this is a sapling or adult
              if (fDiam < mp_fMinAdultDbh[iSpecies]) iType = sapling;
              else
                iType = adult;
              CreateTree(fX, fY, iSpecies, iType, fDiam);
            } // end of for (k = 0; k < fNumTrees; k++)
          } // end of if (fUpperLimit != 0)
          else {
            //The upper limit came out as zero - make sure that the first
            //character is '0' to make sure this isn't a bad string instead
            if (strData[0] != '0') {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
              std::stringstream s;
              s << "Unrecognized tree size class \"" << cData << "\".";
              stcErr.sMoreInfo = s.str();
              delete[] cData; cData = NULL;
              delete[] p_fDensities;
              throw(stcErr);
            }
          }
        } //end of else - not the "seedling" class
      } //end of for (j = 0; j < iNumClasses; j++)
    } //end of for (i = 0; i < iNumNodes; i++)

    //**************************************
    // Create seedlings from seedling height classes
    //**************************************
    seedlingHeightClasses:
    p_oElement = p_oDoc->getDocumentElement();
    FillSingleValue(p_oElement, "tr_seedlingHeightClass1", & fSeedlingHeight1Upper, false);
    FillSingleValue(p_oElement, "tr_seedlingHeightClass2", & fSeedlingHeight2Upper, false);
    if (fSeedlingHeight1Upper == 0 && fSeedlingHeight2Upper == 0)
    {
      delete[] p_fDensities;
      return;
    }

    //Convert from cm to m
    fSeedlingHeight1Upper /= 100;
    fSeedlingHeight2Upper /= 100;

    //Make sure the size classes are at least 0.1 and they don't overlap
    if (fSeedlingHeight1Upper < 0.1 || fSeedlingHeight2Upper < 0.1 || fSeedlingHeight2Upper < fSeedlingHeight1Upper)
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities" ;
      stcErr.sMoreInfo = "Seedling height classes must be greater than 10 cm and cannot overlap.";
      delete[] p_fDensities;
      throw(stcErr);
    }

    //Get the initial densities for the first height class
    for (iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++)
    {
      p_fDensities[iSpecies] = 0;
    }
    FillSpeciesSpecificValue(p_oElement, "tr_seedlingHeight1Density", "tr_sh1dVal", p_fDensities, this, false);
    //Create the trees
    for (iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++)
    {
      //If negative throw an error
      if (p_fDensities[iSpecies] < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
        stcErr.sMoreInfo = "Found negative tree density.";
        delete[] p_fDensities;
        throw(stcErr);
      }
      fNumTrees = clModelMath::Round(p_fDensities[iSpecies] * fPlotAreaInHec, 0);

      //Double-check that it is possible to create trees of this size, taking
      //into account allometry
      fDiam = mp_oAllom->CalcSeedlingDiam10(fSeedlingHeight1Upper, iSpecies);
      if (fDiam >= m_fNewSeedlingDiam10) {

        //Create all trees
        for (k = 0; k < fNumTrees; k++)
        {

          //Get a height for this tree and transform into a diam10
          //Make sure it's taller than 10 cm or the allometry
          //equation crashes
          fTemp = 0; fDiam = 0;
          while (fTemp < 0.1 || fDiam < m_fNewSeedlingDiam10) {
            fRand = clModelMath::GetRand();
            fTemp = fSeedlingHeight1Upper - (fRand * fSeedlingHeight1Upper);
            fDiam = mp_oAllom->CalcSeedlingDiam10(fTemp, iSpecies);
          }

          //Get coordinates for this tree
          fRand = clModelMath::GetRand();
          fX = p_oPlot->CorrectX(fRand * m_fPlotLengthX);
          fRand = clModelMath::GetRand();
          fY = p_oPlot->CorrectY(fRand * m_fPlotLengthY);

          CreateTree(fX, fY, iSpecies, seedling, fDiam);
        } // end of for (k = 0; k < iNumTrees; k++)
      }
    }

    //Get the initial densities for the second height class
    for (iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++)
    {
      p_fDensities[iSpecies] = 0;
    }
    FillSpeciesSpecificValue(p_oElement, "tr_seedlingHeight2Density", "tr_sh2dVal", p_fDensities, this, false);
    //Create the trees
    for (iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++)
    {
      //If negative throw an error
      if (p_fDensities[iSpecies] < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
        stcErr.sMoreInfo = "Found negative tree density.";
        delete[] p_fDensities;
        throw(stcErr);
      }
      fNumTrees = clModelMath::Round(p_fDensities[iSpecies] * fPlotAreaInHec, 0);

      //Create all trees
      for (k = 0; k < fNumTrees; k++)
      {

        //Get a height for this tree and transform into a diam10
        //Make sure it's taller than 10 cm or the allometry equation crashes
        fTemp = 0;
        while (fTemp < 0.1) {
          fRand = clModelMath::GetRand();
          fTemp = fSeedlingHeight2Upper - (fRand * (fSeedlingHeight2Upper - fSeedlingHeight1Upper));
        }
        fDiam = mp_oAllom->CalcSeedlingDiam10(fTemp, iSpecies);

        //Get coordinates for this tree
        fRand = clModelMath::GetRand();
        fX = p_oPlot->CorrectX(fRand * m_fPlotLengthX);
        fRand = clModelMath::GetRand();
        fY = p_oPlot->CorrectY(fRand * m_fPlotLengthY);

        CreateTree(fX, fY, iSpecies, seedling, fDiam);
      } // end of for (k = 0; k < iNumTrees; k++)
    }

    //Get the initial densities for the third height class
    for (iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++)
    {
      p_fDensities[iSpecies] = 0;
    }
    FillSpeciesSpecificValue(p_oElement, "tr_seedlingHeight3Density", "tr_sh3dVal", p_fDensities, this, false);
    //Create the trees
    for (iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++)
    {
      //If negative throw an error
      if (p_fDensities[iSpecies] < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities";
        stcErr.sMoreInfo = "Found negative tree density.";
        delete[] p_fDensities;
        throw(stcErr);
      }
      fNumTrees = clModelMath::Round(p_fDensities[iSpecies] * fPlotAreaInHec, 0);

      //Create all trees
      for (k = 0; k < fNumTrees; k++)
      {

        //Get a height for this tree and transform into a diam10
        //Make sure it's taller than 10 cm or the allometry equation crashes
        fTemp = 0;
        while (fTemp < 0.1) {
          fRand = clModelMath::GetRand();
          fTemp = 1.35 - (fRand * (1.35 - fSeedlingHeight2Upper));
        }
        fDiam = mp_oAllom->CalcSeedlingDiam10(fTemp, iSpecies);

        //Get coordinates for this tree
        fRand = clModelMath::GetRand();
        fX = p_oPlot->CorrectX(fRand * m_fPlotLengthX);
        fRand = clModelMath::GetRand();
        fY = p_oPlot->CorrectY(fRand * m_fPlotLengthY);

        CreateTree(fX, fY, iSpecies, seedling, fDiam);
      } // end of for (k = 0; k < iNumTrees; k++)
    }

    delete[] p_fDensities;

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// CreateTreesFromTreeMap
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::CreateTreesFromTreeMap(DOMDocument * p_oDoc) {
  try
  {
    clTree * p_oTree; //tree object for the trees we'll create
    DOMNodeList * p_oNodeList, //for searching for tags
    * p_oSettingsList, //for cycling through tree settings tags
    * p_oTreeList, //For processing all the trees in the file
    * p_oChildren; //for searching for children
    DOMNode * p_oNode; //for capturing a particular tag
    DOMElement * p_oElement, //for casting a tag to access its data
    * p_oSettings, //For accessing the stuff under a tree setting
    * p_oTreeInMap, //For getting the settings of one tree
    * p_oTreeMap; //root element for this map file
    XMLCh *sVal;

    struct stcCodeTrans
    { //for translating the data member codes in map
      short int iCodeInMap, //what the code is in the map
      iCodeForTree; //what the code is to the tree
    }
    ***p_ints = NULL, ***p_floats = NULL, //array for each data member type
        ***p_chars = NULL, ***p_bools = NULL; //sized species by type
    char * cData; //for capturing text data
    float fMapPlotLenX, fMapPlotLenY, //X & Y grid lengths from map
    fX, fY, fDiam, fHeight, //For getting these required tree values
    fTemp = 0; //For initializing tree values
    long int iNumTrees; //Number of trees to read
    int iTemp = 0; //For initializing grid values
    short int * * p_iNumInts, * * p_iNumFloats, * * p_iNumChars, * * p_iNumBools, iMapCode, //codes for values read from the map file
    iTreeCode, //code translated from map to grid values
    iSpecies, //For extracting species in file
    iType; //For extracting tree types in file
    unsigned short int i, j, k; //loop counters
    bool bTemp = false; //For initializing grid values

    //*************************************************
    // Check to see if a tree map is present - if not exit
    // with no error condition
    //*************************************************
    sVal = XMLString::transcode("tr_treemap");
    p_oNodeList = p_oDoc->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength()) return;
    p_oNode = p_oNodeList->item(0);
    p_oTreeMap = (DOMElement *) p_oNode;

    //*************************************************
    // Validate the plot size data - if too big exit with error -
    // if not present continue
    //*************************************************
    sVal = XMLString::transcode("tm_plotLenX");
    p_oNodeList = p_oTreeMap->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength())
    {
      cData = XMLString::transcode(p_oNodeList->item(0)->getFirstChild()->getNodeValue());
      fMapPlotLenX = atof(cData);
      if (fMapPlotLenX > m_fPlotLengthX)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap";
        std::stringstream s;
        s << "Tree map X length (" << cData << ") larger than plot";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }
    sVal = XMLString::transcode("tm_plotLenY");
    p_oNodeList = p_oTreeMap->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength())
    {
      cData = XMLString::transcode(p_oNodeList->item(0)->getFirstChild()->getNodeValue());
      fMapPlotLenY = atof(cData);
      if (fMapPlotLenY > m_fPlotLengthY)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap";
        std::stringstream s;
        s << "Tree map Y length (" << cData << ") larger than plot";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
        delete[] cData; cData = NULL;
      }
      delete[] cData; cData = NULL;
    }

    //*************************************************
    // Validate the species data - if not a valid subset exit with error -
    // if not present continue
    //*************************************************
    sVal = XMLString::transcode("tm_speciesList");
    p_oNodeList = p_oTreeMap->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength())
    {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("tm_species");
      p_oNodeList = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iTemp = p_oNodeList->getLength();
      if (iTemp > m_iNumSpecies)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
        stcErr.sMoreInfo = "Tree map has more species than the par file";
        throw(stcErr);
      }
      else if (0 != iTemp)
      {
        for (i = 0; i < iTemp; i++)
        {
          p_oNode = p_oNodeList->item(i);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("speciesName");
          cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
          XMLString::release(&sVal);
          iSpecies = TranslateSpeciesNameToCode(cData);
          if (-1 == iSpecies)
          {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
            stcErr.sMoreInfo = "Tree map has species not in par file - ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == iSpecies)
          delete[] cData; cData = NULL;
        } //end of for (i = 0; i < iTemp; i++)
      } //end of else if (0 != iTemp)
    } //end of if (0 != p_oNodeList->getLength())


    //*************************************************
    // Validate the data members - make sure all the data members have already
    // been registered and grab the codes for translation when we're reading trees
    //*************************************************
    //Declare the code arrays
    p_iNumInts = new short int * [m_iNumSpecies];
    p_iNumFloats = new short int * [m_iNumSpecies];
    p_iNumChars = new short int * [m_iNumSpecies];
    p_iNumBools = new short int * [m_iNumSpecies];
    p_ints = new stcCodeTrans * * [m_iNumSpecies];
    p_floats = new stcCodeTrans * * [m_iNumSpecies];
    p_chars = new stcCodeTrans * * [m_iNumSpecies];
    p_bools = new stcCodeTrans * * [m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++)
    {
      p_iNumInts[i] = new short int[m_iNumTypes];
      p_iNumFloats[i] = new short int[m_iNumTypes];
      p_iNumChars[i] = new short int[m_iNumTypes];
      p_iNumBools[i] = new short int[m_iNumTypes];
      p_ints[i] = new stcCodeTrans * [m_iNumTypes];
      p_floats[i] = new stcCodeTrans * [m_iNumTypes];
      p_chars[i] = new stcCodeTrans * [m_iNumTypes];
      p_bools[i] = new stcCodeTrans * [m_iNumTypes];
      for (j = 0; j < m_iNumTypes; j++)
      {
        p_iNumInts[i][j] = 0;
        p_iNumFloats[i][j] = 0;
        p_iNumChars[i][j] = 0;
        p_iNumBools[i][j] = 0;
        p_ints[i][j] = NULL;
        p_floats[i][j] = NULL;
        p_chars[i][j] = NULL;
        p_bools[i][j] = NULL;
      }
    }

    //Get and process each of the tree settings
    sVal = XMLString::transcode("tm_treeSettings");
    p_oSettingsList = p_oTreeMap->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oSettingsList->getLength()) goto cleanup; //no tree settings - exit
    for (i = 0; i < p_oSettingsList->getLength(); i++)
    {
      p_oNode = p_oSettingsList->item(i);
      p_oSettings = (DOMElement *) p_oNode;
      //Extract the type and species attributes for this tree setting
      sVal = XMLString::transcode("sp");
      cData = XMLString::transcode(p_oSettings->getAttributeNode(sVal)->getNodeValue());
      XMLString::release(&sVal);
      iSpecies = TranslateSpeciesNameToCode(cData);
      delete[] cData; cData = NULL;
      sVal = XMLString::transcode("tp");
      cData = XMLString::transcode(p_oSettings->getAttributeNode(sVal)->getNodeValue());
      XMLString::release(&sVal);
      iType = atoi(cData);
      delete[] cData; cData = NULL;
      //Verify type and species
      if (-1 == iSpecies || iType < 0 || iType > woody_debris || (0 == iType && strcmp(cData, "0") != 0))
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clPopulation::CreateTreesFromTreeMap" ;
        std::stringstream s;
        s << "Invalid species or type.  Species: " << iSpecies << " Type: " << iType;
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }

      //Make sure the data codes for each member for this species and type are
      //already registered

      //***************Ints*****************
      sVal = XMLString::transcode("tm_intCodes");
      p_oNodeList = p_oSettings->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength())
      {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("tm_intCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        p_iNumInts[iSpecies][iType] = p_oChildren->getLength();
        if (p_iNumInts[iSpecies][iType] > 0)
        {
          //Declare the translation array for the int codes - throw an error if
          //this has already been done
          if (p_ints[iSpecies][iType])
          {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clPopulation::CreateTreesFromTreeMap" ;
            std::stringstream s;
            s << "Duplicate settings for this species and type.  Species: " <<
                iSpecies << " Type: " << iType;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          p_ints[iSpecies][iType] = new stcCodeTrans[p_iNumInts[iSpecies][iType]];
          for (j = 0; j < p_iNumInts[iSpecies][iType]; j++)
          {
            //Get the code for this data member according to the map
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
            p_ints[iSpecies][iType][j].iCodeInMap = atoi(cData);
            //If it's not a number throw an error
            if (0 == p_ints[iSpecies][iType][j].iCodeInMap && strcmp(cData, "0") != 0)
            {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
              stcErr.sMoreInfo = "Can't convert code to number: ";
              stcErr.sMoreInfo += cData;
              delete[] cData; cData = NULL;
              throw(stcErr);
            }
            delete[] cData; cData = NULL;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_ints[iSpecies][iType][j].iCodeForTree = GetIntDataCode(cData, iSpecies, iType);
            if (-1 == p_ints[iSpecies][iType][j].iCodeForTree)
            {
              //unrecognized label - throw error
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
              stcErr.sMoreInfo = "Unrecognized int data member label: ";
              stcErr.sMoreInfo += cData;
              delete[] cData; cData = NULL;
              throw(stcErr);
            } //end of if (-1 == p_ints[iSpecies][iType][j].iCodeForGrid)
            delete[] cData; cData = NULL;
          } //end of for (j = 0; j < p_iNumInts[iSpecies][iType]; j++)
        } //end of if (p_iNumInts[iSpecies][iType] > 0)
      } //end of if (0 != p_oNodeList->getLength()) -int data member processing

      //***************Floats*****************
      sVal = XMLString::transcode("tm_floatCodes");
      p_oNodeList = p_oSettings->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength())
      {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("tm_floatCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        p_iNumFloats[iSpecies][iType] = p_oChildren->getLength();
        if (p_iNumFloats[iSpecies][iType] > 0)
        {
          //Declare the translation array for the float codes - throw an error
          //if this has already been done
          if (p_floats[iSpecies][iType])
          {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clPopulation::CreateTreesFromTreeMap" ;
            std::stringstream s;
            s << "Duplicate settings for this species and type.  Species: " <<
                iSpecies << " Type: " << iType;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          p_floats[iSpecies][iType] = new stcCodeTrans[p_iNumFloats[iSpecies][iType]];
          for (j = 0; j < p_iNumFloats[iSpecies][iType]; j++)
          {
            //Get the code for this data member according to the map
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
            p_floats[iSpecies][iType][j].iCodeInMap = atoi(cData);
            //If it's not a number throw an error
            if (0 == p_floats[iSpecies][iType][j].iCodeInMap && strcmp(cData, "0") != 0)
            {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
              stcErr.sMoreInfo = "Can't convert code to number: ";
              stcErr.sMoreInfo += cData;
              delete[] cData; cData = NULL;
              throw(stcErr);
            }
            delete[] cData; cData = NULL;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_floats[iSpecies][iType][j].iCodeForTree = GetFloatDataCode(cData, iSpecies, iType);
            if (-1 == p_floats[iSpecies][iType][j].iCodeForTree)
            {
              //unrecognized label - throw error
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
              stcErr.sMoreInfo = "Unrecognized float data member label: ";
              stcErr.sMoreInfo += cData;
              delete[] cData; cData = NULL;
              throw(stcErr);
            } //end of if (-1 == p_floats[iSpecies][iType][j].iCodeForGrid)
            delete[] cData; cData = NULL;
          } //end of for (j = 0; j < p_iNumFloats[iSpecies][iType]; j++)
        } //end of if (p_iNumFloats[iSpecies][iType] > 0)
      } //end of if (0 != p_oNodeList->getLength()) - float data member processing

      //***************Chars*****************
      sVal = XMLString::transcode("tm_charCodes");
      p_oNodeList = p_oSettings->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength())
      {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("tm_charCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        p_iNumChars[iSpecies][iType] = p_oChildren->getLength();
        if (p_iNumChars[iSpecies][iType] > 0)
        {
          //Declare the translation array for the char codes - throw an error if
          //this has already been done
          if (p_chars[iSpecies][iType])
          {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clPopulation::CreateTreesFromTreeMap" ;
            std::stringstream s;
            s << "Duplicate settings for this species and type.  Species: " <<
                iSpecies << " Type: " << iType;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          p_chars[iSpecies][iType] = new stcCodeTrans[p_iNumChars[iSpecies][iType]];
          for (j = 0; j < p_iNumChars[iSpecies][iType]; j++)
          {
            //Get the code for this data member according to the map
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
            p_chars[iSpecies][iType][j].iCodeInMap = atoi(cData);
            //If it's not a number throw an error
            if (0 == p_chars[iSpecies][iType][j].iCodeInMap && strcmp(cData, "0") != 0)
            {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
              stcErr.sMoreInfo = "Can't convert code to number: ";
              stcErr.sMoreInfo += cData;
              delete[] cData; cData = NULL;
              throw(stcErr);
            }
            delete[] cData; cData = NULL;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_chars[iSpecies][iType][j].iCodeForTree = GetStringDataCode(cData, iSpecies, iType);
            if (-1 == p_chars[iSpecies][iType][j].iCodeForTree)
            {
              //unrecognized label - throw error
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
              stcErr.sMoreInfo = "Unrecognized char data member label: ";
              stcErr.sMoreInfo += cData;
              delete[] cData; cData = NULL;
              throw(stcErr);
            } //end of if (-1 == p_chars[iSpecies][iType][j].iCodeForGrid)
            delete[] cData; cData = NULL;
          } //end of for (j = 0; j < p_iNumChars[iSpecies][iType]; j++)
        } //end of if (p_iNumChars[iSpecies][iType] > 0)
      } //end of if (0 != p_oNodeList->getLength()) - char data member processing

      //***************Bools*****************
      sVal = XMLString::transcode("tm_boolCodes");
      p_oNodeList = p_oSettings->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength())
      {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("tm_boolCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        p_iNumBools[iSpecies][iType] = p_oChildren->getLength();
        if (p_iNumBools[iSpecies][iType] > 0)
        {
          //Declare the translation array for the bool codes - throw an error if
          //this has already been done
          if (p_bools[iSpecies][iType])
          {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clPopulation::CreateTreesFromTreeMap" ;
            std::stringstream s;
            s << "Duplicate settings for this species and type.  Species: " <<
                iSpecies << " Type: " << iType;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          p_bools[iSpecies][iType] = new stcCodeTrans[p_iNumBools[iSpecies][iType]];
          for (j = 0; j < p_iNumBools[iSpecies][iType]; j++)
          {
            //Get the code for this data member according to the map
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
            p_bools[iSpecies][iType][j].iCodeInMap = atoi(cData);
            //If it's not a number throw an error
            if (0 == p_bools[iSpecies][iType][j].iCodeInMap && strcmp(cData, "0") != 0)
            {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
              stcErr.sMoreInfo = "Can't convert code to number: ";
              stcErr.sMoreInfo += cData;
              delete[] cData; cData = NULL;
              throw(stcErr);
            }
            delete[] cData; cData = NULL;
            //Get the data label, which is an attribute of the tag
            cData = XMLString::transcode(p_oElement->getAttributeNode(XMLString::transcode("label"))->getNodeValue());
            p_bools[iSpecies][iType][j].iCodeForTree = GetBoolDataCode(cData, iSpecies, iType);
            if (-1 == p_bools[iSpecies][iType][j].iCodeForTree)
            {
              //unrecognized label - throw error
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
              stcErr.sMoreInfo = "Unrecognized bool data member label: ";
              stcErr.sMoreInfo += cData;
              delete[] cData; cData = NULL;
              throw(stcErr);
            } //end of if (-1 == p_bools[iSpecies][iType][j].iCodeForGrid)
            delete[] cData; cData = NULL;
          } //end of for (j = 0; j < p_iNumBools[iSpecies][iType]; j++)
        } //end of if (p_iNumBools[iSpecies][iType] > 0)
      } //end of if (0 != p_oNodeList->getLength()) - bool data member processing
    } //end of for (i = 0; i < p_oSettingsList->getLength(); i++)

    //Validate that all the necessary tree population data members are present
    //All species/type combos that are defined must have X, Y, and diam or
    //height (which can be used to calculate diam)
    for (i = 0; i < m_iNumSpecies; i++)
      for (j = 0; j < m_iNumTypes; j++)
      {
        fX = -1; fY = -1; fDiam = -1;
        if (p_ints[i][j] || p_floats[i][j] || p_chars[i][j] || p_bools[i][j])
        {
          for (k = 0; k < p_iNumFloats[i][j]; k++)
          {
            if (p_floats[i][j][k].iCodeForTree == mp_iXCode[i][j]) fX = 1;
            if (p_floats[i][j][k].iCodeForTree == mp_iYCode[i][j]) fY = 1;
            if (p_floats[i][j][k].iCodeForTree == mp_iHeightCode[i][j]) fDiam = 1;
            if (seedling == j)
            {
              if (p_floats[i][j][k].iCodeForTree == mp_iDiam10Code[i][j]) fDiam = 1;
            }
            else if (sapling == j)
            {
              if (p_floats[i][j][k].iCodeForTree == mp_iDiam10Code[i][j]
                                                                          || p_floats[i][j][k].iCodeForTree == mp_iDbhCode[i][j])fDiam = 1;
            }
            else if (adult == j || snag == j || stump == j)
              if (p_floats[i][j][k].iCodeForTree == mp_iDbhCode[i][j]) fDiam = 1;
          } //end of for (k = 0; k < p_iNumFloats[i][j]; k++)
          if (fX < 0 || fY < 0 || fDiam < 0)
          { //required values absent - error
            modelErr stcErr;
            stcErr.iErrorCode = DATA_MISSING;
            stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
            stcErr.sMoreInfo = "Cannot load map without X, Y, and diameter values.";
            throw(stcErr);
          }
        }
      }

    //*****************************
    //Read the trees in the tree map
    //*****************************
    sVal = XMLString::transcode("tree");
    p_oTreeList = p_oTreeMap->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumTrees = p_oTreeList->getLength();
    for (i = 0; i < iNumTrees; i++)
    {
      fX = -1; fY = -1; fDiam = -1; fHeight = -1;
      p_oNode = p_oTreeList->item(i);
      p_oTreeInMap = (DOMElement *) p_oNode;

      //Get the species
      sVal = XMLString::transcode("sp");
      cData = XMLString::transcode(p_oTreeInMap->getAttributeNode(sVal)->getNodeValue());
      XMLString::release(&sVal);
      iSpecies = atoi(cData);
      //If we don't recognize the species error out
      if ((0 == iSpecies && strcmp(cData, "0") != 0) || iSpecies < 0 || iSpecies >= m_iNumSpecies)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
        stcErr.sMoreInfo = "Unrecognized species in tree map:  ";
        stcErr.sMoreInfo += cData;
        delete[] cData; cData = NULL;
        throw(stcErr);
      }
      delete[] cData; cData = NULL;
      //Check the type
      sVal = XMLString::transcode("tp");
      cData = XMLString::transcode(p_oTreeInMap->getAttributeNode(sVal)->getNodeValue());
      XMLString::release(&sVal);
      iType = atoi(cData);
      //If we don't recognize the type error out
      if ((0 == iType && strcmp(cData, "0") != 0) || iType < 0 || iType >= m_iNumTypes)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
        stcErr.sMoreInfo = "Unrecognized type in tree map: ";
        stcErr.sMoreInfo += cData;
        delete[] cData; cData = NULL;
        throw(stcErr);
      }
      delete[] cData; cData = NULL;

      //Load the values - first search for X, Y, and diam in the floats so we
      //can create the tree
      sVal = XMLString::transcode("fl");
      p_oChildren = p_oTreeInMap->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      for (j = 0; j < p_oChildren->getLength(); j++)
      {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *) p_oNode;
        //Check the code attribute to find out what this value is for
        iTreeCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
        iMapCode = atoi(cData);
        XMLString::release(&sVal); delete[] cData; cData = NULL;
        for (k = 0; k < p_iNumFloats[iSpecies][iType]; k++)
          if (iMapCode == p_floats[iSpecies][iType][k].iCodeInMap)
          {
            iTreeCode = p_floats[iSpecies][iType][k].iCodeForTree;
            break;
          }
        if (-1 == iTreeCode)
        { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized float code in map: " << iMapCode;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        //Get the value and validate it
        cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        fTemp = atof(cData);
        //Make sure it's a valid float
        if (0 == fTemp && cData[0] != '0')
        {
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sMoreInfo = "Invalid float value in map: ";
          stcErr.sMoreInfo += cData;
          delete[] cData; cData = NULL;
          throw(stcErr);
        }
        delete[] cData; cData = NULL;
        if (iTreeCode == mp_iXCode[iSpecies][iType]) //this is X
          fX = fTemp;
        else if (iTreeCode == mp_iYCode[iSpecies][iType]) //this is Y
          fY = fTemp;
        else if (iTreeCode == mp_iHeightCode[iSpecies][iType]) //height
          fHeight = fTemp;
        else if ((seedling == iType && //this is diam
            iTreeCode == mp_iDiam10Code[iSpecies][iType])
            || ((sapling == iType || adult == iType || stump == iType || snag == iType || woody_debris == iType)
                && iTreeCode == mp_iDbhCode[iSpecies][iType]))
          fDiam = fTemp;
        else if (sapling == iType && //special case - convert diam10 to dbh
            iTreeCode == mp_iDiam10Code[iSpecies][iType])
        {
          fDiam = fTemp;
          fDiam = mp_oAllom->ConvertDiam10ToDbh(fDiam, iSpecies);
        }
        else; //any other kind of float - skip for now
      } //end of for (j = 0; j < p_oChildren->getLength(); j++)

      //Check to see if we have height but not diameter - if so, get a
      //diameter from the height value
      if (fDiam < 0 && fHeight > 0)
      {
        if (seedling == iType)
        {
          //First, make sure the seedling is at least 10 cm tall.  Less than
          //that is an error
          if (fHeight < 0.10)
          {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
            stcErr.sMoreInfo = "Seedlings must be at least 10 cm tall.";
            throw(stcErr);
          }
          fDiam = mp_oAllom->CalcSeedlingDiam10(fHeight, iSpecies);
        }
        else if (sapling == iType)
          fDiam = mp_oAllom->CalcSaplingDbh(fHeight, iSpecies);
        else if (adult == iType || snag == iType)
          fDiam = mp_oAllom->CalcAdultDbh(fHeight, iSpecies);

        //Since height was used to define diameter size, avoid assigning it
        //again a few lines later by setting it back to -1
        fHeight = -1;
      }
      //Whew.  OK, check to see that we have X, Y, and diam.  If yes, create a
      //tree.  If not, throw an error
      if (fX < 0 || fY < 0 || fDiam < 0)
      {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
        stcErr.sMoreInfo = "X, Y, or diameter for a tree";
        throw(stcErr);
      }
      //Round fDiam to 3 digits; this helps with values that are equal to cutoffs
      fDiam = clModelMath::Round(fDiam, 3);
      p_oTree = CreateTree(fX, fY, iSpecies, iType, fDiam);

      //Now - if we got both diameter and height, assign height separately;
      //they are not required to agree allometrically
      if (fHeight > 0)
      {
        p_oTree->SetValue(mp_iHeightCode[iSpecies][iType], fHeight, true, false);
      }

      //Now load all other values for this tree
      //*********************Ints*********************
      sVal = XMLString::transcode("int");
      p_oChildren = p_oTreeInMap->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      for (j = 0; j < p_oChildren->getLength(); j++)
      {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *) p_oNode;
        //Get the value's code and translate it to the grid's code
        iTreeCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
        iMapCode = atoi(cData);
        XMLString::release(&sVal); delete[] cData; cData = NULL;
        for (k = 0; k < p_iNumInts[iSpecies][iType]; k++)
          if (iMapCode == p_ints[iSpecies][iType][k].iCodeInMap)
          {
            iTreeCode = p_ints[iSpecies][iType][k].iCodeForTree;
            break;
          }
        if (-1 == iTreeCode)
        { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized int code in map: " << iMapCode;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        iTemp = atoi(cData);
        //Make sure it's a valid int
        if (0 == iTemp && strcmp(cData, "0") != 0)
        {
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sMoreInfo = "Invalid int value in map: ";
          stcErr.sMoreInfo += cData;
          delete[] cData; cData = NULL;
          throw(stcErr);
        }
        delete[] cData; cData = NULL;
        p_oTree->SetValue(iTreeCode, iTemp);
      } //end of for (j = 0; j < p_oChildren->getLength(); j++) - ints

      //*********************Chars*********************
      sVal = XMLString::transcode("ch");
      p_oChildren = p_oTreeInMap->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      for (j = 0; j < p_oChildren->getLength(); j++)
      {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *) p_oNode;
        //Get the value's code and translate it to the grid's code
        iTreeCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
        iMapCode = atoi(cData);
        XMLString::release(&sVal); delete[] cData; cData = NULL;
        for (k = 0; k < p_iNumChars[iSpecies][iType]; k++)
          if (iMapCode == p_chars[iSpecies][iType][k].iCodeInMap)
          {
            iTreeCode = p_chars[iSpecies][iType][k].iCodeForTree;
            break;
          }
        if (-1 == iTreeCode)
        { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized char code in map: " << iMapCode;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        p_oTree->SetValue(iTreeCode, cData);
        delete[] cData; cData = NULL;
      } //end of for (j = 0; j < p_oChildren->getLength(); j++) - chars

      //*********************Bools*********************
      sVal = XMLString::transcode("bl");
      p_oChildren = p_oTreeInMap->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      for (j = 0; j < p_oChildren->getLength(); j++)
      {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *) p_oNode;
        //Get the value's code and translate it to the grid's code
        iTreeCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
        iMapCode = atoi(cData);
        XMLString::release(&sVal); delete[] cData; cData = NULL;
        for (k = 0; k < p_iNumBools[iSpecies][iType]; k++)
          if (iMapCode == p_bools[iSpecies][iType][k].iCodeInMap)
          {
            iTreeCode = p_bools[iSpecies][iType][k].iCodeForTree;
            break;
          }
        if (-1 == iTreeCode)
        { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized bool code in map: " << iMapCode;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        if (strcmp(cData, "true") == 0) bTemp = true;
        else if (strcmp(cData, "false") == 0) bTemp = false;
        else
        { //invalid bool - throw error
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::CreateTreesFromMapFile" ;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sMoreInfo = "Invalid bool value in map: ";
          stcErr.sMoreInfo += cData;
          delete[] cData; cData = NULL;
          throw(stcErr);
        }
        delete[] cData; cData = NULL;
        p_oTree->SetValue(iTreeCode, bTemp);
      } //end of for (j = 0; j < p_oChildren->getLength(); j++) - bools

      //*********************Floats*********************
      //For floats - skip X, Y, or diameter values
      sVal = XMLString::transcode("fl");
      p_oChildren = p_oTreeInMap->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      for (j = 0; j < p_oNodeList->getLength(); j++)
      {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *) p_oNode;
        //Check the code attribute to find out what this value is for
        iTreeCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
        iMapCode = atoi(cData);
        XMLString::release(&sVal); delete[] cData; cData = NULL;
        for (k = 0; k < p_iNumFloats[iSpecies][iType]; k++)
          if (iMapCode == p_floats[iSpecies][iType][k].iCodeInMap)
          {
            iTreeCode = p_floats[iSpecies][iType][k].iCodeForTree;
            break;
          }
        if (-1 == iTreeCode)
        { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized float code in map: " << iMapCode;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        if (iTreeCode != mp_iXCode[iSpecies][iType] && //it's not X
            iTreeCode != mp_iYCode[iSpecies][iType] && //it's not Y
            (seedling == iType && //it's not diam10 for a seedling
                iTreeCode != mp_iDiam10Code[iSpecies][iType]) && (sapling == iType && //it's not diam10 or dbh for sapling
                    iTreeCode != mp_iDbhCode[iSpecies][iType] && iTreeCode != mp_iDiam10Code[iSpecies][iType])
                    && ((adult == iType || snag == iType || woody_debris == iType)
                        && iTreeCode != mp_iDbhCode[iSpecies][iType]))
        {
          cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          fTemp = atof(cData);
          //Make sure it's a valid float
          if (0 == fTemp && cData[0] != '0')
          {
            modelErr stcErr;
            stcErr.sFunction = "clTreePopulation::CreateTreeFromTreeMap" ;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sMoreInfo = "Invalid float value in map: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          p_oTree->SetValue(iTreeCode, fTemp);
        }
      } //end of for (j = 0; j < p_oChildren->getLength(); j++)
    } //end of for (i = 0; i < iNumTrees; i++)


    // Clean up
    cleanup:
    //Delete the code translation arrays
    for (i = 0; i < m_iNumSpecies; i++)
    {
      for (j = 0; j < m_iNumTypes; j++)
      {
        delete[] p_ints[i][j]; p_ints[i][j] = NULL;
        delete[] p_floats[i][j]; p_floats[i][j] = NULL;
        delete[] p_chars[i][j]; p_chars[i][j] = NULL;
        delete[] p_bools[i][j]; p_bools[i][j] = NULL;
      }
      delete[] p_iNumInts[i]; p_iNumInts[i] = NULL;
      delete[] p_iNumFloats[i]; p_iNumFloats[i] = NULL;
      delete[] p_iNumChars[i]; p_iNumChars[i] = NULL;
      delete[] p_iNumBools[i]; p_iNumBools[i] = NULL;
      delete[] p_ints[i]; p_ints[i] = NULL;
      delete[] p_floats[i]; p_floats[i] = NULL;
      delete[] p_chars[i]; p_chars[i] = NULL;
      delete[] p_bools[i]; p_bools[i] = NULL;
    }
    delete[] p_iNumInts; p_iNumInts = NULL;
    delete[] p_iNumFloats; p_iNumFloats = NULL;
    delete[] p_iNumChars; p_iNumChars = NULL;
    delete[] p_iNumBools; p_iNumBools = NULL;
    delete[] p_ints; p_ints = NULL;
    delete[] p_floats; p_floats = NULL;
    delete[] p_chars; p_chars = NULL;
    delete[] p_bools; p_bools = NULL;
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::CreateTreesFromTreeMap" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// Find()
/////////////////////////////////////////////////////////////////////////////*/
clTreeSearch * clTreePopulation::Find(string sArgs) {
  using namespace std;
  stcOpenSearches * p_prevRecord = NULL, * p_nextRecord = NULL, * p_newRecord = NULL; //to add a new search object to the searches list
  clTreeSearch * p_results = NULL;
  //holders for the possible search parameters
  string strTempArg, strArgType, strArgVal, strTemp; //for use by anybody - but don't plan on needing it later
  float fTemp; //for use by anybody - but don't plan on needing it later
  string::size_type pos;
  int iSpecies, iType;
  unsigned long int iTemp; //for use by anybody - but don't plan on needing
  //it later

  //Make sure we have results - if we don't return NULL
  if (sArgs.length() == 0) return p_results;

  //***********************************************
  //Get our new tree search object going
  //***********************************************
  p_results = new clTreeSearch(this, mp_oSimManager->GetPlotObject());

  //Add the new search to the search requests list
  //Get the initial dummy record
  p_prevRecord = mp_openSearches;
  //Get the record it links to as next
  p_nextRecord = p_prevRecord->p_nextSearch;
  //Make the new record and insert it in between the two
  p_newRecord = new stcOpenSearches;
  p_newRecord->p_oSearch = p_results;
  p_prevRecord->p_nextSearch = p_newRecord;
  p_newRecord->p_nextSearch = p_nextRecord;

  //**************************************************
  //Parse out the argument string - loop through it and process the arguments
  //one at a time
  //**************************************************
  while (sArgs.length() > 0)
  {

    //Bust off the first argument and pass it as strTempArg - and delete it
    //from sArgs
    strTempArg = sArgs.substr(0, sArgs.find_first_of("::"));
    pos = sArgs.find_first_of("::") != string::npos ? sArgs.find_first_of("::") + 2 : sArgs.length();
    sArgs.erase(0, pos);

    //strTempArg now holds the first argument - figure out what kind it is
    strArgType = strTempArg.substr(0, strTempArg.find_first_of("="));
    strArgVal = strTempArg.substr(strTempArg.find_first_of("=") + 1);

    // TYPE //
    if ("type" == strArgType)
    {
      if ("" != strArgVal)
      {
        p_results->m_bTypeUsed = true;
        //parse out the values
        while (strArgVal.length() > 0)
        {
          strTemp = strArgVal.substr(0, strArgVal.find_first_of(","));
          //Place a bit in the position corresponding to the type's enum
          //value, counting from the right.  (So if it's "adult" and
          //adult's enum is 3, place a 1 in the third bit from the right.)
          iType = atoi(strTemp.c_str());
          if ((0 == iType && "0" != strTemp) || iType < 0 || iType > clTreePopulation::woody_debris)
          {
            modelErr stcErr;
            stcErr.sFunction = "clTreePopulation::Find" ;
            stcErr.iErrorCode = BAD_DATA;
            std::stringstream s;
            s << "Unrecognized type: " << iType;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          iTemp = (int)pow(2, (double)iType);
          p_results->m_iWhatTypes = p_results->m_iWhatTypes | iTemp;
          pos = strArgVal.find_first_of(",") != string::npos ? strArgVal.find_first_of(",") + 1 : strArgVal.length();
          strArgVal.erase(0, pos);
        }
      } //end of if ("" != strArgVal)
    } //end of if ("type" == strArgType)

    // SPECIES //
    else if ("species" == strArgType)
    {
      if ("" != strArgVal)
      {
        p_results->m_bSpeciesUsed = true;
        while (strArgVal.length() > 0)
        {
          //trim out the species numbers, starting with the first
          strTemp = strArgVal.substr(0, strArgVal.find_first_of(","));
          //convert the species number from a string to an integer
          iSpecies = atoi(strTemp.c_str());
          //Make sure it's a valid species - if not throw an error
          if (iSpecies < 0 || iSpecies >= m_iNumSpecies)
          {
            modelErr stcErr;
            stcErr.sFunction = "clTreePopulation::Find" ;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sMoreInfo = "Unrecognized species number: ";
            stcErr.sMoreInfo += strTemp;
            throw(stcErr);
          }
          p_results->mp_bWhatSpecies[iSpecies] = true;

          //remove the first value, leaving the others in the string
          pos = strArgVal.find_first_of(",");
          pos = strArgVal.find_first_of(",") != string::npos ? strArgVal.find_first_of(",") + 1 : strArgVal.length();
          strArgVal.erase(0, pos);
        } //end of while (strArgVal.length() > 0)
      } //end of if ("" != strArgVal)
    } //end of else if ("species" == strArgType)

    // HEIGHT //
    else if ("height" == strArgType)
    {
      if ("" != strArgVal)
      {
        //Try to translate the value to a float
        fTemp = atof(strArgVal.c_str());
        //If it's 0 - either it wasn't a number or it was really 0 - check
        //string length to know
        if (0 == fTemp && "0" != strArgVal.substr(0, 1))
        {
          //untranslatable text - error
          modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::Find" ;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sMoreInfo = "Unrecognized float value: ";
          stcErr.sMoreInfo += strArgVal;
          throw(stcErr);
        }
        p_results->m_fHeightCutoff = fTemp;
        p_results->m_bDistanceHeightUsed = true;
      } //end of if ("" != strArgVal)
    } //end of else if ("height" == strArgType)

    // DISTANCE //
    else if ("distance" == strArgType)
    {
      if ("" != strArgVal)
      {
        p_results->m_bDistanceHeightUsed = true;
        //get the distance value
        fTemp = atof(strArgVal.substr(0, strArgVal.find("FROM")).c_str());
        p_results->m_fDistanceCutoff = fTemp;
        //get the X and Y
        fTemp = atof(strArgVal.substr(strArgVal.find("x=") + 2, strArgVal.find("Y=")).c_str());
        p_results->m_fFromX = fTemp;
        fTemp = atof(strArgVal.substr(strArgVal.find("y=") + 2).c_str());
        p_results->m_fFromY = fTemp;
      } //end of if ("" != strArgVal)
    } //end of else if ("distance" == strArgType)

    /** ALL */
    else if ("all" == strArgType)
    {
      p_results->m_bAllUsed = true;
    }

    else
    {
      //error condition
      modelErr stcErr;
      stcErr.sFunction = "clTreePopulation::Find" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Unknown search argument: ";
      stcErr.sMoreInfo += strArgType;
      throw(stcErr);
    }
  } //end of while (sArgs.length() > 0)

  //Now we have parsed all our search arguments and passed them to our tree
  //search object.  Trigger the tree search object's Setup() method.
  p_results->Setup();

  return p_results;
}


//////////////////////////////////////////////////////////////////////////////
// UpdateTree()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTree(clTree * p_oTree, short int iCode, int iValue) {
  //For ints - no allometry - so just update the value
  try
  {
    //Make sure the code is a good one - if not, throw error
    if (iCode < 0 || iCode >= mp_iNumTreeIntVals[p_oTree->m_iSpecies][p_oTree->m_iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::UpdateTree(int)" ;
      std::stringstream s;
      s << "Invalid data code: " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    p_oTree->mp_iIntValues[iCode] = iValue;
  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree(int)" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// UpdateTree()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTree(clTree *p_oTree, short int iCode, string sValue) {
  //For strings - no allometry - so just update the value
  try
  {
    //Make sure the code is a good one - if not, throw error
    if (iCode < 0 || iCode >= mp_iNumTreeStringVals[p_oTree->m_iSpecies][p_oTree->m_iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::UpdateTree(string)" ;
      std::stringstream s;
      s << "Invalid data code: " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    p_oTree->mp_sStringValues[iCode] = sValue;
  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree(char)" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// UpdateTree()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTree(clTree * p_oTree, short int iCode, bool bValue) {
  //For bools - no allometry - so just update the value
  try
  {
    //Make sure the code is a good one - if not, throw error
    if (iCode < 0 || iCode >= mp_iNumTreeBoolVals[p_oTree->m_iSpecies][p_oTree->m_iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::UpdateTree(bool)" ;
      std::stringstream s;
      s << "Invalid data code: " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    p_oTree->mp_bBoolValues[iCode] = bValue;
  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree(int)" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// UpdateTree - float()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTree(clTree * p_oTree, short int iCode,
    float fValue, bool bUpdateNow, bool bUpdateAllometry) {
  try
  {
    float fAllomValue; //for calculating allometric values
    unsigned short int iSp = p_oTree->GetSpecies(), iType = p_oTree->GetType();
    bool bAllometryUpdated = false;

    //Make sure the code is a good one - if not, throw error
    if (iCode < 0 || iCode >= mp_iNumTreeFloatVals[iSp][iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::UpdateTree(float)" ;
      std::stringstream s;
      s << "Invalid data code: " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Verify that we're not trying to change X or Y - if so, throw an error.
    if (iCode == mp_iXCode[iSp][iType] || iCode == mp_iYCode[iSp][iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = ILLEGAL_OP;
      stcErr.sFunction = "clTreePopulation::UpdateTree" ;
      stcErr.sMoreInfo = "X and Y cannot be changed.";
      throw(stcErr);
    }

    if (bUpdateAllometry)
    {

      //*******************************
      // Update the tree's allometry
      //*******************************
      //>>>>>>>>>>> Tree is a seedling <<<<<<<<<<<
      if (seedling == p_oTree->m_iType)
      {
        if (mp_iDiam10Code[iSp][iType] == iCode)
        { //update diam10

          //reject a value below the minimum
          if (fValue < MINDIAM) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set seedling diameter below minimum.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[iCode] = fValue;
          //calculate height based on diam10
          fAllomValue = mp_oAllom->CalcSeedlingHeight(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          bAllometryUpdated = true;
        }
        else if (mp_iHeightCode[iSp][iType] == iCode)
        {
          //Do not allow negative values
          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set seedling height to negative value.";
            throw(stcErr);
          }

          p_oTree->mp_fFloatValues[iCode] = fValue;
          //calculate diam10 based on height
          fAllomValue = mp_oAllom->CalcSeedlingDiam10(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
          bAllometryUpdated = true;
        }
        else
        { //this is not an allometric value - just assign it
          p_oTree->mp_fFloatValues[iCode] = fValue;
          return;
        }
        //Check to see if this seedling is big enough to be a sapling
        if (p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] > mp_fMaxSeedlingHeight[iSp])
        {
          //Re-do the allometry based on sapling equations, keeping constant the
          //thing that was assigned
          if (mp_iDiam10Code[iSp][iType] == iCode)
          { //update diam10
            //convert diam10 to dbh
            fAllomValue = mp_oAllom->ConvertDiam10ToDbh(fValue, iSp);
            //calculate height based on diam10
            fAllomValue = mp_oAllom->CalcSaplingHeight(fAllomValue, iSp);
            p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;

          }
          else if (mp_iHeightCode[iSp][iType] == iCode)
          { //update height
            //calculate dbh based on height
            fAllomValue = mp_oAllom->CalcSaplingDbh(fValue, iSp);
            //Convert dbh to diam10 and assign
            fAllomValue = mp_oAllom->ConvertDbhToDiam10(fAllomValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
          }
          ChangeTreeType(p_oTree, sapling);

          //Just in case - is the sapling big enough to be an adult?  If so,
          //convert it a second time
          if (p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][sapling]] >= mp_fMinAdultDbh[iSp])
          {

            //Convert the type and species codes to sapling
            if (mp_iDiam10Code[iSp][iType] == iCode) //diam10 was updated
              iCode = mp_iDbhCode[iSp][sapling]; //make it the dbh code
            else
              iCode = mp_iHeightCode[iSp][sapling]; //height code instead
            iType = sapling;

            //Re-do the allometry based on adult equations, keeping constant the
            //thing that was assigned
            if (mp_iDiam10Code[iSp][iType] == iCode)
            { //update diam10
              //calculate height based on dbh
              fAllomValue = mp_oAllom->CalcAdultHeight(p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]], iSp);
              p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
            }
            else if (mp_iHeightCode[iSp][iType] == iCode)
            { //update height
              //calculate dbh based on height
              fAllomValue = mp_oAllom->CalcAdultDbh(fValue, iSp);
              p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
            }
            else if (mp_iDbhCode[iSp][iType] == iCode)
            { //update dbh
              //calculate height based on dbh
              fAllomValue = mp_oAllom->CalcAdultHeight(fValue, iSp);
              p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
            }
            ChangeTreeType(p_oTree, adult);
          }
        }

      } //end of if (seedling == p_oTree->m_iTreeType)


      //>>>>>>>>>>> Tree is a sapling <<<<<<<<<<<
      else if (sapling == p_oTree->m_iType)
      {
        if (mp_iDiam10Code[iSp][iType] == iCode)
        { //update diam10

          //reject a value below the minimum
          if (fValue < m_fNewSeedlingDiam10) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set sapling diameter below minimum.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[iCode] = fValue;
          //calculate dbh based on diam10
          fAllomValue = mp_oAllom->ConvertDiam10ToDbh(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
          //calculate height based on dbh
          fAllomValue = mp_oAllom->CalcSaplingHeight(fAllomValue, iSp);
          p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          bAllometryUpdated = true;

        }
        else if (mp_iHeightCode[iSp][iType] == iCode)
        { //update height

          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set sapling height to negative value.";
            throw(stcErr);
          }
          //Does this height make the tree shorter than the max seedling height?
          if (fValue < mp_fMaxSeedlingHeight[iSp])
          {
            //Assign height
            p_oTree->mp_fFloatValues[iCode] = fValue;
            //Re-do the allometry based on seedling equations, keeping height
            //constant
            //calculate diam10 based on height and assign
            fAllomValue = mp_oAllom->CalcSeedlingDiam10(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
            ChangeTreeType(p_oTree, seedling);
            bAllometryUpdated = true;
          }
          else
          { //normal - no shrinking back to seedling - update
            p_oTree->mp_fFloatValues[iCode] = fValue;
            //calculate dbh based on height
            fAllomValue = mp_oAllom->CalcSaplingDbh(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
            //calculate diam10 based on dbh
            fAllomValue = mp_oAllom->ConvertDbhToDiam10(fAllomValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
            bAllometryUpdated = true;
          }

        }
        else if (mp_iDbhCode[iSp][iType] == iCode)
        { //update dbh
          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set sapling DBH to negative value.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fValue;
          //calculate diam10 based on dbh
          fAllomValue = mp_oAllom->ConvertDbhToDiam10(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
          //calculate height based on dbh
          fAllomValue = mp_oAllom->CalcSaplingHeight(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          bAllometryUpdated = true;
        }
        else
        { //this is not an allometric value - just assign it
          p_oTree->mp_fFloatValues[iCode] = fValue;
          return;
        }
        //Check to see if this sapling is big enough to be an adult
        if (p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] >= mp_fMinAdultDbh[iSp])
        {
          //Re-do the allometry based on adult equations, keeping constant the
          //thing that was assigned
          if (mp_iDiam10Code[iSp][iType] == iCode)
          { //update diam10
            //calculate height based on dbh
            fAllomValue = mp_oAllom->CalcAdultHeight(p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]], iSp);
            p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          }
          else if (mp_iHeightCode[iSp][iType] == iCode)
          { //update height
            //calculate dbh based on height
            fAllomValue = mp_oAllom->CalcAdultDbh(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
          }
          else if (mp_iDbhCode[iSp][iType] == iCode)
          { //update dbh
            //calculate height based on dbh
            fAllomValue = mp_oAllom->CalcAdultHeight(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          }
          ChangeTreeType(p_oTree, adult);
        }
      } //end of else if (sapling == p_oTree->m_iTreeType)


      //>>>>>>>>>>> Tree is an adult or snag <<<<<<<<<<<
      else if (adult == p_oTree->m_iType || snag == p_oTree->m_iType)
      {
        if (mp_iHeightCode[iSp][iType] == iCode)
        { //update height
          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set adult height to negative value.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[iCode] = fValue;
          //calculate dbh based on height
          fValue = mp_oAllom->CalcAdultDbh(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fValue;
          bAllometryUpdated = true;
        }
        else if (mp_iDbhCode[iSp][iType] == iCode)
        { //update dbh
          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set adult DBH to negative value.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fValue;
          //calculate height based on dbh
          fValue = mp_oAllom->CalcAdultHeight(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fValue;
          bAllometryUpdated = true;
        }
        else
        { //this is not an allometric value - just assign it
          p_oTree->mp_fFloatValues[iCode] = fValue;
          return;
        }
        //Check dbh - smaller than sapling cutoff if adult?  If so, send this back to
        //sapling status
        if (adult == p_oTree->m_iType && p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] < mp_fMinAdultDbh[iSp])
        {
          //Re-do the allometry based on sapling equations, keeping constant the
          //thing that was assigned
          if (mp_iDbhCode[iSp][iType] == iCode)
          { //update dbh
            //calculate height based on dbh
            fAllomValue = mp_oAllom->CalcSaplingHeight(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          }
          else if (mp_iHeightCode[iSp][iType] == iCode)
          { //update height
            //calculate dbh based on height and assign
            fAllomValue = mp_oAllom->CalcSaplingDbh(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
          }
          ChangeTreeType(p_oTree, sapling);
        }
      } //end of else if (adult == p_oTree->m_iTreeType)

      //>>>>>>>>>>> Tree is an another type <<<<<<<<<<<
      else
      {
        //throw an error that this tree is not an expected type - this is mostly
        //to make sure we find this if we're adding other types
        modelErr stcErr;
        stcErr.iErrorCode = TREE_WRONG_TYPE;
        stcErr.sFunction = "clTreePopulation::UpdateTree" ;
        throw(stcErr);
      }
    }
    else
    {

      //Don't automatically update allometry

      //Check to see if we're supposed to update height - if we are,
      //set the "allometry updated" flag so the hash table will be
      //resorted
      if (mp_iHeightCode[iSp][iType] == iCode)
      {
        bAllometryUpdated = true;
      }

      p_oTree->mp_fFloatValues[iCode] = fValue;

      //If this is a sapling, and we're updating either DBH or diam10, update
      //the other
      if (sapling == p_oTree->m_iType)
      {
        if (mp_iDiam10Code[iSp][iType] == iCode)
        {
          //calculate dbh based on diam10
          fAllomValue = mp_oAllom->ConvertDiam10ToDbh(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
        }
        else if (mp_iDbhCode[iSp][iType] == iCode)
        {
          //calculate diam10 based on dbh
          fAllomValue = mp_oAllom->ConvertDbhToDiam10(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
        }
      }

      if (seedling == p_oTree->m_iType && p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] > mp_fMaxSeedlingHeight[iSp])
      {
        //This height makes the tree taller than the max seedling height
        ChangeTreeType(p_oTree, sapling);
      }
      else if (sapling == p_oTree->m_iType && mp_iHeightCode[iSp][iType] == iCode && fValue < mp_fMaxSeedlingHeight[iSp])
      {
        //This height makes the tree shorter than the max seedling height
        ChangeTreeType(p_oTree, seedling);
      }
      //Check to see if this sapling is big enough to be an adult
      else if (sapling == p_oTree->m_iType && p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] >= mp_fMinAdultDbh[iSp])
      {
        ChangeTreeType(p_oTree, adult);
      }
      else if (adult == p_oTree->m_iType && p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] < mp_fMinAdultDbh[iSp])
      {
        ChangeTreeType(p_oTree, sapling);
      }
    }

    //If we're supposed to update now, do it
    if (bAllometryUpdated)
    {
      if (bUpdateNow)
        UpdateTreeInHashTable(p_oTree);
      else
        m_bDoUpdates = true;
    }

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// ChangeTreeType()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::ChangeTreeType(clTree * p_oTree, enum iTreeType iNewType) {
  try
  {
    //Copies of the old arrays
    float * p_fFloatCopy = NULL;
    int * p_iIntCopy = NULL;
    string * p_sStringCopy = NULL;
    bool * p_bBoolCopy = NULL;
    unsigned short int iSp, iOldType, i, j;

    iSp = p_oTree->m_iSpecies;
    iOldType = p_oTree->m_iType;
    //Declare the copy arrays and copy in the tree's values
    p_fFloatCopy = new float[mp_iNumTreeFloatVals[iSp][iOldType]];
    for (i = 0; i < mp_iNumTreeFloatVals[iSp][iOldType]; i++)
      p_fFloatCopy[i] = p_oTree->mp_fFloatValues[i];
    p_iIntCopy = new int[mp_iNumTreeIntVals[iSp][iOldType]];
    for (i = 0; i < mp_iNumTreeIntVals[iSp][iOldType]; i++)
      p_iIntCopy[i] = p_oTree->mp_iIntValues[i];
    p_sStringCopy = new string[mp_iNumTreeStringVals[iSp][iOldType]];
    for (i = 0; i < mp_iNumTreeStringVals[iSp][iOldType]; i++)
      p_sStringCopy[i] = p_oTree->mp_sStringValues[i];
    p_bBoolCopy = new bool[mp_iNumTreeBoolVals[iSp][iOldType]];
    for (i = 0; i < mp_iNumTreeBoolVals[iSp][iOldType]; i++)
      p_bBoolCopy[i] = p_oTree->mp_bBoolValues[i];

    //Delete out the tree's old variable array and redeclare
    delete[] p_oTree->mp_fFloatValues; p_oTree->mp_fFloatValues = NULL;
    delete[] p_oTree->mp_iIntValues; p_oTree->mp_iIntValues = NULL;
    delete[] p_oTree->mp_sStringValues; p_oTree->mp_sStringValues = NULL;
    delete[] p_oTree->mp_bBoolValues; p_oTree->mp_bBoolValues = NULL;
    p_oTree->mp_fFloatValues = new float[mp_iNumTreeFloatVals[iSp][iNewType]];
    for (i = 0; i < mp_iNumTreeFloatVals[iSp][iNewType]; i++)
      p_oTree->mp_fFloatValues[i] = 0.0;
    p_oTree->mp_iIntValues = new int[mp_iNumTreeIntVals[iSp][iNewType]];
    for (i = 0; i < mp_iNumTreeIntVals[iSp][iNewType]; i++)
      p_oTree->mp_iIntValues[i] = 0;
    p_oTree->mp_sStringValues = new string[mp_iNumTreeStringVals[iSp][iNewType]];
    for (i = 0; i < mp_iNumTreeStringVals[iSp][iNewType]; i++)
      p_oTree->mp_sStringValues[i] = "";

    p_oTree->mp_bBoolValues = new bool[mp_iNumTreeBoolVals[iSp][iNewType]];
    for (i = 0; i < mp_iNumTreeBoolVals[iSp][iNewType]; i++)
      p_oTree->mp_bBoolValues[i] = false;

    //Assign the new type
    p_oTree->m_iType = iNewType;

    //Now any other values that are present in both types
    for (i = 0; i < mp_iNumTreeIntVals[iSp][iOldType]; i++)
      for (j = 0; j < mp_iNumTreeIntVals[iSp][iNewType]; j++)
        if (mp_sIntLabels[iSp][iNewType][j].compare(mp_sIntLabels[iSp][iOldType][i]) == 0)
          p_oTree->mp_iIntValues[j] = p_iIntCopy[i];
    for (i = 0; i < mp_iNumTreeFloatVals[iSp][iOldType]; i++)
      for (j = 0; j < mp_iNumTreeFloatVals[iSp][iNewType]; j++)
        if (mp_sFloatLabels[iSp][iNewType][j].compare(mp_sFloatLabels[iSp][iOldType][i]) == 0)
          p_oTree->mp_fFloatValues[j] = p_fFloatCopy[i];
    for (i = 0; i < mp_iNumTreeStringVals[iSp][iOldType]; i++)
      for (j = 0; j < mp_iNumTreeStringVals[iSp][iNewType]; j++)
        if (mp_sStringLabels[iSp][iNewType][j].compare(mp_sStringLabels[iSp][iOldType][i]) == 0)
          p_oTree->mp_sStringValues[j] = p_sStringCopy[i];
    for (i = 0; i < mp_iNumTreeBoolVals[iSp][iOldType]; i++)
      for (j = 0; j < mp_iNumTreeBoolVals[iSp][iNewType]; j++)
        if (mp_sBoolLabels[iSp][iNewType][j].compare(mp_sBoolLabels[iSp][iOldType][i]) == 0)
          p_oTree->mp_bBoolValues[j] = p_bBoolCopy[i];

    //If seedling to sapling, do a dbh
    if (iOldType == seedling && iNewType == sapling)
      p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iNewType]] =
          mp_oAllom->ConvertDiam10ToDbh(p_fFloatCopy[mp_iDiam10Code[iSp][iOldType]], iSp);

    //If adult to sapling, do a diam10
    if (iOldType == adult && iNewType == sapling)
      p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iNewType]] =
          mp_oAllom->ConvertDbhToDiam10(p_fFloatCopy[mp_iDbhCode[iSp][iOldType]], iSp);

    //Delete the copy arrays
    delete[] p_fFloatCopy;
    delete[] p_iIntCopy;
    delete[] p_sStringCopy;
    delete[] p_bBoolCopy;

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetMinAdultDBH()
//////////////////////////////////////////////////////////////////////////////
float clTreePopulation::GetMinAdultDBH(int iSpecies) {
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies)
  {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetMinAdultDBH" ;
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return mp_fMinAdultDbh[iSpecies];
}


///////////////////////////////////////////////////////////////////////////
// AddTreeToHashTable()
// Note : This could be made a bit faster by reordering the if - elses such
// that the most common case occurs first - i.e.a tree to be added to the
// middle of a list - and the least common case last - i.e.no trees in a list.
//////////////////////////////////////////////////////////////////////////*/
void clTreePopulation::AddTreeToHashTable(clTree * p_oNewTree) {
  try
  {
    clTree * p_oNextTree = NULL, * p_oTreeHolder = NULL;
    int iX = 0, iY = 0, iHeightDiv = 0, iSp, iType;
    float fHeight, fNeighHeight, fX, fY;

    iSp = p_oNewTree->GetSpecies();
    iType = p_oNewTree->GetType();
    p_oNewTree->GetValue(mp_iHeightCode[iSp][iType], & fHeight);
    //Validate that the coordinates are within the plot - if they're not delete
    //the tree and return an error.  We need to delete the tree now because it
    //doesn't have an "owner" to delete it in its destructor
    p_oNewTree->GetValue(mp_iXCode[iSp][iType], & fX);
    p_oNewTree->GetValue(mp_iYCode[iSp][iType], & fY);
    if (fX > m_fPlotLengthX || fX < 0 || fY > m_fPlotLengthY || fY < 0)
    {
      modelErr stcErr;
      stcErr.sFunction = "clTreePopulation::AddTreeToHashTable" ;
      stcErr.iErrorCode = BAD_DATA;
      delete p_oNewTree;
      std::stringstream s;
      s << "Tree outside plot: X = " << fX << " Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //First calculate the coordinates of the grid cell this tree will be in
    iX = (int)(fX / m_iLengthGrids);
    iY = (int)(fY / m_iLengthGrids);

    //Get the height division this tree will be in - divide by size of height div
    iHeightDiv = (int)(fHeight / m_iSizeHeightDivs);
    if (iHeightDiv > m_iNumHeightDivs - 1) iHeightDiv = m_iNumHeightDivs - 1;

    //Go through that grid cell and insert the tree in the right place
    p_oNextTree = mp_oTreeShortest[iX][iY][iHeightDiv];
    if (p_oNextTree)
      p_oNextTree->GetValue(mp_iHeightCode[p_oNextTree->GetSpecies()][p_oNextTree->GetType()], & fNeighHeight);
    if (p_oNextTree == NULL)
    { //no trees in this grid cell and height division
      mp_oTreeTallest[iX][iY][iHeightDiv] = p_oNewTree;
      mp_oTreeShortest[iX][iY][iHeightDiv] = p_oNewTree;
      p_oNewTree->mp_oNext = NULL;
      p_oNewTree->mp_oPrevious = NULL;
      //check the height divisions above and below to link across divisions -
      //this will set the link even if some divisions are empty
      for (int i = iHeightDiv - 1; i >= 0; i--)
      {
        p_oNextTree = mp_oTreeTallest[iX][iY][i];
        if (p_oNextTree != NULL)
        {
          p_oNewTree->mp_oPrevious = p_oNextTree;
          p_oNextTree->mp_oNext = p_oNewTree;
          break;
        }
      }
      p_oNextTree = NULL;
      for (int i = iHeightDiv + 1; i < m_iNumHeightDivs; i++)
      {
        p_oNextTree = mp_oTreeShortest[iX][iY][i];
        if (p_oNextTree != NULL)
        {
          p_oNewTree->mp_oNext = p_oNextTree;
          p_oNextTree->mp_oPrevious = p_oNewTree;
          break;
        }
      }
    } //end of if (p_oNextTree == NULL)
    else if (fNeighHeight > fHeight)
    {
      //our tree is shortest in this height division
      p_oNextTree->mp_oPrevious = p_oNewTree;
      p_oNewTree->mp_oNext = p_oNextTree;
      mp_oTreeShortest[iX][iY][iHeightDiv] = p_oNewTree;
      //link the shorter across divisions
      p_oNextTree = NULL;
      p_oNewTree->mp_oPrevious = NULL;
      for (int i = iHeightDiv - 1; i >= 0; i--)
      {
        p_oNextTree = mp_oTreeTallest[iX][iY][i];
        if (p_oNextTree != NULL)
        {
          p_oNewTree->mp_oPrevious = p_oNextTree;
          p_oNextTree->mp_oNext = p_oNewTree;
          break;
        }
      }
    } //end of else if (p_oNextTree->GetHeight() > fHeight)
    else
    { //our tree is not shortest
      while (p_oNextTree != NULL && fHeight >= fNeighHeight)
      {
        p_oNextTree = p_oNextTree->GetTaller();
        if (p_oNextTree)
          p_oNextTree->GetValue(mp_iHeightCode[p_oNextTree->GetSpecies()][p_oNextTree->GetType()], & fNeighHeight);
      }
      if (p_oNextTree == NULL)
      { //our tree is tallest in the grid cell
        p_oNextTree = mp_oTreeTallest[iX][iY][iHeightDiv];
        p_oNextTree->mp_oNext = p_oNewTree;
        p_oNewTree->mp_oPrevious = p_oNextTree;
        mp_oTreeTallest[iX][iY][iHeightDiv] = p_oNewTree;
        //look for the taller tree across height divisions
        p_oNextTree = NULL;
        p_oNewTree->mp_oNext = NULL;
        for (int i = iHeightDiv + 1; i < m_iNumHeightDivs; i++)
        {
          p_oNextTree = mp_oTreeShortest[iX][iY][i];
          if (p_oNextTree != NULL)
          {
            p_oNewTree->mp_oNext = p_oNextTree;
            p_oNextTree->mp_oPrevious = p_oNewTree;
            break;
          }
        }
      } //end of if (p_oNextTree == NULL)
      else
      { //we found a spot in the middle of the list where our tree belongs
        //p_oNextTree is now one taller than p_oNewTree
        //insert p_oNewTree between p_oNextTree and its former shorter neighbor
        p_oTreeHolder = p_oNextTree->GetShorter();
        p_oTreeHolder->mp_oNext = p_oNewTree;
        p_oNextTree->mp_oPrevious = p_oNewTree;
        p_oNewTree->mp_oNext = p_oNextTree;
        p_oNewTree->mp_oPrevious = p_oTreeHolder;
        //The next taller tree may not be in our height div - compare to current
        //tallest
        mp_oTreeTallest[iX][iY]
                             [iHeightDiv]->GetValue(mp_iHeightCode[mp_oTreeTallest[iX][iY][iHeightDiv]->GetSpecies()]
                                                                    [mp_oTreeTallest[iX][iY][iHeightDiv]->GetType()], & fNeighHeight);
        if (fNeighHeight <= fHeight)
          mp_oTreeTallest[iX][iY][iHeightDiv] = p_oNewTree;
      } //end of else
    } //end of else
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::AddTreeToHashTable" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// KillTree()
//////////////////////////////////////////////////////////////////////////////
clTree* clTreePopulation::KillTree(clTree * p_oOldTree, deadCode iReason) {

  //Add tree to the ghost tree population
  mp_oGhosts->AddTree(p_oOldTree, iReason);

  short iType = p_oOldTree->GetType();

  if (harvest == iReason && (sapling == iType || adult == iType) && mp_bMakeStump[p_oOldTree->GetSpecies()])
  {

    //If it's a sapling or adult removed because of harvest, and it's a
    //stumping species, make the tree into a stump and add it to the stump
    //list
    RemoveTreeFromHashTable(p_oOldTree);
    ChangeTreeType(p_oOldTree, stump);

    if (mp_oStumps)
      p_oOldTree->mp_oNext = mp_oStumps;

    mp_oStumps = p_oOldTree;
    return p_oOldTree;

  }
  else if (m_bMakeSnag && adult == iType
      && (natural == iReason || disease == iReason || fire == iReason ||
          storm == iReason || insects == iReason))
  {

    //If this is an adult dead of natural causes or disease, and we're making
    //snags, turn the adult into a snag
    ChangeTreeType(p_oOldTree, snag);
    //Set the age to 0 and the dead code to the dead reason code
    p_oOldTree->SetValue(mp_iAgeCode[p_oOldTree->GetSpecies()], (int)0);
    p_oOldTree->SetValue(mp_iWhyDeadCode[p_oOldTree->GetSpecies()], iReason);
    return p_oOldTree;

  }
  else
  {
    //All others - remove from memory
    RemoveTreeFromHashTable(p_oOldTree);
    delete p_oOldTree;
    return NULL;
  }
}


//////////////////////////////////////////////////////////////////////////////
// RemoveTreeFromHashTable()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::RemoveTreeFromHashTable(clTree * p_oByeTree) {
  try
  {
    clTree * p_oShorter = NULL, * p_oTaller = NULL;
    float fTempHeight, fX, fY;
    int iX = 0, iY = 0, iHeightDiv = 0;
    unsigned short int iSp, iType; //the tree's species and type codes

    iSp = p_oByeTree->GetSpecies();
    iType = p_oByeTree->GetType();
    p_oByeTree->GetValue(mp_iXCode[iSp][iType], & fX);
    p_oByeTree->GetValue(mp_iYCode[iSp][iType], & fY);
    iX = (int)floor(fX / m_iLengthGrids);
    iY = (int)floor(fY / m_iLengthGrids);
    p_oByeTree->GetValue(mp_iHeightCode[iSp][iType], & fTempHeight);
    iHeightDiv = (int)floor(fTempHeight / m_iSizeHeightDivs);
    if (iHeightDiv > m_iNumHeightDivs - 1) iHeightDiv = m_iNumHeightDivs - 1;

    //Get the trees on either side of this tree
    p_oShorter = p_oByeTree->GetShorter();
    p_oTaller = p_oByeTree->GetTaller();

    if (p_oShorter != NULL && p_oTaller != NULL)
    {
      p_oShorter->mp_oNext = p_oTaller;
      p_oTaller->mp_oPrevious = p_oShorter;
      //we need to check to see if this tree was tallest or shortest in its
      //height division - do this by checking the heights of the smaller and
      //taller trees.  Remember that links cross height divisions
      p_oShorter->GetValue(mp_iHeightCode[p_oShorter->GetSpecies()][p_oShorter->GetType()], & fTempHeight);
      if (fTempHeight < iHeightDiv * m_iSizeHeightDivs)
      {
        //deleted tree was shortest - check to see if the taller one is also in
        //the same height division
        p_oTaller->GetValue(mp_iHeightCode[p_oTaller->GetSpecies()][p_oTaller->GetType()], & fTempHeight);
        if (fTempHeight < (iHeightDiv + 1) * m_iSizeHeightDivs)
          mp_oTreeShortest[iX][iY][iHeightDiv] = p_oTaller;
        else
          mp_oTreeShortest[iX][iY][iHeightDiv] = NULL;
      }
      p_oTaller->GetValue(mp_iHeightCode[p_oTaller->GetSpecies()][p_oTaller->GetType()], & fTempHeight);
      if (iHeightDiv != (m_iNumHeightDivs - 1) && fTempHeight >= (iHeightDiv + 1) * m_iSizeHeightDivs)
      {
        //deleted tree was tallest - check to see if the shorter one is also in
        //the same height division
        p_oShorter->GetValue(mp_iHeightCode[p_oShorter->GetSpecies()][p_oShorter->GetType()], & fTempHeight);
        if (fTempHeight > iHeightDiv * m_iSizeHeightDivs)
          mp_oTreeTallest[iX][iY][iHeightDiv] = p_oShorter;
        else
          mp_oTreeTallest[iX][iY][iHeightDiv] = NULL;
      }
    }
    else if (p_oShorter == NULL && p_oTaller != NULL)
    { //tree was the shortest
      p_oTaller->mp_oPrevious = NULL;
      p_oTaller->GetValue(mp_iHeightCode[p_oTaller->GetSpecies()][p_oTaller->GetType()], & fTempHeight);
      //if the next tallest tree is in the deleted tree's height division, make
      //the smallest pointer point to it
      if (fTempHeight >= iHeightDiv * m_iSizeHeightDivs && (iHeightDiv == (m_iNumHeightDivs-1) || //if tallest height div this is enough
          fTempHeight < (iHeightDiv + 1) * m_iSizeHeightDivs))
        mp_oTreeShortest[iX][iY][iHeightDiv] = p_oTaller;
      else
      { //otherwise - this height div is now empty
        mp_oTreeShortest[iX][iY][iHeightDiv] = NULL;
        mp_oTreeTallest[iX][iY][iHeightDiv] = NULL;
      }
    }
    else if (p_oShorter != NULL && p_oTaller == NULL)
    { //our tree was the tallest
      p_oShorter->mp_oNext = NULL;
      //if the next shortest tree is in the deleted tree's height division, make
      //the tallest pointer point to it
      p_oShorter->GetValue(mp_iHeightCode[p_oShorter->GetSpecies()][p_oShorter->GetType()], & fTempHeight);
      if (fTempHeight >= iHeightDiv * m_iSizeHeightDivs && (iHeightDiv == (m_iNumHeightDivs-1) || //if tallest height div this is enough
          fTempHeight < (iHeightDiv + 1) * m_iSizeHeightDivs))
        mp_oTreeTallest[iX][iY][iHeightDiv] = p_oShorter;
      else
      { //otherwise - this height div is now empty
        mp_oTreeShortest[iX][iY][iHeightDiv] = NULL;
        mp_oTreeTallest[iX][iY][iHeightDiv] = NULL;
      }
    }
    else
    { //our tree was alone in its grid cell and size division
      mp_oTreeShortest[iX][iY][iHeightDiv] = NULL;
      mp_oTreeTallest[iX][iY][iHeightDiv] = NULL;
    }

    p_oByeTree->mp_oNext = NULL;
    p_oByeTree->mp_oPrevious = NULL;
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::RemoveTreeFromHashTable" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// UpdateTreeInHashTable()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTreeInHashTable(clTree * p_oChangedTree) {
  try
  {
    clTree * p_oTaller = NULL, * p_oShorter = NULL, * p_oOldTaller = NULL, * p_oOldShorter = NULL, * p_oHolder;
    float fHeight, fShorterHeight = 0, fTallerHeight = 0, fX, fY;
    int iX = 0, iY = 0, iOldHeightDiv = 0, iNeighHeightDiv = 0, iCurHeightDiv = 0, i;
    bool bWasShortest = false, bWasTallest = false;

    //Get trees on either side of current tree and check heights so we know
    //whether to go up or down in height
    p_oChangedTree->GetValue(mp_iHeightCode[p_oChangedTree->GetSpecies()][p_oChangedTree->GetType()], & fHeight);
    p_oTaller = p_oChangedTree->GetTaller();
    p_oShorter = p_oChangedTree->GetShorter();
    if (p_oTaller != NULL)
      p_oTaller->GetValue(mp_iHeightCode[p_oTaller->GetSpecies()][p_oTaller->GetType()], & fTallerHeight);
    if (p_oShorter != NULL)
      p_oShorter->GetValue(mp_iHeightCode[p_oShorter->GetSpecies()][p_oShorter->GetType()], & fShorterHeight);

    p_oChangedTree->GetValue(mp_iXCode[p_oChangedTree->GetSpecies()][p_oChangedTree->GetType()], & fX);
    p_oChangedTree->GetValue(mp_iYCode[p_oChangedTree->GetSpecies()][p_oChangedTree->GetType()], & fY);
    iX = (int)floor(fX / m_iLengthGrids);
    iY = (int)floor(fY / m_iLengthGrids);
    iCurHeightDiv = (int)(floor(fHeight) / m_iSizeHeightDivs);
    if (iCurHeightDiv > m_iNumHeightDivs - 1)
      iCurHeightDiv = m_iNumHeightDivs - 1;

    //Check to see if our tree is currently a height division marker
    for (i = 0; i < m_iNumHeightDivs; i++)
    {
      if (mp_oTreeTallest[iX][iY][i] == p_oChangedTree)
      {
        bWasTallest = true;
        iOldHeightDiv = i;
        p_oOldShorter = p_oChangedTree->GetShorter();
        p_oOldTaller = p_oChangedTree->GetTaller();
      }
      if (mp_oTreeShortest[iX][iY][i] == p_oChangedTree)
      {
        bWasShortest = true;
        iOldHeightDiv = i;
        p_oOldShorter = p_oChangedTree->GetShorter();
        p_oOldTaller = p_oChangedTree->GetTaller();
      }
      if (bWasTallest || bWasShortest) break;
    } //end of for (i = 0; i < m_iNumHeightDivs; i++)

    //Start by checking whether or not we need to do anything - the tree may not
    //be moving - but also make sure it hasn't moved height divisions if it was
    //the smallest or tallest
    if (((p_oTaller == NULL && p_oShorter == NULL) || (p_oTaller == NULL && fShorterHeight <= fHeight)
        || (p_oShorter == NULL && fTallerHeight >= fHeight) || (fShorterHeight <= fHeight && fTallerHeight >= fHeight))
        && ((!bWasShortest && !bWasTallest) || (iCurHeightDiv == iOldHeightDiv)))
    {
      return;
    }

    //The probability is that the tree is now taller than it used to be so check
    //that case first
    if (p_oTaller != NULL && fTallerHeight < fHeight)
    {
      //Close the gap where our tree used to be
      p_oTaller->mp_oPrevious = p_oShorter;
      if (p_oShorter) p_oShorter->mp_oNext = p_oTaller;

      //Find the new place where our tree will go - keep the last tree found
      //as a placeholder
      p_oHolder = p_oTaller;
      p_oTaller = p_oHolder->GetTaller();
      if (p_oTaller)
        p_oTaller->GetValue(mp_iHeightCode[p_oTaller->GetSpecies()][p_oTaller->GetType()], & fTallerHeight);
      while (p_oTaller != NULL && fTallerHeight <= fHeight)
      {
        p_oHolder = p_oTaller;
        p_oTaller = p_oTaller->GetTaller();
        if (p_oTaller)
          p_oTaller->GetValue(mp_iHeightCode[p_oTaller->GetSpecies()][p_oTaller->GetType()], & fTallerHeight);
      }

      //Assume we did not run out of trees
      if (p_oTaller != NULL)
      {
        //Get the shorter tree below our new taller tree - the gap between them
        //is where our new tree goes
        p_oShorter = p_oTaller->GetShorter();
        p_oShorter->mp_oNext = p_oChangedTree;
        p_oTaller->mp_oPrevious = p_oChangedTree;
        p_oChangedTree->mp_oNext = p_oTaller;
        p_oChangedTree->mp_oPrevious = p_oShorter;
      }
      //If we did run out - get our tallest tree and link it as shorter
      else
      {
        p_oShorter = p_oHolder;
        p_oShorter->mp_oNext = p_oChangedTree;
        p_oChangedTree->mp_oPrevious = p_oShorter;
        p_oChangedTree->mp_oNext = NULL;
      }
    } //end of if (p_oTaller != NULL && p_oTaller->GetHeight < fHeight)

    //The next case to test is that our tree is now shorter than it used to be
    //and it's moving down in the list
    else if (p_oShorter != NULL && fShorterHeight > fHeight)
    {
      //Close the gap where our tree used to be
      if (p_oTaller) p_oTaller->mp_oPrevious = p_oShorter;
      p_oShorter->mp_oNext = p_oTaller;

      //Find the new place where our tree will go and keep the last one found
      //as a placeholder
      p_oHolder = p_oShorter;
      p_oShorter = p_oHolder->GetShorter();
      if (p_oShorter)
        p_oShorter->GetValue(mp_iHeightCode[p_oShorter->GetSpecies()][p_oShorter->GetType()], & fShorterHeight);
      while (p_oShorter != NULL && fShorterHeight >= fHeight)
      {
        p_oHolder = p_oShorter;
        p_oShorter = p_oHolder->GetShorter();
        if (p_oShorter)
          p_oShorter->GetValue(mp_iHeightCode[p_oShorter->GetSpecies()][p_oShorter->GetType()], & fShorterHeight);
      }

      //Assume we did not run out of trees
      if (p_oShorter != NULL)
      {
        //Get the taller tree below our new shorter tree - the gap between them
        //is where our new tree goes
        p_oTaller = p_oShorter->GetTaller();
        p_oShorter->mp_oNext = p_oChangedTree;
        p_oTaller->mp_oPrevious = p_oChangedTree;
        p_oChangedTree->mp_oNext = p_oTaller;
        p_oChangedTree->mp_oPrevious = p_oShorter;
      }
      //If we did run out - get our shortest tree and link it as taller
      else
      {
        p_oTaller = p_oHolder;
        p_oTaller->mp_oPrevious = p_oChangedTree;
        p_oChangedTree->mp_oNext = p_oTaller;
        p_oChangedTree->mp_oPrevious = NULL;
      }
    } //end of else if (p_oShorter != NULL && fShorterHeight > fHeight)

    //Now that our tree is in the right place (or didn't need to change places
    //but did need to change height divisions), it's time for a little height
    //division management

    //If our tree was the shortest in the height division, check to see if the
    //old taller one is in the height division; if so, it's the new shortest; if
    //not, that division is now empty
    if (bWasShortest && !bWasTallest && p_oOldTaller != NULL)
    {
      p_oOldTaller->GetValue(mp_iHeightCode[p_oOldTaller->GetSpecies()][p_oOldTaller->GetType()], & fTallerHeight);
      iNeighHeightDiv = (int)(floor(fTallerHeight) / m_iSizeHeightDivs);
      if (iNeighHeightDiv > m_iNumHeightDivs - 1)
        iNeighHeightDiv = m_iNumHeightDivs - 1;
      if (iOldHeightDiv == iNeighHeightDiv)
        mp_oTreeShortest[iX][iY][iOldHeightDiv] = p_oOldTaller;
      else
      {
        mp_oTreeShortest[iX][iY][iOldHeightDiv] = NULL;
        mp_oTreeTallest[iX][iY][iOldHeightDiv] = NULL;
      }
    } //end of if (bWasShortest && !bWasTallest && p_oOldTaller != NULL)
    //if our tree was the tallest in the height division, check to see if the
    //shorter one is in the height division; if so, it's the new tallest; if
    //not, the division is now empty
    else if (bWasTallest && !bWasShortest && p_oOldShorter != NULL)
    {
      p_oOldShorter->GetValue(mp_iHeightCode[p_oOldShorter->GetSpecies()][p_oOldShorter->GetType()], & fShorterHeight);
      iNeighHeightDiv = (int)(floor(fShorterHeight) / m_iSizeHeightDivs);
      if (iNeighHeightDiv > m_iNumHeightDivs - 1)
        iNeighHeightDiv = m_iNumHeightDivs - 1;
      if (iOldHeightDiv == iNeighHeightDiv)
        mp_oTreeTallest[iX][iY][iOldHeightDiv] = p_oOldShorter;
      else
      {
        mp_oTreeShortest[iX][iY][iOldHeightDiv] = NULL;
        mp_oTreeTallest[iX][iY][iOldHeightDiv] = NULL;
      }
    } //end of else if (bWasTallest && !bWasShortest && p_oOldShorter != NULL)
    else if (bWasTallest && bWasShortest)
    {
      mp_oTreeShortest[iX][iY][iOldHeightDiv] = NULL;
      mp_oTreeTallest[iX][iY][iOldHeightDiv] = NULL;
    }

    //Check to see if our tree has become the tallest and/or shortest in its
    //height division
    //Re-using the variables p_oOldTaller and p_oOldShorter
    fTallerHeight = 0; fShorterHeight = 0;
    p_oOldTaller = mp_oTreeTallest[iX][iY][iCurHeightDiv];
    p_oOldShorter = mp_oTreeShortest[iX][iY][iCurHeightDiv];
    //Special case:  this tree is moving into a height division where there
    //is only one tree, and it is exactly the same height as that tree
    if (p_oOldTaller && p_oOldShorter && p_oOldTaller == p_oOldShorter) {
      p_oOldTaller->GetValue(mp_iHeightCode[p_oOldTaller->GetSpecies()][p_oOldTaller->GetType()], & fTallerHeight);
      if (fTallerHeight == fHeight) {
        if (p_oOldTaller == p_oChangedTree->GetShorter()) {
          //Neighbor tree was originally shorter; leave it as the shortest in
          //the height division, and set the changed tree to taller
          mp_oTreeTallest[iX][iY][iCurHeightDiv] = p_oChangedTree;
          return;
        } else if (p_oOldTaller == p_oChangedTree->GetTaller()) {
          //Neighbor tree was originally taller; leave it as the tallest in the
          //height division, and set the changed tree to shorter
          mp_oTreeShortest[iX][iY][iCurHeightDiv] = p_oChangedTree;
          return;
        }
      }
    }
    if (p_oOldTaller)
      p_oOldTaller->GetValue(mp_iHeightCode[p_oOldTaller->GetSpecies()][p_oOldTaller->GetType()], & fTallerHeight);
    if (fTallerHeight == fHeight && p_oOldTaller == p_oChangedTree->GetShorter()) //special case
      mp_oTreeTallest[iX][iY][iCurHeightDiv] = p_oChangedTree;
    if (p_oOldTaller == NULL || fTallerHeight < fHeight)
      mp_oTreeTallest[iX][iY][iCurHeightDiv] = p_oChangedTree;
    if (p_oOldShorter)
      p_oOldShorter->GetValue(mp_iHeightCode[p_oOldShorter->GetSpecies()][p_oOldShorter->GetType()], & fShorterHeight);
    if (fShorterHeight == fHeight && p_oOldShorter == p_oChangedTree->GetTaller()) //special case
      mp_oTreeShortest[iX][iY][iCurHeightDiv] = p_oChangedTree;
    //LEM 12-19-05 - I replaced the line below with the one after it (changed
    //greater than/equal to to just greater than).  The code above puts a
    //shorter tree moving up above one of equal height.  I'm nervous,
    //but I've got test code for the bug I'm fixing in this situation.  I know
    //I probably put it this way to fix an earlier bug, but I DIDN'T add
    //test code for that, so I'll have to wait for it to come round again.
    //if (p_oOldShorter == NULL || fShorterHeight >= fHeight)
    if (p_oOldShorter == NULL || fShorterHeight > fHeight)
      mp_oTreeShortest[iX][iY][iCurHeightDiv] = p_oChangedTree;
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTreeInHashTable" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// EmptyHashTable()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::EmptyHashTable() {
  try
  {
    clTree * p_oTree = NULL, * p_oNextTree = NULL;
    int i, j, k; //loop counters

    //Delete the trees
    for (i = 0; i < m_iNumXCells; i++)
      for (j = 0; j < m_iNumYCells; j++)
      {
        p_oTree = mp_oTreeShortest[i][j][0];
        if (!p_oTree)
          for (k = 1; k < m_iNumHeightDivs; k++)
          {
            p_oTree = mp_oTreeShortest[i][j][k];
            if (p_oTree)
              break;
          }
        while (p_oTree != NULL)
        {
          p_oNextTree = p_oTree->GetTaller();
          delete p_oTree;
          p_oTree = p_oNextTree;
        }
      }

    //NULL out the hash table
    for (i = 0; i < m_iNumXCells; i++)
      for (j = 0; j < m_iNumYCells; j++)
        for (k = 0; k < m_iNumHeightDivs; k++)
        {
          mp_oTreeShortest[i][j][k] = NULL;
          mp_oTreeTallest[i][j][k] = NULL;
        }
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::EmptyHashTable" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// SortHashTable
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::SortHashTable() {
  try
  {
    clTree * p_oTargetTree, * p_oNeighbor, * p_oOldShorter, * p_oOldTaller, * p_oNewShorter;
    float fTargetHeight, fNeighborHeight;
    int iTemp;
    short iHeightDiv, i, j, k; //loop counters
    bool bMove;

    //Start by sorting the trees using the insertion sort algorithm.  Start at
    //the tallest tree and work down.  Don't worry about the height division
    //pointers - only the tallest pointer, so we have a starting point.
    for (i = 0; i < m_iNumXCells; i++)
      for (j = 0; j < m_iNumYCells; j++)
      {
        for (k = m_iNumHeightDivs - 1; k >= 0; k--)
        {
          p_oTargetTree = mp_oTreeTallest[i][j][k];
          if (p_oTargetTree) break;
        }
        while (p_oTargetTree)
        { //go through all the trees from tallest down
          //Get the height of the tree we're working with
          bMove = false;
          p_oTargetTree->GetValue(mp_iHeightCode[p_oTargetTree->GetSpecies()][p_oTargetTree->GetType()], & fTargetHeight);
          //get the neighbors on either side of the target
          p_oOldShorter = p_oTargetTree->GetShorter();
          p_oOldTaller = p_oTargetTree->GetTaller();
          //the trees taller than this one are already sorted - move this tree
          //up in the list until it gets to where it belongs
          p_oNeighbor = p_oOldTaller;
          while (p_oNeighbor)
          {
            p_oNeighbor->GetValue(mp_iHeightCode[p_oNeighbor->GetSpecies()][p_oNeighbor->GetType()], & fNeighborHeight);
            //if the target is shorter than the neighbor we've found the
            //target's place in the hash table, so break the loop
            if (fNeighborHeight >= fTargetHeight) break;
            else
              bMove = true; //this lets us know we have a switch to make -
            //if the target's in the right place we'll never get to this line
            p_oNewShorter = p_oNeighbor; //we'll hold onto the tree shorter
            //than the neighbor
            p_oNeighbor = p_oNeighbor->GetTaller();
          }
          //Did we need to move?
          if (bMove)
          {
            //Close the gap between the old neighbors
            if (p_oOldShorter) p_oOldShorter->mp_oNext = p_oOldTaller;
            if (p_oOldTaller) p_oOldTaller->mp_oPrevious = p_oOldShorter;
            //If the neighbor exists, put the target as its shorter
            if (p_oNeighbor)
            {
              p_oNewShorter = p_oNeighbor->GetShorter();
              p_oNewShorter->mp_oNext = p_oTargetTree;
              p_oNeighbor->mp_oPrevious = p_oTargetTree;
              p_oTargetTree->mp_oNext = p_oNeighbor;
              p_oTargetTree->mp_oPrevious = p_oNewShorter;
            }
            else
            {
              //If the neighbor doesn't exist, the target is now tallest
              //p_oNewShorter holds the last tree
              p_oNewShorter->mp_oNext = p_oTargetTree;
              p_oTargetTree->mp_oPrevious = p_oNewShorter;
              p_oTargetTree->mp_oNext = NULL;
            }
          } //end of if (bMove)
          p_oTargetTree = p_oOldShorter;
        } //end of while (p_oTargetTree)

        //****************************************
        //Now do the height division pointers for this grid cell
        //****************************************
        for (k = 0; k < m_iNumHeightDivs; k++)
        {
          //Get a tree, any tree
          p_oTargetTree = mp_oTreeShortest[i][j][k];
          if (p_oTargetTree) break;
        }
        //NULL out all current height div pointers
        for (k = 0; k < m_iNumHeightDivs; k++)
        {
          mp_oTreeShortest[i][j][k] = NULL;
          mp_oTreeTallest[i][j][k] = NULL;
        }
        if (p_oTargetTree)
        {
          //Now work down to the smallest tree in the grid cell
          while (p_oTargetTree)
          {
            p_oOldShorter = p_oTargetTree;
            p_oTargetTree = p_oTargetTree->GetShorter();
          }
          p_oTargetTree = p_oOldShorter;
          //OK, p_oTargetTree is now the shortest in the cell.  Work up and
          //do the pointers by comparing to neighbors on either side
          while (p_oTargetTree)
          {
            p_oTargetTree->GetValue(mp_iHeightCode[p_oTargetTree->GetSpecies()][p_oTargetTree->GetType()], & fTargetHeight);
            iHeightDiv =(int)floor(fTargetHeight / m_iSizeHeightDivs);
            iHeightDiv = iHeightDiv < m_iNumHeightDivs - 1 ? iHeightDiv : m_iNumHeightDivs - 1;
            //Check taller - if in a different height div or nonexistent, the
            //target tree is the tallest in this height div
            p_oOldTaller = p_oTargetTree->GetTaller();
            if (!p_oOldTaller) mp_oTreeTallest[i][j][iHeightDiv] = p_oTargetTree;
            else
            {
              p_oOldTaller->GetValue(mp_iHeightCode[p_oOldTaller->GetSpecies()]
                                                     [p_oOldTaller->GetType()], & fNeighborHeight);
              iTemp = (int)floor(fNeighborHeight / m_iSizeHeightDivs);
              iTemp = iTemp < m_iNumHeightDivs - 1 ? iTemp : m_iNumHeightDivs - 1;
              if (iTemp != iHeightDiv)
                mp_oTreeTallest[i][j][iHeightDiv] = p_oTargetTree;
            }
            //Check shorter - if in a different height div or nonexistent, the
            //target tree is the shortest in this height div
            p_oOldShorter = p_oTargetTree->GetShorter();
            if (!p_oOldShorter) mp_oTreeShortest[i][j][iHeightDiv] = p_oTargetTree;
            else
            {
              p_oOldShorter->GetValue(mp_iHeightCode[p_oOldShorter->GetSpecies()]
                                                      [p_oOldShorter->GetType()], & fNeighborHeight);
              iTemp = (int)floor(fNeighborHeight / m_iSizeHeightDivs);
              iTemp = iTemp < m_iNumHeightDivs - 1 ? iTemp : m_iNumHeightDivs - 1;
              if (iTemp != iHeightDiv)
                mp_oTreeShortest[i][j][iHeightDiv] = p_oTargetTree;
            }
            p_oTargetTree = p_oOldTaller;
          } //end of while (p_oTargetTree)
        }
      } //end of for (j = 0; j < m_iNumYCells; j++)

    m_bDoUpdates = false;
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::SortHashTable" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// TimestepCleanup()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::TimestepCleanup() {
  try
  {
    stcOpenSearches * p_record = NULL, * p_nextRecord = NULL;
    clTreeSearch * p_oSearch = NULL;
    clTree * p_oTree;
    float fCrown = -1;
    int iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(),
        i, j, k, //loop counters
        iAge, iSp, iTp;
    //Go through and delete all open tree searches and tree search records

    //Get the initial dummy record
    p_record = mp_openSearches;
    //Loop through the others
    p_nextRecord = p_record->p_nextSearch;
    while (p_nextRecord != NULL)
    {
      p_record = p_nextRecord->p_nextSearch;

      //Delete the search object
      p_oSearch = p_nextRecord->p_oSearch;
      delete p_oSearch; p_oSearch = NULL;

      //Delete the record
      delete p_nextRecord;

      p_nextRecord = p_record;
    }
    //set the next record for the dummy record to NULL
    mp_openSearches->p_nextSearch = NULL;
    m_bDoUpdates = false;

    //Delete all stumps
    DeleteStumps();

    //Update all snag ages and clear crown dimension values
    for (i = 0; i < m_iNumXCells; i++)
      for (j = 0; j < m_iNumYCells; j++)
      {
        p_oTree = mp_oTreeShortest[i][j][0];
        if (!p_oTree)
          for (k = 1; k < m_iNumHeightDivs; k++)
          {
            p_oTree = mp_oTreeShortest[i][j][k];
            if (p_oTree)
              break;
          }
        while (p_oTree != NULL)
        {
          if (snag == p_oTree->GetType())
          {
            iSp = p_oTree->GetSpecies();
            iTp = p_oTree->GetType();
            p_oTree->GetValue(mp_iAgeCode[iSp], & iAge);
            iAge += iNumYearsPerTimestep;
            p_oTree->SetValue(mp_iAgeCode[iSp], iAge);
            p_oTree->SetValue(mp_iCrownDepthCode[iSp][iTp], fCrown);
            p_oTree->SetValue(mp_iCrownRadCode[iSp][iTp], fCrown);
          } else if (adult == p_oTree->GetType() || sapling == p_oTree->GetType()) {
            iSp = p_oTree->GetSpecies();
            iTp = p_oTree->GetType();
            p_oTree->SetValue(mp_iCrownDepthCode[iSp][iTp], fCrown);
            p_oTree->SetValue(mp_iCrownRadCode[iSp][iTp], fCrown);
          }
          p_oTree = p_oTree->GetTaller();
        }
      }

  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::TimestepCleanup" ;
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// TranslateSpeciesNameToCode()
/////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::TranslateSpeciesNameToCode(string sSpeciesName) {
  short int iCode = -1, //preset to "not found" code
      i; //loop counter

  if (NULL == mp_speciesCodes) return iCode;

  //compare the species string passed to each of the name strings in our list
  // - if there's a match set iCode to that species's code
  for (i = 0; i < m_iNumSpecies; i++)
  {
    if (mp_speciesCodes[i].sName.compare(sSpeciesName) == 0)
      iCode = mp_speciesCodes[i].iCode;
  } //end of for (i = 0; i < m_iNumSpecies; i++)

  return iCode;
}


/////////////////////////////////////////////////////////////////////////////
// TranslateSpeciesCodeToName()
/////////////////////////////////////////////////////////////////////////////
string clTreePopulation::TranslateSpeciesCodeToName(int iSpecies) {
  short int i; //loop counter

  if (NULL == mp_speciesCodes) return "";

  //compare the species code passed to each of the codes in our list - if
  //there's a match return that species's name
  for (i = 0; i < m_iNumSpecies; i++)
  {
    if (mp_speciesCodes[i].iCode == iSpecies)
      return mp_speciesCodes[i].sName;
  } //end of for (i = 0; i < m_iNumSpecies; i++)

  return "";
}


//////////////////////////////////////////////////////////////////////////////
// CreateTree()
/////////////////////////////////////////////////////////////////////////////*/
clTree * clTreePopulation::CreateTree(float fX, float fY, int iSp, int iType,
    float fDiam) {
  //Validate the data
  //if either the X or Y coordinate is negative or larger than the plot
  //length, throw an error
  if (fX < 0 || fY < 0 || fX >= m_fPlotLengthX || fY >= m_fPlotLengthY)
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::CreateTree" ;
    std::stringstream s;
    s << "Either X or Y is outside plot.  X = " << fX << " Y = " << fY;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  //if the species is not recognized, throw an error
  if (iSp < 0 || iSp >= m_iNumSpecies)
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::CreateTree" ;
    std::stringstream s;
    s << "Unrecognized species " << iSp;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  //if the diameter is zero (or less) and this is not a seedling, throw an err
  if ((fDiam <= 0 && seedling != iType) || (fDiam < 0 && seedling == iType))
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::CreateTree" ;
    stcErr.sMoreInfo = "Diameter cannot be less than zero.";
    throw(stcErr);
  } //end of if ((fDiam <= 0 && seedling != iType)...
  clTree * p_oTree; //pointer to the new tree

  //Adjust types
  if (seedling == iType && mp_oAllom->CalcSeedlingHeight(fDiam, iSp) > mp_fMaxSeedlingHeight[iSp])
  {
    iType = sapling;
    //Transform the value to a dbh because that's what will be assigned
    fDiam = mp_oAllom->ConvertDiam10ToDbh(fDiam, iSp);
  }
  else if (sapling == iType && fDiam >= mp_fMinAdultDbh[iSp]) iType = adult;
  else if (adult == iType && fDiam < mp_fMinAdultDbh[iSp]) iType = sapling;

  if (seedling == iType)
  {
    //If the diam10 wasn't passed, get a random one
    if (0 == fDiam)
      fDiam = GetRandomDiam10Value();
    p_oTree = new clTree(iType, iSp, mp_iNumTreeFloatVals[iSp][iType], mp_iNumTreeIntVals[iSp][iType],
        mp_iNumTreeStringVals[iSp][iType], mp_iNumTreeBoolVals[iSp][iType], this);
    //Set the X and Y values directly, but let SetValue do the allometry
    //updates for us
    p_oTree->mp_fFloatValues[mp_iXCode[iSp][iType]] = fX;
    p_oTree->mp_fFloatValues[mp_iYCode[iSp][iType]] = fY;
    p_oTree->SetValue(mp_iDiam10Code[iSp][iType], fDiam, false);
    p_oTree->mp_oPrevious = NULL; p_oTree->mp_oNext = NULL;
  } //end of if (seedling == iType)
  else if (sapling == iType)
  {
    p_oTree = new clTree(iType, iSp, mp_iNumTreeFloatVals[iSp][iType], mp_iNumTreeIntVals[iSp][iType],
        mp_iNumTreeStringVals[iSp][iType], mp_iNumTreeBoolVals[iSp][iType], this);
    //Set the X and Y values directly, but let SetValue do the allometry
    //updates for us
    p_oTree->mp_fFloatValues[mp_iXCode[iSp][iType]] = fX;
    p_oTree->mp_fFloatValues[mp_iYCode[iSp][iType]] = fY;
    p_oTree->mp_fFloatValues[mp_iCrownRadCode[iSp][iType]] = -1;
    p_oTree->mp_fFloatValues[mp_iCrownDepthCode[iSp][iType]] = -1;
    p_oTree->SetValue(mp_iDbhCode[iSp][iType], fDiam, false);
    p_oTree->mp_oPrevious = NULL; p_oTree->mp_oNext = NULL;
  } //end of if (sapling == iType)
  else if (adult == iType || snag == iType)
  {
    p_oTree = new clTree(iType, iSp, mp_iNumTreeFloatVals[iSp][iType], mp_iNumTreeIntVals[iSp][iType],
        mp_iNumTreeStringVals[iSp][iType], mp_iNumTreeBoolVals[iSp][iType], this);
    //Set the X and Y values directly, but let SetValue do the allometry
    //updates for us
    p_oTree->mp_fFloatValues[mp_iXCode[iSp][iType]] = fX;
    p_oTree->mp_fFloatValues[mp_iYCode[iSp][iType]] = fY;
    p_oTree->mp_fFloatValues[mp_iCrownRadCode[iSp][iType]] = -1;
    p_oTree->mp_fFloatValues[mp_iCrownDepthCode[iSp][iType]] = -1;
    p_oTree->SetValue(mp_iDbhCode[iSp][iType], fDiam, false);
    p_oTree->mp_oPrevious = NULL; p_oTree->mp_oNext = NULL;
  } //end of if (adult == iType)
  else if (stump == iType)
  {
    p_oTree = new clTree(iType, iSp, mp_iNumTreeFloatVals[iSp][iType], mp_iNumTreeIntVals[iSp][iType],
        mp_iNumTreeStringVals[iSp][iType], mp_iNumTreeBoolVals[iSp][iType], this);

    //Set X, Y, and DBH values
    p_oTree->mp_fFloatValues[mp_iXCode[iSp][iType]] = fX;
    p_oTree->mp_fFloatValues[mp_iYCode[iSp][iType]] = fY;
    p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fDiam;

    p_oTree->mp_oNext = mp_oStumps; //NULL OK
    mp_oStumps = p_oTree;
  }
  else
  {
    //This is some other type - throw an error
    modelErr stcErr;
    stcErr.sFunction = "clTreePopulation::CreateTree" ;
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "The tree type \"" << iType << "\" is not supported.";
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }

  //Add the tree to the hash table if not a stump
  if (iType != stump)
    AddTreeToHashTable(p_oTree);

  //The hash table "sort later" flag has been set but doesn't need to be -
  //un-set it
  m_bDoUpdates = false;
  return p_oTree;
}


//////////////////////////////////////////////////////////////////////////////
// RegisterDataMember()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterDataMember(string sLabel, int iSpecies,
    int iType, short int **p_iNumTreeVals, string ***p_sLabels) {
  short int iNumVals, //number of values for this species/type combo
  iReturnCode = -1, //what we'll return
  i;

  //Check to make sure that the data label is not empty
  if (0 == sLabel.length()) {
    modelErr stcErr;
    stcErr.sFunction = "clTreePopulation::RegisterDataMember" ;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sMoreInfo = "Data label is required.";
    throw(stcErr);
  }

  //Find the first open space for this type and species
  iNumVals = p_iNumTreeVals[iSpecies][iType];
  for (i = 0; i < iNumVals; i++) {
    if (0 == p_sLabels[iSpecies][iType][i].length()) {
      iReturnCode = i;
      break;
    }
    if (p_sLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::RegisterDataMember" ;
          stcErr.iErrorCode = ILLEGAL_OP;
          stcErr.sMoreInfo = "Data member already registered";
          throw(stcErr);
    }
  }

  //If we didn't find anything, throw an error
  if (-1 == iReturnCode)
  {
    modelErr stcErr;
    stcErr.sFunction = "clTreePopulation::RegisterDataMember" ;
    stcErr.iErrorCode = ILLEGAL_OP;
    stcErr.sMoreInfo = "Too many tree data member registrations";
    throw(stcErr);
  }

  p_sLabels[iSpecies][iType][iReturnCode] = sLabel;
  return iReturnCode;
}


//////////////////////////////////////////////////////////////////////////////
// RegisterInt()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterInt(string sLabel, int iSpecies, int iType) {
  return RegisterDataMember(sLabel, iSpecies, iType, mp_iNumTreeIntVals,
      mp_sIntLabels);
}



//////////////////////////////////////////////////////////////////////////////
// RegisterFloat()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterFloat(string sLabel, int iSpecies, int iType) {
  return RegisterDataMember(sLabel, iSpecies, iType, mp_iNumTreeFloatVals,
      mp_sFloatLabels);
}



//////////////////////////////////////////////////////////////////////////////
// RegisterChar()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterChar(string sLabel, int iSpecies, int iType) {
  return RegisterDataMember(sLabel, iSpecies, iType, mp_iNumTreeStringVals,
      mp_sStringLabels);
}



//////////////////////////////////////////////////////////////////////////////
// RegisterBool()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterBool(string sLabel, int iSpecies, int iType) {
  return RegisterDataMember(sLabel, iSpecies, iType, mp_iNumTreeBoolVals,
      mp_sBoolLabels);
}



//////////////////////////////////////////////////////////////////////////////
// GetDataCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetIntDataCode(string sLabel, int iSpecies,
    int iType) {
  short int iReturnCode = -1, i;

  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetIntDataCode" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //Make sure the label passed wasn't an empty string
  if (0 == sLabel.length())
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDataCode" ;
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < mp_iNumTreeIntVals[iSpecies][iType]; i++)
    if (mp_sIntLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}

//----------------------------------------------------------------------------
short int clTreePopulation::GetFloatDataCode(string sLabel, int iSpecies,
    int iType) {
  short int iReturnCode = -1, i;

  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetFloatDataCode" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //Make sure the label passed wasn't an empty string
  if (0 == sLabel.length())
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDataCode" ;
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < mp_iNumTreeFloatVals[iSpecies][iType]; i++)
    if (mp_sFloatLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}

//----------------------------------------------------------------------------
short int clTreePopulation::GetStringDataCode(string sLabel, int iSpecies,
    int iType) {
  short int iReturnCode = -1, i;

  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetStringDataCode" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //Make sure the label passed wasn't an empty string
  if (0 == sLabel.length())
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDataCode" ;
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < mp_iNumTreeStringVals[iSpecies][iType]; i++)
    if (mp_sStringLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}

//----------------------------------------------------------------------------
short int clTreePopulation::GetBoolDataCode(string sLabel, int iSpecies, int iType) {
  short int iReturnCode = -1, i;

  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetBoolDataCode" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //Make sure the label passed wasn't an empty string
  if (0 == sLabel.length())
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDataCode" ;
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < mp_iNumTreeBoolVals[iSpecies][iType]; i++)
    if (mp_sBoolLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetDataLabel
//////////////////////////////////////////////////////////////////////////////
string clTreePopulation::GetIntDataLabel(short int iCode, int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetIntDataLabel" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= mp_iNumTreeIntVals[iSpecies][iType])
    return "";
  else
    return mp_sIntLabels[iSpecies][iType][iCode];
}

//----------------------------------------------------------------------------
string clTreePopulation::GetFloatDataLabel(short int iCode, int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetFloatDataLabel" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= mp_iNumTreeFloatVals[iSpecies][iType])
    return "";
  else
    return mp_sFloatLabels[iSpecies][iType][iCode];
}

//----------------------------------------------------------------------------
string clTreePopulation::GetStringDataLabel(short int iCode, int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetCharDataLabel" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= mp_iNumTreeStringVals[iSpecies][iType])
    return "";
  else
    return mp_sStringLabels[iSpecies][iType][iCode];
}

//----------------------------------------------------------------------------
string clTreePopulation::GetBoolDataLabel(short int iCode, int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetBoolDataLabel" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= mp_iNumTreeBoolVals[iSpecies][iType])
    return "";
  else
    return mp_sBoolLabels[iSpecies][iType][iCode];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetXCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetXCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType
      >= m_iNumTypes) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetXCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iXCode[iSpecies][iType];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetYCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetYCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType
      >= m_iNumTypes) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetYCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iYCode[iSpecies][iType];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetHeightCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetHeightCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType
      >= m_iNumTypes) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetHeightCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iHeightCode[iSpecies][iType];
}


//////////////////////////////////////////////////////////////////////////////
// GetDbhCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetDbhCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || (iType != sapling && iType
      != adult && iType != stump && iType != snag)) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDbhCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iDbhCode[iSpecies][iType];
}

//////////////////////////////////////////////////////////////////////////////
// GetCrownRadiusCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetCrownRadiusCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || (iType != sapling && iType
      != adult && iType != snag)) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetCrownRadiusCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iCrownRadCode[iSpecies][iType];
}

//////////////////////////////////////////////////////////////////////////////
// GetCrownDepthCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetCrownDepthCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || (iType != sapling && iType
      != adult && iType != snag)) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetCrownDepthCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iCrownDepthCode[iSpecies][iType];
}

//////////////////////////////////////////////////////////////////////////////
// GetAgeCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetAgeCode(int iSpecies) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetAgeCode";
    std::stringstream s;
    s << "Unrecognized species. Species = " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iAgeCode[iSpecies];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetWhyDeadCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetWhyDeadCode(int iSpecies) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetAgeCode";
    std::stringstream s;
    s << "Unrecognized species. Species = " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iWhyDeadCode[iSpecies];
}

//----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// GetDiam10Code()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetDiam10Code(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || (iType != sapling && iType
      != seedling)) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDiam10Code";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iDiam10Code[iSpecies][iType];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// DoDataUpdates()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::DoDataUpdates() {
  try
  {
    //If the bDoUpdates flag is true, sort the hash table
    if (m_bDoUpdates)
    {
      SortHashTable();
      m_bDoUpdates = false;
    }

    mp_oAllom->DoDataUpdates();
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::DoDataUpdates" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// DeleteStumps()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::DeleteStumps() {
  try
  {
    clTree * p_oStump, * p_oNextStump;

    p_oStump = mp_oStumps;
    while (p_oStump)
    {
      p_oNextStump = p_oStump->mp_oNext;
      delete p_oStump;
      p_oStump = p_oNextStump;
    }

    mp_oStumps = NULL;
  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::DeleteStumps" ;
    throw(stcErr);
  }
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// DataMemberRegistrations()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::DataMemberRegistrations() {
  try
  {

    clBehaviorBase * p_oBehavior;
    int iNumBehaviors, i;

    //Ask each behavior to register its data members
    iNumBehaviors = mp_oSimManager->GetNumberOfBehaviors();
    for (i = 0; i < iNumBehaviors; i++)
    {
      p_oBehavior = mp_oSimManager->GetBehaviorObject(i);
      p_oBehavior->RegisterTreeDataMembers();
    } //end of for (i = 0; i < iNumBehaviors; i++)

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::DataMemberRegistrations" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// CreateTreesFromTextTreeMap()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::CreateTreesFromTextTreeMap(xercesc::DOMDocument *p_oDoc) {
  using namespace std;
  string sFileName;
  string* p_sColHeader = NULL;
  enum member_type {notassigned, //Not assigned a type
    isfloat, //Float data member
    isint,   //Int data member
    ischar,  //Char data member
    isbool   //Bool data member
  } *p_iMemberType = NULL;
  fstream in;
  int iNumColumns = 0, i;
  try
  {
    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    FillSingleValue(p_oElement, "tr_treemapFile", &sFileName, false);
    if (sFileName.length() == 0) {
      return;
    }

    if (!DoesFileExist(sFileName)) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap";
      std::stringstream s;
      s << "Can't find file \"" << sFileName << "\".";
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    in.open(sFileName.c_str(), ios::in);
    clTree *p_oTree;
    string sSp, sTp, sTemp;
    float fX, fY, fDiam, fHeight = 0, fTemp;
    int iSpecies, iType, iCode, iTemp;
    std::string::size_type pos = 0, pos2 = 0;

    //Get the column headers
    getline(in, sTemp);

    //Count the number of columns
    while (pos != std::string::npos) {
      pos++;
      iNumColumns++;
      pos = sTemp.find("\t", pos);
    }
    iNumColumns -= 6; //this is how many are extra
    //Read the extra columns, if any
    if (iNumColumns > 0) {
      p_sColHeader = new string[iNumColumns];
      p_iMemberType = new member_type[iNumColumns];
      pos = 0;
      for (i = 0; i < 6; i++) {
        pos = sTemp.find("\t", pos); pos++; //skip over known columns
      }
      for (i = 0; i < iNumColumns; i++) {
        pos2 = sTemp.find("\t", pos);
        p_sColHeader[i] = sTemp.substr(pos, pos2-pos);
        pos = pos2 + 1;
        p_iMemberType[i] = notassigned;
      }
      //Figure out what kind of data member each extra column is - just make
      //sure it applies to any tree type whatsoever, and it's not
      //double-registered as different data types
      for (i = 0; i < iNumColumns; i++) {
        for (iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++) {
          for (iType = 0; iType < m_iNumTypes; iType++) {

            //Is it a float?
            if (GetFloatDataCode(p_sColHeader[i], iSpecies, iType) > -1) {
              if (notassigned == p_iMemberType[i]) {
                p_iMemberType[i] = isfloat;
              } else if (isfloat != p_iMemberType[i]) {
                modelErr stcErr;
                stcErr.iErrorCode = BAD_DATA;
                std::stringstream s;
                s << "Tree data member \"" << p_sColHeader[i] << "\" of ambiguous type.";
                stcErr.sMoreInfo = s.str();
                stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap" ;
                throw(stcErr);
              }
            }

            //Is it an int?
            if (GetIntDataCode(p_sColHeader[i], iSpecies, iType) > -1) {
              if (notassigned == p_iMemberType[i]) {
                p_iMemberType[i] = isint;
              } else if (isint != p_iMemberType[i]) {
                modelErr stcErr;
                stcErr.iErrorCode = BAD_DATA;
                std::stringstream s;
                s << "Tree data member \"" << p_sColHeader[i] << "\" of ambiguous type.";
                stcErr.sMoreInfo = s.str();
                stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap" ;
                throw(stcErr);
              }
            }

            //Is it a bool?
            if (GetBoolDataCode(p_sColHeader[i], iSpecies, iType) > -1) {
              if (notassigned == p_iMemberType[i]) {
                p_iMemberType[i] = isbool;
              } else if (isbool != p_iMemberType[i]) {
                modelErr stcErr;
                stcErr.iErrorCode = BAD_DATA;
                std::stringstream s;
                s << "Tree data member \"" << p_sColHeader[i] << "\" of ambiguous type.";
                stcErr.sMoreInfo = s.str();
                stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap" ;
                throw(stcErr);
              }
            }

            //Is it a char?
            if (GetStringDataCode(p_sColHeader[i], iSpecies, iType) > -1) {
              if (notassigned == p_iMemberType[i]) {
                p_iMemberType[i] = ischar;
              } else if (ischar != p_iMemberType[i]) {
                modelErr stcErr;
                stcErr.iErrorCode = BAD_DATA;
                std::stringstream s;
                s << "Tree data member \"" << p_sColHeader[i] << "\" of ambiguous type.";
                stcErr.sMoreInfo = s.str();
                stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap" ;
                throw(stcErr);
              }
            }
          }
        }

        //Make sure we found this data member for this tree type and
        //species
        if (notassigned == p_iMemberType[i]) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Tree data member \"%s\" missing." << p_sColHeader[i];
          stcErr.sMoreInfo = s.str();
          stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap" ;
          throw(stcErr);
        }
      }
    }

    //Read lines
    in >> fX >> fY >> sSp >> sTp >> fDiam >> fHeight;

    while (!in.eof()) {

      //Get species
      iSpecies = TranslateSpeciesNameToCode(sSp);
      if (iSpecies == -1) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap";
        std::stringstream s;
        s << "The file \"" << sFileName << "\" has an invalid species: \"" << sSp << "\".";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }

      //Get tree type
      std::transform(sTp.begin(), sTp.end(), sTp.begin(), (int(*)(int)) std::tolower);
      if (sTp == "adult") iType = adult;
      else if (sTp == "sapling") iType = sapling;
      else if (sTp == "seedling") iType = seedling;
      else if (sTp == "snag") iType = snag;
      else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap";
        std::stringstream s;
        s << "The file \"" << sFileName << "\" has an invalid tree type: \"" << sTp << "\".";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }

      //Validate diameter
      if (fDiam <= 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap";
        std::stringstream s;
        s << "The file \"" << sFileName << "\" has an invalid tree diameter: \"" <<  fDiam << "\".";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }

      //Validate height
      if (fHeight < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap";
        std::stringstream s;
        s << "The file \"" << sFileName << "\" has an invalid tree height: \"" << fHeight << "\".";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }

      p_oTree = CreateTree(fX, fY, iSpecies, iType, fDiam);
      if (fHeight > 0.01) {
        p_oTree->SetValue(mp_iHeightCode[iSpecies][iType], fHeight, true, false);
      }

      //Extra columns as appropriate
      i = 0;
      while (i < iNumColumns) {
        if (isfloat == p_iMemberType[i]) {
          in >> fTemp;
          iCode = GetFloatDataCode(p_sColHeader[i], p_oTree->GetSpecies(), p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->SetValue(iCode, fTemp);
          }
        } else if (isint == p_iMemberType[i]) {
          in >> iTemp;
          iCode = GetIntDataCode(p_sColHeader[i], p_oTree->GetSpecies(), p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->SetValue(iCode, iTemp);
          }
        } else if (isbool == p_iMemberType[i]) {
          in >> iTemp;
          iCode = GetBoolDataCode(p_sColHeader[i], p_oTree->GetSpecies(), p_oTree->GetType());
          if (iCode > -1) {
            if (iTemp == 1) p_oTree->SetValue(iCode, true);
            else p_oTree->SetValue(iCode, false);
          }
        } else if (ischar == p_iMemberType[i]) {
          in >> sTemp;
          iCode = GetStringDataCode(p_sColHeader[i], p_oTree->GetSpecies(), p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->SetValue(iCode, sTemp);
          }
        }
        i++;
      }

      in >> fX >> fY >> sSp >> sTp >> fDiam >> fHeight;
    }
    in.close();
    if (iNumColumns > 0) {
      delete[] p_iMemberType;
      delete[] p_sColHeader;
    }

  } //end of try block
  catch (modelErr & err)
  {
    in.close();
    if (iNumColumns > 0) {
      delete[] p_iMemberType;
      delete[] p_sColHeader;
    }
    throw(err);
  }
  catch (modelMsg & msg)
  {
    in.close();
    if (iNumColumns > 0) {
      delete[] p_iMemberType;
      delete[] p_sColHeader;
    }
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    in.close();
    if (iNumColumns > 0) {
      delete[] p_iMemberType;
      delete[] p_sColHeader;
    }
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::CreateTreesFromTextTreeMap" ;
    throw(stcErr);
  }
}
