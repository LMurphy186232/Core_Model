//---------------------------------------------------------------------------
// ClimateImporter.cpp
//---------------------------------------------------------------------------
#include "ClimateImporter.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clClimateImporter::clClimateImporter(clSimManager * p_oSimManager) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "ClimateImporter";
    m_sXMLRoot = "ClimateImporter";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_fPpt = NULL;
    mp_fSeasonalPpt = NULL;
    mp_fTemp = NULL;
    mp_fWD = NULL;
    mp_fNDep = NULL;

    mp_fLTMPpt = NULL;
    mp_fLTMSeasonalPpt = NULL;
    mp_fLTMTemp = NULL;
    mp_fLTMWD = NULL;

    m_iLTM = 0;
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
    stcErr.sFunction = "clClimateImporter::clClimateImporter";
    throw(stcErr);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clClimateImporter::~clClimateImporter()
{
  delete[] mp_fPpt;
  delete[] mp_fSeasonalPpt;
  delete[] mp_fTemp;
  delete[] mp_fWD;
  delete[] mp_fNDep;

  delete[] mp_fLTMPpt;
  delete[] mp_fLTMSeasonalPpt;
  delete[] mp_fLTMTemp;
  delete[] mp_fLTMWD;
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clClimateImporter::GetData(DOMDocument * p_oDoc)
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  double **p_fMonthlyPpt = NULL,
      **p_fMonthlyTemp = NULL,
      **p_fPreMonthlyPpt = NULL,
      **p_fPreMonthlyTemp = NULL,
      *p_fPrePpt = NULL,
      *p_fPreTemp = NULL,
      *p_fPreSeasonalPpt = NULL,
      *p_fPreWD = NULL,
      *p_fRad = new double[12],
      *p_fPET = new double[12],
      *p_fAET = new double[12],
      *p_fSMS = new double[12];
  double fAWS, fAnnualAET, fAnnualPET, fMean;
  int iNumTimesteps = mp_oSimManager->GetNumberOfTimesteps(),
      iStart, i, j, iArraySize;
  bool bCalendarMean;
  try {

    //----- Declare arrays --------------------------------------------------//
    p_fMonthlyPpt = new double*[12];
    p_fMonthlyTemp = new double*[12];
    for (i = 0; i < 12; i++) {
      p_fMonthlyPpt[i] = new double[iNumTimesteps];
      p_fMonthlyTemp[i] = new double[iNumTimesteps];
      for (j = 0; j < iNumTimesteps; j++) {
        p_fMonthlyPpt[i][j] = 0.0;
        p_fMonthlyTemp[i][j] = 0.0;
      }
    }

    //-----------------------------------------------------------------------//
    // Get parameter file data
    //-----------------------------------------------------------------------//
    //----- Available water storage -----------------------------------------//
    FillSingleValue(p_oElement, "sc_ciAWS", &fAWS, true);
    if (fAWS < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
      stcErr.sMoreInfo = "Available water storage value cannot be negative.";
      throw(stcErr);
    }

    //----- Calendar or growing season mean? --------------------------------//
    bCalendarMean = true;
    FillSingleValue(p_oElement, "sc_ciCalendarMean", &bCalendarMean, false);

    //----- Long term mean length - not required ----------------------------//
    FillSingleValue(p_oElement, "sc_ciLTM", &m_iLTM, false);
    if (m_iLTM < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
      stcErr.sMoreInfo = "Length of long term mean cannot be negative.";
      throw(stcErr);
    }

    //----- Growing season mean means we need one more year of data ---------//
    if (!bCalendarMean) {
      iArraySize = m_iLTM + 1;
    } else {
      iArraySize = m_iLTM;
    }

    //----- If LTM > 0, make arrays for this pre-run time -------------------//
    if (m_iLTM > 0 || !bCalendarMean) {
      p_fPreMonthlyPpt = new double*[12];
      p_fPreMonthlyTemp = new double*[12];
      for (i = 0; i < 12; i++) {
        p_fPreMonthlyPpt[i] = new double[iArraySize];
        p_fPreMonthlyTemp[i] = new double[iArraySize];
        for (j = 0; j < iArraySize; j++) {
          p_fPreMonthlyPpt[i][j] = 0.0;
          p_fPreMonthlyTemp[i][j] = 0.0;
        }
      }

      p_fPrePpt = new double[m_iLTM];
      p_fPreTemp = new double[m_iLTM];
      p_fPreSeasonalPpt = new double[m_iLTM];
      p_fPreWD = new double[m_iLTM];
    }


    //----- What's the first year of data that we want? ---------------------//
    if (m_iLTM == 0) {
      // If there is no LTM - we want to start on year 1
      iStart = 1;
    } else {
      // If there is a LTM - we want one year less (because the LTM for a
      // year includes that year's data); and if it's not calendar year, we
      // need an extra year than otherwise
      if (!bCalendarMean) {
        iStart = -m_iLTM;
      } else {
        iStart = -(m_iLTM - 1);
      }
    }


    //----- Read individual month climate values ----------------------------//
    // We'll read directly into N dep so declare it now
    mp_fNDep = new double[iNumTimesteps];
    ReadParameterFileData(p_oElement, p_fMonthlyPpt, p_fMonthlyTemp,
        p_fPreMonthlyPpt, p_fPreMonthlyTemp, p_fRad, iStart);

    //-----------------------------------------------------------------------//
    // Do the climate calculations
    // I could condense loops here to only have one pass through
    // timestep and months. But I don't think this is going to take
    // all that long and it's only going to be done once at the
    // beginning of a run so I'm making it easier to read.
    //-----------------------------------------------------------------------//
    mp_fPpt = new double[iNumTimesteps];
    mp_fTemp = new double[iNumTimesteps];
    mp_fSeasonalPpt = new double[iNumTimesteps];
    mp_fWD = new double[iNumTimesteps];
    mp_fLTMPpt = new double[iNumTimesteps];
    mp_fLTMTemp = new double[iNumTimesteps];
    mp_fLTMSeasonalPpt = new double[iNumTimesteps];
    mp_fLTMWD = new double[iNumTimesteps];

    //-----------------------------------------------------------------------//
    // Calculate the annual precipitation and mean annual temperature values
    // Be aware of whether calendar or growing season year is desired
    //-----------------------------------------------------------------------//
    if (bCalendarMean) {
      for (i = 0; i < iNumTimesteps; i++) {
        mp_fPpt[i] = 0;
        mp_fTemp[i] = 0;
        for (j = 0; j < 12; j++) {
          mp_fPpt[i] += p_fMonthlyPpt[j][i];
          mp_fTemp[i] += p_fMonthlyTemp[j][i];
        }
        mp_fTemp[i] /= 12.0;
      }

      // Also any values required by the long-term mean
      if (m_iLTM > 0) {
        for (i = 0; i < (m_iLTM-1); i++) {
          p_fPrePpt[i] = 0;
          p_fPreTemp[i] = 0;
          for (j = 0; j < 12; j++) {
            p_fPrePpt[i] += p_fPreMonthlyPpt[j][i];
            p_fPreTemp[i] += p_fPreMonthlyTemp[j][i];
          }
          p_fPreTemp[i] /= 12.0;
        }
      }

    } else {
      // Do the first year separately
      mp_fPpt[0] = 0;
      mp_fTemp[0] = 0;
      for (j = 9; j < 12; j++) {
        mp_fPpt[0] += p_fPreMonthlyPpt[j][0];
        mp_fTemp[0] += p_fPreMonthlyTemp[j][0];
      }
      for (j = 0; j < 9; j++) {
        mp_fPpt[0] += p_fMonthlyPpt[j][0];
        mp_fTemp[0] += p_fMonthlyTemp[j][0];
      }
      mp_fTemp[0] /= 12.0;

      // Now the rest of the years
      for (i = 1; i < iNumTimesteps; i++) {
        mp_fPpt[i] = 0;
        mp_fTemp[i] = 0;
        for (j = 9; j < 12; j++) {
          mp_fPpt[i] += p_fMonthlyPpt[j][(i-1)];
          mp_fTemp[i] += p_fMonthlyTemp[j][(i-1)];
        }
        for (j = 0; j < 9; j++) {
          mp_fPpt[i] += p_fMonthlyPpt[j][i];
          mp_fTemp[i] += p_fMonthlyTemp[j][i];
        }
        mp_fTemp[i] /= 12.0;
      }

      // Also any values required by the long-term mean
      if (m_iLTM > 0) {
        for (i = 0; i < m_iLTM; i++) {
          p_fPrePpt[i] = 0;
          p_fPreTemp[i] = 0;
          for (j = 9; j < 12; j++) {
            p_fPrePpt[i] += p_fPreMonthlyPpt[j][i+1];
            p_fPreTemp[i] += p_fPreMonthlyTemp[j][i+1];
          }
          for (j = 0; j < 9; j++) {
            p_fPrePpt[i] += p_fPreMonthlyPpt[j][i];
            p_fPreTemp[i] += p_fPreMonthlyTemp[j][i];
          }
          p_fPreTemp[i] /= 12.0;
        }

      }
    }
    //-----------------------------------------------------------------------//



    //-----------------------------------------------------------------------//
    // Calculate water deficit and seasonal precipitation
    //-----------------------------------------------------------------------//
    for (i = 0; i < iNumTimesteps; i++) {

      //Calculate PET
      for (j = 0; j < 12; j++) {
        if (p_fMonthlyTemp[j][i] >= 0)
          p_fPET[j] = 0.013 *
          (p_fMonthlyTemp[j][i] / (p_fMonthlyTemp[j][i] + 15)) * (p_fRad[j] + 50);
        else p_fPET[j] = 0;
      }

      //Calculate seasonal precipitation
      mp_fSeasonalPpt[i] = fAWS;
      for (j = 0; j < 12; j++) {
        if (p_fPET[j] >= p_fMonthlyPpt[j][i]) {
          // This is a month in the growing season - add precip
          mp_fSeasonalPpt[i] += p_fMonthlyPpt[j][i];
        } else {
          //This is a non-growing-season month - add PET
          mp_fSeasonalPpt[i] += p_fPET[j];
        }
      }

      // Calculate water deficit:
      //Calculate soil moisture
      //January assumes saturated soil at beginning of month
      p_fSMS[0] = fAWS + p_fMonthlyPpt[0][i] - p_fPET[0];
      if (p_fSMS[0] > fAWS) p_fSMS[0] = fAWS;
      if (p_fSMS[0] < 0) p_fSMS[0] = 0;

      for (j = 1; j < 12; j++) {
        p_fSMS[j] = p_fSMS[j-1] + p_fMonthlyPpt[j][i] - p_fPET[j];
        if (p_fSMS[j] > fAWS) p_fSMS[j] = fAWS;
        if (p_fSMS[j] < 0) p_fSMS[j] = 0;
      }

      //Calculate AET
      //January AET = PET
      p_fAET[0] = p_fPET[0];
      for (j = 1; j < 12; j++) {
        if (p_fSMS[j] > 0) p_fAET[j] = p_fPET[j];
        else p_fAET[j] = p_fSMS[j-1] + p_fMonthlyPpt[j][i];
      }

      fAnnualPET = 0;
      fAnnualAET = 0;
      for (j = 0; j < 12; j++) {
        fAnnualPET += p_fPET[j];
        fAnnualAET += p_fAET[j];
      }

      mp_fWD[i] = fAnnualPET - fAnnualAET;
    }


    //-----------------------------------------------------------------------//
    // Repeat for long-term means
    //-----------------------------------------------------------------------//
    if (m_iLTM > 0) {
      for (i = 0; i < m_iLTM; i++) {

        //Calculate PET
        for (j = 0; j < 12; j++) {
          if (p_fPreMonthlyTemp[j][i] >= 0)
            p_fPET[j] = 0.013 *
            (p_fPreMonthlyTemp[j][i] / (p_fPreMonthlyTemp[j][i] + 15)) * (p_fRad[j] + 50);
          else p_fPET[j] = 0;
        }

        //Calculate seasonal precipitation
        p_fPreSeasonalPpt[i] = fAWS;
        for (j = 0; j < 12; j++) {
          if (p_fPET[j] >= p_fPreMonthlyPpt[j][i]) {
            // This is a month in the growing season - add precip
            p_fPreSeasonalPpt[i] += p_fPreMonthlyPpt[j][i];
          } else {
            //This is a non-growing-season month - add PET
            p_fPreSeasonalPpt[i] += p_fPET[j];
          }
        }

        // Calculate water deficit:
        //Calculate soil moisture
        //January assumes saturated soil at beginning of month
        p_fSMS[0] = fAWS + p_fPreMonthlyPpt[0][i] - p_fPET[0];
        if (p_fSMS[0] > fAWS) p_fSMS[0] = fAWS;
        if (p_fSMS[0] < 0) p_fSMS[0] = 0;

        for (j = 1; j < 12; j++) {
          p_fSMS[j] = p_fSMS[j-1] + p_fPreMonthlyPpt[j][i] - p_fPET[j];
          if (p_fSMS[j] > fAWS) p_fSMS[j] = fAWS;
          if (p_fSMS[j] < 0) p_fSMS[j] = 0;
        }

        //Calculate AET
        //January AET = PET
        p_fAET[0] = p_fPET[0];
        for (j = 1; j < 12; j++) {
          if (p_fSMS[j] > 0) p_fAET[j] = p_fPET[j];
          else p_fAET[j] = p_fSMS[j-1] + p_fPreMonthlyPpt[j][i];
        }

        fAnnualPET = 0;
        fAnnualAET = 0;
        for (j = 0; j < 12; j++) {
          fAnnualPET += p_fPET[j];
          fAnnualAET += p_fAET[j];
        }

        p_fPreWD[i] = fAnnualPET - fAnnualAET;

      }
    }


    //-----------------------------------------------------------------------//
    // Calculate long-term means if desired
    //-----------------------------------------------------------------------//
    if (m_iLTM > 0) {
      for (i = 0; i < iNumTimesteps; i++) {

        //----- Annual temperature ------------------------------------------//
        fMean = 0;
        if (i < m_iLTM) {
          // How many pre-run years do we need?
          iStart = m_iLTM - i - 2;
          for (j = iStart; j >= 0; j--) {
            fMean += p_fPreTemp[j];
          }
          for (j = 0; j <= i; j++) {
            fMean += mp_fTemp[j];
          }

        } else {
          iStart = ((i-m_iLTM)+1);
          for (j = iStart; j <= i; j++) {
            fMean += mp_fTemp[j];
          }
        }
        fMean /= m_iLTM;
        mp_fLTMTemp[i] = fMean;


        //----- Annual precip -----------------------------------------------//
        fMean = 0;
        if (i < m_iLTM) {
          // How many pre-run years do we need?
          iStart = m_iLTM - i - 2;
          for (j = iStart; j >= 0; j--) {
            fMean += p_fPrePpt[j];
          }
          for (j = 0; j <= i; j++) {
            fMean += mp_fPpt[j];
          }

        } else {
          iStart = ((i-m_iLTM)+1);
          for (j = iStart; j <= i; j++) {
            fMean += mp_fPpt[j];
          }
        }
        fMean /= m_iLTM;
        mp_fLTMPpt[i] = fMean;



        //----- Annual seasonal precip --------------------------------------//
        fMean = 0;
        if (i < m_iLTM) {
          // How many pre-run years do we need?
          iStart = m_iLTM - i - 2;
          for (j = iStart; j >= 0; j--) {
            fMean += p_fPreSeasonalPpt[j];
          }
          for (j = 0; j <= i; j++) {
            fMean += mp_fSeasonalPpt[j];
          }

        } else {
          iStart = ((i-m_iLTM)+1);
          for (j = iStart; j <= i; j++) {
            fMean += mp_fSeasonalPpt[j];
          }
        }
        fMean /= m_iLTM;
        mp_fLTMSeasonalPpt[i] = fMean;


        //----- Annual water deficit ----------------------------------------//
        fMean = 0;
        if (i < m_iLTM) {
          // How many pre-run years do we need?
          iStart = m_iLTM - i - 2;
          for (j = iStart; j >= 0; j--) {
            fMean += p_fPreWD[j];
          }
          for (j = 0; j <= i; j++) {
            fMean += mp_fWD[j];
          }

        } else {
          iStart = ((i-m_iLTM)+1);
          for (j = iStart; j <= i; j++) {
            fMean += mp_fWD[j];
          }
        }
        fMean /= m_iLTM;
        mp_fLTMWD[i] = fMean;


      } //for (i = 0; i < iNumTimesteps; i++) {
    } // if (m_iLTM > 0) {




    // Set the initial conditions values according to the first timestep
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    p_oPlot->SetMeanAnnualPrecip(mp_fPpt[0]);
    p_oPlot->SetMeanAnnualTemp(mp_fTemp[0]);
    p_oPlot->SetSeasonalPrecipitation(mp_fSeasonalPpt[0]);
    p_oPlot->SetWaterDeficit(mp_fWD[0]);
    if (m_iLTM > 0) {
      p_oPlot->SetLTMAnnualPrecip(mp_fLTMPpt[0]);
      p_oPlot->SetLTMAnnualTemp(mp_fLTMTemp[0]);
      p_oPlot->SetLTMSeasonalPrecipitation(mp_fLTMSeasonalPpt[0]);
      p_oPlot->SetLTMWaterDeficit(mp_fLTMWD[0]);
    }

    delete[] p_fRad;
    delete[] p_fPET;
    delete[] p_fAET;
    delete[] p_fSMS;
    delete[] p_fPrePpt;
    delete[] p_fPreTemp;
    delete[] p_fPreSeasonalPpt;
    delete[] p_fPreWD;
    for (i = 0; i < 12; i++) {
      delete[] p_fMonthlyPpt[i];
      delete[] p_fMonthlyTemp[i];
    }
    delete[] p_fMonthlyPpt;
    delete[] p_fMonthlyTemp;

    if (p_fPreMonthlyPpt) {
      for (i = 0; i < 12; i++) {
        delete[] p_fPreMonthlyPpt[i];
        delete[] p_fPreMonthlyTemp[i];
      }
      delete[] p_fPreMonthlyPpt;
      delete[] p_fPreMonthlyTemp;
    }

  } catch (modelErr & err) {
    delete[] p_fRad;
    delete[] p_fPET;
    delete[] p_fAET;
    delete[] p_fSMS;
    delete[] p_fPrePpt;
    delete[] p_fPreTemp;
    delete[] p_fPreSeasonalPpt;
    delete[] p_fPreWD;
    for (i = 0; i < 12; i++) {
      delete[] p_fMonthlyPpt[i];
      delete[] p_fMonthlyTemp[i];
    }
    delete[] p_fMonthlyPpt;
    delete[] p_fMonthlyTemp;
    if (p_fPreMonthlyPpt) {
      for (i = 0; i < 12; i++) {
        delete[] p_fPreMonthlyPpt[i];
        delete[] p_fPreMonthlyTemp[i];
      }
      delete[] p_fPreMonthlyPpt;
      delete[] p_fPreMonthlyTemp;
    }
    throw(err);

  } catch (...) {
    delete[] p_fPET;
    delete[] p_fAET;
    delete[] p_fRad;
    delete[] p_fSMS;
    delete[] p_fPrePpt;
    delete[] p_fPreTemp;
    delete[] p_fPreSeasonalPpt;
    delete[] p_fPreWD;
    for (i = 0; i < 12; i++) {
      delete[] p_fMonthlyPpt[i];
      delete[] p_fMonthlyTemp[i];
    }
    delete[] p_fMonthlyPpt;
    delete[] p_fMonthlyTemp;
    if (p_fPreMonthlyPpt) {
      for (i = 0; i < 12; i++) {
        delete[] p_fPreMonthlyPpt[i];
        delete[] p_fPreMonthlyTemp[i];
      }
      delete[] p_fPreMonthlyPpt;
      delete[] p_fPreMonthlyTemp;
    }

    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clClimateImporter::GetData";
    throw(stcErr);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Action
///////////////////////////////////////////////////////////////////////////////
void clClimateImporter::Action()
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  int iTS = mp_oSimManager->GetCurrentTimestep() - 1;

  p_oPlot->SetMeanAnnualPrecip(mp_fPpt[iTS]);
  p_oPlot->SetMeanAnnualTemp(mp_fTemp[iTS]);
  p_oPlot->SetSeasonalPrecipitation(mp_fSeasonalPpt[iTS]);
  p_oPlot->SetWaterDeficit(mp_fWD[iTS]);

  if (mp_fNDep[iTS] > -900) {
    p_oPlot->SetNDeposition(mp_fNDep[iTS]);
  }

  if (m_iLTM > 0) {
    p_oPlot->SetLTMAnnualPrecip(mp_fLTMPpt[iTS]);
    p_oPlot->SetLTMAnnualTemp(mp_fLTMTemp[iTS]);
    p_oPlot->SetLTMSeasonalPrecipitation(mp_fLTMSeasonalPpt[iTS]);
    p_oPlot->SetLTMWaterDeficit(mp_fLTMWD[iTS]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// ReadParameterFileData
///////////////////////////////////////////////////////////////////////////////
void clClimateImporter::ReadParameterFileData(DOMElement * p_oElement, double **p_fPpt,
    double **p_fTemp, double **p_fPrePpt, double **p_fPreTemp,
    double *p_fRad, int iStart) {

  double fVal;
  int i, j, iNumTimesteps = mp_oSimManager->GetNumberOfTimesteps();

  //-------------------------------------------------------------------------//
  // Pre-fill the arrays with bad values so we will know if any data was
  // missing
  //-------------------------------------------------------------------------//
  if (iStart < 1 && p_fTemp) {
    for (i = 0; i < 12; i++) {
      for (j = 0; j < abs(iStart); j++) {
        p_fPreTemp[i][j] = -999;
        p_fPrePpt [i][j] = -999;
      }
      for (j = 0; j < iNumTimesteps; j++) {
        p_fPpt [i][j] = -999;
        p_fTemp[i][j] = -999;
      }
    }
  }
  for (j = 0; j < iNumTimesteps; j++) {
    mp_fNDep[j] = -999;
  }
  //-------------------------------------------------------------------------//



  //-------------------------------------------------------------------------//
  //Radiation for each month
  //-------------------------------------------------------------------------//
  FillSingleValue(p_oElement, "sc_ciJanRad", &fVal, true); p_fRad[0] = fVal;
  FillSingleValue(p_oElement, "sc_ciFebRad", &fVal, true); p_fRad[1] = fVal;
  FillSingleValue(p_oElement, "sc_ciMarRad", &fVal, true); p_fRad[2] = fVal;
  FillSingleValue(p_oElement, "sc_ciAprRad", &fVal, true); p_fRad[3] = fVal;
  FillSingleValue(p_oElement, "sc_ciMayRad", &fVal, true); p_fRad[4] = fVal;
  FillSingleValue(p_oElement, "sc_ciJunRad", &fVal, true); p_fRad[5] = fVal;
  FillSingleValue(p_oElement, "sc_ciJulRad", &fVal, true); p_fRad[6] = fVal;
  FillSingleValue(p_oElement, "sc_ciAugRad", &fVal, true); p_fRad[7] = fVal;
  FillSingleValue(p_oElement, "sc_ciSepRad", &fVal, true); p_fRad[8] = fVal;
  FillSingleValue(p_oElement, "sc_ciOctRad", &fVal, true); p_fRad[9] = fVal;
  FillSingleValue(p_oElement, "sc_ciNovRad", &fVal, true); p_fRad[10] = fVal;
  FillSingleValue(p_oElement, "sc_ciDecRad", &fVal, true); p_fRad[11] = fVal;
  //-------------------------------------------------------------------------//



  if (iStart < 0) {
    //-----------------------------------------------------------------------//
    //Precipitation for each month and timestep
    //-----------------------------------------------------------------------//
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJan", "sc_cimpjanVal", p_fPpt[0] , p_fPrePpt[0] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptFeb", "sc_cimpfebVal", p_fPpt[1] , p_fPrePpt[1] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptMar", "sc_cimpmarVal", p_fPpt[2] , p_fPrePpt[2] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptApr", "sc_cimpaprVal", p_fPpt[3] , p_fPrePpt[3] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptMay", "sc_cimpmayVal", p_fPpt[4] , p_fPrePpt[4] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJun", "sc_cimpjunVal", p_fPpt[5] , p_fPrePpt[5] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJul", "sc_cimpjulVal", p_fPpt[6] , p_fPrePpt[6] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptAug", "sc_cimpaugVal", p_fPpt[7] , p_fPrePpt[7] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptSep", "sc_cimpsepVal", p_fPpt[8] , p_fPrePpt[8] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptOct", "sc_cimpoctVal", p_fPpt[9] , p_fPrePpt[9] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptNov", "sc_cimpnovVal", p_fPpt[10], p_fPrePpt[10], iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptDec", "sc_cimpdecVal", p_fPpt[11], p_fPrePpt[11], iStart, iNumTimesteps);

    //-----------------------------------------------------------------------//
    //Temperature for each month and timestep
    //-----------------------------------------------------------------------//
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJan", "sc_cimtjanVal", p_fTemp[0] , p_fPreTemp[0] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempFeb", "sc_cimtfebVal", p_fTemp[1] , p_fPreTemp[1] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempMar", "sc_cimtmarVal", p_fTemp[2] , p_fPreTemp[2] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempApr", "sc_cimtaprVal", p_fTemp[3] , p_fPreTemp[3] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempMay", "sc_cimtmayVal", p_fTemp[4] , p_fPreTemp[4] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJun", "sc_cimtjunVal", p_fTemp[5] , p_fPreTemp[5] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJul", "sc_cimtjulVal", p_fTemp[6] , p_fPreTemp[6] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempAug", "sc_cimtaugVal", p_fTemp[7] , p_fPreTemp[7] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempSep", "sc_cimtsepVal", p_fTemp[8] , p_fPreTemp[8] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempOct", "sc_cimtoctVal", p_fTemp[9] , p_fPreTemp[9] , iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempNov", "sc_cimtnovVal", p_fTemp[10], p_fPreTemp[10], iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempDec", "sc_cimtdecVal", p_fTemp[11], p_fPreTemp[11], iStart, iNumTimesteps);

  } else {

    //-----------------------------------------------------------------------//
    //Precipitation for each month and timestep
    //-----------------------------------------------------------------------//
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJan", "sc_cimpjanVal", p_fPpt[0] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptFeb", "sc_cimpfebVal", p_fPpt[1] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptMar", "sc_cimpmarVal", p_fPpt[2] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptApr", "sc_cimpaprVal", p_fPpt[3] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptMay", "sc_cimpmayVal", p_fPpt[4] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJun", "sc_cimpjunVal", p_fPpt[5] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJul", "sc_cimpjulVal", p_fPpt[6] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptAug", "sc_cimpaugVal", p_fPpt[7] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptSep", "sc_cimpsepVal", p_fPpt[8] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptOct", "sc_cimpoctVal", p_fPpt[9] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptNov", "sc_cimpnovVal", p_fPpt[10], NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyPptDec", "sc_cimpdecVal", p_fPpt[11], NULL, iStart, iNumTimesteps);

    //-----------------------------------------------------------------------//
    //Temperature for each month and timestep
    //-----------------------------------------------------------------------//
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJan", "sc_cimtjanVal", p_fTemp[0] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempFeb", "sc_cimtfebVal", p_fTemp[1] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempMar", "sc_cimtmarVal", p_fTemp[2] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempApr", "sc_cimtaprVal", p_fTemp[3] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempMay", "sc_cimtmayVal", p_fTemp[4] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJun", "sc_cimtjunVal", p_fTemp[5] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJul", "sc_cimtjulVal", p_fTemp[6] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempAug", "sc_cimtaugVal", p_fTemp[7] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempSep", "sc_cimtsepVal", p_fTemp[8] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempOct", "sc_cimtoctVal", p_fTemp[9] , NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempNov", "sc_cimtnovVal", p_fTemp[10], NULL, iStart, iNumTimesteps);
    ReadMonthlyData(p_oElement, "sc_ciMonthlyTempDec", "sc_cimtdecVal", p_fTemp[11], NULL, iStart, iNumTimesteps);
  }

  //-------------------------------------------------------------------------//
  // Nitrogen data
  //-------------------------------------------------------------------------//
  ReadNitrogenData(p_oElement, iNumTimesteps);

  //Check our data
  for (i = 0; i < 12; i++) {
    if (p_fRad[i] < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
      stcErr.sMoreInfo = "Monthly radiation value is negative.";
      throw(stcErr);
    }
    for (j = 0; j < iNumTimesteps; j++) {

      if (p_fPpt[i][j] < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
        stcErr.sMoreInfo = "Monthly precipitation value is negative.";
        throw(stcErr);
      }
    }
    if (iStart < 1 && p_fTemp) {
      for (i = 0; i < 12; i++) {
        for (j = 0; j < abs(iStart); j++) {
          if (p_fPreTemp[i][j] < -900) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
            stcErr.sMoreInfo = "Monthly temperature value is missing.";
            throw(stcErr);
          }
          if (p_fPrePpt [i][j] < -900) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
            stcErr.sMoreInfo = "Monthly precipitation value is missing.";
            throw(stcErr);
          }
        }
        for (j = 0; j < iNumTimesteps; j++) {
          if (p_fTemp[i][j] < -900) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
            stcErr.sMoreInfo = "Monthly temperature value is missing.";
            throw(stcErr);
          }
          if (p_fPpt [i][j] < -900) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
            stcErr.sMoreInfo = "Monthly precipitation value is missing.";
            throw(stcErr);
          }
        }
      }
    }
  }

  for (j = 0; j < iNumTimesteps; j++) {

    if (mp_fNDep[0] > -900 && mp_fNDep[j] < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
      stcErr.sMoreInfo = "Annual nitrogen value is missing or negative.";
      throw(stcErr);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// ReadMonthlyData
///////////////////////////////////////////////////////////////////////////////
void clClimateImporter::ReadMonthlyData(DOMElement *p_oParent,
    std::string sParentTag, std::string sSubTag, double *p_fVal,
    double *p_fPreVal, int iStart, int iEnd) {

  DOMNodeList *p_oNodeList;
  DOMElement *p_oElement, *p_oChildElement;
  DOMNode *p_oDocNode;
  XMLCh *sVal;
  char *cName;
  double fVal;
  int iNumChildren, iNumNodes, i, iCode, iExpectedYears;

  //-------------------------------------------------------------------------//
  // How many years of data are we expecting? All timesteps plus extras for
  // long-term mean
  //-------------------------------------------------------------------------//
  iExpectedYears = iEnd - iStart;

  //-------------------------------------------------------------------------//
  //Find the parent element
  //-------------------------------------------------------------------------//
  sVal = XMLString::transcode(sParentTag.c_str());
  p_oNodeList = p_oParent->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  iNumNodes = p_oNodeList->getLength();
  if (0 == iNumNodes) {
    modelErr stcErr;
    stcErr.sFunction = "ClimateImporter::ReadMonthlyData";
    stcErr.iErrorCode = DATA_MISSING;
    stcErr.sMoreInfo = sParentTag;
    throw(stcErr);
  }
  p_oDocNode = p_oNodeList->item(0); //get first element returned by the list


  //-------------------------------------------------------------------------//
  //Now get the list of all subelements with the actual data
  //-------------------------------------------------------------------------//
  p_oElement = (DOMElement *) p_oDocNode; //cast to DOM_Element
  sVal = XMLString::transcode(sSubTag.c_str());
  p_oNodeList = p_oElement->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  iNumChildren = p_oNodeList->getLength();
  if (iNumChildren < iExpectedYears) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
    stcErr.sMoreInfo = "Not enough timesteps of data.";
    throw(stcErr);
  }


  //-------------------------------------------------------------------------//
  // Read the data into our arrays
  //-------------------------------------------------------------------------//
  for (i = 0; i < iNumChildren; i++) {
    p_oDocNode = p_oNodeList->item(i);
    p_oChildElement = (DOMElement *) p_oDocNode;

    //----- Read the timestep this is for -----------------------------------//
    sVal = XMLString::transcode("ts");
    cName = XMLString::transcode(p_oChildElement->getAttributeNode(sVal)->getNodeValue());
    XMLString::release(&sVal);
    iCode = atoi(cName);

    //----- A negative might mean string miscoding - might be real ----------//
    if (-1 == iCode && cName[0] != '-') { //throw an error
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadMonthlyData";
      stcErr.sMoreInfo = "Unrecognized timestep in climate importer.";
      throw(stcErr);
    }
    //----- A zero is not allowed -------------------------------------------//
    if (iCode == 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadMonthlyData";
      stcErr.sMoreInfo = "Zero is not an allowed timestep in climate importer.";
      throw(stcErr);
    }

    //----- Proceed only if this is data we need ----------------------------//
    if (iCode >= iStart && iCode <= iEnd) {

      //----- Get the actual value here -------------------------------------//
      cName = XMLString::transcode(p_oChildElement->getChildNodes()->item(0)->getNodeValue());
      fVal = strtod(cName, NULL);
      if (fVal != 0 || cName[0] == '0') {

        // If this is a negative number, put it in the pre-run array
        if (iCode < 0) {
          p_fPreVal[(abs(iCode)-1)] = fVal;
        } else {
          p_fVal[(iCode-1)] = fVal;
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// ReadNitrogenData
///////////////////////////////////////////////////////////////////////////////
void clClimateImporter::ReadNitrogenData(DOMElement *p_oParent, int iEnd) {

  DOMNodeList *p_oNodeList;
  DOMElement *p_oElement, *p_oChildElement;
  DOMNode *p_oDocNode;
  XMLCh *sVal;
  std::string sParentTag = "sc_ciNDep", sSubTag = "sc_cindVal";
  char *cName;
  double fVal;
  int iNumChildren, iNumNodes, i, iCode;

  //-------------------------------------------------------------------------//
  // Find the parent element
  //-------------------------------------------------------------------------//
  sVal = XMLString::transcode(sParentTag.c_str());
  p_oNodeList = p_oParent->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  iNumNodes = p_oNodeList->getLength();
  if (0 == iNumNodes) {
    // This is okay, nitrogen isn't required
    return;
  }
  p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

  //----- Bein' sneaky: make a flag to say we need all values ---------------//
  mp_fNDep[0] = 0;

  //-------------------------------------------------------------------------//
  // Now get the list of all subelements with the actual data
  // At this point, it IS required
  //-------------------------------------------------------------------------//
  p_oElement = (DOMElement *) p_oDocNode; //cast to DOM_Element
  sVal = XMLString::transcode(sSubTag.c_str());
  p_oNodeList = p_oElement->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  iNumChildren = p_oNodeList->getLength();
  if (iNumChildren < iEnd) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clClimateImporter::ReadNitrogenData";
    stcErr.sMoreInfo = "Not enough timesteps of data.";
    throw(stcErr);
  }


  //-------------------------------------------------------------------------//
  // Read the data into our arrays
  //-------------------------------------------------------------------------//
  for (i = 0; i < iNumChildren; i++) {
    p_oDocNode = p_oNodeList->item(i);
    p_oChildElement = (DOMElement *) p_oDocNode;

    //----- Read the timestep this is for -----------------------------------//
    sVal = XMLString::transcode("ts");
    cName = XMLString::transcode(p_oChildElement->getAttributeNode(sVal)->getNodeValue());
    XMLString::release(&sVal);
    iCode = atoi(cName);

    //----- A negative is allowed for other data but not nitrogen ----------//
    if (-1 == iCode) { //throw an error
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadNitrogenData";
      stcErr.sMoreInfo = "Unrecognized timestep in climate importer.";
      throw(stcErr);
    }
    //----- A zero is not allowed -------------------------------------------//
    if (iCode == 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadNitrogenData";
      stcErr.sMoreInfo = "Zero is not an allowed timestep in climate importer.";
      throw(stcErr);
    }

    //----- Proceed only if this is data we need ----------------------------//
    if (iCode <= iEnd) {

      //----- Get the actual value here -------------------------------------//
      cName = XMLString::transcode(p_oChildElement->getChildNodes()->item(0)->getNodeValue());
      fVal = strtod(cName, NULL);
      if (fVal != 0 || cName[0] == '0') {
        mp_fNDep[(iCode-1)] = fVal;
      }
    }
  }
}
