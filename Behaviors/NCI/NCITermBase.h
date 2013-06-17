#ifndef NCITERMBASE_H_
#define NCITERMBASE_H_

#include <xercesc/dom/DOM.hpp>

class clTreePopulation;
class clBehaviorBase;
class clTree;
class clPlot;

/**
 * Provides a base for objects that calculate the NCI term.
 */
class clNCITermBase {


public:

  /**
   * Calculates NCI term.
   * @param p_oTree Tree for which to calculate NCI.
   * @param p_oPop Tree population.
   * @param p_oPlot Plot object.
   */
  virtual float CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot) = 0;

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  virtual void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) = 0;

  /**
   * Destructor
   */
  virtual ~clNCITermBase(){};
};

#endif /* NCITERMBASE_H_ */
