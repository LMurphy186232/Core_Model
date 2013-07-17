#ifndef NCINEIGHBORBA_H_
#define NCINEIGHBORBA_H_

#include "NCITermBase.h"

/**
 * This calculates an NCI term which is the sum of BA of neighbors. The
 * candidate neighbors are those within a certain radius and above a minimum
 * DBH. Optionally, this can sum only those neighbors larger than the target.
 *
 * BA is reported in cm2, but there is a divisor term which can be used to
 * adjust units if desired.
 */
class clNCINeighborBA: public clNCITermBase {
public:

  /**
   * Constructor.
   */
  clNCINeighborBA();

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
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

  ~clNCINeighborBA();

protected:
  /**Maximum search radius, in meters, in which to look for crowding
   * neighbors. Array is sized number of species.*/
  float *mp_fMaxCrowdingRadius;

  /**The minimum DBH, in cm, of neighbors to be included in the neighbor count.
   * Array is sized number of species.*/
  float *mp_fMinimumNeighborDBH;

  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Divisor for neighbor BA.*/
  float m_fBADivisor;

  /** Whether to use all trees larger than the minimum (false) or only neighbors
   * larger than the target (true)*/
  bool m_bUseOnlyLargerNeighbors;
};

#endif /* NCINEIGHBORBA_H_ */
