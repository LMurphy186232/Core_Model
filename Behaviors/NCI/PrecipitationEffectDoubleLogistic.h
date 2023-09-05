#ifndef PRECIPITATIONEFFECTDOUBLELOGISTIC_H_
#define PRECIPITATIONEFFECTDOUBLELOGISTIC_H_

#include "PrecipitationEffectBase.h"

class clPlot;

/**
 * This returns the precipitation effect using a double logistic function. Three
 * possible precipitation values can be used: mean annual precipitation,
 * seasonal precipitation, or annual water deficit. The function
 * is:
 *
 * Precipitation Effect = (al + ((1-al)/(1+(bl/ppt)^cl))) *
 *                        (ah + ((1-ah)/(1+(ppt/bh)^ch)))
 *
 * where:
 * <ul>
 * <li>ppt is the value for the desired form of precipitation, from the plot object</li>
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
class clPrecipitationEffectDoubleLogistic: virtual public clPrecipitationEffectBase {
public:

  /**Precipitation type*/
  enum precipType {
    mean_precip, /**<Mean annual precipitation*/
    seasonal_precip, /**<Seasonal precipitation*/
    water_deficit /**<Water deficit*/
  };

  /**
   * Constructor.
   */
  clPrecipitationEffectDoubleLogistic();

  /**
   * Destructor.
   */
  ~clPrecipitationEffectDoubleLogistic();

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
   * @throws ModelException if precipitation type is not recognized.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Define a type for pointers to plot functions for getting precipitation*/
  typedef double (clPlot::*Ptr2Precip)();
  /**Pointer for the appropriate function for getting precip.*/
  Ptr2Precip m_precip;

  /**Precipitation effect al. Array is sized number of species.*/
  double *mp_fAl;

  /**Precipitation effect bl. Array is sized number of species.*/
  double *mp_fBl;

  /**Precipitation effect cl.  Array is sized number of species.*/
  double *mp_fCl;

  /**Precipitation effect ah. Array is sized number of species.*/
  double *mp_fAh;

  /**Precipitation effect bh. Array is sized number of species.*/
  double *mp_fBh;

  /**Precipitation effect ch.  Array is sized number of species.*/
  double *mp_fCh;
};

#endif
