//---------------------------------------------------------------------------
// MichMenPhotoinhibition
//---------------------------------------------------------------------------
#if !defined(MichMenPhotoinhibition_H)
  #define MichMenPhotoinhibition_H

#include "GrowthBase.h"


/**
* Increments height growth according to an equation originally developed for
* New Zealand seedling growth. This behavior can only be used to create a
* height growth increment.
*
* The equation used in this behavior is:
* @htmlonly
 <center>Y = ((&alpha;/(1 + &alpha;/(&beta;*GLI)) - d*GLI) * H<sup>&phi;</sup> </center>
  @endhtmlonly
* where
* <ul>
* <li>Y = height growth in cm/year</li>
* <li>GLI = light level</li>
* <li>@htmlonly &alpha;, &beta;, &phi;, and d @endhtmlonly = parameters</li>
* <li>H = tree's height in m</li>
* </ul>
*
* This is looped over the number of years per timestep, allowing H to increment
* at each intermediate year.
*
* All trees must have the "Light" data member registered (i.e. must have a light
* behavior applied).
*
* The name string is "michmenphotogrowthshell".  The parameter file call
* string is "MichaelisMentenPhotoinhibitionGrowth height only".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clMichMenPhotoinhibition : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clMichMenPhotoinhibition(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clMichMenPhotoinhibition();

  /**
  * Calculates the amount of height growth increase for a particular tree
  * using the growth equation described ablove.
  *
  * @param p_oTree Tree for which to calculate growth.
  * @param p_oPop Tree population object, just in case it's needed.
  * @param fDiameterGrowth Diameter growth, in cm.
  * @return Amount, in m, by which to increase the tree's height.
  */
  float CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth);

  /**
  * Does setup.  Reads in values from the parameter file, and validates that
  * all species/type combos use light (each must have "Light" registered).
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if any species/type combo to which this behavior is
  * applied does not have a light behavior, or if beta = 0.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /** alpha - sized number of behavior species*/
  float *mp_fAlpha;

  /** beta - sized number of behavior species*/
  float *mp_fBeta;

  /** phi - sized number of behavior species*/
  float *mp_fPhi;

  /** d - sized number of behavior species*/
  float *mp_fD;

  /**To help access the other arrays*/
  int *mp_iIndexes;

  /**Conversion factor to translate the results of the function to the
  * appropriate units per timestep*/
  float m_fYearsPerTimestep;
};
//---------------------------------------------------------------------------
#endif // MichMenPhotoinhibition_H
