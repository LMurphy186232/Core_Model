//---------------------------------------------------------------------------

#ifndef NCIBAGrowthH
#define NCIBAGrowthH
//---------------------------------------------------------------------------
#include "GrowthBase.h"


class clTree;
class clTreePopulation;

/**
* NCI Basal Area growth - Version 1.0
*
* This is a growth shell object which applies the NCI (neighborhood competition
* index) function.  The competition index uses neighbor basal area.
*
* The growth equation for one year's growth is:
* <br>
* <center>Growth = Max Growth * Size Effect * Crowding Effect</center>
* <br>
* where Max Growth is annual amount of diameter growth, and the Effects are
* values between 0 and 1 which serve to reduce the maximum.
*
* The equation for Size Effect is:
* <center>SE = exp(-0.5(ln(DBH/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</center>
*
* where:
* <ul>
* <li>DBH is for the target tree, in cm</li>
* <li>X<sub>0</sub> is the size effect mode, in cm</li>
* <li>X<sub>b</sub> is the size effect variance, in cm</li>
* </ul>
*
* Crowding Effect is calculated as:
* <br>
* @htmlonly
  <center>CE = exp(-C * ( DBH<sup>&gamma;</sup> * &Sigma; BA / 1000)<sup>D</sup>)</center>
  @endhtmlonly
* <br>
* where:
* <ul>
* <li>CE = crowding effect</li>
* <li>C is the NCI slope parameter</li>
* <li>D is the NCI steepness parameter</li>
* <li>DBH is of the target tree, in meters</li>
* <li>@htmlonly &gamma; @endhtmlonly is the size sensitivity to NCI parameter</li>
* <li>@htmlonly &Sigma; BA @endhtmlonly is the sum of the basal areas of
* neighbors, in square cm</li>
* </ul>
*
* Neighbors can be all trees larger than the minimum NCI DBH, or only those
* trees with DBHs larger than the target, in square cm.
*
* Snags and already-dead trees are never counted in the basal area sum.
*
* The amount of growth is in cm/year. For multi-year timesteps, the behavior
* will calculate total growth with a loop. Each loop iteration will increment
* DBH for one year. For each year, the Size Effect (SE) value and the
* @htmlonly DBH<sup>&gamma;</sup> @endhtmlonly portion of the Crowding Effect
* is recalculated with the previous year's new DBH value. All values for each
* year of growth are summed to get the growth for the timestep.
*
* This cannot be applied to seedlings.  An error will be thrown if seedlings
* are passed.
*
* The parameter file call string for this to be diameter-incrementing with
* auto-height updating is "NCIBAGrowth"; for diameter-only incrementing, use
* "NCIBAGrowth diam only".  The namestring for this behavior is
* "ncibagrowthshell".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clNCIBAGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clNCIBAGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clNCIBAGrowth();

  /**
  * Returns the value in the tree's float data member that holds the value
  * that was calculated by PreGrowthCalcs().
  *
  * @param p_oTree Tree to which to apply growth.
  * @param p_oPop Tree population object.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return Amount of diameter growth, in cm.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Calculates growth for all NCI trees.  The values are stashed in the
  * "Growth" tree float data member for later application.
  *
  * Steps:
  * <ol>
  * <li>Get all trees for this behavior.</li>
  * <li>For each tree, calculate the sum of basal areas by calling the function
  * GetBasalAreas(). Stash the value in "Growth" for each tree.</li>
  * <li>Go through all the NCI trees again.  Calculate the amount of growth for
  * each using the equations above.  Use the function pointers to make sure
  * that the proper function forms are used.  Stash the end result in
  * "Growth".</li>
  * </ol>
  * This must be called first of any growth stuff, since it uses other trees'
  * DBHs to calculate NCI, and these must be before growth has been applied.
  *
  * Growth per timestep is calculated by looping over the number of years
  * per timestep and incrementing the DBH.
  *
  * @param p_oPop Tree population object.
  */
  void PreGrowthCalcs( clTreePopulation *p_oPop );

  /**
  * Does setup.
  * <ol>
  * <li>AssembleUniqueTypes() is called to create a list of unique behavior
  * types.</li>
  * <li>ReadParameterFile() is called to read the parameter file's data.</li>
  * <li>ValidateData() is called to validate the data.</li>
  * <li>GetGrowthCodes() is called to get tree data return codes.</li>
  * </ol>
  *
  * @param p_oDoc DOM tree of parsed input tree.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Captures the namestring passed to this behavior.  This is overridden from
  * clBehaviorBase so we can capture the namestring passed.  Since this class
  * can create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

  protected:

  /**Holds return data codes for the "Growth" tree data member.  Array size is
  * number of species to which this behavior applies by 2 (saplings and
  * adults).*/
  short int **mp_iGrowthCodes;

  /**Crowding effect slope. C in Crowding Effect equation above.  Array is
  * sized number of species to which this behavior applies.  This array is
  * accessed by using the index returned for mp_iIndexes[species number].*/
  float *mp_fCrowdingSlope;

  /**Crowding effect steepness. D in Crowding Effect equation above.  Array is
  * sized number of species to which this behavior applies.  This array is
  * accessed by using the index returned for mp_iIndexes[species number].*/
  float *mp_fCrowdingSteepness;

  /**Maximum search radius, in meters, in which to look for crowding
  * neighbors.  For calculating the Crowding Effect.  Array is sized number of
  * species to which this behavior applies.  This array is accessed by using
  * the index returned for mp_iIndexes[species number].
  */
  float *mp_fMaxCrowdingRadius;

  /**The minimum DBH, in cm, of neighbors to be included in NCI calculations.
  * Array is sized number of species.*/
  float *mp_fMinimumNeighborDBH;

  /**Size effect variance parameter. X<sub>b</sub> in Size Effect equation
  * above.  Array is sized number of species to which this behavior applies.
  * This array is accessed by using the index returned for
  * mp_iIndexes[species number].*/
  float *mp_fXb;

  /**Maximum potential growth value, in cm. Array is sized number of species
  * to which this behavior applies. This array is accessed by using the index
  * returned for mp_iIndexes[species number].*/
  float *mp_fMaxPotentialValue;

  /**Size effect mode parameter. X<sub>0</sub> in Size Effect equation above.
  * Array is sized number of species to which this behavior applies. This array
  * is accessed by using the index returned for mp_iIndexes[species number].*/
  float *mp_fX0;

  /**Size sensitivity to NCI parameter. @htmlonly &gamma; @endhtmlonly in
  * Crowding Effect equation above. Array assumed to be sized number of
  * species to which this behavior applies. This array is accessed by using
  * the index returned for mp_iIndexes[species number].*/
  float *mp_fGamma;

  /**Speeds access to the arrays. Array size is number of species.*/
  short int *mp_iIndexes;

  /**List of types managed by this behavior.*/
  short int *mp_iWhatBehaviorTypes;

  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Divisor for neighbor BA.*/
  float m_fBADivisor;

  /**Number of types managed by this behavior.*/
  short int m_iNumBehaviorTypes;

  /**Keep our own copy for the destructor. This is the total number of tree
  * species.*/
  short int m_iNumTotalSpecies;

  /** Whether to use all trees larger than the minimum (false) or only neighbors
   * larger than the target (true)*/
  bool m_bUseOnlyLargerNeighbors;

  /**
  * Makes sure all input data is valid. The following must all be true:
  * <ul>
  * <li>Max radius of neighbor effects must be > 0</li>
  * <li>Max growth for each species must be > 0</li>
  * <li>X0 (size effect mode) for each species must be > 0</li>
  * <li>Xb (size effect variance) for each species must not = 0</li>
  * </ul>
  * @throws modelErr if any of the above conditions are not met.
  */
  void ValidateData();

  /**
  * Gets the return codes for the "Growth" float tree data member for each tree
  * kind to which this behavior applies.
  * @throws modelErr if a code comes back -1 for any species/type combo to
  * which this behavior is applied.
  */
  void GetGrowthCodes();

  /**
  * Calculates the sum of the basal areas for neighbors larger than the
  * target tree's DBH.
  * @param p_oTarget Target tree for which to calculate the sum of neighbor
  * basal areas.
  * @param p_oPop Tree population, for getting neighbors.
  * @returns Sum of neighbor basal areas, in square cm.
  */
  float GetBasalAreasLargerNeighbors(clTree *p_oTarget, clTreePopulation *p_oPop);

  /**
  * Calculates the sum of the basal areas for all neighbors larger than the
  * NCI min DBH.
  * @param p_oTarget Target tree for which to calculate the sum of neighbor
  * basal areas.
  * @param p_oPop Tree population, for getting neighbors.
  * @returns Sum of neighbor basal areas, in square cm.
  */
  float GetBasalAreasAllNeighbors(clTree *p_oTarget, clTreePopulation *p_oPop);

  /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input tree.
  * @throws modelErr if this behavior has been applied to any types except
  * sapling and adult.
  */
  void ReadParameterFile( xercesc::DOMDocument *p_oDoc );

  /**
  * Assembles a unique list of types applied to this behavior and places it in
  * mp_iWhatBehaviorTypes.
  */
  void AssembleUniqueTypes();
};
//---------------------------------------------------------------------------
#endif
