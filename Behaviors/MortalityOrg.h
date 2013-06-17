//---------------------------------------------------------------------------

#ifndef MortalityOrgH
#define MortalityOrgH
//---------------------------------------------------------------------------

class clMortalityBase;
class DOMDocument;
class clSimManager;
class clTreePopulation;

/**
* Mortality org - Version 1.1
*
* This class does the organizational work for tree mortaliy for a timestep.  It
* hooks into a mortality shell object and is triggered by that object when the
* shell is triggered by the behavior manager.
*
* An object of this class will then call each tree and, if any mortality
* applies, all mortality shell objects in turn will get a crack at the tree
* until the tree's marked for death or until it has completely run the gauntlet.
*
* Any tree specie/type combination to which any mortality behavior has been
* applied will have a new bool data member called "dead" registered for it.
* The mortality process does not actually kill the trees, but puts a value of
* true in that "dead" data member.  Trees with this flag will be removed later
* by the clRemoveTrees object, if it exists.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clMortalityOrg {
  public:

  /**
  * Destructor.
  */
  ~clMortalityOrg();

  /**
  * Constructor.
  *
  * @param p_oHookedShell A clMortalityBase object (a mortality shell object)
  * which then becomes the hooked growth shell object.
  */
  clMortalityOrg(clMortalityBase *p_oHookedShell);

  /**
  * Performs mortality calculations.  This will control the application of the
  * mortality shells to the individual trees.  If a tree's species/type combo
  * bucket in mp_bUsesMortality is set to true, then the tree goes through the
  * gauntlet.  This should be called each timestep by the hooked shell's
  * Action() function.
  */
  void DoMortality();

  /**
  * Gets the number of total species.
  * @return Total number of species.
  */
  short int GetNumberOfSpecies() {return m_iTotalSpecies;};

  /**
  * Gets the number of total tree types.
  * @return Total number of tree types.
  */
  short int GetNumberOfTypes() {return m_iTotalTypes;};

  /**
  * Gets the number of mortality shell objects.
  * @return Number of mortality shell objects.
  */
  short int GetNumberOfMortalityShells() {return m_iNumMortShells;};

  /**
  * Registers the mortality data member.  Registers a "dead" boolean data
  * member for each species/type combo that uses mortality, and resets the
  * hooked object's species/type combos.  This is called by the hooked object's
  * RegisterTreeDataMembers() function.
  * @param p_oHooked Hooked mortality object.
  */
  void DoDataMemberRegistrations(clMortalityBase *p_oHooked);

  /**
  * Tells the mortality behaviors how many new tree bools to report to the tree
  * population.  Tree species/type combos are likely to have multiple mortality
  * behaviors applied to them.  If each behavior added a bool, then there would
  * be too many.  Here's what this function does:
  * <ol>
  * <li>Collects its list of mortality objects.
  * <li>Cause each mortality behavior to populate its mp_bUsesThisMortality
  * table.
  * <li>Analyze the behaviors and come up with which species/type combos have
  * any mortality applied.
  * <li>Assign those species/type combos to the hooked object and tell it
  * to register one bool data member.  The hooked object will already have its
  * mp_bUsesThisMortality table populated, so it shouldn't matter that we've
  * changed its assigned combos.
  * </ol>
  * @param p_oSimManager Pointer to the simulation manager.  Since this
  * object is not descended from clWorkerBase, it does not already have its
  * own pointer.
  * @param p_oHooked Hooked object
  */
  void UpdateDataMemberRegistrations(clSimManager *p_oSimManager, clMortalityBase *p_oHooked);

  /**
  * Gets a particular mortality shell.
  *
  * @param iIndex Index number of the mortality shell.
  * @return Mortality shell at that index.
  */
  clMortalityBase* GetMortalityShell(short int iIndex)
      {return mp_oMortShellList[iIndex];};

protected:

  /**Stashed pointer to the tree population object*/
  clTreePopulation *mp_oPop;

  /**Array of mortality shell objects, in order of execution. Each of these
   * shell objects will have a crack at each tree.*/
  clMortalityBase **mp_oMortShellList;

  /**Total number of species. Should equal the tree pop's value.*/
  short int m_iTotalSpecies;

  /**Total number of tree types. Should equal the tree pop's value.*/
  short int m_iTotalTypes;

  /**Number of mortality shell behaviors.*/
  short int m_iNumMortShells;

  /**Return codes for the "dead" tree int data member variable.  Array size
  is number of species by number of tree types (even if not every species and
  type is represented).*/
  short int **mp_iDeadCodes;

  /**2-D array - species by type - of whether or not a species/type combo uses
  mortality.  This array determines whether a tree is fed to the mortality
  gauntlet.*/
  bool **mp_bUsesMortality;

  /**
  * Populates the mortality shell array with the appropriate behavior
  * pointers.  It does this by going through the behaviors and looking for the
  * ones with "mortshell" in their names.
  *
  * @param p_oSimManager Sim Manager object.
  */
  void AssembleMortShellList(clSimManager *p_oSimManager);

  /**
  * Sets which species/type combos use which mortality.
  * This will go through each of the mortality shells in mp_oMortShellList and
  * use the species/type combos they apply to to set the appropriate bucket in
  * mp_bUsesMortality to true.
  */
  void PopulateUsesMortality();
};
//---------------------------------------------------------------------------
#endif
