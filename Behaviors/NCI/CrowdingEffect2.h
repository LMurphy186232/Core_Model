#ifndef CROWDINGEFFECT2_H_
#define CROWDINGEFFECT2_H_

#include "CrowdingEffectBase.h"

/**
 * This class calculates a crowding effect according to the term:
 * @htmlonly
  <center>CE = exp(-C * ( DBH<sup>&gamma;</sup> * NCI)<sup>D</sup>)</center>
  @endhtmlonly
 * <br>
 * where:
 * <ul>
 * <li><i>CE</i> = crowding effect</li>
 * <li><i>C</i>, <i>D</i>, and <i>&gamma; </i> are parameters</li>
 * <li><i>DBH</i> is of the target tree, in meters</li>
 * <li><i>NCI</i> is this tree's NCI value</li>
 * </ul>
 *
 * This is originally from the BA NCI behavior.
 *
 * I'm calling this "Crowding effect 2" because it's just a slight variation
 * on the default and I didn't know what else to call it.
 */
class clCrowdingEffectTwo: public clCrowdingEffectBase {
public:

  /**
   * Constructor.
   */
  clCrowdingEffectTwo();

  /**
   * Destructor.
   */
  ~clCrowdingEffectTwo();

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

#endif /* CROWDINGEFFECT2_H_ */
