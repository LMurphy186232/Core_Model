#include "SizeEffectExponentialHeight.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clSizeEffectExponentialHeight::clSizeEffectExponentialHeight() {
  mp_fA = NULL;
  mp_fB = NULL;
  m_fScaler = 1;
  mp_oAllom = NULL;
  mp_oPop = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSizeEffectExponentialHeight::~clSizeEffectExponentialHeight() {
  delete[] mp_fA;
  delete[] mp_fB;
}

////////////////////////////////////////////////////////////////////////////
// CalculateSizeEffect
////////////////////////////////////////////////////////////////////////////
double clSizeEffectExponentialHeight::CalculateSizeEffect(clTree *p_oTree, const float &fDiam) {
  double fSizeEffect, fHeight;
  int iSpecies = p_oTree->GetSpecies();

  //----- Get the tree's height according to the diameter passed ------------//
  if (p_oTree->GetType() == clTreePopulation::seedling) {
    fHeight = mp_oAllom->CalcSeedlingHeight(fDiam, iSpecies);
  } else if (p_oTree->GetType() == clTreePopulation::sapling) {
    fHeight = mp_oAllom->CalcSaplingHeight(fDiam, iSpecies);
  } else {
    fHeight = mp_oAllom->CalcAdultHeight(fDiam, iSpecies);
  }

  fSizeEffect = (1-mp_fA[iSpecies]*exp(mp_fB[iSpecies]*fHeight));
  //Make sure it's bounded between 0 and 1
  if ( fSizeEffect < 0 ) fSizeEffect = 0;
  if ( fSizeEffect > 1 ) fSizeEffect = 1;
  return fSizeEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clSizeEffectExponentialHeight::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_dTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fA = new double[iNumTotalSpecies];
  mp_fB = new double[iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_dTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_dTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //A
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectExpHeightA", "nseehaVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fA[p_dTempValues[i].code] = p_dTempValues[i].val;

  //B
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectExpHeightB", "nseehbVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fB[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Scaler, not required
  FillSingleValue(p_oElement, "nciSizeEffectExpHeightScaler", &m_fScaler, false);

  delete[] p_dTempValues;

  //Get a pointer to the allometry object
  mp_oAllom = p_oPop->GetAllometryObject();

}
