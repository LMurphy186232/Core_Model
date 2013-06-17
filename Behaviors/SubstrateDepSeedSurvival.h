//---------------------------------------------------------------------------
#ifndef SubstrateDepSeedSurvivalH
#define SubstrateDepSeedSurvivalH
//---------------------------------------------------------------------------

#include "BehaviorBase.h"

class clTreePopulation;
class clGrid;
class clTree;
class clModelMath;
/**
* Substrate Dependent Seed Survival - Version 1.1
*
* This behavior assesses seed survival as a function of substrate composition.
* This behavior can be used in three different configurations:  with gap
* status, without gap status, and with microtopography.
*
* This uses the information in the "Substrate" grid (created and populated by
* clSubstrate) to determine how favorable a location is for seed survival.
* The "Substrate" grid contains, in each cell, the proportion of each of the
* six substrate types.  For each species, this behavior knows, on a scale from
* 0 to 1, how favorable a substrate type is to seeds of that species.  The
* behavior calculates a "favorability index" for each grid cell for each
* species by multiplying the proportion of each substrate by its favorability
* and summing the results.  This index is the proportion of seeds of that
* species that survive at that location.
*
* If this is used with gap status, then there are two possible favorabilities
* for each species for each substrate type:  the favorability in a gap cell
* and the favorability in a canopy cell.  This behavior uses the appropriate
* favorabilities for the gap status of the cell.
*
* If this is used with microtopography, then there are twelve substrate types:
* each of the basic six on the ground and on a mound.  Each substrate type is
* split into mound and ground based on the fixed proportion of each specified
* by the user.  A favorability index is created from these twelve types the
* same way it is created with six, above.
*
* To keep track of seed favorabilities, this behavior creates a grid called
* "Substrate Favorability".
*
* In order to translate between different grid cell sizes in the "Substrate"
* and "Dispersed Seeds" grids, each seed is assigned a temporary random
* location within its "Dispersed Seeds" grid cell.  A random number is compared
* to the favorability at that location and the seed either survives or dies.
* The number of surviving seeds in a "Dispersed Seeds" grid cell is assigned
* back to that cell.
*
* A fatal error is thrown if: there are no disperse behaviors for the run, if
* gap status establishment is used without gap status disperse, or if substrate
* establishment is used without substrate.
*
* The namestring for this class is "Substrate Dependent Seed Survival".  Call
* this from the parameter file using either "NoGapSubstrateSeedSurvival",
* "GapSubstrateSeedSurvival", or "MicrotopographicSubstrateSeedSurvival".
* Apply this behavior to species; use any type, since it will be ignored.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clSubstrateDepSeedSurvival : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clSubstrateDepSeedSurvival(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clSubstrateDepSeedSurvival();

  /**
  * Performs setup.  This calls GetParameterFileData() and SetupGrids().
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Assesses seed survival.  This calculates the favorability index for each
  * grid cell.  For each grid cell, the favorability index is the sum of the
  * proportions of each substrate multiplied by the favorability of that
  * substrate for the cover condition of that grid cell (canopy or gap).
  *
  * Then, for each cell in the seeds grid, each seed's probability of survival
  * is assessed.  It will be given a random location within the grid cell of
  * the "Dispersed Seeds" grid from which it comes.  Then a random number is
  * compared to the substrate favorability at that location to see if the seed
  * survives or not.  This allows us to translate easily between different grid
  * cell resolutions in the "Dispersed Seeds" and "Substrate" grids.
  */
  void Action();

  /**
  * Captures the behavior name passed from the parameter file.  If the name
  * called from the parameter file is "NoGapSubstrateSeedSurvival", then
  * m_bUsesGap is set to false.  If the name called is "Gap Substrate Seed
  * Survival", then m_bUsesGap is set to true.  If the name called is
  * "MicrotopographicSubstrateSeedSurvival", then m_bUsesGap is set to false
  * and m_bUsesMicro is set to true.
  *
  * @param sNameString Behavior name from parameter file.
  */
  void SetNameData(std::string sNameString);

  protected:

  /**Pointer to "Substrate" grid, created by clSubstrate*/
  clGrid *mp_oSubstrateGrid;

  /**
  * The grid called "Substrate Favorability" will have a cell resolution
  * matching the "Substrate" grid.  It will contain the substrate favorability
  * index. The grid cell record will consist of (# species) float data members,
  * "Favorability IndexX".  It will have a member for each species even if the
  * creating behavior does not apply to all, because another behavior in this
  * run may apply to different species.
  *
  * It is possible that a grid map will have been read in for this grid; it
  * will be ignored, and any grid created with such a map will be overwritten.
  */
  clGrid *mp_oSubstrateFavGrid;

  /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
  clGrid *mp_oSeedGrid;

  /**Canopy / ground scarified soil favorability - one per behavior species*/
  float *mp_fCanGroundScarSoilFav;
  /**Canopy / ground tip-up mounds favorability - one per behavior species*/
  float *mp_fCanGroundTipUpFav;
  /**Canopy / ground fresh logs favorability - one per behavior species*/
  float *mp_fCanGroundFreshLogFav;
  /**Canopy / ground decayed logs favorability - one per behavior species*/
  float *mp_fCanGroundDecLogFav;
  /**Canopy / ground forest floor litter favorability - one per behavior species*/
  float *mp_fCanGroundForFlLitterFav;
  /**Canopy / ground forest floor moss favorability - one per behavior species*/
  float *mp_fCanGroundForFlMossFav;

  /**Gap / mound scarified soil favorability - one per behavior species*/
  float *mp_fGapMoundScarSoilFav;
  /**Gap / mound tip-up mounds favorability - one per behavior species*/
  float *mp_fGapMoundTipUpFav;
  /**Gap / mound fresh logs favorability - one per behavior species*/
  float *mp_fGapMoundFreshLogFav;
  /**Gap / mound decayed logs favorability - one per behavior species*/
  float *mp_fGapMoundDecLogFav;
  /**Gap / mound forest floor litter favorability - one per behavior species*/
  float *mp_fGapMoundForFlLitterFav;
  /**Gap / mound forest floor moss favorability - one per behavior species*/
  float *mp_fGapMoundForFlMossFav;

  /**Speeds access to favorability arrays*/
  short int *mp_iIndexes;

  /**Data member codes for substrate grid for favorability index.  Array size
  * is # total species.*/
  short int *mp_iSubstrFavCode;

  /**Data member codes for seed grid for number of seeds.  Array size is #
  * total species.*/
  short int *mp_iSeedGridCode;

  /**Proportion of plot that is mound, if using microtopography - 0 - 1*/
  float m_fMoundProp;

  /**Data member code for gap status in dispersed seed grid*/
  short int m_iIsGapCode;

  //Substrate proportions grid data member codes
  short int m_iScarifiedSoilCode;  /**<Data member code - float - scarsoil grid data member*/
  short int m_iForestFloorLitterCode;  /**<Data member code - float - fflitter grid data member*/
  short int m_iForestFloorMossCode;  /**<Data member code - float - ffmoss grid data member*/
  short int m_iTipUpCode;  /**<Data member code - float - tipup grid data member*/
  short int m_iFreshLogCode; /**<Data member code - float - freshlog grid data member*/
  short int m_iDecayedLogCode; /**<Data member code - float - declog grid data member*/

  /**Whether or not gap status is used by this behavior*/
  bool m_bUsesGap;

  /**Whether or not microtopography is used*/
  bool m_bUsesMicro;

  /**
  * Declares arrays and fills them with parameter file data.  If m_bUsesGap is
  * true, all gap substrate favorabilities are also retrieved.
  *
  * @param p_oDoc Parsed parameter file.
  * @throw modelErr if any of the substrate favorabilities are not between 0
  * and 1, or if the mound proportion is not between 0 and 1.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * For each grid cell in the substrate favorability grid, determines its
  * substrate favorability index.  If m_bUsesGap is true, then the gap status
  * is retrieved from the seed grid and the favorability matches the gap
  * status; otherwise, the favorability is calculated with the canopy
  * parameters.
  *
  * For each grid cell, the favorability index is the sum of the proportions of
  * each substrate multiplied by the favorability of that substrate for the
  * cover condition of that grid cell (canopy or gap).
  */
  void CalculateSubstrateFavorabilityIndex();

  /**
  * Sets up the grids.  This creates the substrate favorability grid and
  * retrieves a pointer to the dispersed seeds grid.
  *
  * @throws modelErr If the "Substrate" grid doesn't exist, or if the
  * "Dispersed Seeds" grid doesn't exist.
  */
  void SetupGrids();
};
//---------------------------------------------------------------------------
#endif
