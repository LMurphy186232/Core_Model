//---------------------------------------------------------------------------

#ifndef DensitySelfThinningGompertz
#define DensitySelfThinningGompertz
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
* Gompertz Density Self Thinning - Version 1.0
*
* This behavior evaluates mortality according to the density of conspecific
* trees in the neigborhood. The function is:
*
* pm = g * exp(-exp(h - i * Den))
*
* where pm is probability of mortality, and Den is neighborhood tree density
* in stems/m2. The target tree is deliberately counted.
*
* This class's namestring is "densitygompertzmortshell".
*
* This class's parameter file call string is "GompertzDensitySelfThinning".
*
* Copyright 2011 Charles D. Canham.
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clDensitySelfThinningGompertz : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clDensitySelfThinningGompertz(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clDensitySelfThinningGompertz();

  /**
  * Reads in values from the parameter file and makes sure all data needed is
  * collected.
  *
  * @param p_oDoc Parsed DOM tree of parameter file.
  * @throw modelErr if the neighborhood radius is not greater than 0.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates the number of conspecific neighborhood trees.
  * @param p_oTree Tree being evaluated.
  * @return fTreeCount Variable into which to place the number of neighborhood
  * trees.
  */
  float CalculateNeighborhoodTreeCount( clTree * p_oTree);

  /**
  * Calculates mortality according to the Gompertz equation.
  * @param p_oTree Tree being evaluated.
  * @param fDbh DBH of tree being evaluated.
  * @param iSpecies Species of the tree being evaluated.
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:
   float * mp_fG; /**<G parameter*/
   float * mp_fH; /**<H parameter*/
   float * mp_fI; /**<I parameter*/
   float * mp_fMinHeight; /**<Minimum height for neighborhood trees*/
   short int * mp_iIndexes; /**<Speeds access to the arrays*/
   float m_fRadius; /**<Radius that defines the neighborhood size (meters)*/
   float m_fNumberYearsPerTimestep; /**<Number of years per timestep*/
};
//---------------------------------------------------------------------------
#endif //DensitySelfThinningGompertz

