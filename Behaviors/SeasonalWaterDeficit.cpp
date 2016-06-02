//---------------------------------------------------------------------------
// SeasonalWaterDeficit.cpp
//---------------------------------------------------------------------------
#include "SeasonalWaterDeficit.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clSeasonalWaterDeficit::clSeasonalWaterDeficit(clSimManager * p_oSimManager) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "SeasonalWaterDeficit";
    m_sXMLRoot = "SeasonalWaterDeficit";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_fPropRainfall = new double[12];
    mp_fRadiation = new double[12];
    mp_fRatioMonthlyAnnualTemp = new double[12];
    mp_fMonthlyTemp = new double[12];
    mp_fMonthlyPrecip = new double[12];
    mp_fMonthlyPET = new double[12];
    mp_fMonthlySMS = new double[12];
    mp_fMonthlyAET = new double[12];

    m_fAWS = 0;
  }
  catch (modelErr& err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSeasonalWaterDeficit::clSeasonalWaterDeficit";
    throw(stcErr);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clSeasonalWaterDeficit::~clSeasonalWaterDeficit()
{
  delete[] mp_fPropRainfall;
  delete[] mp_fRadiation;
  delete[] mp_fRatioMonthlyAnnualTemp;
  delete[] mp_fMonthlyTemp;
  delete[] mp_fMonthlyPrecip;
  delete[] mp_fMonthlyPET;
  delete[] mp_fMonthlySMS;
  delete[] mp_fMonthlyAET;
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clSeasonalWaterDeficit::GetData(DOMDocument * p_oDoc)
{

  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  double fVal;

  //Proportion of rain that falls in each month
  FillSingleValue(p_oElement, "sc_wdJanPptProp", &fVal, true); mp_fPropRainfall[0] = fVal;
  FillSingleValue(p_oElement, "sc_wdFebPptProp", &fVal, true); mp_fPropRainfall[1] = fVal;
  FillSingleValue(p_oElement, "sc_wdMarPptProp", &fVal, true); mp_fPropRainfall[2] = fVal;
  FillSingleValue(p_oElement, "sc_wdAprPptProp", &fVal, true); mp_fPropRainfall[3] = fVal;
  FillSingleValue(p_oElement, "sc_wdMayPptProp", &fVal, true); mp_fPropRainfall[4] = fVal;
  FillSingleValue(p_oElement, "sc_wdJunPptProp", &fVal, true); mp_fPropRainfall[5] = fVal;
  FillSingleValue(p_oElement, "sc_wdJulPptProp", &fVal, true); mp_fPropRainfall[6] = fVal;
  FillSingleValue(p_oElement, "sc_wdAugPptProp", &fVal, true); mp_fPropRainfall[7] = fVal;
  FillSingleValue(p_oElement, "sc_wdSepPptProp", &fVal, true); mp_fPropRainfall[8] = fVal;
  FillSingleValue(p_oElement, "sc_wdOctPptProp", &fVal, true); mp_fPropRainfall[9] = fVal;
  FillSingleValue(p_oElement, "sc_wdNovPptProp", &fVal, true); mp_fPropRainfall[10] = fVal;
  FillSingleValue(p_oElement, "sc_wdDecPptProp", &fVal, true); mp_fPropRainfall[11] = fVal;

  //Make sure the values are between 0 and 1 and add up to 1
  float fTot = 0;
  int i;
  for (i = 0; i < 12; i++) {
    if (mp_fPropRainfall[i] < 0 || mp_fPropRainfall[i] > 1) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSeasonalWaterDeficit::GetData";
      stcErr.sMoreInfo = "Rain proportion values must be between 0 and 1.";
      throw(stcErr);
    }
    fTot += mp_fPropRainfall[i];
  }
  if (fabs(fTot - 1) > 0.0001) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clSeasonalWaterDeficit::GetData";
    stcErr.sMoreInfo = "Rain proportion values must add up to 1.";
    throw(stcErr);
  }

  //Radiation for each month
  FillSingleValue(p_oElement, "sc_wdJanRad", &fVal, true); mp_fRadiation[0] = fVal;
  FillSingleValue(p_oElement, "sc_wdFebRad", &fVal, true); mp_fRadiation[1] = fVal;
  FillSingleValue(p_oElement, "sc_wdMarRad", &fVal, true); mp_fRadiation[2] = fVal;
  FillSingleValue(p_oElement, "sc_wdAprRad", &fVal, true); mp_fRadiation[3] = fVal;
  FillSingleValue(p_oElement, "sc_wdMayRad", &fVal, true); mp_fRadiation[4] = fVal;
  FillSingleValue(p_oElement, "sc_wdJunRad", &fVal, true); mp_fRadiation[5] = fVal;
  FillSingleValue(p_oElement, "sc_wdJulRad", &fVal, true); mp_fRadiation[6] = fVal;
  FillSingleValue(p_oElement, "sc_wdAugRad", &fVal, true); mp_fRadiation[7] = fVal;
  FillSingleValue(p_oElement, "sc_wdSepRad", &fVal, true); mp_fRadiation[8] = fVal;
  FillSingleValue(p_oElement, "sc_wdOctRad", &fVal, true); mp_fRadiation[9] = fVal;
  FillSingleValue(p_oElement, "sc_wdNovRad", &fVal, true); mp_fRadiation[10] = fVal;
  FillSingleValue(p_oElement, "sc_wdDecRad", &fVal, true); mp_fRadiation[11] = fVal;

  //Ratio of monthly temp to annual
  FillSingleValue(p_oElement, "sc_wdJanTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[0] = fVal;
  FillSingleValue(p_oElement, "sc_wdFebTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[1] = fVal;
  FillSingleValue(p_oElement, "sc_wdMarTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[2] = fVal;
  FillSingleValue(p_oElement, "sc_wdAprTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[3] = fVal;
  FillSingleValue(p_oElement, "sc_wdMayTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[4] = fVal;
  FillSingleValue(p_oElement, "sc_wdJunTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[5] = fVal;
  FillSingleValue(p_oElement, "sc_wdJulTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[6] = fVal;
  FillSingleValue(p_oElement, "sc_wdAugTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[7] = fVal;
  FillSingleValue(p_oElement, "sc_wdSepTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[8] = fVal;
  FillSingleValue(p_oElement, "sc_wdOctTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[9] = fVal;
  FillSingleValue(p_oElement, "sc_wdNovTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[10] = fVal;
  FillSingleValue(p_oElement, "sc_wdDecTempRatio", &fVal, true); mp_fRatioMonthlyAnnualTemp[11] = fVal;

  //Available water storage
  FillSingleValue(p_oElement, "sc_wdAWS", &m_fAWS, true);

  Action();
}


///////////////////////////////////////////////////////////////////////////////
// Action
///////////////////////////////////////////////////////////////////////////////
void clSeasonalWaterDeficit::Action()
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  float fAnnualTemp = p_oPlot->GetMeanAnnualTemp(),
        fAnnualPrecip = p_oPlot->GetMeanAnnualPrecip(),
        fAnnualPET = 0, fAnnualAET = 0, fWD;
  int i;

  //Calculate monthly temperature and precipitation
  for (i = 0; i < 12; i++) {
    mp_fMonthlyTemp[i] = fAnnualTemp * mp_fRatioMonthlyAnnualTemp[i];
    mp_fMonthlyPrecip[i] = fAnnualPrecip * mp_fPropRainfall[i];
  }

  //Calculate PET
  for (i = 0; i < 12; i++) {
    if (mp_fMonthlyTemp[i] >= 0)
      mp_fMonthlyPET[i] = 0.013 *
                          (mp_fMonthlyTemp[i] / (mp_fMonthlyTemp[i] + 15)) *
                          (mp_fRadiation[i] + 50);
    else mp_fMonthlyPET[i] = 0;
  }

  //Calculate soil moisture
  //January assumes saturated soil at beginning of month
  mp_fMonthlySMS[0] = m_fAWS + mp_fMonthlyPrecip[0] - mp_fMonthlyPET[0];
  if (mp_fMonthlySMS[0] > m_fAWS) mp_fMonthlySMS[0] = m_fAWS;
  if (mp_fMonthlySMS[0] < 0) mp_fMonthlySMS[0] = 0;

  for (i = 1; i < 12; i++) {
    mp_fMonthlySMS[i] = mp_fMonthlySMS[i-1] + mp_fMonthlyPrecip[i] - mp_fMonthlyPET[i];
    if (mp_fMonthlySMS[i] > m_fAWS) mp_fMonthlySMS[i] = m_fAWS;
    if (mp_fMonthlySMS[i] < 0) mp_fMonthlySMS[i] = 0;
  }

  //Calculate AET
  //January AET = PET
  mp_fMonthlyAET[0] = mp_fMonthlyPET[0];
  for (i = 1; i < 12; i++) {
    if (mp_fMonthlySMS[i] > 0) mp_fMonthlyAET[i] = mp_fMonthlyPET[i];
    else mp_fMonthlyAET[i] = mp_fMonthlySMS[i-1] + mp_fMonthlyPrecip[i];
  }

  for (i = 0; i < 12; i++) {
    fAnnualPET += mp_fMonthlyPET[i];
    fAnnualAET += mp_fMonthlyAET[i];
  }

  fWD = fAnnualPET - fAnnualAET;

  p_oPlot->SetWaterDeficit(fWD);
}

