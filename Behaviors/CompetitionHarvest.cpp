//---------------------------------------------------------------------------
#include "CompetitionHarvest.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "ModelMath.h"
#include <stdio.h>
#include <fstream>
#include <sstream>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clCompetitionHarvest::clCompetitionHarvest( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "CompetitionHarvest";
    m_sXMLRoot = "CompetitionHarvest";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Indicate that this behavior intends to add one tree float data
    //member
    m_iNewTreeFloats = 1;

    //Null pointers, initialize variables to empty
    mp_oHighestCOE = NULL;
    mp_fCOE = NULL;
    mp_fLambda = NULL;
    mp_iCOECodes = NULL;
    mp_fAlpha = NULL;
    mp_fBeta = NULL;
    mp_fC = NULL;
    mp_fD = NULL;
    mp_fGamma = NULL;
    mp_fMaxCrowdingRadius = NULL;
    mp_fPropToCut = NULL;
    mp_fTargetToCut = NULL;
    mp_fAlreadyCut = NULL;
    mp_iDenCutCodes = NULL;
    mp_iBaCutCodes = NULL;
    mp_oResultsGrid = NULL;

    m_fMaxMaxCrowdingRadius = 0;
    m_fQ = 0;
    m_fMinHarvestDBH = 0;
    m_fMaxHarvestDBH = 0;
    m_fAmtToCut = 0;
    m_fBAThreshold = 0;
    m_fMinSaplingHeight = 0;
    m_iInterval = 0;
    m_iReasonCode = notdead;
    m_iTimeSinceLastHarvest = 0;
    m_iTimestepToStartHarvests = 0;
    m_iNumX = 0;
    m_iNumY = 0;
    m_iNumSpecies = 0;
    m_bIsSpeciesSpecific = false;
    harvest = interval_rem;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;
  }
  catch ( modelErr & err )
  {
    throw( err );
  } //non-fatal error
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clCompetitionHarvest::clCompetitionHarvest" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clCompetitionHarvest::~clCompetitionHarvest()
{
  short int i;
  if (mp_oHighestCOE) {
    for (i = 0; i < m_iNumX; i++)
      delete[] mp_oHighestCOE[i];
    delete[] mp_oHighestCOE;
  }

  if (mp_fCOE) {
    for (i = 0; i < m_iNumX; i++)
      delete[] mp_fCOE[i];
    delete[] mp_fCOE;
  }

  if (mp_fLambda) {
    for (i = 0; i < m_iNumSpecies; i++)
      delete[] mp_fLambda[i];
    delete[] mp_fLambda;
  }

  if (mp_iCOECodes) {
    for (i = 0; i < m_iNumSpecies; i++)
      delete[] mp_iCOECodes[i];
    delete[] mp_iCOECodes;
  }

  delete[] mp_fAlpha;
  delete[] mp_fBeta;
  delete[] mp_fC;
  delete[] mp_fD;
  delete[] mp_fGamma;
  delete[] mp_fMaxCrowdingRadius;
  delete[] mp_fPropToCut;
  delete[] mp_fTargetToCut;
  delete[] mp_fAlreadyCut;
  delete[] mp_iDenCutCodes;
  delete[] mp_iBaCutCodes;
}


////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::RegisterTreeDataMembers()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int i, j;

  m_iNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data member
  mp_iCOECodes = new short int * [m_iNumSpecies];
  for ( i = 0; i < m_iNumSpecies; i++ )
  {
    mp_iCOECodes[i] = new short int[2];
    for (j = 0; j < 2; j++)
      mp_iCOECodes[i][j] = -1;
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Make sure that only allowed types have been applied
    if ( clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType
        && clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType )
    {
      modelErr stcErr;
      stcErr.sFunction = "clCompetitionHarvest::RegisterTreeDataMembers" ;
      stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Register the code and capture it
    mp_iCOECodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                 [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
                     p_oPop->RegisterFloat( "COE", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}

//////////////////////////////////////////////////////////////////////////////
// ReadParameterFileData()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::ReadParameterFileData( xercesc::DOMDocument * p_oDoc,
    clTreePopulation *p_oPop )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  float *p_fTempAll = new float[m_iNumSpecies];
  try
  {
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    stringstream sTag;
    float fTemp, fPlotSize = p_oPlot->GetPlotArea();
    int iTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(),
        iTemp;
    short int i; //loop counters

    //Set up our doubleVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare arrays
    mp_fAlpha = new double[m_iNumSpecies];
    mp_fBeta = new double[m_iNumSpecies];
    mp_fC = new double[m_iNumSpecies];
    mp_fD = new double[m_iNumSpecies];
    mp_fGamma = new double[m_iNumSpecies];
    mp_fMaxCrowdingRadius = new double[m_iNumSpecies];
    mp_fPropToCut = new double[m_iNumSpecies];
    mp_fLambda = new double*[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++)
      mp_fLambda[i] = new double[m_iNumSpecies];

    //Maximum crowding radius
    FillSpeciesSpecificValue( p_oElement, "di_compHarvMaxCrowdingRadius",
        "di_chmcrVal", p_fTempValues, m_iNumBehaviorSpecies,
        p_oPop, true );
    //Make sure that the max radius of neighbor effects is > 0
    for (i = 0; i < m_iNumSpecies; i++) mp_fMaxCrowdingRadius[i] = 0;
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      if ( p_fTempValues[i].val <= 0 )
      {
        modelErr stcErr;
        stcErr.sFunction = "clCompetitionHarvest::ReadParameterFileData" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "All values for max crowding radius must be greater than 0.";
        throw( stcErr );
      }
      mp_fMaxCrowdingRadius[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Alpha
    FillSpeciesSpecificValue( p_oElement, "di_compHarvAlpha", "di_chaVal",
        mp_fAlpha, p_oPop, true );

    //Beta
    FillSpeciesSpecificValue( p_oElement, "di_compHarvBeta", "di_chbVal",
        mp_fBeta, p_oPop, true );

    //Gamma
    FillSpeciesSpecificValue( p_oElement, "di_compHarvGamma", "di_chgVal",
        mp_fGamma, p_oPop, true );

    //C
    FillSpeciesSpecificValue( p_oElement, "di_compHarvCrowdingSlope",
        "di_chcsVal", mp_fC, p_oPop, true);

    //D
    FillSpeciesSpecificValue( p_oElement, "di_compHarvCrowdingSteepness",
        "di_chcstVal", mp_fD, p_oPop, true);

    //Proportion of each species to cut
    for (i = 0; i < m_iNumSpecies; i++) mp_fPropToCut[i] = 0;
    FillSpeciesSpecificValue( p_oElement, "di_compHarvProportion", "di_chpVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPropToCut[p_fTempValues[i].code] = p_fTempValues[i].val;
    //Make sure that if the values are not all 1, they add up to 1
    m_bIsSpeciesSpecific = false;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (mp_fPropToCut[mp_iWhatSpecies[i]] > 0 &&
          mp_fPropToCut[mp_iWhatSpecies[i]] < 1) {
        m_bIsSpeciesSpecific = true;
        break;
      }
    }
    if (m_bIsSpeciesSpecific) {
      fTemp = 0;
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        fTemp += mp_fPropToCut[mp_iWhatSpecies[i]];
      }
      if (fabs(fTemp - 1.0) > 0.0001) {
        modelErr stcErr;
        stcErr.sFunction = "clCompetitionHarvest::ReadParameterFileData" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Proportion to cut for different species must add up to 1.";
        throw( stcErr );
      }
    }

    //Lambda
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      sTag << "di_compHarv" << p_oPop->TranslateSpeciesCodeToName( mp_iWhatSpecies[i] ) << "NeighborLambda";
      FillSpeciesSpecificValue( p_oElement, sTag.str(), "di_nlVal",
          mp_fLambda[mp_iWhatSpecies[i]], p_oPop, true );
      sTag.str("");
    }

    //DBH divisor
    FillSingleValue( p_oElement, "di_compHarvDbhDivisor", & m_fQ, true );

    //Minimum DBH for harvesting
    FillSingleValue( p_oElement, "di_compHarvMinHarvDBH", & m_fMinHarvestDBH,
        true );

    //Maximum DBH for harvesting
    FillSingleValue( p_oElement, "di_compHarvMaxHarvDBH", & m_fMaxHarvestDBH,
        true );

    //Type of harvest
    FillSingleValue( p_oElement, "di_compHarvTypeHarvest", & iTemp, true );
    if (interval_rem == iTemp) {
      harvest = interval_rem;
    } else if (ba_amt == iTemp) {
      harvest = ba_amt;
    } else if (ba_prop == iTemp) {
      harvest = ba_prop;
    } else if (interval_prop == iTemp) {
      harvest = interval_prop;
    } else {
      modelErr stcErr;
      stcErr.sFunction = "clCompetitionHarvest::ReadParameterFileData" ;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unknown harvest type \"" << iTemp << "\".";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }

    //Cut amount
    FillSingleValue( p_oElement, "di_compHarvCutAmount", &m_fAmtToCut, true );
    //Validate - if this is a fixed BA threshold harvest with proportion to cut,
    //or a fixed interval cut with proportion to cut, this must be a proportion
    //to cut between 0 and 1; otherwise, it must be greater than 0
    if ((ba_prop == harvest || interval_prop == harvest) && (m_fAmtToCut < 0 || m_fAmtToCut > 1)) {
      modelErr stcErr;
      stcErr.sFunction = "clCompetitionHarvest::ReadParameterFileData" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "The amount to cut must be a proportion between 0 and 1.";
      throw( stcErr );
    } else if (m_fAmtToCut < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clCompetitionHarvest::ReadParameterFileData" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "The amount to cut must be greater than 0.";
      throw( stcErr );
    }
    //If not a proportion, transform from per hectare to total
    if (ba_prop != harvest && interval_prop != harvest) m_fAmtToCut *= fPlotSize;

    //For fixed BA threshold harvests, the minimum interval between harvests and
    //the BA threshold; for fixed interval threshold harvests, the threshold
    if (ba_amt == harvest || ba_prop == harvest) {
      FillSingleValue(p_oElement, "di_compHarvMinInterval", &m_iInterval, true);
      FillSingleValue(p_oElement, "di_compHarvBAThreshold", &m_fBAThreshold,
          true);
    }
    else
      FillSingleValue(p_oElement, "di_compHarvInterval", &m_iInterval, true);

    if (m_iInterval < 1) {
      modelErr stcErr;
      stcErr.sFunction = "clCompetitionHarvest::ReadParameterFileData" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "The harvest interval must be greater than 0.";
      throw( stcErr );
    }

    //Transform the interval from years to timesteps
    m_iInterval /= iTimestep;

    //Read the year to begin harvesting. For backwards compatibility, this is
    //optional
    FillSingleValue(p_oElement, "di_compHarvFirstHarvestYear", &m_iTimestepToStartHarvests, false);
    //Transform from years to timesteps
    m_iTimestepToStartHarvests /= iTimestep;


    //Filename for list of trees harvested - optional
    FillSingleValue( p_oElement, "di_compHarvHarvestedListFile", &m_sHarvestListFilename, false );

    delete[] p_fTempValues;
    delete[] p_fTempAll;
  }
  catch ( modelErr & err )
  {
    delete[] p_fTempValues;
    delete[] p_fTempAll;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTempValues;
    delete[] p_fTempAll;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempValues;
    delete[] p_fTempAll;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clCompetitionHarvest::ReadParameterFileData" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    int i;

    SetupGrids();
    ReadParameterFileData( p_oDoc, p_oPop );
    SetupCOEGrids(p_oPop);

    //Miscellaneous final setup
    m_iReasonCode = whyDead::harvest;
    mp_fTargetToCut = new double[m_iNumSpecies];
    mp_fAlreadyCut = new double[m_iNumSpecies];

    //Set time since last harvest so that harvests can occur the first
    //timestep
    m_iTimeSinceLastHarvest = m_iInterval - 1;

    m_fMinSaplingHeight = 50;
    //Get the minimum sapling height
    for ( i = 0; i < m_iNumSpecies; i++ )
      if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
        m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );


    //Get the maximum competitive radius
    m_fMaxMaxCrowdingRadius = 0;
    for (i = 0; i < m_iNumSpecies; i++)
      if (mp_fMaxCrowdingRadius[i] > m_fMaxMaxCrowdingRadius)
        m_fMaxMaxCrowdingRadius = mp_fMaxCrowdingRadius[i];

    //If a record of all harvested trees is desired, write the header now
    if (m_sHarvestListFilename.length() > 0) {
      fstream out(m_sHarvestListFilename.c_str(), ios::out | ios::trunc);
      out << "X\tY\tSpecies\tDBH\tTimestep\n";
      out.close();
    }


  } //end of try block
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
    stcErr.sFunction = "clCompetitionHarvest::GetData" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetupCOEGrids()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::SetupCOEGrids(clTreePopulation * p_oPop)
{
  try
  {
    int i;

    m_iNumX = p_oPop->GetNumXCells();
    m_iNumY = p_oPop->GetNumYCells();

    mp_oHighestCOE = new clTree**[m_iNumX];
    mp_fCOE = new float*[m_iNumX];

    for (i = 0; i < m_iNumX; i++) {
      mp_oHighestCOE[i] = new clTree*[m_iNumY];
      mp_fCOE[i] = new float[m_iNumY];
    }

  } //end of try block
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
    stcErr.sFunction = "clCompetitionHarvest::SetupCOEGrids" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// ResetResultsGrid()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::ResetResultsGrid()
{
  float fValue = 0;
  int iValue = 0;
  short int iNumXCells = mp_oResultsGrid->GetNumberXCells(),
      iNumYCells = mp_oResultsGrid->GetNumberYCells(),
      i, j, iSp; //loop counters

  //For the harvest results grid - reset all values
  for ( i = 0; i < iNumXCells; i++ )
    for ( j = 0; j < iNumYCells; j++ )
    {

      //Cut Density and Cut Basal Area - set values to 0
      for ( iSp = 0; iSp < m_iNumSpecies; iSp++ ) {
        mp_oResultsGrid->SetValueOfCell( i, j, mp_iDenCutCodes[iSp], iValue );
        mp_oResultsGrid->SetValueOfCell( i, j, mp_iBaCutCodes[iSp], fValue );
      }
    }
}


//////////////////////////////////////////////////////////////////////////////
// SetupGrids()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::SetupGrids()
{
  try
  {
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    std::stringstream sLabel;
    short int i; //loop counters

    mp_iDenCutCodes = new short int[m_iNumSpecies];
    mp_iBaCutCodes = new short int[m_iNumSpecies];

    mp_oResultsGrid = mp_oSimManager->GetGridObject("Competition Harvest Results");
    if (NULL == mp_oResultsGrid) {

      mp_oResultsGrid = mp_oSimManager->CreateGrid( "Competition Harvest Results",
          m_iNumSpecies,              //number of int data members
          m_iNumSpecies,              //num float data members
          0,                          //number of string data members
          0,                          //number of bool data members
          p_oPlot->GetXPlotLength(),  //X cell length
          p_oPlot->GetYPlotLength()); //Y cell length

      for ( i = 0; i < m_iNumSpecies; i++ )
      {
        sLabel << "Cut Density_" << i;
        mp_iDenCutCodes[i] = mp_oResultsGrid->RegisterInt(sLabel.str());
        sLabel.str("");
        sLabel << "Cut Basal Area_" << i;
        mp_iBaCutCodes[i] = mp_oResultsGrid->RegisterFloat(sLabel.str());
        sLabel.str("");
      }
    } else {

      //Grid already exists - get the codes
      for (i = 0; i < m_iNumSpecies; i++) {
        sLabel << "Cut Density_" << i;
        mp_iDenCutCodes[i] = mp_oResultsGrid->GetFloatDataCode(sLabel.str());
        if (-1 == mp_iDenCutCodes[i]) {
          modelErr stcErr;
          stcErr.sFunction = "clCompetitionHarvest::SetupGrids" ;
          std::stringstream s;
          s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Competition Harvest Results\" grid.";
          stcErr.sMoreInfo = s.str();
          stcErr.iErrorCode = BAD_DATA;
          throw( stcErr );
        }

        sLabel << "Cut Basal Area_" << i;
        mp_iBaCutCodes[i] = mp_oResultsGrid->GetFloatDataCode(sLabel.str());
        if (-1 == mp_iBaCutCodes[i]) {
          modelErr stcErr;
          stcErr.sFunction = "clCompetitionHarvest::SetupGrids" ;
          std::stringstream s;
          s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Competition Harvest Results\" grid.";
          stcErr.sMoreInfo = s.str();
          stcErr.iErrorCode = BAD_DATA;
          throw( stcErr );
        }
      }
    }

    //Use the ResetResultsGrid function to initialize the values
    ResetResultsGrid();

  } //end of try block
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
    stcErr.sFunction = "clCompetitionHarvest::SetupHarvestGrid" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// Action()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::Action()
{
  try
  {
    if (mp_oSimManager->GetCurrentTimestep() < m_iTimestepToStartHarvests) return;

    //Reset the results grid
    ResetResultsGrid();

    m_iTimeSinceLastHarvest++;

    if (m_iTimeSinceLastHarvest < m_iInterval) return; //hasn't been long enough

    //Check to see if it's a fixed-interval harvest - if so, it's time
    if (interval_rem == harvest || interval_prop == harvest) {
      float fBA = GetBasalArea();
      if (m_bIsSpeciesSpecific) CutTreesSpeciesSpecific(fBA);
      else CutTreesNotSpeciesSpecific(fBA);

    } else {

      //Evaluate for a fixed basal area threshold harvest. Get the basal area of
      //the plot.
      float fBA = GetBasalArea();
      if (fBA < m_fBAThreshold) return;
      if (m_bIsSpeciesSpecific) CutTreesSpeciesSpecific(fBA);
      else CutTreesNotSpeciesSpecific(fBA);
    }

    m_iTimeSinceLastHarvest = 0;

  } //end of try block
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
    stcErr.sFunction = "clCompetitionHarvest::Action" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetBasalArea()
//////////////////////////////////////////////////////////////////////////////
float clCompetitionHarvest::GetBasalArea()
{
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  clTreeSearch *p_oAllTrees; //all saplings and adults
  clTree * p_oTree;          //tree whose basal area we're getting`
  float fDbh,                //tree's dbh
  fBA = 0;             //total plot basal area
  int iIsDead;
  short int iTp, iSp,        //tree's type and species
  iDeadCode;

  //Ask for all saplings and adults
  p_oAllTrees = p_oPop->Find("type=2,3");
  p_oTree = p_oAllTrees->NextTree();
  while ( p_oTree )
  {
    iSp = p_oTree->GetSpecies();
    iTp = p_oTree->GetType();

    //Is this tree eligible? Any tree that has a COE member and a DBH between the
    //minimum and maximum is eligible.
    if (-1 < mp_iCOECodes[iSp][iTp - clTreePopulation::sapling]) {

      //Make sure this tree is not dead
      iDeadCode = p_oPop->GetIntDataCode( "dead", iSp, iTp );
      if ( -1 != iDeadCode )
        p_oTree->GetValue( iDeadCode, & iIsDead );
      else
        iIsDead = notdead;

      if (notdead == iIsDead) {
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDbh );
        if ( fDbh >= m_fMinHarvestDBH && fDbh <= m_fMaxHarvestDBH )
          fBA += clModelMath::CalculateBasalArea( fDbh );
      }

    } //end of while (p_oTree)
    p_oTree = p_oAllTrees->NextTree();
  }

  return fBA;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateOneCOE()
//////////////////////////////////////////////////////////////////////////////
float clCompetitionHarvest::CalculateOneCOE(clPlot *p_oPlot, clTreePopulation *p_oPop, clTree *p_oTree)
{
  clTreeSearch *p_oNeighbors;
  clTree *p_oNeighbor;
  std::stringstream sQuery; //format search strings into this
  float fCOE = 0, //competitive effect - the end result of all this math
      fDistance, //distance between target and neighbor
      fTargetDbh, //target's dbh
      fNeighDbh, //neighbor's dbh
      fTemp,
      fNeighX, fNeighY, //holders for the neighbor tree's X and Y
      fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iIsDead; //whether a neighbor is dead
  short int iNeighSp, iNeighTp, //species and type for neighbor
  iTargetSp = p_oTree->GetSpecies(), //target tree's species
  iTargetTp = p_oTree->GetType(), //target tree's type
  iDeadCode; //neighbor's dead code

  //Make sure this tree is not dead
  iDeadCode = p_oPop->GetIntDataCode( "dead", iTargetSp, iTargetTp );
  if ( -1 != iDeadCode ) {
    p_oTree->GetValue( iDeadCode, & iIsDead );
    if (iIsDead > notdead) return 0;
  }

  //Format the query to get all competing neighbors
  p_oTree->GetValue( p_oPop->GetXCode( iTargetSp, iTargetTp ), & fTargetX );
  p_oTree->GetValue( p_oPop->GetYCode( iTargetSp, iTargetTp ), & fTargetY );
  p_oTree->GetValue( p_oPop->GetDbhCode( iTargetSp, iTargetTp ), & fTargetDbh );

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings aren't affected by competition
  sQuery << "distance=" << mp_fMaxCrowdingRadius[iTargetSp] << "FROM x="
         << fTargetX << "y=" << fTargetY << "::height=" << m_fMinSaplingHeight;
  p_oNeighbors = p_oPop->Find(sQuery.str());
  sQuery.str("");

  //Loop through and assess the competitive effects of the target on each
  //neighbor
  p_oNeighbor = p_oNeighbors->NextTree();
  while ( p_oNeighbor )
  {
    iNeighSp = p_oNeighbor->GetSpecies();
    iNeighTp = p_oNeighbor->GetType();

    //Make sure the neighbor's not dead
    iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSp, iNeighTp );
    if ( -1 != iDeadCode )
      p_oNeighbor->GetValue( iDeadCode, & iIsDead );
    else
      iIsDead = notdead;

    if ( clTreePopulation::seedling != iNeighTp &&
        clTreePopulation::snag != iNeighTp && notdead == iIsDead)
    {

      //Get the neighbor's X and Y values
      p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSp, iNeighTp ),
          & fNeighX );
      p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSp, iNeighTp ),
          & fNeighY );

      //Get the distance between the two trees
      fDistance = p_oPlot->GetDistance( fTargetX, fTargetY, fNeighX, fNeighY );

      //Only continue if distance is not 0 - it will be a fluke condition to
      //allow a tree that is literally standing on top of another one not to
      //affect it competitively, but there it is
      if ( 0 != fDistance ) {

        //Get the neighbor's dbh
        p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSp, iNeighTp ), & fNeighDbh );

        //Add competitive effect on neighbor
        fTemp = pow(pow(fTargetDbh / m_fQ, mp_fAlpha[iNeighSp]) /
            pow(fDistance, mp_fBeta[iNeighSp]), mp_fD[iNeighSp]);
        fTemp *= mp_fLambda[iTargetSp][iNeighSp] *
            pow(fNeighDbh, mp_fGamma[iNeighSp]) *
            -mp_fC[iNeighSp];
        fCOE += exp(fTemp);
      }
    }

    p_oNeighbor = p_oNeighbors->NextTree();

  }
  return fCOE;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateAllCOEsSpeciesSpecific()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::CalculateAllCOEsSpeciesSpecific(clTreePopulation *p_oPop)
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  clTree *p_oTree;
  bool *p_bEligible = new bool[m_iNumSpecies];
  float fCOE, fHeight, fDbh;
  int iX, iY, iSp, iTp; //grid loop counters

  //Reset the grid array where we stash the current highest COEs
  for (iX = 0; iX < m_iNumX; iX++) {
    for (iY = 0; iY < m_iNumY; iY++) {
      mp_oHighestCOE[iX][iY] = NULL;
      mp_fCOE[iX][iY] = 0;
    }
  }

  //Find the eligible species - those that have a positive difference between
  //target amount to cut and amount already cut
  for (iSp = 0; iSp < m_iNumSpecies; iSp++) {
    if (mp_fTargetToCut[iSp] > mp_fAlreadyCut[iSp])
      p_bEligible[iSp] = true;
    else
      p_bEligible[iSp] = false;
  }

  for (iX = 0; iX < m_iNumX; iX++) {
    for (iY = 0; iY < m_iNumY; iY++) {
      p_oTree = p_oPop->GetTallestTreeInCell(iX, iY);
      while (p_oTree) {

        //Make sure the tree is eligible to have its COE calculated
        iSp = p_oTree->GetSpecies();
        iTp = p_oTree->GetType();
        if ((clTreePopulation::sapling == iTp ||
            clTreePopulation::adult == iTp) &&
            mp_iCOECodes[iSp][iTp - clTreePopulation::sapling] > -1 &&
            p_bEligible[iSp]) {

          p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
          if (fDbh >= m_fMinHarvestDBH && fDbh <= m_fMaxHarvestDBH)
            fCOE = CalculateOneCOE(p_oPlot, p_oPop, p_oTree);

          //If this is the new highest COE - keep a record of it
          if (fCOE > mp_fCOE[iX][iY]) {
            mp_fCOE[iX][iY] = fCOE;
            mp_oHighestCOE[iX][iY] = p_oTree;
          }
        }
        else {
          //Check to see if we've gone into seedling territory - in which case
          //we'll stop
          if (clTreePopulation::seedling == iTp) {
            p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fHeight);
            if (fHeight < m_fMinSaplingHeight)
              break;
          }
        }

        p_oTree = p_oTree->GetShorter();
      }
    }
  }

  delete[] p_bEligible;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateAllCOEsNotSpeciesSpecific()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::CalculateAllCOEsNotSpeciesSpecific(clTreePopulation *p_oPop)
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  clTree *p_oTree;
  float fCOE, fHeight, fDbh;
  int iX, iY, iSp, iTp; //grid loop counters

  //Reset the grid array where we stash the current highest COEs
  for (iX = 0; iX < m_iNumX; iX++) {
    for (iY = 0; iY < m_iNumY; iY++) {
      mp_oHighestCOE[iX][iY] = NULL;
      mp_fCOE[iX][iY] = 0;
    }
  }

  for (iX = 0; iX < m_iNumX; iX++) {
    for (iY = 0; iY < m_iNumY; iY++) {
      p_oTree = p_oPop->GetTallestTreeInCell(iX, iY);
      while (p_oTree) {

        //Make sure the tree is eligible to have its COE calculated
        iSp = p_oTree->GetSpecies();
        iTp = p_oTree->GetType();
        if ((clTreePopulation::sapling == iTp ||
            clTreePopulation::adult == iTp) &&
            mp_iCOECodes[iSp][iTp - clTreePopulation::sapling] > -1) {

          p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
          if (fDbh >= m_fMinHarvestDBH && fDbh <= m_fMaxHarvestDBH) {
            fCOE = CalculateOneCOE(p_oPlot, p_oPop, p_oTree);

            //If this is the new highest COE - keep a record of it
            if (fCOE > mp_fCOE[iX][iY]) {
              mp_fCOE[iX][iY] = fCOE;
              mp_oHighestCOE[iX][iY] = p_oTree;
            }
          }
        }
        else {
          //Check to see if we've gone into seedling territory - in which case
          //we'll stop
          if (clTreePopulation::seedling == iTp) {
            p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fHeight);
            if (fHeight < m_fMinSaplingHeight)
              break;
          }
        }

        p_oTree = p_oTree->GetShorter();
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// RecalculateCOESpeciesSpecific()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::RecalculateCOESpeciesSpecific(clTreePopulation *p_oPop, const float &fX, const float &fY)
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  clTree *p_oTree;
  bool *p_bEligible = new bool[m_iNumSpecies];
  float fCOE, fHeight, fDbh,
  fCellLength = p_oPop->GetGridCellSize();
  int iSp, iTp, iX, iY, iRealX, iRealY, iMinX, iMaxX, iMinY, iMaxY;

  //Find the grids we are going to change
  iMinX = (int)floor((fX - m_fMaxMaxCrowdingRadius)/fCellLength);
  iMaxX = (int)floor((fX + m_fMaxMaxCrowdingRadius)/fCellLength);
  iMinY = (int)floor((fY - m_fMaxMaxCrowdingRadius)/fCellLength);
  iMaxY = (int)floor((fY + m_fMaxMaxCrowdingRadius)/fCellLength);

  //Find the eligible species - those that have a positive difference between
  //target amount to cut and amount already cut
  for (iSp = 0; iSp < m_iNumSpecies; iSp++) {
    if (mp_fTargetToCut[iSp] > mp_fAlreadyCut[iSp])
      p_bEligible[iSp] = true;
    else
      p_bEligible[iSp] = false;
  }

  for (iRealX = iMinX; iRealX <= iMaxX; iRealX++) {
    for (iRealY = iMinY; iRealY <= iMaxY; iRealY++) {
      //Torus correction
      iX = iRealX; iY = iRealY;
      if (iX < 0) iX += m_iNumX;
      if (iX >= m_iNumX) iX -= m_iNumX;
      if (iY < 0) iY += m_iNumY;
      if (iY >= m_iNumY) iY -= m_iNumY;

      mp_fCOE[iX][iY] = 0;
      mp_oHighestCOE[iX][iY] = NULL;

      p_oTree = p_oPop->GetTallestTreeInCell(iX, iY);
      while (p_oTree) {

        //Make sure the tree is eligible to have its COE calculated
        iSp = p_oTree->GetSpecies();
        iTp = p_oTree->GetType();
        if ((clTreePopulation::sapling == iTp ||
            clTreePopulation::adult == iTp) &&
            mp_iCOECodes[iSp][iTp - clTreePopulation::sapling] > -1 &&
            p_bEligible[iSp]) {

          p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
          if (fDbh >= m_fMinHarvestDBH && fDbh <= m_fMaxHarvestDBH &&
              mp_fAlreadyCut[iSp] < mp_fTargetToCut[iSp]) {
            fCOE = CalculateOneCOE(p_oPlot, p_oPop, p_oTree);

            //If this is the new highest COE - keep a record of it
            if (fCOE > mp_fCOE[iX][iY]) {
              mp_fCOE[iX][iY] = fCOE;
              mp_oHighestCOE[iX][iY] = p_oTree;
            }
          }
        }
        else {
          //Check to see if we've gone into seedling territory - in which case
          //we'll stop
          if (clTreePopulation::seedling == iTp) {
            p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fHeight);
            if (fHeight < m_fMinSaplingHeight)
              break;
          }
        }

        p_oTree = p_oTree->GetShorter();
      }
    }
  }

  delete[] p_bEligible;
}

//////////////////////////////////////////////////////////////////////////////
// RecalculateCOENotSpeciesSpecific()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::RecalculateCOENotSpeciesSpecific(clTreePopulation *p_oPop, const float &fX, const float &fY)
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  clTree *p_oTree;
  float fCOE, fHeight, fDbh,
  fCellLength = p_oPop->GetGridCellSize();
  int iSp, iTp, iRealX, iRealY, iX, iY, iMinX, iMaxX, iMinY, iMaxY;

  //Find the grids we are going to change
  iMinX = (int)floor((fX - m_fMaxMaxCrowdingRadius)/fCellLength);
  iMaxX = (int)floor((fX + m_fMaxMaxCrowdingRadius)/fCellLength);
  iMinY = (int)floor((fY - m_fMaxMaxCrowdingRadius)/fCellLength);
  iMaxY = (int)floor((fY + m_fMaxMaxCrowdingRadius)/fCellLength);

  for (iRealX = iMinX; iRealX <= iMaxX; iRealX++) {
    for (iRealY = iMinY; iRealY <= iMaxY; iRealY++) {
      //Torus correction
      iX = iRealX; iY = iRealY;
      if (iX < 0) iX += m_iNumX;
      if (iX >= m_iNumX) iX -= m_iNumX;
      if (iY < 0) iY += m_iNumY;
      if (iY >= m_iNumY) iY -= m_iNumY;

      mp_fCOE[iX][iY] = 0;
      mp_oHighestCOE[iX][iY] = NULL;

      p_oTree = p_oPop->GetTallestTreeInCell(iX, iY);
      while (p_oTree) {

        //Make sure the tree is eligible to have its COE calculated
        iSp = p_oTree->GetSpecies();
        iTp = p_oTree->GetType();
        if ((clTreePopulation::sapling == iTp ||
            clTreePopulation::adult == iTp) &&
            mp_iCOECodes[iSp][iTp - clTreePopulation::sapling] > -1) {

          p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
          if (fDbh >= m_fMinHarvestDBH && fDbh <= m_fMaxHarvestDBH) {
            fCOE = CalculateOneCOE(p_oPlot, p_oPop, p_oTree);

            //If this is the new highest COE - keep a record of it
            if (fCOE > mp_fCOE[iX][iY]) {
              mp_fCOE[iX][iY] = fCOE;
              mp_oHighestCOE[iX][iY] = p_oTree;
            }
          }
        }
        else {
          //Check to see if we've gone into seedling territory - in which case
          //we'll stop
          if (clTreePopulation::seedling == iTp) {
            p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fHeight);
            if (fHeight < m_fMinSaplingHeight)
              break;
          }
        }

        p_oTree = p_oTree->GetShorter();
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// CutTreesNotSpeciesSpecific()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::CutTreesNotSpeciesSpecific(const float &fPlotBA)
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTree *p_oTree;
  fstream out;
  float fX, fY, //coordinates of cut trees
  fTemp,
  fBA, //basal area of trees to cut
  fDbh,
  fMaxCOE, //for finding the highest COE tree
  fCutSoFar = 0, //how much has been cut so far
  fAmtToCutThisTS; //amount to cut this timestep
  int i, iX, iY, //loop counters
  iTemp,
  iTs = mp_oSimManager->GetCurrentTimestep(),
  iSp, iTp,
  iMaxX, iMaxY; //for finding the highest COE tree
  bool bHarvestDone = false, bKillTree;

  if (m_sHarvestListFilename.length() > 0)
    out.open(m_sHarvestListFilename.c_str(), ios::out | ios::app);

  //Calculate the cut targets
  if (interval_rem == harvest) {
    //Fixed interval harvest, target amount remaining. Amount to harvest is the
    //difference between the current basal area and the amount to cut back to.
    fAmtToCutThisTS = fPlotBA - m_fAmtToCut;
  } else if (interval_prop == harvest) {
    //Fixed interval harvest, target proportion to cut. Amount to harvest is the
    //appropriate fraction of total BA.
    fAmtToCutThisTS = fPlotBA * m_fAmtToCut;
  } else if (ba_amt == harvest) {
    //Fixed basal area harvest. Target is constant.
    fAmtToCutThisTS = m_fAmtToCut;
  } else {
    //Fixed basal area harvest. Target amount is proportion of total.
    fAmtToCutThisTS = fPlotBA * m_fAmtToCut;
  }

  //Now take the amount to cut and proportion it among the species. Set them all
  //very high so that any are eligible for harvesting.
  for (i = 0; i < m_iNumSpecies; i++) {
    mp_fAlreadyCut[i] = 0;
    mp_fTargetToCut[i] = 1000000;
  }

  //Calculate competitive effects of all trees
  CalculateAllCOEsNotSpeciesSpecific(p_oPop);

  while (false == bHarvestDone) {

    //Find the tree with the highest COE
    fMaxCOE = 0;
    for (iX = 0; iX < m_iNumX; iX++) {
      for (iY = 0; iY < m_iNumY; iY++) {
        if (mp_fCOE[iX][iY] > fMaxCOE) {
          fMaxCOE = mp_fCOE[iX][iY];
          iMaxX = iX; iMaxY = iY;
        }
      }
    }
    if (0 < fMaxCOE) {

      p_oTree = mp_oHighestCOE[iMaxX][iMaxY];
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Get the tree's basal area
      p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
      fBA = clModelMath::CalculateBasalArea( fDbh );

      //Cut this tree?
      bKillTree = false;
      if ((fCutSoFar + fBA) < fAmtToCutThisTS)
        bKillTree = true;
      else
        //Cutting this tree will cause overshoot - use a random number to
        //decide whether to cut it
        bKillTree = clModelMath::GetRand() < (fAmtToCutThisTS - fCutSoFar) / fBA;

      if (bKillTree) {
        //Yep, cut it
        //Get the tree's coordinates so we'll be able to recalculate the COEs
        //of its neighbors
        p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
        p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

        p_oPop->KillTree(p_oTree, m_iReasonCode);
        fCutSoFar += fBA;

        //Add this tree's stats to the results grid
        mp_oResultsGrid->GetValueAtPoint( fX, fY, mp_iDenCutCodes[iSp], &iTemp);
        iTemp += 1;
        mp_oResultsGrid->SetValueAtPoint( fX, fY, mp_iDenCutCodes[iSp], iTemp);

        mp_oResultsGrid->GetValueAtPoint( fX, fY, mp_iBaCutCodes[iSp], &fTemp);
        fTemp += fBA;
        mp_oResultsGrid->SetValueAtPoint( fX, fY, mp_iBaCutCodes[iSp], fTemp);

        //Write to output file, if using
        if (out) {
          out << fX << "\t" << fY << "\t" << iSp << "\t" << fDbh << "\t"
              << iTs << "\n";
        }

        //Recalculate COEs in the neighboring area
        RecalculateCOENotSpeciesSpecific(p_oPop, fX, fY);
      }
      else
        bHarvestDone = true;
    } else {
      //No more trees
      bHarvestDone = true;
    }
  }

  if (out) out.close();
}

//////////////////////////////////////////////////////////////////////////////
// CutTreesSpeciesSpecific()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionHarvest::CutTreesSpeciesSpecific(const float &fPlotBA)
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTree *p_oTree;
  fstream out;
  float fX, fY, //coordinates of cut trees
  fTemp,
  fBA, //basal area of trees to cut
  fDbh,
  fMaxCOE, //for finding the highest COE tree
  fAmtToCutThisTS; //amount to cut this timestep
  int i, iX, iY, //loop counters
  iTemp,
  iTs = mp_oSimManager->GetCurrentTimestep(),
  iSp, iTp,
  iMaxX, iMaxY; //for finding the highest COE tree
  bool bHarvestDone = false, bKillTree;

  if (m_sHarvestListFilename.length() > 0)
    out.open(m_sHarvestListFilename.c_str(), ios::out | ios::app);

  //Calculate the cut targets
    if (interval_rem == harvest) {
      //Fixed interval harvest, target amount remaining. Amount to harvest is the
      //difference between the current basal area and the amount to cut back to.
      fAmtToCutThisTS = fPlotBA - m_fAmtToCut;
    } else if (interval_prop == harvest) {
      //Fixed interval harvest, target proportion to cut. Amount to harvest is the
      //appropriate fraction of total BA.
      fAmtToCutThisTS = fPlotBA * m_fAmtToCut;
    } else if (ba_amt == harvest) {
      //Fixed basal area harvest. Target is constant.
      fAmtToCutThisTS = m_fAmtToCut;
    } else {
      //Fixed basal area harvest. Target amount is proportion of total.
      fAmtToCutThisTS = fPlotBA * m_fAmtToCut;
    }

  //Now take the amount to cut and proportion it among the species.
  for (i = 0; i < m_iNumSpecies; i++) {
    mp_fAlreadyCut[i] = 0;
    mp_fTargetToCut[i] = fAmtToCutThisTS * mp_fPropToCut[i];
  }

  //Calculate competitive effects of all trees
  CalculateAllCOEsSpeciesSpecific(p_oPop);

  while (false == bHarvestDone) {

    //Find the tree with the highest COE of an uncut species
    fMaxCOE = 0;
    for (iX = 0; iX < m_iNumX; iX++) {
      for (iY = 0; iY < m_iNumY; iY++) {
        if (mp_fCOE[iX][iY] > fMaxCOE) {
          fMaxCOE = mp_fCOE[iX][iY];
          iMaxX = iX; iMaxY = iY;
        }
      }
    }
    if (0 < fMaxCOE) {

      p_oTree = mp_oHighestCOE[iMaxX][iMaxY];
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Get the tree's basal area
      p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
      fBA = clModelMath::CalculateBasalArea( fDbh );

      //Cut this tree?
      bKillTree = false;
      if ((mp_fAlreadyCut[iSp] + fBA) < mp_fTargetToCut[iSp])
        bKillTree = true;
      else
        //Cutting this tree will cause overshoot - use a random number to
        //decide whether to cut it
        bKillTree = clModelMath::GetRand() < (mp_fTargetToCut[iSp] - mp_fAlreadyCut[iSp]) / fBA;

      if (bKillTree) {
        //Yep, cut it
        //Get the tree's coordinates so we'll be able to recalculate the COEs
        //of its neighbors
        p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
        p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

        p_oPop->KillTree(p_oTree, m_iReasonCode);
        mp_fAlreadyCut[iSp] += fBA;

        //Add this tree's stats to the results grid
        mp_oResultsGrid->GetValueAtPoint( fX, fY, mp_iDenCutCodes[iSp], &iTemp);
        iTemp += 1;
        mp_oResultsGrid->SetValueAtPoint( fX, fY, mp_iDenCutCodes[iSp], iTemp);

        mp_oResultsGrid->GetValueAtPoint( fX, fY, mp_iBaCutCodes[iSp], &fTemp);
        fTemp += fBA;
        mp_oResultsGrid->SetValueAtPoint( fX, fY, mp_iBaCutCodes[iSp], fTemp);

        //Write to output file, if using
        if (out) {
          out << fX << "\t" << fY << "\t" << iSp << "\t" << fDbh << "\t"
              << iTs << "\n";
        }

        //Recalculate COEs in the neighboring area
        RecalculateCOESpeciesSpecific(p_oPop, fX, fY);
      }
      else {
        //This was the last tree of this species - set the amount killed
        //above the target so no more of this species will be killed
        mp_fAlreadyCut[iSp] = mp_fTargetToCut[iSp] + 0.1;

        //Check to see if all species' targets have been met
        bHarvestDone = true;
        for (i = 0; i < m_iNumSpecies; i++) {
          if (mp_fAlreadyCut[i] < mp_fTargetToCut[i]) {
            bHarvestDone = false;
            //Recalculate COEs in the neighboring area - this knocks off
            //trees of species that are done harvesting
            RecalculateCOESpeciesSpecific(p_oPop, fX, fY);
            break;
          }
        }
      }
    } else {
      //No more trees
      bHarvestDone = true;
    }
  }

  if (out) out.close();
}
