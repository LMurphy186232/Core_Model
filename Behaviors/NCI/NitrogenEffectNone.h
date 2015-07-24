#ifndef NONITROGENEFFECT_H_
#define NONITROGENEFFECT_H_

#include "NitrogenEffectBase.h"

/**
 * Provides no nitrogen effect (returns a value of 1).
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>June 27, 2013 - Created (LEM)
 */
class clNitrogenEffectNone: virtual public clNitrogenEffectBase {
public:


  /**
   * Calculates nitrogen effect for a particular species.
   * @param p_oPlot Plot object for querying for nitrogen values.
   * @param iSpecies Species for which to calculate nitrogen effect.
   */
  float CalculateNitrogenEffect(clPlot *p_oPlot, int iSpecies) {return 1.0;};

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement){};
};

#endif /* NONITROGENEFFECT_H_ */
