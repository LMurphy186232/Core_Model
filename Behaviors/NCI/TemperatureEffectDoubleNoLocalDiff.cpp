#include "TemperatureEffectDoubleNoLocalDiff.h"
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
clTemperatureEffectDoubleNoLocalDiff::clTemperatureEffectDoubleNoLocalDiff() {
  m_fPrevTemp = 0;
  m_fCurrTemp = 0;
  mp_fCurrA = NULL;
  mp_fCurrBLo = NULL;
  mp_fCurrBHi = NULL;
  mp_fCurrC = NULL;
  mp_fPrevA = NULL;
  mp_fPrevBLo = NULL;
  mp_fPrevBHi = NULL;
  mp_fPrevC = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clTemperatureEffectDoubleNoLocalDiff::~clTemperatureEffectDoubleNoLocalDiff() {
  delete [] mp_fCurrA;
  delete [] mp_fCurrBLo;
  delete [] mp_fCurrBHi;
  delete [] mp_fCurrC;
  delete [] mp_fPrevA;
  delete [] mp_fPrevBLo;
  delete [] mp_fPrevBHi;
  delete [] mp_fPrevC;
}

////////////////////////////////////////////////////////////////////////////
// CalculateTemperatureEffect
////////////////////////////////////////////////////////////////////////////
double clTemperatureEffectDoubleNoLocalDiff::CalculateTemperatureEffect(clPlot *p_oPlot, int iSpecies) {
  double fEffect, fPrev, fCurrent, fTemp;

  //----- Current year portion ----------------------------------------------//
  fCurrent = 0;
  fTemp = m_fCurrTemp - mp_fCurrC[iSpecies];
  fTemp *= fTemp;
  if (m_fCurrTemp < mp_fCurrC[iSpecies]) {
    fCurrent = mp_fCurrA[iSpecies] * pow(mp_fCurrBLo[iSpecies], fTemp);
  } else {
    fCurrent = mp_fCurrA[iSpecies] * pow(mp_fCurrBHi[iSpecies], fTemp);
  }

  //----- Previous year portion ---------------------------------------------//
  fPrev = 0;
  fTemp = m_fPrevTemp - mp_fPrevC[iSpecies];
  fTemp *= fTemp;
  if (m_fPrevTemp < mp_fPrevC[iSpecies]) {
    fPrev = mp_fPrevA[iSpecies] * pow(mp_fPrevBLo[iSpecies], fTemp);
  } else {
    fPrev = mp_fPrevA[iSpecies] * pow(mp_fPrevBHi[iSpecies], fTemp);
  }

  fEffect = fPrev + fCurrent;
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

///////////////////////////////////////////////////////////////////////////////
// PreCalcs
///////////////////////////////////////////////////////////////////////////////
void clTemperatureEffectDoubleNoLocalDiff::PreCalcs(clPlot *p_oPlot) {
  m_fPrevTemp = m_fCurrTemp;
  m_fCurrTemp = p_oPlot->GetMeanAnnualTemp();
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clTemperatureEffectDoubleNoLocalDiff::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values

  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fCurrA = new double[iNumTotalSpecies];
  mp_fCurrBLo = new double[iNumTotalSpecies];
  mp_fCurrBHi = new double[iNumTotalSpecies];
  mp_fCurrC = new double[iNumTotalSpecies];
  mp_fPrevA = new double[iNumTotalSpecies];
  mp_fPrevBLo = new double[iNumTotalSpecies];
  mp_fPrevBHi = new double[iNumTotalSpecies];
  mp_fPrevC = new double[iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //----- Temperature effect current a ------------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocTempEffCurrA",
      "ndnltecaVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrA[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current b lo ---------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocTempEffCurrBLo",
      "ndnltecblVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrBLo[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current b hi ---------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocTempEffCurrBHi",
      "ndnltecbhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrBHi[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect current c ------------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocTempEffCurrC",
      "ndnlteccVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCurrC[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous a -----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocTempEffPrevA",
      "ndnltepaVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevA[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous b lo --------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocTempEffPrevBLo",
      "ndnltepblVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevBLo[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous b hi --------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocTempEffPrevBHi",
      "ndnltepbhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevBHi[p_fTempValues[i].code] = p_fTempValues[i].val;


  //----- Temperature effect previous c -----------------------------------//
  FillSpeciesSpecificValue( p_oElement, "nciDoubNoLocTempEffPrevC",
      "ndnltepcVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fPrevC[p_fTempValues[i].code] = p_fTempValues[i].val;


  delete[] p_fTempValues;

  //----- Get the initial conditions value - will be previous for timestep 1 //
  clSimManager *p_oSimManager = p_oPop->GetSimManager();
  clPlot *p_oPlot = p_oSimManager->GetPlotObject();
  m_fCurrTemp = p_oPlot->GetMeanAnnualTemp();
}

