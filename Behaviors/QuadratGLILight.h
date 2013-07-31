//---------------------------------------------------------------------------

#ifndef QuadratGLILightH
#define QuadratGLILightH
//---------------------------------------------------------------------------
#include "GLIBase.h"


class clGrid;
class clTreePopulation;

/**
 * Quadrat GLI Light - Version 1.0
 *
 * This behavior uses a grid object to help it assign GLI values to the trees
 * to which it is assigned.  The grid cells are the quadrats, in this case; this
 * is a throwback to old SORTIE terminology.  Each grid cell in which there is
 * a tree to which this behavior applies has a GLI value calculated at its
 * center, at a height that the user specifies.  All other trees to which this
 * behavior applies that are in that same grid cell get that same GLI value.
 * This behavior saves having to calculate a different GLI value for each tree.
 *
 * The namestring is "quadratglilightshell". The parameter file call string is
 * "QuadratLight".
 *
 * The sky brightness array used by this behavior is potentially identical to
 * that for regular GLI light - class name clGliLight, namestring
 * "glilightshell".  Before committing to the calculation of a brightness
 * array, this behavior will ask that behavior if it's already done it and this
 * behavior can just copy.  (The reverse could also happen.)
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */
class clQuadratGLILight : public clGLIBase {

public:

  friend class clTestQuadratGLILight; /**<To ease testing of parameter setup*/

  /**
   * Constructor.  Sets the namestring.
   *
   * @param p_oSimManager Sim Manager object.
   */
  clQuadratGLILight(clSimManager *p_oSimManager);

  //~clQuadratGLILight(); //use default destructor

  /**
   * Reads some extra parameters from the parameter file.  If there's no
   * "quadratGliLight" tag, then this will get what it needs from the "gliLight"
   * tag.  This must have number of azimuth angles and number of altitude angles.
   * This will check to see if it can share the brightness array with clGliLight.
   *
   * @param p_oDoc DOM tree of parsed input file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Provides a GLI value for a tree.
   * @param p_oTree Tree for which to get GLI.
   * @param p_oPop Tree population object.
   * @return GLI as a percentage of full sun between 0 and 100.  The GLI value
   * is that calculated for the tree's quadrat.
   */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop);

  /**
   * Resets all the values in the grid to -1.
   */
  void TimestepCleanup();

  /**
   * Finishes the GLI calculations. This starts by calling the base class's
   * version of Action. Then, if m_bCalcAll is true, this will calculate GLI
   * for any cells that were missed.
   */
  void Action();

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
  float GetMaxSearchRad() {return m_fMaxSearchRad;};

protected:

  /**
   * Grid object which holds the quadrat values.  The grid cell resolution
   * defaults to 2mX2m, and it has one float data member ("GLI") for holding GLI.
   * The grid name is "Quadrat GLI".
   *
   * A map in the parameter file can only set cell resolution; any values in the
   * map will be ignored.
   */
  clGrid *mp_oQuadrats;

  float m_fLightHeight; /**<The height above the ground, in m, at which the
   quadrat light values are calculated.  Defaults to
   0.675 m.*/
  float m_fMaxSearchRad; /**<The maximum distance, in m, to search for shading
   neighbors.*/
  short int m_iGridGliCode;/**<Return code for the quadrats grid to get and set
   GLI in the cells.*/
  bool m_bCalcAll; /**<Whether to always calculate all GLIs (true) or only
  calculate on an as-needed basis (false)*/

};
//---------------------------------------------------------------------------
#endif
