//---------------------------------------------------------------------------

#ifndef ConstantGLIH
#define ConstantGLIH
//---------------------------------------------------------------------------
#include "LightBase.h"
/**
* Constant GLI Light - Version 1.0
*
* This behavior assigns a constant GLI value to all trees to which it has been
* assigned.
*
* The namestring for this behavior is "constglilightshell".  The parameter file
* call string is "ConstantGLI".
*
* Copyright 2006 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clConstantGLI : public clLightBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clConstantGLI(clSimManager *p_oSimManager);

  //~clConstantGLI(); //use default destructor

  /**
  * Gets the GLI for a tree.
  *
  * @param p_oTree Tree for which to get GLI.
  * @param p_oPop Tree population object.
  * @return User-defined GLI value.
  */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop)
    {return m_fGLI;};

  /**
  * Reads the parameter file for the GLI value.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if the GLI parameter is not between 0 and 100.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  protected:

  /** GLI value. */
  float m_fGLI;

};
//---------------------------------------------------------------------------
#endif
