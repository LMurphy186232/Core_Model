//---------------------------------------------------------------------------

#ifndef SizeDependentLogisticMortalityH
#define SizeDependentLogisticMortalityH
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
 * Size Dependent Logistic Mortality - Version 1.0
 *
 * This evaluates mortality according to a logistic function of size.
 *
 * Probability of mortality is:
 *
 * P = MAX/(1+(diam/X0)^Xb))
 *
 * This class's namestring is "sizedeplogisticmortshell". The parameter file
 * call string is "SizeDependentLogisticMortality".
 *
 * Copyright 2014 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>April 8, 2014 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */

class clSizeDependentLogisticMortality : virtual public clMortalityBase {
  //note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
   * Constructor. Sets the namestring and NULLs pointers.
   * @param p_oSimManager Sim Manager object.
   */
  clSizeDependentLogisticMortality(clSimManager *p_oSimManager);

  /**
   * Destructor. Deletes arrays.
   */
  ~clSizeDependentLogisticMortality();

  /**
   * Reads in values from the parameter file.
   * @param p_oDoc DOM tree of parsed input file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Calculates mortality according to the size dependent logistic mortality
   * function.
   *
   * @param p_oTree Tree being evaluated
   * @param fDiam DBH of tree being evaluated - for seedlings will be 0
   * @param iSpecies Species of the tree being evaluated
   * @return natural if the tree is to die, notdead if it lives.
   */
  deadCode DoMort(clTree *p_oTree, const float &fDiam, const short int &iSpecies);

protected:

  /**Max parameter. Array size # species */
  double *mp_fMax;

  /**X0 parameter. Array size # species */
  double *mp_fX0;

  /**Xb parameter. Array size # species */
  double *mp_fXb;

  /**Number years/timestep*/
  int m_iNumberYearsPerTimestep;
};
//---------------------------------------------------------------------------
#endif
