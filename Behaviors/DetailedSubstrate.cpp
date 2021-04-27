//---------------------------------------------------------------------------
#include "DetailedSubstrate.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "ModelMath.h"
#include "Disturbance.h"
#include "Plot.h"
#include <stdio.h>
#include <math.h>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clDetailedSubstrate::clDetailedSubstrate( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "DetailedSubstrate";
    m_sXMLRoot = "DetailedSubstrate";

    //NULL our pointers
    mp_iDeadCodes = NULL;
    mp_iFallCodes = NULL;
    mp_iNewBreakCodes = NULL;
    mp_iOldBreakCodes = NULL;
    mp_bSubstrateApplies = NULL;
    mp_fPropOfFallenThatUproot = NULL;
    mp_fPropOfSnagsThatUproot = NULL;
    mp_fLogDecayProp = NULL;
    mp_fScarSoilDecayProp = NULL;
    mp_fTipupDecayProp = NULL;
    mp_fPropFallenTreesLogDecayClass = NULL;
    mp_fPropFallenSnagsLogDecayClass = NULL;
    mp_fInitLog = NULL;
    mp_fPartCutLog = NULL;
    mp_fGapCutLog = NULL;
    mp_fClearCutLog = NULL;
    mp_fLogDecayAlpha = NULL;
    mp_fLogDecayBeta = NULL;
    mp_iLogCodes = NULL;
    mp_iLogVolCodes = NULL;
    mp_iLogCalcsCode = NULL;
    mp_iNewLogCalcsCode = NULL;
    mp_fLogDecayProp = NULL;
    mp_iLogSpGroupAssignment = NULL;
    mp_fInitialLogMeanDiameter = NULL;
    mp_fPartCutLogMeanDiameter = NULL;
    mp_fGapCutLogMeanDiameter = NULL;
    mp_fClearCutLogMeanDiameter = NULL;
    m_iNumTotalSpecies = 0;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1.0;
    m_fMinimumVersionNumber = 1.0;

    m_fClearCutTipUp = 0;
    mp_oHarvestGrid = NULL;
    m_fGridCellArea = 0;
    m_bParFileHasSubstrateMap = false;
    m_iPkgAgeCode = -1;
    m_iOrigFreshLogCode = -1;
    m_iMaxDecayYears = 0;
    m_fTipUpB = 0;
    m_iNumLogSpGroupsUsed = 0;
    m_fClearCutScarifiedSoil = 0;
    m_iTipupCode = -1;
    m_fInitTipUp = 0;
    m_fPartCutTipUp = 0;
    m_iOrigFFMossCode = -1;
    mp_oOriginalSubstrateGrid = NULL;
    m_fScarifiedSoilB = 0;
    m_fScarifiedSoilA = 0;
    m_iOrigTipupCode = -1;
    m_fRootTipupFactor = 0;
    m_fMossProportion = 0;
    m_iOrigFFLitterCode = -1;
    mp_oCalcGrid = NULL;
    m_fInitScarifiedSoil = 0;
    m_iHarvestTypeCode = -1;
    m_iPkgScarSoilCode = -1;
    m_iMaxDecayTimesteps = 0;
    m_fTipUpA = 0;
    m_fPartCutScarifiedSoil = 0;
    m_iOrigScarSoilCode = -1;
    m_iFFMossCode = -1;
    m_fGapCutScarifiedSoil = 0;
    mp_oSubstrateGrid = NULL;
    m_iPkgTipupCode = -1;
    m_iTotalLogVolCode = -1;
    m_fLogSizeClassBoundary = 0;
    m_bUseDirectionalTreeFall = false;
    m_iOrigDecLogCode = -1;
    m_iScarSoilCode = -1;
    m_iNewTipupCalcsCode = -1;
    m_fGapCutTipUp = 0;
    m_iTotalLogCode = -1;
    m_iFFLitterCode = -1;
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
    stcErr.sFunction = "clDetailedSubstrate::clDetailedSubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clDetailedSubstrate::~clDetailedSubstrate() {

  int i, j, k, t, //loop counters
  iNumLogSpGroupsUsed = 3;

  if (mp_iDeadCodes && mp_bSubstrateApplies) {
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      delete[] mp_iDeadCodes[i];
      delete[] mp_bSubstrateApplies[i];
      delete[] mp_iNewBreakCodes[i];
    }
  }

  delete[] mp_iDeadCodes;
  delete[] mp_bSubstrateApplies;
  delete[] mp_iNewBreakCodes;
  delete[] mp_fScarSoilDecayProp;
  delete[] mp_fTipupDecayProp;
  delete[] mp_iFallCodes;
  delete[] mp_iOldBreakCodes;

  delete[] mp_fPropOfFallenThatUproot;
  delete[] mp_fPropOfSnagsThatUproot;
  delete[] mp_fPropFallenTreesLogDecayClass;
  delete[] mp_fPropFallenSnagsLogDecayClass;
  delete[] mp_iLogSpGroupAssignment;

  delete[] mp_fInitialLogMeanDiameter;
  delete[] mp_fPartCutLogMeanDiameter;
  delete[] mp_fGapCutLogMeanDiameter;
  delete[] mp_fClearCutLogMeanDiameter;

  for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
    for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
      delete[] mp_fInitLog[i][j];
      delete[] mp_fLogDecayAlpha[i][j];
      delete[] mp_fLogDecayBeta[i][j];
    }
    delete[] mp_fInitLog[i];
    delete[] mp_fLogDecayAlpha[i];
    delete[] mp_fLogDecayBeta[i];
  }

  if (mp_fPartCutLog) {
    for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
      for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
        delete[] mp_fPartCutLog[i][j];
        delete[] mp_fGapCutLog[i][j];
        delete[] mp_fClearCutLog[i][j];
      }
      delete[] mp_fPartCutLog[i];
      delete[] mp_fGapCutLog[i];
      delete[] mp_fClearCutLog[i];
    }
  }

  if (mp_fLogDecayProp) {
    for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
      for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
        for (k = 5; k >= 1; k--) {   //k is decay class (1-5)
          for (t = 0; t < m_iMaxDecayTimesteps; t++) {
            delete[] mp_fLogDecayProp[i][j][k-1][t];
          }
          delete[] mp_fLogDecayProp[i][j][k-1];
        }
        delete[] mp_fLogDecayProp[i][j];
      }
      delete[] mp_fLogDecayProp[i];
    }
  }

  if (mp_iLogCodes) {
    for (i = 0; i < iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
      for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
        for (k = 5; k >= 1; k--) {   //k is decay class (1-5)
          delete[] mp_iLogCalcsCode[i][j][k-1];
        }
        delete[] mp_iLogCodes[i][j];
        delete[] mp_iLogVolCodes[i][j];
        delete[] mp_iLogCalcsCode[i][j];
        delete[] mp_iNewLogCalcsCode[i][j];
      }
      delete[] mp_iLogCodes[i];
      delete[] mp_iLogVolCodes[i];
      delete[] mp_iLogCalcsCode[i];
      delete[] mp_iNewLogCalcsCode[i];
    }
  }

  delete[] mp_fInitLog;
  delete[] mp_fPartCutLog;
  delete[] mp_fGapCutLog;
  delete[] mp_fClearCutLog;
  delete[] mp_fLogDecayAlpha;
  delete[] mp_fLogDecayBeta;
  delete[] mp_iLogCodes;
  delete[] mp_iLogVolCodes;
  delete[] mp_iLogCalcsCode;
  delete[] mp_fLogDecayProp;
  delete[] mp_iNewLogCalcsCode;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::GetData(xercesc::DOMDocument * p_oDoc) {
  try
  {
    //Do this now - the presence of the Harvest grid is the proxy for
    //whether Harvest is in the behavior list
    mp_oHarvestGrid = mp_oSimManager->GetGridObject( "Harvest Results" );

    //Read in the values from the parameter file
    GetParameterFileData( p_oDoc );

    //Pre-calculate decay proportions
    CalculateDecayProportions();

    //Set up the substrate grids
    SetupSubstrateGrids();

    //Put the initial conditions into the substrate grid
    SetInitialSubstrate();

    //Get the codes for the various data members
    GetCodes();
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
    stcErr.sFunction = "clDetailedSubstrate::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::GetParameterFileData(xercesc::DOMDocument * p_oDoc) {
  try
  {
    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    doubleVal * p_fTempValues; //for getting species-specific values
    intVal * p_iTempValues; //for getting species-specific values
    float fTemp;

    int i,j,k; //loop counters
    std::string p_sSizes[] = {"Small","Large"}; //translates array indexes to size label in parameter names
    std::stringstream sConcatenatedString;
    bool bBadDecayValue; //indicates decay parameters are outside valid range

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

    //Declare the permanent arrays
    mp_fPropOfFallenThatUproot = new double[m_iNumTotalSpecies];
    mp_fPropOfSnagsThatUproot = new double[m_iNumTotalSpecies];
    mp_iLogSpGroupAssignment = new int[m_iNumTotalSpecies];
    mp_fPropFallenTreesLogDecayClass = new double[5];
    mp_fPropFallenSnagsLogDecayClass = new double[5];
    mp_fInitialLogMeanDiameter = new double[2];
    mp_fPartCutLogMeanDiameter = new double[2];
    mp_fGapCutLogMeanDiameter = new double[2];
    mp_fClearCutLogMeanDiameter = new double[2];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    p_iTempValues = new intVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fTempValues[i].code = mp_iWhatSpecies[i];
      p_iTempValues[i].code = mp_iWhatSpecies[i];
    }

    //*****************************************
    // Read in the values from the parameter file
    //*****************************************

    //Log size class boundary
    FillSingleValue( p_oElement, "su_logSizeClassBoundary", & m_fLogSizeClassBoundary, true );

    //Log species group assignments
    m_iNumLogSpGroupsUsed=0;

    FillSpeciesSpecificValue( p_oElement, "su_logSpGroupAssignment", "su_lsgaVal", p_iTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );

    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_iLogSpGroupAssignment[p_iTempValues[i].code] = p_iTempValues[i].val - 1; //subtract 1 to make array zero-indexed

      //calc to find number of groups used
      if (p_iTempValues[i].val > m_iNumLogSpGroupsUsed)
        m_iNumLogSpGroupsUsed = p_iTempValues[i].val;
    }

    //Area tipup matter exposed
    FillSingleValue( p_oElement, "su_rootTipupFactor", & m_fRootTipupFactor, true );

    //Proportion of fallen live trees in each decay class
    //Proportion of fallen snags in each decay class
    for (k=1;k<=5;k++) {
      sConcatenatedString << "su_propFallenTreesLogDecayClass" << k;
      FillSingleValue( p_oElement, sConcatenatedString.str(), & mp_fPropFallenTreesLogDecayClass[k-1], true );
      sConcatenatedString.str("");

      sConcatenatedString << "su_propFallenSnagsLogDecayClass" << k;
      FillSingleValue( p_oElement, sConcatenatedString.str(), & mp_fPropFallenSnagsLogDecayClass[k-1], true );
      sConcatenatedString.str("");
    }

    //Proportion of fallen that uproot
    FillSpeciesSpecificValue( p_oElement, "su_propOfFallUproot", "su_pofuVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, turning each value into a
    //per-timestep proportion on the way
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPropOfFallenThatUproot[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Proportion of snags that uproot
    FillSpeciesSpecificValue( p_oElement, "su_propOfSnagsUproot", "su_posuVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPropOfSnagsThatUproot[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Scarified soil decay alpha
    FillSingleValue( p_oElement, "su_scarSoilDecayAlpha", & m_fScarifiedSoilA, true );
    //Scarified soil decay beta
    FillSingleValue( p_oElement, "su_scarSoilDecayBeta", & m_fScarifiedSoilB, true );
    //Tipup decay alpha
    FillSingleValue( p_oElement, "su_tipupDecayAlpha", & m_fTipUpA, true );
    //Tipup decay beta
    FillSingleValue( p_oElement, "su_tipupDecayBeta", & m_fTipUpB, true );

    //Maximum number of decay years
    FillSingleValue( p_oElement, "su_maxNumberDecayYears", & m_iMaxDecayYears, true );

    //Proportion of forest floor that is moss
    FillSingleValue( p_oElement, "su_mossProportion", & m_fMossProportion, true );

    //Directional tree fall?
    FillSingleValue( p_oElement, "su_directionalTreeFall", & m_bUseDirectionalTreeFall, true );

    //Initial conditions - scarified soil
    FillSingleValue( p_oElement, "su_initialScarSoil", & m_fInitScarifiedSoil, true );
    //Initial conditions - tipup
    FillSingleValue( p_oElement, "su_initialTipup", & m_fInitTipUp, true );

    //Initial conditions - mean diameter by size class

    //default values if parameters not specified
    mp_fInitialLogMeanDiameter[0] = 0.5 * m_fLogSizeClassBoundary;
    mp_fInitialLogMeanDiameter[1] = 1.5 * m_fLogSizeClassBoundary;
    //try to read in values
    FillSingleValue( p_oElement, "su_initialSmallLogMeanDiameter", & mp_fInitialLogMeanDiameter[0], false );
    FillSingleValue( p_oElement, "su_initialLargeLogMeanDiameter", & mp_fInitialLogMeanDiameter[1], false );

    //Initial conditions and decay parameters - logs

    mp_fInitLog = new double **[m_iNumLogSpGroupsUsed];
    mp_fLogDecayAlpha = new double **[m_iNumLogSpGroupsUsed];
    mp_fLogDecayBeta = new double **[m_iNumLogSpGroupsUsed];

    for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
      mp_fInitLog[i] = new double *[2];
      mp_fLogDecayAlpha[i] = new double *[2];
      mp_fLogDecayBeta[i] = new double *[2];

      for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
        mp_fInitLog[i][j] = new double [5];
        mp_fLogDecayAlpha[i][j] = new double [5];
        mp_fLogDecayBeta[i][j] = new double [5];

        for (k = 1; k <= 5; k++) { //k is decay class (1-5)
          sConcatenatedString << "su_initialLogSpGroup" << i+1 << p_sSizes[j] << "DecayClass" << k;
          FillSingleValue( p_oElement, sConcatenatedString.str(), & (mp_fInitLog[i][j][k-1]), true );
          sConcatenatedString.str("");

          sConcatenatedString << "su_logSpGroup" << i+1 << p_sSizes[j]
                              << "DecayClass" << k << "DecayAlpha";
          FillSingleValue( p_oElement, sConcatenatedString.str(), & (mp_fLogDecayAlpha[i][j][k-1]), true );
          sConcatenatedString.str("");

          sConcatenatedString << "su_logSpGroup" << i+1 << p_sSizes[j]
                              << "DecayClass" << k << "DecayBeta";
          FillSingleValue( p_oElement, sConcatenatedString.str(), & (mp_fLogDecayBeta[i][j][k-1]), true );
          sConcatenatedString.str("");
        } //end for k
      } //end for j
    }// end for i


    //Only do the harvest conditions if harvest is in the behavior list
    if ( mp_oHarvestGrid )
    {
      //Partial cut conditions - scarified soil
      FillSingleValue( p_oElement, "su_partialCutScarSoil", & m_fPartCutScarifiedSoil, true );
      //Gap cut conditions - scarified soil
      FillSingleValue( p_oElement, "su_gapCutScarSoil", & m_fGapCutScarifiedSoil, true );
      //Clear cut conditions - scarified soil
      FillSingleValue( p_oElement, "su_clearCutScarSoil", & m_fClearCutScarifiedSoil, true );

      //Partial cut conditions - tipup
      FillSingleValue( p_oElement, "su_partialCutTipup", & m_fPartCutTipUp, true );
      //Gap cut conditions - tipup
      FillSingleValue( p_oElement, "su_gapCutTipup", & m_fGapCutTipUp, true );
      //Clear cut conditions - tipup
      FillSingleValue( p_oElement, "su_clearCutTipup", & m_fClearCutTipUp, true );

      //Partial cut conditions - mean diameter by size class

      //default values if parameters not specified
      mp_fPartCutLogMeanDiameter[0] = 0.5 * m_fLogSizeClassBoundary;
      mp_fPartCutLogMeanDiameter[1] = 1.5 * m_fLogSizeClassBoundary;

      mp_fGapCutLogMeanDiameter[0] = 0.5 * m_fLogSizeClassBoundary;
      mp_fGapCutLogMeanDiameter[1] = 1.5 * m_fLogSizeClassBoundary;

      mp_fClearCutLogMeanDiameter[0] = 0.5 * m_fLogSizeClassBoundary;
      mp_fClearCutLogMeanDiameter[1] = 1.5 * m_fLogSizeClassBoundary;

      //try to read in values
      FillSingleValue( p_oElement, "su_partialCutSmallLogMeanDiameter", & mp_fPartCutLogMeanDiameter[0], false );
      FillSingleValue( p_oElement, "su_partialCutLargeLogMeanDiameter", & mp_fPartCutLogMeanDiameter[1], false );
      //Gap cut conditions - mean diameter by size class
      FillSingleValue( p_oElement, "su_gapCutSmallLogMeanDiameter", & mp_fGapCutLogMeanDiameter[0], false );
      FillSingleValue( p_oElement, "su_gapCutLargeLogMeanDiameter", & mp_fGapCutLogMeanDiameter[1], false );
      //Clear cut conditions - mean diameter by size class
      FillSingleValue( p_oElement, "su_clearCutSmallLogMeanDiameter", & mp_fClearCutLogMeanDiameter[0], false );
      FillSingleValue( p_oElement, "su_clearCutLargeLogMeanDiameter", & mp_fClearCutLogMeanDiameter[1], false );

      //Partial cut conditions - logs
      //Gap cut conditions - logs
      //Clear cut conditions - logs

      mp_fPartCutLog = new double **[m_iNumLogSpGroupsUsed];
      mp_fGapCutLog = new double **[m_iNumLogSpGroupsUsed];
      mp_fClearCutLog = new double **[m_iNumLogSpGroupsUsed];

      for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
        mp_fPartCutLog[i] = new double *[2];
        mp_fGapCutLog[i] = new double *[2];
        mp_fClearCutLog[i] = new double *[2];

        for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
          mp_fPartCutLog[i][j] = new double [5];
          mp_fGapCutLog[i][j] = new double [5];
          mp_fClearCutLog[i][j] = new double [5];

          for (k = 1; k <= 5; k++) { //k is decay class (1-5)
            sConcatenatedString << "su_partialCutLogSpGroup" << i+1
                 << p_sSizes[j] << "DecayClass" << k;
            FillSingleValue( p_oElement, sConcatenatedString.str(), & (mp_fPartCutLog[i][j][k-1]), true );
            sConcatenatedString.str("");

            sConcatenatedString << "su_gapCutLogSpGroup" << i+1
                 << p_sSizes[j] << "DecayClass" << k;
            FillSingleValue( p_oElement, sConcatenatedString.str(), & (mp_fGapCutLog[i][j][k-1]), true );
            sConcatenatedString.str("");

            sConcatenatedString << "su_clearCutLogSpGroup" << i+1
                 << p_sSizes[j] << "DecayClass" << k;
            FillSingleValue( p_oElement, sConcatenatedString.str(), & (mp_fClearCutLog[i][j][k-1]), true );
            sConcatenatedString.str("");
          } //end for k
        } //end for j
      } //end for i
    } //end if harvest


    //*****************************************
    // Validation and subsequent calculations - all of the proportions must add
    // up to less than or equal to 1.  If they don't, throw error.  If they do,
    // forest floor pool is the remainder needed to make it all add up to 1.
    //*****************************************

    //Max number of timesteps
    m_iMaxDecayTimesteps = (int)ceil( m_iMaxDecayYears / mp_oSimManager->GetNumberOfYearsPerTimestep() ) + 1;

    //Species group assignments
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      if ( mp_iLogSpGroupAssignment[i] < 0 || mp_iLogSpGroupAssignment[i] > 2)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "Species must be assigned a group numbered 1-3.";
        throw( stcErr );
      }
    }

    //Decay parameters
    bBadDecayValue = false;
    for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
      for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
        for (k = 1; k <= 5; k++) {  //k is decay class (1-5)
          if (mp_fLogDecayBeta[i][j][k-1] > 30.0/log(m_iMaxDecayTimesteps))
            bBadDecayValue = true;
          else if (mp_fLogDecayAlpha[i][j][k-1] > 0.0)
            bBadDecayValue = true;
          else if (mp_fLogDecayAlpha[i][j][k-1] * pow(m_iMaxDecayTimesteps,mp_fLogDecayBeta[i][j][k-1]) < -30.0)
            bBadDecayValue = true;
        }
      }
    }

    if (m_fScarifiedSoilB > 30.0/log(m_iMaxDecayTimesteps))
      bBadDecayValue = true;
    else if (m_fScarifiedSoilA > 0.0)
      bBadDecayValue = true;
    else if (m_fScarifiedSoilA * pow(m_iMaxDecayTimesteps,m_fScarifiedSoilB) < -30.0)
      bBadDecayValue = true;

    if (m_fTipUpB > 30.0/log(m_iMaxDecayTimesteps))
      bBadDecayValue = true;
    else if (m_fTipUpA > 0.0)
      bBadDecayValue = true;
    else if (m_fTipUpA * pow(m_iMaxDecayTimesteps,m_fTipUpB) < -30.0)
      bBadDecayValue = true;

    if (bBadDecayValue)
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
      stcErr.sMoreInfo = "Substrate decay parameter(s) outside of valid range.";
      throw( stcErr );
    }

    //Initial conditions substrate proportions
    fTemp = 0.0;
    for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
      for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
        for (k = 1; k <= 5; k++)  //k is decay class (1-5)
          fTemp += mp_fInitLog[i][j][k-1];

    fTemp += m_fInitScarifiedSoil + m_fInitTipUp;
    if ( fTemp > 1.0 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
      stcErr.sMoreInfo = "Initial conditions substrate proportions must add up to be less than 1";
      throw( stcErr );
    }

    //Initial conditions mean diameters

    if (mp_fInitialLogMeanDiameter[0] > m_fLogSizeClassBoundary)
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
      stcErr.sMoreInfo = "su_initialSmallLogMeanDiameter must be less than su_logSizeClassBoundary";
      throw( stcErr );
    }
    if (mp_fInitialLogMeanDiameter[1] < m_fLogSizeClassBoundary)
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
      stcErr.sMoreInfo = "su_initialLargeLogMeanDiameter must be greater than su_logSizeClassBoundary";
      throw( stcErr );
    }

    //Proportion of fallen live trees in each decay class
    fTemp=0.0;
    for (k = 1; k <= 5; k++)  //k is decay class (1-5)
      fTemp -= mp_fPropFallenTreesLogDecayClass[k-1];

    fTemp += 1;
    if ( fabs(fTemp) > 0.0001 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
      stcErr.sMoreInfo = "Proportion of fallen live trees in decay classes 1-5 must sum to 1.";
      throw( stcErr );
    }

    //Proportion of fallen snags in each decay class
    fTemp=0.0;
    for (k = 1; k <= 5; k++)  //k is decay class (1-5)
      fTemp -= mp_fPropFallenSnagsLogDecayClass[k-1];

    fTemp += 1;
    if ( fabs(fTemp) > 0.0001 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
      stcErr.sMoreInfo = "Proportion of fallen snags in decay classes 1-5 must sum to 1.";
      throw( stcErr );
    }

    if ( mp_oHarvestGrid )
    {

      //Partial cut substrate proportions
      fTemp=0.0;
      for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
        for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
          for (k = 1; k <= 5; k++)  //k is decay class (1-5)
            fTemp += mp_fPartCutLog[i][j][k-1];

      fTemp += m_fPartCutScarifiedSoil + m_fPartCutTipUp;
      if ( fTemp > 1.0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "Partial cut substrate proportions must add up to be less than 1";
        throw( stcErr );
      }

      //Gap cut substrate proportions
      fTemp=0.0;
      for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
        for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
          for (k = 1; k <= 5; k++)  //k is decay class (1-5)
            fTemp += mp_fGapCutLog[i][j][k-1];

      fTemp += m_fGapCutScarifiedSoil + m_fGapCutTipUp;
      if ( fTemp > 1.0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "Gap cut substrate proportions must add up to be less than 1";
        throw( stcErr );
      }

      //Clear cut substrate proportions
      fTemp=0.0;
      for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
        for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
          for (k = 1; k <= 5; k++)  //k is decay class (1-5)
            fTemp += mp_fClearCutLog[i][j][k-1];

      fTemp += m_fClearCutScarifiedSoil + m_fClearCutTipUp;
      if ( fTemp > 1.0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "Clear cut substrate proportions must add up to be less than 1";
        throw( stcErr );
      }

      //Partial cut mean diameters

      if (mp_fPartCutLogMeanDiameter[0] > m_fLogSizeClassBoundary)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "su_partialCutSmallLogMeanDiameter must be less than su_logSizeClassBoundary";
        throw( stcErr );
      }
      if (mp_fPartCutLogMeanDiameter[1] < m_fLogSizeClassBoundary)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "su_partialCutLargeLogMeanDiameter must be greater than su_logSizeClassBoundary";
        throw( stcErr );
      }

      //Gap cut mean diameters

      if (mp_fGapCutLogMeanDiameter[0] > m_fLogSizeClassBoundary)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "su_gapCutSmallLogMeanDiameter must be less than su_logSizeClassBoundary";
        throw( stcErr );
      }
      if (mp_fGapCutLogMeanDiameter[1] < m_fLogSizeClassBoundary)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "su_gapCutLargeLogMeanDiameter must be greater than su_logSizeClassBoundary";
        throw( stcErr );
      }

      //Clear cut mean diameters

      if (mp_fClearCutLogMeanDiameter[0] > m_fLogSizeClassBoundary)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "su_clearCutSmallLogMeanDiameter must be less than su_logSizeClassBoundary";
        throw( stcErr );
      }
      if (mp_fClearCutLogMeanDiameter[1] < m_fLogSizeClassBoundary)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "su_clearCutLargeLogMeanDiameter must be greater than su_logSizeClassBoundary";
        throw( stcErr );
      }

    }

    delete[] p_fTempValues;
    delete[] p_iTempValues;

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
    stcErr.sFunction = "clDetailedSubstrate::GetParameterFileData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// CalculateDecayProportions()
////////////////////////////////////////////////////////////////////////////

void clDetailedSubstrate::CalculateDecayProportions() {
  try
  {
    float fTemp;
    int iNumYearsPerTS = mp_oSimManager->GetNumberOfYearsPerTimestep(),
        i, j, k, t, u; //loop counters

    mp_fScarSoilDecayProp = new double[m_iMaxDecayTimesteps + 1];
    mp_fTipupDecayProp = new double[m_iMaxDecayTimesteps + 1];

    mp_fLogDecayProp = new double ****[m_iNumLogSpGroupsUsed];
    for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
      mp_fLogDecayProp[i] = new double ***[2];
      for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
        mp_fLogDecayProp[i][j] = new double **[5];
        for (k = 1; k <= 5; k++) {   //k is decay class (1-5)
          mp_fLogDecayProp[i][j][k-1] = new double *[m_iMaxDecayTimesteps];
          for (t = 0; t < m_iMaxDecayTimesteps; t++) {
            mp_fLogDecayProp[i][j][k-1][t] = new double [iNumYearsPerTS];
          }
        }
      }
    }

    //Calculate proportions for logs. Indexed by species (i), size (j), decay class (k),
    //age in timesteps (t), and years within a timestep (u)
    for (t = 0; t < m_iMaxDecayTimesteps; t++) {
      for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
        for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
          for (k = 1; k <= 5; k++) { //k is decay class (1-5)
            for (u = 0; u < iNumYearsPerTS; u++) { //iterate years within a timestep
              //Make sure we don't crash if beta = 0 when t = 0 - we know the intention
              if (0 == mp_fLogDecayBeta[i][j][k-1])
                fTemp = 1;
              else
                fTemp = pow((t * iNumYearsPerTS + u), mp_fLogDecayBeta[i][j][k-1]);
              mp_fLogDecayProp[i][j][k-1][t][u] = exp(mp_fLogDecayAlpha[i][j][k-1] * fTemp);
            } //end for i, j, k, u
          }
        }
      }

      //Calculate proportions for scarified soil and tipups.  Indexed by age in timesteps (t).
      if (0 == m_fScarifiedSoilB)
        fTemp = 1;
      else
        fTemp = pow((t * iNumYearsPerTS), m_fScarifiedSoilB);
      mp_fScarSoilDecayProp[t] = exp(m_fScarifiedSoilA * fTemp);

      if (0 == m_fTipUpB)
        fTemp = 1;
      else
        fTemp = pow((t * iNumYearsPerTS), m_fTipUpB);
      mp_fTipupDecayProp[t] = exp(m_fTipUpA * fTemp);
    } //end for t
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
    stcErr.sFunction = "clDetailedSubstrate::CalculateDecayProportions" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetupSubstrateGrids()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::SetupSubstrateGrids() {
  try
  {
    float fXCellLength = 0, fYCellLength = 0;
    const static char * p_cSizes[] = {"small","large"}; //for converting array indexes to parameter names
    int iNumLogSpGroupsUsed = 3;
    int i,j,k,t; //loop counters
    std::stringstream sLabel; //label for log codes
    int iNoLogCode; //set to -1 if any log codes are missing

    //Create the log code arrays
    mp_iLogCodes = new int **[iNumLogSpGroupsUsed];
    mp_iLogVolCodes = new int **[iNumLogSpGroupsUsed];

    mp_iLogCalcsCode = new int *** [iNumLogSpGroupsUsed];
    mp_iNewLogCalcsCode = new int ** [iNumLogSpGroupsUsed];

    for (i = 0; i < iNumLogSpGroupsUsed; i++) {
      mp_iLogCodes[i] = new int *[2];
      mp_iLogVolCodes[i] = new int *[2];
      mp_iLogCalcsCode[i] = new int ** [2];
      mp_iNewLogCalcsCode[i] = new int * [2];

      for (j = 0; j <= 1; j++) {
        mp_iLogCodes[i][j] = new int [5];
        mp_iLogVolCodes[i][j] = new int [5];
        mp_iLogCalcsCode[i][j] = new int * [5];
        mp_iNewLogCalcsCode[i][j] = new int [5];

        for (k = 1; k <= 5; k++) {
          mp_iLogCalcsCode[i][j][k-1] = new int [m_iMaxDecayTimesteps];
        } //end for k
      } //end for j
    } //end for i

    //See if the harvest results grid exists.  If it does, change the cell
    //length to match
    if ( mp_oHarvestGrid )
    {
      fXCellLength = mp_oHarvestGrid->GetLengthXCells();
      fYCellLength = mp_oHarvestGrid->GetLengthYCells();
    }

    //**************************************
    //Create the main substrate grid
    //**************************************
    //Check for its pre-existence
    mp_oSubstrateGrid = mp_oSimManager->GetGridObject( "DetailedSubstrate" );

    if ( NULL == mp_oSubstrateGrid )
    {

      m_bParFileHasSubstrateMap = false;

      //There was no map of this grid, so create it
      //If there's no harvest grid, the cell lengths will be 0 and therefore
      //the grid defaults will be used
      mp_oSubstrateGrid = mp_oSimManager->CreateGrid( "DetailedSubstrate", //grid name
          0, //number of int data members
          2+4+20*iNumLogSpGroupsUsed, //number of float data members
          0, //number of string data members
          0, //number of bool data members
          fXCellLength, //X cell length
          fYCellLength ); //Y cell length

      //Package data structure
      mp_oSubstrateGrid->ChangePackageDataStructure( 1, //number of package int data members
          2, //number of package float data members
          0, //number of package string data members
          0 ); //number of package bool data members

      //Register data members
      m_iScarSoilCode = mp_oSubstrateGrid->RegisterFloat( "scarsoil" );
      m_iFFMossCode = mp_oSubstrateGrid->RegisterFloat( "ffmoss" );
      m_iFFLitterCode = mp_oSubstrateGrid->RegisterFloat( "fflitter" );
      m_iTipupCode = mp_oSubstrateGrid->RegisterFloat( "tipup" );

      m_iPkgAgeCode = mp_oSubstrateGrid->RegisterPackageInt( "age" );
      m_iPkgScarSoilCode = mp_oSubstrateGrid->RegisterPackageFloat( "scarsoil" );
      m_iPkgTipupCode = mp_oSubstrateGrid->RegisterPackageFloat( "tipup" );

      m_iTotalLogCode = mp_oSubstrateGrid->RegisterFloat("totallog");
      m_iTotalLogVolCode = mp_oSubstrateGrid->RegisterFloat("totallogvol");

      for (i = 0; i < iNumLogSpGroupsUsed; i++) {
        for (j = 0; j <= 1; j++) {
          for (k = 1; k <= 5; k++) {
            sLabel << "loggroup" << i+1 << p_cSizes[j] << "decay" << k;
            mp_iLogCodes[i][j][k-1] = mp_oSubstrateGrid->RegisterFloat(sLabel.str());
            sLabel.str("");
            sLabel << "vloggroup" << i+1 << p_cSizes[j] << "decay%d" << k;
            mp_iLogVolCodes[i][j][k-1] = mp_oSubstrateGrid->RegisterFloat(sLabel.str());
            sLabel.str("");
          } //end for k
        } //end for j
      } //end for i
    }
    else //The substrate grid was created from a map file. Validate it.
    {

      m_bParFileHasSubstrateMap = true;

      //Make sure the cell lengths are right
      if ( ( fXCellLength > 0 && mp_oSubstrateGrid->GetLengthXCells() != fXCellLength )
          || ( fYCellLength > 0 && mp_oSubstrateGrid->GetLengthYCells() != fYCellLength ) )
      {
        //Throw an eror
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Grid map values for grid \"DetailedSubstrate\" contain cell lengths which do not match the allowed values.";
        throw( stcErr );
      }

      if (fXCellLength == 0) {
        fXCellLength = mp_oSubstrateGrid->GetLengthXCells();
      }
      if (fYCellLength == 0) {
        fYCellLength = mp_oSubstrateGrid->GetLengthYCells();
      }

      //Get the data member codes
      m_iScarSoilCode = mp_oSubstrateGrid->GetFloatDataCode( "scarsoil" );
      m_iFFMossCode = mp_oSubstrateGrid->GetFloatDataCode( "ffmoss" );
      m_iFFLitterCode = mp_oSubstrateGrid->GetFloatDataCode( "fflitter" );
      m_iTipupCode = mp_oSubstrateGrid->GetFloatDataCode( "tipup" );
      m_iTotalLogCode = mp_oSubstrateGrid->GetFloatDataCode("totallog");
      m_iTotalLogVolCode = mp_oSubstrateGrid->GetFloatDataCode("totallogvol");

      iNoLogCode=1;
      for (i = 0; i < iNumLogSpGroupsUsed; i++) {
        for (j = 0; j <= 1; j++) {
          for (k = 1; k <= 5; k++) {
            sLabel << "loggroup" << i+1 << p_cSizes[j] << "decay" << k;
            mp_iLogCodes[i][j][k-1] = mp_oSubstrateGrid->GetFloatDataCode(sLabel.str());
            sLabel.str("");
            if (mp_iLogCodes[i][j][k-1]== (-1))
              iNoLogCode = -1;
            sLabel << "vloggroup" << i+1 << p_cSizes[j] << "decay" << k;
            mp_iLogVolCodes[i][j][k-1] = mp_oSubstrateGrid->GetFloatDataCode(sLabel.str());
            sLabel.str("");
            if (mp_iLogVolCodes[i][j][k-1]== (-1))
              iNoLogCode = -1;
          }
        }
      }

      //Throw an error if any of them do not exist - we can't register them at
      //this point
      if ( -1 == m_iScarSoilCode || -1 == m_iFFMossCode || -1 == m_iFFLitterCode
          || -1 == m_iTipupCode || -1 == iNoLogCode || -1 == m_iTotalLogCode ||
          -1 == m_iTotalLogVolCode)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Grid map setup values for grid \"DetailedSubstrate\" did not contain all required grid data members.";
        throw( stcErr );
      }

      //The packages - try to find age, which would not be automatically present
      m_iPkgAgeCode = mp_oSubstrateGrid->GetPackageIntDataCode( "age" );

      if ( -1 == m_iPkgAgeCode )
      {

        //Packages were not included in the map - declare them now
        mp_oSubstrateGrid->ChangePackageDataStructure( 1, //number of package int data members
            2, //number of package float data members
            0, //number of package string data members
            0 ); //number of package bool data members

        m_iPkgAgeCode = mp_oSubstrateGrid->RegisterPackageInt( "age" );
        m_iPkgScarSoilCode = mp_oSubstrateGrid->RegisterPackageFloat( "scarsoil" );
        m_iPkgTipupCode = mp_oSubstrateGrid->RegisterPackageFloat( "tipup" );

      }
      else //age code is present
      {

        //Packages should have been in the map - get their codes
        m_iPkgScarSoilCode = mp_oSubstrateGrid->GetPackageFloatDataCode( "scarsoil" );
        m_iPkgTipupCode = mp_oSubstrateGrid->GetPackageFloatDataCode( "tipup" );

        //Throw an error if any are missing
        if ( -1 == m_iPkgScarSoilCode || -1 == m_iPkgTipupCode )
        {

          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
          stcErr.sMoreInfo = "Grid map setup values for grid \"DetailedSubstrate\" did not contain all required grid data members.";
          throw( stcErr );
        } //end if (missing codes)
      } //end else (age present)
    } //end else (substrate map present)

    //**************************************
    //Create the calculations grid
    //**************************************
    //Check for its pre-existence
    mp_oCalcGrid = mp_oSimManager->GetGridObject( "detailedsubstratecalcs" );

    if ( NULL == mp_oCalcGrid )
    {

      mp_oCalcGrid = mp_oSimManager->CreateGrid( "detailedsubstratecalcs",
          0, //number of ints
          1 + (10*iNumLogSpGroupsUsed * (m_iMaxDecayTimesteps+1)), //number of floats
          0, //number of chars
          0, //number of bools
          fXCellLength, //X cell length - must match the grid above!
          fYCellLength ); //Y cell length - must match the grid above!

      //Register data members
      m_iNewTipupCalcsCode = mp_oCalcGrid->RegisterFloat( "newtipup" );

      for (i = 0; i < iNumLogSpGroupsUsed; i++) {
        for (j = 0; j <= 1; j++) {
          for (k = 1; k <= 5; k++) {
            sLabel << "newloggroup" << i+1 << p_cSizes[j] << "decay" << k;
            mp_iNewLogCalcsCode[i][j][k-1] = mp_oCalcGrid->RegisterFloat(sLabel.str());
            sLabel.str("");

            for ( t = 0; t < m_iMaxDecayTimesteps; t++ ) {
              sLabel << "loggroup" << i+1 << p_cSizes[j] << "decay" << k << "_" << t;
              mp_iLogCalcsCode[i][j][k-1][t] = mp_oCalcGrid->RegisterFloat(sLabel.str());
              sLabel.str("");
            } //end for t (decay timesteps)
          } //end for k (decay class)
        } //end for j (size class)
      } //end for i (species group)
    } //end if


    else //Calc grid was created.
    {
      //It was already created - make sure it was set up right
      //Make sure the cell lengths are right
      if ( (mp_oCalcGrid->GetLengthXCells() != mp_oSubstrateGrid->GetLengthXCells())
          || ( mp_oCalcGrid->GetLengthYCells() != mp_oSubstrateGrid->GetLengthYCells()) )
      {
        //Throw an eror
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Grid map values for grid \"detailedsubstratecalcs\" contain cell lengths which do not match the allowed values.";
        throw( stcErr );
      }

      //Get the data member codes
      m_iNewTipupCalcsCode = mp_oCalcGrid->GetFloatDataCode( "newtipup" );
      if (-1 == m_iNewTipupCalcsCode) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Grid map setup values for grid \"substratecalcs\" did not contain the \"newtipup\" required grid data member.";
        throw( stcErr );
      }

      //mp_iLogCalcsCode = new int *** [iNumLogSpGroupsUsed];
      //mp_iNewLogCalcsCode = new int ** [iNumLogSpGroupsUsed];

      for (i = 0; i < iNumLogSpGroupsUsed; i++) {
        //mp_iLogCalcsCode[i] = new int ** [2];

        //mp_iNewLogCalcsCode[i] = new int * [2];

        for (j = 0; j <= 1; j++) {
          //mp_iLogCalcsCode[i][j] = new int * [5];
          //mp_iNewLogCalcsCode[i][j] = new int [5];

          for (k = 1; k <= 5; k++) {
            //mp_iLogCalcsCode[i][j][k-1] = new int [m_iMaxDecayTimesteps];
            sLabel << "newloggroup" << i+1 << p_cSizes[j] << "decay" << k;
            mp_iNewLogCalcsCode[i][j][k-1] = mp_oCalcGrid->GetFloatDataCode(sLabel.str());
            sLabel.str("");

            if (-1 == mp_iNewLogCalcsCode[i][j][k-1]) {
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
              stcErr.sMoreInfo = "Grid map setup values for grid \"detailedsubstratecalcs\" did not contain all \"newloggroup\" required grid data members.";
              throw( stcErr );
            }

            for ( t = 0; t < m_iMaxDecayTimesteps; t++ ) {
              sLabel << "loggroup" << i+1 << p_cSizes[j] << "decay" << k << "_" << t;
              mp_iLogCalcsCode[i][j][k-1][t] = mp_oCalcGrid->GetFloatDataCode(sLabel.str());
              sLabel.str("");

              if (-1 == mp_iLogCalcsCode[i][j][k-1][t]) {
                modelErr stcErr;
                stcErr.iErrorCode = BAD_DATA;
                stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
                stcErr.sMoreInfo = "Grid map setup values for grid \"detailedsubstratecalcs\" did not contain all \"loggroup\" required grid data members.";
                throw( stcErr );
              }
            } //end for t (decay timesteps)
          } //end for k (decay class)
        } //end for j (size class)
      } //end for i (species group)
    } //end else (calc grid present)

    //Get the area and the reciprocal of the area of a grid cell
    m_fGridCellArea = mp_oSubstrateGrid->GetLengthXCells() * mp_oSubstrateGrid->GetLengthYCells();

    //**************************************
    //If there's a harvest grid, get the data code for harvest type
    //**************************************
    if ( mp_oHarvestGrid )
    {
      m_iHarvestTypeCode = mp_oHarvestGrid->GetIntDataCode( "Harvest Type" );
      //If the code is -1 throw an error
      if ( m_iHarvestTypeCode == -1 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Harvest grid set up in unexpected way.";
        throw( stcErr );
      }
    }

    else //no harvest grid
      m_iHarvestTypeCode = -1;

    //**************************************
    //Create the original substrate grid
    //**************************************

    mp_oOriginalSubstrateGrid = mp_oSimManager->CreateGrid( "Substrate", //grid name
        0, //number of int data members
        6, //number of float data members
        0, //number of string data members
        0, //number of bool data members
        fXCellLength, //X cell length
        fYCellLength ); //Y cell length      

    //Register data members
    m_iOrigScarSoilCode = mp_oOriginalSubstrateGrid->RegisterFloat( "scarsoil" );
    m_iOrigFFMossCode = mp_oOriginalSubstrateGrid->RegisterFloat( "ffmoss" );
    m_iOrigFFLitterCode = mp_oOriginalSubstrateGrid->RegisterFloat( "fflitter" );
    m_iOrigTipupCode = mp_oOriginalSubstrateGrid->RegisterFloat( "tipup" );
    m_iOrigFreshLogCode = mp_oOriginalSubstrateGrid->RegisterFloat( "freshlog" );
    m_iOrigDecLogCode = mp_oOriginalSubstrateGrid->RegisterFloat( "declog" );

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
    stcErr.sFunction = "clDetailedSubstrate::SetupSubstrateGrid" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetInitialSubstrate()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::SetInitialSubstrate() {
  try
  {
    clPackage * p_oPackage; //for working with substrate grid packages
    int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(),
        iNumYCells = mp_oSubstrateGrid->GetNumberYCells();
    int iX, iY, i, j, k; //loop counters
    float fInitScarSoil, //initial amounts, either from param file or map
    fInitTipup,
    p_fInitLog[m_iNumLogSpGroupsUsed][2][5],
    fForestFloor, //moss + litter
    fTemp,//temp holder
    fAreaTotal, fVolTotal; //totals to be written to substrate grid

    fInitScarSoil = m_fInitScarifiedSoil;
    fInitTipup = m_fInitTipUp;

    //find remainder to be assigned to moss and litter
    fForestFloor = 1.0 - fInitScarSoil - fInitTipup;

    for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
      for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
        for (k = 1; k <= 5; k++) {   //k is decay class (1-5)
          p_fInitLog[i][j][k-1] = mp_fInitLog[i][j][k-1];
          fForestFloor -= p_fInitLog[i][j][k-1];
        }
      }
    }

    for ( iX = 0; iX < iNumXCells; iX++ ) {
      for ( iY = 0; iY < iNumYCells; iY++ ) {
        if ( m_bParFileHasSubstrateMap ) {

          //there's a substrate map, so just set the variables to write to calc grid and packages
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iScarSoilCode, &fInitScarSoil);
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iTipupCode, &fInitTipup);

          for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
            for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
              for (k = 1; k <= 5; k++)    //k is decay class (1-5)
                mp_oSubstrateGrid->GetValueOfCell(iX, iY, mp_iLogCodes[i][j][k-1], &(p_fInitLog[i][j][k-1]));

        } else {

          //no substrate map, so write the initial values to the substrate grid
          mp_oSubstrateGrid->SetValueOfCell(iX, iY, m_iScarSoilCode, fInitScarSoil);
          mp_oSubstrateGrid->SetValueOfCell(iX, iY, m_iTipupCode, fInitTipup);

          for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
            for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
              for (k = 1; k <= 5; k++) {    //k is decay class (1-5)
                mp_oSubstrateGrid->SetValueOfCell(iX, iY, mp_iLogCodes[i][j][k-1], p_fInitLog[i][j][k-1]);
                //Calc vol equivalent
                fTemp = (1.0/3.0) * p_fInitLog[i][j][k-1] * mp_fInitialLogMeanDiameter[j]/2.0 * M_PI * 10000.0 * 0.01;
                mp_oSubstrateGrid->SetValueOfCell(iX, iY, mp_iLogVolCodes[i][j][k-1], fTemp);
              }
            }
          }

          mp_oSubstrateGrid->SetValueOfCell(iX, iY, m_iFFMossCode, (float)(fForestFloor * m_fMossProportion));
          mp_oSubstrateGrid->SetValueOfCell(iX, iY, m_iFFLitterCode, (float)(fForestFloor * (1 - m_fMossProportion)));

        } //end else (no map)

        //Create new package and set proportions for tipups and scarsoil to initial conditions
        p_oPackage = mp_oSubstrateGrid->CreatePackageOfCell( iX, iY );
        p_oPackage->SetValue( m_iPkgScarSoilCode, fInitScarSoil);
        p_oPackage->SetValue( m_iPkgTipupCode, fInitTipup );
        p_oPackage->SetValue( m_iPkgAgeCode, 0 );

        fAreaTotal=0.0;
        fVolTotal=0.0;

        for (i = 0; i < m_iNumLogSpGroupsUsed; i++) { //i is SpGroup (0-2)
          for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)
            for (k = 1; k <= 5; k++) {   //k is decay class (1-5)

              //Add the amount of logs
              mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][0], p_fInitLog[i][j][k-1] );

              //Calculate total log cover and volume
              mp_oSubstrateGrid->GetValueOfCell( iX, iY, mp_iLogCodes[i][j][k-1], & fTemp );
              fAreaTotal += fTemp;
              mp_oSubstrateGrid->GetValueOfCell( iX, iY, mp_iLogVolCodes[i][j][k-1], & fTemp );
              fVolTotal += fTemp;
            }
          }
        }

        //write totals to substrate grid
        mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iTotalLogCode, fAreaTotal );
        mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iTotalLogVolCode, fVolTotal );

      } // end for Y cells
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
    stcErr.sFunction = "clDetailedSubstrate::SetInitialSubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetCodes()
////////////////////////////////////////////////////////////////////////////

void clDetailedSubstrate::GetCodes() {
  try
  {
    clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;
    std::string sDeadLabel = "dead", //labels used to tell what type of dead tree we have.
        sFallLabel = "Fall",
        sNewBreakLabel = "NewBreakHeight",
        sOldBreakLabel = "SnagOldBreakHeight";

    int iTotalTypes = p_oPop->GetNumberOfTypes();
    int i, j; //loop counters

    //Declare and initialize the return codes and substrate applies arrays
    mp_iFallCodes = new int [m_iNumTotalSpecies];  //adults only
    mp_iOldBreakCodes = new int [m_iNumTotalSpecies]; //snag only

    mp_iNewBreakCodes = new int * [m_iNumTotalSpecies]; //adults and snags
    mp_iDeadCodes = new int * [m_iNumTotalSpecies]; //everything
    mp_bSubstrateApplies = new bool * [m_iNumTotalSpecies]; //everything
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      mp_iFallCodes[i] = -1;
      mp_iOldBreakCodes[i] = -1;

      mp_iDeadCodes[i] = new int[iTotalTypes];
      mp_iNewBreakCodes[i] = new int[iTotalTypes];
      mp_bSubstrateApplies[i] = new bool[iTotalTypes];

      for ( j = 0; j < iTotalTypes; j++ )
      {
        mp_iDeadCodes[i][j] = -1;
        mp_iNewBreakCodes[i][j] = -1;
        mp_bSubstrateApplies[i] [j] = false;
      }
    }

    //Now go through the species/type combos and get the code for each
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ ) {

      //If any type is seedling or sapling, throw an error
      if ( mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::seedling )
      {
        modelErr stcErr;
        stcErr.sFunction = "clDetailedSubstrate::GetCodes" ;
        stcErr.sMoreInfo = "Detailed substrate cannot be applied to seedlings.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      mp_iFallCodes[mp_whatSpeciesTypeCombos[i].iSpecies] = p_oPop->GetBoolDataCode( sFallLabel, mp_whatSpeciesTypeCombos[i].iSpecies, clTreePopulation::adult );

      mp_iOldBreakCodes[mp_whatSpeciesTypeCombos[i].iSpecies] = p_oPop->GetFloatDataCode( sOldBreakLabel, mp_whatSpeciesTypeCombos[i].iSpecies, clTreePopulation::snag );

      mp_iNewBreakCodes[mp_whatSpeciesTypeCombos[i].iSpecies][mp_whatSpeciesTypeCombos[i].iType] =
          p_oPop->GetFloatDataCode( sNewBreakLabel, mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

      mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] =
          p_oPop->GetIntDataCode( sDeadLabel, mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

      mp_bSubstrateApplies[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] = true;

      //If the return code is -1, throw an error
      if ( -1 == mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] )
      {
        modelErr stcErr;
        stcErr.sFunction = "clDetailedSubstrate::GetCodes" ;
        std::stringstream s;
        s << "Type/species combo species="
            << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
            << mp_whatSpeciesTypeCombos[i].iType
            << " must have a 'dead' data member assigned by a mortality or snag dynamics behavior be compatible with substrate.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //If any fall and break codes (set by snag dynamics) are -1, throw an error
      if ((-1 == mp_iFallCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                               && mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::adult) ||
          (-1 == mp_iOldBreakCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                                   && mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::snag) ||
                                   (-1 == mp_iNewBreakCodes[mp_whatSpeciesTypeCombos[i].iSpecies][mp_whatSpeciesTypeCombos[i].iType]
                                                                                                  && ((mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::snag)
                                                                                                      || (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::adult)) ) )
      {
        modelErr stcErr;
        stcErr.sFunction = "clDetailedSubstrate::GetCodes" ;
        std::stringstream s;
        s << "Type/species combo species = "
            << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
            << mp_whatSpeciesTypeCombos[i].iType
            << " must have a snag dynamics behavior assigned to be compatible with substrate.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

    } //end for each combo

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
    stcErr.sFunction = "clDetailedSubstrate::GetCodes" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::Action() {
  try
  {

    //Add new substrate due to harvest
    AddHarvestSubstrate();

    //Calculate main grid totals
    CalcMainGridTotals();

    //Decay existing substrate
    DecaySubstrate();

    //Calculate post-decay volume amounts 
    CalcVolumeAfterDecay();

    //Add new substrate due to mortality
    MortalitySubstrate();

    //Calculate main grid totals again
    CalcMainGridTotals();

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
    stcErr.sFunction = "clDetailedSubstrate::Action" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// AddHarvestSubstrate()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::AddHarvestSubstrate() {
  try
  {

    clPackage * p_oPackage, * p_oNextToLastPackage; //for working with substrate grid packages

    float fNewTipup, //amount of new tipup for harvest
    fNewScarSoil, //amount of new scarified soil for harvest
    p_fNewLog[m_iNumLogSpGroupsUsed][2][5], //amount of new log added by harvest
    fTemp, //temp holder
    fHarvestSubstrateAdded, //total new substrate added by harvest
    p_fNewLogMeanDiameter[2]; //mean diameter of new logs added by harvest, by size class;
    int iCutType; //the harvest cut type that occurred in a grid cell
    int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(),
        iNumYCells = mp_oSubstrateGrid->GetNumberYCells();
    int iX, iY, i, j, k, t; //loop counters
    int iAge; //holder for package age
    bool bSubstrateAdded; //was any substrate added to this cell before harvesting (mortality or initial conditions)?

    //If there's no harvest, skip out
    if ( !mp_oHarvestGrid )
      return;

    for ( iX = 0; iX < iNumXCells; iX++ )
      for ( iY = 0; iY < iNumYCells; iY++ ) {

        //Was there a harvest in this cell?
        mp_oHarvestGrid->GetValueOfCell( iX, iY, m_iHarvestTypeCode, & iCutType );

        //If there was no cut, skip this cell
        if ( -1 == iCutType )
          goto nextCell;

        if (iCutType == clDisturbance::partial) { //partial cut
          fNewScarSoil = m_fPartCutScarifiedSoil;
          fNewTipup = m_fPartCutTipUp;
          p_fNewLogMeanDiameter[0] = mp_fPartCutLogMeanDiameter[0];
          p_fNewLogMeanDiameter[1] = mp_fPartCutLogMeanDiameter[1];
        }
        else if (iCutType == clDisturbance::gap) { //gap cut
          fNewScarSoil = m_fGapCutScarifiedSoil;
          fNewTipup = m_fGapCutTipUp;
          p_fNewLogMeanDiameter[0] = mp_fGapCutLogMeanDiameter[0];
          p_fNewLogMeanDiameter[1] = mp_fGapCutLogMeanDiameter[1];
        }
        else {//clearcut
          fNewScarSoil = m_fClearCutScarifiedSoil;
          fNewTipup = m_fClearCutTipUp;
          p_fNewLogMeanDiameter[0] = mp_fClearCutLogMeanDiameter[0];
          p_fNewLogMeanDiameter[1] = mp_fClearCutLogMeanDiameter[1];
        }

        fHarvestSubstrateAdded = fNewScarSoil + fNewTipup;

        for (i = 0; i < m_iNumLogSpGroupsUsed; i++) {//i is SpGroup (0-2)
          for (j = 0; j <= 1; j++) {//j is index of size class (small=0, large=1)
            for (k = 1; k <= 5; k++) {  //k is decay class (1-5)
              if (iCutType == clDisturbance::partial)
                p_fNewLog[i][j][k-1] = mp_fPartCutLog[i][j][k-1];
              else if (iCutType == clDisturbance::gap)
                p_fNewLog[i][j][k-1] = mp_fGapCutLog[i][j][k-1];
              else //clearcut
                p_fNewLog[i][j][k-1] = mp_fClearCutLog[i][j][k-1];

              fHarvestSubstrateAdded += p_fNewLog[i][j][k-1];
            }
          }
        }

        //Re-scale all existing packages
        bSubstrateAdded = false;

        p_oPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );
        while ( p_oPackage )
        {
          p_oPackage->GetValue( m_iPkgScarSoilCode, & fTemp );
          p_oPackage->SetValue( m_iPkgScarSoilCode, fTemp * (1-fHarvestSubstrateAdded) );

          p_oPackage->GetValue( m_iPkgTipupCode, & fTemp );
          p_oPackage->SetValue( m_iPkgTipupCode, fTemp * (1-fHarvestSubstrateAdded) );

          //If the package has an age of 0 (created this timestep), it's either
          //initial conditions or mortality from end of last timestep.  In this case,
          //add the new substrates to this package
          p_oPackage->GetValue( m_iPkgAgeCode, & iAge );

          if ( iAge == 0 ) {

            p_oPackage->GetValue( m_iPkgTipupCode, & fTemp );
            fTemp += fNewTipup;
            p_oPackage->SetValue( m_iPkgTipupCode, fTemp );

            p_oPackage->GetValue( m_iPkgScarSoilCode, & fTemp );
            fTemp += fNewScarSoil;
            p_oPackage->SetValue( m_iPkgScarSoilCode, fTemp );

            bSubstrateAdded = true;

          } //end if

          p_oPackage = p_oPackage->GetNextPackage();
        } //end while

        //If there was no package already created for this timestep, create one
        //now and put it last
        if ( !bSubstrateAdded ) {

          p_oNextToLastPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );
          if (!p_oNextToLastPackage) {
            //there aren't any packages already, so the new one goes first
            p_oPackage = mp_oSubstrateGrid->CreatePackageOfCell( iX, iY );
          } else {
            while ( p_oNextToLastPackage ) {
              p_oPackage = p_oNextToLastPackage->GetNextPackage();
              if ( !p_oPackage )
              {
                //we're at the last package, so add the new one after it
                p_oPackage = mp_oSubstrateGrid->CreatePackage( p_oNextToLastPackage );
                //Break the loop
                p_oNextToLastPackage = NULL;
              } else
                p_oNextToLastPackage = p_oPackage;
            }
          }

          //set the values for the new package
          p_oPackage->SetValue( m_iPkgAgeCode, ( int )0 );
          p_oPackage->SetValue( m_iPkgTipupCode, fNewTipup );
          p_oPackage->SetValue( m_iPkgScarSoilCode, fNewScarSoil );

        } //end if (!bSubstrateAdded)

        //Re-scale existing log data
        for ( t = 0; t < m_iMaxDecayTimesteps; t++ )
          for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
            for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
              for (k = 1; k <= 5; k++) { //k is decay class (1-5)
                mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][t], &fTemp );
                mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][t], fTemp * (1-fHarvestSubstrateAdded) );
              }

        //Add logs from harvest to calc grid, and volume to substrate grid
        for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
          for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
            for (k = 1; k <= 5; k++) { //k is decay class (1-5)
              //area
              mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][0], &fTemp);
              fTemp += p_fNewLog[i][j][k-1];
              mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][0], fTemp);

              //volume
              mp_oSubstrateGrid->GetValueOfCell( iX, iY, mp_iLogVolCodes[i][j][k-1], &fTemp);
              //re-scale old logs by amount of scarified soil
              fTemp *= (1.0 - fNewScarSoil);
              //calculate and add new volume
              fTemp += (1.0/3.0) * p_fNewLog[i][j][k-1] * p_fNewLogMeanDiameter[j]/2.0 * M_PI * 10000.0 * 0.01;
              mp_oSubstrateGrid->SetValueOfCell( iX, iY, mp_iLogVolCodes[i][j][k-1], fTemp);
            }

        nextCell:
        ;
      } //end for (iY)
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
    stcErr.sFunction = "clDetailedSubstrate::AddHarvestSubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// CalcMainGridTotals()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::CalcMainGridTotals() {

  try
  {
    clPackage * p_oPackage, * p_oNextPackage;
    float fTotalSubstrate, fScarSoil, fTipup, fLog, fMoss, fLitter; //should be self-evident by now
    int i, j, k, iX, iY, t; //loop counters
    int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(),
        iNumYCells = mp_oSubstrateGrid->GetNumberYCells();
    float fTemp, fTotalLogArea, fTotalVolume, fDecay123LogArea, fDecay45LogArea; //holders for grid values

    for (iX = 0; iX < iNumXCells; iX++)
      for (iY = 0; iY < iNumYCells; iY++) {

        // Forest floor and error correction
        fScarSoil = 0.0;
        fTipup = 0.0;
        p_oPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );

        // Add up total tipup and scarified soil
        while ( p_oPackage ) {

          p_oNextPackage = p_oPackage->GetNextPackage();

          //Scarified soil
          p_oPackage->GetValue( m_iPkgScarSoilCode, & fTemp );
          fScarSoil += fTemp;
          //Tipup
          p_oPackage->GetValue( m_iPkgTipupCode, & fTemp);
          fTipup += fTemp;

          p_oPackage = p_oNextPackage;
        } //end of while (p_oPackage)

        //Assign the package totals back to the main substrate
        mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iScarSoilCode, fScarSoil );
        mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iTipupCode, fTipup );

        fTotalLogArea=0.0;

        for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
          for (j = 0; j <= 1; j++)  //j is index of size class (small=0, large=1)
            for (k = 1; k <= 5; k++) {  //k is decay class (1-5)
              fLog=0.0;
              for (t = 0; t < m_iMaxDecayTimesteps; t++) {
                mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][t], &fTemp);
                fLog += fTemp;
              }

              //write sum of each age bin to substrate grid
              mp_oSubstrateGrid->SetValueOfCell( iX, iY, mp_iLogCodes[i][j][k-1], fLog);
              //and keep summing up total substrate
              fTotalLogArea += fLog;
            }//end for i,j,k

        fTotalSubstrate = fScarSoil + fTipup + fTotalLogArea;

        //Check that the proportions are less than 1
        if ( fTotalSubstrate <= 1 ) {

          //Calculate the forest floor litter and moss values as the remainder
          //needed to make the total substrate proportions add up to 1 and
          //assign

          fMoss = (1 - fTotalSubstrate) * m_fMossProportion;
          fLitter = (1 - fTotalSubstrate) * (1 - m_fMossProportion);

          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFFMossCode, fMoss );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFFLitterCode, fLitter );

        } else {

          //The proportions add up to more than 1 - re-proportion them and
          //re-assign to the grid cell.  This isn't expected to happen, but just in case....

          fScarSoil /= fTotalSubstrate;
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iScarSoilCode, fScarSoil );

          fTipup /= fTotalSubstrate;
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iTipupCode, fTipup );

          for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
            for (j = 0; j <= 1; j++)  //j is index of size class (small=0, large=1)
              for (k = 1; k <= 5; k++) {  //k is decay class (1-5)

                mp_oSubstrateGrid->GetValueOfCell( iX, iY, mp_iLogCodes[i][j][k-1], & fTemp );
                fTemp /= fTotalSubstrate;
                mp_oSubstrateGrid->SetValueOfCell( iX, iY, mp_iLogCodes[i][j][k-1], fTemp );

              }

          fMoss = 0.0;
          fLitter = 0.0;

          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFFMossCode, fMoss );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFFLitterCode, fLitter );

        } //end else

        //Calculate and assign the total area and volume of logs to the substrate grid

        fDecay123LogArea=0.0;
        fDecay45LogArea=0.0;
        fTotalVolume=0.0;

        for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
          for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
            for (k = 1; k <= 3; k++) {   //k is decay class (1-5)

              mp_oSubstrateGrid->GetValueOfCell( iX, iY, mp_iLogCodes[i][j][k-1], & fTemp );
              fDecay123LogArea += fTemp;

              mp_oSubstrateGrid->GetValueOfCell( iX, iY, mp_iLogVolCodes[i][j][k-1], & fTemp );
              fTotalVolume += fTemp;
            }

        for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
          for (j = 0; j <= 1; j++) //j is index of size class (small=0, large=1)
            for (k = 4; k <= 5; k++) {   //k is decay class (1-5)

              mp_oSubstrateGrid->GetValueOfCell( iX, iY, mp_iLogCodes[i][j][k-1], & fTemp );
              fDecay45LogArea += fTemp;

              mp_oSubstrateGrid->GetValueOfCell( iX, iY, mp_iLogVolCodes[i][j][k-1], & fTemp );
              fTotalVolume += fTemp;
            }

        fTotalLogArea = fDecay123LogArea + fDecay45LogArea;
        mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iTotalLogCode, fTotalLogArea );
        mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iTotalLogVolCode, fTotalVolume );

        //Assign all the values to the original substrate grid for compatibility with behaviors that use it
        mp_oOriginalSubstrateGrid->SetValueOfCell( iX, iY, m_iOrigScarSoilCode, fScarSoil );
        mp_oOriginalSubstrateGrid->SetValueOfCell( iX, iY, m_iOrigTipupCode, fTipup );
        mp_oOriginalSubstrateGrid->SetValueOfCell( iX, iY, m_iOrigFreshLogCode, fDecay123LogArea );
        mp_oOriginalSubstrateGrid->SetValueOfCell( iX, iY, m_iOrigDecLogCode, fDecay45LogArea );
        mp_oOriginalSubstrateGrid->SetValueOfCell( iX, iY, m_iOrigFFMossCode, fMoss );
        mp_oOriginalSubstrateGrid->SetValueOfCell( iX, iY, m_iOrigFFLitterCode, fLitter );

      } //end for (iY)
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
    stcErr.sFunction = "clDetailedSubstrate::CalcMainGridTotals" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// DecaySubstrate()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::DecaySubstrate() {
  try
  {
    clPackage * p_oPackage, //for working with packages
    * p_oNextPackage; //for working with packages

    int iNumYearsPerTS = (int)(mp_oSimManager->GetNumberOfYearsPerTimestep());

    //array for the area of logs that decayed this timestep by decay class 
    //and # of years since reaching that decay class
    float p_fNewAmountHolder[5][iNumYearsPerTS];

    //array for the initial (age 0, before any decay) area of logs by decay class 
    //and # of timesteps since reaching that decay class     
    float p_fInitialAmountHolder[5][m_iMaxDecayTimesteps];

    float fTemp; //for getting/setting grid values
    int iAge, //package age
    iNumXCells = mp_oSubstrateGrid->GetNumberXCells(),
    iNumYCells = mp_oSubstrateGrid->GetNumberYCells(),
    iX, iY, i, j, k, t, u, v; //various loop counters

    //loop through each grid cell
    for ( iX = 0; iX < iNumXCells; iX++ )
      for ( iY = 0; iY < iNumYCells; iY++ )
      {

        //***************************************
        // Log decay
        //***************************************

        //loop through the species and size classes.  All decay is repeated the same way for each
        for (i = 0; i < m_iNumLogSpGroupsUsed; i++) //i is SpGroup (0-2)
          for (j = 0; j <= 1; j++) { //j is index of size class (small=0, large=1)

            //zero out the new amounts array
            for (u=0; u<iNumYearsPerTS; u++)
              for (k=1;k<=5;k++)
                p_fNewAmountHolder[k-1][u]=0.0;

            //calculate the initial amounts (age 0, before any decay) using the calculated decay proportions
            for (t=0; t < m_iMaxDecayTimesteps; t++) //t is timestep number
              for (k=1;k<=5;k++) { //k is decay class (1-5)
                if (mp_fLogDecayProp[i][j][k-1][t][0] < 0.00001) { //avoid dividing by zero
                  p_fInitialAmountHolder[k-1][t] = 0.0;
                } else {
                  mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][t], & fTemp );
                  p_fInitialAmountHolder[k-1][t]= fTemp / mp_fLogDecayProp[i][j][k-1][t][0];
                }
              } //end for t, k

            //We'll be iterating the decay on an annual basis
            //so that logs can move through >1 decay class in a timestep.
            //This loop does the annual decay iterations.
            for (u=0; u<iNumYearsPerTS; u++) { //iterate years per timestep times

              //If it's not the first iteration, start by running the decay
              //for logs that have already advanced during this timestep.
              if (u!=0)
                for (k=5; k>=2; k--) { //for each decay class...
                  for (v=u-1; v>=0; v--) { //and each of the iteration years so far...

                    //note that these loops go backwards so we can advance the higher decay classes
                    //and years before overwriting their positions in the array

                    //calc the amount remaining and add it to the following year's bin
                    p_fNewAmountHolder[k-1][v+1] = p_fNewAmountHolder[k-1][v] * mp_fLogDecayProp[i][j][k-1][0][v+1];

                    //calc the amount that left and add it to year 0 bin for the next decay class
                    if (k<5)
                      p_fNewAmountHolder[k][0] += p_fNewAmountHolder[k-1][v] * (1 - mp_fLogDecayProp[i][j][k-1][0][v+1]);

                  } //end for v

                  //reset year 0 amounts
                  p_fNewAmountHolder[k-1][0] = 0;

                } //end if u!=0, for k

              //Now run the decay for log's that haven't yet advanced a decay class
              //since the start of this timestep, but are doing so in this annual iteration.

              //for each age bin
              for (t=0; t < m_iMaxDecayTimesteps; t++) { //t is timestep number
                for (k=4;k>=1;k--) { //k is decay class (1-4, 5 just leaves)

                  //and add the amount that left to year 0 bin of the next decay class
                  if (u < (iNumYearsPerTS-1)) //amount remaining is in next year within same timestep
                    p_fNewAmountHolder[k][0] += p_fInitialAmountHolder[k-1][t] * (mp_fLogDecayProp[i][j][k-1][t][u] - mp_fLogDecayProp[i][j][k-1][t][u+1]);
                  else if (t < m_iMaxDecayTimesteps - 1) //amount remaining is in first year of next timestep
                    p_fNewAmountHolder[k][0] += p_fInitialAmountHolder[k-1][t] * (mp_fLogDecayProp[i][j][k-1][t][u] - mp_fLogDecayProp[i][j][k-1][t+1][0]);
                  else //exceeded max decay timesteps; amount remaining is 0 and we dump everything that's left
                    p_fNewAmountHolder[k][0] += p_fInitialAmountHolder[k-1][t] * mp_fLogDecayProp[i][j][k-1][t][u];

                } //end for k
              } //end for t
            } //end for u, we're done the annual decay iterations

            //Now we've finished calculating all the annual the decay amounts
            //and are ready to assign values back to the calcs grid.

            for (k=1; k<=5; k++) { //k is decay class

              //Sum (over each year of the last timestep) the amounts that advanced
              //1 or more decay classes, and write this to either age 0 or 1 of the
              //calcs grid (rounded from year of transition).
              if (k>1) { //nothing advanced to decay class 1

                //if they advanced less than 1/2 a timestep ago, set age to 0
                fTemp=0.0;
                for (u=0; u<(iNumYearsPerTS/2); u++)
                  fTemp += p_fNewAmountHolder[k-1][u];

                if (iNumYearsPerTS % 2 == 0) //even number of years per timestep, divide middle year in half
                  fTemp += 0.5 * p_fNewAmountHolder[k-1][iNumYearsPerTS/2];
                else //odd number of years per timestep, the middle year all goes to age 0
                  fTemp += p_fNewAmountHolder[k-1][iNumYearsPerTS/2];

                mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][0], fTemp);

                //if they advanced more than 1/2 a timestep ago, set age to 1
                fTemp=0.0;
                if (iNumYearsPerTS % 2 == 0) //even number of years per timestep, divide middle year in half
                  fTemp += 0.5 * p_fNewAmountHolder[k-1][iNumYearsPerTS/2];

                for (u=(iNumYearsPerTS/2 + 1); u<iNumYearsPerTS; u++)
                  fTemp += p_fNewAmountHolder[k-1][u];

                mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][1], fTemp);

              } //end if (k>1)

              //Set the value for existing pieces that advanced to decay class 1
              //over the timestep to zero.
              else { //k==1

                fTemp=0.0;
                mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][0][0], fTemp);

              } //end else

              //Lastly, set the amounts for pieces that didn't advance
              //over the last timestep using the calculated decay proportions.
              //If they're already at max decay timesteps, they get dropped.

              for (t=0; t<m_iMaxDecayTimesteps-1; t++) {
                if (t==0 && k>1)
                  //some pieces that made the transition this timestep are already at age 1,
                  //so add them in too
                  mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][1], &fTemp);
                else
                  fTemp = 0.0;

                fTemp += p_fInitialAmountHolder[k-1][t] * mp_fLogDecayProp[i][j][k-1][t+1][0];
                mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][t+1], fTemp);

              } //end for t
            } //end for k
          } //end for i, j (species and size classes)

        //***********************************************
        // Decay packages for tipup and scarified soil
        //***********************************************
        p_oPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );
        if ( p_oPackage )
        {
          while ( p_oPackage )
          {
            p_oNextPackage = p_oPackage->GetNextPackage();
            p_oPackage->GetValue( m_iPkgAgeCode, & iAge );

            //Increment the package's age
            iAge++;

            if ( iAge >= m_iMaxDecayTimesteps ) {

              //If the package has reached the max decay age, delete it
              mp_oSubstrateGrid->DeletePackage( p_oPackage );

            } else {

              //Otherwise, set new age and decay scarified soil and tipup amounts
              p_oPackage->SetValue( m_iPkgAgeCode, iAge );

              //Scarified soil
              p_oPackage->GetValue( m_iPkgScarSoilCode, & fTemp );
              fTemp *= mp_fScarSoilDecayProp[iAge]/mp_fScarSoilDecayProp[iAge-1];
              p_oPackage->SetValue(m_iPkgScarSoilCode, fTemp);

              //Tipup
              p_oPackage->GetValue( m_iPkgTipupCode, & fTemp );
              fTemp *= mp_fTipupDecayProp[iAge]/mp_fTipupDecayProp[iAge-1];
              p_oPackage->SetValue(m_iPkgTipupCode, fTemp);

            } //end else (not too old yet)

            p_oPackage = p_oNextPackage;
          } //end while (p_oPackage)
        } //end if (p_oPackage)
      } //end for (iY)
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
    stcErr.sFunction = "clDetailedSubstrate::DecaySubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// CalcVolumeAfterDecay()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::CalcVolumeAfterDecay() {
  try
  {
    int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(),
        iNumYCells = mp_oSubstrateGrid->GetNumberYCells();
    int iX, iY, i, j, k, t; //loop counters
    float fOldVolAllDecayClasses, //the pre-decay volume of logs in all decay classes
    fOldAreaAllDecayClasses, //the pre-decay area of logs in all decay classes
    fNewAreaByDecayClass, //the post-decay area of logs in one decay class
    fNewVolByDecayClass, //the post-decay volume of logs in one decay class
    fTemp; //temp holder

    //Log volume is not tracked by age so the DecaySubstrate() method won't work for them.
    //Instead, we take a shortcut by scaling the change in log volume each timestep
    //to the change in area by decay class.
    for (iX=0; iX<iNumXCells; iX++)
      for (iY=0; iY<iNumYCells; iY++) {
        for (i=0;i<m_iNumLogSpGroupsUsed;i++) //i is SpGroup (0-2)
          for (j=0;j<=1;j++) {  //j is index of size class (small=0, large=1)

            //First compute the old (pre-decay) area and volume of logs from the substrate grid.
            fOldVolAllDecayClasses=0.0;
            fOldAreaAllDecayClasses=0.0;

            for (k=1;k<=5;k++) { //k is decay class (1-5)

              mp_oSubstrateGrid->GetValueOfCell(iX, iY, mp_iLogVolCodes[i][j][k-1], &fTemp);
              fOldVolAllDecayClasses += fTemp;

              mp_oSubstrateGrid->GetValueOfCell(iX, iY, mp_iLogCodes[i][j][k-1], &fTemp);
              fOldAreaAllDecayClasses += fTemp;

            } // end for k

            for (k=1;k<=5;k++) { //k is decay class (1-5)

              //If the old area is close to zero, we just set the new volume to zero to avoid dividing by zero.
              if (fOldAreaAllDecayClasses < 0.000001)
                fNewVolByDecayClass = 0.0;

              else {

                //Get the new (post-decay) area of logs from the calcs grid.
                fNewAreaByDecayClass=0.0;

                for (t=0; t<m_iMaxDecayTimesteps; t++) {
                  mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][t], &fTemp);
                  fNewAreaByDecayClass += fTemp;
                }

                //The change in volume is proportional to the change in log area.  Use this relationship to
                //calculate the new log volume by decay class.
                fNewVolByDecayClass = fOldVolAllDecayClasses * fNewAreaByDecayClass / fOldAreaAllDecayClasses;

              }

              //Assign it back to the substrate grid.
              mp_oSubstrateGrid->SetValueOfCell(iX,iY, mp_iLogVolCodes[i][j][k-1], fNewVolByDecayClass);

            }//end for k
          } //end for j
      } //end for iY
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
    stcErr.sFunction = "clDetailedSubstrate::CalcVolumeAfterDecay" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// MortalitySubstrate()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::MortalitySubstrate() {
  try
  {

    //Go through the trees that fell and calculate additions
    AddNewDeadTrees();

    //Re-scale the existing substrate to make room for additions
    AdjustSubstrateForMortality();
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
    stcErr.sFunction = "clDetailedSubstrate::MortalitySubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// AddNewDeadTrees()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::AddNewDeadTrees() {
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    clTreeSearch * p_oAllTrees; //search object for accessing all trees
    clTree * p_oTree; //one individual tree to work with

    float fRandom, //random number
    fRadius, //tree's radius in meters
    fX, fY, //tree's X and Y coordinates
    fDbh, //tree's dbh
    fAngle, //angle of tree fall, in the case of directional tree fall
    fTrunkInterval = 0.5, fStep, //to help apportion log area
    fHeight, //tree's height
    fRadiusHeightRatio, //ratio of a tree trunk's radius to its height (when modelled as a cone)
    fExcludedTopHeight, //the height broken off the top of a broken snag
    fTrunkChunkArea, //one chunk of fresh log, area
    fTrunkChunkVol, //one chunk of fresh log, volume
    fLowerChunkHeight, //the distance from the top of a tree to the bottom of a trunk section
    fChunkDiam, //mid-point diameter of a chunk
    fSubstrate, //temp holder for getting and setting substrate amounts
    fNewBreakHeight, //the height at which a tree or snag has broken, given that it broke during this timestep
    fOldBreakHeight; //the height at which a tree or snag has broken, given that it broke before this timestep

    int iNumTrunkChunks, //number of chunks into which to divide the tree
    iDecayClass, //decay class the log enters
    iDiamClass, //a log chunk's diameter class
    iDead, //is the tree dead?
    iNumXCells = mp_oSubstrateGrid->GetNumberXCells(),
    iNumYCells = mp_oSubstrateGrid->GetNumberYCells();
    int iSp, iTp; //species and type of a tree
    short int i, j, k, iStepX, iStepY, iX, iY; //loop counters

    bool bFall, //did the tree or snag fall over?
    bTopOnly, //is this the top of a broken snag?
    bBottomOnly; //is this the bottom of a broken snag?


    //initialize all new additions to 0
    for ( iX = 0; iX < iNumXCells; iX++ )
      for ( iY = 0; iY < iNumYCells; iY++ ) {
        for (i=0;i<m_iNumLogSpGroupsUsed;i++) //i is SpGroup (0-2)
          for (j=0;j<=1;j++) //j is index of size class (small=0, large=1)
            for (k=1;k<=5;k++) {  //k is decay class (1-5)
              mp_oCalcGrid->SetValueOfCell(iX, iY, mp_iNewLogCalcsCode[i][j][k-1], (float) 0.0);
            } //end i, j, k

        mp_oCalcGrid->SetValueOfCell(iX, iY, m_iNewTipupCalcsCode, (float) 0.0);
      } //end iY

    //Access all trees one at a time and see who's dead - only those who
    //qualify for substrate will be considered.  We could also do a species/type
    //search, but I decided to do it this way because I think it will be
    //marginally faster.
    p_oAllTrees = p_oPop->Find( "all" );
    p_oTree = p_oAllTrees->NextTree();
    while ( p_oTree )
    {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();
      if ( !mp_bSubstrateApplies[iSp] [iTp] )
        goto nextTree;

      //get dead, fall, and break codes for this tree  
      p_oTree->GetValue( mp_iDeadCodes[iSp] [iTp], & iDead );
      bFall = false;
      if (iTp == clTreePopulation::adult) {
        p_oTree->GetValue( mp_iFallCodes[iSp], & bFall );
        p_oTree->GetValue( mp_iNewBreakCodes[iSp][iTp], & fNewBreakHeight );
      }
      else if (iTp == clTreePopulation::snag) {
        p_oTree->GetValue( mp_iNewBreakCodes[iSp][iTp], & fNewBreakHeight );
        p_oTree->GetValue( mp_iOldBreakCodes[iSp], & fOldBreakHeight );
      }

      //skip this tree if it isn't adding log substrate
      //(note: all dead saplings fall over in the timestep that they die)

      if ( iTp != clTreePopulation::snag && notdead == iDead )
        //tree is still alive
        goto nextTree;
      if ( iTp == clTreePopulation::adult && iDead > notdead && !bFall && fNewBreakHeight<0.0)
        //tree is a new intact snag
        goto nextTree;
      if ( iTp == clTreePopulation::snag && notdead == iDead && fNewBreakHeight<0.0)
        //tree is an old snag that did not break this timestep
        goto nextTree;

      //If we're still here, then this tree is adding substrate
      bTopOnly=false;
      bBottomOnly=false;

      //find out if only part of the tree is adding to log substrate
      if (iTp == clTreePopulation::adult && !bFall && fNewBreakHeight>0.0)
        //tree broke along bole, top is added as log substrate
      {bTopOnly = true;}
      else if (iTp == clTreePopulation::snag && notdead == iDead && fNewBreakHeight>0.0)
        //tree is a snag that has broken this timestep
        bTopOnly = true;
      else if (iTp == clTreePopulation::snag && iDead > notdead && fOldBreakHeight>0.0)
        //tree is a broken snag that fell over
        bBottomOnly = true;

      p_oTree->GetValue( p_oPop->GetXCode( iSp, iTp ), & fX );
      p_oTree->GetValue( p_oPop->GetYCode( iSp, iTp ), & fY );
      p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDbh );
      p_oTree->GetValue( p_oPop->GetHeightCode( iSp, iTp ), & fHeight );

      //if this is a tree that died and fell this timestep, remove it so it doesn't remain a snag
      if (iTp != clTreePopulation::snag && bFall)
        p_oPop->KillTree( p_oTree, remove_tree );

      //Use random number to find the decay class this log will be entering
      fRandom = clModelMath::GetRand();

      iDecayClass = 0;
      while (fRandom > 0.0) {
        iDecayClass++;
        if (iTp != clTreePopulation::snag)
          fRandom -= mp_fPropFallenTreesLogDecayClass[iDecayClass-1];
        else
          fRandom -= mp_fPropFallenSnagsLogDecayClass[iDecayClass-1];
      }

      //Get a random azimuth direction for the tree to fall
      fAngle = 2.0 * M_PI * ( clModelMath::GetRand() );

      //Intermediate variables for log area calculations.  The trunk area is modeled as a cone 
      //and calculated in sq m.
      fRadius = fDbh * 0.01 * 0.5; //*0.01 converts from cm to m, *0.5 converts from diameter to radius
      fRadiusHeightRatio = fRadius/(fHeight-1.3); //to convert between radius and height

      if (bTopOnly) //we're only looking at the top part, so reduce the effective height accordingly
        fHeight -= fNewBreakHeight;

      //If we're just dealing with the bottom part, calculate the excluded top height.
      if (bBottomOnly && fHeight > fOldBreakHeight)
        //this is just the bottom of a snag who's top already fell off
        fExcludedTopHeight = fHeight-fOldBreakHeight;
      else
        //this is an intact snag
        fExcludedTopHeight = 0;

      //Divide the trunk into even chunks based on the number of 0.5 meter sections of trunk.       
      iNumTrunkChunks = (int)ceil( (fHeight-fExcludedTopHeight) * 2);

      //Walk out the length of the trunk in 0.5 m intervals and put a chunk of
      //fresh log in each cell we hit.
      for ( i = 0; i < iNumTrunkChunks; i++ ) {

        //Calculate the distance from the the top of the tree to the bottom of the chunk 
        //(closest to base, with larger diam), and the chunk's area and volume 
        //(vol is m3 per ha, area is just m2)

        if (i==0) {

          //this is the bottommost piece, who's length will be less than 0.5 m
          //because the trees don't divide into chunks evenly
          fLowerChunkHeight = fHeight;

          fTrunkChunkArea = fRadiusHeightRatio * ( pow(fLowerChunkHeight,2)
              - pow(fExcludedTopHeight + (iNumTrunkChunks - 1) * fTrunkInterval, 2 ));
          fTrunkChunkVol = (1.0/3.0) * M_PI * pow(fRadiusHeightRatio,2) * ( pow(fLowerChunkHeight, 3 )
              - pow(fExcludedTopHeight + (iNumTrunkChunks - 1) * fTrunkInterval, 3 )) / (m_fGridCellArea/10000.0);

        } else {

          //this is not the bottommost piece and it's height is 0.5 m
          //(slightly different formulas used)
          fLowerChunkHeight = fExcludedTopHeight + (iNumTrunkChunks - i) * fTrunkInterval;

          fTrunkChunkArea = fRadiusHeightRatio * ( pow(fLowerChunkHeight,2)
              - pow(fLowerChunkHeight - fTrunkInterval, 2 ));
          fTrunkChunkVol = (1.0/3.0) * M_PI * pow(fRadiusHeightRatio,2) * ( pow(fLowerChunkHeight,3)
              - pow(fLowerChunkHeight - fTrunkInterval, 3 )) / (m_fGridCellArea/10000.0);

        }

        //Calculate the mid-point diameter of the chunk
        fChunkDiam = 2 * 100 * fRadiusHeightRatio * (fLowerChunkHeight - 0.5 * fTrunkInterval);

        if (fChunkDiam < m_fLogSizeClassBoundary)
          iDiamClass = 0;
        else
          iDiamClass = 1;

        //Find the cell the chunk will go into            
        if (m_bUseDirectionalTreeFall) {
          //Find the X and Y grid cells of the point at the end (closest to the base) of this 0.5 m
          //interval
          fStep = fHeight - fLowerChunkHeight;
        } else {
          //put the chunk at the base of the tree because we're not using directional fall    
          fStep = 0;
        }

        mp_oCalcGrid->GetCellOfPoint( p_oPlot->CorrectX( p_oPlot->GetUncorrectedX( fX, fAngle, fStep ) ),
            p_oPlot->CorrectY( p_oPlot->GetUncorrectedY( fY, fAngle, fStep ) ), & iStepX, & iStepY );

        //Get the amount of new log currently in this tree's grid cell from the calculation grid
        mp_oCalcGrid->GetValueOfCell( iStepX, iStepY, mp_iNewLogCalcsCode[mp_iLogSpGroupAssignment[iSp]][iDiamClass][iDecayClass-1], & fSubstrate );
        //Drop a trunk chunk in this cell
        fSubstrate += fTrunkChunkArea;
        mp_oCalcGrid->SetValueOfCell( iStepX, iStepY, mp_iNewLogCalcsCode[mp_iLogSpGroupAssignment[iSp]][iDiamClass][iDecayClass-1], fSubstrate );

        //same for volume
        mp_oSubstrateGrid->GetValueOfCell( iStepX, iStepY, mp_iLogVolCodes[mp_iLogSpGroupAssignment[iSp]][iDiamClass][iDecayClass-1], & fSubstrate );
        fSubstrate += fTrunkChunkVol;
        mp_oSubstrateGrid->SetValueOfCell( iStepX, iStepY, mp_iLogVolCodes[mp_iLogSpGroupAssignment[iSp]][iDiamClass][iDecayClass-1], fSubstrate );

      } //end for each trunkchunk

      //Now see if this fallen tree uprooted and left a tip-up mound - in which
      //case we'll add the amount of new tipup matter to the tree's grid cell
      //in the calculation grid.

      if (bTopOnly) //it's a top, so there's no tip-up mound
        goto nextTree;
      else if (iTp == clTreePopulation::sapling) //it's a sapling, so there's no tip-up mound
        goto nextTree;

      fRandom = clModelMath::GetRand();

      if (( iTp == clTreePopulation::adult && fRandom <= mp_fPropOfFallenThatUproot[iSp]) ||
          ( iTp == clTreePopulation::snag  && fRandom <= mp_fPropOfSnagsThatUproot[iSp] ))
      {
        //Yes, it uprooted - add the new tipup exposed to this grid's new
        //tipup variable
        mp_oCalcGrid->GetValueAtPoint( fX, fY, m_iNewTipupCalcsCode, & fSubstrate );
        fSubstrate += M_PI * pow( fRadius * m_fRootTipupFactor, 2 );
        mp_oCalcGrid->SetValueAtPoint( fX, fY, m_iNewTipupCalcsCode, fSubstrate );

      } //end if (uprooted)

      nextTree:
      p_oTree = p_oAllTrees->NextTree();

    } //end of while (p_oTree)
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
    stcErr.sFunction = "clDetailedSubstrate::AddNewDeadTrees" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// AdjustSubstrateForMortality()
////////////////////////////////////////////////////////////////////////////
void clDetailedSubstrate::AdjustSubstrateForMortality() {
  try
  {
    clPackage * p_oPackage, //for working with substrate grid packages
    * p_oNextToLastPackage; //for getting the last package
    float fNewSubstrateCreated, //total amount of new substrate created as a proportion
    fTemp1, fTemp2, //temp variables for working with grid vals
    fTotalNew, //amount of new substrate added in m2
    fNewTipup, //amount of new tipup added in m2
    fTotalDivisor; //used to convert new substrate added to proportion of grid cell
    int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(),
        iNumYCells = mp_oSubstrateGrid->GetNumberYCells();
    int iX, iY, i, j, k, t; //loop counters

    //All the dead trees have been added to substrate.  Now go through each
    //grid cell and re-scale the old proportions according to the area of new dead trees added
    for ( iX = 0; iX < iNumXCells; iX++ )
      for ( iY = 0; iY < iNumYCells; iY++ ) {

        fTotalNew=0.0;

        //Get the amount of new substrate added for this grid cell
        mp_oCalcGrid->GetValueOfCell( iX, iY, m_iNewTipupCalcsCode, & fNewTipup );
        fTotalNew += fNewTipup;

        for (i=0;i<m_iNumLogSpGroupsUsed;i++) //i is SpGroup (0-2)
          for (j=0;j<=1;j++) //j is index of size class (small=0, large=1)
            for (k=1;k<=5;k++) {  //k is decay class (1-5)

              mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iNewLogCalcsCode[i][j][k-1], & fTemp1 );
              fTotalNew += fTemp1;

            }

        if ( fTotalNew == 0.0 ) //no new substrate added - nothing to do
          goto nextCell;

        //Was there more substrate area added than there is area in the grid?
        //If not, make the new values a proportion of the total area.  If so,
        //make the new values a proportion of total new substrate
        if ( fTotalNew <= m_fGridCellArea ) {
          fTotalDivisor = m_fGridCellArea;
          fNewSubstrateCreated = fTotalNew / m_fGridCellArea;
        } else {
          fTotalDivisor = fTotalNew;
          fNewSubstrateCreated = 1.0;
        }

        //Reduce each package's substrate proportions by the amount of new
        //substrate created
        p_oPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );
        while ( p_oPackage )
        {
          //Decrement the initial package according to the new proportions created
          p_oPackage->GetValue( m_iPkgScarSoilCode, & fTemp1 );
          fTemp1 *= ( 1 - fNewSubstrateCreated );
          p_oPackage->SetValue( m_iPkgScarSoilCode, fTemp1 );

          p_oPackage->GetValue( m_iPkgTipupCode, & fTemp1 );
          fTemp1 *= ( 1 - fNewSubstrateCreated );
          p_oPackage->SetValue( m_iPkgTipupCode, fTemp1 );

          p_oPackage = p_oPackage->GetNextPackage();
        } //end of while (p_oPackage)

        //adjust the log proportions likewise
        //(note that no adjustment is made to log volume)
        for (t=0; t<m_iMaxDecayTimesteps; t++)
          for (i=0;i<m_iNumLogSpGroupsUsed;i++) //i is SpGroup (0-2)
            for (j=0;j<=1;j++) //j is index of size class (small=0, large=1)
              for (k=1;k<=5;k++) {  //k is decay class (1-5)

                mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][t], & fTemp1 );
                fTemp1 *= ( 1 - fNewSubstrateCreated );
                mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][t], fTemp1 );

              } //end for t, i, j, k

        //If new tipup was added, Create a package now and put it last
        if (fNewTipup > 0.0) {

          p_oNextToLastPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );
          if (!p_oNextToLastPackage)
            //there's no packages already, so add it as a first one
            p_oPackage = mp_oSubstrateGrid->CreatePackageOfCell( iX, iY );
          else
            while ( p_oNextToLastPackage ) {

              p_oPackage = p_oNextToLastPackage->GetNextPackage();
              if ( !p_oPackage ) {

                //we're at the last package, so add a new one after it
                p_oPackage = mp_oSubstrateGrid->CreatePackage( p_oNextToLastPackage );
                //Now break the loop
                p_oNextToLastPackage = NULL;

              } else
                p_oNextToLastPackage = p_oPackage;
            }

          p_oPackage->SetValue( m_iPkgAgeCode, (int) 0 );
          p_oPackage->SetValue( m_iPkgTipupCode, fNewTipup / fTotalDivisor );

          //Leave scarified soil at zero

        } //end if (fNewTipup > 0.0)

        //Add the logs back to the grid
        for (i=0;i<m_iNumLogSpGroupsUsed;i++) //i is SpGroup (0-2)
          for (j=0;j<=1;j++) //j is index of size class (small=0, large=1)
            for (k=1;k<=5;k++) {  //k is decay class (1-5)

              mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][0], & fTemp1 );
              mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iNewLogCalcsCode[i][j][k-1], & fTemp2 );
              fTemp1 += fTemp2/fTotalDivisor;
              mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iLogCalcsCode[i][j][k-1][0], fTemp1 );

            }

        nextCell:
        ;
      } //end for (iY)
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
    stcErr.sFunction = "clDetailedSubstrate::AdjustSubstrateForMortality" ;
    throw( stcErr );
  }
}
