//---------------------------------------------------------------------------
// SimpleLinearGrowth
//---------------------------------------------------------------------------
#if !defined(SimpleLinearGrowth_H)
  #define SimpleLinearGrowth_H

#include "GrowthBase.h"

/**
* Increments growth according to a simple linear equation.  This can be used to
* create a growth increment with no automatic height adjustment, a growth
* increment with automatic height adjustment, or a height increment.
*
* The equation used in this behavior is:
* <center>Y = a + b * GLI</center>
* where
* <ul>
* <li>Y = either radial growth in mm/year or height growth in cm/yr
* <li>a = growth intercept
* <li>b = growth slope
* <li>GLI is global light index, from 0 to 100
* </ul>
*
* The name string is "simplelineargrowthshell".  In the parameter file:  For
* diameter growth with no automatic height adjustment, call "SimpleLinearGrowth
* diam only".  For diameter growth with automatic height adjustment,
* call "SimpleLinearGrowth".  For height growth, call "SimpleLinearGrowth
* height only".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clSimpleLinearGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clSimpleLinearGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clSimpleLinearGrowth();

  /**
  * Calculates the amount of height growth increase for a particular tree using
  * the simple linear growth equation.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fDiameterGrowth Amount of diameter growth for this tree, in cm.
  * @return Amount, in m, by which to increase the tree's height.
  */
  float CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth)
    {return CalculateFunctionValue(p_oTree) * m_fConversionFactor;};

  /**
  * Calculates the amount of diameter growth increase for a particular tree
  * using the simple linear growth equation.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount, in cm, by which to increase the tree's diameter.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth)
    {return CalculateFunctionValue(p_oTree) * m_fConversionFactor;};

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

  /**Growth slope - b - sized number of species*/
  double *mp_fSlope;

  /**Growth intercept - a - sized number of species*/
  double *mp_fIntercept;

  /**Conversion factor to translate the results of the function to the
  * appropriate units per timestep, depending on the type of growth behavior
  * this is*/
  double m_fConversionFactor;

  /**
  * Calculates the value of the simple linear growth function.  The meaning of
  * what is returned depends on the type of growth the behavior is doing.
  * @param p_oTree Tree for which to calculate growth.  This is needed to get
  * the GLI.
  * @return The value of the simple linear growth function.  Units depend on the
  * type of growth that this is.
  */
  float CalculateFunctionValue(clTree *p_oTree);
};
//---------------------------------------------------------------------------
#endif // SimpleLinearGrowth_H
