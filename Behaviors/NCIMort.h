//---------------------------------------------------------------------------
// NCIMort
//---------------------------------------------------------------------------
#if !defined(NCIMort_H)
  #define NCIMort_H

#include "MortalityBase.h"
#include "NCIBase.h"


class clTree;
class clTreePopulation;
class clPlot;

/**
* NCI Mortality - Version 2.0
*
* This is a mortality shell object which applies an NCI (neighborhood
* competition index) function to assess probability of survival.
*
* The function for annual survival probability:
* <center><i>Survival probability = Max Probability * Damage Effect *
* Size Effect * Crowding Effect * Shading Effect</i></center>
*
* All NCI calculations are performed according to clNCIBase.
*
* The annual survival probability is compounded for multi-year timesteps by
* taking it to the X power, where X is the number of years per timestep.
*
* This is a generic behavior. It is expected that not all terms in the
* equation will be used. A user can turn off some of the terms by setting key
* parameters to 0 or 1. To be efficient, this behavior looks for this and
* has several alternate function forms to avoid extra math. It calls the
* correct function form through function pointers, which it sets during setup.
*
* The namestring for this class is "ncimortshell". The parameter file call
* string is "NCIMortality".
*
* This behavior adds a new boolean data member called "NCI Mort". This holds
* the result of the mortality calculation that happens before the main
* mortality process. This is so neighbors that die this timestep aren't
* artificially excluded from the NCI calculation.
*
* This behavior can only be applied to saplings and adults.
*
* If the user is using damage parameters, this behavior must be used in
* conjunction with the storm damage behavior. If the user is using shading,
* then this must be used in conjunction with a light behavior.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clNCIMort : virtual public clMortalityBase, clNCIBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
   clNCIMort(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clNCIMort();

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

  /**Return codes for the "dead" tree int data member variable.  Array size
   * is number of species by number of tree types (even if not every species
   * and type is represented).*/
  short int **mp_iDeadCodes;

  float m_fNumberYearsPerTimestep; /**<Number of years per timestep. From sim
       manager*/

  char *m_cQuery; /**<Query string to get NCI trees*/

  /**
  * Makes sure all input data is valid.  The following must all be true:
  * <ul>
  * <li>Max. radius of Crowding Effects must be > 0</li>
  * <li>Max probability of survival for each species must be 0-1</li>
  * <li>X0  (Size effect mode) for each species must not = 0</li>
  * <li>Xb (Size effect variance) for each species must not = 0</li>
  * <li>Eta for each species for each damage category beyond undamaged must be
  * between 0 and 1</li>
  * <li>Storm Effect parameters for each species for each damage category beyond
  * undamaged must be between 0 and 1</li>
  * <li>Minimum neighbor DBH must not be less than 0</li>
  * </ul>
  * @throws modelErr if any of the above conditions are not met.
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
  * Gets the return codes for needed tree data members.  This declares and
  * populates the mp_iDamageCodes array with the return codes for the
  * "stm_dmg" tree int data member, and does the same for the mp_iLightCodes
  * array and the "Light" tree float data member for any species which uses
  * the shading effect (shading coefficient != 0).
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
#endif // PuertoRicoNCIMort_H
