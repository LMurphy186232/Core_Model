/*
 * DefaultSizeEffect.cpp
 *
 *  Created on: Nov 27, 2012
 *      Author: LORA
 */

#include "DefaultSizeEffect.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clDefaultSizeEffect::clDefaultSizeEffect() {
  mp_fXb = NULL;
  mp_fX0 = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clDefaultSizeEffect::~clDefaultSizeEffect() {
  delete[] mp_fXb;
  delete[] mp_fX0;
}

////////////////////////////////////////////////////////////////////////////
// CalculateSizeEffect
////////////////////////////////////////////////////////////////////////////
float clDefaultSizeEffect::CalculateSizeEffect(int iSpecies, float fDiam) {
  float fSizeEffect;
  fSizeEffect = exp(-0.5 * pow(log(fDiam / mp_fX0[iSpecies] ) / mp_fXb[iSpecies], 2));
  //Make sure it's bounded between 0 and 1
  if ( fSizeEffect < 0 ) fSizeEffect = 0;
  if ( fSizeEffect > 1 ) fSizeEffect = 1;
  return fSizeEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clDefaultSizeEffect::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_dTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fX0 = new double[iNumTotalSpecies];
  mp_fXb = new double[iNumTotalSpecies];

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_dTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_dTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Size effect mode (X0)
  FillSpeciesSpecificValue( p_oElement, "gr_nciSizeEffectMode", "gr_nsemVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fX0[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect variance (Xb)
  FillSpeciesSpecificValue(p_oElement, "gr_nciSizeEffectVariance", "gr_nsevVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++)
    mp_fXb[p_dTempValues[i].code] = p_dTempValues[i].val;

  delete[] p_dTempValues;

  //Make sure that the size effect mode is not 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (0 >= mp_fX0[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDefaultSizeEffect::DoSetup";
      stcErr.sMoreInfo = "NCI size effect mode (X0) values must be greater than 0.";
      throw( stcErr );
    }

    //Make sure that the size effect variance is not 0
    if (0 == mp_fXb[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDefaultSizeEffect::DoSetup";
      stcErr.sMoreInfo = "NCI size effect variance (Xb) values cannot be 0.";
      throw( stcErr );
    }
  }
}
