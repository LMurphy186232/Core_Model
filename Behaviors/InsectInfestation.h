//---------------------------------------------------------------------------
// InsectInfestation
//---------------------------------------------------------------------------
#if !defined(InsectInfestation_H)
  #define InsectInfestation_H

#include "BehaviorBase.h"


/**
* Insect Infestation version 1.0.
*
* This behavior simulates a spreading insect infestation throughout the tree
* population.
*
* The proportion of trees infested as a lognormal function of time is as
* follows:
*
* P = I +((MAX-I)/(1+(T/X0)^Xb))
*
* where:
* <ul>
* <li>P is the proportion infested </li>
* <li>I is the function intercept, or the initial infestation rate </li>
* <li>MAX is the maximum infestation rate </li>
* <li>T is the time since the start of the infestation </li>
* <li>X0 is the time at which 50% of the population is infested </li>
* <li>Xb controls the steepness of the infestation curve </li>
* </ul>
*
* Infestation begins at a time step chosen by the user.  Infestation continues
* until there are no more infested trees in the plot.
*
* The proportion of trees infested at time T does not depend on additions to or
* subtractions from the tree population.  The number of trees of a species at
* that time is counted and the number of trees that are newly infested is the
* number required to cause the appropriate proportion to be infested.  If for
* some reason there are more trees infested than there should be at that time,
* no additional trees are infested.
*
* There is no spatial component to the selection of trees for infestation. It is
* assumed that all trees have an equal chance of becoming infested no matter
* where they are in the plot. The number of trees that should be infested for a
* given time step minus the number that actually are gives the probability that
* an uninfested tree will become infested this time step. A random number is
* used against this probability for each uninfested tree to determine whether or
* not it will become infested.
*
* An integer data member is added to trees that tracks the number of years that
* they have been infested.  This data member is called "YearsInfested".
*
* This behavior will not infest trees below a minimum DBH set by the user.
* Because of the need for DBH, obviously seedlings are ignored.
*
* This behavior's call string and name string are both "InsectInfestation".
*
* Copyright 2011  Charles D. Canham
* @author Lora E. Murphy
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clInsectInfestation : virtual public clBehaviorBase {

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clInsectInfestation(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clInsectInfestation();

  /**
  * Does behavior setup.  Calls the following functions:
  * <ol>
  * <li>ReadParFile()</li>
  * <li>FormatQueryString()</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Does insect infestation each timestep. If the timestep has not yet reached
  * the value in m_iFirstTimestep, or if the value in m_iYearsOfInfestation is
  * 0 (indicating an infestation is over), then nothing happens.
  *
  * The counter in m_iYearsOfInfestation is incremented and the target level
  * of infestation is calculated for each species according to the function
  * above. GetInfestationRate() calculates the actual current rate. The
  * difference between the target and actual rates gives the probability of an
  * uninfested tree becoming infested. Then for each tree to which this behavior
  * is applied, if the tree is not infested, a random number compared to the
  * infestation probability determines if it will become infested. If so, a
  * value of 1 is entered into the "YearsInfested" data member. If the tree was
  * already infested, the data member value is incremented.
  */
  void Action();

  /**
  * Registers the "YearsInfested" int data member.  The return codes are
  * captured in the mp_iDataCodes array.
  */
  void RegisterTreeDataMembers();

  protected:

  /**Return codes for the "YearsInfested" int tree data member.  Array index
   * one is sized m_iTotalNumSpecies; array index two is sized number of total
   * types.*/
  short int **mp_iDataCodes;

  /** Intercept parameter - rate of infestation at time 1.  Array size is
   * total # species. */
  float *mp_fIntercept;

  /** Maximum infestation rate for each species.  Array size is total #
   * species. */
  float *mp_fMax;

  /** The "X0" parameter for each species.  Array size is total # species. */
  float *mp_fX0;

  /** The "Xb" parameter for each species.  Array size is total # species. */
  float *mp_fXb;

  /** Minimum DBH for trees to be infested.  Array size is total # species. */
  float *mp_fMinDBH;

  /**String to pass to clTreePopulation::Find() in order to get the trees to
  * apply damage to.  This will instigate a species/type search for all the
  * species and types to which this behavior applies.*/
  char *m_cQuery;

  /** Timestep to begin infestation */
  int m_iFirstTimestep;

  /** Years since infestation began - 0 if there is no current infestation */
  int m_iYearsOfInfestation;

  /** Total number of species.  Primarily for the destructor. */
  int m_iTotalNumSpecies;

  /**
  * Reads in parameters from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @param p_oPop Tree population object.
  * @throws modelErr if:
  * <ul>
  * <li>A value in the minimum DBH is less than 0</li>
  * <li>The timestep to start infestation is less than 0</li>
  * <li>The maximum infestation rate for any species is not between 0 and 1</li>
  * <li>The infestation intercept for any species is not between 0 and 1</li>
  * <li>The value for X0 for any species is 0</li>
  * </ul>
  */
  void ReadParFile(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  * @param p_oPop Tree population object.
  */
  void FormatQueryString(clTreePopulation *p_oPop);

  /**
   * Gets the current rate of infestation in the tree population.  This counts
   * the total number of trees of each species eligible for infestation, and the
   * number of those that are currently infested.
   * @param p_iTotalTrees Pointer to an array, sized m_iTotalNumSpecies, into
   * which to put the total number of trees found of each species.
   * @param p_iInfTrees Pointer to an array, sized m_iTotalNumSpecies, into
   * which to put the infested number of trees found of each species.
   */
  void GetInfestationRate(long *p_iTotalTrees, long *p_iInfTrees);

};
//---------------------------------------------------------------------------
#endif // InsectInfestation_H
