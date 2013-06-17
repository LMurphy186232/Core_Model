//---------------------------------------------------------------------------
#include "GLILight.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "LightOrg.h"
#include "Allometry.h"
#include <stdio.h>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clGliLight::clGliLight(clSimManager *p_oSimManager) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clLightBase( p_oSimManager ), clGLIBase(p_oSimManager) {

  //Set the namestring
  m_sNameString = "glilightshell";
  m_sXMLRoot = "GLILight";

  m_iPhotoDepth = clLightOrg::mid;
}

/////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clGliLight::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  try {
    DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
    clBehaviorBase *p_oTemp;   //for searching for a behavior
    clLightBase *p_oGli;       //quadrat gli light object
    float fAngChunk;
    int iVal;
    short int iHalfAzi,        //for partitioning the sky hemisphere
    i, j;            //loop counters

    //Get our values
    //Number of alitude angles
    FillSingleValue(p_oElement, "li_numAltGrids", &m_iNumAltAng, true);
    //Number of azimuth angles
    FillSingleValue(p_oElement, "li_numAziGrids", &m_iNumAziAng, true);
    //Minimum sun angle
    FillSingleValue(p_oElement, "li_minSunAngle", &m_fMinSunAngle, true);

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
    mp_fBrightness = new float*[m_iNumAltAng];
    mp_fPhoto = new float*[m_iNumAltAng];
    for (i = 0; i < m_iNumAltAng; i++)  {
      mp_fBrightness[i] = new float[m_iNumAziAng];
      mp_fPhoto[i] = new float[m_iNumAziAng];
      for (j = 0; j < m_iNumAziAng; j++)
        mp_fBrightness[i][j] = 0;
    }

    m_fSinMinSunAng = sin(m_fMinSunAngle);
    m_iMinAngRow = (int)floor(m_fSinMinSunAng * m_iNumAltAng);
    m_fAziChunkConverter = m_iNumAziAng/360.0; //force float conversion
    m_fRcpTanMinAng = 1 / (tan(m_fMinSunAngle));

    mp_fAziSlope = new float[m_iNumAziAng];
    iHalfAzi = m_iNumAziAng/2;
    //Get the size of each azimuth chunk in radians
    fAngChunk = (2.0 * M_PI) / m_iNumAziAng;
    for (i = 0; i < iHalfAzi; i++) {
      //slope = tan of azimuth angle
      mp_fAziSlope[i] = 1/(tan(fAngChunk*(i + 0.5)));
      mp_fAziSlope[i + iHalfAzi] = mp_fAziSlope[i];
    }

    //Now - we need to fill the sky brightness array.  Check to see if the
    //"quadratglilightshell" object has been created
    p_oTemp = mp_oSimManager->GetBehaviorObject("quadratglilightshell");
    if (NULL != p_oTemp) {

      p_oGli = dynamic_cast<clLightBase*>(p_oTemp);

      if (p_oGli->m_iNumAltAng == m_iNumAltAng &&
          p_oGli->m_iNumAziAng == m_iNumAziAng) {

        //Good!  We can assume that their photo brightness array is done, and
        //copy (if it wasn't done the sky resolution data wouldn't match)
        for (i = 0; i < m_iNumAltAng; i++)
          for (j = 0; j < m_iNumAziAng; j++)
            mp_fBrightness[i][j] = p_oGli->mp_fBrightness[i][j];

      } else {

        //Create our own sky brightness array
        PopulateGLIBrightnessArray();

      }
    } else {   //p_oTemp is NULL
      //Create our own sky brightness array
      PopulateGLIBrightnessArray();
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGliLight::DoShellSetup";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcLightValue()
////////////////////////////////////////////////////////////////////////////
float clGliLight::CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop) {
  clTreeSearch *p_oShaders; //neighborhood trees who can shade each target
  clTree *p_oNeighbor; //shading neighbors
  clPlot *p_oPlot; //plot object - for calculating distances
  clAllometry *p_oAllom = p_oPop->GetAllometryObject();
  char cQuery[75]; //format search strings into this
  float fTreeHeight, //height of target tree - if fisheye is taken at mid-crown
  //this must be calculated
  fMaxSearchRad,  //maximum search radius
  fGli,   //global light index - the end result of all this math
  fX, fY; //holders for the tree's X and Y location
  int i, j; //loop counter variables
  short int iSpecies, iType; //for holding species and type of a tree

  //Get plot object
  p_oPlot = mp_oSimManager->GetPlotObject();

  //Initialize photo array
  for (i = 0; i < m_iNumAltAng; i++)
    for (j = 0; j < m_iNumAziAng; j++)
      mp_fPhoto[i][j] = 1.0;

  iSpecies = p_oTree->GetSpecies();
  iType = p_oTree->GetType();

  //Calculate the height of the fish-eye photo, either mid or top crown
  p_oTree->GetValue(p_oPop->GetHeightCode(iSpecies, iType), &fTreeHeight);
  if (m_iPhotoDepth == clLightOrg::mid) {
    if (clTreePopulation::adult == iType) {
      fTreeHeight = fTreeHeight -
          (0.5 * p_oAllom->CalcAdultCrownDepth(p_oTree));
    } else if (clTreePopulation::sapling == iType) {//sapling
      fTreeHeight = fTreeHeight -
          (0.5 * p_oAllom->CalcSaplingCrownDepth(p_oTree));
    }
    //else ; //seedlings - tree height is already OK
  }

  //Calculate search radius to look for shading neighbors
  fMaxSearchRad = (mp_oLightOrg->GetMaxTreeHeight() - fTreeHeight) *
      m_fRcpTanMinAng + p_oAllom->GetMaxCrownRadius();

  //Get a list of all trees that are within the search radius and taller than
  //the fish-eye photo height
  p_oTree->GetValue(p_oPop->GetXCode(iSpecies, iType), &fX);
  p_oTree->GetValue(p_oPop->GetYCode(iSpecies, iType), &fY);
  sprintf(cQuery, "%s%f%s%f%s%f%s%f", "distance=", fMaxSearchRad, " FROM x=",
      fX, "y=", fY, "::height=", fTreeHeight);
  p_oShaders = p_oPop->Find(cQuery);

  p_oNeighbor = p_oShaders->NextTree();
  while (p_oNeighbor != NULL) {

    //Skip seedlings, which don't shade, and the target tree if returned
    if (clTreePopulation::seedling != p_oNeighbor->GetType() &&
        p_oNeighbor != p_oTree) {

      //Add the effect of the neighbor to the simulated fisheye photo
      AddTreeToGliFishEye(fX, fY, fTreeHeight, p_oNeighbor, p_oPlot, p_oPop,
          p_oAllom);

    }

    p_oNeighbor = p_oShaders->NextTree();
  } //end of while (neighbor != NULL)

  //Write photo array
  /*   fstream photo("Gli Light Photo array.xls", ios::app | ios::out);
   photo << "\nPhoto array for tree X = " << fX << " Y = " << fY << "\n";
   photo << "Segment";
   for (i = 0; i < m_iNumAziAng; i++)
     photo << "\t" << i;
   for (i = 0; i < m_iNumAltAng; i++) {
     photo << "\n" << i;
     for (j = 0; j < m_iNumAziAng; j++)
       photo << "\t" << mp_fPhoto[i][j];
   }
   photo.close();*/

  //Calculate GLI
  fGli = 0.0;
  for (i = 0; i < m_iNumAltAng; i++)
    for (j = 0; j < m_iNumAziAng; j++)
      fGli += (mp_fPhoto[i][j] * mp_fBrightness[i][j]);
  fGli *= 100;
  return fGli;
}
