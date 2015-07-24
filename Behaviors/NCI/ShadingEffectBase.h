#ifndef SHADINGEFFECTBASE_H_
#define SHADINGEFFECTBASE_H_

#include <xercesc/dom/DOM.hpp>

class clTreePopulation;
class clBehaviorBase;
class clTree;

/**
 * Provides a base for objects that calculate a shading effect.
 */
class clShadingEffectBase {

public:

  /**
   * Calculates shading effect.
   * @param p_oTree Tree for which to calculate shading effect.
   */
  virtual float CalculateShadingEffect(clTree *p_oTree) = 0;

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
  virtual ~clShadingEffectBase(){};

  /**
  * Performs calculations like either clGrowthBase::PreGrowthCalcs or
  * clMortalityBase::PreMortCalcs.
  */
  virtual void PreCalcs( clTreePopulation *p_oPop ){;};

};


#endif /* SHADINGEFFECTBASE_H_ */
