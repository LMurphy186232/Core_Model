//---------------------------------------------------------------------------
#ifndef DisperseBaseH
#define DisperseBaseH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"

class clGrid;

class clDisperseOrg;

/**
* Disperse base - Version 1.0
*
* This is the base class for disperse behaviors.  This holds common data.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clDisperseBase : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  friend class clDisperseOrg;

  public:

  /**
  * Constructor.  Sets the hook flag.
  */
  clDisperseBase(clSimManager *p_oSimManager);

  /**
  * Destroys common objects.
  */
  virtual ~clDisperseBase();


  /**
  * Performs all disperse calculations.  This will be the same for all
  * descendent classes - they do not need to  override.  If a particular object
  * is hooked, it calls the disperse org object's DoDisperse.  Otherwise
  * it does nothing.
  */
  void Action();

  /**
   * Resets all values in the seed grid to 0 and sets the static
   * m_bUpdatedGapStatus to false if this is a hooked object.
   */
 void TimestepCleanup();

  protected:

  /**What PDF is used by a species*/
  enum function {weibull, /**<Weibull function type*/
                lognormal /**<Lognormal function type*/
                };

  /**Cover status*/
  enum cover {gap, /**<Gap cover*/
              canopy /**<Canopy cover*/
             };

  /**<Whether or not this shell object is hooked to clDisperseOrg.
   * clDisperseOrg will set this flag.*/
  bool m_bHooked;

  /**
   * Triggers all disperse setup if an object is hooked.  This will be the same
   * for all descendent classes.  If a particular object is hooked, it calls the
   * disperse org object's DoSetup() function, which calls the function
   * DoShellSetup() - if a descendent class has specific setup needs, it can
   * overload that function.
   * @param p_oDoc DOM tree of parsed input file.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Setup for a descendent class.  If a descendent class has specific setup
   * needs, it can overload this function.
   * @param p_oDoc DOM tree of parsed input file.
   */
  virtual void DoShellSetup(xercesc::DOMDocument *p_oDoc) = 0;

  /**
  * Performs disperse and adds seeds to seed grid.
  */
  virtual void AddSeeds() = 0;



////////////////////////////////////////////////////////////////////////////
//Static section
//Just in case there are multiple instances of this object created in one run,
//they can share the seed grid and associated functions for dealing with it.
////////////////////////////////////////////////////////////////////////////

  /**clDisperseOrg object - this pointer is held in common by all shells*/
  static clDisperseOrg *mp_oDisperseOrg;

  /**
   * Seed grid.  This is the grid which contains the numbers and locations of
   * seeds produced by all parent trees.  The resolution defaults to the
   * standard grid default if it is not set by a grid map or grid map header.
   *
   * The grid's name is "Dispersed Seeds".  The grid is static, because multiple
   * instances of this object could be instantiated to handle different
   * dispersal situations.
   *
   * Data members:
   * <table>
   * <tr>
   * <th>Member name</th> <th>Data type</th> <th>Description</th>
   * </tr>
   * <tr>
   * <td>seeds_x</td>     <td>float</td>     <td>Number of seeds for each tree
   *                                         species.  "x" = species number.
   *                                         There is no check to make sure that
   *                                         each species actually uses disperse. </td>
   * </tr>
   * <tr>
   * <td>Is Gap</td>      <td>bool</td>     <td>Gap status of each grid cell.</td>
   * </tr>
   * <tr>
   * <td>count</td>       <td>int</td>     <td>Count of reproductively mature
   *                                       trees that use disperse.  This number
   *                                       is used to determine gap status.</td>
   * </tr>
   * </table>
   */
  static clGrid *mp_oSeedGrid;

  /**Data member codes for "dispersed seeds" grid for "seeds_x" data member.
   * Array size is # species.*/
  static short int *mp_iNumSeedsCode;

  /**Data member code - for counting trees for gap status*/
  static short int m_iGapCountCode;

  /**Data member code for substrate grid for gap status.*/
  static short int m_iIsGapCode;

  /**Total number of species*/
  static short int m_iTotalSpecies;

   /**
   * A flag indicating whether or not the gap status of the seed grid has been
   * updated this timestep.  This will be reset to false during the timestep
   * cleanup.
   */
  static bool m_bUpdatedGapStatus;

  /**Whether dispersal is stochastic (true) or deterministic (false).*/
  static bool m_bIsStochastic;

 /**
  * Does disperse base setup.  Sets up the "dispersed_seeds" grid, if the
  * static pointer is NULL, and declares mp_fDbhForReproduction.
  */
  void SetUpBase();
};
//---------------------------------------------------------------------------

#endif
