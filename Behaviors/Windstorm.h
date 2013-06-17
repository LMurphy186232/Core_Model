//---------------------------------------------------------------------------
// Windstorm
//---------------------------------------------------------------------------
#if !defined(Windstorm_H)
  #define Windstorm_H

#include "BehaviorBase.h"

class clGrid;

/**
* Windstorm version 2.0.
*
* Windstorm creates storm events and kills trees as a result.
*
* Windstorm defines 11 different storm return intervals: 1, 5, 10, 20, 40,
* 80, 160, 320, 640, 1280, and 2560. (This code was present in pre-6.0
* versions of SORTIE as well.  At that point, it did not have the 1-year
* return interval. I added it for testing purposes, and thought others might
* want access to it as well.) For each return interval, the user provides
* a storm severity, from 0 (no damage) to 1 (complete devastation).
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
* <li>x = 4*t/Sr, where t = timestep since storms started and Sr = sine
* periodicity parameter</li>
* <li>d is a parameter that controls the sine curve's amplitude</li>
* <li>f is a parameter that controls the sine curve's frequency</li>
* <li>g is a parameter that controls where on the curve we are when storms
* start occuring</li>
* <li>m is a parameter that gives the trend line's slope</li>
* <li>i is a parameter that gives the trend line's intercept</li>
* </ul>
*
* Each timestep, this behavior decides what storms will occur.  The probability
* for each storm is calculated as in the equation above.  SORTIE uses
* independent random number draws for each year of the timestep for each
* return interval.  This means multiple storms can happen per timestep, and if
* the timestep length is more than one year, multiple storms of the same
* severity can happen.
*
* A tree's probability of dying in a given storm is:
*
* <center>mort = exp(a + c * s * DBH<sup> b</sup>)/(1 + exp(a + c * s * DBH<sup> b</sup>))</center>
*
* where:
* <ul>
* <li>mort is the tree's probability of death from the storm</li>
* <li>a, b, and d are parameters</li>
* <li>DBH is the tree's DBH in cm</li>
* <li>s is the severity for storms of this return interval</li>
* </ul>
*
* Each storm gets a crack at all the trees "independently" of the others.  This
* means that, if two storms happen in one timestep, this behavior will cycle
* twice through all the trees in the plot and assess each one's mortality for
* the two storms separately.  Of course, the two events cannot be truly
* independent, because the second storm can only kill trees not killed by the
* first storm.
*
* Storms of severity below a certain minimum (currently set at 0.1) do not use
* the equation above; the storm severity is used directly as a mortality
* probability for trees.
*
* Storms of severity 0 cannot occur.  Any return interval with a severity of
* 0 is skipped.
*
* The user has the option to not start storms at the beginning of the run.
* They can set a timestep for storms to start.  Before this timestep no
* storms are allowed to occur.
*
* This behavior will not kill trees below a minimum DBH set by the user.
* Because of the need for DBH, obviously seedlings are ignored. Snags and
* already-dead trees are ignored as well.  All trees that die get their
* "dead" flags set to "storm". (This flag comes from mortality behaviors and is
* not added by this behavior.)  This behavior then has nothing more to do with
* these trees.
*
* Storm results are placed in a grid called "Windstorm Results".
*
* This behavior's call string and name string are both "Windstorm".
*
* Copyright 2011 Charles D. Canham
* @author Lora E. Murphy
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clWindstorm : virtual public clBehaviorBase {

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clWindstorm(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clWindstorm();

  /**
  * Does behavior setup.  Calls the following functions:
  * <ol>
  * <li>ReadParFile()</li>
  * <li>DoGridSetup()</li>
  * <li>GetDeadCodes()</li>
  * <li>FormatQueryString()</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Does windstorms each timestep.  If the timestep has not yet reached the
  * value in m_iFirstStormTimestep, then nothing happens.  Otherwise: first,
  * the "Windstorm Results" grid is cleared.  Then the cyclical portion of
  * storm probability is calculated.  Then, a random number is drawn
  * for each year of the timestep for each storm return interval.  If the
  * random number is less than the probability of that storm, then that
  * storm occurs.  For each storm that occurs, this will fetch all trees that
  * can be killed in storms, and each that is not already dead has its
  * probability of dying in the current storm calculated.  A random number is
  * compared to this probability to see if the tree dies. Those trees that die
  * have their "dead" flag set to "storm".
  */
  void Action();

  protected:

  /**
  * Windstorm Results grid
  * This grid is named "Windstorm Results" and it contains the data for all
  * storms that happened in the previous timestep.  There is one grid cell per
  * plot, and this is not user-changeable.
  *
  * Each storm gets one package.  The packages occur in the order than the
  * storms occurred in the timestep.  Each package contains the following data
  * members:  One float called "severity", which holds the severity of the
  * storm as a value from 0 to 1.  Then there is one float per species called
  * "ba_x", where x is the species number.  This holds the amount of basal area
  * per hectare killed in the storm for that species.  Then there is another
  * float per species called "density_x", where x is the species number.  This
  * holds the density per hectare killed in the storm for that species.
  */
  clGrid *mp_oResultsGrid;

  /**Return codes for the "dead" bool tree data member.  Array index one is
  * sized m_iTotalNumSpecies; array index two is sized 2 (for saplings and
  * adults).*/
  int **mp_iDeadCodes;

  /**
  * Annual probability of the occurence of a storm of each return interval.  If
  * the user has set the storm severity of a given return interval to 0, its
  * probability is also set to 0. Array size is m_iNumReturnIntervals.*/
  float *mp_fStormProbabilities;

  /** Severity of each storm return interval.  Array size is
  m_iNumReturnIntervals. */
  float *mp_fStormSeverities;

  /** Storm intercept for tree mortality (a) parameter.  Array size is
   * total # species. */
  float *mp_fA;

  /** The "b" parameter for each species.  Array size is total # species. */
  float *mp_fB;

  /** The "d" parameter for each species.  Array size is total # species. */
  float *mp_fC;

  /** Minimum DBH for trees to be killed in storms.  Array size is total #
   * species. */
  float *mp_fMinDBH;

  /** Return codes for the "ba_x" package float data members of the "Windstorm
   * Results" grid.  Array size is total # species.*/
  short int *mp_iBa_xCodes;

  /** Return codes for the "density_x" package float data members of the
   * "Windstorm Results" grid.  Array size is total # species.*/
  short int *mp_iDensity_xCodes;

  /**String to pass to clTreePopulation::Find() in order to get the trees to
  * apply damage to.  This will instigate a species/type search for all the
  * species and types to which this behavior applies.*/
  char *m_cQuery;

  /** SST periodicity (Sr) */
  float m_fSSTPeriod;

  /** Sine function d */
  float m_fSineD;

  /** Sine function f */
  float m_fSineF;

  /** Sine function g */
  float m_fSineG;

  /** Trend function slope (m) */
  float m_fTrendSlopeM;

  /** Trend function intercept (i) */
  float m_fTrendInterceptI;

  /**The total number of storm return intervals.  Hardcoded at 11.*/
  int m_iNumReturnIntervals;

  /** The timestep to start storms. */
  int m_iFirstStormTimestep;

  /** Total number of species.  Primarily for the destructor. */
  int m_iTotalNumSpecies;

  /**Return code for the "severity" package float data member of the
   * "Windstorm Results" grid*/
  int m_iSeverityCode;

  /**
  * Sets up the "Windstorm Results" grid.  Any maps in the parameter file are
  * ignored.
  */
  void DoGridSetup();

  /**
  * Reads in parameters from the parameter file.  This also calculates baseline
  * storm probabilities.
  * @param p_oDoc DOM tree of parsed input file.
  * @param p_oPop Tree population object.
  * @throws modelErr if:
  * <ul>
  * <li>A value in the minimum DBH is less than 0</li>
  * <li>The timestep to start storms is less than 0</li>
  * <li>The storm severity for any return interval is not between 0 and 1</li>
  * <li>The storm periodicity is 0</li>
  * </ul>
  */
  void ReadParFile(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

  /**
  * Gets codes for the "dead" data member for each tree type to which this
  * behavior applies.
  * @param p_oPop Tree population object.
  * @throws modelErr if the codes are not available for every tree type to
  * which this behavior is applied, or if this behavior is applied to
  * seedlings.
  */
  void GetDeadCodes(clTreePopulation *p_oPop);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  * @param p_oPop Tree population object.
  */
  void FormatQueryString(clTreePopulation *p_oPop);

};
//---------------------------------------------------------------------------
#endif // Windstorm_H
