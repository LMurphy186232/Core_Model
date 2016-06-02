#ifndef NCITERMNCIBARATIO_H_
#define NCITERMNCIBARATIO_H_

#include "NCITermBase.h"

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
class clNCITermNCIBARatio: virtual public clNCITermBase {
public:

  /**
   * Constructor.
   */
  clNCITermNCIBARatio(bool bUseDefaultBA);

  /**
   * Destructor.
   */
  ~clNCITermNCIBARatio();

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
   * if DBH divisor is <= 0.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Pointer to the plot object, so we can get the temperature*/
  clPlot *mp_oPlot;

  /**Lambas. Array sized number of total species by number of total
   * species.*/
  double **mp_fLambda;

  /**Neighbor DBH effect. @htmlonly &alpha; @endhtmlonly variable in
   * equation above. Array sized number of species.*/
  double *mp_fAlpha;

  /**Neighbor distance effect. @htmlonly &beta; @endhtmlonly variable in
   * equation above. Array sized number of species.*/
  double *mp_fBeta;

  /**The minimum DBH, in cm, of neighbors to be included in NCI calculations.
   * Array assumed to be sized total number of species.*/
  double *mp_fMinimumNeighborDBH;

  /**List of distinct species.*/
  short int *mp_iWhatSpecies;

  /**Maximum search radius, in meters, in which to look for
   * neighbors.*/
  double m_fMaxCrowdingRadius;

  /**Maximum search radius, in meters, in which to look for crowding adult
   * neighbors.*/
  double m_fMaxAdultRadius;

  /**Maximum search radius, in meters, in which to look for crowding sapling
   * neighbors.*/
  double m_fMaxSaplingRadius;

  /**The units adjustment factor for DBH in NCI. <i>q</i> in the NCI equation
   * above. May be set to 1.*/
  double m_fDbhAdjustor;

  /**Minimum sapling height.  For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Basal area for default size, if m_bUseDefaultBA = true.*/
  double m_fDefaultBA;

  /**Number of behavior species.*/
  int m_iNumBehaviorSpecies;

  /**Number of total species. For the destructor.*/
  int m_iNumTotalSpecies;

  /**Whether or not to use a default BA, as with quadrat calcs*/
  bool m_bUseDefaultBA;
};

#endif /* NCITERMNCIBARATIO_H_ */
