//---------------------------------------------------------------------------
#include "SizeDependentLogisticMortality.h"
//---------------------------------------------------------------------------
#include "MortalityOrg.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ModelMath.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clSizeDependentLogisticMortality::clSizeDependentLogisticMortality(clSimManager *p_oSimManager) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clMortalityBase( p_oSimManager ) {
  try {

    m_sNameString = "sizedeplogisticmortshell";
    m_sXMLRoot = "SizeDependentLogisticMortality";

    mp_fMax = NULL;
    mp_fX0 = NULL;
    mp_fXb = NULL;

    m_iNumberYearsPerTimestep = 0;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSizeDependentLogisticMortality::clSizeDependentLogisticMortality";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSizeDependentLogisticMortality::~clSizeDependentLogisticMortality() {
  delete[] mp_fMax;
  delete[] mp_fX0;
  delete[] mp_fXb;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clSizeDependentLogisticMortality::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  try {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
    doubleVal *p_fTempValues;  //for getting species-specific values
    short int iNumSpecies = mp_oMortalityOrg->GetNumberOfSpecies(),
        i; //loop counter

    m_iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare the arrays for holding the variables
    mp_fMax = new double[iNumSpecies];
    mp_fX0 = new double[iNumSpecies];
    mp_fXb = new double[iNumSpecies];

    //Capture the values from the parameter file

    //Max
    FillSpeciesSpecificValue(p_oElement, "mo_sizeDepLogMax", "mo_sdlmVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fMax[p_fTempValues[i].code] = p_fTempValues[i].val;

    //X0
    FillSpeciesSpecificValue(p_oElement, "mo_sizeDepLogX0", "mo_sdlx0Val",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fX0[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Xb
    FillSpeciesSpecificValue(p_oElement, "mo_sizeDepLogXb", "mo_sdlxbVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fXb[p_fTempValues[i].code] = p_fTempValues[i].val;

    delete[] p_fTempValues;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSizeDependentLogisticMortality::DoShellSetup";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clSizeDependentLogisticMortality::DoMort(clTree *p_oTree, const float &fDiam, const short int &iSpecies) {
  float fRandom = clModelMath::GetRand(), //random number
        fAnnualDeathProb,    //annual probability of death
        fTimestepDeathProb;  //death probability for the entire timestep

  fAnnualDeathProb = mp_fMax[iSpecies]/(1+pow(fDiam/mp_fX0[iSpecies], mp_fXb[iSpecies]));

  //Compound into the death probability for the whole timestep
  fTimestepDeathProb = 1 -
      pow(1 - fAnnualDeathProb, m_iNumberYearsPerTimestep);

  if (fRandom < fTimestepDeathProb)
    return natural;
  else
    return notdead;
}
