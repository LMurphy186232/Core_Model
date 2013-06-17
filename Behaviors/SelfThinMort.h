//---------------------------------------------------------------------------

#ifndef SelfThinningH
#define SelfThinningH
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
 * Self-Thinning Mortality - Version 1.0
 *
 * This evaluates self-thinning mortality.
 *
 * This class's namestring is "selfthinmortshell". The parameter file call
 * string is "SelfThinning".
 *
 * Self-thinning mortality departs from our traditional modeling approach in
 * that parameters are 'tuned' to site data for specific conditions such as
 * highly dense, young spruce-aspen stands. Self-thinning is a pseudo-density
 * dependent function that can overcomes the shortcoming of Sortie in killing
 * small trees in highly dense uniform aged stands. This approach is applicable
 * only in these specific conditions. It is a heuristic model that should be
 * replaced with a generalized multi-species self-thinning model when the data
 * become available.
 *
 * Copyright 2003 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */

class clSelfThinMort : virtual public clMortalityBase {
  //note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
   * Constructor.  Sets the namestring and NULLs pointers.
   * @param p_oSimManager Sim Manager object.
   */
  clSelfThinMort(clSimManager *p_oSimManager);

  /**
   * Destructor.  Deletes arrays.
   */
  ~clSelfThinMort();

  /**
   * Reads in values from the parameter file.
   * @param p_oDoc DOM tree of parsed input file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Calculates mortality according to the self-thinning function.
   *
   * @param p_oTree Tree being evaluated
   * @param fDiam DBH of tree being evaluated - for seedlings will be 0
   * @param iSpecies Species of the tree being evaluated
   * @return natural if the tree is to die, notdead if it lives.
   */
  deadCode DoMort(clTree *p_oTree, const float &fDiam, const short int &iSpecies);

protected:
  float *mp_fSelfThinSlope; /**<Slope of self-thinning function*/
  float *mp_fSelfThinIntercept; /**<Intercept of self-thinning function*/
  float *mp_fSelfThinMaxDbh; /**<Max DBH for self-thinning.*/
  float m_fNumberYearsPerTimestep; /**<Number years/timestep-from sim manager*/
};
//---------------------------------------------------------------------------
#endif
