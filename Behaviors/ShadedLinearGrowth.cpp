//---------------------------------------------------------------------------
// ShadedLinearGrowth.cpp
//---------------------------------------------------------------------------
#include "ShadedLinearGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "GrowthOrg.h"
#include "ModelMath.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////*/
clShadedLinearGrowth::clShadedLinearGrowth( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clGrowthBase( p_oSimManager )
{

  try
  {
    mp_fSlope = NULL;
    mp_fIntercept = NULL;
    mp_fShader = NULL;
    mp_iIndexes = NULL;

    m_fNumberOfYearsPerTimestep = 0;
    m_fConversionFactor = 0;

    m_sNameString = "shadedlineargrowthshell";
    m_sXMLRoot = "ShadedLinearGrowth";
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
    stcErr.sFunction = "clShadedLinearGrowth::clShadedLinearGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clShadedLinearGrowth::~clShadedLinearGrowth()
{
  delete[] mp_fSlope;
  delete[] mp_fIntercept;
  delete[] mp_fShader;
  delete[] mp_iIndexes;
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clShadedLinearGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Conversion factor
    if ( height_only == m_iGrowthMethod )
    {
      //The conversion factor needs to convert from cm to m
      m_fConversionFactor = 0.01;
    } else {
      //The conversion factor needs to convert from mm to cm and from radial to diameter
      m_fConversionFactor = 0.2;
    }

    m_fNumberOfYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Make the list of indexes
    mp_iIndexes = new short int[iNumSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the arrays we'd like read
    mp_fSlope = new float[m_iNumBehaviorSpecies];
    mp_fIntercept = new float[m_iNumBehaviorSpecies];
    mp_fShader = new float[m_iNumBehaviorSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    if ( height_only == m_iGrowthMethod )
    {
      //Growth intercept
      FillSpeciesSpecificValue( p_oElement, "gr_heightShadedLinearIntercept", "gr_hshliVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fIntercept[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Growth slope
      FillSpeciesSpecificValue( p_oElement, "gr_heightShadedLinearSlope", "gr_hshlsVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shade exponent
      FillSpeciesSpecificValue( p_oElement, "gr_heightShadedLinearShadeExp", "gr_hshlseVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShader[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }
    else
    {
      //Growth intercept
      FillSpeciesSpecificValue( p_oElement, "gr_diamShadedLinearIntercept", "gr_dshliVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fIntercept[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Growth slope
      FillSpeciesSpecificValue( p_oElement, "gr_diamShadedLinearSlope", "gr_dshlsVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shade exponent
      FillSpeciesSpecificValue( p_oElement, "gr_diamShadedLinearShadeExp", "gr_dshlseVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShader[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Make sure all species/type combos have "Light" registered
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      if (-1 == mp_oGrowthOrg->GetLightCode(mp_whatSpeciesTypeCombos[i].iSpecies,
          mp_whatSpeciesTypeCombos[i].iType)) {
        modelErr stcErr;
        stcErr.sFunction = "clShadedLinearGrowth::DoShellSetup";
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " does not have a required light behavior.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

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
    stcErr.sFunction = "clShadedLinearGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
/////////////////////////////////////////////////////////////////////////////*/
void clShadedLinearGrowth::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("ShadedLinearGrowth") == 0) {
      m_iGrowthMethod = diameter_auto;
    } else if (sNameString.compare("ShadedLinearGrowth diam only") == 0) {
      m_iGrowthMethod = diameter_only;
    } else if (sNameString.compare("ShadedLinearGrowth height only") == 0) {
      m_iGrowthMethod = height_only;
      m_bGoLast = true;
    }
    else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sFunction = "clShadedLinearGrowth::SetNameData";
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clShadedLinearGrowth::SetNameData";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalculateFunctionValue()
/////////////////////////////////////////////////////////////////////////////*/
float clShadedLinearGrowth::CalculateFunctionValue(int iSpecies, float fGLI, float fDiam) {
  short int iIndex = mp_iIndexes[iSpecies];

  //Calculate the function value
  return clModelMath::CalcPointValue(fDiam, mp_fSlope[iIndex], mp_fIntercept[iIndex]) * pow(fGLI/100, mp_fShader[iIndex]);
}

//////////////////////////////////////////////////////////////////////////////
// CalcHeightGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clShadedLinearGrowth::CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth) {

  float fGLI, fDiam, fDiamIncrement, fHeightIncrement = 0;
  int iSpecies = p_oTree->GetSpecies(),
      iType = p_oTree->GetType(),
      iLightCode = mp_oGrowthOrg->GetLightCode(iSpecies, iType),
      iDiamCode = mp_oGrowthOrg->GetDiamCode(iSpecies, iType),
      i;

  //Get the GLI and the diameter
  p_oTree->GetValue(iLightCode, &fGLI);
  p_oTree->GetValue(iDiamCode, &fDiam);

  //Calculate the diameter increment
  fDiamIncrement = fDiameterGrowth / m_fNumberOfYearsPerTimestep;

  //Increment the height
  for (i = 0; i < m_fNumberOfYearsPerTimestep; i++) {
    fHeightIncrement += CalculateFunctionValue(iSpecies, fGLI, fDiam);
    fDiam += fDiamIncrement;
  }

  return fHeightIncrement * m_fConversionFactor;
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clShadedLinearGrowth::CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth) {

  float fGLI, fDiam, fDiamIncrement = 0, fTotalDiamIncrement = 0;
  int iSpecies = p_oTree->GetSpecies(),
      iType = p_oTree->GetType(),
      iLightCode = mp_oGrowthOrg->GetLightCode(iSpecies, iType),
      iDiamCode = mp_oGrowthOrg->GetDiamCode(iSpecies, iType),
      i;

  //Get the GLI and the diameter
  p_oTree->GetValue(iLightCode, &fGLI);
  p_oTree->GetValue(iDiamCode, &fDiam);

  //Increment the height
  for (i = 0; i < m_fNumberOfYearsPerTimestep; i++) {
    fDiamIncrement = CalculateFunctionValue(iSpecies, fGLI, fDiam) * m_fConversionFactor;
    fTotalDiamIncrement += fDiamIncrement;
    fDiam += fDiamIncrement;
  }

  return fTotalDiamIncrement;
}
