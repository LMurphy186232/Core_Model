//---------------------------------------------------------------------------
// PRStormBiLevelGrowth.cpp
//---------------------------------------------------------------------------
#include "PRStormBiLevelGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "GrowthOrg.h"
#include "Allometry.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////*/
clPRStormBiLevelGrowth::clPRStormBiLevelGrowth( clSimManager * p_oSimManager ) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
     clGrowthBase( p_oSimManager )
{

  try
  {
    mp_oStormLight = NULL;
    mp_oStorm = NULL;
    mp_fLoLightSlope = NULL;
    mp_fLoLightIntercept = NULL;
    mp_fHiLightA = NULL;
    mp_fHiLightB = NULL;
    mp_fHiLightThreshold = NULL;
    mp_iIndexes = NULL;

    m_iGrowthMethod = diameter_auto;
    m_iLightCode = -1;
    m_iStormtimeCode = -1;
    m_fYearsPerTimestep = 0;

    m_sNameString = "prstormbilevelgrowthshell";
    m_sXMLRoot = "PRStormBilevelGrowth";
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
    stcErr.sFunction = "clPRStormBiLevelGrowth::clPRStormBiLevelGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clPRStormBiLevelGrowth::~clPRStormBiLevelGrowth()
{
  delete[] mp_fLoLightSlope;
  delete[] mp_fLoLightIntercept;
  delete[] mp_fHiLightA;
  delete[] mp_fHiLightB;
  delete[] mp_fHiLightThreshold;
  delete[] mp_iIndexes;
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clPRStormBiLevelGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  floatVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Get number of years per timestep
    m_fYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    mp_fLoLightSlope = new float[m_iNumBehaviorSpecies];
    mp_fLoLightIntercept = new float[m_iNumBehaviorSpecies];
    mp_fHiLightA = new float[m_iNumBehaviorSpecies];
    mp_fHiLightB = new float[m_iNumBehaviorSpecies];
    mp_fHiLightThreshold = new float[m_iNumBehaviorSpecies];
    mp_iIndexes = new int[iNumSpecies];

    //Set up the array indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Find the storm grid
    mp_oStorm = mp_oSimManager->GetGridObject("Storm Damage");
    if (NULL == mp_oStorm) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPRStormBiLevelGrowth::DoShellSetup";
      stcErr.sMoreInfo = "The storm behavior must be used with PR Storm Bi-Level Growth.";
      throw(stcErr);
    }
    m_iStormtimeCode = mp_oStorm->GetFloatDataCode("stormtime");
    if (-1 == m_iStormtimeCode) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPRStormBiLevelGrowth::DoShellSetup";
      stcErr.sMoreInfo = "The \"stormtime\" was not found in the \"Storm Damage\" grid.";
      throw(stcErr);
    }

    //Find the storm light grid
    mp_oStormLight = mp_oSimManager->GetGridObject("Storm Light");
    if (NULL == mp_oStormLight) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPRStormBiLevelGrowth::DoShellSetup";
      stcErr.sMoreInfo = "The storm light behavior must be used with PR Storm Bi-Level Growth.";
      throw(stcErr);
    }
    m_iLightCode = mp_oStormLight->GetFloatDataCode("Light");
    if (-1 == m_iLightCode) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPRStormBiLevelGrowth::DoShellSetup";
      stcErr.sMoreInfo = "The \"Light\" was not found in the \"Storm Light\" grid.";
      throw(stcErr);
    }

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Low-light growth intercept
    FillSpeciesSpecificValue( p_oElement, "gr_prBilevStmGrwthLoLiteIntercept",
        "gr_pbsglliVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fLoLightIntercept[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Low-light growth slope
    FillSpeciesSpecificValue( p_oElement, "gr_prBilevStmGrwthLoLiteSlope",
        "gr_pbsgllsVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fLoLightSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //High-light threshold
    FillSpeciesSpecificValue( p_oElement, "gr_prBilevStmGrwthHiLiteThreshold",
      "gr_pbsghltVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 100
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val < 0 || p_fTempValues[i].val > 100) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clPRStormBiLevelGrowth::DoShellSetup";
        stcErr.sMoreInfo = "All values in high-light growth threshold must be between 0 and 100.";
        throw(stcErr);
      }
      mp_fHiLightThreshold[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //High-light growth "a"
    FillSpeciesSpecificValue( p_oElement, "gr_prBilevStmGrwthHiLiteA",
      "gr_pbsghlaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array - transform this from cm to m
    //of height
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fHiLightA[mp_iIndexes[p_fTempValues[i].code]] =
         p_fTempValues[i].val/100.0;

    //High-light growth "b"
    FillSpeciesSpecificValue( p_oElement, "gr_prBilevStmGrwthHiLiteB",
      "gr_pbsghlbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fHiLightB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

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
    stcErr.sFunction = "clPRStormBiLevelGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clPRStormBiLevelGrowth::CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth) {
  clAllometry *p_oAllom;
  float fLightLevel, fDiam, fX, fY, fNumYears, fHeight;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();

  //Get the storm light level at the tree's location
  p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
  p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

  mp_oStormLight->GetValueAtPoint(fX, fY, m_iLightCode, &fLightLevel);

  //Get the tree's diameter
  p_oTree->GetValue(mp_oGrowthOrg->GetDiamCode(iSp, iTp), &fDiam);

  //Calculate the function value according to the light level
  if (fLightLevel < mp_fHiLightThreshold[mp_iIndexes[iSp]]) {
    return m_fYearsPerTimestep * clModelMath::CalcPointValue(fDiam, mp_fLoLightSlope[mp_iIndexes[iSp]], mp_fLoLightIntercept[mp_iIndexes[iSp]]);
  }

  p_oAllom = p_oPop->GetAllometryObject();

  //This is high-light growth - get the number of years since last storm
  mp_oStorm->GetValueAtPoint(fX, fY, m_iStormtimeCode, &fNumYears);

  //Get the tree's height
  p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fHeight);

  //Calculate the new tree height
  fHeightGrowth = m_fYearsPerTimestep * fDiam * mp_fHiLightA[mp_iIndexes[iSp]]
                                                             * exp(-mp_fHiLightB[mp_iIndexes[iSp]] * fNumYears);
  fHeight += fHeightGrowth;
  //Make sure it's not higher than species max - dangerously reuse
  //the fHeightGrowth variable
  fHeightGrowth = p_oAllom->GetMaxTreeHeight(iSp);
  if (fHeight > fHeightGrowth) fHeight = fHeightGrowth - 0.01;

  //Transform this height back to diameter - dangerously reuse the
  //fHeightGrowth variable
  if (clTreePopulation::seedling == iTp) {
    fHeightGrowth = p_oAllom->CalcSeedlingDiam10(fHeight, iSp);
  }
  else if (clTreePopulation::sapling == iTp) {
    fHeightGrowth = p_oAllom->CalcSaplingDbh(fHeight, iSp);
    //Remember to transform this from DBH to diam10
    fHeightGrowth = p_oAllom->ConvertDbhToDiam10(fHeightGrowth, iSp);
  }
  else {
    fHeightGrowth = p_oAllom->CalcAdultDbh(fHeight, iSp);
  }
  return fHeightGrowth - fDiam;
}
