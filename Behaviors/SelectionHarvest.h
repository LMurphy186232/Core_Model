//---------------------------------------------------------------------------

#ifndef SelectionHarvest
#define SelectionHarvest
//---------------------------------------------------------------------------
#include "BehaviorBase.h"
#define NUM_SIZE_CLASSES 4

class clGrid;
class clPackage;
class clTree;
class clPopulationBase;

/**
 * SelectionHarvest - Version 1.0
 *
 * In the case that the parameter file specifies a target basal area, rather
 * than an absolute value or percentage value, this behaviour will calculate the
 * basal area to be removed based as the difference betweent the target basal
 * area the actual basal area (the amount present prior to harvest).(JM)
 *
 * This behaviour will be able to set a target basal area for each of the 4 size
 * classes. The behaviour will determine the target basal areas in one of two
 * ways. First, if the user specifies their target basal area in all four size
 * classes, the behaviour will calculate the difference, for each size class,
 * between the actual basal area and the target basal area. That delta will be
 * the basal area to remove from each size class. The second option is for the
 * user to provide a 'Q-ratio' and a total target basal area for the 4 size
 * classes combined. These values will be set in the parameter file. The
 * combination of these two values is used to determine the target basal area in
 * each size class based on the exponential distribution. So, regardless of the
 * method, this behaviour will calculate an absolute amount of basal area to
 * remove from each size class, based on the current basal area in each grid
 * cell.(JM)
 *
 * Prior to each harvest event, this behaviour will compute the actual basal in
 * each grid cell prior to harvest.  Then, based on the user input target BA
 * values, or the Q-ratio method, the behaviour will determine how much basal
 * area must be removed from each size class in order to meet the target basal
 * area.(JM)
 *
 * If by chance, the current basal area of the system in a size class is less
 * than the target basal area, no trees will be harvested from that size class.
 * Furthermore, the shortage will not be made up for by overharvesting another
 * size class.(JM)
 *
 * This behaviour will then create a set of packages for the harvest grids. Once
 * the packages are created, the behaviour will insert them into the proper
 * place in the harvest grids.(JM)
 *
 * The namestring and parameter file call string for this behavior are both
 * "SelectionHarvest". (LEM)
 *
 * Copyright 2011 Charles D. Canham.
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */
class clSelectionHarvest : virtual public clBehaviorBase {

public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clSelectionHarvest(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clSelectionHarvest();

  /**
   * Performs the selection cut setup for a timestep. For a timestep, this
   * function will call CalculateBasalAreaDifference() and
   * TellHarvestToCutTrees() for each selection cut for this timestep.
   */
  void Action();

  /**
   * Determines the time that has elapsed since the last harvest for each cell
   * in the harvest results grid.
   */
  void GetTimeSinceHarvest();

  /**
   * Types of harvest cuts.  These are listed in increasing order of severity
   * (that's important!)
   */
  enum cutType {partial,  /**<Partial cut*/
               gap,       /**<Gap cut*/
               clear};    /**Clear cut*/

  /**How amount to cut is defined.*/
  enum amtType {percentBA,   /**<As a percentage of total basal area in cut range*/
               absBA,       /**<As an amount of BA in sq m in that cut range*/
               percentDen,  /**<As a percentage of total density in that cut range*/
               absDen};     /**<As a total number of trees in that cut range*/


protected:

  /**The lower bound for each DBH size class*/
  float *mp_fLowDBH;

  /**The upper bound for each DBH size class*/
  float *mp_fHighDBH;

  /**To hold the target basal area for each size class, in m2/ha*/
  float *mp_fTargetBA;

  /**Holds the basal area for each size class for all species across the landscape*/
  float *mp_fLandscapeBasalArea;

  /**Holds the basal area of 1 species for each size class*/
  float *mp_fBasalArea;

  /**Used to ensure that TargetBA does not get overwritten in the
   * CalculateBasalAreaDifference function*/
  float *mp_fTempTargetBA;

  /**Plot area, in hectares*/
  float m_fPlotArea;

  /**The array into which GetBasalArea will store the Basal area values for
   * each size class*/
  float m_fTotalBasalArea[][NUM_SIZE_CLASSES];

  /**The array into which CalculateBasalAreaDifference will place the absolute
   * basal area to remove from each size class*/
  float m_fBasalAreaDifference[][NUM_SIZE_CLASSES];


  clPackage *mp_oOldPackage,   /**<For finding where to place a new package*/
            *mp_oNewPackage;   /**<A newly created package in the grids*/

  clTreePopulation *mp_oPop;   /**<Stashed pointer to tree population*/

  /**
   * HARVEST MASTER CUTS
   *
   * The grid called "harvestmastercuts" will have a single cell. This is where
   * the cut events are defined.
   *
   * Each cut event is one package and is applied to a list of species. It has a
   * timestep, cut amount, cut type, and number of cut ranges defined. It also
   * has a unique ID number.  The grid cells to which it is applied are in the
   * "harvestcutevents" grid.
   *
   * For the cut ranges, four dbh ranges can be defined to which the cuts will
   * be applied.  The ranges are ordered as 1 is the smallest minimum dbh.
   * Ranges cannot overlap.
   *
   * The packages are in timestep order, earliest first.
   *
   * It is possible that a grid map will have been read in for this grid; it
   * will be ignored, and any grid created with such a map will be overwritten.
   *
   * Data members - all for packages:
   * <table>
   * <tr>
   * <th>Data member</th> <th>Data</th> <th>Description</th>
   * </tr>
   * <tr>
   * <td>id</td>          <td>int</td>  <td>ID number of cut</td>
   * </tr>
   * <tr>
   * <td>timestep</td>    <td>int</td>  <td>Timestep at which to apply the
   *                                    cut</td>
   * </tr>
   * <tr>
   * <td>cuttype</td>     <td>int</td>  <td>Matches a value of the enum
   *                                    "cutType"</td>
   * </tr>
   * <tr>
   * <td>amttype</td>     <td>int</td>  <td>Matches a value of the enum
   *                                    "amtType"</td>
   * </tr>
   * <tr>
   * <td>species(x)</td>  <td>bool</td> <td>One of each of these for each
   *                                    species. If true, this species is being
   *                                    cut.</td>
   * </tr>
   * <tr>
   * <td>rangeamt(x)</td> <td>float</td><td>Range amount to cut. There are one
   *                                    of each of the following for
   *                                    m_iNumAllowedCutRanges, where x is the
   *                                    index, starting at 0 (thus "rangemin0"
   *                                    and "rangeamt2").</td>
   * </tr>
   * <tr>
   * <td>rangemin(x)</td> <td>float</td><td>Range minimum dbh value. There are
   *                                    one of each of the following for
   *                                    m_iNumAllowedCutRanges, where x is the
   *                                    index, starting at 0.</td>
   * </tr>
   * <tr>
   * <td>rangemax(x)</td> <td>float</td><td>Range maximum dbh value. There are
   *                                    one of each of the following for
   *                                    m_iNumAllowedCutRanges, where x is the
   *                                    index, starting at 0.</td>
   * </tr>
   * </table>
   */
  clGrid *mp_oMasterCutsGrid;

  /**
   * HARVEST CUT EVENTS
   *
   * The grid called "harvestcutevents" will have a cell resolution that matches
   * the tree population.  This is where data about harvest cutting events is
   * stored.
   *
   * The need to cut a grid cell is signaled by the presence of a package.  The
   * package has an ID number which matches a package in the "harvestmastercuts"
   * grid, which contains the information about how the cut is actually to be
   * performed. Packages are in timestep order, earliest first.
   *
   * It is possible that a grid map will have been read in for this grid; it
   * will be ignored, and any grid created with such a map will be overwritten.
   *
   * Data members - all for packages:
   *
   * <table>
   * <tr>
   * <th>Data member</th> <th>Data</th> <th>Description</th>
   * </tr>
   * <tr>
   * <td>id</td>          <td>int</td>  <td>ID number matching master package in
   *                                    "harvestmastercuts"</td>
   * </tr>
   * <tr>
   * <td>timestep</td>    <td>int</td>  <td>Timestep at which to apply the
   *                                    cut</td>
   * </tr>
   * </table>
   */
  clGrid *mp_oCutEventsGrid;

  /**
   * RESULTS GRID
   *
   * If this behavior is harvesting, the grid is called "Harvest Results"; if
   * it is episodic mortality, the grid is called "Mortality Episode Results".
   * The grid has a cell resolution matching that of "harvestcutevents/
   * mortepisodecutevents".  This is where data on actual cut/kill results is
   * stored.  The data is stored raw - no conversion to per-hectare amounts.
   *
   * It is possible that a grid map will have been read in for this grid; it
   * will be ignored, and any grid created with such a map will be overwritten.
   * <table>
   * <tr>
   * <th>Data member</th>     <th>Data type</th> <th>Description</th>
   * </tr>
   * <tr>
   * <td>Harvest Type</td>    <td>int</td>       <td>Type of harvest that
   *                                             occurred in the current
   *                                             timestep - -1 if none has
   *                                             occurred.</td>
   * </tr>
   * <tr>
   * <td>Cut Density_x_sp</td><td>int</td>       <td>Number of trees cut in the
   *                                             current timestep. There are one
   *                                             of each of the following for
   *                                             m_iNumAllowedCutRanges times
   *                                             number of species, where x is
   *                                             the cut range index, starting
   *                                             at 0, and sp is the species
   *                                             number.</td>
   * </tr>
   * <tr>
   * <td>Cut Basal Area_x_sp</td><td>float</td>  <td>Total basal area cut in the
   *                                             current timestep. There are one
   *                                             of each of the following for
   *                                             m_iNumAllowedCutRanges times
   *                                             number of species, where x is
   *                                             the cut range index, starting
   *                                             at 0, and sp is the species
   *                                             number.</td>
   * </tr>
   * </table>
   */
  clGrid *mp_oResultsGrid;

  /**
   * TIME SINCE HARVEST
   *
   * The grid called "timesinceharvest" will have a cell resolution that matches
   * the harvest results grid. This is where the time since the last harvest is
   * stored.
   *
   * Data members - all for packages:
   *
   * <table>
   * <tr>
   * <th>Data member</th><th>Data type</th> <th>Description</th>
   * </tr>
   * <tr>
   * <td>Time</td>       <td>int</td>       <td>Time since last harvest</td>
   * </tr>
   * </table>
   */

  clGrid *mp_oTimeSinceHarvestGrid;


  //These hold the return codes for the data members
  int m_iInitialAge; /**<Initial age*/

  /**timestep data member code in "harvestmastercuts" grid*/
  short int m_iMasterTimestepCode;
  /**id data member code in "harvestmastercuts" grid*/
  short int m_iMasterIDCode;
  /**Species data member code in "harvestmastercuts" grid. Array size is number
   * species. Array index matches species number*/
  short int *mp_iSpeciesCodes;
  /**cuttype data member code in "harvestmastercuts" grid*/
  short int m_iCutTypeCode;
  /**amttype data member code in "harvestmastercuts" grid*/
  short int m_iAmountTypeCode;
  /**rangeminx data member codes in "harvestmastercuts" grid. Array size is
   * number cut ranges*/
  short int *mp_iRangeMinCodes;
  /**rangemaxx data member codes in "harvestmastercuts" grid. Array size is
   * number cut ranges*/
  short int *mp_iRangeMaxCodes;
  /**rangeamtx data member codes in "harvestmastercuts" grid. Array size is
   * number cut ranges*/
  short int *mp_iRangeAmountCodes;

  /**timestep data member code in "harvestcutevents" grid*/
  short int m_iCutTimestepCode;
  /**id data member code in "harvestcutevents" grid*/
  short int m_iCutIDCode;

  /**Harvest Type data member code in "Harvest Results" grid*/
  short int m_iHarvestTypeCode;
  /**dencut data member codes in "Harvest Results" grid. Array size is number
   * cut ranges by number of species*/
  short int **mp_iDenCutCodes;
  /**bacut data member codes in "Harvest Results" grid. Array size is number
   * cut ranges by number of species*/
  short int **mp_iBaCutCodes;

  /**Data member return code for "time" data member of "timesinceharvest" grid*/
  short int m_iTime;

  /**Number of cut ranges allowed.*/
  short int m_iNumAllowedCutRanges;
  /**Total number of species*/
  short int m_iNumSpecies;

  /**
   * Reads all the harvest events from the parameter file.  It doesn't perform
   * validation on them other than basic data types - more in-depth logical
   * validation is left to ValidatePackages().
   * @param p_oDoc DOM tree of parsed input file.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Gets the basal area in a cut area for all species.  This gets the basal
   * area in each cut range and puts it in an array whose pointer was passed.
   * @param p_oTreePop Pointer to the tree population that will have the
   * selection cut applied to it.
   * @param p_fTotalBasalArea Array (size m_iNumAllowedCutRanges) in which to
   * put the total basal area calculation.
   * @param p_fLoDbh Array (size m_iNumAllowedCutRanges) of lower-limit dbhs for
   * the cut ranges that have been defined.
   * @param p_fHiDbh Array (size m_iNumAllowedCutRanges) of upper-limit dbhs for
   * the cut ranges that have been defined.
   */
  void GetBasalArea(clTreePopulation *p_oTreePop, float *p_fTotalBasalArea,
      float *p_fLoDbh, float *p_fHiDbh);

  /**
   * Gets the basal area in a cut area for a single species.  The cut area is
   * defined by a linked list of grid cells.  This gets the basal area in each
   * cut range and puts it in an array whose pointer was passed.
   *
   * @param p_oTreePop Tree population
   * @param iSpecies Species of tree for which to calculate basal area.
   * @param p_fTotalBasalArea Array (size m_iNumAllowedCutRanges) in which to
   * put the total basal area calculation.
   * @param p_fLoDbh Array (size m_iNumAllowedCutRanges) of lower-limit dbhs for
   * the cut ranges that have been defined.
   * @param p_fHiDbh Array (size m_iNumAllowedCutRanges) of upper-limit dbhs for
   * the cut ranges that have been defined.
   */
  void GetBasalArea(clTreePopulation *p_oTreePop, int iSpecies, float *p_fTotalBasalArea, float *p_fLoDbh, float *p_fHiDbh);

  /**
   * Determines the dbh to cut in each grid cell by calculating the difference
   * between the target basal area and the actual current basal area. The
   * difference is stored in an array whose pointer is passed to the function.
   *
   * mp_fLanscapeBasalArea[] will contain the amount of basal area to remove
   * from each size class.  this value is not per hectare, but for the whole
   * plot.
   * @param p_fCurrentBasalArea Pointer to array containing the basal area in
   * each size class
   */
  int CalculateBasalAreaDifference(float *p_fCurrentBasalArea);

  /**
   * This function will use the values extracted from the parameter file in
   * order to create a package which will be inserted into the master cut events
   * grid.
   */
  void CreateMasterCutPackage(int iSpecies);

  /**
   * This function will edit the cut events grid so that it holds ID and
   * timestep values for the selection harvest created in the behaviour. The
   * function will traverse through every cell in the cut event grid and create
   * a package containing the selection harvest ID and a timestep.
   */
  void EditCutEventsGrid();

  /** Obtains a pointer to the harvest grids and gets all the return codes*/
  void GetHarvestGrids();

  /**
   * Makes sure that the data in the event harvest packages makes sense. For
   * each cut event (each package) - this checks the following:
   * <ul>
   * <li>No negative values in cut ranges
   * <li>Cut ranges do not overlap
   * </ul>
   */
   void ValidatePackages();
};
//---------------------------------------------------------------------------
#endif

