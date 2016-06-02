//---------------------------------------------------------------------------
#include "RelativeGrowth.h"
#include "GrowthOrg.h"
#include "Tree.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include <math.h>
#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clRelativeGrowth::clRelativeGrowth(clSimManager *p_oSimManager) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
  clGrowthBase( p_oSimManager ), clMichMenBase(p_oSimManager)
 {
    m_sNameString = "relativegrowthshell";
    //Version 1.1
    m_fVersionNumber = 1.2;
    m_fMinimumVersionNumber = 1.1;
    mp_fExp = NULL;

    m_iNumberYearsPerTimestep = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clRelativeGrowth::~clRelativeGrowth() {
  delete[] mp_fExp;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clRelativeGrowth::SetNameData(std::string sNameString)
{
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("RelRadialGrowth") == 0 )
    {
      m_bConstRadialLimited = true;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = diameter_auto;
      m_sXMLRoot = "RelRadialGrowth";
    }
    else if (sNameString.compare("RelBAGrowth") == 0 )
    {
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = true;
      m_iGrowthMethod = diameter_auto;
      m_sXMLRoot = "RelBAGrowth";
    }
    else if (sNameString.compare("RelUnlimGrowth") == 0 )
    {
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = diameter_auto;
      m_sXMLRoot = "RelUnlimGrowth";
    }
    else if (sNameString.compare("RelRadialGrowth diam only") == 0 )
    {
      m_bConstRadialLimited = true;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = diameter_only;
      m_sXMLRoot = "RelRadialGrowth";
    }
    else if (sNameString.compare("RelBAGrowth diam only") == 0 )
    {
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = true;
      m_iGrowthMethod = diameter_only;
      m_sXMLRoot = "RelBAGrowth";
    }
    else if (sNameString.compare("RelUnlimGrowth diam only") == 0 )
    {
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = diameter_only;
      m_sXMLRoot = "RelUnlimGrowth";
    }
    else if (sNameString.compare("RelativeHeight") == 0 )
    {
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = height_only;
      m_sXMLRoot = "RelMMHeight";
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
    stcErr.sFunction = "clRelativeGrowth::SetNameData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
//////////////////////////////////////////////////////////////////////////////
void clRelativeGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    doubleVal * p_fTempValues; //for getting species-specific values
    short int iNumSpecies = mp_oGrowthOrg->GetNumberOfSpecies(), i;

    //Set up our doubleVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    m_iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    if (diameter_auto == m_iGrowthMethod || diameter_only == m_iGrowthMethod) {
      mp_fAsympDiamGrowth = new double[iNumSpecies];
      mp_fSlopeDiamGrowthResponse = new double[iNumSpecies];
    } else {
      mp_fAsympHeightGrowth = new double[iNumSpecies];
      mp_fSlopeHeightGrowthResponse = new double[iNumSpecies];
    }

    mp_fExp = new double[iNumSpecies];

    if ( m_bConstRadialLimited )
    {
      mp_fAdultConstRadInc = new double[iNumSpecies];
    }

    if ( m_bConstBasalAreaLimited )
    {
      mp_fAdultConstBAInc = new double[iNumSpecies];
    }

    //Read the base variables
    GetParameterFileData( p_oDoc );

    if (diameter_auto == m_iGrowthMethod || diameter_only == m_iGrowthMethod) {
      //Get the diameter exponent
      FillSpeciesSpecificValue( p_oElement, "gr_relGrowthDiamExp", "gr_rgdeVal", p_fTempValues,
           m_iNumBehaviorSpecies, p_oPop, true );
    } else {
      //Get the height exponent
      FillSpeciesSpecificValue( p_oElement, "gr_relHeightGrowthHeightExp", "gr_rhgheVal", p_fTempValues,
           m_iNumBehaviorSpecies, p_oPop, true );
    }

    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fExp[p_fTempValues[i].code] = p_fTempValues[i].val;


    //Make sure all species/type combos have "Light" registered
    for ( int i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      if ( -1 == mp_oGrowthOrg->GetLightCode(mp_whatSpeciesTypeCombos[i].iSpecies,
                                           mp_whatSpeciesTypeCombos[i].iType ) )
      {
        modelErr stcErr;
        stcErr.sFunction = "clRelativeGrowth::DoShellSetup";
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
    stcErr.sFunction = "clRelativeGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clRelativeGrowth::CalcDiameterGrowthValue( clTree * p_oTree, clTreePopulation * p_oPop, float fHeightGrowth )
{
  float fAnnualRelativeGrowth, //amount of annual relative growth
        fGli, //tree's gli value
        fDiam, //tree's diameter value
        fCompoundRelativeGrowth, //relative growth compounded to a complete
        //timestep
        fAmountDiamIncrease; //amount by which the tree's diameter will
                             //increase
  short int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType(),
            iLightCode = mp_oGrowthOrg->GetLightCode( iSpecies, iType );

  //***************************************
  // Calculate the amount of growth
  //***************************************

  //Get the gli value for this tree
  p_oTree->GetValue( iLightCode, & fGli );

  //Get the appropriate diameter for this tree
  p_oTree->GetValue( mp_oGrowthOrg->GetDiamCode( iSpecies, iType ), & fDiam );

  //Get the value of the Michaelis-Menton function
  fAnnualRelativeGrowth = CalculateMichaelisMentonDiam( iSpecies, fGli );

  //Compound the relative growth over the number of years/time step
  fCompoundRelativeGrowth = pow( 1.0 + fAnnualRelativeGrowth, m_iNumberYearsPerTimestep );

  //Calculate amount of diameter increase based on the tree's diameter
  fAmountDiamIncrease = pow(fDiam, mp_fExp[iSpecies]) * ( fCompoundRelativeGrowth - 1.0 );

  fAmountDiamIncrease = ApplyGrowthLimits( iSpecies, fAmountDiamIncrease, fDiam );

  return fAmountDiamIncrease;
}

//////////////////////////////////////////////////////////////////////////////
// CalcHeightGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clRelativeGrowth::CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth)
{
  float fAnnualRelativeGrowth, //amount of annual relative growth
       fGli, //tree's gli value
       fHeight, //tree's height value
       fNewHeight, //tree's new height value
       fAmountHeightIncrease; //amount by which the tree's height will
                              //increase
  short int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType(),
            iLightCode = mp_oGrowthOrg->GetLightCode( iSpecies, iType ),
            iYr;

  //Get the gli value for this tree
  p_oTree->GetValue( iLightCode, & fGli );

  //Get the height for this tree
  p_oTree->GetValue( p_oPop->GetHeightCode( iSpecies, iType ), & fHeight );
  fHeight *= 100.0; //transform to cm
  fNewHeight = fHeight;

  //Get the value of the Michaelis-Menton function
  fAnnualRelativeGrowth = CalculateMichaelisMentonHeight( iSpecies, fGli );

  //Compound the relative growth over the number of years/time step
  for (iYr = 0; iYr < m_iNumberYearsPerTimestep; iYr++) {
    fNewHeight += fAnnualRelativeGrowth * pow(fNewHeight, mp_fExp[iSpecies]);
  }
  //Don't allow a negative new height - set to 10 cm
  if (fNewHeight <= 10.0) fNewHeight = 10.0;

  fAmountHeightIncrease = fNewHeight - fHeight;
  //Transform to m
  fAmountHeightIncrease /= 100.0;

  return fAmountHeightIncrease;
}

