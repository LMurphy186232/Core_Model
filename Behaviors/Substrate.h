//---------------------------------------------------------------------------

#ifndef SubstrateH
#define SubstrateH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"

class clGrid;


/**
* Substrate - Version 2.1
*
* Substrate keeps track of what the forest floor in a particular location is
* made of at any given time.  There are six kinds of substrate: fresh logs,
* decayed logs, scarified soil, tip-up, forest floor litter, and forest floor
* moss.  This behavior tracks the input of new fresh logs and tip-up through
* the fall of dead trees, the changing of substrate conditions due to harvest,
* and the decay of all other substrate types into forest floor litter and moss.
* Forest floor litter and forest floor moss act as a common pool, with one in
* fixed proportion to the other.
*
* Freshly dead live trees have a certain probability of falling to create new
* fresh log substrate, and the fallen have a certain probability of creating
* new tip-up.  Dead snags always contribute new fresh log, and have their own
* probability of creating tip-up.  The amount of fresh logs and tip-up
* contributed by a downed tree depends on the size of the tree.
*
* Trees can collapse vertically to put all of their fresh log area in the grid
* cell in which they are rooted, or they can fall in a random direction and
* contribute fresh log area to more than one cell.  This option is controlled
* by the user.  All tip-up area is always contributed to the cell in which the
* tree is rooted.
*
* The change in substrate contributed by harvest events is set directly in the
* parameter file.  Harvest is the only way to introduce new scarified soil
* substrate.
*
* The substrate behavior will create a grid called "Substrate."  In this grid
* are contained, for each grid cell, the relative proportions of the various
* types of substrate as a value between 0 and 1.
*
* An input of new substrate through mortality or harvest is tracked in a grid
* package.  For a package, the relative proportions of substrate are subjected
* through time to increasing levels of substrate decay, until the package
* reaches the maximum age limit and is removed.  The master grid cell substrate
* proportions are calculated from the input of all the packages associated with
* that cell.
*
* Substrate parameters relating to harvest are not required unless the Harvest
* behavior is being used for a run.
*
* Substrate should be applied to all the species/type combos for trees that can
* contribute to new fresh log generation.  Substrate depends on there being a
* bool data member of the trees called "dead" in these trees (the absence of
* this code being previously defined will cause a fatal error).  The data
* member should have a value of true if a tree is dead.  Dead trees should not
* yet have been removed from the tree population.  Substrate cannot be applied
* to seedlings; trying to do so causes a fatal error.
*
* If Substrate determines that a dead tree falls over, it calls the tree
* population's KillTree() function, with a reason code of "remove", to force
* its removal from memory and to prevent its becoming a snag.  Note that if
* any other behaviors become interested in dead trees, this could be a problem.
*
* The namestring and parameter file call string of this behavior are both
* "Substrate".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clSubstrate : virtual public clBehaviorBase {

 public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clSubstrate(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clSubstrate();

  /**
  * Performs setup for substrate.  First, GetParameterFileData() reads values
  * from the parameter file.  Then, SetupSubstrateGrids() is called.  Then
  * PopulateInitialConditions().  Then GetDeadCodes().
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Computes substrate for a timestep.  First HarvestSubstrate() adds the
  * effects of any harvests that occurred this timestep.  Then
  * MortalitySubstrate() adds new dead trees.  Then DecaySubstrate() decays
  * cohorts and comes up with the aggregate substrate amounts.
  */
  void Action();

  protected:

  /**Proportion of dead trees that fall. Array size is total number of species.*/
  double *mp_fPropOfDeadThatFall;

  /**Proportion of fallen dead that uproot.  Array size is total number of
  * species.*/
  double *mp_fPropOfFallenThatUproot;

  /**Proportion of dead snags that uproot.  Array size is total number of
  * species.*/
  double *mp_fPropOfSnagsThatUproot;

  /**Decay proportion for each timestep for fresh logs. This is
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly.  Array size
  * is m_iMaxDecayTimesteps + 1.*/
  double *mp_fFLogDecayProp;

  /**Decay proportion for each timestep for scarified soil. This is
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly.  Array size
  * is m_iMaxDecayTimesteps + 1.*/
  double *mp_fScarSoilDecayProp;

  /**Decay proportion for each timestep for tip-up mounds. This is
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly.  Array size
  * is m_iMaxDecayTimesteps + 1.*/
  double *mp_fTipupDecayProp;

  /**Decay proportion for each timestep for decayed logs. This is
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly.  Array size
  * is m_iMaxDecayTimesteps + 1.*/
  double *mp_fDecLogDecayProp;

  /**Array (species by type) of codes for the "dead" tree bool data member*/
  short int **mp_iDeadCodes;

  /**Code for using freshlog_i data member of calculations grid*/
  short int *mp_iFLogCalcsCode;

  /**Code for using declog_i data member of calculations grid*/
  short int *mp_iDecLogCalcsCode;

  /**Array (species by type) of whether or not substrate applies to this tree
  * kind*/
  bool **mp_bSubstrateApplies;

  /**Scarified soil alpha decay parameter.*/
  double m_fScarifiedSoilA;

  /**Tip-up alpha decay parameter.*/
  double m_fTipUpA;

  /**Fresh log alpha decay parameter.*/
  double m_fFreshLogA;

  /**Decayed logs alpha decay parameter.*/
  double m_fDecayedLogA;

  /**Scarified soil beta decay parameter.*/
  double m_fScarifiedSoilB;

  /**Tip-up beta decay parameter.*/
  double m_fTipUpB;

  /**Fresh log beta decay parameter.*/
  double m_fFreshLogB;

  /**Decayed logs beta decay parameter.*/
  double m_fDecayedLogB;

  /**Scarified soil initial conditions proportion*/
  double m_fInitScarifiedSoil;

  /**Tip-up initial conditions proportion*/
  double m_fInitTipUp;

  /**Fresh log initial conditions proportion*/
  double m_fInitFreshLog;

  /**Decayed log initial conditions proportion*/
  double m_fInitDecayedLog;

  /**Scarified soil partial cut proportion.  Not required if Harvest is not
  * being used.*/
  double m_fPartCutScarifiedSoil;

  /**Tip-up partial cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fPartCutTipUp;

  /**Fresh log partial cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fPartCutFreshLog;

  /**Decayed log partial cut proportion.  Not required if Harvest is not
  * being used.*/
  double m_fPartCutDecayedLog;

  /**Scarified soil gap cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fGapCutScarifiedSoil;

  /**Tip-up gap cut proportion.  Not required if Harvest is not being used.*/
  double m_fGapCutTipUp;

  /**Fresh log gap cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fGapCutFreshLog;

  /**Decayed log gap cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fGapCutDecayedLog;

  /**Scarified soil clear cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fClearCutScarifiedSoil;

  /**Tip-up clear cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fClearCutTipUp;

  /**Fresh log clear cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fClearCutFreshLog;

  /**Decayed log clear cut proportion.  Not required if Harvest is not being
  * used.*/
  double m_fClearCutDecayedLog;

  /**Amount by which the tree radius is multiplied when calculating the tip-up
  * mound to account for the effects of root rip-out*/
  double m_fRootTipupFactor;

  /**Proportion of the litter/moss pool that is moss*/
  double m_fMossProportion;

  /**Grid cell area of substrate grid*/
  float m_fGridCellArea;

  /**Reciprocal of grid cell area - for proportion-of-area calculations*/
  float m_fRecipOfGridCellArea;

  /**Maximum number of years a package is allowed to exist*/
  int m_iMaxDecayYears;

  /**m_iMaxDecayYears divided by the number of years per timestep, for setting
  * up grid data members.*/
  int m_iMaxDecayTimesteps;

  short int m_iNumTotalSpecies;   /**<Total number of species - for destructor*/
  short int m_iScarSoilCode;      /**<Code for using scarsoil grid data member*/
  short int m_iFFMossCode;        /**<Code for using ffmoss grid data member*/
  short int m_iFFLitterCode;      /**<Code for using fflitter grid data member*/
  short int m_iTipupCode;         /**<Code for using tipup grid data member*/
  short int m_iFreshLogCode;      /**<Code for using freshlog grid data member*/
  short int m_iDecLogCode;        /**<Code for using declog grid data member*/
  short int m_iPkgAgeCode;        /**<Code for using age package data member*/
  short int m_iPkgScarSoilCode;   /**<Code for using scarsoil package data member*/
  short int m_iPkgTipupCode;      /**<Code for using tipup package data member*/
  short int m_iPkgFreshLogCode;   /**<Code for using freshlog package data member*/
  short int m_iHarvestTypeCode;   /**<Code for using harvesttype data member of harvest grid*/
  short int m_iTipupCalcsCode;    /**<Code for using newtipup data member of calculations grid*/

  /**Whether or not the parameter file had a substrate map in it.  If it did,
  * initial conditions will not be populated.*/
  bool m_bParFileHasSubstrateMap;

  /**Whether to use directional tree fall.  If true, trees fall in a random
  * direction and contribute fresh log area to potentially multiple grid cells.
  * If false, trees that fall contribute all of their fresh log area to the
  * cell in which they are rooted.
  */
  bool m_bUseDirectionalTreeFall;

  /**
  * Grid object containing substrate data, called "Substrate".
  *
  * If there is a harvest grid, the grid cell resolution must match it.  If
  * this is not the case, and there is no user-defined grid cell resolution,
  * then the default grid resolution is used.
  *
  * Maps of this grid in the parameter file will be honored, but they must
  * define all data members below (even if they don't include values for all of
  * them).
  *
  * The grid cell record will consist of 5 float data members. They are:
  *
  * <table>
  * <tr>
  * <th>Data member name</th>  <th>Data type</th>  <th>Description</th></tr>
  * <tr>
  * <td>scarsoil</td>           <td>float</td>      <td>Scarified soil, expressed as a proportion between 0 and 1.</td>
  * </tr>
  * <tr>
  * <td>ffmoss</td>            <td>float</td>      <td>Forest floor moss, expressed as a proportion between 0 and 1.</td>
  * </tr>
  * <tr>
  * <td>fflitter</td>          <td>float</td>      <td>Forest floor litter, expressed as a proportion between 0 and 1.</td>
  * </tr>
  * <tr>
  * <td>tipup</td>             <td>float</td>      <td>Tip-up, expressed as a proportion between 0 and 1.</td>
  * </tr>
  * <tr>
  * <td>freshlog</td>          <td>float</td>      <td>Fresh logs, expressed as a proportion between 0 and 1.</td>
  * </tr>
  * <tr>
  * <td>declog</td>            <td>float</td>      <td>Decayed logs, expressed as a proportion between 0 and 1.</td>
  * </tr>
  * </table>
  *
  * The packages will have the following structure:
  * <table>
  * <tr>
  * <th>Data member name</th>  <th>Data type</th>  <th>Description</th></tr>
  * <tr>
  * <td>age</td>               <td>int</td>        <td>Age of package in timesteps</td>
  * </tr>
  * <tr>
  * <td>scarsoil</td>          <td>float</td>      <td>Scarified soil added this package.  Should not be changed.</td>
  * </tr>
  * <tr>
  * <td>tipup</td>             <td>float</td>      <td>Tip-up added this package.  Should not be changed.</td>
  * </tr>
  * <tr>
  * <td>freshlog</td>          <td>float</td>      <td>Fresh logs added this package.  Should not be changed.</td>
  * </tr>
  * </table>
  *
  * The packages will be in the order of oldest first.
  */
  clGrid *mp_oSubstrateGrid;

  /**
  * Grid object for intermediate calculations.  This grid is called
  * "substratecalcs".  The grid cell resolution matches "Substrate".  Data
  * members:
  * <table>
  * <tr>
  * <th>Data member name</th>  <th>Data type</th>  <th>Description</th>
  * </tr>
  * <tr>
  * <td>newtipup</td>          <td>float</td>      <td>New tip-up area by grid cell, in square meters</td>
  * </tr>
  * <tr>
  * <td>freshlog_i</td>        <td>float</td>      <td>Fresh logs i timesteps ago up to m_iMaxDecayTimesteps, as a proportion of grid cell area</td>
  * </tr>
  * <tr>
  * <td>declog_i</td>          <td>float</td>      <td>Decayed logs added i timesteps ago up to m_iMaxDecayTimesteps, as a proportion of grid cell area</td>
  * </tr>
  * </table>
  */
  clGrid *mp_oCalcGrid;

  /**Harvest grid object, if it exists*/
  clGrid *mp_oHarvestGrid;

  /**
  * Gets the values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Sets up the substrate grids and registers all their data members.  If the
  * harvest grid exists, substrate is required to match its grid cell
  * resolution.
  */
  void SetupSubstrateGrids();

  /**
  * Converts new fresh logs and tip-up from square meters to a proportion of
  * the grid cell.  If the area of the new substrates is greater than the area
  * of the grid cell, each substrate is divided by the combined area of the two
  * to get a proportion.
  */
  void AdjustNewSubstrateToProportion();

  /**
  * Gives each substrate grid cell the proportions of substrate defined as the
  * initial conditions in the parameter file.  Then each cell is given a
  * package with the same initial conditions proportions of substrate.  The
  * fresh logs data member in the calcs grid for this timestep is set to the
  * amount of initial conditions fresh logs.
  *
  * If a grid map was read in with the parameter file, as indicated by
  * m_bParFileHasSubstrateMap = true, this function does not do anything.
  */
  void PopulateInitialConditions();

  /**
  * Populates the mp_iDeadCodes array with the data member codes for the "dead"
  * data member.
  * @throw Error if there is a species/type combo which participates in
  * substrate but does not have a bool data member called "dead" registered.
  */
  void GetDeadCodes();

  /**
  * Calculates the proportion of decay for each timestep for fresh logs, tip-
  * up mounds, and scarified soil.  This calculates
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly for each
  * timestep up to m_iMaxDecayTimesteps.
  */
  void CalculateDecayProportions();

  /**
  * Adds the effects of harvests to the substrate grid.  If Harvest isn't being
  * used, or no harvest occurred this timestep, nothing happens.  If there was
  * a harvest, each grid cell in which harvest occurred is evaluated
  * separately.  All existing packages for a cell are deleted, and a new
  * harvest package is created with the conditions from the parameter file
  * matching the harvest type in the "harvestresults" grid.  The age is set to
  * 0 so that no decay occurs and the values in the package will match the
  * final grid cell values.  The amount of fresh log in square meters that was
  * created by harvest is added to the calculations grid's newfreshlogs data
  * member.  All other fresh logs are erased.
  */
  void HarvestSubstrate();

  /**
  * Updates the ages of the fresh logs and decayed logs created over previous
  * timesteps.  The values are held in the data members freshlogs_i and
  * declogs_i, where i is the new substrate proportion i timesteps ago.  This
  * shifts all values back one.  For example, the value in freshlogs_2 is put
  * in freshlogs_3, whose value is put in freshlogs_4, etc.  The last value is
  * discarded.  freshlogs_0 and declogs_0 is set to 0, ready for the additions
  * of the current timestep.
  *
  * Also, this sets the value of the organics and fltemp calcs grid data
  * members to 0.
  *
  * This does not execute the first timestep because initial conditions has
  * populated a value into the first bucket of the array that should stay there.
  */
  void UpdateSubstrateAges();

  /**
  * Adds the effects of the new dead trees to the substrate grid by calling
  * AddNewDeadTrees() and AdjustSubstrateForMortality().
  */
  void MortalitySubstrate();

  /**
  * Finds new dead trees to add to substrate.  Dead trees are identifiable by a
  * value of true in their "dead" data member.  For saplings and adults, a
  * random number is used to determine if a tree falls over (and thus becomes
  * part of the tally of new fresh log substrate).  Snags that are "dead" are
  * already considered fallen.  For fallen trees, another random number
  * determines if it uproots (and thus adds to the new tip-up area exposed).
  * Saplings can't create tip-up.  Adults and snags have separate probabilities
  * of uproot.
  *
  * Trees that fall are passed to clTreePopulation::KillTree() with remove as
  * the code to force them to go away.  This ensures they do not become snags.
  *
  * Both the amount of new fresh log and the amount of tip-up added are a
  * function of tree size.  New fresh log area is calculated as FA = r*h,
  * where FA is new fresh log area in square meters, r is tree radius in
  * meters, and h is tree height in meters.  New tip-up area is calculated as
  * OA = pi * (r*F)<sup>2</sup>, where OA is the new tip-up area in square
  * meters, r is the tree radius in meters, and F is the value in
  * m_fRootTipupFactor, which accounts for the effects of root disturbance.
  * If m_bDirectionalTreeFall is true, trees that fall do so in a random
  * direction.  The area of the log is divided evenly into h*2 chunks, where
  * h is the tree height in m.  The function walks out the length of the log
  * in 0.5-m intervals, and whatever grid cell the end of the interval falls
  * in gets a fresh log area chunk.  If m_bDirectionalTreeFall is false, all
  * fresh log area is put in the cell in which the tree was rooted.
  *
  * For each tree, its amounts of new fresh logs and new tip-up, in square
  * meters, are added to the freshlog_0 and newtipup data members of the
  * calculations grid to the grid cell in which the tree was rooted.
  */
  void AddNewDeadTrees();

  /**
  * For each grid cell in the substrate array, this adjusts the package values
  * for new substrate added in AddNewDeadTrees().  First, the amounts of new
  * substrate for fresh logs and tip-up in square meters of area is converted
  * to a proportion of total substrate.  Then the proportions of all substrate
  * cohort packages are reduced accordingly to make way for the new stuff.  If
  * a substrate event has already happened for this timestep (it is the first
  * timestep or there was a harvest event), the new substrate is added to that
  * event's package.  Otherwise, a new package with the new substrate
  * proportions is added.
  */
  void AdjustSubstrateForMortality();

  /**
  * Adds the amount of new fresh logs and new tip-up created by a harvest or
  * in the initial conditions to the calculations grid.  These values are
  * just those set in the parameter file for initial conditions or for the type
  * of harvest that occurred.  This is a separate step so that the calculations
  * for mortality don't get screwed up when the amount of substrate in cohort
  * packages is being reduced.  If there has been no harvest, or this is later
  * than timestep 1, nothing will happen.
  */
  void AddHarvestAndInitialNewSubstrate();

  /**
  * Performs substrate decay.
  * The first part of this function is about package management: removing
  * packages that are too old, and adding the amounts of fresh logs, tip-up,
  * and scarified soil together from the others, decaying them appropriately
  * according to the package's age.  Then these amounts are assigned to the
  * master substrate list for each cell and decayed logs and forest floor
  * calculated.
  */
  void DecaySubstrate();
};
//---------------------------------------------------------------------------
#endif
