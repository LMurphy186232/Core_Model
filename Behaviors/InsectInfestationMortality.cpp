//---------------------------------------------------------------------------
#include "InsectInfestationMortality.h"
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
clInsectInfestationMortality::clInsectInfestationMortality( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
  clMortalityBase( p_oSimManager )
{
  try
  {
    m_sNameString = "insect infestation mortshell";
    m_sXMLRoot = "InsectInfestationMortality";

    mp_fMortProbs = NULL;
    mp_fIntercept = NULL;
    mp_fMax = NULL;
    mp_fX0 = NULL;
    mp_fXb = NULL;
    mp_iDataCodes = NULL;

    m_iNumSpecies = 0;
    m_iMaxMortTime = 0;
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
    stcErr.sFunction = "clInsectInfestationMortality::clInsectInfestationMortality" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clInsectInfestationMortality::~clInsectInfestationMortality()
{
  delete[] mp_fIntercept;
  delete[] mp_fMax;
  delete[] mp_fX0;
  delete[] mp_fXb;

  if ( mp_iDataCodes ) {
    for ( int i = 0; i < m_iNumSpecies; i++ ) {
      delete[] mp_iDataCodes[i];
    }
  }
  delete[] mp_iDataCodes;

  if ( mp_fMortProbs ) {
    for ( int i = 0; i < m_iNumSpecies; i++ ) {
      delete[] mp_fMortProbs[i];
    }
  }
  delete[] mp_fMortProbs;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clInsectInfestationMortality::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int i, j, iNumTypes = p_oPop->GetNumberOfTypes();

    m_iNumSpecies = p_oPop->GetNumberOfSpecies();

    //Declare the arrays we'd like read
    mp_fIntercept = new double[m_iNumSpecies];
    mp_fMax = new double[m_iNumSpecies];
    mp_fX0 = new double[m_iNumSpecies];
    mp_fXb = new double[m_iNumSpecies];

    //Declare the species-specific temp array and pre-load with the species
    //that this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Intercept
    FillSpeciesSpecificValue( p_oElement, "mo_insectMortIntercept", "mo_imiVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 1
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( 0 > p_fTempValues[i].val || 1 < p_fTempValues[i].val ) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clInsectInfestationMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "All values for mortality intercept must be between 0 and 1.";
        throw( stcErr );
      }
      mp_fIntercept[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Max mortality probability
    FillSpeciesSpecificValue( p_oElement, "mo_insectMortMaxRate", "mo_immrVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 1
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( 0 > p_fTempValues[i].val || 1 < p_fTempValues[i].val ) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clInsectInfestationMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "All values for max mortality must be between 0 and 1.";
        throw( stcErr );
      }
      mp_fMax[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //X0
    FillSpeciesSpecificValue( p_oElement, "mo_insectMortX0", "mo_imx0Val",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, making sure no values are 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( 0 == p_fTempValues[i].val) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clInsectInfestationMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "Values for insect mortality X0 cannot be 0.";
        throw( stcErr );
      }
      mp_fX0[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Xb
    FillSpeciesSpecificValue( p_oElement, "mo_insectMortXb", "mo_imxbVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fXb[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Collect the "YearsInfested" codes
    mp_iDataCodes = new short int *[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++) {
      mp_iDataCodes[i] = new short int[iNumTypes];
      for (j = 0; j < iNumTypes; j++) {
        mp_iDataCodes[i][j] = -1;
      }
    }
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      mp_iDataCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                      [mp_whatSpeciesTypeCombos[i].iType] =
            p_oPop->GetIntDataCode("YearsInfested",
                                    mp_whatSpeciesTypeCombos[i].iSpecies,
                                    mp_whatSpeciesTypeCombos[i].iType );
      if (-1 == mp_iDataCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                                 [mp_whatSpeciesTypeCombos[i].iType] )
      {
        modelErr stcErr;
        stcErr.sFunction = "clInsectInfestationMortality::DoShellSetup" ;
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " does not have the insect infestation behavior.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    delete[] p_fTempValues;

    //Pre-calculate mortalities
    m_iMaxMortTime = 0;
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      if ((int)(mp_fX0[mp_iWhatSpecies[i]] * 2) > m_iMaxMortTime)
        m_iMaxMortTime = (int)(mp_fX0[mp_iWhatSpecies[i]] * 2);
    }
    m_iMaxMortTime++;
    mp_fMortProbs = new double *[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++) {
      mp_fMortProbs[i] = new double[m_iMaxMortTime];
      for (j = 0; j < m_iMaxMortTime; j++) mp_fMortProbs[i][j] = 0;
    }

    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      for (j = 1; j < m_iMaxMortTime; j++) {
        mp_fMortProbs[mp_iWhatSpecies[i]][j] = mp_fIntercept[mp_iWhatSpecies[i]] +
        ((mp_fMax[mp_iWhatSpecies[i]] - mp_fIntercept[mp_iWhatSpecies[i]])/
            (1+pow(j/mp_fX0[mp_iWhatSpecies[i]], mp_fXb[mp_iWhatSpecies[i]])));
      }
    }
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
    stcErr.sFunction = "clInsectInfestationMortality::DoShellSetup" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////
// DoMort()
///////////////////////////////////////////////////////////////////////////
deadCode clInsectInfestationMortality::DoMort( clTree * p_oTree, const float & fDiam, const short int & iSpecies )
{
  short int iType = p_oTree->GetType();
  float fProb;
  int iYrsInfested;

  //Get the infestation status for this tree
  p_oTree->GetValue( mp_iDataCodes[iSpecies][iType], &iYrsInfested);

  if ( 0 == iYrsInfested ) return notdead;

  if ( iYrsInfested < m_iMaxMortTime ) {
    fProb = mp_fMortProbs[iSpecies][iYrsInfested];
  } else {
    fProb = mp_fIntercept[iSpecies] +
        ((mp_fMax[iSpecies] - mp_fIntercept[iSpecies])/
            (1+pow(iYrsInfested/mp_fX0[iSpecies], mp_fXb[iSpecies])));
  }

  if ( clModelMath::GetRand() < fProb )
    return insects;
  else
    return notdead;
}
