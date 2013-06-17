//---------------------------------------------------------------------------
#include "SailLight.h"
#include "ParsingFunctions.h"
#include "LightOrg.h"
#include "Plot.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "SimManager.h"
#include <stdio.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clSailLight::clSailLight(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clLightBase(p_oSimManager) {

  mp_fAltTans = NULL;
  m_sXMLRoot = "SailLight";
  m_iCrownDepthEqualsHeight = no;
  m_fMaxShadingRadius = 0;
  m_iPhotoDepth = clLightOrg::mid;

  //Set the namestring
  m_sNameString = "saillightshell";
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clSailLight::DoShellSetup(xercesc::DOMDocument * p_oDoc) {
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    float fAngle;
    int iTemp, iVal;
    short int i, j; //loop counters

    m_iNumAltAng = 90;
    m_iNumAziAng = 360;

    //Get our values
    //Minimum sun angle
    FillSingleValue( p_oElement, "li_skyMaskAngle", & m_fMinSunAngle, true );
    //Max shading radius
    FillSingleValue( p_oElement, "li_maxShadingRadius", & m_fMaxShadingRadius, true );
    //Crown depth fraction of height
    FillSingleValue( p_oElement, "li_crownFracOfHeight", & iTemp, true );
    //Throw error if photo crown depth does not have an allowed value
    if (yes == iTemp)
    m_iCrownDepthEqualsHeight = yes;
    else if (no == iTemp)
    m_iCrownDepthEqualsHeight = no;
    else
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSailLight::DoShellSetup" ;
      std::stringstream s;
      s << "li_crownFracOfHeight has invalid value \"" << iTemp << "\".";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }

    //Photo crown depth
    iVal = clLightOrg::top;
    FillSingleValue( p_oElement, "li_heightOfFishEyePhoto", & iVal, false );
    //Throw error if photo crown depth does not have an allowed value
    if ( iVal != clLightOrg::mid && iVal != clLightOrg::top )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clLightOrg::GetParameterFileData" ;
      std::stringstream s;
      s << "li_heightOfFishEyePhoto has invalid value \"" << iVal << "\".";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }
    if (iVal == clLightOrg::mid) m_iPhotoDepth = clLightOrg::mid;
    else m_iPhotoDepth = clLightOrg::top;

    //Declare the sky brightness and photo arrays
    mp_fBrightness = new float * [m_iNumAltAng];
    mp_fPhoto = new float * [m_iNumAltAng];
    for ( i = 0; i < m_iNumAltAng; i++ )
    {
      mp_fBrightness[i] = new float[m_iNumAziAng];
      mp_fPhoto[i] = new float[m_iNumAziAng];
      for ( j = 0; j < m_iNumAziAng; j++ )
      mp_fBrightness[i] [j] = 0;
    }

    m_iMinAngRow = (int)floor( m_fMinSunAngle ) - 1;
    if ( m_iMinAngRow < 0 ) m_iMinAngRow = 0;

    //Fill the sky brightness array
    PopulateSailLightBrightnessArray();

    //Declare the array - sized at number of altitude angles
    mp_fAltTans = new float[m_iNumAltAng];

    //Populate altitude tangents array
    //For each array bucket, calculate the tangent to the degree that forms its
    //lower bound (array index 0 = 0 degrees, array index 30 = 30 degrees).
    //Watch out for 90!  Set it to 10000.
    for ( i = 0; i < m_iNumAltAng; i++ )
    {
      if ( i == 90 )
      {
        mp_fAltTans[i] = 10000;
      }
      else
      {
        //Convert i to radians
        fAngle = i * CONVERT_TO_RADIANS;
        mp_fAltTans[i] = tan( fAngle );
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
    stcErr.sFunction = "clSailLight::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcLightValue()
////////////////////////////////////////////////////////////////////////////
float clSailLight::CalcLightValue(clTree * p_oTree, clTreePopulation * p_oPop) {
  clTreeSearch * p_oShaders; //neighborhood trees who can shade each target
  clTree * p_oNeighbor; //shading neighbors
  clPlot * p_oPlot; //plot object - for calculating distances
  clAllometry * p_oAllom = p_oPop->GetAllometryObject();
  char cQuery[75]; //format search strings into this
  float fTreeHeight, //height of target tree - if fisheye is taken at mid-crown
  //this must be calculated
  fFracShade, //fraction shade - the end result of all this math
  fTargetX, fTargetY; //holders for the tree's X and Y location
  int i, j; //loop counter variables
  short int iSpecies, iType; //for holding species and type of a tree

  //Get plot object
  p_oPlot = mp_oSimManager->GetPlotObject();

  //Initialize photo array
  for ( i = 0; i < m_iNumAltAng; i++ )
    for ( j = 0; j < m_iNumAziAng; j++ )
      mp_fPhoto[i] [j] = 1.0;

  //Target tree information
  iSpecies = p_oTree->GetSpecies();
  iType = p_oTree->GetType();
  p_oTree->GetValue( p_oPop->GetXCode( iSpecies, iType ), & fTargetX );
  p_oTree->GetValue( p_oPop->GetYCode( iSpecies, iType ), & fTargetY );

  //Calculate the height of the fish-eye photo, either mid or top crown
  p_oTree->GetValue( p_oPop->GetHeightCode( iSpecies, iType ), & fTreeHeight );
  if (m_iPhotoDepth == clLightOrg::mid ) {
    if ( clTreePopulation::adult == iType ) {
      fTreeHeight = fTreeHeight - (0.5 * p_oAllom->CalcAdultCrownDepth( p_oTree ));
    } else if (clTreePopulation::sapling == iType ) { //sapling
      fTreeHeight = fTreeHeight - (0.5 * p_oAllom->CalcSaplingCrownDepth( p_oTree));
    }
  }
  //else ; seedlings are already OK with full tree height

  //Get a list of all trees that are within the search radius and taller than
  //the fish-eye photo height
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fMaxShadingRadius, " FROM x=", fTargetX, "y=", fTargetY,
      "::height=", fTreeHeight );
  p_oShaders = p_oPop->Find( cQuery );

  p_oNeighbor = p_oShaders->NextTree();
  while ( p_oNeighbor != NULL )
  {

    //Skip seedlings, which don't shade, and the target tree if returned
    if ( clTreePopulation::seedling != p_oNeighbor->GetType() && p_oNeighbor != p_oTree )
    {

      //Add the effect of the neighbor to the simulated fisheye photo
      AddTreeToSailFishEye( fTargetX, fTargetY, fTreeHeight, p_oNeighbor, p_oPlot, p_oPop, p_oAllom );

    }

    p_oNeighbor = p_oShaders->NextTree();
  } //end of while (neighbor != NULL)

  //Write photo array
  //        fstream brightness("Photo.xls", ios::trunc | ios::out);
  //    brightness << "For X = " << fTargetX << " Y = " << fTargetY << "\n"; brightness << "Segment";
  //    for (i = 0; i < m_iNumAltAng; i++) brightness << "\t" << i; for (i = 0; i < m_iNumAziAng; i++) { brightness << "\n" << i;
  //    for (j = 0; j < m_iNumAltAng; j++) brightness << "\t" << mp_fPhoto[j][i]; } brightness.close();

  //Calculate fraction shade
  fFracShade = 0.0;
  for ( i = 0; i < m_iNumAltAng; i++ )
    for ( j = 0; j < m_iNumAziAng; j++ )
      fFracShade += ( mp_fPhoto[i] [j] * mp_fBrightness[i] [j] );
  fFracShade = 1 - fFracShade;
  if (fFracShade < 0) fFracShade = 0;
  if (fFracShade > 1) fFracShade = 1;
  return fFracShade;
}

///////////////////////////////////////////////////////////////////////////////
// AddTreeToSailFishEye()
/////////////////////////////////////////////////////////////////////////////*/
void clSailLight::AddTreeToSailFishEye(const float & fTargetX,
    const float & fTargetY, const float & fTargetHeight, clTree * p_oNeighbor,
    clPlot * p_oPlot, clTreePopulation * p_oPop, clAllometry * p_oAllom) {
  try
  {
    float fCrownAngle, //degrees of sky blocked by 0.5 canopy width
    //(1 radius); = atan(crown radius/distance to tree)
    fCrownBase, //height of base of neighbor crown
    fTemp, //for making intermediate calculations
    fDistance, //distance between target and neighbor trees
    fLightExtCoeff, //light extinction coefficient of shading neighbor
    fNeighCanrad, //canopy radius of neighbor tree
    fNeighX, fNeighY, //x and y coordinates of shading neighbor
    fNeighHeight, //height of neighbor tree
    fNeighDbh, //DBH of neighbor tree
    fNeighAzi; //azimuth to neighbor
    int iIsDead; //whether or not a neighbor's dead
    short int iAziStart = 0, iAziEnd = 0, //start and end sky azimuth coords
    iAltStart = 0, iAltEnd = 0, //start and end sky altitude coords
    iSpecies, iType, //species and type of neighbor tree
    iDeadCode, //code for whether or not a neighbor's dead
    i, j;

    iSpecies = p_oNeighbor->GetSpecies();
    iType = p_oNeighbor->GetType();

    //Make sure the neighbor's not dead
    iDeadCode = p_oPop->GetIntDataCode( "dead", iSpecies, iType );
    if ( -1 != iDeadCode )
    {
      p_oNeighbor->GetValue( iDeadCode, & iIsDead );
      if ( iIsDead > notdead ) return;
    }

    //Get X and Y coordinates, height, height of crown base, and canopy radius
    //of shading neighbor
    p_oNeighbor->GetValue(p_oPop->GetXCode(iSpecies, iType), & fNeighX );
    p_oNeighbor->GetValue(p_oPop->GetYCode(iSpecies, iType), & fNeighY );
    p_oNeighbor->GetValue(p_oPop->GetDbhCode(iSpecies, iType), & fNeighDbh );
    p_oNeighbor->GetValue(p_oPop->GetHeightCode(iSpecies, iType), &fNeighHeight);
    if ( p_oNeighbor->GetType() == clTreePopulation::sapling )
      fNeighCanrad = p_oAllom->CalcSaplingCrownRadius( p_oNeighbor );
    else
      fNeighCanrad = p_oAllom->CalcAdultCrownRadius( p_oNeighbor );

    //Get distance between tree centers
    fDistance = p_oPlot->GetDistance( fTargetX, fTargetY, fNeighX, fNeighY );

    //If distance is 0, exit; the geometry doesn't work
    if ( 0 == fDistance ) return;

    //>>>>>>>>>>>>>>>Beginning and ending altitude angles<<<<<<<<<<<<<<<<<
    //User can choose whether crown goes to the ground.  If they don't, then
    //if the base of the neighbor crown is higher than target tree height,
    //calc. altitude angle to it; otherwise use start angle of 0

    //Top altitude angle
    fTemp = ( fNeighHeight - fTargetHeight ) / fDistance; //this is tan(alt)
    for ( i = 0; i < m_iNumAltAng; i++ )
    {
      if ( mp_fAltTans[i] <= fTemp && mp_fAltTans[i + 1] > fTemp )
      {
        iAltEnd = i; break;
      }
    }
    iAltEnd--;
    //If this angle is below the minimum sun angle, there's
    //no shading to add; exit
    if ( iAltEnd < m_iMinAngRow ) return;

    if ( m_iCrownDepthEqualsHeight == no )
    {
      //Crown base
      if ( clTreePopulation::adult == iType )
      fCrownBase = fNeighHeight - p_oAllom->CalcAdultCrownDepth( p_oNeighbor );
      else //sapling
      fCrownBase = fNeighHeight - p_oAllom->CalcSaplingCrownDepth( p_oNeighbor );

      //If crown base is higher than tree height - calc altitude start
      if ( fCrownBase >= fTargetHeight )
      {
        fTemp = ( fCrownBase - fTargetHeight ) / fDistance; //tan(alt)
        for ( i = 0; i < m_iNumAltAng; i++ )
        {
          if ( mp_fAltTans[i] <= fTemp && mp_fAltTans[i + 1] > fTemp )
          {
            iAltStart = i; break;
          }
        }
        iAltStart--;
        if ( iAltStart < m_iMinAngRow ) iAltStart = m_iMinAngRow;
      }
      else
      iAltStart = m_iMinAngRow; //crown base is lower than tree height
    }
    else
    iAltStart = m_iMinAngRow; //crown depth extends to ground

    //>>>>>>>>>>>>>>>Beginning and ending azimuth angles<<<<<<<<<<<<<<<<<
    //Find out the coordinates of the area of sky blocked by neighbor
    //Start with azimuth - find azimuth to neighbor and angle of sky blocked
    //to come back with coords
    //Start by getting the azimuth angle to the center of the neighbor, in
    //degrees
    fNeighAzi = p_oPlot->GetFastAzimuthAngle( fTargetX, fTargetY, fNeighX, fNeighY );
    //Calculate the amount of the horizon that half of the neighbor crown
    //subtends and convert to degrees
    fCrownAngle = ( atan( fNeighCanrad / fDistance ) ) * CONVERT_TO_DEGREES;

    //Correct when a tree is near north - start may be less than zero or end more
    //than 360 degrees
    iAziStart = (int)floor( fNeighAzi - fCrownAngle ) - 1; //start angle
    if ( iAziStart < 0 ) iAziStart += 360;

    iAziEnd = (int)floor( fNeighAzi + fCrownAngle ) - 1; //end angle
    if ( iAziEnd >= 360 ) iAziEnd -= 360;

    //Use coords to loop thru chunk of sky array and mark it as blocked
    fLightExtCoeff = GetLightExtinctionCoefficient( p_oNeighbor );

    if ( iAziEnd >= iAziStart )
    {

      for ( j = iAziStart; j <= iAziEnd; j++ )
      for ( i = iAltStart; i <= iAltEnd; i++ )
      mp_fPhoto[i] [j] *= fLightExtCoeff;
      //correct when the tree straddles north
    }
    else
    {
      for ( i = iAltStart; i <= iAltEnd; i++ )
      {
        for ( j = iAziStart; j < m_iNumAziAng; j++ )
        mp_fPhoto[i] [j] *= fLightExtCoeff;
        for ( j = 0; j <= iAziEnd; j++ )
        mp_fPhoto[i] [j] *= fLightExtCoeff;
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
    stcErr.sFunction = "clSailLight::AddTreeToSailFishEye" ;
    throw( stcErr );
  }
}
