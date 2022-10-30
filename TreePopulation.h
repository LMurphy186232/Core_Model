#ifndef TreePopulationH
#define TreePopulationH

#include "TreeSearch.h"
#include "Tree.h"
#include "PopulationBase.h"
#include "Constants.h"
#include "ModelMath.h"
#include <string.h>
#include <sstream>

class clAllometry;
class clGhostTreePopulation;

#define MINDIAM 0.001
#define MINHEIGHT 0.001
#define MINCROWN 0.001

using namespace whyDead;
/**
* TREE POPULATION CLASS - Version 1.4
*
* The tree population class encapsulates all the types of trees and provides
* a common interface for accessing them. The trees are organized into a hash
* table. The trees are organized spatially in an internal grid and then by
* height within that grid. All seedlings, saplings, adults, and snags appear
* in the hash table. Stumps are organized separately since they have no
* height (if they are even used at all); seeds and woody debris currently don't
* exist.
*
* When behavior objects operate on trees, they receive a pointer to the
* tree population in order to do so. Then they use the search method to get the
* trees they need.
*
* Plot trees can only be created by the tree population class (if other objects
* want to create their own local trees for some reason, that's fine). Behaviors
* can ask the pop. to create seedlings (i.e. as a reproduction calculation).
* If it turns out to be necessary for other kinds of trees to be created then
* we can add those methods.
*
* The population takes care of all organization and housekeeping skills related
* to trees. When a tree dies it tells the population so that the population
* can remove it. When a tree changes its height it tells the population so that
* the population can update its position in the hash table.
*
* The hash table array holding the trees is based on grid cells of 8X8 m (no
* matter what the user has them set to), and the trees within each grid cell are
* organized by height. By far, the greatest number of searches that SORTIE has
* to do are distance and height searches (neighborhood searches) so the table
* is optimized for this. This class keeps some shortcut values calculated in
* order to handle other types of searches more quickly. The reason the grid is
* 8X8 m is because since 8 is a power of two, we can do very fast bitwise shifts
* instead of very slow division when we're doing grid cell calculations. (I
* leave it to the compiler to turn my divisions into shifts - makes the code
* easier to read.)
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
* <br>November 12, 2012 - Chars became strings (LEM)
*/
class clTreePopulation : public clPopulationBase {
  friend class clTreeSearch;
  friend class clTree;
  friend class clAllometry;

  private:
  /**
  * Copy constructor. Off limits!  It's too much to try to copy all the trees,
  * and too dangerous to allow two objects running around with pointers to the
  * same trees.
  */
  clTreePopulation(const clTreePopulation &oldPop);

  public:

  /**
  * Destructor.
  */
  ~clTreePopulation();

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clTreePopulation(clSimManager *p_oSimManager);

  /**Tree life history stages, or tree type.*/
  enum iTreeType {seed, /**<Seed. Currently not supported*/
                  seedling, /**<Trees shorter than 1.35 m tall and have no DBH*/
                  sapling, /**<Trees with a DBH smaller than the minimum
                        adult DBH for growth.*/
                  adult, /**<Trees with a DBH equal to or greater than the minimum
                        adult DBH for growth.*/
                  stump, /**<Trees cut by harvest. Stumps only last one timestep.
                        They are currently only used in dispersal.*/
                  snag, /**<Standing dead tree.*/
                  woody_debris  /**<Downed dead tree. Currently not supported.*/
  };

  /**
  * Acquires trees which meet certain criteria.
  * Any function needing to act on trees uses this function to find them. A
  * clTreeSearch object is returned, and the trees can then be accessed
  * repeatedly by calling the clTreeSearch object's NextTree() method. Stumps
  * are not returned by this method. Use the GetFirstStump() method to access
  * them directly.
  *
  * To set up a search, a search string is passed to Find(). The string is
  * formatted as follows:
  * search term 1::search term 2:: ... search term n
  * where the search terms are described below. Only those terms of interest
  * are used; when a search term is ommitted, it is assumed that any value for
  * that term is acceptable.
  *
  * Valid terms:
  * <ul>
  * <li>all (all trees are returned)
  * <li>type=seed, seedling, sapling, adult, snag, woody_debris (as ints);
  * separate multiple choices with commas. Unrecognized type causes error to
  * be thrown.
  * <li>species=int(,int,int). Unrecognized species causes error to be thrown.
  * <li>distance=float FROM x=float,y=float::height=float. This will return
  * all trees less than the distance and greater than the height.
  * </ul>
  * Type and species may be combined or separate. Distance must be alone.
  *
  * Example:
  * Find("species=0,2,3::type=1") or
  * Find("distance=10.5 FROM x=5.5,y=5.6::height=4.5")
  *
  * If no trees are found as the result of a search, the NextTree() method of
  * the returned treeSearch object returns NULL.
  *
  * @param sArgs String with query terms for the search.
  * @throws Error if the query cannot be understood, such as an unrecognized
  * argument or text that cannot be turned into a number.
  */
  clTreeSearch* Find(std::string sArgs);

  /**
  * Creates a new tree and organizes it into the hash table.
  * If this is supposed to be a seedling, and the diam10 creates a height taller
  * than 1.35 m, the tree becomes a sapling. If the dbh is larger than the min
  * adult dbh and the type is given as sapling, the tree is created as an adult.
  *
  * @param fX X coordinate of the new tree
  * @param fY Y coordinate of the new tree
  * @param iSp Species number of the new tree
  * @param iType The type (life history stage) of the new tree
  * @param fDiam The diameter of the new tree. If this is a seedling, this is
  * diam at 10 cm. For all others, this is dbh. If this is set to 0, a random
  * diam10 is chosen.
  * @return Pointer to the newly created tree
  * @throw BAD_DATA error if:
  * <ul>
  * <li>Species is unrecognized
  * <li>Type (life history stage) is unrecognized
  * <li>Tree coordinates are outside of the plot
  * </ul>
  */
  clTree* CreateTree(float fX, float fY, int iSp, int iType, float fDiam);

  /**
  * Kills a tree. What happens to the tree depends on what kind it is, why it
  * died, and what behaviors are set up.
  * <ul>
  * <li>If the reason code is remove, the tree is deleted from memory.
  * <li>If a tree is a seedling, it is deleted from memory no matter why it
  * died.
  * <li>If a tree is a sapling or adult, the reason killed is harvest, and the
  * tree is of a species which makes stumps (indicated by the mp_bMakeStump
  * flag), the tree is removed from memory and a stump of the same DBH is
  * added to the stumps list.
  * <li>If the tree is a sapling killed by any reason other than harvest, or
  * if it is killed by harvest and the species does not make stumps, then it is
  * deleted from memory.
  * <li>If the tree is an adult killed by harvest and is of a species that does
  * not make stumps, it is deleted from memory.
  * <li>If the tree is an adult killed by reason "natural", "disease", "fire",
  * "storm", or "insects", and snags are supported (m_bMakeSnags = true), the
  * tree is replaced by a snag with the same DBH and height.
  * <li>If the tree is an adult killed by reason "natural", "disease", "fire",
  * "storm", or "insects", and snags are not supported (m_bMakeSnags = false),
  * the tree is removed from memory.
  * <li>If the tree is a snag, it is removed from memory.
  * </ul>
  * No matter what the reason, a copy of the tree is transferred to the ghost
  * tree population.
  * @param p_deadTree Tree to delete.
  * @param iReason Why the tree is being deleted. This should be an enum from
  * iWhyDead.
  * @return Tree that was killed, if it became a snag or a stump; otherwise
  * NULL is returned.
  */
  clTree* KillTree(clTree *p_deadTree, deadCode iReason);

  /**
  * Reads type and species info from the parameter file.  This function should
  * be called before GetData. This is because behaviors need to validate
  * against species and type in the CreateObjects phase.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void BarebonesDataSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Translates a species name string into its code.
  *
  * @param sSpeciesName Species name to translate.
  * @return Code of the species, or -1 if the species isn't recognized.
  */
  short int TranslateSpeciesNameToCode(std::string sSpeciesName);

  /**
  * Translates a species code into its name string.
  *
  * @param iSpecies Species code to translate.
  * @return Name string of species, or empty string if the code isn't
  * recognized.
  */
  std::string TranslateSpeciesCodeToName(int iSpecies);

  /**
  * Adds an integer data member to the tree data structure. A behavior must
  * only register as many data members as it indicated that it wanted to add in
  * its new data member variables. This does not check to make sure that there
  * is not already a variable registered with that name.
  *
  * @param sLabel The name of the new data member - keep it short.
  * @param iSpecies Species to register the new variable for
  * @param iType Tree type to register the new variable for
  * @return A code that will be used to access values of that type in the
  * future for that species/type combo. This is faster than trying to do
  * lookups by character string.
  * @throw ILLEGAL_OP error if there is no more space for new data members.
  */
  short int RegisterInt(std::string sLabel, int iSpecies, int iType);

  /**
  * Adds a float data member to the tree data structure. A behavior must only
  * register as many data members as it indicated that it wanted to add in its
  * new data member variables. This does not check to make sure that there is
  * not already a variable registered with that name.
  *
  * @param sLabel The name of the new data member - keep it short.
  * @param iSpecies Species to register the new variable for
  * @param iType Tree type to register the new variable for
  * @return A code that will be used to access values of that type in the
  * future for that species/type combo. This is faster than trying to do
  * lookups by character string.
  * @throw ILLEGAL_OP error if there is no more space for new data members.
  */
  short int RegisterFloat(std::string sLabel, int iSpecies, int iType);

  /**
  * Adds a char data member to the tree data structure. A behavior must only
  * register as many data members as it indicated that it wanted to add in its
  * new data member variables. This does not check to make sure that there is
  * not already a variable registered with that name.
  *
  * @param sLabel The name of the new data member - keep it short.
  * @param iSpecies Species to register the new variable for
  * @param iType Tree type to register the new variable for
  * @return A code that will be used to access values of that type in the
  * future for that species/type combo. This is faster than trying to do
  * lookups by character string.
  * @throw ILLEGAL_OP error if there is no more space for new data members.
  */
  short int RegisterChar(std::string sLabel, int iSpecies, int iType);

  /**
  * Adds a boolean data member to the tree data structure. A behavior must
  * only register as many data members as it indicated that it wanted to add in
  * its new data member variables. This does not check to make sure that there
  * is not already a variable registered with that name.
  *
  * @param sLabel The name of the new data member - keep it short.
  * @param iSpecies Species to register the new variable for
  * @param iType Tree type to register the new variable for
  * @return A code that will be used to access values of that type in the
  * future for that species/type combo. This is faster than trying to do
  * lookups by character string.
  * @throw ILLEGAL_OP error if there is no more space for new data members.
  */
  short int RegisterBool(std::string sLabel, int iSpecies, int iType);

  /**
  * Gets the data code for accessing an integer tree data member. This code
  * can be used for accessing its value through the tree's GetValue() and
  * SetValue() functions. This function will not be responsible for duplicate
  * labels.
  *
  * @param sLabel The data member's label.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code, or -1 if the label is unrecognized.
  * @throw An error if an unrecognized species or type is passed.
  */
  short int GetIntDataCode(std::string sLabel, int iSpecies, int iType);

  /**
  * Gets the data code for accessing a float tree data member. This code can
  * be used for accessing its value through the tree's GetValue() and
  * SetValue() functions. This function will not be responsible for duplicate
  * labels.
  *
  * @param sLabel The data member's label.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code, or -1 if the label is unrecognized.
  * @throw An error if an unrecognized species or type is passed.
  */
  short int GetFloatDataCode(std::string sLabel, int iSpecies, int iType);

  /**
  * Gets the data code for accessing a string tree data member. This code can be
  * used for accessing its value through the tree's GetValue() and SetValue()
  * functions. This function will not be responsible for duplicate labels.
  *
  * @param sLabel The data member's label.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code, or -1 if the label is unrecognized.
  * @throw An error if an unrecognized species or type is passed.
  */
  short int GetStringDataCode(std::string sLabel, int iSpecies, int iType);

  /**
  * Gets the data code for accessing a bool tree data member. This code can be
  * used for accessing its value through the tree's GetValue() and SetValue()
  * functions. This function will not be responsible for duplicate labels.
  *
  * @param sLabel The data member's label.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code, or -1 if the label is unrecognized.
  * @throw An error if an unrecognized species or type is passed.
  */
  short int GetBoolDataCode(std::string sLabel, int iSpecies, int iType);

  /**
  * Gets the label for an integer tree data member. This function will not be
  * responsible for duplicate labels.
  *
  * @param iCode The data member's label.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member label, or an empty string if the code is unrecognized.
  * @throw An error if an unrecognized species or type is passed.
  */
  std::string GetIntDataLabel(short int iCode, int iSpecies, int iType);

  /**
  * Gets the label for a float tree data member.
  * This function will not be responsible for duplicate labels.
  *
  * @param iCode The data member's label.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member label, or an empty string if the code is unrecognized.
  * @throw An error if an unrecognized species or type is passed.
  */
  std::string GetFloatDataLabel(short int iCode, int iSpecies, int iType);

  /**
  * Gets the label for a string tree data member. This function will not be
  * responsible for duplicate labels.
  *
  * @param iCode The data member's label.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member label, or an empty string if the code is unrecognized.
  * @throw An error if an unrecognized species or type is passed.
  */
  std::string GetStringDataLabel(short int iCode, int iSpecies, int iType);

  /**
  * Gets the label for a bool tree data member.
  * This function will not be responsible for duplicate labels.
  *
  * @param iCode The data member's label.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member label, or an empty string if the code is unrecognized.
  * @throw An error if an unrecognized species or type is passed.
  */
  std::string GetBoolDataLabel(short int iCode, int iSpecies, int iType);

  /**
  * Determines if the hash table needs to be sorted by checking
  * the bDoUpdates flag, and if it does, it sorts it.
  */
  void DoDataUpdates();

  /**
   * Deletes all trees from the hash table without deleting the table itself.
   */
  void EmptyHashTable();

  /**
   * Sorts the hash table by height.
   * This does a complete sort of the entire hash table using the Insertion Sort
   * algorithm, which is good for almost-sorted datasets, which the hash table
   * probably will be. This function will be used to sort the hash table if a
   * behavior has been updating trees and has let them get out of order.
   */
  void SortHashTable();

  /**
  * Gets a random diameter at 10 cm value for a seedling. This function
  * slightly randomizes the diameter at 10 cm value around a "seed" value.
  *
  * @param fDiam10Seed The "seed" value around which to randomize the new
  * value. Optional. If no "seed" is passed, the new seedling diam10 value is
  * used.
  * @return Random diameter at 10 cm.
  */
  float GetRandomDiam10Value(float fDiam10Seed = 0);

  /**
  * Gets the code for the "X" data member, which is registered by the tree
  * population.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code.
  * @throw Error if given bad species or type codes.
  */
  short int GetXCode(int iSpecies, int iType);

  /**
  * Gets the code for the "Y" data member, which is registered by the tree
  * population.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code.
  * @throw Error if given bad species or type codes.
  */
  short int GetYCode(int iSpecies, int iType);

  /**
  * Gets the code for the "Height" data member, which is registered by the tree
  * population.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code.
  * @throw Error if given bad species or type codes.
  */
  short int GetHeightCode(int iSpecies, int iType);

  /**
  * Gets the code for the "DBH" data member, which is registered by the tree
  * population.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code.
  * @throw Error if given bad species or type codes.
  */
  short int GetDbhCode(int iSpecies, int iType);

  /**
  * Gets the code for the "Diam10" data member, which is registered by the tree
  * population.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code.
  * @throw Error if given bad species or type codes.
  */
  short int GetDiam10Code(int iSpecies, int iType);

  /**
  * Gets the code for the "Crown Radius" data member, which is registered by the
  * tree population.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code.
  * @throw Error if given bad species or type codes.
  */
  short int GetCrownRadiusCode(int iSpecies, int iType);

  /**
  * Gets the code for the "Crown Depth" data member, which is registered by the
  * tree population.
  * @param iSpecies The species.
  * @param iType The tree type.
  * @return Data member code.
  * @throw Error if given bad species or type codes.
  */
  short int GetCrownDepthCode(int iSpecies, int iType);

  /**
  * Gets the code for the "Why dead" snag data member, which is registered by
  * the tree population.
  * @param iSpecies The species.
  * @return Data member code.
  * @throw Error if given a bad species code.
  */
  short int GetWhyDeadCode(int iSpecies);

  /**
  * Gets the code for the "Age" snag data member, which is registered by
  * the tree population.
  * @param iSpecies The species.
  * @return Data member code.
  * @throw Error if given a bad species code.
  */
  short int GetAgeCode(int iSpecies);

  /**
  * Gets the minimum adult DBH.
  *
  * @param iSpecies Species for which to get the minimum adult DBH.
  * @return Minimum adult DBH in cm.
  * @throw Error if the species isn't valid.
  */
  float GetMinAdultDBH(int iSpecies);

  /**
  * Gets the allometry object.
  *
  * @return The allometry object.
  */
  clAllometry* GetAllometryObject() {return mp_oAllom;};

  /**
  * Gets the total number of species.
  *
  * @return Total number of species.
  */
  int GetNumberOfSpecies() {return m_iNumSpecies;};

  /**
  * Gets the total number of tree types (life history stages).
  *
  * @return Total number of tree types.
  */
  int GetNumberOfTypes() {return m_iNumTypes;};

  /**
  * Gets the tree population internal grid cell size (grids are square).
  *
  * @return Grid cell size in meters.
  */
  int GetGridCellSize() {return m_iLengthGrids;};

  /**
  * Gets the number of cells in the X direction for the tree population's
  * internal tree organizational grid.
  *
  * @return Number of cells in the X direction.
  */
  int GetNumXCells() {return m_iNumXCells;};

  /**
  * Gets the number of cells in the Y direction for the tree population's
  * internal tree organizational grid.
  *
  * @return Number of cells in the Y direction.
  */
  int GetNumYCells() {return m_iNumYCells;};

  /**
  * Gets whether or not this run uses snags.
  * @return true if snags will be made from dead trees, false if they will not.
  */
  bool GetUsesSnags() {return m_bMakeSnag;};

  /**
   * Gets the number of size classes defined.
   * @return Number of size classes.
   */
  int GetNumberSizeClasses() {return m_iNumSizeClasses;};

  /**
   * Gets a particular size class, up to number of size classes (0 indexing).
   * @throw Error if the size class number is not valid.
   * @return The size class.
   */
  float GetSizeClass(int iSizeClass) {
    if (iSizeClass < 0 || iSizeClass >= m_iNumSizeClasses) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::GetSizeClass";
      std::stringstream s;
      s << "Invalid tree size class number: \"" << iSizeClass << "\".";
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    return mp_fSizeClasses[iSizeClass];
  };

  /**
  * Gets the length in the X direction of the tree's internal grid (should
  * match the value received from the plot object).
  *
  * @return X length of plot in meters.
  */
  float GetXPlotLength() {return m_fPlotLengthX;};

  /**
  * Gets the length in the Y direction of the tree's internal grid (should
  * match the value received from the plot object).
  *
  * @return Y length of plot in meters.
  */
  float GetYPlotLength() {return m_fPlotLengthY;};

  /**
  * Gets the maximum possible seedling height for a species.
  * @param iSpecies Species for which to get the maximum height.
  * @return Maximum possible seedling height, in meters.
  */
  float GetMaxSeedlingHeight(int iSpecies) {return mp_fMaxSeedlingHeight[iSpecies];};

  /**
  * Gets the minimum possible adult height.
  * @return The smallest possible adult height, in meters.
  */
  float GetMinimumAdultHeight() {return m_fMinAdultHeight;};

  /**
  * Gets the default new seedling diameter at 10 cm.
  * @return Default new seedling diameter at 10 cm.
  */
  float GetNewSeedlingDiam10() {return m_fNewSeedlingDiam10;};

 /**
  * Gets the max sapling height.
  * @return The max sapling height.
  */
  float GetMaxSaplingHeight() {return m_fMaxSaplingHeight;}

  /**
  * Allows direct access to the tree hash table.
  * @param iX X cell number of tree population internal grid cell.
  * @param iY Y cell number of tree population internal grid cell.
  * @return Shortest tree in the internal cell, or NULL if there are no trees
  * in that cell.
  */
  clTree* GetShortestTreeInCell(int iX, int iY);

  /**
  * Allows direct access to the tree hash table.
  * @param iX X cell number of tree population internal grid cell.
  * @param iY Y cell number of tree population internal grid cell.
  * @return Tallest tree in the internal cell, or NULL if there are no trees in
  * that cell.
  */
  clTree* GetTallestTreeInCell(int iX, int iY);

  clTree* GetShortestInCellHeightDiv(int iX, int iY, int iDiv) {return mp_oTreeShortest[iX][iY][iDiv];}
  clTree* GetTallestInCellHeightDiv(int iX, int iY, int iDiv) {return mp_oTreeTallest[iX][iY][iDiv];}

  /**
  * Gets first stump in the stump linked list.
  * @return First stump in the stump linked list.
  */
  clTree* GetFirstStump(){return mp_oStumps;};

///////////////////////////////////////////////////////////////////////////
//                            PROTECTED
///////////////////////////////////////////////////////////////////////////
  protected:


  /**Holds open tree searches.*/
  struct stcOpenSearches {
     clTreeSearch *p_oSearch;  /**<Pointer to a search*/
     stcOpenSearches *p_nextSearch; /**<Pointer to the next search on the list*/
   } *mp_openSearches; /**<The linked list of open search requests. The first
   record is always a dummy record.*/
  double m_fNewSeedlingDiam10; /**<New diameter at 10 cm for seedlings. Actual
        values are randomized around this value. From the parameter file.*/
  float m_fMinAdultHeight; /**<Shortest possible adult tree height, in meters.*/
  float m_fMaxSaplingHeight; /**<Maximum possible height a sapling can have, in
        meters. In practice this number is actually a smidge taller than the
        tallest height.*/
  double *mp_fMaxSeedlingHeight; /**<Maximum possible seedling height, in meters*/
  double *mp_fMinAdultDbh; /**<Minimum adult DBH, in cm, for each species.
        From parameter file.*/
  float m_fPlotLengthX;  /**<Plot length in X dimension, in meters.*/
  float m_fPlotLengthY;  /**<Plot length in Y dimension, in meters.*/
  float *mp_fSizeClasses; /**<Size classes for live trees. Number of size
        classes is user-settable. The value in each bin of the array is the
        upper limit in cm of the dbh for that class. There can also
        be a size class each for seeds and seedlings.*/

  int m_iNumHeightDivs; /**<Number of height divisions in tree hash table.*/
  int m_iSizeHeightDivs; /**<size of height divisions in tree hash table, in meters.*/
  int m_iNumSpecies;   /**<Number of species. From parameter file.*/
  int m_iNumTypes;  /**<Number of tree types (life history stages).*/
  int m_iNumXCells; /**<Mumber of tree population internal grid cells in the
        X direction.*/
  int m_iNumYCells; /**<Mumber of tree population internal grid cells in the
        Y direction.*/
  int m_iNumSizeClasses; /**<Number of tree size classes defined.*/
  int m_iLengthGrids; /**<Length of internal grid cells, in meters. Make
        this always a power of 2!*/
  bool m_bDoUpdates; /**<Whether or not to sort the hash table when DoDataUpdates
                   is called.*/
  bool *mp_bMakeStump; /**<Whether a harvested tree should be made into a stump,
        for each species.*/
  bool m_bMakeSnag; /**<Whether a dead adult should be made into a snag*/

  short int **mp_iNumTreeIntVals; /**<Number of tree integer data members that
        have been defined for this run. Array size is number of species by
        number of types.*/
  short int **mp_iNumTreeFloatVals; /**<Number of tree float data members that
        have been defined for this run. Array size is number of species by
        number of types.*/
  short int **mp_iNumTreeStringVals; /**<Number of tree string data members that
        have been defined for this run. Array size is number of species by
        number of types.*/
  short int **mp_iNumTreeBoolVals; /**<Number of tree bool data members that
        have been defined for this run. Array size is number of species by
        number of types.*/

  /**List of tree integer data member labels. Array size is number of species
   * by number of types by number of integer data members. The code for a data
   * member equals the third array index.*/
  std::string ***mp_sIntLabels;

  /**List of tree float data member labels. Array size is number of species by
   * number of types by number of integer data members. The code for a data
   * member equals the third array index.*/
  std::string ***mp_sFloatLabels;

  /**List of tree string data member labels. Array size is number of species by
   * number of types by number of integer data members. The code for a data
   * member equals the third array index.*/
  std::string ***mp_sStringLabels;

  /**List of tree bool data member labels. Array size is number of species by
   * number of types by number of integer data members. The code for a data
   * member equals the third array index.*/
  std::string ***mp_sBoolLabels;

  short int **mp_iXCode; /**<Codes for "X" tree data member. Array size is
        number of species by number of types.*/
  short int **mp_iYCode; /**<Codes for "Y" tree data member. Array size is
        number of species by number of types.*/
  short int **mp_iHeightCode; /**<Codes for "Height" tree data member. Array size is
        number of species by number of types.*/
  short int **mp_iDiam10Code; /**<Codes for "Diam10" tree data member. Array size is
        number of species by number of types.*/
  short int **mp_iDbhCode;  /**<Codes for "DBH" tree data member. Array size is
        number of species by number of types.*/
  short int **mp_iCrownRadCode;  /**<Codes for "Crown Radius" tree data member.
         Array size is number of species by number of types.*/
  short int **mp_iCrownDepthCode;  /**<Codes for "Crown Depth" tree data member.
         Array size is number of species by number of types.*/
  short int *mp_iAgeCode; /**<Codes for "Age" snag tree data member. Array
        size is number of species.*/
  short int *mp_iWhyDeadCode; /**<Codes for "Why dead" snag tree data member.
        Array size is number of species.*/

  /**
   * Triggers the tree data member registration process for all behaviors.
   */
  void DataMemberRegistrations();

  /**
  * Performs setup. If the input file is a parameter file, this calls:
  * <ul>
  * <li>GetPlotDimensions() to get plot dimensions;</li>
  * <li>ReadParameters() to get parameter file data;</li>
  * <li>SetupCalculations() to perform other calculations;</li>
  * <li>CreateHashTable() to allocate hash table memory;</li>
  * <li>DoTreeDataStructureSetup() to create the tree data structure;</li>
  * <li>DoDataMemberRegistrations() to perform the data member
  * registrations;</li>
  * <li>CreateTreesFromInitialDensities() and CreateTreesFromTreeMap() to
  * create trees.</li>
  * </ul>
  *
  * If the input file is a tree map or detailed output file, only the
  * CreateTrees... functions are called.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs end-of-timestep cleanup tasks.
  * <ul>
  * <li>Deletes all open tree searches from the last timestep</li>
  * <li>Deletes all stumps.</li>
  * <li>Updates the ages of all snags in years.</li>
  * <li>Resets all crown radius and depth values to -1.</li>
  * </ul>
  */
  void TimestepCleanup();

  /**
  * Retrieves the plot dimensions.
  * @throws modelErr if the plot object cannot be found.
  */
  void GetPlotDimensions();

  /**
  * Performs setup calculations. This function creates the clAllometry object,
  * and calculates m_fMinAdultHeight and m_fMaxSaplingHeight.
  */
  void SetupCalculations();

  /**
  * Allocates memory for the hash table. GetPlotDimensions() must be called
  * first.
  */
  void CreateHashTable();


  /**
   * Core function for registering new tree data members.
   * @param sLabel Name of the new data member.
   * @param iSpecies Species for which to register the new data member
   * @param iType Tree type (life history stage) for which to register the new
   * data member
   * @param p_iNumTreeVals Pointer to the correct mp_iNumTree[x]Vals array
   * @param p_sLabels Pointer to the correct mp_s[x]Labels array
   * @return Code that will be used to access values of that type in the future
   * for that species/type combo. This is faster than trying to do lookups
   * by character string.
   * @throw BAD_DATA or ILLEGAL_OP error if the registration was unsuccessful.
   */
  short int RegisterDataMember(std::string sLabel, int iSpecies, int iType,
   short int **p_iNumTreeVals, std::string ***p_sLabels);

  /**
   * Queries all behaviors for data members that they wish to add to the tree's
   * data structure. The data members to be added are only added for the
   * species and types that the behavior works on. This will also check for any
   * behaviors that work on stumps and snags. If any are found, the appropriate
   * bucket in mp_bMakeStump is set to true or m_bMakeSnag is set to true,
   * respectively.
   *
   * The tree population also uses this opportunity to add its own tree data
   * member registrations. It registers "Diam10" for seedlings and saplings;
   * "DBH" for saplings, adults, stumps, and snags; "Height" for seedlings,
   * saplings, adults, and snags; "Age" and "Why dead" for snags; and "X" and
   * "Y" for seedlings, saplings, adults, stumps, and snags.
   */
  void DoTreeDataStructureSetup();

  /**For maintaining a list of species and their codes.*/
  struct speciesCodes {unsigned short int iCode; /**<Species code*/
                       std::string sName; /**<Species name string*/
                       };
  speciesCodes *mp_speciesCodes; /**<List of species codes and names.*/

  /**Allometry object*/
  clAllometry *mp_oAllom;

  /**
  * Allocates memory and reads data from the parameter file into arrays.
  */
  void ReadParameters(xercesc::DOMDocument *p_oDoc);

  /**
  * Creates trees according to initial density information in an input file.
  * If initial density information it is not present, no trees are created.
  * This is not considered an error condition. In addition to as many DBH size
  * classes as the user wants, this can also handle three height classes for
  * seedling initial densities.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void CreateTreesFromInitialDensities(xercesc::DOMDocument *p_oDoc);

  /**
    * Creates snags according to initial density information in an input file.
    * If initial density information it is not present, no trees are created.
    * This is not considered an error condition. In addition to as many DBH size
    * classes as the user wants, this can also handle three height classes for
    * seedling initial densities.
    *
    * @param p_oDoc DOM tree of parsed input file.
    */
    void CreateSnagsFromInitialDensities(xercesc::DOMDocument *p_oDoc);

  /**
  * Creates trees according to tree map information in an input file.
  * If tree map information it is not present, no trees are created. This
  * is not considered an error condition.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throw BAD_DATA error if the tree map data member codes do not match those
  * already defined.
  */
  void CreateTreesFromTreeMap(xercesc::DOMDocument *p_oDoc);

  /**
   * Creates trees by reading in a text tree map, if such a map has been
   * specified. This type of tree map file is a tab-delimited text file, with
   * the first row being column headers. The columns are in the following
   * order: X, Y, Species (as a text string), Type (as a text string), Diam,
   * and Height. If height is 0, the allometric equations will set it.
   * @param p_oDoc DOM tree of parsed input file.
   * @throw BAD_FILE if the file can't be found. BAD_DATA if coordinates,
   * diameters, or heights are negative, or if the species or type is
   * unrecognized.
   */
  void CreateTreesFromTextTreeMap(xercesc::DOMDocument *p_oDoc);

  /**
  * Adds a tree to the hash table.
  *
  * @param p_oNewTree Tree to add to hash table.
  */
  void AddTreeToHashTable(clTree *p_oNewTree);

  /**
  * Updates a tree's position in the hash table.
  *
  * p_oChangedTree Tree whose position is to be updated.
  */
  void UpdateTreeInHashTable(clTree *p_oChangedTree);

  /**
  * Removes a tree from the hash table. The gap it left in the linked list is
  * closed and the tallest/shortest markers are updated as necessary.
  *
  * @param p_oByeTree Tree to remove from hash table.
  */
  void RemoveTreeFromHashTable(clTree *p_oByeTree);

  /**
  * Deletes stumps from the stump linked list and sets the linked list pointer
  * to NULL.
  */
  void DeleteStumps();

  /**
  * Changes a tree's type (life history stage).
  * This will change a tree's type and copy over all the applicable variables.
  * If a variable has no counterpart in the new type, it will be discarded.
  * Allometric values will not be recalculated (with the exception of the
  * seedling to sapling transition, where the new dbh value will be calculated
  * and assigned).
  *
  * @param p_oTree The tree whose type will be changed
  * @param iNewType The new tree type
  */
  void ChangeTreeType(clTree *p_oTree, enum iTreeType iNewType);

  /**
  * Updates an integer data member of a tree.
  * If a tree receives a request to be updated, it passes it off to the tree
  * population using this function.
  *
  * @param p_oTree The tree to be updated.
  * @param iCode The data member code - as passed to the tree.
  * @param iValue The new value - as passed to the tree.
  * @throws modelErr if the code is invalid.
  */
  void UpdateTree(clTree *p_oTree, short int iCode, int iValue);

  /**
  * Updates a float data member of a tree.
  * If a tree receives a request to be updated, it passes it off to the tree
  * population using this function. The default is for this function to
  * update the other parameters based on allometric relationships (for
  * instance, if DBH is changed, height will automatically be updated too). If
  * the update causes a tree move to a new life history stage, it will
  * automatically be transitioned. So, if setting a value of either diam10 or
  * height on a seedling causes it to be greater than the seedling height
  * cutoff, the tree will become a sapling and all of its allometric parameters
  * will be recalculated to match the value that was set.
  *
  * You can override automatic allometry updating. If you do that, be aware
  * that none of the other tree data members will be updated and you will need
  * to do that yourself. (<b>THERE IS ONE EXCEPTION.</b>  For saplings, DBH
  * and diam10 are always kept in sync.)  Be especially careful when there is a
  * possibility of tree type transition. If the tree is a seedling and you set
  * its height to a value greater than the height cutoff, it will transition,
  * but its diam10 will not change. If the tree is a sapling and you set its
  * DBH to be greater than the minimum adult DBH, it will transition. The
  * setting of any other value will not cause transition. So, if you give a
  * seedling a diam10 value that would, under automatic allometry updating,
  * give it a height greater than the cutoff and cause it to become a sapling,
  * the seedling will not become a sapling unless you explicitly set the height
  * too.
  *
  * @param p_oTree The tree to be updated.
  * @param iCode The data member code - as passed to the tree.
  * @param fValue The new value - as passed to the tree.
  * @param bUpdateNow Whether or not the tree's hash table needs to be updated
  *       right away. If not, the hash table will be updated all at once later.
  * @param bUpdateAllometry If the change involves allometric changes, whether or
  * not to automatically update the tree's other dimensions. For example, if
  * this is set to false and you are updating a DBH value, tree height will not
  * automatically be re-calculated.
  * @throws modelErr if the code is invalid, or if an X or Y value is trying to
  * be changed.
  */
  void UpdateTree(clTree *p_oTree, short int iCode,float fValue, bool bUpdateNow, bool bUpdateAllometry);

  /**
   * Updates a char data member of a tree.
   * If a tree receives a request to be updated, it passes it off to the tree
   * population using this function.
   *
   * @param p_oTree The tree to be updated.
   * @param iCode The data member code - as passed to the tree.
   * @param sValue The new value - as passed to the tree.
   * @throws modelErr if the code is invalid.
   */
  void UpdateTree(clTree *p_oTree,short int iCode, std::string sValue);

  /**
   * Updates a bool data member of a tree.
   * If a tree receives a request to be updated, it passes it off to the tree
   * population using this function.
   *
   * @param p_oTree The tree to be updated.
   * @param iCode The data member code - as passed to the tree.
   * @param bValue The new value - as passed to the tree.
   * @throws modelErr if the code is invalid.
   */
  void UpdateTree(clTree *p_oTree, short int iCode, bool bValue);

  clTree ****mp_oTreeTallest;  /**<Hash table of tallest tree links.
        Array size is number of X grid cells by number of Y grid cells
        by number of height divisions. For instance, [2][3] is the grid cell
        which runs from X = 16-23 m and Y = 24-31 m. The tree pointer starts
        at the tallest, and the tree's "shorter" pointer can be used to run
        downwards through all the trees in the cell. Tree shorter pointers run
        across height divisions.*/
  clTree ****mp_oTreeShortest; /**<Hash table of shortest tree links.
        Array size is number of X grid cells by number of Y grid cells
        by number of height divisions. For instance, [2][3] is the grid cell
        which runs from X = 16-23 m and Y = 24-31 m. The tree pointer starts
        at the shortest, and the tree's "taller" pointer can be used to run
        downwards through all the trees in the cell. Tree taller pointers run
        across height divisions.*/
  clTree *mp_oStumps; /**<Linked list of stumps. Not sorted in any way.*/

  /** Link to the ghost population. */
  clGhostTreePopulation *mp_oGhosts;

}; //end of class treePopulation
//----------------------------------------------------------------------------
#endif

