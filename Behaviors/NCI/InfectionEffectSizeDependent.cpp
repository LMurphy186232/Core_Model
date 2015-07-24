#include "InfectionEffectSizeDependent.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "BehaviorBase.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clInfectionEffectSizeDependent::clInfectionEffectSizeDependent() {
  mp_fA = NULL;
  mp_fB = NULL;
  mp_fXb = NULL;
  mp_fXp = NULL;
  mp_fX0 = NULL;
  mp_iYearsInfestedCodes = NULL;
  mp_iDiamCodes = NULL;
  m_iTotalNumSpecies = 0;
  bRequiresTargetDiam = true;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clInfectionEffectSizeDependent::~clInfectionEffectSizeDependent() {
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fXb;
  delete[] mp_fXp;
  delete[] mp_fX0;
  if (mp_iYearsInfestedCodes) {
    int i;
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_iYearsInfestedCodes[i];
  }
  delete[] mp_iYearsInfestedCodes;
  if (mp_iDiamCodes) {
    int i;
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_iDiamCodes[i];
  }
  delete[] mp_iDiamCodes;
}

////////////////////////////////////////////////////////////////////////////
// CalculateInfectionEffect
////////////////////////////////////////////////////////////////////////////
float clInfectionEffectSizeDependent::CalculateInfectionEffect(clTree *p_oTree) {
  float fEffect, fDiam;
  int iYrsInfested, iSpecies = p_oTree->GetSpecies(),
      iType = p_oTree->GetType();

  p_oTree->GetValue(mp_iYearsInfestedCodes[iSpecies][iType], &iYrsInfested);

  if (iYrsInfested == 0) return 1;

  p_oTree->GetValue(mp_iDiamCodes[iSpecies][iType], &fDiam);

  fEffect = (mp_fA[iSpecies] * log(iYrsInfested) + mp_fB[iSpecies]) *
      exp(-0.5 * pow(log((fDiam + mp_fXp[iSpecies]) / mp_fX0[iSpecies] ) / mp_fXb[iSpecies], 2));
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clInfectionEffectSizeDependent::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {

  floatVal * p_fTempValues; //for getting species-specific values
  stcSpeciesTypeCombo c;
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTypes = p_oPop->GetNumberOfTypes(), i, j;

  m_iTotalNumSpecies = p_oPop->GetNumberOfSpecies();

  mp_fA = new float[m_iTotalNumSpecies];
  mp_fB = new float[m_iTotalNumSpecies];
  mp_fX0 = new float[m_iTotalNumSpecies];
  mp_fXb = new float[m_iTotalNumSpecies];
  mp_fXp = new float[m_iTotalNumSpecies];

  p_fTempValues = new floatVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) {
    p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);
  }

  //a
  FillSpeciesSpecificValue( p_oElement, "nciInfectionEffectA", "nieaVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;

  //b
  FillSpeciesSpecificValue( p_oElement, "nciInfectionEffectB", "niebVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

  //X0
  FillSpeciesSpecificValue( p_oElement, "nciInfectionEffectX0", "niex0Val", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fX0[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Xb
  FillSpeciesSpecificValue( p_oElement, "nciInfectionEffectXb", "niexbVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fXb[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Xp
  FillSpeciesSpecificValue( p_oElement, "nciInfectionEffectXp", "niexpVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fXp[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;

  //Make sure that X0 is not 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (0 >= mp_fX0[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clInfectionEffectSizeDependent::DoSetup";
      stcErr.sMoreInfo = "Infection effect mode (X0) values must be greater than 0.";
      throw( stcErr );
    }

    //Make sure that Xb is not 0
    if (0 == mp_fXb[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clInfectionEffectSizeDependent::DoSetup";
      stcErr.sMoreInfo = "Infection effect variance (Xb) values cannot be 0.";
      throw( stcErr );
    }
  }

  //Get the "Years Infested" data member
  mp_iYearsInfestedCodes = new short int * [m_iTotalNumSpecies];
  for ( i = 0; i < m_iTotalNumSpecies; i++ ) {
    mp_iYearsInfestedCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iYearsInfestedCodes[i][j] = -1;
    }
  }
  for (i = 0; i < p_oNCI->GetNumSpeciesTypeCombos(); i++) {
    c = p_oNCI->GetSpeciesTypeCombo(i);
    mp_iYearsInfestedCodes[c.iSpecies][c.iType] =
        p_oPop->GetIntDataCode("YearsInfested", c.iSpecies, c.iType);
    if (-1 == mp_iYearsInfestedCodes[c.iSpecies][c.iType]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clInfectionEffectSizeDependent::DoSetup";
      stcErr.sMoreInfo = "An infestation behavior must be applied to all trees to which NCI infection effect is applied.";
      throw( stcErr );
    }
  }

  //Get the diameter data members
  mp_iDiamCodes = new short int * [m_iTotalNumSpecies];
  for ( i = 0; i < m_iTotalNumSpecies; i++ ) {
    mp_iDiamCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iDiamCodes[i][j] = -1;
    }
  }
  for (i = 0; i < p_oNCI->GetNumSpeciesTypeCombos(); i++) {
    c = p_oNCI->GetSpeciesTypeCombo(i);
    if (c.iType == clTreePopulation::seedling) {
      mp_iDiamCodes[c.iSpecies][c.iType] = p_oPop->GetDiam10Code(c.iSpecies, c.iType);
    } else if (c.iType == clTreePopulation::sapling ||
               c.iType == clTreePopulation::adult ||
               c.iType == clTreePopulation::snag) {
      mp_iDiamCodes[c.iSpecies][c.iType] = p_oPop->GetDbhCode(c.iSpecies, c.iType);
    } else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clInfectionEffectSizeDependent::DoSetup";
      stcErr.sMoreInfo = "NCI infection effect may only be applied to trees.";
      throw( stcErr );
    }
  }

}

