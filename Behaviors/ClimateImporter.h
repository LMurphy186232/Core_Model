//---------------------------------------------------------------------------
// ClimateImporter
//---------------------------------------------------------------------------
#if !defined(ClimateImporter_H)
  #define ClimateImporter_H

#include "BehaviorBase.h"


/**
* Climate importer (and manager)
*
* This calculates timestep climate variables based on imported climate data.
* It will calculate all values ahead of time and its only job each timestep
* will be to set the correct values.
*
* Annual values are calculated from monthly values. There are two possible
* definitions of "annual": based on the calendar year (Jan-Dec) or based on
* growing season (Oct-Sep).
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
* This can also calculate the long-term mean of these values. There is a
* parameter for X number of years for "long-term". The mean for each timestep
* is the mean of the X prior years. If this is used, there must be sufficient
* data supplied for the years prior to the start of the run. A value of 0
* means that there will be no long-term calculations done.
*
* This expects monthly values to be provided for each timestep. If the long-
* term mean is used, data must be provided for time before the run starts so
* that there are good long-term means for the first timesteps. These will
* be given negative timestep numbers. (There is no year 0.) For example:
*
* <sc_ciMonthlyTempJan>
*   <sc_cimtjanVal ts="-3">-7.76</sc_cimtjanVal>
*   <sc_cimtjanVal ts="-2">-0.72</sc_cimtjanVal>
*   <sc_cimtjanVal ts="-1">1.3</sc_cimtjanVal>
*   <sc_cimtjanVal ts="1">-2.57</sc_cimtjanVal>
*   <sc_cimtjanVal ts="2">-3.46</sc_cimtjanVal>
*   <sc_cimtjanVal ts="3">-6.93</sc_cimtjanVal>
* </sc_ciMonthlyTempJan>"
*
* All values are then stored in clPlot.
*
* This class's namestring is "ClimateImporter". The parameter file call
* string is "ClimateImporter".
*
*
* Copyright Charles D. Canham.
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

  /** Long term mean annual precipitation. Array length is # timesteps. */
  double *mp_fLTMPpt;

  /** Long term mean annual mean temperature. Array length is # timesteps. */
  double *mp_fLTMTemp;

  /** Long term mean seasonal precipitation. Array length is # timesteps. */
  double *mp_fLTMSeasonalPpt;

  /** Long term mean water deficit. Array length is # timesteps.*/
  double *mp_fLTMWD;

  /** Length of time of long-term mean, if desired */
  int m_iLTM;

  /**
   * Read parameter file data.
   * @param p_oElement Parent element to read from.
   * @param p_fPpt Array in which to read monthly precipitation data.
   * @param p_fTemp Array in which to read monthly temperature data.
   * @param p_fPrePpt Array in which to read pre-run monthly precipitation
   * data, if using a long-term mean. This can be NULL if not.
   * @param p_fPreTemp Array in which to read pre-run monthly temperature
   * data, if using a long-term mean. This can be NULL if not.
   * @param p_fRad Array in which to read monthly radiation data.
   * @param iStart Starting year. Will be negative if long-term mean values
   * are required, 1 otherwise
   */
  void ReadParameterFileData(DOMElement * p_oElement, double **p_fPpt,
      double **p_fTemp, double **p_fPrePpt, double **p_fPreTemp,
      double *p_fRad, int iStart);

  /**
   * Does the heavy lifting to extract XML values.
   * @param p_oParent Parent tag to look in.
   * @param sParentTag Tag of parent element.
   * @param sSubTag Tag of children of parent.
   * @param p_fVal Array to read into.
   * @param p_fPreVal Second array of before-the-timestep values, if requred
   * for long-term mean; can be NULL otherwise.
   * @param iStart Starting year. Will be negative if long-term mean values
   * are required, 1 otherwise
   * @param iEnd Ending year. In case there is extra data, this will not
   * try to run past the array end.
   */
  void ReadMonthlyData(xercesc::DOMElement *p_oParent, std::string sParentTag,
      std::string sSubTag, double *p_fVal, double *p_fPreVal, int iStart,
      int iEnd);
};
//---------------------------------------------------------------------------
#endif // ClimateImporter_H

