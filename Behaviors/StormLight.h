//---------------------------------------------------------------------------

#ifndef StormLightH
#define StormLightH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"
#include "DataTypes.h"


class clGrid;
class clTreePopulation;

/**
* Storm Light - Version 1.1
*
* This behavior calculates light level as a function of local storm damage.
* It uses a grid to store these light values (light values are not assigned to
* individual trees).
*
* The light level is calculated as follows:
* <br><center><i>GLA = ((1 - T/M) * 100) + (a + b * N)</i></center>
* where:
* <ul>
* <li>GLA is the light level, as a value between 0 and 100</li>
* <li>T is the total number of adults and snags (damaged and undamaged)</li>
* <li>M is the minimum number of adults and snags for a full canopy</li>
* <li>a is the intercept of the function</li>
* <li>b is the slope of the function</li>
* <li>N is the proportion of "trees that count" (see below) within a
* radius R</li>
* </ul>
*
* The first term in the equation corrects for the possibility of not being
* under a full canopy.  If T >= M, then the term drops out and only the linear
* second half is used.  If T < M, then the trees get at least the proportion
* of full sun equal to the proportion of trees missing from the full canopy.
* When counting up T, all adults and snags count, whether storm-damaged or not.
*
* For the second term in the equation, "trees that count" are those that have
* been heavily damaged in recent storms or those that have died from natural
* mortality. Trees count as heavily damaged if they are either snags with a
* storm counter (i.e. trees killed in storms) or live adults with a storm
* damage flag indicating heavy damage.  Saplings and seedlings never count.
* Storm-damaged trees have a counter set for time since the
* storm, and natural mortality snags have an age.  There is a maximum time,
* after which a tree is not counted in light calculations even if it is still
* damaged (or hanging around as a snag).
*
* The trees are counted within a user-set radius of R from the center of the
* grid cell. The proportion is heavily damaged adults and snags divided by
* total adults and snags.
*
* The light levels can be randomized, if desired.  The user can choose to use
* this deterministically, in which case the values equal the GLA value as
* calculated in the function above.  Alternately, this value can be used as
* the mean for a random draw on a probability distribution function.  The user
* can specify the PDF to use.
*
* This behavior creates a new grid called "Storm Light".
*
* The namestring and parameter file call string for this behavior are both
* "StormLight".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clStormLight : virtual public clBehaviorBase {

 public:

  /**
  * Constructor.  Sets the namestring.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clStormLight(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clStormLight();

  /**
  * Does setup for this behavior.  This reads parameter file values and sets
  * up the grid.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if the value for stochasticity is unrecognized, or if
  * the minimum number of canopy trees is a negative number.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs light calculations.  This goes through each cell in "Storm Light"
  * and calculates its GLA value as described above.
  */
  void Action();

  protected:

  /**Enum to translate our types to array index*/
  enum types {adult, snag};

  /**Grid object which holds the light values.  The name of this grid is "Storm
  * Light".  It has one float data member called "Light".  It uses the default
  * grid cell resolution unless otherwise instructed in the parameter file.
  * A map of this grid in the parameter file will be ignored.
  */
  clGrid *mp_oLightGrid;

  /**Return codes for the "stm_dmg" int tree data member.  Array index one is
  * sized m_iNumTypes; array index two is sized total number of species.*/
  int **mp_iStmDmgCodes;

  /**The max radius, in m, to search for storm-damaged neighbors.*/
  float m_fMaxRadius;

  /**The slope of the light function.*/
  float m_fSlope;

  /**The intercept of the light function.*/
  float m_fIntercept;

  /**Standard deviation if normal or lognormal distribution is desired. Or
   * nothing, if we are using deterministic light levels.*/
  float m_fRandParameter;

  /**The minimum number of trees within the search radius for the site to
  * qualify as under full canopy.*/
  float m_fMinCanopyTrees;

  /**The max time since damage, in years, that a live tree can count toward
  * light calculations.*/
  int m_iMaxDmgTime;

  /**The max time since damage, in years, that a snag can count toward
  * light calculations.*/
  int m_iMaxSnagDmgTime;

  /**Number of types - 2.*/
  int m_iNumTypes;

  /**What stochasticity to apply to light levels*/
  pdf m_iStochasticity;

  /**Return code for the "Light" data member.*/
  short int m_iGridLightCode;
};
//---------------------------------------------------------------------------
#endif
