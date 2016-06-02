#include "CrowdingEffectTempDep.h"
#include "ParsingFunctions.h"
#include "Tree.h"
#include "Plot.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "NCIBehaviorBase.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clCrowdingEffectTempDep::clCrowdingEffectTempDep() {
  mp_fC = NULL;
  mp_fD = NULL;
  mp_fX0 = NULL;
  mp_fXb = NULL;
  mp_fGamma = NULL;

  mp_oPlot = NULL;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clCrowdingEffectTempDep::~clCrowdingEffectTempDep() {
  delete[] mp_fC;
  delete[] mp_fD;
  delete[] mp_fX0;
  delete[] mp_fXb;
  delete[] mp_fGamma;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateCrowdingEffect
//////////////////////////////////////////////////////////////////////////////
double clCrowdingEffectTempDep::CalculateCrowdingEffect(clTree *p_oTree, const float &fDiam, const clNCITermBase::ncivals nci, const int &iSpecies) {
  double fCrowdingEffect, fTerm1, fTerm2, fCT,
        fTemp = mp_oPlot->GetMeanAnnualTemp() + 273.15;
  if (!m_b2ValNCI) {
    fTerm1 = fDiam;
    fTerm2 = nci.fNCI1;
  } else {
    fTerm1 = nci.fNCI1;
    fTerm2 = nci.fNCI2;
  }
  fTemp = (fTemp-mp_fX0[iSpecies])/mp_fXb[iSpecies];
  fTemp *= fTemp;
  fCT = mp_fC[iSpecies] * (1-exp(-0.5 * fTemp));

  //Avoid a domain error - if NCI is 0, return 1
  if ( fTerm2 > 0 ) {
    fCrowdingEffect = exp( -fCT * pow( fTerm1, mp_fGamma[iSpecies] )
        * pow( fTerm2, mp_fD[iSpecies]));
    if ( fCrowdingEffect < 0 ) fCrowdingEffect = 0;
    if ( fCrowdingEffect > 1 ) fCrowdingEffect = 1;
    return fCrowdingEffect;
  }
  return 1.0;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clCrowdingEffectTempDep::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, clNCIBehaviorBase *p_oNCIBase, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  //Find out how many NCI terms
  clNCITermBase *p_oNCITerm = p_oNCIBase->GetNCITerm();
  if (p_oNCITerm->GetNumberNCIs() == 2) m_b2ValNCI = true;

  //Get a pointer to the plot object
  mp_oPlot = p_oNCI->GetSimManager()->GetPlotObject();

  mp_fC = new double[iNumTotalSpecies];
  mp_fX0 = new double[iNumTotalSpecies];
  mp_fXb = new double[iNumTotalSpecies];
  mp_fD = new double[iNumTotalSpecies];
  mp_fGamma = new double[iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Size sensitivity to NCI parameter (gamma)
  FillSpeciesSpecificValue( p_oElement, "nciCrowdingGamma", "ncgVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fGamma[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Crowding Slope (C)
  FillSpeciesSpecificValue( p_oElement, "nciCrowdingC", "nccVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fC[p_fTempValues[i].code] = p_fTempValues[i].val;

  //X0
  FillSpeciesSpecificValue( p_oElement, "nciCrowdingCX0", "nccx0Val", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fX0[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Xb
  FillSpeciesSpecificValue( p_oElement, "nciCrowdingCXb", "nccxbVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fXb[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Crowding Steepness (D)
  FillSpeciesSpecificValue( p_oElement, "nciCrowdingD", "ncdVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fD[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;
}
