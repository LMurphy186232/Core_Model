#ifndef NOINFECTIONEFFECT_H_
#define NOINFECTIONEFFECT_H_

#include "InfectionEffectBase.h"

/**
 * Provides no infection effect (returns a value of 1).
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>November 1, 2013 - Created (LEM)
 */
class clInfectionEffectNone: virtual public clInfectionEffectBase {
public:


  /**
   * Calculates infection effect for a particular species.
   * @param p_oTree Tree for which to calculate infection effect.
   */
  double CalculateInfectionEffect(clTree *p_oTree) {return 1.0;};

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement){};
};

#endif /* NOINFECTIONEFFECT_H_ */
