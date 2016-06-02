#include "TemperatureEffectDoubleLogistic.h"
#include "TreePopulation.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include <stddef.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clTemperatureEffectDoubleLogistic::clTemperatureEffectDoubleLogistic() {
  mp_fAl = NULL;
  mp_fBl = NULL;
  mp_fCl = NULL;
  mp_fAh = NULL;
  mp_fBh = NULL;
  mp_fCh = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clTemperatureEffectDoubleLogistic::~clTemperatureEffectDoubleLogistic() {
  delete[] mp_fAl;
  delete[] mp_fBl;
  delete[] mp_fCl;
  delete[] mp_fAh;
  delete[] mp_fBh;
  delete[] mp_fCh;
}

////////////////////////////////////////////////////////////////////////////
// CalculateTemperatureEffect
////////////////////////////////////////////////////////////////////////////
double clTemperatureEffectDoubleLogistic::CalculateTemperatureEffect(clPlot *p_oPlot, int iSpecies) {
  double fEffect,
         fTemp = p_oPlot->GetMeanAnnualTemp() + 273.15;
  fEffect = (mp_fAl[iSpecies] + ((1-mp_fAl[iSpecies])/(1+pow(mp_fBl[iSpecies]/fTemp, mp_fCl[iSpecies])))) *
            (mp_fAh[iSpecies] + ((1-mp_fAh[iSpecies])/(1+pow(fTemp/mp_fBh[iSpecies], mp_fCh[iSpecies]))));
  if (fEffect < 0) fEffect = 0;
  if (fEffect > 1) fEffect = 1;
  return fEffect;
}

////////////////////////////////////////////////////////////////////////////
// DoSetup
////////////////////////////////////////////////////////////////////////////
void clTemperatureEffectDoubleLogistic::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fAl = new double[iNumTotalSpecies];
  mp_fBl = new double[iNumTotalSpecies];
  mp_fCl = new double[iNumTotalSpecies];
  mp_fAh = new double[iNumTotalSpecies];
  mp_fBh = new double[iNumTotalSpecies];
  mp_fCh = new double[iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Temperature effect al
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogTempEffAl",
      "ndltealVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fAl[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Temperature effect bl
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogTempEffBl",
      "ndlteblVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fBl[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Temperature effect cl
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogTempEffCl",
      "ndlteclVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCl[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Temperature effect ah
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogTempEffAh",
      "ndlteahVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fAh[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Temperature effect bh
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogTempEffBh",
      "ndltebhVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fBh[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Temperature effect ch
  FillSpeciesSpecificValue( p_oElement, "nciDoubLogTempEffCh",
      "ndltechVal", p_fTempValues, iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fCh[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;
}

