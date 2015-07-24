#ifndef NCITERMBARATIODBHDEFAULT_H_
#define NCITERMBARATIODBHDEFAULT_H_

#include "NCITermBase.h"

/**
 * This calculates a basal area ratio for a tree, using a single default value
 * for target tree size.
 *
 * This returns two terms: term #1 is the total basal area of neighbors; and
 * term #2 is the ratio of mean basal area of neighbors to target
 * basal area.
 *
 * NCI ignores neighbors with disturbance and harvest death codes. Natural
 * deaths are NOT ignored, because it presumes that those deaths occurred in
 * the current timestep and they should still be considered as live neighbors.
 *
 * This behavior uses two different distances to look for neighbors. One is for
 * saplings, and one is for adults.
 *
 */
class clNCITermBARatioDBHDefault: virtual public clNCITermBase {
public:

  /**
   * Constructor.
   */
  clNCITermBARatioDBHDefault();

  /**
   * Destructor. Not needed.
   */
  //~clNCITermBARatioDBHDefault();

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
   * @throws ModelException if the max radius of neighbor effects is < 0.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Maximum search radius, in meters, in which to look for
   * neighbors.*/
  float m_fMaxCrowdingRadius;

  /**Maximum search radius, in meters, in which to look for crowding adult
   * neighbors.*/
  float m_fMaxAdultRadius;

  /**Maximum search radius, in meters, in which to look for crowding sapling
   * neighbors.*/
  float m_fMaxSaplingRadius;

  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Basal area for default size.*/
  float m_fDefaultBA;
};

#endif /* DEFAULTNCITERM_H_ */
