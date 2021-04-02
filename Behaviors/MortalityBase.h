//---------------------------------------------------------------------------

#ifndef MortalityBaseH
#define MortalityBaseH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"

class clTree;
class clMortalityOrg;


using namespace whyDead;

/**
* Mortality base - Version 1.1
*
* This is the base class for mortality behavior shell classes.  These shells
* each evaluate different kinds mortality for a tree.
*
* The bulk of the organizational work is done by the class clMortalityOrg,
* which calls the needed shell functions for each tree.  The clMortalityOrg
* object hooks into one particular growth shell object, which calls
* clMortalityOrg when it is triggered by the behavior manager.  It does not
* particularly matter which shell object is hooked; the first one will suffice.
* Any non-hooked shells do nothing when their Action() and other functions are
* called.
*
* Every object made from this class or a descendent class must have the string
* "mortshell" in its namestring somewhere.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>April 28, 2004 - Submitted as beta (LEM)
* <br>April 29, 2004 - Updated to version 1.1 by adding a new mortality tree
*                      data member code for a death flag
* <br>July 21, 2004 - Fixed a bug in data member registrations (LEM)
* <br>April 5, 2005 - Added a pre-calculation stage for the sake of NCI (LEM)
* <br>February 8, 2008 - Changed mortality flags from simple booleans to reason
* codes (LEM)
*/
class clMortalityBase : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  friend class clMortalityOrg;

  public:

  /**
  * Constructor. Checks to see if the mortality org object has been created -
  * if not, it creates it.
  * @param p_oSimManager Sim Manager object.
  */
  clMortalityBase(clSimManager *p_oSimManager);

  /**
  * Destructor.
  * Deletes the growth org object if it was the hooked object.
  */
  virtual ~clMortalityBase();

  /**
  * Performs mortality calculations.
  * This will be the same for all descendent classes - they do not need to
  * override. If a particular object is hooked, it calls the growth org
  * object's DoGrowthAssignments.  Otherwise it does nothing.
  */
  void Action();

  /**
  * Determines whether or not a tree dies.  This must be overridden by all
  * child classes.
  *
  * @param p_oTree Tree being evaluated.  It should not be modified, but will
  * be available in case extra data is needed.
  * @param fDbh DBH of tree being evaluated - for seedlings will be 0
  * @param iSpecies Species of the tree being evaluated
  * @return Why the tree is dead (including the code meaning "not dead").
  */
  virtual deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies) = 0;

  /**
  * Gets the mortality org object.
  * @return Mortality org object.
  */
  clMortalityOrg* GetMortOrg() {return mp_oMortalityOrg;};

  /**
  * Triggers the mortality data member registrations.  If this is the hooked
  * mortality object, then clMortalityOrg::DoDataMemberRegistrations is
  * called.
  */
  void RegisterTreeDataMembers();

  /**
  * Overridden from clBehaviorBase.  We want to register one int data member
  * for each species/type combo that is applied to mortality.  If this is a
  * hooked object, this calls clMortalityOrg::UpdateDataMemberRegistrations().
  * Hooked or not, it then returns m_iNewTreeInts as usual.
  * @return Number of new tree ints, or zero if no new members are to be
  * registered.
  */
  short int GetNewTreeInts();

  /**
  * Whether or not a species/type combo uses this mortality object.
  *
  * @param iSp Species.
  * @param iTp Tree type.
  * @return True if this combo uses this mortality object, false if not.
  */
  bool UsesThisMortality(short int iSp, short int iTp)
      {return mp_bUsesThisMortality[iSp][iTp];};

  /**
  * Performs calculations before any trees have been killed.  Override this
  * function to perform some processing before any mortality has been
  * applied.  There are no guarantees as to what order the behaviors will be
  * called for this function, but it is guaranteed that no trees will have been
  * killed for mortality.  To make this guarantee worth something, DO NOT
  * KILL ANY TREES in this function!
  */
  virtual void PreMortCalcs( clTreePopulation *p_oPop ){;};

  protected:

  static clMortalityOrg *mp_oMortalityOrg; /**<clMortalityOrg object.  This
             pointer is held in common by all shells.*/

  short int m_iNumTotalSpecies; /**<Keep a copy for the constructor.*/
  bool m_bHooked; /**<Whether or not this shell object is hooked to
             clMortalityOrg. clMortalityOrg will set this flag.*/

  bool **mp_bUsesThisMortality; /**<2D array, species by type, of whether or not
                 this mortality shell applies to a given species/type combo.*/

  /**
  * Triggers all mortality setup.
  * This will be the same for all descendent classes.  If a particular object
  * is hooked, it calls the growth org object's DoSetup() function, which calls
  * the function DoShellSetup() - if a descendent class has specific setup
  * needs, it can overload that function.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Declares and populates the mp_bUsesThisMortality array.
  */
  void PopulateUsesThisMortality();

  /**
  * If a descendent class has specific setup needs, it can overload this
  * function.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  virtual void DoShellSetup(xercesc::DOMDocument *p_oDoc){;};
};
//---------------------------------------------------------------------------
#endif
