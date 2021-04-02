//---------------------------------------------------------------------------

#ifndef GLILightH
#define GLILightH
//---------------------------------------------------------------------------
#include "GLIBase.h"
#include "LightOrg.h"


class clTreePopulation;

/**
 * GLI Light - Version 1.0
 *
 * This calculates a GLI (global light index) for a tree.  The namestring for
 * this behavior is "glilightshell". The parameter file call string is
 * "GLILight".
 *
 * The sky brightness array used by this behavior is potentially identical to
 * that for quadrat-based GLI light - class name clQuadratGliLight, namestring
 * "quadratglilightshell".  Before committing to the calculation of a brightness
 * array, this behavior will ask that behavior if it's already done it and this
 * behavior can just copy.
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/

class clGliLight : virtual public clGLIBase {
//note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
   * Constructor.  The constructor will set the namestring.
   * @param p_oSimManager Sim manager.
   */
  clGliLight(clSimManager *p_oSimManager);

  //~clGliLight(); //use default destructor

  /**
   * Reads some extra parameters from the parameter file.  Number of azimuth
   * angles and number of altitude angles are required to be present in the
   * "glilight" tag.
   * @param p_oDoc DOM tree of parsed parameter file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Calculates a GLI value.
   * @param p_oTree Tree for which to calculate GLI.
   * @param p_oPop Pointer to the tree population object.
   * @return GLI, as a percentage of full sun between 0 and 100.
   */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop);

  clLightOrg::fotocrowndpth m_iPhotoDepth; /**<Depth of fisheye photo in tree.
  Defaults to top of crown, and thus is not required in the parameter file. Old
  parameter fotodepth*/

};
//---------------------------------------------------------------------------
#endif

