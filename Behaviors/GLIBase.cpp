//---------------------------------------------------------------------------
#include "GLIBase.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "LightOrg.h"
#include "Plot.h"
#include "SimManager.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor()
//////////////////////////////////////////////////////////////////////////////
clGLIBase::clGLIBase(clSimManager *p_oSimManager) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clLightBase(p_oSimManager)
{
  mp_fAziSlope = NULL;
  m_fRcpTanMinAng = 0;
  m_fAziChunkConverter = 0;
  m_fSinMinSunAng = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor()
//////////////////////////////////////////////////////////////////////////////
clGLIBase::~clGLIBase() {
  delete[] mp_fAziSlope;
}

///////////////////////////////////////////////////////////////////////////////
// AddTreeToGliFishEye()
//////////////////////////////////////////////////////////////////////////////
void clGLIBase::AddTreeToGliFishEye(const float &fTargetX, const float
    &fTargetY, const float &fTargetHeight, clTree *p_oNeighbor, clPlot *p_oPlot,
    clTreePopulation *p_oPop, clAllometry *p_oAllom)
{
  try {
    float fDistance,         //distance between target and neighbor trees
          fDistToNearEdge,   //distance to nearest edge of neighbor
          fNeighCanrad,      //canopy radius of neighbor tree
          fNeighX, fNeighY,  //x and y coordinates of shading neighbor
          fNeighHeight,      //height of neighbor tree
          fNeighDbh;         //DBH of neighbor tree
    int iIsDead;             //whether or not a neighbor's dead
    short int iSpecies,      //for getting species-specific values
              iDeadCode,     //code for whether or not a neighbor's dead
              iType;         //tree type

    iSpecies = p_oNeighbor->GetSpecies();
    iType = p_oNeighbor->GetType();

    //Make sure the neighbor's not dead - need to rethink this maybe?
    iDeadCode = p_oPop->GetIntDataCode("dead", iSpecies, iType);
    if (-1 != iDeadCode) {
      p_oNeighbor->GetValue(iDeadCode, &iIsDead);
      //if (iIsDead != notdead && iIsDead != natural) return;
      if (iIsDead != notdead) return;
    }

    //Get X and Y coordinates, height, height of crown base, and canopy radius
    //of shading neighbor
    p_oNeighbor->GetValue(p_oPop->GetXCode(iSpecies, iType), &fNeighX);
    p_oNeighbor->GetValue(p_oPop->GetYCode(iSpecies, iType), &fNeighY);
    p_oNeighbor->GetValue(p_oPop->GetDbhCode(iSpecies, iType), &fNeighDbh);
    p_oNeighbor->GetValue(p_oPop->GetHeightCode(iSpecies, iType), &fNeighHeight);
    if (iType == clTreePopulation::sapling)
      fNeighCanrad = p_oAllom->CalcSaplingCrownRadius(p_oNeighbor);
    else
      fNeighCanrad = p_oAllom->CalcAdultCrownRadius(p_oNeighbor);


    //Get distance between tree centers, and distance from target tree center
    //to both the near and far edge of the neighbor's crown
    fDistance = p_oPlot->GetDistance(fTargetX, fTargetY, fNeighX, fNeighY);
    fDistToNearEdge = fDistance - fNeighCanrad;

    //************************************************
    // Target tree is not under neighbor canopy
    //************************************************
    if (fDistToNearEdge > 0) {

      GLIFisheyeNoCanopyIntersect(fTargetX, fTargetY, fNeighX, fNeighY,
          fNeighCanrad, fNeighHeight, iSpecies, fTargetHeight,
          fDistToNearEdge, p_oNeighbor, p_oPlot, p_oAllom);

    } //end of target tree not under neighbor canopy

    /************************************************
      Target tree is under neighbor canopy
    ************************************************/
    else {

      GLIFisheyeCanopyIntersect(fTargetX, fTargetY, fNeighX, fNeighY,
          fNeighCanrad, fNeighHeight, iSpecies, fTargetHeight,
          p_oNeighbor, p_oPlot, p_oAllom);

    } //end of target is under neighbor canopy
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.sFunction = "clTradLight::FishEye";
    stcErr.iErrorCode = UNKNOWN;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// GLIFisheyeNoCanopyIntersect()
//////////////////////////////////////////////////////////////////////////////
void clGLIBase::GLIFisheyeNoCanopyIntersect(const float &fTargetX, const float
  &fTargetY, const float &fNeighX, const float &fNeighY, const float
  &fNeighCanrad, const float &fNeighHeight, const short int &iNeighSpecies,
  const float &fTargetHeight, const float &fDistToNearEdge,
  clTree *p_oNeighbor, clPlot *p_oPlot, clAllometry *p_oAllom) {

 try {
   float fSquareOfNearDist,      //square of the distance to the nearest point
                                 //that is a solution to the system of equations
         fSquareOfFarDist,       //square of the distance to the farthest point
                                 //that is a solution to the system of equations
         fFarX, fNearX, fFarY, fNearY,//two solutions of system of equations -
                                 //for the far and near sides of neighbor crown
         fA, fB, fC,             //variables in quadratic formula
         fXDist, fYDist,         //distance between two trees in X and Y planes
         fCrownBase,             //height of base of neighbor crown
         fLightExtCoeff,         //light extinction coefficient
         fNeighAzi,              //azimuth to neighbor
         fHeightDiff,            //difference in crown top heights
         fHeightDiffSquared,     //squared difference in heights
         fHeightDiffChunkConv,   //height diff combined with chunk conversion
                                 //factor for calculating altitude chunk
         fCrownBaseDiff,         //difference in height between neighbor crown
                                 //base and target tree crown top
         fCrownBaseDiffSquared,  //squared difference between crown base and ht.
         fCrownBaseDiffChunkConv,//crown base/height diff combined with chunk
                                 //conversion factor for calc. altitude chunk
         fAziChunkSlope;         //slope to azimuth chunk
   int iAziChunkIncrementer,     //increments the azi chunk - or decrements it,
                                 //depending on the direction we're searching
       iAltStart, iAltEnd,       //start and end azimuth chunk coordinates
       iHomeAziChunk,            //azimuth chunk the neighbor's trunk is in
       iAziChunk,                //azi chunk we're working with
       iChunkCounter = 0,        //how many azi chunks we've done
       iHalfTheAzis = m_iNumAziAng/2, //half the azimuth chunks
       i;                        //loop counter

   //Make sure this neighbor tree actually will shade
   //Calculate difference in height between target and neighbor
   fHeightDiff = fNeighHeight - fTargetHeight;
   if (m_fSinMinSunAng > fHeightDiff /
        sqrt((fHeightDiff * fHeightDiff) + (fDistToNearEdge * fDistToNearEdge)))
     return;

   //**************************************
   //Values we'll need
   //**************************************
   //Crown base
   if (clTreePopulation::sapling == p_oNeighbor->GetType()) {
     fCrownBase = fNeighHeight - p_oAllom->CalcSaplingCrownDepth(p_oNeighbor);
   } else {
     fCrownBase = fNeighHeight - p_oAllom->CalcAdultCrownDepth(p_oNeighbor);
   }
   //Difference in height sqared - pre-calc this to save calcs later
   fHeightDiffSquared = fHeightDiff * fHeightDiff;
   //Height difference times chunk conversion factor for altitude - saves calcs
   //later
   fHeightDiffChunkConv = fHeightDiff * m_iNumAltAng;
   //Height difference between neighbor crown base and target crown top
   fCrownBaseDiff = fCrownBase - fTargetHeight;
   //Square the above - saves calcs later
   fCrownBaseDiffSquared = fCrownBaseDiff * fCrownBaseDiff;
   //Crown base/height diff times chunk conversion factor for altitude
   fCrownBaseDiffChunkConv = fCrownBaseDiff * m_iNumAltAng;
   //Distance between the trees in the X and Y planes
   fXDist = p_oPlot->GetXDistance(fTargetX, fNeighX);
   fYDist = p_oPlot->GetYDistance(fTargetY, fNeighY);
   //Neighbor's light extinction coefficient
   fLightExtCoeff = GetLightExtinctionCoefficient(p_oNeighbor);
   //Start the azimuth searching to the west of the tree
   iAziChunkIncrementer = -1;
   //Slope of line to azimuth chunk - for the first time use actual slope to
   //ensure that we get a hit on the azimuth chunk
   fAziChunkSlope = (fXDist == 0) ? 100000.0 : fYDist / fXDist;

   //**************************************
   //Find the azimuth chunk containing the neighbor -
   //our starting point
   //**************************************
   //Get the azimuth to the neighbor
   fNeighAzi = p_oPlot->GetFastAzimuthAngle(fTargetX, fTargetY, fNeighX,
                   fNeighY);
   iHomeAziChunk = (int)floor(fNeighAzi * m_fAziChunkConverter);
   iAziChunk = iHomeAziChunk;

   //**************************************
   //Find all azimuth chunks blocked by the neighbor.  Check chunks on either
   //side of the starting point until we find them all - and for each one,
   //calculate the altitude chunks covered and add the whole business to the
   //photo array
   //**************************************

   while (1) {

     //We have to use linear algebra to solve the system of equations of
     //the circle of the neighbor's canopy and the line from the target to
     //the center of the azimuth chunk in question.  The slope of the line
     //is in mp_fAziSlope.  Do all calculations as though the target tree
     //is at the origin.

     //Plug in for a, b, and c in the quadratic formula
     fA = 1.0 + fAziChunkSlope * fAziChunkSlope;
     fB = -2.0 * (fXDist + fAziChunkSlope * fYDist);
     fC = fYDist * fYDist + fXDist * fXDist - fNeighCanrad * fNeighCanrad;
     fC = fB * fB - 4.0 * fA * fC;

     //If fC is positive, it means that the system of equations has a real
     //solution, the circle and line intersect, and thus the azimuth chunk is
     //blocked.
     if (fC >= 0.0) {

       //Finish solving the quadratic formula so we can calculate altitude angles.
       //We'll have an X,Y point for the slope's intersection with the near and
       //far sides of the neighbor's crown
       fC = sqrt((double) fC);
       fNearX = (-fB - fC) / (2.0 * fA);
       fFarX = (-fB + fC) / (2.0 * fA);
       fNearY = fAziChunkSlope * fNearX;
       fFarY = fAziChunkSlope * fFarX;

       //Calculate the squares of the distances to the near and far points on the
       //neighbor's crown - we don't need to bother taking the square roots
       fSquareOfNearDist = fNearX * fNearX + fNearY * fNearY;
       fSquareOfFarDist = fFarX * fFarX + fFarY * fFarY;

       //Switch them in case we were wrong about which point was near
       if (fSquareOfNearDist > fSquareOfFarDist) {
         fA = fSquareOfNearDist; //dangerously reusing fA!
         fSquareOfNearDist = fSquareOfFarDist;
         fSquareOfFarDist = fA;
       }

       //Calculate the starting and ending altitude chunks for this azimuth
       //chunk
       iAltStart = (int) (fCrownBaseDiffChunkConv /
                    sqrt((double)(fSquareOfFarDist + fCrownBaseDiffSquared)));
       if (iAltStart < m_iMinAngRow)  iAltStart = m_iMinAngRow;

       iAltEnd = (int) (fHeightDiffChunkConv /
                    sqrt((double)(fSquareOfNearDist + fHeightDiffSquared)));
       if (iAltEnd >= m_iNumAltAng)  iAltEnd = m_iNumAltAng - 1;

       //Now multiply the light extinction coefficient into the photo array for
       //the altitude chunks for this azimuth angle
       for (i = iAltStart; i <= iAltEnd; i++)
         mp_fPhoto[i][iAziChunk] *= fLightExtCoeff;

     } //end of if (fC >= 0.0)

     //If that azimuth chunk wasn't blocked, we're done searching in this
     //direction.  If we've also searched the other direction, quit
     else if (iAziChunkIncrementer == 1) return;

     //We still need to search the other direction
     else {

       //Go back to the home chunk - we'll increment past it below
       iAziChunk = iHomeAziChunk;
       //Flip the incrementer to the other direction
       iAziChunkIncrementer = 1;
       //Start the chunk counter over
       iChunkCounter = 0;
     }

      //Increment the azimuth chunk in the appropriate direction
     iAziChunk += iAziChunkIncrementer;
     iChunkCounter++;

     //If we've done half the azimuth angles, quit even if we're still
     //finding solutions
     if (iChunkCounter == iHalfTheAzis) return;

     //If we've gone too far in either direction, wrap around
     if (iAziChunk < 0)
     iAziChunk = m_iNumAziAng - 1;
     else if(iAziChunk == m_iNumAziAng)
     iAziChunk = 0;

     fAziChunkSlope = mp_fAziSlope[iAziChunk];
   } //end of while (1)

 } //end of try block
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.sFunction = "clTradLight::GLIFisheyeNoCanopyIntersect";
   stcErr.iErrorCode = UNKNOWN;
   throw(stcErr);
 }
}

//////////////////////////////////////////////////////////////////////////////
// GLIFisheyeCanopyIntersect()
//////////////////////////////////////////////////////////////////////////////
void clGLIBase::GLIFisheyeCanopyIntersect(const float &fTargetX, const float
  &fTargetY, const float &fNeighX, const float &fNeighY, const float
  &fNeighCanrad, const float &fNeighHeight, const short int &iNeighSpecies,
  const float &fTargetHeight, clTree *p_oNeighbor, clPlot *p_oPlot,
  clAllometry *p_oAllom) {
 try {
   float fXDist, fYDist, //distance between two trees in X and Y directions
         fLightExtCoeff, //light extinction coefficient
         fDistToFarEdge,    //square of distance to farthest edge of neighbor
         fCrownBase,     //height of base of neighbor crown
         fX1, fX2, fY1, fY2, //two solutions of system of equations
         fXSolution, fYSolution, //X and Y solutions to system of equations
         fHeightDiff,    //difference in target height and crown base
         fA, fB, fC,     //variables in quadratic formula
         fCCoeff;        //one quadratic coefficient that can be done ahead
   short int iAziStart = 0, iAziEnd = 0,   //start and end sky azimuth coords
             iAltStart = 0, iAltEnd = 0,   //start and end sky altitude coords
             iHalfAzi,                     //counter for half of azimuth grid
             iAziChunk,                    //azi grid chunk we're working with
             i, j, k;                      //loop counters


   //If the target is under the canopy, then there is shading in every
   //azimuth direction.  For each azimuth segment, find the bottom of the
   //crown and what altitude segment it blocks.
   iAltEnd = m_iNumAltAng - 1; //the zenith is always blocked

   fXDist = p_oPlot->GetXDistance(fTargetX, fNeighX);
   fYDist = p_oPlot->GetYDistance(fTargetY, fNeighY);

   iHalfAzi = m_iNumAziAng/2;
   fLightExtCoeff = GetLightExtinctionCoefficient(p_oNeighbor);
   //Crown base
   if (clTreePopulation::sapling == p_oNeighbor->GetType()) {
     fCrownBase = fNeighHeight -
            p_oAllom->CalcSaplingCrownDepth(p_oNeighbor);
   } else {
     fCrownBase = fNeighHeight -
            p_oAllom->CalcAdultCrownDepth(p_oNeighbor);
   }
   fHeightDiff = fCrownBase - fTargetHeight;
   fCCoeff = (fXDist*fXDist) + (fYDist*fYDist) - (fNeighCanrad * fNeighCanrad);

   if (fCrownBase <= fTargetHeight) {//neighbor blocks whole sky

     iAltStart = m_iMinAngRow;
     iAziStart = 0;
     iAziEnd = m_iNumAziAng - 1;
     for (i = iAltStart; i <= iAltEnd; i++)
       for (j = iAziStart; j <= iAziEnd; j++)
         mp_fPhoto[i][j] *= fLightExtCoeff;

   } else { //neighbor blocks part of sky
     //Do half the sky - we'll solve in both directions at once
     for (i = 0; i < iHalfAzi; i++) {

       //we have to use linear algebra to solve the system of equations of
       //the circle of the neighbor's canopy and the line from the target to
       //the center of the azimuth chunk in question.  The slope of the line
       //is in mp_fAziSlope.  Do all calculations as though the target tree
       //is at the origin.

       //Solve the quadratic formula
       fA = 1.0 + mp_fAziSlope[i] * mp_fAziSlope[i];
       fB = -2.0 * (fXDist + mp_fAziSlope[i] * fYDist);
       fC = fB * fB - 4.0 * fA * fCCoeff;
       if (fC < 0) fC = 0; //test for sqrt bug
       fC = sqrt((double) fC);

       //Get the solutions to find the intersection points between the circle
       //and line - these points are (fX1, fY1) and (fX2, fY2)
       fX1 = (-fB + fC) / (2.0 * fA);
       fX2 = (-fB - fC) / (2.0 * fA);
       fY1 = mp_fAziSlope[i] * fX1;
       fY2 = mp_fAziSlope[i] * fX2;

       //If first point is to the west of the second, switch 'em
       if (fX1  < 0.0) {
         fC = fX1;   //dangerously reuse fC
         fX1 = fX2;
         fX2 = fC;
         fC = fY1;
         fY1 = fY2;
         fY2 = fC;
       }

       //For each point, calculate the altitude start point
       for (j = 0; j < 2; j++) {
         fXSolution = j == 0 ? fX1 : fX2;
         fYSolution = j == 0 ? fY1 : fY2;
         iAziChunk = j == 0 ? i : (i + iHalfAzi);
         fDistToFarEdge = (fXSolution*fXSolution) + (fYSolution*fYSolution);
         iAltStart = (int)(floor(((fHeightDiff * m_iNumAltAng)
              / sqrt((fDistToFarEdge) + (fHeightDiff * fHeightDiff)))));
         if (iAltStart < m_iMinAngRow) iAltStart = m_iMinAngRow;

         //Use coords to loop thru chunk of sky array and mark it as blocked
         for (k = iAltStart; k <= iAltEnd; k++)
           mp_fPhoto[k][iAziChunk] *= fLightExtCoeff;
       } //end of for (int j = 0; j < 2; j++)
     } //end of for (i = 0; i < iHalfAzi; i++)
   } //end of else
 } //end of try block
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.sFunction = "clTradLight::GLIFisheyeNoCanopyIntersect";
   stcErr.iErrorCode = UNKNOWN;
   throw(stcErr);
 }
}
//----------------------------------------------------------------------------
