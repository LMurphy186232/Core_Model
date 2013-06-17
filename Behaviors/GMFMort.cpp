//---------------------------------------------------------------------------
#include "GMFMort.h"
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
clGMFMort::clGMFMort(clSimManager *p_oSimManager) :
      clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
      clMortalityBase(p_oSimManager) {
 try {

   m_sNameString = "gmfmortshell";
   m_sXMLRoot = "GMFMortality";

   mp_fMortAtZeroGrowth = NULL;
   mp_fLightDepMort = NULL;
   mp_iGrowthCodes = NULL;

 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clGMFMort::clGMFMort";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clGMFMort::~clGMFMort() {
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
void clGMFMort::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
 try {
   clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
   DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
   floatVal *p_fTempValues;  //for getting species-specific values
   short int i; //loop counter

   //Throw an error if the timestep length is not 5 years
   if (5 != mp_oSimManager->GetNumberOfYearsPerTimestep()) {
     modelErr stcErr;
     stcErr.iErrorCode = BAD_DATA;
     stcErr.sMoreInfo = "GMF Mortality cannot be used if the timestep length is not 5 years.";
     stcErr.sFunction = "clGMFMort::DoShellSetup";
     throw(stcErr);
   }

   //Declare the temp array and populate it with the species to which this
   //behavior applies
   p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
   for (i = 0; i < m_iNumBehaviorSpecies; i++)
     p_fTempValues[i].code = mp_iWhatSpecies[i];

   //Declare the arrays for holding the variables
   mp_fMortAtZeroGrowth = new float[m_iNumTotalSpecies];
   mp_fLightDepMort = new float[m_iNumTotalSpecies];

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

   //Get array codes for each "Growth" value
   GetGrowthVariableCodes();

   delete[] p_fTempValues;

 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clGMFMort::DoShellSetup";
   throw(stcErr);
 }
}

//////////////////////////////////////////////////////////////////////////////
// GetGrowthVariableCodes()
//////////////////////////////////////////////////////////////////////////////
void clGMFMort::GetGrowthVariableCodes() {
 try {
   clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
   char cLabel[] = "Growth";
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
         mp_iGrowthCodes[i][j] = p_oPop->GetFloatDataCode(cLabel, i, j);

         //If the return code is -1, throw an error
         if (-1 == mp_iGrowthCodes[i][j]) {
           modelErr stcErr;
           stcErr.sFunction = "clGMFMort::GetGrowthVariableCodes";
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
   stcErr.sFunction = "clGMFMort::GetGrowthVariableCodes";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clGMFMort::DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies) {
  float fRandom = clModelMath::GetRand(), //random number
      fGrowth,                //growth value
      fDeathProb;          //probability of death for this tree

  p_oTree->GetValue(mp_iGrowthCodes[p_oTree->GetSpecies()][p_oTree->GetType()], &fGrowth);

  //Kill slow growers outright
  if (fGrowth < 0.001)
    return natural;

  else

    //Calculate the probability of death
    fDeathProb = mp_fMortAtZeroGrowth[iSpecies] *
    exp(-1.0 * mp_fLightDepMort[iSpecies] * fGrowth);

  //If the probability of death is already greater than one, kill tree
  if (fDeathProb >= 1.0)
    return natural;

  else
    //Make this the five year mortality rate
    fDeathProb = fDeathProb * (2.0 - fDeathProb);

  //If the probability of death is greater than the random number, kill the
  //tree
  if (fRandom <  fDeathProb)
    return natural;
  else
    return notdead;
}
