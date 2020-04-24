//---------------------------------------------------------------------------

#ifndef CompetitionHarvestH
#define CompetitionHarvestH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"

class clGrid;
class clTree;
class clTreePopulation;
class clPlot;

using namespace whyDead;

/**
* Competition Based Harvest - Version 1.2
*
* This is a behavior which performs harvests by preferentially removing those
* trees that exert the most competitive pressure on their neighbors. This
* behavior is also more autonomous - it decides for itself when harvests occur
* and how much cutting to do based on criteria provided by the user.
*
* There are two ways to specify when harvests occur.
*
* 1)  Fixed basal area threshold. A harvest occurs every time the total plot
* basal area exceeds a threshold set by the user. There is a minimum interval
* between harvests which can be used to make sure they do not occur too
* frequently. The harvests cut either an absolute amount of basal area or a
* percentage of the total.
*
* 2)  Fixed interval. The user specifies how often harvests occur. The harvests
* cut the plot back to EITHER a set total basal area, or a proportion of the
* current basal area.
*
* Either way, the user can specify a minimum and maximum DBH for trees to
* harvest.
*
* Harvests may be species-specific or non-species-specific. If they are
* species-specific, the user specifies the relative amounts to remove of each
* species (including 0 for non-participating species). 100% participation for
* all species indicates that all species can be treated as a single group.
* Only participating species and size ranges are used to establish basal area
* criteria for initiating a harvest.
*
* The harvest preferentially removes the most competitive trees. The
* competitive effect (COE) of a tree i on neighbors j within a given radius is
* calculated as follows:
*
* @htmlonly
 <center><i>COE = exp(-C*DBH<sup>&gamma;</sup>*&lambda;*
 (DBH<sup>&alpha;</sup>/distance<sup>&beta;</sup>)<sup>D</sup>)</i></center>
  @endhtmlonly

* summed over all neighbors. The values of C, gamma, alpha, beta, and D are
* specific to the neighbor's species. Lambda is for the effect of the target
* species i on the neighbor species j. DBH is the neighbor's DBH, and distance
* is the distance in meters between the target and neighbor. Optionally, DBH
* can be divided by a divisor term if there needs to be a units correction.
*
* Trees in the plot are ranked by COE. The harvest behavior works its way down
* the list, removing trees in order, until the amount to harvest requirements
* have been met. After each tree is removed, the COE of all trees in its
* immediate vicinity are recalculated and the COE list will be resorted.
*
* If removing a tree will cause the harvest to overshoot its cutting target, a
* random number will be compared to the amount of overshoot to determine if the
* tree will be removed. Harvesting ends with this tree.
*
* If the cut is species-specific, a tree high on the competitiveness list will
* be skipped if a sufficient amount of that species has already been removed.
*
* The behavior uses a grid to report on actual harvest amounts removed across
* the plot, and optionally can provide a list of all individuals removed each
* timestep, along with their basal area, biomass and bole volume.
*
* Seedlings, snags, and dead trees have neither competitive effect on neighbors
* or have competitive effects put on them by other trees. In other words, they
* are always ignored.
*
* This adds one float data member to trees to which it is applied to hold their
* competitive effect. This member is called "COE". It has limited usefulness
* outside of this behavior.
*
* This behavior's namestring and parameter file call string are both
* "CompetitionHarvest".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
* <br>November 7, 2019 - Added the ability to specify a %BA to cut to a fixed
* interval harvest
* <br>January 10, 2020 - Added the option to let the model run for a period of
* time before beginning harvests
* <br>January 31, 2020 - Added the option to cut from least to most competitive
*/
class clCompetitionHarvest : virtual public clBehaviorBase {

 public:

 /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
 clCompetitionHarvest(clSimManager *p_oSimManager);

 /**
  * Destructor.
  */
 ~clCompetitionHarvest();

 /**
  * Performs the harvesting. First this calls ResetResultsGrid(). Then this
  * decides whether a harvest will occur. If this is a fixed interval harvest,
  * it checks the amount of time that has passed. If this is a fixed BA harvest,
  * this calls GetBasalArea() to find out whether the basal area is over the
  * threshold. If a harvest is to occur, this calls CutTreesSpeciesSpecific() or
  * CutTreesNotSpeciesSpecific().
  */
 void Action();

 /**
  * Registers the "COE" tree float data member. The return codes are captured
  * in the mp_iCOECodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * saplings and adults.
  */
 void RegisterTreeDataMembers();

 protected:

 /**
 * RESULTS GRID
 * This grid is called "Competition Harvest Results". It holds the actual amount
 * cut for the current timestep. The grid's cell resolution is up to the user,
 * but defaults to one grid cell covering the whole plot.
 * The data is stored raw - no conversion to per-hectare amounts.
 *
 * Any parameter file grid map for this grid it will be ignored.
 * <table>
 * <tr>
 * <th>Data member name</th> <th>Type</th> <th>Description</th>
 * </tr>
 * <tr>
 * <td>Cut Density_sp</td> <td>int</td>    <td>Number of trees cut. There is one
 *                                         for each species; sp is the species
 *                                         number.</td>
 * </tr>
 * <tr>
 * <td>Cut Basal Area_sp</td><td>float</td><td>Total basal area cut. There is
 *                                        one for each species; sp is the
 *                                        species number.</td>
 * </tr>
 * </table>
 */
 clGrid *mp_oResultsGrid;

 /**Which tree in each grid cell (matching the cells of the clTreePopulation
  * class) has the most cut-eligible COE (highest if we're cutting most
  * competitive first, lowest if we're cutting least competitive first). Array
  * size is number of X cells (in variable m_iNumX) by number of Y cells (held
  * in variable m_iNumY).*/
 clTree ***mp_oMostestCOE;

 /**The value for the COE of each tree in mp_oMostestCOE. Array size is number
  * of X cells (in variable m_iNumX) by number of Y cells (held in variable
  * m_iNumY).*/
 float **mp_fCOE;

 /**Lambda for NCI. Array is sized number of species by number of species. The
  * first index is for targets, the second for neighbors. Thus mp_fLambda[2][3]
  * is the effect of target species 2 on species 3 neighbors.*/
 double **mp_fLambda;

 /**Data member codes for the "COE" tree float data member. Array size is number
  * of species by 2 (saplings and adults).*/
 short int **mp_iCOECodes;

 /**Target DBH effect on each neighbor species. @htmlonly &alpha; @endhtmlonly
  * variable in COE equation above. Array is sized number of species and is
  * required for all species.*/
 double *mp_fAlpha;

 /**Target distance effect. @htmlonly &beta; @endhtmlonly variable in
  * COE equation above. Array is sized number of species and is
  * required for all species.*/
 double *mp_fBeta;

 /**Crowding effect slope. C in COE equation above. Array is sized number of
  * species and is required for all species.*/
 double *mp_fC;

 /**Crowding effect steepness. D in COE equation above. Array is sized number
  * of species and is required for all species.*/
 double *mp_fD;

 /**Size sensitivity to competition parameter. @htmlonly &gamma; @endhtmlonly in
  * COE equation above. Array is sized number of species and is required for
  * all species.*/
 double *mp_fGamma;

 /**Maximum radius, in meters, for which a target has a crowding effect on
  * neighbors. Array is sized number of species but is only required for those
  * species to which this behavior applies.*/
 double *mp_fMaxCrowdingRadius;

 /**Holds the target left to cut for each species. Ignored unless
  * m_bIsSpeciesSpecific is true. CutTrees() manages this array to
  * properly direct the behavior of the other functions. Array size is number
  * of species.*/
 double *mp_fTargetToCut;

 /**The amount cut this timestep for each species. Ignored unless
  * m_bIsSpeciesSpecific is true. This can be compared against
  * the mp_fTargetToCut array to decide whether an individual of a certain
  * species is eligible for cutting. Array size is number of species.*/
 double *mp_fAlreadyCut;

 /**Proportion of each species to cut as a value between 0 and 1. All 1s
  * mean treat the species as a common pool (and m_bSpeciesSpecific = false).
  * Array is sized number of species but is only required for those species to
  * which this behavior applies.*/
 double *mp_fPropToCut;

 /**Data member codes for the "Cut Density_sp" data member of the "Competition
  * Harvest Results" grid. Array size is number species.*/
 short int *mp_iDenCutCodes;

 /**Data member codes for the "Cut Basal Area_sp" data member of the
  * "Competition Harvest Results" grid. Array size is number species.*/
 short int *mp_iBaCutCodes;

 /**The filename to which to write the list of all trees harvested each
  * timestep. This is optional. If the file is to be written, it is a
  * tab-delimited text file. The columns are X, Y, Species, DBH, and Timestep
  * cut.
  */
 std::string m_sHarvestListFilename;

  /**Maximum crowding radius across all species - used when recalculating COEs
  * after a tree has been cut.*/
  double m_fMaxMaxCrowdingRadius;

  /**Target DBH divisor. The target's DBH is divided by this term before being
  * used in the COE equation. This is in case any units corrections need to be
  * made.*/
 double m_fQ;

 /**Mininum DBH for harvesting.*/
 double m_fMinHarvestDBH;

 /**Maxinum DBH for harvesting.*/
 double m_fMaxHarvestDBH;

 /**Cut amount - if this is a fixed interval harvest with a target amount to
  * leave, this is the amount to which the plot is cut back, in m2 of basal
  * area;
  *
  * if this is a fixed interval harvest with a proportion of BA to cut, this
  * is the proportion to cut;
  *
  * if this is a fixed BA threshold harvest with fixed amount to cut, this is
  * that amount to cut, in m2 of basal area;
  *
  * if this is a fixed BA threshold harvest with proportion to cut, this is
  * the proportion to cut between 0 and 1. BA amounts are read as per ha from
  * the parameter file, but they are stored as a total amount here to avoid
  * having to convert back and forth.*/
 double m_fAmtToCut;

 /**For fixed BA threshold harvests, the BA threshold that triggers a harvest.
  * This is read in from the parameter file as m2/ha, but stored as total
  * m2 of basal area.*/
 double m_fBAThreshold;

  /**Minimum sapling height.  For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**For fixed BA threshold harvests, the minimum interval between harvests. For
  * fixed interval harvests, the interval between harvests. This is read from
  * the parameter file in years but is stored as timesteps.*/
  int m_iInterval;

  /**Timestep to begin harvesting*/
  int m_iTimestepToStartHarvests;

  /**Reason code to pass to the tree population when trees are killed.*/
  deadCode m_iReasonCode;

  /**How many timesteps it's been since the last harvest.*/
  short int m_iTimeSinceLastHarvest;

  /**Number of cells in the X direction for the mp_oHighestCOE and mp_fCOE
   * arrays.*/
  short int m_iNumX;

  /**Number of cells in the Y direction for the mp_oHighestCOE and mp_fCOE
   * arrays.*/
  short int m_iNumY;

  /**Number of species. For destructor.*/
  short int m_iNumSpecies;

  /**Whether to cut the most competitive first (true) or the least
   * competitive.*/
  bool m_bCutMostCompetitive;



 /**Type of harvest*/
 enum cutType {interval_rem,  /**<Fixed interval, with target to leave           */
               ba_amt,        /**<Fixed BA threshold with fixed amount to cut    */
               ba_prop,       /**<Fixed BA threshold with proportion to cut      */
               interval_prop  /**<Fixed interval, with proportion to cut         */
               } harvest;     /**<What kind of harvest is being used for this run*/

 /**Whether or not this is a species-specific harvest.*/
 bool m_bIsSpeciesSpecific;

 /**
  * Resets all the values in the "Competition Harvest Results" grid. All values
  * are set to 0.*/
 void ResetResultsGrid();

 /**
  * Performs setup.
  * <ol>
  * <li>Calls SetupGrids()</li>
  * <li>Calls ReadParameterFileData()</li>
  * <li>Calls SetupCOEGrids()</li>
  * <li>Does some other miscellaneous setup calculations</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  */
 void GetData(xercesc::DOMDocument *p_oDoc);

 /**
 * Reads harvest data from the parameter file.
 * @param p_oDoc DOM tree of parsed input file.
 * @param p_oPop Tree population object.
 * @throws modelErr if:
 * <ul>
 * <li>Harvest type is fixed interval but the interval value is less than 1</li>
 * <li>Harvest type is fixed BA with proportion to cut but cut amount is not a
 * proportion between 0 and 1</li>
 * <li>Species proportions to cut do not add up to 1 (unless they are all
 * 1)</li>
 * <li>Harvest type is fixed BA but the minimum interval is less than 1</li>
 * <li>Any value in max crowding radius is not greater than 0</li>
 * </ul>
 */
 void ReadParameterFileData(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

 /**
  * Sets up the "Competition Harvest Results" grid and gets all the return
  * codes.
  */
 void SetupGrids();

 /**
  * Sets up the mp_oHighestCOE and mp_fCOE arrays.
  * @param p_oPop Tree population object.
  */
 void SetupCOEGrids(clTreePopulation * p_oPop);

 /**
  * Performs species-specific harvests. This calculates basal area cut targets,
  * depending on the type of harvest being performed. Then this calls
  * CalculateCOESpeciesSpecific() to get tree competitive effects. The most
  * competitive tree across the entire plot is cut, and then
  * RecalculateCOESpeciesSpecific() is called. This process is repeated until
  * cut targets have been reached. Only the highest-COE trees of species that
  * are left are put in the mp_oHighestCOE and mp_fCOE arrays. If cutting a tree
  * would cause an overshoot of that species' target BA, then a random number is
  * compared to the amount of difference between the target and the overshoot to
  * decide whether that tree is cut.
  * @param fPlotBA Basal area of the plot.
  */
 void CutTreesSpeciesSpecific(const float &fPlotBA);

 /**
  * Performs non-species-specific harvests. This calculates basal area cut
  * targets, depending on the type of harvest being performed. Then this calls
  * CalculateCOENotSpeciesSpecific() to get tree competitive effects. The most
  * competitive tree across the entire plot is cut, and then
  * RecalculateCOENotSpeciesSpecific() is called. This process is repeated until
  * cut targets have been reached. If cutting a tree would cause an overshoot of
  * the target BA, then a random number is compared to the amount of difference
  * between the target and the overshoot to decide whether that tree is cut.
  * @param fPlotBA Basal area of the plot.
  */
 void CutTreesNotSpeciesSpecific(const float &fPlotBA);

 /**
  * Calculates competitive effects for those trees that are eligible for
  * harvest. After calculating the COE, this finds the tree with the highest
  * COE in each grid cell and populates the mp_oHighestCOE and mp_fCOE arrays.
  * Eligible trees are those that have a COE tree data member, and whose species
  * has a positive difference between mp_fTargetToCut and mp_fAlreadyCut.
  */
 void CalculateAllCOEsSpeciesSpecific(clTreePopulation *p_oPop);

 /**
  * Calculates competitive effects for those trees that are eligible for
  * harvest. After calculating the COE, this finds the tree with the highest
  * COE in each grid cell and populates the mp_oHighestCOE and mp_fCOE arrays.
  * Eligible trees are those that have a COE tree data member.
  */
 void CalculateAllCOEsNotSpeciesSpecific(clTreePopulation *p_oPop);

 /**
  * Recalculates COEs in the neighborhood of a recently cut tree. All trees
  * within a distance of m_fMaxMaxCrowdingRadius from the point fX, fY have
  * their COEs recalculated. Any applicable updates are made to mp_oHighestCOE
  * and mp_fCOE. Only trees whose species still have a positive difference
  * between mp_fTargetToCut and mp_fAlreadyCut are recalculated and considered
  * for a spot on mp_oHighestCOE.
  * @param p_oPop Tree population object.
  * @param fX X coordinate of tree that was removed.
  * @param fY Y coordinate of tree that was removed.
  */
 void RecalculateCOESpeciesSpecific(clTreePopulation *p_oPop, const float &fX, const float &fY);

 /**
  * Recalculates COEs in the neighborhood of a recently cut tree. All trees
  * within a distance of m_fMaxMaxCrowdingRadius from the point fX, fY have
  * their COEs recalculated. Any applicable updates are made to mp_oHighestCOE
  * and mp_fCOE.
  * @param p_oPop Tree population object.
  * @param fX X coordinate of tree that was removed.
  * @param fY Y coordinate of tree that was removed.
  */
 void RecalculateCOENotSpeciesSpecific(clTreePopulation *p_oPop, const float &fX, const float &fY);

 /**
  * Gets the basal area across the plot for those trees that are eligible for
  * harvest. Eligible trees are those that are of a species participating in
  * harvests (those to which this behavior is applied) whose DBHs are between
  * the minimum and the maximum.
  * @return The amount of basal area, in m2.
  */
 float GetBasalArea();

 /**
  * Calculates the competitive effect for one tree. If the tree is dead, the
  * answer is zero. All dead trees are also excluded as neighbors.
  * @param p_oPlot Plot object.
  * @param p_oPop Tree population object.
  * @param p_oTree Tree for which to calculate competitive effect.
  * @return This tree's COE.
  */
 float CalculateOneCOE(clPlot *p_oPlot, clTreePopulation *p_oPop, clTree *p_oTree);
};
//---------------------------------------------------------------------------
#endif
