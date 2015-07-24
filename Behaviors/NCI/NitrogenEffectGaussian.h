#ifndef GAUSSIANNITROGENEFFECT_H_
#define GAUSSIANNITROGENEFFECT_H_

#include "NitrogenEffectBase.h"

/**
 * This returns the nitrogen effect using a gaussian function. The function
 * is:
 * <br>
 * @htmlonly
   Nitrogen Effect = exp(-0.5*((NDEP - X0) / Xb)<sup>2</sup>)
   @endhtmlonly
 * where:
 * <ul>
 * <li>NDEP is the annual nitrogen deposition, from the plot object</li>
 * <li>X0 and Xb are parameters</li>
 * </ul>
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>June 27, 2013 - Created (LEM)
 */
class clNitrogenEffectGaussian: public virtual clNitrogenEffectBase {
public:

  /**
   * Constructor.
   */
  clNitrogenEffectGaussian();

  /**
   * Destructor.
   */
  ~clNitrogenEffectGaussian();

  /**
   * Calculates nitrogen effect for a particular species.
   * @param p_oPlot Plot object for querying for nitrogen values.
   * @param iSpecies Species for which to calculate nitrogen effect.
   */
  float CalculateNitrogenEffect(clPlot *p_oPlot, int iSpecies);

  /**
   * Does setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement);

protected:

  /**Nitrogen effect X0. Array is sized number of species.*/
  float *mp_fX0;

  /**Nitrogen effect Xb. Array is sized number of species.*/
  float *mp_fXb;

};

#endif /* GAUSSIANNITROGENEFFECT_H_ */
