//---------------------------------------------------------------------------

#ifndef PopulationsH
#define PopulationsH

#include "ObjectManagerBase.h"

/**
 * Populations - Version 1.1
 * Population object manager.
 *
 * Copyright 2003 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>April 28, 2004 - Submitted as beta (LEM)
 * <br>January 6, 2011 - Added ghost tree population (LEM)
*/
class clPopulationManager : public clObjectManagerBase {
  public:

 /**
  * This constructor structure makes sure that there's no default constructor
  * while also saying that this child class doesn't need its own constructor
  * to do anything
  */
  clPopulationManager(clSimManager *p_oSimManager) :
                            clObjectManagerBase(p_oSimManager){;};
  //~clPopulationManager(); No destructor needed


 /**
  * Creates the tree population objects.
  */
  void CreateObjects(xercesc::DOMDocument *p_oDoc);
};
//---------------------------------------------------------------------------
#endif
