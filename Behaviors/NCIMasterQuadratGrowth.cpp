//---------------------------------------------------------------------------
#include "NCIMasterQuadratGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include "Allometry.h"
#include "Grid.h"

#include "NCI/CrowdingEffectBase.h"
#include "NCI/DamageEffectBase.h"
#include "NCI/NCITermBase.h"
#include "NCI/ShadingEffectBase.h"
#include "NCI/SizeEffectBase.h"
#include "NCI/TemperatureEffectBase.h"
#include "NCI/PrecipitationEffectBase.h"
#include "NCI/InfectionEffectBase.h"
#include "NCI/NitrogenEffectBase.h"

#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIMasterQuadratGrowth::clNCIMasterQuadratGrowth(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clGrowthBase(p_oSimManager), clNCIBehaviorBase() {

  //Set namestring
  m_sNameString = "nciquadratgrowthshell";
  m_sXMLRoot = "NCIMasterQuadratGrowth";

  //Version 3
  m_fVersionNumber = 1.0;
  m_fMinimumVersionNumber = 1.0;

  //Null out our pointers
  mp_fMaxPotentialValue = NULL;
  mp_fRandParameter = NULL;
  mp_fRandInt = NULL;
  mp_fRandSigma = NULL;
  mp_iGridGrowthCodes = NULL;
  mp_oGrid = NULL;

  m_iStochasticGrowthMethod = deterministic_pdf;
  Adjust = NULL;

  m_iNumTotalSpecies = 0;

}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clNCIMasterQuadratGrowth::~clNCIMasterQuadratGrowth() {

  delete[] mp_fMaxPotentialValue;
  delete[] mp_fRandParameter;
  delete[] mp_iGridGrowthCodes;
  delete[] mp_fRandInt;
  delete[] mp_fRandSigma;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterQuadratGrowth::DoShellSetup(xercesc::DOMDocument * p_oDoc) {
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  doubleVal * p_fTempValues; //for getting species-specific values
  int iTemp;
  short int i; //loop counters

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Maximum potential growth
  mp_fMaxPotentialValue = new double[m_iNumTotalSpecies];
  FillSpeciesSpecificValue( p_oElement, "gr_nciMaxPotentialGrowth", "gr_nmpgVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fMaxPotentialValue[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Get the type of growth stochasticity desired
  FillSingleValue(p_oElement, "gr_stochGrowthMethod", &iTemp, true);

  //Make sure the value is valid
  if (deterministic_pdf == iTemp) {

    m_iStochasticGrowthMethod = deterministic_pdf;
    Adjust = & clNCIMasterQuadratGrowth::DeterministicAdjust;

  } else if (lognormal_pdf == iTemp) {

    m_iStochasticGrowthMethod = lognormal_pdf;
    Adjust = & clNCIMasterQuadratGrowth::LognormalAdjust;

  } else if (normal_pdf == iTemp) {

    m_iStochasticGrowthMethod = normal_pdf;
    Adjust = & clNCIMasterQuadratGrowth::NormalAdjust;

  } else if (heteroscedastic_normal_pdf == iTemp) {
    Adjust = & clNCIMasterQuadratGrowth::HeteroscedasticNormalAdjust;
    m_iStochasticGrowthMethod = heteroscedastic_normal_pdf;

  } else {
    modelErr stcErr;
    stcErr.sFunction = "clNCIMasterQuadratGrowth::DoShellSetup";
    std::stringstream s;
    s << "Unrecognized value for gr_stochGrowthMethod: " << iTemp;
    stcErr.sMoreInfo = s.str();
    stcErr.iErrorCode = BAD_DATA;
    throw(stcErr);
  }

  //If lognormal or normal, populate the parameters array
  if (lognormal_pdf == m_iStochasticGrowthMethod ||
      normal_pdf == m_iStochasticGrowthMethod) {

    mp_fRandParameter = new double[m_iNumTotalSpecies];
    FillSpeciesSpecificValue(p_oElement, "gr_standardDeviation", "gr_sdVal",
          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fRandParameter[p_fTempValues[i].code] = p_fTempValues[i].val;

  }

  //If heteroscedastic normal, get intercept and sigma
  if (heteroscedastic_normal_pdf == m_iStochasticGrowthMethod) {
    mp_fRandInt   = new double[m_iNumTotalSpecies];
    mp_fRandSigma = new double[m_iNumTotalSpecies];

    FillSpeciesSpecificValue(p_oElement, "gr_hetNormInt", "gr_hniVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fRandInt[p_fTempValues[i].code] = p_fTempValues[i].val;

    FillSpeciesSpecificValue(p_oElement, "gr_hetNormSigma", "gr_hnsVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fRandSigma[p_fTempValues[i].code] = p_fTempValues[i].val;
  }

  delete[] p_fTempValues;

  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    //Make sure that the maximum growth for each species is > 0
    if (mp_fMaxPotentialValue[mp_iWhatSpecies[i]] <= 0) {
      modelErr err;
      err.sFunction = "clNCIMasterQuadratGrowth::ValidateData";
      err.iErrorCode = BAD_DATA;
      err.sMoreInfo = "All values for max potential growth must be greater than 0.";
      throw(err);
    }
  }

  ReadParameterFile(p_oElement, p_oPop, this, false);
  SetupGrid(p_oPop);


}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clNCIMasterQuadratGrowth::SetNameData(std::string sNameString) {

  //Check the string passed and set the flags accordingly
  if (sNameString.compare("NCIMasterQuadratGrowth") == 0 )
  {
    m_iGrowthMethod = diameter_auto;
  }
  else if (sNameString.compare("NCIMasterQuadratGrowth diam only") == 0 )
  {
    m_iGrowthMethod = diameter_only;
  }
  else
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Unrecognized behavior name \"" << sNameString << "\".";
    stcErr.sMoreInfo = s.str();
    stcErr.sFunction = "clNCIMasterQuadratGrowth::SetNameData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcGrowthValue
////////////////////////////////////////////////////////////////////////////
float clNCIMasterQuadratGrowth::CalcDiameterGrowthValue(clTree * p_oTree,
    clTreePopulation * p_oPop, float fHeightGrowth) {

  float fX, fY, fGrowth;

  //Get this tree's location
  p_oTree->GetValue(p_oPop->GetXCode(p_oTree->GetSpecies(), p_oTree->GetType()), &fX);
  p_oTree->GetValue(p_oPop->GetYCode(p_oTree->GetSpecies(), p_oTree->GetType()), &fY);
  mp_oGrid->GetValueAtPoint(fX, fY, mp_iGridGrowthCodes[p_oTree->GetSpecies()], &fGrowth);

  if (fGrowth < 0 ) {
    modelErr stcErr;
    stcErr.sMoreInfo = "Panic: expected growth not calculated.";
    stcErr.sFunction = "clNCIMasterQuadratGrowth::CalcDiameterGrowthValue" ;
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }

  //Adjust stochastically (if deterministic, we'll get the same
  //number back
  fGrowth = (*this.*Adjust) (fGrowth, p_oTree->GetSpecies());

  return fGrowth;
}

////////////////////////////////////////////////////////////////////////////
// PreGrowthCalcs
////////////////////////////////////////////////////////////////////////////
void clNCIMasterQuadratGrowth::PreGrowthCalcs(clTreePopulation * p_oPop) {
  float *p_fTempEffect = NULL,
        *p_fPrecipEffect = NULL,
        *p_fNEffect = NULL;
  try
  {

    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clNCITermBase::ncivals nci;
    float fCrowdingEffect, //tree's crowding effect
    fX, fY,
    fGrowth,
    fInfectionEffect; //tree's infection effect
    int iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    short int iSpecies, iSp, //type and species of a tree
    iNumXCells = mp_oGrid->GetNumberXCells(),
    iNumYCells = mp_oGrid->GetNumberYCells(),
    //iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
    i, iX, iY;

    //Reset the values in the growth grid to -1
    //No - if there are multiple copies of this behavior, they'll overwrite
    //each other
    //fGrowth = -1.0;
    //for (iX = 0; iX < iNumXCells; iX++)
    //  for (iY = 0; iY < iNumYCells; iY++) {
    //    for (iSpecies = 0; iSpecies < iNumTotalSpecies; iSpecies++)
    //      mp_oGrid->SetValueOfCell(iX, iY, mp_iGridGrowthCodes[iSpecies], fGrowth);
    //  }

    p_fPrecipEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fTempEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fNEffect = new float[p_oPop->GetNumberOfSpecies()];
    //Calculate climate effects for all species
    for (i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fPrecipEffect[mp_iWhatSpecies[i]] = mp_oPrecipEffect->CalculatePrecipitationEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fTempEffect[mp_iWhatSpecies[i]] = mp_oTempEffect->CalculateTemperatureEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fNEffect[mp_iWhatSpecies[i]] = mp_oNEffect->CalculateNitrogenEffect(p_oPlot, mp_iWhatSpecies[i]);
    }

    //Pre-calcs for other effects
    mp_oCrowdingEffect->PreCalcs(p_oPop);
    mp_oDamageEffect->PreCalcs(p_oPop);
    mp_oNCITerm->PreCalcs(p_oPop);
    mp_oShadingEffect->PreCalcs(p_oPop);
    mp_oSizeEffect->PreCalcs(p_oPop);
    mp_oInfectionEffect->PreCalcs(p_oPop);

    // Loop through the grid
    for (iX = 0; iX < iNumXCells; iX++) {
      for (iY = 0; iY < iNumYCells; iY++)
      {
        mp_oGrid->GetPointOfCell(iX, iY, &fX, &fY);

        //Do only behavior species
        for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++)
        {
          iSpecies = mp_iWhatSpecies[iSp];

          //Get NCI
          nci = mp_oNCITerm->CalculateNCITerm(NULL, p_oPop, p_oPlot, fX, fY, iSpecies);

          //Get the infection effect
          fInfectionEffect = mp_oInfectionEffect->CalculateInfectionEffect(NULL);

          //Get the crowding effect
          fCrowdingEffect = mp_oCrowdingEffect->CalculateCrowdingEffect(NULL, 0, nci, iSpecies);

          //Calculate actual growth in cm/yr
          fGrowth = mp_fMaxPotentialValue[iSpecies] * fCrowdingEffect *
              p_fPrecipEffect[iSpecies] * p_fTempEffect[iSpecies] *
              p_fNEffect[iSpecies] * fInfectionEffect;

          //Transform to per timestep
          fGrowth *= iNumberYearsPerTimestep;

          //Assign the growth back to the grid
          mp_oGrid->SetValueOfCell(iX, iY, mp_iGridGrowthCodes[iSpecies], fGrowth );
        }
      }
    }

    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;

  }
  catch ( modelErr & err )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;
    throw( err );
  }
  catch ( ... )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clNCIMasterQuadratGrowth::PreCalcGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// DeterministicAdjust()
/////////////////////////////////////////////////////////////////////////////
float clNCIMasterQuadratGrowth::DeterministicAdjust(float fNumber, int iSpecies) {
  return fNumber;
}

//////////////////////////////////////////////////////////////////////////////
// NormalAdjust()
/////////////////////////////////////////////////////////////////////////////
float clNCIMasterQuadratGrowth::NormalAdjust(float fNumber, int iSpecies) {
  float fReturn = fNumber + clModelMath::NormalRandomDraw(mp_fRandParameter[iSpecies]);
  if (fReturn < 0) fReturn = 0;
  return fReturn;
}


//////////////////////////////////////////////////////////////////////////////
// LognormalAdjust()
/////////////////////////////////////////////////////////////////////////////
float clNCIMasterQuadratGrowth::LognormalAdjust(float fNumber, int iSpecies) {
  float fReturn = clModelMath::LognormalRandomDraw(fNumber, mp_fRandParameter[iSpecies]);
  if (fReturn < 0) fReturn = 0;
  return fReturn;
}


//////////////////////////////////////////////////////////////////////////////
// HeteroscedasticNormalAdjust()
/////////////////////////////////////////////////////////////////////////////
float clNCIMasterQuadratGrowth::HeteroscedasticNormalAdjust(float fNumber, int iSpecies) {
  //Calculate standard deviation in cm for parameters in mm
  float fSD = mp_fRandInt[iSpecies] + pow((fNumber*10.0), mp_fRandSigma[iSpecies]);
  fSD /= 10.0;
  float fReturn = fNumber + clModelMath::NormalRandomDraw(fSD);
  if (fReturn < 0) fReturn = 0;
  return fReturn;
}


/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clNCIMasterQuadratGrowth::SetupGrid(clTreePopulation *p_oPop)
{
  std::stringstream sLabel;
  short int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
      i; //loop counter

  //Declare the arrays for our grid codes
  mp_iGridGrowthCodes = new short int[iNumTotalSpecies];

  //Check to see if this grid has already been created
  mp_oGrid = mp_oSimManager->GetGridObject("NCI Quadrat Growth");

  if (NULL == mp_oGrid) {

    //Create the grid with five float data members for each species
    mp_oGrid = mp_oSimManager->CreateGrid( "NCI Quadrat Growth",
        0,                //number of ints
        iNumTotalSpecies, //number of floats
        0,                //number of chars
        0);               //number of bools

    //Register the data member called "growth_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "growth_" << i;
      mp_iGridGrowthCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
  }
  else {
    //Grid already exists - get the codes
    //Get the data member called "growth_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "growth_" << i;
      mp_iGridGrowthCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iGridGrowthCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clNCIMasterQuadratGrowth::SetupGrid";
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str()
            << "\" member of the \"NCI Quadrat Growth\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }
  }
}
