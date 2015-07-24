#ifndef CROWDINGEFFECTTEMPDEP_H_
#define CROWDINGEFFECTTEMPDEP_H_

#include "CrowdingEffectBase.h"

/**
 * This class calculates a crowding effect using the function:
 *
 * @htmlonly
  <center><i>CE = exp(-CT * diam <sup>&gamma;</sup> * nci<sup>D</sup>)</i></center>
  @endhtmlonly
 * <br>
 * where:
 * <ul>
 * <li><i>CE</i> = crowding effect</li>
 * <li><i>D</i> is the NCI steepness parameter</li>
 * <li><i>diam</i> is diameter of the target tree, in cm (d10 for seedlings,
 * DBH for everyone else))</li>
 * <li>@htmlonly <i>&gamma;</i> @endhtmlonly is the size sensitivity to NCI parameter</li>
 * <li><i>nci</i> is the NCI value</li>
 * </ul>
 *
 @htmlonly
 <center><i>CT = C * (1-exp(-0.5 * ((T-X0)/Xb)^2))</i></center>
 @endhtmlonly
 <br>
 * where:
 * <ul>
 * <li><i>C</i> is a parameter</li>
 * <li><i>X0</i> is a parameter</li>
 * <li><i>Xb</i> is a parameter</li>
 * <li><i>T</i> is temperature in K</li>
 * </ul
 *
 * Alternately, this can use NCI behaviors returning 2 values, in which case
 * the format is
 * @htmlonly
  <center><i>CE = exp(-CT * nci1 <sup>&gamma;</sup> * nci2<sup>D</sup>)</i></center>
  @endhtmlonly
 */
class clCrowdingEffectTempDep: public clCrowdingEffectBase {
public:

  /**
   * Constructor.
   */
  clCrowdingEffectTempDep();

  /**
   * Destructor.
   */
  ~clCrowdingEffectTempDep();

  /**
   * Calculates crowding effect.
   * @param p_oTree Tree for which to calculate crowding effect.
   * @param fDiam Diameter of tree.
   * @param fNCI NCI term.
   * @param iSpecies Species for which to calculate effect.
   */
  float CalculateCrowdingEffect(clTree *p_oTree, const float &fDiam, const clNCITermBase::ncivals nci, const int &iSpecies);

  /**
   * Does setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, clNCIBehaviorBase *p_oNCIBase, xercesc::DOMElement *p_oElement);



protected:
  /**Pointer to the plot object, so we can get the temperature*/
  clPlot *mp_oPlot;

  /**Crowding effect slope. C in Crowding Effect equation above. Array
   * sized number of species.*/
  float *mp_fC;

  /**Crowding effect steepness. D in Crowding Effect equation above. Array
   * sized number of species.*/
  float *mp_fD;

  /**X0 in Crowding Effect equation above. Array sized number of species.*/
  float *mp_fX0;

  /**Xb in Crowding Effect equation above. Array sized number of species.*/
  float *mp_fXb;

  /**Size sensitivity to NCI parameter. @htmlonly &gamma; @endhtmlonly in
   * Crowding Effect equation above.  Array sized number of species.*/
  float *mp_fGamma;
};

#endif /* CROWDINGEFFECTTEMPDEP_H_ */
