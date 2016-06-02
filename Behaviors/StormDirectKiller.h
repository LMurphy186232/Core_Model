//---------------------------------------------------------------------------
// StormDirectKiller
//---------------------------------------------------------------------------
#if !defined(StormDirectKiller_H)
  #define StormDirectKiller_H

#include "BehaviorBase.h"


class clGrid;

/**
* Storm Direct Killer - Version 1.0
*
* This kills trees directly after a storm based solely on the damage at their
* location. Whether storms have occurred is assessed by clStorm.  This behavior
* uses the values in the "Storm Damage" grid for storm severity, with 0 meaning
* that no storm has occurred.
*
* A given tree's probability of mortality is
* <center><i>P = exp(a + b * s) / (1 + exp(a + b * s)))</i></center>
* where:
* <ul>
* <li><i>a</i> and <i>b</i> are parameters</li>
* <li><i>s</i> is the storm damage index between 0 and 1, which comes from the
* tree's grid cell in "Storm Damage"</li>
* </ul>
*
* This behavior will use a random number to determine whether a tree dies based
* on its probability of mortality. A tree that dies gets its "dead" flag set
* to "storm". (This flag comes from mortality behaviors and is not added by this
* behavior.)
*
* The call string for this is "StormDirectKiller".
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clStormDirectKiller : virtual public clBehaviorBase {

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager clSimManager object.
  */
  clStormDirectKiller(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clStormDirectKiller();

  /**
  * Does behavior setup. Calls:
  * <ol>
  * <li>GetParameterFileData()</li>
  * <li>FormatQueryString()</li>
  * <li>GetDeadCodes()</li>
  * <li>GetGridInfo()</li>
  * </ol>
  * @param p_oDoc Parsed parameter file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Applies storm damage to trees. It uses the value in m_cQuery to get the
  * trees to which this behavior applies. Then, for each tree, it will assess
  * its probability of mortality for each storm that occurred in the current
  * timestep. Random numbers are used to determine which trees die. Those that
  * die get their "dead" flag set to "storm".
  */
  void Action();

  protected:

  /**Pointer to the "Storm Damage" grid.  This grid is created by clStorm.*/
  clGrid *mp_oStormDamageGrid;

  /**String to pass to clTreePopulation::Find() in order to get the trees to
  * apply damage to.  This will instigate a species/type search for all the
  * species and types to which this behavior applies.*/
  char *m_cQuery;

  /**Storm "a" parameter. Array size is total number of species. From the
   * parameter file.*/
  double *mp_fA;

  /**Storm "b" parameter. Array size is total number of species. From the
   * parameter file.*/
  double *mp_fB;

  /**Return codes for the "dead" bool tree data member.  Array index one is
  * sized m_iNumTypes; array index two is sized total number of species.*/
  int **mp_iDeadCodes;

  /**Number of total types.*/
  int m_iNumTypes;

  /**Return code for the "1dmg_index" float data member of the "Storm Damage"
  * grid.*/
  int m_iDmgIndexCode;

  /**
  * Reads parameter file data.
  * @param p_oDoc Parsed parameter file.
  * @param p_oPop Tree population object.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

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
   * Gets the pointer to the "Storm Damage" grid and its appropriate data
   * members.
   * @throws modelErr if the "Storm Damage" grid has not been created.
   */
  void GetGridInfo();
};
//---------------------------------------------------------------------------
#endif // StormDirectKiller_H
