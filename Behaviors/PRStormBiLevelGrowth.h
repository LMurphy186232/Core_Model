//---------------------------------------------------------------------------
// PRStormBiLevelGrowth
//---------------------------------------------------------------------------
#if !defined(PRStormBiLevelGrowth_H)
  #define PRStormBiLevelGrowth_H

#include "GrowthBase.h"

class clGrid;

/**
* PR Storm Bi-Level Growth, version 1.0
* This behavior was created for Puerto Rico, and is so bizarre that I have
* included the "PR" in the name to warn off unsuspecting folks.  This behavior
* increments growth using one of two equations, one for low light levels and
* one for high light levels.
*
* The equation used for low light levels is:
* <center>Y = a + b * diam</center>
* where
* <ul>
* <li>Y = diameter growth in cm/year</li>
* <li>a = growth intercept</li>
* <li>b = growth slope</li>
* <li>diam = diam10 for seedlings and saplings, or DBH for adults</li>
* </ul>
*
* The equation used for high light levels is:
* <center>H = a * diam * exp(-b * N)</center>
* where
* <ul>
* <li>H = <b>height</b> growth in cm/year</li>
* <li>a and b = parameters</li>
* <li>diam = diam10 for seedlings and saplings, or DBH for adults</li>
* <li>N = number of years since the last storm, from the "Storm Damage"
*     grid</li>
* </ul>
*
* Note that the high-light equation calculates height growth, while the
* low-light equation calculates diameter growth.  If the high-light equation
* is used, the amount of height growth is transformed into an amount of
* diameter growth with the help of the clAllometry class.  Because of this
* arrangement, this behavior refuses to act as a diam only incrementer.
*
* This behavior gets its light levels from the "Storm Light" grid object
* produced by clStormLight.  It gets the number of years since the last storm
* from the "Storm Damage" grid object produced by clStorm.  Both of these
* grids, and thus the behaviors that create them, are required.
*
* The name string is "prstormbilevelgrowthshell".  In the parameter file,
* call "PRStormBilevelGrowth".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clPRStormBiLevelGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clPRStormBiLevelGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clPRStormBiLevelGrowth();

  /**
  * Calculates the amount of diameter growth increase for a particular tree
  * using the appropriate growth equation.  First this retrieves the light
  * level in the tree's grid cell.  If it is above the threshold, the
  * high-light equation used.  If it is below the threshold, the low-light
  * equation is used.
  *
  * In the case of tree life-history-stage transitions due to high-light height
  * growth, this isn't going to be too smart.  It will calculate a new height,
  * then back-calculate a diameter, then return the difference between the
  * existing and new diamter.
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
  * @throws modelErr if:
  * <ul>
  * <li>the high-light growth threshold is not between 0 and 100;</li>
  * <li>the storm behavior is missing (no "Storm Damage" grid); or</li>
  * <li>the storm light behavior is missing (no "Storm Light" grid)</li>
  * </ul>
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /** "Storm Light" grid object*/
  clGrid *mp_oStormLight;

  /** "Storm Damage" grid object*/
  clGrid *mp_oStorm;

  /**Slope of growth equation in low light - b - sized number of behavior
  * species*/
  float *mp_fLoLightSlope;

  /**Intercept of growth equation in low light - a - sized number of behavior
  * species*/
  float *mp_fLoLightIntercept;

  /**High light growth equaiton "a" - sized number of behavior species*/
  float *mp_fHiLightA;

  /**High light growth equaiton "b" - sized number of behavior species*/
  float *mp_fHiLightB;

  /** Threshold between low light and high light growth, as a value between 0
  * and 100 - sized number of behavior species
  */
  float *mp_fHiLightThreshold;

  /** To help access the other arrays*/
  int *mp_iIndexes;

  /** Conversion factor to translate the results of the function to the
  * appropriate units per timestep*/
  float m_fYearsPerTimestep;

  /** Code for the "Light" data member of the "Storm Light" grid*/
  int m_iLightCode;

  /** Code for the "stormtime" data member of the "Storm Damage" grid*/
  int m_iStormtimeCode;
};
//---------------------------------------------------------------------------
#endif // SimpleLinearGrowth_H
