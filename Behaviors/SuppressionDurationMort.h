//---------------------------------------------------------------------------

#ifndef SuppressionDurationMortH
#define SuppressionDurationMortH
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
* Suppression duration mortality - Version 1.0
*
* This causes mortality as a function of tree age. Age is tracked by the class
* clTreeAgeCalculator.
*
* Probability of mortality is p = max/(1+(age/X0)^Xb). If tree age is 10000,
* mortality probability = 0 because that is an initial conditions tree.
*
* Dead reason code is natural.
*
* This class's namestring is "suppressiondurationmortshell". The parameter file
* call string is "SuppressionDurationMortality".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clSuppressionDurationMort : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clSuppressionDurationMort(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clSuppressionDurationMort();

  /**
  * Reads in values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>"Age" is not registered for any tree to which this behavior
  * is applied</li>
  * <li>max is not between 0 and 1</li>
  * <li>X0 = 0</li>
  * </ul>
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality. This retrieves the tree's age. The mortality
  * probability for that year is compared to a random number to see if the tree
  * lives or dies.
  *
  * @param p_oTree Tree being evaluated
  * @param fDbh Tree's DBH
  * @param iSpecies Species of the tree being evaluated
  * @return insects if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /** Precalculated mortality probabilities, out to m_iMaxMortTime; any
   * age beyond this can be calculated as a one-off. First index
   * is species, second is age */
  float **mp_fMortProbs;

  /**Max mort rate - sized number of species.*/
  float *mp_fMax;

  /**X0 - sized number of species.*/
  float *mp_fX0;

  /**Xb - sized number of species.*/
  float *mp_fXb;

  /** Codes for "Tree Age" data member. Array size is number of species by
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
