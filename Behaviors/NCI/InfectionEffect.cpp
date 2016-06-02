#include "InfectionEffect.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "BehaviorBase.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clInfectionEffect::clInfectionEffect() {
  mp_fA = NULL;
  mp_fB = NULL;
  mp_iYearsInfestedCodes = NULL;
  m_iTotalNumSpecies = 0;
  bRequiresTargetDiam = true;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clInfectionEffect::~clInfectionEffect() {
  delete[] mp_fA;
  delete[] mp_fB;
  if (mp_iYearsInfestedCodes) {
    int i;
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_iYearsInfestedCodes[i];
  }
  delete[] mp_iYearsInfestedCodes;
}

////////////////////////////////////////////////////////////////////////////
// CalculateInfectionEffect
////////////////////////////////////////////////////////////////////////////
double clInfectionEffect::CalculateInfectionEffect(clTree *p_oTree) {
  double fEffect;
  int iYrsInfested, iSpecies = p_oTree->GetSpecies();

  p_oTree->GetValue(mp_iYearsInfestedCodes[iSpecies][p_oTree->GetType()], &iYrsInfested);

  if (iYrsInfested == 0) return 1;

  fEffect = mp_fA[iSpecies] * log(iYrsInfested) + mp_fB[iSpecies];
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clInfectionEffect::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {

  doubleVal * p_fTempValues; //for getting species-specific values
  stcSpeciesTypeCombo c;
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTypes = p_oPop->GetNumberOfTypes(), i, j;

  m_iTotalNumSpecies = p_oPop->GetNumberOfSpecies();

  mp_fA = new double[m_iTotalNumSpecies];
  mp_fB = new double[m_iTotalNumSpecies];

  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
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

  delete[] p_fTempValues;

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
      stcErr.sFunction = "clInfectionEffect::DoSetup";
      stcErr.sMoreInfo = "An infestation behavior must be applied to all trees to which NCI infection effect is applied.";
      throw( stcErr );
    }
  }
}

