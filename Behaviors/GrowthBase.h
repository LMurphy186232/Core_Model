//---------------------------------------------------------------------------

#ifndef GrowthBaseH
#define GrowthBaseH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"
#include "Constants.h"


class clTree;
class clTreePopulation;
class clGrowthOrg;
class clPlot;
class clAllometry;

/**
* Base class for behaviors that implement tree growth.  Tree growth is the
* change of a tree's diameter and/or height.  (The normal method is to increase
* these dimensions.  This class has not been tested with code that causes
* trees to shrink.)
*
* There are two ways to change a tree's size.  The first method is to
* increment the tree's diameter, and allow the tree population to automatically
* change the height to match.  The second method is to increment the diameter
* and height separately and explicitly, and override the tree population's
* auto-update of the other dimension.  This base class can accommodate child
* classes that use either method.  Each object of each child class is
* responsible for figuring out how the user intends it to work and setting the
* appropriate flags, especially if it can work by either method.
*
* Sometimes growth behaviors need to make calculations before any DBHs have
* been incremented; for instance, neighborhood competition growth behaviors
* need to be assured that no trees have been changed when performing these
* calculations.  If a growth behavior has to make these sorts of calculations,
* it can override the PreGrowthCalcs() function.
*
* Classes descended from this class are referred to as shell classes.  (That's
* my term - there's probably a better one.)  This is because these child
* classes don't function as standalone behaviors, but leave the organizational
* work to another class (clGrowthOrg).  The clGrowthOrg class coordinates the
* efforts of all objects of the shell classes for maximum efficiency.
*
* Since clGrowthOrg is not descended from clBehaviorBase, it doesn't receive
* the triggers from the simulation manager that tell it when to work (calls to
* GetData(), Action(), etc).  So, one shell object (any one, it doesn't matter
* which) is "hooked" to clGrowthOrg.  When it receives the triggers from
* clSimManager, it passes them on to clGrowthOrg for processing.  All other
* shells ignore these triggers, since they receive their instructions from
* clGrowthOrg.
*
* Every object made from this class or a descendent class must have the string
* "growthshell" in its namestring somewhere, to identify it as a shell object.
*
* This base class also declares a new tree float data member on behalf of each
* child shell object; the growth org object then registers it.  The descendents
* don't need to do anything. The new data member is called "Growth", and will
* store the amount of diameter growth in mm/yr calculated.
*
* You cannot create an object that is an instance of this class, although you
* can certainly cast pointers to instances of child classes to be of type
* clGrowthBase if needed.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>April 28, 2004 - Submitted as beta (LEM)
* <br>May 21, 2004 - Added support for separate diameter / height
* increments (LEM)
* <br>February 24, 2005 - Added new data member "Growth" to replace "lgm" (LEM)
*/

class clGrowthBase : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  friend class clGrowthOrg;

  public:

  /**Values describing the method by which the object instance of this class
  * plans to implement tree growth*/
  enum growthType {diameter_auto, /**<The object calculates a diameter increase,
  and the height update is left to the tree population's auto-update process*/
                   diameter_only, /**<The object sets a diameter increase only.
                   The height should be set manually by another object.*/
                   height_only /**<The object sets a height increase only.
                   The diameter should be set manually by another object.*/
                   };
  /**
  * Constructor.  The constructor checks to see if the growth org object has
  * been created - if not, it creates it.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clGrowthBase(clSimManager *p_oSimManager);

  /**
  * Destructor.  The destructor deletes the growth org object if it was the
  * hooked object.
  */
  virtual ~clGrowthBase();

  /**
  * Performs all growth calculations if "hooked".  This function is the same for
  * all descendent classes - they do not need to override it (in fact, they
  * can't).  If a particular object is hooked, it calls mp_oGrowthOrg's
  * DoGrowthAssignments function.  Otherwise it does nothing.
  */
  void Action();

  /**
  * Calculates the amount of diameter growth increase for a particular tree,
  * if applicable.  If overridden, this function must not change the tree's
  * diameter (clGrowthOrg will do that).
  * <p>
  * <b>REMEMBER to appropriately compound growth by the number of years per
  * timestep!</b>
  *
  * The tree being passed will NOT yet have had any growth applied to it.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fHeightGrowth Amount of height growth, in m.  <b>ONLY USE</b> if
  * this behavior has set m_bGoLast to true; otherwise this value will not be
  * useful.
  * @return Amount, in cm, by which to increase the tree's diameter.
  */
  virtual float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth) {return 0;};

  /**
  * Calculates the amount of height growth increase for a particular tree,
  * if applicable.  If overridden, this function must not change the tree's
  * height (clGrowthOrg will do that).
  * <p>
  * <b>REMEMBER to appropriately compound growth by the number of years per
  * timestep!</b>
  *
  * The tree being passed will NOT yet have had any growth applied to it.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fDiameterGrowth Amount of diameter growth for this tree, in cm.
  * @return Amount, in m, by which to increase the tree's height.
  */
  virtual float CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth) {return 0;};

  /**
  * Calculates the value to go into a tree's "Growth" data member as the amount
  * of growth.  If overridden, this function must not change the tree's
  * "Growth" data member (clGrowthOrg will do that).  This will only be called
  * if this behavior was the DiameterIncrementer (either diameter_auto or
  * diameter_only) for the tree being passed.
  *
  * The tree being passed will NOT yet have had any growth applied to it.
  *
  * @param p_oTree Tree to get "Growth" for.
  * @param fDiameterGrowth Amount of diameter growth to be added, in mm per
  * year.
  * @return Value to place in "Growth", in mm radial growth/yr.
  */
  virtual float GetGrowthMemberValue(clTree *p_oTree, float fDiameterGrowth) {return fDiameterGrowth * m_fConvertCmPerTSToMmPerYr;};

  /**
  * Performs calculations before any DBHs have been changed.  Override this
  * function to perform some processing before any growth change has been
  * applied.  There are no guarantees as to what order the behaviors will be
  * called for this function, but it is guaranteed that no growth incrementing
  * has been applied.  To make this guarantee worth something, DO NOT CHANGE
  * ANY TREE SIZE VALUES in this function!
  */
  virtual void PreGrowthCalcs( clTreePopulation *p_oPop ){;};

  /**
  * Gets the method by which this behavior increments growth.  The possible
  * values are those values in the growthType enum.
  * @return Growth method, as diameter_only, diameter_auto, or height_only.
  */
  growthType GetGrowthMethod() {return m_iGrowthMethod;};

  /**
  * Get the growth org object.
  *
  * @return Growth org object.
  */
  clGrowthOrg* GetGrowthOrg() {return mp_oGrowthOrg;};

  /**
  * Performs data member registrations for "Growth".
  * This will be the same for all descendent classes - they do not need to
  * override.  If a particular object is hooked, it calls the growth org
  * object's DoTreeDataMemberRegistrations().  Otherwise it does nothing.
  */
  void RegisterTreeDataMembers();

  protected:

  static clGrowthOrg *mp_oGrowthOrg; /**<clGrowthOrg object - this pointer is
  held in common by all shells*/

  growthType m_iGrowthMethod; /**<The method by which this object plans to
  update tree dimensions*/

  float m_fConvertCmPerTSToMmPerYr; /**<Conversion factor from cm diameter
              growth per timestep to mm radial growth per year*/
  float m_fConvertMmPerYearToCmPerTS; /**<Conversion factor to take
              amounts from mm of radial growth per year to cm of diameter
              growth per timestep*/

  bool m_bHooked; /**<Wwhether or not this shell object is hooked to clGrowthOrg.
             clGrowthOrg will set this flag.*/

  /**Whether or not this behavior's growth should go last when used with a
  * complementary growth behavior.  Some height-incrementing growth behaviors
  * depend on the results of the growth-incrementers, and vice-versa.  If this
  * behavior is diam-with-auto-height growth, this flag is ignored.  If
  * separate growth and height behaviors are used, precedence is as follows:
  * if one behavior requests to be last and the other does not, then the
  * behavior that wants to go last will do so.  If both of these flags are
  * false or both are true, height-only behaviors win and get to go last.
  * This defaults to false.*/
  bool m_bGoLast;

  /**
  * Triggers all growth setup if an object is hooked.  This will be the same
  * for all descendent classes.  If a particular object is hooked, it calls
  * mp_oGrowthOrg's DoSetup() function, which calls the function DoShellSetup()
  * - if a descendent class has specific setup needs, it can overload that
  * function.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Setup for a descendent class.  If a descendent class has specific setup
   * needs, it can overload this function.
   * @param p_oDoc DOM tree of parsed input file.
   */
  virtual void DoShellSetup(xercesc::DOMDocument *p_oDoc){;};
};
//---------------------------------------------------------------------------
#endif
