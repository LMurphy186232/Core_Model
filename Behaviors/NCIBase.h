//---------------------------------------------------------------------------
// NCIBase
//---------------------------------------------------------------------------
#if !defined(NCIBase_H)
  #define NCIBase_H

class clTree;
class clTreePopulation;
class clPlot;

/**
* NCI Base Class - version 1.1
*
* This class provides a base class for NCI functionality.  Child classes can
* descend both from this base and from the appropriate behavior base.
*
* The basic pattern for NCI calculations is as follows:
* <center><i>Value = Max Value * Damage Effect * Size Effect * Crowding Effect * Shading Effect</i></center>
*
* <i>Value</i> is the value being calculated, and <i>Max Value</i> is the
* maximum possible amount of that value in whatever units are appropriate.
* <i>Damage Effect</i>, <i>Size Effect</i>, <i>Shading Effect</i>, and
* <i>Crowding Effect</i> are multipliers bounded between 0 and 1 which act to
* reduce <i>Max Value</i>.
*
* Size Effect is calculated as:
* <center><i>SE = exp(-0.5(ln(DBH/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</i></center>
*
* where:
* <ul>
* <li><i>DBH</i> is for the target tree, in cm</li>
* <li><i>X<sub>0</sub></i> is the size effect mode, in cm</li>
* <li><i>X<sub>b</sub></i> is the size effect variance, in cm</li>
* </ul>
*
* Crowding Effect is calculated as:
* @htmlonly
  <center><i>CE = exp(-C * DBH <sup>&gamma;</sup> * NCI<sup>D</sup>)</i></center>
  @endhtmlonly
* <br>
* where:
* <ul>
* <li><i>CE</i> = crowding effect</li>
* <li><i>C</i> is the NCI slope parameter</li>
* <li><i>D</i> is the NCI steepness parameter</li>
* <li><i>DBH</i> is of the target tree, in meters</li>
* <li>@htmlonly <i>&gamma;</i> @endhtmlonly is the size sensitivity to NCI parameter</li>
* <li><i>NCI</i> is this tree's NCI value</li>
* </ul>
*
* NCI<sub>i</sub> is calculated as follows (simplifying the notation):
* @htmlonly
  <center><i>NCI<sub>i</sub> = &Sigma; &eta; &lambda;<sub>k</sub>((DBH<sub>k</sub>/q)<sup>&alpha;</sup>/distance<sup>&beta;</sup>)</i></center>

  where:
  <ul>
  <li>we're summing over k = 0 to N neighbors greater than the minimum
  neighbor DBH</li>
  <li><i>&eta;</i> is the storm damage parameter of the target,
  depending on the damage status.  It’s 1 if the neighbor is undamaged.</li>
  <li><i>&alpha;</i> is the neighbor DBH effect parameter</li>
  <li><i>&beta;</i> is the neighbor distance effect parameter</li>
  <li><i>DBH</i> is of the target tree, in cm</li>
  <li><i>q</i> is the DBH divisor parameter</li>
  <li><i>DBH<sub>k</sub></i> is the DBH of the kth neighbor, in meters</li>
  <li><i>&gamma;</i> is the size sensitivity to NCI parameter</li>
  <li><i>&lambda;<sub>k</sub></i> is the NCI lambda parameter for the species
  of the kth neighbor</li>
  <li><i>distance</i> is distance from target to neighbor, in meters</li>
  </ul>
  <br>
  @endhtmlonly
* NCI ignores neighbors with disturbance and harvest death codes. Natural deaths
* are NOT ignored, because it presumes that those deaths occurred in the current
* timestep and they should still be considered as live neighbors.
*
* Shading effect is calculated as follows:
* <center><i>SE = exp(-m * S<sup>n</sup>)</i></center>
*
* where:
* <ul>
* <li><i>SE</i> = Shading Effect</li>
* <li><i>m</i> = shading coefficient</li>
* <li><i>S</i> = amount of shade cast by neighbors, as calculated by clSailLight,
* between 0 and 1</li>
* <li><i>n</i> = shading exponent</li>
* </ul>
*
* Storm Effect is an input parameter.  There is one for trees with medium
* damage and one for trees with full damage.  Each is a value between 0 and 1.
* If the damage counter of the target tree = 0 (tree is undamaged),
* Storm Effect equals 1.
*
* It is expected that not all terms in the equation will be used for all trees.
* A user can turn off some of the terms by setting key parameters to 0 or 1.
* To be efficient, this behavior looks for this and has several alternate
* function forms to avoid extra math. It calls the correct function form
* through function pointers.  Different species can have different
* configurations.
*
* There are parameters defined, but this class does not populate the values nor
* validate them. That is the responsibility of the child class.  This class
* also does not bother checking for valid arrays before trying to access them.
* This class declares the arrays, NULLs pointers in the constructor, and
* deletes them in the constructor.
*
* Copyright 2005 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clNCIBase {

  protected:

  /**
  * Constructor.  NULLs out pointers.  It's OK for this to be protected since
  * it will only be called by children.
  */
  clNCIBase();

  /**
  * Destructor.  Deletes all arrays, freeing memory.
  */
  ~clNCIBase();

  /**
  * Declares array memory.  Obviously, call this before reading any parameters
  * into the arrays.
  * @param iNumBehaviorSpecies Number of species to which the behavior is
  * applied.
  * @param p_oPop Tree population, for getting seedling height
  */
  void DoNCISetup(clTreePopulation *p_oPop, int iNumBehaviorSpecies);

  /**Lamba for NCI. Array assumed to be sized number of behavior species by
  * number of total species.  This array is accessed by using the species
  * number as an array index.*/
  float **mp_fLambda;

  /**Neighbor DBH effect. @htmlonly &alpha; @endhtmlonly variable in Crowding
  * Effect equation above.  Array assumed to be sized number of species to
  which this behavior applies.  This array is accessed by using the index
  returned for mp_iIndexes[species number].*/
  float *mp_fAlpha;

  /**Neighbor distance effect. @htmlonly &beta; @endhtmlonly variable in
  * Crowding Effect equation above.  Array assumed to be sized number of
  * species to which this behavior applies.  This array is accessed by using
  * the index returned for mp_iIndexes[species number].*/
  float *mp_fBeta;

  /**Crowding effect slope. C in Crowding Effect equation above.  Array
  * assumed to be sized number of species to which this behavior applies.  This
  * array is accessed by using the index returned for
  * mp_iIndexes[species number].*/
  float *mp_fCrowdingSlope;

  /**Crowding effect steepness. D in Crowding Effect equation above.  Array
  * assumed to be sized number of species to which this behavior applies.  This
  * array is accessed by using the index returned for
  * mp_iIndexes[species number].*/
  float *mp_fCrowdingSteepness;

  /**The minimum DBH, in cm, of neighbors to be included in NCI calculations.
  * Array assumed to be sized total number of species.*/
  float *mp_fMinimumNeighborDBH;

  /**Size effect variance parameter. X<sub>b</sub> in Size Effect equation
  * above.  Array assumed to be sized number of species to which this behavior
  * applies.  This array is accessed by using the index returned for
  * mp_iIndexes[species number].*/
  float *mp_fXb;

  /**Maximum value, whatever that may be, in appropriate units.  Array assumed
  * to be sized number of species to which this behavior applies.  This array
  * is accessed by using the index returned for mp_iIndexes[species number].*/
  float *mp_fMaxPotentialValue;

  /**Size effect mode parameter. X<sub>0</sub> in Size Effect equation above.
  * Array assumed to be sized number of species to which this behavior applies.
  * This array is accessed by using the index returned for
  * mp_iIndexes[species number].*/
  float *mp_fX0;

  /**Shading coefficient in Shading Effect equation.  The user can set this to
  * 0 to turn off Shading Effect.  Array assumed to be sized number of species
  * to which this behavior applies.  This array is accessed by using the index
  * returned for mp_iIndexes[species number].*/
  float *mp_fShadingCoefficient;

  /**Shading exponent in Shading Effect equation.  Array assumed to be sized
  * number of species to which this behavior applies.  This array is accessed
  * by using the index returned for mp_iIndexes[species number].*/
  float *mp_fShadingExponent;

  /**NCI neighbor storm medium damage parameter.
  * @htmlonly &eta;<sub>k</sub> @endhtmlonly in NCI equation above.  This is
  * omitted or set to 1 if not used.  Array assumed to be sized
  * number of species to which this behavior applies.  This array is accessed
  * by using the index returned for mp_iIndexes[species number].*/
  float *mp_fMedDamageEta;

  /**NCI neighbor storm full damage parameter.
  * @htmlonly &eta;<sub>k</sub> @endhtmlonly in NCI equation above.  This is
  * omitted or set to 1 if not used.  Array assumed to be sized
  * number of species to which this behavior applies.  This array is accessed
  * by using the index returned for mp_iIndexes[species number].*/
  float *mp_fFullDamageEta;

  /**Size sensitivity to NCI parameter. @htmlonly &gamma; @endhtmlonly in
  * Crowding Effect equation above.  Array assumed to be sized number of
  * species to which this behavior applies.  This array is accessed by using
  * the index returned for mp_iIndexes[species number].*/
  float *mp_fGamma;

  /**Damage Effect parameter for target trees with medium damage.  This is
  * omitted or set to 1 if not used.  Array assumed to be sized number of
  * species to which this behavior applies.  This array is accessed by using
  * the index returned for mp_iIndexes[species number].*/
  float *mp_fMedDamageStormEff;

  /**Damage Effect parameter for target trees with full damage.  This is
  * omitted or set to 1 if not used.  Array assumed to be sized number of
  * species to which this behavior applies.  This array is accessed by using
  * the index returned for mp_iIndexes[species number].*/
  float *mp_fFullDamageStormEff;

  /**Maximum search radius, in meters, in which to look for crowding
  * neighbors.  For calculating the Crowding Effect.  Array assumed to be sized
  * number of species to which this behavior applies.  This array is accessed
  * by using the index returned for mp_iIndexes[species number].
  */
  float *mp_fMaxCrowdingRadius;

  /**The value to divide DBH by in NCI. <i>q</i> in the NCI equation above.
  * May be set to 1.*/
  float m_fDbhDivisor;

  /**Holds return data codes for the "stm_dmg" tree data member, if used.
  * Array size is assumed to be number of total species by 2 (saplings and
  * adults).  The "stm_dmg" tree member is only used if the species uses storm
  * damage.
  */
  short int **mp_iDamageCodes;

  /**Holds return data codes for the "Light" tree data member.  Array size is
  * assumed to be number of species to which this behavior applies by 2
  * (saplings and adults).  The "Light" tree member is only used if the species
  * uses the Shading Effect (i.e. mp_fShadingCoefficient != 0).*/
  short int **mp_iLightCodes;

  /**Speeds access to the arrays.  Array size is assumed to be number of
  * species.*/
  short int *mp_iIndexes;

  /**Minimum sapling height.  For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Keep our own copy for the destructor.  This is the total number of tree
  * species.*/
  short int m_iNumTotalSpecies;

  /**Keep our own copy for the destructor.  This is the number of tree species
  * applied to this behavior.*/
  short int m_iNumBehSpecies;

  /**Whether or not to include snags in NCI*/
  bool m_bIncludeSnags;

  /**Define a type for pointers to functions of the ShadingEffect type*/
  typedef float (clNCIBase::*Ptr2ShadingEffect)(clTree *);

  /**Function pointer array for the appropriate function for calculating
  * Shading Effect.  Array size is number of species to which this behavior
  * applies.*/
  Ptr2ShadingEffect* mp_ShadingEffect;

  /**Define a type for pointers to functions of the CalculateNCI type*/
  typedef float (clNCIBase::*Ptr2CalculateNCI)(clTree *, clTreePopulation *, clPlot *);

  /**Function pointer array for the appropriate function for calculating NCI.
  * Array size is number of species to which this behavior applies.*/
  Ptr2CalculateNCI* mp_NCI;

  /**Define a type for pointers to functions of the CalculateCrowding type*/
  typedef float (clNCIBase::*Ptr2CrowdingEffect)(const float&, const float&, const int&);

  /**Function pointer array for the appropriate function for calculating
  * Crowding Effect.  Array size is number of species to which this behavior
  * applies.*/
  Ptr2CrowdingEffect* mp_CrowdingEffect;


  /**Function pointer to the appropriate damage effect calculator.  Only one
  * is needed, because there cannot be variability between species.*/
  float (clNCIBase::*DamageEffect)(clTree *);

  /**
  * Sets the function pointers for <i>Crowding Effect</i>, <i>Shading
  * Effect</i>, and <i>NCI</i>.  This function analyzes the parameters to
  * discern the user's intentions, then for each species selects the simplest
  * function form that provides what they need.
  *
  * For Crowding Effect:
  * <ul>
  * <li>If D = 1 and @htmlonly &gamma; @endhtmlonly = 0, CrowdingEffect =
  * CalculateCrowdingEffectSimple().</li>
  * <li>If D = 1 but @htmlonly &gamma; @endhtmlonly != 0, CrowdingEffect =
  * CalculateCrowdingEffectNoExp().</li>
  * <li>If @htmlonly &gamma; @endhtmlonly = 0 but D != 1, CrowdingEffect =
  * CalculateCrowdingEffectNoDbh().</li>
  * <li>If D != 1 and @htmlonly &gamma; @endhtmlonly != 0, CrowdingEffect =
  * CalculateCrowdingEffect().</li>
  * <li>If C = 0 or radius = 0, CrowdingEffect =
  * CalculateNoCrowdingEffect().</li>
  * </ul>
  *
  * For Shading Effect:
  * <ul>
  * <li>If m = 0, ShadingEffect = CalculateNoShadingEffect().</li>
  * <li>If m != 0 and n = 1, ShadingEffect = CalculateShadingEffectNoExp().</li>
  * <li>If m != 0 and n != 1, ShadingEffect = CalculateShadingEffect().</li>
  * </ul>
  *
  * For NCI:
  * Storm damage is used if any storm damage value is not 1.  Otherwise, it is
  * not used.
  * <ul>
  * <li>If storm damage is used and the DBH divisor != 1, NCI =
  * CalculateNCI().</li>
  * <li>If storm damage is used and the DBH divisor = 1, NCI =
  * CalculateNCINoDivisor().</li>
  * <li>If storm damage is not used and the DBH divisor != 1, NCI =
  * CalculateNCINoEta().</li>
  * <li>If storm damage is not used and the DBH divisor = 1, NCI =
  * CalculateNCINoEtaNoDivisor().</li>
  *
  * For Storm Effect:
  * If any storm damage value is not 1, CalculateStormEffect() is used.
  * Otherwise, it is CalculateNoStormEffect().
  * </ul>
  *
  * All parameter values should be filled before calling this function.  So,
  * by extension, DeclareArrays() should also have been called.
  */
  void SetFunctionPointers();

  /**
  * Calculates the Shading Effect portion of growth.  This version calculates
  * the full equation with all terms, viz:
  * <center>SE = exp(-m * S<sup>n</sup>)</center>
  * where:
  * <ul>
  * <li>SE = Shading Effect</li>
  * <li>m = shading coefficient</li>
  * <li>S = amount of shade cast by neighbors, as placed in the "Light" data
  * member</li>
  * <li>n = shading exponent</li>
  * </ul>
  *
  * @param p_oTree Tree for which to calculate the shading effect.
  * @return Shading Effect, not guaranteed to be between 0 and 1.
  */
  float CalculateShadingEffect(clTree *p_oTree);

  /**
  * Negates the Shading Effect.  Use this version if the user has indicated
  * that the Shading Effect is not to be used - i.e. m = 0.
  * @param p_oTree Tree for which to calculate the shading effect.
  * @return 1, to remove Shading Effect from growth.
  */
  float CalculateNoShadingEffect(clTree *p_oTree) {return 1.0;};

  /**
  * Calculates the Shading Effect portion of growth.  This version is
  * used when the user has set n = 1, allowing us to skip a calculation.  This
  * version returns:
  * <center>SE = exp(-m * S)</center>
  * where:
  * <ul>
  * <li>SE = Shading Effect</li>
  * <li>m = shading coefficient</li>
  * <li>S = amount of shade cast by neighbors, as placed in the "Light" data
  * member</li>
  * </ul>
  *
  * @param p_oTree Tree for which to calculate the shading effect.
  * @return Shading Effect, not guaranteed to be between 0 and 1.
  */
  float CalculateShadingEffectNoExp(clTree *p_oTree);

  /**
  * Calculates the Crowding Effect portion of growth.  This uses the full
  * equation:
  * @htmlonly
    <center>CE = exp(-C * DBH<sup>&gamma;</sup> * NCI<sup>D</sup>)</center>
    @endhtmlonly
  * where:
  * <ul>
  * <li>CE = crowding effect</li>
  * <li>C is the NCI slope parameter</li>
  * <li>D is the NCI steepness parameter</li>
  * <li>DBH is of the target tree, in meters</li>
  * <li>@htmlonly &gamma; @endhtmlonly is the size sensitivity to NCI
  * parameter</li>
  * <li>NCI is this tree’s NCI value</li>
  * </ul>
  *
  * @param fDbh DBH of the tree for which to calculate the Crowding Effect.
  * @param fNCI Tree's NCI.
  * @param iSpecies Tree's species.
  * @return Crowding Effect, not guaranteed to be between 0 and 1.
  */
  float CalculateCrowdingEffect(const float &fDbh, const float &fNCI, const int &iSpecies);

  /**
  * Calculates the Crowding Effect portion of growth.  This form is used if the
  * user sets D = 1, which cancels out a power term:
  * @htmlonly
    <center>CE = exp(-C * DBH<sup>&gamma;</sup> * NCI)</center>
    @endhtmlonly
  * where:
  * <ul>
  * <li>CE = crowding effect</li>
  * <li>C is the NCI slope parameter</li>
  * <li>DBH is of the target tree, in meters</li>
  * <li>@htmlonly &gamma; @endhtmlonly is the size sensitivity to NCI
  * parameter</li>
  * <li>NCI is this tree’s NCI value</li>
  * </ul>
  *
  * @param fDbh DBH of the tree for which to calculate the Crowding Effect.
  * @param fNCI Tree's NCI
  * @param iSpecies Tree's species.
  * @return Crowding Effect, not guaranteed to be between 0 and 1.
  */
  float CalculateCrowdingEffectNoExp(const float &fDbh, const float &fNCI, const int &iSpecies);

  /**
  * Calculates the Crowding Effect portion of growth.  This form is used if the
  * user sets @htmlonly &gamma; @endhtmlonly in the Crowding Effect equation
  * to 0, eliminating the DBH term:
  * <center>CE = exp(-C * NCI<sup>D</sup>)</center>
  * where:
  * <ul>
  * <li>CE = crowding effect</li>
  * <li>C is the NCI slope parameter</li>
  * <li>D is the NCI steepness parameter</li>
  * <li>NCI is this tree’s NCI value</li>
  * </ul>
  *
  * @param fDbh DBH of the tree for which to calculate the Crowding Effect.
  * @param fNCI Tree's NCI.
  * @param iSpecies Tree's species.
  * @return Crowding Effect, not guaranteed to be between 0 and 1.
  */
  float CalculateCrowdingEffectNoDbh(const float &fDbh, const float &fNCI, const int &iSpecies);

  /**
  * Calculates the Crowding Effect portion of growth.  This form is used if the
  * user wants the simplest crowding effect.  In this, the user sets
  * @htmlonly &gamma; @endhtmlonly in the Crowding Effect equation to 0,
  * eliminating the DBH term, and d = 1, eliminating the exponent:
  * <center>CE = exp(-C * NCI)</center>
  * where:
  * <ul>
  * <li>CE = crowding effect</li>
  * <li>C is the NCI slope parameter</li>
  * <li>NCI is this tree’s NCI value</li>
  * </ul>
  *
  * @param fDbh DBH of the tree for which to calculate the Crowding Effect.
  * @param fNCI Tree's NCI.
  * @param iSpecies Tree's species.
  * @return Crowding Effect, between 0 and 1.
  */
  float CalculateCrowdingEffectSimple(const float &fDbh, const float &fNCI, const int &iSpecies);

  /**
   * Negates the Crowding Effect.  Use this version if the user has indicated
   * that the Shading Effect is not to be used - i.e. C = 0 or radius = 0.
   * @param fDbh DBH of the tree for which to calculate the Crowding Effect.
   * @param fNCI Tree's NCI.
   * @param iSpecies Tree's species.
   * @return 1, to remove Crowding Effect from growth.
   *
   */
  float CalculateNoCrowdingEffect(const float &fDbh, const float &fNCI, const int &iSpecies) {return 1.0;};

  /**
  * Calculates the NCI value for a tree.  This is the full form:
  @htmlonly
  <center>NCI<sub>i</sub> = &Sigma; &eta;<sub>k</sub>&lambda;<sub>k</sub>((DBH<sub>k</sub>/q)<sup>&alpha;</sup>/distance<sup>&beta;</sup>)</center>
  @endhtmlonly
  * This makes allowances for the possibility that not all species and types
  * will necessarily have storm damage applied to them.
  * @param p_oTree Tree for which to calculate NCI.
  * @param p_oPop Tree population object.
  * @param p_oPlot Plot object.
  * @return NCI value.
  */
  float CalculateNCI(clTree *p_oTree, clTreePopulation *p_oPop, clPlot *p_oPlot);

  /**
  * Calculates the NCI value for a tree.  This form is used when the user is
  * not using storm damage but is using the DBH divisor term:
  * @htmlonly
  <center>NCI<sub>i</sub> = &Sigma; &lambda;<sub>k</sub>((DBH<sub>k</sub>/q)<sup>&alpha;</sup>/distance<sup>&beta;</sup>)</center>
  @endhtmlonly
  * @param p_oTree Tree for which to calculate NCI.
  * @param p_oPop Tree population object.
  * @param p_oPlot Plot object.
  * @return NCI value.
  */
  float CalculateNCINoEta(clTree *p_oTree, clTreePopulation *p_oPop, clPlot *p_oPlot);

  /**
  * Calculates the NCI value for a tree.  This form is used when the user is
  * using storm damage but has set the DBH divisor term to 1:
  * @htmlonly
  <center>NCI<sub>i</sub> = &Sigma; &eta;<sub>k</sub>&lambda;<sub>k</sub>(DBH<sub>k</sub><sup>&alpha;</sup>/distance<sup>&beta;</sup>)</center>
  @endhtmlonly
  * This makes allowances for the possibility that not all species and types
  * will necessarily have storm damage applied to them.
  * @param p_oTree Tree for which to calculate NCI.
  * @param p_oPop Tree population object.
  * @param p_oPlot Plot object.
  * @return NCI value.
  */
  float CalculateNCINoDivisor(clTree *p_oTree, clTreePopulation *p_oPop, clPlot *p_oPlot);

  /**
  * Calculates the NCI value for a tree.  This form is used when the user is
  * not using either storm damage or the DBH divisor:
  * @htmlonly
  <center>NCI<sub>i</sub> = &Sigma; &lambda;<sub>k</sub>(DBH<sub>k</sub><sup>&alpha;</sup>/distance<sup>&beta;</sup>)</center>
  @endhtmlonly
  * @param p_oTree Tree for which to calculate NCI.
  * @param p_oPop Tree population object.
  * @param p_oPlot Plot object.
  * @return NCI value.
  */
  float CalculateNCINoEtaNoDivisor(clTree *p_oTree, clTreePopulation *p_oPop, clPlot *p_oPlot);

  /**
  * Returns 1 when there is no storm effect.
  *
  * @param p_oTree Tree for which to calculate the storm effect.
  * @return 1.
  */
  float CalculateNoDamageEffect(clTree *p_oTree) {return 1.0;};

  /**
  * Calculates the storm effect for a tree when storm effect is used.  This
  * retrieves the tree's damage and determines whether the tree is undamaged
  * (damage value of 0), medium damaged (1000 < value < 2000), or fully
  * damaged (value > 2000).  It then retrieves the appropriate value from
  * the appropriate array and returns it.
  * @param p_oTree Tree for which to calculate the storm effect.
  * @return The storm effect, between 0 and 1
  */
  float CalculateDamageEffect(clTree *p_oTree);
};
//---------------------------------------------------------------------------
#endif // NCIBase_H
