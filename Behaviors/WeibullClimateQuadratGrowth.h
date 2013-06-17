//---------------------------------------------------------------------------

#ifndef WeibullClimateQuadratGrowthH
#define WeibullClimateQuadratGrowthH
//---------------------------------------------------------------------------
#include "GrowthBase.h"


class clTree;
class clTreePopulation;
class clGrid;

/**
* Weibull Climate growth - Version 1.0
*
* This is a growth shell object which calculates growth as a function of
* climate and neighbor density. This simplifies the process by calculating
* annual growth on a per quadrat basis.
*
* The growth equation for one year's growth is:
* <br>
* <center>Growth = Max Growth * Precipitation Effect * Climate Effect * Competition Effect</center>
* <br>
* where Max Growth is annual amount of diameter growth, and the Effects are
* values between 0 and 1 which serve to reduce the maximum.
*
* Precipitation Effect and Temperature Effect use the same function form. The
* function is:
* <br>
* Climate Effect <- exp(-0.5*(abs(CV - C)/A)<sup>B</sup>)
* where:
* <ul>
* <li>Climate Effect is either Precipitation Effect or Temperature Effect</li>
* <li>CV is either the mean annual precipitation in mm or the mean annual
* temperature in C, from the plot object</li>
* <li>A, B, and C are parameters </li>
* </ul>
*
* Competition Effect is calculated as:
* <br>
* <center>CE = exp(-(C * nd<sup>D</sup>)</center>
* <br>
* where:
* <ul>
* <li>CE = competition effect</li>
* <li>C is the competition effect C parameter</li>
* <li>nd is the number of neighbors with a DBH larger than the minimum
* neighbor DBH parameter, within the specified radius</li>
* </ul>
*
* Snags, seedlings, and already-dead trees are never counted in the neighbor
* count.
*
* The annual growth for each species is calculated for each grid cell in the
* "Weibull Climate Quadrat Growth" grid, which this behavior creates.  Trees
* take their growth from the grid cell in which they are found.
*
* The parameter file call string for this to be diameter-incrementing with
* auto-height updating is "WeibullClimateQuadratGrowth"; for diameter-only
* incrementing, use "WeibullClimateQuadratGrowth diam only".  The namestring for this
* behavior is "WeibullClimateQuadratGrowthshell".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clWeibullClimateQuadratGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clWeibullClimateQuadratGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clWeibullClimateQuadratGrowth();

  /**
  * Queries the grid for the growth value for this tree's cell that was
  * calculated by PreGrowthCalcs() and returns it.
  *
  * @param p_oTree Tree to which to apply growth.
  * @param p_oPop Tree population object.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount of diameter growth, in cm.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Calculates growth. This retrieves trees to which this behavior applies,
  * and for each one makes sure that growth has been calculated for its species
  * and grid cell.  This does not assign a growth value to the trees; that
  * happens in CalcDiameterGrowthValue. Doing it this way ensures two things:
  * that growth is not calculated if it is not needed, and that we don't have
  * to worry about accidentally assigning growth to a tree to which this
  * behavior is not applied.
  *
  * Steps:
  * <ol>
  * <li>Reset all growth values in the grid to -1.
  * <li>Calculate temperature and precipitation effects for each species for the
  * current values of the plot climate variables</li>
  * <li>Get all trees for this behavior.</li>
  * <li>For each tree, determine whether or not growth has been calculated
  * for the grid cell that it is in; if yes, do nothing </li>
  * <li>If growth needs to be calculated, count the number of neighbors</li>
  * <li>Calculate the amount of growth for each using the equations above.
  * Stash the end result in "Growth".</li>
  * </ol>
  * This must be called first of any growth stuff, since it uses other trees'
  * DBHs to count neighbors, and this must be done before growth has been
  * applied.
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
  * <li>SetupGrid() is called to create the growth grid.</li>
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

  /**
   * Grid holding amount of growth for each species.  The grid name is
   * "Weibull Climate Quadrat Growth". It has X+1 float data members, where X =
   * the total number of species.  The data member names are "growth_x", for the
   * amount of diameter growth (where "x" is the species number), and
   * "num_neigh" for the number of neighbors.
   */
  clGrid* mp_oGrid;

  /**Competition effect C. Array size is number of species to which this
   * behavior applies.*/
  float *mp_fCompC;

  /**Competition effect D. Array size is number of species to which this
   * behavior applies.*/
  float *mp_fCompD;

  /**The minimum DBH, in cm, of neighbors to be included in the neighbor count.
  * Array is sized number of species.*/
  float *mp_fMinimumNeighborDBH;

   /**Precipitation effect A. Array size is number of species to which this
   * behavior applies.*/
  float *mp_fPrecipA;

  /**Precipitation effect B. Array size is number of species to which this
   * behavior applies.*/
  float *mp_fPrecipB;

  /**Precipitation effect C. Array size is number of species to which this
   * behavior applies.*/
  float *mp_fPrecipC;

  /**Temperature effect A. Array size is number of species to which this
   * behavior applies.*/
  float *mp_fTempA;

  /**Temperature effect B. Array size is number of species to which this
   * behavior applies.*/
  float *mp_fTempB;

  /**Temperature effect C. Array size is number of species to which this
   * behavior applies.*/
  float *mp_fTempC;

  /**Maximum potential growth value, in cm. Array is sized number of species
  * to which this behavior applies.*/
  float *mp_fMaxRG;

  /**Speeds access to the arrays. Array size is number of species.*/
  short int *mp_iIndexes;

  /** Holds data member codes for the "growth_x" data members of the "Weibull
   * Climate Quadrat Growth" grid.  Array size is total # species.*/
  short int *mp_iGridGrowthCodes;

  /**Query string to get target trees*/
  char *m_cQuery;

  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Maximum search radius, in meters, in which to look for crowding
  * neighbors. For calculating the Competition Effect.*/
  float m_fMaxCrowdingRadius;

  /**Return code for the "num_neigh" grid data member.*/
  short int m_iNumNeighCode;

  /**
  * Makes sure all input data is valid. The following must all be true:
  * <ul>
  * <li>Max radius of neighbor effects must be > 0</li>
  * <li>Max growth for each species must be > 0</li>
  * <li>Temp and precip A for each species must be > 0</li>
  * </ul>
  * @param p_oPop Tree population object.
  * @throws modelErr if any of the above conditions are not met.
  */
  void ValidateData( clTreePopulation *p_oPop );

  /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input tree.
  * @param p_oPop Tree population object.
  * @throws modelErr if this behavior has been applied to any types except
  * sapling and adult.
  */
  void ReadParameterFile( xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop );

    /**
  * Counts the number of trees a target's neighborhood that have a DBH bigger
  * than the minimum cutoff.
  * @param fX X location from  which to count the larger neighbors.
  * @param fY Y location from  which to count the larger neighbors.
  * @param p_oPop Tree population, for getting neighbors.
  * @returns Number of larger neighbors.
  */
  int GetNumNeighbors(float &fX, float &fY, clTreePopulation *p_oPop);

  /**
  * Populates m_cQuery with the query for getting target trees.
  * @param p_oPop Tree population object.
  */
  void FormatQuery( clTreePopulation *p_oPop );

  /**
  * Sets up the "Weibull Climate Quadrat Growth" grid.  This ignores any maps.
  * @param p_oPop Tree population object.
  */
  void SetupGrid( clTreePopulation *p_oPop );
};
//---------------------------------------------------------------------------
#endif
