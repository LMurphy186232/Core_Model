//---------------------------------------------------------------------------
// ClimateChange
//---------------------------------------------------------------------------
#if !defined(ClimateChange_H)
  #define ClimateChange_H

#include "BehaviorBase.h"

class clGrid;


/**
* Climate Change - version 1.0
*
* This changes plot temperature or precipitation over time. There can be two
* versions of this behavior, one doing temperature and one doing precipitation,
* in the same run.
*
* Temperature or precipitation at time t is:
* P = P1 + b * t^c
* where P1 is the starting value, c and b are parameters, and t is time elapsed,
* in years, since the beginning of the run.
*
* The user can set upper and lower bounds on the climate variable.
*
* If this is updating precipitation, the user has the option to update the
* seasonal precipitation by the same proportion of change.
*
* This class's namestring is "climate change". The parameter file call string
* for the temperature version is "TemperatureClimateChange". For
* precipitation, it's "PrecipitationClimateChange".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
  <br>Edit history:
  <br>-----------------
  <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
  <br>November 11, 2013 - Added updating of other precip variables (LEM)
*/
class clClimateChange : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

 public:

 /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
 clClimateChange(clSimManager *p_oSimManager);

 /**
  * Destructor - not needed
  */
 //~clClimateChange();

 /**
  * Reads in values from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  */
 void GetData(xercesc::DOMDocument *p_oDoc);

 /**
  * Updates the plot temperature or precipitation according to the function
  * above.
  */
 void Action();

 /**
  * Captures the namestring passed to this behavior.  This is overridden from
  * clBehaviorBase so we can capture the namestring passed.  Since this class
  * can create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

 protected:

  /** Time, in years, elapsed since the beginning of the run.*/
  float m_fTimeElapsed;

  /** Length of timestep, in years.*/
  float m_fTimestepLength;

  /** B parameter.*/
  float m_fB;

  /** C parameter.*/
  float m_fC;

  /** Starting value, either temperature or precipitation.*/
  float m_fStartingValue;

  /** Lower bound.*/
  float m_fMin;

  /** Upper bound.*/
  float m_fMax;

  /** Whether this is for temperature (True) or precipitation (False).*/
  bool m_bIsTemp;

  /** Whether to update the other precip variables. */
  bool m_bUpdateAllPrecip;

};
//---------------------------------------------------------------------------
#endif // ClimateChange_H

