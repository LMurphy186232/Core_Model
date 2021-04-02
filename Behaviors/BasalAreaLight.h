//---------------------------------------------------------------------------

#ifndef BasalAreaLightH
#define BasalAreaLightH
//---------------------------------------------------------------------------
#include "LightBase.h"
#include <math.h>


class clGrid;
class clTreePopulation;

/**
* Basal Area Light - Version 1.0
*
* This behavior calculates light level as a function of local tree basal area.
* Basal area is split between conifers and angiosperms.  The two basal area
* totals are used to calculate a mean light level, which is used to choose
* a random value from a lognormal probability distribution.  In order to ensure
* some level of continuity through time, a grid cell's light level is not
* recalculated unless the local tree basal area has changed by more than a
* threshold set by the user.
*
* A grid, "Basal Area Light", is used to store light values.  Trees get the
* light levels of their grid cell.
*
* The mean light level for a grid cell is calculated as follows:
* <br><center>GLI<sub>m</sub> =
*      a / (1 + (BA<sub>a</sub>/c<sub>a</sub>) <sup>b<sub>a</sub></sup> +
*       (1 + (BA<sub>c</sub>/c<sub>c</sub>) <sup>b<sub>c</sub></sup>)</center>
* where:
* <ul>
* <li>GLI<sub>m</sub> is the mean light level, as a value between 0 and
* 100</li>
* <li>BA<sub>a</sub> is angiosperm basal area, in square meters, within the
* user-specified radius of the "Basal Area Light" cell's center</li>
* <li>BA<sub>c</sub> is conifer basal area, in square meters, within the
* user-specified radius of the "Basal Area Light" cell's center</li>
* <li>a is a parameter</li>
* <li>b<sub>a</sub> and c<sub>a</sub> are parameters for angiosperms</li>
* <li>b<sub>c</sub> and c<sub>c</sub> are parameters for conifers</li>
* </ul>
*
* For purposes of basal area calculation, each species is assigned to "conifer"
* or "angiosperm" status.  Seedlings and snags are never counted in basal area.
* Other trees only count if they have a DBH above a user-set minimum.
*
* After the mean light level is calculated, it is turned into a "location
* parameter" (mu) for the lognormal PDF as follows:
* @htmlonly <center>&mu; = ln(mean) - (&sigma; <sup>2</sup>)/ 2</center> @endhtmlonly
*
* The mu value is used as the zeta value in a lognormal random draw.
*
* Note that this behavior is dependent on the size of grid cells for its
* behavior.
*
* This behavior creates a new grid called "Basal Area Light".  There's more on
* that grid below.
*
* The namestring for this behavior is "basalarealightshell"; the parameter file
* call string is "BasalAreaLight".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clBasalAreaLight : virtual public clLightBase {

 public:

  /**
  * Constructor.  Sets the namestring.
  * @param p_oSimManager Sim Manager object.
  */
  clBasalAreaLight(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clBasalAreaLight();

  /**
  * Does setup for this behavior.  This reads parameter file values and sets
  * up the grid.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>The value for either c = 0</li>
  * <li>If the density change threshold < 0</li>
  * <li>If the minimum DBH < 0</li>
  * <li>If the neighbor search radius < 0</li>
  * </ul>
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Sets the grid updated flag back to false.
  */
  void TimestepCleanup() {m_bGridUpdated = false;};

  /**
  * Gets the light value for a particular tree.  First, it checks to see if the
  * grid has been updated this timestep.  If not, then UpdateGridValues() is
  * called.  Then the value in the tree's grid cell of the "Basal Area Light"
  * grid is returned.
  * @returns GLI value, as recorded in the tree's "Density Light" grid cell.
  */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop);

  /**
  * Calculates the mean GLI for a given pair of basal areas.  This is a
  * separate function largely for testing purposes.
  * @param fConiferBA The conifer basal area, in square meters.
  * @param fAngiospermBA The angiosperm basal area, in square meters.
  * @returns Mean GLI, bounded between 0 and 100.
  */
  inline float GetMeanGLI(const float &fConiferBA, const float &fAngiospermBA) {
    float fTemp = m_fA / (1 + pow(fConiferBA / m_fConiferC, m_fConiferB) +
                        pow(fAngiospermBA / m_fAngiospermC, m_fAngiospermB));
    fTemp = fTemp >= 0 ? fTemp : 0;
    return fTemp <= 100 ? fTemp : 100;
  };

  protected:

  /**Grid object which holds the light values.  The name of this grid is
  * "Basal Area Light".  It has one float data member called "Light", and two
  * float data members called "Con BA" and "Ang BA", which hold the basal area
  * of conifers and angiosperms, respectively, in the cell.  It uses the
  * default grid cell resolution unless otherwise instructed in the parameter
  * file.  A map of this grid in the parameter file will be honored.
  */
  clGrid *mp_oLightGrid;

  /**Species type status*/
  enum iTreeType {angiosperm, /**<Angiosperm*/
                  conifer /**<Conifer*/
  } *mp_iSpeciesTypes; /**<What type each species is*/

  /**The status of each species, as angiosperm or conifer.*/
  float *mp_fStatus;

  /**The "a" parameter for the mean light function.*/
  double m_fA;

  /**The conifer "b" parameter for the mean light function.*/
  double m_fConiferB;

  /**The angiosperm "b" parameter for the mean light function.*/
  double m_fAngiospermB;

  /**The conifer "c" parameter for the mean light function.*/
  double m_fConiferC;

  /**The angiosperm "c" parameter for the mean light function.*/
  double m_fAngiospermC;

  /**The sigma parameter of the lognormal PDF function.*/
  double m_fSigma;

  /**The minimum DBH for trees to count towards density in a cell.*/
  double m_fMinDbh;

  /**The search radius for trees.*/
  double m_fRadius;

  /**The amount by which total basal area in a cell has to change in order to
  * trigger a new light calculation.*/
  double m_fChangeThreshold;

  /**Return code for the "Light" data member of the "Basal Area Light" grid.*/
  short int m_iGridLightCode;

  /**Return code for the "Con BA" data member of the "Basal Area Light" grid.*/
  short int m_iGridConBACode;

  /**Return code for the "Ang BA" data member of the "Basal Area Light" grid.*/
  short int m_iGridAngBACode;

  /**Whether or not the "Basal Area Light" grid has been updated this
   * timestep.*/
  bool m_bGridUpdated;

  /**
  * Performs the grid updating each timestep.  It totals up angiosperm and
  * conifer basal area within the search radius for each grid cell.  It then
  * compares them to the previous timestep's total.  For any cell that has
  * changed by more than the change threshold, a new light level is calculated.
  * At the end, this sets m_bGridUpdated to true.
  * @param p_oPop Pointer to the tree population.
  */
  void UpdateGridValues(clTreePopulation * p_oPop);
};
//---------------------------------------------------------------------------
#endif
