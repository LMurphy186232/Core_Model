//---------------------------------------------------------------------------

#ifndef NCIJuvenileGrowthH
#define NCIJuvenileGrowthH
//---------------------------------------------------------------------------
#include "GrowthBase.h"


class clTree;
class clTreePopulation;
class clAllometry;

/**
* NCI juvenile growth - Version 1
*
* This is a growth shell object which applies a variant on the NCI (neighborhood
* competition index) function designed for juvenile trees.
*
* Growth per year is Growth = Max Growth * Size Effect * Crowding Effect.
* The amount of growth is in cm/year. For multi-year timesteps, the behavior
* will calculate total growth with a loop. Each loop iteration will increment
* DBH for one year. For each year, the Size Effect (SE) value  is recalculated
* with the previous year's new DBH value. All values for each year of growth
* are summed to get the growth for the timestep.
*
* Size Effect = a * d10 ^ b, where d10 is diameter at 10 cm height. Crowding
* Effect = exp(-C * NCI ^ D). NCI is calculated with d10, not DBH, even for
* adults; the d10 - DBH conversion equation will be used with the sapling
* parameters.
*
* This can only be applied to seedlings and saplings.  An error will be thrown
* otherwise.
*
* The parameter file call string for this to be diameter-incrementing with
* auto-height updating is "NCIJuvenileGrowth"; for diameter-only incrementing,
* use "NCIJuvenileGrowth diam only".  The namestring for this behavior is
* "ncijuvenilegrowthshell".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clNCIJuvenileGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clNCIJuvenileGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clNCIJuvenileGrowth();

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
  * <li>For each tree, calculate NCI<sub>i</sub> by calling the function in the
  * function pointer NCI.  Stash the value in "Growth" for each tree.</li>
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
  * <li>GetTreeMemberCodes() is called to get tree data return codes.</li>
  * <li>SetFunctionPointers() is called to set up our function pointers.</li>
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

  short int **mp_iGrowthCodes; /**<Holds return data codes for the "Growth"
  tree data member.  Array size is number of species to which this behavior
  applies by 2 (seedlings and saplings).*/
  short int *mp_iWhatBehaviorTypes; /**<List of types managed by this behavior.*/
  short int m_iNumBehaviorTypes; /**<Number of types managed by this behavior.*/

  /**Lamba for NCI. Array is sized number of behavior species by number of total
   * species.  This array is accessed by using the species number as an array
   * index.*/
  float **mp_fLambda;

  /**Neighbor diam10 effect. @htmlonly &alpha; @endhtmlonly variable in Crowding
  * Effect equation.  Array is sized number of species to which this behavior
  * applies.  This array is accessed by using the index returned for
  * mp_iIndexes[species number].*/
  float *mp_fAlpha;

  /**Neighbor distance effect. @htmlonly &beta; @endhtmlonly variable in
  * Crowding Effect equation.  Array is sized number of species to which
  * this behavior applies.  This array is accessed by using the index returned
  * for mp_iIndexes[species number].*/
  float *mp_fBeta;

  /**Crowding effect slope. C in Crowding Effect equation.  Array is sized
   * number of species to which this behavior applies.  This array is accessed
   * by using the index returned for mp_iIndexes[species number].*/
  float *mp_fCrowdingSlope;

  /**Crowding effect steepness. D in Crowding Effect equation. Array is sized
   * number of species to which this behavior applies.  This array is accessed
   * by using the index returned for mp_iIndexes[species number].*/
  float *mp_fCrowdingSteepness;

  /**The minimum Diam10, in cm, of neighbors to be included in NCI calculations.
  * Array is sized total number of species.*/
  float *mp_fMinimumNeighborDiam10;

  /**Size effect "a" parameter. Array is sized number of species to which this
   * behavior applies.  This array is accessed by using the index returned for
  * mp_iIndexes[species number].*/
  float *mp_fSizeEffectA;

  /**Size effect "b" parameter. Array is sized number of species to which this
   * behavior applies.  This array is accessed by using the index returned for
  * mp_iIndexes[species number].*/
  float *mp_fSizeEffectB;

  /**Maximum growth, cm/yr.  Array is sized number of species to which this
   * behavior applies.  This array is accessed by using the index returned for
   * mp_iIndexes[species number].*/
  float *mp_fMaxGrowth;

  /**Maximum search radius, in meters, in which to look for crowding
  * neighbors.  For calculating the Crowding Effect.  Array is sized
  * number of species to which this behavior applies.  This array is accessed
  * by using the index returned for mp_iIndexes[species number].
  */
  float *mp_fMaxCrowdingRadius;

  /**The value to divide diam10 by in NCI. <i>q</i> in the NCI equation above.
  * May be set to 1.*/
  float m_fDiam10Divisor;

  /**Speeds access to the arrays.  Array size is is number of
  * species.*/
  short int *mp_iIndexes;

  /**Whether or not to include snags in NCI*/
  bool m_bIncludeSnags;

  /**
  * Calculates the NCI value for a tree.
  * @htmlonly
  <center>NCI<sub>i</sub> = &Sigma; &lambda;<sub>k</sub>((D10<sub>k</sub>/q)<sup>&alpha;</sup>/distance<sup>&beta;</sup>)</center>
  @endhtmlonly
  * @param p_oTree Tree for which to calculate NCI.
  * @param p_oPop Tree population object.
  * @param p_oAllom Allometry object.
  * @param p_oPlot Plot object.
  * @return NCI value.
  */
  float CalculateNCI(clTree * p_oTree, clTreePopulation * p_oPop, clAllometry *p_oAllom, clPlot * p_oPlot);

  /**
  * Makes sure all input data is valid.  The following must all be true:
  * <ul>
  * <li>Max radius of neighbor effects must be >= 0</li>
  * <li>Max growth for each species must be > 0</li>
  * <li>DBH divisor must be > 0</li>
  * </ul>
  * @throws modelErr if any of the above conditions are not met.
  */
  void ValidateData();

  /**
  * Gets the return codes for needed tree data members.
  * @throws modelErr if a code comes back -1 for any species/type combo to
  * which this behavior is applied.
  */
  void GetTreeMemberCodes();

  /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input tree.
  * @throws modelErr if this behavior has been applied to any types except
  * sapling and seedling.
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
