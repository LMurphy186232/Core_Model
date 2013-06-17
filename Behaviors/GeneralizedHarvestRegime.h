//---------------------------------------------------------------------------

#ifndef GeneralizedHarvestRegimeH
#define GeneralizedHarvestRegimeH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"

class clTree;

using namespace whyDead;

/**
* Generalized Harvest Regime - Version 1.0
*
* Logs based on plot basal area and biomass.
*
* The probability that the plot is logged in a given time step is a function of
* the total plot biomass, as follows:
* P = a * exp(-m * X^b)
* Where P is the probability, X is the total plot adult biomass in Mg/ha, and
* a, m, and b are the log probability parameters. This is evaluated each time
* step; it does not matter whether logging occurred the previous time step.
*
* If the plot is to be logged, the amount of adult basal area to remove is:
* BAR = alpha * exp(-mu * X^beta)
* where BAR is the percentage to remove (between 0 and 100), X is the total plot
* adult biomass, and alpha, beta, and mu are removal parameters. This percentage
* is then used as the mean in a draw on a gamma distribution.
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
* All adults of all species must participate. All must have "Dimension Analysis"
* applied. Seedlings and saplings are always ignored by this behavior.
*
* The parameter file call string and namestring are "GeneralizedHarvestRegime".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>January 16, 2011 - Created (LEM)
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
  float *mp_fCutProbAlpha;

  /** Beta in the species specific cut probability function. Array size is
   * total number of species.*/
  float *mp_fCutProbBeta;

  /** Gamma in the species specific cut probability function. Array size is
   * total number of species.*/
  float *mp_fCutProbGamma;

  /** Mu in the species specific cut probability function. Array size is
   * total number of species.*/
  float *mp_fCutProbMu;

  /**Probability of logging a */
  float m_fLogProbA;

  /**Probability of logging m */
  float m_fLogProbM;

  /**Probability of logging b */
  float m_fLogProbB;

  /**Removal amount alpha */
  float m_fRemoveA;

  /**Removal amount beta */
  float m_fRemoveB;

  /**Removal amount mu */
  float m_fRemoveM;

  /**Gamma random draw scale parameter */
  float m_fScale;

  /** A in the function for calculating sigma in the cut probability function */
  float m_fCutProbA;

  /** B in the function for calculating sigma in the cut probability function */
  float m_fCutProbB;

  /** C in the function for calculating sigma in the cut probability function */
  float m_fCutProbC;

  /** Allowed max deviation from desired BA for one-pass harvesting */
  float m_fAllowedRange;

  /**Total plot BA this timestep */
  float m_fTotalBA;

  /**Total plot biomass this timestep */
  float m_fTotalBiomass;

  /**Total number of species.*/
  int m_iNumSpecies;

  /**Reason code to pass to the tree population when trees are killed.*/
  deadCode m_iReasonCode;

 /**
  * Reads harvest data from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  */
 void ReadHarvestParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
   * Decides whether or not a harvest will occur this timestep. This gets the
   * total amount of biomass and evaluates the probability equation. The
   * result is compared to a random number to determine logging probability.
   *
   * While this is totaling biomass, it will also total basal area for
   * m_fTotalBA.
   * @return True if logging is to occur, false if not.
   */
  bool CutThisTimestep();

  /**
   * Gets the "Biomass" data member code for adults of each species.
   * @throw modelErr if there is a missing code.
   */
  void GetDataCodes();
};
//---------------------------------------------------------------------------
#endif

