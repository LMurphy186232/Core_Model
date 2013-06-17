//---------------------------------------------------------------------------
// LinearBiLevelGrowth.cpp
//---------------------------------------------------------------------------
#include "LinearBiLevelGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "ModelMath.h"
#include "GrowthOrg.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////*/
clLinearBiLevelGrowth::clLinearBiLevelGrowth( clSimManager * p_oSimManager ) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
     clGrowthBase( p_oSimManager )
{

  try
  {
    mp_oStormLight = NULL;
    mp_fLoLightSlope = NULL;
    mp_fLoLightIntercept = NULL;
    mp_fHiLightSlope = NULL;
    mp_fHiLightIntercept = NULL;
    mp_fHiLightThreshold = NULL;
    mp_iIndexes = NULL;

    m_iLightCode = -1;
    m_fYearsPerTimestep = 0;

    m_sNameString = "linearbilevelgrowthshell";
    m_sXMLRoot = "LinearBilevelGrowth";
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
    stcErr.sFunction = "clLinearBiLevelGrowth::clLinearBiLevelGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clLinearBiLevelGrowth::~clLinearBiLevelGrowth()
{
  delete[] mp_fLoLightSlope;
  delete[] mp_fLoLightIntercept;
  delete[] mp_fHiLightSlope;
  delete[] mp_fHiLightIntercept;
  delete[] mp_fHiLightThreshold;
  delete[] mp_iIndexes;
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clLinearBiLevelGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  floatVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Get number of years per timestep
    m_fYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    mp_fLoLightSlope = new float[m_iNumBehaviorSpecies];
    mp_fLoLightIntercept = new float[m_iNumBehaviorSpecies];
    mp_fHiLightSlope = new float[m_iNumBehaviorSpecies];
    mp_fHiLightIntercept = new float[m_iNumBehaviorSpecies];
    mp_fHiLightThreshold = new float[m_iNumBehaviorSpecies];
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
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Low-light growth intercept
    FillSpeciesSpecificValue( p_oElement, "gr_linearBilevLoLiteIntercept",
        "gr_lblliVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fLoLightIntercept[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Low-light growth slope
    FillSpeciesSpecificValue( p_oElement, "gr_linearBilevLoLiteSlope",
        "gr_lbllsVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fLoLightSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Get high-light parameters if we have a storm light grid
    if (NULL != mp_oStormLight) {

      //High-light threshold
      FillSpeciesSpecificValue( p_oElement, "gr_linearBilevHiLiteThreshold",
        "gr_lbhltVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array, making sure all values are
      //between 0 and 100
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        if (p_fTempValues[i].val < 0 || p_fTempValues[i].val > 100) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clLinearBiLevelGrowth::DoShellSetup";
          stcErr.sMoreInfo = "All values in high-light growth threshold must be between 0 and 100.";
          throw(stcErr);
        }
        mp_fHiLightThreshold[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      }

      //High-light growth intercept
      FillSpeciesSpecificValue( p_oElement, "gr_linearBilevHiLiteIntercept",
        "gr_lbhliVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fHiLightIntercept[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //High-light growth slope
      FillSpeciesSpecificValue( p_oElement, "gr_linearBilevHiLiteSlope",
        "gr_lbhlsVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fHiLightSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

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
    stcErr.sFunction = "clLinearBiLevelGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
/////////////////////////////////////////////////////////////////////////////*/
void clLinearBiLevelGrowth::SetNameData(std::string sNameString) {
 try {

   //Check the string passed and set the flags accordingly
   if (sNameString.compare("LinearBilevelGrowth") == 0) {
     m_iGrowthMethod = diameter_auto;
   } else if (sNameString.compare("LinearBilevelGrowth diam only") == 0) {
     m_iGrowthMethod = diameter_only;
   } else {
     modelErr stcErr;
     stcErr.iErrorCode = BAD_DATA;
     std::stringstream s;
     s << "Unrecognized behavior name \"" << sNameString << "\".";
     stcErr.sFunction = "clLinearBiLevelGrowth::SetNameData";
     stcErr.sMoreInfo = s.str();
     throw(stcErr);
   }
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clLinearBiLevelGrowth::SetNameData";
   throw(stcErr);
 }
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clLinearBiLevelGrowth::CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth) {

  float fLightLevel = 0, fDiam;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();

  //If applicable, get the storm light level at the tree's location
  if (NULL != mp_oStormLight) {
    float fX, fY;
    p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
    p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

    mp_oStormLight->GetValueAtPoint(fX, fY, m_iLightCode, &fLightLevel);
  }

  //Get the tree's diameter
  p_oTree->GetValue(mp_oGrowthOrg->GetDiamCode(iSp, iTp), &fDiam);

  //Calculate the function value according to the light level
  if (fLightLevel < mp_fHiLightThreshold[mp_iIndexes[iSp]]) {
    return m_fYearsPerTimestep * clModelMath::CalcPointValue(fDiam, mp_fLoLightSlope[mp_iIndexes[iSp]], mp_fLoLightIntercept[mp_iIndexes[iSp]]);
  }
  return m_fYearsPerTimestep * clModelMath::CalcPointValue(fDiam, mp_fHiLightSlope[mp_iIndexes[iSp]], mp_fHiLightIntercept[mp_iIndexes[iSp]]);
}
