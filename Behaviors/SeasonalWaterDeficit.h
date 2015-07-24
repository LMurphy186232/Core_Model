//---------------------------------------------------------------------------
// SeasonalWaterDeficit
//---------------------------------------------------------------------------
#if !defined(SeasonalWaterDeficit_H)
  #define SeasonalWaterDeficit_H

#include "BehaviorBase.h"

class clGrid;


/**
* Seasonal Water Deficit - version 1.0
*
* This calculates seasonal water deficit based on current annual temperature
* and precipitation.
*
* Annual precipitation is first partitioned by month, by multiplying the total
* by the fraction of precipitation that falls in each month (entered as a
* parameter). Then monthly temperature is calculated, by multiplying annual
* mean temperature by the ratio of the month's mean to annual mean (entered as
* a parameter).
*
* Then PET (monthly potential evapotranspiration) is calculated as:
*
* PET<sub>i</sub> = 0.013 * [T/(T+15)] * (Rs + 50)
*
* Where
* <ul>
* <li>PET is monthly potential evapotranspiration in mm</li>
* <li>T is mean monthly temperature in degrees C</li>
* <li>Rs is monthly global radiation received at the earth's surface, in cal
* cm-2 (entered as a parameter)</li>
* </ul>
*
* If T &le; 0, PET = 0.
*
* Then monthly soil moisture storage is calculated as:
*
* sms<sub>i</sub> = 0 &lt; sms<sub>i-1</sub> + precip<sub>i</sub> - PET<sub>i</sub> &lt; MaxAWS
*
* where:
* <ul>
* <li>precip<sub>i</sub> is the monthly precipitation (mm) for the current month</li>
* <li>PET<sub>i</sub> is for the current month</li>
* <li>MaxAWS is the available soil water storage, in mm (entered as a parameter)</li>
* </ul>
*
* January's previous sms = MaxAWS.
*
* Then monthly actual evapotranspiration (AET) is calculated as:
*
* AET<sub>i</sub> = sms<sub>i</sub> + precip<sub>i</sub> if sms<sub>i+1</sub> &le; 0;
* or
* AET<sub>i</sub> = PET<sub>i</sub> if sms<sub>i+1</sub> &gt; 0.
*
* Then annual AET is the sum of monthly AET, and annual PET is the sum of
* monthly PET; and Seasonal Water Deficit = PET - AET.
*
* This value is then stored in clPlot.
*
* This class's namestring is "SeasonalWaterDeficit". The parameter file call
* string is "SeasonalWaterDeficit".
*
*
* Copyright 2014 Charles D. Canham.
* @author Lora E. Murphy
*
  <br>Edit history:
  <br>-----------------
  <br>January 22, 2014 - Created (LEM)
*/
class clSeasonalWaterDeficit : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

 public:

 /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
 clSeasonalWaterDeficit(clSimManager *p_oSimManager);

 /**
  * Destructor
  */
 ~clSeasonalWaterDeficit();

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

  /** Proportion of rain that falls in each month. Array length is 12. */
  float *mp_fPropRainfall;

  /** Radiation for each month, cal/cm2. Array length is 12. */
  float *mp_fRadiation;

  /** Ratio of monthly temp to annual. Array length is 12. */
  float *mp_fRatioMonthlyAnnualTemp;

  /** Monthly temperature. Calculated each timestep. Array length is 12. */
  float *mp_fMonthlyTemp;

  /** Monthly precip. Calculated each timestep. Array length is 12. */
  float *mp_fMonthlyPrecip;

  /** Monthly PET. Calculated each timestep. Array length is 12. */
  float *mp_fMonthlyPET;

  /** Monthly soil moisture. Calculated each timestep. Array length is 12. */
  float *mp_fMonthlySMS;

  /** Monthly AET. Calculated each timestep. Array length is 12. */
  float *mp_fMonthlyAET;

  /** Available water storage. */
  float m_fAWS;

};
//---------------------------------------------------------------------------
#endif // SeasonalWaterDeficit_H

