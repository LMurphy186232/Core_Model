//---------------------------------------------------------------------------
#include <stdio.h>
#include <sstream>
#include "Allometry.h"
#include "SimManager.h"
#include "Plot.h"
#include "ModelMath.h"
#include "ParsingFunctions.h"

using namespace whyDead;
using namespace std;
////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clAllometry::clAllometry(clTreePopulation *p_oPop) {

  m_oPop = p_oPop;

  mp_AdultHeight = NULL;
  mp_AdultDiam = NULL;
  mp_AdultCrownRad = NULL;
  mp_AdultCrownDepth = NULL;
  mp_SaplingHeight = NULL;
  mp_SaplingDiam = NULL;
  mp_SaplingCrownRad = NULL;
  mp_SaplingCrownDepth = NULL;
  mp_SeedlingHeight = NULL;
  mp_SeedlingDiam = NULL;
  mp_fMaxTreeHeight = NULL;
  mp_fAsympCrownRad = NULL;
  mp_fCrownRadExp = NULL;
  mp_fMaxCrownRad = NULL;
  mp_fDbhToDiam10Slope = NULL;
  mp_fDbhToDiam10Intercept = NULL;
  mp_fAsympCrownDepth = NULL;
  mp_fCrownDepthExp = NULL;
  mp_fSlopeHeightDiam10 = NULL;
  mp_fSlopeAsympHeight = NULL;
  mp_fAdultLinearSlope = NULL;
  mp_fAdultLinearIntercept = NULL;
  mp_fAdultReverseLinearSlope = NULL;
  mp_fAdultReverseLinearIntercept = NULL;
  mp_fSaplingLinearSlope = NULL;
  mp_fSaplingLinearIntercept = NULL;
  mp_fSaplingReverseLinearSlope = NULL;
  mp_fSaplingReverseLinearIntercept = NULL;
  mp_fSeedlingLinearSlope = NULL;
  mp_fSeedlingLinearIntercept = NULL;
  mp_fSeedlingReverseLinearSlope = NULL;
  mp_fSeedlingReverseLinearIntercept = NULL;
  mp_fCRCrownRadIntercept = NULL;
  mp_fCRAsympCrownRad = NULL;
  mp_fCRCrownRadShape1 = NULL;
  mp_fCRCrownRadShape2 = NULL;
  mp_fCRCrownHtIntercept = NULL;
  mp_fCRAsympCrownHt = NULL;
  mp_fCRCrownHtShape1 = NULL;
  mp_fCRCrownHtShape2 = NULL;
  mp_fPowerA = NULL;
  mp_fPowerExpB = NULL;
  mp_fNonSpatDensDepInstCHA = NULL;
  mp_fNonSpatDensDepInstCHB = NULL;
  mp_fNonSpatDensDepInstCHC = NULL;
  mp_fNonSpatDensDepInstCHD = NULL;
  mp_fNonSpatDensDepInstCHE = NULL;
  mp_fNonSpatDensDepInstCHF = NULL;
  mp_fNonSpatDensDepInstCHG = NULL;
  mp_fNonSpatDensDepInstCHH = NULL;
  mp_fNonSpatDensDepInstCHI = NULL;
  mp_fNonSpatDensDepInstCHJ = NULL;
  mp_fNonSpatExpDensDepCRD1 = NULL;
  mp_fNonSpatExpDensDepCRA = NULL;
  mp_fNonSpatExpDensDepCRB = NULL;
  mp_fNonSpatExpDensDepCRC = NULL;
  mp_fNonSpatExpDensDepCRD = NULL;
  mp_fNonSpatExpDensDepCRE = NULL;
  mp_fNonSpatExpDensDepCRF = NULL;
  mp_fNonSpatDensDepInstCRA = NULL;
  mp_fNonSpatDensDepInstCRB = NULL;
  mp_fNonSpatDensDepInstCRC = NULL;
  mp_fNonSpatDensDepInstCRD = NULL;
  mp_fNonSpatDensDepInstCRE = NULL;
  mp_fNonSpatDensDepInstCRF = NULL;
  mp_fNonSpatDensDepInstCRG = NULL;
  mp_fNonSpatDensDepInstCRH = NULL;
  mp_fNonSpatDensDepInstCRI = NULL;
  mp_fNonSpatDensDepInstCRJ = NULL;
  mp_fNonSpatLogDensDepCHA = NULL;
  mp_fNonSpatLogDensDepCHB = NULL;
  mp_fNonSpatLogDensDepCHC = NULL;
  mp_fNonSpatLogDensDepCHD = NULL;
  mp_fNonSpatLogDensDepCHE = NULL;
  mp_fNonSpatLogDensDepCHF = NULL;
  mp_fNonSpatLogDensDepCHG = NULL;
  mp_fNCIMaxCrownRadius = NULL;
  mp_fNCICRLambda = NULL;
  mp_fNCICRAlpha = NULL;
  mp_fNCICRBeta = NULL;
  mp_fNCICRGamma = NULL;
  mp_fNCICRMaxCrowdingRadius = NULL;
  mp_fNCICRN = NULL;
  mp_fNCICRD = NULL;
  mp_fNCICRMinNeighborDBH = NULL;
  mp_fNCIMaxCrownDepth = NULL;
  mp_fNCICDLambda = NULL;
  mp_fNCICDAlpha = NULL;
  mp_fNCICDBeta = NULL;
  mp_fNCICDGamma = NULL;
  mp_fNCICDMaxCrowdingRadius = NULL;
  mp_fNCICDN = NULL;
  mp_fNCICDD = NULL;
  mp_fNCICDMinNeighborDBH = NULL;

  m_fPlotDensity = -1;
  m_fPlotBasalArea = -1;

  m_iNumSpecies = 0;
  m_fMaxCrownRad = 0;
  m_fMinSaplingHeight = 0;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clAllometry::~clAllometry() {
  delete[] mp_AdultHeight;
  delete[] mp_AdultDiam;
  delete[] mp_AdultCrownRad;
  delete[] mp_AdultCrownDepth;
  delete[] mp_SaplingHeight;
  delete[] mp_SaplingDiam;
  delete[] mp_SaplingCrownRad;
  delete[] mp_SaplingCrownDepth;
  delete[] mp_SeedlingHeight;
  delete[] mp_SeedlingDiam;
  delete[] mp_fMaxTreeHeight;
  delete[] mp_fAsympCrownRad;
  delete[] mp_fCrownRadExp;
  delete[] mp_fMaxCrownRad;
  delete[] mp_fDbhToDiam10Slope;
  delete[] mp_fDbhToDiam10Intercept;
  delete[] mp_fAsympCrownDepth;
  delete[] mp_fCrownDepthExp;
  delete[] mp_fSlopeHeightDiam10;
  delete[] mp_fSlopeAsympHeight;
  delete[] mp_fAdultLinearSlope;
  delete[] mp_fAdultLinearIntercept;
  delete[] mp_fAdultReverseLinearSlope;
  delete[] mp_fAdultReverseLinearIntercept;
  delete[] mp_fSaplingLinearSlope;
  delete[] mp_fSaplingLinearIntercept;
  delete[] mp_fSaplingReverseLinearSlope;
  delete[] mp_fSaplingReverseLinearIntercept;
  delete[] mp_fSeedlingLinearSlope;
  delete[] mp_fSeedlingLinearIntercept;
  delete[] mp_fSeedlingReverseLinearSlope;
  delete[] mp_fSeedlingReverseLinearIntercept;
  delete[] mp_fCRCrownRadIntercept;
  delete[] mp_fCRAsympCrownRad;
  delete[] mp_fCRCrownRadShape1;
  delete[] mp_fCRCrownRadShape2;
  delete[] mp_fCRCrownHtIntercept;
  delete[] mp_fCRAsympCrownHt;
  delete[] mp_fCRCrownHtShape1;
  delete[] mp_fCRCrownHtShape2;
  delete[] mp_fPowerA;
  delete[] mp_fPowerExpB;
  delete[] mp_fNonSpatDensDepInstCHA;
  delete[] mp_fNonSpatDensDepInstCHB;
  delete[] mp_fNonSpatDensDepInstCHC;
  delete[] mp_fNonSpatDensDepInstCHD;
  delete[] mp_fNonSpatDensDepInstCHE;
  delete[] mp_fNonSpatDensDepInstCHF;
  delete[] mp_fNonSpatDensDepInstCHG;
  delete[] mp_fNonSpatDensDepInstCHH;
  delete[] mp_fNonSpatDensDepInstCHI;
  delete[] mp_fNonSpatDensDepInstCHJ;
  delete[] mp_fNonSpatExpDensDepCRD1;
  delete[] mp_fNonSpatExpDensDepCRA;
  delete[] mp_fNonSpatExpDensDepCRB;
  delete[] mp_fNonSpatExpDensDepCRC;
  delete[] mp_fNonSpatExpDensDepCRD;
  delete[] mp_fNonSpatExpDensDepCRE;
  delete[] mp_fNonSpatExpDensDepCRF;
  delete[] mp_fNonSpatDensDepInstCRA;
  delete[] mp_fNonSpatDensDepInstCRB;
  delete[] mp_fNonSpatDensDepInstCRC;
  delete[] mp_fNonSpatDensDepInstCRD;
  delete[] mp_fNonSpatDensDepInstCRE;
  delete[] mp_fNonSpatDensDepInstCRF;
  delete[] mp_fNonSpatDensDepInstCRG;
  delete[] mp_fNonSpatDensDepInstCRH;
  delete[] mp_fNonSpatDensDepInstCRI;
  delete[] mp_fNonSpatDensDepInstCRJ;
  delete[] mp_fNonSpatLogDensDepCHA;
  delete[] mp_fNonSpatLogDensDepCHB;
  delete[] mp_fNonSpatLogDensDepCHC;
  delete[] mp_fNonSpatLogDensDepCHD;
  delete[] mp_fNonSpatLogDensDepCHE;
  delete[] mp_fNonSpatLogDensDepCHF;
  delete[] mp_fNonSpatLogDensDepCHG;

  if (mp_fNCICDLambda) {
    for (int i = 0; i < m_iNumSpecies; i++)
      delete[] mp_fNCICDLambda[i];
  }
  if (mp_fNCICRLambda) {
    for (int i = 0; i < m_iNumSpecies; i++)
      delete[] mp_fNCICRLambda[i];
  }
  delete[] mp_fNCIMaxCrownRadius;
  delete[] mp_fNCICRLambda;
  delete[] mp_fNCICRAlpha;
  delete[] mp_fNCICRBeta;
  delete[] mp_fNCICRGamma;
  delete[] mp_fNCICRMaxCrowdingRadius;
  delete[] mp_fNCICRN;
  delete[] mp_fNCICRD;
  delete[] mp_fNCICRMinNeighborDBH;
  delete[] mp_fNCIMaxCrownDepth;
  delete[] mp_fNCICDLambda;
  delete[] mp_fNCICDAlpha;
  delete[] mp_fNCICDBeta;
  delete[] mp_fNCICDGamma;
  delete[] mp_fNCICDMaxCrowdingRadius;
  delete[] mp_fNCICDN;
  delete[] mp_fNCICDD;
  delete[] mp_fNCICDMinNeighborDBH;
}

////////////////////////////////////////////////////////////////////////////
// GetData()
////////////////////////////////////////////////////////////////////////////
void clAllometry::GetData(DOMDocument * p_oDoc, clTreePopulation * p_oPop) {
  DOMElement * p_oElement = p_oDoc->getDocumentElement();
  int i;

  m_iNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare our arrays that we'll always need
  mp_fMaxTreeHeight = new double[m_iNumSpecies];
  mp_fDbhToDiam10Slope = new double[m_iNumSpecies];
  mp_fDbhToDiam10Intercept = new double[m_iNumSpecies];

  //Get all-species parameters
  FillSpeciesSpecificValue(p_oElement, "tr_canopyHeight", "tr_chVal",
      mp_fMaxTreeHeight, p_oPop, true);
  FillSpeciesSpecificValue(p_oElement, "tr_conversionDiam10ToDBH",
      "tr_cdtdVal", mp_fDbhToDiam10Slope, p_oPop, true);
  FillSpeciesSpecificValue(p_oElement, "tr_interceptDiam10ToDBH", "tr_idtdVal",
      mp_fDbhToDiam10Intercept, p_oPop, true);

  //If the value of the dbh to diam10 conversion factor is zero, throw error
  for (int i = 0; i < m_iNumSpecies; i++) {
    if (mp_fDbhToDiam10Slope[i] == 0) {
      modelErr stcErr;
      stcErr.sFunction = "clAllometry::GetData";
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Diameter at 10 cm to DBH conversion may not equal 0.";
      throw(stcErr);
    }
  }

  //Start by getting the allometry function for each species for each life
  //history stage
  SetupAdultHeightDiam(p_oElement);
  SetupAdultCrownDepth(p_oElement);
  SetupAdultCrownRadius(p_oElement);
  SetupSaplingHeightDiam(p_oElement);
  SetupSaplingCrownDepth(p_oElement);
  SetupSaplingCrownRadius(p_oElement);
  SetupSeedlingHeightDiam(p_oElement);

  // Calculate max canopy radius
  m_fMaxCrownRad = 0;
  float fTempRad = 0;
  for (i = 0; i < m_iNumSpecies; i++) {
    if (&clAllometry::CalcStandardSapAdultCrownRad == mp_AdultCrownRad[i]) {
      fTempRad = mp_fMaxCrownRad[i];
    } else if (&clAllometry::CalcChapRichSapAdultCrownRad == mp_AdultCrownRad[i]) {
      fTempRad = mp_fCRAsympCrownRad[i] + mp_fCRCrownRadIntercept[i];
    } else if (&clAllometry::CalcNonSpatDensDepExpAdultCrownRad == mp_AdultCrownRad[i]) {
      fTempRad = 1000;
    } else if (&clAllometry::CalcNCICrownRad == mp_AdultCrownRad[i]) {
      fTempRad = mp_fNCIMaxCrownRadius[i];
    }

    if (m_fMaxCrownRad < fTempRad)
      m_fMaxCrownRad = fTempRad;
  }
  for (i = 0; i < m_iNumSpecies; i++) {
    if (&clAllometry::CalcStandardSapAdultCrownRad == mp_SaplingCrownRad[i]) {
      fTempRad = mp_fMaxCrownRad[i];
    } else if (&clAllometry::CalcChapRichSapAdultCrownRad == mp_SaplingCrownRad[i]) {
      fTempRad = mp_fCRAsympCrownRad[i] + mp_fCRCrownRadIntercept[i];
    } else if (&clAllometry::CalcNCICrownRad == mp_SaplingCrownRad[i]) {
      fTempRad = mp_fNCIMaxCrownRadius[i];
    }

    if (m_fMaxCrownRad < fTempRad)
      m_fMaxCrownRad = fTempRad;
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcAdultHeight()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcAdultHeight(const float & fDbh, const int & iSpecies) {
  if (iSpecies >= m_iNumSpecies || iSpecies < 0) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcAdultHeight";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return (*this.*mp_AdultHeight[iSpecies])(fDbh, iSpecies);
}

////////////////////////////////////////////////////////////////////////////
// CalcAdultDbh()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcAdultDbh(float fHeight, const int & iSpecies) {
  if (iSpecies >= m_iNumSpecies || iSpecies < 0) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcAdultDbh";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  //If the height is greater than or equal to canopy height, throw bad data error
  if (fHeight >= mp_fMaxTreeHeight[iSpecies]) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcAdultDbh";
    stcErr.sMoreInfo = "Cannot set tree height greater than maximum canopy height for this species.";
    throw(stcErr);
  }
  return (*this.*mp_AdultDiam[iSpecies])(fHeight, iSpecies);
}

////////////////////////////////////////////////////////////////////////////
// CalcSaplingHeight()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcSaplingHeight(const float & fDbh, const int & iSpecies) {
  if (iSpecies >= m_iNumSpecies || iSpecies < 0) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcSaplingHeight";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return (*this.*mp_SaplingHeight[iSpecies])(fDbh, iSpecies);
}

////////////////////////////////////////////////////////////////////////////
// CalcSaplingDbh()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcSaplingDbh(float fHeight, const int & iSpecies) {
  if (iSpecies >= m_iNumSpecies || iSpecies < 0) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcSaplingDbh";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  //If the height is greater than or equal to canopy height, throw bad data error
  if (fHeight >= mp_fMaxTreeHeight[iSpecies]) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcSaplingDbh";
    stcErr.sMoreInfo = "Cannot set tree height greater than maximum canopy height for this species.";
    throw(stcErr);
  }
  return (*this.*mp_SaplingDiam[iSpecies])(fHeight, iSpecies);
}

////////////////////////////////////////////////////////////////////////////
// CalcAdultCrownRadius()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcAdultCrownRadius(clTree *p_oTree) {
  float fCR;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();
  p_oTree->GetValue(m_oPop->GetCrownRadiusCode(iSp, iTp), &fCR);
  if (fCR > 0)
    return fCR;
  fCR = (*this.*mp_AdultCrownRad[iSp])(p_oTree);
  p_oTree->SetValue(m_oPop->GetCrownRadiusCode(iSp, iTp), fCR);
  return fCR;
}

////////////////////////////////////////////////////////////////////////////
// CalcAdultCrownDepth()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcAdultCrownDepth(clTree *p_oTree) {
  float fCD;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();
  p_oTree->GetValue(m_oPop->GetCrownDepthCode(iSp, iTp), &fCD);
  if (fCD > 0)
    return fCD;
  fCD = (*this.*mp_AdultCrownDepth[iSp])(p_oTree);
  p_oTree->SetValue(m_oPop->GetCrownDepthCode(iSp, iTp), fCD);
  return fCD;
}

////////////////////////////////////////////////////////////////////////////
// CalcSaplingCrownRadius()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcSaplingCrownRadius(clTree *p_oTree) {
  float fCR;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();
  p_oTree->GetValue(m_oPop->GetCrownRadiusCode(iSp, iTp), &fCR);
  if (fCR > 0)
    return fCR;
  fCR = (*this.*mp_SaplingCrownRad[iSp])(p_oTree);
  p_oTree->SetValue(m_oPop->GetCrownRadiusCode(iSp, iTp), fCR);
  return fCR;
}

////////////////////////////////////////////////////////////////////////////
// CalcSaplingCrownDepth()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcSaplingCrownDepth(clTree *p_oTree) {
  float fCD;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();
  p_oTree->GetValue(m_oPop->GetCrownDepthCode(iSp, iTp), &fCD);
  if (fCD > 0)
    return fCD;
  fCD = (*this.*mp_SaplingCrownDepth[iSp])(p_oTree);
  p_oTree->SetValue(m_oPop->GetCrownDepthCode(iSp, iTp), fCD);
  return fCD;
}

////////////////////////////////////////////////////////////////////////////
// CalcSeedlingHeight()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcSeedlingHeight(float fDiam10, const int & iSpecies) {
  if (iSpecies >= m_iNumSpecies || iSpecies < 0) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcSeedlingHeight";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return (*this.*mp_SeedlingHeight[iSpecies])(fDiam10, iSpecies);
}

////////////////////////////////////////////////////////////////////////////
// CalcSeedlingDiam10()
////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcSeedlingDiam10(float fHeight, const int & iSpecies) {
  if (iSpecies >= m_iNumSpecies || iSpecies < 0) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcSeedlingDiam10";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  //If the height is greater than or equal to 30.1, throw bad data error
  if (fHeight >= 30.1) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::CalcSeedlingDiam10";
    stcErr.sMoreInfo = "Cannot set tree height greater than maximum tree height for this species.";
    throw(stcErr);
  }
  return (*this.*mp_SeedlingDiam[iSpecies])(fHeight, iSpecies);
}

////////////////////////////////////////////////////////////////////////////
// ConvertDiam10ToDbh()
////////////////////////////////////////////////////////////////////////////
float clAllometry::ConvertDiam10ToDbh(float fDiam10, const int & iSpecies) {
  if (iSpecies >= m_iNumSpecies || iSpecies < 0) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::ConvertDiam10ToDbh";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return (fDiam10 * mp_fDbhToDiam10Slope[iSpecies])
      + mp_fDbhToDiam10Intercept[iSpecies];
}

////////////////////////////////////////////////////////////////////////////
// ConvertDbhToDiam10()
////////////////////////////////////////////////////////////////////////////
float clAllometry::ConvertDbhToDiam10(const float & fDbh, const int & iSpecies) {
  if (iSpecies >= m_iNumSpecies || iSpecies < 0) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clAllometry::ConvertDbhToDiam10";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return (fDbh - mp_fDbhToDiam10Intercept[iSpecies])
      / mp_fDbhToDiam10Slope[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// GetMaxTreeHeight()
//////////////////////////////////////////////////////////////////////////////
double clAllometry::GetMaxTreeHeight(int iSpecies) {
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies) {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetMaxTreeHeight";
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return mp_fMaxTreeHeight[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// CalcStandardSapAdultHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcStandardSapAdultHeight(const float &fDbh,
    const int &iSpecies) {
  float fHeight = (1.35 + ((mp_fMaxTreeHeight[iSpecies] - 1.35) * (1.0 - exp(
      (double) (-mp_fSlopeAsympHeight[iSpecies] * fDbh)))));
  if (fHeight > 0.001)
    return fHeight;
  else
    return 0.001;
}

//////////////////////////////////////////////////////////////////////////////
// CalcLinearAdultHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcLinearAdultHeight(const float &fDbh, const int &iSpecies) {
  float fHeight = fDbh * mp_fAdultLinearSlope[iSpecies]
                                              + mp_fAdultLinearIntercept[iSpecies];
  if (fHeight > mp_fMaxTreeHeight[iSpecies])
    return mp_fMaxTreeHeight[iSpecies];
  if (fHeight <= 0.001)
    return 0.001;
  return fHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcReverseLinearAdultHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcReverseLinearAdultHeight(const float &fDbh,
    const int &iSpecies) {
  float fHeight = (fDbh - mp_fAdultReverseLinearIntercept[iSpecies])
          / mp_fAdultReverseLinearSlope[iSpecies];
  if (fHeight > mp_fMaxTreeHeight[iSpecies])
    return mp_fMaxTreeHeight[iSpecies];
  if (fHeight <= 0.001)
    return 0.001;
  return fHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcLinearSaplingHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcLinearSaplingHeight(const float &fDbh,
    const int &iSpecies) {
  float fHeight = fDbh * mp_fSaplingLinearSlope[iSpecies]
                                                + mp_fSaplingLinearIntercept[iSpecies];
  if (fHeight > mp_fMaxTreeHeight[iSpecies])
    return mp_fMaxTreeHeight[iSpecies];
  if (fHeight <= 0.001)
    return 0.001;
  return fHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcReverseLinearSaplingHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcReverseLinearSaplingHeight(const float &fDbh,
    const int &iSpecies) {
  float fHeight = (fDbh - mp_fSaplingReverseLinearIntercept[iSpecies])
          / mp_fSaplingReverseLinearSlope[iSpecies];
  if (fHeight > mp_fMaxTreeHeight[iSpecies])
    return mp_fMaxTreeHeight[iSpecies];
  if (fHeight <= 0.001)
    return 0.001;
  return fHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcStandardSeedlingHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcStandardSeedlingHeight(const float &fDiam10,
    const int &iSpecies) {
  float fHeight = 0.1 + 30 * (1.0 - exp(
      (double) (-mp_fSlopeHeightDiam10[iSpecies] * fDiam10)));
  if (fHeight > 0.001)
    return fHeight;
  else
    return 0.001;
}

//////////////////////////////////////////////////////////////////////////////
// CalcLinearSeedlingHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcLinearSeedlingHeight(const float &fDiam10,
    const int &iSpecies) {
  float fHeight = fDiam10 * mp_fSeedlingLinearSlope[iSpecies]
                                                    + mp_fSeedlingLinearIntercept[iSpecies];
  if (fHeight > mp_fMaxTreeHeight[iSpecies])
    return mp_fMaxTreeHeight[iSpecies];
  if (fHeight <= 0.001)
    return 0.001;
  return fHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcReverseLinearSeedlingHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcReverseLinearSeedlingHeight(const float &fDiam10,
    const int &iSpecies) {
  float fHeight = (fDiam10 - mp_fSeedlingReverseLinearIntercept[iSpecies])
          / mp_fSeedlingReverseLinearSlope[iSpecies];
  if (fHeight > mp_fMaxTreeHeight[iSpecies])
    return mp_fMaxTreeHeight[iSpecies];
  if (fHeight <= 0.001)
    return 0.001;
  return fHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcStandardSapAdultDbh()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcStandardSapAdultDbh(const float &fHeight,
    const int &iSpecies) {
  return log(1 - ((fHeight - 1.35) / (mp_fMaxTreeHeight[iSpecies] - 1.35)))
      / (-mp_fSlopeAsympHeight[iSpecies]);
}

//////////////////////////////////////////////////////////////////////////////
// CalcLinearAdultDbh()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcLinearAdultDbh(const float &fHeight, const int &iSpecies) {
  return (fHeight - mp_fAdultLinearIntercept[iSpecies])
      / mp_fAdultLinearSlope[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// CalcReverseLinearAdultDbh()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcReverseLinearAdultDbh(const float &fHeight,
    const int &iSpecies) {
  return fHeight * mp_fAdultReverseLinearSlope[iSpecies]
                                               + mp_fAdultReverseLinearIntercept[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// CalcLinearSaplingDbh()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcLinearSaplingDbh(const float &fHeight,
    const int &iSpecies) {
  return (fHeight - mp_fSaplingLinearIntercept[iSpecies])
      / mp_fSaplingLinearSlope[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// CalcReverseLinearSaplingDbh()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcReverseLinearSaplingDbh(const float &fHeight,
    const int &iSpecies) {
  return fHeight * mp_fSaplingReverseLinearSlope[iSpecies]
                                                 + mp_fSaplingReverseLinearIntercept[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// CalcStandardSeedlingDiam10()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcStandardSeedlingDiam10(const float &fHeight,
    const int &iSpecies) {
  return (log(1 - ((fHeight - 0.1) / 30))) / -mp_fSlopeHeightDiam10[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// CalcLinearSeedlingDiam10()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcLinearSeedlingDiam10(const float &fHeight,
    const int &iSpecies) {
  return (fHeight - mp_fSeedlingLinearIntercept[iSpecies])
      / mp_fSeedlingLinearSlope[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// CalcReverseLinearSeedlingDiam10()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcReverseLinearSeedlingDiam10(const float &fHeight,
    const int &iSpecies) {
  return fHeight * mp_fSeedlingReverseLinearSlope[iSpecies]
                                                  + mp_fSeedlingReverseLinearIntercept[iSpecies];
}

//////////////////////////////////////////////////////////////////////////////
// CalcStandardSapAdultCrownRad()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcStandardSapAdultCrownRad(clTree *p_oTree) {
  float fDbh;
  int iSpecies = p_oTree->GetSpecies();
  p_oTree->GetValue(m_oPop->GetDbhCode(iSpecies, p_oTree->GetType()), &fDbh);
  float fRad = mp_fAsympCrownRad[iSpecies] * pow(fDbh,
      mp_fCrownRadExp[iSpecies]);
  return fRad > mp_fMaxCrownRad[iSpecies] ? mp_fMaxCrownRad[iSpecies] : fRad;
}

//////////////////////////////////////////////////////////////////////////////
// CalcChapRichSapAdultCrownRad()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcChapRichSapAdultCrownRad(clTree *p_oTree) {
  float fDbh;
  int iSpecies = p_oTree->GetSpecies();
  p_oTree->GetValue(m_oPop->GetDbhCode(iSpecies, p_oTree->GetType()), &fDbh);
  return mp_fCRCrownRadIntercept[iSpecies] + (mp_fCRAsympCrownRad[iSpecies]
                                                                  * (pow((1 - exp(-mp_fCRCrownRadShape1[iSpecies] * fDbh)),
                                                                      mp_fCRCrownRadShape2[iSpecies])));
}

//////////////////////////////////////////////////////////////////////////////
// CalcStandardSapAdultCrownDepth()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcStandardSapAdultCrownDepth(clTree *p_oTree) {
  float fHeight;
  int iSpecies = p_oTree->GetSpecies();
  p_oTree->GetValue(m_oPop->GetHeightCode(iSpecies, p_oTree->GetType()),
      &fHeight);
  float fCrownHeight = mp_fAsympCrownDepth[iSpecies] * pow(fHeight,
      mp_fCrownDepthExp[iSpecies]);
  return fCrownHeight > fHeight ? fHeight : fCrownHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcChapRichSapAdultCrownDepth()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcChapRichSapAdultCrownDepth(clTree *p_oTree) {
  float fHeight;
  int iSpecies = p_oTree->GetSpecies();
  p_oTree->GetValue(m_oPop->GetHeightCode(iSpecies, p_oTree->GetType()),
      &fHeight);
  float fCHeight = mp_fCRCrownHtIntercept[iSpecies]
                                          + (mp_fCRAsympCrownHt[iSpecies] * (pow((1 - exp(
                                              -mp_fCRCrownHtShape1[iSpecies] * fHeight)),
                                              mp_fCRCrownHtShape2[iSpecies])));
  return (fCHeight > fHeight) ? fHeight : fCHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcPowerSaplingHeight()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcPowerSaplingHeight(const float &fDbh,
    const int &iSpecies) {
  if (fDbh <= 0.001)
    return 0.001;
  //Turn DBH into diam10
  float fDiam10 = ConvertDbhToDiam10(fDbh, iSpecies), fHeight =
      mp_fPowerA[iSpecies] * pow(fDiam10, mp_fPowerExpB[iSpecies]);
  if (fHeight > mp_fMaxTreeHeight[iSpecies])
    return mp_fMaxTreeHeight[iSpecies];
  if (fHeight <= 0.001)
    return 0.001;
  return fHeight;
}

//////////////////////////////////////////////////////////////////////////////
// CalcPowerSaplingDbh()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcPowerSaplingDbh(const float &fHeight,
    const int &iSpecies) {
  if (fHeight <= 0.001)
    return 0.001;
  float fDiam =
      pow(fHeight / mp_fPowerA[iSpecies], 1 / mp_fPowerExpB[iSpecies]);
  //Convert to DBH
  return ConvertDiam10ToDbh(fDiam, iSpecies);
}

//////////////////////////////////////////////////////////////////////////////
// CalcNonSpatDensDepExpAdultCrownRad()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcNonSpatDensDepExpAdultCrownRad(clTree *p_oTree) {

  stringstream sSearcher;
  sSearcher << "type=" << clTreePopulation::adult;
  clPlot *p_oPlot = m_oPop->mp_oSimManager->GetPlotObject();
  clTreeSearch *p_oAdults = m_oPop->Find(sSearcher.str());
  clTree *p_oNeigh = p_oAdults->NextTree();
  float fPlotArea = p_oPlot->GetPlotArea(), fHeight, fDbh, fNeighDbh,
      fNeighHeight, fCHi, //instrumental crown height
      fCrownRad, //crown radius
      fBAL = 0; //basal area of trees taller than the current tree
  int iSpecies = p_oTree->GetSpecies();

  p_oTree->GetValue(m_oPop->GetHeightCode(p_oTree->GetSpecies(),
      p_oTree->GetType()), &fHeight);
  p_oTree->GetValue(m_oPop->GetDbhCode(p_oTree->GetSpecies(),
      p_oTree->GetType()), &fDbh);

  //Find out if the plot basal area and density have been calculated yet
  if (m_fPlotDensity < 0) {
    //Calculate the total plot density and basal area

    m_fPlotDensity = 0;
    m_fPlotBasalArea = 0;

    while (p_oNeigh) {
      p_oNeigh->GetValue(m_oPop->GetDbhCode(p_oNeigh->GetSpecies(),
          p_oNeigh->GetType()), &fNeighDbh);
      m_fPlotDensity++;
      m_fPlotBasalArea += clModelMath::CalculateBasalArea(fNeighDbh);

      p_oNeigh = p_oAdults->NextTree();
    }
    //Reset for the taller-than searching
    p_oAdults->StartOver();
    p_oNeigh = p_oAdults->NextTree();

    m_fPlotDensity /= fPlotArea;
    m_fPlotBasalArea /= fPlotArea;
  }

  //Find the basal area of all adults taller than this one
  while (p_oNeigh) {
    p_oNeigh->GetValue(m_oPop->GetHeightCode(p_oNeigh->GetSpecies(),
        p_oNeigh->GetType()), &fNeighHeight);
    if (fNeighHeight > fHeight) {
      p_oNeigh->GetValue(m_oPop->GetDbhCode(p_oNeigh->GetSpecies(),
          p_oNeigh->GetType()), &fNeighDbh);
      fBAL += clModelMath::CalculateBasalArea(fNeighDbh);
    }
    p_oNeigh = p_oAdults->NextTree();
  }
  fBAL /= fPlotArea;

  //Calculate the instrumental crown height
  fCHi = mp_fNonSpatDensDepInstCHA[iSpecies]
                                   + mp_fNonSpatDensDepInstCHB[iSpecies] * fDbh
                                   + mp_fNonSpatDensDepInstCHC[iSpecies] * fHeight
                                   + mp_fNonSpatDensDepInstCHD[iSpecies] * fDbh * fDbh
                                   + mp_fNonSpatDensDepInstCHE[iSpecies] * fHeight * fHeight
                                   + mp_fNonSpatDensDepInstCHF[iSpecies] / fDbh
                                   + mp_fNonSpatDensDepInstCHG[iSpecies] * m_fPlotDensity
                                   + mp_fNonSpatDensDepInstCHH[iSpecies] * m_fPlotBasalArea
                                   + mp_fNonSpatDensDepInstCHI[iSpecies] * fBAL
                                   + mp_fNonSpatDensDepInstCHJ[iSpecies] * (fHeight / fDbh);

  //Don't let it be negative
  if (fCHi < 0)
    fCHi = fHeight * 0.5;

  //Calculate the crown radius
  fCrownRad = mp_fNonSpatExpDensDepCRD1[iSpecies] * pow(fDbh,
      mp_fNonSpatExpDensDepCRA[iSpecies]) * pow(fHeight,
          mp_fNonSpatExpDensDepCRB[iSpecies]) * pow(fCHi,
              mp_fNonSpatExpDensDepCRC[iSpecies]) * pow(m_fPlotDensity,
                  mp_fNonSpatExpDensDepCRD[iSpecies]) * pow(m_fPlotBasalArea,
                      mp_fNonSpatExpDensDepCRE[iSpecies]) * pow(fBAL,
                          mp_fNonSpatExpDensDepCRF[iSpecies]);

  if (fCrownRad > 1000)
    fCrownRad = 1000;

  return fCrownRad;
}

//////////////////////////////////////////////////////////////////////////////
// CalcNonSpatDensDepLogAdultCrownDepth()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcNonSpatDensDepLogAdultCrownDepth(clTree *p_oTree) {

  stringstream sSearcher;
    sSearcher << "type=" << clTreePopulation::adult;
  clPlot *p_oPlot = m_oPop->mp_oSimManager->GetPlotObject();
  clTreeSearch *p_oAdults = m_oPop->Find(sSearcher.str());
  clTree *p_oNeigh = p_oAdults->NextTree();
  float fPlotArea = p_oPlot->GetPlotArea(), fHeight, fDbh, fNeighDbh,
      fNeighHeight, fCRi, //instrumental crown radius
      fCrownHeight, //crown height
      fBAL = 0; //basal area of trees taller than the current tree
  int iSpecies = p_oTree->GetSpecies();

  p_oTree->GetValue(m_oPop->GetHeightCode(p_oTree->GetSpecies(),
      p_oTree->GetType()), &fHeight);
  p_oTree->GetValue(m_oPop->GetDbhCode(p_oTree->GetSpecies(),
      p_oTree->GetType()), &fDbh);

  //Find out if the plot basal area and density have been calculated yet
  if (m_fPlotDensity < 0) {
    //Calculate the total plot density and basal area

    m_fPlotDensity = 0;
    m_fPlotBasalArea = 0;

    while (p_oNeigh) {
      p_oNeigh->GetValue(m_oPop->GetDbhCode(p_oNeigh->GetSpecies(),
          p_oNeigh->GetType()), &fNeighDbh);
      m_fPlotDensity++;
      m_fPlotBasalArea += clModelMath::CalculateBasalArea(fNeighDbh);

      p_oNeigh = p_oAdults->NextTree();
    }
    //Reset for the taller-than searching
    p_oAdults->StartOver();
    p_oNeigh = p_oAdults->NextTree();

    m_fPlotDensity /= fPlotArea;
    m_fPlotBasalArea /= fPlotArea;
  }

  //Find the basal area of all adults taller than this one
  while (p_oNeigh) {
    p_oNeigh->GetValue(m_oPop->GetHeightCode(p_oNeigh->GetSpecies(),
        p_oNeigh->GetType()), &fNeighHeight);
    if (fNeighHeight > fHeight) {
      p_oNeigh->GetValue(m_oPop->GetDbhCode(p_oNeigh->GetSpecies(),
          p_oNeigh->GetType()), &fNeighDbh);
      fBAL += clModelMath::CalculateBasalArea(fNeighDbh);
    }
    p_oNeigh = p_oAdults->NextTree();
  }
  fBAL /= fPlotArea;

  //Calculate the instrumental crown radius
  fCRi = mp_fNonSpatDensDepInstCRA[iSpecies]
         + mp_fNonSpatDensDepInstCRB[iSpecies] * fDbh
         + mp_fNonSpatDensDepInstCRC[iSpecies] * fHeight
         + mp_fNonSpatDensDepInstCRD[iSpecies] * fDbh * fDbh
         + mp_fNonSpatDensDepInstCRE[iSpecies] * fHeight * fHeight
         + mp_fNonSpatDensDepInstCRF[iSpecies] / fDbh
         + mp_fNonSpatDensDepInstCRG[iSpecies] * m_fPlotDensity
         + mp_fNonSpatDensDepInstCRH[iSpecies] * m_fPlotBasalArea
         + mp_fNonSpatDensDepInstCRI[iSpecies] * fBAL
         + mp_fNonSpatDensDepInstCRJ[iSpecies] * (fHeight / fDbh);

  //Calculate the crown height
  fCrownHeight = fHeight / (1 + exp(mp_fNonSpatLogDensDepCHA[iSpecies]
                 + mp_fNonSpatLogDensDepCHB[iSpecies] * fDbh
                 + mp_fNonSpatLogDensDepCHC[iSpecies] * fHeight
                 + mp_fNonSpatLogDensDepCHD[iSpecies] * fCRi
                 + mp_fNonSpatLogDensDepCHE[iSpecies] * m_fPlotDensity
                 + mp_fNonSpatLogDensDepCHF[iSpecies] * m_fPlotBasalArea
                 + mp_fNonSpatLogDensDepCHG[iSpecies] * fBAL));

  return fCrownHeight;
}

//////////////////////////////////////////////////////////////////////////////
// DoDataUpdates()
//////////////////////////////////////////////////////////////////////////////
void clAllometry::DoDataUpdates() {
  m_fPlotDensity = -1;
  m_fPlotBasalArea = -1;
}

//////////////////////////////////////////////////////////////////////////////
// SetupAdultHeightDiam()
//////////////////////////////////////////////////////////////////////////////
void clAllometry::SetupAdultHeightDiam(xercesc::DOMElement * p_oElement) {
  doubleVal * p_fTemp = NULL; //for extracting values for a subset of all species
  int * p_iWhatFunction = NULL; //for getting which function to use for each
  //species
  int i, iHowMany;
  try {
    p_iWhatFunction = new int[m_iNumSpecies];

    //Declare our arrays
    mp_AdultHeight = new Ptr2Allometry[m_iNumSpecies];
    mp_AdultDiam = new Ptr2Allometry[m_iNumSpecies];

    FillSpeciesSpecificValue(p_oElement, "tr_whatAdultHeightDiam",
        "tr_wahdVal", p_iWhatFunction, m_oPop, true);

    //Use the values to set function pointers - if any are not a valid function
    //enum number, throw an error
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard == p_iWhatFunction[i]) {
        mp_AdultHeight[i] = &clAllometry::CalcStandardSapAdultHeight;
        mp_AdultDiam[i] = &clAllometry::CalcStandardSapAdultDbh;
      } else if (linear == p_iWhatFunction[i]) {
        mp_AdultHeight[i] = &clAllometry::CalcLinearAdultHeight;
        mp_AdultDiam[i] = &clAllometry::CalcLinearAdultDbh;
      } else if (reverse_linear == p_iWhatFunction[i]) {
        mp_AdultHeight[i] = &clAllometry::CalcReverseLinearAdultHeight;
        mp_AdultDiam[i] = &clAllometry::CalcReverseLinearAdultDbh;
      } else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clAllometry::GetData";
        std::stringstream s;
        s << "Invalid adult height-diam function code: " << p_iWhatFunction[i];
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //If any of the functions are standard, read standard parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (standard == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare the arrays we'll need
      if (!mp_fSlopeAsympHeight) {
        mp_fSlopeAsympHeight = new double[m_iNumSpecies];
      }

      FillSpeciesSpecificValue(p_oElement, "tr_slopeOfAsymHeight",
          "tr_soahVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fSlopeAsympHeight[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are linear, read linear parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (linear == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare the arrays we'll need
      mp_fAdultLinearSlope = new double[m_iNumSpecies];
      mp_fAdultLinearIntercept = new double[m_iNumSpecies];

      //Linear adult slope - throw an error if any values are 0
      FillSpeciesSpecificValue(p_oElement, "tr_adultLinearSlope", "tr_alsVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        if (0 == p_fTemp[i].val) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clAllometry::GetData";
          stcErr.sMoreInfo = "No values in adult linear slope may be 0.";
          throw(stcErr);

        }
        mp_fAdultLinearSlope[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Linear adult intercept
      FillSpeciesSpecificValue(p_oElement, "tr_adultLinearIntercept",
          "tr_aliVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fAdultLinearIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are reverse linear, read linear parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (reverse_linear == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (reverse_linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      mp_fAdultReverseLinearSlope = new double[m_iNumSpecies];
      mp_fAdultReverseLinearIntercept = new double[m_iNumSpecies];

      //Reverse linear adult slope - throw an error if any are 0
      FillSpeciesSpecificValue(p_oElement, "tr_adultReverseLinearSlope",
          "tr_arlsVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        if (0 == p_fTemp[i].val) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clAllometry::GetData";
          stcErr.sMoreInfo = "No values in adult reverse linear slope may be 0.";
          throw(stcErr);

        }
        mp_fAdultReverseLinearSlope[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Reverse linear adult intercept
      FillSpeciesSpecificValue(p_oElement, "tr_adultReverseLinearIntercept",
          "tr_arliVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fAdultReverseLinearIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    delete[] p_iWhatFunction;
    delete[] p_fTemp;
  } catch ( modelErr & err )
  {
    delete[] p_iWhatFunction;
    delete[] p_fTemp;
    throw( err );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetupAdultCrownRadius()
//////////////////////////////////////////////////////////////////////////////
void clAllometry::SetupAdultCrownRadius(xercesc::DOMElement * p_oElement) {

  doubleVal * p_fTemp = NULL; //for extracting values for a subset of all species
  int * p_iWhatFunction = NULL; //for getting which function to use for each
  //species

  int i, iHowMany;
  try {
    p_iWhatFunction = new int[m_iNumSpecies];

    mp_AdultCrownRad = new Ptr2CrownDimension[m_iNumSpecies];

    FillSpeciesSpecificValue(p_oElement, "tr_whatAdultCrownRadDiam",
        "tr_wacrdVal", p_iWhatFunction, m_oPop, true);

    //Use the values to set function pointers - if any are not a valid function
    //enum number, throw an error
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard_cr == p_iWhatFunction[i]) {
        mp_AdultCrownRad[i] = &clAllometry::CalcStandardSapAdultCrownRad;
      } else if (chapman_richards_cr == p_iWhatFunction[i]) {
        mp_AdultCrownRad[i] = &clAllometry::CalcChapRichSapAdultCrownRad;
      } else if (non_spat_exp_dens_dep_cr == p_iWhatFunction[i]) {
        mp_AdultCrownRad[i] = &clAllometry::CalcNonSpatDensDepExpAdultCrownRad;
      } else if (nci_cr == p_iWhatFunction[i]) {
        mp_AdultCrownRad[i] = &clAllometry::CalcNCICrownRad;
      } else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clAllometry::GetData";
        std::stringstream s;
        s << "Invalid adult crown radius-diam function code: " << p_iWhatFunction[i];
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //-----------------------------------------
    //If any of the functions are standard, read standard parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (standard == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      if (!mp_fAsympCrownRad)
        mp_fAsympCrownRad = new double[m_iNumSpecies];
      if (!mp_fCrownRadExp)
        mp_fCrownRadExp = new double[m_iNumSpecies];
      if (!mp_fMaxCrownRad)
        mp_fMaxCrownRad = new double[m_iNumSpecies];

      FillSpeciesSpecificValue(p_oElement, "tr_stdAsympCrownRad", "tr_sacrVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fAsympCrownRad[p_fTemp[i].code] = p_fTemp[i].val;
      }

      FillSpeciesSpecificValue(p_oElement, "tr_stdCrownRadExp", "tr_screVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCrownRadExp[p_fTemp[i].code] = p_fTemp[i].val;
      }

      FillSpeciesSpecificValue(p_oElement, "tr_stdMaxCrownRad", "tr_smcrVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fMaxCrownRad[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //-----------------------------------------
    //If any of the functions are Chapman-Richards, read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (chapman_richards_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare the arrays we'll need
      if (!mp_fCRCrownRadIntercept)
        mp_fCRCrownRadIntercept = new double[m_iNumSpecies];
      if (!mp_fCRAsympCrownRad)
        mp_fCRAsympCrownRad = new double[m_iNumSpecies];
      if (!mp_fCRCrownRadShape1)
        mp_fCRCrownRadShape1 = new double[m_iNumSpecies];
      if (!mp_fCRCrownRadShape2)
        mp_fCRCrownRadShape2 = new double[m_iNumSpecies];

      //Crown radius intercept
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownRadIntercept",
          "tr_crcriVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownRadIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Asymptotic crown radius
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownRadAsymp",
          "tr_crcraVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRAsympCrownRad[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Shape parameter 1
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownRadShape1b",
          "tr_crcrs1bVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownRadShape1[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Shape parameter 2
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownRadShape2c",
          "tr_crcrs2cVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownRadShape2[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //-----------------------------------------
    //If any of the functions are non-spatial exponential density dependent,
    //read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (non_spat_exp_dens_dep_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (non_spat_exp_dens_dep_cr == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      mp_fNonSpatDensDepInstCHA = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHB = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHC = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHD = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHE = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHF = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHG = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHH = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHI = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCHJ = new double[m_iNumSpecies];
      mp_fNonSpatExpDensDepCRD1 = new double[m_iNumSpecies];
      mp_fNonSpatExpDensDepCRA = new double[m_iNumSpecies];
      mp_fNonSpatExpDensDepCRB = new double[m_iNumSpecies];
      mp_fNonSpatExpDensDepCRC = new double[m_iNumSpecies];
      mp_fNonSpatExpDensDepCRD = new double[m_iNumSpecies];
      mp_fNonSpatExpDensDepCRE = new double[m_iNumSpecies];
      mp_fNonSpatExpDensDepCRF = new double[m_iNumSpecies];

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHA",
          "tr_nsddichaVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHA[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHB",
          "tr_nsddichbVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHB[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHC",
          "tr_nsddichcVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHC[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHD",
          "tr_nsddichdVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHD[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHE",
          "tr_nsddicheVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHE[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHF",
          "tr_nsddichfVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHF[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHG",
          "tr_nsddichgVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHG[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHH",
          "tr_nsddichhVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHH[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHI",
          "tr_nsddichiVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHI[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCHJ",
          "tr_nsddichjVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCHJ[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatExpDensDepCRD1",
          "tr_nseddcrd1Val", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatExpDensDepCRD1[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatExpDensDepCRA",
          "tr_nseddcraVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatExpDensDepCRA[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatExpDensDepCRB",
          "tr_nseddcrbVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatExpDensDepCRB[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatExpDensDepCRC",
          "tr_nseddcrcVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatExpDensDepCRC[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatExpDensDepCRD",
          "tr_nseddcrdVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatExpDensDepCRD[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatExpDensDepCRE",
          "tr_nseddcreVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatExpDensDepCRE[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatExpDensDepCRF",
          "tr_nseddcrfVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatExpDensDepCRF[p_fTemp[i].code] = p_fTemp[i].val;
    }

    //-----------------------------------------
    //If any of the functions are NCI, read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (nci_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      int j;

      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (nci_cr == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      if (!mp_fNCIMaxCrownRadius)
        mp_fNCIMaxCrownRadius = new double[m_iNumSpecies];
      if (!mp_fNCICRAlpha)
        mp_fNCICRAlpha = new double[m_iNumSpecies];
      if (!mp_fNCICRBeta)
        mp_fNCICRBeta = new double[m_iNumSpecies];
      if (!mp_fNCICRGamma)
        mp_fNCICRGamma = new double[m_iNumSpecies];
      if (!mp_fNCICRMaxCrowdingRadius)
        mp_fNCICRMaxCrowdingRadius = new double[m_iNumSpecies];
      if (!mp_fNCICRN)
        mp_fNCICRN = new double[m_iNumSpecies];
      if (!mp_fNCICRD)
        mp_fNCICRD = new double[m_iNumSpecies];
      if (!mp_fNCICRMinNeighborDBH)
        mp_fNCICRMinNeighborDBH = new double[m_iNumSpecies];
      if (!mp_fNCICRLambda) {
        mp_fNCICRLambda = new double*[m_iNumSpecies];
        for (i = 0; i < m_iNumSpecies; i++)
          mp_fNCICRLambda[i] = new double[m_iNumSpecies];
      }

      FillSpeciesSpecificValue(p_oElement, "tr_nciCRMaxCrownRadius",
          "tr_ncrmcrVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCIMaxCrownRadius[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRMaxCrowdingRadius",
          "tr_ncrmcrVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRMaxCrowdingRadius[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRAlpha", "tr_ncraVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRAlpha[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRBeta", "tr_ncrbVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRBeta[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRGamma", "tr_ncrgVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRGamma[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRCrowdingN", "tr_nccrnVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRN[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue( p_oElement, "tr_nciCRMinNeighborDBH",
          "tr_ncrmndVal", mp_fNCICRMinNeighborDBH, m_oPop, true );
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRSizeEffectD", "tr_ncrsedVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRD[p_fTemp[i].code] = p_fTemp[i].val;
      }

      for (i = 0; i < m_iNumSpecies; i++) {
        stringstream s;
        s << "tr_nciCR" << m_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambda";
        FillSpeciesSpecificValue(p_oElement, s.str(), "tr_ncrlVal", p_fTemp,
            iHowMany, m_oPop, true);
        for (j = 0; j < iHowMany; j++) {
          mp_fNCICRLambda[p_fTemp[j].code][i] = p_fTemp[j].val;
        }
      }

      m_fMinSaplingHeight = 50;
      //Get the minimum sapling height
      for ( i = 0; i < m_iNumSpecies; i++ )  {
        if ( m_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
          m_fMinSaplingHeight = m_oPop->GetMaxSeedlingHeight( i );
      }
    }

    delete[] p_iWhatFunction;
    delete[] p_fTemp;
  } catch ( modelErr & err )
  {
    delete[] p_iWhatFunction;
    delete[] p_fTemp;
    throw( err );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetupSaplingHeightDiam()
//////////////////////////////////////////////////////////////////////////////
void clAllometry::SetupSaplingHeightDiam(xercesc::DOMElement * p_oElement) {

  doubleVal * p_fTemp = NULL; //for extracting values for a subset of all species
  int * p_iWhatFunction = NULL; //for getting which function to use for each
  //species

  int i, iHowMany;

  try {
    p_iWhatFunction = new int[m_iNumSpecies];
    mp_SaplingHeight = new Ptr2Allometry[m_iNumSpecies];
    mp_SaplingDiam = new Ptr2Allometry[m_iNumSpecies];

    //***********************
    // Saplings - height diameter
    //***********************
    FillSpeciesSpecificValue(p_oElement, "tr_whatSaplingHeightDiam",
        "tr_wsahdVal", p_iWhatFunction, m_oPop, true);

    //Use the values to set function pointers - if any are not a valid function
    //enum number, throw an error
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard == p_iWhatFunction[i]) {
        mp_SaplingHeight[i] = &clAllometry::CalcStandardSapAdultHeight;
        mp_SaplingDiam[i] = &clAllometry::CalcStandardSapAdultDbh;
      } else if (linear == p_iWhatFunction[i]) {
        mp_SaplingHeight[i] = &clAllometry::CalcLinearSaplingHeight;
        mp_SaplingDiam[i] = &clAllometry::CalcLinearSaplingDbh;
      } else if (reverse_linear == p_iWhatFunction[i]) {
        mp_SaplingHeight[i] = &clAllometry::CalcReverseLinearSaplingHeight;
        mp_SaplingDiam[i] = &clAllometry::CalcReverseLinearSaplingDbh;
      } else if (power == p_iWhatFunction[i]) {
        mp_SaplingHeight[i] = &clAllometry::CalcPowerSaplingHeight;
        mp_SaplingDiam[i] = &clAllometry::CalcPowerSaplingDbh;
      } else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clAllometry::GetData";
        std::stringstream s;
        s << "Invalid sapling height-diam function code: " << p_iWhatFunction[i];
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //If any of the functions are standard, read standard parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (standard == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      if (!mp_fSlopeAsympHeight) {
        mp_fSlopeAsympHeight = new double[m_iNumSpecies];
      }

      FillSpeciesSpecificValue(p_oElement, "tr_slopeOfAsymHeight",
          "tr_soahVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fSlopeAsympHeight[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are linear, read linear parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (linear == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      mp_fSaplingLinearSlope = new double[m_iNumSpecies];
      mp_fSaplingLinearIntercept = new double[m_iNumSpecies];

      //Linear sapling slope - throw an error if any values are 0
      FillSpeciesSpecificValue(p_oElement, "tr_saplingLinearSlope",
          "tr_salsVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        if (0 == p_fTemp[i].val) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clAllometry::GetData";
          stcErr.sMoreInfo = "No values in sapling linear slope may be 0.";
          throw(stcErr);

        }
        mp_fSaplingLinearSlope[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Linear sapling intercept
      FillSpeciesSpecificValue(p_oElement, "tr_saplingLinearIntercept",
          "tr_saliVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fSaplingLinearIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are reverse linear, read reverse linear
    //parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (reverse_linear == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (reverse_linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      mp_fSaplingReverseLinearSlope = new double[m_iNumSpecies];
      mp_fSaplingReverseLinearIntercept = new double[m_iNumSpecies];

      //Reverse linear sapling slope - throw an error if any values are 0
      FillSpeciesSpecificValue(p_oElement, "tr_saplingReverseLinearSlope",
          "tr_sarlsVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        if (0 == p_fTemp[i].val) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clAllometry::GetData";
          stcErr.sMoreInfo = "No values in sapling reverse linear slope may be 0.";
          throw(stcErr);

        }
        mp_fSaplingReverseLinearSlope[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Reverse linear sapling intercept
      FillSpeciesSpecificValue(p_oElement, "tr_saplingReverseLinearIntercept",
          "tr_sarliVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fSaplingReverseLinearIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are power, read power function parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (power == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (power == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      mp_fPowerA = new double[m_iNumSpecies];
      mp_fPowerExpB = new double[m_iNumSpecies];

      //Power function "a"
      FillSpeciesSpecificValue(p_oElement, "tr_saplingPowerA", "tr_sapaVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fPowerA[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Power function "b"
      FillSpeciesSpecificValue(p_oElement, "tr_saplingPowerB", "tr_sapbVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fPowerExpB[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    delete[] p_iWhatFunction;
    delete[] p_fTemp;

  } catch ( modelErr & err )
  {
    delete[] p_iWhatFunction;
    delete[] p_fTemp;
    throw( err );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetupSaplingCrownRadius()
//////////////////////////////////////////////////////////////////////////////
void clAllometry::SetupSaplingCrownRadius(xercesc::DOMElement * p_oElement) {

  doubleVal * p_fTemp = NULL; //for extracting values for a subset of all species
  int * p_iWhatFunction = NULL; //for getting which function to use for each
  //species

  int i, iHowMany;
  try {
    p_iWhatFunction = new int[m_iNumSpecies];
    mp_SaplingCrownRad = new Ptr2CrownDimension[m_iNumSpecies];

    FillSpeciesSpecificValue(p_oElement, "tr_whatSaplingCrownRadDiam",
        "tr_wscrdVal", p_iWhatFunction, m_oPop, true);

    //Use the values to set function pointers - if any are not a valid function
    //enum number, throw an error
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard_cr == p_iWhatFunction[i]) {
        mp_SaplingCrownRad[i] = &clAllometry::CalcStandardSapAdultCrownRad;
      } else if (chapman_richards_cr == p_iWhatFunction[i]) {
        mp_SaplingCrownRad[i] = &clAllometry::CalcChapRichSapAdultCrownRad;
      } else if (nci_cr == p_iWhatFunction[i]) {
        mp_SaplingCrownRad[i] = &clAllometry::CalcNCICrownRad;
      } else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clAllometry::GetData";
        std::stringstream s;
        s << "Invalid sapling crown radius-diam function code: " << p_iWhatFunction[i];
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //If any of the functions are standard, read standard parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (standard == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      if (!mp_fAsympCrownRad)
        mp_fAsympCrownRad = new double[m_iNumSpecies];
      if (!mp_fCrownRadExp)
        mp_fCrownRadExp = new double[m_iNumSpecies];
      if (!mp_fMaxCrownRad)
        mp_fMaxCrownRad = new double[m_iNumSpecies];

      FillSpeciesSpecificValue(p_oElement, "tr_stdAsympCrownRad", "tr_sacrVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fAsympCrownRad[p_fTemp[i].code] = p_fTemp[i].val;
      }

      FillSpeciesSpecificValue(p_oElement, "tr_stdCrownRadExp", "tr_screVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCrownRadExp[p_fTemp[i].code] = p_fTemp[i].val;
      }

      FillSpeciesSpecificValue(p_oElement, "tr_stdMaxCrownRad", "tr_smcrVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fMaxCrownRad[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are Chapman-Richards, read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (chapman_richards_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare the arrays we'll need
      if (!mp_fCRCrownRadIntercept)
        mp_fCRCrownRadIntercept = new double[m_iNumSpecies];
      if (!mp_fCRAsympCrownRad)
        mp_fCRAsympCrownRad = new double[m_iNumSpecies];
      if (!mp_fCRCrownRadShape1)
        mp_fCRCrownRadShape1 = new double[m_iNumSpecies];
      if (!mp_fCRCrownRadShape2)
        mp_fCRCrownRadShape2 = new double[m_iNumSpecies];

      //Crown radius intercept
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownRadIntercept",
          "tr_crcriVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownRadIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Asymptotic crown radius
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownRadAsymp",
          "tr_crcraVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRAsympCrownRad[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Shape parameter 1
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownRadShape1b",
          "tr_crcrs1bVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownRadShape1[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Shape parameter 2
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownRadShape2c",
          "tr_crcrs2cVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownRadShape2[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //-----------------------------------------
    //If any of the functions are NCI, read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (nci_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      int j;

      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (nci_cr == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      if (!mp_fNCIMaxCrownRadius)
        mp_fNCIMaxCrownRadius = new double[m_iNumSpecies];
      if (!mp_fNCICRAlpha)
        mp_fNCICRAlpha = new double[m_iNumSpecies];
      if (!mp_fNCICRBeta)
        mp_fNCICRBeta = new double[m_iNumSpecies];
      if (!mp_fNCICRGamma)
        mp_fNCICRGamma = new double[m_iNumSpecies];
      if (!mp_fNCICRMaxCrowdingRadius)
        mp_fNCICRMaxCrowdingRadius = new double[m_iNumSpecies];
      if (!mp_fNCICRN)
        mp_fNCICRN = new double[m_iNumSpecies];
      if (!mp_fNCICRD)
        mp_fNCICRD = new double[m_iNumSpecies];
      if (!mp_fNCICRMinNeighborDBH)
        mp_fNCICRMinNeighborDBH = new double[m_iNumSpecies];
      if (!mp_fNCICRLambda) {
        mp_fNCICRLambda = new double*[m_iNumSpecies];
        for (i = 0; i < m_iNumSpecies; i++)
          mp_fNCICRLambda[i] = new double[m_iNumSpecies];
      }

      FillSpeciesSpecificValue(p_oElement, "tr_nciCRMaxCrownRadius",
          "tr_ncrmcrVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCIMaxCrownRadius[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRMaxCrowdingRadius",
          "tr_ncrmcrVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRMaxCrowdingRadius[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRAlpha", "tr_ncraVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRAlpha[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRBeta", "tr_ncrbVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRBeta[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRGamma", "tr_ncrgVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRGamma[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRCrowdingN", "tr_nccrnVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRN[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue( p_oElement, "tr_nciCRMinNeighborDBH",
          "tr_ncrmndVal", mp_fNCICRMinNeighborDBH, m_oPop, true );
      FillSpeciesSpecificValue(p_oElement, "tr_nciCRSizeEffectD", "tr_ncrsedVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICRD[p_fTemp[i].code] = p_fTemp[i].val;
      }

      for (i = 0; i < m_iNumSpecies; i++) {
        stringstream s;
        s << "tr_nciCR" << m_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambda";
        FillSpeciesSpecificValue(p_oElement, s.str(), "tr_ncrlVal", p_fTemp,
            iHowMany, m_oPop, true);
        for (j = 0; j < iHowMany; j++) {
          mp_fNCICRLambda[p_fTemp[j].code][i] = p_fTemp[j].val;
        }
      }

      m_fMinSaplingHeight = 50;
      //Get the minimum sapling height
      for ( i = 0; i < m_iNumSpecies; i++ )  {
        if ( m_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
          m_fMinSaplingHeight = m_oPop->GetMaxSeedlingHeight( i );
      }
    }

    delete[] p_iWhatFunction;
    delete[] p_fTemp;

  } catch ( modelErr & err )
  {
    delete[] p_iWhatFunction;
    delete[] p_fTemp;
    throw( err );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetupSaplingCrownDepth()
//////////////////////////////////////////////////////////////////////////////
void clAllometry::SetupSaplingCrownDepth(xercesc::DOMElement * p_oElement) {

  doubleVal * p_fTemp = NULL; //for extracting values for a subset of all species
  int * p_iWhatFunction = NULL; //for getting which function to use for each
  //species

  int i, iHowMany;
  try {
    p_iWhatFunction = new int[m_iNumSpecies];
    mp_SaplingCrownDepth = new Ptr2CrownDimension[m_iNumSpecies];

    FillSpeciesSpecificValue(p_oElement, "tr_whatSaplingCrownHeightHeight",
        "tr_wschhVal", p_iWhatFunction, m_oPop, true);

    //Use the values to set function pointers - if any are not a valid function
    //enum number, throw an error
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard_cd == p_iWhatFunction[i]) {
        mp_SaplingCrownDepth[i] = &clAllometry::CalcStandardSapAdultCrownDepth;
      } else if (chapman_richards_cd == p_iWhatFunction[i]) {
        mp_SaplingCrownDepth[i] = &clAllometry::CalcChapRichSapAdultCrownDepth;
      } else if (nci_cd == p_iWhatFunction[i]) {
        mp_SaplingCrownDepth[i] = &clAllometry::CalcNCICrownDepth;
      } else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clAllometry::GetData";
        std::stringstream s;
        s << "Invalid sapling crown height-diam function code: " << p_iWhatFunction[i];
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //If any of the functions are standard, read standard parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (standard == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      if (!mp_fAsympCrownDepth)
        mp_fAsympCrownDepth = new double[m_iNumSpecies];
      if (!mp_fCrownDepthExp)
        mp_fCrownDepthExp = new double[m_iNumSpecies];

      FillSpeciesSpecificValue(p_oElement, "tr_stdAsympCrownHt", "tr_sachVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fAsympCrownDepth[p_fTemp[i].code] = p_fTemp[i].val;
      }

      FillSpeciesSpecificValue(p_oElement, "tr_stdCrownHtExp", "tr_scheVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCrownDepthExp[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are Chapman-Richards, read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (chapman_richards_cr == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare any arrays we'll need
      if (!mp_fCRCrownHtIntercept)
        mp_fCRCrownHtIntercept = new double[m_iNumSpecies];
      if (!mp_fCRAsympCrownHt)
        mp_fCRAsympCrownHt = new double[m_iNumSpecies];
      if (!mp_fCRCrownHtShape1)
        mp_fCRCrownHtShape1 = new double[m_iNumSpecies];
      if (!mp_fCRCrownHtShape2)
        mp_fCRCrownHtShape2 = new double[m_iNumSpecies];

      //Crown height intercept
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownHtIntercept",
          "tr_crchiVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownHtIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Asymptotic crown height
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownHtAsymp",
          "tr_crchaVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRAsympCrownHt[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Shape parameter 1
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownHtShape1b",
          "tr_crchs1bVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownHtShape1[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Shape parameter 2
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownHtShape2c",
          "tr_crchs2cVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownHtShape2[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //-----------------------------------------
    //If any of the functions are NCI, read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (nci_cd == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      int j;

      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (nci_cd == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      if (!mp_fNCIMaxCrownDepth)
        mp_fNCIMaxCrownDepth = new double[m_iNumSpecies];
      if (!mp_fNCICDAlpha)
        mp_fNCICDAlpha = new double[m_iNumSpecies];
      if (!mp_fNCICDBeta)
        mp_fNCICDBeta = new double[m_iNumSpecies];
      if (!mp_fNCICDGamma)
        mp_fNCICDGamma = new double[m_iNumSpecies];
      if (!mp_fNCICDMaxCrowdingRadius)
        mp_fNCICDMaxCrowdingRadius = new double[m_iNumSpecies];
      if (!mp_fNCICDN)
        mp_fNCICDN = new double[m_iNumSpecies];
      if (!mp_fNCICDD)
        mp_fNCICDD = new double[m_iNumSpecies];
      if (!mp_fNCICDMinNeighborDBH)
        mp_fNCICDMinNeighborDBH = new double[m_iNumSpecies];
      if (!mp_fNCICDLambda) {
        mp_fNCICDLambda = new double*[m_iNumSpecies];
        for (i = 0; i < m_iNumSpecies; i++)
          mp_fNCICDLambda[i] = new double[m_iNumSpecies];
      }

      FillSpeciesSpecificValue(p_oElement, "tr_nciCDMaxCrownDepth",
          "tr_ncdmcrVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCIMaxCrownDepth[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDMaxCrowdingRadius",
          "tr_ncdmcrVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDMaxCrowdingRadius[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDAlpha", "tr_ncdaVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDAlpha[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDBeta", "tr_ncdbVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDBeta[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDGamma", "tr_ncdgVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDGamma[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDCrowdingN", "tr_nccdnVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDN[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue( p_oElement, "tr_nciCDMinNeighborDBH",
          "tr_ncdmndVal", mp_fNCICDMinNeighborDBH, m_oPop, true );
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDSizeEffectD", "tr_ncdsedVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDD[p_fTemp[i].code] = p_fTemp[i].val;
      }

      for (i = 0; i < m_iNumSpecies; i++) {
        stringstream s;
        s << "tr_nciCD" << m_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambda";
        FillSpeciesSpecificValue(p_oElement, s.str(), "tr_ncdlVal", p_fTemp,
            iHowMany, m_oPop, true);
        for (j = 0; j < iHowMany; j++) {
          mp_fNCICDLambda[p_fTemp[j].code][i] = p_fTemp[j].val;
        }
      }

      m_fMinSaplingHeight = 50;
      //Get the minimum sapling height
      for ( i = 0; i < m_iNumSpecies; i++ )  {
        if ( m_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
          m_fMinSaplingHeight = m_oPop->GetMaxSeedlingHeight( i );
      }
    }

    delete[] p_iWhatFunction;
    delete[] p_fTemp;

  } catch ( modelErr & err )
  {
    delete[] p_iWhatFunction;
    delete[] p_fTemp;
    throw( err );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetupAdultCrownDepth()
//////////////////////////////////////////////////////////////////////////////
void clAllometry::SetupAdultCrownDepth(xercesc::DOMElement * p_oElement) {

  doubleVal * p_fTemp = NULL; //for extracting values for a subset of all species
  int * p_iWhatFunction = NULL; //for getting which function to use for each
  //species

  int i, iHowMany;
  try {
    p_iWhatFunction = new int[m_iNumSpecies];
    mp_AdultCrownDepth = new Ptr2CrownDimension[m_iNumSpecies];

    FillSpeciesSpecificValue(p_oElement, "tr_whatAdultCrownHeightHeight",
        "tr_wachhVal", p_iWhatFunction, m_oPop, true);

    //Use the values to set function pointers - if any are not a valid function
    //enum number, throw an error
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard_cd == p_iWhatFunction[i]) {
        mp_AdultCrownDepth[i] = &clAllometry::CalcStandardSapAdultCrownDepth;
      } else if (chapman_richards_cd == p_iWhatFunction[i]) {
        mp_AdultCrownDepth[i] = &clAllometry::CalcChapRichSapAdultCrownDepth;
      } else if (non_spat_log_dens_dep_cd == p_iWhatFunction[i]) {
        mp_AdultCrownDepth[i]
                           = &clAllometry::CalcNonSpatDensDepLogAdultCrownDepth;
      } else if (nci_cd == p_iWhatFunction[i]) {
        mp_AdultCrownDepth[i] = &clAllometry::CalcNCICrownDepth;

      } else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clAllometry::GetData";
        std::stringstream s;
        s << "Invalid adult crown height-diam function code: " << p_iWhatFunction[i];
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //-----------------------------------------
    //If any of the functions are standard, read standard parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard_cd == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (standard_cd == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      if (!mp_fAsympCrownDepth)
        mp_fAsympCrownDepth = new double[m_iNumSpecies];
      if (!mp_fCrownDepthExp)
        mp_fCrownDepthExp = new double[m_iNumSpecies];

      FillSpeciesSpecificValue(p_oElement, "tr_stdAsympCrownHt", "tr_sachVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fAsympCrownDepth[p_fTemp[i].code] = p_fTemp[i].val;
      }

      FillSpeciesSpecificValue(p_oElement, "tr_stdCrownHtExp", "tr_scheVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCrownDepthExp[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //-----------------------------------------
    //If any of the functions are Chapman-Richards, read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (chapman_richards_cd == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (chapman_richards_cd == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare any arrays we'll need
      if (!mp_fCRCrownHtIntercept)
        mp_fCRCrownHtIntercept = new double[m_iNumSpecies];
      if (!mp_fCRAsympCrownHt)
        mp_fCRAsympCrownHt = new double[m_iNumSpecies];
      if (!mp_fCRCrownHtShape1)
        mp_fCRCrownHtShape1 = new double[m_iNumSpecies];
      if (!mp_fCRCrownHtShape2)
        mp_fCRCrownHtShape2 = new double[m_iNumSpecies];

      //Crown height intercept
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownHtIntercept",
          "tr_crchiVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownHtIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Asymptotic crown height
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownHtAsymp",
          "tr_crchaVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRAsympCrownHt[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Shape parameter 1
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownHtShape1b",
          "tr_crchs1bVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownHtShape1[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Shape parameter 2
      FillSpeciesSpecificValue(p_oElement, "tr_chRichCrownHtShape2c",
          "tr_crchs2cVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fCRCrownHtShape2[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //-----------------------------------------
    //If any of the functions are non-spatial logistic density dependent,
    //read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (non_spat_log_dens_dep_cd == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (non_spat_log_dens_dep_cd == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare any arrays we'll need
      mp_fNonSpatDensDepInstCRA = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRB = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRC = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRD = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRE = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRF = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRG = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRH = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRI = new double[m_iNumSpecies];
      mp_fNonSpatDensDepInstCRJ = new double[m_iNumSpecies];
      mp_fNonSpatLogDensDepCHA = new double[m_iNumSpecies];
      mp_fNonSpatLogDensDepCHB = new double[m_iNumSpecies];
      mp_fNonSpatLogDensDepCHC = new double[m_iNumSpecies];
      mp_fNonSpatLogDensDepCHD = new double[m_iNumSpecies];
      mp_fNonSpatLogDensDepCHE = new double[m_iNumSpecies];
      mp_fNonSpatLogDensDepCHF = new double[m_iNumSpecies];
      mp_fNonSpatLogDensDepCHG = new double[m_iNumSpecies];

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRA",
          "tr_nsddicraVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRA[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRB",
          "tr_nsddicrbVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRB[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRC",
          "tr_nsddicrcVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRC[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRD",
          "tr_nsddicrdVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRD[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRE",
          "tr_nsddicreVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRE[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRF",
          "tr_nsddicrfVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRF[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRG",
          "tr_nsddicrgVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRG[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRH",
          "tr_nsddicrhVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRH[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRI",
          "tr_nsddicriVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRI[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatDensDepInstCRJ",
          "tr_nsddicrjVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatDensDepInstCRJ[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatLogDensDepCHA",
          "tr_nslddchaVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatLogDensDepCHA[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatLogDensDepCHB",
          "tr_nslddchbVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatLogDensDepCHB[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatLogDensDepCHC",
          "tr_nslddchcVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatLogDensDepCHC[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatLogDensDepCHD",
          "tr_nslddchdVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatLogDensDepCHD[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatLogDensDepCHE",
          "tr_nslddcheVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatLogDensDepCHE[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatLogDensDepCHF",
          "tr_nslddchfVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatLogDensDepCHF[p_fTemp[i].code] = p_fTemp[i].val;

      FillSpeciesSpecificValue(p_oElement, "tr_nonSpatLogDensDepCHG",
          "tr_nslddchgVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++)
        mp_fNonSpatLogDensDepCHG[p_fTemp[i].code] = p_fTemp[i].val;
    }

    //-----------------------------------------
    //If any of the functions are NCI, read parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (nci_cd == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      int j;

      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (nci_cd == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      if (!mp_fNCIMaxCrownDepth)
        mp_fNCIMaxCrownDepth = new double[m_iNumSpecies];
      if (!mp_fNCICDAlpha)
        mp_fNCICDAlpha = new double[m_iNumSpecies];
      if (!mp_fNCICDBeta)
        mp_fNCICDBeta = new double[m_iNumSpecies];
      if (!mp_fNCICDGamma)
        mp_fNCICDGamma = new double[m_iNumSpecies];
      if (!mp_fNCICDMaxCrowdingRadius)
        mp_fNCICDMaxCrowdingRadius = new double[m_iNumSpecies];
      if (!mp_fNCICDN)
        mp_fNCICDN = new double[m_iNumSpecies];
      if (!mp_fNCICDD)
        mp_fNCICDD = new double[m_iNumSpecies];
      if (!mp_fNCICDMinNeighborDBH)
        mp_fNCICDMinNeighborDBH = new double[m_iNumSpecies];
      if (!mp_fNCICDLambda) {
        mp_fNCICDLambda = new double*[m_iNumSpecies];
        for (i = 0; i < m_iNumSpecies; i++)
          mp_fNCICDLambda[i] = new double[m_iNumSpecies];
      }

      FillSpeciesSpecificValue(p_oElement, "tr_nciCDMaxCrownDepth",
          "tr_ncdmcrVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCIMaxCrownDepth[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDMaxCrowdingRadius",
          "tr_ncdmcrVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDMaxCrowdingRadius[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDAlpha", "tr_ncdaVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDAlpha[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDBeta", "tr_ncdbVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDBeta[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDGamma", "tr_ncdgVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDGamma[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDCrowdingN", "tr_nccdnVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDN[p_fTemp[i].code] = p_fTemp[i].val;
      }
      FillSpeciesSpecificValue( p_oElement, "tr_nciCDMinNeighborDBH",
          "tr_ncdmndVal", mp_fNCICDMinNeighborDBH, m_oPop, true );
      FillSpeciesSpecificValue(p_oElement, "tr_nciCDSizeEffectD", "tr_ncdsedVal",
          p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fNCICDD[p_fTemp[i].code] = p_fTemp[i].val;
      }

      for (i = 0; i < m_iNumSpecies; i++) {
        stringstream s;
        s << "tr_nciCD" << m_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambda";
        FillSpeciesSpecificValue(p_oElement, s.str(), "tr_ncdlVal", p_fTemp,
            iHowMany, m_oPop, true);
        for (j = 0; j < iHowMany; j++) {
          mp_fNCICDLambda[p_fTemp[j].code][i] = p_fTemp[j].val;
        }
      }

      m_fMinSaplingHeight = 50;
      //Get the minimum sapling height
      for ( i = 0; i < m_iNumSpecies; i++ )  {
        if ( m_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
          m_fMinSaplingHeight = m_oPop->GetMaxSeedlingHeight( i );
      }
    }

    delete[] p_iWhatFunction;
    delete[] p_fTemp;

  } catch ( modelErr & err )
  {
    delete[] p_iWhatFunction;
    delete[] p_fTemp;
    throw( err );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetupSeedlingHeightDiam()
//////////////////////////////////////////////////////////////////////////////
void clAllometry::SetupSeedlingHeightDiam(xercesc::DOMElement * p_oElement) {

  doubleVal * p_fTemp = NULL; //for extracting values for a subset of all species
  int * p_iWhatFunction = NULL; //for getting which function to use for each
  //species

  int i, iHowMany;
  try {
    p_iWhatFunction = new int[m_iNumSpecies];
    mp_SeedlingHeight = new Ptr2Allometry[m_iNumSpecies];
    mp_SeedlingDiam = new Ptr2Allometry[m_iNumSpecies];

    //***********************
    // Seedlings
    //***********************
    FillSpeciesSpecificValue(p_oElement, "tr_whatSeedlingHeightDiam",
        "tr_wsehdVal", p_iWhatFunction, m_oPop, true);

    //Use the values to set function pointers - if any are not a valid function
    //enum number, throw an error
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard == p_iWhatFunction[i]) {
        mp_SeedlingHeight[i] = &clAllometry::CalcStandardSeedlingHeight;
        mp_SeedlingDiam[i] = &clAllometry::CalcStandardSeedlingDiam10;
      } else if (linear == p_iWhatFunction[i]) {
        mp_SeedlingHeight[i] = &clAllometry::CalcLinearSeedlingHeight;
        mp_SeedlingDiam[i] = &clAllometry::CalcLinearSeedlingDiam10;
      } else if (reverse_linear == p_iWhatFunction[i]) {
        mp_SeedlingHeight[i] = &clAllometry::CalcReverseLinearSeedlingHeight;
        mp_SeedlingDiam[i] = &clAllometry::CalcReverseLinearSeedlingDiam10;
      } else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clAllometry::GetData";
        std::stringstream s;
        s << "Invalid seedling height-diam function code: " << p_iWhatFunction[i];
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //If any of the functions are standard, read standard parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (standard == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (standard == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare array we'll need
      mp_fSlopeHeightDiam10 = new double[m_iNumSpecies];

      FillSpeciesSpecificValue(p_oElement, "tr_slopeOfHeight-Diam10",
          "tr_sohdVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fSlopeHeightDiam10[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are linear, read linear parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (linear == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      mp_fSeedlingLinearSlope = new double[m_iNumSpecies];
      mp_fSeedlingLinearIntercept = new double[m_iNumSpecies];

      //Linear seedling slope - throw error if any values are 0
      FillSpeciesSpecificValue(p_oElement, "tr_seedlingLinearSlope",
          "tr_selsVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        if (0 == p_fTemp[i].val) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clAllometry::GetData";
          stcErr.sMoreInfo = "No values in seedling linear slope may be 0.";
          throw(stcErr);

        }
        mp_fSeedlingLinearSlope[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Linear seedling intercept
      FillSpeciesSpecificValue(p_oElement, "tr_seedlingLinearIntercept",
          "tr_seliVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fSeedlingLinearIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    //If any of the functions are reverse linear, read linear parameters
    iHowMany = 0;
    for (i = 0; i < m_iNumSpecies; i++) {
      if (reverse_linear == p_iWhatFunction[i])
        iHowMany++;
    }
    if (iHowMany > 0) {
      if (p_fTemp) {
        delete[] p_fTemp;
        p_fTemp = NULL;
      }
      p_fTemp = new doubleVal[iHowMany];
      iHowMany = 0;
      for (i = 0; i < m_iNumSpecies; i++) {
        if (reverse_linear == p_iWhatFunction[i]) {
          p_fTemp[iHowMany].code = i;
          iHowMany++;
        }
      }

      //Declare arrays we'll need
      mp_fSeedlingReverseLinearSlope = new double[m_iNumSpecies];
      mp_fSeedlingReverseLinearIntercept = new double[m_iNumSpecies];

      //Reverse linear seedling slope - throw an error if any are 0
      FillSpeciesSpecificValue(p_oElement, "tr_seedlingReverseLinearSlope",
          "tr_serlsVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        if (0 == p_fTemp[i].val) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clAllometry::GetData";
          stcErr.sMoreInfo = "No values in seedling reverse linear slope may be 0.";
          throw(stcErr);
        }
        mp_fSeedlingReverseLinearSlope[p_fTemp[i].code] = p_fTemp[i].val;
      }

      //Reverse linear seedling intercept
      FillSpeciesSpecificValue(p_oElement, "tr_seedlingReverseLinearIntercept",
          "tr_serliVal", p_fTemp, iHowMany, m_oPop, true);
      for (i = 0; i < iHowMany; i++) {
        mp_fSeedlingReverseLinearIntercept[p_fTemp[i].code] = p_fTemp[i].val;
      }
    }

    delete[] p_iWhatFunction;
    delete[] p_fTemp;

  } catch ( modelErr & err )
  {
    delete[] p_iWhatFunction;
    delete[] p_fTemp;
    throw( err );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcNCICrownRad()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcNCICrownRad(clTree *p_oTree) {
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  clPlot * p_oPlot = m_oPop->mp_oSimManager->GetPlotObject();
  char cQuery[75];
  float fCrowdingEffect, //tree's crowding effect
  fNCI = 0, //the NCI
  fSizeEffect, //tree's size effect
  fTDbh, //tree's dbh
  fDistance, //distance between target and neighbor
  fNDbh, //neighbor's dbh
  fCrownRadius,
  fNeighX, fNeighY, //holders for the neighbor tree's X and Y
  fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iIsDead;
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iTargetSpecies = p_oTree->GetSpecies(), //target tree's species
  iTargetType = p_oTree->GetType(),
  iDeadCode; //neighbor's dead code

  p_oTree->GetValue(m_oPop->GetDbhCode(iTargetSpecies, iTargetType), &fTDbh);

  //Get NCI
  //Format the query to get all competing neighbors
  p_oTree->GetValue(m_oPop->GetXCode(iTargetSpecies, iTargetType), &fTargetX);
  p_oTree->GetValue(m_oPop->GetYCode(iTargetSpecies, iTargetType), &fTargetY);

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf(cQuery, "%s%f%s%f%s%f%s%f", "distance=",
      mp_fNCICRMaxCrowdingRadius[iTargetSpecies], "FROM x=", fTargetX, "y=",
      fTargetY, "::height=", m_fMinSaplingHeight);
  p_oAllNeighbors = m_oPop->Find(cQuery);

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while (p_oNeighbor) {
    if (p_oNeighbor != p_oTree) {
      iNeighSpecies = p_oNeighbor->GetSpecies();
      iNeighType = p_oNeighbor->GetType();
      if (clTreePopulation::seedling != iNeighType && (clTreePopulation::snag
          != iNeighType)) {
        //Get the neighbor's dbh
        p_oNeighbor->GetValue(m_oPop->GetDbhCode(iNeighSpecies, iNeighType),
            &fNDbh);
        if (fNDbh >= mp_fNCICRMinNeighborDBH[iNeighSpecies]) {

          //Make sure the neighbor's not dead
          iDeadCode = m_oPop->GetIntDataCode("dead", p_oNeighbor->GetSpecies(),
              p_oNeighbor->GetType());
          if (-1 != iDeadCode)
            p_oNeighbor->GetValue(iDeadCode, &iIsDead);
          else
            iIsDead = notdead;

          if (notdead == iIsDead) {

            //Get the neighbor's X and Y values
            p_oNeighbor->GetValue(m_oPop->GetXCode(iNeighSpecies, iNeighType),
                &fNeighX);
            p_oNeighbor->GetValue(m_oPop->GetYCode(iNeighSpecies, iNeighType),
                &fNeighY);

            //Get the distance between the two trees
            fDistance = p_oPlot->GetDistance(fTargetX, fTargetY, fNeighX,
                fNeighY);

            //Only continue if distance is not 0 - it will be a fluke condition to
            //allow a tree that is literally standing on top of another one not to
            //affect it competitively, but there it is
            if (0 != fDistance)

              //Add competitive effect to NCI
              fNCI += mp_fNCICRLambda[iTargetSpecies][iNeighSpecies] * (pow(
                  fNDbh, mp_fNCICRAlpha[iTargetSpecies]) / pow(fDistance,
                      mp_fNCICRBeta[iTargetSpecies]));

          }
        }
      }
    }
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  //Calculate the crowding effect
  fNCI *= exp(mp_fNCICRGamma[iTargetSpecies] * fTDbh);
  fCrowdingEffect = exp(-mp_fNCICRN[iTargetSpecies] * fNCI);
  //Make sure it's between 0 and 1
  if (fCrowdingEffect < 0)
    fCrowdingEffect = 0;
  if (fCrowdingEffect > 1)
    fCrowdingEffect = 1;

  //Get the tree's size effect
  fSizeEffect = 1 - exp(-mp_fNCICRD[iTargetSpecies] * fTDbh);

  //Make sure it's bounded between 0 and 1
  if (fSizeEffect < 0)
    fSizeEffect = 0;
  if (fSizeEffect > 1)
    fSizeEffect = 1;

  //Calculate crown radius
  fCrownRadius = mp_fNCIMaxCrownRadius[iTargetSpecies] * fSizeEffect * fCrowdingEffect;
  return fCrownRadius;

}

//////////////////////////////////////////////////////////////////////////////
// CalcNCICrownDepth()
//////////////////////////////////////////////////////////////////////////////
float clAllometry::CalcNCICrownDepth(clTree *p_oTree) {
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  clPlot * p_oPlot = m_oPop->mp_oSimManager->GetPlotObject();
  char cQuery[75];
  float fCrowdingEffect, //tree's crowding effect
  fNCI = 0, //the NCI
  fSizeEffect, //tree's size effect
  fTDbh, //tree's dbh
  fDistance, //distance between target and neighbor
  fNDbh, //neighbor's dbh
  fCrownDepth,
  fHeight,
  fNeighX, fNeighY, //holders for the neighbor tree's X and Y
  fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iIsDead;
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iTargetSpecies = p_oTree->GetSpecies(), //target tree's species
  iTargetType = p_oTree->GetType(),
  iDeadCode; //neighbor's dead code

  p_oTree->GetValue(m_oPop->GetDbhCode(iTargetSpecies, iTargetType), &fTDbh);

  //Get NCI
  //Format the query to get all competing neighbors
  p_oTree->GetValue(m_oPop->GetXCode(iTargetSpecies, iTargetType), &fTargetX);
  p_oTree->GetValue(m_oPop->GetYCode(iTargetSpecies, iTargetType), &fTargetY);

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf(cQuery, "%s%f%s%f%s%f%s%f", "distance=",
      mp_fNCICDMaxCrowdingRadius[iTargetSpecies], "FROM x=", fTargetX, "y=",
      fTargetY, "::height=", m_fMinSaplingHeight);
  p_oAllNeighbors = m_oPop->Find(cQuery);

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while (p_oNeighbor) {
    if (p_oNeighbor != p_oTree) {
      iNeighSpecies = p_oNeighbor->GetSpecies();
      iNeighType = p_oNeighbor->GetType();
      if (clTreePopulation::seedling != iNeighType && (clTreePopulation::snag
          != iNeighType)) {
        //Get the neighbor's dbh
        p_oNeighbor->GetValue(m_oPop->GetDbhCode(iNeighSpecies, iNeighType),
            &fNDbh);
        if (fNDbh >= mp_fNCICDMinNeighborDBH[iNeighSpecies]) {

          //Make sure the neighbor's not dead
          iDeadCode = m_oPop->GetIntDataCode("dead", p_oNeighbor->GetSpecies(),
              p_oNeighbor->GetType());
          if (-1 != iDeadCode)
            p_oNeighbor->GetValue(iDeadCode, &iIsDead);
          else
            iIsDead = notdead;

          if (notdead == iIsDead) {

            //Get the neighbor's X and Y values
            p_oNeighbor->GetValue(m_oPop->GetXCode(iNeighSpecies, iNeighType),
                &fNeighX);
            p_oNeighbor->GetValue(m_oPop->GetYCode(iNeighSpecies, iNeighType),
                &fNeighY);

            //Get the distance between the two trees
            fDistance = p_oPlot->GetDistance(fTargetX, fTargetY, fNeighX,
                fNeighY);

            //Only continue if distance is not 0 - it will be a fluke condition to
            //allow a tree that is literally standing on top of another one not to
            //affect it competitively, but there it is
            if (0 != fDistance)

              //Add competitive effect to NCI
              fNCI += mp_fNCICDLambda[iTargetSpecies][iNeighSpecies] * (pow(
                  fNDbh, mp_fNCICDAlpha[iTargetSpecies]) / pow(fDistance,
                      mp_fNCICDBeta[iTargetSpecies]));

          }
        }
      }
    }
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  //Calculate the crowding effect
  fNCI *= exp(mp_fNCICDGamma[iTargetSpecies] * fTDbh);
  fCrowdingEffect = exp(-mp_fNCICDN[iTargetSpecies] * fNCI);
  //Make sure it's between 0 and 1
  if (fCrowdingEffect < 0)
    fCrowdingEffect = 0;
  if (fCrowdingEffect > 1)
    fCrowdingEffect = 1;

  //Get the tree's size effect
  fSizeEffect = 1 - exp(-mp_fNCICDD[iTargetSpecies] * fTDbh);

  //Make sure it's bounded between 0 and 1
  if (fSizeEffect < 0)
    fSizeEffect = 0;
  if (fSizeEffect > 1)
    fSizeEffect = 1;

  //Calculate crown depth
  fCrownDepth = mp_fNCIMaxCrownDepth[iTargetSpecies] * fSizeEffect * fCrowdingEffect;

  //Make sure crown depth is not greater than total tree height
  p_oTree->GetValue(m_oPop->GetHeightCode(iTargetSpecies, iTargetType), &fHeight);
  if (fCrownDepth > fHeight) fCrownDepth = fHeight;

  return fCrownDepth;
}
