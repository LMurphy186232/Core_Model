//---------------------------------------------------------------------------

#ifndef DisturbanceH
#define DisturbanceH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"

class clGrid;
class clPackage;
class clTree;

using namespace whyDead;

/**
* Disturbance - Version 2.1
*
* This class allows for highly specific disturbance events. Two kinds of
* disturbance can be performed: harvest or episodic mortality (disease,
* insect outbreaks, etc). They work the exact same way, differing only in the
* reason code passed to the tree population when killing the tree (Harvest in
* the case of harvest, and disease in the case of episodic mortality).
*
* Harvest/mortality episodes can be defined on a grid cell level for each
* timestep for each species. If more than one event is defined for a species
* for a single grid cell and timestep, the user takes a chance on what happens.
* Cells with common disturbance criteria are grouped by the user into groups
* which will be evaluated together.
*
* If the disturbance is harvest, there are three kinds of harvests that can be
* performed: gap cut, partial cut, and clear cut. Partial cuts may remove
* only a portion of the trees, but otherwise, there is no difference in how the
* trees are removed for each cut type. The type of cut is recorded for the
* benefit of other behaviors for whom this might be important. The user can
* define different ranges of dbh values to cut, and each can have a portion of
* the trees removed, as defined by percentage of basal area, percentage of
* density, absolute amount of basal area (in square meters per hectare), or
* absolute amount of density (in stems per hectare). For gap cut and clear
* cut, for all species affected, all trees are removed. Any attempt to define
* cut ranges etc. for gap and clear cuts will cause an error to be thrown (the
* cut ranges could be ignored but throwing an error alerts the user in case
* they didn't know, so they could amend the parameter file to get what they
* meant to get).
*
* Trees can be cut either largest to smallest or smallest to largest.
*
* Trees can be prioritized by any tree data member. Trees meeting the criterion
* will be cut before any others. There can be up to three priorities. The
* trees meeting each priority will be exhausted before moving to the next
* priority. If a priority does not apply to a tree or cut, it will be ignored.
* Prioritizing data members cannot be of type char. Priorities are not allowed
* in percent of density cut types because they would be nonsensical.
*
* Harvests can affect seedlings as well, by randomly killing a proportion of
* those found in the cut area. The kill rate is species specific and does not
* have to be the same as the species being harvested.
*
* Episodic mortality events are processed like harvest partial cuts.
*
* The disturbance event data is stored in a grid. Each cut event, defined as
* the killing to be done for one species for one timestep for one grid cell,
* is stored in a grid package. These packages are sorted in timestep order.
* Each timestep, all the cut events that are defined for that timestep are
* executed, then those packages are removed.
*
* This behavior can be called from the parameter file by either "Harvest" or
* "EpisodicMortality". The namestring is "harvest".
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
* <br>February 23, 2012 - Added priorities (LEM)
* <br>April 18, 2017 - Allowed reverse cut order (LEM)
*/
class clDisturbance : virtual public clBehaviorBase {

 public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clDisturbance(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clDisturbance();

  /**
   * Performs the harvest or mortality episode for a timestep.  This goes
   * through the master package lists in either the harvestmastercuts or
   * mortepisodemastercuts grid and calls CutTrees() for each one that is for
   * this timestep.
   */
  void Action();

  /**
   * Types of harvest cuts.  These are listed in increasing order of severity
   * (that's important!)
   */
  enum cutType {partial,  /**<Partial cut*/
                gap,      /**<Gap cut*/
                clear     /**<Clear cut*/
 };

  /** How amount to cut is defined. */
  enum amtType {percentBA,   /**<As a percentage of total basal area in cut range*/
               absBA,       /**<As an amount of BA in sq m in that cut range*/
               percentDen,  /**<As a percentage of total density in that cut range*/
               absDen       /**<As a total number of trees in that cut range*/
  };

  /** Tree data member type for priority. */
  enum priorityDataType {intType, floatType, boolType};

  /**
   * Gets the number of total allowed cut ranges.
   * @return Number of total allowed cut ranges.
   */
  static int GetNumberOfCutRanges() {return m_iNumAllowedCutRanges;};

 /**
  * Captures the parameter file behavior string passed to this behavior.  This
  * is overridden from clBehaviorBase so we can capture the namestring passed.
  * Since this class can create multiple kinds of behaviors that function
  * differently, this will capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's parameter file names.
  * @throws modelErr if the name is not recognized.
  */
  void SetNameData(std::string sNameString);


 protected:
  clTreePopulation *mp_oPop;   /**<Stashed pointer to tree population*/

  /**
  * MASTER CUTS GRID
  *
  * If this behavior is harvesting, the grid is called "harvestmastercuts"; if
  * it is episodic mortality, the grid is called "mortepisodemastercuts".  The
  * grid has a single cell.  This is where the cut events are defined.
  *
  * Grid cell size defaults to match the tree population. A map is allowed only
  * to set cell size.
  *
  * Each cut event is one package and is applied to a list of species.  It has a
  * timestep, cut amount, cut type, and number of cut ranges defined.  It also
  * has a unique ID number.  The grid cells to which it is applied are in either
  * the "harvestcutevents" or "mortepisodecutevents" grid, depending on what
  * this behavior is.
  *
  * Species are treated individually.  A cut event removing 20% of three species
  * removes 20% of each species individually; one that removes 5 sq.m/ha of
  * basal area removes 5 from each species.
  *
  * For the cut ranges, four dbh ranges can be defined to which the cuts will be
  * applied.  The ranges are ordered as 1 is the smallest minimum dbh.  Ranges
  * cannot overlap.
  *
  * The packages are in timestep order, earliest first.
  *
  * It is possible that a grid map will have been read in for this grid; it will
  * be ignored, and any grid created with such a map will be overwritten.
  *
  * Data members - all for packages:
  * <table>
  * <tr>
  * <th>Name</th>           <th>Type</th> <th>Description</th>
  * </tr>
  * <tr>
  * <td>id</td>             <td>int</td>  <td>ID number of cut</td>
  * </tr>
  * <tr>
  * <td>timestep</td>       <td>int</td>  <td>Timestep at which to apply
  *                                       the cut</td>
  * </tr>
  * <tr>
  * <td>cuttype</td>        <td>int</td>  <td>Matches a value of the enum
  *                                       "cutType"</td>
  * </tr>
  * <tr>
  * <td>amttype</td>        <td>int</td>  <td>Matches a value of the enum
  *                                       "amtType". Not used if
  *                                       episodic mortality</td>
  * </tr>
  * <tr>
  * <td>tallestfirst</td>   <td>bool</td> <td>Whether to cut the tallest
  *                                       trees first (true) or shortest (false)</td>
  * </tr>
  * <tr>
  * <td>species(x)</td>     <td>bool</td> <td>One of each of these for each
  *                                       species.  If true, this species is
  *                                       being cut.</td>
  * </tr>
  * <tr>
  * <td>rangeamt(x)</td>    <td>float</td><td>Range amount to cut. There are
  *                                       one of each of the following for
  *                                       m_iNumAllowedCutRanges, where x is the
  *                                       index, starting at 0 (thus "rangemin0"
  *                                       and "rangeamt2"). Units depend on
                                          amttype and are either % BA, % density,
                                          absolute amount BA (in square m/ha),
                                          or absolute amount of density (in
                                          stems per ha)</td>
  * </tr>
  * <tr>
  * <td>rangemin(x)</td>    <td>float</td><td>Range min dbh value. There are one
  *                                       of each of the following for
  *                                       m_iNumAllowedCutRanges, where x is the
  *                                       index, starting at 0.</td>
  * </tr>
  * <tr>
  * <td>rangemax(x)</td>    <td>float</td><td>Range max dbh value. There are one
  *                                       of each of the following for
  *                                       m_iNumAllowedCutRanges, where x is the
  *                                       index, starting at 0.</td>
  * </tr>
  * <tr>
  * <td>priorityname(x)</td><td>char</td> <td>The name of the tree data
  *                                       member for priority X.</td>
  * </tr>
  * <tr>
  * <td>prioritytype(x)</td><td>int</td>  <td>The tree data member type for
  *                                       priority X. Matches one of the values
  *                                       in enum priorityDataType.</td>
  * </tr>
  * <tr>
  * <td>prioritymin(x)</td> <td>float</td><td>Min value. A boolean should be 0
  *                                       or 1.</td>
  * </tr>
  * <tr>
  * <td>prioritymax(x)</td> <td>float</td><td>Max value. Can equal min value if
  *                                       only a single value is acceptable.</td>
  * </tr>
  * <tr>
  * <td>seedling(x)</td>    <td>float</td><td>Proportion (0-1) of seedlings
  *                                       of each species to destroy.</td>
  * </tr>
  * </table>
  */
  clGrid  *mp_oMasterCutsGrid;

  /**
  * CUT EVENTS GRID
  *
  * If this behavior is harvesting, the grid is called "harvestcutevents"; if
  * it is episodic mortality, the grid is called "mortepisodecutevents". This is
  * where data about cutting events is stored.
  *
  * Grid cell size matches the master cuts grid.
  *
  * The need to cut a grid cell is signaled by the presence of a package.  The
  * package has an ID number which matches a package in either the
  * "harvestmastercuts" or "mortepisodemastercuts" grid, which contains the
  * information about how the cut is actually to be performed.  Packages are in
  * timestep order, earliest first.
  *
  * It is possible that a grid map will have been read in for this grid; it will
  * be ignored, and any grid created with such a map will be overwritten.
  *
  * Data members - all for packages:
  *
  * <table>
  * <tr>
  * <th>Data member name</th><th>Data type</th><th>Description</th>
  * </tr>
  * <tr>
  * <td>id</td>              <td>int</td>      <td>ID number matching master
  *                                             package in "harvestmastercuts"</td>
  * </tr>
  * <tr>
  * <td>timestep</td>        <td>int</td>      <td>Timestep at which to apply
  *                                            the cut</td>
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
  * It is possible that a grid map will have been read in for this grid; it will
  * be ignored, and any grid created with such a map will be overwritten.
  * <table>
  * <tr>
  * <th>Data member name</th> <th>Data type</th> <th>Description</th>
  * </tr>
  * <tr>
  * <td>Harvest Type</td>     <td>int</td>       <td>Type of harvest that
  *                                              occurred in the current timestep -
  *                                              -1 if none has occurred. This
  *                                              data member is not registered
  *                                              if this grid is "Mortality
  *                                              Episode Results".</td>
  * </tr>
  * <tr>
  * <td>Cut Density_x_sp</td> <td>int</td>       <td>Number of trees cut in the
  *                                              current timestep. There are one
  *                                              of each of the following for
  *                                              m_iNumAllowedCutRanges times
  *                                              number of species, where x is
  *                                              the cut range index, starting
  *                                              at 0, and sp is the species
  *                                              number.</td>
  * </tr>
  * <tr>
  * <td>Cut Basal Area_x_sp</td><td>float</td>   <td>Total basal area cut in the
  *                                              current timestep. There are one
  *                                              of each of the following for
  *                                              m_iNumAllowedCutRanges times
  *                                              number of species, where x is
  *                                              the cut range index, starting
  *                                              at 0, and sp is the species
  *                                              number.</td>
  * </tr>
  * <tr>
  * <td>Cut Seedlings_sp</td> <td>int</td>      <td>Number of seedlings cut in
  *                                             the current timestep.</td>
  * </table>
  */
  clGrid *mp_oResultsGrid;

  //These hold the return codes for the data members

  //"harvestmastercuts/mortepisodemastercuts" grid data members
  /**Timestep data member code in "harvestmastercuts" grid*/
  short int m_iMasterTimestepCode;
  /**ID data member code in "harvestmastercuts" grid*/
  short int m_iMasterIDCode;
  /**Species data member code in "harvestmastercuts" grid. Array size is number
   * species. Array index matches species number*/
  short int *mp_iSpeciesCodes;
  /**cuttype data member code in "harvestmastercuts" grid*/
  short int m_iCutTypeCode;
  /**amttype data member code in "harvestmastercuts" grid*/
  short int m_iAmountTypeCode;
  /**tallestfirst data member code in "harvestmastercuts" grid*/
  short int m_iTallestFirstCode;
  /**rangeminx data member codes in "harvestmastercuts" grid. Array size is
   * number cut ranges*/
  short int *mp_iRangeMinCodes;
  /**rangemaxx data member codes in "harvestmastercuts" grid. Array size is
   * number cut ranges*/
  short int *mp_iRangeMaxCodes;
  /**rangeamtx data member codes in "harvestmastercuts" grid. Array size is
   * number cut ranges*/
  short int *mp_iRangeAmountCodes;
  /**Seedlings to cut data member code in "harvestmastercuts" grid. Array size
   * is number species. Array index matches species number*/
  short int *mp_iSeedlingCodes;
  /**prioritynamex data member codes in "harvestmastercuts" grid. Array size is
   * number priorities*/
  short int *mp_iPriorityNameCodes;
  /**prioritytypex data member codes in "harvestmastercuts" grid. Array size is
   * number priorities*/
  short int *mp_iPriorityTypeCodes;
  /**priorityminx data member codes in "harvestmastercuts" grid. Array size is
   * number priorities*/
  short int *mp_iPriorityMinCodes;
  /**prioritymaxx data member codes in "harvestmastercuts" grid. Array size is
   * number priorities*/
  short int *mp_iPriorityMaxCodes;

  //"harvestcutevents/mortepisodecutevents" grid data members
  /**timestep data member code in "harvestcutevents" grid*/
  short int m_iCutTimestepCode;

  /**id data member code in "harvestcutevents" grid*/
  short int m_iCutIDCode;

  //"Harvest Results/Mortality Episode Results" grid data members
  /**Harvest Type data member code in "Harvest Results" grid*/
  short int m_iHarvestTypeCode;
  /**Cut Density data member codes in "Harvest Results" grid. Array size is
   * number of cut ranges by number of species*/
  short int **mp_iDenCutCodes;
  /**Cut Basal Area data member codes in "Harvest Results" grid. Array size is
   * number cut ranges by number of species*/
  short int **mp_iBaCutCodes;
  /**Cut Seedlings data member codes in "Harvest Results" grid. Array size is
   * number of species.*/
  short int *mp_iSeedlingCutCodes;

  /**Disturbance grid cell X length*/
  float m_fDistXCellLen;

  /**Disturbance grid cell Y length*/
  float m_fDistYCellLen;

  /**Tree population grid cell length*/
  float m_fPopCellLen;

  //Other short int variables which are not data member return codes
  static short int m_iNumAllowedCutRanges;  /**<Number of cut ranges allowed.*/

  /**Number of allowed priorities*/
  static short int m_iNumAllowedPriorities;

  /**Reason code to pass to the tree population when trees are killed. This is
  * HARVEST in the case of harvest, and DISEASE in the case of episodic
  * mortality.*/
  deadCode m_iReasonCode;

  /**What kind of behavior this is. If true, it's a harvest. If false, it's
   * episodic mortality.*/
  bool m_bIsHarvest;

  /**Holds a linked list of grid cells.*/
  struct stcGridList {
     stcGridList *next;  /**<Next grid cell in the list*/
     short int iDistX;   /**<Disturbance grids cell X number*/
     short int iDistY;   /**<Disturbance grids cell Y number*/
  };

  /**Holds a linked list of tree grid cells that correspond to the harvest area.*/
  struct stcTreeGridList {
     clTree *p_oTree;    /**<Tallest tree of a particular species in the cell*/
     stcTreeGridList *next;  /**<Next grid cell in the list*/
     float fDbh;         /**<DBH of the tallest tree in the cell*/
     short int iTreeX;   /**<Tree population grid cell X number*/
     short int iTreeY;   /**<Tree population grid cell Y number*/
  };

  /**
   * Resets all the values in the Harvest Results or Episodic Mortality Results
   * grid. If Harvest, Harvest Type becomes -1; all others are 0.
   */
  void ResetResultsGrid();

  /**
   * Performs setup.  First, SetUpGrids() is called to create our grids.  Then,
   * if this is harvest, ReadHarvestParameterFileData() is called; if episodic
   * mortality, ReadEpMortParameterFileData() is called. Then ValidatePackages()
   * is called to validate data from the parameter file.
   * @param p_oDoc DOM tree of parsed input file.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Reads harvest data from the parameter file.  It doesn't perform validation
   * other than basic data types - more in-depth logical validation is left to
   * ValidatePackages().
   * @param p_oDoc DOM tree of parsed input file.
   */
  void ReadHarvestParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
   * Reads episodic mortality data from the parameter file.  It doesn't perform
   * validation other than basic data types - more in-depth logical validation
   * is left to ValidatePackages().
   * @param p_oDoc DOM tree of parsed input file.
   */
  void ReadMortEpParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
   * Sets up the harvest or mortality episode grids and gets all the return
   * codes.
   */
  void SetupGrids();

  /**
   * Makes sure that the data in the event harvest packages makes sense.  For
   * each cut event (each package) - this checks the following:
   * <ul>
   * <li>No negative values in cut ranges</li>
   * <li>Cut ranges do not overlap</li>
   * <li>Cut amounts expressed in percentages do not have values over 100</li>
   * <li>There are no cut ranges defined for gap and clear cuts</li>
   * </ul>
   * For harvest gap and clear cuts, this will set up one cut range so the cut
   * happens correctly.  The cut range will cover dbhs from 0 to 3000, removing
   * 100% of density.
   */
  void ValidatePackages();

  /**
   * Performs the tree cutting for a master cut package.  It starts by getting
   * a linked list of cells to cut and their tallest trees of a given species,
   * then selects trees to cut according to the cut type.
   * @param p_oMasterPackage The master package to cut
   */
  void CutTrees(clPackage *p_oMasterPackage);

  /**
   * Cut trees for a species - absolute density.
   * @param iSp Species
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param p_fLoDbh Array of the low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array of the high dbhs of the package's cut ranges.
   * @param p_fAmountToRemove Amount to remove in each cut range.
   * @param p_fAmountRemoved Amount already removed.
   * @param bTallestFirst Whether to cut the tallest trees first (true)
   * or shortest first (false).
   */
  void CutSpeciesAbsDen(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, bool bTallestFirst);

  /**
   * Cut trees for a species with a priority - absolute density.
   * @param iSp Species
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param p_fLoDbh Array of the low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array of the high dbhs of the package's cut ranges.
   * @param p_fAmountToRemove Amount to remove in each cut range.
   * @param p_fAmountRemoved Amount already removed.
   * @param sPriorityName Name of priority data member.
   * @param iPriorityType Priority type.
   * @param fPriorityMin Priority minimum value.
   * @param fPriorityMax Priority maximum value.
   * @param bTallestFirst Whether to cut the tallest trees first (true)
   * or shortest first (false).
   */
  void CutSpeciesAbsDenPriority(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, std::string sPriorityName, int &iPriorityType,
    float &fPriorityMin, float &fPriorityMax, bool bTallestFirst);

  /**
   * Cut trees for a species - absolute basal area.
   * @param iSp Species
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param p_fLoDbh Array of the low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array of the high dbhs of the package's cut ranges.
   * @param p_fAmountToRemove Amount to remove in each cut range.
   * @param p_fAmountRemoved Amount already removed.
   * @param bTallestFirst Whether to cut the tallest trees first (true)
   * or shortest first (false).
   */
  void CutSpeciesAbsBA(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, bool bTallestFirst);

  /**
   * Cut trees for a species with a priority - absolute basal area.
   * @param iSp Species
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param p_fLoDbh Array of the low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array of the high dbhs of the package's cut ranges.
   * @param p_fAmountToRemove Amount to remove in each cut range.
   * @param p_fAmountRemoved Amount already removed.
   * @param sPriorityName Name of priority data member.
   * @param iPriorityType Priority type.
   * @param fPriorityMin Priority minimum value.
   * @param fPriorityMax Priority maximum value.
   * @param bTallestFirst Whether to cut the tallest trees first (true)
   * or shortest first (false).
   * @return Whether to keep cutting (true) or stop the harvest (false).
   * False indicates that there are more priority trees left but they
   * were not cut because we were too close to the target. It would not
   * make sense then in this case to cut other trees.
   */
  bool CutSpeciesAbsBAPriority(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, std::string sPriorityName, int &iPriorityType,
    float &fPriorityMin, float &fPriorityMax, bool bTallestFirst);

  /**
   * Cut trees for a species - percent density.
   * @param iSp Species
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param p_fLoDbh Array of the low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array of the high dbhs of the package's cut ranges.
   * @param p_fAmountToRemove Amount to remove in each cut range.
   * @param p_fAmountRemoved Amount already removed.
   */
  void CutSpeciesPercentDen(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved);

  /**
   * Cut trees for a species - percent basal area.
   * @param iSp Species
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param p_fLoDbh Array of the low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array of the high dbhs of the package's cut ranges.
   * @param p_fAmountToRemove Amount to remove in each cut range.
   * @param p_fAmountRemoved Amount already removed.
   * @param p_fTotalBasalArea Total basal area.
   * @param bTallestFirst Whether to cut the tallest trees first (true)
   * or shortest first (false).
   */
  void CutSpeciesPercentBA(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, float *p_fTotalBasalArea, bool bTallestFirst);

  /**
   * Cut trees for a species with a priority - percent basal area.
   * @param iSp Species
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param p_fLoDbh Array of the low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array of the high dbhs of the package's cut ranges.
   * @param p_fAmountToRemove Amount to remove in each cut range.
   * @param p_fAmountRemoved Amount already removed.
   * @param p_fTotalBasalArea Total basal area.
   * @param sPriorityName Name of priority data member.
   * @param iPriorityType Priority type.
   * @param fPriorityMin Priority minimum value.
   * @param fPriorityMax Priority maximum value.
   * @param bTallestFirst Whether to cut the tallest trees first (true)
   * or shortest first (false).
   * @return Whether to keep cutting (true) or stop the harvest (false).
   * False indicates that there are more priority trees left but they
   * were not cut because we were too close to the target. It would not
   * make sense then in this case to cut other trees.
   */
  bool CutSpeciesPercentBAPriority(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, float *p_fTotalBasalArea, std::string sPriorityName,
    int &iPriorityType, float &fPriorityMin, float &fPriorityMax, bool bTallestFirst);

  /**
   * Add tree to results grid.
   * @param p_oTree Tree to add.
   * @param p_fLoDbh Array (sized m_iNumAllowedCutRanges) into which to put the
   * low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array (sized m_iNumAllowedCutRanges) into which to put the
   * high dbhs of the package's cut ranges.
   * @param fDbh DBH.
   */
  void AddTreeStatsToResults(clTree *p_oTree, float *p_fLoDbh,
      float *p_fHiDbh, float &fDbh);

  /**
   * Finds all the grid cells affected by a given cut.  The cut in question is
   * found in one master cut package.  The grid cells are identified by matching
   * ID numbers on packages in the "harvestcutevents" / "mortepisodecutevents"
   * grid. The cut area cells are assembled into a linked list.  All packages
   * which went into the linked list are then deleted.  This also extracts the
   * cut range data and cut amount data.
   *
   * @param p_oMasterPackage The master package for which the cut area list is
   * being assembled.  (This package will be deleted.)
   * @param iNumXCells Number of X cells in mp_oCutEventsGrid.
   * @param iNumYCells Number of Y cells in mp_oCutEventsGrid.
   * @param p_cutArea Pointer in which to start the linked list.
   * @param p_fLoDbh Array (sized m_iNumAllowedCutRanges) into which to put the
   * low dbhs of the package's cut ranges.
   * @param p_fHiDbh Array (sized m_iNumAllowedCutRanges) into which to put the
   * high dbhs of the package's cut ranges.
   * @param p_fAmountToRemove Array (sized m_iNumAllowedCutRanges) into which to
   * put the amount of basal area to remove for each of the package's cut ranges.
   * @param p_bSpeciesCut Array of all species for whether or not a species is
   * affected by this cut.
   *
   * @return The number of the cells in the cut area.
   */
  int AssembleCutArea(clPackage *p_oMasterPackage, const int &iNumXCells,
   const int &iNumYCells, stcGridList *&p_cutArea, stcTreeGridList * &p_treeArea,
   float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove, bool *p_bSpeciesCut);

  /**
   * Gets the basal area in a cut area for a single species.  The cut area is
   * defined by a linked list of grid cells.  This gets the basal area in each
   * cut range and puts it in an array whose pointer was passed.
   *
   * @param p_cutArea Pointer to the linked list.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param iSpecies Species of tree for which to calculate basal area.
   * @param p_fTotalBasalArea Array (size m_iNumAllowedCutRanges) in which to
   * put the total basal area calculation.
   * @param p_fLoDbh Array (size m_iNumAllowedCutRanges) of lower-limit dbhs for
   * the cut ranges that have been defined.
   * @param p_fHiDbh Array (size m_iNumAllowedCutRanges) of upper-limit dbhs for
   * the cut ranges that have been defined.
   */
  void GetBasalArea(stcGridList *p_cutArea, stcTreeGridList * & p_treeArea, const short int &iSpecies,
     float *p_fTotalBasalArea, float *p_fLoDbh, float *p_fHiDbh);

  /**
   * Kills seedlings for a harvest or mortality episode event. Unlike with
   * larger trees, all species are done at the same time. Each seedling found
   * within the affected area gets a random number comparison to the kill
   * probability for its species.
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param p_fKillProb Kill probability for each species.
   */
  void KillSeedlings(stcGridList *p_cutAreap_cutArea, stcTreeGridList * & p_treeArea, float *p_fKillProb);

  /**
  * Finds the first tree of a species within a cut area to cut, based on bTallestFirst.
  * This is a convenience function that will call either GetTallestTreeInCutArea or
  * GetShortestTreeInCutArea.
  *
  * @param p_cutArea Pointer to the linked list of cut grid cells.
  * @param p_treeArea Pointer to the linked list of tree grid cells.
  * @param iSpecies Species of tree to populate.
  * @return Either the tallest or the shortest tree in the area, depending on
  * m_bTallestFirst, or NULL if there are no trees of the desired species.
  */
  clTree* GetFirstTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
   const short int &iSpecies, const bool bTallestFirst);

 /**
  * Finds the tallest tree of a species within a cut area.  The cut area is
  * defined by a linked list of grid cells.  This assumes that the tallest tree
  * pointers for each grid cell in the linked list is not populated.  This will
  * populate and sort them.  Which means that the pointer p_treeArea could be
  * changed.
  * @param p_cutArea Pointer to the linked list of cut grid cells.
  * @param p_treeArea Pointer to the linked list of tree grid cells.
  * @param iSpecies Species of tree to populate.
  * @return Tallest tree in the area, or NULL if there are no trees of the
  * desired species.
  */
  clTree* GetTallestTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
   const short int &iSpecies);

  /**
  * Finds the shortest tree of a species within a cut area.  The cut area is
  * defined by a linked list of grid cells.  This assumes that the shortest tree
  * pointers for each grid cell in the linked list is not populated.  This will
  * populate and sort them.  Which means that the pointer p_treeArea could be
  * changed.
  * @param p_cutArea Pointer to the linked list of cut grid cells.
  * @param p_treeArea Pointer to the linked list of tree grid cells.
  * @param iSpecies Species of tree to populate.
  * @return Shortest tree in the area, or NULL if there are no trees of the
  * desired species.
  */
  clTree* GetShortestTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
   const short int &iSpecies);

  /**
   * Finds the next tree of a species within a cut area to cut, based on bTallestFirst.
   * This is a convenience function that will call either GetNextTallestTreeInCutArea or
   * GetNextShortestTreeInCutArea.
   *
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param iSpecies Species being cut.
   * @return Next tallest tree in the area, or NULL if there are no trees of the
   * desired species.
   */
  clTree* GetNextTreeInCutArea(stcGridList *&p_cutArea, stcTreeGridList * & p_treeArea,
      const short int &iSpecies, const bool bTallestFirst);

   /**
   * Gets the next tallest tree of a species within a cut area.  The cut area is
   * defined by a linked list of grid cells.  This assumes that the pointers to
   * the tallest trees for each grid cell are populated, and that the current
   * tallest tree is no longer wanted (although if it is to be killed it hasn't
   * been killed yet).  The current tallest tree could still be alive; but it
   * will be ignored for further consideration.
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param iSpecies Species being cut.
   * @return Next tallest tree in the area, or NULL if there are no trees of the
   * desired species.
   */
  clTree* GetNextTallestTreeInCutArea(stcGridList *&p_cutArea, stcTreeGridList * & p_treeArea,
      const short int &iSpecies);

  /**
   * Gets the next shortest tree of a species within a cut area.  The cut area is
   * defined by a linked list of grid cells.  This assumes that the pointers to
   * the shortest trees for each grid cell are populated, and that the current
   * shortest tree is no longer wanted (although if it is to be killed it hasn't
   * been killed yet).  The current shortest tree could still be alive; but it
   * will be ignored for further consideration.
   * @param p_cutArea Pointer to the linked list of cut grid cells.
   * @param p_treeArea Pointer to the linked list of tree grid cells.
   * @param iSpecies Species being cut.
   * @return Next tallest tree in the area, or NULL if there are no trees of the
   * desired species.
   */
  clTree* GetNextShortestTreeInCutArea(stcGridList *&p_cutArea, stcTreeGridList * & p_treeArea,
      const short int &iSpecies);

  /**
   * Sets the Harvest Type data member of the Harvest Results grid.  For each
   * grid cell in the cut area, this makes sure that the data member is set to
   * the most severe cut type that occurred in a timestep.
   * @param p_cutArea Pointer to the linked list of the cut area.
   * @param iCutType Most severe cut type from this cut area.
   */
  void SetCutFlags(stcGridList *p_cutArea, const int &iCutType);

  /**
   * Figures out whether or not a tree falls in a disturbance cut area.
   * @param p_oTree Tree to check
   * @param p_cutArea Cut area to check
   * @return True if the tree falls in the cut area, false if not
   */
  bool IsTreeInCutArea(clTree *p_oTree, stcGridList *p_cutArea);

};
//---------------------------------------------------------------------------
#endif
