//---------------------------------------------------------------------------
// Germination
//---------------------------------------------------------------------------
#if !defined(Germination_H)
  #define Germination_H

#include "BehaviorBase.h"

class clGrid;


/**
* Germination - version 1.0
*
* This performs germination.  For a species subjected to germination, its
* number of seeds is reduced to a set proportion of the total.
*
* A fatal error is thrown if a disperse behavior is not also defined for the
* run.
*
* This class's namestring and parameter call string are both "Germination".
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clGermination : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clGermination(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clGermination();

  /**
   * Reads in values from the parameter file.
   * @param p_oDoc DOM tree of parsed input file.
   * @throws modelErr if the values in proportion germinating are not between 0
   * and 1.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Performs germination.  For each grid cell in the seed grid, the number of
   * seeds for a germinating species is multiplied by the germinating proportion
   * for that species.  A random round is used to take care of fractional parts
   * of seeds.
   */
  void Action();

protected:

  /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
  clGrid *mp_oSeedGrid;

  /**Substrate proportion.  Size is # total species.*/
  double *mp_fProportionGerminating;

  /**Data member codes for seed grid for number of seeds.  Array size is #
   * total species.*/
  short int *mp_iSeedGridCode;

};
//---------------------------------------------------------------------------
#endif // Germination_H

