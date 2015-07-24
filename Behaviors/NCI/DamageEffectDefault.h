#ifndef DEFAULTDAMAGEEFFECT_H_
#define DEFAULTDAMAGEEFFECT_H_

#include "DamageEffectBase.h"

/**
 * This returns the damage effect due to storms. Storm Effect is an input
 * parameter. There is one for trees with medium damage and one for trees with
 * full damage. Each is a value between 0 and 1. If the damage counter of the
 * target tree = 0 (tree is undamaged), Storm Effect equals 1.
 */
class clDamageEffectDefault: virtual public clDamageEffectBase {
public:

  /**
   * Constructor.
   */
  clDamageEffectDefault();

  /**
   * Destructor.
   */
  ~clDamageEffectDefault();

  /**
   * Calculates damage effect.
   * @param p_oTree Tree for which to calculate damage effect.
   */
  float CalculateDamageEffect(clTree *p_oTree);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if Storm Effect parameters are not between 0 and 1.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:
  /**Holds return data codes for the "stm_dmg" tree data member. Array size is
   * number of total species by number of types.
   */
  short int **mp_iDamageCodes;

  /**Damage Effect parameter for target trees with medium damage. Array is sized
   * number of species.*/
  float *mp_fMedDamageStormEff;

  /**Damage Effect parameter for target trees with full damage. Array is sized
   * number of species.*/
  float *mp_fFullDamageStormEff;

  /**
   * Number of total species. For the destructor.
   */
  int m_iNumberTotalSpecies;
};

#endif /* DEFAULTDAMAGEEFFECT_H_ */
