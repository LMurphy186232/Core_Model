//---------------------------------------------------------------------------
// LogisticGrowth
//---------------------------------------------------------------------------
#if !defined(LogisticGrowth_H)
  #define LogisticGrowth_H

#include "GrowthBase.h"

/**
* Increments growth according to a logistic equation.  This can be used to
* create a growth increment with no automatic height adjustment, a growth
* increment with automatic height adjustment, or a height increment.
*
* The equation used in this behavior is:
* <center>Y = a/(1 + e<sup>(b - c * GLI)</sup>)</center>
* where
* <ul>
* <li>Y = either radial growth in mm/year or height growth in cm/yr
* <li>a = asymptotic growth at full light
* <li>b and c are shape parameters
* <li>GLI is global light index, from 0 to 100
* </ul>
*
* The name string is "logisticgrowthshell".  In the parameter file:  For
* diameter growth with no automatic height adjustment, call "LogisticGrowth
* diam only".  For diameter growth with automatic height adjustment, call
* "LogisticGrowth".  For height growth, call "LogisticGrowth height". XML root
* is "LogisticGrowth".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clLogisticGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clLogisticGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clLogisticGrowth();

  /**
  * Calculates the amount of height growth increase for a particular tree using
  * the logistic growth equation.
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
  * using the logistic growth equation.
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

  /**Asymptotic growth at full light - a - sized number of behavior species*/
  double *mp_fAsympGrowthAtFullLight;

  /**Shape parameter 1 - b - sized number of behavior species*/
  double *mp_fShape1;

  /**Shape parameter 2 - c - sized number of behavior species*/
  double *mp_fShape2;

  /**For accessing the other arrays*/
  short int *mp_iIndexes;

  /**Conversion factor to translate the results of the function to the
  * appropriate units per timestep, depending on the type of growth behavior
  * this is*/
  float m_fConversionFactor;

  /**
  * Calculates the value of the logistic growth function.  The meaning of
  * what is returned depends on the type of growth the behavior is doing.
  * @param p_oTree Tree for which to calculate growth.  This is needed to get
  * the GLI.
  * @return The value of the logistic growth function.  Units depend on the
  * type of growth that this is.
  */
  float CalculateFunctionValue(clTree *p_oTree);
};
//---------------------------------------------------------------------------
#endif // LogisticGrowth_H
