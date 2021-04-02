//---------------------------------------------------------------------------
#ifndef ConstantRadialGrowthH
#define ConstantRadialGrowthH
//---------------------------------------------------------------------------
#include "MichMenGrowthBase.h"

/**
 * Constant radial growth - Version 1.0
 *
 * This is a growth shell object which applies a constant radial growth to all
 * trees which use it. The radial growth increment, which should have been
 * converted to diameter increment in cm/timestep, is returned directly.
 *
 * This class's namestring is "constradgrowthshell".  The parameter file call
 * string is "ConstRadialGrowth", for diameter growth with auto-height updating,
 * or "ConstRadialGrowth diam only" for diameter-only growth.
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clConstantRadialGrowth : virtual public clMichMenBase {
//note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
   * Constructor.  The constructor sets the namestring.
   * @param p_oSimManager Sim Manager object.
   */
 clConstantRadialGrowth(clSimManager *p_oSimManager);

 //~clConstantRadialGrowth(); use default destructor

  /**
  * This applies the growth.
  *
  * @param p_oTree Tree to which to calculate growth.
  * @param p_oPop Tree population object.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount of diameter increase, in cm.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

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
  * Reads in values from the parameter file.
  *
  * @param p_oDoc DOM tree from parsed parameter file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

};
//---------------------------------------------------------------------------
#endif
