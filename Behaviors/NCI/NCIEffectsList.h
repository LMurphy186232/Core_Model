#ifndef NCIEFFECTSLIST_H_
#define NCIEFFECTSLIST_H_

/**
 * Flag values for which shading effect term is desired.
 */
enum shading_effect {
  no_shading, /**<No shading (class clNoShadingEffect) */
  default_shading, /**<Default shading (class clDefaultShadingEffect) */
  power_shading /**<Power function of light (class clShadingEffectPower) */
};

/**
 * Flag values for which size effect term is desired.
 */
enum size_effect {
  no_size_effect, /**<No size effect (class clNoSizeEffect) */
  default_size_effect, /**<Default size effect (class clDefaultSizeEffect) */
  size_effect_bounded, /**<Size effect with minimum DBH (class clSizeEffectLowerBounded) */
  size_effect_power_function, /**<Power function size effect (class clSizeEffectPowerFunction) */
  size_effect_shifted_lognormal, /**<Shifted lognormal size effect (class clShiftedLognormalSizeEffect) */
  size_effect_compound_exp, /**<Compound exponential size effect (class clSizeEffectCompoundExponential) */
  size_effect_shifted_log_inf, /**<Shifted lognormal w/ infection size effect (class clSizeEffectShiftedLogInf) */
  size_effect_compound_exp_inf, /**<Compound exponential w/ infection size effect (class clSizeEffectCompoundExpInf) */
  size_effect_exp_height /**<Exponential function of height (class clSizeEffectExponentialHeight) */
};

/**
 * Flag values for which damage effect term is desired.
 */
enum damage_effect {
  no_damage_effect, /**<No damage effect (class clNoDamageEffect) */
  default_damage_effect /**<Default damage effect (class clDefaultDamageEffect)*/
};

/**
 * Flag values for which NCI term is desired.
 */
enum nci_term {
  no_nci_term, /**<No NCI term (class clNoNCITerm) */
  default_nci_term, /**<Default NCI term (class clDefaultNCITerm) */
  nci_with_neighbor_damage, /**<NCI term with neighbor damage (class clNCITermWithNeighborDamage)*/
  larger_neighbors, /**<Count of larger neighbors (class clNCILargerNeighbors)*/
  neighbor_ba, /**<Neighbor BA (class clNCINeighborBA)*/
  nci_with_seedlings, /**<NCI with seedling competition (class clNCIWithSeedlings)*/
  nci_ba_ratio, /**<NCI with ba ratio (class clNCITermBARatio)*/
  nci_ba_ratio_dbh_default, /**<NCI with ba ratio (class clNCITermBARatioDBHDefault)*/
  nci_nci_ba_ratio, /**<NCI with temp dependent lambda (class clNCITermNCIBARatio) */
  nci_nci_ba_ratio_ba_default, /**<NCI with temp dependent lambda (class clNCITermNCIBARatio) */
  nci_nci_temp_dep_ba_ratio, /**<NCI with temp dependent lambda (class clNCITermNCITempDepBARatio) */
  nci_nci_temp_dep_ba_ratio_ba_default /**<NCI with temp dependent lambda (class clNCITermNCITempDepBARatio) */
};

/**
 * Flag values for which crowding effect term is desired.
 */
enum crowding_effect {
  no_crowding_effect, /**<No crowding effect (class clNoCrowdingEffect) */
  default_crowding_effect, /**<Default crowding effect (class clDefaultCrowdingEffect) */
  crowding_effect_two, /**<Crowding effect 2 (class clCrowdingEffectTwo)*/
  crowding_effect_no_size, /**<Crowding effect with no DBH term (class clCrowdingEffectNoSize) */
  crowding_effect_temp_dep /**<Crowding effect with temperature dependence (class clCrowdingEffectTempDep) */
};

/**
 * Flag values for which temperature effect term is desired.
 */
enum temperature_effect {
  no_temp_effect, /**<No temperature effect (class clNoTemperatureEffect) */
  weibull_temp_effect, /**<Weibull temperature effect (class clTemperatureEffectWeibull)*/
  double_logistic_temp_effect, /**<Double logistic temperature effect (class clTemperatureEffectDoubleLogistic)*/
  double_no_local_diff_temp_effect, /**<Class clTemperatureEffectDoubleNoLocalDiff*/
  double_local_diff_temp_effect /**<Class clTemperatureEffectDoubleLocalDiff*/
};

/**
 * Flag values for which precipitation effect term is desired.
 */
enum precipitation_effect {
  no_precip_effect, /**<No precip effect (class clNoPrecipitationEffect) */
  weibull_precip_effect, /**<Weibull precipitation effect (class clPrecipitationEffectWeibull)*/
  double_logistic_precip_effect, /**<Double logistic precipitation effect (class clPrecipitationEffectDoubleLogistic)*/
  double_no_local_diff_precip_effect, /**<Class clPrecipitationEffectDoubleNoLocalDiff*/
  double_local_diff_precip_effect /**<Class clPrecipitationEffectDoubleLocalDiff*/
};

/**
 * Flag values for which nitrogen effect term is desired.
 */
enum nitrogen_effect {
  no_nitrogen_effect, /**<No N effect (class clNoNitrogenEffect) */
  gauss_nitrogen_effect /**<Gaussian nitrogen effect (class clGaussianNitrogenEffect)*/
};

/**
 * Flag values for which infection effect term is desired.
 */
enum infection_effect {
  no_infection_effect, /**<No effect (class clNoInfectionEffect) */
  infection_effect, /**<Infection effect non-size dependent (class clInfectionEffect)*/
  infection_effect_size_dep, /**<Infection effect size dependent (class clInfectionEffectSizeDependent)*/
};

#endif /* NCIEFFECTSLIST_H_ */
