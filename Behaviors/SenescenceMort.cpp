//---------------------------------------------------------------------------
#include "SenescenceMort.h"
//---------------------------------------------------------------------------
#include "MortalityOrg.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ModelMath.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clSenescenceMort::clSenescenceMort( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clMortalityBase( p_oSimManager )
{
  try
  {

    m_sNameString = "senescencemortshell";
    m_sXMLRoot = "Senescence";

    mp_fRandomAlpha = NULL;
    mp_fRandomBeta = NULL;
    mp_fDbhAtOnset = NULL;
    mp_fMortProb = NULL;
    m_iMaxDbh = 100;
    m_iNumTotalSpecies = 0;
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
    stcErr.sFunction = "clSenescenceMort::clSenescenceMort" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSenescenceMort::~clSenescenceMort()
{
  delete[] mp_fRandomAlpha;
  delete[] mp_fRandomBeta;
  delete[] mp_fDbhAtOnset;
  for ( int i = 0; i < m_iNumTotalSpecies; i++ )
    delete[] mp_fMortProb[i];
  delete[] mp_fMortProb;
}


////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clSenescenceMort::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int i; //loop counter

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

    //Declare the floatVal arrays and populate with the species to which this
    //behavior applies
    mp_fRandomAlpha = new floatVal[m_iNumBehaviorSpecies];
    mp_fRandomBeta = new floatVal[m_iNumBehaviorSpecies];
    mp_fDbhAtOnset = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fRandomAlpha[i].code = mp_iWhatSpecies[i];
      mp_fRandomBeta[i].code = mp_iWhatSpecies[i];
      mp_fDbhAtOnset[i].code = mp_iWhatSpecies[i];
    }

    //Capture the values from the parameter file

    //Random mortality alpha
    FillSpeciesSpecificValue( p_oElement, "mo_senescenceAlpha", "mo_saVal",
        mp_fRandomAlpha, m_iNumBehaviorSpecies, p_oPop, true );


    //Random mortality beta
    FillSpeciesSpecificValue( p_oElement, "mo_senescenceBeta", "mo_sbVal",
        mp_fRandomBeta, m_iNumBehaviorSpecies, p_oPop, true );


    //Dbh at onset of senescence
    FillSpeciesSpecificValue( p_oElement, "mo_dbhAtOnsetOfSenescence",
        "mo_daoosVal", mp_fDbhAtOnset, m_iNumBehaviorSpecies, p_oPop, true );


    //Max dbh - that of asymptotic max mortality
    FillSingleValue( p_oElement, "mo_dbhAtAsympMaxMort", & m_iMaxDbh, false );

    CalculateMortalityProbability();

    //Make sure that only saplings and adults were assigned to this behavior
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos->iType
          && clTreePopulation::adult != mp_whatSpeciesTypeCombos->iType )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSenescenceMort::DoShellSetup" ;
        stcErr.sMoreInfo = "This behavior cannot be applied to seedlings";
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
    stcErr.sFunction = "clSenescenceMort::DoShellSetup" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clSenescenceMort::DoMort(clTree * p_oTree, const float & fDiam, const short int & iSpecies )
{
  double fFracDbh, fIntDbh; //for splitting diam into integer and
  //fractional parts
  float fRandom = clModelMath::GetRand(), fDeathProb; //probability of death for this tree

  //Split the diameter into integer and fractional parts
  fFracDbh = modf( ( double )fDiam, & fIntDbh );

  if ( fIntDbh >= m_iMaxDbh - 1 )
    fDeathProb = mp_fMortProb[iSpecies] [m_iMaxDbh - 1];
  else
    fDeathProb = ( mp_fMortProb[iSpecies] [( int )fIntDbh] + fFracDbh
        * ( mp_fMortProb[iSpecies] [( int )fIntDbh + 1] - mp_fMortProb[iSpecies] [( int )fIntDbh] ) );


  //If the probability of death is greater than the random number, kill the
  //tree
  if ( fRandom < fDeathProb )
    return natural;
  else
    return notdead;
}


////////////////////////////////////////////////////////////////////////////
// CalculateMortalityProbability()
////////////////////////////////////////////////////////////////////////////
void clSenescenceMort::CalculateMortalityProbability()
{
  try
  {
    float fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(), fDeathProb;
    short int iSpecies, //the species that we're working with
         i, iDbh; //loop counters

    //Declare the array
    mp_fMortProb = new float * [m_iNumTotalSpecies];
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      mp_fMortProb[i] = new float[m_iMaxDbh];
      for ( iDbh = 0; iDbh < m_iMaxDbh; iDbh++ )
        mp_fMortProb[i] [iDbh] = 0;
    }

    //For each of the species that are covered by this behavior, calculate the
    //probability
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      iSpecies = mp_fRandomAlpha[i].code;

      for ( iDbh = 0; iDbh < m_iMaxDbh; iDbh++ )
      {

        fDeathProb = exp( mp_fRandomAlpha[i].val + mp_fRandomBeta[i].val * ( iDbh - mp_fDbhAtOnset[i].val ) )
             / ( 1 + exp( mp_fRandomAlpha[i].val + mp_fRandomBeta[i].val * ( iDbh - mp_fDbhAtOnset[i].val ) ) );


        mp_fMortProb[iSpecies] [iDbh] = 1 - pow( 1 - fDeathProb, fNumberYearsPerTimestep );
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
    stcErr.sFunction = "clSenescenceMort::CalculateMortalityProbability" ;
    throw( stcErr );
  }
}
