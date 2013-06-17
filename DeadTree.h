//---------------------------------------------------------------------------

#ifndef DeadTreeH
#define DeadTreeH
#include <string>
#include "DataTypes.h"

using namespace whyDead;

/**
* Dead tree class - Version 1.0
*
* One object of this class represents one individual dead tree in the model. The
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
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>January 6, 2011 - Created (LEM)
*/
class clDeadTree {
  friend class clTreePopulation;
  friend class clGhostTreePopulation;
  friend class clTree;
  public:

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
  * Get death reason code of tree.
  * @return Type, as a member of whyDead.
  */
  deadCode GetDeadReasonCode(){return m_iDeadCode;};

  /**
  * Set death reason code of tree.
  * @param iCode Code, as a member of whyDead.
  */
  void SetDeadReasonCode(deadCode &iCode) {m_iDeadCode = iCode;};

 /**
  * Get tree taller than this one.
  * @return Next taller tree, or NULL if there isn't one.
  */
  clDeadTree* GetNext() {return mp_oNext;};


  protected:
 /**
  * Constructor.  Arrays are sized here.  All values will be initialized
  * to 0, empty string, and false.
  *
  * @param iType New tree's type.
  * @param iSpecies New tree's species.
  * @param iNumFloats Number of floats this tree will have.
  * @param iNumInts Number of integers this tree will have.
  * @param iNumStrings Number of chars this tree will have.
  * @param iNumBools Number of bools this tree will have.
  */
  clDeadTree(int iType, int iSpecies, int iNumFloats, int iNumInts, int iNumStrings,
    int iNumBools);

 /**
  * Destructor.  Deletes arrays.
  */
  ~clDeadTree();

  short unsigned int m_iSpecies; /**<Species*/
  short unsigned int m_iType; /**<Type*/
  deadCode m_iDeadCode; /**<Death reason code*/
  //These are the arrays of values that can be registered as part of this class
  //The char array should be an array of pointers, not actual values - thus the
  //double pointer
  float *mp_fFloatValues; /**<Array of float data members*/
  int *mp_iIntValues; /**<Array of integer data members*/
  std::string *mp_sStringValues; /**<Array of string data members*/
  bool *mp_bBoolValues; /**<Array of bool data members*/
  clDeadTree* mp_oNext; /**<Pointer to next tree in linked list of tree population*/

}; //end of class clDeadTree
//---------------------------------------------------------------------------
#endif
