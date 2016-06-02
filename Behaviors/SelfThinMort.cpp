//---------------------------------------------------------------------------
#include "SelfThinMort.h"
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
clSelfThinMort::clSelfThinMort(clSimManager *p_oSimManager) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clMortalityBase( p_oSimManager ) {
  try {

    m_sNameString = "selfthinmortshell";
    m_sXMLRoot = "SelfThinning";

    mp_fSelfThinSlope = NULL;
    mp_fSelfThinIntercept = NULL;
    mp_fSelfThinMaxDbh = NULL;

    m_iNumberYearsPerTimestep = 0;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelfThinMort::clSelfThinMort";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSelfThinMort::~clSelfThinMort() {
  delete[] mp_fSelfThinSlope;
  delete[] mp_fSelfThinIntercept;
  delete[] mp_fSelfThinMaxDbh;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clSelfThinMort::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
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
    mp_fSelfThinSlope = new double[iNumSpecies];
    mp_fSelfThinIntercept = new double[iNumSpecies];
    mp_fSelfThinMaxDbh = new double[iNumSpecies];

    //Capture the values from the parameter file

    //Self-thinning slope
    FillSpeciesSpecificValue(p_oElement, "mo_selfThinSlope", "mo_stsVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fSelfThinSlope[p_fTempValues[i].code] = p_fTempValues[i].val;


    //Self-thinning intercept
    FillSpeciesSpecificValue(p_oElement, "mo_selfThinIntercept",
        "mo_stiVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fSelfThinIntercept[p_fTempValues[i].code] = p_fTempValues[i].val;


    //Max self-thinning dbh
    FillSpeciesSpecificValue(p_oElement, "mo_selfThinMaxDbh",
        "mo_stmdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fSelfThinMaxDbh[p_fTempValues[i].code] = p_fTempValues[i].val;


    delete[] p_fTempValues;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSelfThinMort::DoShellSetup";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clSelfThinMort::DoMort(clTree *p_oTree, const float &fDiam, const short int &iSpecies) {
  float fRandom = clModelMath::GetRand(), //random number
      fAnnualDeathProb,    //annual probability of death
      fTimestepDeathProb;  //death probability for the entire timestep

  //If this is adult self-thinning make sure we're below the max dbh - if
  //not, exit with no death
  if (fDiam > mp_fSelfThinMaxDbh[iSpecies])
    return notdead;

  fAnnualDeathProb = clModelMath::CalcPointValue(fDiam,
      mp_fSelfThinSlope[iSpecies],
      mp_fSelfThinIntercept[iSpecies]);

  //The function's linear - the result can be less than zero - make zero
  //the minimum
  if (fAnnualDeathProb < 0.0)
    fAnnualDeathProb = 0.0;

  //Compound into the death probability for the whole timestep
  fTimestepDeathProb = 1 -
      pow(1 - fAnnualDeathProb, m_iNumberYearsPerTimestep);

  if(fRandom < fTimestepDeathProb)
    return natural;
  else
    return notdead;
}
