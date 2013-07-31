//---------------------------------------------------------------------------
// GLIMap
//---------------------------------------------------------------------------
#if !defined(GLIMap_H)
  #define GLIMap_H

#include "GLIBase.h"


class clGrid;
class clTreePopulation;

/**
* GLIMapCreator - Version 1.0
*
* This behavior creates a map of GLI values in a grid.  The user sets the
* height of the point at which to calculate GLI, and then this behavior
* calculates the GLI at that height in the center of each cell.  (This behavior
* is almost identical to clQuadratGliLight, except it doesn't apply light to
* trees and calculates a GLI for every grid cell no matter what.)  This
* behavior does not need to be applied to any trees.
*
* Nothing else is done with the GLI values.  This is an analysis behavior that
* prepares some calculations; the user can output the GLI map grid and then
* use the values however they see fit.
*
* The namestring and parameter file call string for this behavior is
* "GLIMapCreator".  This class is descended from clGLIBase so it can use its
* light calculation methods, but since the string "lightshell" is ommitted from
* this class's namestring, it should not be treated as a light behavior.
*
* The sky brightness array used by this behavior is potentially identical to
* that for other GLI light behaviors.  Before committing to the calculation of
* a brightness array, this behavior will ask those behaviors if their settings
* are identical.  If they are, and that class has already calculated the
* brightness array, this behavior can just copy.
*
* Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clGLIMap : public clGLIBase {

  friend class clTestGLIMap; /**<To ease testing of parameter setup*/

  public:

  /**
  * Constructor.  Sets the namestring.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clGLIMap(clSimManager *p_oSimManager);

 //~clGLIMap(); //use default destructor

  /**
  * Does setup for this object.  Calls the following functions:
  * <ol>
  * <li>clLightBase::GetData(), to make sure mp_oLightOrg gets set up if this
  * is the "hooked" light object</li>
  * <li>ReadParameterFileData</li>
  * <li>SetUpBrightnessArray</li>
  * <li>DoSetupCalculations</li>
  * <li>SetUpGrid</li>
  * </ol>
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Creates the GLI map.  Each cell in the "GLI Map" grid, below, gets its GLI
  * calculated at the height value in m_fLightHeight in the center of the cell.
  */
  void Action();

  /**
  * Required overridden function - doesn't do anything.
  * @param p_oTree Ignored.
  * @param p_oPop Ignored.
  * @return 0.
  */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop) {return 0;};

  /**
   * Gets the height above the ground, in m, at which the quadrat light values
   * are calculated.
   * @return Light height.
   */
  float GetLightHeight() {return m_fLightHeight;};

  /**
   * Gets the maximum search distance for shading neighbors.
   * @return Maximum search distance for shading neighbors.
   */
  float GetMaxSearchDistance() {return m_fMaxSearchDistance;};

 protected:

  /**
  * Grid object which holds the GLI values.  The name of the grid is "GLI Map".
  * There is one float data member ("GLI") for holding GLI as a value from 0 to
  * 100.  Grid data in the parameter file can only set cell resolution; any map
  * values will be ignored.
  */
  clGrid *mp_oMapGrid;

  float m_fLightHeight;  /**<The height above the ground, in m, at which the
                        quadrat light values are calculated.*/
  float m_fMaxSearchDistance; /**<Maximum search distance for shading
                                 neighbors.*/
  short int m_iGridGliCode;/**<Return code for the "GLI Map" grid to get and set
                        GLI in the cells.*/

  /**
  * Reads in needed parameter file data.  First this looks for the "gliMap" tag
  * and tries to read in all its data from that (in case the user has provided
  * different sky grid settings to different light objects).  If there's no
  * sky grid data in the "gliMap" tag, then this will get what it needs from
  * the first tag it finds with these values. (The "gliMap" tag, no matter
  * what, must have the height of photo.)
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>The tag "gliMap" or its required data is not present</li>
  * <li>Number of azimuth sky divisions is less than or equal to 0</li>
  * <li>Number of altitude sky divisions is less than or equal to 0</li>
  * <li>Height of GLI point is less than 0</li>
  * </ul>
  */
  void ReadParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Sets up the light brightness array.  This will check to see if it can share
  * the brightness array with clGliLight or clQuadratGliLight, and if not, it
  * creates the sky brightness array.
  */
  void SetUpBrightnessArray();

  /**
  * Performs setup calculations.  This calculates the values for
  * m_fAziChunkConverter, m_fRcpTanMinAng, mp_fAziSlope, and
  * m_fMaxSearchDistance.
  */
  void DoSetupCalculations();

  /**
  * Sets up the "GLI Map" grid.  The parameter file is allowed to set grid cell
  * resolution; map values will be ignored.
  */
  void SetUpGrid();

};
//---------------------------------------------------------------------------
#endif // GLIMap_H
