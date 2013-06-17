//---------------------------------------------------------------------------

#ifndef CompetitionMort
#define CompetitionMort
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
 * Competition-Dependent Mortality - Version 1.0
 *
 * This evaluates mortality according to Relative Increment (Actual Diameter
 * Growth(i.e. Growth)/Potential Diameter Growth).
 * The probability of mortality is a function of Relative Increment.
 *
 * Competition-Dependent Mortality can only be used in conjunction with NCI
 * Growth.
 *
 * This class's namestring is "competitionmortshell".  The parameter file call
 * string is "CompetitionMortality".
 *
 * Copyright 2011 Charles D. Canham.
 * @authors Rasmus Astrup, Marissa LeBlanc and Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */
class clCompetitionMort : virtual public clMortalityBase {
  //note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
   * Constructor.  Sets the namestring.
   */
  clCompetitionMort(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clCompetitionMort();

  /**
   * Reads in values from the parameter file and makes sure all data needed is
   * collected.
   * @param p_oDoc Parsed DOM tree of parameter file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Calculates mortality according to the competition mortality equation.
   * @param p_oTree Tree being evaluated.
   * @param fDbh DBH of tree being evaluated.
   * @param iSpecies Species of the tree being evaluated.
   * @return natural if the tree is to die, notdead if it lives.
   */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

protected:

  /**Data member codes for "Growth" member - species by type*/
  short int **mp_iGrowthCodes;

  /**Shape parameter for competition-dependent mortality function. Array sized
   * number of species to which this behavior applies.*/
  float *mp_fCompMort;

  /**Parameter for competition-dependent morality function that determines the
   * maximum Relative Increment subject to mortality. Array sized number of
   * species to which this behavior applies.*/
  float *mp_fCompMortMax;

  /**Number of years per timestep*/
  float m_fNumberYearsPerTimestep;

  //Parameters to calculate potential growth as in NCI growth
  /**Size effect variance parameter (Xb). Array sized number of species to
   * which this behavior applies.*/
  float *mp_fXb;
  /**Maximum potential growth in cm/yr. Array sized number of species to which
   * this behavior applies.*/
  float *mp_fMaxPotentialGrowth;
  /**Size effect mode parameter (X0). Array sized number of species to which
   * this behavior applies.*/
  float *mp_fX0;
  /**Speeds access to the arrays.*/
  short int *mp_iIndexes;

  /**
   * Queries for the return codes of the "Growth" float data member of a tree.
   * This data member should have been registered by the growth behavior.
   * Return codes are captured in the mp_iGrowthCodes array.
   * @throws modelErr if there is no code for any species/type combo which uses
   * this behavior.
   */
  void GetGrowthVariableCodes();
};
//---------------------------------------------------------------------------
#endif

