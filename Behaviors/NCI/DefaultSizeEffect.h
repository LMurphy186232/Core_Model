#ifndef DEFAULTSIZEEFFECT_H_
#define DEFAULTSIZEEFFECT_H_

#include "SizeEffectBase.h"

/**
 * Calculates the default size effect.
 * Size Effect is calculated as:
 * <center><i>SE = exp(-0.5(ln(DBH/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</i></center>
 *
 * where:
 * <ul>
 * <li><i>DBH</i> is for the target tree, in cm</li>
 * <li><i>X<sub>0</sub></i> is the size effect mode, in cm</li>
 * <li><i>X<sub>b</sub></i> is the size effect variance, in cm</li>
 * </ul>
 */
class clDefaultSizeEffect: virtual public clSizeEffectBase {
public:

  /**
   * Constructor.
   */
  clDefaultSizeEffect();

  /**
   * Destructor.
   */
  ~clDefaultSizeEffect();

  /**
   * Calculates size effect. This is bounded between 0 and 1.
   * @param iSpecies Species of tree.
   * @param fDiam Diameter of tree. Diameter at 10 cm for seedlings, DBH for
   * all other types.
   */
  float CalculateSizeEffect(int iSpecies, float fDiam);

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
};

#endif /* DEFAULTSIZEEFFECT_H_ */
