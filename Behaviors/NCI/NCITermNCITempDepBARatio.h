#ifndef NCITERMNCITEMPDEPBARATIO_H_
#define NCITERMNCITEMPDEPBARATIO_H_

#include "NCITermNCIBARatio.h"

/**
 * This calculates NCI for a tree according to the function:
 *
 * @htmlonly
  <center><i>NCI<sub>i</sub> = &Sigma; &lambda;<sub>k</sub>((DBH<sub>k</sub>*q)<sup>&alpha;</sup>/distance<sup>&beta;</sup>)</i></center>

  where:
  <ul>
  <li>we're summing over k = 0 to N neighbors greater than the minimum
  neighbor DBH</li>
  <li><i>&alpha;</i> is the neighbor DBH effect parameter</li>
  <li><i>&beta;</i> is the neighbor distance effect parameter</li>
  <li><i>DBH<sub>k</sub></i> is the DBH of the kth neighbor, in cm</li>
  <li><i>q</i> is the DBH units adjustment factor</li>
  <li><i>&gamma;</i> is the size sensitivity to NCI parameter</li>
  <li><i>&lambda;<sub>k</sub></i> is the NCI lambda parameter for the species
  of the kth neighbor</li>
  <li><i>distance</i> is distance from target to neighbor, in meters</li>
  </ul>
  <br>
  @endhtmlonly
 *
 @htmlonly
 <center>&lambda; = Xa * exp(-0.5*((T-X0)/Xb)^2)</center>
 @endhtmlonly
 *
 * where:
 * <ul>
 * <li><i>Xa</i> is a parameter</li>
 * <li><i>X0</i> is a parameter</li>
 * <li><i>Xb</i> is a parameter</li>
 * <li><i>T</i> is the temperature in K</li>
 * </ul>
 *
 * NCI ignores neighbors with disturbance and harvest death codes. Natural deaths
 * are NOT ignored, because it presumes that those deaths occurred in the current
 * timestep and they should still be considered as live neighbors.
 *
 * This returns two terms: term #1 is the ratio of mean basal area of neighbors
 * to target basal area; term #2 is the NCI.
 *
 * This behavior uses two different distances to look for neighbors. One is for
 * saplings, and one is for adults.
 */
class clNCITermNCITempDepBARatio: virtual public clNCITermNCIBARatio {
public:

  /**
   * Constructor.
   */
  clNCITermNCITempDepBARatio(bool bUseDefaultBA);

  /**
   * Destructor.
   */
  ~clNCITermNCITempDepBARatio();

  /**
  * Calculates current lambdas.
  *
  * @param p_oPop Tree population object.
  */
  void PreCalcs( clTreePopulation *p_oPop );

  /**
   * Does setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if the max radius of neighbor effects is < 0, or
   * if DBH divisor is <= 0.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:


  /**Xa for lamba. Array sized number of total species by number of total
   * species.*/
  double **mp_fXa;

  /**X0 for lamba. Array sized number of total species by number of total
   * species.*/
  double **mp_fX0;

  /**Xb for lamba. Array sized number of total species by number of total
   * species.*/
  double **mp_fXb;
};

#endif /* NCITERMNCITEMPDEPBARATIO_H_ */
