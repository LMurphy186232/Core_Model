//---------------------------------------------------------------------------

#ifndef BCMortH
#define BCMortH
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
* BC Mortality - Version 1.0
*
* This evaluates mortality according to the BC mortality equation.
*
* This class's namestring is "bcmortshell". The parameter file call string is
* "BCMortality".
*
* NOTE from old code: mortality function in Kobe and Coates(1997) uses annual mm
* growth new function is not correct if m1 != 1.0  (Confirm this statement)
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clBCMort : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clBCMort(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clBCMort();

  /**
  * Reads in values from the parameter file and makes sure all data needed is
  * collected.
  *
  * @param p_oDoc Parsed DOM tree of parameter file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality according to the BC mortality equation.
  *
  * @param fDbh DBH of tree being evaluated - for seedlings will be 0
  * @param p_oTree Tree being evaluated
  * @param iSpecies Species of the tree being evaluated
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /**Data member codes for "Growth" member - species by type*/
  short int **mp_iGrowthCodes;

  /**Mortality at zero growth - old code m1*/
  float *mp_fMortAtZeroGrowth;

  /**Light dependent mortality - old code m2*/
  float *mp_fLightDepMort;

  /**Number of years per timestep*/
  float m_fNumberYearsPerTimestep;

  /**
  * Queries for the return codes of the "Growth" float data member of a tree.
  * This data member should have been registered by clGrowthBase child classes.
  * Return codes are captured in the mp_iGrowthCodes array.
  * @throws modelErr if there is no code for any species/type combo which uses
  * this behavior.
  */
  void GetGrowthVariableCodes();
};
//---------------------------------------------------------------------------
#endif
