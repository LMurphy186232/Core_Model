#ifndef CROWDINGEFFECTNONE_H_
#define CROWDINGEFFECTNONE_H_

#include "CrowdingEffectBase.h"

/**
 * Class that provides no crowding effect (value of 1).
 */
class clCrowdingEffectNone: virtual public clCrowdingEffectBase {
public:

  /**
   * Returns a constant crowding effect of 1.
   * @param p_oTree Tree for which to calculate crowding effect. If
   * bRequiresTargetDiam is false, be prepared for this to be NULL.
   * @param fDiam Diameter of tree. May not be the same as the one recorded in
   * the tree record if it is being updated for consecutive years in a timestep.
   * @param nci NCI term. May be 0 if NCI not used.
   * @param iSpecies Species for which to calculate effect. This is separate
   * in case p_oTree is NULL because there is no target.
   */
  float CalculateCrowdingEffect(clTree *p_oTree, const float &fDiam, const clNCITermBase::ncivals nci, const int &iSpecies) {return 1;};

  /**
   * No setup required.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oNCIBase NCI parent behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, clNCIBehaviorBase *p_oNCIBase, xercesc::DOMElement *p_oElement) {;};
};

#endif
