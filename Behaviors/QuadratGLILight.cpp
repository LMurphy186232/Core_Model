//---------------------------------------------------------------------------
#include "QuadratGLILight.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "LightOrg.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clQuadratGLILight::clQuadratGLILight(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clLightBase(p_oSimManager), clGLIBase(p_oSimManager) {

  //Set the namestring
  m_sNameString = "quadratglilightshell";
  m_sXMLRoot = "QuadratLight";

  //Set the default quadrat light height
  m_fLightHeight = 0.675;
  m_fMaxSearchRad = 0;
  m_iGridGliCode = -1;
  mp_oQuadrats = NULL;

  m_bCalcAll = false;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clQuadratGLILight::DoShellSetup(xercesc::DOMDocument * p_oDoc) {
  try
  {
    DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
    clBehaviorBase * p_oTemp; //for searching for a behavior
    clLightBase * p_oGli; //quadrat gli light object
    float fAngChunk;
    short int iHalfAzi, //for partitioning the sky hemisphere
    iNumXCells, iNumYCells, //number X and Y grid cells
    i, j; //loop counters

    //**********************************************
    //Read values from the parameter file
    //**********************************************
    m_iNumAltAng = 0;
    m_iNumAziAng = 0;
    m_fMinSunAngle = 0;

    //Number of alitude angles
    FillSingleValue( p_oElement, "li_numAltGrids", & m_iNumAltAng, true );
    //Number of azimuth angles
    FillSingleValue( p_oElement, "li_numAziGrids", & m_iNumAziAng, true );
    //Minimum sun angle
    FillSingleValue( p_oElement, "li_minSunAngle", & m_fMinSunAngle, true );
    //Height of quadrat light
    FillSingleValue( p_oElement, "li_quadratLightHeight", & m_fLightHeight, true );
    //Whether to calculate all values
    FillSingleValue( p_oElement, "li_quadratAllGLIs", & m_bCalcAll, true );

    //Validate the data
    if ( 0 >= m_iNumAltAng )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clQuadratGLILight::DoShellSetup" ;
      stcErr.sMoreInfo = "The number of sky altitude divisions must be greater than 0.";
      throw( stcErr );
    }

    if ( 0 >= m_iNumAziAng )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clQuadratGLILight::DoShellSetup" ;
      stcErr.sMoreInfo = "The number of sky azimuth divisions must be greater than 0.";
      throw( stcErr );
    }

    if ( 0 > m_fLightHeight )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clQuadratGLILight::DoShellSetup" ;
      stcErr.sMoreInfo = "The height of the GLI point for the GLI map cannot be less than 0.";
      throw( stcErr );
    }

    //**********************************************
    //Populate the sky brightness and photo arrays
    //**********************************************
    //Declare 'em
    mp_fBrightness = new float * [m_iNumAltAng];
    mp_fPhoto = new float * [m_iNumAltAng];
    for ( i = 0; i < m_iNumAltAng; i++ )
    {
      mp_fBrightness[i] = new float[m_iNumAziAng];
      mp_fPhoto[i] = new float[m_iNumAziAng];
      for ( j = 0; j < m_iNumAziAng; j++ )
      mp_fBrightness[i] [j] = 0;
    }

    m_fSinMinSunAng = sin( m_fMinSunAngle );
    m_iMinAngRow = (int)floor( m_fSinMinSunAng * m_iNumAltAng );

    //Now - we need to fill the sky brightness array.  Check to see if the
    //"glilightshell" object has been created
    p_oTemp = mp_oSimManager->GetBehaviorObject( "glilightshell" );
    if ( NULL != p_oTemp )
    {

      p_oGli = dynamic_cast < clLightBase * > ( p_oTemp );

      if ( p_oGli->m_iNumAltAng == m_iNumAltAng && p_oGli->m_iNumAziAng == m_iNumAziAng
          && p_oGli->m_iMinAngRow == m_iMinAngRow )
      {

        //Good!  We can assume that their photo brightness array is done, and
        //copy (if it wasn't done the sky resolution data wouldn't match)
        for ( i = 0; i < m_iNumAltAng; i++ )
        for ( j = 0; j < m_iNumAziAng; j++ )
        mp_fBrightness[i] [j] = p_oGli->mp_fBrightness[i] [j];

      }
      else
      {

        //Create our own sky brightness array
        PopulateGLIBrightnessArray();

      }
    }
    else
    { //p_oTemp is NULL
      //Create our own sky brightness array
      PopulateGLIBrightnessArray();
    }

    m_fAziChunkConverter = m_iNumAziAng / 360.0; //force float conversion
    m_fRcpTanMinAng = 1 / ( tan( m_fMinSunAngle ) );
    //Calculate search radius to look for shading neighbors
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    m_fMaxSearchRad = ( mp_oLightOrg->GetMaxTreeHeight() - m_fLightHeight ) * m_fRcpTanMinAng + p_oPop->GetAllometryObject()->GetMaxCrownRadius();

    mp_fAziSlope = new float[m_iNumAziAng];
    iHalfAzi = m_iNumAziAng / 2;
    //Get the size of each azimuth chunk in radians
    fAngChunk = ( 2.0 * M_PI ) / m_iNumAziAng;
    for ( i = 0; i < iHalfAzi; i++ )
    {
      //slope = tan of azimuth angle
      mp_fAziSlope[i] = 1 / (tan( fAngChunk * ( i + 0.5 ) ));
      mp_fAziSlope[i + iHalfAzi] = mp_fAziSlope[i];
    }

    //**********************************************
    //Set up the quadrat grid
    //**********************************************
    //Is there already a grid from the parameter file?
    mp_oQuadrats = mp_oSimManager->GetGridObject( "Quadrat GLI" );

    if ( !mp_oQuadrats )
    {
      //Create the grid with one float data member
      mp_oQuadrats = mp_oSimManager->CreateGrid( "Quadrat GLI", 0, 1, 0, 0,
          QUADRAT_CELL_SIZE, QUADRAT_CELL_SIZE );
      //Register the data member - called "GLI"
      m_iGridGliCode = mp_oQuadrats->RegisterFloat( "GLI" );
    }
    else
    {
      //Get the data member code
      m_iGridGliCode = mp_oQuadrats->GetFloatDataCode( "GLI" );
      if ( -1 == m_iGridGliCode )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clQuadratGLILight::DoShellSetup" ;
        stcErr.sMoreInfo = "Quadrat GLI grid was incorrectly set up in the parameter file.  Missing float \"GLI\".";
        throw( stcErr );
      }
    }

    //Set all the values to -1
    iNumXCells = mp_oQuadrats->GetNumberXCells();
    iNumYCells = mp_oQuadrats->GetNumberYCells();
    fAngChunk = -1; //dangerously reuse
    for ( i = 0; i < iNumXCells; i++ )
    for ( j = 0; j < iNumYCells; j++ )
    mp_oQuadrats->SetValueOfCell( i, j, m_iGridGliCode, fAngChunk );

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
    stcErr.sFunction = "clQuadratGLILight::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcLightValue()
////////////////////////////////////////////////////////////////////////////
float clQuadratGLILight::CalcLightValue(clTree * p_oTree,
    clTreePopulation * p_oPop) {
  clTreeSearch * p_oShaders; //neighborhood trees who can shade each target
  clTree * p_oNeighbor; //shading neighbors
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  clAllometry * p_oAllom = p_oPop->GetAllometryObject();
  char cQuery[75]; //format search strings into this
  float fGli, //global light index - end result of all this math
  fX, fY; //holders for the tree's X and Y location
  short int iSpecies, iType, //for holding species and type
  iXCell, iYCell, //quadrat numbers
  i, j; //loop counter

  iSpecies = p_oTree->GetSpecies();
  iType = p_oTree->GetType();
  p_oTree->GetValue( p_oPop->GetXCode( iSpecies, iType ), & fX );
  p_oTree->GetValue( p_oPop->GetYCode( iSpecies, iType ), & fY );

  //Check to see if there's already a value in the quadrat array for this
  //tree's quadrat - if so, assign it and exit
  mp_oQuadrats->GetValueAtPoint( fX, fY, m_iGridGliCode, & fGli );
  if ( fGli > 0 ) return fGli;

  //Nope - no value for this point yet - calculate the gli
  //First find the point at which to calculate gli - center of the tree's
  //quadrat - place in fX and fY
  mp_oQuadrats->GetCellOfPoint( fX, fY, & iXCell, & iYCell );
  mp_oQuadrats->GetPointOfCell( iXCell, iYCell, & fX, & fY );

  //Initialize photo array
  for ( i = 0; i < m_iNumAltAng; i++ )
    for ( j = 0; j < m_iNumAziAng; j++ )
      mp_fPhoto[i] [j] = 1.0;

  //Get a list of all trees that are within the search radius and taller than
  //the fish-eye photo height
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fMaxSearchRad, " FROM x=", fX, "y=", fY, "::height=", m_fLightHeight );
  p_oShaders = p_oPop->Find( cQuery );

  p_oNeighbor = p_oShaders->NextTree();
  while ( p_oNeighbor != NULL )
  {

    //Skip seedlings, which don't shade, and the target tree if returned
    if ( clTreePopulation::seedling != p_oNeighbor->GetType() )

      //Add the effect of the neighbor to the simulated fisheye photo
      AddTreeToGliFishEye( fX, fY, m_fLightHeight, p_oNeighbor, p_oPlot, p_oPop, p_oAllom );

    p_oNeighbor = p_oShaders->NextTree();
  } //end of while (neighbor != NULL)

  //Calculate GLI
  fGli = 0.0;
  for ( i = 0; i < m_iNumAltAng; i++ )
    for ( j = 0; j < m_iNumAziAng; j++ )
      fGli += ( mp_fPhoto[i] [j] * mp_fBrightness[i] [j] );
  fGli *= 100;

  //Assign GLI to the grid
  mp_oQuadrats->SetValueAtPoint( fX, fY, m_iGridGliCode, fGli );

  return fGli;
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clQuadratGLILight::Action() {
  //Call the base class function - if this is the hooked clLightBase object,
  //light assignments proceed normally
  clLightBase::Action();

  if (!m_bCalcAll)
    return;

  clTreeSearch * p_oShaders; //neighborhood trees who can shade each target
  clTree * p_oNeighbor; //shading neighbors
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  clTreePopulation *p_oPop =
      (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  clAllometry * p_oAllom = p_oPop->GetAllometryObject();
  char cQuery[75]; //format search strings into this
  float fGli, //global light index - end result of all this math
      fX, fY; //holders for the tree's X and Y location
  short int iNumXCells = mp_oQuadrats->GetNumberXCells(), iNumYCells =
      mp_oQuadrats->GetNumberXCells(), iXCell, iYCell, //quadrat numbers
      i, j; //loop counter

  for (iXCell = 0; iXCell < iNumXCells; iXCell++) {
    for (iYCell = 0; iYCell < iNumYCells; iYCell++) {

      mp_oQuadrats->GetValueOfCell(iXCell, iYCell, m_iGridGliCode, &fGli);
      if (fGli < 0) {

        //First find the point at which to calculate gli - center of the quadrat
        mp_oQuadrats->GetPointOfCell(iXCell, iYCell, &fX, &fY);

        //Initialize photo array
        for (i = 0; i < m_iNumAltAng; i++)
          for (j = 0; j < m_iNumAziAng; j++)
            mp_fPhoto[i] [j] = 1.0;

        //Get a list of all trees that are within the search radius and taller than
        //the fish-eye photo height
        sprintf(cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fMaxSearchRad,
            " FROM x=", fX, "y=", fY, "::height=", m_fLightHeight);
        p_oShaders = p_oPop->Find(cQuery);

        p_oNeighbor = p_oShaders->NextTree();
        while (p_oNeighbor != NULL) {

          //Skip seedlings, which don't shade
          if (clTreePopulation::seedling != p_oNeighbor->GetType() )

            //Add the effect of the neighbor to the simulated fisheye photo
            AddTreeToGliFishEye(fX, fY, m_fLightHeight, p_oNeighbor, p_oPlot,
                p_oPop, p_oAllom);

          p_oNeighbor = p_oShaders->NextTree();
        } //end of while (neighbor != NULL)

        //Calculate GLI
        fGli = 0.0;
        for (i = 0; i < m_iNumAltAng; i++)
          for (j = 0; j < m_iNumAziAng; j++)
            fGli += (mp_fPhoto[i] [j] * mp_fBrightness[i] [j] );
        fGli *= 100;

        //Assign GLI to the grid
        mp_oQuadrats->SetValueAtPoint(fX, fY, m_iGridGliCode, fGli);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// TimestepCleanup()
////////////////////////////////////////////////////////////////////////////
void clQuadratGLILight::TimestepCleanup() {
try
{
  //Set all the values to -1
  float fVal = -1;
  short int iNumXCells = mp_oQuadrats->GetNumberXCells();
  short int iNumYCells = mp_oQuadrats->GetNumberYCells();
  for ( int i = 0; i < iNumXCells; i++ )
  for ( int j = 0; j < iNumYCells; j++ )
  mp_oQuadrats->SetValueOfCell( i, j, m_iGridGliCode, fVal );
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
  stcErr.sFunction = "clQuadratGLILight::TimestepCleanup" ;
  throw( stcErr );
}
}
