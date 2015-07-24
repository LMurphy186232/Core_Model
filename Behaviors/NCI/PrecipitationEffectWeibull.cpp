#include "PrecipitationEffectWeibull.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clPrecipitationEffectWeibull::clPrecipitationEffectWeibull() {
  mp_fPrecipA = NULL;
  mp_fPrecipB = NULL;
  mp_fPrecipC = NULL;
  m_precip = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clPrecipitationEffectWeibull::~clPrecipitationEffectWeibull() {
  delete[] mp_fPrecipA;
  delete[] mp_fPrecipB;
  delete[] mp_fPrecipC;
}

////////////////////////////////////////////////////////////////////////////
// CalculatePrecipitationEffect
////////////////////////////////////////////////////////////////////////////
float clPrecipitationEffectWeibull::CalculatePrecipitationEffect(clPlot *p_oPlot, int iSpecies) {
  float fEffect,
        fPrecipitation = ((p_oPlot)->*(this->m_precip))();
  fEffect = exp(-0.5*pow(fabs(fPrecipitation - mp_fPrecipC[iSpecies])/mp_fPrecipA[iSpecies], mp_fPrecipB[iSpecies]));
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clPrecipitationEffectWeibull::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  floatVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i, iPrecipType;

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

  //Precip value desired. For backwards compatibility, don't require this
  //and set it to mean annual precip
  iPrecipType = mean_precip;
  FillSingleValue(p_oElement, "nciWeibPrecipType", &iPrecipType, false);
  if (iPrecipType == mean_precip) {
    m_precip = &clPlot::GetMeanAnnualPrecip;
  } else if (iPrecipType == seasonal_precip) {
    m_precip = &clPlot::GetSeasonalPrecipitation;
  } else if (iPrecipType == water_deficit) {
    m_precip = &clPlot::GetWaterDeficit;
  } else {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clPrecipitationEffectWeibull::DoSetup";
    stcErr.sMoreInfo = "Unrecognized precipitation type value.";
    throw( stcErr );
  }

  delete[] p_fTempValues;

  //Make sure that the a value is not 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (0 == mp_fPrecipA[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPrecipitationEffectWeibull::DoSetup";
      stcErr.sMoreInfo = "NCI precipitation effect \"a\" values cannot be 0.";
      throw( stcErr );
    }
  }
}
