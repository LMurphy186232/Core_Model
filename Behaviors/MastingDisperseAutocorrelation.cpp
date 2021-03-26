/*
 * MastingDisperseAutocorrelation.cpp
 *
 *  Created on: Mar 2, 2021
 *      Author: lora
 */
#include "MastingDisperseAutocorrelation.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "Plot.h"
#include "ModelMath.h"
#include "DataTypes.h"
#include <math.h>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
clMastingDisperseAutocorrelation::clMastingDisperseAutocorrelation( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clDisperseBase( p_oSimManager)
{
  try
  {
    m_sNameString = "MastingDisperseAutocorrelation";
    m_sXMLRoot = "MastingDisperseAutocorrelation";

    mp_iSpsCodes = NULL;
    mp_iPrevseedsCodes = NULL;

    mp_fMastTimeSeries = NULL;
    mp_fDbhForReproduction = NULL;
    mp_fMaxDbh = NULL;
    mp_fA = NULL;
    mp_fB = NULL;
    mp_fC = NULL;
    mp_fACF = NULL;
    mp_fRhoNoiseSD = NULL;
    mp_fPRA = NULL;
    mp_fPRB = NULL;
    mp_fSPSSD = NULL;

    mp_fSeedCDF = NULL;
    mp_fStrMean = NULL;
    mp_fBeta = NULL;
    mp_bIsUsed = NULL;
    mp_iIndexes = NULL;
    m_cQuery = NULL;

    m_iMaxDistance = 0;
    m_iMaxTimesteps = 0;
    m_iNumYearsPerTimestep = 0;

    //Indicate that this behavior intends to add 2 tree float data members
    m_iNewTreeFloats = 2;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

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
    stcErr.sFunction = "clMastingDisperseAutocorrelation::clMastingDisperseAutocorrelation" ;
    throw( stcErr );
  }
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
clMastingDisperseAutocorrelation::~clMastingDisperseAutocorrelation()
{
  short int i; //loop counters

  if (mp_fSeedCDF) {
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      delete[] mp_fSeedCDF[i];
    delete[] mp_fSeedCDF;
  }

  if ( mp_bIsUsed ) {
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_bIsUsed[i];
    delete[] mp_bIsUsed;
  }

  if (mp_iSpsCodes) {
    for (i = 0; i < m_iTotalSpecies; i++) {
      delete[] mp_iSpsCodes[i];
    }
    delete[] mp_iSpsCodes;
  }

  if (mp_iPrevseedsCodes) {
    for (i = 0; i < m_iTotalSpecies; i++) {
      delete[] mp_iPrevseedsCodes[i];
    }
    delete[] mp_iPrevseedsCodes;
  }

  delete[] mp_fStrMean;
  delete[] mp_fBeta;
  delete[] mp_fDbhForReproduction;
  delete[] mp_fMaxDbh;
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fC;
  delete[] mp_fACF;
  delete[] mp_fRhoNoiseSD;
  delete[] mp_fPRA;
  delete[] mp_fPRB;
  delete[] mp_fSPSSD;
  delete[] mp_iIndexes;
  delete[] m_cQuery;
  delete[] mp_fMastTimeSeries;
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::RegisterTreeDataMembers() {

  clTreePopulation
      * p_oPop =
          ( clTreePopulation * ) mp_oSimManager->GetPopulationObject("treepopulation");
  short int i, j, iNumSpecies = p_oPop->GetNumberOfSpecies(), iNumTypes = p_oPop->GetNumberOfTypes();


  //Declare the array and register our new data member
  mp_iSpsCodes = new short int * [iNumSpecies];
  for (i = 0; i < iNumSpecies; i++) {
    mp_iSpsCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iSpsCodes[i][j] = -1;
    }
  }

  mp_iPrevseedsCodes = new short int * [iNumSpecies];
    for (i = 0; i < iNumSpecies; i++) {
      mp_iPrevseedsCodes[i] = new short int[iNumTypes];
      for (j = 0; j < iNumTypes; j++) {
        mp_iPrevseedsCodes[i][j] = -1;
      }
    }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    //Make sure that only allowed types have been applied
    if (clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType
        && clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
        && clTreePopulation::seedling != mp_whatSpeciesTypeCombos[i].iType) {

      modelErr stcErr;
      stcErr.sFunction = "clMastingDisperseAutocorrelation::RegisterTreeDataMembers";
      stcErr.sMoreInfo = "This behavior can only be applied to seedlings, saplings, and adults.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Register the code and capture it
    mp_iSpsCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
        = p_oPop->RegisterFloat("sps", mp_whatSpeciesTypeCombos[i].iSpecies,
            mp_whatSpeciesTypeCombos[i].iType);

    mp_iPrevseedsCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
            = p_oPop->RegisterFloat("prevseeds", mp_whatSpeciesTypeCombos[i].iSpecies,
                mp_whatSpeciesTypeCombos[i].iType);
  }
}



///////////////////////////////////////////////////////////////////////////////
// DoShellSetup
///////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    xercesc::DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    int i;
    m_iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    mp_iIndexes = new short int[m_iTotalSpecies];
    for (i = 0; i < m_iTotalSpecies; i++) mp_iIndexes[i] = -1;

    //Make the list of indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    PopulateUsedTable( p_oPop );
    GetMastTimeseries(p_oElement);
    GetParameterFileData( p_oElement, p_oPop );
    CalcSeedCDF( p_oElement, p_oPop );
    FormatQueryString( p_oPop );
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
    stcErr.sFunction = "clMastingDisperseAutocorrelation::DoShellSetup" ;
    throw( stcErr );
  }
}


///////////////////////////////////////////////////////////////////////////////
// PopulateUsedTable()
///////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::PopulateUsedTable( clTreePopulation * p_oPop )
{
  short int iNumTypes = p_oPop->GetNumberOfTypes(), i, j; //loop counters

  //Declare the table - it's # species by # types
  mp_bIsUsed = new bool * [m_iTotalSpecies];
  for ( i = 0; i < m_iTotalSpecies; i++ )
  {
    mp_bIsUsed[i] = new bool[iNumTypes];
    for ( j = 0; j < iNumTypes; j++ )
      mp_bIsUsed[i] [j] = false;
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Make sure that the types are only sapling, adult, or stump
    if ( mp_whatSpeciesTypeCombos[i].iType != clTreePopulation::sapling
         && mp_whatSpeciesTypeCombos[i].iType != clTreePopulation::adult )
         {
           modelErr stcErr;
           stcErr.sFunction = "clMastingDisperseAutocorrelation::PopulateUsedTable" ;
           stcErr.sMoreInfo = "Dispersal may only be applied to saplings and adults.";
           stcErr.iErrorCode = BAD_DATA;
           throw( stcErr );
    }
    mp_bIsUsed[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] = true;
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetParameterFileData
///////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::GetParameterFileData( xercesc::DOMElement * p_oElement, clTreePopulation * p_oPop)
{
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  try {
    short int i; //loop counter

    //Declare arrays
    mp_fStrMean = new double[m_iNumBehaviorSpecies];
    mp_fBeta = new double[m_iNumBehaviorSpecies];
    mp_fDbhForReproduction = new double[m_iNumBehaviorSpecies];
    mp_fMaxDbh = new double[m_iNumBehaviorSpecies];

    mp_fA = new double[m_iNumBehaviorSpecies];
    mp_fB = new double[m_iNumBehaviorSpecies];
    mp_fC = new double[m_iNumBehaviorSpecies];
    mp_fACF = new double[m_iNumBehaviorSpecies];
    mp_fRhoNoiseSD = new double[m_iNumBehaviorSpecies];
    mp_fPRA = new double[m_iNumBehaviorSpecies];
    mp_fPRB = new double[m_iNumBehaviorSpecies];
    mp_fSPSSD = new double[m_iNumBehaviorSpecies];

    p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTemp[i].code = mp_iWhatSpecies[i];

    //Minimum DBH for reproduction
    FillSpeciesSpecificValue( p_oElement, "di_minDbhForReproduction", "di_mdfrVal", p_fTemp,
       m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fDbhForReproduction[i] = p_fTemp[i].val;

    //Maximum DBH for size effects
    FillSpeciesSpecificValue( p_oElement, "di_maxDbhForSizeEffect", "di_mdfseVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fMaxDbh[i] = p_fTemp[i].val;
      if (mp_fMaxDbh[i] < mp_fDbhForReproduction[i]) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
        stcErr.sMoreInfo = "Max DBH must be greater than minimum repro DBH.";
        throw( stcErr );
      }
    }

    //Beta
    FillSpeciesSpecificValue( p_oElement, "di_spatialBeta", "di_sbVal", p_fTemp,
                            m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fBeta[i] = p_fTemp[i].val;
      if (p_fTemp[i].val > 25) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
        stcErr.sMoreInfo = "One or more beta values is too large and will cause math errors.";
        throw( stcErr );
      }
    }

    //Mean STR
    FillSpeciesSpecificValue( p_oElement, "di_spatialSTR", "di_sstrVal", p_fTemp,
                            m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStrMean[i] = p_fTemp[i].val;

    //A, B, and C for fraction participating
    FillSpeciesSpecificValue( p_oElement, "di_mdaReproFracA", "di_mdarfaVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fA[i] = p_fTemp[i].val;
      if (p_fTemp[i].val ==  0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
        stcErr.sMoreInfo = "Fraction participating A cannot be 0.";
        throw( stcErr );
      }
    }

    FillSpeciesSpecificValue( p_oElement, "di_mdaReproFracB", "di_mdarfbVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB[i] = p_fTemp[i].val;

    FillSpeciesSpecificValue( p_oElement, "di_mdaReproFracC", "di_mdarfcVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fC[i] = p_fTemp[i].val;
      //Enforce that C is between 0 and 1
      if (p_fTemp[i].val > 1 || p_fTemp[i].val < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
        stcErr.sMoreInfo = "Reproduction fraction c must be between 0 and 1.";
        throw( stcErr );
      }
    }

    //Autocorrelation factor for rho - make sure isn't 0
    FillSpeciesSpecificValue( p_oElement, "di_mdaRhoACF", "di_mdaraVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fACF[i] = p_fTemp[i].val;
      if (p_fTemp[i].val <=  0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
        stcErr.sMoreInfo = "Rho autocorrelation factor cannot be less than or equal to 0.";
        throw( stcErr );
      }
    }

    //Standard deviation for noise for rho - must be greater than 0
    FillSpeciesSpecificValue( p_oElement, "di_mdaRhoNoiseSD", "di_mdarnsdVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTemp[i].val < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
        stcErr.sMoreInfo = "Standard deviation of rho noise cannot be less than 0";
        throw( stcErr );
      }
      mp_fRhoNoiseSD[i] = p_fTemp[i].val;
    }

    //Slope and intercept of the linear function of DBH for individual
    //probability of reproducting; this function is truncated at 0 and 1 so
    //we are unbothered by outlandish values
    FillSpeciesSpecificValue( p_oElement, "di_mdaPRA", "di_mdapraVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPRA[i] = p_fTemp[i].val;

    FillSpeciesSpecificValue( p_oElement, "di_mdaPRB", "di_mdaprbVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPRB[i] = p_fTemp[i].val;

    //Seed producer standard deviation. Must be greater than 0.
    FillSpeciesSpecificValue( p_oElement, "di_mdaSPSSD", "di_mdaspssdVal", p_fTemp,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTemp[i].val < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
        stcErr.sMoreInfo = "Standard deviation of rho noise cannot be less than 0";
        throw( stcErr );
      }
      mp_fSPSSD[i] = p_fTemp[i].val;
    }

    delete[] p_fTemp;
  }
  catch ( modelErr & err )
  {
    delete[] p_fTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalcSeedCDF
///////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::CalcSeedCDF(xercesc::DOMElement *p_oElement, clTreePopulation *p_oPop)
{

  float *p_fDispersalX0 = new float[m_iNumBehaviorSpecies],
        *p_fThetaXb = new float[m_iNumBehaviorSpecies];
  function *p_iWhatFunction = new function[m_iNumBehaviorSpecies];
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  intVal * p_iTemp = NULL; //for getting species-specific values
  try {
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    float fXPlotLength = p_oPlot->GetXPlotLength(),
          fYPlotLength = p_oPlot->GetYPlotLength();
    int iNumFunctionSpecies, //number of species using a given function
        i, j;

    //Which species use which function - set up our temp int array and pre-load
    //with this behavior's species
    p_iTemp = new intVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_iTemp[i].code = mp_iWhatSpecies[i];


    //Calculate the maximum dispersal distance - which is the maximum dimension
    //of the grid with max of 1000 m
    if ( fXPlotLength > fYPlotLength )
      if ( fXPlotLength > 1000 )
        m_iMaxDistance = 1000;
      else
        m_iMaxDistance = (int)fXPlotLength;
    else if ( fYPlotLength > 1000 )
      m_iMaxDistance = 1000;
    else
      m_iMaxDistance = (int)fYPlotLength;

    mp_fSeedCDF = new float*[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      mp_fSeedCDF[i] = new float[m_iMaxDistance];
      for (j = 0; j < m_iMaxDistance; j++) {
        mp_fSeedCDF[i][j] = 0;
      }
    }

    //Function used for seed dispersal kernel
    FillSpeciesSpecificValue( p_oElement, "di_canopyFunction", "di_cfVal",
        p_iTemp, m_iNumBehaviorSpecies, p_oPop, true );

    //Transfer the values over - if any are not a valid function enum number,
    //throw an error
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( p_iTemp[i].val != weibull && p_iTemp[i].val != lognormal )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
        std::stringstream s;
        s << "Invalid disperse function code " << p_iTemp[i].val;
        stcErr.sMoreInfo = s.str();
        throw( stcErr );
      }
      else
      {
        p_iWhatFunction[mp_iIndexes[p_iTemp[i].code]] = (function)p_iTemp[i].val;
      }


    //Read the parameters for calculating seed CDF - keeping track of which
    //species use weibull and which use lognormal
    //Those who use weibull:
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( weibull == p_iWhatFunction[i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    p_fTemp = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( weibull == p_iWhatFunction[i] )
      {
        p_fTemp[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {
      //Dispersal
      FillSpeciesSpecificValue( p_oElement, "di_weibullCanopyDispersal",
                                "di_wcdVal", p_fTemp, iNumFunctionSpecies,
                                p_oPop, true );
      //Transfer to array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        p_fDispersalX0[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Theta
      FillSpeciesSpecificValue( p_oElement, "di_weibullCanopyTheta", "di_wctVal",
                                p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to array
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        p_fThetaXb[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
        if (p_fTemp[i].val >= 50) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clMastingDisperseAutocorrelation::CalcSeedCDF" ;
          stcErr.sMoreInfo = "One or more weibull theta values is too large and will cause math errors.";
          throw( stcErr );
        }
      }
    } //end of if (iNumFunctionSpecies > 0) {

    //Those who use lognormal:
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( lognormal == p_iWhatFunction[i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTemp;
    p_fTemp = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( lognormal == p_iWhatFunction[i] )
      {
        p_fTemp[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {
      //X0
      FillSpeciesSpecificValue( p_oElement, "di_lognormalCanopyX0", "di_lcx0Val",
                                p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        p_fDispersalX0[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Xb
      FillSpeciesSpecificValue( p_oElement, "di_lognormalCanopyXb", "di_lcxbVal",
                                p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        p_fThetaXb[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
    }

    //Calculate cumulative probability arrays
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      CalculateProbabilityDistribution( mp_fSeedCDF[i], m_iMaxDistance,
                              p_iWhatFunction[mp_iIndexes[mp_iWhatSpecies[i]]],
                              p_fDispersalX0[mp_iIndexes[mp_iWhatSpecies[i]]],
                              p_fThetaXb[mp_iIndexes[mp_iWhatSpecies[i]]]);
    }

    delete[] p_fDispersalX0;
    delete[] p_fThetaXb;
    delete[] p_fTemp;
    delete[] p_iTemp;
  }
  catch ( modelErr & err )
  {
    delete[] p_fDispersalX0;
    delete[] p_fThetaXb;
    delete[] p_fTemp;
    delete[] p_iTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fDispersalX0;
    delete[] p_fThetaXb;
    delete[] p_fTemp;
    delete[] p_iTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fDispersalX0;
    delete[] p_fThetaXb;
    delete[] p_fTemp;
    delete[] p_iTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMastingDisperseAutocorrelation::GetParameterFileData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalculateProbabilityDistribution()
/////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::CalculateProbabilityDistribution( float * p_fProbArray,
  const int &iMaxDistance, const function &iFunction, const float &fDispersalX0,
  const float &fThetaXb)
{
  float fCumProb = 0, //area under the probability distribution function curve -
                      //sum of all the probabilities for each distance
        fInc, //incremental probability between adjacent units
        fNormalizer, //normalizing factor for cumulative probability array
        fErr = 0.0001; //prevents underflow errors
  int iDistance; //distance in meters - loop counter

  //Calculate cumulative probability for seed dispersal from 0 out to the
  //maximum dispersal distance
  for ( iDistance = 0; iDistance < iMaxDistance; iDistance++ )
  {
    if (weibull == iFunction) {
      fInc = M_PI * ( 1 + 2 * iDistance ) *
           clModelMath::CalculateWeibullFunction( fDispersalX0, fThetaXb,
                                                  iDistance + 1);
    } else {
      fInc = M_PI * ( 1 + 2 * iDistance ) *
           clModelMath::CalculateLognormalFunction( fDispersalX0, fThetaXb,
                                                    iDistance + 1);
    }

    //Avoid dealing with super-small float-underflow type numbers
    if ( fInc < fErr )
      fInc = 0.0;

    //Add up all the probabilities as they are calculated
    fCumProb += fInc;
  }

  //Normalize the cumulative probability array
  //Error trap - otherwise fNormalizer overflows
  if ( fCumProb < fErr ) fNormalizer = 0;
  else fNormalizer = 1.0 / fCumProb;
  fCumProb = 0.0;

  for ( iDistance = 0; iDistance < iMaxDistance; iDistance++ )
  {
    if (weibull == iFunction) {
      fInc = fNormalizer * M_PI * ( 1 + 2 * iDistance ) *
           clModelMath::CalculateWeibullFunction( fDispersalX0, fThetaXb,
                                                  iDistance + 1);
    } else {
      fInc = fNormalizer * M_PI * ( 1 + 2 * iDistance ) *
           clModelMath::CalculateLognormalFunction( fDispersalX0, fThetaXb,
                                                    iDistance + 1);
    }

    //Add up the cumulative probability to this point - that's the value that
    //goes into the array
    fCumProb += fInc;
    //Don't want the value to go over 1
    if ( fCumProb > 1 ) fCumProb = 1;

    //Special case - the last bucket in the array should have a value of 1
    if ( iDistance == iMaxDistance - 1 )
      p_fProbArray[iDistance] = 1;
    else
      p_fProbArray[iDistance] = fCumProb;
  }

  //Walk out one more time.  Each value should be greater than the value in
  //the bucket before it (unless 1 has been reached), or it's an error.
  for ( iDistance = 1; iDistance < iMaxDistance; iDistance++ )
  {
    if ( p_fProbArray[iDistance] < p_fProbArray[iDistance - 1] )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisperseBase::CalculateProb..." ;
      stcErr.sMoreInfo = "Probability array does not increase sequentially.";
      throw( stcErr );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// AddSeeds()
/////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::AddSeeds()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTreeSearch * p_oAllTrees; //search object for getting all trees
    clTree * p_oTree; //for working with a single tree
    float *p_fFractionParticipating = new float[m_iNumBehaviorSpecies];
    float fDbh, //tree's dbh
          fTemp, fReproProbability,
          fMastingLevel,
          fRho,
          fPrevSeeds,
          fPotentialSeeds, fSeeds,
          fSps; //tree's seed producer score
    short int iSp, iType, //species and type of a given tree
              i;

    //Get the masting level
    fMastingLevel = mp_fMastTimeSeries[(mp_oSimManager->GetCurrentTimestep()-1)];

    //Calculate the fraction of trees participating as a sigmoidal function
    //of mast level
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      p_fFractionParticipating[i] =
          mp_fC[i] /(1+pow((fMastingLevel/mp_fA[i]), mp_fB[i]));
    }

    //Ask the tree population to find the trees in our query
    p_oAllTrees = p_oPop->Find( m_cQuery );

    //Go through the trees one at a time
    p_oTree = p_oAllTrees->NextTree();
    while ( p_oTree )
    {
      //Get the tree's species and type
      iSp = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      //Check to see if this species/type combo is used
      if ( false == mp_bIsUsed[iSp] [iType] ) goto nextTree;

      // Get the tree's sps code - assign if new
      p_oTree->GetValue(mp_iSpsCodes[iSp][iType], &fSps);
      if (fSps == 0) {
        fSps = 1 + clModelMath::NormalRandomDraw(mp_fSPSSD[mp_iIndexes[iSp]]);
        p_oTree->SetValue(mp_iSpsCodes[iSp][iType], fSps);
      }

      // Get the tree's dbh and verify that it is big enough to reproduce
      p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iType ), & fDbh );
      if ( fDbh < mp_fDbhForReproduction[mp_iIndexes[iSp]]) goto nextTree;

      // Get the probability that the tree will reproduce. The probability is
      // the individual probability (linear function of DBH, truncated at 0
      // and 1) times the population fraction reproducing.
      fReproProbability = mp_fPRA[mp_iIndexes[iSp]] + mp_fPRB[mp_iIndexes[iSp]] * fDbh;
      fReproProbability = fmax(fReproProbability, 0.0);
      fReproProbability = fmin(fReproProbability, 1.0);
      fReproProbability *= p_fFractionParticipating[mp_iIndexes[iSp]];

      // Test to see if the tree will be reproducing
      if (clModelMath::GetRand() > fReproProbability) {
        // Nope. Set 0 as the seeds for this year to be saved for next year
        fPrevSeeds = 0;
        p_oTree->SetValue(mp_iPrevseedsCodes[iSp][iType], fPrevSeeds);
        goto nextTree;
      }

      //Calculate rho - make sure it is truncated at the max DBH
      fTemp = fDbh > mp_fMaxDbh[mp_iIndexes[iSp]] ? mp_fMaxDbh[mp_iIndexes[iSp]] : fDbh;

      fRho = ((fTemp - mp_fDbhForReproduction[mp_iIndexes[iSp]]) /
          (mp_fMaxDbh[mp_iIndexes[iSp]] - mp_fDbhForReproduction[mp_iIndexes[iSp]])) *
              mp_fACF[mp_iIndexes[iSp]];

      // Add noise to rho if desired by the user
      if (mp_fRhoNoiseSD[mp_iIndexes[iSp]] > 0) {
        fRho += clModelMath::NormalRandomDraw(mp_fRhoNoiseSD[mp_iIndexes[iSp]]);
      }

      // Get last year's seed count. If 0, assume this tree is new (or this
      // is the first timestep) and give it a mean fecundity as a starting
      // point
      p_oTree->GetValue(mp_iPrevseedsCodes[iSp][iType], &fPrevSeeds);
      if (fPrevSeeds == 0) {
        fPrevSeeds = fSps * mp_fStrMean[mp_iIndexes[iSp]] * pow(fDbh/30.0, mp_fBeta[mp_iIndexes[iSp]]);
        fPrevSeeds = clModelMath::RandomRound(fPrevSeeds);
      }

      // Calculate potential seeds
      fPotentialSeeds = fSps * mp_fStrMean[mp_iIndexes[iSp]] * pow(fDbh/30.0, mp_fBeta[mp_iIndexes[iSp]]);
      fSeeds = fRho * (fPrevSeeds - fPotentialSeeds) + fPotentialSeeds;
      fSeeds = fSeeds * fMastingLevel * m_iNumYearsPerTimestep;
      fSeeds = clModelMath::RandomRound(fSeeds);

      if (fSeeds < 0) fSeeds = 0;

      // Save this for next year
      p_oTree->SetValue(mp_iPrevseedsCodes[iSp][iType], fSeeds);

      // Send out these seeds
      DisperseOneParentSeeds( p_oTree, p_oPop, p_oPlot, fSeeds );

      nextTree:
      p_oTree = p_oAllTrees->NextTree();
    } //end of while (p_oTree) */
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
    stcErr.sFunction = "clMastingDisperseAutocorrelation::AddSeeds" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// DisperseOneParentSeeds
//////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::DisperseOneParentSeeds( clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, float fSeeds )
{
  try
  {
    float fRand, //random number for determining distance and direction of
    //a seed's dispersal
    fDistance, //distance from the parent tree at which a seed lands
    fPrevBucket, //value in the previous bucket of the probability array
    fX, fY, //coordinates of tree
    fNumGridSeeds, //number of seeds in the seed grid
    fSeedX, fSeedY, //coordinates of seed
    fAngle, //angle from the parent tree from which a seed lands
    f; //loop counter
    int iDistanceCounter = 0; //used to walk out the cumulative probability array
    short int iSp = p_oTree->GetSpecies(), //reproducing tree's species
    iType = p_oTree->GetType(), //reproducing tree's type
    iSpIndex = mp_iIndexes[iSp];

    //Get the X and Y coords of the tree
    p_oTree->GetValue( p_oPop->GetXCode( iSp, iType ), & fX );
    p_oTree->GetValue( p_oPop->GetYCode( iSp, iType ), & fY );

    //Disperse the seeds produced
    for ( f = 0; f < fSeeds; f++ )
    {
      //Use the cumulative probability array to determine how far away from the
      //parent tree the seed lands - use a random number and find the first
      //probability array bucket with a value greater than the random
      iDistanceCounter = 0;
      fRand = clModelMath::GetRand();
      while ( fRand > mp_fSeedCDF[iSpIndex][iDistanceCounter] )
        iDistanceCounter++;

      //Get the value in the array bucket before the target one
      fPrevBucket = ( iDistanceCounter == 0 ) ? 0.0 : mp_fSeedCDF[iSpIndex][iDistanceCounter - 1];

      //Calculate the distance at which the seed will land
      fDistance = ( iDistanceCounter + ( fRand - fPrevBucket )
           / ( mp_fSeedCDF[iSpIndex][iDistanceCounter] - fPrevBucket ) );

      //Get a random direction to pitch the seed
      fRand = clModelMath::GetRand();
      fAngle = 2.0 * M_PI * fRand;

      //Using the angle and distance, get an X and Y value for the seed
      fSeedX = p_oPlot->CorrectX( cos( fAngle ) * fDistance + fX );
      fSeedY = p_oPlot->CorrectY( sin( fAngle ) * fDistance + fY );

      //Increment the seed counter at the seed's location in the grid
      mp_oSeedGrid->GetValueAtPoint( fSeedX, fSeedY, mp_iNumSeedsCode[iSp], & fNumGridSeeds );
      fNumGridSeeds++;
      mp_oSeedGrid->SetValueAtPoint( fSeedX, fSeedY, mp_iNumSeedsCode[iSp], fNumGridSeeds );
    } //end of for (i = 0; i < iNumSeeds; i++)
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
    stcErr.sFunction = "clMastingDisperseAutocorrelation::DisperseOneParentSeeds" ;
    throw( stcErr );
  }
}



//////////////////////////////////////////////////////////////////////////////
// FormatQueryString
///////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::FormatQueryString(clTreePopulation * p_oPop) {
  int i;
  bool bAdults = false, bSaplings = false;

  //Find out what types this is being applied to
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType )
      bAdults = true;
    else if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType )
      bSaplings = true;
  }

  //Assemble the type query
  m_cQuery = new char[15];
  if ( bAdults && !bSaplings )
    sprintf( m_cQuery, "%s%d", "type=", clTreePopulation::adult );
  else if ( !bAdults && bSaplings )
    sprintf( m_cQuery, "%s%d", "type=", clTreePopulation::sapling );
  else
    sprintf( m_cQuery, "%s%d%s%d", "type=", clTreePopulation::sapling, ",", clTreePopulation::adult );
}

//////////////////////////////////////////////////////////////////////////////
// GetMastTimeseries
///////////////////////////////////////////////////////////////////////////////
void clMastingDisperseAutocorrelation::GetMastTimeseries(DOMElement *p_oParent) {

  DOMNodeList *p_oNodeList;
  DOMElement *p_oElement, *p_oChildElement;
  DOMNode *p_oDocNode;
  XMLCh *sVal;
  std::string sParentTag = "di_mdaMastTS",
              sSubTag = "di_mdaMTS";
  char *cName;
  double fVal;
  int iNumChildren, iNumNodes, i, iCode, iTimesteps = mp_oSimManager->GetNumberOfTimesteps();

  mp_fMastTimeSeries = new double[iTimesteps];

  //-------------------------------------------------------------------------//
  // Determine whether or not the user has included a masting time series in
  // the parameter file
  //-------------------------------------------------------------------------//
  //Find the parent element
  sVal = XMLString::transcode(sParentTag.c_str());
  p_oNodeList = p_oParent->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  iNumNodes = p_oNodeList->getLength();

  //-------------------------------------------------------------------------//
  // If no list, randomly generate mast numbers between 0 and 1 and quit
  //-------------------------------------------------------------------------//
  if (0 == iNumNodes) {
    for (i = 0; i < iTimesteps; i++) {
      mp_fMastTimeSeries[i] = clModelMath::GetRand();
    }

    return;
  }

  //-------------------------------------------------------------------------//
  // If list, read it
  //-------------------------------------------------------------------------//
  p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

  //Now get the list of all subelements with the actual data
  p_oElement = (DOMElement *) p_oDocNode; //cast to DOM_Element
  sVal = XMLString::transcode(sSubTag.c_str());
  p_oNodeList = p_oElement->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  iNumChildren = p_oNodeList->getLength();
  if (iNumChildren != mp_oSimManager->GetNumberOfTimesteps()) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clMastingDisperseAutocorrelation::GetMastTimeseries";
    stcErr.sMoreInfo = "Not enough timesteps of data.";
    throw(stcErr);
  }

  for (i = 0; i < iNumChildren; i++) {
    p_oDocNode = p_oNodeList->item(i);
    p_oChildElement = (DOMElement *) p_oDocNode;
    sVal = XMLString::transcode("ts");
    cName = XMLString::transcode(p_oChildElement->getAttributeNode(sVal)->getNodeValue());
    XMLString::release(&sVal);
    iCode = atoi(cName);
    if (0 > iCode || iTimesteps < iCode) { //throw an error
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clMastingDisperseAutocorrelation::GetMastTimeseries";
      stcErr.sMoreInfo = "Unrecognized timestep in mast level timeseries.";
      throw(stcErr);
    }
    cName = XMLString::transcode(p_oChildElement->getChildNodes()->item(0)->getNodeValue());
    fVal = strtod(cName, NULL);
    if (0 > fVal || 1 < fVal) { //throw an error
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clMastingDisperseAutocorrelation::GetMastTimeseries";
      stcErr.sMoreInfo = "Mast level must be between 0 and 1.";
      throw(stcErr);
    }
    if (fVal != 0 || cName[0] == '0') {
      mp_fMastTimeSeries[(iCode-1)] = fVal;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetMastLevel
///////////////////////////////////////////////////////////////////////////////
double clMastingDisperseAutocorrelation::GetMastLevel(int iTs) {
  if (mp_fMastTimeSeries && iTs > 0 && iTs <= mp_oSimManager->GetNumberOfTimesteps())
    return mp_fMastTimeSeries[(iTs-1)];
  else return -1;
}

/*/ ////////////////////////////////////////////////////////////////////////////
WriteCumProbArray
/////////////////////////////////////////////////////////////////////////////*/
/*void clMastingDisperseAutocorrelation::WriteCumProbArray() { fstream matrices; int iMaxDistance = mp_oDisperseOrg->GetMaxDistance();
short int n, iSp, iNumSpecies = mp_oDisperseOrg->GetNumberOfSpecies();

matrices.open("NewCumProb.xls", ios::out | ios::app); matrices << "Cumulative Probability Matrix - Weibull:\n";
matrices << "Canopy:\nSpecies:"; for (n = 0; n < iMaxDistance; n++) matrices << "\t" << n; matrices << "\n";
for (iSp = 0; iSp < iNumSpecies; iSp++) { if (mp_fCumProb[weibull][canopy][iSp]) {
matrices << "Species " << mp_iWhatSpecies[iSp] << "\t"; for (n = 0; n < iMaxDistance; n++)
matrices << mp_fCumProb[weibull][canopy][iSp][n] << "\t"; matrices << "\n"; } }

matrices << "\n\nGap:\nSpecies:"; for (n = 0; n < iMaxDistance; n++) matrices << "\t" << n; matrices << "\n";
for (iSp = 0; iSp < iNumSpecies; iSp++) { if (mp_fCumProb[weibull][gap][iSp]) {
matrices << "Species " << mp_iWhatSpecies[iSp] << "\t"; for (n = 0; n < iMaxDistance; n++)
matrices << mp_fCumProb[weibull][gap][iSp][n] << "\t"; matrices << "\n"; } }

matrices << "\n\nCumulative Probability Matrix - Lognormal:\nCanopy:\nSpecies:"; for (n = 0; n < iMaxDistance; n++)
matrices << "\t" << n; matrices << "\n"; for (iSp = 0; iSp < iNumSpecies; iSp++) { if (mp_fCumProb[lognormal][canopy][iSp]) {
matrices << "Species " << mp_iWhatSpecies[iSp] << "\t"; for (n = 0; n < iMaxDistance; n++)
matrices << mp_fCumProb[lognormal][canopy][iSp][n] << "\t"; matrices << "\n"; } }

matrices << "\n\nGap:\nSpecies:"; for (n = 0; n < iMaxDistance; n++) matrices << "\t" << n; matrices << "\n";
for (iSp = 0; iSp < iNumSpecies; iSp++) { if (mp_fCumProb[lognormal][gap][iSp]) {
matrices << "Species " << mp_iWhatSpecies[iSp] << "\t"; for (n = 0; n < iMaxDistance; n++)
matrices << mp_fCumProb[lognormal][gap][iSp][n] << "\t"; matrices << "\n"; } } matrices.close(); } */
//---------------------------------------------------------------------------
