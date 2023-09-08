#ifndef PRECIPITATIONEFFECTDOUBLENOLOCALDIFF_H_
#define PRECIPITATIONEFFECTDOUBLENOLOCALDIFF_H_

#include "PrecipitationEffectBase.h"

class clPlot;

/**
 * This returns the precipitation effect using a double function. Three
 * possible precipitation values can be used: mean annual precipitation,
 * seasonal precipitation, or annual water deficit. The function
 * is:
 *
 * Precipitation Effect = current + prev
 *
 * where both current and prev have the form:
 *
 * effect = a * b.lo ^((ppt-c)^2) if ppt < c
 *        = a * b.hi ^((ppt-c)^2) if ppt >= c
 *
 * where:
 * <ul>
 * <li>ppt is the value for the desired form of precipitation, from the plot
 * object; current year for current, previous year for prev; in meters</li>
 * <li>a, b.lo, b.hi, and c are parameters</li>
 * </ul>
 *
 * Copyright 2023 Charles D. Canham.
 * @author Lora E. Murphy
 *
  <br>Edit history:
  <br>-----------------
  <br>September 5, 2023 - Created (LEM)
 */
class clPrecipitationEffectDoubleNoLocalDiff: virtual public clPrecipitationEffectBase {
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
  clPrecipitationEffectDoubleNoLocalDiff();

  /**
   * Destructor.
   */
  ~clPrecipitationEffectDoubleNoLocalDiff();

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

  /**
   * Manages the assignment of current and previous precipitation.
   */
  void PreCalcs(clPlot *p_oPlot);

protected:

  /**Define a type for pointers to plot functions for getting precipitation*/
  typedef double (clPlot::*Ptr2Precip)();
  /**Pointer for the appropriate function for getting precip.*/
  Ptr2Precip m_precip;

  /** Current year precipitation value */
  double m_fCurrPrecip;

  /** Previous year precipitation value */
  double m_fPrevPrecip;

  /**Precipitation effect a for current year. Array is sized number of
   * species.*/
  double *mp_fCurrA;

  /**Precipitation effect b.lo for current year. Array is sized number of
     * species.*/
  double *mp_fCurrBLo;

  /**Precipitation effect b.hi for current year. Array is sized number of
     * species.*/
  double *mp_fCurrBHi;

  /**Precipitation effect c for current year. Array is sized number of
     * species.*/
  double *mp_fCurrC;

  /**Precipitation effect a for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevA;

  /**Precipitation effect b.lo for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevBLo;

  /**Precipitation effect b.hi for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevBHi;

  /**Precipitation effect c for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevC;

};

#endif
