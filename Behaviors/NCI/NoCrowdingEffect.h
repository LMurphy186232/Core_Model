#ifndef NOCROWDINGEFFECT_H_
#define NOCROWDINGEFFECT_H_

#include "CrowdingEffectBase.h"

/**
 * Class that provides no crowding effect (value of 1).
 */
class clNoCrowdingEffect: virtual public clCrowdingEffectBase {
public:
  float CalculateCrowdingEffect(clTree *p_oTree, float fDiam, float fNCI) {return 1;};
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {;};
};

#endif /* NOCROWDINGEFFECT_H_ */
