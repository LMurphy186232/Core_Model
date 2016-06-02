#ifndef NOTEMPERATUREEFFECT_H_
#define NOTEMPERATUREEFFECT_H_

#include "TemperatureEffectBase.h"

/**
 * Provides no temperature effect (returns a value of 1).
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>June 13, 2013 - Created for transferal of Weibull Climate Growth to
 * the NCI framework (LEM)
 */
class clTemperatureEffectNone: virtual public clTemperatureEffectBase {
public:


  /**
   * Calculates temperature effect for a particular species.
   * @param p_oPlot Plot object for querying for temperature values.
   * @param iSpecies Species for which to calculate temperature effect.
   */
  double CalculateTemperatureEffect(clPlot *p_oPlot, int iSpecies) {return 1.0;};

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement){};
};

#endif /* NOTEMPERATUREEFFECT_H_ */
