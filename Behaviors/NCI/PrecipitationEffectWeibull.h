#ifndef WEIBULLPRECIPITATIONEFFECT_H_
#define WEIBULLPRECIPITATIONEFFECT_H_

#include "PrecipitationEffectBase.h"

class clPlot;

/**
 * This returns the precipitation effect using a Weibull function. Three
 * possible precipitation values can be used: mean annual precipitation,
 * seasonal precipitation, or annual water deficit. The function
 * is:
 * <br>
 * @htmlonly
   Precipitation Effect = exp(-0.5*(abs(ppt - C)/A)<sup>B</sup>)
   @endhtmlonly
 * where:
 * <ul>
 * <li>ppt is the value for the desired form of precipitation, from the plot object</li>
 * <li>A, B, and C are parameters</li>
 * </ul>
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
  <br>Edit history:
  <br>-----------------
  <br>June 13, 2013 - Created for transferal of Weibull Climate Growth to
  the NCI framework (LEM)
  <br>October 31, 2013 - Added options for water deficit and seasonal
  precipitation (LEM)
 */
class clPrecipitationEffectWeibull: virtual public clPrecipitationEffectBase {
public:

  /**
   * Constructor.
   */
  clPrecipitationEffectWeibull();

  /**
   * Destructor.
   */
  ~clPrecipitationEffectWeibull();

  /**
   * Calculates precipitation effect for a particular species.
   * @param p_oPlot Plot object for querying for precipitation values.
   * @param iSpecies Species for which to calculate precipitation effect.
   */
  double CalculatePrecipitationEffect(clPlot *p_oPlot, int iSpecies);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if Storm Effect parameters are not between 0 and 1.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Define a type for pointers to plot functions for getting precipitation*/
  typedef double (clPlot::*Ptr2Precip)();
  /**Pointer for the appropriate function for getting precip.*/
  Ptr2Precip m_precip;

  /**Precipitation effect A. Array is sized number of species.*/
  double *mp_fPrecipA;

  /**Precipitation effect B. Array is sized number of species.*/
  double *mp_fPrecipB;

  /**Precipitation effect C.  Array is sized number of species.*/
  double *mp_fPrecipC;
};

#endif /* WEIBULLPRECIPITATIONEFFECT_H_ */
