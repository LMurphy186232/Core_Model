//---------------------------------------------------------------------------

#ifndef GLIBaseH
#define GLIBaseH
//---------------------------------------------------------------------------
#include "LightBase.h"

class clTree;
class clPlot;
class clTreePopulation;
class clAllometry;


/**
* GLI base - Version 1.0
*
* This groups the common GLI light functions and data members.  Child classes
* can use the functions here, but there are a set of requirements they must
* fill.  They must:
* <ul>
* <li>Calculate and set a value for m_fSinMinSunAng (see its entry for
* equation)</li>
* <li>Calculate and set a value for m_fAziChunkConverter (see its entry for
* equation)</li>
* <li>Calculate and set a value for m_fRcpTanMinAng (see its entry for
* equation)</li>
* <li>Calculate and set values for mp_fAziSlope (see its entry for
* equation)</li>
* </ul>
*
* I could have done a common base class setup function to do this, but those
* are starting to nest pretty deep between clBehaviorBase and clLightBase.  So
* I left it to the child classes.  Each child has to do this separately since
* each might have different sky setup equations.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>April 28, 2004 - Submitted as beta (LEM)
*/
class clGLIBase : virtual public clLightBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Pointer to the Sim Manager.
  */
  clGLIBase(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  virtual ~clGLIBase();

  /**
   * Gets the sine of the minimum sun angle.
   * @return Sine of the minimum sun angle.
   */
  float GetSinMinSunAng() {return m_fSinMinSunAng;};

  /**
   * Gets the reciprocal of the size of a single azimuth sky grid division, in
   * degrees.
   * @return Converter.
   */
  float GetAziChunkConverter() {return m_fAziChunkConverter;};

  /**
   * Gets the reciprocal of the tangent of the minimum sun angle.
   * @return Reciprocal of the tangent of the minimum sun angle.
   */
  float GetRcpTanMinAng() {return m_fRcpTanMinAng;};

  protected:

  float m_fSinMinSunAng;/**<Sin of the min sun angle. Saves tons of calculations
  to pre-calculate this.  Child classes must set this; it's just
  sin(m_fMinSunAngle).*/
  float m_fAziChunkConverter; /**<Reciprocal of the size of a single azimuth
  sky grid division, in degrees.  For converting to azimuth rows in brightness
  array - just multiply by an azimuth heading in degrees. This saves tons of
  calculations when pre-calculated.  Since the size of azimuth sky grid
  divisions may differ, child classes must set this; it's m_iNumAziAng/360.0.*/
  float m_fRcpTanMinAng; /**<1/tan (m_fMinSunAngle). Child classes must set
  this.*/
  float *mp_fAziSlope;  /**<For each azimuth chunk of sky, this holds the slope
  of the line to the middle of the chunk.  Array size = m_iNumAziAng.  Each
  value is 1/(tan(fAngChunk*(i + 0.5))), where i = array index, and fAngChunk
  is the size of each azimuth chunk in radians.  Child classes must set this.*/


  /**
   * Adds one shading neighbor to the fisheye photo array for a GLI
   * calculation.
   *
   * @param fTargetX X coordinate of target tree
   * @param fTargetY Y coordinate of target tree
   * @param fTargetHeight Height of target tree, in meters
   * @param p_oNeighbor The neighbor to be added
   * @param p_oPlot Pointer to the plot object
   * @param p_oPop Pointer to the tree population object
   * @param p_oAllom Pointer to an allometry object
   */
  void AddTreeToGliFishEye(const float &fTargetX, const float &fTargetY, const
    float &fTargetHeight, clTree *p_oNeighbor, clPlot *p_oPlot,
    clTreePopulation *p_oPop, clAllometry *p_oAllom);

  /**
   * Adds one shading neighbor to the fisheye photo array for a GLI
   * calculation when the canopies of the two trees do not overlap.  Called by
   * AddTreeToGliFishEye().
   *
   * @param fTargetX X coordinate of target tree
   * @param fTargetY Y coordinate of target tree
   * @param fNeighX X coordinate of shading neighbor
   * @param fNeighY Y coordinate of shading neighbor
   * @param fNeighCanrad Shading neighbor canopy radius, in meters
   * @param fNeighHeight Shading neighbor height, in meters
   * @param iNeighSpecies Neighbor species
   * @param fTargetHeight Height of target tree, in meters
   * @param fDistToNearEdge Distance to nearest edge of shading neighbor
   * canopy, in m.
   * @param p_oNeighbor The neighbor to be added
   * @param p_oPlot Pointer to plot object
   * @param p_oAllom Pointer to an allometry object
   */
  void GLIFisheyeNoCanopyIntersect(const float &fTargetX, const float
      &fTargetY, const float &fNeighX, const float &fNeighY, const float
      &fNeighCanrad, const float &fNeighHeight,
      const short int &iNeighSpecies, const float &fTargetHeight,
      const float &fDistToNearEdge, clTree *p_oNeighbor, clPlot *p_oPlot,
      clAllometry *p_oAllom);

  /**
   * Adds one shading neighbor to the fisheye photo array for a GLI
   * calculation when the canopies of the two trees overlap.  Called by
   * AddTreeToGliFishEye().
   *
   * @param fTargetX X coordinate of target tree
   * @param fTargetY Y coordinate of target tree
   * @param fNeighX X coordinate of shading neighbor
   * @param fNeighY Y coordinate of shading neighbor
   * @param fNeighCanrad Shading neighbor canopy radius, in meters
   * @param fNeighHeight Shading neighbor height, in meters
   * @param iNeighSpecies Neighbor species
   * @param fTargetHeight Height of target tree, in meters
   * @param p_oNeighbor The neighbor to be added
   * @param p_oPlot Pointer to plot object
   * @param p_oAllom Pointer to an allometry object
   */
  void GLIFisheyeCanopyIntersect(const float &fTargetX, const float
      &fTargetY, const float &fNeighX, const float &fNeighY, const float
      &fNeighCanrad, const float &fNeighHeight,
      const short int &iNeighSpecies, const float &fTargetHeight,
      clTree *p_oNeighbor, clPlot *p_oPlot, clAllometry *p_oAllom);
};
//---------------------------------------------------------------------------
#endif
