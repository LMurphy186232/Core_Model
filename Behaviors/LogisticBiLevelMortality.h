//---------------------------------------------------------------------------

#ifndef LogisticBiLevelMortalityH
#define LogisticBiLevelMortalityH
//---------------------------------------------------------------------------
#include "MortalityBase.h"

class clGrid;
class clTreePopulation;
/**
* Logistic Bi-Level Mortality - Version 1.0
*
* This evaluates mortality according to a logistic equation, with the
* possibility of two sets of parameters for each species.  The two sets of
* parameters can be used for two different mortality rates at high and low
* light.
*
* The equation used in this behavior is:
* <center>p = exp(a + b * D) / (1 + exp(a + b * D))</center>
* where
* <ul>
* <li>p = annual probability of survival</li>
* <li>a = parameter</li>
* <li>b = parameter</li>
* <li>D = tree diameter, in cm; diam10 for seedlings, DBH for others</li>
* </ul>
*
* This behavior can also take into account light levels coming from the "Storm
* Light" grid object produced by clStormLight.  This behavior can use two
* different sets of parameter values - one at low light and one at high light.
* The user sets the threshold between the two.  The equation remains the same.
*
* This class's namestring is "logistic bilevel mortshell".  The parameter file
* call string is "LogisticBiLevelMortality".
*
* Copyright 2005 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>May 5, 2005 - Created (LEM)
*/
class clLogisticBiLevelMortality : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clLogisticBiLevelMortality(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clLogisticBiLevelMortality();

  /**
  * Reads in values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality according to the logistic equation.  If the light grid
  * is present, this retrieves the light level in the tree's grid cell.  If it
  * is above the threshold, the high-light parameters are used.  If it is below
  * the threshold, the low-light parameters are used.  If the light grid is not
  * present, the low-light parameters are used.
  *
  * @param p_oTree Tree being evaluated
  * @param fDbh Tree's DBH
  * @param iSpecies Species of the tree being evaluated
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /**"Storm Light" grid object*/
  clGrid *mp_oStormLight;

  /**Tree population - for getting data codes*/
  clTreePopulation *mp_oPop;

  /**Mortality equation low light "b" - sized number of behavior species*/
  float *mp_fLoLightB;

  /**Mortality equation low light "a" - sized number of behavior species*/
  float *mp_fLoLightA;

  /**Mortality equation high light "b" - sized number of behavior species*/
  float *mp_fHiLightB;

  /**Mortality equation high light "a" - sized number of behavior species*/
  float *mp_fHiLightA;

  /**Threshold between low light and high light mortality, as a value between 0
  * and 100 - sized number of behavior species*/
  float *mp_fHiLightThreshold;

  /**To help access the other arrays*/
  int *mp_iIndexes;

  /**Conversion factor to translate the results of the function to the
  * appropriate units per timestep*/
  float m_fYearsPerTimestep;

  /**Code for the "Light" data member of the "Storm Light" grid*/
  int m_iLightCode;

};
//---------------------------------------------------------------------------
#endif
