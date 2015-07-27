//---------------------------------------------------------------------------
// DensDepInfestation
//---------------------------------------------------------------------------
#if !defined(DensDepInfestation_H)
  #define DensDepInfestation_H

#include "BehaviorBase.h"


/**
* Density Dependent Infestation
*
* This behavior simulates the spread of a disease or infestation throughout a
* tree population.
*
* The proportion of trees infested as a Gompertz function of time is as
* follows:
*
* P = max * exp(-exp(a - b* T))
*
* where:
* <ul>
* <li>P is the proportion infested </li>
* <li>max is the maximum infestation rate </li>
* <li>T is the time since the start of the infestation, in years</li>
* <li>a is a parameter</li>
* <li>b is calculated, as below</li>
* </ul>
*
* b is calculated as follows:
*
* b = bx * BA + by
*
* where BA is the proportion of basal area comprised of trees to which this
* behavior has been assigned (0 - 1).
*
* This behavior allows for the possibility that there are various levels of
* resistance in the tree population. Users can assign a percentage of trees
* that are "resistant" or "conditionally susceptible". This behavior will not
* infest "resistant" trees but otherwise does nothing with this information -
* other behaviors are planned that take advantage of this.
*
* All species are treated as a common pool and there are no species-specific
* parameters for infestation. There are species-specific parameters for the
* rates of resistance and conditional susceptibility.
*
* The proportion of trees infested at time T does not depend on additions to or
* subtractions from the tree population.  The number of trees of a species at
* that time is counted and the number of trees that are newly infested is the
* number required to cause the appropriate proportion to be infested.  If for
* some reason there are more trees infested than there should be at that time,
* no additional trees are infested.
*
* Susceptible individuals come in two size classes, with the cutoff DBH set as
* a parameter. All the individuals above this DBH are infected before any below
* the cutoff are. Note that when max < 1, it is possible that no individuals
* below the DBH cutoff will ever be infected.
*
* There is no spatial component to the selection of trees for infestation. It is
* assumed that all trees have an equal chance of becoming infested no matter
* where they are in the plot. The number of trees that should be infested for a
* given time step minus the number that actually are gives the probability that
* an uninfested tree will become infested this time step. A random number is
* used against this probability for each uninfested tree to determine whether or
* not it will become infested.
*
* Infestation begins at a year specified by the user. This can be negative,
* indicating that the infestation began before the beginning of the run. In
* this case, the behavior setup before the run will include the step of
* determining the number of infested initial conditions trees and their length
* of infestation.
*
* Two integer data member is added to trees. The first is called "YearsInfested"
* and tracks the number of years that a tree has been infested. The second is
* called "DensDepResistanceStatus" and equals a value in the enum densDepResistanceStatus
* in DataTypes.h.
*
* This behavior will not infest trees below a minimum DBH set by the user.
* Because of the need for DBH, obviously seedlings are ignored.
*
* This behavior's call string and name string are both "DensDepInfestation".
*
* Copyright 2013  Charles D. Canham
* @author Lora E. Murphy
* <br>Edit history:
* <br>-----------------
* <br>August 5, 2013 - Created (LEM)
* <br>July 27, 2015 - Added infection end date (LEM)
*/
class clDensDepInfestation : virtual public clBehaviorBase {

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clDensDepInfestation(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clDensDepInfestation();

  /**
  * Does behavior setup.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Does insect infestation each timestep.
  *
  * TreeInventory() collects data about the tree population. This is used to
  * calculate first the b parameter and then the proportion of trees infested.
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
  * Registers the "YearsInfested" and "DensDepResistanceStatus" int data members.
  */
  void RegisterTreeDataMembers();

  protected:

  /**Return codes for the "YearsInfested" int tree data member.  Array index
   * one is sized m_iTotalNumSpecies; array index two is sized number of total
   * types.*/
  short int **mp_iYearsInfestedCodes;

  /**Return codes for the "DensDepResistanceStatus" int tree data member. Array
   * index one is sized m_iTotalNumSpecies; array index two is sized number of
   * total types.*/
  short int **mp_iDensDepResistanceStatusCodes;

  /** Minimum DBH for trees to be infested. Array size is total # species. */
  float *mp_fMinDBH;

  /** Cohort cutoff DBH. Trees above this size are infested before trees below
   * it. Array size is total # species. */
  float *mp_fCohortDBH;

  /** Probability that a tree is resistant, 0-1. Array size is total #
   * species. */
  float *mp_fProbResistant;

  /** Probability that a tree is conditionally susceptible, 0-1. Array size is
   * total # species. */
  float *mp_fProbConditionallySusceptible;

  /** Maximum infestation rate parameter. */
  float m_fMax;

  /** The "a" parameter. */
  float m_fA;

  /** The "bx" parameter. */
  float m_fBx;

  /** The "by" parameter. */
  float m_fBy;

  /** Total plot BA. */
  double m_fPlotBA;

  /** BA in the pool of trees to which this behavior applies (including
   * resistant trees). */
  double m_fPoolBA;

  /** Current probability of infestation for the smaller cohort */
  double m_fSmallCohortProb;

  /** Current probability of infestation for the larger cohort */
  double m_fLargeCohortProb;

  /** Number of trees in the pool to which this behavior applies (including
   * resistant trees), below the cohort cutoff */
  long m_iNumSmallCohortPoolTrees;

  /** Number of trees in the pool to which this behavior applies (including
   * resistant trees), equal to or above the cohort cutoff */
  long m_iNumLargeCohortPoolTrees;

  /** Number of infested trees in the pool to which this behavior applies
   * (including resistant trees), below the cohort cutoff */
  long m_iNumSmallCohortInfestedTrees;

  /** Number of infested trees in the pool to which this behavior applies
   * (including resistant trees), equal to or above the cohort cutoff */
  long m_iNumLargeCohortInfestedTrees;

  /** Number of trees that can be infested (non-resistant, regardless of current
   * infestation status), below the cohort cutoff */
  long m_iNumSmallInfestibleTrees;

  /** Number of trees that can be infested (non-resistant, regardless of current
   * infestation status), equal to or above the cohort cutoff */
  long m_iNumLargeInfestibleTrees;

  /** Timestep to begin infestation */
  int m_iFirstTimestep;

  /** Timestep to end infestation */
  int m_iLastTimestep;

  /** Years since infestation began - 0 if there is no current infestation */
  int m_iYearsOfInfestation;

  /**String to pass to clTreePopulation::Find() in order to get the trees to
  * apply damage to.  This will instigate a species/type search for all the
  * species and types to which this behavior applies.*/
  char *m_cQuery;

  /** Total number of species. Primarily for the destructor. */
  int m_iTotalNumSpecies;

  /**
  * Reads in parameters from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @param p_oPop Tree population object.
  * @throws modelErr if:
  * <ul>
  * <li>A value in the minimum DBH is less than 0</li>
  * <li>The maximum infestation rate is not between 0 and 1</li>
  * <li>A value in the cohort cutoff DBH is less than 0</li>
  * <li>The probability of resistance is not between 0 and 1</li>
  * <li>The probability of conditional susceptibility is not between 0 and 1</li>
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
   * This traverses the tree population and does a few things:
   * <ul>
   * <li>Gets the total plot BA and puts it in m_fPlotBA</li>
   * <li>Gets the BA of the pool of trees to which this behavior applies
   * (all trees above minimum infestible DBH, regardless of resistance
   * status) and puts it in m_fPoolBA</li>
   * <li>Counts the number of trees in the pool to which this behavior applies
   * and puts it in m_iNumSmallCohortPoolTrees and m_iNumLargeCohortPoolTrees</li>
   * <li>Checks for new trees with no current infestation status, and
   * assigns them a status</li>
   * <li>Counts currently infested trees and puts it in
   * m_iNumSmallCohortInfestedTrees and m_iNumLargeCohortInfestedTrees</li>
   * <li>Counts the number of infestible trees (not resistant), regardless
   * of infestation status, and puts it in m_iNumSmallInfestibleTrees and
   * m_iNumLargeInfestibleTrees</li>
   * </ul>
   */
  void TreeInventory();

  /**
   * Assigns infestation status and length to initial conditions trees in the
   * case that infestation starts before the beginning of the run.
   */
  void InfestInitialConditionsTrees();

  /**
   * Calculates each cohort's probability of infestation for individual trees,
   * based on the results of TreeInventory(). First, the desired number of
   * new trees to infest is calculated. If it is 0, both probabilities are also
   * 0. If there are enough infestible trees in the larger cohort to satisfy
   * the new number, then the smaller cohort's probability is set to 0 and the
   * larger cohort's probability is the number of new trees to infest divided
   * by the number of non-infested trees. If there are not enough trees in the
   * larger cohort, its probability is set to 1, and the smaller cohort's
   * probability is set to the number of new trees to infest minus infestible
   * trees in the larger cohort divided by number of non-infested small cohort
   * trees.
   *
   * Results are placed in m_fSmallCohortProb and m_fLargeCohortProb.
   *
   * @param fTargetRate Total current proportion of trees to be infested.
   */
  void DetermineCohortInfestationProbability(double fTargetRate);

  /**
   * Ends infestation. All flags are reset to 0 and m_iYearsOfInfestation is
   * set to 0 which will make sure no trees are infected in the future.
   */
  void EndInfestation();

};
//---------------------------------------------------------------------------
#endif // DensDepInfestation_H
