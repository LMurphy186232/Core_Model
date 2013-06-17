//---------------------------------------------------------------------------
// DoubleMMRelGrowth.cpp
//---------------------------------------------------------------------------
#include "DoubleMMRelGrowth.h"
#include "GrowthOrg.h"
#include "Tree.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "Grid.h"
#include "ParsingFunctions.h"
#include <math.h>
#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clDoubleMMRelGrowth::clDoubleMMRelGrowth( clSimManager * p_oSimManager ) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clGrowthBase( p_oSimManager), clMichMenBase( p_oSimManager )
{

  m_sNameString = "doublemmrelgrowthshell";
  m_sXMLRoot = "DoubleResourceRelative";

  mp_fResourceInfluence = NULL;
  m_bConstRadialLimited = false;
  m_bConstBasalAreaLimited = false;

  mp_oResourceGrid = NULL;
  m_fNumberYearsPerTimestep = 0;
  m_iResourceCode = -1;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clDoubleMMRelGrowth::~clDoubleMMRelGrowth()
{
  delete[] mp_fResourceInfluence;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clDoubleMMRelGrowth::SetNameData(std::string sNameString)
{
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("DoubleResourceRelative") == 0 )
    {
      m_iGrowthMethod = diameter_auto;
    }
    else if (sNameString.compare("DoubleResourceRelative diam only") == 0 )
    {
      m_iGrowthMethod = diameter_only;
    }
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
    stcErr.sFunction = "clDoubleMMRelGrowth::SetNameData" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
//////////////////////////////////////////////////////////////////////////////
void clDoubleMMRelGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  floatVal * p_fTempValues = NULL;
  try
  {
    short int iNumSpecies = mp_oGrowthOrg->GetNumberOfSpecies(), i;

    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    mp_fAsympDiamGrowth = new float[iNumSpecies];
    mp_fSlopeDiamGrowthResponse = new float[iNumSpecies];
    mp_fResourceInfluence = new float[iNumSpecies];

    //Read the base variables with the base class function
    GetParameterFileData( p_oDoc );

    //Now read this behavior's additional parameter
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];
    FillSpeciesSpecificValue( p_oElement, "gr_diamDoubleMMResourceInfluence", "gr_ddmmriVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fResourceInfluence[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //UNSCALE the slope of growth response parameter to undo what the base
    //function did because we don't want it
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( mp_fSlopeDiamGrowthResponse[mp_iWhatSpecies[i]] != 0.0 )
        mp_fSlopeDiamGrowthResponse[mp_iWhatSpecies[i]] =
            ( 1 / mp_fSlopeDiamGrowthResponse[mp_iWhatSpecies[i]] ) * mp_fAsympDiamGrowth[mp_iWhatSpecies[i]];

    //Make sure all species/type combos have "Light" registered
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      if ( -1 == mp_oGrowthOrg->GetLightCode( mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType ) )
      {
        modelErr stcErr;
        stcErr.sFunction = "clDoubleMMRelGrowth::DoShellSetup" ;
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

    //Get the "Resource" grid
    mp_oResourceGrid = mp_oSimManager->GetGridObject( "Resource" );
    if ( NULL == mp_oResourceGrid )
    {
      modelErr stcErr;
      stcErr.sFunction = "clDoubleMMRelGrowth::DoShellSetup" ;
      stcErr.sMoreInfo = "Can't find required grid object \"Resources\".";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw( stcErr );
    }

    m_iResourceCode = mp_oResourceGrid->GetFloatDataCode( "Resource" );
    if ( -1 == m_iResourceCode )
    {
      modelErr stcErr;
      stcErr.sFunction = "clDoubleMMRelGrowth::DoShellSetup" ;
      stcErr.sMoreInfo = "Grid object \"Resources\" is set up incorrectly.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw( stcErr );
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
    stcErr.sFunction = "clDoubleMMRelGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clDoubleMMRelGrowth::CalcDiameterGrowthValue( clTree * p_oTree, clTreePopulation * p_oPop, float fHeightGrowth )
{
  float fAnnualRelativeGrowth, //amount of annual relative growth
  fGli, //tree's gli value
  fDiam, //tree's diameter value
  fCompoundRelativeGrowth, //relative growth compounded to a complete
  //timestep
  fResource, //amount of the second resource
  fX, fY, //tree's coordinates
  fAmountDiamIncrease; //amount by which the tree's diameter will
  //increase
  short int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType(),
      iLightCode = mp_oGrowthOrg->GetLightCode( iSpecies, iType );

  //***************************************
  // Calculate the amount of growth
  //***************************************

  //Get the gli value for this tree
  p_oTree->GetValue( iLightCode, & fGli );

  //Get the proper resource level from the tree's coordinates
  p_oTree->GetValue(p_oPop->GetXCode(iSpecies, iType), &fX);
  p_oTree->GetValue(p_oPop->GetYCode(iSpecies, iType), &fY);
  mp_oResourceGrid->GetValueAtPoint(fX, fY, m_iResourceCode, &fResource);

  //Get the appropriate diameter for this tree
  p_oTree->GetValue( mp_oGrowthOrg->GetDiamCode( iSpecies, iType ), & fDiam );

  //Get the value of the Michaelis-Menton function
  fAnnualRelativeGrowth = mp_fAsympDiamGrowth[iSpecies] + (mp_fResourceInfluence[iSpecies] * fResource);
  fAnnualRelativeGrowth = ( fAnnualRelativeGrowth * fGli ) / ( (fAnnualRelativeGrowth / mp_fSlopeDiamGrowthResponse[iSpecies]) + fGli );

  //Compound the relative growth over the number of years/time step
  fCompoundRelativeGrowth = pow( 1.0 + fAnnualRelativeGrowth, m_fNumberYearsPerTimestep );

  //Calculate amount of diameter increase based on the tree's diameter
  fAmountDiamIncrease = fDiam * ( fCompoundRelativeGrowth - 1.0 );

  return fAmountDiamIncrease;
}
