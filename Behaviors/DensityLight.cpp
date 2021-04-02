//---------------------------------------------------------------------------
#include "DensityLight.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "ModelMath.h"
#include <stdio.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clDensityLight::clDensityLight( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), 
  clBehaviorBase( p_oSimManager ), clLightBase(p_oSimManager)
{

  //Set the namestring
  m_sNameString = "densitylightshell";

  //Allowed file types
  m_iNumAllowedTypes = 2;
  mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
  mp_iAllowedFileTypes[0] = parfile;
  mp_iAllowedFileTypes[1] = detailed_output;

  //Versions
  m_fVersionNumber = 1;
  m_fMinimumVersionNumber = 1;

  m_bNeedsCommonParameters = false;

  m_iGridCountCode = -1;
  m_bGridUpdated = false;
  m_fMinDbh = 0;
  m_iGridLightCode = -1;
  m_fSigma = 0;
  m_fC = 0;
  m_fB = 0;
  mp_oLightGrid = NULL;
  m_iChangeThreshold = 0;
  m_fA = 0;
}


////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clDensityLight::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    float fLight;
    int i, j, iNumXCells, iNumYCells, iCount;
    bool bMap;

    m_bGridUpdated = false;

    //**********************************************
    //Read values from the parameter file
    //**********************************************
    //GLI mean value "a" parameter
    FillSingleValue( p_oElement, "li_densLightA", & m_fA, true );

    //GLI mean value "b" parameter
    FillSingleValue( p_oElement, "li_densLightB", & m_fB, true );

    //GLI mean value "c" parameter
    FillSingleValue( p_oElement, "li_densLightC", & m_fC, true );
    //Error if this is equal to 0
    if (0 == m_fC) {
      modelErr stcErr;
      stcErr.sFunction = "clDensityLight::GetData" ;
      stcErr.sMoreInfo = "Density light \"c\" parameter cannot equal 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Density light sigma parameter for lognormal PDF
    FillSingleValue( p_oElement, "li_densLightSigma", & m_fSigma, true );

    //Density light threshold for new trees in grid cell for recalculating GLI
    FillSingleValue( p_oElement, "li_densLightChangeThreshold", & m_iChangeThreshold, true );
    //Error if this is less than 0
    if (m_iChangeThreshold < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clDensityLight::GetData" ;
      stcErr.sMoreInfo = "Density light change threshold cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Density light minimum DBH (cm) for a tree counting towards the density
    FillSingleValue( p_oElement, "li_densLightMinDBH", & m_fMinDbh, true );
    //Error if this is equal to 0
    if (m_fMinDbh < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clDensityLight::GetData" ;
      stcErr.sMoreInfo = "Density light minimum DBH cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //**********************************************
    //Set up the light grid
    //**********************************************
    //Is there already a grid from the parameter file?
    mp_oLightGrid = mp_oSimManager->GetGridObject( "Density Light" );

    if ( !mp_oLightGrid )
    {
      //Create the grid with one float and one int data member
      mp_oLightGrid = mp_oSimManager->CreateGrid( "Density Light", 1, 1, 0, 0 );
      //Register the data member called "Light"
      m_iGridLightCode = mp_oLightGrid->RegisterFloat( "Light" );
      //Register the data member called "Count"
      m_iGridCountCode = mp_oLightGrid->RegisterInt( "Count" );
    }
    else
    {
      //Get the data member codes
      m_iGridLightCode = mp_oLightGrid->GetFloatDataCode( "Light" );
      m_iGridCountCode = mp_oLightGrid->GetIntDataCode( "Count" );
      if ( -1 == m_iGridLightCode || -1 == m_iGridCountCode )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensityLight::GetData" ;
        stcErr.sMoreInfo = "\"Density Light\" grid was incorrectly set up in the parameter file.  Data members missing.";
        throw( stcErr );
      }
    }

    //Verify that the grid cell lengths divide evenly into the plot lengths
    iNumXCells = mp_oLightGrid->GetNumberXCells();
    iNumYCells = mp_oLightGrid->GetNumberYCells();
    if (p_oPlot->GetXPlotLength() - (iNumXCells * mp_oLightGrid->GetLengthXCells()) > 0.01 ||
        p_oPlot->GetYPlotLength() - (iNumYCells * mp_oLightGrid->GetLengthYCells()) > 0.01) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDensityLight::GetData" ;
      stcErr.sMoreInfo = "\"Density Light\" grid cell lengths must divide evenly into the plot lengths.";
      throw( stcErr );
    }

    //Set the counts of trees for each grid cell to below the minimum change
    //threshold, to be sure that a light level is calculated for each cell
    //the first timestep; but first make sure that there's no grid map
    bMap = false;
    for (i = 0; i < iNumXCells; i++) {
      for (j = 0; j < iNumYCells; j++) {
        mp_oLightGrid->GetValueOfCell(i, j, m_iGridLightCode, &fLight);
        mp_oLightGrid->GetValueOfCell(i, j, m_iGridCountCode, &iCount);
        if (fLight > 0 || iCount > 0) {
          bMap = true;
          break;
        }
      }
    }

    if (!bMap) {
      iCount = -1 - m_iChangeThreshold;
      for (i = 0; i < iNumXCells; i++) {
        for (j = 0; j < iNumYCells; j++) {
          mp_oLightGrid->SetValueOfCell(i, j, m_iGridCountCode, iCount);
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
    stcErr.sFunction = "clDensityLight::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcLightValue
////////////////////////////////////////////////////////////////////////////
float clDensityLight::CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop)
{
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();
  float fX, fY, fGLI;

  //Check to see if the light grid has been updated this timestep, and do it
  //if it hasn't
  if (!m_bGridUpdated) UpdateGridValues(p_oPop);

  //Get the tree's location, then return the GLI from that cell of "Density
  //Light"
  p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
  p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);
  mp_oLightGrid->GetValueAtPoint(fX, fY, m_iGridLightCode, &fGLI);
  return fGLI;
}

////////////////////////////////////////////////////////////////////////////
// UpdateGridValues()
////////////////////////////////////////////////////////////////////////////
void clDensityLight::UpdateGridValues(clTreePopulation * p_oPop)
{
  try
  {
    clTreeSearch * p_oTrees; //saplings and adults
    clTree * p_oTree; //single tree to count
    std::stringstream sQuery; //format search string into this
    int **p_iCountGrid; //holds our density for this timestep
    float fLight, //light level
          fDbh, //tree's DBH
          fX, fY, //holders for the tree's X and Y location
          fXCellLength = mp_oLightGrid->GetLengthXCells(),
          fYCellLength = mp_oLightGrid->GetLengthYCells();
    int iSp, iTp,
        iNumXCells = mp_oLightGrid->GetNumberXCells(),
        iNumYCells = mp_oLightGrid->GetNumberYCells(),
        iCount, iX, iY; //loop counter

    p_iCountGrid = new int*[iNumXCells];
    for (iX = 0; iX < iNumXCells; iX++ ) {
      p_iCountGrid[iX] = new int[iNumYCells];
      for ( iY = 0; iY < iNumYCells; iY++ ) {
        p_iCountGrid[iX][iY] = 0;
      }
    }

    //Get all saplings and adults and count them into our temporary grid
    sQuery << "type=" << clTreePopulation::sapling << "," << clTreePopulation::adult;
    p_oTrees = p_oPop->Find(sQuery.str());
    p_oTree = p_oTrees->NextTree();
    while (NULL != p_oTree) {

      //Get the tree's DBH and make sure it's eligible
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();
      p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);

      if (fDbh >= m_fMinDbh) {
        //Get the tree's location
        p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
        p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

        //Increment the cell's count
        p_iCountGrid[(int)(fX / fXCellLength)][(int)(fY / fYCellLength)]++;
      }
      p_oTree = p_oTrees->NextTree();
    }

    //Calculate each cell's light level
    for ( iX = 0; iX < iNumXCells; iX++ )
    {
      for ( iY = 0; iY < iNumYCells; iY++ )
      {
        //Has the count changed by more than the minimum threshold?
        mp_oLightGrid->GetValueOfCell(iX, iY, m_iGridCountCode, &iCount);
        if (fabs(p_iCountGrid[iX][iY] - iCount) > m_iChangeThreshold) {

          //Use the density to calculate a mean GLI
          fLight = GetMeanGLI(p_iCountGrid[iX][iY]);

          //Calculate the location parameter, mu
          fLight = log(fLight) - (pow(m_fSigma, 2)/2);

          //Do a random draw on the lognormal PDF
          fLight = clModelMath::LognormalRandomDraw(fLight, m_fSigma);

          if (fLight < 0) fLight = 0;
          if (fLight > 100) fLight = 100;

          //Assign GLI to the grid
          mp_oLightGrid->SetValueOfCell( iX, iY, m_iGridLightCode, fLight );

          //Assign the new density to the grid
          mp_oLightGrid->SetValueOfCell( iX, iY, m_iGridCountCode, p_iCountGrid[iX][iY] );
        }
      }
    }

    //Delete our temporary density grid
    for (iX = 0; iX < iNumXCells; iX++)
      delete[] p_iCountGrid[iX];

    delete[] p_iCountGrid;

    m_bGridUpdated = true;
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
    stcErr.sFunction = "clDensityLight::Action" ;
    throw( stcErr );
  }
}
