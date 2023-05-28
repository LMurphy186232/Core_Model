#ifndef SIZEEFFECTEXPONENTIALHEIGHT_H_
#define SIZEEFFECTEXPONENTIALHEIGHT_H_

#include "SizeEffectBase.h"
#include "Allometry.h"

/**
 * Calculates the size effect with an exponential function of height.
 * Size Effect is calculated as:
 * <center><i>SE = (1-a*exp(b*height))</i></center>
 *
 * where <i>height</i> is the diameter of the target tree, in cm, and <i>a</i>
 * and <i>b</i> are parameters. This will incrementally adjust height for
 * multi-year timesteps from the diameter.
 *
 * If DBH and height allometry are not coupled, this just won't work.
 *
 * Life stage history transitions can be weird with the multi-year timestep.
 * For instance, a sapling might grow to adult size in the middle of the
 * timestep and now do we use the sapling or adult height equation? I have
 * decided not to take transitions into account, because unexpected function
 * discontinuities could make weird things happen. The obvious solution if
 * this is a problem is to switch to 1 year timesteps.
 *
 * Size can be scaled to appropriate units with the scaler.
  */
class clSizeEffectExponentialHeight: virtual public clSizeEffectBase {
public:

  /**
   * Constructor.
   */
  clSizeEffectExponentialHeight();

  /**
   * Destructor.
   */
  ~clSizeEffectExponentialHeight();

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

  /**Size scaler. Can be used to change units of size from SORTIE defaults.*/
  double m_fScaler;

  /**Keep a pointer to the tree population object.*/
  clTreePopulation* mp_oPop;

  /**Keep a pointer to the allometry object for calculating height.*/
  clAllometry* mp_oAllom;
};

#endif /* SIZEEFFECTEXPONENTIALHEIGHT_H_ */
