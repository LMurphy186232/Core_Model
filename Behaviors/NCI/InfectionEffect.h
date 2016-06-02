#ifndef INFECTIONEFFECT_H_
#define INFECTIONEFFECT_H_

#include "InfectionEffectBase.h"

/**
 * This returns the infection effect using the function:
 * <br>
 * @htmlonly
   Infection Effect = a * ln(T) + b
   @endhtmlonly
 * where:
 * <ul>
 * <li>T is the time of infestation, as recorded in the "YearsInfested" tree
 * data member</li>
 * <li>a and b are parameters</li>
 * </ul>
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>November 1, 2013 - Created (LEM)
 */
class clInfectionEffect: public virtual clInfectionEffectBase {
public:

  /**
   * Constructor.
   */
  clInfectionEffect();

  /**
   * Destructor.
   */
  ~clInfectionEffect();

  /**
   * Calculates infection effect for a particular species.
   * @param p_oTree Tree for which to calculate infection effect.
   */
  double CalculateInfectionEffect(clTree *p_oTree);

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

  /**Infection effect a. Array is sized number of species.*/
  double *mp_fA;

  /**Infection effect b. Array is sized number of species.*/
  double *mp_fB;

  /** Total number of species. Primarily for the destructor. */
  int m_iTotalNumSpecies;

};

#endif /* INFECTIONEFFECT_H_ */
