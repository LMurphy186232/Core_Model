#include "SizeEffectShiftedLogInf.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clSizeEffectShiftedLogInf::clSizeEffectShiftedLogInf() {
  mp_iDataCodes = NULL;
  mp_fXb = NULL;
  mp_fX0 = NULL;
  mp_fXp = NULL;
  mp_fXbInf = NULL;
  mp_fX0Inf = NULL;
  mp_fXpInf = NULL;
  mp_fMinDiam = NULL;
  m_iNumSpecies = 0;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSizeEffectShiftedLogInf::~clSizeEffectShiftedLogInf() {
  delete[] mp_fXb;
  delete[] mp_fX0;
  delete[] mp_fXp;

  delete[] mp_fXbInf;
  delete[] mp_fX0Inf;
  delete[] mp_fXpInf;

  delete[] mp_fMinDiam;

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
double clSizeEffectShiftedLogInf::CalculateSizeEffect(clTree *p_oTree, const float &fDiam) {
  double fSizeEffect = 0, fDiam2;
  int iSpecies = p_oTree->GetSpecies(), iYrsInfested;

  //Get the infestation status for this tree
  p_oTree->GetValue( mp_iDataCodes[iSpecies][p_oTree->GetType()], &iYrsInfested);

  //Make sure the diameter is above the minimum
  fDiam2 = (fDiam < mp_fMinDiam[iSpecies]) ? mp_fMinDiam[iSpecies] : fDiam;

  if (iYrsInfested == 0) { //not infested
    fSizeEffect = exp(-0.5 * pow(log((fDiam2 + mp_fXp[iSpecies]) / mp_fX0[iSpecies] ) / mp_fXb[iSpecies], 2));
  } else {
    fSizeEffect = exp(-0.5 * pow(log((fDiam2 + mp_fXpInf[iSpecies]) / mp_fX0Inf[iSpecies] ) / mp_fXbInf[iSpecies], 2));
  }
  //Make sure it's bounded between 0 and 1
  if ( fSizeEffect < 0 ) fSizeEffect = 0;
  if ( fSizeEffect > 1 ) fSizeEffect = 1;
  return fSizeEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clSizeEffectShiftedLogInf::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_dTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTypes = p_oPop->GetNumberOfTypes(), i, j;

  m_iNumSpecies = p_oPop->GetNumberOfSpecies();

  mp_fX0 = new double[m_iNumSpecies];
  mp_fXb = new double[m_iNumSpecies];
  mp_fXp = new double[m_iNumSpecies];
  mp_fX0Inf = new double[m_iNumSpecies];
  mp_fXbInf = new double[m_iNumSpecies];
  mp_fXpInf = new double[m_iNumSpecies];
  mp_fMinDiam = new double[m_iNumSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_dTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_dTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Size effect mode (X0)
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectX0", "nsex0Val",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fX0[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect variance (Xb)
  FillSpeciesSpecificValue(p_oElement, "nciSizeEffectXb", "nsexbVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++)
    mp_fXb[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect shift (Xp)
  FillSpeciesSpecificValue(p_oElement, "nciSizeEffectXp", "nsexpVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++)
    mp_fXp[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect mode (X0) - infected
  FillSpeciesSpecificValue( p_oElement, "nciSizeEffectInfX0", "nseix0Val",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fX0Inf[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect variance (Xb) - infected
  FillSpeciesSpecificValue(p_oElement, "nciSizeEffectInfXb", "nseixbVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++)
    mp_fXbInf[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect shift (Xp) - infected
  FillSpeciesSpecificValue(p_oElement, "nciSizeEffectInfXp", "nseixpVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++)
    mp_fXpInf[p_dTempValues[i].code] = p_dTempValues[i].val;

  //Size effect lower bound
  FillSpeciesSpecificValue(p_oElement, "nciSizeEffectLowerBound", "nselbVal",
      p_dTempValues, iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++)
    mp_fMinDiam[p_dTempValues[i].code] = p_dTempValues[i].val;

  delete[] p_dTempValues;

  //Make sure that the size effect mode is not 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (0 >= mp_fX0[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSizeEffectShiftedLogInf::DoSetup";
      stcErr.sMoreInfo = "NCI size effect mode (X0) values must be greater than 0.";
      throw( stcErr );
    }

    if (0 >= mp_fX0Inf[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSizeEffectShiftedLogInf::DoSetup";
      stcErr.sMoreInfo = "NCI size effect mode (X0) values must be greater than 0.";
      throw( stcErr );
    }

    //Make sure that the size effect variance is not 0
    if (0 == mp_fXb[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSizeEffectShiftedLogInf::DoSetup";
      stcErr.sMoreInfo = "NCI size effect variance (Xb) values cannot be 0.";
      throw( stcErr );
    }

    if (0 == mp_fXbInf[p_oNCI->GetBehaviorSpecies(i)]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSizeEffectShiftedLogInf::DoSetup";
      stcErr.sMoreInfo = "NCI size effect variance (Xb) values cannot be 0.";
      throw( stcErr );
    }
  }

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
      stcErr.sFunction = "clSizeEffectShiftedLogInf::DoSetup" ;
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
