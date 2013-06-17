#ifndef DEFAULTCROWDINGEFFECT_H_
#define DEFAULTCROWDINGEFFECT_H_

#include "CrowdingEffectBase.h"

/**
 * This class calculates a crowding effect according to the term:
 * @htmlonly
  <center><i>CE = exp(-C * DBH <sup>&gamma;</sup> * NCI<sup>D</sup>)</i></center>
  @endhtmlonly
 * <br>
 * where:
 * <ul>
 * <li><i>CE</i> = crowding effect</li>
 * <li><i>C</i> is the NCI slope parameter</li>
 * <li><i>D</i> is the NCI steepness parameter</li>
 * <li><i>DBH</i> is of the target tree, in meters</li>
 * <li>@htmlonly <i>&gamma;</i> @endhtmlonly is the size sensitivity to NCI parameter</li>
 * <li><i>NCI</i> is this tree's NCI value</li>
 * </ul>
 */
class clDefaultCrowdingEffect: public clCrowdingEffectBase {
public:

  /**
   * Constructor.
   */
  clDefaultCrowdingEffect();

  /**
   * Destructor.
   */
  ~clDefaultCrowdingEffect();

  /**
   * Calculates crowding effect.
   * @param p_oTree Tree for which to calculate crowding effect.
   * @param fDiam Diameter of tree.
   * @param fNCI NCI term.
   */
  float CalculateCrowdingEffect(clTree *p_oTree, float fDiam, float fNCI);

  /**
   * Does setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);



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
