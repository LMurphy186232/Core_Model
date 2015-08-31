//---------------------------------------------------------------------------
// GLIPoints.cpp
//---------------------------------------------------------------------------
#include "GLIPoints.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "LightOrg.h"
#include "Constants.h"
#include "Plot.h"
#include <stdio.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clGLIPoints::clGLIPoints( clSimManager * p_oSimManager ) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
     clLightBase( p_oSimManager ), clGLIBase( p_oSimManager )
{
  //Set the namestring
  m_sNameString = "GLIPointCreator";
  m_sXMLRoot = "GLIPointCreator";
  mp_pointsList = NULL;
  m_iNumPoints = 0;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clGLIPoints::~clGLIPoints()
{
  delete[] mp_pointsList;
}

////////////////////////////////////////////////////////////////////////////
// GetData()
////////////////////////////////////////////////////////////////////////////
void clGLIPoints::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
  	using namespace std;
    clLightBase::GetData( p_oDoc );
    ReadParameterFileData( p_oDoc );
    SetUpBrightnessArray();
    DoSetupCalculations();

    fstream oOut( m_sFileName.c_str(), ios::trunc | ios::out );
    oOut << "Points file for parameter file " << mp_oSimManager->GetParFilename() << "\n";
    oOut.close();
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
    stcErr.sFunction = "clGLIPoints::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoSetupCalculations()
////////////////////////////////////////////////////////////////////////////
void clGLIPoints::DoSetupCalculations()
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
}


////////////////////////////////////////////////////////////////////////////
// SetUpBrightnessArray()
////////////////////////////////////////////////////////////////////////////
void clGLIPoints::SetUpBrightnessArray()
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

    if ( p_oGli->m_iNumAltAng == m_iNumAltAng &&
         p_oGli->m_iNumAziAng == m_iNumAziAng &&
         p_oGli->m_iMinAngRow == m_iMinAngRow &&
         fabs(p_oGli->m_fAzimuthOfNorth - m_fAzimuthOfNorth) < 0.001) {

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
// ReadParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clGLIPoints::ReadParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  DOMNodeList * p_oPointsList; //for getting the list of points
  DOMNode * p_oDocNode; //a search result
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc),
             *p_oPoint;
  XMLCh *sVal;
  char *cData;
  float fPlotLenX = p_oPlot->GetXPlotLength(),
        fPlotLenY = p_oPlot->GetYPlotLength();
  int j; //loop counter

  m_iNumAltAng = 0;
  m_iNumAziAng = 0;
  m_fMinSunAngle = 0;

  //Get the filename
  FillSingleValue( p_oElement, "li_GLIPointsFilename", &m_sFileName, true );
  //Make sure the file extension is on - if not, append it
  if (std::string::npos == m_sFileName.find(TEXT_FILE_EXT)) m_sFileName += TEXT_FILE_EXT;

  //*******************************
  //Get the points list
  //*******************************
  sVal = XMLString::transcode( "li_GLIPoint" );
  p_oPointsList = p_oElement->getElementsByTagName( sVal );
  XMLString::release(&sVal);
  m_iNumPoints = p_oPointsList->getLength();
  if ( 0 == m_iNumPoints )
  {
    modelErr stcErr;
    stcErr.iErrorCode = DATA_MISSING;
    stcErr.sFunction = "clGLIPoints::ReadParameterFileData" ;
    stcErr.sMoreInfo = "li_GLIPoint";
    throw( stcErr );
  }

  mp_pointsList = new stcCoords[m_iNumPoints];
  for ( j = 0; j < m_iNumPoints; j++ )
  {
    p_oDocNode = p_oPointsList->item( j );
    p_oPoint = ( DOMElement * ) p_oDocNode;
    sVal = XMLString::transcode( "x" );
    cData = XMLString::transcode( p_oPoint->getAttributeNode( sVal )->getNodeValue() );
    mp_pointsList[j].fX = atof( cData );
    delete[] cData; cData = NULL;
    XMLString::release(&sVal);
    sVal = XMLString::transcode( "y" );
    cData = XMLString::transcode( p_oPoint->getAttributeNode( sVal )->getNodeValue() );
    mp_pointsList[j].fY = atof( cData );
    delete[] cData; cData = NULL;
    XMLString::release(&sVal);
    sVal = XMLString::transcode( "h" );
    cData = XMLString::transcode( p_oPoint->getAttributeNode( sVal )->getNodeValue() );
    mp_pointsList[j].fHeight = atof( cData );
    delete[] cData; cData = NULL;
    XMLString::release(&sVal);
    //Make sure that the values are within the plot
    if ( mp_pointsList[j].fX >= fPlotLenX || mp_pointsList[j].fX < 0 ||
         mp_pointsList[j].fY >= fPlotLenY || mp_pointsList[j].fY < 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clGLIPoints::ReadParameterFileData" ;
      std::stringstream s;
      s << "Point value outside plot - " << mp_pointsList[j].fX << ", "
        << mp_pointsList[j].fY;
      stcErr.sMoreInfo = s.str();
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Make sure the height is not negative
    if ( mp_pointsList[j].fHeight < 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clGLIPoints::ReadParameterFileData" ;
      stcErr.sMoreInfo = "Height values cannot be negative.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  } //end of for (j = 0; j < m_iNumPoints; j++)

  //*******************************
  // Get light values
  //*******************************
  //Get our values
  //Number of alitude angles
  FillSingleValue( p_oElement, "li_numAltGrids", & m_iNumAltAng, true );
  //Number of azimuth angles
  FillSingleValue( p_oElement, "li_numAziGrids", & m_iNumAziAng, true );
  //Minimum sun angle
  FillSingleValue( p_oElement, "li_minSunAngle", & m_fMinSunAngle, true );
  //Azimuth of north - not required for backwards compatibility
  FillSingleValue( p_oElement, "li_AziOfNorth", & m_fAzimuthOfNorth, false );

  //Validate the data
  if ( 0 >= m_iNumAltAng )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGLIPoints::ReadParameterFileData" ;
    stcErr.sMoreInfo = "The number of sky altitude divisions must be greater than 0.";
    throw( stcErr );
  }

  if ( 0 >= m_iNumAziAng )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGLIPoints::ReadParameterFileData" ;
    stcErr.sMoreInfo = "The number of sky azimuth divisions must be greater than 0.";
    throw( stcErr );
  }

  if (0 > m_fAzimuthOfNorth || (2.0 * M_PI) < m_fAzimuthOfNorth) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGLIPoints::ReadParameterFileData" ;
    stcErr.sMoreInfo = "Azimuth of north must be between 0 and 2PI.";
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clGLIPoints::Action()
{
  //Call the base class function - if this is the hooked clLightBase object,
  //light assignments proceed normally
  clLightBase::Action();
  try
  {
  	using namespace std;
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oShaders; //neighborhood trees who can shade each target
    fstream out( m_sFileName.c_str(), ios::app | ios::out );
    clTree * p_oNeighbor; //shading neighbors
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clAllometry * p_oAllom = p_oPop->GetAllometryObject();
    char cQuery[75]; //format search strings into this
    float fGli, //global light index - end result of all this math
         fMaxSearchRad; //maximum search radius
    int iPoint, i, j; //loop counters

    //Write header row for this timestep
    //Write data to file
    out << "\nTimestep: " << mp_oSimManager->GetCurrentTimestep() << "\nX\tY\tHeight\tGLI\n";

    //Loop through the points
    for ( iPoint = 0; iPoint < m_iNumPoints; iPoint++ )
    {
      //Initialize photo array
      for ( i = 0; i < m_iNumAltAng; i++ )
        for ( j = 0; j < m_iNumAziAng; j++ )
          mp_fPhoto[i][j] = 1.0;

      //Calculate search radius to look for shading neighbors
      fMaxSearchRad = ( mp_oLightOrg->GetMaxTreeHeight() - mp_pointsList[iPoint].fHeight )
           * m_fRcpTanMinAng + p_oAllom->GetMaxCrownRadius();

      //Get a list of all trees that are within the search radius and taller than
      //the fish-eye photo height
      sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", fMaxSearchRad, " FROM x=", mp_pointsList[iPoint].fX, "y=",
           mp_pointsList[iPoint].fY, "::height=", mp_pointsList[iPoint].fHeight );
      p_oShaders = p_oPop->Find( cQuery );

      p_oNeighbor = p_oShaders->NextTree();
      while ( p_oNeighbor != NULL )
      {

        //Skip seedlings, which don't shade
        if ( clTreePopulation::seedling != p_oNeighbor->GetType() )

             //Add the effect of the neighbor to the simulated fisheye photo
               AddTreeToGliFishEye( mp_pointsList[iPoint].fX, mp_pointsList[iPoint].fY, mp_pointsList[iPoint].fHeight, p_oNeighbor, p_oPlot, p_oPop, p_oAllom );

        p_oNeighbor = p_oShaders->NextTree();
      } //end of while (neighbor != NULL)

      //Calculate GLI
      fGli = 0.0;
      for ( i = 0; i < m_iNumAltAng; i++ )
        for ( j = 0; j < m_iNumAziAng; j++ )
          fGli += ( mp_fPhoto[i] [j] * mp_fBrightness[i] [j] );
      fGli *= 100;

      //Write the point
      out << mp_pointsList[iPoint].fX << "\t" << mp_pointsList[iPoint].fY << "\t" << mp_pointsList[iPoint].fHeight << "\t" << fGli << "\n";

    }
    out.close();
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
    stcErr.sFunction = "clGLIPoints::Action" ;
    throw( stcErr );
  }
}
