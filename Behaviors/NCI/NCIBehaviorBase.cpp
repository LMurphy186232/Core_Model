//---------------------------------------------------------------------------
#include "NCIBehaviorBase.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "Allometry.h"

#include "NCI/CrowdingEffectDefault.h"
#include "NCI/CrowdingEffect2.h"
#include "NCI/CrowdingEffectNoSize.h"
#include "NCI/CrowdingEffectTempDep.h"
#include "NCI/CrowdingEffectNone.h"
#include "NCI/DamageEffectDefault.h"
#include "NCI/ShadingEffectDefault.h"
#include "NCI/ShadingEffectNone.h"
#include "NCI/ShadingEffectPower.h"
#include "NCI/NCITermBARatio.h"
#include "NCI/NCITermBARatioDBHDefault.h"
#include "NCI/NCITermDefault.h"
#include "NCI/NCITermNone.h"
#include "NCI/NCITermWithNeighborDamage.h"
#include "NCI/NCILargerNeighbors.h"
#include "NCI/NCINeighborBA.h"
#include "NCI/NCIWithSeedlings.h"
#include "NCI/NCITermNCIBARatio.h"
#include "NCI/NCITermNCITempDepBARatio.h"
#include "NCI/DamageEffectNone.h"
#include "NCI/TemperatureEffectNone.h"
#include "NCI/TemperatureEffectWeibull.h"
#include "NCI/TemperatureEffectDoubleLogistic.h"
#include "NCI/TemperatureEffectDoubleNoLocalDiff.h"
#include "NCI/PrecipitationEffectNone.h"
#include "NCI/PrecipitationEffectWeibull.h"
#include "NCI/PrecipitationEffectDoubleLogistic.h"
#include "NCI/PrecipitationEffectDoubleNoLocalDiff.h"
#include "NCI/SizeEffectNone.h"
#include "NCI/SizeEffectDefault.h"
#include "NCI/SizeEffectLowerBounded.h"
#include "NCI/SizeEffectPowerFunction.h"
#include "NCI/NitrogenEffectNone.h"
#include "NCI/NitrogenEffectGaussian.h"
#include "NCI/InfectionEffectNone.h"
#include "NCI/InfectionEffect.h"
#include "NCI/InfectionEffectSizeDependent.h"
#include "NCI/SizeEffectShiftedLognormal.h"
#include "NCI/SizeEffectCompoundExponential.h"
#include "NCI/SizeEffectShiftedLogInf.h"
#include "NCI/SizeEffectCompoundExpInf.h"
#include "NCI/SizeEffectExponentialHeight.h"

#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIBehaviorBase::clNCIBehaviorBase() { //clSimManager * p_oSimManager) : clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  mp_oCrowdingEffect = NULL;
  mp_oDamageEffect = NULL;
  mp_oNCITerm = NULL;
  mp_oShadingEffect = NULL;
  mp_oSizeEffect = NULL;
  mp_oPrecipEffect = NULL;
  mp_oTempEffect = NULL;
  mp_oNEffect = NULL;
  mp_oInfectionEffect = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clNCIBehaviorBase::~clNCIBehaviorBase() {

  delete mp_oCrowdingEffect;
  delete mp_oDamageEffect;
  delete mp_oNCITerm;
  delete mp_oShadingEffect;
  delete mp_oSizeEffect;
  delete mp_oPrecipEffect;
  delete mp_oTempEffect;
  delete mp_oNEffect;
  delete mp_oInfectionEffect;
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clNCIBehaviorBase::ReadParameterFile(DOMElement * p_oElement,
    clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, bool bUsingDiam) {
  int iVal;

  //Which shading term?
  if (bUsingDiam) {
    FillSingleValue(p_oElement, "nciWhichShadingEffect", &iVal, true);
    if (iVal == no_shading) {
      mp_oShadingEffect = new clShadingEffectNone();
    } else if (iVal == default_shading) {
      mp_oShadingEffect = new clShadingEffectDefault();
    } else if (iVal == power_shading) {
      mp_oShadingEffect = new clShadingEffectPower();
    } else {
      modelErr err;
      err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
      err.iErrorCode = BAD_DATA;
      err.sMoreInfo = "Unrecognized shading term.";
      throw(err);
    }
  } else {
    mp_oShadingEffect = new clShadingEffectNone();
  }

  //Which crowding term?
  FillSingleValue(p_oElement, "nciWhichCrowdingEffect", &iVal, true);
  if (iVal == no_crowding_effect) {
    mp_oCrowdingEffect = new clCrowdingEffectNone();
  } else if (iVal == default_crowding_effect) {
    mp_oCrowdingEffect = new clCrowdingEffectDefault();
  } else if (iVal == crowding_effect_two) {
    mp_oCrowdingEffect = new clCrowdingEffectTwo();
  } else if (iVal == crowding_effect_no_size) {
    mp_oCrowdingEffect = new clCrowdingEffectNoSize();
  } else if (iVal == crowding_effect_temp_dep) {
    mp_oCrowdingEffect = new clCrowdingEffectTempDep();
  } else {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized crowding term.";
    throw(err);
  }

  //Check for diameter requirements
  if (bUsingDiam == false && mp_oCrowdingEffect->DoesRequireTargetDiam()) {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Crowding term cannot be used because it requires a target diameter.";
    throw(err);
  }

  //Which NCI term?
  FillSingleValue(p_oElement, "nciWhichNCITerm", &iVal, true);
  if (iVal == no_nci_term) {
    mp_oNCITerm = new clNCITermNone();
  } else if (iVal == default_nci_term) {
    mp_oNCITerm = new clNCITermDefault();
  } else if (iVal == nci_with_neighbor_damage) {
    mp_oNCITerm = new clNCITermWithNeighborDamage();
  } else if (iVal == larger_neighbors) {
    mp_oNCITerm = new clNCILargerNeighbors();
  } else if (iVal == neighbor_ba) {
    mp_oNCITerm = new clNCINeighborBA();
  } else if (iVal == nci_with_seedlings) {
    mp_oNCITerm = new clNCIWithSeedlings();
  } else if (iVal == nci_ba_ratio) {
    mp_oNCITerm = new clNCITermBARatio();
  } else if (iVal == nci_ba_ratio_dbh_default) {
    mp_oNCITerm = new clNCITermBARatioDBHDefault();
  } else if (iVal == nci_nci_ba_ratio) {
    mp_oNCITerm = new clNCITermNCIBARatio(false);
  } else if (iVal == nci_nci_ba_ratio_ba_default) {
    mp_oNCITerm = new clNCITermNCIBARatio(true);
  } else if (iVal == nci_nci_temp_dep_ba_ratio) {
    mp_oNCITerm = new clNCITermNCITempDepBARatio(false);
  } else if (iVal == nci_nci_temp_dep_ba_ratio_ba_default) {
    mp_oNCITerm = new clNCITermNCITempDepBARatio(true);
  } else {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized NCI term.";
    throw(err);
  }

  //Check for diameter requirements
  if (bUsingDiam == false && mp_oNCITerm->DoesRequireTargetDiam()) {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "NCI term cannot be used because it requires a target diameter.";
    throw(err);
  }

  //Which size effect term?
  if (bUsingDiam) {
    FillSingleValue(p_oElement, "nciWhichSizeEffect", &iVal, true);
    if (iVal == no_size_effect) {
      mp_oSizeEffect = new clSizeEffectNone();
    } else if (iVal == default_size_effect) {
      mp_oSizeEffect = new clSizeEffectDefault();
    } else if (iVal == size_effect_bounded) {
      mp_oSizeEffect = new clSizeEffectLowerBounded();
    } else if (iVal == size_effect_power_function) {
      mp_oSizeEffect = new clSizeEffectPowerFunction();
    } else if (iVal == size_effect_shifted_lognormal) {
      mp_oSizeEffect = new clSizeEffectShiftedLognormal();
    } else if (iVal == size_effect_compound_exp) {
      mp_oSizeEffect = new clSizeEffectCompoundExponential();
    } else if (iVal == size_effect_shifted_log_inf) {
      mp_oSizeEffect = new clSizeEffectShiftedLogInf();
    } else if (iVal == size_effect_compound_exp_inf) {
      mp_oSizeEffect = new clSizeEffectCompoundExpInf();
    } else if (iVal == size_effect_exp_height) {
      mp_oSizeEffect = new clSizeEffectExponentialHeight();
    } else {
      modelErr err;
      err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
      err.iErrorCode = BAD_DATA;
      err.sMoreInfo = "Unrecognized size effect term.";
      throw(err);
    }
  } else {
    mp_oSizeEffect = new clSizeEffectNone();
  }

  //Which damage effect term?
  if (bUsingDiam) {
    FillSingleValue(p_oElement, "nciWhichDamageEffect", &iVal, true);
    if (iVal == no_damage_effect) {
      mp_oDamageEffect = new clDamageEffectNone();
    } else if (iVal == default_damage_effect) {
      mp_oDamageEffect = new clDamageEffectDefault();
    } else {
      modelErr err;
      err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
      err.iErrorCode = BAD_DATA;
      err.sMoreInfo = "Unrecognized damage effect term.";
      throw(err);
    }
  } else {
    mp_oDamageEffect = new clDamageEffectNone();
  }

  //Which precipitation effect term?
  FillSingleValue(p_oElement, "nciWhichPrecipitationEffect", &iVal, true);
  if (iVal == no_precip_effect) {
    mp_oPrecipEffect = new clPrecipitationEffectNone();
  } else if (iVal == weibull_precip_effect) {
    mp_oPrecipEffect = new clPrecipitationEffectWeibull();
  } else if (iVal == double_logistic_precip_effect) {
    mp_oPrecipEffect = new clPrecipitationEffectDoubleLogistic();
  } else if (iVal == double_no_local_diff_precip_effect) {
    mp_oPrecipEffect = new clPrecipitationEffectDoubleNoLocalDiff();
  }else {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized precipitation effect term.";
    throw(err);
  }

  //Check for diameter requirements
  if (bUsingDiam == false && mp_oPrecipEffect->DoesRequireTargetDiam()) {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Precipitation term cannot be used because it requires a target diameter.";
    throw(err);
  }

  //Which temperature effect term?
  FillSingleValue(p_oElement, "nciWhichTemperatureEffect", &iVal, true);
  if (iVal == no_temp_effect) {
    mp_oTempEffect = new clTemperatureEffectNone();
  } else if (iVal == weibull_temp_effect) {
    mp_oTempEffect = new clTemperatureEffectWeibull();
  } else if (iVal == double_logistic_temp_effect) {
    mp_oTempEffect = new clTemperatureEffectDoubleLogistic();
  } else if (iVal == double_no_local_diff_temp_effect) {
    mp_oTempEffect = new clTemperatureEffectDoubleNoLocalDiff();
  } else {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized temperature effect term.";
    throw(err);
  }

  //Check for diameter requirements
  if (bUsingDiam == false && mp_oTempEffect->DoesRequireTargetDiam()) {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Temperature term cannot be used because it requires a target diameter.";
    throw(err);
  }

  //Which nitrogen effect term?
  FillSingleValue(p_oElement, "nciWhichNitrogenEffect", &iVal, true);
  if (iVal == no_nitrogen_effect) {
    mp_oNEffect = new clNitrogenEffectNone();
  } else if (iVal == gauss_nitrogen_effect) {
    mp_oNEffect = new clNitrogenEffectGaussian();
  } else {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized nitrogen effect term.";
    throw(err);
  }

  //Check for diameter requirements
  if (bUsingDiam == false && mp_oNEffect->DoesRequireTargetDiam()) {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Nitrogen term cannot be used because it requires a target diameter.";
    throw(err);
  }

  //Which infection effect term?
  FillSingleValue(p_oElement, "nciWhichInfectionEffect", &iVal, true);
  if (iVal == no_infection_effect) {
    mp_oInfectionEffect = new clInfectionEffectNone();
  } else if (iVal == infection_effect) {
    mp_oInfectionEffect = new clInfectionEffect();
  } else if (iVal == infection_effect_size_dep) {
    mp_oInfectionEffect = new clInfectionEffectSizeDependent();
  } else {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized infection effect term.";
    throw(err);
  }

  //Check for diameter requirements
  if (bUsingDiam == false && mp_oInfectionEffect->DoesRequireTargetDiam()) {
    modelErr err;
    err.sFunction = "clNCIBehaviorBase::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Infection term cannot be used because it requires a target diameter.";
    throw(err);
  }

  mp_oCrowdingEffect->DoSetup(p_oPop, p_oNCI, this, p_oElement);
  mp_oDamageEffect->DoSetup(p_oPop, p_oNCI, p_oElement);
  mp_oNCITerm->DoSetup(p_oPop, p_oNCI, p_oElement);
  mp_oShadingEffect->DoSetup(p_oPop, p_oNCI, p_oElement);
  mp_oSizeEffect->DoSetup(p_oPop, p_oNCI, p_oElement);
  mp_oPrecipEffect->DoSetup(p_oPop, p_oNCI, p_oElement);
  mp_oTempEffect->DoSetup(p_oPop, p_oNCI, p_oElement);
  mp_oNEffect->DoSetup(p_oPop, p_oNCI, p_oElement);
  mp_oInfectionEffect->DoSetup(p_oPop, p_oNCI, p_oElement);
}
