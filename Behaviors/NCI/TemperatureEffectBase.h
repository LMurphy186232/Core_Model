#ifndef TEMPERATUREEFFECTBASE_H_
#define TEMPERATUREEFFECTBASE_H_

#include <xercesc/dom/DOM.hpp>

class clBehaviorBase;
class clTreePopulation;
class clPlot;

/**
 * Provides a base for objects that calculate a temperature effect.
 */
class clTemperatureEffectBase {


public:

  /**
   * Calculates temperature effect for a particular species.
   * @param p_oPlot Plot object for querying for temperature values.
   * @param iSpecies Species for which to calculate temperature effect.
   */
  virtual float CalculateTemperatureEffect(clPlot *p_oPlot, int iSpecies) = 0;

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  virtual void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) = 0;

  /**
   * Destructor
   */
  virtual ~clTemperatureEffectBase(){};
};

#endif
