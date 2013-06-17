//---------------------------------------------------------------------------
#include "MichMenGrowthBase.h"
#include "GrowthOrg.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "TreePopulation.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clMichMenBase::clMichMenBase(clSimManager *p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clGrowthBase(p_oSimManager) {
  mp_fSlopeDiamGrowthResponse = NULL;
  mp_fSlopeHeightGrowthResponse = NULL;
  mp_fAsympDiamGrowth = NULL;
  mp_fAsympHeightGrowth = NULL;
  mp_fAdultConstRadInc = NULL;
  mp_fAdultConstBAInc = NULL;
  m_bConstBasalAreaLimited = false;
  m_bConstRadialLimited = false;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clMichMenBase::~clMichMenBase() {
  if (mp_fSlopeDiamGrowthResponse) {
    delete[] mp_fSlopeDiamGrowthResponse;
    mp_fSlopeDiamGrowthResponse = NULL;
  }

  if (mp_fSlopeHeightGrowthResponse) {
    delete[] mp_fSlopeHeightGrowthResponse;
    mp_fSlopeHeightGrowthResponse = NULL;
  }

  if (mp_fAsympDiamGrowth) {
    delete[] mp_fAsympDiamGrowth;
    mp_fAsympDiamGrowth = NULL;
  }

  if (mp_fAsympHeightGrowth) {
    delete[] mp_fAsympHeightGrowth;
    mp_fAsympHeightGrowth = NULL;
  }

  if (mp_fAdultConstRadInc) {
    delete[] mp_fAdultConstRadInc;
    mp_fAdultConstRadInc = NULL;
  }

  if (mp_fAdultConstBAInc) {
    delete[] mp_fAdultConstBAInc;
    mp_fAdultConstBAInc = NULL;
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
//////////////////////////////////////////////////////////////////////////////
void clMichMenBase::GetParameterFileData(DOMDocument *p_oDoc) {
  try {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
    floatVal *p_fTempValues;
    float fConversionFactor, //for scaling values from per year to per timestep
    fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    short int i; //loop counter

    //Diameter and height growth sort themselves out automatically by which
    //arrays are declared.

    //Set up the temp array
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
    p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Fill the values from the parameter file
    //Slope of diameter growth response
    if (mp_fSlopeDiamGrowthResponse) {
      FillSpeciesSpecificValue(p_oElement, "gr_slopeGrowthResponse", "gr_sgrVal",
          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

      //Transfer to the array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fSlopeDiamGrowthResponse[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Slope of height growth response
    if (mp_fSlopeHeightGrowthResponse) {
      FillSpeciesSpecificValue(p_oElement, "gr_slopeHeightGrowthResponse",
          "gr_shgrVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

      //Transfer to the array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fSlopeHeightGrowthResponse[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Asymptotic diameter growth
    if (mp_fAsympDiamGrowth) {
      FillSpeciesSpecificValue(p_oElement, "gr_asympDiameterGrowth", "gr_adgVal",
          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

      //Transfer to the array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fAsympDiamGrowth[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Asymptotic height growth
    if (mp_fAsympHeightGrowth) {
      FillSpeciesSpecificValue(p_oElement, "gr_asympHeightGrowth", "gr_ahgVal",
          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

      //Transfer to the array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fAsympHeightGrowth[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Adult constant radial growth increment
    if (mp_fAdultConstRadInc) {
      FillSpeciesSpecificValue(p_oElement, "gr_adultConstRadialInc",
          "gr_acriVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

      for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fAdultConstRadInc[p_fTempValues[i].code] = p_fTempValues[i].val;

    }

    //Adult constant area growth increment
    if (mp_fAdultConstBAInc) {

      FillSpeciesSpecificValue(p_oElement, "gr_adultConstAreaInc", "gr_acaiVal",
          p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

      for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fAdultConstBAInc[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Scale the constant radial increment from mm of radial growth per year to
    //cm of diameter growth per timestep.
    //'*2'=>convert radius to dbh; '/10'=>convert mm to cm; *number of years
    //per timestep
    fConversionFactor = 0.2 * fNumberYearsPerTimestep;

    if (mp_fAdultConstRadInc) {
      for (i = 0; i < m_iNumBehaviorSpecies; i++)

      mp_fAdultConstRadInc[mp_iWhatSpecies[i]] *= fConversionFactor;
    }

    //Scale basal area from square cm/year to sqare mm/timestep.
    //*100=>convert from square cm to square mm
    fConversionFactor = 100 * fNumberYearsPerTimestep;

    if (mp_fAdultConstBAInc) {
      for (i = 0; i < m_iNumBehaviorSpecies; i++)

      mp_fAdultConstBAInc[mp_iWhatSpecies[i]] =
      mp_fAdultConstBAInc[mp_iWhatSpecies[i]] * fConversionFactor;
    }

    //Scale the slope of growth response parameter
    if (mp_fSlopeDiamGrowthResponse && mp_fAsympDiamGrowth) {
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
      if (mp_fSlopeDiamGrowthResponse[mp_iWhatSpecies[i]] != 0.0)
      mp_fSlopeDiamGrowthResponse[mp_iWhatSpecies[i]] =
      mp_fAsympDiamGrowth[mp_iWhatSpecies[i]] /
      mp_fSlopeDiamGrowthResponse[mp_iWhatSpecies[i]];
    }

    if (mp_fSlopeHeightGrowthResponse && mp_fAsympHeightGrowth) {
        for (i = 0; i < m_iNumBehaviorSpecies; i++)
        if (mp_fSlopeHeightGrowthResponse[mp_iWhatSpecies[i]] != 0.0)
          mp_fSlopeHeightGrowthResponse[mp_iWhatSpecies[i]] =
            mp_fAsympHeightGrowth[mp_iWhatSpecies[i]] /
        mp_fSlopeHeightGrowthResponse[mp_iWhatSpecies[i]];
    }

    delete[] p_fTempValues;

  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMichMenBase::GetParameterFileData";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// ApplyGrowthLimits()
//////////////////////////////////////////////////////////////////////////////
float clMichMenBase::ApplyGrowthLimits(const short int &iSpecies,
    const float &fAmountDiamIncrease, const float &fDiam) {
  float fGrowthFromIncrement;

  //If growth is limited to either constant radial increment or constant
  //basal increment, take the minimum of that and fAmountDiamIncrease.
  if (m_bConstRadialLimited) {
    fGrowthFromIncrement = (mp_fAdultConstRadInc[iSpecies]);
    if (fGrowthFromIncrement < fAmountDiamIncrease)
      return fGrowthFromIncrement;
    else return fAmountDiamIncrease;
  } else if (m_bConstBasalAreaLimited) {
    fGrowthFromIncrement = mp_fAdultConstBAInc[iSpecies] / fDiam;
    if (fGrowthFromIncrement < fAmountDiamIncrease)
      return fGrowthFromIncrement;
    else return fAmountDiamIncrease;
  }
  else return fAmountDiamIncrease;
}

//////////////////////////////////////////////////////////////////////////////
// GetGrowthMemberValue()
//////////////////////////////////////////////////////////////////////////////
float clMichMenBase::GetGrowthMemberValue(clTree *p_oTree, float fDiameterGrowth) {

  float fGrowth, fDiam = 0, fGli = 0;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType(),
      iGLICode = mp_oGrowthOrg->GetLightCode(iSp, iTp);

  //Get the tree's diameter and GLI
  p_oTree->GetValue(mp_oGrowthOrg->GetDiamCode(iSp, iTp), &fDiam);
  if (iGLICode > -1) {
    p_oTree->GetValue(mp_oGrowthOrg->GetLightCode(iSp, iTp), &fGli);
  }

  //Prevent older trees that get plenty of light and grow slowly from dying
  //by setting a higher value (if they're basal-area limited)
  if (m_bConstBasalAreaLimited && fDiam > 30.0 && fGli > 10.0)
    fGrowth = 10.0;
  else
    //Otherwise use the amount of growth - convert to radial growth in mm/year
    fGrowth = fDiameterGrowth * m_fConvertCmPerTSToMmPerYr;

  return fGrowth;
}

