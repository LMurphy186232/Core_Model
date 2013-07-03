#include "DefaultDamageEffect.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clDefaultDamageEffect::clDefaultDamageEffect() {
  mp_fMedDamageStormEff = NULL;
  mp_fFullDamageStormEff = NULL;
  mp_iDamageCodes = NULL;
  m_iNumberTotalSpecies = 0;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clDefaultDamageEffect::~clDefaultDamageEffect() {
  delete[] mp_fMedDamageStormEff;
  delete[] mp_fFullDamageStormEff;
  if (mp_iDamageCodes) {
    for (int i = 0; i < m_iNumberTotalSpecies; i++)
      delete[] mp_iDamageCodes[i];
  }
  delete[] mp_iDamageCodes;
}

////////////////////////////////////////////////////////////////////////////
// CalculateDamageEffect
////////////////////////////////////////////////////////////////////////////
float clDefaultDamageEffect::CalculateDamageEffect(clTree *p_oTree) {
  int iDamage;
  if (-1 == mp_iDamageCodes[p_oTree->GetSpecies()] [p_oTree->GetType()]) return 1;

  p_oTree->GetValue( mp_iDamageCodes[p_oTree->GetSpecies()] [p_oTree->GetType()], & iDamage );
  if ( 0 == iDamage ) return 1; //no damage
  else if ( 2000 <= iDamage ) //full damage
    return mp_fFullDamageStormEff[p_oTree->GetSpecies()];

  //medium damage
  return mp_fMedDamageStormEff[p_oTree->GetSpecies()];
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clDefaultDamageEffect::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  stcSpeciesTypeCombo c;
  floatVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTypes = p_oPop->GetNumberOfTypes(), i, j;

  m_iNumberTotalSpecies = p_oPop->GetNumberOfSpecies();
  mp_fMedDamageStormEff = new float[m_iNumberTotalSpecies];
  mp_fFullDamageStormEff = new float[m_iNumberTotalSpecies];

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Storm effect - medium damage
  FillSpeciesSpecificValue(p_oElement, "nciStormEffMedDmg", "nsemdVal",
      p_fTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for (i = 0; i < iNumBehaviorSpecies; i++)
    mp_fMedDamageStormEff[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Storm effect - full damage
  FillSpeciesSpecificValue(p_oElement, "nciStormEffFullDmg", "nsefdVal",
      p_fTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fFullDamageStormEff[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Make sure that the storm effect values are between 0 and 1
  for (j = 0; j < iNumBehaviorSpecies; j++ ) {
    i = p_oNCI->GetBehaviorSpecies(j);
    if (0 > mp_fMedDamageStormEff[i] || mp_fMedDamageStormEff[i] > 1) {
      modelErr err;
      err.sMoreInfo = "NCI storm effect values must be between 0 and 1.";
      err.sFunction = "clDefaultDamageEffect::DoSetup";
      err.iErrorCode = BAD_DATA;
      throw(err);
    }

    if (0 > mp_fFullDamageStormEff[i] || mp_fFullDamageStormEff[i] > 1) {
      modelErr err;
      err.sMoreInfo = "NCI storm effect values must be between 0 and 1.";
      err.sFunction = "clDefaultDamageEffect::DoSetup";
      err.iErrorCode = BAD_DATA;
      throw(err);
    }
  }

  mp_iDamageCodes = new short int*[m_iNumberTotalSpecies];
  for (i = 0; i < m_iNumberTotalSpecies; i++) {
    mp_iDamageCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++)
      mp_iDamageCodes[i][j] = -1;

  }
  for (i = 0; i < p_oNCI->GetNumSpeciesTypeCombos(); i++) {
    c = p_oNCI->GetSpeciesTypeCombo(i);
    mp_iDamageCodes[c.iSpecies][c.iType] =
        p_oPop->GetIntDataCode("stm_dmg", c.iSpecies, c.iType);
  }

  delete[] p_fTempValues;
}
