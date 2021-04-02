//---------------------------------------------------------------------------
#include "BasalAreaLight.h"
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
clBasalAreaLight::clBasalAreaLight( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ),
  clBehaviorBase( p_oSimManager ), clLightBase(p_oSimManager)
{

  //Set the namestring
  m_sNameString = "basalarealightshell";
  m_sXMLRoot = "BasalAreaLight";

  //Versions
  m_fVersionNumber = 1;
  m_fMinimumVersionNumber = 1;

  m_bNeedsCommonParameters = false;

  mp_fStatus = NULL;
  mp_oLightGrid = NULL;

  mp_iSpeciesTypes = NULL;
  m_fA = 0;
  m_fConiferB = 0;
  m_fAngiospermB = 0;
  m_fConiferC = 0;
  m_fAngiospermC = 0;
  m_fSigma = 0;
  m_fMinDbh = 0;
  m_fRadius = 0;
  m_fChangeThreshold = 0;
  m_iGridLightCode = -1;
  m_iGridConBACode = -1;
  m_iGridAngBACode = -1;
  m_bGridUpdated = false;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clBasalAreaLight::~clBasalAreaLight()
{
  delete[] mp_iSpeciesTypes;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clBasalAreaLight::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  int *p_iTemp = NULL;
  try
  {
  	clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    float fLight, fConBA, fAngBA;
    int i, j, iNumXCells, iNumYCells,
        iNumSpecies = p_oPop->GetNumberOfSpecies();
    bool bMap;

    p_iTemp = new int[iNumSpecies];

    m_bGridUpdated = false;

    mp_iSpeciesTypes = new iTreeType[iNumSpecies];

    //**********************************************
    //Read values from the parameter file
    //**********************************************
    //GLI mean value "a" parameter
    FillSingleValue( p_oElement, "li_baLightA", & m_fA, true );

    //Conifer "b" parameter
    FillSingleValue( p_oElement, "li_baConiferLightB", & m_fConiferB, true );

    //Angiosperm "b" parameter
    FillSingleValue( p_oElement, "li_baAngiospermLightB", & m_fAngiospermB, true );

    //Conifer "c" parameter
    FillSingleValue( p_oElement, "li_baConiferLightC", & m_fConiferC, true );
    //Error if this is equal to 0
    if (0 == m_fConiferC) {
      modelErr stcErr;
      stcErr.sFunction = "clBasalAreaLight::DoShellSetup" ;
      stcErr.sMoreInfo = "Basal area light conifer \"c\" parameter cannot equal 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Angiosperm "c" parameter
    FillSingleValue( p_oElement, "li_baAngiospermLightC", & m_fAngiospermC, true );
    //Error if this is equal to 0
    if (0 == m_fAngiospermC) {
      modelErr stcErr;
      stcErr.sFunction = "clBasalAreaLight::DoShellSetup" ;
      stcErr.sMoreInfo = "Basal area light angiosperm \"c\" parameter cannot equal 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Sigma parameter for lognormal PDF
    FillSingleValue( p_oElement, "li_baLightSigma", & m_fSigma, true );

    //Basal area change threshold for grid cells for recalculating GLI
    FillSingleValue( p_oElement, "li_baLightChangeThreshold", & m_fChangeThreshold, true );
    //Error if this is less than 0
    if (m_fChangeThreshold < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clBasalAreaLight::DoShellSetup" ;
      stcErr.sMoreInfo = "Basal area light change threshold cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Density light minimum DBH (cm) for a tree counting towards the density
    FillSingleValue( p_oElement, "li_baLightMinDBH", & m_fMinDbh, true );
    //Error if this is less thano 0
    if (m_fMinDbh < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clBasalAreaLight::DoShellSetup" ;
      stcErr.sMoreInfo = "Basal area light minimum DBH cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Search radius for neighbors
    FillSingleValue( p_oElement, "li_baLightSearchRadius", & m_fRadius, true );
    //Error if this is less than 0
    if (m_fRadius < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clBasalAreaLight::DoShellSetup" ;
      stcErr.sMoreInfo = "Basal area light radius cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Whether a species is an angiosperm or conifer
    FillSpeciesSpecificValue( p_oElement, "li_baTreeType", "li_bttVal", p_iTemp, p_oPop, true );
    for (i = 0; i < iNumSpecies; i++) {
      if (angiosperm == p_iTemp[i]) mp_iSpeciesTypes[i] = angiosperm;
      else if (conifer == p_iTemp[i]) mp_iSpeciesTypes[i] = conifer;
      else {
      	modelErr stcErr;
        stcErr.sFunction = "clBasalAreaLight::DoShellSetup" ;
        std::stringstream s;
        s << "Unrecognized basal area light tree type: " << p_iTemp[i];
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //**********************************************
    //Set up the light grid
    //**********************************************
    //Is there already a grid from the parameter file?
    mp_oLightGrid = mp_oSimManager->GetGridObject( "Basal Area Light" );

    if ( !mp_oLightGrid )
    {
      //Create the grid with three float data members
      mp_oLightGrid = mp_oSimManager->CreateGrid( "Basal Area Light", 0, 3, 0, 0 );
      //Register the data member called "Light"
      m_iGridLightCode = mp_oLightGrid->RegisterFloat( "Light" );
      //Register the data member called "Con BA"
      m_iGridConBACode = mp_oLightGrid->RegisterFloat( "Con BA" );
      //Register the data member called "Ang BA"
      m_iGridAngBACode = mp_oLightGrid->RegisterFloat( "Ang BA" );
    }
    else
    {
      //Get the data member codes
      m_iGridLightCode = mp_oLightGrid->GetFloatDataCode( "Light" );
      m_iGridConBACode = mp_oLightGrid->GetFloatDataCode( "Con BA" );
      m_iGridAngBACode = mp_oLightGrid->GetFloatDataCode( "Ang BA" );
      if ( -1 == m_iGridLightCode || -1 == m_iGridConBACode || -1 == m_iGridAngBACode)
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clBasalAreaLight::DoShellSetup" ;
        stcErr.sMoreInfo = "\"Basal Light\" grid was incorrectly set up in the parameter file.  Data members missing.";
        throw( stcErr );
      }
    }

    iNumXCells = mp_oLightGrid->GetNumberXCells();
    iNumYCells = mp_oLightGrid->GetNumberYCells();

    //Set the basal area totals of trees for each grid cell to below the
    //minimum change threshold, to be sure that a light level is calculated for
    //each cell the first timestep; but first make sure that there's no grid map
    bMap = false;
    for (i = 0; i < iNumXCells; i++) {
      for (j = 0; j < iNumYCells; j++) {
        mp_oLightGrid->GetValueOfCell(i, j, m_iGridLightCode, &fLight);
        mp_oLightGrid->GetValueOfCell(i, j, m_iGridConBACode, &fConBA);
        mp_oLightGrid->GetValueOfCell(i, j, m_iGridAngBACode, &fAngBA);
        if (fLight > 0 || fConBA > 0 || fAngBA > 0) {
          bMap = true;
          break;
        }
      }
    }

    if (!bMap) {
      fConBA = -1 - m_fChangeThreshold;
      for (i = 0; i < iNumXCells; i++) {
        for (j = 0; j < iNumYCells; j++) {
          mp_oLightGrid->SetValueOfCell(i, j, m_iGridConBACode, fConBA);
        }
      }
    }

    delete[] p_iTemp;
  }
  catch ( modelErr & err )
  {
    delete[] p_iTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_iTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_iTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBasalAreaLight::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcLightValue
////////////////////////////////////////////////////////////////////////////
float clBasalAreaLight::CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop)
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
void clBasalAreaLight::UpdateGridValues(clTreePopulation * p_oPop)
{
  try
  {
    clTreeSearch * p_oTrees; //saplings and adults
    clTree * p_oTree; //single tree to count
    std::stringstream sQuery; //format search string into this
    float fLight, //light level
          fDbh, //tree's DBH
          fBA, //tree's basal area
          fConBA, fAngBA, //conifer and angiosperm basal areas
          fLastConBA, fLastAngBA, //last timestep's conifer and angiosperm ba
          fX, fY; //holders for the tree's X and Y location
    int iSp, iTp,
        iNumXCells = mp_oLightGrid->GetNumberXCells(),
        iNumYCells = mp_oLightGrid->GetNumberYCells(),
        iX, iY; //loop counter

    //Calculate each cell's light level
    for ( iX = 0; iX < iNumXCells; iX++ )
    {
      for ( iY = 0; iY < iNumYCells; iY++ )
      {
        fAngBA = 0;
        fConBA = 0;

        //Get all neighbors within the specified radius and calculate
        //basal area
        mp_oLightGrid->GetPointOfCell( iX, iY, & fX, & fY );
        sQuery << "distance=" << m_fRadius << "FROM x=" << fX << "y=" <<
                fY << "::height=" << 0.0;
        p_oTrees = p_oPop->Find(sQuery.str());
        sQuery.str("");
        p_oTree = p_oTrees->NextTree();
        while (NULL != p_oTree) {

          //Get the tree's DBH and make sure it's eligible
          iSp = p_oTree->GetSpecies();
          iTp = p_oTree->GetType();
          if (iTp == clTreePopulation::sapling ||
              iTp == clTreePopulation::adult) {

            p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);

            if (fDbh >= m_fMinDbh) {

              //Get tree's basal area
              fBA = clModelMath::CalculateBasalArea(fDbh);

              //Increment the cell's basal area total
              if (angiosperm == mp_iSpeciesTypes[iSp])
                fAngBA += fBA;
              else
                fConBA += fBA;
            }
              }
          p_oTree = p_oTrees->NextTree();
        }

        //Has the total basal area changed by more than the minimum threshold?
        mp_oLightGrid->GetValueOfCell(iX, iY, m_iGridConBACode, &fLastConBA);
        mp_oLightGrid->GetValueOfCell(iX, iY, m_iGridAngBACode, &fLastAngBA);
        if (fabs((fAngBA + fConBA) - (fLastConBA + fLastAngBA)) > m_fChangeThreshold) {

          //Use the density to calculate a mean GLI
          fLight = GetMeanGLI(fConBA, fAngBA);

          //Calculate the location parameter, mu
          fLight = log(fLight) - (pow(m_fSigma, 2)/2);

          //Do a random draw on the lognormal PDF
          fLight = clModelMath::LognormalRandomDraw(fLight, m_fSigma);

          if (fLight < 0) fLight = 0;
          if (fLight > 100) fLight = 100;

          //Assign GLI to the grid
          mp_oLightGrid->SetValueOfCell( iX, iY, m_iGridLightCode, fLight );

          //Assign the new basal areas to the grid
          mp_oLightGrid->SetValueOfCell( iX, iY, m_iGridConBACode, fConBA );
          mp_oLightGrid->SetValueOfCell( iX, iY, m_iGridAngBACode, fAngBA );
        }
      }
    }
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
    stcErr.sFunction = "clBasalAreaLight::Action" ;
    throw( stcErr );
  }
}
