//---------------------------------------------------------------------------

#ifndef StochasticBiLevelMortalityH
#define StochasticBiLevelMortalityH
//---------------------------------------------------------------------------
#include "MortalityBase.h"

class clGrid;
class clTreePopulation;
/**
 * Stochastic Bi-Level Mortality - Version 2.0
 *
 * This evaluates mortality at a constant rate, with a separate rate at high and
 * low light for each species.  Trees are chosen at random to die.
 *
 * Light levels come from either the "Storm Light" grid object produced by
 * clStormLight or a GLI value stored in the "Light" data member of a tree.
 *
 * This class's namestring is "stochastic bilevel mortshell".  The parameter
 * file call string is "StochasticBiLevelMortality" or
 * "StochasticBiLevelMortality - GLI".
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */
class clStochasticBiLevelMortality : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clStochasticBiLevelMortality(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clStochasticBiLevelMortality();

  /**
  * Reads in values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if the "Storm Light" grid is missing, or if any mortality
  * rate is not between 0 and 1.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality.  This retrieves the light level in the tree's grid
  * cell.  If it is above the threshold, the high-light mortality probability
  * is used.  If it is below the threshold, the low-light probability is used.
  * A random number is compared to the appropriate rate to see if the tree
  * lives or dies.
  *
  * @param p_oTree Tree being evaluated
  * @param fDbh Tree's DBH
  * @param iSpecies Species of the tree being evaluated
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  /**
  * Captures the behavior name passed from the parameter file to set flags
  * based on expected behavior.
  *
  * @param sNameString Behavior name from parameter file.
  */
  void SetNameData(std::string sNameString);


  protected:

    /**Define a type for pointers to functions to get light level*/
  typedef float (clStochasticBiLevelMortality::*Ptr2LightLevel)(clTree *, const short int &);

  /**Function pointer for using the appropriate light level getting function.*/
  Ptr2LightLevel getLightLevel;

  /**"Storm Light" grid object*/
  clGrid *mp_oStormLight;

  /**Tree population - for getting data codes*/
  clTreePopulation *mp_oPop;

  /**Probability of mortality at low light - sized number of behavior species.
  * This value comes from the parameter file as an annual probability, which
  * is transformed in DoShellSetup() to a timestep probability.*/
  double *mp_fLoLightMortProb;

  /**Probability of mortality at low light - sized number of behavior species.
  * This value comes from the parameter file as an annual probability, which
  * is transformed in DoShellSetup() to a timestep probability.*/
  double *mp_fHiLightMortProb;

  /**Threshold between low light and high light mortality, as a value between 0
  * and 100 - sized number of behavior species*/
  double *mp_fHiLightThreshold;

  /**To help access the other arrays*/
  int *mp_iIndexes;

  /**Data member return codes for "Light" data member, if being used. First
   * index is species, second is type. */
  int **mp_iGLILightCodes;

  /**Code for the "Light" data member of the "Storm Light" grid, if used*/
  int m_iLightCode;

  /** Whether this is the GLI version (true) or the Storm Light version (false)*/
  bool m_bIsGLI;

  /**
   * Gets the light level from the Storm Light grid for a tree.
   * @param p_oTree Tree to get light level for.
   * @param iSpecies Tree's species.
   * @return Light level.
   */
  float GetStormLightLevel(clTree *p_oTree, const short int & iSpecies);

  /**
   * Gets the GLI light level for a tree.
   * @param p_oTree Tree to get light level for.
   * @param iSpecies Tree's species.
   * @return Light level.
   */
  float GetGLILightLevel(clTree *p_oTree, const short int & iSpecies);

};
//---------------------------------------------------------------------------
#endif
