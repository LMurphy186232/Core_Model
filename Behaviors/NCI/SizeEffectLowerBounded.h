#ifndef SIZEEFFECTLOWERBOUNDED_H_
#define SIZEEFFECTLOWERBOUNDED_H_

#include "SizeEffectBase.h"

/**
 * Calculates the size effect.
 * Size Effect is calculated as:
 * <center><i>SE = exp(-0.5(ln(diam/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</i></center>
 *
 * where:
 * <ul>
 * <li><i>diam</i> is target tree's diameter (d10 for seedlings, DBH for all others), in cm</li>
 * <li><i>X<sub>0</sub></i> is the size effect mode, in cm</li>
 * <li><i>X<sub>b</sub></i> is the size effect variance, in cm</li>
 * </ul>
 *
 * Size effect is subject to a minimum value for diameter, below which all
 * trees will just get the minimum.
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>June 13, 2013 - Created for transferal of Weibull Climate Growth to
 * the NCI framework (LEM)
 */
class clSizeEffectLowerBounded: virtual public clSizeEffectBase {
public:

  /**
   * Constructor.
   */
  clSizeEffectLowerBounded();

  /**
   * Destructor.
   */
  ~clSizeEffectLowerBounded();

  /**
   * Calculates size effect. This is bounded between 0 and 1.
   * @param p_oTree Tree for which to calculate size effect.
   * @param fDiam Diameter of tree. Diameter at 10 cm for seedlings, DBH for
   * all other types.
   */
  double CalculateSizeEffect(clTree *p_oTree, const float &fDiam);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if X0 is < 0 or Xb = 0.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:
  /**Size effect variance parameter. X<sub>b</sub> in Size Effect equation
   * above. Array is sized number of species.*/
  double *mp_fXb;

  /**Size effect mode parameter. X<sub>0</sub> in Size Effect equation above.
   * Array is sized number of species.*/
  double *mp_fX0;

  /**Size effect minimum diameter. Array is sized number of species.*/
  double *mp_fMinDiam;
};

#endif /* DEFAULTSIZEEFFECT_H_ */
