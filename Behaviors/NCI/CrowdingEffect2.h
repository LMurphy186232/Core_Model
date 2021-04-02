#ifndef CROWDINGEFFECT2_H_
#define CROWDINGEFFECT2_H_

#include "CrowdingEffectBase.h"

/**
 * Calculates a crowding effect which is a slight variation on the one in
 * clDefaultCrowdingEffect.
 *
 * This class calculates a crowding effect according to the term:
 * @htmlonly
  <center>CE = exp(-C * ( diam<sup>&gamma;</sup> * NCI)<sup>D</sup>)</center>
  @endhtmlonly
 * <br>
 * where:
 * <ul>
 * <li><i>CE</i> = crowding effect</li>
 * <li><i>C</i>, <i>D</i>, and <i>&gamma; </i> are parameters</li>
 * <li><i>diam</i> is diameter of the target tree, in cm (d10 for seedlings, DBH for everyone else)</li>
 * <li><i>NCI</i> is this tree's NCI value</li>
 * </ul>
 *
 * This is originally from the BA NCI behavior.
 *
 * I'm calling this "Crowding effect 2" because it's just a slight variation
 * on the default and I didn't know what else to call it.
 *
 * Alternately, this can use NCI behaviors returning 2 values, in which case
 * the format is
 * @htmlonly
  <center><i>CE = exp(-C * (nci1 <sup>&gamma;</sup> * nci2)<sup>D</sup>)</i></center>
  @endhtmlonly
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
   * @param nci NCI term.
   * @param iSpecies Species for which to calculate effect.
   */
  double CalculateCrowdingEffect(clTree *p_oTree, const float &fDiam, const clNCITermBase::ncivals nci, const int &iSpecies);

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
  double *mp_fC;

  /**Crowding effect steepness. D in Crowding Effect equation above. Array
   * sized number of species.*/
  double *mp_fD;

  /**Size sensitivity to NCI parameter. @htmlonly &gamma; @endhtmlonly in
   * Crowding Effect equation above.  Array sized number of species.*/
  double *mp_fGamma;
};

#endif /* CROWDINGEFFECT2_H_ */
