//---------------------------------------------------------------------------

#ifndef ConstantBAGrowthH
#define ConstantBAGrowthH
//---------------------------------------------------------------------------
#include "MichMenGrowthBase.h"

/**
 * Constant basal area growth - Version 1.0
 *
 * This is a growth shell object which applies a constant basal area growth to
 * all trees which use it.  The constant basal increment is found by dividing
 * the annual area increment by the diameter of the tree (recall that the
 * increment is in squared units).  The area increment is already in units per
 * timestep.
 *
 * This class's namestring is "constbagrowthshell".  Call it in the parameter
 * file with the string "ConstBAGrowth", or "ConstBAGrowth diam only" for
 * diameter-only updating.
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */


class clConstantBAGrowth : virtual public clMichMenBase {
//note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
   * Constructor.  The constructor sets the namestring.
   * @param p_oSimManager Sim Manager object.
   */
  clConstantBAGrowth(clSimManager *p_oSimManager);

 //~clConstantBAGrowth(); use default destructor

 /**
  * Captures the namestring passed to this behavior.  This is overridden from
  * clBehaviorBase so we can capture the namestring passed.  Since this class can
  * create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

  /**
   * This applies the growth.
   * @param p_oTree Tree to which to calculate growth.
   * @param p_oPop Tree population object.
   * @param fHeightGrowth Amount of height growth, in m (ignored).
   * @return Amount of diameter increase, in cm.
   */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
   * Reads in values from the parameter file.
   * @param p_oDoc DOM tree from parsed parameter file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

};
//---------------------------------------------------------------------------
#endif
