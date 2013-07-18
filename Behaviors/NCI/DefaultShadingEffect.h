/*
 * DefaultShadingEffect.h
 *
 *  Created on: Nov 27, 2012
 *      Author: LORA
 */

#ifndef DEFAULTSHADINGEFFECT_H_
#define DEFAULTSHADINGEFFECT_H_

#include "ShadingEffectBase.h"

/**
 * This implements the default shading effect.
 *
 * Shading effect is calculated as follows:
 * <center><i>SE = exp(-m * S<sup>n</sup>)</i></center>
 *
 * where:
 * <ul>
 * <li><i>SE</i> = Shading Effect</li>
 * <li><i>m</i> = shading coefficient</li>
 * <li><i>S</i> = light level</li>
 * <li><i>n</i> = shading exponent</li>
 * </ul>
 */
class clDefaultShadingEffect: virtual public clShadingEffectBase {
public:
  /**
   * Constructor.
   */
  clDefaultShadingEffect();

  /**
   * Destructor.
   */
  ~clDefaultShadingEffect();

  /**
   * Calculates shading effect.
   * @param p_oTree Tree for which to calculate shading effect.
   */
  float CalculateShadingEffect(clTree *p_oTree);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if any species does not have a light behavior
   * assigned (has data member "Light" registered).
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Holds return data codes for the "Light" tree data member. Array size is
   * number of species by number of types.*/
  short int **mp_iLightCodes;

  /**Shading coefficient in Shading Effect equation. Array is sized number of
   * species.*/
  float *mp_fShadingCoefficient;

  /**Shading exponent in Shading Effect equation. Array is sized number of
   * species.*/
  float *mp_fShadingExponent;

  /**
   * Number of total species. For the destructor.
   */
  int m_iNumberTotalSpecies;
};

#endif /* DEFAULTSHADINGEFFECT_H_ */
