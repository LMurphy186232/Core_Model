//---------------------------------------------------------------------------

#ifndef SailLightH
#define SailLightH
//---------------------------------------------------------------------------

#include "LightBase.h"
#include "LightOrg.h"

/**
* Sail Light - Version 1.0
*
* This calculates the percent shade for a tree using the sail light method.
* The namestring for this behavior is "saillightshell". The parameter file call
* string is "SailLight".
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clSailLight : public clLightBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clSailLight(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clSailLight() {delete[] mp_fAltTans;};

  /**
  * Reads some extra parameters from the parameter file and does setup.  Number
  * of azimuth angles and number of altitude angles are required to be present
  * in the "sailLight" tag.  The sky brightness array is populated for sail
  * light.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates a fraction shade value.
  *
  * @param p_oTree Tree for which to calculate the fraction shade.
  * @param p_oPop Tree population object.
  * @return Fraction of shade between 0 and 1.
  */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop);

  protected:

  double m_fMaxShadingRadius; /**<Maximum radius at which trees can shade, in meters*/
  double *mp_fAltTans; /**<Tangent of each altitude angle.  Array size is 90 (degrees)*/

  /**The fraction of shading neighbor height which is assumed to be crown*/
  enum crowndpthequalsheight {yes, /**<The crown extends the full length of the trunk*/
                             no /**<The crown has a bottom according to normal
                             allometry equations*/
   } m_iCrownDepthEqualsHeight;  /**<Crown depth from parameter file*/
  clLightOrg::fotocrowndpth m_iPhotoDepth; /**<Depth of fisheye photo in tree.
  Defaults to top of crown, and thus is not required in the parameter file. Old
  parameter fotodepth*/

  /**
  * Adds one shading neighbor to the fisheye photo array for a sail light
  * calculation.
  *
  * @param fTargetX X coordinate of target tree
  * @param fTargetY Y coordinate of target tree
  * @param fTargetHeight Height of target tree, in meters
  * @param p_oNeighbor The neighbor to be added
  * @param p_oPlot Pointer to the plot object
  * @param p_oPop Pointer to the tree population object
  * @param p_oAllom Pointer to an allometry object
  */
  void AddTreeToSailFishEye(const float &fTargetX, const float &fTargetY, const
    float &fTargetHeight, clTree *p_oNeighbor, clPlot *p_oPlot,
    clTreePopulation *p_oPop, clAllometry *p_oAllom);
};
//---------------------------------------------------------------------------
#endif
