//---------------------------------------------------------------------------

#ifndef AllometryH
#define AllometryH

#include <math.h>
#include "TreePopulation.h"

/**
 * Allometry Class - Version 2.4
 * This handles allometric calculations for trees.  None of these functions
 * check for over or underflow.
 *
 * Copyright 2005 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>April 28, 2004 - Submitted as beta (LEM)
 * <br>April 25, 2005 - Updated to allow user-selected allometric equations
 * and updated version to 2.0 (LEM)
 * <br>July 8, 2005 - Added Chapman-Richards function for crown shape;
 * standardized on the name "crown" instead of "canopy"; and upgraded version
 * to 2.1 (LEM)
 * <br>January 25, 2006 - Added power function for saplings and upgraded version
 * to 2.2 (LEM)
 * <br>February 21, 2008 - Added non-spatial density dependent crown width and
 * crown depth relationships and upgraded version to 2.3 (LEM)
 * <br>April 9, 2009 - Added NCI  crown width and crown depth relationships;
 * caused the crown shape functions to take a tree as an argument; and upgraded
 * version to 2.4 (LEM)
 */
class clAllometry {

public:
  /**Functions for height-diam relationships.*/
  enum iHeightDiamFunction {
    standard, /**<Old standard function*/
    linear, /**<Height is a linear function of DBH*/
    reverse_linear, /**<DBH is a linear function of height*/
    power /**<Height is a power function of diam10*/
  };

  /**Functions for crown radius-diam relationships.*/
  enum iCrownRadDiamFunction {
    standard_cr, /**<Old standard function*/
    chapman_richards_cr, /**<Chapman-Richards*/
    non_spat_exp_dens_dep_cr, /**<Non-spatial exponential density-dependent*/
    nci_cr /**<NCI*/
  };

  /**Functions for crown depth-diam relationships.*/
  enum iCrownDepthDiamFunction {
    standard_cd, /**<Old standard function*/
    chapman_richards_cd, /**<Chapman-Richards*/
    non_spat_log_dens_dep_cd, /**<Non-spatial logistic density-dependent*/
    nci_cd /**<NCI*/
  };

  /**
   * Constructor. NULLs pointers.
   * @param p_oPop Parent tree population object.
   */
  clAllometry(clTreePopulation *p_oPop);

  /**
   * Destructor.  Frees memory.
   */
  ~clAllometry();

  /**
   * Reads data from the parameter file.
   * @param p_oDoc Parsed parameter file document.
   * @param p_oPop Tree population object.
   */
  void GetData(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

  /**
   * Gets the maximum tree height.
   *
   * @param iSpecies Species for which to get the maximum tree height.
   * @return Max tree height.
   * @throw Error if the species isn't valid.
   */
  float GetMaxTreeHeight(int iSpecies);

  /**
   * Calculates height according to the adult DBH-height relationship for the
   * given species.
   * @param fDbh DBH in cm for which to calculate tree height.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Height in meters.
   * @throw ModelErr If the species isn't recognized.
   */
  float CalcAdultHeight(const float &fDbh, const int &iSpecies);

  /**
   * Calculates DBH according to the adult DBH-height relationship for the
   * given species.
   * @param fHeight Height in meters.
   * @param iSpecies Species of tree for which to calculate DBH.
   * @return DBH in cm.
   * @throw ModelErr If the species isn't recognized or if the height is greater
   * than the maximum height for the species.
   */
  float CalcAdultDbh(float fHeight, const int &iSpecies);

  /**
   * Calculates crown radius according to the adult crown radius-DBH equation
   * for the species. This will check to see if it has already been calculated
   * and placed in the "Crown Radius" tree float data member.
   * @param p_oTree Tree for which to calculate crown radius.
   */
  float CalcAdultCrownRadius(clTree *p_oTree);

  /**
   * Calculates adult crown depth according to the adult allometry equation for
   * the species. This will check to see if it has already been calculated
   * and placed in the "Crown Depth" tree float data member.
   * @param p_oTree Tree for which to calculate crown depth.
   */
  float CalcAdultCrownDepth(clTree *p_oTree);

  /**
   * Calculates height according to the sapling DBH-height relationship for the
   * given species.
   * @param fDbh DBH in cm for which to calculate tree height.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Height in meters.
   * @throw ModelErr If the species isn't recognized.
   */
  float CalcSaplingHeight(const float &fDbh, const int &iSpecies);

  /**
   * Calculates DBH according to the sapling DBH-height relationship for the
   * given species.
   * @param fHeight Height in meters.
   * @param iSpecies Species of tree for which to calculate height.
   * @return DBH in cm.
   * @throw ModelErr If the species isn't recognized or if the height is greater
   * than the maximum height for the species.
   */
  float CalcSaplingDbh(float fHeight, const int &iSpecies);

   /**
   * Calculates crown radius according to the sapling crown radius-DBH equation
   * for the species. This will check to see if it has already been calculated
   * and placed in the "Crown Radius" tree float data member.
   * @param p_oTree Tree for which to calculate crown radius.
   */
  float CalcSaplingCrownRadius(clTree *p_oTree);

  /**
   * Calculates crown depth according to the sapling allometry equation for
   * the species. This will check to see if it has already been calculated
   * and placed in the "Crown Depth" tree float data member.
   * @param p_oTree Tree for which to calculate crown depth.
   */
  float CalcSaplingCrownDepth(clTree *p_oTree);

  /**
   * Calculates height according to the seedling diam<sub>10</sub>-height
   * relationship for the given species.
   * @param fDiam10 Diameter at 10 cm in cm for which to calculate tree height.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Height in meters.
   * @throw ModelErr If the species isn't recognized.
   */
  float CalcSeedlingHeight(float fDiam10, const int &iSpecies);

  /**
   * Calculates diameter at 10 cm according to the seedling
   * diam<sub>10</sub>-height relationship for the given species.
   * @param fHeight Height in meters.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Diameter at 10 cm in cm.
   * @throw ModelErr If the species isn't recognized or if the height is greater
   * than the maximum height for the species.
   */
  float CalcSeedlingDiam10(float fHeight, const int &iSpecies);

  /**
   * Converts a diameter at 10 cm value to a DBH.  The equation is:
   * DBH = diam<sub>10</sub> * R where DBH is the DBH in cm, diam<sub>10</sub> is
   * the diameter at 10 cm height in cm, and R is the DBH to diam<sub>10</sub>
   * ratio.
   * @param fDiam10 Diameter at 10 cm in cm.
   * @param iSpecies Species of tree.
   * @return DBH value in cm.
   * @throw ModelErr If the species isn't recognized.
   */
  float ConvertDiam10ToDbh(float fDiam10, const int &iSpecies);

  /**
   * Converts a DBH value to a diameter at 10 cm value.  The equation is:
   * diam<sub>10</sub> = DBH / R where DBH is the DBH in cm, diam<sub>10</sub> is
   * the diameter at 10 cm height in cm, and R is the DBH to diam<sub>10</sub>
   * ratio.
   * @param fDbh DBH in cm.
   * @param iSpecies Species of tree.
   * @return Diameter at 10 cm value in cm.
   * @throw ModelErr If the species isn't recognized.
   */
  float ConvertDbhToDiam10(const float &fDbh, const int &iSpecies);

  /**
   * Gets the maximum possible crown radius.
   * @return Maximum possible crown radius, in m.
   */
  float GetMaxCrownRadius() {return m_fMaxCrownRad;};

  /**
   * Cleanup function that resets the plot density and basal area information.
   */
  void DoDataUpdates();

protected:

  clTreePopulation *m_oPop; /**<Tree population object*/

  /**Typedef to all allometry calculating functions except crown functions*/
  typedef float (clAllometry::*Ptr2Allometry)(const float&, const int&);

  /**Typedef to crown dimension calculating functions*/
  typedef float (clAllometry::*Ptr2CrownDimension)(clTree*);

  /**Function pointer array for the appropriate adult height-calculating
   * function.  Array size is number of species.*/
  Ptr2Allometry* mp_AdultHeight;

  /**Function pointer array for the appropriate adult DBH-calculating
   * function.  Array size is number of species.*/
  Ptr2Allometry* mp_AdultDiam;

  /**Function pointer array for the appropriate adult crown radius-calculating
   * function.  Array size is number of species.*/
  Ptr2CrownDimension* mp_AdultCrownRad;

  /**Function pointer array for the appropriate adult crown depth-calculating
   * function.  Array size is number of species.*/
  Ptr2CrownDimension* mp_AdultCrownDepth;

  /**Function pointer array for the appropriate sapling height-calculating
   * function.  Array size is number of species.*/
  Ptr2Allometry* mp_SaplingHeight;

  /**Function pointer array for the appropriate sapling DBH-calculating
   * function.  Array size is number of species.*/
  Ptr2Allometry* mp_SaplingDiam;

  /**Function pointer array for the appropriate sapling crown radius-calculating
   * function.  Array size is number of species.*/
  Ptr2CrownDimension* mp_SaplingCrownRad;

  /**Function pointer array for the appropriate sapling crown depth-calculating
   * function.  Array size is number of species.*/
  Ptr2CrownDimension* mp_SaplingCrownDepth;

  /**Function pointer array for the appropriate seedling height-calculating
   * function.  Array size is number of species.*/
  Ptr2Allometry* mp_SeedlingHeight;

  /**Function pointer array for the appropriate seedling diam10-calculating
   * function.  Array size is number of species.*/
  Ptr2Allometry* mp_SeedlingDiam;

  /**Tree maximum height for each species.  Array size is total # species.
   * Values are required for all species.*/
  float *mp_fMaxTreeHeight;

  /**Asymptotic crown radius.  Array size is total # species.*/
  float *mp_fAsympCrownRad;

  /**Crown radius exponent.  Array size is total # species.*/
  float *mp_fCrownRadExp;

  /**Slope of DBH to diameter at 10 cm.  For converting from one to the other.
   * Array size is total # species.*/
  float *mp_fDbhToDiam10Slope;

  /**Intercept of DBH to diameter at 10 cm.  For converting from one to the
   * other. Array size is total # species.*/
  float *mp_fDbhToDiam10Intercept;

  /**Asymptotic crown depth.  Array size is total # species.*/
  float *mp_fAsympCrownDepth;

  /**Crown depth exponent.  Array size is total # species.*/
  float *mp_fCrownDepthExp;

  /**Slope of the height-diameter at 10 cm conversion relationship.  Array size
   * is total # species.*/
  float *mp_fSlopeHeightDiam10;

  /**Slope of asymptotic height.  Array size is total # species.*/
  float *mp_fSlopeAsympHeight;

  /**Slope of adult linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fAdultLinearSlope;

  /**Intercept of adult linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fAdultLinearIntercept;

  /**Slope of adult reverse linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fAdultReverseLinearSlope;

  /**Intercept of adult reverse linear height-dbh equation.  Array size is
   * total # species.*/
  float *mp_fAdultReverseLinearIntercept;

  /**Slope of sapling linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fSaplingLinearSlope;

  /**Intercept of sapling linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fSaplingLinearIntercept;

  /**Slope of sapling reverse linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fSaplingReverseLinearSlope;

  /**Intercept of sapling reverse linear height-dbh equation.  Array size is
   * total # species.*/
  float *mp_fSaplingReverseLinearIntercept;

  /**Slope of seedling linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fSeedlingLinearSlope;

  /**Intercept of seedling linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fSeedlingLinearIntercept;

  /**Slope of seedling reverse linear height-dbh equation.  Array size is total #
   * species.*/
  float *mp_fSeedlingReverseLinearSlope;

  /**Intercept of seedling reverse linear height-dbh equation.  Array size is
   * total # species.*/
  float *mp_fSeedlingReverseLinearIntercept;

  /**Crown radius intercept for the Chapman-Richards equation.  Array size is
   * total # species.*/
  float *mp_fCRCrownRadIntercept;

  /**Asymptotic crown radius for the Chapman-Richards equation.  Array size is
   * total # species.*/
  float *mp_fCRAsympCrownRad;

  /**Chapman-Richards crown radius equation shape parameter 1.  Array size is
   * total # species.*/
  float *mp_fCRCrownRadShape1;

  /**Chapman-Richards crown radius equation shape parameter 2.  Array size is
   * total # species.*/
  float *mp_fCRCrownRadShape2;

  /**Crown depth intercept for the Chapman-Richards equation.  Array size is
   * total # species.*/
  float *mp_fCRCrownHtIntercept;

  /**Asymptotic crown depth for the Chapman-Richards equation.  Array size is
   * total # species.*/
  float *mp_fCRAsympCrownHt;

  /**Chapman-Richards crown depth equation shape parameter 1.  Array size is
   * total # species.*/
  float *mp_fCRCrownHtShape1;

  /**Chapman-Richards crown depth equation shape parameter 2.  Array size is
   * total # species.*/
  float *mp_fCRCrownHtShape2;

  /**Power function a parameter.  Array size is total # species.*/
  float *mp_fPowerA;

  /**Power function b parameter (exponent).  Array size is total # species.*/
  float *mp_fPowerExpB;

  /**Non-spatial density dependent instrumental crown depth "a". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHA;

  /**Non-spatial density dependent instrumental crown depth "b". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHB;

  /**Non-spatial density dependent instrumental crown depth "c". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHC;

  /**Non-spatial density dependent instrumental crown depth "d". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHD;

  /**Non-spatial density dependent instrumental crown depth "e". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHE;

  /**Non-spatial density dependent instrumental crown depth "f". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHF;

  /**Non-spatial density dependent instrumental crown depth "g". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHG;

  /**Non-spatial density dependent instrumental crown depth "h". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHH;

  /**Non-spatial density dependent instrumental crown depth "i". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHI;

  /**Non-spatial density dependent instrumental crown depth "j". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCHJ;

  /**Non-spatial exponential density dependent crown radius "D1". Array size is
   * total # species.*/
  float *mp_fNonSpatExpDensDepCRD1;

  /**Non-spatial exponential density dependent crown radius "a". Array size is
   * total # species.*/
  float *mp_fNonSpatExpDensDepCRA;

  /**Non-spatial exponential density dependent crown radius "b". Array size is
   * total # species.*/
  float *mp_fNonSpatExpDensDepCRB;

  /**Non-spatial exponential density dependent crown radius "c". Array size is
   * total # species.*/
  float *mp_fNonSpatExpDensDepCRC;

  /**Non-spatial exponential density dependent crown radius "d". Array size is
   * total # species.*/
  float *mp_fNonSpatExpDensDepCRD;

  /**Non-spatial exponential density dependent crown radius "e". Array size is
   * total # species.*/
  float *mp_fNonSpatExpDensDepCRE;

  /**Non-spatial exponential density dependent crown radius "f". Array size is
   * total # species.*/
  float *mp_fNonSpatExpDensDepCRF;

  /**Non-spatial density dependent instrumental crown radius "a". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRA;

  /**Non-spatial density dependent instrumental crown radius "b". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRB;

  /**Non-spatial density dependent instrumental crown radius "c". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRC;

  /**Non-spatial density dependent instrumental crown radius "d". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRD;

  /**Non-spatial density dependent instrumental crown radius "e". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRE;

  /**Non-spatial density dependent instrumental crown radius "f". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRF;

  /**Non-spatial density dependent instrumental crown radius "g". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRG;

  /**Non-spatial density dependent instrumental crown radius "h". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRH;

  /**Non-spatial density dependent instrumental crown radius "i". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRI;

  /**Non-spatial density dependent instrumental crown radius "j". Array size is
   * total # species.*/
  float *mp_fNonSpatDensDepInstCRJ;

  /**Non-spatial logistic density dependent crown depth "a". Array size is
   * total # species.*/
  float *mp_fNonSpatLogDensDepCHA;

  /**Non-spatial logistic density dependent crown depth "b". Array size is
   * total # species.*/
  float *mp_fNonSpatLogDensDepCHB;

  /**Non-spatial logistic density dependent crown depth "c". Array size is
   * total # species.*/
  float *mp_fNonSpatLogDensDepCHC;

  /**Non-spatial logistic density dependent crown depth "d". Array size is
   * total # species.*/
  float *mp_fNonSpatLogDensDepCHD;

  /**Non-spatial logistic density dependent crown depth "e". Array size is
   * total # species.*/
  float *mp_fNonSpatLogDensDepCHE;

  /**Non-spatial logistic density dependent crown depth "f". Array size is
   * total # species.*/
  float *mp_fNonSpatLogDensDepCHF;

  /**Non-spatial logistic density dependent crown depth "g". Array size is
   * total # species.*/
  float *mp_fNonSpatLogDensDepCHG;

  /**Maximum crown radius value. Array is sized number of species.*/
  float *mp_fNCIMaxCrownRadius;

  /**Lamba for NCI crown radius. Array is sized number of species by number of
   * species. */
  float **mp_fNCICRLambda;

  /**NCI crown radius alpha. Array is sized number of species.*/
  float *mp_fNCICRAlpha;

  /**NCI crown radius beta. Array is sized number of species.*/
  float *mp_fNCICRBeta;

  /**NCI crown radius gamma.  Array is sized number of species.*/
  float *mp_fNCICRGamma;

  /**NCI crown radius maximum search distance for neighbors, in meters. Array
   * is sized number of species. */
  float *mp_fNCICRMaxCrowdingRadius;

  /**NCI crown radius crowding effect n.  Array is sized number of species.*/
  float *mp_fNCICRN;

  /**NCI crown radius size effect d.  Array is sized number of species.*/
  float *mp_fNCICRD;

  /**The minimum DBH, in cm, of neighbors to be included in NCI calculations.
  * Array is sized total number of species.*/
  float *mp_fNCICRMinNeighborDBH;

  /**Maximum crown depth value. Array is sized number of species.*/
  float *mp_fNCIMaxCrownDepth;

  /**Lamba for NCI crown depth. Array is sized number of species by number of
   * species. */
  float **mp_fNCICDLambda;

  /**NCI crown depth alpha. Array is sized number of species.*/
  float *mp_fNCICDAlpha;

  /**NCI crown depth beta. Array is sized number of species.*/
  float *mp_fNCICDBeta;

  /**NCI crown depth gamma.  Array is sized number of species.*/
  float *mp_fNCICDGamma;

  /**NCI crown depth maximum search distance for neighbors, in meters. Array
   * is sized number of species. */
  float *mp_fNCICDMaxCrowdingRadius;

  /**NCI crown depth crowding effect n.  Array is sized number of species.*/
  float *mp_fNCICDN;

  /**NCI crown depth size effect d.  Array is sized number of species.*/
  float *mp_fNCICDD;

  /**The minimum DBH, in cm, of neighbors to be included in NCI calculations.
  * Array is sized total number of species.*/
  float *mp_fNCICDMinNeighborDBH;

  /**Max crown radius for standard equation*/
  float m_fMaxStdCrownRad;

  /**Maximum possible crown radius*/
  float m_fMaxCrownRad;

  /**The plot's density, in stems/ha; for non-spatial density dependent crown
   * functions*/
  float m_fPlotDensity;

  /**The plot's basal area, in m2/ha; for non-spatial density dependent crown
   * functions*/
  float m_fPlotBasalArea;

  /**Minimum sapling height.  For doing NCI neighbor searches.*/
  float m_fMinSaplingHeight;

  /**Total number of species*/
  int m_iNumSpecies;

  /**
   * Sorts out and reads in parameters for adult height-diam relationships.
   * @param p_oElement Document element
   */
  void SetupAdultHeightDiam(xercesc::DOMElement * p_oElement);

  /**
   * Sorts out and reads in parameters for adult crown radius relationships.
   * @param p_oElement Document element
   */
  void SetupAdultCrownRadius(xercesc::DOMElement * p_oElement);

  /**
   * Sorts out and reads in parameters for adult crown depth relationships.
   * @param p_oElement Document element
   */
  void SetupAdultCrownDepth(xercesc::DOMElement * p_oElement);

  /**
   * Sorts out and reads in parameters for sapling height-diam relationships.
   * @param p_oElement Document element
   */
  void SetupSaplingHeightDiam(xercesc::DOMElement * p_oElement);

  /**
   * Sorts out and reads in parameters for sapling crown radius relationships.
   * @param p_oElement Document element
   */
  void SetupSaplingCrownRadius(xercesc::DOMElement * p_oElement);

  /**
   * Sorts out and reads in parameters for sapling crown depth relationships.
   * @param p_oElement Document element
   */
  void SetupSaplingCrownDepth(xercesc::DOMElement * p_oElement);

  /**
   * Sorts out and reads in parameters for seedling height-diam relationships.
   * @param p_oElement Document element
   */
  void SetupSeedlingHeightDiam(xercesc::DOMElement * p_oElement);

  /**
   * Calculates height from DBH according to the standard sapling-adult
   * allometry function.  This equation is:
   * <br>Height = 1.35 + (H<sub>1</sub> - 1.35)*(1 - e<sup>-B * DBH</sup>)
   * <br>
   * where
   * Height is height in meters, H<sub>1</sub> is max tree height, B is slope
   * of asymptotic height, and DBH is tree DBH in cm.
   * @param fDbh DBH in cm for which to calculate tree height.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Height in meters.
   */
  float CalcStandardSapAdultHeight(const float &fDbh, const int &iSpecies);

  /**
   * Calculates height from DBH according to the linear function.  This equation
   * is:
   * <br>Height = a + b * DBH
   * <br>
   * where b is the adult linear slope and a is the adult linear intercept.
   * The height is limited to the maximum height for the species.
   * @param fDbh DBH in cm for which to calculate tree height.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Height in meters.
   */
  float CalcLinearAdultHeight(const float &fDbh, const int &iSpecies );

  /**
   * Calculates height from DBH according to the reverse linear function.  This
   * equation is:
   * <br>Height = (DBH - a) / b
   * <br>
   * where b is the adult reverse linear slope and a is the adult reverse linear
   * intercept.  The height is limited to the maximum height for the species.
   * @param fDbh DBH in cm for which to calculate tree height.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Height in meters.
   */
  float CalcReverseLinearAdultHeight(const float &fDbh, const int &iSpecies );

  /**
   * Calculates height from DBH according to the linear function.  This equation
   * is:
   * <br>Height = a + b * DBH
   * <br>
   * where b is the sapling linear slope and a is the sapling linear intercept.
   * The height is limited to the maximum height for the species.
   * @param fDbh DBH in cm for which to calculate tree height.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Height in meters.
   */
  float CalcLinearSaplingHeight(const float &fDbh, const int &iSpecies );

  /**
   * Calculates height from DBH according to the reverse linear function.  This
   * equation is:
   * <br>Height = (DBH - a) / b
   * <br>
   * where b is the sapling reverse linear slope and a is the sapling reverse
   * linear intercept.  The height is limited to the maximum height for the
   * species.
   * @param fDbh DBH in cm for which to calculate tree height.
   * @param iSpecies Species of tree for which to calculate height.
   * @return Height in meters.
   */
  float CalcReverseLinearSaplingHeight(const float &fDbh, const int &iSpecies );

  /**
  * Calculates height from DBH according to the power function.  This
  * equation is:
  * <br>Height = a * D <sup>b</sup>
  * <br>
  * where D is in fact Diam10 - this will convert back and forth with DBH.
  * The height is limited to the maximum height for the species.  If fDbh is
  * negative or 0, this returns the minimum height (0.001).
  * @param fDbh DBH in cm for which to calculate tree height.
  * @param iSpecies Species of tree for which to calculate height.
  * @return Height in meters.
  */
  float CalcPowerSaplingHeight(const float &fDbh, const int &iSpecies );

  /**
  * Calculates height according to standard seedling diam10-height allometry
  * equation.  This equation is:
  * <br>height = 0.1 + 30*(1 - e<sup>-alpha * diam<sub>10</sub>)</sup>)<br>
  * where height is tree height in m, alpha is the slope of the height -
  * diam<sub>10</sub> relationship, and diam<sub>10</sub> is the tree's diameter
  * at 10 cm height.
  * @param fDiam10 Diameter at 10 cm in cm for which to calculate tree height.
  * @param iSpecies Species of tree for which to calculate height.
  * @return Height in meters.
  */
  float CalcStandardSeedlingHeight( const float &fDiam10, const int &iSpecies );

  /**
  * Calculates height from diameter at 10 cm according to the linear function.
  * This equation is:
  * <br>Height = a + b * diam<sub>10</sub>
  * <br>
  * where b is the seedling linear slope and a is the seedling linear
  * intercept.  The height is limited to the maximum height for the species.
  * @param fDiam10 Diameter at 10 cm in cm for which to calculate tree height.
  * @param iSpecies Species of tree for which to calculate height.
  * @return Height in meters.
  */
  float CalcLinearSeedlingHeight(const float &fDiam10, const int &iSpecies );

  /**
  * Calculates height from diameter at 10 cm according to the reverse linear
  * function. This equation is:
  * <br>Height = (diam<sub>10</sub> - a) / b
  * <br>
  * where b is the seedling reverse linear slope and a is the seedling reverse
  * linear intercept.
  * @param fDiam10 Diameter at 10 cm in cm for which to calculate tree height.
  * @param iSpecies Species of tree for which to calculate height.
  * @return Height in meters.
  */
  float CalcReverseLinearSeedlingHeight(const float &fDiam10, const int &iSpecies );

  /**
  * Calculates DBH from height according to the standard sapling-adult
  * allometry function.  This equation (which is just the height equation
  * switched around) is:
  * DBH = log(1-((Height-1.35)/(H<sub>1</sub>-1.35)))/-B where
  * Height is height in meters, H<sub>1</sub> is max tree height, B is slope
  * of asymptotic height, and DBH is tree DBH in cm.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return DBH in cm.
  */
  float CalcStandardSapAdultDbh( const float &fHeight, const int &iSpecies );

  /**
  * Calculates DBH from height according to the linear function.  This equation
  * is:
  * <br>DBH = (Height - a) / b
  * <br>
  * where b is the adult linear slope and a is the adult linear intercept.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return DBH in cm.
  */
  float CalcLinearAdultDbh(const float &fHeight, const int &iSpecies );

  /**
  * Calculates DBH from height according to the reverse linear function.  This
  * equation is:
  * <br>DBH = a + b * Height
  * <br>
  * where b is the adult reverse linear slope and a is the adult reverse linear
  * intercept.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return DBH in cm.
  */
  float CalcReverseLinearAdultDbh(const float &fHeight, const int &iSpecies );

  /**
  * Calculates DBH from height according to the linear function.  This equation
  * is:
  * <br>DBH = (Height - a) / b
  * <br>
  * where b is the sapling linear slope and a is the sapling linear intercept.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return DBH in cm.
  */
  float CalcLinearSaplingDbh(const float &fHeight, const int &iSpecies );

  /**
  * Calculates DBH from height according to the reverse linear function.  This
  * equation is:
  * <br>DBH = a + b * Height
  * <br>
  * where b is the sapling reverse linear slope and a is the sapling reverse
  * linear intercept.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return DBH in cm.
  */
  float CalcReverseLinearSaplingDbh(const float &fHeight, const int &iSpecies );

  /**
  * Calculates DBH from height according to the reverse linear function.  This
  * equation is:
  * <br>D = (H / a) <sup>1/b</sup>
  * <br>
  * d is the diam10.  Then the conversion to DBH is made.  If fHeight is less
  * than or approximately equal to 0, this returns 0.001.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return DBH in cm.
  */
  float CalcPowerSaplingDbh(const float &fHeight, const int &iSpecies );

  /**
  * Calculates diameter at 10 cm according to the standard seedling height -
  * diam10 allometry equation (which is just the height equation switched
  * around).  The equation is:
  * diam<sub>10</sub> = (log(1-((height-0.1)/30)))/-alpha where
  * height is tree height in m, alpha is the slope of the height -
  * diam<sub>10</sub> relationship, and diam<sub>10</sub> is the tree's diameter
  * at 10 cm height.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return Diameter at 10 cm in cm.
  */
  float CalcStandardSeedlingDiam10( const float &fHeight, const int &iSpecies );

  /**
  * Calculates height from diameter at 10 according to the linear function.
  * This equation is:
  * <br>diam<sub>10</sub> = (Height - a) / b
  * <br>
  * where b is the seedling linear slope and a is the seedling linear intercept.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return DBH in cm.
  */
  float CalcLinearSeedlingDiam10(const float &fHeight, const int &iSpecies );

  /**
  * Calculates height from DBH according to the reverse linear function.  This
  * equation is:
  * <br>diam<sub>10</sub> = a + b * Height
  * <br>
  * where b is the seedling reverse linear slope and a is the seedling reverse
  * linear intercept.
  * @param fHeight Height in meters.
  * @param iSpecies Species of tree for which to calculate height.
  * @return DBH in cm.
  */
  float CalcReverseLinearSeedlingDiam10(const float &fHeight, const int &iSpecies );

  /**
  * Calculates crown radius according to the standard equation.  This
  * equation is: crad = C<sub>1</sub> * DBH <sup>a</sup> where
  * crad is the crown radius in meters, C<sub>1</sub> is the asymptotic
  * crown radius, DBH is the DBH of the tree in cm, and a is the crown radius
  * exponent.  The value of crad cannot exceed 10 m.
  * @param p_oTree The tree for which to calculate crown radius.
  * @return Crown radius in meters, up to a maximum value of
  * m_fMaxStdCrownRad.
  */
  float CalcStandardSapAdultCrownRad(clTree *p_oTree);

  /**
  * Calculates adult crown radius using the Chapman-Richards equation.  This
  * equation is:  CR = i + a * (1 - exp(-b * DBH)) <sup>c</sup> where CR =
  * crown radius in meters, i = crown radius intercept in meters, a =
  * asymptotic crown radius (in meters), b = shape parameter 1, c = shape
  * parameter 2, DBH = DBH of the tree in cm.  Since this equation is already
  * self-limiting, it's not subjected to the limit of 10 m like the standard
  * equation is.
  * @param p_oTree The tree for which to calculate crown radius.
  * @return Crown radius in meters.
  */
  float CalcChapRichSapAdultCrownRad(clTree *p_oTree);

  /**
  * Calculates adult crown radius using the non-spatial density dependent
  * exponential equation.  This equation is:
  * @htmlonly
   <center>rad = D1 * DBH<sup>a</sup> * Height<sup>b</sup> * chi<sup>c</sup> *
   STPH<sup>d</sup> * BAPH<sup>e</sup> * BAL<sup>f</sup></center>
   where:
   <ul>
   <li>rad is the crown radius, in meters</li>
   <li>D1 is the exponential density dependent slope parameter</li>
   <li>a is the exponential density dependent "a" parameter</li>
   <li>b is the exponential density dependent "b" parameter</li>
   <li>c is the exponential density dependent "c" parameter for the estimated
   value of crown depth from the instrumental equation of chi</li>
   <li>d is the exponential density dependent "d" parameter</li>
   <li>e is the exponential density dependent "e" parameter</li>
   <li>f is the exponential density dependent "f" parameter</li>
   <li>DBH is the tree's DBH, in cm</li>
   <li>Height is the tree height, in meters</li>
   <li>chi is the crown depth of the target tree, in meters, estimated from
   instrumental variables</li>
   <li>STPH is number of stems per hectare of adult trees within the plot</li>
   <li>BAPH is the basal area, in squared meters per hectare, of adult trees
   within the plot</li>
   <li>BAL is the sum of the basal area of trees larger than the height of the
   target tree, in square meters per hectare, within the plot</li>
   </ul>
   The instrumental equation for calculating chi is as follows:
   <center>chi = a + b * DBH + c*Height + d*DBH<sup>2</sup> +
   e*Height<sup>2</sup> + f*1/DBH + g*STPH + h*BAPH + i*BAL + j*HD</center>
   where:
   <ul>
   <li>a is the intercept parameter</li>
   <li>b is the instrumental linear slope "b" parameter</li>
   <li>c is the instrumental linear "c" slope parameter</li>
   <li>d is the instrumental linear "d" slope parameter</li>
   <li>e is the instrumental linear "e" slope parameter</li>
   <li>f is the instrumental linear slope "f" parameter</li>
   <li>g is the instrumental linear slope "g" parameter</li>
   <li>h is the instrumental linear slope "h" parameter</li>
   <li>i is the instrumental linear slope "i" parameter</li>
   <li>j is the instrumental linear slope "j" parameter</li>
   <li>DBH is the DBH of the tree, in cm</li>
   <li>Height is the tree height, in meters</li>
   <li>STPH is the number of stems per hectare, of adult trees within the
   plot</li>
   <li>BAPH is the density, in basal area per hectare, of adult trees within
   the plot</li>
   <li>BAL is the sum of the basal area of trees larger than the height of the
   target tree, in square m per hectare, within the plot</li>
   <li>HD is the slenderness ratio (height/DBH) for the target tree</li>
   </ul>
   @endhtmlonly
  * @param p_oTree The tree for which to calculate crown radius.
  * @return Crown radius in meters.
  */
  float CalcNonSpatDensDepExpAdultCrownRad(clTree *p_oTree);

  /**
  * Calculates crown depth according to the standard equation. This
  * equation is: CH = C<sub>2</sub> * Height <sup>a</sup> where
  * CH is the depth of the crown in meters, C<sub>2</sub> is the asymptotic
  * crown depth, Height is the tree's height in meters, and a is the crown
  * depth exponent.
  * @param p_oTree The tree for which to calculate crown depth .
  * @return Adult crown depth in meters.
  * @throw ModelErr If the species isn't recognized.
  */
  float CalcStandardSapAdultCrownDepth(clTree *p_oTree);

  /**
  * Calculates crown depth using the Chapman-Richards equation.  This
  * equation is:  CH = i + a * (1 - exp(-b * H)) <sup>c</sup> where CH =
  * crown depth in meters, i = crown depth intercept in meters, a =
  * asymptotic crown depth (in meters), b = shape parameter 1, c = shape
  * parameter 2, DBH = DBH of the tree in cm.
  * @param p_oTree The tree for which to calculate crown radius.
  * @return Crown radius in meters.
  */
  float CalcChapRichSapAdultCrownDepth(clTree *p_oTree);

  /**
  * Calculates crown depth using the non-spatial density dependent logistic
  * function. The function is:
  * @htmlonly
  <center>ch = Height / <br>
  (1 + exp(a + b*DBH + c*Height + d*radi + e*STPH + f*BAPH + g*BAL))
  </center>
  where:
  <ul>
  <li>ch is the crown depth, in meters</li>
  <li>a is the density dependent logistic crown depth shape parameter</li>
  <li>b is the density dependent logistic "b"  exponent parameter</li>
  <li>c is the density dependent logistic "c" exponent parameter</li>
  <li>d is the density dependent logistic "d" exponent parameter for the
  estimated value of crown radius from the instrumental equation of radi</li>
  <li>e is the density dependent logistic "e" exponent parameter</li>
  <li>f is the density dependent logistic "f" exponent parameter</li>
  <li>g is the density dependent logistic "g" exponent parameter</li>
  <li>DBH is the tree's DBH, in cm</li>
  <li>radi is the crown radius of the target tree, in meters, estimated from
  instrumental variables</li>
  <li>STPH is the number of stems per hectare of adult trees within the
  plot</li>
  <li>BAPH is the basal area, in square meters per hectare, of adult trees
  within the plot</li>
  <li>BAL is the sum of the basal area of trees larger than the height of the
  target tree, in square meters per hectare, within the plot</li>
  </ul>
  The instrumental equation for calculating radi is as follows:
  <center>radi = a + b*DBH + c*Height + d*DBH<sup>2</sup> + e*Height<sup>2</sup>
   + f*1/DBH + g*STPH + h*BAPH + i*BAL + j*HD</center>
  where:
  <ul>
  <li>a is the intercept parameter</li>
  <li>b is the instrumental linear slope "b" parameter</li>
  <li>c is the instrumental linear "c" slope parameter</li>
  <li>d is the instrumental linear "d" slope parameter</li>
  <li>e is the instrumental linear "e" slope parameter</li>
  <li>f is the instrumental linear slope "f" parameter</li>
  <li>g is the instrumental linear slope "g" parameter</li>
  <li>h is the instrumental linear slope "h" parameter</li>
  <li>i is the instrumental linear slope "i" parameter</li>
  <li>j is the instrumental linear slope "j" parameter</li>
  <li>DBH is the DBH of the tree, in cm</li>
  <li>Height is the tree height, in meters</li>
  <li>STPH is the number of stems per hectare, of adult trees within the
  plot</li>
  <li>BAPH is the density, in basal area per hectare, of adult trees within
  the plot</li>
  <li>BAL is the sum of the basal area of trees larger than the height of the
  target tree, in square m per hectare, within the plot</li>
  <li>HD is the slenderness ratio (height/DBH) for the target tree</li>
  </ul>
  @endhtmlonly
  * @param p_oTree The tree for which to calculate crown radius.
  * @return Crown radius in meters.
  */
  float CalcNonSpatDensDepLogAdultCrownDepth(clTree *p_oTree);

  /**
  * Calculates crown radius according to the NCI function.
  * <center><i>Crown radius = Max crown radius * Size Effect * Crowding Effect</i></center>
  *
  * Crowding Effect is calculated as:
  @htmlonly
  <center><i>CE = exp(-n * NCI)</i></center>
  @endhtmlonly
  * <br>
  * where:
  * <ul>
  * <li><i>CE</i> = crowding effect</li>
  * <li><i>n</i> is a parameter</li>
  * <li><i>NCI</i> is this tree's NCI value</li>
  * </ul>
  *
  * NCI is calculated as follows (simplifying the notation):
  @htmlonly
  <center><i>NCI = (&Sigma; &lambda;<sub>k</sub>(DBH<sub>k</sub><sup>&alpha;</sup>/distance<sup>&beta;</sup>)) * exp (&gamma; DBH ) </i></center>

  where:
  <ul>
  <li>we're summing over k = 0 to N neighbors greater than the minimum
  neighbor DBH</li>
  <li><i>&alpha;</i> is the neighbor DBH effect parameter</li>
  <li><i>&beta;</i> is the neighbor distance effect parameter</li>
  <li><i>DBH</i> is of the target tree, in cm</li>
  <li><i>DBH<sub>k</sub></i> is the DBH of the kth neighbor, in meters</li>
  <li><i>&gamma;</i> is the size sensitivity to NCI parameter</li>
  <li><i>&lambda;<sub>k</sub></i> is the NCI lambda parameter for the species
  of the kth neighbor</li>
  <li><i>distance</i> is distance from target to neighbor, in meters</li>
  </ul>
  <br>
  @endhtmlonly
  *
  * Size effect is calculated as follows:
  * <center><i>SE = 1 - exp (-d * DBH) </i></center>
  * where d is a parameter.
  * @param p_oTree The tree for which to calculate crown radius.
  * @return Crown radius in meters.
  */
  float CalcNCICrownRad(clTree *p_oTree);

  /**
  * Calculates crown depth according to the NCI function.
  * <center><i>Crown depth = Max crown depth * Size Effect * Crowding Effect</i></center>
  *
  * Crowding Effect is calculated as:
  @htmlonly
  <center><i>CE = exp(-n * NCI)</i></center>
  @endhtmlonly
  * <br>
  * where:
  * <ul>
  * <li><i>CE</i> = crowding effect</li>
  * <li><i>n</i> is a parameter</li>
  * <li><i>NCI</i> is this tree's NCI value</li>
  * </ul>
  *
  * NCI is calculated as follows (simplifying the notation):
  @htmlonly
  <center><i>NCI = (&Sigma; &lambda;<sub>k</sub>(DBH<sub>k</sub><sup>&alpha;</sup>/distance<sup>&beta;</sup>)) * exp (&gamma; DBH ) </i></center>

  where:
  <ul>
  <li>we're summing over k = 0 to N neighbors greater than the minimum
  neighbor DBH</li>
  <li><i>&alpha;</i> is the neighbor DBH effect parameter</li>
  <li><i>&beta;</i> is the neighbor distance effect parameter</li>
  <li><i>DBH</i> is of the target tree, in cm</li>
  <li><i>DBH<sub>k</sub></i> is the DBH of the kth neighbor, in meters</li>
  <li><i>&gamma;</i> is the size sensitivity to NCI parameter</li>
  <li><i>&lambda;<sub>k</sub></i> is the NCI lambda parameter for the species
  of the kth neighbor</li>
  <li><i>distance</i> is distance from target to neighbor, in meters</li>
  </ul>
  <br>
  @endhtmlonly
  *
  * Size effect is calculated as follows:
  * <center><i>SE = 1 - exp (-d * DBH) </i></center>
  * where d is a parameter.
  *
  * Crown depth is limited to the total tree height.
  * @param p_oTree The tree for which to calculate crown radius.
  * @return Crown radius in meters, up to a maximum value of
  * m_fMaxStdCrownRad.
  */
  float CalcNCICrownDepth(clTree *p_oTree);

}; //end of class clAllometry


//---------------------------------------------------------------------------
#endif
