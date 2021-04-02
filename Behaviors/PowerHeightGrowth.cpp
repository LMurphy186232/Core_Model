//---------------------------------------------------------------------------
// PowerHeightGrowth.cpp
//---------------------------------------------------------------------------
#include "PowerHeightGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "GrowthOrg.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////*/
clPowerHeightGrowth::clPowerHeightGrowth(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager), clGrowthBase(
      p_oSimManager) {

  try {
    mp_fN = NULL;
    mp_fB = NULL;

    m_iGrowthMethod = height_only;
    m_iNumberYearsPerTimestep = 0;

    m_sNameString = "powergrowthshell";
    m_sXMLRoot = "PowerGrowth";

  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPowerHeightGrowth::clPowerHeightGrowth";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clPowerHeightGrowth::~clPowerHeightGrowth() {

  delete[] mp_fN;
  delete[] mp_fB;
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clPowerHeightGrowth::DoShellSetup(DOMDocument * p_oDoc) {
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try {
    clTreePopulation * p_oPop =
        (clTreePopulation *) mp_oSimManager->GetPopulationObject(
            "treepopulation");
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Declare the arrays we'd like read
    mp_fB = new double[iNumSpecies];
    mp_fN = new double[iNumSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //N
    FillSpeciesSpecificValue(p_oElement, "gr_powerHeightN", "gr_phnVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

    //Transfer values to our permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fN[p_fTempValues[i].code] = p_fTempValues[i].val;

    //B
    FillSpeciesSpecificValue(p_oElement, "gr_powerHeightExp", "gr_pheVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

    //Transfer values to our permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

    m_iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    delete[] p_fTempValues;
  } catch (modelErr& err) {
    delete[] p_fTempValues;
    throw(err);
  } catch (modelMsg & msg) {
    delete[] p_fTempValues;
    throw(msg);
  } //non-fatal error
  catch (...) {
    delete[] p_fTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPowerHeightGrowth::GetNameData";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcHeightGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clPowerHeightGrowth::CalcHeightGrowthValue(clTree *p_oTree,
    clTreePopulation *p_oPop, float fDiameterGrowth) {
  float fHeight, //tree's height value
      fNewHeight, //tree's new height value
      fAmountHeightIncrease; //amount by which the tree's height will
  //increase
  short int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType(), iYr;

  //Get the height for this tree
  p_oTree->GetValue(p_oPop->GetHeightCode(iSpecies, iType), &fHeight);
  fHeight *= 100.0; //transform to cm
  fNewHeight = fHeight;

  //Compound the relative growth over the number of years/time step
  for (iYr = 0; iYr < m_iNumberYearsPerTimestep; iYr++) {
    fNewHeight += mp_fN[iSpecies] * pow(fNewHeight, mp_fB[iSpecies]);
  }
  //Don't allow a negative new height - set to 10 cm
  if (fNewHeight <= 10.0)
    fNewHeight = 10.0;

  fAmountHeightIncrease = fNewHeight - fHeight;
  //Transform to m
  fAmountHeightIncrease /= 100.0;

  return fAmountHeightIncrease;
}
