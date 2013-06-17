#ifndef SIZEEFFECT_H_
#define SIZEEFFECT_H_

#include "SizeEffectBase.h"

/**
 * Class that provides no size effect (value of 1).
 */
class clNoSizeEffect: virtual public clSizeEffectBase {
public:
  float CalculateSizeEffect(int iSpecies, float fDiam) {return 1;};
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {;};
};

#endif /* NOSIZEEFFECT_H_ */
