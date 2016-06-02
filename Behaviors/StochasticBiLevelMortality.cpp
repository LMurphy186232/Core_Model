//---------------------------------------------------------------------------
#include "StochasticBiLevelMortality.h"
#include "MortalityOrg.h"
#include "Grid.h"
#include "ModelMath.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clStochasticBiLevelMortality::clStochasticBiLevelMortality( clSimManager * p_oSimManager ) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
     clMortalityBase( p_oSimManager )
{
  try
  {

    m_sNameString = "stochastic bilevel mortshell";
    m_sXMLRoot = "StochasticBiLevelMortality";

    mp_oStormLight = NULL;
    mp_fLoLightMortProb = NULL;
    mp_fHiLightMortProb = NULL;
    mp_fHiLightThreshold = NULL;
    mp_iGLILightCodes = NULL;
    mp_iIndexes = NULL;
    mp_oPop = NULL;
    m_iLightCode = -1;
    getLightLevel = NULL;
    m_bIsGLI = false;
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
    stcErr.sFunction = "clStochasticBiLevelMortality::clStochasticBiLevelMortality" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clStochasticBiLevelMortality::~clStochasticBiLevelMortality()
{
  delete[] mp_fLoLightMortProb;
  delete[] mp_fHiLightMortProb;
  delete[] mp_fHiLightThreshold;
  delete[] mp_iIndexes;
  if (mp_iGLILightCodes) {
    for (int i = 0; i < m_iNumTotalSpecies; i++)
      delete[] mp_iGLILightCodes[i];
    delete[] mp_iGLILightCodes;
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clStochasticBiLevelMortality::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    mp_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    int iYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    short int iNumSpecies = mp_oPop->GetNumberOfSpecies(),
              iNumTypes = mp_oPop->GetNumberOfTypes(),
              iSp, iTp, i, j;

    //Declare the arrays we'd like read
    mp_fLoLightMortProb = new double[m_iNumBehaviorSpecies];
    mp_fHiLightMortProb = new double[m_iNumBehaviorSpecies];
    mp_fHiLightThreshold = new double[m_iNumBehaviorSpecies];
    mp_iIndexes = new int[iNumSpecies];

    //Set up the array indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    if (m_bIsGLI) {

      getLightLevel = &clStochasticBiLevelMortality::GetGLILightLevel;

      mp_iGLILightCodes = new int*[iNumSpecies];
      for (i = 0; i < iNumSpecies; i++) {
        mp_iGLILightCodes[i] = new int[iNumTypes];
        for (j = 0; j < iNumTypes; j++) mp_iGLILightCodes[i][j] = -1;
      }
      for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
        iSp = mp_whatSpeciesTypeCombos[i].iSpecies;
        iTp = mp_whatSpeciesTypeCombos[i].iType;
        mp_iGLILightCodes[iSp][iTp] = mp_oPop->GetFloatDataCode("Light", iSp, iTp);
        if (-1 == mp_iGLILightCodes[iSp][iTp]) {
          modelErr stcErr;
        stcErr.iErrorCode = CANT_FIND_OBJECT;
        stcErr.sFunction = "clStochasticBiLevelMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "A light behavior must be assigned to all trees for Stochastic Bi-Level Mortality.";
        throw( stcErr );
        }
      }

    } else {
      getLightLevel = &clStochasticBiLevelMortality::GetStormLightLevel;

      //See if we can find the storm light grid
      mp_oStormLight = mp_oSimManager->GetGridObject( "Storm Light" );
      if ( NULL != mp_oStormLight )
      {
        m_iLightCode = mp_oStormLight->GetFloatDataCode( "Light" );
      }
      else
      {
        modelErr stcErr;
        stcErr.iErrorCode = CANT_FIND_OBJECT;
        stcErr.sFunction = "clStochasticBiLevelMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "\"Storm Light\" grid is missing.";
        throw( stcErr );
      }
    }

    //Declare the species-specific temp array and pre-load with the species
    //that this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Low-light mortality probability
    FillSpeciesSpecificValue( p_oElement, "mo_stochBilevLoLiteMortProb", "mo_sbllmpVal", p_fTempValues,
         m_iNumBehaviorSpecies, mp_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 1 and translating to a timestep probability
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( 0 > p_fTempValues[i].val || 1 < p_fTempValues[i].val )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clStochasticBiLevelMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "All values for probability of mortality must be between 0 and 1.";
        throw( stcErr );
      }
      mp_fLoLightMortProb[mp_iIndexes[p_fTempValues[i].code]] = 1 - pow( 1 - p_fTempValues[i].val, iYearsPerTimestep );
    }

    //Translate the values

    //High-light mortality probability
    FillSpeciesSpecificValue( p_oElement, "mo_stochBilevHiLiteMortProb", "mo_sbhlmpVal", p_fTempValues,
         m_iNumBehaviorSpecies, mp_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 1 and translating to a timestep probability
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( 0 > p_fTempValues[i].val || 1 < p_fTempValues[i].val )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clStochasticBiLevelMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "All values for probability of mortality must be between 0 and 1.";
        throw( stcErr );
      }
      mp_fHiLightMortProb[mp_iIndexes[p_fTempValues[i].code]] = 1 - pow( 1 - p_fTempValues[i].val, iYearsPerTimestep );
    }

    //High-light threshold
    FillSpeciesSpecificValue( p_oElement, "mo_stochBilevHiLiteThreshold", "mo_sbhltVal", p_fTempValues,
         m_iNumBehaviorSpecies, mp_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 100
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( p_fTempValues[i].val < 0 || p_fTempValues[i].val > 100 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clStochasticBiLevelMortality::DoShellSetup" ;
        stcErr.sMoreInfo = "All values in high-light growth threshold must be between 0 and 100.";
        throw( stcErr );
      }
      mp_fHiLightThreshold[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
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
    stcErr.sFunction = "clStochasticBiLevelMortality::DoShellSetup" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////
// DoMort()
///////////////////////////////////////////////////////////////////////////
deadCode clStochasticBiLevelMortality::DoMort( clTree * p_oTree, const float & fDiam, const short int & iSpecies )
{

  float fLightLevel = (*this.*getLightLevel)(p_oTree, iSpecies);

  //Calculate the function value according to the light level
  if ( fLightLevel < mp_fHiLightThreshold[mp_iIndexes[iSpecies]] ) {
    if ( clModelMath::GetRand() < mp_fLoLightMortProb[mp_iIndexes[iSpecies]] )
      return natural;
    else
      return notdead;

  }
  else
  {
    if ( clModelMath::GetRand() < mp_fHiLightMortProb[mp_iIndexes[iSpecies]] )
      return natural;
    else
      return notdead;
  }
}

////////////////////////////////////////////////////////////////////////////
// SetNameData()
////////////////////////////////////////////////////////////////////////////
void clStochasticBiLevelMortality::SetNameData(std::string sNameString) {
 if (sNameString.compare("StochasticBiLevelMortality - GLI") == 0)
   m_bIsGLI = true;
 else
   m_bIsGLI = false;
}

////////////////////////////////////////////////////////////////////////////
// GetStormLightLevel()
////////////////////////////////////////////////////////////////////////////
float clStochasticBiLevelMortality::GetStormLightLevel(clTree *p_oTree, const short int & iSpecies) {
  float fLightLevel, fX, fY;
  int iTp = p_oTree->GetType();

  //Get the storm light level at the tree's location
  p_oTree->GetValue( mp_oPop->GetXCode( iSpecies, iTp ), & fX );
  p_oTree->GetValue( mp_oPop->GetYCode( iSpecies, iTp ), & fY );

  mp_oStormLight->GetValueAtPoint( fX, fY, m_iLightCode, & fLightLevel );
  return fLightLevel;
}

////////////////////////////////////////////////////////////////////////////
// GetGLILightLevel()
////////////////////////////////////////////////////////////////////////////
float clStochasticBiLevelMortality::GetGLILightLevel(clTree *p_oTree, const short int & iSpecies) {
  float fLightLevel;
  int iTp = p_oTree->GetType();
  p_oTree->GetValue(mp_iGLILightCodes[iSpecies][iTp], &fLightLevel);
  return fLightLevel;
}
