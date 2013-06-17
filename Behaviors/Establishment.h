//---------------------------------------------------------------------------
#ifndef EstablishmentH
#define EstablishmentH
//---------------------------------------------------------------------------

#include "BehaviorBase.h"

class clTreePopulation;
class clGrid;
class clTree;
class clModelMath;
/**
* Establishment - Version 1.0
*
* This behavior germinates seeds into seedlings.  For each species to which
* this behavior is applied, this behavior takes all seeds in each grid cell of
* the seed grid and randomly places them around the area encompassed by the
* grid cell.  The tree population is allowed to randomly pick a diameter at 10
* cm.
*
* To reduce memory requirements, a seedling efficiency routine can be turned
* on.  For each seedling created, it will immediately assess its probability
* of surviving the next timestep, based on conditions in this timestep
* (with certain exceptions).  If it is determined that the seedling is not
* likely to survive, it is removed right away.  This cuts down on the total
* number of seedlings that needs to be under management at any one time - which
* can be significant for particularly fecund trees; millions of seedlings can
* be created, of which only a few percent will survive the next timestep, but
* which suck RAM tremendously in the meantime.
*
* A fatal error is thrown if there are no disperse behaviors for the run.
*
* The namestring and parameter file call string for this class is
* "Establishment". Apply this behavior to species; use any type, since it will
* be ignored.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clEstablishment : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clEstablishment(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clEstablishment();

  /**
  * Does setup.  This reads in the "use efficiency routine" flag from the
  * parameter file and gets pointers to the dispersed seeds grid.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if there is not a disperse behavior used for each species
  * that gets establishment.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs establishment. All seeds get a random location within the grid
  * cell they are in and then become seedlings.
  */
  void Action();

  /**
   * Zeroes out the seed grid.
   */
  void TimestepCleanup();

  protected:

  /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
  clGrid *mp_oSeedGrid;

  /**Data member codes for seed grid for number of seeds.  Array size is #
  * total species.*/
  short int *mp_iSeedGridCode;

};
//---------------------------------------------------------------------------
#endif
