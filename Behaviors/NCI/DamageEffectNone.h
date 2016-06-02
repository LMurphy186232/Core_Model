#ifndef NODAMAGEEFFECT_H_
#define NODAMAGEEFFECT_H_

#include "DamageEffectBase.h"

/**
 * Class that provides no damage effect (value of 1).
 */
class clDamageEffectNone: virtual public clDamageEffectBase {
public:
  double CalculateDamageEffect(clTree *p_oTree) {return 1;};
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {;};
};

#endif /* NODAMAGEEFFECT_H_ */
