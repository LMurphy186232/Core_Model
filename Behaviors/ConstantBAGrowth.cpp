//---------------------------------------------------------------------------
#include "ConstantBAGrowth.h"
#include "Tree.h"
#include "GrowthOrg.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// clConstantBAGrowth()
//////////////////////////////////////////////////////////////////////////////
clConstantBAGrowth::clConstantBAGrowth(clSimManager *p_oSimManager) :
clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager), clGrowthBase(p_oSimManager),
clMichMenBase(p_oSimManager) {
  m_sNameString = "constbagrowthshell";
  m_sXMLRoot = "ConstBAGrowth";
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clConstantBAGrowth::CalcDiameterGrowthValue(clTree *p_oTree,
    clTreePopulation *p_oPop, float fHeightGrowth) {
  float fDiam;                   //tree's diameter value

  //Get the appropriate diameter for this tree
  p_oTree->GetValue(mp_oGrowthOrg->GetDiamCode(p_oTree->GetSpecies(), p_oTree->GetType()), &fDiam);

  return mp_fAdultConstBAInc[p_oTree->GetSpecies()] / fDiam;
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clConstantBAGrowth::SetNameData(std::string sNameString) {

  //Check the string passed and set the flags accordingly
  if (sNameString.compare("ConstBAGrowth") == 0) {
    m_iGrowthMethod = diameter_auto;
  } else if (sNameString.compare("ConstBAGrowth diam only") == 0){
    m_iGrowthMethod = diameter_only;
  } else {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Unrecognized behavior name \"" << sNameString << "\".";
    stcErr.sFunction = "clConstantBAGrowth::SetNameData";
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
//////////////////////////////////////////////////////////////////////////////
void clConstantBAGrowth::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  short int iNumSpecies = mp_oGrowthOrg->GetNumberOfSpecies();

  mp_fAdultConstBAInc = new double[iNumSpecies];

  //Read the base variables
  GetParameterFileData(p_oDoc);
}
