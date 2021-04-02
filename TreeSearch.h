
//---------------------------------------------------------------------------

#ifndef TreeSearchH
#define TreeSearchH
//---------------------------------------------------------------------------

#include "Tree.h"

class clPlot;


/**
* Tree Search Class - Version 1.1
* This class controls the feeding of individual trees to behavior
* objects.  For each request for a population to return a set of trees, a
* treeSearch object will be created.  It will keep the required search
* attributes and control the tree flow.
*
* When a clTreeSearch object is returned as a result of a query to a
* clTreePopulation object, use NextTree() (including for the first tree) to
* return trees until NextTree() returns NULL, indicating the end of the list.
*
* A clTreeSearch object acts like a simultaneous placeholder and filter in the
* hash table.  It keeps its place and, when NextTree() is called, sifts through
* until it finds the next tree meeting the appropriate criteria.
*
* The clTreeSearch class and the clTreePopulation class are friends of each
* other. This lets them access each other's methods and data, which is
* importantvbecause the tree searching code lives in the clTreeSearch object
* but accesses trees in the clTreePopulation object.
*
* Requirements for a successful search:
* <ul>
* <li>All types and species must be valid.
* <li>No negative numbers.
* <li>Points to calculate distance from must be within the plot.
* <li>If you specify height, you must specify distance.
* <li>If you do a distance/height search, you cannot specify any other search
*   parameters.
* </ul>
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>April 28, 2004 - Submitted as beta (LEM)
* <br>March 3, 2005 - Changed species from bit-shifting int to bool array (LEM)
* <br>August 14, 2007 - Decided to skip edge testing for distance-height
* searches. It makes sense in theory but most searches are 10 meters or less.
* Plus there's a bug - short end cells are treated as full-size and may lead
* to their regular-sized neighbors being skipped. (LEM)
*/
class clTreeSearch {
  friend class clTreePopulation;
  friend class clTestTreePopulation; /**<For testing purposes*/

  public:

  /**
   * Gets starting height division to search in tree hash table in a type
   * search.
   * @return Starting height division to search in tree hash table in a type
   * search.
   */
  int GetStartHeightDiv() {return m_iStartHeightDiv;}

  /**
   * Gets ending height division to search in tree hash table in a type
   * search.
   * @return Ending height division to search in tree hash table in a type
   * search.
   */
  int GetEndHeightDiv() {return m_iEndHeightDiv;}

  /**
  * Returns the next tree matching this search's criteria.
  * NextTree just calls the appropriate FindNext... function.
  */
  clTree* NextTree() {
    clTree *p_oReturnTree; //tree to return
    p_oReturnTree = mp_oCurrentTree;
    mp_oCurrentTree = (*this.*function)();
    return p_oReturnTree;
  };

  /**
  * Starts the tree search over.  After this is called, the next call to
  * NextTree() will return the first tree, and the whole list can be traversed
  * again.  (This does NOT trigger a new search.)
  */
  void StartOver();

  /**
  * Constructor.
  * The constructor initializes variables and sets up the tree results linked
  * list.
  *
  * In setting up the linked list, it makes the initial dummy record and sets
  * mp_currentTree to that record.  This makes sure that even if no trees are
  * returned or there are errors thrown during searching, NextTree correctly
  * returns NULL to the object that originally requested the search.
  *
  * @param p_oTreePop Pointer to the tree population object
  * @param p_oPlot Pointer to the plot object
  */
  clTreeSearch(clTreePopulation* p_oTreePop, clPlot *p_oPlot);

  /**
  * Destructor.  Frees memory.
  */
  ~clTreeSearch();

  protected:
  /**Pointer to tree population*/
  clTreePopulation *mp_oTreePop;
  /**Pointer to plot object*/
  clPlot *mp_oPlot;

  /**Function pointer for the appropriate "FindNext..." function*/
  clTree* (clTreeSearch::*function)();

  /**
  * Optimized for finding trees where only distance and height are specified.
  *
  * @return The next tree in the search, or NULL if all trees have been found.
  */
  clTree* FindNextDHTree();

  /**
  * Optimized for finding trees where only type and species are specified.
  *
  * @return The next tree in the search, or NULL if all trees have been found.
  */
  clTree* FindNextTSTree();

  /**
  * Optimized for finding trees where only type is specified.
  *
  * @return The next tree in the search, or NULL if all trees have been found.
  */
  clTree* FindNextTTree();

  /**
  * Sets up the object to find all trees.  Finds the first one and sets
  * mp_gridList to hold the grid cell the search is traversing so it can
  * traverse on the fly when requested.
  *
  * @return The next tree in the search, or NULL if all trees have been found.
  */
  clTree* FindNextAllTree();

  /**
  * Finds the first tree in a distance/height search.  Then sets up the current
  * tree pointer and the current grid numbers.
  *
  * @return The first tree in the search, or NULL if no trees match.
  */
  clTree* FindFirstDHTree();

  /**
  * Finds the first tree in a type/species search.  Then sets up the current
  * tree pointer and the current grid numbers.
  *
  * @return The first tree in the search, or NULL if no trees match.
  */
  clTree* FindFirstTSTree();

  /**
  * Finds the first tree in a type search.  Then sets up the current tree
  * pointer and the current grid numbers.
  *
  * @return The first tree in the search, or NULL if no trees match.
  */
  clTree* FindFirstTTree();

  /**
  * Finds the first tree in an "all" search.  Then sets up the current tree
  * pointer and the current grid numbers.
  *
  * @return The first tree in the search, or NULL if no trees match.
  */
  clTree* FindFirstAllTree();
  
  /**For distance-height searches, gets the min and max X grid cells for a
   * given Y grid cell. We search a circle, and we know XY of center, and Y of 
   * point on edge of circle - we can solve for X as 
   * X2 = X1 - square root(distance^2 - (Y1 - Y2)^2). The min and max values
   * are set in m_iMinX and m_iMaxX, uncorrected for torus.
   * @param iY The Y grid cell, uncorrected for torus.
   * @param iGrLen Grid length
   */
  void GetMinMaxX(const int &iY, const int &iGrLen);

  /**
  * Does final data validation and gets things ready for the NextTree() methods.
  */
  void Setup();

  clTree  *mp_oCurrentTree; /**<The tree that the search is currently on*/

  //Flags for what kinds of arguments were specified
  bool m_bDistanceHeightUsed;  /**<Whether or not this is a distance and height search*/
  bool m_bTypeUsed;            /**<Whether or not this is searching by tree type*/
  bool m_bSpeciesUsed;         /**<Whether or not this is searching by species*/
  bool m_bAllUsed;             /**<Whether or not this should simply return all -
                              trumps everything else*/

  //"All" parameters
  int m_iCurrentXGrid; /**<Current X grid number in an "all" search*/
  int m_iCurrentYGrid; /**<Current Y grid number in an "all" search*/

  //Distance/height parameters
  float m_fFromX; /**<X coordinate of search point in a distance/height search*/
  float m_fFromY; /**<Y coordinate of search point in a distance/height search*/
  float m_fDistanceCutoff; /**<Target maximum distance, in meters, in a
       distance/height seach*/
  float m_fHeightCutoff;  /**<Target minimum height, in meters, in a
        distance/height search - can be zero*/
  short int m_iMaxX; /**<Maximum X grid cell number to search in a distance/height search*/
  short int m_iMaxY; /**<Maximum Y grid cell number to search in a distance/height search*/
  short int m_iMinX; /**<Minimum X grid cell number to search in a distance/height search*/
  short int m_iMinY; /**<Minimum Y grid cell number to search in a distance/height search*/
  short int m_iHomeX; /**<X grid cell of target point in a distance/height search*/
  short int m_iHomeY; /**<Y grid cell of target point in a distance/height search*/


  // Square search parameters
  //Re-use the following from distance/height search:
  //m_fFromX and m_fFromY, m_iMaxX, m_iMaxY, m_iMinX, m_iMinY, m_bCellOnEdge
  // float m_fToX, m_fToY; //XY coords of end of range

  //Type parameters

  short int m_iWhatTypes; /**<Which types to search for in a search involving
        type.  The types are indicated by flipped bits in this variable.
        From right to left the bits are seedling, sapling, adult.  More in the
        header file.*/
  short int m_iStartHeightDiv; /**<Starting height division to search in tree
        hash table in a type search.*/
  short int m_iEndHeightDiv; /**<Ending height division to search in tree
        hash table in a type search.*/
  bool m_bSeeds; /**<Whether seeds are being searched for in a type search.*/
  bool m_bSeedlings; /**<Whether seedlings are being searched for in a type search.*/
  bool m_bSaplings; /**<Whether saplings are being searched for in a type search.*/
  bool m_bAdults;/**<Whether adults are being searched for in a type search.*/
  bool m_bSnags; /**<Whether snags are being searched for in a type search.*/
  bool m_bWoody_Debris;/**<Whether woody debris is being searched for in a type search.*/

  //Species parameters
  bool *mp_bWhatSpecies; /**<Which species to search for in a
        search involving species.  This array is sized # species.*/      
};

////////////////////////////////////////////////////////////////////////////////
// A note on the storage of type information:
// The type information is stored in an int which are being treated as a boolean
// array.  The unsigned long int has 32 bits in it; each bit, starting with the
// rightmost, represents a boolean for whether or not that type is included.  So
// if types 1, 3, and 6 are to be used, the last 8 bits of the int would look
// like 00100101.

// To extract this information, bitwise shifts and bitwise operators will be
// used. More on this in the relevant code.
//////////////////////////////////////////////////////////////////////////////*/

#endif
