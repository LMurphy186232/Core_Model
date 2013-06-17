//---------------------------------------------------------------------------
// StateReporter.h
//---------------------------------------------------------------------------
#if !defined(StateReporter_H)
  #define StateReporter_H

#include "BehaviorBase.h"

class clGrid;

/**
* State reporter
*
* This behavior collects state information variables and stores them in a grid
* where they are available for output and reporting.
*
* The values are collected into a grid called "State Variables". Currently
* saved are temperature and precipitation.
*
* This class's namestring and parameter file call string are "StateReporter".
* Any tree type/species assignments are ignored.
*
* <br>Edit history:
* <br>-----------------
* <br>January 7, 2010 - Created (LEM)
*/
class clStateReporter : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clStateReporter(clSimManager *p_oSimManager);

  /**
  * Destructor. Not needed.
  */
  //~clStateReporter();

  /**
  * Retrieves and stores climate information in the "State Variables" grid.
  */
  void Action();

  /**
  * Does setup for this behavior. Calls Action() so that the initial conditions
  * value will be added.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc) {SetupGrid(); Action();};

  protected:

  /**
   * Sets up the "State Variables" grid and registers the data members.
   */
  void SetupGrid();

  /** Grid holding state variables, named "State Variables". One cell grid for
   * the plot. Two float members: "Temp.C" holds temperature, and "Precip.mm"
   * holds precipitation.*/
  clGrid* mp_oGrid;


  /** Holds the code for the "Temp.C" data member of the "State Variables"
   * grid.*/
  short int m_iTempGridCode;

  /** Holds the code for the "Precip.mm" data member of the "State Variables"
   * grid.*/
  short int m_iPrecipGridCode;
};
//---------------------------------------------------------------------------
#endif //StateReporter_H
