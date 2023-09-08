#include "PrecipitationEffectDoubleNoLocalDiff.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "SimManager.h"
#include <stddef.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
clPrecipitationEffectDoubleNoLocalDiff::clPrecipitationEffectDoubleNoLocalDiff() {
	m_fPrevPrecip = 0;
	m_fCurrPrecip = 0;
	mp_fCurrA = NULL;
	mp_fCurrBLo = NULL;
	mp_fCurrBHi = NULL;
	mp_fCurrC = NULL;
	mp_fPrevA = NULL;
	mp_fPrevBLo = NULL;
	mp_fPrevBHi = NULL;
	mp_fPrevC = NULL;
	m_precip = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
clPrecipitationEffectDoubleNoLocalDiff::~clPrecipitationEffectDoubleNoLocalDiff() {
	delete [] mp_fCurrA;
	delete [] mp_fCurrBLo;
	delete [] mp_fCurrBHi;
	delete [] mp_fCurrC;
	delete [] mp_fPrevA;
	delete [] mp_fPrevBLo;
	delete [] mp_fPrevBHi;
	delete [] mp_fPrevC;
}

///////////////////////////////////////////////////////////////////////////////
// CalculatePrecipitationEffect
///////////////////////////////////////////////////////////////////////////////
double clPrecipitationEffectDoubleNoLocalDiff::CalculatePrecipitationEffect(clPlot *p_oPlot, int iSpecies) {
  double fEffect, fPrev, fCurrent, fTemp;

  //----- Current year portion ----------------------------------------------//
  fCurrent = 0;
  fTemp = m_fCurrPrecip - mp_fCurrC[iSpecies];
  fTemp *= fTemp;
  if (m_fCurrPrecip < mp_fCurrC[iSpecies]) {
    fCurrent = mp_fCurrA[iSpecies] * pow(mp_fCurrBLo[iSpecies], fTemp);
  } else {
    fCurrent = mp_fCurrA[iSpecies] * pow(mp_fCurrBHi[iSpecies], fTemp);
  }

  //----- Previous year portion ---------------------------------------------//
  fPrev = 0;
  fTemp = m_fPrevPrecip - mp_fPrevC[iSpecies];
  fTemp *= fTemp;
  if (m_fPrevPrecip < mp_fPrevC[iSpecies]) {
    fPrev = mp_fPrevA[iSpecies] * pow(mp_fPrevBLo[iSpecies], fTemp);
  } else {
    fPrev = mp_fPrevA[iSpecies] * pow(mp_fPrevBHi[iSpecies], fTemp);
  }

  fEffect = fPrev + fCurrent;
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

///////////////////////////////////////////////////////////////////////////////
// PreCalcs
///////////////////////////////////////////////////////////////////////////////
void clPrecipitationEffectDoubleNoLocalDiff::PreCalcs(clPlot *p_oPlot) {
  m_fPrevPrecip = m_fCurrPrecip;
  m_fCurrPrecip = ((p_oPlot)->*(this->m_precip))();
  m_fCurrPrecip /= 1000.0; //to meters
}

///////////////////////////////////////////////////////////////////////////////
// DoSetup
///////////////////////////////////////////////////////////////////////////////
void clPrecipitationEffectDoubleNoLocalDiff::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values

  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i, iPrecipType;

  mp_fCurrA = new double[iNumTotalSpecies];
  mp_fCurrBLo = new double[iNumTotalSpecies];
  mp_fCurrBHi = new double[iNumTotalSpecies];
  mp_fCurrC = new double[iNumTotalSpecies];
  mp_fPrevA = new double[iNumTotalSpecies];
  mp_fPrevBLo = new double[iNumTotalSpecies];
  mp_fPrevBHi = new double[iNumTotalSpecies];
  mp_fPrevC = new double[iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //----- Precipitation effect current a ------------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocPrecipEffCurrA",
      "ndnlpecaVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrA[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Precipitation effect current b lo ---------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocPrecipEffCurrBLo",
      "ndnlpecblVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrBLo[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Precipitation effect current b hi ---------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocPrecipEffCurrBHi",
      "ndnlpecbhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrBHi[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Precipitation effect current c ------------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocPrecipEffCurrC",
      "ndnlpeccVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrC[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Precipitation effect previous a -----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocPrecipEffPrevA",
      "ndnlpepaVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevA[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Precipitation effect previous b lo --------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocPrecipEffPrevBLo",
      "ndnlpepblVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevBLo[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Precipitation effect previous b hi --------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocPrecipEffPrevBHi",
      "ndnlpepbhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevBHi[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Precipitation effect previous c -----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocPrecipEffPrevC",
      "ndnlpepcVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevC[p_fTempValues[i].code] = p_fTempValues[i].val;


  delete[] p_fTempValues;

  //Precip value desired
  iPrecipType = mean_precip;
  FillSingleValue(p_oElement, "nciDoubNoLocPrecipType", &iPrecipType, true);
  if (iPrecipType == mean_precip) {
    m_precip = &clPlot::GetMeanAnnualPrecip;
  } else if (iPrecipType == seasonal_precip) {
    m_precip = &clPlot::GetSeasonalPrecipitation;
  } else if (iPrecipType == water_deficit) {
    m_precip = &clPlot::GetWaterDeficit;
  } else {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clPrecipitationEffectDoubleNoLocalDiff::DoSetup";
    stcErr.sMoreInfo = "Unrecognized precipitation type value.";
    throw( stcErr );
  }

  //----- Get the initial conditions value - will be previous for timestep 1 //
  clSimManager *p_oSimManager = p_oPop->GetSimManager();
  clPlot *p_oPlot = p_oSimManager->GetPlotObject();
  m_fCurrPrecip = ((p_oPlot)->*(this->m_precip))();
  m_fCurrPrecip /= 1000.0; //to meters
}
