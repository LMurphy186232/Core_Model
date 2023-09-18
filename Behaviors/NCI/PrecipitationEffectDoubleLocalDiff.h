#ifndef PRECIPITATIONEFFECTDOUBLELOCALDIFF_H_
#define PRECIPITATIONEFFECTDOUBLELOCALDIFF_H_

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
 * effect = a * b.lo ^((ppt-lts)^2) if ppt <  lts
 *        = a * b.hi ^((ppt-lts)^2) if ppt >= lts
 *
 * where:
 * <ul>
 * <li>ppt is the value for the desired form of precipitation, from the plot
 * object; current year for current, previous year for prev</li>
 * <li>b.lo, b.hi, and c are parameters</li>
 * </ul>
 *
 * lts = ltm + c
 * where ltm is the long-term mean of temperature and c is a parameter.
 *
 * a = a.a * a.b.lo ^((ltm-a.c)^2) if ltm <  a.c
 *   = a.a * a.b.hi ^((ltm-a.c)^2) if ltm >= a.c
 *
 * where a.a, a.b.lo, a.b.hi, and a.c are parameters.
 *
 * This has two possible forms: one where a.a.prev = 1-a.a.curr, and one
 * where it is an independent parameter.
 *
 * Copyright 2023 Charles D. Canham.
 * @author Lora E. Murphy
 *
  <br>Edit history:
  <br>-----------------
  <br>September 11, 2023 - Created (LEM)
 */
class clPrecipitationEffectDoubleLocalDiff: virtual public clPrecipitationEffectBase {
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
  clPrecipitationEffectDoubleLocalDiff();

  /**
   * Destructor.
   */
  ~clPrecipitationEffectDoubleLocalDiff();

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
  /**Pointer for the appropriate function for getting long-term precip.*/
  Ptr2Precip m_ltm_precip;

  /** Current year precipitation value */
  double m_fCurrPrecip;

  /** Previous year precipitation value */
  double m_fPrevPrecip;

  /**Precipitation effect a.a for current year. Array is sized number of
   * species.*/
  double *mp_fCurrAA;

  /**Precipitation effect a.b.lo for current year. Array is sized number of
   * species.*/
  double *mp_fCurrABLo;

  /**Precipitation effect a.b.hi for current year. Array is sized number of
   * species.*/
  double *mp_fCurrABHi;

  /**Precipitation effect a.a for current year. Array is sized number of
   * species.*/
  double *mp_fCurrAC;

  /**Precipitation effect b.lo for current year. Array is sized number of
   * species.*/
  double *mp_fCurrBLo;

  /**Precipitation effect b.hi for current year. Array is sized number of
   * species.*/
  double *mp_fCurrBHi;

  /**Precipitation effect c for current year. Array is sized number of
   * species.*/
  double *mp_fCurrC;

  /**Precipitation effect a.a for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevAA;

  /**Precipitation effect a.b.lo for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevABLo;

  /**Precipitation effect a.b.hi for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevABHi;

  /**Precipitation effect a.a for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevAC;

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
