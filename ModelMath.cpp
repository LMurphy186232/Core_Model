//---------------------------------------------------------------------------
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <string.h>
#include "ModelMath.h"
#include "Messages.h"

gsl_rng *clModelMath::randgen = gsl_rng_alloc(gsl_rng_mt19937);

/////////////////////////////////////////////////////////////////////////////
// GetRand()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::GetRand() {
  return (gsl_rng_uniform(randgen));
}

/////////////////////////////////////////////////////////////////////////////
// CalcPointValue()
//////////////////////////////////////////////////////////////////////////////
float clModelMath::CalcPointValue(float fX, float fSlope, float fIntercept) {
  return ( fSlope * fX + fIntercept );
}

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CalculateBasalArea()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::CalculateBasalArea(float fDbh) {
  float SQ_M_PER_SQ_CM = 0.0001; //conversion factor for sq cm to sq m

  if ( fDbh <= 0 )
  { //throw error
    modelErr stcErr;
    stcErr.sFunction = "clModelMath::CalculateBasalArea" ;
    stcErr.sMoreInfo = "DBH is not a positive number.";
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }

  return ( pow( fDbh * 0.5, 2 ) * M_PI * SQ_M_PER_SQ_CM );
}

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// Round()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::Round(float fNumber, int iNumDigits) {
  double fIntegerPart, //we'll split a float number into integer and
  //fractional parts - this is the integer part
  fBigNumber; //for calculations

  //Take the number to round and multiply by 10^ number of digits +1
  fBigNumber = fNumber * pow( 10.0, ( float )( iNumDigits + 1 ) );
  //Adjust the number depending on whether or not the original number was neg
  if ( fNumber < 0.0 )
    fBigNumber -= 5.0;
  else
    fBigNumber += 5.0;

  fBigNumber /= 10.0;
  //Split off the integer part - discard fractional part
  modf( fBigNumber, & fIntegerPart );

  //Divide by 10 ^ num digits to get our number back
  return fIntegerPart / pow( 10.0, ( float )iNumDigits );
}

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// RandomRound()
/////////////////////////////////////////////////////////////////////////////
int clModelMath::RandomRound(float fNumber) {
  double fIntegerPart = 0, //we'll split a float number into integer and
  //fractional parts - this is the integer part
  fFractionalPart = 0; //fractional part

  //Split the number into integer and fractional part
  fFractionalPart = modf( fNumber, & fIntegerPart );

  return ( GetRand() <= fFractionalPart ? ( int )( fIntegerPart + 1 ) : ( int )fIntegerPart );
}

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CalculateWeibullFunction()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::CalculateWeibullFunction(float fDispersal, float fTheta,
    float fDistance) {
  //prevent underflow
  float fTemp = -fDispersal * pow( fDistance , fTheta );
  if (fTemp < -50) return 0;
  else return exp(fTemp);
}

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CalculateLognormalFunction()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::CalculateLognormalFunction(float fX0, float fXb,
    float fDistance) {
  //Error if either fX0 or fXb is zero
  if ( 0 == fXb )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clModelMath::CalculateLognormalFunction" ;
    stcErr.sMoreInfo = "Xb cannot be zero.";
    throw( stcErr );
  }
  if (0 >= fX0 || 0 > fDistance) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clModelMath::CalculateLognormalFunction" ;
    stcErr.sMoreInfo = "X0 and distance must be greater than zero.";
    throw( stcErr );
  }
  if (fDistance == 0) return 0.0;

  //prevent underflow
  float fTemp = -0.5 * pow( ( log( ( fDistance ) / fX0 ) / fXb ), 2 );
  if (fTemp < -50) return 0;
  else return exp(fTemp);
}

/////////////////////////////////////////////////////////////////////////////
// PoissonRandomDraw()
/////////////////////////////////////////////////////////////////////////////
int clModelMath::PoissonRandomDraw(float fLambda) {
  return (gsl_ran_poisson(randgen, fLambda));
}

/////////////////////////////////////////////////////////////////////////////
// LognormalRandomDraw()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::LognormalRandomDraw(float fMean, float fStdDev) {
  return (gsl_ran_lognormal(randgen, fMean, fStdDev));
}

/////////////////////////////////////////////////////////////////////////////
// NegBinomialRandomDraw()
/////////////////////////////////////////////////////////////////////////////
int clModelMath::NegBinomialRandomDraw(float fMean, float fClumping) {
  double fPDF, fCDF;
  float fRand = GetRand();
  int iCount = 0;
  fPDF = pow((fClumping/(fClumping+fMean)), fClumping); //term for PDF for zero counts
  fCDF = fPDF;
  while (fRand > fCDF) {
    iCount++;
    //Uses recursive eqn. from Hilborn and Mangel pg 87
    fPDF = ((iCount+fClumping-1)/iCount)*(fMean/(fClumping+fMean))*fPDF;
    fCDF = fCDF + fPDF;
  }

  return iCount;
}

/////////////////////////////////////////////////////////////////////////////
// NormalRandomDraw()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::NormalRandomDraw(float fStdDev) {
  return gsl_ran_gaussian(randgen, fStdDev);
}

/////////////////////////////////////////////////////////////////////////////
// AddBarkToDBH()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::AddBarkToDBH(float fDIB, float fA, float fB, float fC) {
  if (fDIB <= 0) {
    modelErr stcErr;
    stcErr.sFunction = "clModelMath::AddBarkToDBH";
    stcErr.sMoreInfo = "DBH is not a positive number.";
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }
  if (fDIB >= 1E18) {
    modelErr stcErr;
    stcErr.sFunction = "clModelMath::AddBarkToDBH";
    stcErr.sMoreInfo = "DBH is too large.";
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }

  return (fA + (fB * fDIB) + (fC * (fDIB * fDIB)));
}

/////////////////////////////////////////////////////////////////////////////
// InverseGaussianRandomDraw()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::InverseGaussianRandomDraw(float fMu, float fLambda) {
  float n, v, w, c, x, p;

  n = gsl_ran_gaussian(randgen, 1);
  v = n*n;
  w = fMu*v;
  c = fMu/(2.0*fLambda);

  x = fMu + c*(w - sqrt(w*(4.0*fLambda + w)));
  p = fMu/(fMu + x);

  if (p > gsl_rng_uniform(randgen))
    return (x);
  else
    return (fMu*fMu/x);
}

/////////////////////////////////////////////////////////////////////////////
// GammaRandomDraw()
/////////////////////////////////////////////////////////////////////////////
float clModelMath::GammaRandomDraw(float fMean, float fScale) {
  return gsl_ran_gamma(randgen, fMean / fScale, fScale);
}
