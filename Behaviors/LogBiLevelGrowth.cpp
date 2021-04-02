//---------------------------------------------------------------------------
// LogBiLevelGrowth.cpp
//---------------------------------------------------------------------------
#include "LogBiLevelGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////*/
clLogBiLevelGrowth::clLogBiLevelGrowth( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clGrowthBase( p_oSimManager )
{

  try
  {

    m_iGrowthMethod = height_only;

    mp_oStormLight = NULL;
    mp_fLoLightMaxGrowth = NULL;
    mp_fLoLightX0 = NULL;
    mp_fLoLightXb = NULL;
    mp_fHiLightMaxGrowth = NULL;
    mp_fHiLightX0 = NULL;
    mp_fHiLightXb = NULL;
    mp_fHiLightThreshold = NULL;
    mp_iIndexes = NULL;

    m_iYearsPerTimestep = 0;
    m_iLightCode = -1;

    m_sNameString = "logbilevelgrowthshell";
    m_sXMLRoot = "LogBilevelGrowth";
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
    stcErr.sFunction = "clLogBiLevelGrowth::clLogBiLevelGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clLogBiLevelGrowth::~clLogBiLevelGrowth()
{
  delete[] mp_fLoLightMaxGrowth;
  delete[] mp_fLoLightX0;
  delete[] mp_fLoLightXb;
  delete[] mp_fHiLightMaxGrowth;
  delete[] mp_fHiLightX0;
  delete[] mp_fHiLightXb;
  delete[] mp_fHiLightThreshold;
  delete[] mp_iIndexes;
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clLogBiLevelGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Get number of years per timestep
    m_iYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    mp_fLoLightMaxGrowth = new double[m_iNumBehaviorSpecies];
    mp_fLoLightX0 = new double[m_iNumBehaviorSpecies];
    mp_fLoLightXb = new double[m_iNumBehaviorSpecies];
    mp_fHiLightMaxGrowth = new double[m_iNumBehaviorSpecies];
    mp_fHiLightX0 = new double[m_iNumBehaviorSpecies];
    mp_fHiLightXb = new double[m_iNumBehaviorSpecies];
    mp_fHiLightThreshold = new double[m_iNumBehaviorSpecies];
    mp_iIndexes = new int[iNumSpecies];

    //Set up the array indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //See if we can find the storm light grid
    mp_oStormLight = mp_oSimManager->GetGridObject("Storm Light");
    if (NULL != mp_oStormLight) {
      m_iLightCode = mp_oStormLight->GetFloatDataCode("Light");
    }

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Low-light X0
    FillSpeciesSpecificValue( p_oElement, "gr_lognormalBilevLoLiteX0",
        "gr_lbllx0Val", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array while making sure none of them
    //are 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val == 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clLogBiLevelGrowth::DoShellSetup";
        stcErr.sMoreInfo = "X0 values cannot equal 0.";
        throw(stcErr);
      }
      mp_fLoLightX0[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Low-light Xb
    FillSpeciesSpecificValue( p_oElement, "gr_lognormalBilevLoLiteXb",
        "gr_lbllxbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array while making sure none of them
    //are 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val == 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clLogBiLevelGrowth::DoShellSetup";
        stcErr.sMoreInfo = "Xb values cannot equal 0.";
        throw(stcErr);
      }
      mp_fLoLightXb[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }


    //Low-light max growth
    FillSpeciesSpecificValue( p_oElement, "gr_lognormalBilevLoLiteMaxGrowth",
        "gr_lbllmgVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array while making sure none of them
    //are 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clLogBiLevelGrowth::DoShellSetup";
        stcErr.sMoreInfo = "Max growth values must be greater than 0.";
        throw(stcErr);
      }
      mp_fLoLightMaxGrowth[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Get high-light parameters if we have a storm light grid
    if (NULL != mp_oStormLight) {

      //High-light threshold
      FillSpeciesSpecificValue( p_oElement, "gr_lognormalBilevHiLiteThreshold",
        "gr_lobhltVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array, making sure all values are
      //between 0 and 100
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        if (p_fTempValues[i].val < 0 || p_fTempValues[i].val > 100) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clLogBiLevelGrowth::DoShellSetup";
          stcErr.sMoreInfo = "All values in high-light growth threshold must be between 0 and 100.";
          throw(stcErr);
        }
        mp_fHiLightThreshold[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      }

      //High-light X0
      FillSpeciesSpecificValue( p_oElement, "gr_lognormalBilevHiLiteX0",
          "gr_lbhlx0Val", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array while making sure none of them
      //are 0
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        if (p_fTempValues[i].val == 0) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clLogBiLevelGrowth::DoShellSetup";
          stcErr.sMoreInfo = "X0 values cannot equal 0.";
          throw(stcErr);
        }
        mp_fHiLightX0[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      }

      //High-light Xb
      FillSpeciesSpecificValue( p_oElement, "gr_lognormalBilevHiLiteXb",
          "gr_lbhlxbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array while making sure none of them
      //are 0
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        if (p_fTempValues[i].val == 0) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clLogBiLevelGrowth::DoShellSetup";
          stcErr.sMoreInfo = "Xb values cannot equal 0.";
          throw(stcErr);
        }
        mp_fHiLightXb[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      }

      //High-light max growth
      FillSpeciesSpecificValue( p_oElement, "gr_lognormalBilevHiLiteMaxGrowth",
          "gr_lbhlmgVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array while making sure none of them
      //are 0
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        if (p_fTempValues[i].val < 0) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clLogBiLevelGrowth::DoShellSetup";
          stcErr.sMoreInfo = "Max growth values must be greater than 0.";
          throw(stcErr);
        }
        mp_fHiLightMaxGrowth[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      }

    } else {
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        mp_fHiLightThreshold[i] = 100;
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
    stcErr.sFunction = "clLogBiLevelGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcHeightGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clLogBiLevelGrowth::CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth) {

  float fLightLevel = 0, fHeight;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();

  //If applicable, get the storm light level at the tree's location
  if (NULL != mp_oStormLight) {
    float fX, fY;
    p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
    p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

    mp_oStormLight->GetValueAtPoint(fX, fY, m_iLightCode, &fLightLevel);
  }

  //Get the tree's height
  p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fHeight);

  //Calculate the function value according to the light level
  if (fLightLevel < mp_fHiLightThreshold[mp_iIndexes[iSp]]) {
    return m_iYearsPerTimestep * mp_fLoLightMaxGrowth[mp_iIndexes[iSp]] * exp( -0.5 * pow( log( fHeight / mp_fLoLightX0[mp_iIndexes[iSp]] ) / mp_fLoLightXb[mp_iIndexes[iSp]], 2 ) );
  }
  return m_iYearsPerTimestep * mp_fHiLightMaxGrowth[mp_iIndexes[iSp]] * exp( -0.5 * pow( log( fHeight / mp_fHiLightX0[mp_iIndexes[iSp]] ) / mp_fHiLightXb[mp_iIndexes[iSp]], 2 ) );
}
