//---------------------------------------------------------------------------

#ifndef AverageLightH
#define AverageLightH
//---------------------------------------------------------------------------
#include "LightBase.h"
#include <math.h>


class clGrid;
class clTreePopulation;

/**
* Average Light - Version 1.0
*
* This behavior averages light values taken over a finer scale to a coarser
* resolution.  It gets the light levels for averaging from the GLI Map
* Creator's grid (class clGLIMap). This grid is called "GLI Map X" where X
* is the position in the behavior list. This will average the
* first GLI Map X grid that it finds. Putting multiple copies of the GLI Map
* Creator in a run would be a problem: because of the clLightOrg hooking
* process, it is impossible to guarantee that all copies of GLI Map
* Creator will run before Average Light.
*
* The Average Light behavior produces one grid, called "Average Light".
* The user sets the grid resolution on this grid to control the area over
* which light is averaged, up to the whole plot.
*
* If the grid cell area of Average Light is not an exact multiple of the GLI
* Map grid cell area, then the averaging will be over all the cells touched by
* the Average Light grid, with no weighting for partial cells (see the
* clGrid::GetAverageFloatValue method).
*
* The namestring for this behavior is "averagelightshell"; the parameter file
* call string is "AverageLight".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
* <br>May 18, 20145 - Added support for multiple GLI Map grids (LEM)
*/
class clAverageLight : virtual public clLightBase {

 public:

  /**
  * Constructor.  Sets the namestring.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clAverageLight(clSimManager *p_oSimManager);

  //~clAverageLight(); use default

  /**
  * Does setup for this behavior.  This grabs a pointer to the "GLI Map" grid
  * and either grabs the "Average Light" grid or sets it up if it doesn't
  * exist yet.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if the "GLI Map" grid is not present.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Sets the "average calculated" flag back to false.
  */
  void TimestepCleanup() {m_bAvgCalculated = false;};

  /**
  * Gets the light value for a particular tree.  First, it checks to see if the
  * average has been calculated this timestep.  If not, then CalculateAverage()
  * is called.  Then the average value is returned.
  * @param p_oTree Tree for which to get light.
  * @param p_oPop Tree population.
  * @returns Average GLI value.
  */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop);

  protected:

  /**Grid object which holds the GLI light values to average.  This grid is not
   * created by this class, but by the clGLIMap class.
  */
  clGrid *mp_oGLIMapGrid;

  /**Grid object which holds the averaged light values.  This grid is
   * created by this class.  Its name is "Average Light" and it has one float
   * data member, "GLI".
  */
  clGrid *mp_oAvgLightGrid;

  /**Return code for the "GLI" data member of the "GLI Map" grid.*/
  short int m_iMapLightCode;

  /**Return code for the "GLI" data member of the "Average Light" grid.*/
  short int m_iAvgLightCode;

  /**Whether or not the average GLI value has been calculated this
   * timestep.*/
  bool m_bAvgCalculated;

  /**
  * Calculates the average light value each timestep.  For each cell in the
  * "Average Light" grid, it calculates an average of the grid cells in "GLI
  * Map" that fall within its values using clGrid::GetAverageFloatValue. At
  * the end, this sets m_bAvgCalculated to true.
  * @param p_oPop Tree population object
  */
  void CalculateAverage(clTreePopulation *p_oPop);
};
//---------------------------------------------------------------------------
#endif
