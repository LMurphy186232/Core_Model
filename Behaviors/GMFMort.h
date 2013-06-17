//---------------------------------------------------------------------------

#ifndef GMFMortH
#define GMFMortH
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
* GMF Mortality - Version 1.0
*
* This evaluates growth-based mortality according to the GMF mortality equation.
* Since this equation assumes a timestep length of 5 years, and the equation
* has not yet been reworked, this will throw a fatal error if the timestep
* length is not five years.
*
* All species/type combos wishing to use this behavior must have the "Growth"
* data member registered.  It is assumed that the value in that data member
* when mortality is called is the amount of growth for this timestep.
*
* This class's namestring is "gmfmortshell". The parameter file call string
* is "GMFMortality".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clGMFMort : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  * @param p_oSimManager Pointer to Simulation Manager.
  */
  clGMFMort(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clGMFMort();

  /**
  * Performs setup.  This reads in values from parameter file.  It also collects
  * data member codes for "Growth" for each type/species combo to which it is
  * assigned.
  * @param p_oDoc DOM Tree from parsed parameter file.
  * @throws modelErr if there is a species/type code for which "Growth" has not
  * been registered as a float.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality according to the GMF mortality equation.
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

  /**Mortality at zero growth. Old parameter m1*/
  float *mp_fMortAtZeroGrowth;

  /**Light dependent mortality. Old parameter m2*/
  float *mp_fLightDepMort;

  /**
  * Queries for the return codes of the "Growth" float data member of a tree.
  * This data member should have been registered by light.  Return codes are
  * captured in the mp_iGrowthCodes array.
  * @throws modelErr if there is no code for any species/type combo which uses
  * this behavior.
  */
  void GetGrowthVariableCodes();

};
//---------------------------------------------------------------------------
#endif
