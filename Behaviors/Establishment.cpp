//---------------------------------------------------------------------------
#include "Establishment.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Grid.h"
#include "GrowthOrg.h"
#include "Plot.h"
#include "ModelMath.h"
#include <stdio.h>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clEstablishment::clEstablishment( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "Establishment";
    m_sXMLRoot = "Establishment";

    mp_iSeedGridCode = NULL;
    mp_oSeedGrid = NULL;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;
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
    stcErr.sFunction = "clEstablishment::clEstablishment" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clEstablishment::~clEstablishment()
{
  delete[] mp_iSeedGridCode;
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clEstablishment::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    std::stringstream sLabel;
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Get the dispersed seeds grid
    mp_oSeedGrid = mp_oSimManager->GetGridObject("Dispersed Seeds");
    if ( NULL == mp_oSeedGrid )
    {
      modelErr stcErr;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      stcErr.sFunction = "clEstablishment::GetData" ;
      stcErr.sMoreInfo = "Disperse behaviors are required if establishment is to be used.";
      throw( stcErr );
    }
    mp_iSeedGridCode = new short int[iNumSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      sLabel << "seeds_" << mp_iWhatSpecies[i];
      mp_iSeedGridCode[mp_iWhatSpecies[i]] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
      if ( -1 == mp_iSeedGridCode[mp_iWhatSpecies[i]] )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clEstablishment::GetData" ;
        stcErr.sMoreInfo = "A disperse behavior is required for all species using establishment.";
        throw( stcErr );
      }
      sLabel.str("");
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
    stcErr.sFunction = "clEstablishment::GetData" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Action
///////////////////////////////////////////////////////////////////////////////
void clEstablishment::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );;
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    float fSeedX, fSeedY, //coordinates of seed
    fXOrig, fYOrig, //coordinates of origin of a grid cell
    fXCellLength = mp_oSeedGrid->GetLengthXCells(), //length of grid's cells in X direction
    fYCellLength = mp_oSeedGrid->GetLengthYCells(), //length of grid's cells in Y direction
    fXEdgeLength, //length of the cells in the last row of the plot, in the X direction
    fYEdgeLength, //length of the cells in the last row of the plot, in the Y direction
    fXThisCellLength, //length of current cell in X direction
    fYThisCellLength, //length of current cell in X direction
    fXCellEnd, fYCellEnd, //ending coordinates of cell
    fNumSeeds; //number of seeds per grid
    int iNumXCells = mp_oSeedGrid->GetNumberXCells(), iNumYCells = mp_oSeedGrid->GetNumberYCells(), iSp, i, j, iX, iY; //loop counters

    //Get the length of the edge cells
    fXEdgeLength = p_oPlot->GetXPlotLength() - (fXCellLength * floor(p_oPlot->GetXPlotLength() / fXCellLength));
    fXEdgeLength = (fXEdgeLength == 0 ? fXCellLength : fXEdgeLength);
    fYEdgeLength = p_oPlot->GetYPlotLength() - (fYCellLength * floor(p_oPlot->GetYPlotLength() / fYCellLength));
    fYEdgeLength = (fYEdgeLength == 0 ? fYCellLength : fYEdgeLength);
    //Reduce them down 1 cm to make sure no trees get too close to the edge of
    //the plot - this ensures that output writing won't round them up to the
    //plot length
    fXEdgeLength -= 0.01;
    fYEdgeLength -= 0.01;

    //Loop through each grid cell
    for ( iX = 0; iX < iNumXCells; iX++ )
    {

      if (iX == iNumXCells - 1) fXThisCellLength = fXEdgeLength;
      else fXThisCellLength = fXCellLength;
      for ( iY = 0; iY < iNumYCells; iY++ )
      {
        if (iY == iNumYCells - 1) fYThisCellLength = fYEdgeLength;
        else fYThisCellLength = fYCellLength;

        //Get the origin of this cell (min X and Y values)
        fXOrig = fXCellLength * iX;
        fYOrig = fYCellLength * iY;
        fXCellEnd = fXOrig + fXThisCellLength;
        fYCellEnd = fYOrig + fYThisCellLength;

        //Loop through each behavior species
        for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        {

          iSp = mp_iWhatSpecies[i];

          //Get the number of seeds in this cell for this species
          mp_oSeedGrid->GetValueOfCell( iX, iY, mp_iSeedGridCode[iSp], & fNumSeeds );
          fNumSeeds = clModelMath::RandomRound(fNumSeeds);

          //Disperse the seeds produced
          for ( j = 0; j < fNumSeeds; j++ )
          {
            //Get a random X and Y value for this seed within the grid cell
            fSeedX = fXOrig + ( clModelMath::GetRand() * fXThisCellLength );
            fSeedY = fYOrig + ( clModelMath::GetRand() * fYThisCellLength );

            while (fSeedX < fXOrig || fSeedY < fYOrig ||
                fSeedX >= fXCellEnd || fSeedY >= fYCellEnd) {
              fSeedX = fXOrig + ( clModelMath::GetRand() * fXThisCellLength );
              fSeedY = fYOrig + ( clModelMath::GetRand() * fYThisCellLength );
            }

            //Create the new seedling - allow a random diameter at 10 cm
            p_oPop->CreateTree( fSeedX, fSeedY, iSp, clTreePopulation::seedling, 0 );
          } //end of for (i = 0; i < fNumSeeds; i++)
        }
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
    stcErr.sFunction = "clEstablishment::Action" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// TimestepCleanup
///////////////////////////////////////////////////////////////////////////////
void clEstablishment::TimestepCleanup()
{
  float fNumSeeds = 0;
  int iNumXCells = mp_oSeedGrid->GetNumberXCells(),
  iNumYCells = mp_oSeedGrid->GetNumberYCells(),
  iSp, iX, iY; //loop counters

  for ( iX = 0; iX < iNumXCells; iX++ )
  {
    for ( iY = 0; iY < iNumYCells; iY++ )
    {
      for ( iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++ )
      {
        mp_oSeedGrid->SetValueOfCell( iX, iY, mp_iSeedGridCode[mp_iWhatSpecies[iSp]], fNumSeeds );
      }
    }
  }
}
