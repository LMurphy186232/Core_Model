//---------------------------------------------------------------------------
// PowerHeightGrowth
//---------------------------------------------------------------------------
#if !defined(PowerHeightGrowth_H)
  #define PowerHeightGrowth_H

#include "GrowthBase.h"

/**
* Increments height growth according to a power equation.
*
* The equation used in this behavior is:
* <center>Y = n * H<sup>b</sup></center>
* where
* <ul>
* <li>Y = height growth in cm/yr
* <li>n and b are parameters
* <li>H is height in cm
* </ul>
*
* This is looped over the number of years per timestep, allowing H to increment
* at each intermediate year.
*
* The name string is "powergrowthshell".  In the parameter file, call
* "PowerGrowth height only".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>April 7, 2009 - Created (LEM)
*/
class clPowerHeightGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clPowerHeightGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clPowerHeightGrowth();

  /**
  * Calculates the amount of height growth increase for a particular tree using
  * the power growth equation.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fDiameterGrowth Amount of diameter growth for this tree, in cm.
  * @return Amount, in m, by which to increase the tree's height.
  */
  float CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth);

  /**
  * Does the setup for this behavior.  This reads in the parameters from the
  * parameter file.
  * @param p_oDoc Parsed parameter file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /** n - sized number of species*/
  float *mp_fN;

  /** b - sized number of species*/
  float *mp_fB;

  /**Number of years per timestep - from sim manager*/
  float m_fNumberYearsPerTimestep;

};
//---------------------------------------------------------------------------
#endif // PowerHeightGrowth_H
