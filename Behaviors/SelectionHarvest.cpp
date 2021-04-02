//---------------------------------------------------------------------------
#include "SelectionHarvest.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "Disturbance.h"
#include "Plot.h"
#include <stdio.h>
#include <fstream>
#define MAX_GRID_LABEL_SIZE 25


/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clSelectionHarvest::clSelectionHarvest(clSimManager *p_oSimManager)
: clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ) {
  try {
    m_sNameString = "SelectionHarvest";
    m_sXMLRoot = "SelectionHarvest";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Null pointers, initialize variables to empty
    mp_oMasterCutsGrid = NULL;
    mp_oCutEventsGrid = NULL;
    mp_oNewPackage = NULL;
    mp_oOldPackage = NULL;
    mp_oTimeSinceHarvestGrid = NULL;
    mp_oPop = NULL;
    mp_oResultsGrid = NULL;
    mp_iSpeciesCodes = NULL;
    mp_iRangeMinCodes = NULL;
    mp_iRangeMaxCodes = NULL;
    mp_iRangeAmountCodes = NULL;
    mp_iBaCutCodes = NULL;
    mp_iDenCutCodes = NULL;

    mp_fLowDBH = NULL;
    mp_fHighDBH = NULL;
    mp_fTargetBA = NULL;
    mp_fTempTargetBA = NULL;
    mp_fBasalArea = NULL;
    mp_fLandscapeBasalArea = NULL;

    m_iNumSpecies = 0;
    m_iCutIDCode = -1;
    m_iHarvestTypeCode = -1;
    m_iInitialAge = 0;
    m_iMasterTimestepCode = -1;
    m_iTime = 0;
    m_iMasterIDCode = -1;
    m_iCutTypeCode = -1;
    m_iAmountTypeCode = -1;
    m_iCutTimestepCode = -1;
    m_iTallestFirstCode = -1;
    m_fPlotArea = 0;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;


    //Number of allowed cut ranges
    m_iNumAllowedCutRanges = 4;
  }
  catch(modelErr &err) {throw(err);} //non-fatal error
  catch(...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelectionHarvest::clSelectionHarvest";//(JM)
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clSelectionHarvest::~clSelectionHarvest() {
  delete[] mp_iSpeciesCodes;
  delete[] mp_iRangeMinCodes;
  delete[] mp_iRangeMaxCodes;
  delete[] mp_iRangeAmountCodes;
  delete[] mp_iBaCutCodes;
  delete[] mp_fLowDBH;
  delete[] mp_fHighDBH;
  delete[] mp_fTargetBA;
  delete[] mp_fTempTargetBA;
  delete[] mp_fBasalArea;
  delete[] mp_fLandscapeBasalArea;
}


//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clSelectionHarvest::GetData(DOMDocument *p_oDoc) {
  try {

    DOMNodeList *p_oSelectionHarvest;  //Declare a node list that will store all the Selection Harvest elements
    DOMNode *p_oNode;    //Declare a node within the node list
    DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
               //*p_oInitialAgeElement;    //Declare an element within the node list
    XMLCh *sVal;

    short int i;      //loop counters

    //initialize array
    mp_fBasalArea = new double[m_iNumAllowedCutRanges];
    mp_fLandscapeBasalArea = new double[m_iNumAllowedCutRanges];
    mp_fTempTargetBA = new double[m_iNumAllowedCutRanges];

    //p_oInitialAgeElement = p_oDoc->getDocumentElement();
    FillSingleValue(p_oElement, "sha_InitialAge", &m_iInitialAge, false);

    //Get the cut ranges from the SelectionHarvest node list
    sVal = XMLString::transcode("sha_CutRange");
    p_oSelectionHarvest = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);

    m_iNumAllowedCutRanges = p_oSelectionHarvest->getLength();

    //initialize the arrays
    mp_fLowDBH = new double[m_iNumAllowedCutRanges];
    mp_fHighDBH = new double[m_iNumAllowedCutRanges];
    mp_fTargetBA = new double[m_iNumAllowedCutRanges];

    //if the user specified zero size classes, i'd better throw an error
    if(0 == m_iNumAllowedCutRanges) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSelectionHarvest::GetData";
      stcErr.sMoreInfo = "Invalid number of cut ranges. At least 1 cut range must be specified.";
      throw(stcErr);
    }

    mp_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");

    //*******************************************
    //Go through each size class and get the range and target basal area
    //*******************************************
    for(i=0; i < m_iNumAllowedCutRanges; i++)
    {
      p_oNode = p_oSelectionHarvest->item(i);
      p_oElement = (DOMElement *) p_oNode;

      //Get the target basal area value for the size class
      FillSingleValue(p_oElement, "sha_target_BA", &mp_fTargetBA[i], true);
      //if the target basal area value is invalid throw an error
      if(mp_fTargetBA[i] < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSelectionHarvest::GetData";
        stcErr.sMoreInfo = "Invalid Target Basal Area value. Value must be greater than or equal to zero.";
        throw(stcErr);
      }
      //Get the low value for the size class
      FillSingleValue(p_oElement, "sha_loDBH", &mp_fLowDBH[i], true);
      //if the target basal area value is invalid throw an error

      //Get the high value for the size class
      FillSingleValue(p_oElement, "sha_hiDBH", &mp_fHighDBH[i], true);
      //if the target basal area value is invalid throw an error
      if(mp_fHighDBH[i] < 0 || mp_fLowDBH[i] < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSelectionHarvest::GetData";
        stcErr.sMoreInfo = "Invalid bound on size class. Values must be greater than or equal to zero.";
        throw(stcErr);
      }
      //ensure that the lower bound is always smaller than the upper bound
      if(mp_fLowDBH[i] >= mp_fHighDBH[i]) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSelectionHarvest::GetData";
        stcErr.sMoreInfo = "Invalid bound on size class. The lower bound must be smaller than the upper bound.";
        throw(stcErr);
      }
      //ensure that each size class does not overlap with others
      if (i != 0) {
        if(mp_fLowDBH[i] <= mp_fHighDBH[i-1]) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clSelectionHarvest::GetData";
          stcErr.sMoreInfo = "Invalid bound on size class. The values must not overlap.";
          throw(stcErr);
        }
      }
    }
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelectionHarvest::GetData";
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// CalculateBasalAreaDifference()
/////////////////////////////////////////////////////////////////////////////
int clSelectionHarvest::CalculateBasalAreaDifference(double *mp_fLandscapeBasalArea)
{
  int i;    //counter
  short int iTemp = 0;

  //Compute the total plot area in hectares
  m_fPlotArea = mp_oSimManager->GetPlotObject()->GetPlotArea();

  //Now p_fCurrentBA contains a total basal area for each size class
  //Compare these values to the target basal area for each size class
  //and compute the delta
  for (i=0; i<m_iNumAllowedCutRanges; i++)
  {
    //Determine the target BA for the whole plot by size class, rather than by
    //hectare. Need to use a temp place holder so that I don't modify the
    //actualy Target Basal Areas.
    mp_fTempTargetBA[i] = mp_fTargetBA[i] * m_fPlotArea;

    //Calculate the difference between the current basal area per hectare and
    //the target basal area per hectare
    mp_fLandscapeBasalArea[i] = mp_fLandscapeBasalArea[i] - mp_fTempTargetBA[i];

    //if the value is negative, set it to zero. this simply means that harvest
    //will not remove any units of basal area as the size class' current basal
    //area less than the target basal area
    if(mp_fLandscapeBasalArea[i] < 0)
    {
      mp_fLandscapeBasalArea[i] = 0;
      iTemp += 1;
    }
  }
  //if each size class is to have zero units of basal area removed, we
  //exit the behaviour without creating the package
  if(iTemp == m_iNumAllowedCutRanges){
    return 1;
  }

  return 0;
}


//////////////////////////////////////////////////////////////////////////////
// Action()
//////////////////////////////////////////////////////////////////////////////

void clSelectionHarvest::Action()
{
  int iSpecies;
  m_iNumSpecies = mp_oPop->GetNumberOfSpecies();
  //this function will ensure that the master cuts grid and the cut events grid
  //were created by clHarvest::SetupHarvestGrids()
  if (1 == mp_oSimManager->GetCurrentTimestep()) {
    GetHarvestGrids();
  }
  //This function create a package for the master cuts grid, based on a
  //selection harvest scheme. A package is created for each species, thus call
  //the function once for each species
  for(iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++)
  {
    CreateMasterCutPackage(iSpecies);
  }
  //this function will calculate the time since last harvest for each cell in
  //the harvest results grid
  GetTimeSinceHarvest();
  //This function will ensure that the master cuts package and the cut events
  //packages have valid values
  ValidatePackages();
}



//////////////////////////////////////////////////////////////////////////////
// createMasterCutPackage()
//////////////////////////////////////////////////////////////////////////////

void clSelectionHarvest::CreateMasterCutPackage(int iSpecies)
{
  int iCutType,		//the cut type
  iTemp,   //temporary integer storage
  iTimestep = mp_oSimManager->GetCurrentTimestep(), //the current timestep
  iNumSpecies = mp_oPop->GetNumberOfSpecies(),      //number of species in the population
  iNumRows = 0, //number of rows in the cutEventsGrid
  iNumCols = 0,//number of columns in the cutEventsGrid
  iReturnCode, //the return code for the call to CalculateBasalAreaDifference
  j, k;

  for(int z=0; z<m_iNumAllowedCutRanges; z++)
  {
    mp_fTempTargetBA[z] = 0;
    mp_fBasalArea[z] = 0;
    mp_fLandscapeBasalArea[z] = 0;
  }

  //Add a package to the master cut events grid - they're in timestep order,
  //so keep it that way

  mp_oOldPackage = NULL;
  mp_oNewPackage = NULL;
  mp_oOldPackage = mp_oMasterCutsGrid->GetFirstPackageOfCell(0, 0);

  //Call getBasalArea with a species argument to determine the basal area
  //of a particular species in the whole stand
  GetBasalArea(mp_oPop, iSpecies, mp_fBasalArea, mp_fLowDBH, mp_fHighDBH);

  //Call getBasalArea to determine the basal area of all species in the whole
  //stand
  GetBasalArea(mp_oPop, mp_fLandscapeBasalArea, mp_fLowDBH, mp_fHighDBH);

  //Create a ratio of species basal area to total basal area. This ratio will
  //be used to determine how much basal area in each class to remove for each
  //species
  for(int i = 0; i < m_iNumAllowedCutRanges; i++)
  {
    if((int)mp_fLandscapeBasalArea[i] == 0 || (int)mp_fBasalArea[i] == 0)
    {
      mp_fBasalArea[i] = 0;
    }
    else
    {
      mp_fBasalArea[i] = mp_fBasalArea[i] / mp_fLandscapeBasalArea[i];
    }
  }

  //What I'm doing here is calling CalculateBasalAreaDifference to determine
  //the basal area to remove from each size class.  If there are zero units of
  //basal area to remove from each size class, then we exit the behaviour.
  iReturnCode = CalculateBasalAreaDifference(mp_fLandscapeBasalArea);

  //if iReturnCode is a non-zero value, it means that the amount to cut in every
  //size class is zero.  In this case, we do not want to continue the behaviour
  //any longer.
  if(iReturnCode != 0)
  {
    return;
  }

  //This array will be how much BA to remove for the current species. This value
  //will represent the total amount of basal area to remove from the landscape
  //rather than per hectare.  So, i then divide the amount to cut by the plot
  //area in hectares to determine the amount to cut per hectare.

  //Compute the total plot area in hectares
  m_fPlotArea = mp_oSimManager->GetPlotObject()->GetPlotArea();

  for(int i = 0; i < m_iNumAllowedCutRanges; i++)
  {
    mp_fBasalArea[i] = mp_fBasalArea[i] * mp_fLandscapeBasalArea[i];
    mp_fBasalArea[i] = mp_fBasalArea[i] / m_fPlotArea;
  }

  //From what I can understand, this while loop will go through all the harvest
  //master cut packages from the first timestep to the last, if it needs to. It
  //is looking for where to place the new package that we are creating here.
  //Once the timestep of the current harvest master cut package is equal to the
  //the timestep of the package we are creating we break out of the loop as
  //we've found the proper place for our new package
  while (mp_oOldPackage) {
    //Get this package's timestep
    mp_oOldPackage->GetValue(m_iMasterTimestepCode, &iTemp);

    if (iTemp >= iTimestep) //here's where we'll drop it
      break;

    mp_oNewPackage = mp_oOldPackage; //use temporarily as a holder
    mp_oOldPackage = mp_oOldPackage->GetNextPackage();
  }

  //Create the new package
  if (!mp_oNewPackage) //either there's no packages in grid, or new is first
    mp_oNewPackage = mp_oMasterCutsGrid->CreatePackageOfCell(0, 0);
  else   //make the new one after the one in mp_oNewPackage
    mp_oNewPackage = mp_oMasterCutsGrid->CreatePackage(mp_oNewPackage);


  //Okay, so in my creation of a new package, I'm going to have to add the
  //following variables: id, timestep, cuttype, amttype, species, rangeamt,
  //rangemin and rangemax. I will need to create a new package for each
  //timestep. cuttype will just be 'partial', which is a current cuttype.
  //amttype will always be absBA, species will be true for all species, rangeamt
  //will be determined by CalculateBasalAreaDifference(). rangemin and rangemax
  //are specified in the parameter file.

  //Load up the new package with the timestep value
  mp_oNewPackage->SetValue(m_iMasterTimestepCode, iTimestep);

  //Set the tallest first flag
  mp_oNewPackage->SetValue(m_iTallestFirstCode, true);

  //Set the species value to true for the species we are currently dealing with.

  for (j = 0; j < iNumSpecies; j++)
  {
    if(j == iSpecies)
    {
      mp_oNewPackage->SetValue(mp_iSpeciesCodes[j], true);
    }
    else
    {
      mp_oNewPackage->SetValue(mp_iSpeciesCodes[j], false);
    }
  }

  //Just set iCutType to partial explicitly
  iCutType = partial;
  //add it to the package
  mp_oNewPackage->SetValue(m_iCutTypeCode, iCutType);

  //Get the amount type if this is a partial cut
  //For now, I'm going to assume that amt type will always be absolute
  //basal area.
  mp_oNewPackage->SetValue(m_iAmountTypeCode, absBA);

  //Set the cut ranges.
  for (j = 0; j < m_iNumAllowedCutRanges; j++)
  {
    mp_oNewPackage->SetValue(mp_iRangeMinCodes[j], (float)mp_fLowDBH[j]);
    mp_oNewPackage->SetValue(mp_iRangeMaxCodes[j], (float)mp_fHighDBH[j]);
  }

  //I'm going to have to set an absolute basal area to cut for each size
  //class, so loop once for each size class
  for (k = 0; k < m_iNumAllowedCutRanges; k++)
  {
    mp_oNewPackage->SetValue(mp_iRangeAmountCodes[k], (float)mp_fBasalArea[k]);
  }

  //I use the species # for the selection harvest ID, so that each species
  //will be represented by a cut package.
  mp_oNewPackage->SetValue(m_iMasterIDCode, iSpecies);

  //Come up with a way to assign this cut to all cells in the grid. There must
  //be a way to edit the cut events grid such that each cell has an id and
  //timestep that corresponds to the ID and timestep of the master cut package
  //that I have just created.

  //get the number of rows and columns in the grid
  iNumRows = mp_oCutEventsGrid->GetNumberYCells();
  iNumCols = mp_oCutEventsGrid->GetNumberXCells();

  //for each row in the grid
  for(int i=0; i<iNumRows; i++)
  {
    //for each column in the grid
    for(int j=0; j<iNumCols; j++)
    {
      mp_oOldPackage = NULL;
      mp_oNewPackage = NULL;
      mp_oOldPackage = mp_oCutEventsGrid->GetFirstPackageOfCell(i, j);
      while ( mp_oOldPackage )
      {
        //Get this package's timestep
        mp_oOldPackage->GetValue( m_iCutTimestepCode, & iTemp );

        if ( iTemp >= iTimestep ) //here's where we'll drop it
          break;

        mp_oNewPackage = mp_oOldPackage; //use temporarily as a holder
        mp_oOldPackage = mp_oOldPackage->GetNextPackage();
      }

      //Create the new package
      if ( !mp_oNewPackage ) //either there's no packages in grid, or new is first
        mp_oNewPackage = mp_oCutEventsGrid->CreatePackageOfCell(i, j);
      else //make the new one after the one in p_oNewPackage
        mp_oNewPackage = mp_oCutEventsGrid->CreatePackage( mp_oNewPackage );

      //Load up the new package with the timestep and id values
      mp_oNewPackage->SetValue( m_iCutTimestepCode, iTimestep );
      mp_oNewPackage->SetValue( m_iCutIDCode, iSpecies );

    }
  }
  return;

}


//////////////////////////////////////////////////////////////////////////////
// GetBasalArea()
//////////////////////////////////////////////////////////////////////////////
void clSelectionHarvest::GetBasalArea(clTreePopulation *p_oTreePop, double *p_fTotalBasalArea,
    double *p_fLoDbh, double *p_fHiDbh)
{
  try {
    clTree *p_oTree;                   //tree whose basal area we're getting
    clTreeSearch *p_oTreeList;         //list of all trees in the population
    float fDbh;                        //tree's dbh
    short int iTreeType, iTreeSpecies, //tree's type and species
    i;                       //loop counter
    char cQuery[10];                   //format search strings into this

    sprintf(cQuery, "%s", "all");
    p_oTreeList = p_oTreePop->Find(cQuery);
    p_oTree = p_oTreeList->NextTree();

    while (p_oTree != NULL)
    {
      iTreeSpecies = p_oTree->GetSpecies();
      iTreeType = p_oTree->GetType();
      if (clTreePopulation::seedling != iTreeType)
      {
        p_oTree->GetValue(mp_oPop->GetDbhCode(iTreeSpecies,iTreeType),&fDbh);
        for (i = 0; i < m_iNumAllowedCutRanges; i++)
        {
          if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
          {
            p_fTotalBasalArea[i] += clModelMath::CalculateBasalArea(fDbh);
          }
        }
      } //end of if (clTreePopulation::seedling != iTreeType)
        p_oTree = p_oTreeList->NextTree();
    } //end of while (p_oTree != NULL)
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelectionHarvest::GetBasalArea";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetBasalArea()
//////////////////////////////////////////////////////////////////////////////
void clSelectionHarvest::GetBasalArea(clTreePopulation *p_oTreePop,  int iSpecies, double * p_fTotalBasalArea,
    double * p_fLoDbh, double * p_fHiDbh )
{
  try {
    clTree *p_oTree;                   //tree whose basal area we're getting
    clTreeSearch *p_oTreeList;         //list of all trees in the population
    float fDbh;                        //tree's dbh
    short int iTreeType, iTreeSpecies, //tree's type and species
    i;                       //loop counter
    char cQuery[10];                   //format search strings into this

    sprintf(cQuery, "%s", "all");
    p_oTreeList = p_oTreePop->Find(cQuery);
    p_oTree = p_oTreeList->NextTree();

    while (p_oTree != NULL)
    {
      //the function GetSpecies returns an integer representation of the species
      iTreeSpecies = p_oTree->GetSpecies();
      iTreeType = p_oTree->GetType();
      if (clTreePopulation::seedling != iTreeType && iTreeSpecies == iSpecies)
      {
        p_oTree->GetValue(mp_oPop->GetDbhCode(iTreeSpecies,iTreeType),&fDbh);
        for (i = 0; i < m_iNumAllowedCutRanges; i++)
        {
          if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
          {
            mp_fBasalArea[i] += clModelMath::CalculateBasalArea(fDbh);
          }
        }
      } //end of if (clTreePopulation::seedling != iTreeType)
        p_oTree = p_oTreeList->NextTree();
    } //end of while (p_oTree != NULL)

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelectionHarvest::GetBasalArea";
    throw(stcErr);
  }

}


//////////////////////////////////////////////////////////////////////////////
// SetupHarvestGrids()
//////////////////////////////////////////////////////////////////////////////
void clSelectionHarvest::GetHarvestGrids() {
  try {
    int i, iNumSpecies;
    char cFloatLabel[MAX_GRID_LABEL_SIZE];

    iNumSpecies = mp_oPop->GetNumberOfSpecies();
    mp_iSpeciesCodes = new short int[iNumSpecies];
    mp_iRangeMinCodes = new short int[m_iNumAllowedCutRanges];
    mp_iRangeMaxCodes = new short int[m_iNumAllowedCutRanges];
    mp_iRangeAmountCodes = new short int[m_iNumAllowedCutRanges];


    //*************************************
    //harvestmastercuts grid
    //*************************************

    //Get a pointer to the harvest master cuts grid
    mp_oMasterCutsGrid = mp_oSimManager->GetGridObject("harvestmastercuts");

    //If the harvest master cuts grid pointer is NULL, throw an error
    if (!mp_oMasterCutsGrid)
    {
      modelErr stcErr;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      stcErr.sFunction = "clSelectionHarvest::GetHarvestGrids";
      stcErr.sMoreInfo = "Master cuts grid required.";
      throw(stcErr);
    }

    //Get the data member codes for the harvest master cuts grid
    m_iMasterTimestepCode = mp_oMasterCutsGrid->GetPackageIntDataCode("timestep");
    m_iMasterIDCode = mp_oMasterCutsGrid->GetPackageIntDataCode("id");
    m_iCutTypeCode = mp_oMasterCutsGrid->GetPackageIntDataCode("cuttype");
    m_iAmountTypeCode = mp_oMasterCutsGrid->GetPackageIntDataCode("amttype");
    m_iTallestFirstCode = mp_oMasterCutsGrid->GetPackageBoolDataCode("tallestfirst");

    for (i = 0; i < iNumSpecies; i++)
    {
      sprintf(cFloatLabel, "%s%d", "species", i);
      mp_iSpeciesCodes[i] = mp_oMasterCutsGrid->GetPackageBoolDataCode(cFloatLabel);
    }

    for (i = 0; i < m_iNumAllowedCutRanges; i++)
    {
      sprintf(cFloatLabel, "%s%d", "rangemin", i);
      mp_iRangeMinCodes[i] =
        mp_oMasterCutsGrid->GetPackageFloatDataCode(cFloatLabel);
      sprintf(cFloatLabel, "%s%d", "rangemax", i);
      mp_iRangeMaxCodes[i] =
        mp_oMasterCutsGrid->GetPackageFloatDataCode(cFloatLabel);
      sprintf(cFloatLabel, "%s%d", "rangeamt", i);
      mp_iRangeAmountCodes[i] =
        mp_oMasterCutsGrid->GetPackageFloatDataCode(cFloatLabel);
    }

    //If any of the codes are -1, indicating that the grid is not set
    //up in the way we expect, throw an error
    if (-1 == m_iAmountTypeCode || -1 == m_iMasterIDCode || -1 == m_iMasterTimestepCode || -1 == m_iCutTypeCode)
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSelectionHarvest::GetHarvestGrids";
      stcErr.sMoreInfo = "Unexpected master grid data return code.";
      throw(stcErr);
    }
    for (i = 0; i < iNumSpecies; i++) {
      if (-1 == mp_iSpeciesCodes[i]) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSelectionHarvest::GetHarvestGrids";
        stcErr.sMoreInfo = "Unexpected master grid data return code for species.";
        throw(stcErr);
      }
    }
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      if (-1 == mp_iRangeMinCodes[i] || -1 == mp_iRangeMaxCodes[i] || -1 == mp_iRangeAmountCodes[i]) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSelectionHarvest::GetHarvestGrids";
        stcErr.sMoreInfo = "Unexpected master grid data return code for range.";
        throw(stcErr);
      }
    }

    //*************************************
    //harvesteventcuts grid
    //*************************************

    //Get a pointer to the harvest cut events grid
    mp_oCutEventsGrid = mp_oSimManager->GetGridObject("harvestcutevents");

    //If the harvest cut events grid pointer is NULL, throw an error
    if (!mp_oMasterCutsGrid) {
      modelErr stcErr;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      stcErr.sFunction = "clSelectionHarvest::GetHarvestGrids";
      stcErr.sMoreInfo = "Cut events grid required.";
      throw(stcErr);
    }

    //Get the data member codes for the harvest cut events grid
    m_iCutTimestepCode = mp_oCutEventsGrid->GetPackageIntDataCode("timestep");
    m_iCutIDCode = mp_oCutEventsGrid->GetPackageIntDataCode("id");


    //If any of the codes are -1, indicating that the grid is not set
    //up in the way we expect, throw an error
    if (-1 == m_iCutTimestepCode || -1 == m_iCutIDCode) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSelectionHarvest::GetHarvestGrids";
      stcErr.sMoreInfo = "Unexpected grid data return code.";
      throw(stcErr);
    }
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelectionHarvest::GetHarvestGrids";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// ValidatePackages()
//////////////////////////////////////////////////////////////////////////////
void clSelectionHarvest::ValidatePackages() {
  try {
    clPackage *p_oPackage,  //a package to validate
    *p_oNextPackage; //for comparing other packages to this one
    int iID,              //package ID value
    iNextID;          //for comparing other packages' ID values
    short int iNumXCells = mp_oCutEventsGrid->GetNumberXCells(),
    iNumYCells = mp_oCutEventsGrid->GetNumberYCells(),
    i, j;

    //****************************************************
    // Validate packages in cut events grid
    //****************************************************
    for (i = 0; i < iNumXCells; i++)
      for (j = 0; j < iNumYCells; j++) {
        p_oPackage = mp_oCutEventsGrid->GetFirstPackageOfCell(i, j);
        while (p_oPackage) {

          //Go through all the other packages in this grid cell and make sure that
          //there is no other with the same ID number
          p_oPackage->GetValue(m_iCutIDCode, &iID);

          p_oNextPackage = p_oPackage->GetNextPackage();
          while (p_oNextPackage) {

            p_oNextPackage->GetValue(m_iCutIDCode, &iNextID);
            if (iID == iNextID) {
              //throw an error
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clSelectionHarvest::ValidatePackages";
              stcErr.sMoreInfo = "Harvest cut may not be applied twice to the same grid cell.";
              throw(stcErr);
            }
            p_oNextPackage = p_oNextPackage->GetNextPackage();
          } //end of while (p_oNextPackage)
          p_oPackage = p_oPackage->GetNextPackage();
        } //end of while (p_oPackage)
      } //end of for (j = 0; j < iNumYCells; j++)
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelectionHarvest::ValidatePackages";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetTimeSinceLastHarvest()
//////////////////////////////////////////////////////////////////////////////
void clSelectionHarvest::GetTimeSinceHarvest()
{
  try
  {
    short int iNumXCells = mp_oCutEventsGrid->GetNumberXCells(),
    iNumYCells = mp_oCutEventsGrid->GetNumberYCells(),
    i, j;
    int iHarvestType, iTime, iInitialResult = -1;
    int iTimestep = mp_oSimManager->GetCurrentTimestep();
    int iTimestepLength = mp_oSimManager->GetNumberOfYearsPerTimestep();

    float fCellLength = mp_oPop->GetGridCellSize();

    //Get a pointer to the harvest results grid
    mp_oResultsGrid = mp_oSimManager->GetGridObject("Harvest Results");

    //If the Results grid pointer is NULL, throw an error
    if (!mp_oResultsGrid)
    {
      modelErr stcErr;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      stcErr.sFunction = "clSelectionHarvest::GetTimeSinceHarvest";
      stcErr.sMoreInfo = "Results cuts grid required.";
      throw(stcErr);
    }

    m_iHarvestTypeCode = mp_oResultsGrid->GetPackageIntDataCode("Harvest Type");

    //If this is the first timestep, then I need to create the grid and populate
    //it with inital values. Initially, I will set an inital time since harvest
    //of 5000 however, once I get it working properly, I will work on allowing
    //it to be user defined
    //create a grid
    if(iTimestep == 1)
    {
      mp_oTimeSinceHarvestGrid = mp_oSimManager->CreateGrid("Time Since Harvest", //grid name
          1, //number of int data members
          0, //number of float data members
          0, //number of char data members
          0, //number of bool data members
          fCellLength, //Number of X cells
          fCellLength); //Number of Y cells

      //register the data members
      m_iTime = mp_oTimeSinceHarvestGrid->RegisterInt( "Time" );

      //traverse through cells of TimeSinceHarvestGrid
      for(i=0; i<iNumXCells; i++)
      {
        for(j=0; j<iNumYCells; j++)
        {
          //populate each cell in the TimeSinceHarvestGrid with a starting value
          mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTime, m_iInitialAge);
          //populate each cell in the ResultsGrid with a starting value
          mp_oResultsGrid->SetValueOfCell( i, j, m_iHarvestTypeCode, iInitialResult);
        }
      }

    }
    //End of First timestep administrative stuff
    else
    {
      //traverse through cells of harvestResultsGrid and check harvestType value
      for(i=0; i<iNumXCells; i++)
      {
        for(j=0; j<iNumYCells; j++)
        {
          //Get the time since harvest from the TimeSinceHarvest grid
          mp_oTimeSinceHarvestGrid->GetValueOfCell( i, j, m_iTime, &iTime );
          //Get the harvest type value from the grid cell
          mp_oResultsGrid->GetValueOfCell(i, j, m_iHarvestTypeCode, &iHarvestType);
          //if the value is -1, it means that no harvest occured

          if(iHarvestType == -1)
          {
            //if no harvest occurred, then increment the value by the timestep length
            iTime = iTime + iTimestepLength;
            mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTime, iTime );
          }
          else
          {
            iTime = iTimestepLength;
            mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTime, iTime );
          }
        }
      }
    }
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelectionHarvest::GetTimeSinceHarvest";
    throw(stcErr);
  }

}
