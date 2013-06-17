//---------------------------------------------------------------------------

#ifndef WeibullClimateSurvivalH
#define WeibullClimateSurvivalH
//---------------------------------------------------------------------------
#include "MortalityBase.h"


class clTree;
class clTreePopulation;

/**
* Weibull Climate survival - Version 1.0
*
* This is a mortality shell object which calculates survival as a function of
* climate and neighbor density.
*
* The equation for one year's survival is:
* <center>Probability of survival = Max Probability * Size Effect * Precipitation Effect * Climate Effect * Competition Effect</center>
* where Max Probability is the maximum annual survival possible, and the Effects
* are values between 0 and 1 which serve to reduce the maximum.
*
* The equation for Size Effect is:
* <center>SE = exp(-0.5(ln(DBH/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</center>
*
* where:
* <ul>
* <li>DBH is for the target tree, in cm</li>
* <li>X<sub>0</sub> is the size effect mode, in cm</li>
* <li>X<sub>b</sub> is the size effect variance, in cm</li>
* </ul>
*
* Size effect is subject to a minimum value for DBH, below which all trees will
* just get the minimum.
*
* Precipitation Effect and Temperature Effect use the same function form.
* The function is:
* @htmlonly
  Climate Effect <- exp(-0.5*(abs(CV - C)/A)<sup>B</sup>)
  @endhtmlonly
* where:
* <ul>
* <li>Climate Effect is either Precipitation Effect or Temperature Effect</li>
* <li>CV is either the mean annual precipitation in mm or the mean annual
* temperature in C, from the plot object</li>
* <li>A, B, and C are parameters</li>
* </ul>
*
* Competition Effect is calculated as:
* <br>
* @htmlonly
  <center>CE = exp(-(C * DBH<sup> &gamma; </sup> * nd<sup>D</sup>)</center>
  @endhtmlonly
* <br>
* where:
* <ul>
* <li>CE = competition effect</li>
* <li>C is the competition effect C parameter</li>
* <li>D is the competition effect D parameter</li>
* <li>DBH is of the target tree, in cm</li>
* <li>nd is the number of neighbors with a DBH larger than both the minimum</li>
* neighbor DBH parameter and the target tree DBH, within the specified
* radius</li>
* <li>@htmlonly &gamma; @endhtmlonly is the competition effect
* gamma parameter</li>
* </ul>
*
* Snags, seedlings, and trees that are already dead from disturbance events are
* never counted in the neighbor count.
*
* For multi year time steps, the annual probability of survival is raised to the
* power of the number of years per time step.
*
* This cannot be applied to seedlings.  An error will be thrown if seedlings
* are passed.
*
* The parameter file call string for this is "WeibullClimateSurvival". The
* namestring for this behavior is "WeibullClimatemortshell".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clWeibullClimateSurvival : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clWeibullClimateSurvival(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clWeibullClimateSurvival();

  /**
  * Determines mortality for a tree. For this tree, count the number of larger
  * neighbors.  Then calculate the probability of survival for each using the
  * equations above. Use the random number generator to decide life or death.
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
  * </ol>
  *
  * @param p_oDoc DOM tree of parsed input tree.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates temperature and precipitation effects for each species for the
  * current time step.
  * @param p_oPop Tree population.
  */
  void PreMortCalcs( clTreePopulation *p_oPop );

  protected:

  /**Maximum potential annual survival value. Array is sized number of species
   * to which this behavior applies.*/
  float *mp_fMaxRG;

  /**Competition effect C. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fCompC;

  /**Competition effect D. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fCompD;

  /**Size sensitivity to crowding parameter.
  * @htmlonly &gamma; @endhtmlonly in Competition Effect equation above. Array
  * is sized number of species to which this behavior applies.*/
  float *mp_fGamma;

  /**Size effect X0. This must be an array of doubles in order to support very
   * small values. Array is sized number of species to which this behavior
   * applies.*/
  double *mp_fSizeX0;

  /**Size effect Xb. Array is sized number of species to which this behavior
   * applies.*/
  float *mp_fSizeXb;

  /**Precipitation effect A. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fPrecipA;

  /**Precipitation effect B. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fPrecipB;

  /**Precipitation effect C. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fPrecipC;

  /**Temperature effect A. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fTempA;

  /**Temperature effect B. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fTempB;

  /**Temperature effect C. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fTempC;

  /**Temperature effect.  Keep a copy so we only have to calculate once per
   * time step.  Array is sized number of species to which this behavior
   * applies.*/
  float *mp_fTempEffect;

  /**Precipitation effect. Keep a copy so we only have to calculate once per
   * time step. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fPrecipEffect;

  /**Maximum search radius, in meters, in which to look for crowding
  * neighbors. For calculating the Competition Effect. Array is sized number of
  * species to which this behavior applies.*/
  float *mp_fMaxCrowdingRadius;

  /**The minimum DBH, in cm, of neighbors to be included in the neighbor count.
  * Array is sized number of species.*/
  float *mp_fMinimumNeighborDBH;

  /**Size effect minimum DBH. Array is sized number of species to which this
   * behavior applies.*/
  float *mp_fSizeMinDBH;

  /**Speeds access to the arrays. Array size is number of species.*/
  short int *mp_iIndexes;

  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Keep our own copy for the destructor. This is the total number of tree
  * species.*/
  short int m_iNumTotalSpecies;

  /**
  * Makes sure all input data is valid. The following must all be true:
  * <ul>
  * <li>Max radius of neighbor effects must be > 0</li>
  * <li>Max survival for each species must be between 0 and 1</li>
  * <li>Temp and precip A for each species must be > 0</li>
  * <li>X0 (size effect mode) for each species must be > 0</li>
  * <li>Xb (size effect variance) for each species must not = 0</li>
  * </ul>
  * @throws modelErr if any of the above conditions are not met.
  */
  void ValidateData();

    /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input tree.
  * @throws modelErr if this behavior has been applied to any types except
  * sapling and adult.
  */
  void ReadParameterFile( xercesc::DOMDocument *p_oDoc );

  /**
  * Counts the number of trees a target's neighborhood that have a DBH bigger
  * than the target. Neighbors must have a DBH greater than the minimum. They
  * also cannot be dead from a disturbance event; but any trees that have a
  * dead code of "natural" are assumed to have died in the current time step
  * mortality cycle and thus should be counted.
  * @param p_oTarget Target tree for which to count the larger neighbors.
  * @returns Number of larger neighbors.
  */
  int GetNumLargerNeighbors(clTree *p_oTarget);

};
//---------------------------------------------------------------------------
#endif
