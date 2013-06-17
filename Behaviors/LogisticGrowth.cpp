//---------------------------------------------------------------------------
// LogisticGrowth.cpp
//---------------------------------------------------------------------------
#include "LogisticGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "GrowthOrg.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clLogisticGrowth::clLogisticGrowth( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clGrowthBase( p_oSimManager )
  {

  try
  {
    mp_fAsympGrowthAtFullLight = NULL;
    mp_fShape1 = NULL;
    mp_fShape2 = NULL;
    mp_iIndexes = NULL;

    m_sNameString = "logisticgrowthshell";
    m_sXMLRoot = "LogisticGrowth";

    m_fConversionFactor = 0;
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
    stcErr.sFunction = "clLogisticGrowth::clLogisticGrowth" ;
    throw( stcErr );
  }
}



//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clLogisticGrowth::~clLogisticGrowth()
{

  delete[] mp_fAsympGrowthAtFullLight;
  delete[] mp_fShape1;
  delete[] mp_fShape2;
  delete[] mp_iIndexes;
}



//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
//////////////////////////////////////////////////////////////////////////////
void clLogisticGrowth::DoShellSetup( DOMDocument * p_oDoc )
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
      //The conversion factor needs to convert from cm to m and from
      //annual to timestep
      m_fConversionFactor = mp_oSimManager->GetNumberOfYearsPerTimestep() / 100;
    } else {
      //The conversion factor needs to convert from mm to cm, from annual to
      //timestep, and from radial to diameter
      m_fConversionFactor = mp_oSimManager->GetNumberOfYearsPerTimestep() / 10 * 2;
    }

    //Make the list of indexes
    mp_iIndexes = new short int[iNumSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the arrays we'd like read
    mp_fAsympGrowthAtFullLight = new float[m_iNumBehaviorSpecies];
    mp_fShape1 = new float[m_iNumBehaviorSpecies];
    mp_fShape2 = new float[m_iNumBehaviorSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    if ( height_only == m_iGrowthMethod )
    {
      //Asymptotic growth at full light
      FillSpeciesSpecificValue( p_oElement, "gr_heightAsympGrowthAtFullLight", "gr_hagaflVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fAsympGrowthAtFullLight[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shape parameter 1
      FillSpeciesSpecificValue( p_oElement, "gr_heightShape1b", "gr_hs1bVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShape1[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shape parameter 2
      FillSpeciesSpecificValue( p_oElement, "gr_heightShape2c", "gr_hs2cVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShape2[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }
    else
    {
      //Asymptotic growth at full light
      FillSpeciesSpecificValue( p_oElement, "gr_diamAsympGrowthAtFullLight", "gr_dagaflVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fAsympGrowthAtFullLight[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shape parameter 1
      FillSpeciesSpecificValue( p_oElement, "gr_diamShape1b", "gr_ds1bVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fShape1[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Shape parameter 2
      FillSpeciesSpecificValue( p_oElement, "gr_diamShape2c", "gr_ds2cVal", p_fTempValues,
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
        stcErr.sFunction = "clLogisticGrowth::DoShellSetup" ;
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies
          << " type=" << mp_whatSpeciesTypeCombos[i].iType
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
    stcErr.sFunction = "clLogisticGrowth::GetNameData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clLogisticGrowth::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("LogisticGrowth") == 0) {
      m_iGrowthMethod = diameter_auto;
    } else if (sNameString.compare("LogisticGrowth diam only") == 0) {
      m_iGrowthMethod = diameter_only;
    } else if (sNameString.compare("LogisticGrowth height only") == 0) {
      m_iGrowthMethod = height_only;
    }
    else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sFunction = "clLogisticGrowth::SetNameData";
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clLogisticGrowth::SetNameData";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalculateFunctionValue()
//////////////////////////////////////////////////////////////////////////////
float clLogisticGrowth::CalculateFunctionValue(clTree *p_oTree) {
    float fGli;
    short int iSpecies = p_oTree->GetSpecies(),
    iIndex = mp_iIndexes[iSpecies],
    iLightCode = mp_oGrowthOrg->GetLightCode(iSpecies, p_oTree->GetType());

    //Get the gli value for this tree
    p_oTree->GetValue(iLightCode, &fGli);

    //Calculate the function value
    return mp_fAsympGrowthAtFullLight[iIndex] /
    (1 + exp(mp_fShape1[iIndex] - (mp_fShape2[iIndex] * fGli)));
}
