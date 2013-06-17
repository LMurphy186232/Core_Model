//---------------------------------------------------------------------------
#ifndef SenescenceMortH
#define SenescenceMortH
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
 * Senescence - Version 1.0
 *
 * This evaluates mortality due to senescence.  The senescence equation is
 * evaluated to determine the probability of death.  The probability is applied
 * to all trees to which senescence is assigned, but in practice death rate
 * increases due to senescence don't kick in until the DBH at onset parameter is
 * reached.  Death rates rise until the asymptotic max DBH value is reached,
 * after which they level off and don't rise further.
 *
 * This cannot be assigned to seedlings.
 *
 * This class's namestring is "senescencemortshell". The parameter file name
 * string is "senescence".
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */

class clSenescenceMort : virtual public clMortalityBase {
  //note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clSenescenceMort(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clSenescenceMort();

  /**
   * Reads in values from the parameter file.
   * @param p_oDoc DOM tree of parsed input file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Calculates mortality according to senescence.
   * @param fDiam DBH of tree being evaluated - for seedlings will be 0
   * @param p_oTree Tree being evaluated
   * @param iSpecies Species of the tree being evaluated
   * @return natural if the tree is to die, notdead if it lives.
   */
  deadCode DoMort(clTree *p_oTree, const float &fDiam, const short int &iSpecies);

protected:

  floatVal *mp_fRandomAlpha; /**<Random mortality alpha, for calculating annual probability of death.  Array size is number of species to which this behavior applies.*/
  floatVal *mp_fRandomBeta; /**<Random mortality beta, for calculating annual probability of death.  Array size is number of species to which this behavior applies.*/
  floatVal *mp_fDbhAtOnset; /**<DBH at onset of senescence.  Old parameter elderlyMort.  Array size is number of species to which this behavior applies.*/
  float **mp_fMortProb; /**<Probability of death per timestep for each DBH value out to the maximum.  Array of total number of species by m_iMaxDbh.*/

  int m_iMaxDbh; /**<DBH of asymptotic maximum mortality.  Defaults to 100*/
  short int m_iNumTotalSpecies; /**<Keep our own copy for the destructor.*/

  /**
   * Calculates mortality probability for a given dbh out to the maximum.  This
   * probability will have the number of years per timestep already taken into
   * account.
   */
  void CalculateMortalityProbability();
};
//---------------------------------------------------------------------------
#endif
