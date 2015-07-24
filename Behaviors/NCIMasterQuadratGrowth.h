//---------------------------------------------------------------------------
#ifndef NCIMasterQuadratGrowthH
#define NCIMasterQuadratGrowthH
//---------------------------------------------------------------------------
#include "GrowthBase.h"
#include "NCI/NCIBehaviorBase.h"

class clGrid;

/**
* NCI quadrat growth - Version 1.0
*
* This is a growth shell object which applies the NCI (neighborhood competition
* index) function. The basic function is a maximum rate of growth that is
* reduced by a set of multiplicative effects. The set of effects used is up
* to the user. This simplifies the process by calculating annual growth on a
* per quadrat basis.
*
* The main difference between this and NCI growth is that this uses no
* size, shading, or damage effects.
*
* The amount of growth is in cm/year. For multi-year timesteps, the behavior
* will calculate a single year's growth and then multiply by number of years
* per timestep.
*
* The annual growth for each species is calculated for each grid cell in the
* "NCI Quadrat Growth" grid, which this behavior creates.  Trees take their
* growth from the grid cell in which they are found. This value can be applied
* as-is or used as the basis for a random draw to introducing stochasticity.
*
* The parameter file call string for this to be diameter-incrementing with
* auto-height updating is "NCIMasterQuadratGrowth"; for diameter-only
* incrementing, use "NCIMasterQuadratGrowth diam only". The namestring for this
* behavior is "nciquadratgrowthshell". The XML string is
* "NCIMasterQuadratGrowth".
*
* Copyright 2013 Charles D. Canham.
* @author Lora E. Murphy
*
  <br>Edit history:
  <br>-----------------
  <br>January 7, 2013: Created (LEM)
*/
class clNCIMasterQuadratGrowth : virtual public clGrowthBase, clNCIBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clNCIMasterQuadratGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clNCIMasterQuadratGrowth();

  /**
  * Returns the value in the tree's float data member that holds the value
  * that was calculated by PreGrowthCalcs().
  *
  * @param p_oTree Tree to which to apply growth.
  * @param p_oPop Tree population object.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount of diameter growth, in cm.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Calculates growth for all NCI trees. Climate effects, which do not depend
  * on individual trees, are called once; effects which do not depend on a
  * target tree's size are called once per tree; and effects with a size
  * component are called once per year, looping over the years in a timestep.
  * The values are stashed in the "Growth" tree float data member for later
  * application.

  * This must be called first of any growth stuff, since it uses other trees'
  * DBHs to calculate NCI, and these must be before growth has been applied.
  *
  * Growth per timestep is calculated by looping over the number of years
  * per timestep and incrementing the DBH.
  *
  * @param p_oPop Tree population object.
  */
  void PreGrowthCalcs( clTreePopulation *p_oPop );

  /**
  * Does setup.
  * <ol>
  * <li>ReadParameterFile() is called to read the parameter file's data.</li>
  * <li>FormatQueryString() is called.</li>
  * <li>SetupGrid() is called.</li>
  * </ol>
  *
  * @param p_oDoc DOM tree of parsed input tree.
  * @throws modelErr if all values of max growth are not greater than 0.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Captures the namestring passed to this behavior. This is overridden from
  * clBehaviorBase so we can capture the namestring passed. Since this class
  * can create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

  protected:

  /**
   * Grid holding amount of growth for each species. The grid name is "NCI
   * Quadrat Growth". It has X float data members, where X = the total number
   * of species. The data member names are "growth_x", for the amount of
   * diameter growth (where "x" is the species number).
   */
  clGrid* mp_oGrid;

  /**Maximum growth value. Array sized number of species.*/
  float *mp_fMaxPotentialValue;

  /**Standard deviation if normal or lognormal distribution is desired. One for
   * each species.*/
  float *mp_fRandParameter;

  /** Holds data member codes for the "growth_x" data members of the "NCI
   * Climate Quadrat Growth" grid.  Array size is total # species.*/
  short int *mp_iGridGrowthCodes;

  /**Total number of species - for the destructor */
  short int m_iNumTotalSpecies;

  /**What stochastic growth distribution applies to this run*/
  pdf m_iStochasticGrowthMethod;

  /**
  * Sets up the "NCI Climate Quadrat Growth" grid. This ignores any maps.
  * @param p_oPop Tree population object.
  */
  void SetupGrid( clTreePopulation *p_oPop );

  /**
  * Performs a deterministic adjustment of growth.
  * @param fNumber Growth to adjust.
  * @param iSpecies Species.
  * @return Final growth.
  */
  float DeterministicAdjust(float fNumber, int iSpecies);

  /**
  * Performs a stochastic adjustment of growth according to a normal
  * distribution.
  * @param fNumber Growth to adjust.
  * @param iSpecies Species.
  * @return Final growth.
  */
  float NormalAdjust(float fNumber, int iSpecies);

  /**
  * Performs a stochastic adjustment of growth according to a lognormal
  * distribution.
  * @param fNumber Growth to adjust.
  * @param iSpecies Species.
  * @return Final growth.
  */
  float LognormalAdjust(float fNumber, int iSpecies);

  /**
  * Sets the Adjust function pointer according to the value of
  * m_iStochasticGrowthMethod.
  */
  void SetFunctionPointer();

  /**Function pointer for the appropriate Adjust function*/
  float (clNCIMasterQuadratGrowth::*Adjust)(float fNumber, int iSpecies);

};
//---------------------------------------------------------------------------
#endif
