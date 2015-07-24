#ifndef WEIBULLTEMPERATUREEFFECT_H_
#define WEIBULLTEMPERATUREEFFECT_H_

#include "TemperatureEffectBase.h"

/**
 * This returns the temperature effect using a Weibull function. The function
 * is:
 * <br>
 * @htmlonly
   Temperature Effect = exp(-0.5*(abs(temp - C)/A)<sup>B</sup>)
   @endhtmlonly
 * where:
 * <ul>
 * <li>temp is the mean annual temperature in C, from the plot object</li>
 * <li>A, B, and C are parameters</li>
 * </ul>
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>June 13, 2013 - Created for transferal of Weibull Climate Growth to
 * the NCI framework (LEM)
 */
class clTemperatureEffectWeibull: virtual public clTemperatureEffectBase {
public:

  /**
   * Constructor.
   */
  clTemperatureEffectWeibull();

  /**
   * Destructor.
   */
  ~clTemperatureEffectWeibull();

  /**
   * Calculates temperature effect for a particular species.
   * @param p_oPlot Plot object for querying for temperature values.
   * @param iSpecies Species for which to calculate temperature effect.
   */
  float CalculateTemperatureEffect(clPlot *p_oPlot, int iSpecies);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if Storm Effect parameters are not between 0 and 1.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Temperature effect A. Array is sized number of species.*/
  float *mp_fTempA;

  /**Temperature effect B. Array is sized number of species.*/
  float *mp_fTempB;

  /**Temperature effect C.  Array is sized number of species.*/
  float *mp_fTempC;
};

#endif /* WEIBULLTEMPERATUREEFFECT_H_ */
