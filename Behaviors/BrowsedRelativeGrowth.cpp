//---------------------------------------------------------------------------
#include "BrowsedRelativeGrowth.h"
#include "GrowthOrg.h"
#include "Tree.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clBrowsedRelativeGrowth::clBrowsedRelativeGrowth(clSimManager *p_oSimManager) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
  clGrowthBase( p_oSimManager )
{
  m_sNameString = "browsedrelgrowthshell";
  m_sXMLRoot = "BrowsedRelativeGrowth";

  //Version 1
  m_fVersionNumber = 1;
  m_fMinimumVersionNumber = 1;

  mp_fUnbrowsedS = NULL;
  mp_fBrowsedS = NULL;
  mp_fUnbrowsedA = NULL;
  mp_fBrowsedA = NULL;
  mp_fUnbrowsedDiamExp = NULL;
  mp_fBrowsedDiamExp = NULL;
  mp_iLightCodes = NULL;
  mp_iBrowsedCodes = NULL;

  m_iNumSpecies = 0;
  m_fNumberYearsPerTimestep = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clBrowsedRelativeGrowth::~clBrowsedRelativeGrowth() {
  delete[] mp_fUnbrowsedS;
  delete[] mp_fBrowsedS;
  delete[] mp_fUnbrowsedA;
  delete[] mp_fBrowsedA;
  delete[] mp_fUnbrowsedDiamExp;
  delete[] mp_fBrowsedDiamExp;
  if ( mp_iBrowsedCodes )
  {
    for ( int i = 0; i < m_iNumSpecies; i++ )
    {
      delete[] mp_iBrowsedCodes[i];
    }
  }
  delete[] mp_iBrowsedCodes;

  if ( mp_iLightCodes )
  {
    for ( int i = 0; i < m_iNumSpecies; i++ )
    {
      delete[] mp_iLightCodes[i];
    }
  }
  delete[] mp_iLightCodes;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clBrowsedRelativeGrowth::SetNameData(std::string sNameString)
{
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("BrowsedRelativeGrowth") == 0 )
    {
      m_iGrowthMethod = diameter_auto;
    }
    else if (sNameString.compare("BrowsedRelativeGrowth diam only") == 0 )
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
    stcErr.sFunction = "clBrowsedRelativeGrowth::SetNameData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
//////////////////////////////////////////////////////////////////////////////
void clBrowsedRelativeGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  floatVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int i, j, iNumTypes = p_oPop->GetNumberOfTypes();

    m_iNumSpecies = p_oPop->GetNumberOfSpecies();

    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays
    mp_fUnbrowsedS = new float[m_iNumSpecies];
    mp_fBrowsedS = new float[m_iNumSpecies];
    mp_fUnbrowsedA = new float[m_iNumSpecies];
    mp_fBrowsedA = new float[m_iNumSpecies];
    mp_fUnbrowsedDiamExp = new float[m_iNumSpecies];
    mp_fBrowsedDiamExp = new float[m_iNumSpecies];

    //Unbrowsed S
    FillSpeciesSpecificValue(p_oElement, "gr_slopeGrowthResponse", "gr_sgrVal",
                          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fUnbrowsedS[p_fTempValues[i].code] = p_fTempValues[i].val;


    //Unbrowsed A
    FillSpeciesSpecificValue(p_oElement, "gr_asympDiameterGrowth", "gr_adgVal",
                          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fUnbrowsedA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Unbrowsed diameter exponent
    FillSpeciesSpecificValue( p_oElement, "gr_relGrowthDiamExp", "gr_rgdeVal",
                          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fUnbrowsedDiamExp[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Browsed S
    FillSpeciesSpecificValue(p_oElement, "gr_browsedSlopeGrowthResponse",
       "gr_bsgrVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fBrowsedS[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Browsed A
    FillSpeciesSpecificValue(p_oElement, "gr_browsedAsympDiameterGrowth",
       "gr_badgVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fBrowsedA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Browsed diameter exponent
    FillSpeciesSpecificValue( p_oElement, "gr_browsedRelGrowthDiamExp",
        "gr_brgdeVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBrowsedDiamExp[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Scale the browsed and unbrowsed S values
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      if (mp_fBrowsedS[mp_iWhatSpecies[i]] != 0.0) {
        mp_fBrowsedS[mp_iWhatSpecies[i]] =
                                          mp_fBrowsedA[mp_iWhatSpecies[i]] /
                                          mp_fBrowsedS[mp_iWhatSpecies[i]];
      }
      if (mp_fUnbrowsedS[mp_iWhatSpecies[i]] != 0.0) {
        mp_fUnbrowsedS[mp_iWhatSpecies[i]] =
                                           mp_fUnbrowsedA[mp_iWhatSpecies[i]] /
                                           mp_fUnbrowsedS[mp_iWhatSpecies[i]];
      }
    }

    //Collect the "Light" codes
    mp_iLightCodes = new short int *[m_iNumSpecies];
    mp_iBrowsedCodes = new short int *[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++) {
      mp_iLightCodes[i] = new short int[iNumTypes];
      mp_iBrowsedCodes[i] = new short int[iNumTypes];
      for (j = 0; j < iNumTypes; j++) {
        mp_iLightCodes[i][j] = -1;
        mp_iBrowsedCodes[i][j] = -1;
      }
    }
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      mp_iLightCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                    [mp_whatSpeciesTypeCombos[i].iType] =
            mp_oGrowthOrg->GetLightCode(mp_whatSpeciesTypeCombos[i].iSpecies,
                                           mp_whatSpeciesTypeCombos[i].iType );
      if (-1 ==  mp_iLightCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                               [mp_whatSpeciesTypeCombos[i].iType] )
      {
        modelErr stcErr;
        stcErr.sFunction = "clBrowsedRelativeGrowth::DoShellSetup" ;
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " does not have a required light behavior.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      mp_iBrowsedCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                      [mp_whatSpeciesTypeCombos[i].iType] =
            p_oPop->GetBoolDataCode("Browsed",
                                    mp_whatSpeciesTypeCombos[i].iSpecies,
                                    mp_whatSpeciesTypeCombos[i].iType );
      if (-1 ==  mp_iBrowsedCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                                 [mp_whatSpeciesTypeCombos[i].iType] )
      {
        modelErr stcErr;
        stcErr.sFunction = "clBrowsedRelativeGrowth::DoShellSetup" ;
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " does not have the browse behavior.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
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
    stcErr.sFunction = "clBrowsedRelativeGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clBrowsedRelativeGrowth::CalcDiameterGrowthValue( clTree * p_oTree, clTreePopulation * p_oPop, float fHeightGrowth )
{
  float fAnnualRelativeGrowth, //amount of annual relative growth
  fGli, //tree's gli value
  fDiam, //tree's diameter value
  fCompoundRelativeGrowth, //relative growth compounded to a complete
  //timestep
  fAmountDiamIncrease; //amount by which the tree's diameter will
  //increase
  short int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType();
  bool bBrowsed;

  //***************************************
  // Calculate the amount of growth
  //***************************************

  //Get the gli value for this tree
  p_oTree->GetValue( mp_iLightCodes[iSpecies][iType], &fGli );

  //Get the browse status for this tree
  p_oTree->GetValue( mp_iBrowsedCodes[iSpecies][iType], &bBrowsed);

  //Get the appropriate diameter for this tree
  p_oTree->GetValue( mp_oGrowthOrg->GetDiamCode( iSpecies, iType ), & fDiam );

  //Get the value of the Michaelis-Menton function
  if (bBrowsed) {
    fAnnualRelativeGrowth = (mp_fBrowsedA[iSpecies] * fGli) /
        (mp_fBrowsedS[iSpecies] + fGli);
    //Compound the relative growth over the number of years/time step
    fCompoundRelativeGrowth = pow( 1.0 + fAnnualRelativeGrowth,
        m_fNumberYearsPerTimestep );

    //Calculate amount of diameter increase based on the tree's diameter
    fAmountDiamIncrease = pow(fDiam, mp_fBrowsedDiamExp[iSpecies]) *
        ( fCompoundRelativeGrowth - 1.0 );
  } else {
    fAnnualRelativeGrowth = (mp_fUnbrowsedA[iSpecies] * fGli) /
        (mp_fUnbrowsedS[iSpecies] + fGli);
    //Compound the relative growth over the number of years/time step
    fCompoundRelativeGrowth = pow( 1.0 + fAnnualRelativeGrowth,
        m_fNumberYearsPerTimestep );

    //Calculate amount of diameter increase based on the tree's diameter
    fAmountDiamIncrease = pow(fDiam, mp_fUnbrowsedDiamExp[iSpecies]) *
        ( fCompoundRelativeGrowth - 1.0 );
  }

  return fAmountDiamIncrease;
}
