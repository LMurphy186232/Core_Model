#ifndef TEMPERATUREEFFECTDOUBLELOCAL_H_
#define TEMPERATUREEFFECTDOUBLELOCAL_H_

#include "TemperatureEffectBase.h"

/**
 * This returns the temperature effect using a double function.
 * is:
 *
 * Temperature Effect = current + prev
 *
 * where both current and prev have the form:
 *
 * effect = a * b.lo ^((ppt-lts)^2) if ppt <  lts
 *        = a * b.hi ^((ppt-lts)^2) if ppt >= lts
 *
 * where:
 * <ul>
 * <li>ppt is the value for temperature, from the plot
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
class clTemperatureEffectDoubleLocalDiff: virtual public clTemperatureEffectBase {
public:

  /**
   * Constructor.
   */
  clTemperatureEffectDoubleLocalDiff();

  /**
   * Destructor.
   */
  ~clTemperatureEffectDoubleLocalDiff();

  /**
   * Calculates temperature effect for a particular species.
   * @param p_oPlot Plot object for querying for temperature values.
   * @param iSpecies Species for which to calculate temperature effect.
   */
  double CalculateTemperatureEffect(clPlot *p_oPlot, int iSpecies);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if Storm Effect parameters are not between 0 and 1.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

  /**
   * Manages the assignment of current and previous precipitation.
   */
  void PreCalcs(clPlot *p_oPlot);

protected:

  /** Current year temperature value */
  double m_fCurrTemp;

  /** Previous year temperature value */
  double m_fPrevTemp;

  /**Temperature effect a.a for current year. Array is sized number of
   * species.*/
  double *mp_fCurrAA;

  /**Temperature effect a.b.lo for current year. Array is sized number of
   * species.*/
  double *mp_fCurrABLo;

  /**Temperature effect a.b.hi for current year. Array is sized number of
   * species.*/
  double *mp_fCurrABHi;

  /**Temperature effect a.a for current year. Array is sized number of
   * species.*/
  double *mp_fCurrAC;

  /**Temperature effect b.lo for current year. Array is sized number of
   * species.*/
  double *mp_fCurrBLo;

  /**Temperature effect b.hi for current year. Array is sized number of
   * species.*/
  double *mp_fCurrBHi;

  /**Temperature effect c for current year. Array is sized number of
   * species.*/
  double *mp_fCurrC;

  /**Temperature effect a.a for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevAA;

  /**Temperature effect a.b.lo for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevABLo;

  /**Temperature effect a.b.hi for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevABHi;

  /**Temperature effect a.a for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevAC;

  /**Temperature effect b.lo for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevBLo;

  /**Temperature effect b.hi for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevBHi;

  /**Temperature effect c for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevC;
};

#endif
