//---------------------------------------------------------------------------

#ifndef BrowsedRelativeGrowthH
#define BrowsedRelativeGrowthH
//---------------------------------------------------------------------------
#include "GrowthBase.h"


class clTree;
class clTreePopulation;

/**
* Browsed relative Michaelis-Menton growth - Version 1.0
*
* This is a growth shell object which applies the Michaelis-Menton function to
* find relative growth. There are two growth rates: one for browsed, and one
* for unbrowsed.
*
* Browsedness is determined by the Random Browse behavior (class
* clRandomBrowse). This class expects that behavior to be enabled and the
* "Browsed" bool data member to be present for all trees to which this behavior
* is applied.
*
* The growth equation is:
* <center><i>Y = (A * gli) / ((A/S) + gli)</i></center>
* where Y is the relative annual radial growth, A is asymptotic diameter
* growth, gli is the amount of light, and S is the slope of growth response.
*
* The actual amount of new growth is:
* <center><i>G = ((Y + 1)<sup>T</sup> - 1) * diam<sup>Q</sup></i></center>
* where Y is from the equation above, T is the number of years per timestep,
* diam is the tree's diameter, and Q is the diameter exponent.
*
* (There are some strangenesses in the math - 1s are added and subtracted at
* various times and I don't know why.  This is based on code I didn't
* originally write.)
*
* The namestring for this class is "browsedrelgrowthshell".  Call in the
* parameter file via "BrowsedRelativeGrowth" or "BrowsedRelativeGrowth
* diam only".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clBrowsedRelativeGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clBrowsedRelativeGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clBrowsedRelativeGrowth();

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
  * Does setup.  Reads in values from the parameter file,, and validates that
  * all species/type combos use light and browse.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if any species/type combo to which this behavior is
  * applied does not have a light behavior or the browse behavior.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /** Slope of growth response, unbrowsed. Array size is number of species.*/
  double *mp_fUnbrowsedS;

  /** Slope of growth response, browsed. Array size is number of species.*/
  double *mp_fBrowsedS;

  /** Asymptotic diameter growth, unbrowsed. Array size is number of species.*/
  double *mp_fUnbrowsedA;

  /** Asymptotic diameter growth, browsed. Array size is number of species.*/
  double *mp_fBrowsedA;

  /** Diameter exponent, unbrowsed. Array size is number of species*/
  double *mp_fUnbrowsedDiamExp;

  /** Diameter exponent, browsed. Array size is number of species*/
  double *mp_fBrowsedDiamExp;

  /** Codes for "Light" data member. Array size is number of species by number
   * of types.*/
  short int **mp_iLightCodes;

  /** Codes for "Browsed" data member. Array size is number of species by number
   * of types.*/
  short int **mp_iBrowsedCodes;

  /**Number of years per timestep - from sim manager*/
  int m_ifNumberYearsPerTimestep;

  /** Number of species. For destructor.*/
  short int m_iNumSpecies;
};
//---------------------------------------------------------------------------
#endif
