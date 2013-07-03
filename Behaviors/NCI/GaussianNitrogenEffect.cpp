#include "GaussianNitrogenEffect.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "BehaviorBase.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clGaussianNitrogenEffect::clGaussianNitrogenEffect() {
  mp_fX0 = NULL;
  mp_fXb = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clGaussianNitrogenEffect::~clGaussianNitrogenEffect() {
  delete[] mp_fX0;
  delete[] mp_fXb;
}

////////////////////////////////////////////////////////////////////////////
// CalculateNitrogenEffect
////////////////////////////////////////////////////////////////////////////
float clGaussianNitrogenEffect::CalculateNitrogenEffect(clPlot *p_oPlot, int iSpecies) {
  float fEffect,
  fNDep = p_oPlot->GetNDeposition();
  fEffect = (fNDep - mp_fX0[iSpecies]) / mp_fXb[iSpecies];
  fEffect *= fEffect;
  fEffect = exp(-0.5*fEffect);
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clGaussianNitrogenEffect::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  floatVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fX0 = new float[iNumTotalSpecies];
  mp_fXb = new float[iNumTotalSpecies];

  p_fTempValues = new floatVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) {
    p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);
  }

  //X0
  FillSpeciesSpecificValue( p_oElement, "nciNitrogenX0", "nnx0Val", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fX0[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Xb
  FillSpeciesSpecificValue( p_oElement, "nciNitrogenXb", "nnxbVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fXb[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Make sure that the size effect variance is not 0
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) {
    if (0 == mp_fXb[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGaussianNitrogenEffect::DoSetup";
      stcErr.sMoreInfo = "NCI Nitrogen Xb values cannot be 0.";
      throw( stcErr );
    }
  }
}

