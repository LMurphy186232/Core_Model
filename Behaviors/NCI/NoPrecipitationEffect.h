#ifndef NOPRECIPITATIONEFFECT_H_
#define NOPRECIPITATIONEFFECT_H_

#include "PrecipitationEffectBase.h"

/**
 * Provides no precipitation effect (returns a value of 1).
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>June 13, 2013 - Created for transferal of Weibull Climate Growth to
 * the NCI framework (LEM)
 */
class clNoPrecipitationEffect: virtual public clPrecipitationEffectBase {
public:


  /**
   * Calculates precipitation effect for a particular species.
   * @param p_oPlot Plot object for querying for temperature values.
   * @param iSpecies Species for which to calculate precipitation effect.
   */
  float CalculatePrecipitationEffect(clPlot *p_oPlot, int iSpecies) {return 1.0;};

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement){};
};

#endif /* NOPRECIPITATIONEFFECT_H_ */
