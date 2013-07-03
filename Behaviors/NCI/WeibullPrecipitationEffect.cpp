#include "WeibullPrecipitationEffect.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clWeibullPrecipitationEffect::clWeibullPrecipitationEffect() {
  mp_fPrecipA = NULL;
  mp_fPrecipB = NULL;
  mp_fPrecipC = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clWeibullPrecipitationEffect::~clWeibullPrecipitationEffect() {
  delete[] mp_fPrecipA;
  delete[] mp_fPrecipB;
  delete[] mp_fPrecipC;
}

////////////////////////////////////////////////////////////////////////////
// CalculatePrecipitationEffect
////////////////////////////////////////////////////////////////////////////
float clWeibullPrecipitationEffect::CalculatePrecipitationEffect(clPlot *p_oPlot, int iSpecies) {
  float fEffect,
        fPrecipitation = p_oPlot->GetMeanAnnualPrecip();
  fEffect = exp(-0.5*pow(fabs(fPrecipitation - mp_fPrecipC[iSpecies])/mp_fPrecipA[iSpecies], mp_fPrecipB[iSpecies]));
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clWeibullPrecipitationEffect::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  floatVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fPrecipA = new float[iNumTotalSpecies];
  mp_fPrecipB = new float[iNumTotalSpecies];
  mp_fPrecipC = new float[iNumTotalSpecies];

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Precipitation effect a
  FillSpeciesSpecificValue( p_oElement, "nciWeibPrecipEffA",
      "nwpeaVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrecipA[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Precipitation effect b
  FillSpeciesSpecificValue( p_oElement, "nciWeibPrecipEffB",
      "nwpebVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrecipB[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Precipitation effect c
  FillSpeciesSpecificValue( p_oElement, "nciWeibPrecipEffC",
      "nwpecVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrecipC[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;

  //Make sure that the a value is not 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (0 == mp_fPrecipA[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clWeibullPrecipitationEffect::DoSetup";
      stcErr.sMoreInfo = "NCI precipitation effect \"a\" values cannot be 0.";
      throw( stcErr );
    }
  }
}
