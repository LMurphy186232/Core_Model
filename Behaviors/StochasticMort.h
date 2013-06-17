//---------------------------------------------------------------------------

#ifndef StochasticMortH
#define StochasticMortH
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
* Stochastic Mortality - Version 1.0
*
* This evaluates stochastic mortality.
*
* This class's namestring is "stochasticmortshell".
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clStochasticMort : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clStochasticMort(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clStochasticMort();

  /**
  * Reads in values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality stochastically.
  *
  * @param fDbh DBH of tree being evaluated - for seedlings will be 0
  * @param p_oTree Tree being evaluated
  * @param iSpecies Species of the tree being evaluated
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /**Random mortality rate per species.  This is read as an annual value and
  * compounded to the number of years per timestep during setup.*/
  float *mp_fRandomMort;

};
//---------------------------------------------------------------------------
#endif
