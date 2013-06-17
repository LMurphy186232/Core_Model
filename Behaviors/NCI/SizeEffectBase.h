#ifndef SIZEEFFECTBASE_H_
#define SIZEEFFECTBASE_H_

#include <xercesc/dom/DOM.hpp>

class clTreePopulation;
class clBehaviorBase;
class clTree;

/**
 * Provides a base for objects that calculate a size effect.
 */
class clSizeEffectBase {

public:

  /**
   * Calculates size effect.
   * @param iSpecies Species of tree.
   * @param fDiam Diameter of tree. Diameter at 10 cm for seedlings, DBH for
   * all other types.
   */
  virtual float CalculateSizeEffect(int iSpecies, float fDiam) = 0;

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
  virtual ~clSizeEffectBase(){};
};

#endif /* SIZEEFFECTBASE_H_ */
