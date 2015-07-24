#include "SizeEffectCompoundExponential.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clSizeEffectCompoundExponential::clSizeEffectCompoundExponential() {
  mp_fA = NULL;
  mp_fB = NULL;
  mp_fC = NULL;
  mp_fD = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSizeEffectCompoundExponential::~clSizeEffectCompoundExponential() {
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fC;
  delete[] mp_fD;
}

////////////////////////////////////////////////////////////////////////////
// CalculateSizeEffect
////////////////////////////////////////////////////////////////////////////
float clSizeEffectCompoundExponential::CalculateSizeEffect(clTree *p_oTree, const float &fDiam) {
  float fSizeEffect;
  int iSpecies = p_oTree->GetSpecies();

  fSizeEffect = (1-mp_fA[iSpecies]*exp(mp_fB[iSpecies]*(fDiam/100)))*
                 exp(mp_fC[iSpecies]*pow((fDiam/100),mp_fD[iSpecies]));
  //Make sure it's bounded between 0 and 1
  if ( fSizeEffect < 0 ) fSizeEffect = 0;
  if ( fSizeEffect > 1 ) fSizeEffect = 1;
  return fSizeEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clSizeEffectCompoundExponential::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_dTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fA = new double[iNumTotalSpecies];
  mp_fB = new double[iNumTotalSpecies];
  mp_fC = new double[iNumTotalSpecies];
  mp_fD = new double[iNumTotalSpecies];

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_dTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_dTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //A
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectA", "nseaVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fA[p_dTempValues[i].code] = p_dTempValues[i].val;

  //B
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectB", "nsebVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fB[p_dTempValues[i].code] = p_dTempValues[i].val;

  //C
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectC", "nsecVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fC[p_dTempValues[i].code] = p_dTempValues[i].val;

  //D
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectD", "nsedVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fD[p_dTempValues[i].code] = p_dTempValues[i].val;

  delete[] p_dTempValues;

}
