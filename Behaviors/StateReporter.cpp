//---------------------------------------------------------------------------
// StateReporter.cpp
//---------------------------------------------------------------------------
#include "StateReporter.h"
#include "Plot.h"
#include "SimManager.h"
#include "Grid.h"
#include "Constants.h"

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clStateReporter::clStateReporter( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "StateReporter";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_oGrid = NULL;
    m_iPrecipGridCode = -1;
    m_iTempGridCode = -1;

  }
  catch ( modelErr & err )
  {
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
  	modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clStateReporter::clStateReporter" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clStateReporter::SetupGrid()
{
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();

  //Create the grid
  mp_oGrid = mp_oSimManager->CreateGrid( "State Variables",
                        0,                      //number of ints
                        2, //number of floats
                        0,                      //number of chars
                        0,                      //number of bools
                        p_oPlot->GetXPlotLength(),  //X cell length
                        p_oPlot->GetYPlotLength()); //Y cell length

  m_iTempGridCode = mp_oGrid->RegisterFloat("Temp.C");
  m_iPrecipGridCode = mp_oGrid->RegisterFloat("Precip.mm");
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clStateReporter::Action()
{
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  float fValue; //grid values

  fValue = p_oPlot->GetMeanAnnualPrecip();
  mp_oGrid->SetValueOfCell(0, 0, m_iPrecipGridCode, fValue);

  fValue = p_oPlot->GetMeanAnnualTemp();
  mp_oGrid->SetValueOfCell(0, 0, m_iTempGridCode, fValue);

}
