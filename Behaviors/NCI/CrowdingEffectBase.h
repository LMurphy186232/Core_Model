#ifndef CROWDINGEFFECTBASE_H_
#define CROWDINGEFFECTBASE_H_

#include <xercesc/dom/DOM.hpp>
#include "NCITermBase.h"

class clTreePopulation;
class clNCIBehaviorBase;
class clTree;

/**
 * Provides a base for objects that calculate a crowding effect.
 */
class clCrowdingEffectBase {

public:

  /**
   * Constructor. Sets defaults.
   */
  clCrowdingEffectBase() {m_bRequiresTargetDiam = false; m_b2ValNCI = false;};

  /**
   * Destructor
   */
  virtual ~clCrowdingEffectBase(){};

  /**
   * Calculates crowding effect.
   * @param p_oTree Tree for which to calculate crowding effect. If
   * bRequiresTargetDiam is false, be prepared for this to be NULL.
   * @param fDiam Diameter of tree. May not be the same as the one recorded in
   * the tree record if it is being updated for consecutive years in a timestep.
   * @param nci NCI term. May be 0 if NCI not used.
   * @param iSpecies Species for which to calculate effect. This is separate
   * in case p_oTree is NULL because there is no target.
   */
  virtual double CalculateCrowdingEffect(clTree *p_oTree, const float &fDiam, const clNCITermBase::ncivals nci, const int &iSpecies) = 0;

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oNCIBase NCI parent behavior object.
   * @param p_oElement Root element of the behavior.
   */
  virtual void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, clNCIBehaviorBase *p_oNCIBase, xercesc::DOMElement *p_oElement) = 0;

  /**
  * Performs calculations like either clGrowthBase::PreGrowthCalcs or
  * clMortalityBase::PreMortCalcs.
  */
  virtual void PreCalcs( clTreePopulation *p_oPop ){;};



  bool DoesRequireTargetDiam() {return m_bRequiresTargetDiam;};

protected:

  /** Whether or not this effect depends on a target diameter being available.*/
  bool m_bRequiresTargetDiam;

  /**Whether it's a two-value NCI.*/
  bool m_b2ValNCI;
};

#endif /* CROWDINGEFFECTBASE_H_ */
