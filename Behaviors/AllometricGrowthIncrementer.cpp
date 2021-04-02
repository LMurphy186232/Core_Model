//---------------------------------------------------------------------------
// AllometricGrowthIncrementer.cpp
//---------------------------------------------------------------------------
#include "AllometricGrowthIncrementer.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "GrowthOrg.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clAllometricGrowthIncrementer::clAllometricGrowthIncrementer(
    clSimManager * p_oSimManager) : clWorkerBase(p_oSimManager),
    clBehaviorBase(p_oSimManager), clGrowthBase(p_oSimManager) {

  try
  {
    m_sNameString = "allometric incrementer growthshell";
    m_bGoLast = true;

    mp_oAllom = NULL;
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
    stcErr.sFunction = "clAllometricGrowthIncrementer::clAllometricGrowthIncrementer" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup
//////////////////////////////////////////////////////////////////////////////
void clAllometricGrowthIncrementer::DoShellSetup(DOMDocument * p_oDoc) {
  clTreePopulation * p_oPop =
      ( clTreePopulation * ) mp_oSimManager->GetPopulationObject("treepopulation");

  //Get the allometry object
  mp_oAllom = p_oPop->GetAllometryObject();
}

//////////////////////////////////////////////////////////////////////////////
// CalcHeightGrowthValue
//////////////////////////////////////////////////////////////////////////////
float clAllometricGrowthIncrementer::CalcHeightGrowthValue(clTree * p_oTree,
    clTreePopulation * p_oPop, float fDiameterGrowth) {
  float fBeginningDiam, //diameter before growth is applied
  fEndDiam, //diameter after growth is applied
  fBeginningHeight, //height for beginning diameter
  fEndHeight, //height for end diameter
  fHeightInc; //height increment to return
  int iTp = p_oTree->GetType(), iSp = p_oTree->GetSpecies();

  //Get the beginning and ending diameters
  p_oTree->GetValue(mp_oGrowthOrg->GetDiamCode(iSp, iTp), &fBeginningDiam);
  fEndDiam = fBeginningDiam + fDiameterGrowth;

  //Get the beginning and ending height according to tree type
  if (clTreePopulation::seedling == iTp) {
    fBeginningHeight = mp_oAllom->CalcSeedlingHeight(fBeginningDiam, iSp);
    fEndHeight = mp_oAllom->CalcSeedlingHeight(fEndDiam, iSp);
  }
  else if (clTreePopulation::sapling == iTp) {
    //Transform these diameters to DBH
    fBeginningDiam = mp_oAllom->ConvertDiam10ToDbh(fBeginningDiam, iSp);
    fEndDiam = mp_oAllom->ConvertDiam10ToDbh(fEndDiam, iSp);
    fBeginningHeight = mp_oAllom->CalcSaplingHeight(fBeginningDiam, iSp);
    fEndHeight = mp_oAllom->CalcSaplingHeight(fEndDiam, iSp);
  } else {
    fBeginningHeight = mp_oAllom->CalcAdultHeight(fBeginningDiam, iSp);
    fEndHeight = mp_oAllom->CalcAdultHeight(fEndDiam, iSp);
  }

  fHeightInc = fEndHeight - fBeginningHeight;
  return fHeightInc;
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue
//////////////////////////////////////////////////////////////////////////////
float clAllometricGrowthIncrementer::CalcDiameterGrowthValue(clTree *p_oTree,
    clTreePopulation *p_oPop, float fHeightGrowth) {
  float fBeginningHeight, //height before growth is applied
  fEndHeight, //height after growth is applied
  fBeginningDiam, //diameter for beginning height
  fEndDiam, //diameter for end height
  fDiamInc; //diameter increment to return
  int iTp = p_oTree->GetType(), iSp = p_oTree->GetSpecies();

  //Get the beginning and ending heights
  p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fBeginningHeight);
  fEndHeight = fBeginningHeight + fHeightGrowth;

  //Get the beginning and ending diameter according to tree type
  if (clTreePopulation::seedling == iTp) {
    fBeginningDiam = mp_oAllom->CalcSeedlingDiam10(fBeginningHeight, iSp);
    fEndDiam = mp_oAllom->CalcSeedlingDiam10(fEndHeight, iSp);
    //Don't let it go below the minimum
    if (fEndDiam < p_oPop->GetNewSeedlingDiam10())
      fEndDiam = p_oPop->GetNewSeedlingDiam10();
  }
  else if (clTreePopulation::sapling == iTp) {
    fBeginningDiam = mp_oAllom->CalcSaplingDbh(fBeginningHeight, iSp);
    fEndDiam = mp_oAllom->CalcSaplingDbh(fEndHeight, iSp);
    //Transform these DBHs to diam10s
    fBeginningDiam = mp_oAllom->ConvertDbhToDiam10(fBeginningDiam, iSp);
    fEndDiam = mp_oAllom->ConvertDbhToDiam10(fEndDiam, iSp);
  } else {
    fBeginningDiam = mp_oAllom->CalcAdultDbh(fBeginningHeight, iSp);
    fEndDiam = mp_oAllom->CalcAdultDbh(fEndHeight, iSp);
  }

  fDiamInc = fEndDiam - fBeginningDiam;

  return fDiamInc;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clAllometricGrowthIncrementer::SetNameData(std::string sNameString) {

  //Check the string passed and set the flags accordingly
  if (sNameString.compare("HeightIncrementer") == 0) {
    m_iGrowthMethod = height_only;
  } else if (sNameString.compare("DiameterIncrementer") == 0) {
    m_iGrowthMethod = diameter_only;
  }
  else {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Unrecognized behavior name \"" << sNameString << "\".";
    stcErr.sFunction = "clAllometricGrowthIncrementer::SetNameData";
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
}
