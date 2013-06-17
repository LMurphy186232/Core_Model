//---------------------------------------------------------------------------
// PRSemiStochGrowth
//---------------------------------------------------------------------------
#if !defined(PRSemiStochGrowth_H)
  #define PRSemiStochGrowth_H

#include "GrowthBase.h"

/**
* Implements diameter growth according to a completely whacked-out scheme for
* Puerto Rico.  This was meant for palm growth, and palms are freaky.  Not
* nice trees at all.  This is merely for explanation for the insanity of this
* particular growth behavior.  Of course it's no excuse but then again I don't
* make this stuff up.
*
* Trees of less than a certain height get what is essentially an allometry
* shortcut:  diameter is incremented in whatever direction is necessary to get
* it to match the outcome of an equation involving height.  Trees above the
* height get a random diameter drawn from a normal distribution with a mean
* and standard deviation supplied by the user.
*
* This can only increment diameter along with a HeightIncrementer - i.e. no
* diam-with-auto-height use.
*
* For trees below the cutoff, their growth is figured as follows:
* <center>Y = (A * EXP (-B*Height)) - Diam</center>
* where
* <ul>
* <li>Y = diameter growth for the timestep, in cm</li>
* <li>a = parameter</li>
* <li>b = parameter</li>
* <li>Height = tree height in cm</li>
* <li>diam = diameter of the tree at which to apply growth (before growth),
* in cm</li>
* </ul>
*
* The name string is "prsemistochgrowthshell".  The parameter file call string
* is "PRSemiStochastic diam only".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clPRSemiStochGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim manager for this run.
  */
  clPRSemiStochGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clPRSemiStochGrowth();

  /**
  * Calculates the amount of diameter growth increase for a particular tree.
  * If the tree is less than the height cutoff for stochastic growth, the
  * equation above is calculated.  Otherwise, this returns the difference
  * between the existing diameter and a randomly drawn diameter.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fHeightGrowth Amount of height growth, in m.
  * @return Amount, in cm, by which to increase the tree's diameter.
  */
  float CalcDiameterGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fHeightGrowth);

  /**
  * Does the setup for this behavior.  This reads in the parameters from the
  * parameter file.
  * @param p_oDoc Parsed parameter file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /**Height threshold between deterministic and stochastic growth - in m -
  * sized number of behavior species*/
  float *mp_fHeightThreshold;

  /**"a" parameter for deterministic growth - sized number of behavior
  * species*/
  float *mp_fA;

  /**"b" parameter for deterministic growth - sized number of behavior
  * species*/
  float *mp_fB;

  /**Mean diameter (cm) for stochastic growth - sized number of behavior
  * species*/
  float *mp_fMeanDiam;

  /**Diameter standard deviation (cm) for stochastic growth - sized number of
  * behavior species*/
  float *mp_fDiamStdDev;

  /**For accessing the other arrays*/
  short int *mp_iIndexes;
};
//---------------------------------------------------------------------------
#endif // PRSemiStochGrowth_H
