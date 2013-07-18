#ifndef NCIEFFECTSLIST_H_
#define NCIEFFECTSLIST_H_

/**
 * Flag values for which shading effect term is desired.
 */
enum shading_effect {
  no_shading, /**<No shading (class clNoShadingEffect) */
  default_shading /**<Default shading (class clDefaultShadingEffect) */
};

/**
 * Flag values for which size effect term is desired.
 */
enum size_effect {
  no_size_effect, /**<No size effect (class clNoSizeEffect) */
  default_size_effect, /**<Default size effect (class clDefaultSizeEffect) */
  size_effect_bounded, /**<Size effect with minimum DBH (class clSizeEffectLowerBounded) */
  size_effect_power_function /**<Power function size effect (class clSizeEffectPowerFunction) */
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
  nci_with_seedlings /**<NCI with seedling competition (class clNCIWithSeedlings)*/
};

/**
 * Flag values for which crowding effect term is desired.
 */
enum crowding_effect {
  no_crowding_effect, /**<No crowding effect (class clNoCrowdingEffect) */
  default_crowding_effect, /**<Default crowding effect (class clDefaultCrowdingEffect) */
  crowding_effect_two, /**<Crowding effect 2 (class clCrowdingEffectTwo)*/
  crowding_effect_no_size /**<Crowding effect with no DBH term (class clCrowdingEffectNoSize) */
};

/**
 * Flag values for which temperature effect term is desired.
 */
enum temperature_effect {
  no_temp_effect, /**<No temperature effect (class clNoTemperatureEffect) */
  weibull_temp_effect /**<Weibull temperature effect (class clWeibullTemperatureEffect)*/
};

/**
 * Flag values for which precipitation effect term is desired.
 */
enum precipitation_effect {
  no_precip_effect, /**<No precip effect (class clNoPrecipitationEffect) */
  weibull_precip_effect /**<Weibull precipitation effect (class clWeibullPrecipitationEffect)*/
};

/**
 * Flag values for which nitrogen effect term is desired.
 */
enum nitrogen_effect {
  no_nitrogen_effect, /**<No N effect (class clNoNitrogenEffect) */
  gauss_nitrogen_effect /**<Gaussian nitrogen effect (class clGaussianNitrogenEffect)*/
};


#endif /* NCIEFFECTSLIST_H_ */
