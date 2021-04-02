//---------------------------------------------------------------------------
// GLIPoints
//---------------------------------------------------------------------------
#if !defined(GLIPoints_H)
#define GLIPoints_H

#include "GLIBase.h"


class clTreePopulation;

/**
* GLI Points File Creator - Version 1.0
*
* This behavior creates a GLI value for each of a list of user-defined points.
* The user includes in the parameter file a points file with a set of X and Y
* coordinates and a height.  For each timestep, this will calculate the GLI
* for each of those points and then write the results to a tab-delimited text
* file.  This behavior does not need to be applied to any trees.  Nothing else
* is done with the GLI values.  This is an analysis behavior that prepares
* some calculations; the user can use the values however they see fit.
*
* The namestring and parameter file call string for this behavior is
* "GLIPointCreator". This class is descended from clGLIBase so it can use its
* light calculation methods, but since the string "lightshell" is ommitted from
* this class's namestring, it should not be treated as a light behavior.
*
* The sky brightness array used by this behavior is potentially identical to
* that for other GLI light behaviors.  Before committing to the calculation of
* a brightness array, this behavior will ask those behaviors if their settings
* are identical.  If they are, and that class has already calculated the
* brightness array, this behavior can just copy.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clGLIPoints : public clGLIBase {

public:

  /**
  * Constructor.  Sets the namestring.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clGLIPoints(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clGLIPoints();

  /**
  * Does setup for this object.  Calls the following functions:
  * <ol>
  * <li>clLightBase::GetData(), to make sure mp_oLightOrg gets set up if this
  * is the "hooked" light object</li>
  * <li>ReadParameterFileData</li>
  * <li>SetUpBrightnessArray</li>
  * <li>DoSetupCalculations</li>
  * </ol>
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates the GLI values.  For each point, this calculates its GLI and
  * then writes the result to the output file.
  */
  void Action();

  /**
  * Required overridden function - doesn't do anything.
  * @param p_oTree Ignored.
  * @param p_oPop Ignored.
  * @return 0.
  */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop) {return 0;};

protected:

  /**
  * Structure for holding GLI points
  */
  struct stcCoords {
    /**X coordinate*/
    float fX;
    /**Y coordinate*/
    float fY;
    /**Height in m*/
    float fHeight;
  } *mp_pointsList; /**<List of points for which to calculate
  GLI*/

  int m_iNumPoints; /**<How many points are in mp_pointsList*/

  /**Points output file file name.  If it already exists, the file is
  * overwritten*/
  std::string m_sFileName;

  /**
  * Reads in needed parameter file data.  First this looks for the "gliPoints"
  * tag and tries to read in all its data from that (in case the user has
  * provided different sky grid settings to different light objects).  If
  * there's no sky grid data in the "gliPoints" tag, then this will get what it
  * needs from the first tag it finds with these values. (The "gliPoints" tag,
  * no matter what, must have the GLI points list and the filename of the
  * output file.)
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>The tag "gliPoints" or its required data (points list, filename) is
  * not present</li>
  * <li>Number of azimuth sky divisions is less than or equal to 0</li>
  * <li>Number of altitude sky divisions is less than or equal to 0</li>
  * <li>Any point has an X or Y coordinate outside the plot</li>
  * <li>Any point has a light height that is negative</li>
  * </ul>
  */
  void ReadParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Sets up the light brightness array.  This will check to see if it can share
  * the brightness array with clGliLight or clQuadratGliLight, and if not, it
  * creates the sky brightness array.
  */
  void SetUpBrightnessArray();

  /**
  * Performs setup calculations.  This calculates the values for
  * m_fAziChunkConverter, m_fRcpTanMinAng, and mp_fAziSlope.
  */
  void DoSetupCalculations();

};
//---------------------------------------------------------------------------
#endif // GLIPoints_H
