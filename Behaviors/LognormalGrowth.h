//---------------------------------------------------------------------------
// LognormalGrowth
//---------------------------------------------------------------------------
#if !defined(LognormalGrowth_H)
  #define LognormalGrowth_H

#include "GrowthBase.h"

/**
* Increments growth according to a lognormal equation.  This can be used to
* create a growth increment with no automatic height adjustment, a growth
* increment with automatic height adjustment, or a height increment.
*
* The equation used in this behavior is:
* <center>Y = a * e<sup>(-0.5(ln(diam/36)/b)<sup>2</sup>)</sup>)*(GLI/100)<sup>c</sup></center>
* where
* <ul>
* <li>Y = either radial growth in mm/year or height growth in cm/yr
* <li>a = growth increment at diameter 36
* <li>b = Shape parameter
* <li>c = Effect of shade
* <li>diam = diameter of the tree at which to apply growth, in cm
* <li>GLI is global light index, from 0 to 100
* </ul>
*
* This behavior compounds this result for multi-year timesteps.  In the case
* of diameter growth, a copy of the diameter value is incremented X times (with
* X being the number of years per timestep) and the final increment is the sum
* of all the interim increments.  In the case of height growth, the amount of
* diameter growth is divided by X and this value used to increment height X
* times, with the nth diameter (n = 1 to X) being the original diameter plus
* (n * diameter growth / X).
*
* The name string is "lognormalgrowthshell". In the parameter file: For diameter
* growth with no automatic height adjustment, call "LognormalGrowth diam only".  For diameter growth with automatic height adjustment, call
* "LognormalGrowth".  For height growth, call "LognormalGrowth height only".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clLognormalGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clLognormalGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clLognormalGrowth();

  /**
  * Calculates the amount of height growth increase for a particular tree using
  * the lognormal growth equation. The function value is calculated for each
  * year of the timestep by using the original diameter plus fDiameterGrowth
  * divided by the years per timestep times the loop counter for the year being
  * compounded. The final increment is the sum of all the intermediate
  * increments.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fDiameterGrowth Amount of diameter growth for this tree, in cm.
  * @return Amount, in m, by which to increase the tree's height.
  */
  float CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth);

  /**
  * Calculates the amount of diameter growth increase for a particular tree
  * using the lognormal growth equation. The function value is calculated for
  * each year of the timestep by repeatedly incrementing a copy of the diameter
  * (the tree's actual diameter remains untouched, as it is supposed to). The
  * final increment is the sum of all the intermediate increments.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount, in cm, by which to increase the tree's diameter.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Does the setup for this behavior.  This reads in the parameters from the
  * parameter file, and validates that all species/type combos use light (each
  * must have "Light" registered).
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if any species/type combo to which this behavior is
  * applied does not have a light behavior.
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

  /**Growth increment at diameter 36 - a - sized number of behavior species*/
  double *mp_fIncAtDiam36;

  /**Shape parameter - b - sized number of behavior species*/
  double *mp_fShape;

  /**Effect of shade - c - sized number of behavior species*/
  double *mp_fShade;

  /**For accessing the other arrays*/
  short int *mp_iIndexes;

  /**Conversion factor to translate the results of the function to the
  * appropriate units, depending on the type of growth behavior this is*/
  float m_fConversionFactor;

  /**Number of years per timestep*/
  int m_iNumberOfYearsPerTimestep;

  /**
  * Calculates the value of the lognormal growth function for one increment. The
  * meaning of what is returned depends on the type of growth the behavior is
  * doing.
  * @param iSpecies Tree species.
  * @param fGLI The GLI value.
  * @param fDiam The diameter for which to calculate this increment.
  * @return The value of the size dependent logistic growth function.  Units
  * depend on the type of growth that this is.
  */
  inline float CalculateFunctionValue(int iSpecies, float fGLI, float fDiam);
};
//---------------------------------------------------------------------------
#endif // LognormalGrowth_H
