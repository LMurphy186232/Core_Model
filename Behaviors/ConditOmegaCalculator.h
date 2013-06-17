//---------------------------------------------------------------------------
// ConditOmegaCalculator.h
//---------------------------------------------------------------------------
#if !defined(ConditOmegaCalculator_H)
  #define ConditOmegaCalculator_H

#include "BehaviorBase.h"

class clGrid;
class clTree;
class clTreePopulation;
class clPlot;

/**
* Condit's Relative Neighborhood Density Index (Omega) Calculator Version 1.0
*
* This behavior calculates Condit's relative neighborhood density index (omega).
* This index measures the aggregation of species. It is calculated for
* successive distances out to a maximum. The user sets the maximum distance and
* the increment. Omega for a species for a given distance
* x + @htmlonly &Delta;x @endhtmlonly is calculated as:
*
* @htmlonly &Omega; = (&Sigma;N<sub>x</sub> / (T * A<sub>x</sub>))/den  @endhtmlonly
*
* where A<sub>x</sub> is the area of the annulus described by the radii x and
* @htmlonly &Delta;x @endhtmlonly, in square meters, N<sub>x</sub> is the number
* of conspecific neighbors of trees between x and @htmlonly &Delta;x
* @endhtmlonly, T is the total number of trees of that species in the plot, and
* den is the species density in the plot.
*
* In other words, @htmlonly &Omega; @endhtmlonly is the average density of
* conspecific neighbors at a specific distance divided by the plot's density of
* that species. Saplings and adults are counted as neighbors. All other types
* are ignored.
*
* Conceptually, the way to carry out this search is to take each tree, find the
* density of its conspecific neighbors between each pair of radii x and
* @htmlonly &Delta;x @endhtmlonly, average those values by species, and divide
* by the plot density for each species. In practice I can use a quicker method.
* The search proceeds by tree population grid cell. For each cell, a square of
* cells around it is searched. A matrix is used to keep track of which cells
* have already been paired so they are not paired again. Each pair of
* conspecifics is counted into the appropriate distance bucket (twice, once for
* when each tree is acting as a neighbor to the other's target). Then these
* buckets are used to calculate @htmlonly &Omega; @endhtmlonly according to the
* formula above.
*
* The statistic is calculated for all individual species.
*
* The values are collected into a grid called "Relative Neighborhood Density".
*
* This class's namestring and parameter file call string is "ConditsOmega".
* Any tree type/species assignments are ignored.
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clConditOmegaCalculator : virtual public clBehaviorBase {
  friend class clTestConditOmegaCalculator;
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clConditOmegaCalculator(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clConditOmegaCalculator();

  /**
  * Calculates Condit's Omega.  First, the values in the "Relative Neighborhood
  * Density" grid are cleared. Then this moves through the grid cells looking
  * for tree pairs. For each cell this searches cells in a circle, using
  * ProcessCell() and ProcessOwnCell() to find pairs. Once tree pairs have been
  * found, this finishes the Condit's Omega calculation.
  */
  void Action();

  /**
  * Does setup for this behavior. Calls:
  * <ol>
  * <li>GetParameterFileData()</li>
  * <li>GetTreeCodes()</li>
  * <li>SetupGrid()</li>
  * <li>SetUpSearching()</li>
  * <li>Action() so that the initial conditions value will be added</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  protected:

  /** Grid holding Condit's Omega values for the plot plus each species. The
   * grid name is "Relative Neighborhood Density". It has one data member for
   * each increment step out to the max for each species. The names are X_Y,
   * where X is the increment step number (starting at zero) and Y is the
   * species number (starting at zero). It also has two more for the length of
   * distance increment ("inc") and the max distance ("dist"), to make output
   * display easier.
   *
   * This grid is always one cell per plot. Changes to grid cell resolution
   * are ignored.*/
  clGrid* mp_oGrid;

  /** Values holding Condit's Omega for each timestep. Array size is number of
   * species by m_iNumIncs.*/
  float **mp_fOmegaValues;

  /** Holds the codes for the "Relative Neighborhood Density" grid. The first
   * array index is number of species, with array size being m_iNumTotalSpecies.
   * The second array index is buckets for values along the distance line, and
   * is sized m_iNumIncs.*/
  short int **mp_iGridCodes;

  /** Holds codes for X data member.  First array index is total # species,
   * second is number types (2 - sapling, adult).*/
  short int **mp_iXCodes;

  /** Holds codes for Y data member.  First array index is total # species,
   * second is number types (2 - sapling, adult).*/
  short int **mp_iYCodes;

  /** Which cells have already been paired the current timestep. The matrix
   * size is number of total cells by number of total cells (flattening the
   * 2D tree population cell matrix into a single array). The index for a tree
   * population grid cell is number of X cells * X cell index + Y cell index.
   * Since every grid cell is paired with every other, once a pair of grid
   * cells is searched, two matrix cells must be switched to true.*/
  bool **mp_bCellsSearched;

  /** Distance increment values - array size is m_iNumIncs */
  float *mp_fIncs;

  /** Area of each annulus - array size is m_iNumIncs */
  float *mp_fAnnulusAreas;

  /** The max distance to which to calculate Condit's Omega. */
  float m_fMaxDistance;

  /** The distance increment used to step out to the max distance. */
  float m_fIncrement;

  /** Number of increments for which to calculate Condit's Omega.*/
  int m_iNumIncs;

  /** The number of cells to search in the X direction - if the plot is a
   * rectangle and the search distance is long the distances along the two
   * axes may be different*/
  int m_iNumXToSearch;

  /** The number of cells to search in the Y direction - if the plot is a
   * rectangle and the search distance is long the distances along the two
   * axes may be different*/
  int m_iNumYToSearch;

  /** Number of tree population grid cells in X direction */
  int m_iNumXCells;

  /** Number of tree population grid cells in Y direction */
  int m_iNumYCells;

  /** Total number of species.  For the destructor.*/
  short int m_iNumTotalSpecies;

  /**
  * Reads values from the parameter file and performs setup related to the
  * parameters. This validates the increment and distance.
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if either the increment or the max distance are less than
  * or equal to zero, or if the increment is less than the max distance.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
   * Sets up the "Relative Neighborhood Density" grid and registers the data
   * members.
   */
  void SetupGrid();

  /**
  * Gets the X and Y tree data codes.
  * @param p_oPop Tree population object
  */
  void GetTreeCodes(clTreePopulation *p_oPop);

  /**
   * Sets up the structures for searching controls. This sets up the
   * mp_bCellsSearched matrix and finds the search distances in the X and Y
   * directions. If the search distance is more than half the plot length in
   * either direction, then it will be limited to half the plot length. This
   * also sets up the mp_fIncs and mp_fAnnulusAreas arrays.
   * @param p_oPop Tree population object
   */
  void SetUpSearching(clTreePopulation *p_oPop);

  /**
   * Processes a neighboring cell's trees to find distance pairs. This finds
   * all pairs of conspecific saplings and adults within the max distance and
   * counts each pair into the appropriate bin in the Condit's Omega calculator
   * array. Each pair is counted in twice (once for each tree acting as the
   * other's neighbor).
   * @param p_oPop Tree population object.
   * @param p_oFirstTree The shortest valid tree in the home cell.
   * @param iX X coordinate of home cell.
   * @param iY Y coordinate of home cell.
   * @param iNeighX X coordinate of the neighbor cell to search for tree pairs.
   * @param iNeighY Y coordinate of the neighbor cell to search for tree pairs.
   */
  void ProcessCell(clTreePopulation *p_oPop, clTree *p_oFirstTree,
     const int &iX, const int &iY, const int &iNeighX, const int &iNeighY);

  /**
   * Processes a single cell's trees to find distance pairs. This finds all
   * pairs of conspecific saplings and adults within the max distance, being
   * careful not to pair a tree with itself, and counts each pair into the
   * appropriate bin in the Condit's Omega calculator array. Each pair is
   * counted in twice (once for each tree acting as the other's neighbor). This
   * also counts all the trees in the cell for the purposes of finding the
   * plot's density.
   * @param p_oFirstTree The shortest valid tree in the home cell.
   * @param p_fNumTrees The array for number of saplings and adults in the cell
   * for each species.
   * @param iX X coordinate of home cell.
   * @param iY Y coordinate of home cell.
   */
  void ProcessOwnCell(clTree *p_oFirstTree, float *p_fNumTrees, const int &iX,
     const int &iY);
};
//---------------------------------------------------------------------------
#endif //ConditOmegaCalculator_H
