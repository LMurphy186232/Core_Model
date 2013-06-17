//---------------------------------------------------------------------------

#ifndef DetailedSubstrateH
#define DetailedSubstrateH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"

class clGrid;


/**
* DetailedSubstrate - Version 1.0
*
* Substrate keeps track of what the forest floor in a particular location is
* made of at any given time.  There are five kinds of substrate: logs,
* scarified soil, tip-up, forest floor litter, and forest floor
* moss.  Logs are in turn divided into up to 3 species groups, 2 size 
* classes, and 5 decay classes, which are all tracked separately. This behavior tracks 
* the input of new logs and tip-up through the fall of dead trees, the changing 
* of substrate conditions due to harvest, and the decay of all other substrate 
* types into forest floor litter and moss. Forest floor litter and forest floor 
* moss act as a common pool, with one in fixed proportion to the other.
*
* In addition to tracking the area covered by all five substrate types, logs are 
* tracked by volume as well.  Unlike area, log volume has no upper limit and is not 
* overlaid by new logs and tipups.  Log volume is, however, reduced by scarified soil 
* after harvesting to simulate its destruction by harvest and site preparation 
* operations. 
*
* New dead saplings, fallen or broken dead adult trees, and snags can create log substrate, 
* and fallen adults and snags have a certain probability of creating new tip-up.  Dead
* saplings do not create snags and are presumed to fall in the same timestep that they 
* die.  Dead saplings do not create tip-up mounds. Dead snags have their own decay class 
* probability distribution and probability of creating tip-up.  The amount of logs and 
* tip-up contributed by a downed tree depends on the size of the tree.
*  
* Trees can collapse vertically to put all of their log area in the grid
* cell in which they are rooted, or they can fall in a random direction and
* contribute fresh log area to more than one cell.  This option is controlled
* by the user.  All tip-up area is always contributed to the cell in which the
* tree is rooted.
*
* The change in substrate contributed by harvest events is set directly in the
* parameter file.  Harvest is the only way to introduce new scarified soil
* substrate.
*
* The substrate behavior will create a grid called "DetailedSubstrate."  In this grid
* are contained, for each grid cell, the relative proportions of the various
* types of substrate as a value between 0 and 1. It also contains the volume of 
* each type of log, in m3 per ha. Values in the DetailedSubstrate grid are also 
* summarized written to a grid called "Substrate" for compatibility with behaviors 
* that use that grid.
*
* An input of new substrate through mortality or harvest is tracked in a grid
* package (scarified soil and tipup) and a separate calculations grid (logs).  
* The relative proportions of substrate are subjected through time to increasing 
* levels of substrate decay, until the substrate reaches the maximum age limit 
* and is removed.  The master grid cell substrate proportions are calculated 
* from the input of all the packages and calculations grid data members associated 
* with that cell. Forest floor litter and moss is calculated as the remaining area
* not covered by scarified soil, tipup, and logs.
*
* Substrate parameters relating to harvest are not required unless the Harvest
* behavior is being used for a run.
*
* Substrate should be applied to saplings, adults and/or snags of all the species 
* that can contribute to new log generation.  Substrate depends on there being
* bool data members called "dead" (all) and "Fall" (adults), and float data members
* called "NewBreakHeight" (adults, snags), and "SnagOldBreakHeight" (snags) in these 
* trees (the absence of these code being previously defined will cause a fatal error).  
* Dead trees should not have been changed to snags and fallen snags should not yet 
* have been removed from the tree population. Substrate cannot be applied to 
* seedlings; trying to do so causes a fatal error.
*
* If an adult tree falls over, DetailedSubstrate calls the tree
* population's KillTree() function, with a reason code of "remove", to force
* its removal from memory.  
*
* The namestring and parameter file call string of this behavior are both
* "DetailedSubstrate".
*
* Copyright 2003 Charles D. Canham.
* @author Mark Vanderwel
*
* <br>Edit history:
* <br>-----------------
* <br>October 11, 2007 - Created from existing substrate behavior.
*/
class clDetailedSubstrate : virtual public clBehaviorBase {

 public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clDetailedSubstrate(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clDetailedSubstrate();

  /**
  * Performs setup for substrate.  First, GetParameterFileData() reads values
  * from the parameter file.  Then CalculateDecayProportions() is called. Then 
  * SetupSubstrateGrids().  Then PopulateInitialConditions().  Then GetCodes().
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Computes substrate for a timestep.  First HarvestSubstrate() adds the
  * effects of any harvests that occurred this timestep.  Then CalcMainGridTotals()
  * writes the results to the substrate grid.  Then DecaySubstrate() decays
  * substrate through the calculations grid.  Then CalcVolumeAfterDecay() calculates
  * new decay-class-specific log volumes.  Then MortalitySubstrate() adds new dead trees.  
  * Then CalcMainGridTotals() is called again to write everything back to the substrate grid. 
  */
  void Action();

  protected:
  
  
  /**Array (species by type) of codes for the "dead" tree bool data member*/
  int **mp_iDeadCodes;
    
  /**Array of codes for the "Fall" data member. Index is # of species.*/
	int * mp_iFallCodes;
  /**Array of codes for the "NewBreakHeight" data member. First array index 
   * is # of species, and second is type #.*/
	int ** mp_iNewBreakCodes;
  /**Array of codes for the "SnagOldBreakHeight" data member. Index
   * is # of species.*/
	int * mp_iOldBreakCodes;  

	/**Mean diameter of small and large logs at initial conditions.  Used to 
   * calculate volume.  Array index is size class. If not provided, mean 
   * diameters will be 0.5 * m_fLogSizeClassBoundary and 1.5 * 
   * m_fLogSizeClassBoundary for small and large logs, respectively.*/
	float * mp_fInitialLogMeanDiameter;
	
	/**Mean diameter of small and large logs added by a harvest partial cut. Used 
   * to calculate volume.  Array index is size class. */
	float * mp_fPartCutLogMeanDiameter;
  /**Mean diameter of small and large logs added by a harvest gap cut. Used 
   * to calculate volume.  Array index is size class. */
	float * mp_fGapCutLogMeanDiameter;
  /**Mean diameter of small and large logs added by a harvest clear cut. Used 
   * to calculate volume.  Array index is size class. */
	float * mp_fClearCutLogMeanDiameter;

	/**Proportion of live trees that enter decay classes 1-5.
	 * Array index is decay class #.*/
	float * mp_fPropFallenTreesLogDecayClass;
  /**Proportion of snags that enter decay classes 1-5. Array index is decay 
   * class #.*/
	float * mp_fPropFallenSnagsLogDecayClass;
	
	/**Species grouping to which each tree species falls into.
	 * Array index is total number of species.*/  
	int * mp_iLogSpGroupAssignment;

	/**Alpha decay parameter for logs.  Array indexes are species group, 
	 * size, and decay class.*/
	float *** mp_fLogDecayAlpha;
  /**Beta decay parameter for logs.  Array indexes are species group, 
   * size, and decay class.*/
	float *** mp_fLogDecayBeta;
	
	/**The number of species groups used (1-3).*/
	int m_iNumLogSpGroupsUsed;
	
	/**Boundary between two log diameter classes, in cm.*/
	float m_fLogSizeClassBoundary;
	  
  /**Proportion of fallen dead trees that uproot. Array size is total number 
   * of species.*/
  float *mp_fPropOfFallenThatUproot;
  /**Proportion of snags that uproot. Array size is total number of species.*/
  float *mp_fPropOfSnagsThatUproot;

  /**Decay proportion each year for logs. This is
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly.  
  * Array indexes are species group, size, decay class, 
  * m_iMaxDecayTimesteps, and years per timestep.*/
  float *****mp_fLogDecayProp;

  /**Decay proportion for each timestep for scarified soil. This is
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly.  Array size
  * is m_iMaxDecayTimesteps.*/
  float *mp_fScarSoilDecayProp;

  /**Decay proportion for each timestep for tip-up mounds. This is
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly.  Array size
  * is m_iMaxDecayTimesteps.*/
  float *mp_fTipupDecayProp;

  /**Codes for using log data members of calculations grid.  Array indexes are 
   * species group, size, decay class, and age (in timesteps)*/
  int ****mp_iLogCalcsCode;
  
  /**Codes for using the new log data members of calculations grid.  Array 
   * indexes are species group, size, and decay class.*/
  int ***mp_iNewLogCalcsCode;

	/**Code for using newtipup data member of calculations grid*/
  int m_iNewTipupCalcsCode;    

  /**Array (species by type) of whether or not substrate applies to this tree
  * kind*/
  bool **mp_bSubstrateApplies;

  /**Scarified soil alpha decay parameter.*/
  float m_fScarifiedSoilA;

  /**Tip-up alpha decay parameter.*/
  float m_fTipUpA;

  /**Scarified soil beta decay parameter.*/
  float m_fScarifiedSoilB;

  /**Tip-up beta decay parameter.*/
  float m_fTipUpB;

  /**Scarified soil initial conditions proportion*/
  float m_fInitScarifiedSoil;

  /**Tip-up initial conditions proportion*/
  float m_fInitTipUp;

  /**Log initial conditions proportion*/
  float ***mp_fInitLog;

  /**Scarified soil partial cut proportion.  Not required if Harvest is not
  * being used.*/
  float m_fPartCutScarifiedSoil;

  /**Tip-up partial cut proportion.  Not required if Harvest is not being
  * used.*/
  float m_fPartCutTipUp;

  /**Log partial cut proportion.  Array indexes are species group, size, 
   * and decay class. Not required if Harvest is not being used.*/
  float *** mp_fPartCutLog;


  /**Scarified soil gap cut proportion.  Not required if Harvest is not being
  * used.*/
  float m_fGapCutScarifiedSoil;

  /**Tip-up gap cut proportion.  Not required if Harvest is not being used.*/
  float m_fGapCutTipUp;

  /**Log gap cut proportion.  Array indexes are species group, size, 
   * and decay class. Not required if Harvest is not being used.*/
  float *** mp_fGapCutLog;

  /**Scarified soil clear cut proportion.  Not required if Harvest is not being
  * used.*/
  float m_fClearCutScarifiedSoil;

  /**Tip-up clear cut proportion.  Not required if Harvest is not being
  * used.*/
  float m_fClearCutTipUp;

  /**Log clear cut proportion.  Array indexes are species group, size, 
   * and decay class. Not required if Harvest is not being used.*/
  float *** mp_fClearCutLog;

  /**Amount by which the tree radius is multiplied when calculating the tip-up
  * mound to account for the effects of root rip-out*/
  float m_fRootTipupFactor;

  /**Proportion of the litter/moss pool that is moss*/
  float m_fMossProportion;

  /**Grid cell area of substrate grid*/
  float m_fGridCellArea;

  /**Maximum number of years a package is allowed to exist*/
  int m_iMaxDecayYears;

  /**m_iMaxDecayYears divided by the number of years per timestep, for setting
  * up grid data members.*/
  int m_iMaxDecayTimesteps;

  int m_iNumTotalSpecies;   /**<Total number of species - for destructor*/
  int m_iScarSoilCode;      /**<Code for using scarsoil grid data member*/
  int m_iFFMossCode;        /**<Code for using ffmoss grid data member*/
  int m_iFFLitterCode;      /**<Code for using fflitter grid data member*/
  int m_iTipupCode;         /**<Code for using tipup grid data member*/
  
  int m_iOrigScarSoilCode;	/**<Code for using scarsoil grid data member in original substrate grid*/
  int m_iOrigFFMossCode;	/**<Code for using ffmoss grid data member in original substrate grid*/
  int m_iOrigFFLitterCode;	/**<Code for using fflitter grid data member in original substrate grid*/
  int m_iOrigTipupCode;		/**<Code for using tipup grid data member in original substrate grid*/
  int m_iOrigFreshLogCode;	/**<Code for using freshlog grid data member in original substrate grid*/
  int m_iOrigDecLogCode;	/**<Code for using declog grid data member in original substrate grid*/
  
  
  /**Code for using log area grid data members.  Array indexes are species
   * group, size, and decay class.*/
  int ***mp_iLogCodes;      
  
  /**Code for using log volume grid data members.  Array indexes are species
   * group, size, and decay class.*/
  int ***mp_iLogVolCodes;
  
  int m_iPkgAgeCode;        /**<Code for using age package data member*/
  int m_iPkgScarSoilCode;   /**<Code for using scarsoil package data member*/
  int m_iPkgTipupCode;      /**<Code for using tipup package data member*/
  int m_iHarvestTypeCode;   /**<Code for using harvesttype data member of harvest grid*/


	/**Code for using the total log area grid data member.*/
  int m_iTotalLogCode;
  
  /**Code for using the total log volume grid data member.*/
  int m_iTotalLogVolCode;

  /**Whether or not the parameter file had a substrate map in it.  If it did,
  * initial conditions will not be populated.*/
  bool m_bParFileHasSubstrateMap;

  /**Whether to use directional tree fall.  If true, trees fall in a random
  * direction and contribute log area to potentially multiple grid cells.
  * If false, trees that fall contribute all of their fresh log area to the
  * cell in which they are rooted.
  */
  bool m_bUseDirectionalTreeFall;

  /**
  * Grid object containing substrate data, called "DetailedSubstrate".
  *
  * If there is a harvest grid, the grid cell resolution must match it.  If
  * this is not the case, and there is no user-defined grid cell resolution,
  * then the default grid resolution is used.
  *
  * Maps of this grid in the parameter file will be honored, but they must
  * define all data members below (even if they don't include values for all of
  * them).
  *
  * The grid cell record will consist of 66 float data members. They are:
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
  * <td>loggroup<1-3><small/large>decay<1-5></td>	<td>float</td>	<td> Species group <1-3>, small (or large), decay class <1-5> logs, as a proportion between 0 and 1.  
  * </tr>
  * <tr>
  * <td>vloggroup<1-3><small/large>decay<1-5></td>	<td>float</td>	<td> Species group <1-3>, small (or large), decay class <1-5> logs, as volume in m3 per ha.  
  * </tr>
  * <tr>
  * <td>totallog</td>	<td>float</td>	<td> Total logs, expressed as a proportion between 0 and 1. Should be the sum of all log proportions</td>
  * </tr>
  * <tr>
  * <td>totallogvol</td>	<td>float</td>	<td> Total log volume, in m3 per ha. Should be the sum of all log volumes</td>
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
  * </table>
  *
  * The packages will be in the order of oldest first.
  */
  clGrid *mp_oSubstrateGrid;

  /**
  * Grid object for intermediate calculations.  This grid is called
  * "detailedsubstratecalcs".  The grid cell resolution matches "Detailed 
  * Substrate".  Data members:
  * <table>
  * <tr>
  * <th>Data member name</th>  <th>Data type</th>  <th>Description</th>
  * </tr>
  * <tr>
  * <td>newtipup</td>          <td>float</td>      <td>New tip-up area by grid cell, in square meters</td>
  * </tr>
  * <tr>
  * <td>newloggroup<1-3><small/large>decay<1-5></td>        <td>float</td>      <td>Area of new species group <1-3>, small/large, decay class <1-5> logs added this timestep, in square meters</td>
  * </tr>
  * <tr>
  * <td>loggroup<1-3><small/large>decay<1-5>_<0-m_iMaxDecayTimesteps></td>          <td>float</td>      <td>Species group <1-3>, small/large, decay class <1-5> logs added <0-m_iMaxDecayTimesteps> timesteps ago, as a proportion of grid cell area</td>
  * </tr>
  * </table>
  */
  clGrid *mp_oCalcGrid;

  /**Harvest grid object, if it exists*/
  clGrid *mp_oHarvestGrid;
  
  /**Original "Substrate" grid with same data members as the original substrate
   * behavior.  Values are copied to it for compatibility with behaviors looking 
   * for this grid*/ 
  clGrid *mp_oOriginalSubstrateGrid;

  /**
  * Gets the values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);
  
  /**
  * Calculates the proportion of decay for each timestep for logs, tip-
  * up mounds, and scarified soil.  This calculates
  * @htmlonly exp( &alpha; * t <sup> &beta; </sup>) @endhtmlonly for each
  * timestep up to m_iMaxDecayTimesteps.
  */
  void CalculateDecayProportions();

  /**
  * Sets up the substrate grids and registers all their data members.  If the
  * harvest grid exists, substrate is required to match its grid cell
  * resolution.
  */
  void SetupSubstrateGrids();
  
  /**
  * Adds the amount of substrate in the substrate map or parameter file to the 
  * substrate and calculations grids.
  */
  void SetInitialSubstrate();

  /**
  * Populates the mp_iDeadCodes, mp_iFallCodes, mp_iOldBreakCodes, mp_iNewBreakCodes
  * arrays with the data member codes for the "dead", "Fall", "SnagOldBreakHeight", and 
  * "NewBreakHeight" data members, respectively. Also fills in the mp_bSubstrateApplies
  * array.
  * @throw Error if there is a species/type combo which participates in
  * substrate but does not have a bool data member called "dead" registered.
  * @throw Error if the appropriate species/type combos that participate in
  * substrate do not have a bool data member called "Fall" and float data members 
  * called "SnagOldBreakHeight" and "NewBreakHeight" registered.
  */
  void GetCodes();


  /**
  * Adds the effects of harvests to the substrate grid.  If Harvest isn't being
  * used, or no harvest occurred this timestep, nothing happens.  If there was
  * a harvest, each grid cell in which harvest occurred is evaluated
  * separately.  All existing substrate for a cell is scaled down by the amount
  * of new substrate created. New harvest substrate is created with the conditions 
  * from the parameter file matching the harvest type in the "harvestresults" grid.  
  * Age is set to 0, but harvest occurs before decay so there will be 1 timestep's 
  * decay before the outputting final grid cell values.  
  */
  void AddHarvestSubstrate();

  /**
   * Calculates totals for packages and the calculations grid and writes them to the 
   * main substrate grid. It also write the values to the original "substrate" grid 
   * for compatibility with behaviors that use it.
   * */
   void CalcMainGridTotals();


  /**
  * Performs substrate decay. Log decay is iterated on an annual basis so that 
  * logs can pass through >1 decay class over the timestep.  Each year, the amount of
  * log in a decay class advancing to the next decay class is calculated and stored in 
  * a temporary array.  After all annual iterations, the logs that decayed are written to 
  * age 0 or 1 positions of the calculations grid (rounded based on the year of the last 
  * transition).  The age-specific amount of log that did not decay is written to the next 
  * higher age bin in the calculations grid.  
  * 
  * Tipup and scarified soil packages are reduced by the amount of decay they experience 
  * and their age is incremented. Packages that exceed the maximum age are deleted.
  */
  void DecaySubstrate();

  /**
  * Calculates the post-decay log volumes for each grid cell. Log volume is not tracked by 
  * age, so changes in volume are made proportional to the changes in area after
  * decay.
  * */ 
  void CalcVolumeAfterDecay();

  /**
  * Adds the effects of the new dead trees to the substrate grid by calling
  * AddNewDeadTrees() and AdjustSubstrateForMortality().
  */
  void MortalitySubstrate();

  /**
  * Finds new fallen dead trees, and fallen snags, and broken snags to add to 
  * substrate.  Fallen dead trees are identifiable as adults with a value of true 
  * in their "dead" and "Fall" data members. Fallen snags are identifiable as snags 
  * with a value of true in their "dead" data member. The tops and bottoms of broken 
  * snags also add to substrate.  A snag contributing it's top to substrate has a value 
  * of false in it's "dead" data member and a value >0.0 in its "NewBreakHeight" data 
  * member, with that value indicating the height of the breakage.  A broken snag 
  * contributing its bottom to substrate has a value of true in it's "dead" data member 
  * and a value >0.0 in its "SnagOldBreakHeight" data member, with that value indicating
  * the height of the breakage. The values of "Fall", "NewBreakHeight", and "SnagOldBreakHeight" 
  * data members should be set by a snag dynamics behavior that determines whether 
  * snags and dead trees have fallen or broken in a given timestep.
  *  
  * For fallen trees, a random number determines if it uproots (and thus adds to the new 
  * tip-up area exposed).  Adults and snags have separate probabilities of uproot.
  *
  * Newly dead trees that fall are passed to clTreePopulation::KillTree() with remove as
  * the code to force them to go away.  This ensures they do not become snags. Fallen
  * snags are removed later by the Dead Tree Remover behavior.
  *
  * The decay class a fallen tree enters is determined by comparing a random 
  * number to the probabilities of entering each decay class in 
  * mp_fPropFallenTreesLogDecayClass[1-5]. 
  * 
  * Both the amount of new log and the amount of tip-up added are a
  * function of tree size.  New log area is calculated as A = r*h,
  * where A is new log area in square meters, r is tree radius in
  * meters, and h is tree height in meters.  New log volume is calculated as
  * V = 1/3 * pi * r<sup>2</sup> * h. New tip-up area is calculated as
  * OA = pi * (r*F)<sup>2</sup>, where OA is the new tip-up area in square
  * meters, r is the tree radius in meters, and F is the value in
  * m_fRootTipupFactor, which accounts for the effects of root disturbance.
  * 
  * The area of the log is divided evenly into h*2 chunks, where h is the tree 
  * height in m. The size class of a chunk of fallen tree is determined by 
  * comparing it's midpoint diameter to mp_fLogSizeClassBoundary. 
  * 
  * If m_bDirectionalTreeFall is true, trees that fall do so in a random
  * direction.  The function walks out the length of the log
  * in 0.5-m intervals, and whatever grid cell the end of the interval falls
  * in gets a log area chunk.  If m_bDirectionalTreeFall is false, all
  * log area is put in the cell in which the tree was rooted. Tipup area is 
  * always added to the cell in which the tree was rooted.
  * 
  * For each tree, its amounts of new logs and new tip-up, in square
  * meters, are added to the newloggroup<1-3><small/large>decay<1-5> and 
  * newtipup data members of the calculations grid. It's new volume is added 
  * to vloggroup<1-3><small/large>decay<1-5> data members of the substrate grid.
  */
  void AddNewDeadTrees();

  /**
  * For each grid cell in the substrate and calculations arrays, this adjusts 
  * the package and log values for new substrate added in AddNewDeadTrees().  
  * First, the amounts of new substrate for new logs and new tip-up in square 
  * meters of area is converted to a proportion of total substrate.  Then the 
  * proportions of all existing substrate are reduced accordingly to make way 
  * for the new stuff. The new substrate from mortality is then added in.
  */
  void AdjustSubstrateForMortality();


};
//---------------------------------------------------------------------------
#endif
