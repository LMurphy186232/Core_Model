#ifndef SIZEEFFECTSHIFTEDLOGINF_H_
#define SIZEEFFECTSHIFTEDLOGINF_H_

#include "SizeEffectBase.h"

/**
 * Calculates a size effect based on a shifted lognormal function. This uses
 * two different sets of parameters, depending on whether a tree is infected
 * or not.
 *
 * Size Effect is calculated as:
 * <center><i>SE = exp(-0.5(ln((diam+X<sub>p</sub>)/X<sub>0</sub>)/X<sub>b</sub>)<sup>2</sup>)</i></center>
 *
 * where:
 * <ul>
 * <li><i>diam</i> is the diameter of the target tree, in cm (d10 for seedlings, DBH for everyone else)</li>
 * <li><i>X<sub>0</sub></i> is the size effect mode, in cm</li>
 * <li><i>X<sub>b</sub></i> is the size effect variance, in cm</li>
 * <li><i>X<sub>p</sub></i> is the size effect shift, in cm</li>
 * </ul>
 *
 * Insect infestation is determined by the behavior of class
 * clInsectInfestation or clDensDepInfestation. This class expects to find a
 * "YearsInfested" int data member to be present for all trees to which this
 * behavior is applied.
 */
class clSizeEffectShiftedLogInf: virtual public clSizeEffectBase {
public:

  /**
   * Constructor.
   */
  clSizeEffectShiftedLogInf();

  /**
   * Destructor.
   */
  ~clSizeEffectShiftedLogInf();

  /**
   * Calculates size effect. This is bounded between 0 and 1.
   @param p_oTree Tree for which to calculate size effect.
   * @param fDiam Diameter of tree. Diameter at 10 cm for seedlings, DBH for
   * all other types.
   */
  double CalculateSizeEffect(clTree *p_oTree, const float &fDiam);

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   * @throws ModelException if X0 is < 0 or Xb = 0.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /** Codes for "YearsInfested" data member. Array size is number of species by
   * number of types.*/
  short int **mp_iDataCodes;

  /**Size effect variance parameter for uninfested trees. X<sub>b</sub> in Size
   * Effect equation above. Array is sized number of species.*/
  double *mp_fXb;

  /**Size effect mode parameter for uninfested trees. X<sub>0</sub> in Size
   * Effect equation above. Array is sized number of species.*/
  double *mp_fX0;

  /**Size effect shift parameter for uninfested trees. X<sub>p</sub> in Size
   * Effect equation above. Array is sized number of species.*/
  double *mp_fXp;

  /**Size effect variance parameter for infested trees. X<sub>b</sub> in Size
   * Effect equation above. Array is sized number of species.*/
  double *mp_fXbInf;

  /**Size effect mode parameter for infested trees. X<sub>0</sub> in Size
   * Effect equation above. Array is sized number of species.*/
  double *mp_fX0Inf;

  /**Size effect shift parameter for infested trees. X<sub>p</sub> in Size
   * Effect equation above. Array is sized number of species.*/
  double *mp_fXpInf;

  /**Size effect minimum diameter. Array is sized number of species.*/
  double *mp_fMinDiam;

  /** Number of species. For destructor.*/
  short int m_iNumSpecies;
};

#endif /* SIZEEFFECTSHIFTEDLOGINF_H_ */
