//---------------------------------------------------------------------------
// LightDepSeedSurvival.h
//---------------------------------------------------------------------------
#if !defined(LightDepSeedSurvival_H)
  #define LightDepSeedSurvival_H

#include "GLIBase.h"

class clTreePopulation;
class clTree;
class clGrid;
class clLightOrg;
/**
* Light Dependent Seed Survival - Version 1.1
*
* This behavior assesses seed survival based on light levels.  The starting
* number of seeds is created by any disperse behavior and placed in the grid
* "Dispersed Seeds".  The number of seeds surviving out of this number is a
* function of GLI.  Because of the need to calculate light levels, this class
* descends from clGLIBase.
*
* The number of seeds of a given species that survive in a particular
* "Dispersed Seeds" grid cell is:
* <br><center>R<sub>sp</sub> = S<sub>sp</sub> * LE</center><br>
* where:
* <ul>
* <li>R<sub>sp</sub> is the number of seeds surviving</li>
* <li>S<sub>sp</sub> is the starting number of seeds of the species, i.e.
* the number of seeds dispersed to that grid cell
* <li>LE is the light favorability, from 0 to 1</li>
* </ul>
*
* The number of seeds is given a random round.
*
* LE is calculated from GLI as:
* @htmlonly
 <ul>
 <li>If GLI &lt; GLI<sub>opt</sub>, LE = 1 - (S<sub>lo</sub>(GLI<sub>opt</sub> - GLI))</li>
 <li>If GLI = GLI<sub>opt</sub>, LE = 1</li>
 <li>If GLI &gt; GLI<sub>opt</sub>, LE = 1 - (S<sub>hi</sub>(GLI - GLI<sub>opt</sub>))</li>
 </ul>
 @endhtmlonly
* where
* <ul>
* <li>GLI is the global light index - GLI at the center of the "Dispersed Seeds" grid cell at a
* user-specified height</li>
* <li>GLI<sub>opt</sub> is the optimum GLI for the given species</li>
* <li>S<sub>lo</sub> is the slope of the dropoff in light favorability at
* GLIs below GLI<sub>opt</sub></li>
* <li>S<sub>hi</sub> is the slope of the dropoff in light favorability at
* GLIs above GLI<sub>opt</sub></li>
* </ul>
*
* GLI can come in two ways.  The first method is that this behavior will
* calculate GLI like all other GLIs.  There is one difference:  neighbor
* trees that have hurricane damage have different light extinction coefficients
* from their undamaged neighbors. To achieve this, we override the function
* clLightBase::GetLightExtinctionCoefficient().  This effect is not required.
*
* The second way is to get the GLI from the Storm Light grid.
*
* A fatal error will be thrown if a disperse behavior is not used, or if Storm
* Light is the source of GLIs but it is not used.
*
* The namestring and parameter file call string for this class is
* "LightDependentSeedSurvival" (for having this behavior calculate GLI) or
* "StormLightDependentSeedSurvival" (for having Storm Light be the GLI
* source).  Apply this behavior to the desired species;
* use any type, since type will be ignored.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clLightDepSeedSurvival : virtual public clGLIBase {
//note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clLightDepSeedSurvival(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clLightDepSeedSurvival();

  /**
  * Performs setup.  Calls:
  * <ul>
  * <li>GetParameterFileData()</li>
  * <li>GetTreeDataMemberCodes()</li>
  * <li>SetupGrid()</li>
  * <li>DoLightSetup()</li>
  * </ul>
  * @param p_oDoc DOM tree of parsed input file.
  */
   void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs establishment.  For each grid cell in "Dispersed Seeds", for each
  * species, this takes the number of seeds and determines how many of them
  * germinate.  Steps for each grid cell:
  * <ol>
  * <li>Call GetGLI() to get the light level at the center of the grid
  * cell.</li>
  * <li>For each species, call GetLightFavorability() to get the light
  * favorability.</li>
  * <li>Calculate @htmlonly R<sub>sp</sub> = S<sub>sp</sub>*LE*exp(-c*S<sub>sp</sub><sup>&delta;</sup>)@endhtmlonly
  * for each species.</li>
  * <li>Create a seedling for each of R<sub>sp</sub> for each species.</li>
  * </ol>
  */
   void Action();

  /**
  * Captures the namestring passed to this behavior.  This is overridden from
  * clBehaviorBase so we can capture the namestring passed.  Since this class
  * can create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

  /**Overridden from clLightBase to do nothing*/
  void RegisterTreeDataMembers() {;};
  /**Overridden from clLightBase to do nothing*/
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop) {return 0;};

  protected:

  /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
  clGrid *mp_oSeedGrid;

  /**Pointer to the "Storm Light" grid created by clStormLight, if needed.*/
  clGrid *mp_oStormLightGrid;

  /**Storm light extinction coefficients, if m_bUseStormLight is false.  Array
  * size is m_iNumTotalSpecies by 2 - number of damage categories.*/
  double **mp_fLightExtCoeff;

  /**Optimum light level at which there is no reduction in establishment -
  * one per behavior species*/
  double *mp_fOptimumGLI;

  /**Slope of favorability dropoff below the optimum light level - one per
  * behavior species*/
  double *mp_fLowGLISlope;

  /**Slope of favorability dropoff above the optimum light level - one per
  * behavior species*/
  double *mp_fHighGLISlope;

  /**Height at which light is to be calculated, if m_bUseStormLight is false.*/
  double m_fLightHeight;

  /**Maximum search distance for shading neighbors, if m_bUseStormLight is
  * false.*/
  double m_fMaxSearchDistance;

  /**Data member codes for seed grid for number of seeds.  Array size is #
  * behavior species.*/
  short int *mp_iSeedGridCode;

  short int **mp_iDamageCodes; /**<Holds return data codes for the "stm_dmg"
  tree data member, if m_bUseStormLight is false.  Array size is number of
  total species by 2 (saplings and adults).*/

  /**Speeds access to arrays*/
  short int *mp_iIndexes;

  /**Number of species. For the destructor.*/
  short int m_iNumTotalSpecies;

  /**Index for the "Light" data member of the Storm Light grid, if
  * m_bUseStormLight is true*/
  short int m_iLightCode;

  /**Whether or not to have this behavior calculate GLI (false) or get GLI
  * from the Storm Light grid (true)*/
  bool m_bUseStormLight;

  /**
  * Declares arrays and fills them with parameter file data.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if:
  * <ul>
  * <li>m_bUseStormLight is false, and any of the light extinction coefficient
  * values are not between 0 and 1.</li>
  * <li>m_bUseStormLight is false, and the value for m_fLightHeight is not zero
  * or greater</li>
  * <li>The value for mp_fOptimumGLI is not between 0 and 100.</li>
  * <li>m_bUseStormLight is false, and the value for m_iNumAltDiv is not
  * greater than 0.</li>
  * <li>m_bUseStormLight is false, and the value for m_iNumAziDiv is not
  * greater than 0.</li>
  * </ul>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Populates required grid pointers.  This gets a pointer to the "Dispersed
  * Seeds" grid and all its data members, and does the same for the "Storm
  * Light" grid if m_bUseStormLight is true.
  * @throws modelErr if the "Dispersed Seeds" grid does not exist, or if
  * the "Storm Light" grid does not exist and m_bUseStormLight is true.
  */
  void SetupGrid();

  /**
  * Performs calculations required of light.  This calculates the brightness
  * array for GLI and the values for m_fAziChunkConverter, m_fRcpTanMinAng,
  * mp_fAziSlope, m_fSinMinSunAng, m_iMinAngRow, and m_fMaxSearchDistance.
  * This function does nothing if m_bUseStormLight is false.
  */
  void DoLightSetup();

  /**
  * Gets the GLI for a given point.  If m_bUseStormLight is true, this gets the
  * value of "Storm Light" for that grid.  If false, this calculates GLI at
  * the height in m_fLightHeight.
  * @param p_oPop Tree population, for getting shading neighbors.
  * @param fX X coordinate of point for which to get GLI.
  * @param fY Y coordinate of point for which to get GLI.
  * @return GLI value.
  */
  float GetGLI(clTreePopulation *p_oPop, const float &fX, const float &fY);

  /**
  * Gets the return codes for needed tree data members.  This declares and
  * populates the mp_iDamageCodes array with the return codes for the
  * "stm_dmg" tree int data member.  It is not an error if they are not
  * present.
  * This function does nothing if m_bUseStormLight is false.
  */
  void GetTreeDataMemberCodes();

  /**
  * Calculates the light favorability for a given species and GLI.  This GLI is
  * turned into a proportion germinating according to the following:
  * <ul>
  * <li>If GLI = optimum GLI for a species, then proportion germinating = 1.
  * <li>If GLI < optimum GLI for a species, then proportion germinating = 1 -
  * <li>(slope below optimum*(optimum GLI - actual GLI)).
  * <li>If GLI > optimum GLI for a species, then proportion germinating = 1 -
  * (slope above optimum*(actual GLI - optimum GLI)).
  * </ul>
  * @param iSpecies Species for which to calculate light favorability.
  * @param fGLI GLI value.
  */
  float GetLightFavorability(const int &iSpecies, const float &fGLI);

  /**
  * Gets the light extinction coefficent.  If the tree has storm damage, the
  * light extinction coefficient appropriate to the tree's damage category is
  * returned.  Otherwise the default light extinction coefficient behavior of
  * clLightOrg is used.
  * @param p_oTree Tree for which to obtain the light extinction coefficient.
  * @return The light extinction coefficient, as the proportion of light
  * transmitted by the tree, as a value between 0 and 1.
  */
  float GetLightExtinctionCoefficient(clTree *p_oTree);
};
//---------------------------------------------------------------------------
#endif // PuertoRicoEstablishment_H
