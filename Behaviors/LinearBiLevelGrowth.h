//---------------------------------------------------------------------------
// LinearBiLevelGrowth
//---------------------------------------------------------------------------
#if !defined(LinearBiLevelGrowth_H)
  #define LinearBiLevelGrowth_H

#include "GrowthBase.h"

class clGrid;

/**
* Increments growth according to a simple linear equation, with the possibility
* of two sets of parameters for each species.  The two sets of parameters can
* be used for two different growth rates at high and low light.  This behavior
* can be used to create a growth increment with no automatic height adjustment
* or a growth increment with automatic height adjustment.
*
* The equation used in this behavior is:
* <center>Y = a + b * diam</center>
* where
* <ul>
* <li>Y = diameter growth in cm/year</li>
* <li>a = growth intercept</li>
* <li>b = growth slope</li>
* <li>diam = diam10 for seedlings and saplings, or DBH for adults</li>
* </ul>
*
* This behavior can also take into account light levels coming from the "Storm
* Light" grid object produced by clStormLight.  This behavior can use two
* different sets of parameter values - one at low light and one at high light.
* The user sets the threshold between the two.  The equation remains the same.
*
* The name string is "linearbilevelgrowthshell".  In the parameter file:  For
* diameter growth with no automatic height adjustment, call
* "LinearBilevelGrowth diam only".  For diameter growth with automatic height
* adjustment, call "LinearBilevelGrowth".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clLinearBiLevelGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clLinearBiLevelGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clLinearBiLevelGrowth();

  /**
  * Calculates the amount of diameter growth increase for a particular tree
  * using the linear growth equation.  If the light grid is present, this
  * retrieves the light level in the tree's grid cell.  If it is above the
  * threshold, the high-light parameters are used.  If it is below the
  * threshold, the low-light parameters are used.  If the light grid is not
  * present, the low-light parameters are used.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount, in cm, by which to increase the tree's diameter.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Does the setup for this behavior.  This reads in the parameters from the
  * parameter file, and retrieves the "Storm Light" grid if present.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if the high-light growth threshold is not between 0 and
  * 100.
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

  /**"Storm Light" grid object*/
  clGrid *mp_oStormLight;

  /**Slope of growth equation in low light - b - sized number of behavior
  * species*/
  double *mp_fLoLightSlope;

  /**Intercept of growth equation in low light - a - sized number of behavior
  * species*/
  double *mp_fLoLightIntercept;

  /**Slope of growth equation in high light - b - sized number of behavior
  * species*/
  double *mp_fHiLightSlope;

  /**Intercept of growth equation in high light - a - sized number of behavior
  * species*/
  double *mp_fHiLightIntercept;

  /**Threshold between low light and high light growth, as a value between 0
  * and 100 - sized number of behavior species
  */
  double *mp_fHiLightThreshold;

  /**To help access the other arrays*/
  int *mp_iIndexes;

  /**Conversion factor to translate the results of the function to the
  * appropriate units per timestep*/
  int m_iYearsPerTimestep;

  /**Code for the "Light" data member of the "Storm Light" grid*/
  int m_iLightCode;
};
//---------------------------------------------------------------------------
#endif // SimpleLinearGrowth_H
