#ifndef CROWDINGEFFECTNONE_H_
#define CROWDINGEFFECTNONE_H_

#include "CrowdingEffectBase.h"

/**
 * Class that provides no crowding effect (value of 1).
 */
class clCrowdingEffectNone: virtual public clCrowdingEffectBase {
public:
  float CalculateCrowdingEffect(clTree *p_oTree, const float &fDiam, const clNCITermBase::ncivals nci, const int &iSpecies) {return 1;};
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, clNCIBehaviorBase *p_oNCIBase, xercesc::DOMElement *p_oElement) {;};
};

#endif
