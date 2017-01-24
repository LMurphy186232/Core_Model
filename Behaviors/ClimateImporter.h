//---------------------------------------------------------------------------
// ClimateImporter
//---------------------------------------------------------------------------
#if !defined(ClimateImporter_H)
  #define ClimateImporter_H

#include "BehaviorBase.h"


/**
* Seasonal Water Deficit - version 1.0
*
* This calculates timestep climate variables based on imported climate data.
* It will calculate all values ahead of time and its only job each timestep
* will be to set the correct values.
*
* Annual precipitation is summed from monthly precipitation for the timestep.
*
* Mean annual temperature is calculated by averaging monthly temperatures for
* the timestep.
*
* Water deficit is calculated as in the water deficit behavior.
*
* Seasonal precipitation is soil water storage, plus growing season precip,
* plus non-growing-season PET. The growing season is months where PET >= PPT.
*
* All values are then stored in clPlot.
*
* This class's namestring is "ClimateImporter". The parameter file call
* string is "ClimateImporter".
*
*
* Copyright 2017 Charles D. Canham.
* @author Lora E. Murphy
*
  <br>Edit history:
  <br>-----------------
  <br>January 19, 2017 - Created (LEM)
*/
class clClimateImporter : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

 public:

 /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
 clClimateImporter(clSimManager *p_oSimManager);

 /**
  * Destructor
  */
 ~clClimateImporter();

 /**
  * Reads in values from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if proportion of rain values don't add up to 1.
  */
 void GetData(xercesc::DOMDocument *p_oDoc);

 /**
  * Updates the plot seasonal water deficit.
  */
 void Action();

 protected:

  /** Annual precipitation. Array length is # timesteps. */
  double *mp_fPpt;

  /** Annual mean temperature. Array length is # timesteps. */
  double *mp_fTemp;

  /** Seasonal precipitation. Array length is # timesteps. */
  double *mp_fSeasonalPpt;

  /** Water deficit. Array length is # timesteps.*/
  double *mp_fWD;

  /**
   * Read parameter file data.
   * @param p_oElement Parent element to read from.
   * @param p_fPpt Array in which to read monthly precipitation data.
   * @param p_fTemp Array in which to read monthly temperature data.
   * @param p_fRad Array in which to read monthly radiation data.
   */
  void ReadParameterFileData(DOMElement * p_oElement, double **p_fPpt,
      double **p_fTemp, double *p_fRad);

  /**
   * Does the heavy lifting to extract XML values.
   * @param p_oParent Parent tag to look in.
   * @param sParentTag Tag of parent element.
   * @param sSubTag Tag of children of parent.
   * @param p_fVal Array to read into.
   */
  void ReadMonthlyData(xercesc::DOMElement *p_oParent, std::string sParentTag,
      std::string sSubTag, double *p_fVal);
};
//---------------------------------------------------------------------------
#endif // ClimateImporter_H

