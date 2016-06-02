#ifndef NCIWITHSEEDLINGS_H_
#define NCIWITHSEEDLINGS_H_

#include "NCITermBase.h"

/**
 * Calculates NCI and allows seedlings to compete.
 *
 * NCI<sub>i</sub> is calculated as follows (simplifying the notation):
 * @htmlonly
  <center><i>NCI<sub>i</sub> = &Sigma; &lambda;<sub>k</sub>((d10<sub>k</sub>/q)<sup>&alpha;</sup>/distance<sup>&beta;</sup>)</i></center>

  where:
  <ul>
  <li>we're summing over k = 0 to N neighbors greater than the minimum
  neighbor DBH</li>
  <li><i>&alpha;</i> is the neighbor DBH effect parameter</li>
  <li><i>&beta;</i> is the neighbor distance effect parameter</li>
  <li><i>q</i> is the DBH divisor parameter</li>
  <li><i>d10<sub>k</sub></i> is the diameter of the kth neighbor, in cm</li>
  <li><i>&gamma;</i> is the size sensitivity to NCI parameter</li>
  <li><i>&lambda;<sub>k</sub></i> is the NCI lambda parameter for the species
  of the kth neighbor</li>
  <li><i>distance</i> is distance from target to neighbor, in meters</li>
  </ul>
  <br>
  @endhtmlonly
 * Seedlings are allowed to compete as neighbors.
 *
 * NCI ignores neighbors with disturbance and harvest death codes. Natural deaths
 * are NOT ignored, because it presumes that those deaths occurred in the current
 * timestep and they should still be considered as live neighbors.
 *
 * This returns 1 value for NCI.
 */
class clNCIWithSeedlings: virtual public clNCITermBase {
public:

  /**
   * Constructor.
   */
  clNCIWithSeedlings();

  /**
   * Destructor.
   */
  ~clNCIWithSeedlings();

  /**
   * Calculates NCI according to above equation.
   * @param p_oTree Tree for which to calculate NCI.
   * @param p_oPop Tree population.
   * @param p_oPlot Plot object.
   * @param fX X coordinate for which to calculate NCI.
   * @param fY Y coordinate for which to calculate NCI.
   * @param iSpecies Species for which to calculate NCI.
   */
  ncivals CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies);

  /**
   * Does setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if the max radius of neighbor effects is < 0, or
   * if DBH divisor is = 0.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Lamba for NCI. Array sized number of total species by number of total
   * species.*/
  double **mp_fLambda;

  /**Maximum search radius, in meters, in which to look for crowding
   * neighbors. Array sized number of species.
   */
  double *mp_fMaxCrowdingRadius;

  /**Neighbor DBH effect. @htmlonly &alpha; @endhtmlonly variable in
   * equation above. Array sized number of species.*/
  double *mp_fAlpha;

  /**Neighbor distance effect. @htmlonly &beta; @endhtmlonly variable in
   * equation above. Array sized number of species.*/
  double *mp_fBeta;

  /**The minimum Diam10, in cm, of neighbors to be included in NCI calculations.
  * Array is sized total number of species.*/
  double *mp_fMinimumNeighborDiam10;

  /**The value to divide diam10 by in NCI. <i>q</i> in the NCI equation above.
  * May be set to 1.*/
  double m_fDiam10Divisor;

  /**Whether or not to include snags in NCI*/
  bool m_bIncludeSnags;

  /**Number of total species. For the destructor.*/
  int m_iNumTotalSpecies;
};

#endif /* NCIWITHSEEDLINGS_H_ */
