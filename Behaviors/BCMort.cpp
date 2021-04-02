//---------------------------------------------------------------------------
#include "BCMort.h"
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
clBCMort::clBCMort(clSimManager *p_oSimManager) :
      clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
      clMortalityBase(p_oSimManager) {
 try {

   m_sNameString = "bcmortshell";
   m_sXMLRoot = "BCMortality";

   mp_fMortAtZeroGrowth = NULL;
   mp_fLightDepMort = NULL;
   mp_iGrowthCodes = NULL;

   m_iNumberYearsPerTimestep = 0;

 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clBCMort::clBCMort";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clBCMort::~clBCMort() {
 int i; //loop counter

 if (mp_iGrowthCodes) {
   for (i = 0; i < m_iNumTotalSpecies; i++) {
     delete[] mp_iGrowthCodes[i];
   }
 }

 delete[] mp_iGrowthCodes;

 delete[] mp_fMortAtZeroGrowth;
 delete[] mp_fLightDepMort;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clBCMort::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
 try {
   clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
   DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
   doubleVal *p_fTempValues;  //for getting species-specific values
   short int i; //loop counter

   //Declare the temp array and populate it with the species to which this
   //behavior applies
   p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
   for (i = 0; i < m_iNumBehaviorSpecies; i++)
     p_fTempValues[i].code = mp_iWhatSpecies[i];

   //Declare the arrays for holding the variables
   mp_fMortAtZeroGrowth = new double[m_iNumTotalSpecies];
   mp_fLightDepMort = new double[m_iNumTotalSpecies];

   //Capture the values from the parameter file

   //Mortality at zero growth
   FillSpeciesSpecificValue(p_oElement, "mo_mortAtZeroGrowth", "mo_mazgVal",
                    p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
   //Transfer to the appropriate array buckets
   for (i = 0; i < m_iNumBehaviorSpecies; i++)
     mp_fMortAtZeroGrowth[p_fTempValues[i].code] = p_fTempValues[i].val;

   //Light dependent mortality
   FillSpeciesSpecificValue(p_oElement, "mo_lightDependentMortality",
             "mo_ldmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
   //Transfer to the appropriate array buckets
   for (i = 0; i < m_iNumBehaviorSpecies; i++)
     mp_fLightDepMort[p_fTempValues[i].code] = p_fTempValues[i].val;

   m_iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

   //Get array codes for each "Growth" value
   GetGrowthVariableCodes();

   delete[] p_fTempValues; p_fTempValues = NULL;
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clBCMort::DoShellSetup";
   throw(stcErr);
 }
}

//////////////////////////////////////////////////////////////////////////////
// GetGrowthVariableCodes()
//////////////////////////////////////////////////////////////////////////////
void clBCMort::GetGrowthVariableCodes() {
 try {
   clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
   std::string sLabel = "Growth";
   short int i, j,//loop counters
             iTotalTypes = p_oPop->GetNumberOfTypes(); //number of tree types

   //Declare and initialize the return codes array
   mp_iGrowthCodes = new short int*[m_iNumTotalSpecies];
   for (i = 0; i < m_iNumTotalSpecies; i++) {
     mp_iGrowthCodes[i] = new short int[iTotalTypes];
     for (j = 0; j < iTotalTypes; j++)
       mp_iGrowthCodes[i][j] = -1;
   }

   //Now go through the growth functions table and get the code for
   //each species/type combo with a valid pointer
   for (i = 0; i < m_iNumTotalSpecies; i++)
     for (j = 0; j < iTotalTypes; j++)
       if (mp_bUsesThisMortality[i][j]) {
         mp_iGrowthCodes[i][j] = p_oPop->GetFloatDataCode(sLabel, i, j);

         //If the return code is -1, throw an error
         if (-1 == mp_iGrowthCodes[i][j]) {
           modelErr stcErr;
           stcErr.sFunction = "clBCMort::GetGrowthVariableCodes";
           std::stringstream s;
           s << "Type/species combo species=" << i << " type=" << j
             << " does not have a growth behavior compatible with its mortality behavior.";
           stcErr.iErrorCode = BAD_DATA;
           stcErr.sMoreInfo = s.str();
           throw(stcErr);
         }
       }

 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clBCMort::GetGrowthVariableCodes";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clBCMort::DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies) {
  float fRandom = clModelMath::GetRand(), //random number
      fGrowth,                //growth value
      fDeathProb;          //probability of death for this tree

  p_oTree->GetValue(mp_iGrowthCodes[p_oTree->GetSpecies()][p_oTree->GetType()], &fGrowth);

  fDeathProb = 1 - exp(-m_iNumberYearsPerTimestep * mp_fMortAtZeroGrowth[iSpecies] *
      exp(-(mp_fLightDepMort[iSpecies] * fGrowth)));

  //If the probability of death is greater than the random number, kill the
  //tree
  if (fRandom <  fDeathProb)
    return natural;
  else
    return notdead;
}
