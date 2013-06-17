#ifndef CROWDINGEFFECTBASE_H_
#define CROWDINGEFFECTBASE_H_

#include <xercesc/dom/DOM.hpp>

class clTreePopulation;
class clBehaviorBase;
class clTree;

/**
 * Provides a base for objects that calculate a crowding effect.
 */
class clCrowdingEffectBase {


public:

  /**
   * Calculates crowding effect.
   * @param p_oTree Tree for which to calculate crowding effect.
   * @param fDiam Diameter of tree. May not be the same as the one recorded in
   * the tree record if it is being updated for consecutive years in a timestep.
   * @param fNCI NCI term. May be 0 if NCI not used.
   */
  virtual float CalculateCrowdingEffect(clTree *p_oTree, float fDiam, float fNCI) = 0;

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
  virtual ~clCrowdingEffectBase(){};
};

#endif /* CROWDINGEFFECTBASE_H_ */
