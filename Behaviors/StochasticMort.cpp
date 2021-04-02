//---------------------------------------------------------------------------
#include "StochasticMort.h"
#include "MortalityOrg.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ModelMath.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clStochasticMort::clStochasticMort(clSimManager *p_oSimManager) :
      clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clMortalityBase(p_oSimManager)
{
 try {

   m_sNameString = "stochasticmortshell";
   m_sXMLRoot = "StochasticMortality";
   mp_fRandomMort = NULL;
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clStochasticMort::clStochasticMort";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clStochasticMort::~clStochasticMort() {
 delete[] mp_fRandomMort;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clStochasticMort::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
 try {
   clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
   DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
   doubleVal *p_fTempValues;  //for getting species-specific values
   int iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
   short int iNumSpecies = mp_oMortalityOrg->GetNumberOfSpecies(),
             i; //loop counter

   //Declare the temp array and populate it with the species to which this
   //behavior applies
   p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
   for (i = 0; i < m_iNumBehaviorSpecies; i++)
     p_fTempValues[i].code = mp_iWhatSpecies[i];

   //Declare the arrays for holding the variables
   mp_fRandomMort = new float[iNumSpecies];

   //Capture the values from the parameter file

   //Random mortality
   FillSpeciesSpecificValue(p_oElement, "mo_stochasticMortRate",
               "mo_smrVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
   //Transfer to the appropriate array buckets and compound by the number
   //of years per timestep
   for (i = 0; i < m_iNumBehaviorSpecies; i++) {
     mp_fRandomMort[p_fTempValues[i].code] = 1 - pow(1 - p_fTempValues[i].val, iNumberYearsPerTimestep);
   }

   delete[] p_fTempValues;
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clStochasticMort::DoShellSetup";
   throw(stcErr);
 }
}

///////////////////////////////////////////////////////////////////////////
// DoMort()
///////////////////////////////////////////////////////////////////////////
deadCode clStochasticMort::DoMort(clTree *p_oTree, const float &fDiam,
     const short int &iSpecies) {
  if (clModelMath::GetRand() < mp_fRandomMort[iSpecies])
    return natural;
  else
    return notdead;
}
