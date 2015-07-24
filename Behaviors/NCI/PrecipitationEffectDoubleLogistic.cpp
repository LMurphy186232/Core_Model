#include "PrecipitationEffectDoubleLogistic.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clPrecipitationEffectDoubleLogistic::clPrecipitationEffectDoubleLogistic() {
  mp_fAl = NULL;
  mp_fBl = NULL;
  mp_fCl = NULL;
  mp_fAh = NULL;
  mp_fBh = NULL;
  mp_fCh = NULL;
  m_precip = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clPrecipitationEffectDoubleLogistic::~clPrecipitationEffectDoubleLogistic() {
  delete[] mp_fAl;
  delete[] mp_fBl;
  delete[] mp_fCl;
  delete[] mp_fAh;
  delete[] mp_fBh;
  delete[] mp_fCh;
}

////////////////////////////////////////////////////////////////////////////
// CalculatePrecipitationEffect
////////////////////////////////////////////////////////////////////////////
float clPrecipitationEffectDoubleLogistic::CalculatePrecipitationEffect(clPlot *p_oPlot, int iSpecies) {
  float fEffect,
        fPpt = ((p_oPlot)->*(this->m_precip))();
  fEffect = (mp_fAl[iSpecies] + ((1-mp_fAl[iSpecies])/(1+pow(mp_fBl[iSpecies]/fPpt, mp_fCl[iSpecies])))) *
            (mp_fAh[iSpecies] + ((1-mp_fAh[iSpecies])/(1+pow(fPpt/mp_fBh[iSpecies], mp_fCh[iSpecies]))));
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clPrecipitationEffectDoubleLogistic::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  floatVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i, iPrecipType;

  mp_fAl = new float[iNumTotalSpecies];
  mp_fBl = new float[iNumTotalSpecies];
  mp_fCl = new float[iNumTotalSpecies];
  mp_fAh = new float[iNumTotalSpecies];
  mp_fBh = new float[iNumTotalSpecies];
  mp_fCh = new float[iNumTotalSpecies];

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Precipitation effect al
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogPrecipEffAl",
      "ndlpealVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fAl[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Precipitation effect bl
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogPrecipEffBl",
      "ndlpeblVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fBl[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Precipitation effect cl
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogPrecipEffCl",
      "ndlpeclVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCl[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Precipitation effect ah
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogPrecipEffAh",
      "ndlpeahVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fAh[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Precipitation effect bh
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogPrecipEffBh",
      "ndlpebhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fBh[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Precipitation effect ch
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogPrecipEffCh",
      "ndlpechVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCh[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;

  //Precip value desired
  iPrecipType = mean_precip;
  FillSingleValue(p_oElement, "nciDoubleLogisticPrecipType", &iPrecipType, true);
  if (iPrecipType == mean_precip) {
    m_precip = &clPlot::GetMeanAnnualPrecip;
  } else if (iPrecipType == seasonal_precip) {
    m_precip = &clPlot::GetSeasonalPrecipitation;
  } else if (iPrecipType == water_deficit) {
    m_precip = &clPlot::GetWaterDeficit;
  } else {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clPrecipitationEffectDoubleLogistic::DoSetup";
    stcErr.sMoreInfo = "Unrecognized precipitation type value.";
    throw( stcErr );
  }
}
