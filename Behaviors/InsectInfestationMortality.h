//---------------------------------------------------------------------------

#ifndef InsectInfestationMortalityH
#define InsectInfestationMortalityH
//---------------------------------------------------------------------------
#include "MortalityBase.h"

class clTreePopulation;
/**
* Insect Infestation Mortality - Version 1.0
*
* This evaluates mortality for trees that have an insect infestation.
*
* The probability of death is a function of the number of years infested, as
* follows:
*
* P = I +((MAX - I)/(1+(T/X0)^Xb))
*
* where P is mortality probability, I is the intercept (mortality at time of
* infestation), MAX is max mortality probability, T is infestation time in
* years, X0 is time at which mortality prob = 0.5, and Xb controls steepness
* of the mortality increase.
*
* A random number determines whether the trees die. Those that die have "insect"
* passed as the reason code.
*
* Insect infestation is determined by the behavior of class
* clInsectInfestation. This class expects that behavior to be enabled and the
* "YearsInfested" int data member to be present for all trees to which this
* behavior is applied.
*
* This class's namestring is "insect infestation mortshell".  The parameter
* file call string is "InsectInfestationMortality".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clInsectInfestationMortality : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clInsectInfestationMortality(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clInsectInfestationMortality();

  /**
  * Reads in values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>"YearsInfested" is not registered for any tree to which this behavior
  * is applied</li>
  * <li>MAX is not between 0 and 1</li>
  * <li>Intercept is not between 0 and 1</li>
  * <li>X0 = 0</li>
  * </ul>
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality. This retrieves the tree's time of infestation. If it
  * is infested, the mortality probability for that year is compared to a
  * random number to see if the tree lives or dies. If it is uninfested, it does
  * not die.
  *
  * @param p_oTree Tree being evaluated
  * @param fDbh Tree's DBH
  * @param iSpecies Species of the tree being evaluated
  * @return insects if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /** Precalculated mortality probabilities, out to m_iMaxMortTime; any
   * infestation time beyond this can be calculated as a one-off. First index
   * is species, second is time of infestation */
  float **mp_fMortProbs;

  /**Intercept - sized number of species.*/
  float *mp_fIntercept;

  /**Max mort rate - sized number of species.*/
  float *mp_fMax;

  /**X0 - sized number of species.*/
  float *mp_fX0;

  /**Xb - sized number of species.*/
  float *mp_fXb;

  /** Codes for "YearsInfested" data member. Array size is number of species by
   * number of types.*/
  short int **mp_iDataCodes;

  /** Number of species. For destructor.*/
  short int m_iNumSpecies;

  /** Max number of pre-calculated mortalities. This will be the biggest value
   * of 2*X0.*/
  short int m_iMaxMortTime;
};
//---------------------------------------------------------------------------
#endif
