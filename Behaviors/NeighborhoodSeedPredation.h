//---------------------------------------------------------------------------
// NeighborhoodSeedPredation.h
//---------------------------------------------------------------------------
#if !defined(NeighborhoodSeedPredation_H)
#define NeighborhoodSeedPredation_H

#include "BehaviorBase.h"

class clGrid;
/**
* Neighborhood Seed Predationon - version 1.1
*
* This behavior reduces the number of seeds by simulating seed predation. Rate
* of offtake is a function of neighborhood composition.
*
* This can be used in two forms. In the standalone form, masting is also used to
* determine offtake. In the linked form, the clFuncResponseSeedPredation class
* determines the offtake rate, and this class figures out where seeds get eaten.
* In this case, masting is ignored.
*
* In the standalone version, there are two choices for assessing masting: in the
* first, masting is assessed by counting the seeds to which this behavior
* applies and which participate in masting (two different things - the latter is
* controlled by a boolean parameter ) and comparing it to a density per timestep
* threshold. Above the threshold is masting; below is not. The second choice
* is to ask the masting disperse behaviors if any species has masted; if any
* have, then it is a masting event. (If masting disperse behaviors are not
* used, then masting never occurs.)  The only difference between masting and
* non-masting timesteps is the parameters used.
*
* For both versions, the proportion of seed removed is calculated as:
* @htmlonly
  <br><center>logit(Y) = p<sub>0</sub> + &Sigma; p<sub>n</sub> * RBA <sub>n</sub></center>
  @endhtmlonly
* where, summing over the species from n = 1...N, p<sub>n</sub> is the loading
* factor for species n, and RBA<sub>n</sub> is the relative basal area of
* species n. There is a species specific minimum DBH for neighbors to include.
*
* In the standalone version, this is used directly as the amount of seed
* removed. In the linked version, it is used to distribute the offtake rate
* from the functional response seed predation behavior. Conceptually, the
* local offtake rate is adjusted by a correction factor, calculated by dividing
* the FR offtake Z by the mean off the local offtakes, so the plot-level offtake
* remains the same. Mathematically it is a little more complicated:
* <ol>
* <li>Calculate logit(Z)</li>
* <li>Calculate the logit(Ys) and subtract the minimum value from each (as well
* as the logit(Z)) so they will all be positive</li>
* <li>Average the logit(Y)s</li>
* <li>Divide logit(Z) by average logit(Y) to get a correction factor</li>
* <li>Multiply each logit(Y) by the correction factor</li>
* <li>Add back the same minimum value formerly subtracted</li>
* <li>Backtransform logit(Y) to Y and use when removing seeds</li>
* </ol>
*
* A fatal error is thrown if a disperse behavior is not also defined for the
* run, and additionally if this is linked and the clFuncResponseSeedPredation
* class is missing.
*
* This class's namestring is "NeighborhoodSeedPredation". If standalone, the
* parameter call string is "NeighborhoodSeedPredation". If linked, the
* parameter call string is "LinkedNeighborhoodSeedPredation".
*
* <br>Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clNeighborhoodSeedPredation : public clBehaviorBase {

public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clNeighborhoodSeedPredation(clSimManager *p_oSimManager);

  /**
   * Destructor.  Frees memory.
   */
  ~clNeighborhoodSeedPredation();

  /**
   * Performs neighborhood seed predation. If the behavior is in linked mode,
   * this calls DoLinkedPredation.  If not, this calls DoStandalonePredation.
   * Before the appropriate predation function is called, this copies the values
   * from the "Dispersed Seeds" grid to the "startseeds_x" data members of the
   * output grid; after the function is called, this calculates and posts the
   * values of the "propeaten_x" data members.
   */
  void Action();

  /**
   * Captures the behavior name passed from the parameter file.  This is useful
   * since this class can produce different kinds of behaviors.
   *
   * @param sNameString Behavior name from parameter file.
   */
  void SetNameData(std::string sNameString);

protected:

  /**
    * Does setup. This reads in values from the parameter file, gets the pointer
    * to the dispersed seeds grid, and sets up the output grid.
    *
    * @param p_oDoc DOM tree of parsed input file.
    * @throws modelErr if this is linked and there is no functional response
    * seed predation.
    */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
  clGrid *mp_oSeedGrid;

  /**Grid to output details of seed predation. The name of this grid
   * Seed Predation Predation". Its cell size matches that of the "Dispersed
   * Seeds" grid.  For each species there are two data members: "startseeds_x"
   * and "propeaten_x" for the pre-predation seed numbers and proportion eaten,
   * respectively, where X is the species number.  Both of the data members are
   * float. Initial conditions maps of this grid are not respected.
   */
  static clGrid *mp_oOutputGrid;

  /**Non-masting "p0" parameter. Array size is number of species to which this
   * behavior applies.*/
  double *mp_fNonMastingP0;

  /**Non-masting pn. Array size is number of behavior species by
   * number of total species.*/
  double **mp_fNonMastingPn;

  /**Masting "p0" parameter. Array size is number of species to which this
   * behavior applies.*/
  double *mp_fMastingP0;

  /**Minimum neighbor DBH.*/
  double *mp_fMinNeighDBH;

  /**Masting pn. Array size is number of behavior species by
   * number of total species.*/
  double **mp_fMastingPn;

  /**Data member codes for seed grid for number of seeds. Array size is number
   * of species to which this behavior applies.*/
  short int *mp_iSeedGridCode;

  /**Data member codes for the "startseeds_x" member of the output grid. Array
   * size is total number of species.*/
  short int *mp_iStartSeedsCode;

  /**Data member codes for the "propeaten_x" member of the output grid. Array
   * size is total number of species.*/
  short int *mp_iPropEatenCode;

  /** For accessing the other arrays.*/
  short int *mp_iIndexes;

  /** Whether or not a species counts when making masting decisions. Only
   * necessary if m_bUseThresholdToDecideMast is true.*/
  bool *mp_bCountInMast;

  /**Area of the plot, in square meters.*/
  float m_fPlotArea;

  /**Radius of neighborhoods.*/
  double m_fRadius;

  /**Threshold of seeds per square meter for a masting timestep. This comes as
   * an annual amount in the parameter file and is converted to a timestep
   * amount. Only necessary if m_bUseThresholdToDecideMast is true. */
  double m_fMastingThreshold;

  /**Whether this is the linked (true) or standalone (false) version*/
  bool m_bIsLinked;

  /**Whether to use a seed density threshold (true) or ask the masting
   * disperse behaviors (false) when deciding whether this is a masting
   * timestep*/
  bool m_bUseThresholdToDecideMast;

  /**
   * Performs neighborhood seed predation for the standalone version of this
   * behavior. First, for the whole plot, this assesses whether masting has
   * occurred by comparing the density of seeds of species to which this
   * behavior applies to the threshold. Then for each cell, the
   * neighborhood relative BAs and the proportion to remove is calculated.
   * A random round is used to take care of fractional parts of seeds. This also
   * checks plot adult tree density; if it is zero, it is automatically not a
   * masting time step.
   */
  void DoStandalonePredation();

  /**
   * Performs neighborhood seed predation for the linked version of this
   * behavior. This calculates all offtakes, then adjusts them according to
   * the offtake calculated by clFuncResponseSeedPredation. A random round is
   * used to take care of fractional parts of seeds.
   */
  void DoLinkedPredation();

  /**
   * Gets the offtake (Y) values for all behavior species for a given grid cell.
   * @param p_oPop Tree population object.
   * @param p_fP0 p0 values to use, as an array sized m_iNumBehaviorSpecies.
   * @param p_fPn pn values to use, as an array sized m_iNumBehaviorSpecies by
   * the total number of species.
   * @param p_fOfftake Place to put offtake values, as an array sized
   * m_iNumBehaviorSpecies.
   * @param iX Seed grid X cell number.
   * @param iY Seed grid Y cell number.
   * @return Array of offtakes sized m_iNumBehaviorSpecies.
   */
    void GetOfftakes(clTreePopulation * p_oPop, const double* p_fP0,
        double **p_fPn, float *p_fOfftake, const int &iX, const int &iY);

    /**
     * Gets a pointer to the "Dispersed Seeds" grid, and sets up the output
     * grid.
     * @throws modelErr if the seed grid is not found
     */
    void SetupGrids();

    /**
   * Reads parameter file data.
   * @param p_oDoc DOM tree of parsed input file.
   * @throws modelErr if any of the following are negative: neighborhood radius,
   * min neighbor DBH, masting threshold
   */
    void ReadParameterFileData(xercesc::DOMDocument *p_oDoc);
};
//---------------------------------------------------------------------------
#endif // NeighborhoodSeedPredation_H
