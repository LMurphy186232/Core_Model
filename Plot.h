//---------------------------------------------------------------------------

#ifndef PlotH
#define PlotH

#include "WorkerBase.h"
#include <math.h>
/**
* PLOT CLASS
* This class represents the underlying plot.  It controls for torus topology
* and manages the grid coordinates. It also contains geographic and climate
* information about the plot, for those behaviors that require it.
*
* There are some changes in the coordinate structure.  The origin is in the
* southwest corner of the plot (unwrapped).  The Y axis is NS and values
* increase to the north.  The X axis is EW and values increase to the east.
* There are no negative values.  The azimuth is north zero, east positive.
*
* The truly underlying grid cell structure is no longer in the hands of the
* users.  The base coordinates will be structured as 8 X 8 grid  cells, which
* is what the trees will be organized into.
*
* This class inherits from clWorkerBase to take advantage of the XML parsing
* functions in that class.
*
* Copyright Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
* <br>June 27, 2013 - Added N deposition value (LEM)
* <br>October 31, 2013 - Added water deficit and seasonal precipitation
* <br>August 5, 2023 - Added long-term means (LEM)
*/
class clPlot : virtual public clWorkerBase {

 public:

  /**
  * Destructor.
  */
  ~clPlot();

 /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clPlot(clSimManager *p_oSimManager);
 //clPlot(const clPlot &oldPlot); //copy constructor


 /**
  * Gets the latitude.
  * @return Latitude, in decimal degrees.
  */
  double GetLatitude() {return m_fLatitude;};

 /**
  * Gets the plot X length.
  * @return Length of plot in X direction in meters.
  */
  double GetXPlotLength() {return m_fPlotLenX;};

 /**
  * Gets the plot Y length.
  * @return Length of plot in Y direction in meters.
  */
  double GetYPlotLength() {return m_fPlotLenY;};

 /**
  * Gets the length of cells in the X direction.
  * @return Length of cells in the X direction, in m.
  */
  float GetXCellLength() {return m_iCellSize;};

 /**
  * Gets the length of cells in the Y direction.
  * @return Length of cells in the Y direction, in m.
  */
  float GetYCellLength() {return m_iCellSize;};

 /**
  * Gets the plot area.
  * @return Plot area, in hectares.
  */
  float GetPlotArea() {return m_fPlotArea;};

  /**
   * Gets the mean annual precipitation.
   * @return Mean annual precipitation, in mm.
   */
  double GetMeanAnnualPrecip() {return m_fMeanAnnualPrecipMm;};

  /**
   * Sets the mean annual precipitation.
   * @param fMeanAnnualPrecip Mean annual precipitation, in mm.
   */
  void SetMeanAnnualPrecip(double fMeanAnnualPrecip)
    {m_fMeanAnnualPrecipMm = fMeanAnnualPrecip;};

  /**
   * Gets the long-term mean annual precipitation.
   * @return Long-term mean annual precipitation, in mm.
   */
  double GetLTMAnnualPrecip() {return m_fLTMPrecipMm;};

  /**
   * Sets the long-term mean annual precipitation.
   * @param fLTMPrecip Long-term mean annual precipitation, in mm.
   */
  void SetLTMAnnualPrecip(double fLTMPrecip)
  {m_fLTMPrecipMm = fLTMPrecip;};

  /**
   * Gets the mean annual temperature.
   * @return Mean annual precipitation, in degrees C.
   */
  double GetMeanAnnualTemp() {return m_fMeanTempC;};

  /**
   * Sets the mean annual temperature.
   * @param fMeanTemp Mean annual precipitation, in degrees C.
   */
  void SetMeanAnnualTemp(double fMeanTemp)
    {m_fMeanTempC = fMeanTemp;};

  /**
   * Gets the long-term mean annual temperature.
   * @return Long-term mean annual precipitation, in degrees C.
   */
  double GetLTMAnnualTemp() {return m_fLTMTempC;};

  /**
   * Sets the long-term mean annual temperature.
   * @param fLTMTemp Mean annual precipitation, in degrees C.
   */
  void SetLTMAnnualTemp(double fLTMTemp)
  {m_fLTMTempC = fLTMTemp;};

  /**
   * Gets the annual seasonal precipitation.
   * @return The annual seasonal precipitation.
   */
  double GetSeasonalPrecipitation() {
    return m_fSeasonalPrecipitation;
  }

  /**
   * Sets the annual seasonal precipitation.
   * @param fSeasonalPrecipitation The annual seasonal precipitation.
   */
  void SetSeasonalPrecipitation(double fSeasonalPrecipitation) {
    m_fSeasonalPrecipitation = fSeasonalPrecipitation;
  }

  /**
   * Gets the long-term mean seasonal precipitation.
   * @return The long-term mean seasonal precipitation.
   */
  double GetLTMSeasonalPrecipitation() {
    return m_fLTMSeasonalPrecipitation;
  }

  /**
   * Sets the long-term mean seasonal precipitation.
   * @param fLTMSeasonalPrecipitation Long-term mean seasonal precipitation.
   */
  void SetLTMSeasonalPrecipitation(double fLTMSeasonalPrecipitation) {
    m_fLTMSeasonalPrecipitation = fLTMSeasonalPrecipitation;
  }

  /**
   * Gets the annual water deficit.
   * @return The annual water deficit.
   */
  double GetWaterDeficit() {
    return m_fWaterDeficit;
  }

  /**
   * Sets the annual water deficit.
   * @param fWaterDeficit The annual water deficit.
   */
  void SetWaterDeficit(double fWaterDeficit) {
    m_fWaterDeficit = fWaterDeficit;
  }

  /**
   * Gets the long-term mean annual water deficit.
   * @return The long-term mean annual water deficit.
   */
  double GetLTMWaterDeficit() {
    return m_fLTMWaterDeficit;
  }

  /**
   * Sets the long-term mean annual water deficit.
   * @param fLTMWaterDeficit The long-term mean annual water deficit.
   */
  void SetLTMWaterDeficit(double fLTMWaterDeficit) {
    m_fLTMWaterDeficit = fLTMWaterDeficit;
  }

  /**
   * Gets the annual N deposition.
   * @return Annual N deposition.
   */
  double GetNDeposition() {return m_fNDep;};

  /**
   * Sets the annual N deposition.
   * @param fNDep Annual N deposition.
   */
  void SetNDeposition(double fNDep)
  {m_fNDep = fNDep;};

 /**
  * Gets the plot title.
  * @return Plot title.
  */
  std::string GetPlotTitle() {return m_sPlotTitle;};

 /**
  * Gets number of X grid cells.
  * @return Number of grid cells in the X direction.
  */
  int GetNumXGrids() {return m_iNumXGrids;};

 /**
  * Gets number of Y grid cells.
  * @return Number of grid cells in the Y direction.
  */
 int GetNumYGrids() {return m_iNumYGrids;};

 /**
  * Gets the area of a grid cell.
  * @return The area of a grid cell in square meters.
  */
  float GetGridCellArea() {return m_iCellSize * m_iCellSize;};

 /**
  * Gets the length of a grid cell side (cells are square).
  * @return Grid cell side length, in meters.
  */
  int GetGridCellSize() {return m_iCellSize;};

 /**
  * Translates an X coordinate to a position guaranteed to be valid.
  * If the number passed is already within the plot, it is left alone.  If the
  * number is outside of the plot, it is "wrapped" by adding or subtracting the
  * appropriate plot length until it is inside the plot.  Because of the torus
  * shape of the plot, a number corrected in this way will have the same
  * geometric relationships as the original number.  This function is
  * recursive - it will call itself until the number is correct.  The edge of
  * the plot is actually considered to be m_fMaxX; this catches numbers so close
  * to the edge of the plot that they are likely to be rounded up and then be
  * out of the plot.
  *
  * @param fX Coordinate to correct.
  * @return Corrected version of the coordinate.
  */
  float CorrectX(float fX);

 /**
  * Translates a Y coordinate to a position guaranteed to be valid.
  * If the number passed is already within the plot, it is left alone.  If the
  * number is outside of the plot, it is "wrapped" by adding or subtracting the
  * appropriate plot length until it is inside the plot.  Because of the torus
  * shape of the plot, a number corrected in this way will have the same
  * geometric relationships as the original number.  This function is
  * recursive - it will call itself until the number is correct.  The edge of
  * the plot is actually considered to be m_fMaxY; this catches numbers so close
  * to the edge of the plot that they are likely to be rounded up and then be
  * out of the plot.
  *
  * @param fY Coordinate to correct.
  * @return Corrected version of the coordinate.
  */
  float CorrectY(float fY);

 /**
  * Calculates the effective distance between two points, correcting for the
  * torus.  Note that because of the torus, there are always two possible
  * distances to any two points.  This will return the shorter.
  *
  * @param fFromX X coordinate of the "from" point.
  * @param fFromY Y coordinate of the "from" point.
  * @param fToX X coordinate of the "to" point.
  * @param fToY Y coordinate of the "to" point.
  * @return Distance between the two points in meters.
  * @throw BAD_DATA error if either of the points is not in the plot.
  */
  float GetDistance(float fFromX, float fFromY, float fToX, float fToY);

 /**
  * Calculates the azimuth angle between two points on the plot.
  * @param fFromX X coordinate of the "from" point.
  * @param fFromY Y coordinate of the "from" point.
  * @param fToX X coordinate of the "to" point.
  * @param fToY Y coordinate of the "to" point.
  * @return Azimuth, in RADIANS, north = 0.
  */
  float GetAzimuthAngle(float fFromX, float fFromY, float fToX, float fToY);

 /**
  * Speedy way to do an azimuth calculation.
  * @param fFromX X coordinate of the "from" point.
  * @param fFromY Y coordinate of the "from" point.
  * @param fToX X coordinate of the "to" point.
  * @param fToY Y coordinate of the "to" point.
  * @return Azimuth in number of DEGREES (whole number).
  */
  int GetFastAzimuthAngle(float fFromX, float fFromY, float fToX, float fToY);

 /**
  * Gets the X coordinate of a point defined in polar coordinates relative to
  * another point, uncorrected for torus topology.  If you want a torus-corrected
  * point, use CorrectX() on the result of this function.
  * @param fFromX X coordinate of point that the desired point is defined
  * relative to.
  * @param fAzimuth Azimuth angle from fFromX to the unknown point, in radians,
  * north 0.
  * @param fDistance Distance from fFromX to the unknown point, in meters.
  * @return The uncorrected X value of the point (meaning that it could be
  * negative or greater than the plot length).
  * @throws modelErr if the distance is negative, the azimuth is negative, or the
  * azimuth is greater than 2PI.
  */
  float GetUncorrectedX(float fFromX, float fAzimuth, float fDistance);

 /**
  * Gets the Y coordinate of a point defined in polar coordinates relative to
  * another point, uncorrected for torus topology.  If you want a torus-corrected
  * point, use CorrectY() on the result of this function.
  * @param fFromY Y coordinate of point that the desired point is defined
  * relative to.
  * @param fAzimuth Azimuth angle from fFromX to the unknown point, in radians,
  * north 0.
  * @param fDistance Distance from fFromX to the unknown point, in meters.
  * @return The uncorrected Y value of the point (meaning that it could be
  * negative or greater than the plot length).
  * @throws modelErr if the distance is negative, the azimuth is negative, or the
  * azimuth is greater than 2PI.
  */
  float GetUncorrectedY(float fFromY, float fAzimuth, float fDistance);

 /**
  * Gets the distance between two points in the X plane, corrected for torus
  * wrapping.  If the "to" point is lower than the "from", the answer will be
  * negative.
  *
  * @param fFromX X coordinate of the "from" point.
  * @param fToX X coordinate of the "to" point.
  * @return X distance in meters.
  */
  inline float GetXDistance(float fFromX, float fToX) {
    float fTemp2, fTemp1 = fToX - fFromX;
    if (fTemp1 < 0) fTemp2 = (fToX + m_fPlotLenX) - fFromX;
    else fTemp2 = fToX - (fFromX + m_fPlotLenX);
    if (fabs(fTemp1) < fabs(fTemp2)) return fTemp1;
    else return fTemp2;
  };

 /**
  * Gets the distance between two points in the Y plane, corrected for torus
  * wrapping.  If the "to" point is lower than the "from", the answer will be
  * negative.
  *
  * @param fFromY Y coordinate of the "from" point.
  * @param fToY Y coordinate of the "to" point.
  * @return Y distance in meters.
  */
  inline float GetYDistance(float fFromY, float fToY) {
    float fTemp2, fTemp1 = fToY - fFromY;
    if (fTemp1 < 0) fTemp2 = (fToY + m_fPlotLenY) - fFromY;
    else fTemp2 = fToY - (fFromY + m_fPlotLenY);
    if (fabs(fTemp1) < fabs(fTemp2)) return fTemp1;
    else return fTemp2;
  }


 protected:

 /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

 /**
  * Declares and populates the tangents of the azimuth angles.
  */
  void PopulateAziTans();

  /**Name for this plot. Optional.*/
  std::string m_sPlotTitle;

  /**Number of grid divisions along the X axis.*/
  int m_iNumXGrids;

  /**Number of grid divisions along the Y axis.*/
  int m_iNumYGrids;

  /**Length of one side of a grid cell, in meters. Cells are square. Make this
   * always a power of 2!  The const means the compiler can optimize divisions
   * and multiplications into bit shifts.*/
  int m_iCellSize;

  /**Plot area, in hectares.*/
  float m_fPlotArea;

  /**Plot length along the X axis, in m.*/
  double m_fPlotLenX;

  /**Plot length along the Y axis, in m.*/
  double m_fPlotLenY;

  /**Max allowed X value - m_fPlotLenX minus a small value.*/
  float m_fMaxX;

  /**Max allowed Y value - m_fPlotLenY minus a small value.*/
  float m_fMaxY;

  /**Plot latitude, in decimal degrees - from the par file.*/
  double m_fLatitude;

  /**Mean annual precipitation, mm.*/
  double m_fMeanAnnualPrecipMm;

  /**Long-term mean annual precipitation, mm*/
  double m_fLTMPrecipMm;

  /**Water deficit*/
  double m_fWaterDeficit;

  /**Long-term mean water deficit, mm*/
  double m_fLTMWaterDeficit;

  /** Seasonal precipitation */
  double m_fSeasonalPrecipitation;

  /**Long-term mean seasonal precip, mm*/
  double m_fLTMSeasonalPrecipitation;

  /**Mean annual temperature, degrees Celsius.*/
  double m_fMeanTempC;

  /**Long-term mean annual temperature, C*/
  double m_fLTMTempC;

  /**Annual N deposition.*/
  double m_fNDep;

  /**Tangent of each azimuth angle.  Array size is 360 (degrees).*/
  float *mp_fAziTans;

}  ;
//---------------------------------------------------------------------------
#endif
