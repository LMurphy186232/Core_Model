//---------------------------------------------------------------------------
// LogBiLevelGrowth
//---------------------------------------------------------------------------
#if !defined(LogBiLevelGrowth_H)
  #define LogBiLevelGrowth_H

#include "GrowthBase.h"

class clGrid;

/**
* Increments height growth according to a lognormal equation, with the
* possibility of two sets of parameters for each species.  The two sets of
* parameters can be used for two different growth rates at high and low light.
* This behavior can only be used to create a height growth increment.
*
* The equation used in this behavior is:
* <center>Y = MG * exp(0.5 * (ln(H/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</center>
* where
* <ul>
* <li>Y = height growth in m/year</li>
* <li>MG = max growth</li>
* <li>X<sub>0</sub> = parameter</li>
* <li>X<sub>b</sub> = parameter</li>
* <li>H = tree's height in m</li>
* </ul>
*
* This behavior can also take into account light levels coming from the "Storm
* Light" grid object produced by clStormLight.  This behavior can use two
* different sets of parameter values - one at low light and one at high light.
* The user sets the threshold between the two.  The equation remains the same.
*
* The name string is "logbilevelgrowthshell".  The parameter file call string
* is "LogBilevelGrowth height only".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clLogBiLevelGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clLogBiLevelGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clLogBiLevelGrowth();

  /**
  * Calculates the amount of height growth increase for a particular tree
  * using the lognormal growth equation.  If the light grid is present, this
  * retrieves the light level in the tree's grid cell.  If it is above the
  * threshold, the high-light parameters are used.  If it is below the
  * threshold, the low-light parameters are used.  If the light grid is not
  * present, the low-light parameters are used.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fDiameterGrowth Diameter growth, in cm.
  * @return Amount, in m, by which to increase the tree's height.
  */
  float CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth);

  /**
  * Does the setup for this behavior.  This reads in the parameters from the
  * parameter file, and retrieves the "Storm Light" grid if present.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if:
  * <ul>
  * <li>the high-light growth threshold is not between 0 and 100</li>
  * <li>a value for an X<sub>b</sub> is 0</li>
  * <li>a value for an X<sub>0</sub> is 0</li>
  * <li>a value for max growth is less than 0</li>
  * </ul>
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /**"Storm Light" grid object*/
  clGrid *mp_oStormLight;

  /**Max growth, in m, in low light - sized number of behavior species*/
  float *mp_fLoLightMaxGrowth;

  /**X0 in low light - sized number of behavior species*/
  float *mp_fLoLightX0;

  /**Xb in low light - sized number of behavior species*/
  float *mp_fLoLightXb;

  /**Max growth, in m, in high light - sized number of behavior species*/
  float *mp_fHiLightMaxGrowth;

  /**X0 in high light - sized number of behavior species*/
  float *mp_fHiLightX0;

  /**Xb in high light - sized number of behavior species*/
  float *mp_fHiLightXb;

  /**Threshold between low light and high light growth, as a value between 0
  * and 100 - sized number of behavior species*/
  float *mp_fHiLightThreshold;

  /**To help access the other arrays*/
  int *mp_iIndexes;

  /**Conversion factor to translate the results of the function to the
  * appropriate units per timestep*/
  float m_fYearsPerTimestep;

  /**Code for the "Light" data member of the "Storm Light" grid*/
  int m_iLightCode;
};
//---------------------------------------------------------------------------
#endif // LogBiLevelGrowth_H
