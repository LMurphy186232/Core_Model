#include "ShadingEffectPowerDiam.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clShadingEffectPowerDiam::clShadingEffectPowerDiam() {
  mp_fA = NULL;
  mp_fB = NULL;
  mp_fI = NULL;
  mp_iLightCodes = NULL;
  mp_iDiamCodes = NULL;
  m_iNumberTotalSpecies = 0;
  m_fGLIScaler = 100;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clShadingEffectPowerDiam::~clShadingEffectPowerDiam() {
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fI;
  int i;
  if (mp_iLightCodes) {
    for (i = 0; i < m_iNumberTotalSpecies; i++) delete[] mp_iLightCodes[i];
  }
  delete[] mp_iLightCodes;
  if (mp_iDiamCodes) {
    for (i = 0; i < m_iNumberTotalSpecies; i++) delete[] mp_iDiamCodes[i];
  }
  delete[] mp_iDiamCodes;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateShadingEffect
//////////////////////////////////////////////////////////////////////////////
double clShadingEffectPowerDiam::CalculateShadingEffect(clTree *p_oTree) {
  float fGLI, fDiam;
  double fShadingEffect;
  int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType();

  //Get the tree's GLI
  p_oTree->GetValue( mp_iLightCodes[iSpecies] [iType], &fGLI);

  //Subtract the light compensation point and make sure the result is
  //not negative
  fGLI -= mp_fI[iSpecies];
  if (fGLI < 0) fGLI = 0;

  //Scale by GLI max
  fGLI /= m_fGLIScaler;

  //Get the tree's diameter
  p_oTree->GetValue( mp_iDiamCodes[iSpecies] [iType], &fDiam);

  fShadingEffect = pow(fGLI, mp_fA[iSpecies]) *
      (1 - exp(-mp_fB[iSpecies] * fDiam));
  //Make sure it's between 0 and 1
  if ( fShadingEffect < 0 ) fShadingEffect = 0;
  if ( fShadingEffect > 1 ) fShadingEffect = 1;
  return fShadingEffect;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clShadingEffectPowerDiam::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  stcSpeciesTypeCombo c;
  doubleVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTypes = p_oPop->GetNumberOfTypes(),
      i, j;

  //----- Declare and retrieve parameters -----------------------------------//
  m_iNumberTotalSpecies = p_oPop->GetNumberOfSpecies();
  mp_fA = new double[m_iNumberTotalSpecies];
  mp_fB = new double[m_iNumberTotalSpecies];
  mp_fI = new double[m_iNumberTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) {
    p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);
  }

  //Shading "a"
  FillSpeciesSpecificValue( p_oElement, "nciShadingPowerDiamA", "nspdaVal",
      p_fTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Shading "b"
  FillSpeciesSpecificValue( p_oElement, "nciShadingPowerDiamB", "nspdbVal",
      p_fTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Shading "i"
  FillSpeciesSpecificValue( p_oElement, "nciShadingPowerDiamI", "nspdiVal",
      p_fTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fI[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;

  // GLI scaler
  FillSingleValue(p_oElement, "nciShadingPowerGLIScale", &m_fGLIScaler, true);
  //-------------------------------------------------------------------------//



  //----- Get light codes ---------------------------------------------------//
  mp_iLightCodes = new short int *[m_iNumberTotalSpecies];
  for (i = 0; i < m_iNumberTotalSpecies; i++) {
    mp_iLightCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) mp_iLightCodes[i][j] = -1;
  }

  //Make sure all species/type combos that use shading have "Light" registered
  for (i = 0; i < p_oNCI->GetNumSpeciesTypeCombos(); i++) {
    c = p_oNCI->GetSpeciesTypeCombo(i);
    mp_iLightCodes[c.iSpecies][c.iType] = p_oPop->GetFloatDataCode("Light", c.iSpecies, c.iType);

    if ( -1 == mp_iLightCodes[c.iSpecies][c.iType]) {
      modelErr stcErr;
      stcErr.sFunction = "clShadingEffectPowerDiam::DoSetup";
      stcErr.sMoreInfo = "All trees to which the shading effect of NCI is used must have a light behavior applied.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  }
  //-------------------------------------------------------------------------//



  //----- Get diam codes ----------------------------------------------------//
  mp_iDiamCodes = new short int *[m_iNumberTotalSpecies];
  for (i = 0; i < m_iNumberTotalSpecies; i++) {
    mp_iDiamCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) mp_iDiamCodes[i][j] = -1;
  }

  //If this is a seedling or sapling, do diam10; otherwise do DBH
  for (i = 0; i < p_oNCI->GetNumSpeciesTypeCombos(); i++) {
    c = p_oNCI->GetSpeciesTypeCombo(i);
    if (c.iType == clTreePopulation::seedling ||
        c.iType == clTreePopulation::sapling) {
      mp_iDiamCodes[c.iSpecies][c.iType] = p_oPop->GetDiam10Code(c.iSpecies, c.iType);
    } else {
      mp_iDiamCodes[c.iSpecies][c.iType] = p_oPop->GetDbhCode(c.iSpecies, c.iType);
    }
  }
}
