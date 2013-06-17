//---------------------------------------------------------------------------

#ifndef MastingNonSpatialDisperseH
#define MastingNonSpatialDisperseH
//---------------------------------------------------------------------------
#include "DisperseBase.h"
#include "DataTypes.h"



/**
 * Masting non-spatial disperse - Version 1.0
 *
 * This class performs non-spatial dispersal with masting. The actual dispersal
 * is non-density dependent, or bath, dispersal. This is a constant seed rain
 * which does not depend on the presence of parent trees in the plot. The
 * actual disperal - just dropping a constant amount of seed everywhere - is
 * easy. Figuring out how much seed to drop is the complicated bit.
 *
 * The first decision is whether or not there will be masting. This is done
 * by using a random draw on a binomial probability distribution. Each species
 * gets its parameters for this distribution.
 *
 * Once it is known whether masting will occur, the actual number of seeds per
 * square meter is drawn from a random distribution. Each species can use its
 * own probability function, and can use different functions in masting and
 * non-masting timesteps.
 *
 * Once the number of seeds per square meter is known, they are evenly
 * distributed to the seed grid cells.
 *
 * As an additional refinement, species can mast together in groups. In this
 * case, all the species in a group are treated as one entity - masting
 * together, and using the same parameters for seed dispersal - except that at
 * the end, once the number of seeds per square meter is known, they are
 * divided up according to the relative basal areas of the adults in the group
 * across the entire plot. If there are no adults of any of the species in the
 * plot, the seeds are divided evenly.
 *
 * Apart from grouping, species operate completely independently and the
 * masting activities of one species do not affect the others in any way.
 *
 * This behavior's namestring and parameter file call string are both
 * "MastingNonSpatialDisperse".
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */
class clMastingNonSpatialDisperse : virtual public clDisperseBase {

 public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clMastingNonSpatialDisperse(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clMastingNonSpatialDisperse();

  /**
   * Get whether a given species masted this timestep.
   * @param iSp Species to check
   * @return mast or nonmast.
   */
  mastEvent GetMastEvent(int iSp);

  protected:

    /** Mu parameter for inverse gaussian distribution. The array is 2D - the
     * first index is masting or non-masting. The second index is number
     * behavior species. */
    float **mp_fInvGaussMu;

    /** Lambda parameter for inverse gaussian distribution. The array is 2D -
     * the first index is masting or non-masting. The second index is number
     * behavior species.*/
    float **mp_fInvGaussLambda;

    /** Mean for normal distribution. The array is 2D - the first index is
     * masting or non-masting. The second index is species. */
    float **mp_fNormalMean;

    /** Standard deviation for normal distribution. The array is 2D - the first
   * index is masting or non-masting. The second index is species. */
    float **mp_fNormalStandardDev;

    /** P parameter for binomial distribution for deciding when to mast -
    * array size = number behavior species*/
    float *mp_fBinomialP;

    /** "Basal area" for each species. Since all we care about is relative
     * proportion, this will just add up the squares of the radii. Array size =
     * number behavior species.*/
    float *mp_fBasalArea;

    /**Group affiliation of the species. Any species with the same group number
     * will always mast together. Array size is number behavior species.*/
    short int *mp_iGroup;

    /**Which event is occurring in the current timestep for each species*/
    enum mastEvent *mp_iEvent;

    /**PDF to use for seed draw. The array is 2D - the first index is masting
     * or non-masting. The second index is number behavior species. */
    pdf **mp_iFunction;

    /**Whether or not species are put into groups.*/
    bool m_bGroupsUsed;

    /**
     * Reads in parameter file values.
     * @param p_oDoc DOM tree of parsed parameter file.
     */
    void DoShellSetup(xercesc::DOMDocument *p_oDoc);

    /**
    * Computes the total "basal area" of all adults for each species. This will
    * allow calculation of relative basal area for each group. Basal area is
    * not actually basal area, since we don't need to multiply by pi to get the
    * relative values. We'll just square the radii. Totals are placed in
    * mp_fBasalArea.
    */
    void ComputeBasalArea();

   /**
    * Does dispersal. This calculates basal area as necessary for groups;
    * checks to see if masting is occurring or not; then uses the appropriate
    * random draw and the appropriate parameters to pick the number of seeds to
    * add to each grid cell.
    */
  void AddSeeds();

};
//---------------------------------------------------------------------------
#endif
