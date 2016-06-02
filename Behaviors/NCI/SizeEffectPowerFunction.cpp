#include "SizeEffectPowerFunction.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clSizeEffectPowerFunction::clSizeEffectPowerFunction() {
  mp_fB = NULL;
  mp_fA = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSizeEffectPowerFunction::~clSizeEffectPowerFunction() {
  delete[] mp_fB;
  delete[] mp_fA;
}

////////////////////////////////////////////////////////////////////////////
// CalculateSizeEffect
////////////////////////////////////////////////////////////////////////////
double clSizeEffectPowerFunction::CalculateSizeEffect(clTree *p_oTree, const float &fDiam) {
  double fSizeEffect;
  int iSpecies = p_oTree->GetSpecies();

  fSizeEffect = mp_fA[iSpecies] * pow(fDiam, mp_fB[iSpecies]);
  //Make sure it's bounded between 0 and 1
  if ( fSizeEffect < 0 ) fSizeEffect = 0;
  if ( fSizeEffect > 1 ) fSizeEffect = 1;
  return fSizeEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clSizeEffectPowerFunction::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fA = new double[iNumTotalSpecies];
  mp_fB = new double[iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Size effect a
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectA", "nseaVal",
      p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Size effect b
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectB", "nsebVal",
      p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;
}
