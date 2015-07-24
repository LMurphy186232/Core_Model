//---------------------------------------------------------------------------

#ifndef GeneralizedHarvestRegimeH
#define GeneralizedHarvestRegimeH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"


//#define DEBUG_GEN_HARV

#ifdef DEBUG_GEN_HARV
#include <stdio.h>
#include <fstream>
#endif


class clTree;

using namespace whyDead;

/**
* Generalized Harvest Regime - Version 1.1
*
* Logs based on plot basal area and/or biomass. The user chooses whether to
* base calculations on total plot adult basal area or biomass. In the case
* of biomass, the Dimension Analysis behavior must be applied.
*
* The probability that the plot is logged in a given time step is a function of
* the total plot biomass or basal area, as follows:
* P = a * exp(-m * X^b)
* Where P is the probability, X is the total plot adult biomass in Mg/ha OR
* the total plot adult basal area in m2/ha, and a, m, and b are the log
* probability parameters. This is evaluated each time step; it does not matter
* whether logging occurred the previous time step.
*
* If the plot is to be logged, the amount of adult basal area to remove can
* be calculated using one of two possible distributions. If the gamma
* distribution is chosen, the mean amount to remove is:
* BAR = alpha * exp(-mu * X^beta)
* where BAR is the percentage to remove (between 0 and 100), X is the total plot
* adult biomass OR basal area, and alpha, beta, and mu are removal parameters.
* This percentage is then used as the mean in a draw on a gamma distribution.
* Any draws over 100% cause a re-draw until the chosen value is less than 100%.
*
* The other distribution is a user-defined function with 10 classes. The user
* supplies the midpoint of each class and the probability of that class.
* SORTIE will transform this into a cumulative distribution function and use
* a random draw to determine the class. A random number drawn on a uniform
* distribution determines the exact removal target from that class.
*
* Individual trees have a cut preference function as follows:
* P = (1 - gamma * exp(-beta * R ^ alpha)) * (exp(-0.5*((DBH - mu)/sigma)^2))
* where P is the cut probability, R is the plot percentage of BA to remove,
* gamma, beta, alpha, and mu are species-specific removal probability
* parameters, and sigma is a + b * R^c, where a, b, and c are parameters.
*
* Each tree's removal probability is then compared with a random number to
* determine whether or not it will be removed.  The removal probabilities cannot
* be depended upon to average the target percent of basal area to remove; so if
* the removed amount is not within a designated range of the target, a second
* pass will be made through the trees with all probabilities adjusted either up
* or down in order to get closer to the target.  No more than two passes will be
* made.
*
* If desired, sapling mortality can be incorporated as a consequence of
* harvesting in the plot. The probability of sapling mortality is a logistic
* function of the percent basal area removed:
*
* Mort prob = p + ((1-p)/(1 + (PCR/m)^n))
*
* where PCR is the percent basal area removed (0-100) and p, m, and n are parameters.
* This is turned on by a parameter flag. Saplings removed in this way have a
* reason code of "natural".
*
* All adults of all species must participate. All must have "Dimension Analysis"
* applied if biomass is being used for cut decisions. Seedlings are always
* ignored by this behavior. Saplings are ignored if their optional mortality is
* not used.
*
* The parameter file call string and namestring are "GeneralizedHarvestRegime".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>January 16, 2011 - Created (LEM)
* <br>July 7, 2013 - Added flag for BA or Biomass and made version 1.1 (LEM)
* <br>September 27, 2013 - Added gamma redraw if above 100, rather than a cap
* at 100 (which artificially raises the chances of 100) (LEM)
* <br>May 8, 2014 - Added sapling mortality
* <br>June 3, 2014 - Added user-defined cut distribution
*/
class clGeneralizedHarvestRegime : virtual public clBehaviorBase {

 public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clGeneralizedHarvestRegime(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clGeneralizedHarvestRegime();

  /**
   * Performs the harvest. CutThisTimestep() is called to determine whether a
   * harvest occurs.
   */
  void Action();

  /**
   * Performs setup. This calls:
   * <ul>
   * <li>ReadHarvestParameterFileData</li>
   * <li>GetDataCodes</li>
   * </ul>
   * @param p_oDoc DOM tree of parsed input file.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Cut Probability" tree float data member.  The return codes
  * are captured in the mp_iCutProbCode array.
  * @throw modelErr if this behavior is not applied to adults of all species,
  * or is applied to anything else.
  */
  void RegisterTreeDataMembers();

 protected:
  clTreePopulation *mp_oPop;   /**<Stashed pointer to tree population*/

  /**Code for the "Biomass" float data member for adults of each species.*/
  short int *mp_iBiomassCode;

  /**Code for the "Gen Harvest" bool data member for adults of each species.*/
  short int *mp_iHarvestCode;

  /** Alpha in the species specific cut probability function. Array size is
   * total number of species.*/
  double *mp_fCutProbAlpha;

  /** Beta in the species specific cut probability function. Array size is
   * total number of species.*/
  double *mp_fCutProbBeta;

  /** Gamma in the species specific cut probability function. Array size is
   * total number of species.*/
  double *mp_fCutProbGamma;

  /** Mu in the species specific cut probability function. Array size is
   * total number of species.*/
  double *mp_fCutProbMu;

  /** Upper bounds of intensity classes if we're using a user-defined
   * probability distribution function for determining cut amount. These are
   * expressed in proportions from 0 to 1.*/
  double *mp_fCutAmtIntensityClasses;

  /** Probability of each intensity class if we're using a user-defined
   * probability distribution function for determining cut amount. These are
   * expressed in proportions from 0 to 1.*/
  double *mp_fCutAmtIntensityClassProb;

  /**Probability of logging a */
  double m_fLogProbA;

  /**Probability of logging m */
  double m_fLogProbM;

  /**Probability of logging b */
  double m_fLogProbB;

  /**Removal amount alpha, if we're using gamma PDF for cut amount */
  double m_fGammaMeanRemoveA;

  /**Removal amount beta, if we're using gamma PDF for cut amount */
  double m_fGammaMeanRemoveB;

  /**Removal amount mu, if we're using gamma PDF for cut amount */
  double m_fGammaMeanRemoveM;

  /**Gamma random draw scale parameter */
  double m_fScale;

  /** A in the function for calculating sigma in the cut probability function */
  double m_fCutProbA;

  /** B in the function for calculating sigma in the cut probability function */
  double m_fCutProbB;

  /** C in the function for calculating sigma in the cut probability function */
  double m_fCutProbC;

  /** p in the sapling mortality function */
  double m_fSapP;

  /** m in the sapling mortality function */
  double m_fSapM;

  /** n in the sapling mortality function */
  double m_fSapN;

  /** Allowed max deviation from desired BA for one-pass harvesting */
  double m_fAllowedRange;

  /**Total plot BA this timestep */
  double m_fTotalBA;

  /**Total plot biomass this timestep */
  double m_fTotalBiomass;

  /**Total number of species.*/
  int m_iNumSpecies;

  /**Total number of user-defined size class. */
  int m_iNumUserDefDistSizeClasses;

  /**Reason code to pass to the tree population when trees are killed.*/
  deadCode m_iReasonCode;

  /**Whether to use biomass (true) or basal area (false) in cut decisions */
  bool m_bUseBiomass;

  /**Whether or not to incorporate sapling mortality*/
  bool m_bSaplingMortality;

  /**Whether to use the gamma distribution (true) or user-defined (false) */
  bool m_bUseGammaDist;

 /**
  * Reads harvest data from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if any of the following are true:
  * <ul>
  * <li>Any value in harvest intensity classes is not between 0 and 1</li>
  * <li>Any value in harvest intensity class probabilities is not between 0 and 1</li>
  * <li>Harvest class probabilities don't add up to 1</li>
  * <li>Harvest intensity classes are not monotonically increasing</li>
  * </ul>
  */
 void ReadHarvestParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
   * Decides whether or not a harvest will occur this timestep. This gets the
   * total amount of biomass or basal area and evaluates the probability
   * equation. The result is compared to a random number to determine logging
   * probability.
   *
   * While this is totaling biomass, it will also total basal area for
   * m_fTotalBA.
   * @return True if logging is to occur, false if not.
   */
  bool CutThisTimestep();

  /**
   * Gets the "Biomass" data member code for adults of each species. If basal
   * area is being used, this function exits without doing anything.
   * @throw modelErr if there is a missing code.
   */
  void GetDataCodes();

  /**
   * Uses the appropriate distribution function to pick the percentage to cut.
   * @return Percentage to cut as a value between 0 and 100.
   */
  double GetPercentToCut();

  #ifdef DEBUG_GEN_HARV
  std::fstream out;
  #endif
};
//---------------------------------------------------------------------------
#endif

