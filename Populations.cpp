//---------------------------------------------------------------------------
#include "Populations.h"

//HEADER FILES FOR REGISTERED POPULATION OBJECTS
#include "TreePopulation.h"
#include "GhostTreePopulation.h"

////////////////////////////////////////////////////////////////////////////
// CreateObjects
////////////////////////////////////////////////////////////////////////////
void clPopulationManager::CreateObjects(xercesc::DOMDocument *p_oDoc){
  try {
    //If keep current is false - free memory
    if (m_iNumObjects > 0) FreeMemory();

    if (0 == m_iNumObjects) {
      m_iNumObjects = 2;
      clTreePopulation *p_oTreePop = new clTreePopulation(mp_oSimManager);
      clGhostTreePopulation *p_oGhostPop = new clGhostTreePopulation(mp_oSimManager);
      mp_oObjectArray = new clWorkerBase*[m_iNumObjects];
      mp_oObjectArray[0] = p_oTreePop;
      mp_oObjectArray[1] = p_oGhostPop;
      p_oTreePop->BarebonesDataSetup(p_oDoc);
    } //end of if (0 == m_iNumObjects)
  }
  catch(modelErr &err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch(...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPopulationManager::CreateObjects";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
