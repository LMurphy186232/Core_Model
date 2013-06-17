//---------------------------------------------------------------------------
// ExpResourceMortality
//---------------------------------------------------------------------------
#if !defined(ExpResourceMortality_H)
  #define ExpResourceMortality_H

#include "MortalityBase.h"

class clGrid;

/**
* Exponential Growth and Resource Based Mortality - Version 1.0
*
* This evaluates mortality probability as a function of growth and a second
* (unidentified) resource.
*
* The probability of mortality is:
* <center>Prob = (d+(a*R))*exp[-(b+c*R)*G]</center>
*
* where:
* <ul>
* <li>Prob is the probability of mortality, as a value between 0
* and 1</li>
* <li>R is the amount of some resource (whose identity is
* unimportant)</li>
* <li>G is the amount of diameter growth, in mm</li>
* <li>a is the mortality at zero growth parameter scaled as a function
* of the resource R</li>
* <li>b is the light dependent mortality parameter</li>
* <li>c is the resource dependent mortality parameter</li>
* <li>d is a parameter</li>
* </ul>
*
* To compound the mortality probability over the number of years per
* timestep, use 1-(1-P)<sup>X</sup>, where P is the annual probability of
* mortality and X is the number of years per timestep.
*
* Once a tree's mortality probability has been calculated, a random number is
* used to determine if the tree lives or dies.
*
* The value of the second resource comes from a grid called "Resource", with a
* single float member named "Resource".  This behavior doesn't create this
* grid, and it's considered a fatal error if it is not present at setup.  This
* behavior will not modify the contents of this grid in any way.
*
* This class's namestring is "expresourcemortshell".  Its parameter file call
* string is "ExponentialGrowthResourceMortality".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clExpResourceMortality : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clExpResourceMortality(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clExpResourceMortality();

  /**
  * Performs setup for this behavior.  Calls:
  * <ol>
  * <li>ReadParameterFileData</li>
  * <li>GetTreeDataMemberCodes</li>
  * <li>GetResourceGrid</li>
  * </ol>
  *
  * @param p_oDoc Parsed DOM tree of parameter file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality according to the growth-resource mortality equation.
  * The equation is listed above.
  * @param fDbh DBH of tree being evaluated - for seedlings will be 0
  * @param p_oTree Tree being evaluated
  * @param iSpecies Species of the tree being evaluated
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /**Grid containing the levels of the second resource.  This is not created
  * by this behavior; it is expected to already be available.  It should be
  * named "Resource" and have one float data member called "Resource".*/
  clGrid *mp_oResourceGrid;

  /**Data member codes for "Growth" member - # behavior species by total # types*/
  short int **mp_iGrowthCodes;

  /**Data member codes for "X" tree data member - # behavior species by total
  * # types*/
  short int **mp_iXCodes;

  /**Data member codes for "Y" tree data member - # behavior species by total
  * # types*/
  short int **mp_iYCodes;

  /**Mortality at zero growth parameter - "a" in equation above.  Array size is
   * number of behavior species.*/
  float *mp_fA;

  /**Light dependent mortality parameter - "b" in equation above.  Array size
   * is number of behavior species.*/
  float *mp_fB;

  /**Resource dependent mortality parameter - "c" in equation above.  Array
   * size is number of behavior species.*/
  float *mp_fC;

  /**"d" in equation above.  Array size is number of behavior species.*/
  float *mp_fD;

  /**For array access*/
  short int *mp_iIndexes;

  /**Number of years per timestep*/
  float m_fNumberYearsPerTimestep;

  /**Index for "Resource" data member of the "Resource" grid*/
  short int m_iResourceCode;

  /**
  * Reads in parameter file data.
  * @param p_oDoc Parsed DOM tree of parameter file.
  */
  void ReadParameterFileData (xercesc::DOMDocument *p_oDoc);

  /**
  * Gets the "Resource" grid.
  * @throws modelErr if the grid named "Resource" is not present, or is not
  * set up correctly.
  */
  void GetResourceGrid();

  /**
  * Queries for the return codes of the "Growth", "X", and "Y" float data
  * members of a tree. The "Growth" data member should have been registered by
  * a child class of clGrowthBase.
  * @throws modelErr if there is no code for any species/type combo which uses
  * this behavior.
  */
  void GetTreeDataMemberCodes();
};
//---------------------------------------------------------------------------
#endif // ExpResourceMortality_H
