/*
 * MastingDisperseAutocorrelation.h
 *
 * This class does spatial disperse with masting and autocorrelation in seed
 * production, as developed for Mario Pesendorfer's project.
 *
 * Mean seed rain changes from year to year. This can be randomly generated or
 * input as a time series.
 *
 * The number of trees participating in seed production is a sigmoidal function
 * of the mast level: c/(1+(x/a)^b), where x is the masting level, and a, b,
 * and c are parameters. c is the asymptote of the function maximum and must be
 * between 0 and 1.
 *
 * The number of seeds an individual tree produces is itself a function of its
 * seed producer score (fixed trait assigned at "birth"), its size, and the
 * number of seeds it produced last year.
 *
 * Once the number of seeds produced by a tree is known, they are dispersed
 * using the common kernels - Weibull or lognormal.
 *
 * This adds 2 float tree data members:
 * sps = seed producer score; a value with a mean of one, and user-specified sd
 * prevseeds = last year's seed crop for this tree; initial value is the
 * seed crop produced by the classic Ribbens equation used in other disperse
 * behaviors
 *
 *  Created on: Mar 2, 2021
 *      Author: lora
 */

#ifndef MASTINGDISPERSEAUTOCORRELATION_H_
#define MASTINGDISPERSEAUTOCORRELATION_H_

#include "DisperseBase.h"
#include "DataTypes.h"

class clTreePopulation;
class clPlot;
class clTree;

class clMastingDisperseAutocorrelation: public clDisperseBase {
public:

  /**
   * Constructor.
   *
   * @param p_oSimManager Sim Manager object.
   */
  clMastingDisperseAutocorrelation(clSimManager *p_oSimManager);
  virtual ~clMastingDisperseAutocorrelation();

  /**
     * Registers the data members.
     * @throw modelErr if this behavior is being applied to any tree type except
     * seedlings, saplings, and adults.
     */
    void RegisterTreeDataMembers();

  /**
    * Get mast level for a timestep. Only for testing.
    * @param iTs Timestep, 1 to number of timesteps.
    * @return Value of mast, or -1 if timestep is invalid.
    */
   double GetMastLevel(int iTs);

   /**
    * Get max timesteps.
    * @return Max timesteps.
    */
   int GetMaxTimesteps() {return m_iMaxTimesteps;};

   /**
   * For testing.
   */
   // void WriteCumProbArray();

  protected:

   /**Cumulative probability array for seed dispersal. Array size is # species
    * by max distance. The 0th bucket will never be accessed.*/
   float **mp_fSeedCDF;

   /**Data member codes for "sps" data member. First array index is #
    * species, second is number types*/
   short int **mp_iSpsCodes;

   /**Data member codes for "prevseeds" data member. First array index is #
    * species, second is number types*/
   short int **mp_iPrevseedsCodes;

   /** Time series of masting levels, 0-1. This can be input by the user, or
    * randomly generated. This array is sized number of timesteps.*/
   double *mp_fMastTimeSeries;

   /**STR mean. The index is species. This value comes from the parameter file.*/
   double *mp_fStrMean;

    /**Beta parameter for each species. This value comes from the
    * parameter file.*/
   double *mp_fBeta;

   /**A parameter in the sigmoidal function for fraction reproducing. Controls
    * slope.
    */
   double *mp_fReproFractionA;

   /**B parameter in the sigmoidal function for fraction reproducing. Exponent.
    */
   double *mp_fReproFractionB;

   /**C parameter in the sigmoidal function for fraction reproducing.
    * Function max.
    */
   double *mp_fReproFractionC;

   /**Whether this behavior is used by a species/type combo.  First array index
    * is species, second is type.*/
   bool **mp_bIsUsed;

   /**Array of species with each one's dbh for reproduction*/
   double *mp_fDbhForReproduction;

   /**Max DBH for each species for size effects; DBH above this is truncated
    * to this value */
   double *mp_fMaxDbh;

   /**This will speed access to the other arrays by storing each species' array
    * index so the other arrays only have to be as big as the number of unique
    * species for this behavior.*/
   short int *mp_iIndexes;

   /**Query to perform to search for trees*/
   char *m_cQuery;

   /**Number of years per timestep*/
   int m_iNumYearsPerTimestep;

   /**Maximum distance, in meters, a seed can disperse - which is the maximum
    * dimension of the grid with max of 1000 m*/
   int m_iMaxDistance;

   /** Maximum timesteps before a masting event occurs after a first masting
    * event. This is calculated to be the value at which all species have a prob
    * of 0.9999, or the number of timesteps in the run, whichever is smaller.*/
   int m_iMaxTimesteps;

   /**
   * Does setup. This sets all values in mp_iTimestepsSinceLastMast to 1. Then
   * it calls:
   * <ol>
   * <li>GetParameterFileData</li>
   * <li>CalcMastCDF</li>
   * <li>CalcSeedCDF</li>
   * <li>FormatQueryString</li>
   * <li>PopulateUsedTable</li>
   * <li>SetGetSeedsFunctionPointers</li>
   * </ol>
   *
   * @param p_oDoc DOM tree of parsed input file.
   */
   void DoShellSetup(xercesc::DOMDocument *p_oDoc);

   /**
   * Calculates the cumulative distribution functions for seed dispersal. This
   * reads in the appropriate parameter values, finds the maximum seed dispersal
   * distance, then creates the CDF array. The parameters are thrown away
   * because they are no longer needed.
   * @param p_oElement Parent parameters tag from parsed XML file
   * @param p_oPop Tree population object.
   * @throws stcErr if a weibull theta value is not less than 50 (to prevent pow
   * overflows)
   */
   void CalcSeedCDF(xercesc::DOMElement *p_oElement, clTreePopulation *p_oPop);

   /**
    * Formats the string in m_cQuery. This value will be used in AddSeeds() in
    * order to get the trees to act on.
    * @param p_oPop Tree Population object
    */
   void FormatQueryString(clTreePopulation *p_oPop);

   /**
   * Performs dispersal of seeds for one tree. The number of seeds is calculated
   * by using the pointer in mp_GetSeeds. Each seed is given a random azimuth
   * direction from the parent. Then each seed is given a random distance from
   * the parent that conforms to the chosen probability distribution function.
   * This is done by comparing a random value to successive values in the
   * cumulative probability array until the first array bucket that has a greater
   * value than the random number. Once the seed has an azimuth direction and
   * a distance, it is added to the species total in the appropriate grid cell.
   * @param p_oTree Tree for which to perform dispersal.
   * @param fDbh DBH of the tree, in cm.
   * @param p_oPlot Plot object
   * @param p_oPop Tree Population object
   */
   void DisperseOneParentSeeds(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, float fDbh );

   /**
   * Extracts needed parameter file data. (Some parameters are extracted by
   * other setup functions and thrown away - this gets all other, permanent
   * parameters.)
   * @param p_oParent Parent parameters tag and data.
   * @throws stcErr if:
   * <ul>
   * <li>The function codes are not valid enums</li>
   * <li>A beta value is greater than 25 (to prevent pow overflows)</li>
   * <li>A value in mp_fFractionParticipating is not between 0 and 1</li>
   * </ul>
   * @param p_oPop Tree population object.
   */
   void GetParameterFileData(xercesc::DOMElement *p_oParent, clTreePopulation * p_oPop);

   /**
   * Declares and populates the mp_bIsUsed array.
   * @param p_oPop Tree Population object
   */
   void PopulateUsedTable(clTreePopulation *p_oPop);

   /**
   * Calculates the normalized probability distribution for a function
   * for one species.
   * @param p_fProbArray The array into which to put the normalized values.
   * @param iMaxDistance The maximum distance out to which to calculate the
   * function - which must equal the size of p_fProbArray.
   * @param iFunction Function flag.
   * @param fDispersalX0 Dispersal or X0 of the species in question, depending
   * on the function
   * @param fThetaXb Theta or Xb of the species in question, depending
   * on the function
   */
   void CalculateProbabilityDistribution( float * p_fProbArray,
   const int &iMaxDistance, const function &iFunction, const float &fDispersalX0,
   const float &fThetaXb);

   /**
    * Performs dispersal. This will:
    * <ul>
    * <li>Get the mast level for the timestep</li>
    * <li>Calculate the fraction of the population reproducing</li>
    * <li>For each tree:
    * <ul><li>Calculate its probability of reproducing</li>
    * <li>If reproducing, calculate autocorrelation coefficient</li>
    * <li>Calculate total number of seeds</li>
    * <li>Disperse seeds</li>
    * </ul></li>
    * </ul>
    */
   void AddSeeds();

   /**
     * Reads a mast timeseries, if the user specifies one. If one is not
     * found, this will create a randomly generated time series.
     * @param p_oParent Parent tag to look in.
     */
   void GetMastTimeseries(xercesc::DOMElement *p_oParent);
};

#endif /* MASTINGDISPERSEAUTOCORRELATION_H_ */
