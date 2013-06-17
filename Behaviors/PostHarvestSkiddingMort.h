//---------------------------------------------------------------------------

#ifndef PostHarvestSkiddingMortH
#define PostHarvestSkiddingMortH
//---------------------------------------------------------------------------
#include "MortalityBase.h"
#include "Grid.h"

class clGrid;

/**
* Post-Harvest Skidding Mortality - Version 1.0
*
* This evaluates mortality as a function of time since the last harvest,
* harvest intensity, DBH, and local basal area. If harvesting has not occurred,
* then a constant background mortality rate is applied.
*
* This class's namestring is "PostHarvestSkiddingMortshell". Its parameter
* file call string is "PostHarvestSkiddingMortality".
*
*
* Copyright 2011 Charles D. Canham.
* @author Mark C. Vanderwel
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clPostHarvestSkiddingMort : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clPostHarvestSkiddingMort(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clPostHarvestSkiddingMort();

  /**
  * Reads in values from the parameter file, makes sure all data needed is
  * collected, calls the function to setup the time since harvest grid, and
  * registers the codes for harvest intensity.
  *
  * @param p_oDoc Parsed DOM tree of parameter file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality.
  *
  * @param fDbh DBH of tree being evaluated - for seedlings will be 0
  * @param p_oTree Tree being evaluated
  * @param iSpecies Species of the tree being evaluated
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);


  /**
  * Calls the function to calculate time since the last harvest once per timestep.
  *
  * @param p_oPop Pointer to tree population object.

  */

	void PreMortCalcs( clTreePopulation *p_oPop );





  protected:

  /**Parameter for the annual mortality rate before harvesting.**/
  float *mp_fPreHarvestBackgroundMort;
  /**Parameter for the basic post-harvest increase in windthrow mortality.**/
  float *mp_fWindthrowHarvestBasicProb;
  /**Parameter for the basic post-harvest increase in snag-recruitment mortality**/
  float *mp_fSnagRecruitHarvestBasicProb;
  /**Parameter for the increase in post-harvest windthrow mortality with size.**/
  float *mp_fWindthrowSizeEffect;
  /**Parameter for the increase in post-harvest windthrow mortality with nearby harvesting.**/
  float *mp_fWindthrowHarvestIntensityEffect;
  /**Parameter for the increase in post-harvest snag-recruitment mortality with nearby harvesting.**/
  float *mp_fSnagRecruitHarvestIntensityEffect;
  /**Parameter for the decrease in post-harvest windthrow mortality with crowding.**/
  float *mp_fWindthrowCrowdingEffect;
  /**Parameter for the increase in post-harvest snag-recruitment mortality with crowding.**/
  float *mp_fSnagRecruitCrowdingEffect;
  /**Parameter for rate of decrease in post-harvest windthrow mortality with time since harvest.**/
  float *mp_fWindthrowHarvestRateParam;
  /**Parameter for rate of decrease in post-harvest snag-recruitment mortality with time since harvest.**/
  float *mp_fSnagRecruitHarvestRateParam;
  /**Parameter for background post-harvest windthrow mortality.**/
  float *mp_fWindthrowBackgroundProb;
  /**Parameter for background post-harvest snag-recruitment mortality.**/
  float *mp_fSnagRecruitBackgroundProb;

  /**Distance from a tree at which local basal area is calculated for crowding effect.**/
  float m_fCrowdingEffectRadius;


  /**Code for Harvest Type in Harvest Results grid.**/
  int m_iHarvestTypeCode;

  /**Codes for the harvest intensity around each tree.**/
  int **mp_iHarvestIntensityCodes;

  /**Code for Time (since last harvest) in the Years Since Last Harvest grid.**/
  int m_iTimeCode;
  /**Code for LastUpdated (timestep) in the Years Since Last Harvest grid.**/
  int m_iLastUpdated;
  /**Number of X cells in grids.**/
  int m_iNumXCells;
  /**Number of Y cells in grids.**/
  int m_iNumYCells;

  /**Pointer to Years Since Last Harvest grid.**/
  clGrid *mp_oTimeSinceHarvestGrid;
  /**Pointer to Harvest Results grid.**/
  clGrid *mp_oHarvestResultsGrid;



  float m_fNumberYearsPerTimestep; /**<Number of years per timestep*/

  /**
  * This creates the Years Since Harvest grid at the beginning of the simulation, and
  * sets pointers to grids and saves other relevant grid info in class members.
  */
   	void SetupTimeSinceHarvestGrid();


  /**
  * This calculates the basal area (in m2/ha) of all trees within a certain distance of
  * the tree passed as an argument.
  * @param p_oTree Tree being evaluated
  * @return The local basal area of nearby trees.
  */
   	float LocalBasalAreaAroundTree( clTree *p_oTree );


  /**
   * Returns the Time value from the Time Since Harvest grid at the location of a tree. If
   * there has been no harvest in the tree's grid cell, a value of 1000 will be returned.
   * @param p_oTree Tree being evaluated
   * @return The time since harvest, in years, at the tree's location.
   */

    int GetTimeSinceHarvest( clTree *p_oTree );

  /**
  * Updates the Years Since Last Harvest grid.  Note that harvesting occurs at the beginning of the
  * timestep, but mortality occurs at the end of the timestep.  So, if there was harvesting this
  * timestep post-harvest mortality will be calculated annually for 1 to (Years per timestep) years
  * after harvesting.
  */
  	void CalcTimeSinceHarvest();


};
//---------------------------------------------------------------------------
#endif
