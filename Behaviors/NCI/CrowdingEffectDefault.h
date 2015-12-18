#ifndef DEFAULTCROWDINGEFFECT_H_
#define DEFAULTCROWDINGEFFECT_H_

#include "CrowdingEffectBase.h"

/**
 * This class calculates a crowding effect using the default function.
 *
 * Crowding effect is calculated as follows:
 * @htmlonly
  <center><i>CE = exp(-C * diam <sup>&gamma;</sup> * nci<sup>D</sup>)</i></center>
  @endhtmlonly
 * <br>
 * where:
 * <ul>
 * <li><i>CE</i> = crowding effect</li>
 * <li><i>C</i> is the NCI slope parameter</li>
 * <li><i>D</i> is the NCI steepness parameter</li>
 * <li><i>diam</i> is diameter of the target tree, in cm (d10 for seedlings,
 * DBH for everyone else))</li>
 * <li>@htmlonly <i>&gamma;</i> @endhtmlonly is the size sensitivity to NCI parameter</li>
 * <li><i>nci2</i> is the NCI value</li>
 * </ul>
 *
 * Alternately, this can use NCI behaviors returning 2 values, in which case
 * the format is
 * @htmlonly
  <center><i>CE = exp(-C * nci1 <sup>&gamma;</sup> * nci2<sup>D</sup>)</i></center>
  @endhtmlonly
 */
class clCrowdingEffectDefault: public clCrowdingEffectBase {
public:

  /**
   * Constructor.
   */
  clCrowdingEffectDefault();

  /**
   * Destructor.
   */
  ~clCrowdingEffectDefault();

  /**
   * Calculates crowding effect.
   * @param p_oTree Tree for which to calculate crowding effect.
   * @param fDiam Diameter of tree.
   * @param nci NCI term.
   * @param iSpecies Species for which to calculate effect.
   */
  float CalculateCrowdingEffect(clTree *p_oTree, const float &fDiam, const clNCITermBase::ncivals nci, const int &iSpecies);

  /**
   * Does setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oNCIBase NCI parent behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, clNCIBehaviorBase *p_oNCIBase, xercesc::DOMElement *p_oElement);



protected:
  /**Crowding effect slope. C in Crowding Effect equation above. Array
   * sized number of species.*/
  float *mp_fC;

  /**Crowding effect steepness. D in Crowding Effect equation above. Array
   * sized number of species.*/
  float *mp_fD;

  /**Size sensitivity to NCI parameter. @htmlonly &gamma; @endhtmlonly in
   * Crowding Effect equation above.  Array sized number of species.*/
  float *mp_fGamma;
};

#endif /* DEFAULTCROWDINGEFFECT_H_ */
