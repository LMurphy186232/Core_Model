//---------------------------------------------------------------------------
// SizeDepLogisticGrowth.cpp
//---------------------------------------------------------------------------
#include "SizeDepLogisticGrowth.h"
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
clSizeDepLogisticGrowth::clSizeDepLogisticGrowth( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clGrowthBase( p_oSimManager )
{

  try
  {
    mp_fSlope = NULL;
    mp_fIntercept = NULL;
    mp_fShape1 = NULL;
    mp_fShape2 = NULL;
    mp_iIndexes = NULL;

    m_fConversionFactor = 0;
    m_iNumberOfYearsPerTimestep = 0;

    m_sNameString = "sizedeplogisticgrowthshell";
    m_sXMLRoot = "SizeDependentLogisticGrowth";
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
    stcErr.sFunction = "clSizeDepLogisticGrowth::clSizeDepLogisticGrowth" ;
    throw( stcErr );
  }
}



//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clSizeDepLogisticGrowth::~clSizeDepLogisticGrowth()
{
  delete[] mp_fSlope;
  delete[] mp_fIntercept;
  delete[] mp_fShape1;
  delete[] mp_fShape2;
  delete[] mp_iIndexes;
}



//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clSizeDepLogisticGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    doubleVal * p_fTempValues; //for getting species-specific values
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

    m_iNumberOfYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Make the list of indexes
    mp_iIndexes = new short int[iNumSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the arrays we'd like read
    mp_fSlope = new double[m_iNumBehaviorSpecies];
    mp_fIntercept = new double[m_iNumBehaviorSpecies];
    mp_fShape1 = new double[m_iNumBehaviorSpecies];
    mp_fShape2 = new double[m_iNumBehaviorSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    if ( height_only == m_iGrowthMethod )
    {
      //Growth intercept
      FillSpeciesSpecificValue( p_oElement, "gr_heightSizeDepLogisticIntercept", "gr_hsdliVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fIntercept[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Growth slope
      FillSpeciesSpecificValue( p_oElement, "gr_heightSizeDepLogisticSlope", "gr_hsdlsVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shape parameter 1
      FillSpeciesSpecificValue( p_oElement, "gr_heightSizeDepLogisticShape1c", "gr_hsdls1cVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShape1[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shape parameter 2
      FillSpeciesSpecificValue( p_oElement, "gr_heightSizeDepLogisticShape2d", "gr_hsdls2dVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShape2[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    }
    else
    {
      //Growth intercept
      FillSpeciesSpecificValue( p_oElement, "gr_diamSizeDepLogisticIntercept", "gr_dsdliVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fIntercept[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Growth slope
      FillSpeciesSpecificValue( p_oElement, "gr_diamSizeDepLogisticSlope", "gr_dsdlsVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shape parameter 1
      FillSpeciesSpecificValue( p_oElement, "gr_diamSizeDepLogisticShape1c", "gr_dsdls1cVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShape1[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shape parameter 2
      FillSpeciesSpecificValue( p_oElement, "gr_diamSizeDepLogisticShape2d", "gr_dsdls2dVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShape2[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    }

    //Make sure all species/type combos have "Light" registered
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      if (-1 == mp_oGrowthOrg->GetLightCode(mp_whatSpeciesTypeCombos[i].iSpecies,
          mp_whatSpeciesTypeCombos[i].iType)) {
        modelErr stcErr;
        stcErr.sFunction = "clSizeDepLogisticGrowth::DoShellSetup";
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
    stcErr.sFunction = "clSizeDepLogisticGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
/////////////////////////////////////////////////////////////////////////////*/
void clSizeDepLogisticGrowth::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("SizeDependentLogisticGrowth") == 0) {
      m_iGrowthMethod = diameter_auto;
    } else if (sNameString.compare("SizeDependentLogisticGrowth diam only") == 0) {
      m_iGrowthMethod = diameter_only;
    } else if (sNameString.compare("SizeDependentLogisticGrowth height only") == 0) {
      m_iGrowthMethod = height_only;
      m_bGoLast = true;
    }
    else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sFunction = "clSizeDepLogisticGrowth::SetNameData";
      stcErr.sMoreInfo = s.str();
throw(stcErr);
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSizeDepLogisticGrowth::SetNameData";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalculateFunctionValue()
/////////////////////////////////////////////////////////////////////////////*/
float clSizeDepLogisticGrowth::CalculateFunctionValue(int iSpecies, float fGLI, float fDiam) {

  short int iIndex = mp_iIndexes[iSpecies];

  //Calculate the function value
  return clModelMath::CalcPointValue(fDiam, mp_fSlope[iIndex], mp_fIntercept[iIndex]) /
      (1 + exp(mp_fShape1[iIndex] - fGLI*mp_fShape2[iIndex]));

}


//////////////////////////////////////////////////////////////////////////////
// CalcHeightGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clSizeDepLogisticGrowth::CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth) {

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
  fDiamIncrement = fDiameterGrowth / m_iNumberOfYearsPerTimestep;

  //Increment the height
  for (i = 0; i < m_iNumberOfYearsPerTimestep; i++) {
    fHeightIncrement += CalculateFunctionValue(iSpecies, fGLI, fDiam);
    fDiam += fDiamIncrement;
  }

  return fHeightIncrement * m_fConversionFactor;

}


//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clSizeDepLogisticGrowth::CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth) {

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
  for (i = 0; i < m_iNumberOfYearsPerTimestep; i++) {
    fDiamIncrement = CalculateFunctionValue(iSpecies, fGLI, fDiam) * m_fConversionFactor;
    fTotalDiamIncrement += fDiamIncrement;
    fDiam += fDiamIncrement;
  }

  return fTotalDiamIncrement;
}
