//---------------------------------------------------------------------------
// SimpleLinearGrowth.cpp
//---------------------------------------------------------------------------
#include "SimpleLinearGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "GrowthOrg.h"
#include "ModelMath.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////*/
clSimpleLinearGrowth::clSimpleLinearGrowth(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clGrowthBase(p_oSimManager) {

  try
  {
    mp_fSlope = NULL;
    mp_fIntercept = NULL;
    m_fConversionFactor = 0;

    m_sNameString = "simplelineargrowthshell";
    m_sXMLRoot = "SimpleLinearGrowth";
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
    stcErr.sFunction = "clSimpleLinearGrowth::clSimpleLinearGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clSimpleLinearGrowth::~clSimpleLinearGrowth() {

  delete[] mp_fSlope;
  delete[] mp_fIntercept;
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clSimpleLinearGrowth::DoShellSetup(DOMDocument * p_oDoc) {
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    doubleVal * p_fTempValues; //for getting species-specific values
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Conversion factor
    if ( height_only == m_iGrowthMethod )
    {
      //The conversion factor needs to convert from cm to m and from
      //annual to timestep
      m_fConversionFactor = mp_oSimManager->GetNumberOfYearsPerTimestep() / 100.0;
    } else {
      //The conversion factor needs to convert from mm to cm, from annual to
      //timestep, and from radial to diameter
      m_fConversionFactor = mp_oSimManager->GetNumberOfYearsPerTimestep() / 10.0 * 2.0;
    }

    //Declare the arrays we'd like read
    mp_fSlope = new double[iNumSpecies];
    mp_fIntercept = new double[iNumSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

    if ( height_only == m_iGrowthMethod )
    {
      //Growth intercept
      FillSpeciesSpecificValue( p_oElement, "gr_heightSimpleLinearIntercept", "gr_hsliVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fIntercept[p_fTempValues[i].code] = p_fTempValues[i].val;

      //Growth slope
      FillSpeciesSpecificValue( p_oElement, "gr_heightSimpleLinearSlope", "gr_hslsVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSlope[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    else
    {
      //Growth intercept
      FillSpeciesSpecificValue( p_oElement, "gr_diamSimpleLinearIntercept", "gr_dsliVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fIntercept[p_fTempValues[i].code] = p_fTempValues[i].val;

      //Growth slope
      FillSpeciesSpecificValue( p_oElement, "gr_diamSimpleLinearSlope", "gr_dslsVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSlope[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Make sure all species/type combos have "Light" registered
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      if (-1 == mp_oGrowthOrg->GetLightCode(mp_whatSpeciesTypeCombos[i].iSpecies,
              mp_whatSpeciesTypeCombos[i].iType)) {
        modelErr stcErr;
        stcErr.sFunction = "clSimpleLinearGrowth::DoShellSetup";
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
    stcErr.sFunction = "clSimpleLinearGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
/////////////////////////////////////////////////////////////////////////////*/
void clSimpleLinearGrowth::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("SimpleLinearGrowth") == 0) {
      m_iGrowthMethod = diameter_auto;
    } else if (sNameString.compare("SimpleLinearGrowth diam only") == 0) {
      m_iGrowthMethod = diameter_only;
    } else if (sNameString.compare("SimpleLinearGrowth height only") == 0) {
      m_iGrowthMethod = height_only;
    }
    else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sFunction = "clSimpleLinearGrowth::SetNameData";
      stcErr.sMoreInfo = s.str();
throw(stcErr);
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimpleLinearGrowth::SetNameData";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalculateFunctionValue()
//////////////////////////////////////////////////////////////////////////////
inline float clSimpleLinearGrowth::CalculateFunctionValue(clTree *p_oTree) {

  float fGli;
  short int iSpecies = p_oTree->GetSpecies(),
      iLightCode = mp_oGrowthOrg->GetLightCode(iSpecies, p_oTree->GetType());

  //Get the gli value for this tree
  p_oTree->GetValue(iLightCode, &fGli);

  //Calculate the function value
  return clModelMath::CalcPointValue(fGli, mp_fSlope[iSpecies], mp_fIntercept[iSpecies]);
}

