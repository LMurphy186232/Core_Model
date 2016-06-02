//---------------------------------------------------------------------------
#if !defined(NCIMasterMortality_H)
#define NCIMasterMortality_H
//---------------------------------------------------------------------------
#include "MortalityBase.h"
#include "NCI/NCIBehaviorBase.h"

class clTree;
class clTreePopulation;
class clPlot;

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
* Copyright 2012 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>June 28, 2013 - Created (LEM)
* <br>November 1, 2013: Added infection effect (LEM)
*/
class clNCIMasterMortality : virtual public clMortalityBase, clNCIBehaviorBase {
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
  * Does setup.
  * <ol>
  * <li>ReadParameterFile() is called to read the parameter file's data.</li>
  * <li>GetTreeMemberCodes() is called to get tree data return codes.</li>
  * <li>FormatQueryString() is called.</li>
  * </ol>
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

  /**Maximum survival value. Array sized number of species.*/
  double *mp_fMaxPotentialValue;

  /** The length of the time period of the max survival, if needed for
   * adjustment of survival rates. For instance, if the max survival is for
   * a 5-year time period, then this value is 5, and the 5th root is taken
   * of the final survival rate to arrive at the yearly value. 1 indicates
   * that the max rate is yearly already.*/
  double m_fMaxSurvivalPeriod;

  /**Return codes for the "dead" tree int data member variable.  Array size
   * is number of species by number of tree types (even if not every species
   * and type is represented).*/
  short int **mp_iDeadCodes;

  /**Total number of species - for the destructor */
  short int m_iNumTotalSpecies;

  /**For finding trees*/
  std::string m_sQuery;

  /**
  * Gets the return codes for needed tree data members.
  * @throws modelErr if a light code comes back -1 for any species which uses
  * the shading effect.
  */
  void GetTreeMemberCodes();

};
//---------------------------------------------------------------------------
#endif
