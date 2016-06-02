//---------------------------------------------------------------------------
// LaggedPostHarvestGrowth
//---------------------------------------------------------------------------

#ifndef LaggedPostHarvestGrowth_H
#define LaggedPostHarvestGrowth_H
//---------------------------------------------------------------------------
#include "GrowthBase.h"
#include "Grid.h"


class clTree;
class clTreePopulation;


/**
* Lagged Post-Harvest Growth - Version 1.0
*
* This is a growth shell object that incorporates a lag period after
* harvest during which growth increases gradually to a higher value.
*
* Diameter growth of tree <i>i</i> before (<i>G<sub>pre</sub></i>) and well after
* harvest (<i>G<sub>post</sub></i>) is calculated as:
*
* @htmlonly
* <center>
* G<sub>pre/post</sub> = MG * exp(-p<sub>2</sub>NCI<sub>i</sub>)
* </center>
* Where Maximum growth (MG) is:
* <center>
* MG = p<sub>0</sub> * exp(-p<sub>1</sub> * dbh<sub>i</sub>)
* </center>
* and NCI<sub>i</sub> is:
*
* NCI<sub>i</sub> = (&Sigma;  BA<sub>n</sub>) * exp(-p<sub>3</sub>*dbh<sub>i</sub>)
* <ul>
* <li><i>dbh</i> is the diameter at breast height of the tree, in cm.
* <li><i>BA<sub>n</sub></i> is the basal area, in m<sup>2</sup>, of all adult trees within <i>d</i> m of the tree.
* </ul>
* @endhtmlonly
*
* Growth <i>YSH</i> years after harvest is calculated as:
*
* @htmlonly
* <center>
* G = G<sub>pre</sub> + (G<sub>post</sub> - G<sub>pre</sub>)*(1-exp(-p<sub>4</sub>*YSH))
* </center>
* @endhtmlonly
*
*
* The amount of growth is in cm/year.  For multi-year timesteps, the behavior
* will calculate total growth by multiplying by the number of years per timestep.
*
* This cannot be applied to seedlings.  An error will be thrown if seedlings
* are passed.
*
* This creates one new tree float data member called "PreHarvGr".
*
* This also creates a grid called "Years Since Last Harvest".
*
* The parameter file call string for this to be diameter-incrementing with
* auto-height updating is "LaggedPostHarvestGrowth"; for diameter-only
* incrementing, use "LaggedPostHarvestGrowth diam only".  The namestring for
* this behavior is "laggedpostharvestgrowthshell".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/

class clLaggedPostHarvestGrowth : virtual public clGrowthBase {
	//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clLaggedPostHarvestGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clLaggedPostHarvestGrowth();

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
  * Calculates growth for all trees.  The values are stashed in the
  * "Growth" tree float data member for later application.
  *
  * Steps:
  * <ol>
  * <li>Get all trees for this behavior.</li>
  * <li>For each tree, calculate the amount of growth for
  * each using the equations above.  Stash the end result in
  * "Growth".</li>
  * </ol>
  * This must be called before any growth stuff because it uses nearby BA,
  * and this must be calculated before growth has been applied.
  *
  * Growth per timestep is calculated by multiplying by the number of years
  * per timestep.
  *
  * @param p_oPop Tree population object.
  */
  void PreGrowthCalcs( clTreePopulation *p_oPop );

  /**
  * Does setup.
  * <ol>
  * <li>AssembleUniqueTypes() is called to create a list of unique behavior
  * types.</li>
  * <li>ReadParameterFile() is called to read the parameter file's data.</li>
  * <li>ValidateData() is called to validate the data.</li>
  * <li>GetTreeMemberCodes() is called to get tree data return codes.</li>
  * <li>SetupTimeSinceHarvestGrid() is called to create a time since harvest grid, if it doesn't already exist.</li>
  * </ol>
  *
  * @param p_oDoc DOM tree of parsed input tree.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);



  /**
  * Captures the namestring passed to this behavior.  This is overridden from
  * clBehaviorBase so we can capture the namestring passed.  Since this class
  * can create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

  /**
   * Registers the "PreHarvGr" tree float data member.
   */
  void RegisterTreeDataMembers();

  protected:

  short int **mp_iGrowthCodes; /**<Holds return data codes for the "Growth"
  tree data member.  Array size is number of species to which this behavior
  applies by 2 (saplings and adults).*/
  short int *mp_iWhatBehaviorTypes; /**<List of types managed by this behavior.*/
  short int m_iNumBehaviorTypes; /**<Number of types managed by this behavior.*/


  /**Distance (m) from a given tree out to which local basal area is calculated.*/
  double m_fNciDistanceRadius;

    /**Maximum growth constant @htmlonly , p<sub>0</sub> @endhtmlonly , in growth equation.  Array assumed to be sized number of species to
  which this behavior applies.  This array is accessed by using the index
  returned for mp_iIndexes[species number].*/
  double *mp_fMaxGrowthConstant;

    /**Maximum growth DBH effect @htmlonly , p<sub>1</sub> @endhtmlonly , in growth equation.  Array assumed to be sized number of species to
  which this behavior applies.  This array is accessed by using the index
  returned for mp_iIndexes[species number].*/
  double *mp_fMaxGrowthDbhEffect;

    /**NCI constant @htmlonly , p<sub>2</sub> @endhtmlonly , in growth equation.  Array assumed to be sized number of species to
  which this behavior applies.  This array is accessed by using the index
  returned for mp_iIndexes[species number].*/
  double *mp_fNciConstant;

    /**NCI DBH effect @htmlonly , p<sub>3</sub> @endhtmlonly , in growth equation.  Array assumed to be sized number of species to
  which this behavior applies.  This array is accessed by using the index
  returned for mp_iIndexes[species number].*/
  double *mp_fNciDbhEffect;

    /**Rate parameter @htmlonly , p<sub>4</sub> @endhtmlonly , which determines how quickly after harvest
     * growth increases to it's new value.  Array assumed to be sized
  * number of species to which this behavior applies.  This array is accessed
  * by using the index returned for mp_iIndexes[species number].
  */
  double *mp_fTimeSinceHarvestRateParam;



    /**Speeds access to the arrays.  Array size is assumed to be number of
  * species.*/
  short int *mp_iIndexes;


  /**Keep our own copy for the destructor.  This is the total number of tree
  * species.*/
  short int m_iNumTotalSpecies;

  /**Codes for pre-harvest growth data member.*/
  short int **mp_iPreHarvGrowthCodes;

  /**
  * Makes sure all input data is valid.  The following must all be true:
  * <ul>
  * <li>Maximum potential growth (MPG) must be non-negative.</li>
  * <li>Time since harvest rate parameter must be greater than 0.</li>
  * <li>The basal area distance radius must be greater than 0.</li>
  *
  * </ul>
  * @throws modelErr if any of the above conditions are not met.
  */
  void ValidateData();

  /**
  * Gets the return codes for needed tree "Growth" data members.
  * @throws modelErr if a code comes back -1 for any species/type combo to
  * which this behavior is applied.
  */
  void GetTreeMemberCodes();

  /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input tree.
  * @throws modelErr if this behavior has been applied to any types except
  * sapling and adult.
  */
  void ReadParameterFile( xercesc::DOMDocument *p_oDoc );

  /**
  * Assembles a unique list of types applied to this behavior and places it in
  * mp_iWhatBehaviorTypes.
  */
  void AssembleUniqueTypes();


  /**Code for Harvest Type in Harvest Results grid.**/
  int m_iHarvestTypeCode;
  /**Code for Time (since last harvest) in the Years Since Last Harvest grid.**/
  int m_iTimeCode;
  /**Code for LastUpdated (timestep) in the Years Since Last Harvest grid.**/
  int m_iLastUpdated;
  /**Number of X cells in grids.**/
  int m_iNumXCells;
  /**Number of Y cells in grids.**/
  int m_iNumYCells;

  /**Pointer to Years Since Last Harvest grid.**/
  clGrid *mp_oTimeSinceHarvestGrid;
  /**Pointer to Harvest Results grid.**/
  clGrid *mp_oHarvestResultsGrid;

  int m_iNumberYearsPerTimestep; /**<Number of years per timestep*/

  /**
  * This creates the Years Since Harvest grid at the beginning of the simulation if it doesn't
  * already exist, and sets pointers to grids and saves other relevant grid info in class members.
  */
   	void SetupTimeSinceHarvestGrid();


  /**
  * This calculates the basal area (in m2) of all trees within a certain distance of
  * the tree passed as an argument.
  * @param p_oTree Tree being evaluated
  * @return The local basal area of nearby trees.
  */
   	float LocalBasalAreaAroundTree( clTree *p_oTree );


  /**
   * Returns the Time value from the Years Since Harvest grid at the location of a tree. If
   * there has been no harvest in the tree's grid cell, a value of 1000 will be returned.
   * @param p_oTree Tree being evaluated
   * @return The time since harvest, in years, at the tree's location.
   */

    int GetTimeSinceHarvest( clTree *p_oTree );

  /**
  * Updates the Years Since Last Harvest grid, if it hasn't already been updated.
  */
  	void CalcTimeSinceHarvest();

};
//---------------------------------------------------------------------------
#endif
