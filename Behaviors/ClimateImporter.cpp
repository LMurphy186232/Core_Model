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
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clClimateImporter::GetData(DOMDocument * p_oDoc)
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  double **p_fPpt = NULL,
      **p_fTemp = NULL,
      *p_fRad = new double[12],
      *p_fPET = new double[12],
      *p_fAET = new double[12],
      *p_fSMS = new double[12];
  double fAWS, fAnnualAET, fAnnualPET;
  int iNumTimesteps = mp_oSimManager->GetNumberOfTimesteps(), i, j;
  try {

    //Declare arrays
    p_fPpt = new double*[12];
    p_fTemp = new double*[12];
    for (i = 0; i < 12; i++) {
      p_fPpt[i] = new double[iNumTimesteps];
      p_fTemp[i] = new double[iNumTimesteps];
      for (j = 0; j < iNumTimesteps; j++) {
        p_fPpt[i][j] = 0.0;
        p_fTemp[i][j] = 0.0;
      }
    }

    // Get parameter file data
    ReadParameterFileData(p_oElement, p_fPpt, p_fTemp, p_fRad);
    FillSingleValue(p_oElement, "sc_ciAWS", &fAWS, true);
    if (fAWS < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
      stcErr.sMoreInfo = "Available water storage value cannot be negative.";
      throw(stcErr);
    }

    //---------------------------------------------------------//
    // Do the climate calculations
    // I could condense loops here to only have one pass through
    // timestep and months. But I don't think this is going to take
    // all that long and it's only going to be done once at the
    // beginning of a run so I'm making it easier to read.
    //---------------------------------------------------------//
    mp_fPpt = new double[iNumTimesteps];
    mp_fTemp = new double[iNumTimesteps];
    mp_fSeasonalPpt = new double[iNumTimesteps];
    mp_fWD = new double[iNumTimesteps];

    // Calculate the annual precipitation and mean annual temperature values
    for (i = 0; i < iNumTimesteps; i++) {
      mp_fPpt[i] = 0;
      mp_fTemp[i] = 0;
      for (j = 0; j < 12; j++) {
        mp_fPpt[i] += p_fPpt[j][i];
        mp_fTemp[i] += p_fTemp[j][i];
      }
      mp_fTemp[i] /= 12.0;
    }

    // Calculate water deficit and seasonal precipitation
    for (i = 0; i < iNumTimesteps; i++) {

      //Calculate PET
      for (j = 0; j < 12; j++) {
        if (p_fTemp[j][i] >= 0)
          p_fPET[j] = 0.013 *
          (p_fTemp[j][i] / (p_fTemp[j][i] + 15)) * (p_fRad[j] + 50);
        else p_fPET[j] = 0;
      }

      //Calculate seasonal precipitation
      mp_fSeasonalPpt[i] = fAWS;
      for (j = 0; j < 12; j++) {
        if (p_fPET[j] >= p_fPpt[j][i]) {
          // This is a month in the growing season - add precip
          mp_fSeasonalPpt[i] += p_fPpt[j][i];
        } else {
          //This is a non-growing-season month - add PET
          mp_fSeasonalPpt[i] += p_fPET[j];
        }
      }

      // Calculate water deficit:
      //Calculate soil moisture
      //January assumes saturated soil at beginning of month
      p_fSMS[0] = fAWS + p_fPpt[0][i] - p_fPET[0];
      if (p_fSMS[0] > fAWS) p_fSMS[0] = fAWS;
      if (p_fSMS[0] < 0) p_fSMS[0] = 0;

      for (j = 1; j < 12; j++) {
        p_fSMS[j] = p_fSMS[j-1] + p_fPpt[j][i] - p_fPET[j];
        if (p_fSMS[j] > fAWS) p_fSMS[j] = fAWS;
        if (p_fSMS[j] < 0) p_fSMS[j] = 0;
      }

      //Calculate AET
      //January AET = PET
      p_fAET[0] = p_fPET[0];
      for (j = 1; j < 12; j++) {
        if (p_fSMS[j] > 0) p_fAET[j] = p_fPET[j];
        else p_fAET[j] = p_fSMS[j-1] + p_fPpt[j][i];
      }

      fAnnualPET = 0;
      fAnnualAET = 0;
      for (j = 0; j < 12; j++) {
        fAnnualPET += p_fPET[j];
        fAnnualAET += p_fAET[j];
      }

      mp_fWD[i] = fAnnualPET - fAnnualAET;
    }

    // Set the initial conditions values according to the first timestep
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    p_oPlot->SetMeanAnnualPrecip(mp_fPpt[0]);
    p_oPlot->SetMeanAnnualTemp(mp_fTemp[0]);
    p_oPlot->SetSeasonalPrecipitation(mp_fSeasonalPpt[0]);
    p_oPlot->SetWaterDeficit(mp_fWD[0]);

    delete[] p_fRad;
    delete[] p_fPET;
    delete[] p_fAET;
    delete[] p_fSMS;
    for (i = 0; i < 12; i++) {
      delete[] p_fPpt[i];
      delete[] p_fTemp[i];
    }
    delete[] p_fPpt;
    delete[] p_fTemp;

  } catch (modelErr & err) {
    delete[] p_fRad;
    delete[] p_fPET;
    delete[] p_fAET;
    delete[] p_fSMS;
    for (i = 0; i < 12; i++) {
      delete[] p_fPpt[i];
      delete[] p_fTemp[i];
    }
    delete[] p_fPpt;
    delete[] p_fTemp;
    throw(err);

  } catch (...) {
    delete[] p_fPET;
    delete[] p_fAET;
    delete[] p_fRad;
    delete[] p_fSMS;
    for (i = 0; i < 12; i++) {
      delete[] p_fPpt[i];
      delete[] p_fTemp[i];
    }
    delete[] p_fPpt;
    delete[] p_fTemp;

    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisturbance::CutTrees";
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
}

///////////////////////////////////////////////////////////////////////////////
// ReadParameterFileData
///////////////////////////////////////////////////////////////////////////////
void clClimateImporter::ReadParameterFileData(DOMElement * p_oElement,
    double **p_fPpt, double **p_fTemp, double *p_fRad) {

  double fVal;
  int i, j, iNumTimesteps = mp_oSimManager->GetNumberOfTimesteps();

  //Radiation for each month
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

  //Precipitation for each month and timestep
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJan", "sc_cimpjanVal", p_fPpt[0]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptFeb", "sc_cimpfebVal", p_fPpt[1]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptMar", "sc_cimpmarVal", p_fPpt[2]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptApr", "sc_cimpaprVal", p_fPpt[3]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptMay", "sc_cimpmayVal", p_fPpt[4]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJun", "sc_cimpjunVal", p_fPpt[5]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptJul", "sc_cimpjulVal", p_fPpt[6]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptAug", "sc_cimpaugVal", p_fPpt[7]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptSep", "sc_cimpsepVal", p_fPpt[8]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptOct", "sc_cimpoctVal", p_fPpt[9]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptNov", "sc_cimpnovVal", p_fPpt[10]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyPptDec", "sc_cimpdecVal", p_fPpt[11]);

  //Temperature for each month and timestep
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJan", "sc_cimtjanVal", p_fTemp[0]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempFeb", "sc_cimtfebVal", p_fTemp[1]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempMar", "sc_cimtmarVal", p_fTemp[2]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempApr", "sc_cimtaprVal", p_fTemp[3]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempMay", "sc_cimtmayVal", p_fTemp[4]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJun", "sc_cimtjunVal", p_fTemp[5]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempJul", "sc_cimtjulVal", p_fTemp[6]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempAug", "sc_cimtaugVal", p_fTemp[7]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempSep", "sc_cimtsepVal", p_fTemp[8]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempOct", "sc_cimtoctVal", p_fTemp[9]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempNov", "sc_cimtnovVal", p_fTemp[10]);
  ReadMonthlyData(p_oElement, "sc_ciMonthlyTempDec", "sc_cimtdecVal", p_fTemp[11]);

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
  }
}

///////////////////////////////////////////////////////////////////////////////
// ReadMonthlyData
///////////////////////////////////////////////////////////////////////////////
void clClimateImporter::ReadMonthlyData(DOMElement *p_oParent,
    std::string sParentTag, std::string sSubTag, double *p_fVal) {

  DOMNodeList *p_oNodeList;
  DOMElement *p_oElement, *p_oChildElement;
  DOMNode *p_oDocNode;
  XMLCh *sVal;
  char *cName;
  double fVal;
  int iNumChildren, iNumNodes, i, iCode;

  //Find the parent element
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

  //Now get the list of all subelements with the actual data
  p_oElement = (DOMElement *) p_oDocNode; //cast to DOM_Element
  sVal = XMLString::transcode(sSubTag.c_str());
  p_oNodeList = p_oElement->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  iNumChildren = p_oNodeList->getLength();
  if (iNumChildren != mp_oSimManager->GetNumberOfTimesteps()) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
    stcErr.sMoreInfo = "Not enough timesteps of data.";
    throw(stcErr);
  }

  for (i = 0; i < iNumChildren; i++) {
    p_oDocNode = p_oNodeList->item(i);
    p_oChildElement = (DOMElement *) p_oDocNode;
    sVal = XMLString::transcode("ts");
    cName = XMLString::transcode(p_oChildElement->getAttributeNode(sVal)->getNodeValue());
    XMLString::release(&sVal);
    iCode = atoi(cName);
    if (-1 == iCode) { //throw an error
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clClimateImporter::ReadParameterFileData";
      stcErr.sMoreInfo = "Unrecognized timestep in climate importer.";
      throw(stcErr);
    }
    cName = XMLString::transcode(p_oChildElement->getChildNodes()->item(0)->getNodeValue());
    fVal = strtod(cName, NULL);
    if (fVal != 0 || cName[0] == '0') {
      p_fVal[(iCode-1)] = fVal;
    }
  }
}
