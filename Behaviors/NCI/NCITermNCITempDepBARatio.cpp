#include "NCITermNCITempDepBARatio.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "BehaviorBase.h"
#include "Plot.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include <sstream>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCITermNCITempDepBARatio::clNCITermNCITempDepBARatio(bool bUseDefaultBA) : clNCITermNCIBARatio(bUseDefaultBA) {
  mp_fXa = NULL;
  mp_fX0 = NULL;
  mp_fXb = NULL;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clNCITermNCITempDepBARatio::~clNCITermNCITempDepBARatio() {
  int i;

  if (mp_fXa)
    for (i = 0; i < m_iNumTotalSpecies; i++)
      delete[] mp_fXa[i];
  delete[] mp_fXa;

  if (mp_fX0)
    for (i = 0; i < m_iNumTotalSpecies; i++)
      delete[] mp_fX0[i];
  delete[] mp_fX0;

  if (mp_fXb)
    for (i = 0; i < m_iNumTotalSpecies; i++)
      delete[] mp_fXb[i];
  delete[] mp_fXb;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateNCITerm
//////////////////////////////////////////////////////////////////////////////
void clNCITermNCITempDepBARatio::PreCalcs( clTreePopulation *p_oPop ) {
  float fTemp = mp_oPlot->GetMeanAnnualTemp() + 273.15, f;
  int iSpecies, iNeighSpecies, i;

  // Calculate the new lambdas
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    iSpecies = mp_iWhatSpecies[i];
    for (iNeighSpecies = 0; iNeighSpecies < m_iNumTotalSpecies; iNeighSpecies++) {
      f = (fTemp - mp_fX0[iSpecies][iNeighSpecies])/mp_fXb[iSpecies][iNeighSpecies];
      f *= f;
      mp_fLambda[iSpecies][iNeighSpecies] = mp_fXa[iSpecies][iNeighSpecies]*exp(-0.5*f);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clNCITermNCITempDepBARatio::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  floatVal * p_fTempValues; //for getting species-specific values
  std::stringstream sLabel;
  int i, j;

  m_iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies();
  mp_iWhatSpecies = new short int[m_iNumBehaviorSpecies];
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_iWhatSpecies[i] = p_oNCI->GetBehaviorSpecies(i);
  }

  //Get a pointer to the plot object
  mp_oPlot = p_oNCI->GetSimManager()->GetPlotObject();

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
  m_fMinSaplingHeight = 50;
  //Get the minimum sapling height
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight) {
      m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
    }
  }

  mp_fAlpha = new float[m_iNumTotalSpecies];
  mp_fBeta = new float[m_iNumTotalSpecies];
  mp_fMinimumNeighborDBH = new float[m_iNumTotalSpecies];
  mp_fXa = new float*[m_iNumTotalSpecies];
  mp_fX0 = new float*[m_iNumTotalSpecies];
  mp_fXb = new float*[m_iNumTotalSpecies];
  mp_fLambda = new float*[m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_fXa[i] = new float[m_iNumTotalSpecies];
    mp_fX0[i] = new float[m_iNumTotalSpecies];
    mp_fXb[i] = new float[m_iNumTotalSpecies];
    mp_fLambda[i] = new float[m_iNumTotalSpecies];
    for (j = 0; j < m_iNumTotalSpecies; j++) {
      mp_fLambda[i][j] = -1;
    }
  }

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Max adult crowding radius
  FillSingleValue( p_oElement, "nciMaxAdultCrowdingRadius", &m_fMaxAdultRadius, true );

  //Max sapling crowding radius
  FillSingleValue( p_oElement, "nciMaxSaplingCrowdingRadius", &m_fMaxSaplingRadius, true);

  m_fMaxCrowdingRadius = m_fMaxAdultRadius > m_fMaxSaplingRadius ? m_fMaxAdultRadius : m_fMaxSaplingRadius;

  if (m_bUseDefaultBA) {
    //Get the default DBH
    FillSingleValue(p_oElement, "nciBADefaultDBH", &m_fDefaultBA, true);
    //Calculate the BA
    m_fDefaultBA = clModelMath::CalculateBasalArea(m_fDefaultBA);
  }

  //NCI DBH adjustor
  FillSingleValue(p_oElement, "nciDbhAdjustor", & m_fDbhAdjustor, true);

  //Neighbor DBH effect (alpha)
  FillSpeciesSpecificValue(p_oElement, "nciAlpha", "naVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fAlpha[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Neighbor distance effect (beta)
  FillSpeciesSpecificValue(p_oElement, "nciBeta", "nbVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fBeta[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Lambda Xa
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    sLabel << "nci" << p_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambdaXa";
    FillSpeciesSpecificValue(p_oElement, sLabel.str(), "nlXaVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    sLabel.str("");
    for ( j = 0; j < m_iNumBehaviorSpecies; j++ )      {
      mp_fXa[p_fTempValues[j].code][i] = p_fTempValues[j].val;
    }
  }

  //Lambda X0
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    sLabel << "nci" << p_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambdaX0";
    FillSpeciesSpecificValue(p_oElement, sLabel.str(), "nlX0Val",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    sLabel.str("");
    for ( j = 0; j < m_iNumBehaviorSpecies; j++ )      {
      mp_fX0[p_fTempValues[j].code][i] = p_fTempValues[j].val;
    }
  }

  //Lambda Xb
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    sLabel << "nci" << p_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambdaXb";
    FillSpeciesSpecificValue(p_oElement, sLabel.str(), "nlXbVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    sLabel.str("");
    for ( j = 0; j < m_iNumBehaviorSpecies; j++ )      {
      mp_fXb[p_fTempValues[j].code][i] = p_fTempValues[j].val;
    }
  }

  //Minimum neighbor DBH
  FillSpeciesSpecificValue( p_oElement, "nciMinNeighborDBH", "nmndVal",
      mp_fMinimumNeighborDBH, p_oPop, true);

  delete[] p_fTempValues;

  //Make sure that the max radius of neighbor effects is > 0
  if (m_fMaxCrowdingRadius < 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clNCITermNCIBARatio::DoSetup";
    stcErr.sMoreInfo = "All values for NCI max crowding radius must be greater than 0.";
    throw( stcErr );
  }

  for (i = 0; i < m_iNumTotalSpecies; i++) {
    //Make sure that the minimum neighbor DBH is not negative
    if (0 > mp_fMinimumNeighborDBH[i]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCITermTempDepBARatio::DoSetup";
      stcErr.sMoreInfo = "Minimum neighbor DBH for NCI cannot be less than 0.";
      throw( stcErr );
    }
  }
}
