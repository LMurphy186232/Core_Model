// MicroEstablishment
//---------------------------------------------------------------------------
#if !defined(MicroEstablishment_H)
  #define MicroEstablishment_H

#include "BehaviorBase.h"

class clTree;
class clGrid;
/**
* Establishment with Microtopography - Version 1.0
*
* This behavior germinates seeds cast by dispersal into seedlings.  In
* addition, seeds are given a height above the ground due to microtopography.
*
* All seeds are germinated into seedlings.  A seed's substrate determines its
* rooting height.  In order to seeds to substrates, the number of seeds is
* evenly divided as much as possible among the different substrate types.  The
* clSubstrate class divides each grid cell of the "Substrate" grid into six
* substrate types; this further divides each substrate type in two, for mound
* and ground.  Mound and ground are in fixed relative proportion to each other.
*
* The definition of seedling rooting height comes from the clLightFilter
* class. Since this class makes no sense without that, that behavior is
* required to be in the run (as marked by the presence of the tree data members
* "lf_count" and "z").
*
* Ground substrates (except fresh log) give a seed a rooting height of 0.
* Fresh log and mound substrates give rooting heights that are randomly drawn
* on normal distributions, the shape of which are controlled by means and
* standard deviations provided by the user.
*
* If the seed lands on a log substrate, it also gets a respite from fern
* shading.  The number of years of respite is set in the "lf_count" data
* member.  The number of years is determined by the age of the log substrate on
* which the seedling landed (which is determined by dividing the fresh logs
* pool up by age and doing a random draw to determine which it is), and that
* age is subtracted from the maximum respite time.
*
* Requirements for using an object of this class in a run:
* <ul>
* <li>A disperse behavior is used
* <li>Substrate (clSubstrate) is used
* <li>Light filter (clLightFilter) is used
* </ul>
*
* The namestring and parameter file call string for this class are both
* "MicroEstablishment".  Apply this behavior to the desired species; use any
* type, since type will be ignored.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clMicroEstablishment : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

   /**My unit tester - exists in the testing application.  I didn't want to
   * have to do this but I couldn't figure out a way to effectively unit test
   * any other way.*/
   friend class clTestMicroEstablishment;

   public:

   /**
   * Constructor.
   *
   * @param p_oSimManager Sim Manager object.
   */
   clMicroEstablishment(clSimManager *p_oSimManager);

   /**
   * Destructor.
   */
   ~clMicroEstablishment();

  /**
  * Performs setup.  Calls:
  * <ul>
  * <li>GetParameterFileData()
  * <li>SetupGrids()
  * <li>DoLightSetup(), if m_bUsesLight is true
  * </ul>
  * @param p_oDoc DOM tree of parsed input file.
  */
   void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs establishment.  Steps for each grid cell in Dispersed Seeds:
  * <ol>
  * <li>The proportions of each of the substrate types are calculated for the
  * grid cell by calling GetSubstrateProportions().
  * <li>The seeds are divided up among the substrate types according to their
  * relative proportions.
  * <li>A seedling is created for each seed.
  * <li>The seedling then gets a rooting height and a fern light respite
  * counter, depending on the type of substrate upon which it landed.  If it
  * landed on fresh logs, it's passed to SetFreshLogZAndRespite().  If it
  * landed on a mound substrate, it's passed to SetMoundZAndRespite().  If it
  * landed on a ground substrate, it's passed to SetGroundZAndRespite().
  * </ol>
  */
   void Action();

   /**
   * Zeroes out the seed grid.
   */
  void TimestepCleanup();

  /**
   * Produces an array of substrate proportions.  First, the amount of
   * substrate for each substrate type is placed in the appropriate array
   * bucket. Then mound substrates are multiplied by the proportion of the total
   * area that is mound; ground substrates are multipled by 1 - that proportion.
   * Areas are averaged over grid cells if seed grid cell sizes are bigger than
   * substrate grid cell sizes.
   *
   * @param fFromX Origin X coordinate of the grid cell of "Dispersed Seeds" for
   * which to get substrate proportions.
   * @param fFromY Origin Y coordinate of the grid cell of "Dispersed Seeds" for
   * which to get substrate proportions.
   * @param fToX End X coordinate of the grid cell of "Dispersed Seeds" for
   * which to get substrate proportions.
   * @param fToY End Y coordinate of the grid cell of "Dispersed Seeds" for
   * which to get substrate proportions.
   * @param p_fSubstrate Pointer to an array, sized number_substrates, in which
   * to put the substrate proportion for each value in substrateTypes.
   */
  void GetSubstrateProportions(float *p_fSubstrate, const float &fFromX,
      const float &fFromY, const float &fToX, const float &fToY);

  /**
   * Produces an array of cumulative fresh log proportions added for
   * m_iMaxRespiteTimesteps timesteps.
   *
   * First, the amount of fresh log substrate for each timestep is placed in the
   * appropriate array bucket.  This value comes from substrate cohort packages.
   * We have to decay the amount of fresh log by the age of the cohort; here's
   * how:
   * <center>FL' = FL * exp( a * T<sup> b</sup> )</center>
   * where FL' is the amount of fresh logs left, FL is the amount of original
   * fresh logs (the amount in the "freshlog" package data member), a is the
   * fresh log decay alpha substrate parameter, T is the cohort's age in years,
   * and b is the fresh log beta substrate decay parameter.
   *
   * Then the values are converted to proportion of total fresh log by dividing
   * each array bucket by the total in all buckets.  Then, to each array value,
   * the total value of all previous buckets is added so that the fresh log
   * proportions are cumulative.
   *
   * <b>Note:</b>This also used to collect fresh log mean DBHs.  This has been
   * commented out as not currently needed.
   *
   * Areas are averaged over grid cells if seed grid cell sizes are bigger than
   * substrate grid cell sizes.
   * @param fFromX Origin X coordinate of the grid cell of "Dispersed Seeds" for
   * which to get substrate proportions.
   * @param fFromY Origin Y coordinate of the grid cell of "Dispersed Seeds" for
   * which to get substrate proportions.
   * @param fToX End X coordinate of the grid cell of "Dispersed Seeds" for
   * which to get substrate proportions.
   * @param fToY End Y coordinate of the grid cell of "Dispersed Seeds" for
   * which to get substrate proportions.
   * @param p_fLogs Pointer to an array, sized m_iMaxRespiteTimesteps + 1, in
   * which to put the fresh log proportions.  Each bucket will hold the
   * cumulative proportion of fresh log of the array index's age in timesteps.
   * The last bucket is always 1.
   */
  void GetFreshLogProportions(float *p_fLogs, const float &fFromX, const float &fFromY,
      const float &fToX, const float &fToY);

  /**
   * Sets the rooting height of a seedling in mm if it has landed on a mound
   * substrate that is not fresh logs.  The rooting height is a random
   * normally-distributed value using the mound mean height (in
   * m_fMoundHeightMean) and mound standard deviation (in
   * m_fMoundStandardDeviation).  In this non-fresh-log case, the fern respite
   * counter is always 0.
   * @param p_oSeedling The seedling for which to set rooting height and fern
   * respite.
   */
   void SetMoundZAndRespite(clTree *p_oSeedling);

   /**
   * Sets the rooting height of a seedling in mm if it has landed on a ground
   * substrate that is not fresh logs.  The rooting height and fern respite
   * counters are always 0.
   * @param p_oSeedling The seedling for which to set rooting height and fern
   * respite.
   */
   void SetGroundZAndRespite(clTree *p_oSeedling);

  /**List of substrate types - be extremely careful when changing these!*/
     enum substrateType {mound_scarsoil, /**<Scarified soil on mound*/
                         mound_tipup, /**<Tipup on mound*/
                         mound_freshlogs, /**<Fresh logs on mound*/
                         mound_decayedlogs, /**<Decayed logs on mound*/
                         mound_fflitter, /**<Forest floor litter on mound*/
                         mound_ffmoss, /**<Forest floor moss on mound*/
                         ground_scarsoil, /**<Scarified soil on ground*/
                         ground_tipup, /**<Tipup on ground*/
                         ground_freshlogs, /**<Fresh logs on ground*/
                         ground_decayedlogs, /**<Decayed logs on ground*/
                         ground_fflitter, /**<Forest floor litter on ground*/
                         ground_ffmoss, /**<Forest floor moss on ground*/
                         number_substrates /**<This is now shorthand for the
                                            * total number of substrates*/
                         };

   protected:

   /**Pointer to "Substrate" grid, created by clSubstrate.*/
   clGrid *mp_oSubstrateGrid;

   /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
   clGrid *mp_oSeedGrid;

  /**Data member codes for seed grid for number of seeds.  Array size is
  * # behavior species.*/
  short int *mp_iSeedGridCode;

  /**Holds data member codes for "lf_count" for seedlings - array size
  * is # behavior species.*/
  short int *mp_iCounterCodes;

  /**Holds data member codes for "z" for seedlings - array size is #
   * behavior species.*/
  short int *mp_iZCodes;

  /**Speeds access to arrays*/
  short int *mp_iIndexes;

  /**Array of data member codes for the "Substrate" grid.  It's sized
  * number_substrates; repeat substrates get the same code (i.e. mound
  * and ground fresh logs*/
  short int *mp_iSubstrateCodes;

  /**Proportion of the plot that is mound*/
  float m_fMoundProportion;
  /**Mean mound height. Read in in meters, then converted to mm*/
  float m_fMoundHeightMean;
  /**Mound height standard deviation. Read in in meters, then converted
  * to mm*/
  float m_fMoundStandardDeviation;
  /**Fresh log substrate mean height. Read in in meters, then converted
  * to mm*/
  float m_fFreshLogHeightMean;
  /**Fresh log height standard deviation. Read in in meters, then converted
  * to mm*/
  float m_fFreshLogStandardDeviation;
  /**Fresh log alpha decay parameter; from substrate parameters. This lets
  * this class calculate fresh log ages.*/
  float m_fFreshLogA;
  /**Fresh log alpha decay parameter; from substrate parameters. This lets
  * this class calculate fresh log ages.*/
  float m_fFreshLogB;

  int m_iMaxRespiteYears; /**<Maximum number of years of fern respite -
  from parameter file*/
  int m_iMaxRespiteTimesteps; /**<Maximum number of timesteps of fern respite -
  calculated from m_iMaxRespiteYears*/

  /**Data member code - package float - dec_flog*/
  short int m_iCohFreshLogCode;

  /**Data member code - package int - age*/
  short int m_iCohAgeCode;

  /**
  * Gets parameter file data.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if m_fMoundProportion is not between 0 and 1, inclusive.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Populates required grid pointers.  This gets a pointer to the "Substrate"
  * grid and all its data members, and gets a pointer to the "Dispersed Seeds"
  * grid and all its data members.
  * @throws modelErr if either the "Substrate" or "Dispersed Seeds" grid does
  * not exist.
  */
  void SetupGrids();

  /**
  * Declares and populates the values in the mp_iCounterCodes and mp_iZCodes
  * arrays, for the "lf_count" and "z" data members of seedlings, respectively.
  * @throws ModelErr if there is no code for any species which uses
  * establishment.
  */
  void GetTreeDataMemberCodes();

 /**
 * Sets the rooting height of a tree in mm and a fern respite counter in years,
 * if a seedling has landed on fresh logs.  A random number is compared to
 * successive values in p_fFreshLogProportions until an array value is greater
 * than the random value.  That bucket represents the age of the fresh logs, in
 * timesteps, onto which the seed has landed.  The respite counter is
 * m_iMaxRespite minus the age of the fresh log cohort in years.
 * <p>
 * The rooting height is a normally-distributed random number using the fresh
 * log mean height and standard deviation from the parameter file.  (Fresh logs
 * overlie and obliterate the effects of mounds.)
 * @param p_oSeedling The seedling for which to set rooting height and fern
 * respite.
 * @param p_fFreshLogProportions An array holding the cumulative proportion of
 * fresh logs x timesteps ago, up to m_iMaxRespiteTimesteps.
 */
 void SetFreshLogZAndRespite(clTree *p_oSeedling, float *p_fFreshLogProportions);

};
//---------------------------------------------------------------------------
#endif // MicroEstablishment_H
