//---------------------------------------------------------------------------

#ifndef GrowthOrgH
#define GrowthOrgH
//---------------------------------------------------------------------------

#include <xercesc/dom/DOM.hpp>

class clGrowthBase;
class clSimManager;
class clTreePopulation;
class clNCIGrowth;

/**
* Class for organizing growth behaviors.
*
* This class does the organizational work for growing trees for a timestep.  It
* finds all objects descended from clGrowthBase and coordinates their efforts
* for maximum efficiency.  Since this class is not descended from
* clBehaviorBase, it does not receive the normal triggers from clSimManager
* (GetData(), Action(), etc).  It therefore depends on a "hooked" growth shell
* object to tell it when to work by passing on those triggers that the hooked
* object receives.
*
* Growth shell objects can do one of three jobs:  update a tree's diameter
* and allow clTreePopulation to automatically update its height to match,
* update its diameter only with the expectation that another growth object
* will explicitly update the height, and update its height only with the
* expecation that another growth object will explicitly update the diameter.
* This class will make sure that a given tree species/type combination only
* has one growth method (either diameter with auto updating or explicit setting
* of both diameter and height), and that if explicit diameter/height setting
* is used, that there is one of each (so that one tree dimension does not go
* un-updated).
*
* This class causes trees to grow by accessing each tree in the population.
* It uses a tree's species and type information to determine which growth shell
* (if any) is responsible for updating it.  It then asks that growth shell (or
* pair of shells) to calculate by how much the tree is going to grow, and
* updates the tree dimensions accordingly.
*
* Sometimes growth behaviors need to perform calculations at a time when it can
* be guaranteed that no tree sizes have been changed.  The growth behaviors
* that need this override clGrowthBase::PreGrowthCalcs().  This will call
* this function for all growth behaviors BEFORE it allows any trees to be
* updated.
*
* When updating a tree's diameter, the diameter used depends on the tree type.
* If the tree is a seedling or sapling, diameter increments are applied to
* diam10.  Otherwise, they are applied to DBH.  There is no ambiguity about
* where to apply height increments.
*
* This class is conscious of growth's place in a sequence of biological
* submodels.  Most growth methods take into account the tree's light level,
* so this class will check which tree species/types to which any kind of
* growth is applied also have a light behavior applied.  It determines this by
* seeing if the tree species/type combinations have a float data member called
* "Light" registered.  These access codes are collected.  Child growth behaviors
* are responsible for checking these codes if light is required and throwing
* errors if the codes are -1.  Growth behaviors assume that the value in this
* data member is light level at the time growth is called, and they may expect
* different things from it.  See the child class documentation for more.
*
* Growth expects that mortality will follow it, and some mortality behaviors
* take into account growth rate.  So, this object register a tree float data
* member called "Growth" on behalf of all growth behaviors, which will hold
* amount of diameter growth in mm/yr.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clGrowthOrg {
  public:

  /**
  * Destructor.
  */
  ~clGrowthOrg();

  /**
  * Constructor.
  * @param p_oHookedShell A clGrowthBase object (a growth shell object) that
  * then becomes the hooked growth shell object.
  */
  clGrowthOrg(clGrowthBase *p_oHookedShell);

  /**
  * Controls the calculation of growth values and their assignment to
  * individual trees.  This should be called each timestep by the hooked shell's
  * Action() function.
  *
  * First, all growth shells have their PreCalcGrowth() functions called (if
  * they've put any code there).
  *
  * For each tree, if the pointer for its species/type combination in
  * mp_oDiameterGrowthTable is NULL, then nothing happens for that tree.
  * Otherwise, growth is called.  If a tree uses both height and diameter
  * growth, sometimes the one depends on the results of the other.  So this is
  * how this function decided in which order to call growth:
  * <ul>
  * <li>If diameter growth is diameter_auto, diameter growth goes first.</li>
  * <li>If both height and diameter growth are used, and the height growth
  * requests to go last, diameter goes first.</li>
  * <li>If neither height nor diameter growth requests to go last, diameter
  * goes first.</li>
  * <li>If both height and diameter growth request to go last, diameter goes
  * first.</li>
  * <li>If diameter requests to go last and height has no request, height goes
  * first.</li>
  * </ul>
  *
  * Once diameter growth has been obtained, the tree's diameter growth object's
  * clGrowthBase::GetGrowthMemberValue() is called, and that value is set into
  * the "Growth" data member variable.
  *
  * Once both growth increments are gotten (if applicable), then they are
  * applied.  If the growth shell object identifies itself as diameter_auto in
  * clGrowthBase::m_iGrowthMethod, then the amount of growth increase is added
  * to the tree's diameter and clTreePopulation is allowed to update its
  * allometry.  If the growth shell identifies itself as diameter_only in
  * clGrowthBase::m_iGrowthMethod, then the amount of growth increase is added
  * to the tree's diameter without an auto-height update.  If there is a
  * height increment, that amount is added to the tree's height.
  *
  * In all cases, height is limited to the values in mp_fMaxTreeHeight, which
  * are the maximum tree height for each species minus 0.001.
  */
  void DoGrowthAssignments();

  /**
  * Does the setup for this object.  Steps:
  * <ul>
  * <li>Calls PopulateGrowthTables() to create and populate the growth
  * shell object tables, and verify that the set of growth behaviors is valid.
  * <li>Calls GetLightVariableCodes().
  * <li>Calls GetDiamVariableCodes().
  * <li>Calls GetHeightVariableCodes().
  * <li>Calls GetMaxTreeHeights().
  * </ul>
  *
  * DoSetup() is called by the hooked shell's GetData() function.
  *
  * @param p_oSimManager Pointer to the simulation manager.  Since this object
  * is not descended from clWorkerBase, it does not already have its own
  * pointer.
  * @param p_oDoc Pointer to parsed parameter file.
  */
  void DoSetup(clSimManager *p_oSimManager, xercesc::DOMDocument *p_oDoc);

  /**
  * Gets the number of total species.
  * @return Total number of species.
  */
  short int GetNumberOfSpecies() {return m_iTotalSpecies;};

  /**
  * Gets the total number of tree types.
  * @return Total number of tree types.
  */
  short int GetNumberOfTypes() {return m_iTotalTypes;};

  /**
  * Gets the code for the "Light" data member.
  *
  * @param iSp The species for which to get the code.
  * @param iTp The tree type for which to get the code.
  * @return The "Light" data member code for the specified species and type.
  */
  short int GetLightCode(short int iSp, short int iTp)
   {return mp_iLightCodes[iSp][iTp];};

  /**
  * Gets the code for the "Growth" data member.
  *
  * @param iSp The species for which to get the code.
  * @param iTp The tree type for which to get the code.
  * @return The "Growth" data member code for the specified species and type.
  */
 short int GetGrowthCode(short int iSp, short int iTp)
   {return mp_iGrowthCodes[iSp][iTp];};


  /**
  * Gets the code for the diameter data member to which growth will be applied.
  *
  * @param iSp The species for which to get the code.
  * @param iTp The tree type for which to get the code.
  * @return The diameter data member code for the specified species and type.
  */
 short int GetDiamCode(short int iSp, short int iTp)
   {return mp_iDiamCodes[iSp][iTp];};

  /**
  * Gets a pointer to the diameter-updating growth base object from the table.
  * @param iSp The species for which to get the growth object.
  * @param iTp The tree type for which to get the growth object.
  * @return Pointer to the growth object that applies to the given species and
  * tree type.
  */
 clGrowthBase* GetGrowthShell(short int iSp, short int iTp)
   {return mp_oDiameterGrowthTable[iSp][iTp];};

  /**
  * Does the registration of the "Growth" tree data member variable.  It goes
  * through the growth functions table and, for every species/type combo that
  * has a valid pointer, registers the variable.  Return codes are captured in
  * the mp_iLightCodes array.
  *
  * @param p_oSimManager Pointer to Sim Manager object.
  * @param p_oPop Tree population object.
  */
  void DoTreeDataMemberRegistrations(clSimManager *p_oSimManager,
     clTreePopulation *p_oPop);

  protected:
  clTreePopulation *mp_oPop; /**<A pointer to the tree population object*/

  clGrowthBase ***mp_oDiameterGrowthTable; /**<Array of growth shell objects
  which either update diameter only or update diameter when height is
  automatically updated to match.  The array is sized species by type, and in
  each bucket, there's a pointer to the growth shell object which is handling
  that species/type combo's diameter growth calculation.  A pointer
  may be NULL if a combo is not getting any growth calculations.*/

  clGrowthBase ***mp_oHeightGrowthTable; /**<Array of growth shell objects
  which update height only.  The array is sized species by type, and in
  each bucket, there's a pointer to the growth shell object which is handling
  that species/type combo's height growth calculation.  A pointer
  may be NULL if a combo is not getting any growth calculations, or if its
  diameter and height are not explicitly set separately.*/

  clGrowthBase **mp_oShellList; /**<All shell objects.  These are in no
  particular order but the list is unique.  The array is sized m_iNumShells.*/

  /**Copy of the maximum heights for each species, minus 0.001, to
  * represent the asymptotic max height a tree is allowed to reach.*/
  float *mp_fMaxTreeHeight;

  short int m_iTotalSpecies; /**<Total number of species - should equal the tree
                             pop's value*/
  short int m_iTotalTypes;   /**<Total number of tree types - should equal the
                             tree pop's value*/
  short int m_iNumShells;    /**<Total number of growth shell objects.  This
                             is the size of the mp_oShellList array.*/

  /**
  * Return codes for the "Growth" tree float data member variable.  Array of
  * species by type (even if not every species and type is represented).
  */
  short int **mp_iGrowthCodes;

  /**
  Return codes for the "Light" tree float data member variable.  Array size is
  number of species by number of types (even if not every species and type
  requires light)*/
  short int **mp_iLightCodes;

  /**
  * Return codes for diameter where to apply growth to each type/species combo.
  * Array size is # species by # types
  */
  short int **mp_iDiamCodes;

  /**
  * Return codes for height where to apply growth to each type/species combo.
  * Array size is # species by # types
  */
  short int **mp_iHeightCodes;


  /**
  * Declares and populates the growth object tables.  First, it declares
  * mp_oDiameterGrowthTable and mp_oHeightGrowthTable.  Then it goes through all
  * the behaviors and finds the ones with "growthshell" in their names.  For
  * each species/type combo to which a behavior is applied, a pointer to that
  * behavior is set in the appropriate table at the bucket corresponding to that
  * species/type combo.  If a behavior identifies itself as setting either
  * diameter alone or diameter with automatic height updating, it goes in
  * mp_oDiameterGrowthTable.  If it identifies itself as setting height alone,
  * it goes in mp_oHeightGrowthTable.
  *
  * This behavior depends on m_iTotalSpecies & m_iTotalTypes having the correct
  * values (i.e. not 0).
  * @param p_oSimManager Sim Manager object.
  * @throw modelErr if:
  * <ul>
  * <li>A species/type combo is claimed by more than one behavior in
  * mp_oDiameterGrowthTable
  * <li>A species/type combo is claimed by more than one behavior in
  * mp_oHeightGrowthTable
  * <li>A species/type combo has an explicit height or diameter setting
  * behavior without a complimentary partner (there's a behavior that sets
  * diameter but there's no height setting behavior, or vice versa
  * </ul>
  */
  void PopulateGrowthTables(clSimManager *p_oSimManager);

  /**
  * Queries for the return codes of the "Light" float data member of a tree.
  * "Light" should have been registered by clLightBase objects.  Return codes
  * are captured in the mp_iLightCodes array.  Values of -1, in case light is
  * not used, are okay.
  */
  void GetLightVariableCodes();

  /**
  * Queries for the return codes of the diameters at which to apply growth
  * for each type/species combo.  It's diam10 if seedling or sapling, dbh for
  * adults.  Return codes are captured in the mp_iDiamCodes array.
  *
  * @throw Error if there is no code for any tree which uses growth.
  */
  void GetDiamVariableCodes();

  /**
  * Queries for the return code for height for each type/species combo.  Return
  * codes are captured in the mp_iHeightCodes array.
  *
  * @throw Error if there is no code for any tree which uses growth.
  */
  void GetHeightVariableCodes();

  /**
  * Gets a copy of the maximum tree height for each species.  0.001 is
  * subtracted from the maximum tree height to represent an asymptotic limit
  * to tree height.
  */
  void GetMaxTreeHeights();
};
//---------------------------------------------------------------------------
#endif
