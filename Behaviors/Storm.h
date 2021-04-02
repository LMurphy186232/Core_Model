//---------------------------------------------------------------------------
// Storm
//---------------------------------------------------------------------------
#if !defined(Storm_H)
  #define Storm_H

#include "BehaviorBase.h"
#include "ModelMath.h"

class clGrid;

/**
* Storms version 3.0.
* The storm damage calculator's function is to assess whether or not one or
* more storms has occurred, and if so, what the damage pattern and amount of
* damage were. It creates a map of storm damage across the plot, with each
* location getting a damage index between 0 (no damage) and 1 (most damage).
*
* <b>Deciding when storms occur</b><br>
* The 0 - 1 interval of storm severity values is subdivided into ten storm
* severity classes. Each class is assigned a return interval in the parameter
* file by the user. Taking the reciprocal of the return interval gives us the
* annual probability of each type of storm.
*
* To decide whether a given storm occurs, a random number is compared to the
* annual probability. Each storm severity class is decided individually.
* For multi-year timesteps, there is one random "coin flip" per year per
* severity class. This allows multiple storms to occur in the same timestep.
* For multi-year timesteps, multiple storms of the same severity class can
* occur, up to one per year. Each storm event creates a separate record in the
* storm damage grid. Storm severity is bounded between 0 and 1.
*
* The frequency of storms can optionally be cyclical (sinusoidal), with an
* optional overall trend. The actual probability of any storm that uses
* a cyclical storm regime is:
*
* <center>P'(Fi) = P(Fi) * ([d*sin(pi(x-g)/(2f))] + [mx + i])</center>
* where:
* <ul>
* <li>P'(Fi) = this timestep's probability of a storm of the ith return
* interval</li>
* <li>P(Fi) = the baseline (parameter file) probability of a storm of the ith
* return interval; that is, 1 / return interval</li>
* <li>The first term represents the sinusoidal portion</li>
* <li>The second term represents the trend portion</li>
* <li>x = 4*(t-t0)/Sr, where t = timestep since storms started, t0 = cycle
* start point, and Sr = sine periodicity parameter</li>
* <li>d is a parameter that controls the sine curve's amplitude</li>
* <li>f is a parameter that controls the sine curve's frequency</li>
* <li>g is a parameter that controls where on the curve we are when storms
* start occuring</li>
* <li>m is a parameter that gives the trend line's slope</li>
* <li>i is a parameter that gives the trend line's intercept</li>
* </ul>
*
* Storm events can also be specifically scheduled. In this case, they will
* occur in addition to any background storms. The user supplies a minimum and
* a maximum value for the storm intensity and a timestep.
*
* <b>Calculating damage when storms occur</b><br>
* The amount of damage caused by a timestep's storms is stored in the grid
* named "Storm Damage".  Each cell in the grid can contain 0 or more packages.
* 0 packages indicates no storm occurred. There is one package for each storm
* that occurred that timestep, with a single value between 0 (no damage) and 1
* (total damage).  In addition, a counter is set containing the
* number of years since the last storm.
*
* There are four damage patterns, and the value in the "Storm Damage" cells
* is calculated differently for each:
* <ol>
* <li><i>Varying spatial distribution, deterministic.</i>  In this pattern, a
* static storm vulnerability map is supplied for the plot in the parameter
* file.  The map is a set of values from 0 (no susceptibility) on up.  The
* susceptibility value at a location is multiplied by the damage index of any
* storm that hits to arrive at a final damage index for that location.</li>
* <li><i>Varying spatial distribution, stochastic.</i>  In this pattern, a
* static storm vulnerability map is supplied for the plot in the parameter
* file.  The map is a set of values from 0 (no susceptibility) to 1 (maximum
* susceptibility).  The damage index of a storm is determined as for the
* "Uniform stochastic" method.  Then the susceptibility value at a location is
* multiplied by this damage index to arrive at a final damage index for that
* location.</li>
* <li><i>Uniform deterministic.</i>  All plot locations receive the storm's
* designated damage index.</li>
* <li><i>Uniform stochastic.</i>  The storm's designated severity is used as the
* mean in a probability distribution function to arrive at a final damage
* index.</li>
* </ol>
*
* This behavior's call string and name string are both "Storm".
*
* Copyright 2011 Charles D. Canham
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*
* @author Lora E. Murphy
*/
class clStorm : virtual public clBehaviorBase {

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clStorm(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clStorm();

  /**
  * Does behavior setup.  It does the following:
  * <ol>
  * <li>Calls ReadParFile() to read parameter file values</li>
  * <li>Calls DoGridSetup() to set up the grids</li>
  * <li>Calls CalculateStormProbabilities() to calculate the annual storm
  * probabilities</li>
  * <li>Calls SetStochFuncPointer() to set the stochastic function pointer,
  * if appropriate</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs the storm calculations.  It loops through all of the storm
  * severity classes and, for each class and for each year per timestep, uses a
  * random number against the values in mp_fStormProbabilities to determine if
  * a storm of a given severity has occurred.  If a storm happens,
  * ApplyDamage() is called.
  */
  void Action();

  /**
   * Gets timestep probability of the occurence of a storm at a return
   * interval.
   * @param iReturnInterval Return interval.
   */
  float GetStormProbability(int iReturnInterval);

  /**
   * Sets timestep probability of the occurence of a storm at a return
   * interval.
   * @param iReturnInterval Return interval.
   * @param fValue Probability.
   */
  void SetStormProbability(int iReturnInterval, float fValue);

  protected:

  /**
  * Storms grid
  * This grid is named "Storm Damage" and it contains the severity of all
  * storms that happened in the previous timestep.  It is of user-settable
  * resolution.  It contains two float data members; one called "dmg_index",
  * which is the mean of all storm severities for that grid cell, and one
  * called "stormtime", which contains the time since the last storm, in years.
  * For "stormtime", 0 means a storm occurred in the current timestep. It also
  * can contain 1 or more packages, each with a float data member called
  * "1dmg_index", which contains the damage index as a value from 0 (no damage)
  * to 1 (total damage).
  *
  * If there is a map for the "Storm Susceptibility" grid, and no other grid
  * resolution information for this grid, then this grid will use the resolution
  * of "Storm Susceptibility".
  */
  clGrid *mp_oStormGrid;

  /**
  * Storm susceptibility map
  * This grid is named "Storm Susceptibility".  It has one float data member
  * called "index".  The value in "index" is a value from 0 (no storm
  * susceptibility) to 1 (total storm susceptibility).
  *
  * If the susceptibility = the enum value "mapped", then a grid map for this
  * grid is expected in the parameter file.  The values it contains will not
  * be changed throughout the run.  The grid resolution must match that of
  * "Storm Damage" grid.
  *
  * If the damage pattern is something other than "mapped", then this pointer
  * is left as NULL and the grid is not initialized or used.
  */
  clGrid *mp_oSusceptibilityMap;

  /**
  * Structure for holding scheduled storm events
  */
  struct stcStorms {float fMin; /**<Minimum mean severity*/
                    float fMax; /**<Maximum mean severity*/
                    int iTS; /**<Timestep of event*/
                    } *mp_stormsList; /**<List of scheduled storms - NULL if
                    no storms*/

  /**Function pointer for the appropriate RandomDraw function*/
  float (clStorm::*RandomDraw)(const float &fNumber);

  /**Timestep probability of the occurence of a storm of each return interval.
  * Array size is m_iNumSeverityClasses. These values are calculated in
  * CalculateStormProbabilities().*/
  float *mp_fStormProbabilities;

  /**Standard deviation for normal or lognormal probability distribution
  * functions.  This is only used if m_iStochasticity = stochastic and
  * m_iDistribution = lognormal or normal.  Value comes from the parameter
  * file.*/
  double m_fStdDev;

  /** SST periodicity (Sr) */
  double m_fSSTPeriod;

  /** Sine function d */
  double m_fSineD;

  /** Sine function f */
  double m_fSineF;

  /** Sine function g */
  double m_fSineG;

  /** Trend function slope (m) */
  double m_fTrendSlopeM;

  /** Trend function intercept (i) */
  double m_fTrendInterceptI;

  /**The total number of storm return intervals.  Hardcoded.*/
  int m_iNumSeverityClasses;

  /**Return code for the "1dmg_index" data member of the "Storm Damage" grid*/
  int m_i1DmgIndexCode;

  /**Return code for the "dmg_index" data member of the "Storm Damage" grid*/
  int m_iDmgIndexCode;

  /**Return code for the "stormtime" data member of the "Storm Damage" grid*/
  int m_iStormTimeCode;

  /**Return code for the "index" data member of the "Storm Susceptibility" grid*/
  int m_iSusceptIndexCode;

  /**How many scheduled storms are in mp_stormsList*/
  int m_iNumScheduledStorms;

  /**Enum for describing the damage pattern.  This controls the method used
  * to calculate final damage indexes.*/
  enum susceptibility {mapped,  /**<Susceptibility is entered via a map*/
                       uniform  /**<Susceptibility is uniform across the plot*/
                      }
  m_iSusceptibility; /**<Variable holding the susceptibility pattern value.
  Value comes from parameter file.  If the susceptibility is uniform, all
  locations have a susceptibility index of 1.  If it is mapped, the
  "Storm Susceptibility" grid holds each location's susceptibility between
  0 and 1.*/

  /**Enum for stochasticity.*/
  enum stochasticity {deterministic, /**<Deterministic damage index calculations.*/
                   stochastic     /**<Stochastic damage index calculations.*/
                  }
  m_iStochasticity; /**<Variable holding the stochasticity. Value comes from
  the parameter file.  This controls how the damage pattern is applied -
  whether the same damage index is applied to all cells or whether it is
  randomized for each.*/

  /**Enum for listing probability distribution functions.  Poisson is
  * not included because it creates integers only - not appropriate.*/
  enum distribution_func {lognormal, /**<Lognormal distribution*/
                       normal /**<Normal distribution*/
                      } m_iDistribution; /**<What probability distribution
                      function to use if the value in m_iDamagePattern is
                      "stochastic".  Value comes from the parameter file.*/

  /**
  * Does the grid setup fo the behavior.  Steps:
  * <ol>
  * <li>Check to see if m_iSusceptibility is "mapped".  If it is, get the
  * "Storm Susceptibility" grid and verify that it has been created.</li>
  * <li>Check for the existence of the "Storm Damage" grid.  If it has been
  * created in the parameter file, make sure the grid cell resolution matches
  * "Storm Susceptibility", if that grid exists.</li>
  * <li>If "Storm Damage" was not created in the parameter file, create it with
  * "Storm Susceptibility"'s grid cell resolution if that grid exists, or the
  * default grid cell resolution if it doesn't.</li>
  * <li>Call TimestepCleanup() to make sure that all values in "Storm Damage"
  * are 0.</li>
  * </ol>
  *
  * @throws modelErr if:
  * <ul>
  * <li>The damage pattern as read from the parameter file is "mapped" but there
  * is no map in the parameter file for the grid "Storm Susceptibility"</li>
  * <li>Either grid is in the parameter file but not set up correctly</li>
  * <li>There is grid cell resolution data in the parameter file for both
  * "Storm Damage" and "Storm Susceptibility" and they do not match</li>
  * </ul>
  */
  void DoGridSetup();

  /**
  * Calculates the annual probability of a storm of each severity class.
  * The probability of a storm of a given return interval is calculated as
  * 1/length of the return interval.  The return interval values should already
  * be in mp_fStormProbabilities, put there by ReadParFile().
  */
  void CalculateStormProbabilities();

  /**
  * Applies the damage for a storm of a given return interval.  The storm's
  * mean severity is calculated by taking a random draw on its severity
  * interval.  The damage is applied according to one of the following
  * scenarios:
  * <ul>
  * <li>If the storm stochasticity is deterministic and the susceptibility is
  * uniform, the value of the storm damage index is added to each cell of the
  * "Storm Damage" grid (up to a max of 1).</li>
  * <li>If the storm stochasticity is stochastic and the susceptibility is
  * uniform, for each cell in the "Storm Damage" grid, the value of the storm
  * damage index is fed as the mean to the appropriate random draw function in
  * the math library to get a new storm damage index; then this value is added
  * to the cell (up to a max of 1).</li>
  * <li>If the storm stochasticity is deterministic and the susceptibility is
  * mapped, the storm damage index is multiplied by the appropriate cell value
  * in the "Storm Susceptibility" grid and added to the cell of the "Storm
  * Damage" grid (up to a max of 1).</li>
  * <li>If the storm stochasticity is stochastic and the susceptibility is
  * mapped, for each cell in the "Storm Damage" grid, the value of the storm
  * damage index is fed as the mean to the appropriate random draw function in
  * the math library to get a new storm damage index.  This index is multiplied
  * by the appropriate cell value in the "Storm Susceptibility" grid and added
  * to the cell of the "Storm Damage" grid (up to a max of 1).</li>
  * </ul>
  * @param fMeanSeverity The mean severity to use for this storm.
  */
  void ApplyDamage(float fMeanSeverity);

  /**
  * Reads in parameters from the parameter file.
  * @throws modelErr if:
  * <ul>
  * <li>Any return interval is not greater than 0.</li>
  * <li>The susceptibility value is not recognized.</li>
  * <li>The stochasticity value is not recognized.</li>
  * <li>The stochasticity is "stochastic" and there is no value for
  * probability distribution function, or that value is not recognized.</li>
  * <li>The stochasticity is "stochastic", the probability distribution
  * function is "normal" or "lognormal", and there is no value for standard
  * deviation.</li>
  * </ul>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void ReadParFile(xercesc::DOMDocument *p_oDoc);

  /**
  * Sets the function pointer for a stochastic damage pattern.  If
  * m_iDamagePattern is not "stochastic", nothing happens.  If it is, this sets
  * the value for clStorm::*RandomDraw depending on the value in
  * m_iDistribution.
  */
  void SetStochFuncPointer();

  /**
   * Calculates mean storm severity for each cell.
   */
  void DoSeverityAverages();

  /**
   * Adjusts the time-since-last-storm counter held in the "stormtime"
   * data member of the "Storm Damage" grid.  If there was no storm this
   * timestep, it adds the number of years per timestep to the value.
   * If there was a storm, it sets the value to 0.
   */
  void AdjustTimeSinceLastStormCounter(bool bStormThisTimestep);

  /**
  * Resets all values in the "Storm Damage" grid to 0.
  */
  void PackageCleanup();

  /**
  * Performs a random draw on a normal distribution.  The standard deviation
  * is mp_fStdDev.
  * @param fMean Mean of the normal distribution.
  * @return Random number between 0 and 1.
  */
  inline float NormalDraw(const float &fMean) {
    float fVal = fMean + clModelMath::NormalRandomDraw(m_fStdDev);
    if (fVal < 0) fVal = 0;
    if (fVal > 1) fVal = 1;
    return fVal;
  };

  /**
  * Performs a random draw on a lognormal distribution.  The standard deviation
  * is mp_fStdDev.
  * @param fMean Mean of the lognormal distribution.
  * @return Random number between 0 and 1.
  */
  inline float LognormalDraw(const float &fMean) {
    return clModelMath::LognormalRandomDraw(fMean, m_fStdDev);
  };
};
//---------------------------------------------------------------------------
#endif // Storm_H
