#ifndef SIZEEFFECTBASE_H_
#define SIZEEFFECTBASE_H_

#include <xercesc/dom/DOM.hpp>

class clTreePopulation;
class clBehaviorBase;
class clTree;

/**
 * Provides a base for objects that calculate a size effect. The default is that
 * objects descended from this class can handle any tree type. If this is not
 * the case, child objects must throw the appropriate error during setup.
 */
class clSizeEffectBase {

public:

  /**
   * Calculates size effect.
   * @param p_oTree Tree for which to calculate size effect.
   * @param fDiam Diameter of tree. Diameter at 10 cm for seedlings, DBH for
   * all other types. This can be got from the tree but it's already been
   * extracted so let's pass it in.
   */
  virtual double CalculateSizeEffect(clTree *p_oTree, const float &fDiam) = 0;

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  virtual void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) = 0;

  /**
  * Performs calculations like either clGrowthBase::PreGrowthCalcs or
  * clMortalityBase::PreMortCalcs.
  */
  virtual void PreCalcs( clTreePopulation *p_oPop ){;};

  /**
   * Destructor
   */
  virtual ~clSizeEffectBase(){};

};

#endif /* SIZEEFFECTBASE_H_ */
