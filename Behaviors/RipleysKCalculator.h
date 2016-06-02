//---------------------------------------------------------------------------
// RipleysKCalculator.h
//---------------------------------------------------------------------------
#if !defined(RipleysKCalculator_H)
  #define RipleysKCalculator_H

#include "BehaviorBase.h"

class clGrid;
class clTree;
class clTreePopulation;
class clPlot;

/**
* Ripley's K Calculator Version 1.0
*
* This behavior calculates Ripley's K. Ripley's K is calculated for successive
* distances out to a maximum. The user sets the maximum distance and the
* increment. Ripley's K for a given distance t is calculated as:
*
* @htmlonly K(t) = AX / n<sup>2</sup>@endhtmlonly
*
* where A is the area of the plot in square meters, X is the number of pairs of
* trees <= t meters apart, and n is the total number of trees in the plot. All
* saplings and adults are counted. All other types are ignored.
*
* When searching for tree pairs it is important to make sure we only find each
* pair a single time. So the search proceeds by tree population grid cell. For
* each cell, a square of cells around it is searched. A matrix is used to keep
* track of which cells have already been paired so they are not paired again.
*
* The statistic is calculated both for all trees and for individual species.
*
* The values are collected into a grid called "Ripley's K".
*
* This class's namestring and parameter file call string are "RipleysK". Any
* tree type/species assignments are ignored.
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clRipleysKCalculator : virtual public clBehaviorBase {
  friend class clTestRipleysKCalculator;
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clRipleysKCalculator(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clRipleysKCalculator();

  /**
  * Calculates Ripley's K.  First, the values in the "Ripley's K" grid are
  * cleared. Then this moves through the grid cells looking for tree pairs. For
  * each cell this searches cells in a circle, using ProcessCell() and
  * ProcessOwnCell() to find pairs. Once tree pairs have been found, this
  * finishes the Ripley's K calculation.
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

  /** Grid holding Ripley's K values for the plot plus each species. The grid
   * name is "Ripley's K". It has one data member for each increment step out
   * to the max for each species, plus one more set for the all-species values.
   * For example, if the max distance is 50 meters and the increment is 1 meter,
   * it will have 50 * (X + 1) data members, where X is the number of
   * species. The names are X_Y, where X is the increment step number (starting
   * at zero) and Y is the species number (starting at zero). It also has two
   * more for the length of distance increment ("inc") and the max distance
   * ("dist"), to make output display easier.
   *
   * This grid is always one cell per plot. Changes to grid cell resolution
   * are ignored.*/
  clGrid* mp_oGrid;

  /** Values holding Ripley's K for each timestep. I tried to make this a
   * temp array each timestep but I got weird crashes. Array size is number of
   * species plus 1 by m_iNumIncs.*/
  float **mp_fKValues;

  /** Holds the codes for the "Ripley's K" grid. The first array index is number
   * of species, with array size being m_iNumTotalSpecies + 1 (the last index
   * holds the all-species codes). The second array index is buckets for values
   * along the distance line, and is sized m_iNumIncs.*/
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

  /** The max distance to which to calculate Ripley's K. */
  double m_fMaxDistance;

  /** The distance increment used to step out to the max distance. */
  double m_fIncrement;

  /** Number of increments for which to calculate Ripley's K.*/
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
   * Sets up the "Ripley's K" grid and registers the data members.
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
   * also sets up the mp_fIncs array.
   * @param p_oPop Tree population object
   */
  void SetUpSearching(clTreePopulation *p_oPop);

  /**
   * Processes a neighboring cell's trees to find distance pairs. This finds
   * all pairs of saplings and adults within the max distance and counts each
   * pair into the appropriate bins in the Ripley's K calculator array.
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
   * pairs of saplings and adults within the max distance, being careful not
   * to pair a tree with itself, and counts each pair into the appropriate bins
   * in the Ripley's K calculator array. This also counts all the trees in the
   * cell for the purposes of finding the plot's grand total.
   * @param p_oFirstTree The shortest valid tree in the home cell.
   * @param p_fNumTrees The array for number of saplings and adults in the cell
   * for each species.
   * @param iX X coordinate of home cell.
   * @param iY Y coordinate of home cell.
   */
  void ProcessOwnCell(clTree *p_oFirstTree, float *p_fNumTrees, const int &iX,
     const int &iY);

  /**
   * Tests a cell pair to see if they are worth searching. This finds out
   * whether any part of the neighbor cell is within the max distance of the
   * target cell.
   *
   * I'm putting this on hold for now because of the supreme pain of also
   * doing odd size end cells etc.
   *
   * @param p_oPlot Plot object.
   * @param iX X coordinate of home cell.
   * @param iY Y coordinate of home cell.
   * @param iNeighX X coordinate of neighbor cell, NOT corrected for torus
   * geometry.
   * @param iNeighY Y coordinate of neighbor cell, NOT corrected for torus
   * geometry.
   * @param iCellSize Size of the tree population grid cells.
   */
// bool WorthSearching(clPlot *p_oPlot, const int &iX, const int &iY,
//     const int &iNeighX, const int &iNeighY, const int &iCellSize);
};
//---------------------------------------------------------------------------
#endif //RipleysKCalculator_H
