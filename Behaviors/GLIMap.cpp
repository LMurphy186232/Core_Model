//---------------------------------------------------------------------------
// GLIMap.cpp
//---------------------------------------------------------------------------
#include "GLIMap.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "LightOrg.h"
#include "Allometry.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clGLIMap::clGLIMap( clSimManager * p_oSimManager ) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
     clLightBase( p_oSimManager ), clGLIBase( p_oSimManager )
{
  //Set the namestring
  m_sNameString = "GLIMapCreator";
  m_sXMLRoot = "GLIMapCreator";

  m_iGridGliCode = -1;
  mp_oMapGrid = NULL;
  m_fLightHeight = 0;
  m_fMaxSearchDistance = 0;
}

////////////////////////////////////////////////////////////////////////////
// GetData()
////////////////////////////////////////////////////////////////////////////
void clGLIMap::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clLightBase::GetData(p_oDoc);
    ReadParameterFileData( p_oDoc );
    SetUpBrightnessArray();
    DoSetupCalculations();
    SetUpGrid();
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
    stcErr.sFunction = "clGLIMap::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// SetUpGrid()
////////////////////////////////////////////////////////////////////////////
void clGLIMap::SetUpGrid()
{
  float fGLI; //for setting initial value
  short int iNumXCells, iNumYCells, //number X and Y grid cells
       i, j; //loop counters
  mp_oMapGrid = mp_oSimManager->GetGridObject( "GLI Map" );

  if ( !mp_oMapGrid )
  {
    //Create the grid with one float data member
    mp_oMapGrid = mp_oSimManager->CreateGrid( "GLI Map", 0, 1, 0, 0 );
    //Register the data member - called "GLI"
    m_iGridGliCode = mp_oMapGrid->RegisterFloat( "GLI" );
  }
  else
  {
    //Get the data member code
    m_iGridGliCode = mp_oMapGrid->GetFloatDataCode( "GLI" );
    if ( -1 == m_iGridGliCode )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGLIMap::SetUpGrid" ;
      stcErr.sMoreInfo = "\"GLI Map\" grid was incorrectly set up in the parameter file.  Missing float \"GLI\".";
      throw( stcErr );
    }
  }

  //Set all the values to -1
  iNumXCells = mp_oMapGrid->GetNumberXCells();
  iNumYCells = mp_oMapGrid->GetNumberYCells();
  fGLI = 0;
  for ( i = 0; i < iNumXCells; i++ )
    for ( j = 0; j < iNumYCells; j++ )
      mp_oMapGrid->SetValueOfCell( i, j, m_iGridGliCode, fGLI );

}

////////////////////////////////////////////////////////////////////////////
// DoSetupCalculations()
////////////////////////////////////////////////////////////////////////////
void clGLIMap::DoSetupCalculations()
{
  float fAngChunk;
  short int iHalfAzi, //for partitioning the sky hemisphere
       i; //loop counter

  m_fAziChunkConverter = m_iNumAziAng / 360.0; //force float conversion
  m_fRcpTanMinAng = 1 / ( tan( m_fMinSunAngle ) );

  mp_fAziSlope = new float[m_iNumAziAng];
  iHalfAzi = m_iNumAziAng / 2;
  //Get the size of each azimuth chunk in radians
  fAngChunk = ( 2.0 * M_PI ) / m_iNumAziAng;
  for ( i = 0; i < iHalfAzi; i++ )
  {
    //slope = tan of azimuth angle
    mp_fAziSlope[i] = 1 / ( tan( fAngChunk * ( i + 0.5 ) ) );
    mp_fAziSlope[i + iHalfAzi] = mp_fAziSlope[i];
  }

  //Calculate search radius to look for shading neighbors
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  m_fMaxSearchDistance = ( mp_oLightOrg->GetMaxTreeHeight() - m_fLightHeight ) * m_fRcpTanMinAng + p_oPop->GetAllometryObject()->GetMaxCrownRadius();
}


////////////////////////////////////////////////////////////////////////////
// SetUpBrightnessArray()
////////////////////////////////////////////////////////////////////////////
void clGLIMap::SetUpBrightnessArray()
{

  clBehaviorBase * p_oTemp; //for searching for a behavior
  clLightBase * p_oGli; //gli light object
  short int i, j; //loop counters
  bool bBrightnessFilled = false;

  //Declare the brightness and photo arrays
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

    if ( p_oGli->m_iNumAltAng == m_iNumAltAng && p_oGli->m_iNumAziAng == m_iNumAziAng && p_oGli->m_iMinAngRow == m_iMinAngRow )
    {

      //Good!  We can assume that their photo brightness array is done, and
      //copy (if it wasn't done the sky resolution data wouldn't match)
      for ( i = 0; i < m_iNumAltAng; i++ )
        for ( j = 0; j < m_iNumAziAng; j++ )
          mp_fBrightness[i] [j] = p_oGli->mp_fBrightness[i] [j];

      bBrightnessFilled = true;

    }
  }

  if ( false == bBrightnessFilled )
  {
    //We couldn't get what we needed from clGLILight - try clQuadratGLI
    p_oTemp = mp_oSimManager->GetBehaviorObject( "quadratglilightshell" );
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

             bBrightnessFilled = true;
      }
    }
  }

  if ( false == bBrightnessFilled )
  {

    //We couldn't steal the array - so create our own sky brightness array
    PopulateGLIBrightnessArray();

  }
}



////////////////////////////////////////////////////////////////////////////
// SetUpLightOrg()
////////////////////////////////////////////////////////////////////////////
/*void clGLIMap::SetUpLightOrg( xercesc::DOMDocument * p_oDoc )
{

  clBehaviorBase * p_oTempBehavior; //for looking for light shells
  char cShellMarker[] = "lightshell"; //string we're looking for
  int iNumBehaviors, i; //total number of behaviors
  bool bOtherLightObjs = false; //flag for whether we found a light shell

  //Go through all the behaviors to pick out the light shells - should have
  //the string "lightshell" in their namestrings
  iNumBehaviors = mp_oSimManager->GetNumberOfBehaviors();
  for ( i = 0; i < iNumBehaviors; i++ )
  {
    p_oTempBehavior = mp_oSimManager->GetBehaviorObject( i );
    if ( NULL != strstr( p_oTempBehavior->GetName(), cShellMarker ) )
    {
      bOtherLightObjs = true;
      break;
    } //end of if (NULL != strstr(cBehaviorName, cShellMarker))
  } //end of for (i = 0; i < iNumBehaviors; i++)

  //If there are other light behaviors, the mp_oLightOrg pointer will already
  //be valid and this function does nothing
  if ( true == bOtherLightObjs ) return;

  //There are no other light objects; cause mp_oLightOrg to do its setup
  mp_oLightOrg->DoSetup( mp_oSimManager, p_oDoc );
} */



////////////////////////////////////////////////////////////////////////////
// ReadParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clGLIMap::ReadParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

  m_iNumAltAng = 0;
  m_iNumAziAng = 0;
  m_fMinSunAngle = 0;

  //Get the photo height value - required
  FillSingleValue( p_oElement, "li_mapLightHeight", & m_fLightHeight, true );

  //Get our values - none required because they might be in another light tag
  //Number of alitude angles
  FillSingleValue( p_oElement, "li_numAltGrids", & m_iNumAltAng, true );
  //Number of azimuth angles
  FillSingleValue( p_oElement, "li_numAziGrids", & m_iNumAziAng, true );
  //Minimum sun angle
  FillSingleValue( p_oElement, "li_minSunAngle", & m_fMinSunAngle, true );

  //Validate the data
  if ( 0 >= m_iNumAltAng )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGLIMap::ReadParameterFileData" ;
    stcErr.sMoreInfo = "The number of sky altitude divisions must be greater than 0.";
    throw( stcErr );
  }

  if ( 0 >= m_iNumAziAng )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGLIMap::ReadParameterFileData" ;
    stcErr.sMoreInfo = "The number of sky azimuth divisions must be greater than 0.";
    throw( stcErr );
  }

  if ( 0 > m_fLightHeight )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGLIMap::ReadParameterFileData" ;
    stcErr.sMoreInfo = "The height of the GLI point for the GLI map cannot be less than 0.";
    throw( stcErr );
  }
}



////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clGLIMap::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oShaders; //neighborhood trees who can shade each target
    clTree * p_oNeighbor; //shading neighbors
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clAllometry * p_oAllom = p_oPop->GetAllometryObject();
    char cQuery[75]; //format search strings into this
    float fGli, //global light index - end result of all this math
         fX, fY; //holders for the tree's X and Y location
    int iNumXCells = mp_oMapGrid->GetNumberXCells(), iNumYCells = mp_oMapGrid->GetNumberYCells(),
        iX, iY, i, j; //loop counters

    for ( iX = 0; iX < iNumXCells; iX++ )
    {
      for ( iY = 0; iY < iNumYCells; iY++ )
      {

        //First find the point at which to calculate gli - center of the grid
        //cell - place in fX and fY
        mp_oMapGrid->GetPointOfCell( iX, iY, & fX, & fY );

        //Initialize photo array
        for ( i = 0; i < m_iNumAltAng; i++ )
          for ( j = 0; j < m_iNumAziAng; j++ )
            mp_fPhoto[i] [j] = 1.0;

        //Get a list of all trees that are within the search radius and taller than
        //the fish-eye photo height
        sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fMaxSearchDistance, " FROM x=", fX, "y=", fY,
             "::height=", m_fLightHeight );
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
        mp_oMapGrid->SetValueAtPoint( fX, fY, m_iGridGliCode, fGli );
      }
    }
    //Call the base class function - if this is the hooked clLightBase object,
    //light assignments proceed normally
    clLightBase::Action();
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
    stcErr.sFunction = "clGLIMap::Action" ;
    throw( stcErr );
  }
}

