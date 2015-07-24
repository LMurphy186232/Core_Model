#ifndef NCITERMWITHNEIGHBORDAMAGE_H_
#define NCITERMWITHNEIGHBORDAMAGE_H_

#include "NCITermBase.h"

/**
 * Calculates NCI according to a function that takes into account neighbors
 * damaged by storms (class clStormDamageApplier).
 *
 * NCI<sub>i</sub> is calculated as follows (simplifying the notation):
 * @htmlonly
  <center><i>NCI<sub>i</sub> = &Sigma; &eta; &lambda;<sub>k</sub>((DBH<sub>k</sub>/q)<sup>&alpha;</sup>/distance<sup>&beta;</sup>)</i></center>

  where:
  <ul>
  <li>we're summing over k = 0 to N neighbors greater than the minimum
  neighbor DBH</li>
  <li><i>&eta;</i> is the storm damage parameter of the target,
  depending on the damage status.  It's 1 if the neighbor is undamaged.</li>
  <li><i>&alpha;</i> is the neighbor DBH effect parameter</li>
  <li><i>&beta;</i> is the neighbor distance effect parameter</li>
  <li><i>q</i> is the DBH divisor parameter</li>
  <li><i>DBH<sub>k</sub></i> is the DBH of the kth neighbor, in cm</li>
  <li><i>&gamma;</i> is the size sensitivity to NCI parameter</li>
  <li><i>&lambda;<sub>k</sub></i> is the NCI lambda parameter for the species
  of the kth neighbor</li>
  <li><i>distance</i> is distance from target to neighbor, in meters</li>
  </ul>
  <br>
  @endhtmlonly
 * Note that eta is per target species, not per neighbor (like lambda).
 *
 * NCI ignores neighbors with disturbance and harvest death codes. Natural
 * deaths are NOT ignored, because it presumes that those deaths occurred in the
 * current timestep and they should still be considered as live neighbors.
 *
 * Seedlings do not compete.
 *
 * This returns 1 value for NCI.
 */
class clNCITermWithNeighborDamage: virtual public clNCITermBase {
public:

  /**
   * Constructor.
   */
  clNCITermWithNeighborDamage();

  /**
   * Destructor.
   */
  ~clNCITermWithNeighborDamage();

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
   * @throws ModelException if:
   * <ul>
   * <li>the max radius of neighbor effects is < 0</li>
   * <li>eta for either damage category is not between 0 and 1</li>
   * <li>DBH divisor is <= 0</li>
   * </ul>
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Lamba for NCI. Array sized number of total species by number of total
   * species.*/
  float **mp_fLambda;

  /**Holds return data codes for the "stm_dmg" tree data member. Array size is
   * number of total species by number of types.
   */
  short int **mp_iDamageCodes;

  /**Maximum search radius, in meters, in which to look for crowding
   * neighbors. Array sized number of species.
   */
  float *mp_fMaxCrowdingRadius;

  /**Neighbor DBH effect. @htmlonly &alpha; @endhtmlonly variable in
   * equation above. Array sized number of species.*/
  float *mp_fAlpha;

  /**Neighbor distance effect. @htmlonly &beta; @endhtmlonly variable in
   * equation above. Array sized number of species.*/
  float *mp_fBeta;

  /**NCI neighbor storm medium damage parameter.
   * @htmlonly &eta;<sub>k</sub> @endhtmlonly in NCI equation above. This is
   * omitted or set to 1 if not used. Array sized number of species.*/
  float *mp_fMedDamageEta;

  /**NCI neighbor storm full damage parameter.
   * @htmlonly &eta;<sub>k</sub> @endhtmlonly in NCI equation above. This is
   * omitted or set to 1 if not used. Array sized number of species.*/
  float *mp_fFullDamageEta;

  /**The minimum DBH, in cm, of neighbors to be included in NCI calculations.
   * Array assumed to be sized total number of species.*/
  float *mp_fMinimumNeighborDBH;

  /**The value to divide DBH by in NCI. <i>q</i> in the NCI equation above.
   * May be set to 1.*/
  float m_fDbhDivisor;

  /**Minimum sapling height.  For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Whether or not to include snags in NCI*/
  bool m_bIncludeSnags;

  /**Number of total species. For the destructor.*/
  int m_iNumTotalSpecies;
};

#endif /* NCITERMWITHNEIGHBORDAMAGE_H_ */
