//---------------------------------------------------------------------------

#ifndef RandomSeedLoggerH
#define RandomSeedLoggerH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"
/**
  This is an automated testing behavior.  It will log the actual random seed
  of a run in a file called "RandomSeed.txt".  This is useful if you want to
  be able to go back and replicate a run exactly but 0 was specified in the
  parameter file as the random seed.

  Copyright 2003 Charles D. Canham.
  @author Lora E. Murphy

  <br>Edit history:
  <br>-----------------
  <br>April 28, 2004 - Submitted as beta (LEM)
*/

class clRandomSeedLogger : public clBehaviorBase {
  public:
  /**
  * Does nothing.
  */
  void Action(){;};

  /**
  * Constructor.
  * @param p_oSimManager Sim manager object.
  */
  clRandomSeedLogger(clSimManager *p_oSimManager);

  protected:
  /**
  * Writes the log with the random seed value.
  * @param p_oDoc Parsed parameter file object.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);
};
//---------------------------------------------------------------------------
#endif
