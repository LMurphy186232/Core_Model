//---------------------------------------------------------------------------
// WeibullSnagMort
//---------------------------------------------------------------------------
#if !defined(WeibullSnagMort_H)
  #define WeibullSnagMort_H

#include "MortalityBase.h"
/**
* Snag mortality using Weibull function, V 1.1.
*
* Snags are standing dead trees, and as such "mortality" is something of a
* misnomer.  However, since this behavior fits in with the other mortality
* behaviors, we will continue to use that naming scheme.
*
* Snag death really means that a snag falls over.  The probability that this
* will happen to any one snag is a function of its age as a snag.  The
* percentage standing of a snag population is:
* <center>Percent standing = 100 * e<sup>(-(a*Time<sup>b</sup>))</sup></center>
* where Time is in years.
*
* The rate at which snags fall over changes with snag size.  This behavior
* allows the definition of three snag size classes.  Each size class can take
* a different set of a and b values.
*
* This behavior can only be applied to snags.
*
* The namestring for this behavior is "weibsnagmortshell".  The parameter file
* call string is "WeibullSnagMortality".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clWeibullSnagMort : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring and nulls all pointers.
  */
  clWeibullSnagMort(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
 ~clWeibullSnagMort();

  /**
  * Performs specific setup for this behavior.
  * <ol>
  * <li>Calls ReadParameterFile() in order to get parameter file data.
  * <li>Calls CalculateDeathProbabilities().
  * <li>Calls GetAgeCodes().
  * </ol>
  *
  * @param p_oDoc Parsed DOM tree of parameter file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates whether a snag lives or dies.  If the snag age in timesteps is
  * less than m_iMaxPrecalcAge, then a random number is compared to the
  * appropriate value in mp_fProbabilityOfDeath.  If it is greater than that
  * age, then its probability of death is calculated directly from the Weibull
  * function.  In each case, if the random number is less than or equal to the
  * probability of death, then the snag dies.
  *
  * @param fDbh DBH of tree being evaluated
  * @param p_oTree Tree being evaluated
  * @param iSpecies Species of the tree being evaluated
  * @return @return natural if the tree is to "die", notdead if it "lives".
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  float ***mp_fProbabilityOfDeath; /**<Pre-calculated probabilities of death.
  Array is m_iNumSizeClasses by number of behavior species by
  m_iMaxPrecalcAge.*/
  float **mp_fAParameter; /**<Parameter "a" in Weibull function.  Array size
  is m_iNumSizeClasses by number of behavior species.*/
  float **mp_fBParameter; /**<Parameter "b" in Weibull function.  Array size
  is m_iNumSizeClasses by number of behavior species.*/
  float **mp_fSnagSizeClasses; /**<Upper limit of DBH in each snag size class.
  This array is sized m_iNumSizeClasses by number of behavior species.  The
  value in the third size class bucket for each species is ignored since it is
  always effectively infinity.*/
  float m_fNumYearsPerTimestep; /**<Number of years per timestep*/
  short int *mp_iAgeCodes; /**<Data member codes for "age" snag member for
                             each behavior species*/
  short int *mp_iIndexes; /**<For accessing arrays.  One per species.*/


  short int m_iMaxPrecalcAge; /**<Max age, in timesteps, that the probability
                               * of death is pre-calculated.*/
  short int m_iNumSizeClasses; /**<Number of snag size classes for which
                                separate parameters can be defined.*/

 /**
  * Reads required data from the parameter file.
  *
  * @param p_oDoc Parsed DOM tree of parameter file.
  * @throws modelErr if:
  * <ul>
  * <li>any of the parameters are missing</li>
  * <li>the size classes overlap</li>
  * <li>any value for "a" is zero (this will cause a math error if true)</li>
  * <li>any value for "a" is negative, if "b" is not a whole number (this will
  * cause a math error if true)</li>
  * </ul>
  *
  * Because of the need for backward compatibility with version 1.0, this will
  * accept the size class definitions as single values if they are written
  * this way.
  */
  void ReadParameterFile(xercesc::DOMDocument *p_oDoc);

  /**
  * Declares and populates the mp_fProbabilityOfDeath array.  For each timestep
  * t, it calculates the probability of mortality between t and t-1 using
  * CalculateDeathProbability().
  */
  void FillDeathProbArray();

  /**
  * Calculates a probability of death for snags of a certain age.  The
  * probability is calculated as since the last timestep.  If F<sub>t</sub> is
  * the value of the function above at timestep t, then the probability
  * P<sub>t</sub> of mortality between timesteps t-1 and t is:
  * <center>P<sub>t</sub> = (1-(1-F<sub>t</sub>)/(1-F<sub>t-1</sub>))</center>
  * Any probability value greater than 1 - 1.0E-5 is returned as one, and less
  * than 1.0E-05 is returned as 0.  This function will handle conversions of
  * ages from years to timesteps.
  *
  * If the age is within the first timestep, then the probability is the value
  * of the function at time t.
  * @param iSizeClass Size class number, as 0, 1, or 2.  THIS NUMBER IS NOT
  * CHECKED FOR VALIDITY.
  * @param iSpecies Species number.  NOT species index, just plain old species.
  * @param iAge Age in years.
  * @return Probability of death, between 0 and 1 inclusive.
  */
  float CalculateDeathProbability(int iSizeClass, int iSpecies, int iAge);

  /**
  * Gets the data member return codes for the "age" snag data member.  Return
  * codes are captured in the mp_iAgeCodes array.
  */
  void GetAgeCodes();

  /**
  * Makes sure that this behavior has only been applied to snags.
  * @throws modelErr if any tree type besides snags has been applied to this
  * behavior.
  */
  void ValidateTypes();
};

//---------------------------------------------------------------------------
#endif // WeibullSnagMort_H
