//---------------------------------------------------------------------------
#ifndef SpatialDisperseH
#define SpatialDisperseH
//---------------------------------------------------------------------------
#include "DisperseBase.h"

class clTreePopulation;
class clPlot;
class clTree;

/**
* Spatial dispersal - Version 1.0
*
* This class creates and disperses seeds according to the spatially-explicit
* model.  Parent trees of greater than the minimum reproductive DBH have the
* number of seeds calculated.  The seeds are dispersed around the plot
* according
* to either the Weibull or Lognormal probability distribution functions.
*
* Forest cover can be taken into account during seed distribution (gap) or
* ignored (non-gap).
*
* This behavior can also be used for suckering of stumps.  If this is desired,
* apply this to the species which sucker, tree type stump.  If this is gap
* disperse, stumps reproduce pretending they're always in gap conditions.  They
* use the probability distribution function of their species' gap function.
* If this is non-gap disperse, they reproduce like other adults of their
* species.
*
* The "seeds" are actually tallies by species in a grid.  This behavior does
* not produce any seedlings.  A separate recruitment behavior must "germinate"
* the seeds into seedlings according to its own rules.
*
* To call this as gap disperse in the parameter file, use the string
* "GapDisperse".  To call this as non-gap disperse, use the string
* "NonGapDisperse".  There is a place in the parameters for indicating which
* probability distribution function should be used under each cover type.
* The namestring for this class is "SpatialDisperse".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
* <br>April 2, 2021 - Unified beta and STR for the two functions because
* those are always the same
*/
class clSpatialDispersal : virtual public clDisperseBase {
//note: need the virtual keyword to avoid base class ambiguity.

 public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clSpatialDispersal(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clSpatialDispersal();

  /**
  * Captures the behavior name passed from the parameter file.  This is useful
  * since this class can produce a few different kinds of behaviors.
  * @param sNameString Behavior name from parameter file.
  */
  void SetNameData(std::string sNameString);

  /**
  * For testing.
  */
  // void WriteCumProbArray();

protected:

  /**Dispersal (weibull) or X0 (lognormal) parameter. The array is 3D - first
   * index is which disperse function is used. The second index is cover. The
   * third index is species.  This value comes from the parameter file.*/
  double ***mp_fDispersalX0;

  /**Theta (weibull) or Xb (lognormal) parameter. If this is theta, this value
   * must be less than 50 to protect against pow overflows. The array is 3D -
   * first index is which disperse function is used. The second index is cover.
   * The third index is species. This value comes from the parameter file.*/
  double ***mp_fThetaXb;

  /**STR parameter. The first index is cover. The second index is species.
   * This value comes from the parameter file.*/
  double **mp_fStr;

  /**Beta parameter. The first index is cover. The second index is species.
   * This value comes from the parameter file.*/
  double **mp_fBeta;

  /**Fecundity for 30 cm dbh tree for each species. The array is 3D - first
   * index is which disperse function is used. The second index is cover. The
   * third index is species.*/
  double ***mp_fFecundity;

  /**Beta parameter for stumps. This matches the beta parameter for live trees
   * for this species.*/
  double *mp_fStumpBeta;

  /**STR parameter for stumps. This matches the STR parameter for live trees
   * for this species.*/
  double *mp_fStumpStr;

  /**Fecundity for stumps.*/
  double *mp_fStumpFecundity;

  /**Array of species with each one's dbh for reproduction*/
  double *mp_fDbhForReproduction;

  /**Cumulative probability array for gap and canopy cover. Array size is
   * # functions by # covers by # species by max distance.*/
  double ****mp_fCumProb;

  /**Number of years per timestep*/
  int m_iNumYearsPerTimestep;

  /**Maximum distance, in meters, a seed can disperse*/
  int m_iMaxDistance;

  /**Maximum number of reproductively mature trees allowed in a grid cell for
   * the grid cell to retain gap status*/
  int m_iMaxGapDensity;

  /**This will speed access to the other arrays by storing each species' array
   * index so the other arrays only have to be as big as the number of unique
   * species for this behavior.*/
  short int *mp_iIndexes;

  /**How many functions there are - 2*/
  short int m_iNumFunctions;

  /**How many cover types there are - 2*/
  short int m_iNumCovers;

  /**Which function is used - array indexes are cover by species*/
  function **mp_iWhatFunction;

  /**Query to perform to search for trees*/
  char *m_cQuery;

  /**Whether this behavior is used by a species/type combo. First array index
   * is species, second is type.*/
  bool **mp_bIsUsed;

  /**True if this is gap disperse; false if not.*/
  bool m_bIsGap;

  /**True if this behavior applies to stumps.*/
  bool m_bStumps;

  /**
  * Reads in values from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates the function value at a particular point.
  * @param fDistance Distance at which to calculate function value.
  * @param iSpecies Species for which to calculate function value.
  * @param iFunction Which function to calculate (weibull or lognormal).
  * @param iCover Cover for which to calculate function value.
  * @return Value of function.
  */
  float CalculateFunctionValue(float fDistance, int iSpecies,
     function iFunction, cover iCover);

  /**
  * Declares the arrays.
  */
  void DeclareArrays();

  /**
  * Performs dispersal of seeds for one tree according to gap disperse.  The
  * number of seeds for a tree is calculated with the higher of gap or canopy
  * STR.
  *
  * Each seed is given a random azimuth angle.  It is then given a random
  * distance that conforms to the probability distribution function of the
  * current forest cover of the parent.  (This distance is gotten by comparing
  * a random value to successive values in the appropriate cumulative
  * probability array until the first array bucket that has a greater value
  * than the random number.)  Once the seed has an azimuth and a distance, the
  * function determines which grid cell it should drop in.
  *
  * Once the seed has a target grid cell, that cell's cover is checked.  Then
  * the seed's survival is evaluated.  If the seed is in the cover type with
  * the higher STR, it automatically survives.  Otherwise, a random number is
  * compared to the ratio of the lower STR to the higher STR to determine if it
  * survives.
  *
  * If the seed survives, it may need to be repositioned.  If both parent and
  * seed are under closed canopy, the seed is dropped where it is.  If the
  * parent is in gap and seedling is in canopy, a new distance is calculated as
  * though the parent was also in canopy.  The shortest of the two distances
  * is used to determine where the seed lands.  If the seed lands in a gap cell,
  * the function "walks out" the line of the seed's path from parent to target
  * landing cell, checking each intermediate grid cell's cover along the way.
  * If any of the grid cells in the line are under canopy cover, the seed
  * drops in the first canopy cell it reaches.
  * @param p_oTree Tree for which to perform dispersal.
  * @param fDbh DBH of the tree, in cm.
  * @param p_oPlot Plot object
  * @param p_oPop Tree Population object
  */
  void SpatialDisperse(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, float fDbh );

  /**
  * Performs dispersal of seeds for one tree according to non-gap disperse.
  * The number of seeds is calculated.  Each seed is given a random azimuth
  * direction from the parent.  Then each seed is given a random distance from
  * the parent that conforms to the chosen probability distribution function.
  * This is done by comparing a random value to successive values in the
  * cumulative probability array until the first array bucket that has a greater
  * value than the random number.  Once the seed has an azimuth direction and
  * a distance, it is added to the species total in the appropriate grid cell.
  * @param p_oTree Tree for which to perform dispersal.
  * @param fDbh DBH of the tree, in cm.
  * @param p_oPlot Plot object
  * @param p_oPop Tree Population object
  */
  void NonSpatialDisperse(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, float fDbh );

  /**
  * Extracts needed parameter file data.  This function takes into account
  * whether or not this is gap.
  *
  * The minimum reproductive DBH array is pre-loaded with the minimum adult
  * DBH values, so if there is a species that is not used by this behavior,
  * CalculateGapStatus() still has something to use.
  * @param p_oDoc Parsed parameter file document.
  * @throws stcErr if:
  * <ul>
  * <li>A weibull theta value is not less than 50 (to prevent pow
  * overflows)</li>
  * <li>The function codes are not valid enums</li>
  * <li>A beta value is greater than 25 (to prevent pow overflows)</li>
  * </ul>
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates fecundity and the probability distribution functions.  Fecundity
  * is calculated as <br>
  * <center>fec = STR/30<sup>beta</sup></center>
  */
  void CalcFecAndFunctions();

  /**
  * Declares and populates the mp_bIsUsed array.
  * @param p_oPop Tree Population object
  */
  void PopulateUsedTable(clTreePopulation *p_oPop);

  /**
  * Updates the gap status of the seed grid, if this is "GapDisperse".  It
  * counts all parent trees present in each grid cell (those that have a DBH
  * of at least the minimum reproductive age for their species).  The counts
  * are placed in each grid cell and when the count is higher than the cutoff
  * for gap status, the grid is under canopy; otherwise, it is under gap.
  * <p>
  * This counts all species, not just the ones to which this behavior applies.
  * <b>However</b>, for those species that do not use this behavior, their
  * minimum adult DBH will be used as the cutoff for counting, even if they have
  * a minimum reproductive DBH listed.  That's because I haven't bothered with
  * the logistics of figuring out if a species uses another disperse behavior.
  * @param p_oPop Tree population object.
  */
  void CalculateGapStatus(clTreePopulation *p_oPop);

  /**
  * Calculates the normalized probability distribution for a function
  * for one species.
  *
  * @param p_fProbArray The array into which to put the normalized values.
  * @param iMaxDistance The maximum distance out to which to calculate the
  * function - which must equal the size of p_fProbArray.
  * @param iSpecies Species for which we're calculating.
  * @param iFunction Function flag.
  * @param iCover Cover flag.
  */
  void CalculateProbabilityDistribution(double *p_fProbArray, int iMaxDistance,
       int iSpecies, function iFunction, cover iCover);

  /**
  * Performs dispersal.  This searches for all types to which it is applied.
  * For trees which are of the appropriate species/type combo, either
  * SpatialDisperse() or NonSpatialDisperse() is called.
  */
  void AddSeeds();

  /**
  * Gets the number of seeds to disperse for a tree, corrected for the number
  * of years per timestep.  The equation is
  * @htmlonly seeds = fecundity * DBH<sup>&beta;</sup>, @endhtmlonly
  * where all parameters are appropriate for forest cover if this is gap
  * disperse.
  * @param fDbh DBH of the tree, in cm
  * @param iSp Tree species number
  * @param iCover Forest cover
  * @param iFunc Disperse function used
  * @return Number of seeds.
  */
  float GetNumberOfSeeds(float fDbh, short int iSp, int iCover, int iFunc);

};
//---------------------------------------------------------------------------
#endif
