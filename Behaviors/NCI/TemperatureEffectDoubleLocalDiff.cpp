#include "TemperatureEffectDoubleLocalDiff.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "SimManager.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clTemperatureEffectDoubleLocalDiff::clTemperatureEffectDoubleLocalDiff() {
  m_fPrevTemp = 0;
  m_fCurrTemp = 0;
  mp_fCurrAA = NULL;
  mp_fCurrABLo = NULL;
  mp_fCurrABHi = NULL;
  mp_fCurrAC = NULL;
  mp_fCurrBLo = NULL;
  mp_fCurrBHi = NULL;
  mp_fCurrC = NULL;
  mp_fPrevAA = NULL;
  mp_fPrevABLo = NULL;
  mp_fPrevABHi = NULL;
  mp_fPrevAC = NULL;
  mp_fPrevBLo = NULL;
  mp_fPrevBHi = NULL;
  mp_fPrevC = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clTemperatureEffectDoubleLocalDiff::~clTemperatureEffectDoubleLocalDiff() {
  delete [] mp_fCurrAA;
  delete [] mp_fCurrABLo;
  delete [] mp_fCurrABHi;
  delete [] mp_fCurrAC;
  delete [] mp_fCurrBLo;
  delete [] mp_fCurrBHi;
  delete [] mp_fCurrC;
  delete [] mp_fPrevAA;
  delete [] mp_fPrevABLo;
  delete [] mp_fPrevABHi;
  delete [] mp_fPrevAC;
  delete [] mp_fPrevBLo;
  delete [] mp_fPrevBHi;
  delete [] mp_fPrevC;
}

///////////////////////////////////////////////////////////////////////////////
// CalculateTemperatureEffect
///////////////////////////////////////////////////////////////////////////////
double clTemperatureEffectDoubleLocalDiff::CalculateTemperatureEffect(clPlot *p_oPlot, int iSpecies) {
  double fEffect, fPrev, fCurrent, fTemp, fA,
         fLTM = p_oPlot->GetLTMAnnualTemp(),
         fShifted;

  //-------------------------------------------------------------------------//
  // Current year portion
  //-------------------------------------------------------------------------//

  //----- Calculate "a" -----------------------------------------------------//
  fA = 0;
  fTemp = fLTM - mp_fCurrAC[iSpecies];
  fTemp *= fTemp;
  if (fLTM < mp_fCurrAC[iSpecies]) {
    fA = mp_fCurrAA[iSpecies] * pow(mp_fCurrABLo[iSpecies], fTemp);
  } else {
    fA = mp_fCurrAA[iSpecies] * pow(mp_fCurrABHi[iSpecies], fTemp);
  }
  if (fA < 0) fA = 0;
  if (fA > 1) fA = 1;

  //----- The rest of it ----------------------------------------------------//
  fShifted = fLTM + mp_fCurrC[iSpecies];
  fCurrent = 0;
  fTemp = m_fCurrTemp - fShifted;
  fTemp *= fTemp;
  if (m_fCurrTemp < fShifted) {
    fCurrent = fA * pow(mp_fCurrBLo[iSpecies], fTemp);
  } else {
    fCurrent = fA * pow(mp_fCurrBHi[iSpecies], fTemp);
  }

  //-------------------------------------------------------------------------//
  // Previous year portion
  //-------------------------------------------------------------------------//

  //----- Calculate "a" -----------------------------------------------------//
  fA = 0;
  fTemp = fLTM - mp_fPrevAC[iSpecies];
  fTemp *= fTemp;
  if (fLTM < mp_fPrevAC[iSpecies]) {
    fA = mp_fPrevAA[iSpecies] * pow(mp_fPrevABLo[iSpecies], fTemp);
  } else {
    fA = mp_fPrevAA[iSpecies] * pow(mp_fPrevABHi[iSpecies], fTemp);
  }
  if (fA < 0) fA = 0;
  if (fA > 1) fA = 1;

  //----- The rest of it ----------------------------------------------------//
  fShifted = fLTM + mp_fPrevC[iSpecies];
  fPrev = 0;
  fTemp = m_fPrevTemp - fShifted;
  fTemp *= fTemp;
  if (m_fPrevTemp < fShifted) {
    fPrev = fA * pow(mp_fPrevBLo[iSpecies], fTemp);
  } else {
    fPrev = fA * pow(mp_fPrevBHi[iSpecies], fTemp);
  }

  fEffect = fPrev + fCurrent;
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

///////////////////////////////////////////////////////////////////////////////
// PreCalcs
///////////////////////////////////////////////////////////////////////////////
void clTemperatureEffectDoubleLocalDiff::PreCalcs(clPlot *p_oPlot) {
  m_fPrevTemp = m_fCurrTemp;
  m_fCurrTemp = p_oPlot->GetMeanAnnualTemp();
}

///////////////////////////////////////////////////////////////////////////////
// DoSetup
///////////////////////////////////////////////////////////////////////////////
void clTemperatureEffectDoubleLocalDiff::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values

  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fCurrAA = new double[iNumTotalSpecies];
  mp_fCurrABLo = new double[iNumTotalSpecies];
  mp_fCurrABHi = new double[iNumTotalSpecies];
  mp_fCurrAC = new double[iNumTotalSpecies];
  mp_fCurrBLo = new double[iNumTotalSpecies];
  mp_fCurrBHi = new double[iNumTotalSpecies];
  mp_fCurrC = new double[iNumTotalSpecies];
  mp_fPrevAA = new double[iNumTotalSpecies];
  mp_fPrevABLo = new double[iNumTotalSpecies];
  mp_fPrevABHi = new double[iNumTotalSpecies];
  mp_fPrevAC = new double[iNumTotalSpecies];
  mp_fPrevBLo = new double[iNumTotalSpecies];
  mp_fPrevBHi = new double[iNumTotalSpecies];
  mp_fPrevC = new double[iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //----- Temperature effect current a a ------------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffCurrAA",
      "ndltecaaVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrAA[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current a b lo ---------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffCurrABLo",
      "ndltecablVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrABLo[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current a b hi ---------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffCurrABHi",
      "ndltecabhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrABHi[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current a c ------------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffCurrAC",
      "ndltecacVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrAC[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current b lo -----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffCurrBLo",
      "ndltecblVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrBLo[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current b hi -----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffCurrBHi",
      "ndltecbhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrBHi[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current c --------------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffCurrC",
      "ndlteccVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrC[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous a a -----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffPrevAA",
      "ndltepaaVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevAA[p_fTempValues[i].code] = p_fTempValues[i].val;

  //----- Temperature effect previous a b lo --------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffPrevABLo",
      "ndltepablVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevABLo[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous a b hi --------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffPrevABHi",
      "ndltepabhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevABHi[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous a c -----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffPrevAC",
      "ndltepacVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevAC[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous b lo ----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffPrevBLo",
      "ndltepblVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevBLo[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous b hi ----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffPrevBHi",
      "ndltepbhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevBHi[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous c -------------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubLocTempEffPrevC",
      "ndltepcVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevC[p_fTempValues[i].code] = p_fTempValues[i].val;


  delete[] p_fTempValues;

  //----- Get the initial conditions value - will be previous for timestep 1 //
  clSimManager *p_oSimManager = p_oPop->GetSimManager();
  clPlot *p_oPlot = p_oSimManager->GetPlotObject();
  m_fCurrTemp = p_oPlot->GetMeanAnnualTemp();
}

