#ifndef GhostTreePopulationH
#define GhostTreePopulationH

#include "PopulationBase.h"
#include "Constants.h"
#include "DeadTree.h"

class clTree;
class clDeadTree;

using namespace whyDead;

/**
* GHOST TREE POPULATION CLASS - Version 1
*
* This holds dead trees. They are held for a single timestep then eliminated
* at the end of the timestep. This holds seedlings through snags. Stumps are
* not kept.
*
* The trees are not sorted, just held in a simple linked list.
*
* Copyright 2010 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>December 21, 2010 - Created (LEM)
*/
class clGhostTreePopulation : public clPopulationBase {

  private:
  /**
  * Copy constructor.  Off limits!  It's too much to try to copy all the trees,
  * and too dangerous to allow two objects running around with pointers to the
  * same trees.
  */
  clGhostTreePopulation(const clGhostTreePopulation &oldPop);

  public:

  /**
  * Destructor.
  */
  ~clGhostTreePopulation();

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clGhostTreePopulation(clSimManager *p_oSimManager);


  /**
  * Creates a copy of a tree and adds it to this population. The original tree
  * is not touched or removed from the old population.
  *
  * @param p_oTree tree to copy and add.
  * @param iDeadReasonCode Why the tree died.
  */
  void AddTree(clTree *p_oTree, deadCode iDeadReasonCode);

  /**
  * Gets the first tree in the linked list.
  * @return First tree in the list, or NULL if there are no trees.
  */
  clDeadTree* GetFirstTree() {return mp_oTrees;};

  /**
  * Not needed.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc) {;};


///////////////////////////////////////////////////////////////////////////
//                            PROTECTED
///////////////////////////////////////////////////////////////////////////
  protected:

  /**
  * Deletes all trees.
  */
  void TimestepCleanup();

  /** Linked list of dead trees */
  clDeadTree *mp_oTrees;

}; //end of class GhostTreePopulation
//----------------------------------------------------------------------------
#endif

