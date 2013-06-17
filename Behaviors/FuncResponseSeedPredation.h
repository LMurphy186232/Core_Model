//---------------------------------------------------------------------------
// FuncResponseSeedPredation.h
//---------------------------------------------------------------------------
#if !defined(FuncResponseSeedPredation_H)
#define FuncResponseSeedPredation_H

#include "BehaviorBase.h"

class clGrid;
/**
 * Models functional response seed predation.  This runs as a
 * model-within-a-model, in weekly timesteps for a year.  It doesn't matter if
 * the length of the overall model timestep is longer than a year; the amount
 * of seeds is treated as a one-year pool, which produces the same result.
 *
 * This behavior reduces the number of seeds available in the "Dispersed Seeds"
 * grid. In each cell of that grid, there are a certain number of predators
 * (calculated from the initial density of predators) which have the number of
 * seeds present in that cell as a food source. Each species to which this
 * behavior is applied has its own parameters controlling how it is predated.
 *
 * Seed rain is evenly divided over a set number of timesteps (weeks). The
 * predator population has as a food source the number of seeds added
 * during the current week's rain (if the rain is going on) plus any leftover
 * seeds from previous weeks which have not been consumed.  Beginning at a
 * certain week in the spring, the number of seeds available to the mice is
 * further reduced by a certain percentage each week to simulate germination.
 * Once germination begins, it continues until the predator model finishes
 * running.  In order to correctly calculate mouse consumption and ensure that
 * the seeds which germinate are actually available later, this keeps track of
 * the seeds consumed; it is this number which is subtracted from total seeds
 * at the end.
 *
 * The predator population is updated each timestep as a response to number of
 * seeds consumed. The response parameters can be divided into two seasons.
 *
 * Seed offtake for each week is calculated as<br>
 * @htmlonly
   <center>O = &Sigma; <i>IR<sub>s</sub></i> * <i>N</i></center>
   @endhtmlonly
 * where
 * <ul>
 * <li><i>0</i> = offtake
 * <li><i>IR<sub>s</sub></i> = per capita seed offtake for each species
 * <li><i>N</i> = number of mice
 * </ul>
 * Per capita seed offtake for each species is<br>
 * <center>IR<sub>s</sub> = <i>c<sub>s</sub></i>(1 - <i>e<sup>-(<i>S*D</i>))
 * </sup> * p<sub>s</sub></i></center>
 * where
 * <ul>
 * <li><i>c<sub>s</sub></i> = maximum seed intake rate per predator per day
 * (a species-specific input parameter)
 * <li><i>S</i> = number of seeds per predator per day
 * <li><i>D</i> = foraging efficiency (a species-specific input parameter)
 * <li><i>p<sub>s</sub></i> = the proportion of the total seeds belonging to
 * that species</li>
 * </ul>
 * The number of predators in each cell's population is calculated as<br>
 * <center><i>N<sub>t</sub></i> = <i>N<sub>t-1</sub></i> *
 * <i>e<sup>r<sub>t-1</sub></sup></i></center><br>
 * where
 * <ul>
 * <li><i>N<sub>t</sub></i> = number of predators for the current timestep
 * <li><i>N<sub>t-1</sub></i> = number of predators in the previous timestep
 * <li><i>r<sub>t-1</sub></i> = instantaneous rate of change in predator
 * abundance for the previous timestep.
 * </ul>
 * The instantaneous rate of change, r, can use different parameters for two
 * user-defined seasons. r is calculated as<br>
 * <center><i>r</i> = (<i>a</i> + <i>d(IR)</i> + <i>g(N)</i>)/12</center>
 * where
 * <ul>
 * <li><i>IR</i> = sum of the IRs for each species</li>
 * <li><i>a</i> = the maximum instantaneous rate at which predator abundance
 * delines in the absense of food (an input parameter, season 1 or 2)
 * <li><i>d</i> = population's demographic efficiency (an input parameter,
 * season 1 or 2)
 * <li><i>g</i> = coefficient describing the effect that density-dependent
 * factors have on <i>r</i> (an input parameter, season 1 or 2)
 * <li><i>N</i> = number of predators per hectare
 * <li>divided by 12 - to take from per quarter to per week</li>
 * </ul>
 *
 * This behavior must be used in conjunction with a disperse behavior.  If such
 * a behavior is not present (and thus the "Dispersed Seeds" grid is not
 * present), a fatal error will be thrown during setup.
 *
 * This behavior can be used as an independent behavior where seeds are removed,
 * or it can be linked to another behavior (i.e. the neighborhood seed predation
 *  behavior, clNeighborhoodSeedPredation) and used to calculate a whole-plot
 * offtake amount without actually removing any seeds. It's possible to use
 * both in a single run so separate parameters are used for each.
 *
 * The namestring and parameter file call string are
 * "FunctionalResponseSeedPredation" when used alone, or
 * "LinkedFunctionalResponseSeedPredation" when linked with another behavior.
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */
class clFuncResponseSeedPredation : public clBehaviorBase {

public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clFuncResponseSeedPredation(clSimManager *p_oSimManager);

  /**
   * Destructor.  Frees memory.
   */
  ~clFuncResponseSeedPredation();

  /**
   * Performs the model.  If predator densities are not to be preserved, then
   * the predator grid is initialized with SetPredatorInitialDensity().
   */
  void Action();

  /**
   * Captures the behavior name passed from the parameter file.  This is useful
   * since this class can produce different kinds of behaviors.
   *
   * @param sNameString Behavior name from parameter file.
   */
  void SetNameData(std::string sNameString);

  /**
   * Get the amount of offtake.
   * @return Offtake, as a proportion of seeds eaten between 0 and 1.
   */
  float GetOfftakeRate() {return m_fOfftake;};

protected:

  /**
   * Reads in the parameter file values.
   * @param p_oDoc DOM tree of parsed input file.
   * @throw modelErr if there is no such grid called "Dispersed Seeds", or
   * if the "Dispersed Seeds" grid does not contain a valid return code for all
   * behavior species.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Sets all grid cells in the predator grid with the appropriate initial
   * density.
   */
  void SetPredatorInitialDensity();

  /**
   * Gets the data from the parameter file.
   * @param p_oDoc DOM tree of parsed input file.
   * @throw modelErr if:
   * <ul>
   * <li>Number of weeks of seedfall is less than or equal to zero
   * <li>Initial predator density is less than or equal to zero
   * <li>Number of weeks to run the model is less than 0 or greater than 52
   * <li>Week to start germination is less than 0 or greater than 52
   * <li>Proportion germinating isn't between 0 and 1
   * </ul>
   */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
   * Does the model for a given value of m_fTempSeeds and m_fTempPredators.
   */
  void DoModel();

  /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
  clGrid *mp_oSeedGrid;

  /**Pointer to predator grid.  The name of this grid is "Seed Predators".
   * Its grid cell resolution will match that of "Dispersed Seeds".  It has
   * one float data member - "num preds".  It will accept maps if
   * m_bPreservePredatorDensities = true and if they are of the right
   * resolution.*/
  clGrid *mp_oPredatorGrid;

  /**File to which to write intermediate output, if desired*/
  std::string m_sOutput;

  /**Max instantaneous rate at which predator abundance declines in the
   * absense of food, number of predators per week - array index is 2 (#
   * seasons)*/
  float *mp_fMaxInstantaneousDeclineRate;

  /**Population's demographic efficiency - array size is 2 (# seasons)*/
  float *mp_fDemographicEfficiency;

  /**Density-dependent coefficient - array size is 2 (# seasons)*/
  float *mp_fDensityDependentCoefficient;

  /**Foraging efficiency - array size is # behavior species*/
  float *mp_fForagingEfficiency;

  /**Predator initial density - number per square meter*/
  float m_fPredatorInitialDensity;

  /**Maximum seed intake rate - number of seeds per predator per day - array
   * size is # behavior species*/
  float *mp_fMaxIntakeRate;

  /**Proportion of seeds that germinate each week in the germination
   * period - this must be a number between 0 and 1*/
  float m_fProportionGerminating;

  /**Area of a seed grid grid cell in square meters - when multiplied by the
   * density of predators, produces the number in the cell*/
  float m_fCellArea;

  /** Offtake rate - proportion of total seeds eaten, between 0 and 1.
   * Used if this is linked. */
  float m_fOfftake;

  /** A place to stash the number of seeds, to allow easy collaborative access
   * to this data between functions. Array size is # behavior species. */
  float *mp_fTempSeeds;

  /** A place to stash the number of predators, to allow easy collaborative
   * access to this data between functions */
  float m_fTempPredators;

  /**Number of weeks to run the model*/
  int m_iNumWeeksToModel;

  /**Number of weeks of seed fall*/
  int m_iNumWeeksSeedFall;

  /**Week in which germination begins*/
  int m_iWeekGerminationStarts;

  /**Week in which season 2 begins*/
  int m_iWeekSeason2Starts;

  /**Data member codes for seed grid for number of seeds. Array size is #
   * behavior species.*/
  short int *mp_iSeedGridCode;

  /**Data member in the predators grid*/
  short int m_iNumPredsCode;

  /**Whether or not to carry over mouse population densities*/
  bool m_bPreservePredatorDensities;

  /**Whether this is the linked (true) or standalone (false) version*/
  bool m_bIsLinked;
};
//---------------------------------------------------------------------------
#endif // FuncResponseSeedPredation_H
