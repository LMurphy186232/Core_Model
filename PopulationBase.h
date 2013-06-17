//---------------------------------------------------------------------------

#ifndef PopulationBaseH
#define PopulationBaseH
//--------------------------------------------------------------------------

#include "WorkerBase.h"

/**
 PopulationBase - Version 1.0
 This class acts as a virtual parent for all population classes.  This allows
 the Simulation Manager to work with population objects without knowing anything
 about them.

 There should not be any objects instantiated from this class.

 Copyright 2003 Charles D. Canham.
 @author Lora E. Murphy

 <br>Edit history:
 <br>-----------------
 <br>April 28, 2004 - Submitted as beta (LEM)
*/
class clPopulationBase : virtual public clWorkerBase{

  public:

  /**
   * Performs cleanup between behavior action calls. This can be overridden in
   * case a population has any cleanup or updating to do between action calls.
   */
  virtual void DoDataUpdates() {;};

  /**
   * Destructor.
   */
  virtual ~clPopulationBase() {;};

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clPopulationBase(clSimManager *p_oSimManager):clWorkerBase(p_oSimManager){;};

}; //end of clPopulationBase class

//---------------------------------------------------------------------------
#endif

