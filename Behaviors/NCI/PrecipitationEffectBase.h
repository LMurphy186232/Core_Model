#ifndef PRECIPITATIONEFFECTBASE_H_
#define PRECIPITATIONEFFECTBASE_H_

#include <xercesc/dom/DOM.hpp>

class clBehaviorBase;
class clTreePopulation;
class clPlot;

/**
 * Provides a base for objects that calculate a precipitation effect.
 */
class clPrecipitationEffectBase {

public:

  /**
   * Constructor. Sets defaults.
   */
  clPrecipitationEffectBase() {bRequiresTargetDiam = false;};

  /**
   * Destructor
   */
  virtual ~clPrecipitationEffectBase(){};

  /**Precipitation type*/
  enum precipType {
    mean_precip, /**<Mean annual precipitation*/
    seasonal_precip, /**<Seasonal precipitation*/
    water_deficit /**<Water deficit*/
  };

  /**
   * Calculates precipitation effect for a particular species.
   * @param p_oPlot Plot object for querying for precipitation values.
   * @param iSpecies Species for which to calculate precipitation effect.
   */
  virtual float CalculatePrecipitationEffect(clPlot *p_oPlot, int iSpecies) = 0;

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  virtual void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) = 0;

  bool DoesRequireTargetDiam() {return bRequiresTargetDiam;};

  protected:

  /** Whether or not this effect depends on a target diameter being available.*/
  bool bRequiresTargetDiam;
};

#endif
