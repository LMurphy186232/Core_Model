//---------------------------------------------------------------------------

#ifndef TreeH
#define TreeH
#include <string>

class clTreePopulation;
class clDeadTree;

/**
* Tree class - Version 1.0
*
* One object of this class represents one individual tree in the model.  The
* tree is essentially a data structure; the only actions it takes are to
* communicate changes in its status to the tree population in order to keep
* itself updated.
*
* The number of data members a tree has is dynamic; very little is defined
* ahead of time.  Species and type are, because these must be known to locate
* other values.
*
* Trees can't be created or destroyed except by the tree population.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
* <br>November 12, 2012 - Chars became strings (LEM)
*/
class clTree {
  friend class clTreePopulation;
  friend class clTreeTestBehavior; /**<For automated testing*/
  public:

  /**
   * Sets an integer value.
   *
   * @param iCode Data member code.
   * @param iValue Value to set.
   */
  void SetValue(short int iCode, int iValue);

 /**
  *  Sets a float value.  If the value being set is a size parameter (height,
  * DBH, etc), the default is for the other parameters to also be updated based
  * on allometric relationships (for instance, if DBH is changed, height will
  * automatically be updated too).  If the update causes a tree move to a new
  * life history stage, it will automatically be transitioned.  So, if setting a
  * value of either diam10 or height on a seedling causes it to be greater than
  * the seedling height cutoff, the tree will become a sapling and all of its
  * allometric parameters will be recalculated to match the value that was set.
  *
  * You can override automatic allometry updating.  If you do that, be aware
  * that none of the other tree data members will be updated and you will need
  * to do that yourself.  Be especially careful when there is a possibility of
  * tree type transition.  If the tree is a seedling and you set its height to a
  * value greater than the height cutoff, it will transition, but its diam10
  * will not change and it will have a DBH value of zero.  If the tree is a
  * sapling and you set its DBH to be greater than the minimum adult DBH, it
  * will transition. The setting of any other value will not cause transition.
  * So, if you give a seedling a diam10 value that would, under automatic
  * allometry updating, give it a height greater than the cutoff and cause it to
  * become a sapling, the seedling will not become a sapling unless you
  * explicitly set the height too.
  *
  * @param iCode Data member code.
  * @param fValue Value to set.
  * @param bUpdateNow Whether or not, if there are allometric changes to make,
  * they need to be performed immediately or can wait (so changes could
  * be grouped).
  * @param bUpdateAllometry If the change involves allometric changes, whether or
  * not to automatically update the tree's other dimensions.  For example, if
  * this is set to false and you are updating a DBH value, tree height will not
  * automatically be re-calculated.
  */
  void SetValue(short int iCode, float fValue, bool bUpdateNow = true, bool bUpdateAllometry = true);

 /**
  * Sets a boolean value.
  *
  * @param iCode Data member code.
  * @param bValue Value to set.
  */
  void SetValue(short int iCode, bool bValue);

 /**
  * Sets a string value.
  *
  * @param iCode Data member code.
  * @param sValue Value to set.
  */
  void SetValue(short int iCode, std::string sValue);

 /**
  * Gets an integer value.
  *
  * @param iCode Data member code.
  * @param p_iValHolder Variable into which to place the value.
  */
  void GetValue(short int iCode, int *p_iValHolder);

 /**
  * Gets a float value.
  *
  * @param iCode Data member code.
  * @param p_fValHolder Variable into which to place the value.
  */
  void GetValue(short int iCode, float *p_fValHolder);

 /**
  * Gets a boolean value.
  *
  * @param iCode Data member code.
  * @param p_bValHolder Variable into which to place the value.
  */
  void GetValue(short int iCode, bool *p_bValHolder);

 /**
  * Gets a string value.
  *
  * @param iCode Data member code.
  * @param p_sValHolder Variable into which to place the value.
  */
  void GetValue(short int iCode, std::string *p_sValHolder);

 /**
  * Get species of tree.
  * @return Species.
  */
  short unsigned int GetSpecies(){return m_iSpecies;};

 /**
  * Get type of tree.
  * @return Type, as a member of clTreePopulation::iTreeType.
  */
  short unsigned int GetType(){return m_iType;};

 /**
  * Get tree taller than this one.
  * @return Next taller tree, or NULL if there isn't one.
  */
  clTree* GetTaller() {return mp_oNext;};

 /**
  * Get tree shorter than this one.
  * @return Next shorter tree, or NULL if there isn't one.
  */
  clTree* GetShorter() {return mp_oPrevious;};

  /**
   * Makes a dead clone of this tree.
   * @return Clone.
   */
  clDeadTree* MakeDeadClone();

  protected:
 /**
  * Constructor.  Arrays are sized here.  All values will be initialized
  * to 0, empty string, and false.
  *
  * @param iType New tree's type.
  * @param iSpecies New tree's species.
  * @param iNumFloats Number of floats this tree will have.
  * @param iNumInts Number of integers this tree will have.
  * @param iNumStrings Number of strings this tree will have.
  * @param iNumBools Number of bools this tree will have.
  * @param p_oTreePop Tree population object.
  * @throw BAD_DATA error if a value is less than zero.
  */
  clTree(int iType, int iSpecies, int iNumFloats, int iNumInts, int iNumStrings,
    int iNumBools, clTreePopulation *p_oTreePop);

 /**
  * Destructor.  Deletes arrays.
  */
  ~clTree();

  short unsigned int m_iSpecies; /**<Species*/
  short unsigned int m_iType; /**<Type*/
  //These are the arrays of values that can be registered as part of this class
  //The char array should be an array of pointers, not actual values - thus the
  //double pointer
  float *mp_fFloatValues; /**<Array of float data members*/
  int *mp_iIntValues; /**<Array of integer data members*/
  std::string *mp_sStringValues; /**<Array of string data members*/
  bool *mp_bBoolValues; /**<Array of bool data members*/
  static clTreePopulation *mp_oTreePop; /**<Tree population.  Declare as static
  - we don't need a
  bazillion copies of this running around.  This is somewhat dangerous as we
  have to depend on the tree population to NULL this pointer in its own
  destructor - but worth it*/
  clTree* mp_oNext;     /**<Pointer to next tree in linked list of tree population*/
  clTree* mp_oPrevious; /**<Pointer to previous tree in linked list of tree pop*/

}; //end of class clTree
//---------------------------------------------------------------------------
#endif
