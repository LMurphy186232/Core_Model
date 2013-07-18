#include "SizeEffectLowerBounded.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clSizeEffectLowerBounded::clSizeEffectLowerBounded() {
  mp_fXb = NULL;
  mp_fX0 = NULL;
  mp_fMinDiam = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSizeEffectLowerBounded::~clSizeEffectLowerBounded() {
  delete[] mp_fXb;
  delete[] mp_fX0;
  delete[] mp_fMinDiam;
}

////////////////////////////////////////////////////////////////////////////
// CalculateSizeEffect
////////////////////////////////////////////////////////////////////////////
float clSizeEffectLowerBounded::CalculateSizeEffect(int iSpecies, float fDiam) {
  float fSizeEffect;

  //Make sure the diameter is above the minimum
  if (fDiam < mp_fMinDiam[iSpecies])
    fDiam = mp_fMinDiam[iSpecies];
  fSizeEffect = exp(-0.5 * pow(log(fDiam / mp_fX0[iSpecies] ) / mp_fXb[iSpecies], 2));
  //Make sure it's bounded between 0 and 1
  if ( fSizeEffect < 0 ) fSizeEffect = 0;
  if ( fSizeEffect > 1 ) fSizeEffect = 1;
  return fSizeEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clSizeEffectLowerBounded::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_dTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fX0 = new double[iNumTotalSpecies];
  mp_fXb = new double[iNumTotalSpecies];
  mp_fMinDiam = new double[iNumTotalSpecies];

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_dTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_dTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Size effect mode (X0)
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectX0", "nsex0Val",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fX0[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect variance (Xb)
  FillSpeciesSpecificValue(p_oElement, "nciSizeEffectXb", "nsexbVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++)
    mp_fXb[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect lower bound
  FillSpeciesSpecificValue(p_oElement, "nciSizeEffectLowerBound", "nselbVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++)
    mp_fMinDiam[p_dTempValues[i].code] = p_dTempValues[i].val;

  delete[] p_dTempValues;

  //Make sure that the size effect mode is not 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (0 >= mp_fX0[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSizeEffectLowerBounded::DoSetup";
      stcErr.sMoreInfo = "NCI size effect mode (X0) values must be greater than 0.";
      throw( stcErr );
    }

    //Make sure that the size effect variance is not 0
    if (0 == mp_fXb[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSizeEffectLowerBounded::DoSetup";
      stcErr.sMoreInfo = "NCI size effect variance (Xb) values cannot be 0.";
      throw( stcErr );
    }
  }
}
