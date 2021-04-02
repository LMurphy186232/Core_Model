//---------------------------------------------------------------------------
// MichMenPhotoinhibition.cpp
//---------------------------------------------------------------------------
#include "MichMenPhotoinhibition.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "GrowthOrg.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////*/
clMichMenPhotoinhibition::clMichMenPhotoinhibition( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clGrowthBase( p_oSimManager )
{
  try
  {
    m_sNameString = "michmenphotogrowthshell";
    m_sXMLRoot = "MichaelisMentenPhotoinhibitionGrowth";

    m_iGrowthMethod = height_only;
    mp_fAlpha = NULL;
    mp_fBeta = NULL;
    mp_fPhi = NULL;
    mp_fD = NULL;
    mp_iIndexes = NULL;

    m_iYearsPerTimestep = 0;
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
    stcErr.sFunction = "clMichMenPhotoinhibition::clMichMenPhotoinhibition" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clMichMenPhotoinhibition::~clMichMenPhotoinhibition()
{
  delete[] mp_fAlpha;
  delete[] mp_fBeta;
  delete[] mp_fPhi;
  delete[] mp_fD;
  delete[] mp_iIndexes;
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clMichMenPhotoinhibition::DoShellSetup( DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int i;

    //Get number of years per timestep
    m_iYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    mp_fAlpha = new double[m_iNumBehaviorSpecies];
    mp_fBeta = new double[m_iNumBehaviorSpecies];
    mp_fPhi = new double[m_iNumBehaviorSpecies];
    mp_fD = new double[m_iNumBehaviorSpecies];
    mp_iIndexes = new int[p_oPop->GetNumberOfSpecies()];

    //Set up the array indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Alpha
    FillSpeciesSpecificValue( p_oElement, "gr_mmPhotGrowthAlpha",
        "gr_mmpgaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fAlpha[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Beta
    FillSpeciesSpecificValue( p_oElement, "gr_mmPhotGrowthBeta",
        "gr_mmpgbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array while making sure none of them
    //are 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val == 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMichMenPhotoinhibition::DoShellSetup";
        stcErr.sMoreInfo = "Beta values cannot equal 0.";
        throw(stcErr);
      }
      mp_fBeta[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }


    //D
    FillSpeciesSpecificValue( p_oElement, "gr_mmPhotGrowthD",
        "gr_mmpgdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fD[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Phi
    FillSpeciesSpecificValue( p_oElement, "gr_mmPhotGrowthPhi",
        "gr_mmpgpVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fPhi[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    delete[] p_fTempValues;

    //Make sure all species/type combos have "Light" registered
    for ( int i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      if ( -1 == mp_oGrowthOrg->GetLightCode(mp_whatSpeciesTypeCombos[i].iSpecies,
                                           mp_whatSpeciesTypeCombos[i].iType ) )
      {
        modelErr stcErr;
        stcErr.sFunction = "clRelativeGrowth::DoShellSetup" ;
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " does not have a required light behavior.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }
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
    stcErr.sFunction = "clMichMenPhotoinhibition::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcHeightGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clMichMenPhotoinhibition::CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth) {
  float fAnnualRelGrowth, //amount of annual relative growth
  fGli, //tree's gli value
  fHeight, //tree's height value
  fNewHeight, //tree's new height value
  fAmountHeightIncrease; //amount by which the tree's height will
  //increase
  short int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType(),
      iLightCode = mp_oGrowthOrg->GetLightCode( iSpecies, iType ),
      ind = mp_iIndexes[iSpecies],
      iYr;

  //Get the gli value for this tree
  p_oTree->GetValue( iLightCode, & fGli );

  //Get the height for this tree
  p_oTree->GetValue( p_oPop->GetHeightCode( iSpecies, iType ), & fHeight );
  fHeight *= 100.0; //transform to cm
  fNewHeight = fHeight;

  //Get the value of the Michaelis-Menton function
  fAnnualRelGrowth = (mp_fAlpha[ind] /
      (1+(mp_fAlpha[ind]/(mp_fBeta[ind]*fGli)))) - mp_fD[ind]*fGli ;

  //Compound the relative growth over the number of years/time step
  for (iYr = 0; iYr < m_iYearsPerTimestep; iYr++) {
    fNewHeight += (fAnnualRelGrowth * pow(fNewHeight, mp_fPhi[ind]));
  }
  //Don't allow a negative new height - set to 10 cm
  if (fNewHeight <= 10.0) fNewHeight = 10.0;

  fAmountHeightIncrease = fNewHeight - fHeight;
  //Transform to m
  fAmountHeightIncrease /= 100.0;

  return fAmountHeightIncrease;
}
