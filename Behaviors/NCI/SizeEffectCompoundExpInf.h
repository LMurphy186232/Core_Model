#ifndef SIZEEFFECTCOMPOUNDEXPINF_H_
#define SIZEEFFECTCOMPOUNDEXPINF_H_

#include "SizeEffectBase.h"

/**
 * Calculates the size effect with a compound exponential function. This uses
 * two different sets of parameters, depending on whether a tree is infected
 * or not.
 *
 * Size Effect is calculated as:
 * <center><i>SE = (1-a*exp(b*(diam/100)))*exp(c*((diam/100)<sup>d</sup>))</i></center>
 *
 * where <i>diam</i> is the diameter of the target tree, in cm (d10 for
 * seedlings, DBH for everyone else); and <i>a, b, c,</i> and <i>d</i> are
 * parameters.
 *
 * Insect infestation is determined by the behavior of class
 * clInsectInfestation or clDensDepInfestation. This class expects to find a
 * "YearsInfested" int data member to be present for all trees to which this
 * behavior is applied.
 */
class clSizeEffectCompoundExpInf: virtual public clSizeEffectBase {
public:

  /**
   * Constructor.
   */
  clSizeEffectCompoundExpInf();

  /**
   * Destructor.
   */
  ~clSizeEffectCompoundExpInf();

  /**
   * Calculates size effect. This is bounded between 0 and 1.
   * @param p_oTree Tree for which to calculate size effect.
   * @param fDiam Diameter of tree. Diameter at 10 cm for seedlings, DBH for
   * all other types.
   */
  float CalculateSizeEffect(clTree *p_oTree, const float &fDiam);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /** Codes for "YearsInfested" data member. Array size is number of species by
   * number of types.*/
  short int **mp_iDataCodes;

  /**a in Size Effect equation above for uninfested trees. Array is sized number
   * of species.*/
  double *mp_fA;

  /**b in Size Effect equation above for uninfested trees. Array is sized number
   * of species.*/
  double *mp_fB;

  /**c in Size Effect equation above for uninfested trees. Array is sized number
   * of species.*/
  double *mp_fC;

  /**d in Size Effect equation above for uninfested trees. Array is sized number
   *  of species.*/
  double *mp_fD;

  /**a in Size Effect equation above for infested trees. Array is sized number
   * of species.*/
  double *mp_fAInf;

  /**b in Size Effect equation above for infested trees. Array is sized number
   * of species.*/
  double *mp_fBInf;

  /**c in Size Effect equation above for infested trees. Array is sized number
   * of species.*/
  double *mp_fCInf;

  /**d in Size Effect equation above for infested trees. Array is sized number
   * of species.*/
  double *mp_fDInf;

  /** Number of species. For destructor.*/
  short int m_iNumSpecies;
};

#endif /* DEFAULTSIZEEFFECT_H_ */
