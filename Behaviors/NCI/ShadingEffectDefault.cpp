#include "ShadingEffectDefault.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clShadingEffectDefault::clShadingEffectDefault() {
  mp_fShadingCoefficient = NULL;
  mp_fShadingExponent = NULL;
  mp_iLightCodes = NULL;
  m_iNumberTotalSpecies = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clShadingEffectDefault::~clShadingEffectDefault() {
  delete[] mp_fShadingCoefficient;
  delete[] mp_fShadingExponent;
  if (mp_iLightCodes) {
    for (int i = 0; i < m_iNumberTotalSpecies; i++) delete[] mp_iLightCodes[i];
  }
  delete[] mp_iLightCodes;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateShadingEffect
//////////////////////////////////////////////////////////////////////////////
float clShadingEffectDefault::CalculateShadingEffect(clTree *p_oTree) {
  float fAmountShade, fShadingEffect;
  int iSpecies = p_oTree->GetSpecies();

  //Get the tree's shading
  p_oTree->GetValue( mp_iLightCodes[iSpecies] [p_oTree->GetType()], &fAmountShade);

  fShadingEffect = exp( -mp_fShadingCoefficient[iSpecies] * pow( fAmountShade, mp_fShadingExponent[iSpecies]));
  //Make sure it's between 0 and 1
  if ( fShadingEffect < 0 ) fShadingEffect = 0;
  if ( fShadingEffect > 1 ) fShadingEffect = 1;
  return fShadingEffect;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clShadingEffectDefault::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  stcSpeciesTypeCombo c;
  floatVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTypes = p_oPop->GetNumberOfTypes(),
      i, j;

  m_iNumberTotalSpecies = p_oPop->GetNumberOfSpecies();
  mp_fShadingCoefficient = new float[m_iNumberTotalSpecies];
  mp_fShadingExponent = new float[m_iNumberTotalSpecies];

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Shading coefficient (m)
  FillSpeciesSpecificValue( p_oElement, "nciShadingCoefficient", "nscVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fShadingCoefficient[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Shading exponent (n)
  FillSpeciesSpecificValue( p_oElement, "nciShadingExponent", "nseVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fShadingExponent[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Declare the light codes array
  mp_iLightCodes = new short int *[m_iNumberTotalSpecies];
  for (i = 0; i < m_iNumberTotalSpecies; i++) {
    mp_iLightCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) mp_iLightCodes[i][j] = -1;
  }

  delete[] p_fTempValues;

  //Make sure all species/type combos that use shading have "Light" registered
  for (i = 0; i < p_oNCI->GetNumSpeciesTypeCombos(); i++) {
    c = p_oNCI->GetSpeciesTypeCombo(i);
    mp_iLightCodes[c.iSpecies][c.iType] = p_oPop->GetFloatDataCode("Light", c.iSpecies, c.iType);

    if ( -1 == mp_iLightCodes[c.iSpecies][c.iType]) {
      modelErr stcErr;
      stcErr.sFunction = "clShadingEffectDefault::DoSetup";
      stcErr.sMoreInfo = "All trees to which the shading effect of NCI is used must have a light behavior applied.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  }
}
