//---------------------------------------------------------------------------

#ifndef LightOrgH
#define LightOrgH
//---------------------------------------------------------------------------

#include <xercesc/dom/DOM.hpp>
#include "LightBase.h"

class DOMDocument;

class clTreePopulation;
class clTree;

/**
* Light org - Version 1.0
*
* This class does the organizational work for getting trees their light values
* for a timestep.  It hooks into a light shell object and is triggered by that
* object when it is triggered by the behavior manager.
*
* All light objects require a tree float data member called "Light", which this
* object will register for them.
*
* An object of this class will then call each tree and direct it to the
* appropriate light shell object function for calculation of its light value.
*
* This class has a core job for those light objects that apply to trees.
* However, there are many behaviors which don't calculate tree light levels in
* the traditional sense but calculate light levels for other purposes and are
* descended from the clLightBase class.  This class can still perform useful
* services for them, such as keeping the common light parameters.
*
* A fatal error will be thrown if the number of years per timestep in the
* sim manager is not an integer.  This is less a calculations problem than a
* conceptual one; these light behaviors cannot handle partial growing seasons
* and it would be a shame not to alert someone to that.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clLightOrg {
  public:

  /**
  * Destructor.
  */
  ~clLightOrg();

  /**
  * Constructor.
  * @param p_oHookedShell A clLightBase object (a light shell object) which
  * then becomes the hooked light shell object.
  */
  clLightOrg(clLightBase *p_oHookedShell);

  /**
  * Performs the light calculations.  This will control the calculation of
  * light values and their assignment to individual trees.  This should be
  * called each timestep by the hooked shell's
  * Action() function.
  */
  void DoLightAssignments();

  /**
  * Performs setup functions.  It creates and populates the light functions
  * table.  It can find any light shell behavior as long as the
  * namestring of that object has "lightshell" somewhere in it.  Any behavior
  * which happens to have "lightshell" in its name but is not a light shell
  * behavior will probably cause crashing.
  *
  * DoSetup() is called by the hooked shell's GetData() function.
  *
  * @param p_oSimManager Pointer to the simulation manager.  Since this object
  * is not descended from clWorkerBase, it does not already have its own
  * pointer.
  * @param p_oDoc Pointer to parsed parameter file.
  */
  void DoSetup(clSimManager *p_oSimManager, xercesc::DOMDocument *p_oDoc);

  /**
  * Does the registration of the "Light" tree data member variable.  It goes
  * through the light functions table and, for every species/type combo that
  * has a valid pointer, registers the variable.  Return codes are captured in
  * the mp_iLightCodes array.
  *
  * @param p_oSimManager Pointer to Sim Manager object.
  * @param p_oPop Tree population object.
  */
  void DoTreeDataMemberRegistrations(clSimManager *p_oSimManager,
    clTreePopulation *p_oPop);

  /**Describes the placement of the fisheye photo*/
  enum fotocrowndpth {mid, /**<Middle of the crown (halfway down)*/
                      top  /**<Top of the crown*/
                      };
  /**
  * Gets the light extinction coefficent.  If the tree type is snag, then the
  * snag age is extracted and the coefficient value matching its age is
  * returned.  Otherwise, the value in mp_fLightExtCoef for the appropriate
  * species is returned.
  * @param p_oTree Tree for which to obtain the light extinction coefficient.
  * @return The light extinction coefficient, as the proportion of light
  * transmitted by the tree, as a value between 0 and 1.
  */
  float GetLightExtCoeff(clTree *p_oTree);

  /**
  * Gets the beam fraction of global radiation.
  * @return The beam fraction of global radiation.
  */
  float GetBeamFractionGlobalRadiation() {return m_fBeamFracGlobRad;};

  /**
  * Gets the clear sky transmission coefficient.
  * @return The clear sky transmission coefficient.
  */
  float GetClearSkyTransmissionCoefficient() {return m_fClearSkyTransCoeff;};

  /**
  * Gets the maximum tree height across all species.
  * @return The maximum tree height, in meters.
  */
  float GetMaxTreeHeight() {return m_fMaxTreeHeight;};

  /**
  * Gets the depth of the fisheye photo.
  * @return Depth of the fisheye photo.
  */
  //enum fotocrowndpth GetPhotoDepth() {return m_iPhotoDepth;};

  /**
  * Gets the first day of the growing season.
  * @return First day of the growing season, as a julian day.
  */
  int GetFirstDayOfGrowingSeason() {return m_iFirstJulDay;};

  /**
  * Gets the last day of the growing season.
  * @return Last day of the growing season, as a julian day.
  */
  int GetLastDayOfGrowingSeason() {return m_iLastJulDay;};

  /**
  * Gets a pointer to a light shell object.
  * @param iSp Species for which to get the light shell object.
  * @param iTp Tree type for which to get the light shell object.
  * @return Light shell object for the given species and type, or NULL if it
  * does not exist.
  */
  clLightBase* GetLightShell(short int iSp, short int iTp)
     {return mp_oLightFunctionTable[iSp][iTp];};

  protected:
  clTreePopulation *mp_oPop; /**<Stashed pointer to the tree population object*/

  clLightBase ***mp_oLightFunctionTable; /**<Light shell objects.  Array size is
  number of species by number of types.  For each combo, this points to the
  light shell object which is handling its light calculation.  A pointer may be
  NULL if a combo is not getting any light calculations.*/
  int m_iTotalSpecies; /**<Total number of species*/
  int m_iTotalTypes;   /**<Total number of tree types*/

  //
  //These are the variables common to all light objects
  //

  //Species-specific values - these are in arrays of size = # species and are
  //read in from the parameter file.  These must be provided for all species
  //whether or not they themselves use light.

  double *mp_fLightExtCoef;/**<Light extinction coefficient of live tree crowns.
  One for each species.*/
  double **mp_fSnagLightExtCoef; /**<Light extinction coefficient of snag
  crowns.  First index is sized m_iNumSnagAgeClasses, second is sized
  number of total species.  This is only required from the parameter file if
  the tree population indicates that snags will be made this run.*/
  int *mp_iSnagAgeClasses; /**<Upper limit of age in each snag age class.  This
  is for determining a snag's light extinction coefficient.  This is only
  required from the parameter file if the tree population indicates that
  snags will be made this run.  This array is sized m_iNumSnagAgeClasses,
  but there won't be a value in the last bucket since the value is always
  infinity.*/

  //Single common values - from the parameter file
  double m_fBeamFracGlobRad;    /**<Beam fraction of global radiation.  Old
  parameter kt.*/
  double m_fClearSkyTransCoeff; /**<Clear sky transmission coefficient.  Old
  parameter tran*/
  int m_iFirstJulDay;  /**<First julian day of growing season.  Old parameter jdb*/
  int m_iLastJulDay;   /**<Last julian day of growing season.  Old parameter jde*/
  int m_iNumSnagAgeClasses; /**<Number of age classes for snags, for the
  purpose of dividing up the light extinction coefficients.  The last age class
  upper bound is always infinity.*/

  //Single common values - calculated
  float m_fMaxTreeHeight; /**<Maximum height possible for any tree.  In meters.
  Old parameter maxht.*/

  /**
  Return codes for the "Light" tree float data member variable.  Array size is
  number of species by number of types (even if not every species and type
  requires light)*/
  short int **mp_iLightCodes;

  /**
  * Declares the light functions table and populates it with the appropriate
  * behavior pointers.  It does this by going through the behaviors and looking
  * for the ones with "lightshell" in their names, and then getting the
  * species/type combos those behaviors are supposed to act on.  For each of
  * these combos, their respective place in the table is populated with the
  * behavior's pointer.
  *
  * @param p_oSimManager Sim Manager object.
  * @param p_oPop Tree population object.
  * @throw Error if a species/type combo is claimed by more than one behavior.
  */
  void PopulateLightFunctionsTable(clSimManager *p_oSimManager,
    clTreePopulation *p_oPop);

  /**
  * Reads data from the parameter file.  The first thing to determine is whether
  * there are any light objects in need of the common parameters.  To determine
  * this, this function tests each behavior first to see if it can be casted to
  * type clLightBase.  If it can, its flag "m_bNeedsCommonParameters" is
  * checked.  Parameters are only read if there is a behavior whose flag is set
  * to "true".  Additionally, if the tree population indicates that snags are
  * made, then the light extinction information for them is required.
  *
  * @param p_oSimManager Sim Manager object.
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>The value for m_iPhotoDepth is not a member of fotocrowndepth</li>
  * <li>The snag age classes are not greater than zero, and the second age
  * class is not greater than the first</li>
  * <li>Any of the light extinction coefficients are not between 0 and 1</li>
  * <li>The value of the clear sky transmission coefficient is 0</li>
  * </ul>
  */
  void GetParameterFileData(clSimManager *p_oSimManager, xercesc::DOMDocument *p_oDoc);
};
//---------------------------------------------------------------------------
#endif
