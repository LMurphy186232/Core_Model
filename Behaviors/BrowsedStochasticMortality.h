//---------------------------------------------------------------------------

#ifndef BrowsedStochasticMortalityH
#define BrowsedStochasticMortalityH
//---------------------------------------------------------------------------
#include "MortalityBase.h"

class clTreePopulation;
/**
* Browsed Stochastic Mortality - Version 1.0
*
* This evaluates mortality at a constant rate, with a separate rate for browsed
* and unbrowsed trees.  Trees are chosen at random to die.
*
* Browsedness is determined by the Random Browse behavior (class
* clRandomBrowse). This class expects that behavior to be enabled and the
* "Browsed" bool data member to be present for all trees to which this behavior
* is applied.
*
* This class's namestring is "browsed stochastic mortshell".  The parameter
* file call string is "BrowsedStochasticMortality".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clBrowsedStochasticMortality : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clBrowsedStochasticMortality(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clBrowsedStochasticMortality();

  /**
  * Reads in values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if the browsed behavior is missing, or if any mortality
  * rate is not between 0 and 1.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality. This retrieves the tree's browsed status. If it is
  * browsed, the browsed mortality probability is used. If it is unbrowsed, the
  * unbrowsed probability is used. A random number is compared to the
  * appropriate rate to see if the tree lives or dies.
  *
  * @param p_oTree Tree being evaluated
  * @param fDbh Tree's DBH
  * @param iSpecies Species of the tree being evaluated
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /**Probability of mortality when browsed - sized number of species.
  * This value comes from the parameter file as an annual probability, which
  * is transformed in DoShellSetup() to a timestep probability.*/
  float *mp_fBrowsedMortProb;

  /**Probability of mortality when unbrowsed - sized number of species.
  * This value comes from the parameter file as an annual probability, which
  * is transformed in DoShellSetup() to a timestep probability.*/
  float *mp_fUnbrowsedMortProb;

  /** Codes for "Browsed" data member. Array size is number of species by number
   * of types.*/
  short int **mp_iBrowsedCodes;

  /** Number of species. For destructor.*/
  short int m_iNumSpecies;
};
//---------------------------------------------------------------------------
#endif
