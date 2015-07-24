#ifndef SIZEEFFECT_H_
#define SIZEEFFECT_H_

#include "SizeEffectBase.h"

/**
 * Class that provides no size effect (value of 1).
 */
class clSizeEffectNone: virtual public clSizeEffectBase {
public:
  float CalculateSizeEffect(clTree *p_oTree, const float &fDiam) {return 1;};
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {;};
};

#endif /* NOSIZEEFFECT_H_ */
