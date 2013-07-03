//---------------------------------------------------------------------------
// NCIMasterMortality
//---------------------------------------------------------------------------
#if !defined(NCIMasterMortality_H)
  #define NCIMasterMortality_H

#include "MortalityBase.h"
#include "NCIEffectsList.h"


class clTree;
class clTreePopulation;
class clPlot;
class clShadingEffectBase;
class clDamageEffectBase;
class clSizeEffectBase;
class clNCITermBase;
class clCrowdingEffectBase;
class clPrecipitationEffectBase;
class clTemperatureEffectBase;
class clNitrogenEffectBase;

/**
* NCI Mortality - Version 3.0
*
* This is a mortality shell object which applies an NCI (neighborhood
* competition index) function to assess probability of survival. The basic
* function is a maximum rate of survival that is reduced by a set of
* multiplicative effects. The set of effects used is up to the user.
*
* The annual survival probability is compounded for multi-year timesteps by
* taking it to the X power, where X is the number of years per timestep.
*
* The namestring for this class is "ncimortshell". The parameter file call
* string is "NCIMortality".
*
* This behavior can only be applied to saplings and adults.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>June 28, 2013 - Created (LEM)
*/
class clNCIMasterMortality : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clNCIMasterMortality(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clNCIMasterMortality();

  /**
  * Determines mortality for a tree.
  *
  * @param p_oTree Tree being evaluated.
  * @param fDbh DBH of tree being evaluated.
  * @param iSpecies Species of tree being evaluated.
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort (clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  /**
  * Performs behavior setup.  First, ReadParameterFile() is called to read
  * the parameter file's data.  Then ValidateData() is called to validate the
  * data. Then GetTreeMemberCodes() is called to get tree data return codes.
  *
  * @param p_oDoc DOM tree of parsed input tree.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs calculations before any trees have been killed.  This finds all
  * trees to which this behavior applies and performs their NCI calculations.
  * Then, having done all that work, this function goes ahead and assesses the
  * tree's mortality.  Whether it lives or dies is then stashed in the
  * "NCI Mort" bool tree data member.
  *
  * @param p_oPop Tree population.
  */
  void PreMortCalcs( clTreePopulation *p_oPop );

  protected:

  clShadingEffectBase *mp_oShadingEffect;
  clDamageEffectBase *mp_oDamageEffect;
  clSizeEffectBase *mp_oSizeEffect;
  clCrowdingEffectBase *mp_oCrowdingEffect;
  clNCITermBase *mp_oNCITerm;
  clPrecipitationEffectBase *mp_oPrecipEffect;
  clTemperatureEffectBase *mp_oTempEffect;
  clNitrogenEffectBase *mp_oNEffect;

  /**Return codes for the "dead" tree int data member variable.  Array size
   * is number of species by number of tree types (even if not every species
   * and type is represented).*/
  short int **mp_iDeadCodes;

  float m_fNumberYearsPerTimestep; /**<Number of years per timestep. From sim
       manager*/

  char *m_cQuery; /**<Query string to get NCI trees*/

  /**Maximum survival value. Array sized number of species.*/
  float *mp_fMaxPotentialValue;

  /**
  * Makes sure all input data is valid. Max probability of survival for each
  * species must be 0-1.
  * @throws modelErr if max probability of survival for any species is not
  * between 0 and 1.
  */
  void ValidateData();

  /**
  * Reads data from the parameter file.
  * @param p_oDoc DOM tree of parsed input tree.
  * @throws modelErr if this behavior has been applied to any types except
  * sapling and adult.
  */
  void ReadParameterFile( xercesc::DOMDocument *p_oDoc );

  /**
  * Gets the return codes for needed tree data members.
  * @throws modelErr if a light code comes back -1 for any species which uses
  * the shading effect.
  */
  void GetTreeMemberCodes();

  /**
  * Populates m_cQuery with the query for getting NCI trees.
  */
  void FormatQuery();

};
//---------------------------------------------------------------------------
#endif // NCIMasterMortality_H
