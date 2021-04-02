//---------------------------------------------------------------------------
// ClimateChange.cpp
//---------------------------------------------------------------------------
#include "ClimateChange.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clClimateChange::clClimateChange(clSimManager * p_oSimManager) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "ClimateChange";
    m_sXMLRoot = "ClimateChange";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    m_iTimeElapsed = 0;
    m_iTimestepLength = 0;
    m_fB = 0;
    m_fC = 0;
    m_fStartingValue = 0;
    m_fMin = 0;
    m_fMax = 0;
    m_bIsTemp = false;
    m_bUpdateAllPrecip = false;
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
    stcErr.sFunction = "clClimateChange::clClimateChange";
    throw(stcErr);
  }
}


///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clClimateChange::GetData(DOMDocument * p_oDoc)
{
  try
  {
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

    if (m_bIsTemp) {
      FillSingleValue(p_oElement, "sc_climateChangeTempB", &m_fB, true);
      FillSingleValue(p_oElement, "sc_climateChangeTempC", &m_fC, true);
      FillSingleValue(p_oElement, "sc_climateChangeMinTemp", &m_fMin, true);
      FillSingleValue(p_oElement, "sc_climateChangeMaxTemp", &m_fMax, true);
      m_fStartingValue = p_oPlot->GetMeanAnnualTemp();
    } else {
      FillSingleValue(p_oElement, "sc_climateChangePrecipB", &m_fB, true);
      FillSingleValue(p_oElement, "sc_climateChangePrecipC", &m_fC, true);
      FillSingleValue(p_oElement, "sc_climateChangeMinPrecip", &m_fMin, true);
      FillSingleValue(p_oElement, "sc_climateChangeMaxPrecip", &m_fMax, true);
      FillSingleValue(p_oElement, "sc_climateChangeOtherPrecip", &m_bUpdateAllPrecip, true);
      m_fStartingValue = p_oPlot->GetMeanAnnualPrecip();
    }

    m_iTimeElapsed = 0;
    m_iTimestepLength = mp_oSimManager->GetNumberOfYearsPerTimestep();

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
    stcErr.sFunction = "clClimateChange::GetData";
    throw(stcErr);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Action
///////////////////////////////////////////////////////////////////////////////
void clClimateChange::Action()
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  float fNewValue;

  fNewValue = m_fStartingValue + m_fB * pow(m_iTimeElapsed, m_fC);
  fNewValue = fmax(fNewValue, m_fMin);
  fNewValue = fmin(fNewValue, m_fMax);

  if (m_bIsTemp) {
    p_oPlot->SetMeanAnnualTemp(fNewValue);
  } else {

    if (m_bUpdateAllPrecip) {
      //We're updating seasonal precip - find the proportion change in value
      float fCurrent = p_oPlot->GetMeanAnnualPrecip(),
            fPropChange = (fNewValue - fCurrent) / fCurrent,
            fSeasPrecip = p_oPlot->GetSeasonalPrecipitation();

      fSeasPrecip = fSeasPrecip + (fPropChange * fSeasPrecip);

      p_oPlot->SetSeasonalPrecipitation(fSeasPrecip);
    }

    p_oPlot->SetMeanAnnualPrecip(fNewValue);

  }
  m_iTimeElapsed += m_iTimestepLength;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
/////////////////////////////////////////////////////////////////////////////*/
void clClimateChange::SetNameData(std::string sNameString) {
 try {

   //Check the string passed and set the flags accordingly
   if (sNameString.compare("TemperatureClimateChange") == 0) {
     m_bIsTemp = true;
   } else if (sNameString.compare("PrecipitationClimateChange") == 0) {
     m_bIsTemp = false;
   } else {
     modelErr stcErr;
     stcErr.iErrorCode = BAD_DATA;
     std::stringstream s;
     s << "Unrecognized behavior name \"" << sNameString << "\".";
     stcErr.sFunction = "clClimateChange::SetNameData";
     stcErr.sMoreInfo = s.str();
     throw(stcErr);
   }
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clClimateChange::SetNameData";
   throw(stcErr);
 }
}

