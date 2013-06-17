//---------------------------------------------------------------------------
// PRSemiStochGrowth.cpp
//---------------------------------------------------------------------------
#include "PRSemiStochGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "GrowthOrg.h"
#include "ModelMath.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clPRSemiStochGrowth::clPRSemiStochGrowth( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clGrowthBase( p_oSimManager )
  {

  try
  {
    mp_fHeightThreshold = NULL;
    mp_fA = NULL;
    mp_fB = NULL;
    mp_fMeanDiam = NULL;
    mp_fDiamStdDev = NULL;
    mp_iIndexes = NULL;

    m_iGrowthMethod = diameter_only;
    m_bGoLast = true;

    m_sNameString = "prsemistochgrowthshell";
    m_sXMLRoot = "PRSemiStochastic";
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
    stcErr.sFunction = "clPRSemiStochGrowth::clPRSemiStochGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clPRSemiStochGrowth::~clPRSemiStochGrowth()
{
  delete[] mp_fHeightThreshold;
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fMeanDiam;
  delete[] mp_fDiamStdDev;
  delete[] mp_iIndexes;
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
//////////////////////////////////////////////////////////////////////////////
void clPRSemiStochGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Make the list of indexes
    mp_iIndexes = new short int[iNumSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the arrays we'd like read
    mp_fHeightThreshold = new float[m_iNumBehaviorSpecies];
    mp_fA = new float[m_iNumBehaviorSpecies];
    mp_fB = new float[m_iNumBehaviorSpecies];
    mp_fMeanDiam = new float[m_iNumBehaviorSpecies];
    mp_fDiamStdDev = new float[m_iNumBehaviorSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Height threshold for stochastic growth
    FillSpeciesSpecificValue( p_oElement, "gr_prStochHiteThreshold", "gr_pshtVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fHeightThreshold[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //"a" for deterministic growth
    FillSpeciesSpecificValue( p_oElement, "gr_prStochDetermA", "gr_psdaVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fA[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //"b" for deterministic growth
    FillSpeciesSpecificValue( p_oElement, "gr_prStochDetermB", "gr_psdbVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Mean diam for stochastic growth
    FillSpeciesSpecificValue( p_oElement, "gr_prStochMeanDBH", "gr_psmdVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMeanDiam[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Standard deviation for stochastic growth
    FillSpeciesSpecificValue( p_oElement, "gr_prStochStdDev", "gr_pssdVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fDiamStdDev[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    delete[] p_fTempValues;
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
    stcErr.sFunction = "clPRSemiStochGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clPRSemiStochGrowth::CalcDiameterGrowthValue( clTree * p_oTree, clTreePopulation * p_oPop, float fHeightGrowth )
{
  float fHeight, fDiam, fDiamIncrement;
  int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType(),
      iDiamCode = mp_oGrowthOrg->GetDiamCode( iSpecies, iType );

  //Get the height and the diameter
  p_oTree->GetValue( p_oPop->GetHeightCode(iSpecies, iType), & fHeight );
  p_oTree->GetValue( iDiamCode, & fDiam );

  fHeight += fHeightGrowth;

  //Is the height above the height threshold?
  if (fHeight > mp_fHeightThreshold[mp_iIndexes[iSpecies]]) {
    do {
      //Get a random diameter and return the difference
      fDiamIncrement = (clModelMath::NormalRandomDraw(mp_fDiamStdDev[mp_iIndexes[iSpecies]]) + mp_fMeanDiam[mp_iIndexes[iSpecies]]) -  fDiam;
    } while (fDiam + fDiamIncrement < 0.1);
  }
  else {

    //Calculate the amount of diam increase based on the new height
    fHeight *= 100; //transform to cm
    fDiamIncrement = (mp_fA[mp_iIndexes[iSpecies]] * exp(-mp_fB[mp_iIndexes[iSpecies]] * fHeight)) - fDiam;
    if (fDiamIncrement + fDiam < 0.1)
      fDiamIncrement = 0;
  }

  return fDiamIncrement;
}
