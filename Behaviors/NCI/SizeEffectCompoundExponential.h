#ifndef SIZEEFFECTCOMPOUNDEXPONENTIAL_H_
#define SIZEEFFECTCOMPOUNDEXPONENTIAL_H_

#include "SizeEffectBase.h"

/**
 * Calculates the size effect with a compound exponential function.
 * Size Effect is calculated as:
 * <center><i>SE = (1-a*exp(b*diam))*exp(c*(diam<sup>d</sup>))</i></center>
 *
 * where <i>diam</i> is the diameter of the target tree, in cm (d10 for
 * seedlings, DBH for everyone else); and <i>a, b, c,</i> and <i>d</i> are
 * parameters.
 *
 * Size can be scaled to appropriate units with the scaler.
  */
class clSizeEffectCompoundExponential: virtual public clSizeEffectBase {
public:

  /**
   * Constructor.
   */
  clSizeEffectCompoundExponential();

  /**
   * Destructor.
   */
  ~clSizeEffectCompoundExponential();

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
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:
  /**a in Size Effect equation above. Array is sized number of species.*/
  double *mp_fA;

  /**b in Size Effect equation above. Array is sized number of species.*/
  double *mp_fB;

  /**c in Size Effect equation above. Array is sized number of species.*/
  double *mp_fC;

  /**d in Size Effect equation above. Array is sized number of species.*/
  double *mp_fD;

  /**Size scaler. Can be used to change units of size from SORTIE defaults.
   * For backwards compatibility, this will be set to 0.01 to transform DBH
   * units to meters by default.
   */
  double m_fScaler;
};

#endif /* DEFAULTSIZEEFFECT_H_ */
