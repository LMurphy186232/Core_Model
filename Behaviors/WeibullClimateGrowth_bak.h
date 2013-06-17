//---------------------------------------------------------------------------

#ifndef WeibullClimateGrowthH
#define WeibullClimateGrowthH
//---------------------------------------------------------------------------
#include "GrowthBase.h"


class clTree;
class clTreePopulation;

/**
* Weibull Climate growth - Version 1.0
*
* This is a growth shell object which calculates growth as a function of
* climate and neighbor density.
*
* The growth equation for one year's growth is:
* <br>
* <center>Growth = Max Growth * Size Effect * Precipitation Effect * Climate Effect * Competition Effect</center>
* <br>
* where Max Growth is annual amount of diameter growth, and the Effects are
* values between 0 and 1 which serve to reduce the maximum.
*
* The equation for Size Effect is:
* <center>SE = exp(-0.5(ln(DBH/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</center>
*
* where:
* <ul>
* <li>DBH is for the target tree, in cm</li>
* <li>X<sub>0</sub> is the size effect mode, in cm</li>
* <li>X<sub>b</sub> is the size effect variance, in cm</li>
* </ul>
*
* Size effect is subject to a minimum value for DBH, below which all trees will
* just get the minimum.
*
* Precipitation Effect and Temperature Effect use the same function form.
* The function is:
* <br>
* @htmlonly
  Climate Effect <- exp(-0.5*(abs(CV - C)/A)<sup>B</sup>)
  @endhtmlonly
* where:
* <ul>
* <li>Climate Effect is either Precipitation Effect or Temperature Effect</li>
* <li>CV is either the mean annual precipitation in mm or the mean annual
* temperature in C, from the plot object</li>
* <li>A, B, and C are parameters</li>
* </ul>
*
* Competition Effect is calculated as:
* <br>
* @htmlonly
  <center>CE = exp(-(C * DBH<sup> &gamma; </sup> * nd<sup>D</sup>)</center>
  @endhtmlonly
* <br>
* where:
* <ul>
* <li>CE = competition effect</li>
* <li>C is the competition effect C parameter</li>
* <li>D is the competition effect D parameter</li>
* <li>DBH is of the target tree, in cm</li>
* <li>nd is the number of neighbors with a DBH larger than both the minimum
* neighbor DBH parameter and the target tree DBH, within the specified
* radius</li>
* <li>@htmlonly &gamma; @endhtmlonly is the competition effect gamma
* parameter</li>
* </ul>
*
* Snags, seedlings, and already-dead trees are never counted in the neighbor
* count.
*
* The amount of growth is in cm/year. For multi-year timesteps, the behavior
* will calculate total growth with a loop. Each loop iteration will increment
* DBH for one year. For each year, the Size Effect (SE) value and the
* @htmlonly DBH<sup>&gamma;</sup> @endhtmlonly portion of the Competition Effect
* is recalculated with the previous year's new DBH value. All values for each
* year of growth are summed to get the growth for the timestep.
*
* This cannot be applied to seedlings.  An error will be thrown if seedlings
* are passed.
*
* The parameter file call string for this to be diameter-incrementing with
* auto-height updating is "WeibullClimateGrowth"; for diameter-only
* incrementing, use "WeibullClimateGrowth diam only".  The namestring for this
* behavior is "WeibullClimategrowthshell".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clWeibullClimateGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clWeibullClimateGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clWeibullClimateGrowth();

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
  * Calculates growth.  The values are stashed in the "Growth" tree float data
  * member for later application.
  *
  * Steps:
  * <ol>
  * <li>Calculate temperature and precipitation effects for each species for the
  * current values of the plot climate variables</li>
  * <li>Get all trees for this behavior.</li>
  * <li>For each tree, count the number of larger neighbors</li>
  * <li>Calculate the amount of growth for each using the equations above.
  * Stash the end result in "Growth".</li>
  * </ol>
  * This must be called first of any growth stuff, since it uses other trees'
  * DBHs to count neighbors, and this must be done before growth has been
  * applied.
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
  * <li>AssembleUniqueTypes() is called to create a list of unique behavior
  * types.</li>
  * <li>ReadParameterFile() is called to read the parameter file's data.</li>
  * <li>ValidateData() is called to validate the data.</li>
  * <li>GetGrowthCodes() is called to get tree data return codes.</li>
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

  protected:

  /**Holds return data codes for the "Growth" tree data member.  Array size is
  * number of species to which this behavior applies by 2 (saplings and
  * adults).*/
  short int **mp_iGrowthCodes;

  /**Competition effect C. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fCompC;

  /**Competition effect D. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fCompD;

  /**Size sensitivity to crowding parameter.
  * @htmlonly &gamma; @endhtmlonly in Competition Effect equation above. Array
  * is sized number of species to which this behavior applies.*/
  float *mp_fGamma;

  /**Maximum search radius, in meters, in which to look for crowding
  * neighbors. For calculating the Competition Effect. Array is sized number of
  * species to which this behavior applies.*/
  //float *mp_fMaxCrowdingRadius;

  /**The minimum DBH, in cm, of neighbors to be included in the neighbor count.
  * Array is sized number of species.*/
  //float *mp_fMinimumNeighborDBH;

  /**Size effect X0. This must be an array of doubles in order to support very
   * small values. Array is sized number of species to which this behavior
   * applies.*/
  //double *mp_fSizeX0;

  /**Size effect Xb. Array is sized number of species to which this behavior
   * applies.*/
  //float *mp_fSizeXb;

  /**Size effect minimum DBH. Array is sized number of species to which this
   * behavior applies.*/
  //float *mp_fSizeMinDBH;

  /**Precipitation effect A. Array is sized number of species to which this
   * behavior applies.*/
  //float *mp_fPrecipA;

  /**Precipitation effect B. Array is sized number of species to which this
   * behavior applies.*/
  //float *mp_fPrecipB;

  /**Precipitation effect C.  Array is sized number of species to which this
   * behavior applies.*/
  //*mp_fPrecipC;

  /**Temperature effect A.  Array is sized number of species to which this
   * behavior applies.*/
  //float *mp_fTempA;

  /**Temperature effect B. Array is sized number of species to which this
   * behavior applies.*/
  //float *mp_fTempB;

  /**Temperature effect C. Array is sized number of species to which this
   * behavior applies.*/
  //float *mp_fTempC;

  /**Maximum potential growth value, in cm. Array is sized number of species
  * to which this behavior applies.*/
  float *mp_fMaxRG;

  /**Speeds access to the arrays. Array size is number of species.*/
  short int *mp_iIndexes;

  /**Query string to get target trees*/
  char *m_cQuery;

  /**Minimum sapling height. For doing neighbor searches.*/
  //float m_fMinSaplingHeight;

  /**Keep our own copy for the destructor. This is the total number of tree
  * species.*/
  short int m_iNumTotalSpecies;

  /**
  * Makes sure all input data is valid. The following must all be true:
  * <ul>
  * <li>Max radius of neighbor effects must be > 0</li>
  * <li>Max growth for each species must be > 0</li>
  * <li>Temp and precip A for each species must be > 0</li>
  * <li>X0 (size effect mode) for each species must be > 0</li>
  * <li>Xb (size effect variance) for each species must not = 0</li>
  * </ul>
  * @throws modelErr if any of the above conditions are not met.
  */
  void ValidateData();

  /**
  * Gets the return codes for the "Growth" float tree data member for each tree
  * kind to which this behavior applies.
  * @throws modelErr if a code comes back -1 for any species/type combo to
  * which this behavior is applied.
  */
  void GetGrowthCodes();

  /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input tree.
  * @throws modelErr if this behavior has been applied to any types except
  * sapling and adult.
  */
  void ReadParameterFile( xercesc::DOMDocument *p_oDoc );

    /**
  * Counts the number of trees a target's neighborhood that have a DBH bigger
  * than the target.
  * @param p_oTarget Target tree for which to count the larger neighbors.
  * @param p_oPop Tree population, for getting neighbors.
  * @returns Number of larger neighbors.
  */
  int GetNumLargerNeighbors(clTree *p_oTarget, clTreePopulation *p_oPop);

  /**
  * Populates m_cQuery with the query for getting target trees.
  */
  void FormatQuery();
};
//---------------------------------------------------------------------------
#endif
