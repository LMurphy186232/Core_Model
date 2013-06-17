//---------------------------------------------------------------------------
// MichMenNegGrowth
//---------------------------------------------------------------------------
#if !defined(MichMenNegGrowth_H)
  #define MichMenNegGrowth_H

#include "GrowthBase.h"


/**
* Increments height growth according to an equation originally developed for
* New Zealand seedling growth. This behavior can only be used to create a
* height growth increment.
*
* The equation used in this behavior is:
* @htmlonly
 <center>Y = (alpha;GLI) / (GLI + &alpha;/&beta;) * H<sup>&phi;</sup> - &gamma; </center>
  @endhtmlonly
* where
* <ul>
* <li>Y = height growth in cm/year</li>
* <li>GLI = light level</li>
* <li>@htmlonly &alpha;, &beta;, &phi;, and &gamma; @endhtmlonly = parameters</li>
* <li>H = tree's height in cm</li>
* </ul>
*
* This is looped over the number of years per timestep, allowing H to increment
* at each intermediate year.
*
* Growth has a stochastic component. The user provides a standard deviation for
* a normal distribution of mean zero and a draw on this distribution is added
* to the growth for each year. Growth can also be autocorrelated. Each species
* has a probability of autocorrelation from year to year and if a random draw
* triggers autocorrelation, the next year gets the same random additional growth
* factor as the previous year. This may mean multiple random draws per timestep
* in a multiyear timestep.
*
* All trees must have the "Light" data member registered (i.e. must have a light
* behavior applied).
*
* The name string is "michmenneggrowthshell".  The parameter file call string
* is "MichaelisMentenNegativeGrowth height only". This creates a growth data
* member called "autocorr".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clMichMenNegGrowth : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clMichMenNegGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clMichMenNegGrowth();

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

  /**
  * Registers the float data member  and captures the return codes.
  */
  void RegisterTreeDataMembers();

  protected:

  /** alpha - sized number of behavior species*/
  float *mp_fAlpha;

  /** beta - sized number of behavior species*/
  float *mp_fBeta;

  /** phi - sized number of behavior species*/
  float *mp_fPhi;

  /** gamma - sized number of behavior species*/
  float *mp_fGamma;

  /** standard deviation of growth stochasticity in cm/year*/
  float *mp_fStdDev;

  /** one year probability of autocorrelation */
  float *mp_fProbAutoCorr;

  /**To help access the other arrays*/
  int *mp_iIndexes;

  /**
  * Return codes for the "autocorr" tree float data member variable.  Array of
  * behavior species by type (even if not every type is represented).
  */
  short int **mp_iAutoCorrCodes;

  /**Conversion factor to translate the results of the function to the
  * appropriate units per timestep*/
  float m_fYearsPerTimestep;
};
//---------------------------------------------------------------------------
#endif // MichMenNegGrowth_H
