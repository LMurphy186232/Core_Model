//---------------------------------------------------------------------------
#include "ConstantRadialGrowth.h"
#include "Tree.h"
#include "GrowthOrg.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// clConstantRadialGrowth()
//////////////////////////////////////////////////////////////////////////////
clConstantRadialGrowth::clConstantRadialGrowth(clSimManager *p_oSimManager) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clGrowthBase( p_oSimManager), clMichMenBase(p_oSimManager)
{
  m_sNameString = "constradgrowthshell";
  m_sXMLRoot = "ConstRadialGrowth";
}

//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clConstantRadialGrowth::CalcDiameterGrowthValue(clTree *p_oTree,
    clTreePopulation *p_oPop, float fHeightGrowth) {
  float fDiam;                   //tree's diameter value

  //Get the appropriate diameter for this tree
  p_oTree->GetValue(mp_oGrowthOrg->GetDiamCode(p_oTree->GetSpecies(),
      p_oTree->GetType()),
      &fDiam);

  return mp_fAdultConstRadInc[p_oTree->GetSpecies()];
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
/////////////////////////////////////////////////////////////////////////////*/
void clConstantRadialGrowth::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("ConstRadialGrowth") == 0) {
      m_iGrowthMethod = diameter_auto;
    } else if (sNameString.compare("ConstRadialGrowth diam only") == 0){
      m_iGrowthMethod = diameter_only;
    } else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sFunction = "clConstantRadialGrowth::SetNameData";
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clConstantRadialGrowth::SetNameData";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
//////////////////////////////////////////////////////////////////////////////
void clConstantRadialGrowth::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  try {
    short int iNumSpecies = mp_oGrowthOrg->GetNumberOfSpecies();

    mp_fAdultConstRadInc = new double[iNumSpecies];

    //Read the base variables
    GetParameterFileData(p_oDoc);

  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clRelativeGrowth::GetNameData";
    throw(stcErr);
  }
}
