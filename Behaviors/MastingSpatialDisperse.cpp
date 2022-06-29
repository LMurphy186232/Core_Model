//---------------------------------------------------------------------------
#include "MastingSpatialDisperse.h"
//---------------------------------------------------------------------------
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
clMastingSpatialDisperse::clMastingSpatialDisperse( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clDisperseBase( p_oSimManager)
{
  try
  {
    m_sNameString = "MastingSpatialDisperse";
    m_sXMLRoot = "MastingSpatialDisperse";

    mp_fDbhForReproduction = NULL;
    mp_iTimestepsSinceLastMast = NULL;
    mp_fSeedCDF = NULL;
    mp_fStrMean = NULL;
    mp_fStrStdDev = NULL;
    mp_fBeta = NULL;
    mp_fMastCDF = NULL;
    mp_fFractionParticipating = NULL;
    mp_bIsUsed = NULL;
    mp_fFecundity = NULL;
    mp_iIndexes = NULL;
    mp_iGroup = NULL;
    mp_iWhatPDFForSTR = NULL;
    mp_iWhatFunction = NULL;
    mp_bDrawSTRPerSpecies = NULL;
    mp_iEvent = NULL;
    m_cQuery = NULL;
    mp_GetSeeds = NULL;

    m_iMaxDistance = 0;
    m_iMaxTimesteps = 0;
    m_iNumYearsPerTimestep = 0;

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
    stcErr.sFunction = "clMastingSpatialDisperse::clMastingSpatialDisperse" ;
    throw( stcErr );
  }
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
clMastingSpatialDisperse::~clMastingSpatialDisperse()
{
  short int i, j; //loop counters

  if (mp_fSeedCDF) {
    for (i = 0; i < numevents; i++) {
      for (j = 0; j < m_iNumBehaviorSpecies; j++)
        delete[] mp_fSeedCDF[i][j];
      delete[] mp_fSeedCDF[i];
    }
    delete[] mp_fSeedCDF;
  }

  if (mp_fStrMean) {
    for (i = 0; i < numevents; i++)
      delete[] mp_fStrMean[i];
    delete[] mp_fStrMean;
  }

  if (mp_fStrStdDev) {
    for (i = 0; i < numevents; i++)
      delete[] mp_fStrStdDev[i];
    delete[] mp_fStrStdDev;
  }

  if (mp_fBeta) {
    for (i = 0; i < numevents; i++)
      delete[] mp_fBeta[i];
    delete[] mp_fBeta;
  }

  if (mp_fMastCDF) {
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      delete[] mp_fMastCDF[i];
    delete[] mp_fMastCDF;
  }

  if (mp_fFractionParticipating) {
    for (i = 0; i < numevents; i++)
      delete[] mp_fFractionParticipating[i];
    delete[] mp_fFractionParticipating;
  }

  if ( mp_bIsUsed ) {
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_bIsUsed[i];
    delete[] mp_bIsUsed;
  }

  delete[] mp_fDbhForReproduction;
  delete[] mp_fFecundity;
  delete[] mp_iIndexes;
  delete[] mp_iGroup;
  delete[] mp_iWhatPDFForSTR;
  delete[] mp_iWhatFunction;
  delete[] mp_bDrawSTRPerSpecies;
  delete[] mp_iEvent;
  delete[] m_cQuery;
  delete[] mp_GetSeeds;
  delete[] mp_iTimestepsSinceLastMast;
}


///////////////////////////////////////////////////////////////////////////////
// DoShellSetup
///////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    int i;
    m_iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    mp_iIndexes = new short int[m_iTotalSpecies];
    for (i = 0; i < m_iTotalSpecies; i++) mp_iIndexes[i] = -1;

    //Make the list of indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    mp_iTimestepsSinceLastMast = new int[m_iNumBehaviorSpecies];
    for (int i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_iTimestepsSinceLastMast[i] = 1;

    GetParameterFileData( p_oDoc, p_oPop );
    CalcMastCDF( p_oDoc );
    CalcSeedCDF( p_oDoc, p_oPop );
    FormatQueryString( p_oPop );
    PopulateUsedTable( p_oPop );
    SetGetSeedsFunctionPointers();
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
    stcErr.sFunction = "clMastingSpatialDisperse::DoShellSetup" ;
    throw( stcErr );
  }
}


///////////////////////////////////////////////////////////////////////////////
// PopulateUsedTable()
///////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::PopulateUsedTable( clTreePopulation * p_oPop )
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
           stcErr.sFunction = "clSpatialDisperse::PopulateUsedTable" ;
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
void clMastingSpatialDisperse::GetParameterFileData( xercesc::DOMDocument * p_oDoc, clTreePopulation * p_oPop)
{
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  intVal * p_iTemp = NULL; //for getting species-specific values
  try {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int i; //loop counter

    //Declare arrays
    mp_fStrMean = new double*[numevents];
    mp_fStrStdDev = new double*[numevents];
    mp_fBeta = new double*[numevents];
    mp_fFractionParticipating = new double*[numevents];
    for (i = 0; i < numevents; i++) {
      mp_fStrMean[i] = new double[m_iNumBehaviorSpecies];
      mp_fStrStdDev[i] = new double[m_iNumBehaviorSpecies];
      mp_fBeta[i] = new double[m_iNumBehaviorSpecies];
      mp_fFractionParticipating[i] = new double[m_iNumBehaviorSpecies];
    }
    mp_fDbhForReproduction = new double[m_iNumBehaviorSpecies];
    mp_iGroup = new short int[m_iNumBehaviorSpecies];
    mp_iWhatPDFForSTR = new pdf[m_iNumBehaviorSpecies];
    mp_iWhatFunction = new function[m_iNumBehaviorSpecies];
    mp_bDrawSTRPerSpecies = new bool[m_iNumBehaviorSpecies];
    mp_iEvent = new mastEvent[m_iNumBehaviorSpecies];

    //Which species use which function - set up our temp int array and pre-load
    //with this behavior's species
    p_iTemp = new intVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_iTemp[i].code = mp_iWhatSpecies[i];

    p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTemp[i].code = mp_iWhatSpecies[i];

    //Minimum DBH for reproduction
    FillSpeciesSpecificValue( p_oElement, "di_minDbhForReproduction", "di_mdfrVal", p_fTemp,
       m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fDbhForReproduction[i] = p_fTemp[i].val;

    FillSpeciesSpecificValue( p_oElement, "di_canopyFunction", "di_cfVal",
                            p_iTemp, m_iNumBehaviorSpecies, p_oPop, true );

    //Transfer the values over - if any are not a valid function enum number,
    //throw an error
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( p_iTemp[i].val != weibull && p_iTemp[i].val != lognormal )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingSpatialDisperse::GetParameterFileData" ;
        std::stringstream s;
        s << "Invalid disperse function code " << p_iTemp[i].val;
        stcErr.sMoreInfo = s.str();
        throw( stcErr );
      }
      else
      {
        mp_iWhatFunction[mp_iIndexes[p_iTemp[i].code]] = (function)p_iTemp[i].val;
      }

    //Non-masting beta
    FillSpeciesSpecificValue( p_oElement, "di_spatialBeta", "di_sbVal", p_fTemp,
                            m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fBeta[nonmast][i] = p_fTemp[i].val;
      if (p_fTemp[i].val > 25) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingSpatialDisperse::GetParameterFileData" ;
        stcErr.sMoreInfo = "One or more beta values is too large and will cause math errors.";
        throw( stcErr );
      }
    }

    //Masting beta
    FillSpeciesSpecificValue( p_oElement, "di_mastingBeta", "di_mbVal", p_fTemp,
                            m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fBeta[mast][i] = p_fTemp[i].val;
      if (p_fTemp[i].val > 25) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingSpatialDisperse::GetParameterFileData" ;
        stcErr.sMoreInfo = "One or more beta values is too large and will cause math errors.";
        throw( stcErr );
      }
    }

    //Non-masting mean STR
    FillSpeciesSpecificValue( p_oElement, "di_spatialSTR", "di_sstrVal", p_fTemp,
                            m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStrMean[nonmast][i] = p_fTemp[i].val;

    //Masting mean STR
    FillSpeciesSpecificValue( p_oElement, "di_mastingSTR", "di_mstrVal", p_fTemp,
                            m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStrMean[mast][i] = p_fTemp[i].val;

    //Non-masting STR std dev - we might not need it but read it anyway
    FillSpeciesSpecificValue( p_oElement, "di_spatialSTRStdDev", "di_sstrsdVal",
                            p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStrStdDev[nonmast][i] = p_fTemp[i].val;

    //Masting STR std dev - we might not need it but read it anyway
    FillSpeciesSpecificValue( p_oElement, "di_mastingSTRStdDev", "di_mstrsdVal",
                            p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStrStdDev[mast][i] = p_fTemp[i].val;

    //What kind of STR draw to do
    FillSpeciesSpecificValue( p_oElement, "di_mastSTRPDF", "di_mstrpdfVal",
                            p_iTemp, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (deterministic_pdf == p_iTemp[i].val)
        mp_iWhatPDFForSTR[i] = deterministic_pdf;
      else if (normal_pdf == p_iTemp[i].val)
        mp_iWhatPDFForSTR[i] = normal_pdf;
      else if (lognormal_pdf == p_iTemp[i].val)
        mp_iWhatPDFForSTR[i] = lognormal_pdf;
      else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingSpatialDisperse::GetParameterFileData" ;
        std::stringstream s;
        s << "Invalid STR pdf code " << p_iTemp[i].val;
        stcErr.sMoreInfo = s.str();
        throw( stcErr );
      }
    }

    //Non-mast fraction participating
    FillSpeciesSpecificValue( p_oElement, "di_spatialPropParticipating",
                              "di_sppVal", p_fTemp, m_iNumBehaviorSpecies,
                              p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if (p_fTemp[i].val < 0 || p_fTemp[i].val > 1) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingSpatialDisperse::GetParameterFileData";
        stcErr.sMoreInfo = "Fraction participating must be between 0 and 1.";
        throw( stcErr );
      }
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fFractionParticipating[nonmast][i] = p_fTemp[i].val;

    //Mast fraction participating
    FillSpeciesSpecificValue( p_oElement, "di_mastPropParticipating",
                              "di_mppVal", p_fTemp, m_iNumBehaviorSpecies,
                              p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if (p_fTemp[i].val < 0 || p_fTemp[i].val > 1) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingSpatialDisperse::GetParameterFileData" ;
        stcErr.sMoreInfo = "Fraction participating must be between 0 and 1.";
        throw( stcErr );
      }
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fFractionParticipating[mast][i] = p_fTemp[i].val;

    //Group affiliation for synchrony
    FillSpeciesSpecificValue( p_oElement, "di_mastGroup", "di_mgVal", p_iTemp,
                              m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iGroup[i] = p_iTemp[i].val;

    //Whether to draw STR per species or tree
    FillSpeciesSpecificValue( p_oElement, "di_mastDrawPerSpecies", "di_mdpsVal",
                              p_iTemp, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if (1 == p_iTemp[i].val)
        mp_bDrawSTRPerSpecies[i] = true;
      else
        mp_bDrawSTRPerSpecies[i] = false;

    delete[] p_iTemp;
    delete[] p_fTemp;
  }
  catch ( modelErr & err )
  {
    delete[] p_iTemp;
    delete[] p_fTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_iTemp;
    delete[] p_fTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_iTemp;
    delete[] p_fTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMastingSpatialDisperse::GetParameterFileData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalcSeedCDF
///////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::CalcSeedCDF(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop)
{
  float *p_fDispersalX0 = new float[m_iNumBehaviorSpecies],
        *p_fThetaXb = new float[m_iNumBehaviorSpecies];
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  try {
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    float fXPlotLength = p_oPlot->GetXPlotLength(),
          fYPlotLength = p_oPlot->GetYPlotLength();
    int iNumFunctionSpecies, //number of species using a given function
        i, j, k;

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

    mp_fSeedCDF = new float**[numevents];
    for (i = 0; i < numevents; i++) {
      mp_fSeedCDF[i] = new float*[m_iNumBehaviorSpecies];
      for (j = 0; j < m_iNumBehaviorSpecies; j++) {
        mp_fSeedCDF[i][j] = new float[m_iMaxDistance];
        for (k = 0; k < m_iMaxDistance; k++) {
          mp_fSeedCDF[i][j][k] = 0;
        }
      }
    }

    //****************************************
    // Non-masting
    //****************************************
    //Read the parameters for calculating seed CDF - keeping track of which
    //species use weibull and which use lognormal
    //Those who use weibull:
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( weibull == mp_iWhatFunction[i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    p_fTemp = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( weibull == mp_iWhatFunction[i] )
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
          stcErr.sFunction = "clMastingSpatialDisperse::CalcSeedCDF" ;
          stcErr.sMoreInfo = "One or more weibull theta values is too large and will cause math errors.";
          throw( stcErr );
        }
      }
    } //end of if (iNumFunctionSpecies > 0) {

    //Those who use lognormal:
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( lognormal == mp_iWhatFunction[i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTemp;
    p_fTemp = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( lognormal == mp_iWhatFunction[i] )
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
      CalculateProbabilityDistribution( mp_fSeedCDF[nonmast][i], m_iMaxDistance,
                              mp_iWhatFunction[mp_iIndexes[mp_iWhatSpecies[i]]],
                              p_fDispersalX0[mp_iIndexes[mp_iWhatSpecies[i]]],
                              p_fThetaXb[mp_iIndexes[mp_iWhatSpecies[i]]]);
    }

    //****************************************
    // Masting
    //****************************************
    //Read the parameters for calculating seed CDF - keeping track of which
    //species use weibull and which use lognormal
    //Those who use weibull:
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( weibull == mp_iWhatFunction[i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTemp;
    p_fTemp = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( weibull == mp_iWhatFunction[i] )
      {
        p_fTemp[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {
      //Dispersal
      FillSpeciesSpecificValue( p_oElement, "di_weibullMastingDispersal",
                                "di_wmdVal", p_fTemp, iNumFunctionSpecies,
                                p_oPop, true );
      //Transfer to array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        p_fDispersalX0[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Theta
      FillSpeciesSpecificValue( p_oElement, "di_weibullMastingTheta", "di_wmtVal",
                                p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to array
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        p_fThetaXb[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
        if (p_fTemp[i].val >= 50) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clMastingSpatialDisperse::CalcSeedCDF" ;
          stcErr.sMoreInfo = "One or more weibull theta values is too large and will cause math errors.";
          throw( stcErr );
        }
      }
    } //end of if (iNumFunctionSpecies > 0) {

    //Those who use lognormal:
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( lognormal == mp_iWhatFunction[i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTemp;
    p_fTemp = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( lognormal == mp_iWhatFunction[i] )
      {
        p_fTemp[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {
      //X0
      FillSpeciesSpecificValue( p_oElement, "di_lognormalMastingX0", "di_lmx0Val",
                                p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        p_fDispersalX0[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Xb
      FillSpeciesSpecificValue( p_oElement, "di_lognormalMastingXb", "di_lmxbVal",
                                p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        p_fThetaXb[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
    }

    //Calculate cumulative probability arrays
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      CalculateProbabilityDistribution( mp_fSeedCDF[mast][i], m_iMaxDistance,
                              mp_iWhatFunction[mp_iIndexes[mp_iWhatSpecies[i]]],
                              p_fDispersalX0[mp_iIndexes[mp_iWhatSpecies[i]]],
                              p_fThetaXb[mp_iIndexes[mp_iWhatSpecies[i]]]);
    }

    delete[] p_fDispersalX0;
    delete[] p_fThetaXb;
    delete[] p_fTemp;
  }
  catch ( modelErr & err )
  {
    delete[] p_fDispersalX0;
    delete[] p_fThetaXb;
    delete[] p_fTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fDispersalX0;
    delete[] p_fThetaXb;
    delete[] p_fTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fDispersalX0;
    delete[] p_fThetaXb;
    delete[] p_fTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMastingSpatialDisperse::GetParameterFileData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalculateProbabilityDistribution()
/////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::CalculateProbabilityDistribution( float * p_fProbArray,
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
void clMastingSpatialDisperse::AddSeeds()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTreeSearch * p_oAllTrees; //search object for getting all trees
    clTree * p_oTree; //for working with a single tree
    float fDbh; //tree's dbh
    short int iSp, iType, //species and type of a given tree
              iEvent,
              i;

    //Get the masting decisions
    DecideMast();

    //Calculate fecundities where possible
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      iEvent = mp_iEvent[i];
      if (deterministic_pdf == mp_iWhatPDFForSTR[i]) {
        mp_fFecundity[i] = m_iNumYearsPerTimestep * mp_fStrMean[iEvent][i] / pow(30, mp_fBeta[iEvent][i]);
      } else if (mp_bDrawSTRPerSpecies[i]) {
        float fSTR;
        if (normal_pdf == mp_iWhatPDFForSTR[i]) {
          fSTR = mp_fStrMean[iEvent][i] +
               clModelMath::NormalRandomDraw(mp_fStrStdDev[iEvent][i]);
        } else {
          fSTR = clModelMath::LognormalRandomDraw(mp_fStrMean[iEvent][i],
                                                  mp_fStrStdDev[iEvent][i]);
        }
        mp_fFecundity[i] = m_iNumYearsPerTimestep * fSTR / pow(30, mp_fBeta[iEvent][i]);
      }
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
      if ( mp_bIsUsed[iSp] [iType] )
      {

        //Get the tree's dbh
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iType ), & fDbh );

        //If the tree is larger than the minimum size for reproduction,
        //and a random number lets it participate, let it reproduce
        if ( fDbh >= mp_fDbhForReproduction[mp_iIndexes[iSp]] &&
             clModelMath::GetRand() <
              mp_fFractionParticipating[mp_iEvent[mp_iIndexes[iSp]]][mp_iIndexes[iSp]] )
          DisperseOneParentSeeds( p_oTree, p_oPop, p_oPlot, fDbh );

      }

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
    stcErr.sFunction = "clSpatialDisperse::AddSeeds" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// DisperseOneParentSeeds
//////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::DisperseOneParentSeeds( clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, float fDbh )
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
    fNumSeeds, //number of seeds produced by the tree
    f; //loop counter
    int iDistanceCounter = 0; //used to walk out the cumulative probability array
    short int iSp = p_oTree->GetSpecies(), //reproducing tree's species
    iType = p_oTree->GetType(), //reproducing tree's type
    iSpIndex = mp_iIndexes[iSp],
    iEvent = mp_iEvent[iSpIndex];

    //Get the X and Y coords of the tree
    p_oTree->GetValue( p_oPop->GetXCode( iSp, iType ), & fX );
    p_oTree->GetValue( p_oPop->GetYCode( iSp, iType ), & fY );

    //Calculate number of seeds produced, based on species
    //fecundity and the dbh of the tree
    fNumSeeds = (* this.*mp_GetSeeds[iSpIndex])( fDbh, iSp );

    //Disperse the seeds produced
    for ( f = 0; f < fNumSeeds; f++ )
    {
      //Use the cumulative probability array to determine how far away from the
      //parent tree the seed lands - use a random number and find the first
      //probability array bucket with a value greater than the random
      iDistanceCounter = 0;
      fRand = clModelMath::GetRand();
      while ( fRand > mp_fSeedCDF[iEvent][iSpIndex][iDistanceCounter] )
        iDistanceCounter++;

      //Get the value in the array bucket before the target one
      fPrevBucket = ( iDistanceCounter == 0 ) ? 0.0 : mp_fSeedCDF[iEvent][iSpIndex][iDistanceCounter - 1];

      //Calculate the distance at which the seed will land
      fDistance = ( iDistanceCounter + ( fRand - fPrevBucket )
           / ( mp_fSeedCDF[iEvent][iSpIndex][iDistanceCounter] - fPrevBucket ) );

      //Get a random direction to pitch the seed
      fRand = clModelMath::GetRand();
      fAngle = 2.0 * M_PI * fRand;
      fAngle = std::max(2.0 * M_PI * fRand, (2.0 * M_PI)-0.00001);

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
    stcErr.sFunction = "clSpatialDisperse::DisperseOneParentSeeds" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalcMastCDF
///////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::CalcMastCDF( xercesc::DOMDocument * p_oDoc )
{
  float *p_fA = new float[m_iNumBehaviorSpecies],
        *p_fB = new float[m_iNumBehaviorSpecies];
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  try {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    float fNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(),
          fProb;
    int iNumberRunTimesteps = mp_oSimManager->GetNumberOfTimesteps(),
        iSpeciesMax,
        i, j; //loop counter

    //Set up our temp array and pre-load with this behavior's species
    p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTemp[i].code = mp_iWhatSpecies[i];

    //CDF "a" value
    FillSpeciesSpecificValue( p_oElement, "di_mastCDFA", "di_mcdfaVal", p_fTemp,
         m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if (p_fTemp[i].val == 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingSpatialDisperse::CalcMastCDF" ;
        stcErr.sMoreInfo = "Mast \"a\" cannot be 0.";
        throw( stcErr );
      }
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fA[i] = p_fTemp[i].val;

    //CDF "b" value
    FillSpeciesSpecificValue( p_oElement, "di_mastCDFB", "di_mcdfbVal", p_fTemp,
         m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fB[i] = p_fTemp[i].val;

    //Find the max number of years between mast events for all species
    m_iMaxTimesteps = 0;
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      iSpeciesMax = 0;
      fProb = 0;
      while (iSpeciesMax < iNumberRunTimesteps && fProb < 0.9999) {
        iSpeciesMax++;
        fProb = 1/(1 + pow(((iSpeciesMax*fNumYearsPerTimestep) /
                          p_fA[i]), p_fB[i]));
      }
      if (m_iMaxTimesteps < iSpeciesMax)
        m_iMaxTimesteps = iSpeciesMax;
    }
    //Trap to avoid overflow
    if (m_iMaxTimesteps == 1)
      for (i = 0; i < m_iNumBehaviorSpecies; i++) mp_iTimestepsSinceLastMast[i] = 0;

    //Now declare the CDF array and populate it
    mp_fMastCDF = new float*[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      mp_fMastCDF[i] = new float[m_iMaxTimesteps];
      mp_fMastCDF[i][0] = 0;
      for (j = 1; j < m_iMaxTimesteps; j++) {
        mp_fMastCDF[i][j] = 1/(1 + pow(((j*fNumYearsPerTimestep) / p_fA[i]), p_fB[i]));
        if (mp_fMastCDF[i][j] > 1)
          mp_fMastCDF[i][j] = 1;
        if (mp_fMastCDF[i][j] < 0)
          mp_fMastCDF[i][j] = 0;
      }
      mp_fMastCDF[i][m_iMaxTimesteps-1] = 1;
    }

    delete[] p_fTemp;
    delete[] p_fA;
    delete[] p_fB;

  }
  catch ( modelErr & err )
  {
    delete[] p_fTemp;
    delete[] p_fA;
    delete[] p_fB;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTemp;
    delete[] p_fA;
    delete[] p_fB;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTemp;
    delete[] p_fA;
    delete[] p_fB;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSpatialDisperse::CalcMastCDF" ;
    throw( stcErr );
  }
}



//////////////////////////////////////////////////////////////////////////////
// FormatQueryString
///////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::FormatQueryString(clTreePopulation * p_oPop) {
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
// SetGetSeedsFunctionPointers
///////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::SetGetSeedsFunctionPointers() {

  int i;
  bool bDeclareFecundity = false;

  mp_GetSeeds = new Ptr2GetNumberOfSeeds[m_iNumBehaviorSpecies];

  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    if (deterministic_pdf == mp_iWhatPDFForSTR[i] || mp_bDrawSTRPerSpecies[i]) {
      bDeclareFecundity = true;
      mp_GetSeeds[i] = & clMastingSpatialDisperse::GetNumberOfSeedsNoDraw;
    } else if (normal_pdf == mp_iWhatPDFForSTR[i]) {
      mp_GetSeeds[i] = & clMastingSpatialDisperse::GetNumberOfSeedsDrawNormal;
    } else {
      mp_GetSeeds[i] = & clMastingSpatialDisperse::GetNumberOfSeedsDrawLognormal;
    }
  }

  //If there are any deterministic STRs, calculate fecundity now
  if (bDeclareFecundity) {
    mp_fFecundity = new float[m_iNumBehaviorSpecies];
  }
}


//////////////////////////////////////////////////////////////////////////////
// DecideMast
///////////////////////////////////////////////////////////////////////////////
void clMastingSpatialDisperse::DecideMast() {
  int i, j;
  mastEvent iEvent;

  //Set all the existing values to "numevents" so we can tell which have
  //been completed
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_iEvent[i] = numevents;
  }

  //Make the decision for each species
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {

    //Only continue if the decision hasn't already been made on behalf of a
    //group
    if (numevents == mp_iEvent[i]) {

      if (clModelMath::GetRand() < mp_fMastCDF[i][mp_iTimestepsSinceLastMast[i]])
        iEvent = mast;
      else
        iEvent = nonmast;

      //Share this event with all members of the species's group (including
      //itself)
      for (j = i; j < m_iNumBehaviorSpecies; j++) {
        if (mp_iGroup[j] == mp_iGroup[i]) {
          mp_iEvent[j] = iEvent;
        }
      }
    }
  }

  //Set the time-since-last-mast counters
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    if (mast == mp_iEvent[i])
      mp_iTimestepsSinceLastMast[i] = 1; //will be one next timestep
    else
      mp_iTimestepsSinceLastMast[i]++;
    if (mp_iTimestepsSinceLastMast[i] == m_iMaxTimesteps)
      mp_iTimestepsSinceLastMast[i] = m_iMaxTimesteps-1;
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetNumberOfSeedsNoDraw
///////////////////////////////////////////////////////////////////////////////
float clMastingSpatialDisperse::GetNumberOfSeedsNoDraw(const float &fDbh, const short int &iSp) {
  return clModelMath::RandomRound( mp_fFecundity[mp_iIndexes[iSp]]
       * pow( fDbh, mp_fBeta[mp_iEvent[mp_iIndexes[iSp]]][mp_iIndexes[iSp]] )
       * m_iNumYearsPerTimestep );
}


//////////////////////////////////////////////////////////////////////////////
// GetNumberOfSeedsDrawNormal
///////////////////////////////////////////////////////////////////////////////
float clMastingSpatialDisperse::GetNumberOfSeedsDrawNormal(const float &fDbh, const short int &iSp) {
  float fSTR = mp_fStrMean[mp_iEvent[mp_iIndexes[iSp]]][mp_iIndexes[iSp]] +
               clModelMath::NormalRandomDraw(mp_fStrStdDev[mp_iEvent[mp_iIndexes[iSp]]][mp_iIndexes[iSp]]);
  return fSTR * m_iNumYearsPerTimestep *
        pow( (fDbh / 30), mp_fBeta[mp_iEvent[mp_iIndexes[iSp]]][mp_iIndexes[iSp]]);
}

//////////////////////////////////////////////////////////////////////////////
// GetNumberOfSeedsDrawLognormal
///////////////////////////////////////////////////////////////////////////////
float clMastingSpatialDisperse::GetNumberOfSeedsDrawLognormal(const float &fDbh, const short int &iSp) {
  float fSTR = clModelMath::LognormalRandomDraw(
                mp_fStrMean[mp_iEvent[mp_iIndexes[iSp]]][mp_iIndexes[iSp]],
                mp_fStrStdDev[mp_iEvent[mp_iIndexes[iSp]]][mp_iIndexes[iSp]]);
  return fSTR * m_iNumYearsPerTimestep *
        pow( (fDbh / 30), mp_fBeta[mp_iEvent[mp_iIndexes[iSp]]][mp_iIndexes[iSp]]);
}


/*/ ////////////////////////////////////////////////////////////////////////////
WriteCumProbArray
/////////////////////////////////////////////////////////////////////////////*/
/*void clMastingSpatialDisperse::WriteCumProbArray() { fstream matrices; int iMaxDistance = mp_oDisperseOrg->GetMaxDistance();
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
