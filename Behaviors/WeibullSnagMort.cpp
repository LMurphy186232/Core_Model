//---------------------------------------------------------------------------
// WeibullSnagMort.cpp
//---------------------------------------------------------------------------
#include "WeibullSnagMort.h"
#include "MortalityOrg.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ModelMath.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clWeibullSnagMort::clWeibullSnagMort( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clMortalityBase( p_oSimManager )
{
  try
  {

    m_sNameString = "weibsnagmortshell";
    m_sXMLRoot = "WeibullSnagMortality";

    mp_fProbabilityOfDeath = NULL;
    mp_fAParameter = NULL;
    mp_fBParameter = NULL;
    mp_fSnagSizeClasses = NULL;
    mp_iAgeCodes = NULL;
    mp_iIndexes = NULL;

    m_iNumSizeClasses = 3;
    m_iMaxPrecalcAge = 30;
    m_iNumYearsPerTimestep = 0;

    //Versions
    m_fVersionNumber = 1.1;
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
    stcErr.sFunction = "clWeibullSnagMort::clWeibullSnagMort" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clWeibullSnagMort::~clWeibullSnagMort()
{
  int i, j; //loop counter

  if ( mp_fProbabilityOfDeath )
  {
    for ( i = 0; i < m_iNumSizeClasses; i++ )
    {
      for ( j = 0; j < m_iNumBehaviorSpecies; j++ )
      {
        delete[] mp_fProbabilityOfDeath[i] [j];
      }
      delete[] mp_fProbabilityOfDeath[i];
    }
    delete[] mp_fProbabilityOfDeath;
  }

  if ( mp_fAParameter )
  {
    for ( i = 0; i < m_iNumSizeClasses; i++ )
    {
      delete[] mp_fAParameter[i];
    }
    delete[] mp_fAParameter;
  }

  if ( mp_fBParameter )
  {
    for ( i = 0; i < m_iNumSizeClasses; i++ )
    {
      delete[] mp_fBParameter[i];
    }
    delete[] mp_fBParameter;
  }

  if ( mp_fSnagSizeClasses )
  {
    for ( i = 0; i < m_iNumSizeClasses; i++ )
    {
      delete[] mp_fSnagSizeClasses[i];
    }
    delete[] mp_fSnagSizeClasses;
  }

  delete[] mp_iAgeCodes;
  delete[] mp_iIndexes;
}


////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clWeibullSnagMort::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    m_iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    ValidateTypes();
    ReadParameterFile( p_oDoc );
    FillDeathProbArray();
    GetAgeCodes();

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
    modelErr stcErr; stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clWeibullSnagMort::DoShellSetup" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// ValidateTypes()
////////////////////////////////////////////////////////////////////////////
void clWeibullSnagMort::ValidateTypes()
{
  int i;
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    if ( clTreePopulation::snag != mp_whatSpeciesTypeCombos[i].iType )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Snag weibull mortality can only be applied to snags.";
      stcErr.sFunction = "clWeibullSnagMort::ValidateTypes" ;
      throw( stcErr );
    }
  }
}


////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clWeibullSnagMort::ReadParameterFile( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    doubleVal * p_fTempValues; //for getting species-specific values
    double fVal;
    short int i, j, iNumSpecies; //loop counter
    bool bFound;

    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Index array
    iNumSpecies = p_oPop->GetNumberOfSpecies();
    mp_iIndexes = new short int[iNumSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the arrays for holding the variables
    mp_fAParameter = new double * [m_iNumSizeClasses];
    mp_fBParameter = new double * [m_iNumSizeClasses];
    mp_fSnagSizeClasses = new double * [m_iNumSizeClasses];
    for ( i = 0; i < m_iNumSizeClasses; i++ )
    {
      mp_fAParameter[i] = new double[m_iNumBehaviorSpecies];
      mp_fBParameter[i] = new double[m_iNumBehaviorSpecies];
      mp_fSnagSizeClasses[i] = new double[m_iNumBehaviorSpecies];

      //Initialize size class values to -1 so we know when we've read them
      for (j = 0; j < m_iNumBehaviorSpecies; j++) {
        mp_fSnagSizeClasses[i][j] = -1;
      }
    }

    //Capture the values from the parameter file
    //Weibull parameter "a" for snag age class 1
    FillSpeciesSpecificValue( p_oElement, "mo_snag1WeibullA", "mo_s1waVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fAParameter[0] [mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Weibull parameter "a" for snag age class 2
    FillSpeciesSpecificValue( p_oElement, "mo_snag2WeibullA", "mo_s2waVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fAParameter[1] [mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Weibull parameter "a" for snag age class 3
    FillSpeciesSpecificValue( p_oElement, "mo_snag3WeibullA", "mo_s3waVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fAParameter[2] [mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Weibull parameter "b" for snag age class 1
    FillSpeciesSpecificValue( p_oElement, "mo_snag1WeibullB", "mo_s1wbVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBParameter[0] [mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Weibull parameter "b" for snag age class 2
    FillSpeciesSpecificValue( p_oElement, "mo_snag2WeibullB", "mo_s2wbVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBParameter[1] [mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Weibull parameter "b" for snag age class 3
    FillSpeciesSpecificValue( p_oElement, "mo_snag3WeibullB", "mo_s3wbVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBParameter[2] [mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Snag size class 1 - check for the species-specific version first, but
    //don't require it in case they used the old non-species-specific version
    //Pre-fill the values in temp so we know if we got them
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].val = -1;
    FillSpeciesSpecificValue( p_oElement, "mo_snagSizeClass1DBH", "mo_sc1dVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, false );
    //Was the value present as a species-specific?
    bFound = false;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val >= 0) {
        bFound = true;
        break;
      }
    }
    if (bFound) {
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fSnagSizeClasses[0][mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    } else {
      //The value wasn't present as a species-specific - so find it as a
      //single value and transfer the single value to all species
      FillSingleValue( p_oElement, "mo_snagSizeClass1", & fVal, true );
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        mp_fSnagSizeClasses[0][i] = fVal;
      }
    }

    //Snag size class 2 - check for the species-specific version first, but
    //don't require it in case they used the old non-species-specific version
    //Pre-fill the values in temp so we know if we got them
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].val = -1;
    FillSpeciesSpecificValue( p_oElement, "mo_snagSizeClass2DBH", "mo_sc2dVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, false );
    //Was the value present as a species-specific?
    bFound = false;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val >= 0) {
        bFound = true;
        break;
      }
    }
    if (bFound) {
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fSnagSizeClasses[1][mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    } else {
      //The value wasn't present as a species-specific - so find it as a
      //single value and transfer the single value to all species
      FillSingleValue( p_oElement, "mo_snagSizeClass2", & fVal, true );
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        mp_fSnagSizeClasses[1][i] = fVal;
      }
    }

    //Infinity-like value for size class 3
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      mp_fSnagSizeClasses[2][i] = 100000;
    }

    //Make sure the age classes are positive numbers that don't overlap
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      if ( mp_fSnagSizeClasses[0][i] < 0 || mp_fSnagSizeClasses[1][i] < 0 || mp_fSnagSizeClasses[1][i] < mp_fSnagSizeClasses[0][i] )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clWeibullSnagMort::ReadParameterFile" ;
        stcErr.sMoreInfo = "Snag size classes must be positive and cannot overlap.";
        delete[] p_fTempValues;
        throw( stcErr );
      }
    }

    //Make sure that if any values for a are negative, the value for the
    //corresponding b is a whole number
    for (i = 0; i < m_iNumSizeClasses; i++) {
      for (j = 0; j < m_iNumBehaviorSpecies; j++) {
         if (0 > mp_fAParameter[i][j] && (0 >= mp_fBParameter[i][j] || mp_fBParameter[i][j] - floor(mp_fBParameter[i][j]) != 0)) {
           modelErr stcErr;
           stcErr.iErrorCode = BAD_DATA;
           stcErr.sFunction = "clWeibullSnagMort::ReadParameterFile" ;
           stcErr.sMoreInfo = "All values for \"a\" must be a positive number.";
           delete[] p_fTempValues;
           throw( stcErr );
         }
      }
    }


    delete[] p_fTempValues; p_fTempValues = NULL;
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
    stcErr.sFunction = "clWeibullSnagMort::ReadParameterFile" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetAgeCodes()
//////////////////////////////////////////////////////////////////////////////
void clWeibullSnagMort::GetAgeCodes()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    char cLabel[] = "Age";
    short int i; //loop counter

    //Declare and initialize the return codes array
    mp_iAgeCodes = new short int[m_iNumBehaviorSpecies];

    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_iAgeCodes[i] = p_oPop->GetIntDataCode( cLabel, mp_iWhatSpecies[i], clTreePopulation::snag );

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
    stcErr.sFunction = "clWeibullSnagMort::GetAgeCodes" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// FillDeathProbArray()
//////////////////////////////////////////////////////////////////////////////
void clWeibullSnagMort::FillDeathProbArray()
{
  try
  {
    int i, j, k;
    mp_fProbabilityOfDeath = new float * * [m_iNumSizeClasses];
    for ( i = 0; i < m_iNumSizeClasses; i++ )
    {
      mp_fProbabilityOfDeath[i] = new float * [m_iNumBehaviorSpecies];
      for ( j = 0; j < m_iNumBehaviorSpecies; j++ )
      {
        mp_fProbabilityOfDeath[i] [j] = new float[m_iMaxPrecalcAge];
        for ( k = 0; k < m_iMaxPrecalcAge; k++ )
        {
          mp_fProbabilityOfDeath[i] [j] [k] = CalculateDeathProbability(i, mp_iWhatSpecies[j], m_iNumYearsPerTimestep * k);
        }
      }
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
    stcErr.sFunction = "clWeibullSnagMort::CalculateDeathProbabilities" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalculateDeathProbability()
////////////////////////////////////////////////////////////////////////////
float clWeibullSnagMort::CalculateDeathProbability( int iSizeClass, int iSpecies, int iAge )
{

  float fMinValueAllowed = 1.0E-5,
        fMaxValueAllowed = 1 - fMinValueAllowed,
        fMinExp = -11.51,
        fFunc1, fFunc2, fProb;

  if (0 == iAge) return 0;

  //Get the value of the function at this time
  fFunc1 = -( pow( mp_fAParameter[iSizeClass] [mp_iIndexes[iSpecies]] * iAge,
       mp_fBParameter[iSizeClass] [mp_iIndexes[iSpecies]] ) );
  if (fFunc1 < fMinExp) fFunc1 = 0;
  else fFunc1 = exp( fFunc1 );

  //Get the value of the function at the previous timestep
  iAge -= m_iNumYearsPerTimestep;
  if ( iAge > 0 )
  {
    fFunc2 = -( pow( mp_fAParameter[iSizeClass] [mp_iIndexes[iSpecies]] * iAge,
         mp_fBParameter[iSizeClass] [mp_iIndexes[iSpecies]] ) );
    if (fFunc2 < fMinExp) fFunc2 = 0;
    else fFunc2 = exp( fFunc2 );
  }
  else
  {
    fFunc2 = 1;
  }

  //Correct for exceedlingly large or small function values
  if ( fFunc2 > fMinValueAllowed )
  {
    fProb = ( 1 - ( fFunc1 / fFunc2 ) );
  }
  else
  {
    fProb = 1;
  }


  if ( fProb < fMinValueAllowed )
  {
    fProb = 0;
  }
  else if ( fProb > fMaxValueAllowed )
  {
    fProb = 1;
  }

  return fProb;
}

////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clWeibullSnagMort::DoMort( clTree * p_oTree, const float & fDbh, const short int & iSpecies )
{
  float fRandom = clModelMath::GetRand(), //random number
      fProb; //death probability
  int iAge, //snag age in years
  iAgeInTS, //snag age in timesteps
  iSizeClass = m_iNumSizeClasses - 1, //snag size class
  i;

  //Get the age in timesteps
  p_oTree->GetValue( mp_iAgeCodes[mp_iIndexes[iSpecies]], & iAge );

  //Get the size class
  for ( i = 0; i < m_iNumSizeClasses; i++ )
  {
    if ( fDbh <= mp_fSnagSizeClasses[i][mp_iIndexes[iSpecies]] )
    {
      iSizeClass = i;
      break;
    }
  }

  //Get the function values
  if ( iAge < m_iMaxPrecalcAge )
  {
    //We've already calculated this age
    iAgeInTS = iAge / m_iNumYearsPerTimestep;
    fProb = mp_fProbabilityOfDeath[iSizeClass] [mp_iIndexes[iSpecies]] [iAgeInTS];

  }
  else
  {
    //This snag is older than what we've calculated for already - so
    //calculate the probability directly
    fProb = CalculateDeathProbability(iSizeClass, iSpecies, iAge);

  }

  //Roll the dice
  if ( fRandom <= fProb)
    return natural;

  return notdead;
}
