#ifndef ModMathH
#define ModMathH
//---------------------------------------------------------------------------
/**
* SORTIE math library.
* This library provides some model-specific math functions.
*
* The random number generator is very important.  To make sure that no one can
* mess with the seed, I put the seed and the seed setting function in the
* private portion.  Then clSimManager is a friend class and is the only one
* that can change the seed.
*
* This used to be a library of C-style functions.  But it turns out that the
* random number generator doesn't work right unless the seed is a class member.
*
* I have made some wrappers for GSL, for back-compatibility from before GSL
* was used. But I have also opened up the random number generator so that
* other classes can use the GSL functions directly if they wish.
*
* Copyright 2005 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/

#include <gsl/gsl_rng.h>

class clModelMath {

  public:

  //clModelMath(); //use default constructor

  ~clModelMath() {gsl_rng_free (randgen);};

  /**
  * Returns the value of a linear function.
  * @param fX Point for which to calculate the function.
  * @param fSlope Slope of the function.
  * @param fIntercept Intercept of the function.
  * @return Function value at the given point.
  */
  static float CalcPointValue(float fX, float fSlope, float fIntercept=0.0);

  /**
  * Random number generator. This just wraps a call to GSL's uniform random
  * number generator.
  *
  * @return a random number between 0 and 1.
  */
  static float GetRand();

  /**
  * Calculates basal area.
  * @param fDbh DBH in cm for which to calculate basal area.
  * @return Basal area in square meters.
  * @throw modelErr if DBH is less than or equal to 0.
  */
  static float CalculateBasalArea(float fDbh);

  /**
  * Rounds a number to a specified number of digits.  For this function,
  * credit www.codeproject.com.
  * @param fNumber Number to round.
  * @param iNumDigits Number of digits to round the number to.  If 0, the
  * number is rounded to the nearest integer.
  * @return Rounded number.
  */
  static float Round(float fNumber, int iNumDigits);

  /**
  * Uses a random number to decide whether to round a number up or down to the
  * next integer.  The number is split into integer and fractional parts.  A
  * random number is compared to the fractional part; if it is less than or
  * equal to it, one is added to the integer part; otherwise the integer part
  * is returned as-is.
  * @param fNumber Number to round.
  * @return Rounded number.
  */
  static int RandomRound(float fNumber);

  /**
  * Calculates the exponential of the Weibull probability distribution function
  * at a particular point.
  *
  * @param fDispersal Value of dispersal variable
  * @param fTheta Value of theta variable
  * @param fDistance Distance, in meters, at which to calculate the function
  * @return Value of function
  */
  static float CalculateWeibullFunction(float fDispersal, float fTheta, float fDistance);


  /**
  * Calculates the exponential of the Lognormal probability distribution
  * function at a particular point.
  *
  * @param fX0 Value of X<sub>0</sub> variable
  * @param fXb Value of X<sub>b</sub> variable
  * @param fDistance Distance, in meters, at which to calculate the function
  * @return Value of function
  * @throw modelErr if fX0 or fDistance are not greater than 0, or if fXb is
  * zero.
  */
  static float CalculateLognormalFunction(float fX0, float fXb, float fDistance);

  /**
  * Returns a random Poisson-distributed number. This just wraps the appropriate
  * GSL function.
  *
  * @param fLambda Lambda (mean) of the Poisson function.
  * @return Integer random draw.
  */
  static int PoissonRandomDraw(float fLambda);

  /**
  * Returns a random lognormally-distributed number.  The lognormal
  * distribution has the form
  *
  * p(x) dx = 1/(x * sqrt(2 pi sigma<sup>2</sup>)) exp(-(ln(x) - zeta)<sup>2</sup>/2 sigma<sup>2</sup>) dx
  *
  * for x > 0. Lognormal random numbers are the exponentials of gaussian random
  * numbers.
  *
  * This just wraps the appropriate GSL function.
  * @param fMean Mean of the distribution, or zeta in the above formula.
  * @param fStdDev Standard deviation of the distribution, or sigma in the
  * above formula.
  * @return Random lognormally-distributed number.
  */
  static float LognormalRandomDraw(float fMean, float fStdDev);

  /**
  * Returns a random normally-distributed number with mean zero and standard
  * deviation sigma. The probability distribution for normal random variates is,
  * @htmlonly p(x) dx = {1 / sqrt{2 &pi; &sigma;<sup>2</sup>}} exp (-x<sup>2</sup> / 2&sigma;<sup>2</sup>) dx @endhtmlonly for x
  * in the range -infty to +infty. Use the transformation z = mu + x on the
  * numbers returned to obtain a normal distribution with mean mu.
  *
  * This function just wraps the appropriate GSL function.
  * @param fStdDev Standard deviation.
  * @return Random normally-distributed number.
  */
  static float NormalRandomDraw(float fStdDev);

  /**
  * Returns a random negative binomially-distributed number.  The form of the
  * distribution is from Equation 3.103 from Hilborn and Mangel
  * (The Ecological Detective).  Charlie Canham created this code.
  * @param fMean Function mean.
  * @param fClumping Clumping parameter.
  * @return Random negative-binomially-distributed number.
  */
  static int NegBinomialRandomDraw(float fMean, float fClumping);

  /**
  * Adds bark to a tree diameter.  The equation is:
  * <center><i>DBH = a + b * DIB + c * DIB<sup>2</sup></i></center>
  * where:
  * <ul>
  * <li>DIB = diameter inside bark at 1.35 meters, in cm</li>
  * <li>DBH = diameter outside bark at 1.35 meters, in cm</li>
  * <li>a, b, and c are parameters</li>
  * </ul>
  *
  * @param fDIB Diameter inside bark at 1.35 meters' height, in cm
  * @param fA "a" term in equation above
  * @param fB "b" term in equation above
  * @param fC "c" term in equation above
  * @return Diameter outside the bark at 1.35 meters' height, in cm
  * @throw modelErr if DIB is less than or equal to 0, or large enough to create
  * float overflow.
  */
  static float AddBarkToDBH(float fDIB, float fA, float fB, float fC);

  /**
   * Returns a random number over the inverse Gaussian distribution. I got this
   * code from http://www.cbr.washington.edu/papers/zabel/index.html, which is
   * the dissertation "Spatial and Temporal Models of Migrating Juvenile Salmon
   * with Applications", by Richard W. Zabel, University of Washington.
   *
   * The inverse Gaussian PDF is:
   * @htmlonly
    f(x; &mu;, &lambda;) = [&lambda; /(2&pi;x<sup>3</sup>)]^(1/2) * exp[(-&lambda;(x-&mu;)<sup>2</sup> )/ (2 &mu;<sup>2</sup>x)]
    @endhtmlonly
   * @param fMu: mu.
   * @param fLambda: lambda.
   */
  static float InverseGaussianRandomDraw(float fMu, float fLambda);

  /**
   * Returns a random number drawn from the gamma distribution. This is a
   * wrapper for the GSL function gsl_ran_gamma. For this function, a = shape
   * parameter, and b = scale parameter. The shape parameter a = mean / scale.
   * @param fMean Function mean.
   * @param fScale Scale parameter.
   * @return Desired random deviate.
   */
  static float GammaRandomDraw(float fMean, float fScale);

  /**Random number generator. Currently set to the Mersenne Twister algorithm.*/
  static gsl_rng *randgen;

  /**
  * Sets the random seed.  This should always be a negative number.
  * @param iSeed Seed to set.
  */
  static void SetRandomSeed(long iSeed) {
   gsl_rng_set (randgen, iSeed);
 }
};
//---------------------------------------------------------------------------
#endif

