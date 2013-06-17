//---------------------------------------------------------------------------
// TreeAgeCalculator
//---------------------------------------------------------------------------
#if !defined(TreeAgeCalculator_H)
  #define TreeAgeCalculator_H

#include "MortalityBase.h"
/**
* Tree age calculator, version 1.0
*
* This behavior tracks tree age.  It adds an int data member to the tree called
* "Tree Age", and increments it each timestep with the number of years per
* timestep.  Thus "Tree Age" holds the tree's age, in years.
*
* In order to distinguish initial conditions trees, they are all set to
* ages of 10000.
*
* This behavior should execute towards the end of a run, after establishment.
* This way seedlings don't have an age of 0 at the end of the timestep.
*
* This behavior can only be applied to seedlings, saplings, and adults, since
* the meaning of "age" for snags is debatable.
*
* The namestring and parameter file call string for this behavior are both
* "TreeAgeCalculator".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clTreeAgeCalculator : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  */
  clTreeAgeCalculator(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clTreeAgeCalculator();

  /**
  * Updates ages.  A query is sent to the tree population to get
  * all trees to which this behavior is applied.
  */
  void Action();

  /**
  * Does setup for this behavior.  It formats a query string for finding
  * trees to which this behavior applies.  It then finds any initial conditions
  * trees, and if they do not have an age specified, sets their age to 10000.
  * @param p_oDoc DOM tree of parsed input file.
  *
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Tree Age" int data member.  The return codes are captured
  * in the mp_iAgeCodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * seedlings, saplings, and adults.
  */
  void RegisterTreeDataMembers();

  protected:

  /**Data member codes for "Tree Age" data member. First array index is #
  * species, second is number types*/
  short int **mp_iAgeCodes;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate tree age.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  char *m_cQuery;

  int m_iNumSpecies; /**<Number of species, for destructor*/
};

//---------------------------------------------------------------------------
#endif // TreeAgeCalculator_H
