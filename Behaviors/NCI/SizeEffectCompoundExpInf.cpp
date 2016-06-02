#include "SizeEffectCompoundExpInf.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clSizeEffectCompoundExpInf::clSizeEffectCompoundExpInf() {
  mp_iDataCodes = NULL;
  mp_fA = NULL;
  mp_fB = NULL;
  mp_fC = NULL;
  mp_fD = NULL;
  mp_fAInf = NULL;
  mp_fBInf = NULL;
  mp_fCInf = NULL;
  mp_fDInf = NULL;
  m_iNumSpecies = 0;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSizeEffectCompoundExpInf::~clSizeEffectCompoundExpInf() {
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fC;
  delete[] mp_fD;

  delete[] mp_fAInf;
  delete[] mp_fBInf;
  delete[] mp_fCInf;
  delete[] mp_fDInf;

  if ( mp_iDataCodes ) {
    for ( int i = 0; i < m_iNumSpecies; i++ ) {
      delete[] mp_iDataCodes[i];
    }
  }
  delete[] mp_iDataCodes;
}

////////////////////////////////////////////////////////////////////////////
// CalculateSizeEffect
////////////////////////////////////////////////////////////////////////////
double clSizeEffectCompoundExpInf::CalculateSizeEffect(clTree *p_oTree, const float &fDiam) {
  double fSizeEffect = 0;
  int iSpecies = p_oTree->GetSpecies(), iYrsInfested;

  //Get the infestation status for this tree
  p_oTree->GetValue( mp_iDataCodes[iSpecies][p_oTree->GetType()], &iYrsInfested);

  if (iYrsInfested == 0) { //not infested
    fSizeEffect = (1-mp_fA[iSpecies]*exp(mp_fB[iSpecies]*(fDiam/100)))*
                   exp(mp_fC[iSpecies]*pow((fDiam/100),mp_fD[iSpecies]));
  } else {
    fSizeEffect = (1-mp_fAInf[iSpecies]*exp(mp_fBInf[iSpecies]*(fDiam/100)))*
                   exp(mp_fCInf[iSpecies]*pow((fDiam/100),mp_fDInf[iSpecies]));
  }
  //Make sure it's bounded between 0 and 1
  if ( fSizeEffect < 0 ) fSizeEffect = 0;
  if ( fSizeEffect > 1 ) fSizeEffect = 1;
  return fSizeEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clSizeEffectCompoundExpInf::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_dTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTypes = p_oPop->GetNumberOfTypes(), i, j;

  m_iNumSpecies = p_oPop->GetNumberOfSpecies();

  mp_fA = new double[m_iNumSpecies];
  mp_fB = new double[m_iNumSpecies];
  mp_fC = new double[m_iNumSpecies];
  mp_fD = new double[m_iNumSpecies];

  mp_fAInf = new double[m_iNumSpecies];
  mp_fBInf = new double[m_iNumSpecies];
  mp_fCInf = new double[m_iNumSpecies];
  mp_fDInf = new double[m_iNumSpecies];

  //Set up our doubleVal array that will extract values only for the species
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

  //A, infested
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectInfA", "nseiaVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fAInf[p_dTempValues[i].code] = p_dTempValues[i].val;

  //B, infested
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectInfB", "nseibVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fBInf[p_dTempValues[i].code] = p_dTempValues[i].val;

  //C, infested
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectInfC", "nseicVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCInf[p_dTempValues[i].code] = p_dTempValues[i].val;

  //D, infested
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectInfD", "nseidVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fDInf[p_dTempValues[i].code] = p_dTempValues[i].val;

  delete[] p_dTempValues;

  //Collect the "YearsInfested" codes
  mp_iDataCodes = new short int *[m_iNumSpecies];
  for (i = 0; i < m_iNumSpecies; i++) {
    mp_iDataCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iDataCodes[i][j] = -1;
    }
  }
  for (i = 0; i < p_oNCI->GetNumSpeciesTypeCombos(); i++ )
  {
    mp_iDataCodes[p_oNCI->GetSpeciesTypeCombo(i).iSpecies]
                 [p_oNCI->GetSpeciesTypeCombo(i).iType] =
                      p_oPop->GetIntDataCode("YearsInfested",
                          p_oNCI->GetSpeciesTypeCombo(i).iSpecies,
                          p_oNCI->GetSpeciesTypeCombo(i).iType );
    if (-1 == mp_iDataCodes[p_oNCI->GetSpeciesTypeCombo(i).iSpecies]
                           [p_oNCI->GetSpeciesTypeCombo(i).iType] )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSizeEffectCompoundExpInf::DoSetup" ;
      std::stringstream s;
      s << "Type/species combo species="
        << p_oNCI->GetSpeciesTypeCombo(i).iSpecies << " type="
        << p_oNCI->GetSpeciesTypeCombo(i).iType
        << " does not have the insect infestation behavior.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  }
}
