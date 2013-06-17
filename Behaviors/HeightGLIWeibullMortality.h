//---------------------------------------------------------------------------
#ifndef HeightGLIWeibullMortalityH
#define HeightGLIWeibullMortalityH
//---------------------------------------------------------------------------
#include "MortalityBase.h"

/**
* Height-GLI Weibull Mortality - Version 2.0
*
* This evaluates mortality according to a weibull function of tree height
* and GLI.
*
* The equation used in this behavior is:
* <center>p = M<sub>max</sub> * exp(-a * H <sup>b</sup> - c * GLI <sup>d</sup>)</center>
* where
* <ul>
* <li>p = annual probability of mortality</li>
* <li>M<sub>max</sub> = max mortality parameter
* <li>a, b, c, d = parameters</li>
* <li>H = tree height in meters</li>
* <li>GLI, between 0 and 100</li>
* </ul>
*
* If browsing has been implemented (class clRandomBrowse), then this will
* attempt to implement a second set of parameters for when trees are browsed.
* Lack of the browse behavior will cause use of the unbrowsed parameters only.
* A mix of browse applied and unapplied can be used; however, there must be
* parameters for all the same species for browsed, even if they're not all used.
*
* This class's namestring is "height gli weibull mortshell".  The parameter
* file call string is "HeightGLIWeibullMortality".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clHeightGLIWeibullMortality : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clHeightGLIWeibullMortality(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clHeightGLIWeibullMortality();

  /**
  * Calls GetTreeDataMemberCodes(), then ReadParameterFileData().
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates mortality according to the equation above. Then uses a random
  * number to decide if a tree dies.
  *
  * @param p_oTree Tree being evaluated
  * @param fDbh Tree's DBH
  * @param iSpecies Species of the tree being evaluated
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:

  /** Return codes for the "Light" tree float data member variable.  Array
  * size is number of species to which this behavior applies by number of types
  */
  short int **mp_iLightCodes;

  /** Return codes for the "Height" tree float data member variable.  Array
  * size is number of species to which this behavior applies by number of
  * types*/
  short int **mp_iHeightCodes;

  /** Return codes for the "Browsed" tree bool data member variable.  Array
  * size is number of species to which this behavior applies by number of
  * types*/
  short int **mp_iBrowsedCodes;

  /** Max mortality - sized number of behavior species*/
  float *mp_fMaxMort;

  /** Mortality "a" parameter - sized number of behavior species*/
  float *mp_fA;

  /** Mortality "b" parameter - sized number of behavior species*/
  float *mp_fB;

  /** Mortality "c" parameter - sized number of behavior species*/
  float *mp_fC;

  /** Mortality "d" parameter - sized number of behavior species*/
  float *mp_fD;

  /** Browsed max mortality - sized number of behavior species*/
  float *mp_fBrowsedMaxMort;

  /** Browsed "a" parameter - sized number of behavior species*/
  float *mp_fBrowsedA;

  /** Browsed "b" parameter - sized number of behavior species*/
  float *mp_fBrowsedB;

  /** Browsed "c" parameter - sized number of behavior species*/
  float *mp_fBrowsedC;

  /** Browsed "d" parameter - sized number of behavior species*/
  float *mp_fBrowsedD;

  /** To help access the other arrays*/
  int *mp_iIndexes;

  /** Number of years per timestep - for converting probabilities for multi-
  * year timesteps */
  float m_fYearsPerTimestep;

  /**
  * Queries for the return codes of the "Light", "Browsed", and "Height" tree
  * data members. Return codes are captured in the mp_iLightCodes,
  * mp_iBrowsedCodes, and mp_iHeightCodes arrays.
  * @throws modelErr if any data member codes are missing for light or height.
  * Missing browse is not an error.
  */
  void GetTreeDataMemberCodes();

  /**
  * Reads the parameter file data. Unbrowsed parameters are always read.
  * Browsed parameters are read only if there is a non -1 value somewhere in
  * mp_iBrowsedCodes. However, if there is, then the same set of species had
  * better be present for the browsed parameters as for the unbrowsed.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if the max mortality parameter is not between 0 and 1.
  */
  void ReadParameterFileData(xercesc::DOMDocument *p_oDoc);
};
//---------------------------------------------------------------------------
#endif
