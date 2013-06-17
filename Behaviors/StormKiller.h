//---------------------------------------------------------------------------
// StormKiller
//---------------------------------------------------------------------------
#if !defined(StormKiller_H)
  #define StormKiller_H

#include "BehaviorBase.h"


class clGrid;

/**
* Storm Killer - Version 1.0
*
* The storm killer kills trees that are storm-damaged.
*
* Whether storms have occurred is assessed by clStorm.  This behavior uses the
* values in the "Storm Damage" grid for storm severity, with 0 meaning that
* no storm has occurred.  Damage as a result of the storm is assessed by
* clStormDamageApplier.  Damage is either none, medium, or heavy.
*
* If a tree has medium or heavy damage, its probability of survival is:
* <center><i>P(i) = exp(a<sub>i</sub> + b<sub>i</sub> * DBH) / (1 + exp(a<sub>i</sub> + b<sub>i</sub> * DBH))</i></center>
* where:
* <ul>
* <li><i>a<sub>i</sub></i> and b<sub>i</sub> are parameters for that tree's
* species for that damage category</li>
* <li><i>DBH</i> is the tree's DBH, in cm</li>
* </ul>
*
* There is a minimum DBH for which to apply storms for each species.  Set this
* to zero if storms apply to all trees.
*
* Once the probability for survival has been assessed, this behavior uses a
* random number to determine whether or not the tree actually dies.  Dead
* heavy damaged trees have an additional probability of becoming a tip-up.
* Damaged trees can only die the timestep the storm occurs.
*
* If this behavior is applied to snags, then dead non-tip-ups become snags.
* This behavior sets a counter which holds time-since-damage and the damage
* level that killed them (even if they already had higher damage as an alive
* tree). After the value in the snag lifetime parameter, these snags are
* removed. Snags not created by this behavior are not touched.
*
* If this behavior is not applied to snags, tip-ups become snags whose "dead"
* flag is set to true. (This flag comes from mortality behaviors and is not
* added by this behavior.) This behavior then has nothing more to do with these
* trees.
*
* If this behavior is not applied to snags, then all trees that die get their
* "dead" flags set to true. (This flag comes from mortality behaviors and is
* not added by this behavior.)  This behavior then has nothing more to do with
* these trees.
*
* This behavior cannot be applied to seedlings.  The clStormDamageApplier
* behavior is required.
*
* The namestring and parameter file call string for this is "StormKiller".
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clStormKiller : virtual public clBehaviorBase {

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager clSimManager object.
  */
  clStormKiller(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clStormKiller();

  /**
  * Does behavior setup.
  * <ol>
  * <li>Calls GetParameterFileData() to read in values from the parameter
  * file.</li>
  * <li>Calls FormatQueryString().</li>
  * <li>Calls GetStmDmgCodes() to get the appropriate tree data member codes.</li>
  * </ol>
  * @param p_oDoc Parsed parameter file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Kills storm-damaged trees.  It uses the value in m_cQuery to get the
  * trees to which this behavior applies.  Then, for each tree, it will assess
  * its probability of survival based on its damage and randomly pick whether
  * or not to kill it.  Trees with heavy damage are assessed for the
  * possibility of tip-up by comparing another random number to mp_fPropTipUp.
  *
  * If we have values of -1 for stm_dmg for snags, then all dead are "killed"
  * by getting their "dead" tree data member set to true.  Otherwise, they are
  * passed as killed with a code of "storm" to the tree population.  If they
  * were tip-up, then their "dead" tree data member is set to true.
  */
  void Action();

  protected:

  /**String to pass to clTreePopulation::Find() in order to get the trees to
  * apply damage to.  This will instigate a species/type search for all the
  * species and types to which this behavior applies.*/
  char *m_cQuery;

  /**The minimum DBH, in cm, for trees that can be damaged by storms.  Array
  * size is total number of species.  From the parameter file.*/
  float *mp_fMinStormDBH;

  /**Medium damage a.  Array size is total number of species.  From the
  * parameter file.*/
  float *mp_fStmDmgMedA;

  /**Heavy damage a.  Array size is total number of species.  From the
  * parameter file.*/
  float *mp_fStmDmgHeavyA;

  /**Medium damage b.  Array size is total number of species.  From the
  * parameter file.*/
  float *mp_fStmDmgMedB;

  /**Heavy damage b.  Array size is total number of species.  From the
  * parameter file.*/
  float *mp_fStmDmgHeavyB;

  /**Proportion of dead heavy-damaged trees that tip-up, between 0 and 1.
  * Array size is total number of species.  From the parameter file.*/
  float *mp_fPropTipUp;

  /**Return codes for the "stm_dmg" int tree data member.  Array index one is
  * sized m_iNumTypes; array index two is sized total number of species.*/
  int **mp_iStmDmgCodes;

  /**Return codes for the "dead" bool tree data member.  Array index one is
  * sized m_iNumTypes; array index two is sized total number of species.*/
  int **mp_iDeadCodes;

  /**Number of years snags stick around.  From the parameter file.*/
  int m_iSnagYears;

  /**Number of total types (despite the fact that this behavior won't deal
  * with all of them).*/
  int m_iNumTypes;

  /**
  * Reads parameter file data.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if:
  * <ul>
  * <li>The value for m_iSnagYears is not greater than or equal to 0</li>
  * <li>The value in proportion that become tip-ups is not between 0 and 1</li>
  * </ul>
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  */
  void FormatQueryString();

  /**
  * Gets codes for the "dead" data member for each tree type to which this
  * behavior applies.
  * @param p_oPop Tree population object.
  * @throws modelErr if the codes are not available for every tree type to
  * which this behavior is applied.
  */
  void GetDeadCodes(clTreePopulation *p_oPop);

  /**
  * Gets codes for the "stm_dmg" data member for each tree type to which this
  * behavior applies.
  * @throws modelErr if the codes are not available for every tree type to
  * which this behavior is applied, or if this behavior is being applied to
  * seedlings.
  */
  void GetStmDmgCodes();
};
//---------------------------------------------------------------------------
#endif // StormKiller_H
