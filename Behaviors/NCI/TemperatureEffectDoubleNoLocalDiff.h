#ifndef TEMPERATUREEFFECTDOUBLENOLOCAL_H_
#define TEMPERATUREEFFECTDOUBLENOLOCAL_H_

#include "TemperatureEffectBase.h"

/**
 * This returns the temperature effect using a double function.
 * is:
 *
 * Temperature Effect = current + prev
 *
 * where both current and prev have the form:
 *
 * effect = a * b.lo ^((ppt-c)^2) if ppt < c
 *        = a * b.hi ^((ppt-c)^2) if ppt >= c
 *
 * where:
 * <ul>
 * <li>ppt is the value for the desired form of temperature, from the plot
 * object; current year for current, previous year for prev</li>
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
class clTemperatureEffectDoubleNoLocalDiff: virtual public clTemperatureEffectBase {
public:

  /**
   * Constructor.
   */
  clTemperatureEffectDoubleNoLocalDiff();

  /**
   * Destructor.
   */
  ~clTemperatureEffectDoubleNoLocalDiff();

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

  /**Temperature effect a for current year. Array is sized number of
   * species.*/
  double *mp_fCurrA;

  /**Temperature effect b.lo for current year. Array is sized number of
   * species.*/
  double *mp_fCurrBLo;

  /**Temperature effect b.hi for current year. Array is sized number of
   * species.*/
  double *mp_fCurrBHi;

  /**Temperature effect c for current year. Array is sized number of
   * species.*/
  double *mp_fCurrC;

  /**Temperature effect a for previous year. Array is sized number of
   * species.*/
  double *mp_fPrevA;

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
