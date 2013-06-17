//---------------------------------------------------------------------------
#include "BrowsedStochasticMortality.h"
#include "MortalityOrg.h"
#include "ModelMath.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clBrowsedStochasticMortality::clBrowsedStochasticMortality( clSimManager * p_oSimManager ) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clMortalityBase( p_oSimManager )
{
  try
  {
    m_sNameString = "browsed stochastic mortshell";
    m_sXMLRoot = "BrowsedStochasticMortality";

    mp_fBrowsedMortProb = NULL;
    mp_fUnbrowsedMortProb = NULL;
    mp_iBrowsedCodes = NULL;

    m_iNumTotalSpecies = 0;
    m_iNumSpecies = 0;
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
    stcErr.sFunction = "clBrowsedStochasticMortality::clBrowsedStochasticMortality" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clBrowsedStochasticMortality::~clBrowsedStochasticMortality()
{
  delete[] mp_fBrowsedMortProb;
  delete[] mp_fUnbrowsedMortProb;

  if ( mp_iBrowsedCodes )
  {
    for ( int i = 0; i < m_iNumSpecies; i++ )
    {
      delete[] mp_iBrowsedCodes[i];
    }
  }
  delete[] mp_iBrowsedCodes;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clBrowsedStochasticMortality::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  floatVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    float fYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    short int i, j, iNumTypes = p_oPop->GetNumberOfTypes();

    m_iNumSpecies = p_oPop->GetNumberOfSpecies();

    //Declare the arrays we'd like read
    mp_fBrowsedMortProb = new float[m_iNumSpecies];
    mp_fUnbrowsedMortProb = new float[m_iNumSpecies];

    //Declare the species-specific temp array and pre-load with the species
    //that this behavior affects
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Browsed mortality probability
    FillSpeciesSpecificValue( p_oElement, "mo_browsedRandomMortality", "mo_brmVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 1 and translating to a timestep probability
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( 0 > p_fTempValues[i].val || 1 < p_fTempValues[i].val )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clBrowsedStochasticMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "All values for probability of mortality must be between 0 and 1.";
        throw( stcErr );
      }
      mp_fBrowsedMortProb[p_fTempValues[i].code] = 1 - pow( 1 - p_fTempValues[i].val, fYearsPerTimestep );
    }

    //Unbrowsed mortality probability
    FillSpeciesSpecificValue( p_oElement, "mo_stochasticMortRate", "mo_smrVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 1 and translating to a timestep probability
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( 0 > p_fTempValues[i].val || 1 < p_fTempValues[i].val )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clBrowsedStochasticMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "All values for probability of mortality must be between 0 and 1.";
        throw( stcErr );
      }
      mp_fUnbrowsedMortProb[p_fTempValues[i].code] = 1 - pow( 1 - p_fTempValues[i].val, fYearsPerTimestep );
    }

    //Collect the "Browsed" codes
    mp_iBrowsedCodes = new short int *[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++) {
      mp_iBrowsedCodes[i] = new short int[iNumTypes];
      for (j = 0; j < iNumTypes; j++) {
        mp_iBrowsedCodes[i][j] = -1;
      }
    }
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      mp_iBrowsedCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                       [mp_whatSpeciesTypeCombos[i].iType] =
                           p_oPop->GetBoolDataCode("Browsed",
                               mp_whatSpeciesTypeCombos[i].iSpecies,
                               mp_whatSpeciesTypeCombos[i].iType );
      if (-1 == mp_iBrowsedCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                                 [mp_whatSpeciesTypeCombos[i].iType] )
      {
        modelErr stcErr;
        stcErr.sFunction = "clBrowsedStochasticMortality::DoShellSetup" ;
        std::stringstream s;
        s << "Type/species combo species="
            << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
            << mp_whatSpeciesTypeCombos[i].iType
            << " does not have the browse behavior.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    delete[] p_fTempValues;
  }
  catch ( modelErr & err )
  {
    delete[] p_fTempValues;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTempValues;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBrowsedStochasticMortality::DoShellSetup" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////
// DoMort()
///////////////////////////////////////////////////////////////////////////
deadCode clBrowsedStochasticMortality::DoMort( clTree * p_oTree, const float & fDiam, const short int & iSpecies )
{
  short int iType = p_oTree->GetType();
  bool bBrowsed;

  //Get the browse status for this tree
  p_oTree->GetValue( mp_iBrowsedCodes[iSpecies][iType], &bBrowsed);

  if ( bBrowsed ) {
    if ( clModelMath::GetRand() < mp_fBrowsedMortProb[iSpecies] )
      return natural;
    else
      return notdead;

  }
  else
  {
    if ( clModelMath::GetRand() < mp_fUnbrowsedMortProb[iSpecies] )
      return natural;
    else
      return notdead;
  }
}
