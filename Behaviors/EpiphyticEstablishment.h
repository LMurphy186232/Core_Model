//---------------------------------------------------------------------------
// EpiphyticEstablishment.h
//---------------------------------------------------------------------------
#if !defined(EpiphyticEstablishment_H)
  #define EpiphyticEstablishment_H

#include "GLIBase.h"

class clTreePopulation;
class clTree;
class clLightOrg;
/**
* Tree fern establishment - Version 1.0
*
* Treefern establishment simulates epiphytic seedlings that root when their
* substrate tree dies.  Unusually for behaviors, this is applied to the species
* that serve as substrate rather than the species to which the seedlings will
* belong.  This is because all of the function parameters depend on the species
* of the substrate trees.
*
* When trees of one of the substrate species dies, it may allow exactly one of
* its epiphytic seedlings to root.  The probability that a dead tree will
* produce a seedling is:
*
* P = 1 - (1/ ( 1 + exp( a + b * H + c * GLI )))
*
* where H is the height of the substrate tree, and GLI is the light level at the
* tree's location. The light level is calculated halfway between the ground
* and the base of the crown.
*
* If a seedling is produced, its height is:
*
* HS = m + n*H
*
* where HS is the height of the seedling in cm, and H is the height of the
* substrate tree in m.
*
* The parameters designate which species the seedlings are of, and all seedlings
* produced are only of that species.  The seedling roots in the exact spot where
* the substrate tree was.
*
* Dead substrate trees are identified as such by checking the value in their
* "dead" code.  Thus it is important that this behavior run after mortality but
* before tree removal.
*
* Trees to which this behavior is applied ( that is, substrate trees) must also
* have a mortality behavior applied. This can only be applied to saplings and
* adults.
*
* This behavior's name string and parameter file call string are both
* "EpiphyticEstablishment".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clEpiphyticEstablishment : virtual public clGLIBase {
//note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clEpiphyticEstablishment(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clEpiphyticEstablishment();

  /**
  * Performs setup.  Calls:
  * <ul>
  * <li>GetParameterFileData()</li>
  * <li>GetTreeDataMemberCodes()</li>
  * <li>DoLightSetup()</li>
  * <li>FormatQueryString()</li>
  * </ul>
  * @param p_oDoc DOM tree of parsed input file.
  */
   void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs establishment. This finds dead individuals of the establishment
  * substrate species; calculates the GLI halfway up the trunk; and uses that
  * to determine the probability that a seedling will root. If a seedling roots,
  * its height is calculated.  Then its xy coordinates and height are saved
  * until all dead substrate trees have been processed. Then all seedlings are
  * created at once.  The reason seedlings are created at the end is because
  * the height calculation may in fact make them saplings.  Then they would
  * affect GLI calculations.
  */
   void Action();

  /**Overridden from clLightBase to do nothing*/
  void RegisterTreeDataMembers() {;};
  /**Overridden from clLightBase to do nothing*/
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop) {return 0;};

  protected:

  /**Codes for the "dead" tree data member.  Array size is number of species
   * by number of types. This is the only data member we will keep because it
   * will serve as an easy flag for those trees to which this behavior is
   * applied.*/
  short int **mp_iDeadCodes;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate volume.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  char *m_cQuery;

  /**a in the seedling probability equation.  Array size is number of species.*/
  float *mp_fA;

  /**b in the seedling probability equation.  Array size is number of species.*/
  float *mp_fB;

  /**c in the seedling probability equation.  Array size is number of species.*/
  float *mp_fC;

  /**m in the seedling height equation.  Array size is number of species.*/
  float *mp_fM;

  /**n in the seedling height equation.  Array size is number of species.*/
  float *mp_fN;

  /**Maximum search distance for shading neighbors.*/
  float m_fMaxSearchDistance;

  /**Species of seedlings to disperse.*/
  int m_iSeedlingSpecies;

  /**Number of species. For the destructor.*/
  short int m_iNumTotalSpecies;


  /**
  * Declares arrays and fills them with parameter file data.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if:
  * <ul>
  * <li>Any of the light extinction coefficient values are not between 0
  * and 1.</li>
  * <li>The value for m_iNumAltDiv is not greater than 0.</li>
  * <li>The value for m_iNumAziDiv is not greater than 0.</li>
  * </ul>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs calculations required of light.  This calculates the brightness
  * array for GLI and the values for m_fAziChunkConverter, m_fRcpTanMinAng,
  * mp_fAziSlope, m_fSinMinSunAng, m_iMinAngRow, and m_fMaxSearchDistance.
  */
  void DoLightSetup();

  /**
  * Gets the GLI for a given point.
  * @param p_oPop Tree population, for getting shading neighbors.
  * @param fX X coordinate of point for which to get GLI.
  * @param fY Y coordinate of point for which to get GLI.
  * @param fHeight Height at which to get GLI.
  * @return GLI value.
  */
  float GetGLI(clTreePopulation *p_oPop, const float &fX, const float &fY,
      const float &fHeight);

  /**
  * Gets the return codes for the "dead" tree data member. This declares and
  * populates the mp_iDeadCodes array with the return codes for the
  * "dead" tree int data member.
  * @throws modelErr if a tree to which this species is applied does not have
  * a "dead" code, or if this is applied to anything but saplings or adults.
  */
  void GetTreeDataMemberCodes();

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  */
  void FormatQueryString();

};
//---------------------------------------------------------------------------
#endif // PuertoRicoEstablishment_H
