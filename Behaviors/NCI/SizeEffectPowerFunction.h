#ifndef SIZEEFFECTPOWERFUNCTION_H_
#define SIZEEFFECTPOWERFUNCTION_H_

#include "SizeEffectBase.h"

/**
 * Calculates the size effect as a power function.
 * Size Effect is calculated as:
 * @htmlonly
   <center><i>Size Effect = a * diam<sup>b</sup></i></center>
   @endhtmlonly
 *
 * where:
 * <ul>
 * <li><i>diam</i> is the diameter of the target tree, in cm (d10 for seedlings, DBH for everyone else)</li>
 * <li><i>a</i> and <i>b</i> are parameters</li>
 * </ul>
 */
class clSizeEffectPowerFunction: virtual public clSizeEffectBase {
public:

  /**
   * Constructor.
   */
  clSizeEffectPowerFunction();

  /**
   * Destructor.
   */
  ~clSizeEffectPowerFunction();

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
  /**Size effect "a" parameter. Array is sized number of species.*/
  double *mp_fA;

  /**Size effect "b" parameter. Array is sized number of species.*/
  double *mp_fB;
};

#endif /* SIZEEFFECTPOWERFUNCTION_H_ */
