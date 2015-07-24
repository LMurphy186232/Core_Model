//---------------------------------------------------------------------------
#include "AverageLight.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Constants.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clAverageLight::clAverageLight( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ),
  clBehaviorBase( p_oSimManager ), clLightBase(p_oSimManager)
{

  //Set the namestring
  m_sNameString = "averagelightshell";
  m_sXMLRoot = "AverageLight";

  mp_oGLIMapGrid = NULL;
  mp_oAvgLightGrid = NULL;
  m_bAvgCalculated = false;
  m_iAvgLightCode = -1;
  m_iMapLightCode = -1;

  //Versions
  m_fVersionNumber = 1;
  m_fMinimumVersionNumber = 1;

  m_bNeedsCommonParameters = false;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clAverageLight::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    std::string sGridName = ""; //Name of GLI Map grid to average
    std::string sTemp;
    int i;

    m_bAvgCalculated = false;

    //Find the first GLI Map grid
    for (i = 0; i < mp_oSimManager->GetNumberOfGrids(); i++) {
      sTemp = mp_oSimManager->GetGridObject(i)->GetName();
      if (sTemp.find("GLI Map") != std::string::npos) {
        sGridName = sTemp;
        break;
      }
    }

    //Get the pointer to the light grid
    mp_oGLIMapGrid = mp_oSimManager->GetGridObject( sGridName.c_str() );

    if ( !mp_oGLIMapGrid )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clAverageLight::DoShellSetup" ;
      stcErr.sMoreInfo = "GLI Map behavior must be used with the Average Light behavior.";
      throw( stcErr );
    }
    m_iMapLightCode = mp_oGLIMapGrid->GetFloatDataCode("GLI");
    if ( -1 == m_iMapLightCode )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clAverageLight::DoShellSetup" ;
      stcErr.sMoreInfo = "GLI Map grid set up incorrectly.";
      throw( stcErr );
    }

    //Has the average light grid been set up yet?
    mp_oAvgLightGrid = mp_oSimManager->GetGridObject( "Average Light" );

    if ( !mp_oAvgLightGrid )
    {
      //Create the grid with one float data member
      mp_oAvgLightGrid = mp_oSimManager->CreateGrid( "Average Light", 0, 1, 0, 0 );
      //Register the data member - called "GLI"
      m_iAvgLightCode = mp_oAvgLightGrid->RegisterFloat( "GLI" );

    } else {
      m_iAvgLightCode = mp_oAvgLightGrid->GetFloatDataCode("GLI");
      if ( -1 == m_iAvgLightCode )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clAverageLight::DoShellSetup" ;
        stcErr.sMoreInfo = "Average grid set up incorrectly. Can't find data member \"GLI\".";
        throw( stcErr );
      }
    }
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
    stcErr.sFunction = "clAverageLight::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcLightValue
////////////////////////////////////////////////////////////////////////////
float clAverageLight::CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop)
{
  //Check to see if the light average has been calculated this timestep, and
  //do it if it hasn't
  if (!m_bAvgCalculated) CalculateAverage(p_oPop);

  float fX, fY, fGli;
  int iSpecies = p_oTree->GetSpecies(),
      iType = p_oTree->GetType();

  //Get the location of the tree and find the light value for that location
  p_oTree->GetValue( p_oPop->GetXCode( iSpecies, iType ), & fX );
  p_oTree->GetValue( p_oPop->GetYCode( iSpecies, iType ), & fY );

  //Check to see if there's already a value in the quadrat array for this
  //tree's quadrat - if so, assign it and exit
  mp_oAvgLightGrid->GetValueAtPoint( fX, fY, m_iAvgLightCode, & fGli );
  return fGli;
}

////////////////////////////////////////////////////////////////////////////
// CalculateAverage()
////////////////////////////////////////////////////////////////////////////
void clAverageLight::CalculateAverage(clTreePopulation *p_oPop)
{
  try
  {
    float fStartX, fStartY, fEndX, fEndY,
          fLengthXCells = mp_oAvgLightGrid->GetLengthXCells(),
          fLengthYCells = mp_oAvgLightGrid->GetLengthYCells(),
          fPlotLengthX = p_oPop->GetXPlotLength(),
          fPlotLengthY = p_oPop->GetYPlotLength(),
          fVal;
    int iNumXCells = mp_oAvgLightGrid->GetNumberXCells(),
        iNumYCells = mp_oAvgLightGrid->GetNumberYCells(),
        iX, iY; //loop counter

    //Calculate each cell's light level
    for ( iX = 0; iX < iNumXCells; iX++ )
    {
      fStartX = iX * fLengthXCells;
      fEndX = ((iX + 1) * fLengthXCells) - VERY_SMALL_VALUE;
      if (fEndX >= fPlotLengthX) fEndX = fPlotLengthX - VERY_SMALL_VALUE;

      for ( iY = 0; iY < iNumYCells; iY++ )
      {
        fStartY = iY * fLengthYCells;
        fEndY = ((iY + 1) * fLengthYCells) - VERY_SMALL_VALUE;
        if (fEndY >= fPlotLengthY) fEndY = fPlotLengthY - VERY_SMALL_VALUE;

        fVal = mp_oGLIMapGrid->GetAverageFloatValue(fStartX, fStartY, fEndX, fEndY, m_iMapLightCode);
        mp_oAvgLightGrid->SetValueOfCell(iX, iY, m_iAvgLightCode, fVal);
      }
    }

    m_bAvgCalculated = true;
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
    stcErr.sFunction = "clAverageLight::CalculateAverage" ;
    throw( stcErr );
  }
}
