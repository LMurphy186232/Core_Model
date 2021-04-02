//---------------------------------------------------------------------------

#ifndef AbsoluteGrowthH
#define AbsoluteGrowthH
//---------------------------------------------------------------------------
#include "MichMenGrowthBase.h"

/**
* Absolute growth with suppression
*
* This is a growth shell object which applies the Michaelis-Menton function to
* find absolute growth with suppression and release dynamics.
*
* The growth equation is Y = SF * (A * gli) / ((A/S) + gli), where Y is the
* log10 of new radial growth in mm/yr, SF is the suppression factor,
* A is asymptotic diameter growth, GLI is global light index, and S is the
* slope of growth response.  Growth can be limited to a radial increment or an
* area increment, or unlimited.  An object of this class can support either
* diameter-only updating or diameter updating with automatic height adjustment.
*
* Suppression is calculated as SF = e<sup>(g*YLR - d*YLS)</sup>, where g is
* the value in mp_fLengthCurrReleaseFactor, d is the value in
* mp_fLengthLastSuppFactor, YLR is the years of the last (or current) release
* period, and YLS is the years of the last (or current) suppression period.
*
* An object of this class can be created in several ways.  Use the strings
* "AbsRadialGrowth", "AbsBAGrowth", and "AbsUnlimGrowth" to create behaviors
* that calculate a diameter increase with automatic height adjustment. These
* strings represent absolute growth limited by constant radial increment,
* absolute growth limited by constant basal area increment, and unlimited
* absolute growth, respectively. Add the string " diam only" (example -
* "AbsBAGrowth diam only") to tell this behavior that only the tree's diameter,
* and not its height, should be updated.
*
* The namestring for this class is "absolutegrowthshell".  It is probable that
* there will be more than one object of this class created.
*
* This object will add two int data members to the applicable trees, "ycr"
* and "yls" (for years of current release and years of last suppression).
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/

//note: need the virtual keyword to avoid base class ambiguity.
class clAbsoluteGrowth : virtual public clMichMenBase {

  public:

  /**
  * Constructor.  Sets the namestring.
  * @param p_oSimManager Sim manager for the run.
  */
  clAbsoluteGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clAbsoluteGrowth();

  /**
  * This applies diameter growth as described in the equation above.
  * @param p_oTree Pointer to the tree to which to apply growth.
  * @param p_oPop Pointer to the tree population object.
  * @param fHeightGrowth Amount of height growth, in m (ignored).
  * @return The amount of diameter growth increase, in cm.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Captures the namestring passed to this behavior.  This is overridden from
  * clBehaviorBase so we can capture the namestring passed.  Since this class
  * can create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

  /**
  * Does the setup for this behavior.  It reads in suppression/release data and
  * checks values, and validates that all species/type combos use light (each
  * must have "Light" registered).
  * @param p_oDoc Parsed parameter file object.
  * @throws modelErr if any species/type combo to which this behavior is
  * applied does not have a light behavior.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the two float data members and captures the return codes.
  */
  void RegisterTreeDataMembers();

  protected:
  /**Length last suppression factor - array size is number of species.
  This is the old parameter D.*/
  double *mp_fLengthLastSuppFactor;

  /**Length current release factor - array size is number of species.  This
  is the old parameter G.*/
  double *mp_fLengthCurrReleaseFactor;

  /**GLI threshold below which a tree is suppressed - array size is number
  of species.*/
  double *mp_fGliThreshold;

  /**Mortality rate of suppressed trees.*/
  double m_fMortRateAtSuppression;

  /**Number of years per timestep - from sim manager*/
  int m_iNumberYearsPerTimestep;

  /**Years exceeding threshold before a tree becomes suppressed.*/
  int m_iYrsExceedThresholdBeforeSup;

  /**Maximum number of years we'll track a tree's suppression and release
  data.*/
  int m_iMaxYears;

  /**Holds the return codes for the new float tree data members for a particular
  tree type.*/
  struct stcCodes {
    short int *p_iCodes,  /**<Data member code for each species.*/
              iType; /**<Type to which the species data member array applies.*/
  } *mp_ylrCodes, /**<Return codes for "ylr" tree data member.  Array size is number of types.*/
    *mp_ylsCodes; /**<Return codes for "yls" tree data member.  Array size is number of types.*/

  short int m_iNumBehaviorTypes; /**<Number of types managed by this behavior.*/

  /**
  * Calculates the suppression threshold for each species.  This will have to
  * read mortality parameters to do this.
  * @param p_oDoc Parsed DOM tree of parameter file.
  * @param p_oPop Tree population object.
  */
  void CalculateSuppressionThresholds(xercesc::DOMDocument *p_oDoc,
                                            clTreePopulation *p_oPop);

  /**
  * Calculates a tree's suppression factor.  The suppression factor
  * is a number greater than 0 that is multiplied times growth to modify it.  A
  * factor of 1 returned indicates no suppression.  This will also manage the
  * yls and ycr variables to keep track of length of time of suppression and
  * release.
  *
  * If the years exceeding threshold before suppression variable is greater
  * than the number of years per timestep then the suppression factor is
  * always 1.
  *
  * @param p_oTree The tree for which suppression is being calculated
  * @param fGli The tree's gli value
  * @return Tree's suppression factor.
  */
  float CalculateSuppressionFactor(clTree *p_oTree, const float &fGli);
};
//---------------------------------------------------------------------------
#endif
