#ifndef TEMPERATUREEFFECTDOUBLELOGISTIC_H_
#define TEMPERATUREEFFECTDOUBLELOGISTIC_H_

#include "TemperatureEffectBase.h"

/**
 * This returns the temperature effect using a double logistic function. The
 * function  is:
 *
 *
 * Temperature Effect = (al + ((1-al)/(1+(bl/ppt)^cl))) *
 *                      (ah + ((1-ah)/(1+(ppt/bh)^ch)))
 *
 * where:
 * <ul>
 * <li>temp is the mean annual temperature in K, from the plot object</li>
 * <li>al, bl, cl, ah, bh, and ch are parameters</li>
 * </ul>
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
   <br>Edit history:
   <br>-----------------
   <br>December 20, 2013 - Created (LEM)
 */
class clTemperatureEffectDoubleLogistic: virtual public clTemperatureEffectBase {
public:

  /**
   * Constructor.
   */
  clTemperatureEffectDoubleLogistic();

  /**
   * Destructor.
   */
  ~clTemperatureEffectDoubleLogistic();

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

  /**Temperature effect al. Array is sized number of species.*/
  float *mp_fAl;

  /**Temperature effect bl. Array is sized number of species.*/
  float *mp_fBl;

  /**Temperature effect cl.  Array is sized number of species.*/
  float *mp_fCl;

  /**Temperature effect ah. Array is sized number of species.*/
  float *mp_fAh;

  /**Temperature effect bh. Array is sized number of species.*/
  float *mp_fBh;

  /**Temperature effect ch.  Array is sized number of species.*/
  float *mp_fCh;
};

#endif
