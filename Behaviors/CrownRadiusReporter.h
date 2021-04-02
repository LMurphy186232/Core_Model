//---------------------------------------------------------------------------
// CrownRadiusReporter
//---------------------------------------------------------------------------
#if !defined(CrownRadiusReporter_H)
  #define CrownRadiusReporter_H

#include "MortalityBase.h"
/**
* Crown radius reporter, version 1.0
*
* This behavior reports crown radius. It adds a float data member to the tree 
* called "Crown_Radius" and updates it each timestep. Then it can be added to
* output.
*
* This behavior can only be applied to saplings, adults, and snags.
*
* The namestring and parameter file call string for this behavior are both
* "Crown Radius Reporter".
*
* Copyright 2008 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>June 17, 2008 - Created (LEM)
*/
class clCrownRadiusReporter : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  */
  clCrownRadiusReporter(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clCrownRadiusReporter();

  /**
  * Updates crown radii. A query is sent to the tree population to get
  * all trees to which this behavior is applied.
  */
  void Action();

  /**
  * Does setup for this behavior. It formats a query string for finding
  * trees to which this behavior applies. It then finds any initial conditions
  * trees and sets their radii.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Crown Radius" float data member. The return codes are 
  * captured in the mp_iRadiusCodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * saplings and adults.
  */
  void RegisterTreeDataMembers();

  protected:

  /**Data member codes for "Crown Radius" data member. First array index is #
  * species, second is number types*/
  short int **mp_iRadiusCodes;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to report crown radius.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  std::string m_sQuery;

  int m_iNumSpecies; /**<Number of species, for destructor*/
};

//---------------------------------------------------------------------------
#endif // CrownRadiusReporter_H
