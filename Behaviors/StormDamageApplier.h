//---------------------------------------------------------------------------
// StormDamageApplier
//---------------------------------------------------------------------------
#if !defined(StormDamageApplier_H)
  #define StormDamageApplier_H

#include "BehaviorBase.h"


class clGrid;

/**
* Storm Damage Applier - Version 1.1
*
* The damage applier manages the population of trees that are storm-damaged.
* It adds new trees to the damaged population after storms have occurred, and
* removes trees from the damaged population (back to the healthy population)
* by keeping track of the time they have been damaged until a maximum has been
* reached.  This behavior also keeps track of the damage category into which
* each tree falls (no damage, medium damage, and heavy damage).
*
* Whether storms have occurred is assessed by clStorm.  This behavior uses the
* values in the "Storm Damage" grid for storm severity, with 0 meaning that
* no storm has occurred.
*
* A given tree's probability of damage in a given damage category is
* <center><i>P(i) = exp(a<sub>i</sub> + b*c*DBH^d) / (1+exp(a<sub>i</sub> +
* b*c*DBH^d))</i></center>
* where:
* <ul>
* <li>i is the damage category</li>
* <li><i>a<sub>i</sub></i> is the storm damage intercept for that tree's
* species for that damage category</li>
* <li><i>b</i> is the storm intensity coefficient for that tree's species</li>
* <li><i>c</i> is the storm damage index between 0 and 1, which comes from the
* tree's grid cell in "Storm Damage"</li>
* <li><i>DBH</i> is the tree's DBH, in cm</li>
* <li><i>d</i> is the storm DBH coefficient for that tree's species</li>
* </ul>
*
* There is a minimum DBH for which to apply storms for each species.  Set this
* to zero if storms apply to all trees.
*
* This behavior will use a random number to determine what damage category a
* tree falls in.  If rand <= P(d0), it's undamaged.  If P(d0) < rand <= P(d1),
* it has medium damage.  If P(d1) < rand <= 1, it has heavy damage.
*
* If a tree is damaged, a flag is set with the damage category. Then a counter
* is set to count the time since damage.  When the counter reaches the number
* of years that it takes a damaged tree to heal, the tree is undamaged once
* again.
*
* If a live tree is already damaged and is damaged again (and survives again),
* it gets the most severe damage category that can apply to it and the damage
* counter is reset to the max time for tree healing again.
*
* This behavior adds one tree data member:  an int, "stm_dmg", which holds both
* the damage category and the amount of time left to heal.  The possible damage
* category values are 0, 1, and 2 (no damage, medium damage, full damage).  The
* value in stm_dmg will be x*1000 + count, where x is the damage category and
* count is the number of years since damage.  So a full-damaged tree with 5
* years on its counter has a stm_dmg value of 2005, and a medium-damaged tree
* with 8 years on its counter has a stm_dmg value of 1008.  If a tree is
* undamaged, the value will be 0.
*
* This behavior cannot be applied to seedlings.  It can be applied to snags but
* they will be ignored.  This is potentially a good tactic though, to allow
* clStormKiller to deal with snags.
*
* The call string for this is "StormDamageApplier".
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clStormDamageApplier : virtual public clBehaviorBase {

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager clSimManager object.
  */
  clStormDamageApplier(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clStormDamageApplier();

  /**
  * Registers the "stm_dmg" tree data member.
  * @throws modelErr if this behavior has been applied to seedlings.
  */
  void RegisterTreeDataMembers();

  /**
  * Does behavior setup.  It calls GetParameterFileData() to read in values
  * from the parameter file.  Then it calls FormatQueryString().  Then it gets
  * a pointer to the "Storm Damage" grid.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if the "Storm Damage" grid has not been created.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Applies storm damage to trees.  It uses the value in m_cQuery to get the
  * trees to which this behavior applies.  Then, for each tree, it will assess
  * its damage based on the damage equation.
  *
  * If a tree got no new damage but had existing damage, the healing time
  * counter is reset to 0.
  */
  void Action();

  protected:

  /**Pointer to the "Storm Damage" grid.  This grid is created by clStorm.*/
  clGrid *mp_oStormDamageGrid;

  /**String to pass to clTreePopulation::Find() in order to get the trees to
  * apply damage to.  This will instigate a species/type search for all the
  * species and types to which this behavior applies.*/
  char *m_cQuery;

  /**The minimum DBH, in cm, for trees that can be damaged by storms.  Array
  * size is total number of species.  From the parameter file.*/
  double *mp_fMinStormDBH;

  /**Storm damage intercept for medium damage (a).  Array size is total number
  * of species.  From the parameter file.*/
  double *mp_fStmDmgInterceptMed;

  /**Storm damage intercept for complete damage (a).  Array size is total
  * number of species.  From the parameter file.*/
  double *mp_fStmDmgInterceptFull;

  /**Storm intensity coefficient (b). Array size is total number of species.
  * From the parameter file.*/
  double *mp_fStmIntensityCoeff;

  /**Storm DBH coefficient (d). Array size is total number of species.  From
  * the parameter file.*/
  double *mp_fStmDBHCoeff;

  /**Return codes for the "stm_dmg" int tree data member.  This is the data
  * member that this behavior is adding.  Array index one is sized m_iNumTypes;
  * array index two is sized total number of species.*/
  int **mp_iStmDmgCodes;

  /**Number of years damaged trees stay damaged.  From the parameter file.*/
  int m_iNumYearsToHeal;

  /**Number of total types (despite the fact that this behavior won't deal
  * with all of them).*/
  int m_iNumTypes;

  /**Return code for the "1dmg_index" float data member of the "Storm Damage"
  * grid.*/
  int m_iDmgIndexCode;

  /**
  * Reads parameter file data.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if the value for m_iNumYearsToHeal is not greater than 0.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  */
  void FormatQueryString();
};
//---------------------------------------------------------------------------
#endif // StormDamageApplier_H
