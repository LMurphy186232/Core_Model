//---------------------------------------------------------------------------
// AggregatedMortality
//---------------------------------------------------------------------------
#if !defined(AggregatedMortality_H)
  #define AggregatedMortality_H

#include "MortalityBase.h"

class clTree;
class clTreePopulation;


/**
* Aggregated mortality version 1.0.
*
* This behavior kills trees in a way that is clumped in both time and space.
*
* Mortality occurs in discrete episodes, governed by a return interval
* specified by the user.  Each timestep, the probability of a mortality episode
* is T/RI, where T is the number of years per timestep and RI is the return
* interval in years.  At each timestep, a uniform random number is compared to
* this probability to decide whether or not an episode of mortality occurs. If
* not, no trees are killed by this behavior.
*
* When a mortality episode occurs, trees are killed in clumps of a size set
* by the user.  Each tree has an equal probability of being the first tree to
* die in a clump, and is evaluated individually.  The probability of mortality
* is the total mortality probability given by the user divided by the size
* of the clump.  This probability applies only to trees to which this behavior
* is applied.  Other trees are not killed by this behavior, either directly or
* as part of a clump.
*
* Trees are killed in clumps.  The number of trees in a clump can either be
* deterministic, meaning the clumps are always the same size, or it can be
* drawn from a negative binomial probability distribution.
*
* For each tree in the pool, the behavior uses a random number to see if it will
* start a clump.  If it will, the behavior kills it, then its closest
* neighbors to which the behavior applies, until it has killed the number of
* trees in the clump.  If the clump size is 1, then this behavior replicates
* the clStochasticMort method.
*
* The mortality rate is given as per year.  The per timestep amount to kill is
* 1-(1-AD)^X, where AD is the amount of damage  and X is the number of years
* per timestep.
*
* Because of the clumping thing, this chooses all its trees to kill in the
* PreMortCalcs function.
*
* There could be problems if there are other mortality behaviors that come
* before this one.  Not in a code way, but they might kill again trees this
* behavior has chosen to kill; so fewer than expected trees might die.
*
* The namestring for this class is "aggregatedmortshell".  The parameter file
* call string is "AggregatedMortality".
*
* This behavior is only descended from clMortalityBase in order to take
* advantage of the registration of the "dead" tree data member.
*
* Copyright 2011 Charles D. Canham
* @author Lora E. Murphy
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clAggregatedMortality : virtual public clMortalityBase {

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clAggregatedMortality(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clAggregatedMortality();

  /**
  * Does behavior setup.  Calls the following functions:
  * <ol>
  * <li>ReadParFile()</li>
  * <li>GetDeadCodes()</li>
  * <li>SetUpAppliesTo()</li>
  * </ol>
  * Then final calculations for mortality episode probability and amount of
  * damage are made.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs mortality.  A random number compared to the probability of a
  * mortality episode decides if an episode occurs.  If not, this exits.  If
  * so, this goes through all the trees to which it applies.  For each, if the
  * tree is not already "dead", then a random number decides whether it begins
  * a dead clump.  To find the trees in the clump, it searches in a circle of
  * radius 5 meters for neighbors. The trees eligible to die are killed in
  * order of distance until the number in the clump has been reached.  If not
  * enough trees are found in 5 meters, the circle is widened in 5 meter
  * increments until enough trees are found.
  *
  * @param p_oPop Tree population.
  */
  void PreMortCalcs( clTreePopulation *p_oPop );

  /** For testing purposes
   * @return Probability of mortality
   */
  float GetMortalityEpisodeProbability()
    {return m_fMortalityEpisodeProbability;};

  /** For testing purposes
   * @return Mortality probability
   */
  float GetTreeMortalityProb()
    {return m_fMortalityProb;};

  /**
  * Determines mortality for a tree.  This just returns the value in the
  * "dead" bool tree data member.
  *
  * @param p_oTree Tree being evaluated.
  * @param fDbh DBH of tree being evaluated.
  * @param iSpecies Species of tree being evaluated.
  */
  deadCode DoMort (clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /**Return codes for the "dead" bool tree data member.  Array index one is
  * sized total number of species by total number of life history
   * stages. */
  int **mp_iDeadCodes;

  /** Matrix for which tree types and species this behavior is applied.  Array
   * index is sized total number of species by total number of life history
   * stages. */
  bool **mp_bAppliesTo;

  /** Per-timestep probability of a mortality episode occuring, 0 - 1.  Read
   * from the parameter file as a return interval, then this value is
   * calculated as T/RI, where T is the number of years per timestep and RI is
   * the return interval in years.*/
  float m_fMortalityEpisodeProbability;

  /** Per-timestep proportion of trees to kill, 0 - 1.  Read from the parameter
   * file as an annual amount, then this value is calculated as (1-(1-AD)^X)/C,
   * where AD is the amount of damage, X is the number of years per
   * timestep, and C is the clump size.*/
  float m_fMortalityProb;

  /** Number of trees to kill together in a clump.  This is either a
   * deterministic value or the mean of the negative binomial probability
   * distribution from which to choose a value. */
  int m_iNumTreesToClump;

  /** Clumping parameter for the negative binomial distribution, if
   * applicable. */
  float m_fClumpingParameter;

  /** Total number of species.  Primarily for the destructor. */
  int m_iTotalNumSpecies;

  /** If true, the size of a single clump of trees to kill is deterministic.
   * If false, the size is chosen from a negative binomial probability
   * distribution. */
  bool m_bClumpSizeDeterministic;

  /**
  * Reads in parameters from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>The value for amount to cut is not between 0 and 1</li>
  * <li>The return interval is less than 0</li>
  * </ul>
  */
  void ReadParFile(xercesc::DOMDocument *p_oDoc);

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
   * Sets up the mp_bAppliesTo matrix.
   * @param p_oPop Tree population object.
   */
  void SetUpAppliesTo(clTreePopulation *p_oPop);

  /**
   * Tests a tree to see whether it is eligible for killing.  It must be
   * of a species and tree type to which this behavior is applied, and not
   * dead.
   * @param p_oTree Tree to test.
   * @return true if the tree is eligible; false if not.
   */
  bool IsValid(clTree *p_oTree);

  /**
   * Kills a single clump of trees.
   *
   * This function gets the coordinate of the first tree in the clump.  It
   * searches a circle of an ever-widening radius until it finds enough trees
   * to make a complete clump.  It then sorts the trees it finds in distance
   * order so that the trees closest to the target will be killed to make the
   * clump.
   *
   * @param p_oPop Tree population object.
   * @param p_oTree First tree in the clump.
   * @param iNumToKill Number of trees to kill.
   */
  void KillClump(clTreePopulation *p_oPop, clTree *p_oTree, int iNumToKill);
};
//---------------------------------------------------------------------------
#endif // AggregatedMortality_H
