//---------------------------------------------------------------------------

#ifndef PlantingH
#define PlantingH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"
class clGrid;
class clPackage;
class clTree;

/**
* Planting - Version 1.0
*
* Planting events can be defined on a grid cell level for each timestep for
* each species.  If more than one event is defined for a species for a single
* grid cell and timestep, the user takes a chance on what happens.  Cells with
* common planting criteria are grouped by the user into groups which will be
* evaluated together.
*
* Seedlings to be planted can be placed either randomly around the planting
* cells or in a gridded fashion.  When random, each seedling is placed randomly
* within the entire chunk being planted.  When gridded, each seedling is placed
* within a given grid cell.  This means that gridded plantings are likely to be
* suboptimal unless they divide nicely into the length of a grid cell.  I have
* not figured out a better way to do gridded plantings.
*
* The size of new seedlings can be left as the standard new seedling size or it
* can be changed.
*
* The relative amounts of each species are in the parameter file.  When reading
* the file, this behavior expects values as percentages.
*
* The planting data is stored in a grid.  Each plant event, defined as the
* planting to be done for one species for one timestep for one grid cell, is
* stored in a grid package.  These packages are sorted in timestep order.  Each
* timestep, all the planting events that are defined for that timestep are
* executed, then those packages are removed.
*
* A word on planting grids:  It is possible for there to be grid maps for these
* grids in the parameter file.  These will be ignored, and any grids created
* with these maps will be replaced.
*
* The namestring and parameter file call string for this behavior is "Plant."
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clPlant : virtual public clBehaviorBase {

public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clPlant(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clPlant();

  /**
   * Resets the values in the plantresults grid to 0.
   */
  void TimestepCleanup();

  /**
   * Performs the planting for a timestep.
   * For a timestep, this goes through the master package lists in plantmaster
   * and calls PlantTrees() for each one that is for this timestep.
   */
  void Action();

  /**Type of plant spacing*/
  enum spaceType {gridded,  /**<Plants are spaced on a grid*/
                  random};  /**<Plants are randomly sprinkled around a grid cell*/

 protected:
  clTreePopulation *mp_oPop;       /**<Stashed pointer to tree population*/

  /**
   * PLANTING MASTER
   *
   * The grid called "plantmaster" will have a single cell.  This is where the
   * plant events are defined.
   *
   * Each plant event is one package and is applied to a list of species. It has
   * a unique ID number.  The grid cells to which it is applied are in the
   * "plantevents" grid.
   *
   * The packages are in timestep order, earliest first.
   *
   * Data members - all for packages:
   * <table>
   * <tr>
   * <th>Data member</th> <th>Type</th> <th>Description</th>
   * </tr>
   * <tr>
   * <td>id</td>          <td>int</td>  <td>ID number of plant event</td>
   * </tr>
   * <tr>
   * <td>timestep</td>    <td>int</td>  <td>Timestep to plant</td>
   * </tr>
   * <tr>
   * <td>spacetype</td>   <td>int</td>  <td>Matches a value of the enum
   *                                         "spaceType"</td>
   * </tr>
   * <tr>
   * <td>spcorden</td>    <td>float</td> <td>If spacetype = gridded, spacing of
   *                                     trees, in m. If spacetype = random,
   *                                     total density/ha for all species.</td>
   * </tr>
   * <tr>
   * <td>amtPlant(x)</td> <td>float</td> <td>One of each of these for each
   *                                     species (species # = x). This is the
   *                                     percentage, between 0 and 100, of each
   *                                     species desired. If a species is not to
   *                                     be planted, its value is 0.</td>
   * </tr>
   * </table>
   */
  clGrid *mp_oPlantMasterGrid;

  /**
   * PLANT EVENTS
   *
   * The grid called "plantevents" will have a cell resolution that matches the
   * tree population.  This is where data about planting events is stored.
   *
   * The need to plant a grid cell is signaled by the presence of a package. The
   * package has an ID number which matches a package in the "plantmaster" grid,
   * which contains the information about how the planting is actually to be
   * performed. Packages are in timestep order, earliest first.
   *
   * Data members - all for packages:
   *
   * <table>
   * <tr>
   * <th>Data member</th> <th>Type</th> <th>Description</th>
   * </tr>
   * <tr>
   * <td>id</td>          <td>int</td>  <td>ID number matching master package in
   *                                    "plantmaster"</td>
   * </tr>
   * <tr>
   * <td>timestep</td>    <td>int</td>  <td>Timestep at which to apply the
   *                                    plant</td>
   * </tr>
   * </table>
   */
  clGrid *mp_oPlantEventsGrid;

  /**
   * PLANT RESULTS
   *
   * The grid called "Planting Results" will have a grid cell resolution
   * matching that of "plantevents".  This is where data on planting results is
   * stored. The data is stored raw - no conversion to per-hectare amounts.
   *
   * <table>
   * <tr>
   * <th>Data member</th> <th>Type</th> <th>Description</th>
   * </tr>
   * <tr>
   * <td>planted(x)</td>  <td>int</td>  <td>Number of trees planted in the
   *                                    current timestep - one of these for each
   *                                    species (x = species number)</td>
   * </tr>
   * </table>
   */
  clGrid *mp_oPlantResultsGrid;

  /**Size of seedlings to plant. Actual values are randomized slightly around
   * these values.  Array size is number of species.*/
  float *mp_fInitialDiam10;

  //These hold the return codes for the data members
  /**timestep data member code in "planttmaster" grid*/
  short int m_iMasterTimestepCode;

  /**id code in "planttmaster" grid*/
  short int m_iMasterIDCode;

  /**amtPlant code in "planttmaster" grid. Array size is number of species -
   * array index matches species number*/
  short int *mp_iAmtPlantCodes;

  /**spacetype code in "planttmaster" grid*/
  short int m_iSpaceTypeCode;

  /**spcorden code in "planttmaster" grid*/
  short int m_iSpacingOrDensityCode;

  /**timestep code in "plantevents" grid*/
  short int m_iPlantTimestepCode;

  /**id code in "plantevents" grid*/
  short int m_iPlantIDCode;

  //"plantresults" grid data members
  /**planted codes in "plantresults" grid. Array size is number of species.*/
  short int *mp_iPlantedCodes;

  /**Holds a linked list of grid cells*/
  struct stcGridList {
    stcGridList *next; /**<Next grid cell in the list*/
    short int iX,      /**<Grid cell X number*/
              iY;      /**<Grid cell Y number*/
  };

  /**
   * Reads all the plant events from the parameter file. It doesn't perform
   * validation on them other than basic data types - more in-depth logical
   * validation is left to ValidatePackages().
   * @param p_oDoc DOM tree of parsed input file.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Sets up the plant grids and gets all the return codes.
   */
  void SetupPlantGrids();

  /**
   * Makes sure that the data in the plant master packages makes sense. For each
   * one this checks the following:
   * <ul>
   * <li>No negative values
   * <li>Relative amounts in percentages add up to 100
   * <li>For gridded plantings - grid spacing is less than the grid length
   * </ul>
   */
  void ValidatePackages();

  /**
   * Performs the tree planting for a master plant package. It starts by getting
   * a linked list of cells to plant from the package. When planting randomly,
   * for each tree being planted, a cell is selected at random and then a point
   * within that cell is selected at random. For gridded plantings, each cell is
   * evaluated separately. For a location in the cell, one of the species is
   * picked based on a random number compared to the desired relative amounts of
   * each species.
   * @param p_oMasterPackage The master package to plant
   */
  void PlantTrees(clPackage *p_oMasterPackage);

  /**
   * Finds the planting area for a planting event. For a master plant package,
   * this finds all the grid cells affected by this planting by matching ID
   * numbers on packages in the plantevents grid. The plant area cells are
   * assembled into a linked list. All packages which went into the linked list
   * are then deleted.
   *
   * @param p_oMasterPackage The master package for which the plant area list is
   * being assembled.  (This package will NOT be deleted.)
   * @param iNumXCells mp_oPlantEventsGrid's number of cells in the X direction.
   * @param iNumYCells mp_oPlantEventsGrid's number of cells in the Y direction.
   * @param p_plantArea Pointer in which to place the linked list of grid cells.
   * @return The number of the cells in the planting area.
   */
  int AssemblePlantArea(clPackage *p_oMasterPackage, const int &iNumXCells,
   const int &iNumYCells, stcGridList *&p_plantArea);

};
//---------------------------------------------------------------------------
#endif
