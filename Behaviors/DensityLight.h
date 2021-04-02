//---------------------------------------------------------------------------

#ifndef DensityLightH
#define DensityLightH
//---------------------------------------------------------------------------
#include "LightBase.h"
#include <math.h>


class clGrid;
class clTreePopulation;

/**
* Density Light - Version 1.0
*
* This behavior calculates light level as a function of local tree density.
* The density is used to calculate a mean light level, which is used to choose
* a random value from a lognormal probability distribution.  In order to ensure
* some level of continuity through time, a grid cell's light level is not
* recalculated unless the local tree density has changed by at least an amount
* set by the user.
*
* A grid, "Density Light", is used to store light values.  Trees get the light
* levels of their grid cell.
*
* The mean light level for a grid cell is calculated as follows:
* <br><center><i>GLI<sub>m</sub> = a / (1 + (den/c) <sup>b</sup>)</i></center>
* where:
* <ul>
* <li><i>GLI<sub>m</sub></i> is the mean light level, as a value between 0 and
* 100</li>
* <li><i>a</i>, <i>b</i>, and <i>c</i> are parameters</li>
* <li><i>den</i> is the number of trees in the "Density Light" cell (more on
* density calculations below)</li>
* </ul>
*
* When counting trees for the density, seedlings and snags are never counted.
* Other trees only count if they have a DBH above a user-set minimum.
*
* After the mean light level is calculated, it is turned into a "location
* parameter" (mu) for the lognormal PDF as follows:
* @htmlonly <center><i>&mu; = ln(mean) - (&sigma; <sup>2</sup>)/ 2</i></center> @endhtmlonly
*
* The mu value is used as the zeta value in a lognormal random draw.
*
* Note that this behavior is dependent on the size of grid cells for its
* behavior.
*
* This behavior creates a new grid called "Density Light".  There's more on
* that grid below.
*
* The namestring for this behavior is "densitylightshell"; the parameter file
* call string is "Density Light".
*
* Copyright 2005 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>December 7, 2005 - Created (LEM)
*/
class clDensityLight : virtual public clLightBase {

 public:

  /**
  * Constructor.  Sets the namestring.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clDensityLight(clSimManager *p_oSimManager);

  //~clDensityLight(); use default constructor

  /**
  * Does setup for this behavior.  This reads parameter file values and sets
  * up the grid.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>The value for c = 0</li>
  * <li>If the density change threshold < 0</li>
  * <li>If the minimum DBH < 0</li>
  * <li>Grid cell lengths of "Density Light" don't divide evenly by the plot
  * lengths</li>
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
  * called.  Then the value in the tree's grid cell of the "Density Light" grid
  * is returned.
  * @returns GLI value, as recorded in the tree's "Density Light" grid cell.
  */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop);

  /**
  * Calculates the mean GLI for a given density.  This is a separate function
  * largely for testing purposes.
  * @param iDensity The tree density.
  * @returns Mean GLI, bounded between 0 and 100.
  */
  inline float GetMeanGLI(const int &iDensity) {
    float fTemp = m_fA / (1 + pow(iDensity / m_fC, m_fB));
    fTemp = fTemp >= 0 ? fTemp : 0;
    return fTemp <= 100 ? fTemp : 100;
  };

  protected:

  /**Grid object which holds the light values.  The name of this grid is
  * "Density Light".  It has one float data member called "Light", and one int
  * data member called "Count", which holds the number of trees (that count
  * towards density) in the cell.  It uses the default grid cell resolution
  * unless otherwise instructed in the parameter file.  A map of this grid in
  * the parameter file will be honored.
  */
  clGrid *mp_oLightGrid;

  /**The "a" parameter for the mean light function.*/
  double m_fA;

  /**The "b" parameter for the mean light function.*/
  double m_fB;

  /**The "c" parameter for the mean light function.*/
  double m_fC;

  /**The sigma parameter of the lognormal PDF function.*/
  double m_fSigma;

  /**The minimum DBH for trees to count towards density in a cell.*/
  double m_fMinDbh;

  /**The number by which the density in a cell has to change in order to
  * trigger a new light calculation.*/
  int m_iChangeThreshold;

  /**Return code for the "Light" data member.*/
  short int m_iGridLightCode;

  /**Return code for the "Count" data member.*/
  short int m_iGridCountCode;

  /**Whether or not the "Density Light" grid has been updated this timestep.*/
  bool m_bGridUpdated;

  /**
  * Performs the grid updating each timestep.  It counts up the trees in each
  * cell of the grid.  It then compares them to the previous timestep's
  * density.  For any cell that has changed by more than the change threshold,
  * a new light level is calculated.  At the end, this sets m_bGridUpdated to
  * true.
  * @param p_oPop Pointer to the tree population.
  */
  void UpdateGridValues(clTreePopulation * p_oPop);
};
//---------------------------------------------------------------------------
#endif
