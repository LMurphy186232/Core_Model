//---------------------------------------------------------------------------

#ifndef ClimateCompDepNeighborhoodSurvivalH
#define ClimateCompDepNeighborhoodSurvivalH
//---------------------------------------------------------------------------
#include "MortalityBase.h"


class clTree;
class clTreePopulation;
class clGrid;

/**
* Climate and competition dependent neighborhood survival - Version 1.0
*
* This is a mortality shell object which calculates survival as a function of
* climate and adult neighbor basal area.
*
* The equation for one year's survival is:
* <center>Probability of survival = exp(-1.0*A*BAT^B)*exp(-0.5*(((X-M)/N)^2))*exp(-0.5*(((WD-C)/D)^2))</center>
* where:
* <ul>
* <li>A, B, M, N, C, and D are parameters</li>
* <li>X is the plot mean annual temperature</li>
* <li>WD is the plot water deficit</li>
* <li>BAT is the adult neighborhood basal area within a specified radius,
* in square meters</li>
* </ul>
*
* Rather than calculate a survival rate for each individual tree, this
* calculates a survival rate for each cell in a grid called "Climate Comp Dep
* Neighborhood Survival". Trees then get the survival rate of the grid cell in
* which they are found.
*
* Snags, seedlings, and trees that are already dead from disturbance events are
* never counted in the neighbor count.
*
* For multi year time steps, the annual probability of survival is raised to the
* power of the number of years per time step.
*
* The parameter file call string for this is
* "ClimateCompDepNeighborhoodSurvival". The namestring for this behavior is
* "ClimateCompDepNeighborhoodmortshell".
*
* Copyright 2013 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>January 24, 2014 - Created (LEM)
*/
class clClimateCompDepNeighborhoodSurvival : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clClimateCompDepNeighborhoodSurvival(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clClimateCompDepNeighborhoodSurvival();

  /**
  * Determines mortality for a tree. This starts by checking the grid cell of
  * the tree to see if survival rate has already been calculated. If so, use it.
  * If not, calculate it for this species for this grid cell. If not already
  * determined, get the adult neighborhood basal area for the cell and calculate
  * the probability of survival using the equation above. Use the random number
  * generator to decide life or death; return the result.
  *
  * @param p_oTree Tree being evaluated.
  * @param fDbh DBH of tree being evaluated.
  * @param iSpecies Species of tree being evaluated.
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort (clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  /**
  * Does setup.
  * <ol>
  * <li>ReadParameterFile() is called to read the parameter file's data.</li>
  * <li>ValidateData() is called to validate the data.</li>
  * <li>SetupGrid() is called to create the survival grid.</li>
  * </ol>
  *
  * @param p_oDoc DOM tree of parsed input tree.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Does beginning of the time step setup. This clears all grid values and
  * calculates the temperature portion of the survival function for each
  * species.
  * @param p_oPop Tree population.
  */
  void PreMortCalcs( clTreePopulation *p_oPop );

  protected:

  /**
   * Grid holding survival rate for each species. The grid name is
   * "Climate Comp Dep Neighborhood Survival". It has X+1 float data
   * members, where X = the total number of species. The data member names
   * are "survival_x", for the time step survival rate (where "x" is the
   * species number), and "BAT" for the total adult neighborhood basal area.
   */
  clGrid* mp_oGrid;

  /**Temperature function M - sized number of species.*/
  float *mp_fM;

  /**Temperature function N - sized number of species.*/
  float *mp_fN;

  /**Water deficit function C - sized number of species.*/
  float *mp_fC;

  /**Water deficit function D - sized number of species.*/
  float *mp_fD;

  /**Crowding function A parameter - sized number of species.*/
  float *mp_fA;

  /**Crowding function B parameter - sized number of species.*/
  float *mp_fB;

  /**Temperature portion of the survival function - sized number of species.*/
  float *mp_fTempFunction;

  /**Water deficit portion of the survival function - sized number of species.*/
  float *mp_fWDFunction;

  /** Holds data member codes for the "survival_x" data members of the
   * "Climate Comp Dep Neighborhood Survival" grid. Array size is total
   * # species.*/
  short int *mp_iGridSurvivalCodes;

  /**Neighborhood search radius.*/
  float m_fRadius;

  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Return code for the "BAT" grid data member.*/
  short int m_iBATCode;

  /**
  * Makes sure N and D for each species does not = 0 and that the neighbor
  * search radius >= 0.
  *
  * @throws modelErr if the above conditions are not met.
  */
  void ValidateData();

    /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input tree.
  * @param p_oPop Tree population.
  */
  void ReadParameterFile( xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop );

  /**
  * Gets the total adult neighborhood basal area within the specified radius
  * from a given point. Neighbors must have a DBH greater than the minimum. They
  * also cannot be dead from a disturbance event; but any trees that have a
  * dead code of "natural" are assumed to have died in the current time step
  * mortality cycle and thus should be counted.
  * @param fX X coordinate of point for which to calculate neighborhood basal
  * area
  * @param fY Y coordinate of point for which to calculate neighborhood basal
  * area
  * @param p_oPop Tree population.
  * @returns Total adult basal area in square meters.
  */
  float GetBAT (float &fX, float &fY, clTreePopulation *p_oPop);

  /**
  * Sets up the "Climate Comp Dep Neighborhood Survival" grid. This
  * ignores any maps.
  * @param p_oPop Tree population object.
  */
  void SetupGrid( clTreePopulation *p_oPop );
};
//---------------------------------------------------------------------------
#endif
