//---------------------------------------------------------------------------

#ifndef RelativeGrowthH
#define RelativeGrowthH
//---------------------------------------------------------------------------
#include "MichMenGrowthBase.h"


class clTree;
class clTreePopulation;

/**
* Relative growth - Version 1.2
*
* This is a growth shell object which applies the Michaelis-Menton function to
* find relative growth. It can be used to grow either the diameter or the
* height. Height and diameter growth are calculated slightly differently; the
* diameter growth matches the historical method which goes back to Pacala 1996,
* and the height growth grows the way I think it should be grown.
*
* The growth equation is:
* <center><i>Y = (A * gli) / ((A/S) + gli)</i></center>
* where Y is the relative annual growth (cm radial in the case of diameter, cm
* in the case of height), A is asymptotic growth (diameter or height), gli is
* the amount of light, and S is the slope of growth response.
*
* The actual amount of new diameter growth is:
* <center><i>G = ((Y + 1)<sup>T</sup> - 1) * diam<sup>Q</sup></i></center>
* where Y is from the equation above, T is the number of years per timestep,
* diam is the tree's diameter, and Q is the diameter exponent.
*
* (There are some strangenesses in the math - 1s are added and subtracted at
* various times and I don't know why.  This is based on code I didn't
* originally write.)
*
* Height growth is:
* <center><i>G = Y * H<sup>Q</sup></i></center>
* where G is height grown in cm and H is the height in cm. This is looped over
* the number of years per timestep, allowing H to increment at each intermediate
* year.
*
* Diameter growth may be limited to a max of the adult constant area increment
* or the adult constant radial increment (see clConstantRadialGrowth and
* clConstantBAGrowth for more on how these increments are calculated and
* applied).
*
* An object of this class can be created by the strings "RelRadialGrowth"
* (relative diameter growth limited by constant radial increment),
* "RelBAGrowth" (relative diameter growth limited by constant basal area
* increment), "RelUnlimGrowth" (non-limited relative diameter growth), and
* "relative michaelis-menton height growth" (height growth, always non-limited).
*
* The namestring for this class is "relativegrowthshell".  It is probable that
* there will be more than one object of this class created.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clRelativeGrowth : virtual public clMichMenBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clRelativeGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clRelativeGrowth();

  /**
  * Applies growth as described in the equation above.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount of growth increase, in cm.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Applies growth as described in the equation above.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fDiameterGrowth Amount of diameter growth for this tree, in cm.
  * @return Amount, in m, by which to increase the tree's height.
  */
  float CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth);

  /**
  * Captures the behavior name passed from the parameter file.  This is useful
  * since this class can produce a few different kinds of behaviors.
  *
  * @param sNameString Behavior name from parameter file.
  */
  void SetNameData(std::string sNameString);

  /**
  * Does setup.  Reads in values from the parameter file, and validates that
  * all species/type combos use light (each must have "Light" registered).
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if any species/type combo to which this behavior is
  * applied does not have a light behavior.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /**Diameter or height exponent - array size is number of species*/
  float *mp_fExp;

  /**Number of years per timestep - from sim manager*/
  float m_fNumberYearsPerTimestep;
};
//---------------------------------------------------------------------------
#endif
