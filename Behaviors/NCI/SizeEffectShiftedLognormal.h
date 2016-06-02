#ifndef SIZEEFFECTSHIFTEDLOGNORMAL_H_
#define SIZEEFFECTSHIFTEDLOGNORMAL_H_

#include "SizeEffectBase.h"

/**
 * Calculates a size effect based on a shifted lognormal function.
 * Size Effect is calculated as:
 * <center><i>SE = exp(-0.5(ln((diam+X<sub>p</sub>)/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</i></center>
 *
 * where:
 * <ul>
 * <li><i>diam</i> is the diameter of the target tree, in cm (d10 for seedlings, DBH for everyone else)</li>
 * <li><i>X<sub>0</sub></i> is the size effect mode, in cm</li>
 * <li><i>X<sub>b</sub></i> is the size effect variance, in cm</li>
 * <li><i>X<sub>p</sub></i> is the size effect shift, in cm</li>
 * </ul>
 */
class clSizeEffectShiftedLognormal: virtual public clSizeEffectBase {
public:

  /**
   * Constructor.
   */
  clSizeEffectShiftedLognormal();

  /**
   * Destructor.
   */
  ~clSizeEffectShiftedLognormal();

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

  /**Size effect shift parameter. X<sub>p</sub> in Size Effect equation above.
   * Array is sized number of species.*/
  double *mp_fXp;

  /**Size effect minimum diameter. Array is sized number of species.*/
  double *mp_fMinDiam;
};

#endif /* SIZEEFFECTSHIFTEDLOGNORMAL_H_ */
