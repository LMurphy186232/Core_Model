//---------------------------------------------------------------------------
// DensDepRodentSeedPredation.h
//---------------------------------------------------------------------------
#if !defined(DensDepRodentSeedPredation_H)
#define DensDepRodentSeedPredation_H

#include "BehaviorBase.h"

class clGrid;
/**
* Density Dependent Neighborhood Seed Predation
*
* This behavior reduces the number of seeds by simulating seed predation. Rate
* of offtake is a function of multi-species seed production and the previous
* timestep's rodent population.
*
* Requirements: must be used with the "Masting disperse with autocorrelation"
* behavior.
*
* Determining rodent population
* Rodent population is defined on a scale from 0 to 1. This represents both
* the population level of rodents, and also the amount of seed removal that
* will occur.
* @htmlonly
  <br/><center>&lambda;<sub>t</sub> = funcresp * densdep</center>
  @endhtmlonly
* Where &lambda;<sub>t</sub> is the rodent population index at timestep t,
* funcresp is a functional response term to seed levels, and densdep is a
* lagged density dependence term.
*
* @htmlonly
  <br/><center>funcresp = (exp(&Sigma;slope<sub>sp</sub> * MI<sub>sp</sub>)/exp(&Sigma;slope<sub>sp</sub>))^a</center>
  @endhtmlonly
* The numerator is the sum across all species of each species' masting index
* for the timestep times the species' slope. "a" is the functional response
* exponent parameter.
* @htmlonly
  <br/><center>densdep = 1+(dd.coef)/(ln(&lambda;(t-1)/((1+dd.coef))))</center>
  @endhtmlonly
* Where dd.coef is a parameter and &lambda;<sub>t-1</sub> is the rodent population index
* at timestep t-1. In the first timestep, the densdep term is set to 1.
*
* Determining seeds consumed
* The rodent population index (&lambda;) is a number between 0 and 1 that
* represents the relative size of the rodent population, and also serves as
* proportion of seeds removed. Thus the remaining seeds for each species is
* surviving.seeds= starting.seeds*(1-&lambda;_t)
*
* This class's namestring is "DensDepRodentSeedPredation".
*
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>June 29, 2022 - Created (LEM)
*/
class clDensDepRodentSeedPredation : public clBehaviorBase {

public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clDensDepRodentSeedPredation(clSimManager *p_oSimManager);

  /**
   * Destructor. Frees memory.
   */
  ~clDensDepRodentSeedPredation();

  /**
   * Performs seed predation according to the equations above.
   */
  void Action();

protected:

  /**Pointer to output grid.  The name of this grid is "Rodent Lambda".
   * It has only one grid cell.  It has one float data member - "rodent_lambda".
   * No maps for initial conditions are accepted. This is just in case someone
   * wants to review this as output. */
    clGrid *mp_oRodentGrid;

  /**
    * Does setup. This reads in values from the parameter file, gets the pointer
    * to the dispersed seeds grid, and sets up the output grid.
    *
    * The Masting Disperse with Autocorrelation behavior will have already
    * decided all the masting levels by this time, so we can query the values
    * for the entire run.
    *
    * @param p_oDoc DOM tree of parsed input file.
    * @throws modelErr if this is linked and there is no functional response
    * seed predation.
    */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
  clGrid *mp_oSeedGrid;

  /** Time series of masting levels, 0-1. This can be input by the user, or
   * randomly generated. This array is sized number of timesteps by number
   * of species.*/
  double **mp_fMastTimeSeries;

  /**Parameter for slope of the functional response. Array size is number of
   * species to which this behavior applies.*/
  double *mp_fFuncResponseSlope;

  /**Exponent "a" in the functional response.*/
  double m_fFuncResponseA;

  /**Density dependent coefficient.*/
  double m_fDensDepCoeff;

  /**Data member codes for seed grid for number of seeds. Array size is number
   * of species to which this behavior applies.*/
  short int *mp_iSeedGridCode;

  /** Last timestep's rodent lambda, for easy reference */
  double m_fLastTimestepLambda;

  /** Grid code to the "rodent_lambda" grid data member. */
  short int m_iRodentLambdaCode;

  /** For clearing memory */
  short int m_iNumTimesteps;


  /**
   * Gets a pointer to the "Dispersed Seeds" grid, and sets up the "Rodent
   * Lambda" grid.
   * @throws modelErr if the seed grid is not found
   */
  void SetupGrids();

  /**
   * Reads parameter file data.
   * @param p_oDoc DOM tree of parsed input file.
   */
  void ReadParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
   * Get masting indexes from the Masting Disperse with Autocorrelation
   * behavior. The masting indexes are all worked out before the run
   * starts, so they can be retrieved in the setup stage.
   * @throws modelErr if Masting Disperse with Autocorrelation is not
   * registered for all species of this behavior.
   */
  void GetMastingIndexes();
};
//---------------------------------------------------------------------------
#endif // DensDepRodentSeedPredation_H
