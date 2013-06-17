//---------------------------------------------------------------------------

#ifndef TreeRemoverH
#define TreeRemoverH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"



/**
 * Tree Killer - Version 2.0
 *
 * This behavior will remove all trees that have been marked for death by
 * mortality.  If a tree has a non-not dead value in its "dead" data member,
 * this will direct it to be removed from the tree population by calling
 * clTreePopulation::KillTree().  The reason code is that which is set in
 * "dead".
 *
 * If this is applied to any species/type combo that does not have "dead"
 * registered, an error will be thrown during setup.
 *
 * The namestring and parameter call string for this behavior are both
 * "removedead".
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clTreeRemover : virtual public clBehaviorBase {

  public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
  */
  clTreeRemover(clSimManager *p_oSimManager);

  /**
   * Destructor.
  */
  ~clTreeRemover();

  /**
  * Does all the tree killin'.  This goes through all trees in the tree
  * population to which this behavior is applied, and if any have a non-notdead
  * value in their "dead" data member, they are passed to
  * clTreePopulation::KillTree() with that reason code.
  */
  void Action();

  /**
  * Gets the data member codes for the "dead" data member.
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if a tree type/species combo to which this behavior is
  * applied has no "dead" code.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  protected:
  clTreePopulation *mp_oPop; /**<Stashed pointer to avoid having to keep getting it*/

  /** Return codes for the "dead" tree int data member variable.  Array size
   * is species by type (even if not every species and type is represented)*/
  short int **mp_iDeadCodes;
  short int m_iNumTotalSpecies; /**<Total number of species - for destructor*/
};

//---------------------------------------------------------------------------
#endif
