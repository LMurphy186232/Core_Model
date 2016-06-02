//---------------------------------------------------------------------------

#ifndef LightBaseH
#define LightBaseH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"
#include "Constants.h"
#include <math.h>


class clTree;
class clTreePopulation;
class clLightOrg;
class clPlot;
class clAllometry;
class clQuadratGLILight;

/**
* Light base - Version 1.1
*
* This is the base class for light behavior shell classes. These shells
* implement a common set of functions in different ways which are specific
* to the way light is calculated according to their method.
*
* The bulk of the organizational work is done by the class clLightOrg, which
* calls the needed shell functions for each tree. The clLightOrg object hooks
* into one particular light shell object, which calls clLightOrg when it is
* triggered by the behavior manager. It does not particularly matter which
* shell object is hooked; the first one will suffice. Any non-hooked shells
* do nothing when their Action() and other functions are called. The hooked
* shell also tells this object that it's time to register tree data members.
*
* Every object made from this class or a descendent class must have the string
* "lightshell" in its namestring somewhere. The base class also declares a
* new tree float data member on behalf of each child shell object; the light
* org object then registers it. The descendents don't need to do anything.
* The new data member is called "Light", and will store the amount of light
* calculated.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
* <tr>August 19, 2015 - Added sky rotation (LEM)
*/
class clLightBase : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  friend class clLightOrg;
  friend class clGliLight;
  friend class clQuadratGLILight;
  friend class clGLIMap;
  friend class clGLIPoints;

  public:
  /**
  * Constructor. Checks to see if the light org object has been created -
  * if not, it creates it. Sets m_bNeedsCommonParameters to true, its default.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clLightBase(clSimManager *p_oSimManager);

  /**
  * Destructor. Deletes the light org object if it was the hooked object.
  */
  virtual ~clLightBase();

  /**
  * Performs all light calculations. This will be the same for all descendent
  * classes - they do not need to override. If a particular object is hooked,
  * it calls the light org object's DoLightAssignments. Otherwise it does
  * nothing.
  */
  void Action();

  /**
  * Performs data member registrations for "Light".
  * This will be the same for all descendent classes - they do not need to
  * override. If a particular object is hooked, it calls the light org object's
  * DoTreeDataMemberRegistrations(). Otherwise it does nothing.
  */
  void RegisterTreeDataMembers();

  /**
  * Calculates the light value for a particular tree. This must be overridden
  * by all child classes.
  *
  * The reason that the function does not just assign the value to the tree is
  * because the data codes are managed by the light org object.
  *
  * @param p_oTree Tree for which to calculate light level.
  * @param p_oPop Tree population object.
  * @return Light level, in whatever format is appropriate to the light object
  * that calculated it.
  */
  virtual float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop) = 0;

  /**
  * Gets the light org object.
  *
  * @return The light org object.
  */
  clLightOrg* GetLightOrg() {return mp_oLightOrg;};

  /**
   * Gets the altitude angle below which the sky is assumed to be dark.
   * @return Minimum sun angle.
   */
  float GetMinSunAngle() {return m_fMinSunAngle;};

  /**
   * Gets the number of azimuth angles into which the sky hemisphere is divided.
   * @return Number of azimuth angles into which the sky hemisphere is divided.
   */
  float GetNumAziAng() {return m_iNumAziAng;};

  /**
   * Gets the number of altitude angles into which the sky hemisphere is divided.
   * @return Number of altitude angles into which the sky hemisphere is divided.
   */
  float GetNumAltAng() {return m_iNumAltAng;};

  /**
   * Gets the row in the brightness array corresponding to the minimum solar
   * angle.
   * @return Row in the brightness array corresponding to the minimum solar
   * angle.
   */
  float GetMinAngRow() {return m_iMinAngRow;};

  float GetBrightness(int alt, int azi) {return mp_fBrightness[alt][azi];};

  protected:

  static clLightOrg *mp_oLightOrg; /**<clLightOrg object - this pointer is held
             in common by all shells*/

  bool m_bHooked; /**<Whether or not this shell object is hooked to clLightOrg.
             clLightOrg will set this flag.*/
  bool m_bNeedsCommonParameters; /**<Whether or not this shell object requires
  the common light parameters held in clLightOrg such as
  clLightOrg::m_iLastJulDay. mp_oLightOrg will check this flag to see whether
  it should fill these values.*/

  //Variables that all light objects need but will keep their own copies of

  /**Sky brightness array. Array size is # altitude angles by # azimuth angles.
   * The altitude index increases from horizon to zenith, and the azimuth index
   * increases from north clockwise. Each child object needs its own copy of this.*/
  float **mp_fBrightness;

  /**Simulated fisheye photo array. Array size is # altitude angles by # azimuth
   * angles. The altitude index increases from horizon to zenith, and the azimuth
   * index increases from 0 to 2PI. Each child object needs its own copy of this.*/
  float **mp_fPhoto;

  /**The altitude angle below which the sky is assumed to be dark. Could be in
   * degrees or radians, depending on what the child object is working in. Each
   * child object needs its own copy of this.*/
  double m_fMinSunAngle;

  /**The azimuth angle of north. Not required; will default to 0. Setting this
   * to non-zero allows the user to rotate the sky relative to the plot.*/
  double m_fAzimuthOfNorth;

  /**Number of azimuth angles into which the sky hemisphere is divided. Each
   * child object needs its own copy of this.*/
  int m_iNumAziAng;

  /**Number of altitude angles into which the sky hemisphere is divided. Each
   * child object needs its own copy of this.*/
  int m_iNumAltAng;

  /**Row in the brightness array corresponding to the minimum solar angle. Used
   * to compare calculated object positions. Each child object needs its own
   * copy of this.*/
  int m_iMinAngRow;

  /**
  * Triggers all light setup. This will be the same for all descendent classes.
  * If a particular object is hooked, it calls the light org object's DoSetup()
  * function. Then it calls the function DoShellSetup() - if a descendent class
  * has specific setup needs, it can overload that function.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * If a descendent class has specific setup needs, it can overload this
  * function.
  * @param p_oDoc DOM tree of parsed input file.
  */
  virtual void DoShellSetup(xercesc::DOMDocument *p_oDoc){;};

  /**
  * Populates the GLI brightness array. Yes, yes, this isn't strictly a
  * function common to all light shells, but it's  common to more than one.
  * Plus it's really similar to the sail light brightness array calculator and
  * I may merge
  * them at some point.
  *
  * This assumes that the brightness array has already been allocated.
  */
  void PopulateGLIBrightnessArray();

  /**
  * Populates the sail brightness array.
  * This is so similar to the GLI brightness array function that I wanted them
  * together so I could consider merging them later. But right now it's too
  * much, what with one doing all calcs in radians and one in degrees. If that
  * seems trivial to you, you're welcome to do it yourself.
  *
  * This assumes that the brightness array has already been allocated.
  */
  void PopulateSailLightBrightnessArray();



 //Solar geometry equations - inline

  /**
  * Computes day angle.
  *
  * @param iJulianDay Julian day for which to compute a day angle
  * @return Day angle, in radians.
  */
 inline float GetDayAngle(int iJulianDay)
                   {return (2*M_PI*(iJulianDay-1))/365;};

  /**
  * Computes solar declination.
  *
  * @param fDayAngle Day angle, in radians, for which to compute declination.
  * @return Solar declination, in radians.
  */
 inline float GetDeclination(float &fDayAngle) {
      return 0.006918 - (0.399912*cos(fDayAngle))+
      (0.070257*sin(fDayAngle))-(0.006758*cos(2*fDayAngle))+
      (0.000907*sin(2*fDayAngle))-(0.002697*cos(3*fDayAngle))+
      (0.00148*sin(3*fDayAngle));
 };

  /**
  * Computes solar eccentricity.
  *
  * @param fDayAngle Day angle, in radians, for which to compute eccentricity.
  * @return Solar eccentricity, in radians.
  */
 inline float GetEccentricity(float &fDayAngle) {
      return 1.000110+(0.034221*cos(fDayAngle))+(0.001280*
      sin(fDayAngle))-(0.000719*cos(2*fDayAngle))+(0.000077*sin(2*fDayAngle));
 };

  /**
  * Computes sunrise.
  *
  * @param fLatInRadians Latitude, in radians
  * @param fDeclination Solar declination on a given day, in radians
  * @return Sunrise, in solar time
  */
 inline float GetSunrise(float &fLatInRadians, float &fDeclination) {
      //compute sunrise and sunset times in radians from noon
      //x = cos(hour angle) when sun is at the horizon
      float x = (-1.0*sin(fLatInRadians)*sin(fDeclination))/(cos(fLatInRadians)*
                                                            cos(fDeclination));
      //but I don't understand how you get to sunrise - LEM
      return ((M_PI/2) - atan(x/(sqrt(1-x*x))));
 };

  /**
  * Computes cosine of the zenith angle of the sun at a given time. This is
  * needed in other calculations.
  *
  * @param fDeclination Solar declination, in radians
  * @param fLatInRadians Location latitude in radians
  * @param fTimeNow Current time in solar time
  * @return Cosine of zenith angle of the sun
  */
 inline float GetCosineOfZenithAngle(float &fDeclination, float &fLatInRadians,
                             float &fTimeNow) {
   return (sin(fDeclination)*sin(fLatInRadians))+
                (cos(fDeclination)*cos(fLatInRadians)*cos(fTimeNow));
 };

  /**
  * Computes altitude angle of the sun at a given time in radians
  *
  * @param fCosZenAng Cosine of the sun's zenith angle
  */
 inline float GetAltitudeAngle(float &fCosZenAng) {
      //fCosZenAng is cosine of zenith angle - acos it to get the angle, and
      //subtract from pi/2 to get the altitude angle
      if (fCosZenAng <= -1.0)
         return ((M_PI/2.0) - acos(-1.0));
      else if (fCosZenAng >= 1.0)
        return ((M_PI/2.0) - acos(1.0));
      else return ((M_PI/2.0) - acos(fCosZenAng));
 };

  /**
  * Computes azimuth angle of the sun at a given time in radians in SORTIE
  * azimuth coordinates
  *
  * @param fDeclination Solar declination, in radians
  * @param fLatInRadians Location latitude in radians
  * @param fAltInRad Sun's current altitude position, in radians
  * @param fTimeNow Current solar time
  * @return Azimuth angle of sun
  */
 inline float GetAzimuthAngle(float &fDeclination, float &fLatInRadians,
                              float &fAltInRad, float &fTimeNow) {
        float fCosAzi = (((sin(fAltInRad)*sin(fLatInRadians))-sin(fDeclination))/
                     (cos(fLatInRadians)*cos(fAltInRad)));
        if (fCosAzi <= -1.0)   //just error checking. It is possible above line
           fCosAzi = -1.0;         //results in domain error
        else if (fCosAzi >= 1.0)
          fCosAzi = 1.0;
        fCosAzi = acos(fCosAzi);
        //convert to our azimuth system
        if (fCosAzi <= M_PI)
          return (M_PI - fCosAzi);
        else
          return ((3*M_PI) - fCosAzi);
 };

  /**
  * Computes the airmass effect for a given altitude angle.
  *
  * @param fAltInDeg Altitude angle, in degrees
  * @param fCosZenAng Cosine of solar zenith angle
  * @return Airmass effect
  */
 inline float GetAirmassEffect(float &fAltInDeg, float &fCosZenAng) {
        if (fAltInDeg <= 5) return 10.39;
        else if ((fAltInDeg > 5) && (fAltInDeg <= 15)) return 5.60;
        else if ((fAltInDeg >15) && (fAltInDeg <= 25)) return 2.90;
        else return (1/fCosZenAng);
 };

  /**
  * Computes beam radiation strength.
  *
  * @param fClearSkyTransCoeff Clear sky transmission coefficient
  * @param fAirmass Airmass effect
  * @param fEccentricity Solar eccentricity
  * @param fCosZenAng Cosine of solar zenith angle
  */
 inline float GetBeamRadiation(float &fClearSkyTransCoeff, float &fAirmass,
                               float &fEccentricity, float &fCosZenAng) {
        return (pow(fClearSkyTransCoeff,fAirmass)*fEccentricity*fCosZenAng);
 };

 /**
  * Gets the light extinction coefficent. This passes this through to
  * clLightOrg::GetLightExtCoeff(). This structure allows for overriding
  * by child classes if necessary.
  * @param p_oTree Tree for which to obtain the light extinction coefficient.
  * @return The light extinction coefficient, as the proportion of light
  * transmitted by the tree, as a value between 0 and 1.
  */
  virtual float GetLightExtinctionCoefficient(clTree *p_oTree);
};
//---------------------------------------------------------------------------
#endif
