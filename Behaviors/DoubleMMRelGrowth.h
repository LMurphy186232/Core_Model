//---------------------------------------------------------------------------
// DoubleMMRelGrowth
//---------------------------------------------------------------------------
#if !defined(DoubleMMRelGrowth_H)
  #define DoubleMMRelGrowth_H

#include "MichMenGrowthBase.h"


class clTree;
class clTreePopulation;
class clGrid;

/**
* Double-resource relative growth - Version 1.0
*
* This is a growth shell object which applies a double Michaelis-Menton
* function to find relative growth as a function of light and some other
* resource.
*
* The growth equation is:
* <center><i>Y = ((A + C*R) * GLI) / (((A + C*R)/S) + GLI)</i></center>
* where
* <ul>
* <li><i>Y</i> is the relative annual radial growth</li>
* <li><i>A</i> is asymptotic diameter growth parameter</li>
* <li><i>S</i> is the slope of growth response parameter</li>
* <li><i>C</i> is the resource influence parameter</li>
* <li><i>R</i> is the amount of the second resource</li>
* <li><i>GLI</i> is the global light index, as a number between 0 and 100</li>
* </ul>
*
* To compound the growth over the number of years per timestep, the following
* equation is used:
* <center><i>G = ((Y + 1)<sup>T</sup> - 1) * diam</i></center>
* where:
* <ul>
* <li><i>G</i> is the amount of diameter growth, in cm</li>
* <li><i>Y</i> is the result of the double Michaelis-Menton function,
* above</li>
* <li><i>T</i> is the number of years per timestep</li>
* <li><i>diam</i> is the relevant tree diameter to increase</li>
* </ul>
*
* The identity of the second resource (R) in the Michaelis-Menton function is
* unimportant.  It is assumed that the parameters will be scaled appropriately
* for the data range for whatever resource it is.  The resource levels are
* obtained from a grid object called "Resource", with a single float data
* member called "Resource" which contains the level of the resource in
* question.  If the grid is not present, this is considered a fatal error by
* this behavior.
*
* To call this from the parameter file, use either "DoubleResourceRelative
* diam with auto height" (for diam with auto height growth) or
* "DoubleResourceRelative diam only" (for diam only growth).
*
* The namestring for this class is "doublemmrelgrowthshell".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clDoubleMMRelGrowth : virtual public clMichMenBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clDoubleMMRelGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clDoubleMMRelGrowth();

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
  * Captures the behavior name passed from the parameter file.  This is useful
  * since this class can produce a few different kinds of behaviors.
  *
  * @param sNameString Behavior name from parameter file.
  */
  void SetNameData(std::string sNameString);

  /**
  * Does setup.  Reads in values from the parameter file, and validates that
  * all species/type combos use light (each must have "lgm" registered).
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if any species/type combo to which this behavior is
  * applied does not have a light behavior, or if there is no grid called
  * "Resource".
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /**Grid containing the levels of the second resource.  This is not created
  * by this behavior; it is expected to already be available.  It should be
  * named "Resource" and have one float data member called "Resource".*/
  clGrid *mp_oResourceGrid;

  /**Parameter governing the influence of the second resource (C in equation
  * above).  Array size is number of behavior species.*/
  float *mp_fResourceInfluence;

  /**Number of years per timestep - from sim manager*/
  float m_fNumberYearsPerTimestep;

  /**The float data member code for the resource grid.*/
  short int m_iResourceCode;
};
//---------------------------------------------------------------------------
#endif // DoubleMMRelGrowth_H
