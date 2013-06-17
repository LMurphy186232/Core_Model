/*
 * NoShadingEffect.h
 *
 *  Created on: Nov 27, 2012
 *      Author: LORA
 */

#ifndef NOSHADINGEFFECT_H_
#define NOSHADINGEFFECT_H_

#include "ShadingEffectBase.h"

/**
 * Class that provides no shading effect (value of 1).
 */
class clNoShadingEffect: virtual public clShadingEffectBase {
public:
  float CalculateShadingEffect(clTree *p_oTree) {return 1;};
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {;};
};

#endif /* NOSHADINGEFFECT_H_ */
