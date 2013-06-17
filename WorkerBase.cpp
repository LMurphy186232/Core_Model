//---------------------------------------------------------------------------
#include <stdio.h>
#include <sstream>
#include "WorkerBase.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
//---------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clWorkerBase::clWorkerBase(clSimManager *p_oSimManager) {
  try {
    mp_oSimManager = p_oSimManager;
    mp_iAllowedFileTypes = NULL;
    m_iNumAllowedTypes = 0;

    //namestring - defaults to empty string
    m_sNameString = "";
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clWorkerBase::clWorkerBase";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clWorkerBase::~clWorkerBase() {
  delete[] mp_iAllowedFileTypes;
}


//////////////////////////////////////////////////////////////////////////////
// AssembleFileCode
//////////////////////////////////////////////////////////////////////////////
void clWorkerBase::AssembleFileCode(int iFileType, int iFileVersion, char *cCode) {
  int iMajorModelVersion, iMinorModelVersion; //model major and minor versions

  //Make sure the sim manager pointer isn't NULL - if it is return error
  if (NULL == mp_oSimManager) {
    modelErr stcErr;
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    stcErr.sFunction = "clWorkerBase::AssembleFileCode";
    stcErr.sMoreInfo = "Pointer to Simulation Manager is NULL";
    throw(stcErr);
  }

  //Query the sim manager for the model versions
  iMajorModelVersion = mp_oSimManager->GetMajorVersion();
  iMinorModelVersion = mp_oSimManager->GetMinorVersion();

  //Make sure the codes are all two-digit
  sprintf(cCode, "%02d%02d%02d%02d", iMajorModelVersion, iMinorModelVersion,
     iFileType, iFileVersion);

}


////////////////////////////////////////////////////////////////////////////
// DoObjectSetup()
////////////////////////////////////////////////////////////////////////////
void clWorkerBase::DoObjectSetup(DOMDocument *p_oDoc, fileType iFileType) {
  try {
    bool bOKFile = false;    //flag for whether this is an OK file type

    //Make sure the pointer is not NULL - if it is, throw error
    if (NULL == p_oDoc) {
      modelErr stcErr;
      stcErr.sFunction = "clWorkerBase::DoObjectSetup";
      stcErr.iErrorCode = BAD_FILE;
      throw(stcErr);
    }

    //Make sure that the file is one of the allowed types for this object
  // iFileType = mp_oSimManager->GetFileType(p_oDoc);

    for (int i = 0; i < m_iNumAllowedTypes; i++) {
      if (iFileType == mp_iAllowedFileTypes[i]) {
        bOKFile = true;
        break;
      }
    }
    if (!bOKFile) {
      modelErr stcErr;
      stcErr.sFunction = "clWorkerBase::DoObjectSetup";
      stcErr.iErrorCode = BAD_FILE_TYPE;
      std::stringstream s;
      s << iFileType;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Call GetData for the object
    GetData(p_oDoc);
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clWorkerBase::DoObjectSetup";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
