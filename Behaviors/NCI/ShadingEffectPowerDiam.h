#ifndef SHADINGEFFECTPOWERDIAM_H_
#define SHADINGEFFECTPOWERDIAM_H_

#include "ShadingEffectBase.h"

/**
 * This implements a shading effect that is a power function of tree diameter
 * and GLI.
 *
 * Shading effect is calculated as follows:
 * <center><i>SE = (GLI-i)<sup>a</sup> * (1 - exp(-b * diam))
 *
 * where:
 * <ul>
 * <li><i>SE</i> = Shading Effect</li>
 * <li><i>i</i> = Power Diameter Light Compensation Point (x intercept of the
 * light limitation curve)</li>
 * <li><i>a</i> = Power Diameter Shade Shape (a) parameter</li>
 * <li><i>b</i> = Power Diameter Size Shape (b) parameter</li>
 * <li><i>GLI</i> = global light index, calculated by a light behavior</li>
 * <li><i>diam</i> = diameter (diameter at 10 cm for seedlings and saplings,
 * DBH for adults)</li>
 * </ul>
 *
 * It is important to scale GLI appropriately. It should be on a scale from 0
 * to 1. So if GLI is reported from 0 to 100, it needs to be divided by 100.
 */
class clShadingEffectPowerDiam: virtual public clShadingEffectBase {
public:
  /**
   * Constructor.
   */
  clShadingEffectPowerDiam();

  /**
   * Destructor.
   */
  ~clShadingEffectPowerDiam();

  /**
   * Calculates shading effect.
   * @param p_oTree Tree for which to calculate shading effect.
   */
  double CalculateShadingEffect(clTree *p_oTree);

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

  /**Holds return data codes for the tree diameter. Array size is
   * number of species by number of types.*/
  short int **mp_iDiamCodes;

  /**"a" in Shading Effect equation. Array is sized number of species.*/
  double *mp_fA;

  /**"b" in Shading Effect equation. Array is sized number of species.*/
  double *mp_fB;

  /**"i" in Shading Effect equation. Array is sized number of species.*/
  double *mp_fI;

  /**Max possible GLI - expected to be 100 (if GLI is reported as a %) or
   * 1 (if GLI is reported as a proportion). These two values are not enforced.
   */
  double m_fGLIScaler;

  /**
   * Number of total species. For the destructor.
   */
  int m_iNumberTotalSpecies;
};

#endif /* SHADINGEFFECTPOWERDIAM_H_ */
