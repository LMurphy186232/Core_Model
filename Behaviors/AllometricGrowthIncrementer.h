//---------------------------------------------------------------------------
// AllometricGrowthIncrementer
//---------------------------------------------------------------------------
#if !defined(AllometricGrowthIncrementer_H)
  #define AllometricGrowthIncrementer_H

#include "GrowthBase.h"

/**
* Calculates a growth increment based on allometry. This is designed to be a
* secondary growth method for height or diameter when the growth method for the
* other dimension is the primary one.
*
* If this is acting as a height increment, the increment is calculated as:
* <br><center>inc = f(DIAM<sub>a</sub>) - f(DIAM<sub>b</sub>)</center>
* <br>where inc is the height increment, f(x) is the height as a function
* of diameter allometry equation appropriate for the tree type,
* DIAM<sub>a</sub> is the tree's diameter before the growth increment is
* applied, and DIAM<sub>b</sub> is the tree's diameter after the growth
* increment is applied.  The tree height cannot be greater than the maximum
* tree height for a species. If the tree type is seedling, DIAM is diameter at
* 10 cm.  Otherwise, it's DBH.
*
* If this is acting as a diameter increment, the increment is calculated as:
* <br><center>inc = f(H<sub>a</sub>) - f(H<sub>b</sub>)</center>
* <br>where inc is the diameter increment, f(x) is the height as a function
* of diameter allometry equation appropriate for the tree type,
* H<sub>a</sub> is the tree's diameter before the growth increment is
* applied, and H<sub>b</sub> is the tree's diameter after the growth
* increment is applied.
*
* When calculating an increment, the same allometric equation is used for both
* portions, even if the tree would be making a type transition.  For example,
* if the tree is a seedling and the amount of diameter growth is enough to make
* it into a sapling, the seedling diam10-height relationship is still used for
* both values.
*
* The parameter file call string for this behavior is either "HeightIncrementer"
* or "DiameterIncrementer".  The namestring is "allometric incrementer
* growthshell".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clAllometricGrowthIncrementer : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clAllometricGrowthIncrementer(clSimManager *p_oSimManager);

  //~clAllometricGrowthIncrementer(); //Use default destructor

  /**
  * Calculates the amount of height growth increase for a particular tree.
  * The height increment is calculated as the normal allometric height
  * of the DBH before diameter growth is applied minus the normal allometric
  * height of the DBH after diameter growth is applied.
  *
  * If the tree is a seedling, the diam10 before and after the growth increment
  * is used in the seedling diam10-height allometric equation.  If the tree is
  * a sapling, the diam10 before and after the growth increment is converted to
  * a DBH value and that value is used in the sapling DBH-height allometric
  * equation.  If the tree is an adult, then DBH before and after the growth
  * increment is used in the adult DBH-height allometric equation.
  *
  * This doesn't need to compound height by timestep length, since diameter
  * should already take that into account.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fDiameterGrowth Amount of diameter growth for this tree, in cm.
  * @return Amount, in m, by which to increase the tree's height.
  */
  float CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth);

  /**
  * Calculates the amount of diameter growth increase for a particular tree.
  * The diameter increment is calculated as the diameter derived from height
  * before height growth is applied minus the diameter derived from height
  * after height growth is applied.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount, in cm, by which to increase the tree's diameter.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Does the setup for this behavior. This queries the tree population for a
  * pointer to the allometry object.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Captures the namestring passed to this behavior.  This is overridden from
  * clBehaviorBase so we can capture the namestring passed.  Since this class
  * can create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);


  protected:

  /**Pointer to the allometry object.*/
  clAllometry *mp_oAllom;

};
//---------------------------------------------------------------------------
#endif // AllometricGrowthIncrementer_H
