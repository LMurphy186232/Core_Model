//---------------------------------------------------------------------------

#ifndef DisperseOrgH
#define DisperseOrgH
//---------------------------------------------------------------------------
#include <xercesc/dom/DOM.hpp>
#include "DataTypes.h"
class clDisperseBase;

class clTreePopulation;
class clGrid;
class clModelMath;
class clSimManager;

/**
* Disperse org - Version 1.0
*
* This class coordinates a common task for all dispersal functions: the
* determination of the correct number of seeds per grid cell of each species.
* The number of seeds in a cell can be either deterministic, meaning that the
* number of seeds that is calculated by each disperse object is the number
* that ends up in the cell, or stochastic, meaning the calculated number of
* seeds is used to get a random value from a particular probability
* distribution function.
*
* Disperse objects organized by this function should add the number of seeds to
* each grid cell that they calculate, even if they are not integers.  If the
* number of seeds is deterministic, this class uses a RandomRound() for each
* species to make sure the final number of seeds is an integer.  If the
* number of seeds is stochastic, the number of seeds for each species is used
* as the mean for a random draw on one of the following probability
* distribution functions:  Poisson, normal, lognormal, or negative binomial.
*
* This class hooks into a disperse shell object and is triggered by that
* object when the shell is triggered by the behavior manager.  This class calls
* the "Action" functions of all disperse objects in its care, then adjusts the
* number of seeds accordingly.  All behaviors with "disperse" in their
* namestrings will be brought under the organization of this object.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>April 28, 2004 - Submitted as beta (LEM)
* <br>July 8, 2009 - Allowed fractional seeds (LEM)
*/
class clDisperseOrg {
  public:

  /**
  * Destructor.
  */
  ~clDisperseOrg();

  /**
  * Constructor.
  *
  * @param p_oHookedShell A clDisperseBase object which then becomes
  * the hooked disperse shell object.
  */
  clDisperseOrg(clDisperseBase *p_oHookedShell);

  /**
  * Performs dispersal.  Each object in mp_oDispObjects gets its AddSeeds()
  * function called.  Then the final seed tally is adjusted by calling
  * AdjustSeedNumbers().  This should be called each timestep by the hooked
  * shell's Action() function.
  */
  void DoDisperse();

  /**
  * Does the setup for this object.  This function calls GetDisperseObjects()
  * and GetParameterFileData().
  *
  * DoSetup() is called by the hooked shell's GetData() function.
  *
  * @param p_oSimManager Pointer to the simulation manager.  Since this object
  * is not descended from clWorkerBase, it does not already have its own
  * pointer.
  * @param p_oDoc Pointer to parsed parameter file.
  * @throws modelErr if either object passed is NULL.
  */
  void DoSetup(clSimManager *p_oSimManager, xercesc::DOMDocument *p_oDoc);

  protected:

  /**Dispersed Seeds grid.  This grid is created by the clDisperseBase class.*/
  clGrid *mp_oSeedsGrid;

  /**List of all disperse objects - those with "disperse" somewhere in their
  * namestrings.  These will be in the same order that the behavior manager has
  * them.
  */
  clDisperseBase **mp_oDispObjects;

   /**Standard deviation if normal or lognormal distribution is desired, or
    * clumping parameter if lognormal.  One for each species.  Those that don't
    * use disperse won't have a value filled in.*/
  double *mp_fRandParameter;

  short int *mp_iCodes; /**<Number of seeds code in the seed grid for each
                        species.*/

  int m_iNumDispObjects; /**<Number of disperse objects in mp_oDispObjects*/
  int m_iTotalSpecies; /**<Number of total species for the run*/

  /**What seed adjustment type applies to this run*/
  pdf m_iSeedDistributionMethod;

/**
  Declares and populates the mp_oDispObjects array.  It does this by going
  through the behaviors and looking for the ones with "disperse" in their names.
  @param p_oSimManager Sim Manager object.
*/
  void GetDisperseObjects(clSimManager *p_oSimManager);

/**
  Reads values out of the parameter file.  This gets whether the seedling
  distribution is stochastic or deterministic, and if stochastic, the type
  of prbability distribution function and any additional values needed by
  that function.
  @param p_oSimManager Sim Manager object.
  @param p_oDoc DOM tree from parsed parameter file.
  @throws modelErr if the seed distribution method value from the parameter
  file is invalid.
*/
  void GetParameterFileData(clSimManager *p_oSimManager, xercesc::DOMDocument *p_oDoc);

  /**
  * Gets the seed grid and populates the mp_iCodes array.
  * @param p_oSimManager Sim Manager object.
  * @throws modelErr if the grid is not present or the codes are invalid.
  */
  void GetSeedGridObject(clSimManager *p_oSimManager);

  /**
  * Adjusts seed numbers depending on the type of seedling distribution,
  * stochastic or deterministic.
  */
  void AdjustSeedNumbers();

  /**
  * Performs a deterministic adjustment of seed number.
  * @param fNumber Number of seeds to adjust.
  * @param iSpecies Species of seed.
  * @return Number of adjusted seeds.
  */
  float DeterministicAdjust(float fNumber, int iSpecies);

  /**
  * Performs a stochastic adjustment of seed number according to a normal
  * distribution.
  * @param fNumber Number of seeds to adjust.
  * @param iSpecies Species of seed.
  * @return Number of adjusted seeds.
  */
  float NormalAdjust(float fNumber, int iSpecies);

  /**
  * Performs a stochastic adjustment of seed number according to a lognormal
  * distribution.
  * @param fNumber Number of seeds to adjust.
  * @param iSpecies Species of seed.
  * @return Number of adjusted seeds.
  */
  float LognormalAdjust(float fNumber, int iSpecies);

  /**
  * Performs a stochastic adjustment of seed number according to a Poisson
  * distribution.
  * @param fNumber Number of seeds to adjust.
  * @param iSpecies Species of seed.
  * @return Number of adjusted seeds.
  */
  float PoissonAdjust(float fNumber, int iSpecies);

  /**
  * Performs a stochastic adjustment of seed number according to a negative
  * binomial distribution.
  * @param fNumber Number of seeds to adjust.
  * @param iSpecies Species of seed.
  * @return Number of adjusted seeds.
  */
  float NegativeBinomialAdjust(float fNumber, int iSpecies);

  /**
  * Sets the Adjust function pointer according to the value of
  * m_iSeedDistributionMethod.
  */
  void SetFunctionPointer();

  /**Function pointer for the appropriate Adjust function*/
  float (clDisperseOrg::*Adjust)(float fNumber, int iSpecies);

};
//---------------------------------------------------------------------------
#endif
