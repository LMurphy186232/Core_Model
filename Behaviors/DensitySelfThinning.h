//---------------------------------------------------------------------------

#ifndef DensitySelfThinning
#define DensitySelfThinning
//---------------------------------------------------------------------------
#include "MortalityBase.h"
/**
* Density Self Thinning - Version 1.0
*
* This behavior evaluates seedling and sapling mortality according to the
* density and mean diam10 of a tree's neigborhood.
*
* This class's namestring is "densityselfthinningmortshell".
*
* This class's parameter file call string is "DensitySelfThinning".
*
* Note (LEM): this behavior requires a 1-year timestep.  There is no
* programmatic reason for this; this is what Rasmus wanted, as author, because
* he believes using this behavior with a multi-year timestep is not smart
* scientifically.
*
* Copyright 2011 Charles D. Canham.
* @authors Rasmus Astrup, Marissa LeBlanc
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clDensitySelfThinning : virtual public clMortalityBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clDensitySelfThinning(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clDensitySelfThinning();

  /**
  * Reads in values from the parameter file and makes sure all data needed is
  * collected.
  *
  * @param p_oDoc Parsed DOM tree of parameter file.
  * @throw modelErr if:
  * <ul>
  * <li>The timestep length is greater than 1.</li>
  * <li>The neighborhood radius is not greater than 0.</li>
  * <li>The minimum density for mortality is less than 0.</li>
  * <li>The self-thinning asymptote is not greater than 0.</li>
  * <li>The density effect is not greater than 0.</li>
  * </ul>
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates the number of neighborhood trees and their mean Diam10.
  * @param p_oTree Tree being evaluated.
  * @param p_fTreeCount Variable into which to place the number of neighborhood
  * trees.
  * @param p_fMeanDiam10 Variable into which to place the mean diam10 of the
  * neighborhood trees.
  */
  void CalculateNeighborhoodTreeCountAndMeanDiam10( clTree * p_oTree, float *p_fTreeCount, float *p_fMeanDiam10);

  /**
  * Calculates mortality according to the DensitySelfThinning equation.
  * @param p_oTree Tree being evaluated.
  * @param fDbh DBH of tree being evaluated.
  * @param iSpecies Species of the tree being evaluated.
  * @return natural if the tree is to die, notdead if it lives.
  */
  deadCode DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies);

  protected:
   float m_fNumberYearsPerTimestep; /**<Number of years per timestep*/
   float * mp_fSelfThinRadius; /**<Radius that defines the neighborhood size (meters)*/
   float * mp_fMinDensityForMort; /**<Minimum neighborhood density subject to mortality (trees/ha)*/
   float * mp_fSelfThinAsymptote; /**<Asymptote parameter*/
   float * mp_fSelfThinDiamEffect; /**<Diameter effect parameter*/
   float * mp_fSelfThinDensityEffect; /**<Density effect parameter*/
   short int * mp_iIndexes; /**<Speeds access to the arrays*/
};
//---------------------------------------------------------------------------
#endif //DensitySelfThinning

