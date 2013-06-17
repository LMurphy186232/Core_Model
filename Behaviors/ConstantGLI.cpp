//---------------------------------------------------------------------------
#include "ConstantGLI.h"
#include "ParsingFunctions.h"

//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clConstantGLI::clConstantGLI(clSimManager *p_oSimManager) :
clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager), clLightBase(
    p_oSimManager) {
  m_sNameString = "constglilightshell";
  m_bNeedsCommonParameters = false;
  m_sXMLRoot = "ConstantGLI";

  m_fGLI = 0;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clConstantGLI::DoShellSetup(xercesc::DOMDocument * p_oDoc) {
  try {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

    FillSingleValue(p_oElement, "li_constGLI", &m_fGLI, true);

    if (0 > m_fGLI || 100 < m_fGLI) {
      modelErr stcErr;
      stcErr.sFunction = "clConstantGLI::DoShellSetup";
      stcErr.sMoreInfo = "Constant GLI must be between 0 and 100.";
      stcErr.iErrorCode = BAD_DATA;
      throw(stcErr);
    }
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clConstantGLI::DoShellSetup";
    throw(stcErr);
  }
}
