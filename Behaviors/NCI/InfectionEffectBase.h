#ifndef INFECTIONEFFECTBASE_H_
#define INFECTIONEFFECTBASE_H_

#include <xercesc/dom/DOM.hpp>

class clBehaviorBase;
class clTreePopulation;
class clTree;

/**
 * Provides a base for objects that calculate an infection effect.
 */
class clInfectionEffectBase {

public:

  /**
   * Constructor. Sets defaults.
   */
  clInfectionEffectBase() {bRequiresTargetDiam = false;};

  /**
   * Calculates infection effect for a particular species.
   * @param p_oTree Tree for which to calculate infection effect.
   */
  virtual double CalculateInfectionEffect(clTree *p_oTree) = 0;

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
  virtual ~clInfectionEffectBase(){};

  bool DoesRequireTargetDiam() {return bRequiresTargetDiam;};

  /**
  * Performs calculations like either clGrowthBase::PreGrowthCalcs or
  * clMortalityBase::PreMortCalcs.
  */
  virtual void PreCalcs( clTreePopulation *p_oPop ){;};

protected:

  /** Whether or not this effect depends on a target diameter being available.*/
  bool bRequiresTargetDiam;
};

#endif
