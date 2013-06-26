#include "WeibullTemperatureEffect.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clWeibullTemperatureEffect::clWeibullTemperatureEffect() {
  mp_fTempA = NULL;
  mp_fTempB = NULL;
  mp_fTempC = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clWeibullTemperatureEffect::~clWeibullTemperatureEffect() {
  delete[] mp_fTempA;
  delete[] mp_fTempB;
  delete[] mp_fTempC;
}

////////////////////////////////////////////////////////////////////////////
// CalculateTemperatureEffect
////////////////////////////////////////////////////////////////////////////
float clWeibullTemperatureEffect::CalculateTemperatureEffect(clPlot *p_oPlot, int iSpecies) {
  float fEffect,
        fTemperature = p_oPlot->GetMeanAnnualTemp();
  fEffect = exp(-0.5*pow(fabs(fTemperature - mp_fTempC[iSpecies])/mp_fTempA[iSpecies], mp_fTempB[iSpecies]));
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clWeibullTemperatureEffect::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  floatVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fTempA = new float[iNumTotalSpecies];
  mp_fTempB = new float[iNumTotalSpecies];
  mp_fTempC = new float[iNumTotalSpecies];

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Temperature effect a
  FillSpeciesSpecificValue( p_oElement, "gr_nciWeibTempEffA",
      "gr_nwteaVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fTempA[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Temperature effect b
  FillSpeciesSpecificValue( p_oElement, "gr_nciWeibTempEffB",
      "gr_nwtebVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fTempB[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Temperature effect c
  FillSpeciesSpecificValue( p_oElement, "gr_nciWeibTempEffC",
      "gr_nwtecVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fTempC[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;

  //Make sure that the a value is not 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (0 == mp_fTempA[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clWeibullTemperatureEffect::DoSetup";
      stcErr.sMoreInfo = "NCI temperature effect \"a\" values cannot be 0.";
      throw( stcErr );
    }
  }
}
