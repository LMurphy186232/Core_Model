//---------------------------------------------------------------------------
#include "NCIMasterGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include "Allometry.h"

#include "NCI/CrowdingEffectBase.h"
#include "NCI/DamageEffectBase.h"
#include "NCI/NCITermBase.h"
#include "NCI/ShadingEffectBase.h"
#include "NCI/SizeEffectBase.h"
#include "NCI/TemperatureEffectBase.h"
#include "NCI/PrecipitationEffectBase.h"
#include "NCI/InfectionEffectBase.h"
#include "NCI/NitrogenEffectBase.h"

#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIMasterGrowth::clNCIMasterGrowth(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clGrowthBase(p_oSimManager), clNCIBehaviorBase() {

  //Set namestring
  m_sNameString = "ncigrowthshell";
  m_sXMLRoot = "NCIMasterGrowth";

  //Version 3
  m_fVersionNumber = 3.0;
  m_fMinimumVersionNumber = 3.0;

  //Null out our pointers
  mp_iGrowthCodes = NULL;
  mp_fMaxPotentialValue = NULL;
  mp_fRandParameter = NULL;

  m_iStochasticGrowthMethod = deterministic_pdf;
  Adjust = NULL;

  m_iNumTotalSpecies = 0;

}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clNCIMasterGrowth::~clNCIMasterGrowth() {

  int i;
  if (mp_iGrowthCodes) {
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      delete[] mp_iGrowthCodes[i];
    }
    delete[] mp_iGrowthCodes;
  }
  delete[] mp_fMaxPotentialValue;
  delete[] mp_fRandParameter;
}


////////////////////////////////////////////////////////////////////////////
// GetTreeMemberCodes()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::GetTreeMemberCodes() {
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  int iNumTypes = p_oPop->GetNumberOfTypes(),
      i, j;

  //Get codes for growth
  mp_iGrowthCodes = new short int * [m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_iGrowthCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iGrowthCodes[i][j] = -1;
    }
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    //Get the code from growth org
    mp_iGrowthCodes[mp_whatSpeciesTypeCombos[i].iSpecies][mp_whatSpeciesTypeCombos[i].iType]
        = mp_oGrowthOrg->GetGrowthCode(mp_whatSpeciesTypeCombos[i].iSpecies,
            mp_whatSpeciesTypeCombos[i].iType);
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::DoShellSetup(xercesc::DOMDocument * p_oDoc) {
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  floatVal * p_fTempValues; //for getting species-specific values
  int iTemp;
  short int i; //loop counters

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Maximum potential growth
  mp_fMaxPotentialValue = new float[m_iNumTotalSpecies];
  FillSpeciesSpecificValue( p_oElement, "gr_nciMaxPotentialGrowth", "gr_nmpgVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fMaxPotentialValue[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Get the type of growth stochasticity desired
  FillSingleValue(p_oElement, "gr_stochGrowthMethod", &iTemp, true);

  //Make sure the value is valid
  if (deterministic_pdf == iTemp) {

    m_iStochasticGrowthMethod = deterministic_pdf;
    Adjust = & clNCIMasterGrowth::DeterministicAdjust;

  } else if (lognormal_pdf == iTemp) {

    m_iStochasticGrowthMethod = lognormal_pdf;
    Adjust = & clNCIMasterGrowth::LognormalAdjust;

  } else if (normal_pdf == iTemp) {

    m_iStochasticGrowthMethod = normal_pdf;
    Adjust = & clNCIMasterGrowth::NormalAdjust;

  } else {
    modelErr stcErr;
    stcErr.sFunction = "clNCIMasterGrowth::DoShellSetup";
    std::stringstream s;
    s << "Unrecognized value for gr_stochGrowthMethod: " << iTemp;
    stcErr.sMoreInfo = s.str();
    stcErr.iErrorCode = BAD_DATA;
    throw(stcErr);
  }

  //If lognormal or normal, populate the parameters array
  if (lognormal_pdf == m_iStochasticGrowthMethod ||
      normal_pdf == m_iStochasticGrowthMethod) {

    mp_fRandParameter = new float[m_iNumTotalSpecies];
    FillSpeciesSpecificValue(p_oElement, "gr_standardDeviation", "gr_sdVal",
          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fRandParameter[p_fTempValues[i].code] = p_fTempValues[i].val;

  }

  delete[] p_fTempValues;

  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    //Make sure that the maximum growth for each species is > 0
    if (mp_fMaxPotentialValue[mp_iWhatSpecies[i]] <= 0) {
      modelErr err;
      err.sFunction = "clNCIMasterGrowth::ValidateData";
      err.iErrorCode = BAD_DATA;
      err.sMoreInfo = "All values for max potential growth must be greater than 0.";
      throw(err);
    }
  }

  m_sQuery = FormatSpeciesTypeQueryString();
  ReadParameterFile(p_oElement, p_oPop, this, true);
  GetTreeMemberCodes();

}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::SetNameData(std::string sNameString) {
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("NCIMasterGrowth") == 0 )
    {
      m_iGrowthMethod = diameter_auto;
    }
    else if (sNameString.compare("NCIMasterGrowth diam only") == 0 )
    {
      m_iGrowthMethod = diameter_only;
    }
    else
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sMoreInfo = s.str();
      stcErr.sFunction = "clNCIMasterGrowth::SetNameData" ;
      throw( stcErr );
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
    stcErr.sFunction = "clNCIMasterGrowth::SetNameData" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// CalcGrowthValue
////////////////////////////////////////////////////////////////////////////
float clNCIMasterGrowth::CalcDiameterGrowthValue(clTree * p_oTree,
    clTreePopulation * p_oPop, float fHeightGrowth) {
  float fAmountDiamIncrease; //amount diameter increase

  //Get the tree's growth - it's already calculated
  p_oTree->GetValue( mp_iGrowthCodes[p_oTree->GetSpecies()][p_oTree->GetType()],
      & fAmountDiamIncrease );

  return fAmountDiamIncrease;
}

////////////////////////////////////////////////////////////////////////////
// PreGrowthCalcs
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::PreGrowthCalcs(clTreePopulation * p_oPop) {
  float *p_fTempEffect,
        *p_fPrecipEffect,
        *p_fNEffect;
  try
  {
#ifdef NCI_WRITER
    using namespace std;
    int iTS = mp_oSimManager->GetCurrentTimestep();
    char cFilename[100];
    sprintf(cFilename, "%s%d%s", "GrowthNCI", iTS, ".txt");
    fstream out( cFilename, ios::trunc | ios::out );
    out << "Timestep\tSpecies\tDBH\tNCI\tSize Effect\tCrowding Effect\tDamage Effect\tGrowth\n";
#endif

    clTreeSearch * p_oNCITrees; //trees that this growth behavior applies to
    clAllometry * p_oAllom = p_oPop->GetAllometryObject();
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTree * p_oTree; //a single tree we're working with
    clNCITermBase::ncivals nci;
    float fDamageEffect, //this tree's damage effect
    fCrowdingEffect, //tree's crowding effect
    fSizeEffect, //tree's size effect
    fShadingEffect, //tree's shading effect
    fInfectionEffect, //tree's infection effect
    fDiam, //tree's diameter
    fX, fY,
    fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(), fAmountDiamIncrease, //amount diameter increase
    fTempDiamIncrease; //amount diameter increase - intermediate
    int iIsDead;
    short int iSpecies, iType, //type and species of a tree
    i, //loop counter
    iDeadCode; //tree's dead code

    p_fPrecipEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fTempEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fNEffect = new float[p_oPop->GetNumberOfSpecies()];
    //Calculate climate effects for all species
    for (i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fPrecipEffect[mp_iWhatSpecies[i]] = mp_oPrecipEffect->CalculatePrecipitationEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fTempEffect[mp_iWhatSpecies[i]] = mp_oTempEffect->CalculateTemperatureEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fNEffect[mp_iWhatSpecies[i]] = mp_oNEffect->CalculateNitrogenEffect(p_oPlot, mp_iWhatSpecies[i]);
    }

    //Pre-calcs for other effects
    mp_oCrowdingEffect->PreCalcs(p_oPop);
    mp_oDamageEffect->PreCalcs(p_oPop);
    mp_oNCITerm->PreCalcs(p_oPop);
    mp_oShadingEffect->PreCalcs(p_oPop);
    mp_oSizeEffect->PreCalcs(p_oPop);
    mp_oInfectionEffect->PreCalcs(p_oPop);

    p_oNCITrees = p_oPop->Find( m_sQuery );

    //************************************
    // Loop through and to calculate growth for each tree
    //************************************
    p_oTree = p_oNCITrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      if ( -1 != mp_iGrowthCodes[iSpecies][iType]) {

        //Make sure tree's not dead
        iDeadCode = p_oPop->GetIntDataCode( "dead", p_oTree->GetSpecies(), p_oTree->GetType() );
        if ( -1 != iDeadCode )
        {
          p_oTree->GetValue( iDeadCode, & iIsDead );
        }
        else
        iIsDead = notdead;

        if (notdead == iIsDead) {

          if (iType == clTreePopulation::seedling) {
            p_oTree->GetValue(p_oPop->GetDiam10Code(iSpecies, iType), &fDiam);
          } else {
            p_oTree->GetValue(p_oPop->GetDbhCode(iSpecies, iType), &fDiam);
          }

          //First calculate the pieces that have no DBH component and thus will
          //not change in our loop

          //Get NCI
          p_oTree->GetValue( p_oPop->GetXCode( iSpecies, iType ), & fX );
          p_oTree->GetValue( p_oPop->GetYCode( iSpecies, iType ), & fY );
          nci = mp_oNCITerm->CalculateNCITerm(p_oTree, p_oPop, p_oPlot, fX, fY, iSpecies);

          //Get the tree's damage effect
          fDamageEffect = mp_oDamageEffect->CalculateDamageEffect(p_oTree);

          //Get the tree's shading effect
          fShadingEffect = mp_oShadingEffect->CalculateShadingEffect(p_oTree);

          //Get the tree's infection effect
          fInfectionEffect = mp_oInfectionEffect->CalculateInfectionEffect(p_oTree);

          //To correctly compound growth over the number of years per timestep,
          //we have to loop over the number of years, re-calculating the parts
          //with DBH and incrementing the DBH each time
          fAmountDiamIncrease = 0;
          for ( i = 0; i < fNumberYearsPerTimestep; i++ ) {

            fCrowdingEffect = mp_oCrowdingEffect->CalculateCrowdingEffect(p_oTree, fDiam, nci, iSpecies);

            //Get the tree's size effect
            fSizeEffect = mp_oSizeEffect->CalculateSizeEffect(p_oTree, fDiam);

            //Calculate actual growth in cm/yr
            fTempDiamIncrease = mp_fMaxPotentialValue[iSpecies] * fSizeEffect *
                fCrowdingEffect * fShadingEffect * fDamageEffect *
                p_fPrecipEffect[iSpecies] * p_fTempEffect[iSpecies] *
                p_fNEffect[iSpecies] * fInfectionEffect;

            //Add it to the running total of diameter increase
            fAmountDiamIncrease += fTempDiamIncrease;

            //Increase the DBH for the next loop.  If this is a sapling,
            //convert to a dbh value from what would be a diam10 increase
            if ( clTreePopulation::sapling == iType )
            {
              fDiam += p_oAllom->ConvertDiam10ToDbh( fTempDiamIncrease, iSpecies );
            }
            else
            {
              fDiam += fTempDiamIncrease;
            }
          }

          //Adjust stochastically (if deterministic, we'll get the same
          //number back
          fAmountDiamIncrease = (*this.*Adjust) (fAmountDiamIncrease, iSpecies);

          //Assign the growth back to "Growth" and hold it
          p_oTree->SetValue( mp_iGrowthCodes[iSpecies][iType], fAmountDiamIncrease );

#ifdef NCI_WRITER
            p_oTree->GetValue( p_oPop->GetDbhCode( iSpecies, iType ), & fDbh );
            out << iTS << "\t" << p_oTree->GetSpecies() << "\t" << fDbh
                << "\t" << fNCI << "\t" << fSizeEffect << "\t" << fCrowdingEffect
                << "\t" << fDamageEffect << "\t" << fAmountDiamIncrease << "\n";
#endif

        } //end of if (bIsNCITree)
      }

      p_oTree = p_oNCITrees->NextTree();
    }

    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;

#ifdef NCI_WRITER
    out.close();
#endif

  }
  catch ( modelErr & err )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;
    throw( err );
  }
  catch ( ... )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clNCIMasterGrowth::PreCalcGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// DeterministicAdjust()
/////////////////////////////////////////////////////////////////////////////
float clNCIMasterGrowth::DeterministicAdjust(float fNumber, int iSpecies) {
  return fNumber;
}

//////////////////////////////////////////////////////////////////////////////
// NormalAdjust()
/////////////////////////////////////////////////////////////////////////////
float clNCIMasterGrowth::NormalAdjust(float fNumber, int iSpecies) {
  float fReturn = fNumber + clModelMath::NormalRandomDraw(mp_fRandParameter[iSpecies]);
  if (fReturn < 0) fReturn = 0;
  return fReturn;
}


//////////////////////////////////////////////////////////////////////////////
// LognormalAdjust()
/////////////////////////////////////////////////////////////////////////////
float clNCIMasterGrowth::LognormalAdjust(float fNumber, int iSpecies) {
  float fReturn = clModelMath::LognormalRandomDraw(fNumber, mp_fRandParameter[iSpecies]);
  if (fReturn < 0) fReturn = 0;
  return fReturn;
}
