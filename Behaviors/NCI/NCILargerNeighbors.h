#ifndef NCILARGERNEIGHBORS_H_
#define NCILARGERNEIGHBORS_H_

#include "NCITermBase.h"

/**
 * The NCI term is simply a count of sapling and adult neighbors with a larger
 * DBH than the target within a certain radius, subject to a minimum value.
 *
 * This cannot be applied to seedlings. No real reason, I could allow this if
 * someone wants it.
 */
class clNCILargerNeighbors: public clNCITermBase {
public:

  /**
   * Constructor.
   */
  clNCILargerNeighbors();

  /**
   * Calculates NCI term.
   * @param p_oTree Tree for which to calculate NCI.
   * @param p_oPop Tree population.
   * @param p_oPlot Plot object.
   */
  float CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if this is applied to seedlings.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

  ~clNCILargerNeighbors();

protected:
  /**Maximum search radius, in meters, in which to look for crowding
   * neighbors. Array is sized number of species.*/
  float *mp_fMaxCrowdingRadius;

  /**The minimum DBH, in cm, of neighbors to be included in the neighbor count.
   * Array is sized number of species.*/
  float *mp_fMinimumNeighborDBH;

  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;
};

#endif /* NCILARGERNEIGHBORS_H_ */
