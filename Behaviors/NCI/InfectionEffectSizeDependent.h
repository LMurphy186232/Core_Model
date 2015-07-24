#ifndef INFECTIONEFFECTSIZEDEPENDENT_H_
#define INFECTIONEFFECTSIZEDEPENDENT_H_

#include "InfectionEffectBase.h"

/**
 * This returns the infection effect using the function:
 * <br>
 * @htmlonly
   Infection Effect = [a * ln(T) + b] * exp(-0.5*[( ((DBH-Xp)/X0) / Xb)^2]
   @endhtmlonly
 * where:
 * <ul>
 * <li>T is the time of infestation, as recorded in the "YearsInfested" tree
 * data member</li>
 * <li>DBH is the tree's DBH, or diam10 if a seedling</li>
 * <li>a, b, X0, Xb, and Xp are parameters</li>
 * </ul>
 *
 * Copyright 2014 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>June 9, 201 - Created (LEM)
 */
class clInfectionEffectSizeDependent: public virtual clInfectionEffectBase {
public:

  /**
   * Constructor.
   */
  clInfectionEffectSizeDependent();

  /**
   * Destructor.
   */
  ~clInfectionEffectSizeDependent();

  /**
   * Calculates infection effect for a particular species.
   * @param p_oTree Tree for which to calculate infection effect.
   */
  float CalculateInfectionEffect(clTree *p_oTree);

  /**
   * Does setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if there is a tree to which this behavior applies
   * that does not have a "YearsInfested" code.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Return codes for the "YearsInfested" int tree data member.  Array index
   * one is sized m_iTotalNumSpecies; array index two is sized number of total
   * types.*/
  short int **mp_iYearsInfestedCodes;

   /**Return codes for diameter.  Array index one is sized m_iTotalNumSpecies;
    * array index two is sized number of total types.*/
  short int **mp_iDiamCodes;

  /**Infection effect a. Array is sized number of species.*/
  float *mp_fA;

  /**Infection effect b. Array is sized number of species.*/
  float *mp_fB;

  /**Infection effect X0. Array is sized number of species.*/
  float *mp_fX0;

  /**Infection effect Xb. Array is sized number of species.*/
  float *mp_fXb;

  /**Infection effect Xp. Array is sized number of species.*/
  float *mp_fXp;

  /** Total number of species. Primarily for the destructor. */
  int m_iTotalNumSpecies;

};

#endif /* INFECTIONEFFECTSIZEDEPENDENT_H_ */
