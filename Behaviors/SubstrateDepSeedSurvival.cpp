//---------------------------------------------------------------------------
#include "SubstrateDepSeedSurvival.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "Plot.h"
#include <stdio.h>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clSubstrateDepSeedSurvival::clSubstrateDepSeedSurvival(
    clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try {
    m_sNameString = "Substrate Dependent Seed Survival";
    m_sXMLRoot = "SubstrateDependentSeedSurvival";

    mp_iIndexes = NULL;
    mp_fCanGroundScarSoilFav = NULL;
    mp_fCanGroundTipUpFav = NULL;
    mp_fCanGroundFreshLogFav = NULL;
    mp_fCanGroundDecLogFav = NULL;
    mp_fCanGroundForFlLitterFav = NULL;
    mp_fCanGroundForFlMossFav = NULL;
    mp_fGapMoundScarSoilFav = NULL;
    mp_fGapMoundTipUpFav = NULL;
    mp_fGapMoundFreshLogFav = NULL;
    mp_fGapMoundDecLogFav = NULL;
    mp_fGapMoundForFlLitterFav = NULL;
    mp_fGapMoundForFlMossFav = NULL;
    mp_iSubstrFavCode = NULL;
    mp_iSeedGridCode = NULL;

    mp_oSubstrateFavGrid = NULL;
    mp_oSubstrateGrid = NULL;
    mp_oSeedGrid = NULL;

    m_iTipUpCode = -1;
    m_iScarifiedSoilCode = -1;
    m_iForestFloorMossCode = -1;
    m_bUsesGap = false;
    m_fMoundProp = 0;
    m_iFreshLogCode = -1;
    m_iIsGapCode = -1;
    m_bUsesMicro = false;
    m_iDecayedLogCode = -1;
    m_iForestFloorLitterCode = -1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1.1;
    m_fMinimumVersionNumber = 1;
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSubstrateDepSeedSurvival::clSubstrateDepSeedSurvival";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clSubstrateDepSeedSurvival::~clSubstrateDepSeedSurvival() {

  delete[] mp_fCanGroundScarSoilFav;
  delete[] mp_fCanGroundTipUpFav;
  delete[] mp_fCanGroundFreshLogFav;
  delete[] mp_fCanGroundDecLogFav;
  delete[] mp_fCanGroundForFlLitterFav;
  delete[] mp_fCanGroundForFlMossFav;
  delete[] mp_fGapMoundScarSoilFav;
  delete[] mp_fGapMoundTipUpFav;
  delete[] mp_fGapMoundFreshLogFav;
  delete[] mp_fGapMoundDecLogFav;
  delete[] mp_fGapMoundForFlLitterFav;
  delete[] mp_fGapMoundForFlMossFav;
  delete[] mp_iIndexes;
  delete[] mp_iSubstrFavCode;
  delete[] mp_iSeedGridCode;
}

///////////////////////////////////////////////////////////////////////////////
// SetNameData
///////////////////////////////////////////////////////////////////////////////
void clSubstrateDepSeedSurvival::SetNameData(std::string sNameString) {
  try {
    if (sNameString.compare("NoGapSubstrateSeedSurvival") == 0) {
      m_bUsesGap = false;
      m_bUsesMicro = false;
    } else if (sNameString.compare("GapSubstrateSeedSurvival") == 0) {
      m_bUsesGap = true;
      m_bUsesMicro = false;
    } else if (sNameString.compare("MicrotopographicSubstrateSeedSurvival") == 0) {
      m_bUsesMicro = true;
      m_bUsesGap = false;
    } else { //error!
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSubstrateDepSeedSurvival::SetNameData";
      stcErr.sMoreInfo = "Unrecognized name ";
      stcErr.sMoreInfo += sNameString;
      throw(stcErr);
    }
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSubstrateDepSeedSurvival::SetNameData";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetParameterFileData
/////////////////////////////////////////////////////////////////////////////
void clSubstrateDepSeedSurvival::GetParameterFileData(
    xercesc::DOMDocument * p_oDoc) {
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  try {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop =
        (clTreePopulation *) mp_oSimManager->GetPopulationObject(
            "treepopulation");

    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Declare our data arrays
    mp_iIndexes = new short int[iNumSpecies];
    //Load the indexes array
    //Make the list of indexes
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the canopy array values
    mp_fCanGroundScarSoilFav = new double[m_iNumBehaviorSpecies];
    mp_fCanGroundTipUpFav = new double[m_iNumBehaviorSpecies];
    mp_fCanGroundFreshLogFav = new double[m_iNumBehaviorSpecies];
    mp_fCanGroundDecLogFav = new double[m_iNumBehaviorSpecies];
    mp_fCanGroundForFlLitterFav = new double[m_iNumBehaviorSpecies];
    mp_fCanGroundForFlMossFav = new double[m_iNumBehaviorSpecies];

    //Set up our temp array - pre-load with this behavior's species
    p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      p_fTemp[i].code = mp_iWhatSpecies[i];

    //Canopy scarified soil favorability
    FillSpeciesSpecificValue(p_oElement, "es_scarifiedSoilCanopyFav",
        "es_sscfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fCanGroundScarSoilFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

    //Canopy tip-up mound favorability
    FillSpeciesSpecificValue(p_oElement, "es_tipUpCanopyFav", "es_tucfVal",
        p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fCanGroundTipUpFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

    //Canopy fresh log favorability
    FillSpeciesSpecificValue(p_oElement, "es_freshLogCanopyFav", "es_flcfVal",
        p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fCanGroundFreshLogFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

    //Canopy decayed log favorability
    FillSpeciesSpecificValue(p_oElement, "es_decayedLogCanopyFav",
        "es_dlcfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fCanGroundDecLogFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

    //Canopy forest floor litter favorability
    FillSpeciesSpecificValue(p_oElement, "es_forestFloorLitterCanopyFav",
        "es_fflcfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fCanGroundForFlLitterFav[mp_iIndexes[p_fTemp[i].code]]
          = p_fTemp[i].val;

    //Canopy forest floor moss favorability
    FillSpeciesSpecificValue(p_oElement, "es_forestFloorMossCanopyFav",
        "es_ffmcfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fCanGroundForFlMossFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

    //Verify that all values are between 0 and 1
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      if (mp_fCanGroundScarSoilFav[i] < 0 || mp_fCanGroundScarSoilFav[i] > 1
          || mp_fCanGroundTipUpFav[i] < 0 || mp_fCanGroundTipUpFav[i] > 1
          || mp_fCanGroundFreshLogFav[i] < 0 || mp_fCanGroundFreshLogFav[i] > 1
          || mp_fCanGroundDecLogFav[i] < 0 || mp_fCanGroundDecLogFav[i] > 1
          || mp_fCanGroundForFlLitterFav[i] < 0
          || mp_fCanGroundForFlLitterFav[i] > 1 || mp_fCanGroundForFlMossFav[i]
          < 0 || mp_fCanGroundForFlMossFav[i] > 1) {
        modelErr stcErr;
        stcErr.sFunction = "clSubstrateDepSeedSurvival::GetParameterFileData";
        stcErr.sMoreInfo = "All substrate favorabilities must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        throw(stcErr);
      }
    }

    if (m_bUsesGap) {
      //Declare the gap array values
      mp_fGapMoundScarSoilFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundTipUpFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundFreshLogFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundDecLogFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundForFlLitterFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundForFlMossFav = new double[m_iNumBehaviorSpecies];

      //Load the gap values
      //Gap scarified soil favorability
      FillSpeciesSpecificValue(p_oElement, "es_scarifiedSoilGapFav",
          "es_ssgfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundScarSoilFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Gap tip-up mound favorability
      FillSpeciesSpecificValue(p_oElement, "es_tipUpGapFav", "es_tugfVal",
          p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundTipUpFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Gap fresh log favorability
      FillSpeciesSpecificValue(p_oElement, "es_freshLogGapFav", "es_flgfVal",
          p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundFreshLogFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Gap decayed log favorability
      FillSpeciesSpecificValue(p_oElement, "es_decayedLogGapFav", "es_dlgfVal",
          p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundDecLogFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Gap forest floor litter favorability
      FillSpeciesSpecificValue(p_oElement, "es_forestFloorLitterGapFav",
          "es_fflgfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundForFlLitterFav[mp_iIndexes[p_fTemp[i].code]]
            = p_fTemp[i].val;

      //Gap forest floor moss favorability
      FillSpeciesSpecificValue(p_oElement, "es_forestFloorMossGapFav",
          "es_ffmgfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundForFlMossFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Verify that all values are between 0 and 1
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        if (mp_fGapMoundScarSoilFav[i] < 0 || mp_fGapMoundScarSoilFav[i] > 1
            || mp_fGapMoundTipUpFav[i] < 0 || mp_fGapMoundTipUpFav[i] > 1
            || mp_fGapMoundFreshLogFav[i] < 0 || mp_fGapMoundFreshLogFav[i] > 1
            || mp_fGapMoundDecLogFav[i] < 0 || mp_fGapMoundDecLogFav[i] > 1
            || mp_fGapMoundForFlLitterFav[i] < 0
            || mp_fGapMoundForFlLitterFav[i] > 1 || mp_fGapMoundForFlMossFav[i]
            < 0 || mp_fGapMoundForFlMossFav[i] > 1) {
          modelErr stcErr;
          stcErr.sFunction = "clSubstrateDepSeedSurvival::GetParameterFileData";
          stcErr.sMoreInfo = "All substrate favorabilities must be between 0 and 1.";
          stcErr.iErrorCode = BAD_DATA;
          throw(stcErr);
        }
      }
    }

    //If using microtopography...
    if (m_bUsesMicro) {
      //Declare the gap array values
      mp_fGapMoundScarSoilFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundTipUpFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundFreshLogFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundDecLogFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundForFlLitterFav = new double[m_iNumBehaviorSpecies];
      mp_fGapMoundForFlMossFav = new double[m_iNumBehaviorSpecies];

      //Proportion of plot that is mound
      FillSingleValue(p_oElement, "es_moundProportion", &m_fMoundProp, true);
      if (m_fMoundProp < 0 || m_fMoundProp > 1) {
        modelErr stcErr;
        stcErr.sFunction = "clSubstrateDepSeedSurvival::GetParameterFileData";
        stcErr.sMoreInfo = "Mound proportion must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        throw(stcErr);
      }

      //Mound scarified soil favorability
      FillSpeciesSpecificValue(p_oElement, "es_scarifiedSoilMoundFav",
          "es_ssmfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundScarSoilFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Mound tip-up mound favorability
      FillSpeciesSpecificValue(p_oElement, "es_tipUpMoundFav", "es_tumfVal",
          p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundTipUpFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Mound fresh log favorability
      FillSpeciesSpecificValue(p_oElement, "es_freshLogMoundFav", "es_flmfVal",
          p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundFreshLogFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Mound decayed log favorability
      FillSpeciesSpecificValue(p_oElement, "es_decayedLogMoundFav",
          "es_dlmfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundDecLogFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Mound forest floor litter favorability
      FillSpeciesSpecificValue(p_oElement, "es_forestFloorLitterMoundFav",
          "es_fflmfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundForFlLitterFav[mp_iIndexes[p_fTemp[i].code]]
            = p_fTemp[i].val;

      //Mound forest floor moss favorability
      FillSpeciesSpecificValue(p_oElement, "es_forestFloorMossMoundFav",
          "es_ffmmfVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      //Now transfer the values to the permanent array
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fGapMoundForFlMossFav[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Verify that all values are between 0 and 1
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        if (mp_fGapMoundScarSoilFav[i] < 0 || mp_fGapMoundScarSoilFav[i] > 1
            || mp_fGapMoundTipUpFav[i] < 0 || mp_fGapMoundTipUpFav[i] > 1
            || mp_fGapMoundFreshLogFav[i] < 0 || mp_fGapMoundFreshLogFav[i] > 1
            || mp_fGapMoundDecLogFav[i] < 0 || mp_fGapMoundDecLogFav[i] > 1
            || mp_fGapMoundForFlLitterFav[i] < 0
            || mp_fGapMoundForFlLitterFav[i] > 1 || mp_fGapMoundForFlMossFav[i]
            < 0 || mp_fGapMoundForFlMossFav[i] > 1) {
          modelErr stcErr;
          stcErr.sFunction = "clSubstrateDepSeedSurvival::GetParameterFileData";
          stcErr.sMoreInfo = "All substrate favorabilities must be between 0 and 1.";
          stcErr.iErrorCode = BAD_DATA;
          throw(stcErr);
        }
      }
    }

    delete[] p_fTemp;
  } catch (modelErr& err) {
    delete[] p_fTemp;
    throw(err);
  } catch (modelMsg & msg) {
    delete[] p_fTemp;
    throw(msg);
  } //non-fatal error
  catch (...) {
    delete[] p_fTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSubstrateDepSeedSurvival::GetParameterFileData";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clSubstrateDepSeedSurvival::GetData(xercesc::DOMDocument * p_oDoc) {
  try {

    GetParameterFileData(p_oDoc);

    //Set up the grids
    SetupGrids();
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSubstrateDepSeedSurvival::GetData";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Action
///////////////////////////////////////////////////////////////////////////////
void clSubstrateDepSeedSurvival::Action() {
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  float
      fCover, //value of a particular substrate cover
      fIndex, //favorability index
      fSeedX,
      fSeedY, //coordinates of seed
      fXOrig,
      fYOrig, //coordinates of origin of a grid cell
      fNumSeeds, //number of seeds per grid
      fSurvivingSeeds, //number of seeds surviving per grid
      fXCellLength = mp_oSeedGrid->GetLengthXCells(), fYCellLength =
          mp_oSeedGrid->GetLengthYCells(), fScarifiedSoilFav,
      fForestFloorLitterFav, fTipUpFav, fFreshLogFav, fDecayedLogFav,
      fForestFloorMossFav;
  short int iNumXCells = mp_oSubstrateFavGrid->GetNumberXCells(),
            iNumYCells = mp_oSubstrateFavGrid->GetNumberYCells(),
            iX, iY, iSp, i, j, k; //loop counters
  bool bIsGap; //whether or not a grid cell is under gap cover

  if (false == m_bUsesMicro) {
    for (iX = 0; iX < iNumXCells; iX++)
      for (iY = 0; iY < iNumYCells; iY++) {

        //Get the gap status for this grid cell if gap is being used
        if (m_bUsesGap) {
          mp_oSubstrateFavGrid->GetPointOfCell(iX, iY, &fSeedX, &fSeedY);
          mp_oSeedGrid->GetValueAtPoint(fSeedX, fSeedY, m_iIsGapCode, &bIsGap);
        } else {
          bIsGap = false;
        }
        //Calculate the index for each species
        for (k = 0; k < m_iNumBehaviorSpecies; k++) {
          if (bIsGap) {
            fScarifiedSoilFav
                = mp_fGapMoundScarSoilFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fForestFloorLitterFav
                = mp_fGapMoundForFlLitterFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fForestFloorMossFav
                = mp_fGapMoundForFlMossFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fTipUpFav = mp_fGapMoundTipUpFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fFreshLogFav
                = mp_fGapMoundFreshLogFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fDecayedLogFav
                = mp_fGapMoundDecLogFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          } else {
            fScarifiedSoilFav
                = mp_fCanGroundScarSoilFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fForestFloorLitterFav
                = mp_fCanGroundForFlLitterFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fForestFloorMossFav
                = mp_fCanGroundForFlMossFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fTipUpFav = mp_fCanGroundTipUpFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fFreshLogFav
                = mp_fCanGroundFreshLogFav[mp_iIndexes[mp_iWhatSpecies[k]]];
            fDecayedLogFav
                = mp_fCanGroundDecLogFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          }

          //Get scarified soil
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iScarifiedSoilCode,
              &fCover);
          //Multiply it by scarified soil favorability for our cover
          fIndex = fCover * fScarifiedSoilFav;

          //Get forest floor litter, multiply it by the favorability, and add
          //it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iForestFloorLitterCode,
              &fCover);
          fIndex += fCover * fForestFloorLitterFav;

          //Get forest floor moss, multiply it by the favorability, and add
          //it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iForestFloorMossCode,
              &fCover);
          fIndex += fCover * fForestFloorMossFav;

          //Get tip-up, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iTipUpCode, &fCover);
          fIndex += fCover * fTipUpFav;

          //Get fresh log, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iFreshLogCode, &fCover);
          fIndex += fCover * fFreshLogFav;

          //Get decayed log, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iDecayedLogCode, &fCover);
          fIndex += fCover * fDecayedLogFav;

          //Put the index in our grid
          mp_oSubstrateFavGrid->SetValueOfCell(iX, iY,
              mp_iSubstrFavCode[mp_iWhatSpecies[k]], fIndex);
        }
      } //end of for (j = 0; j < iNumYCells; j++)
  } else {
    float fIndex2;
    //Loop with microtopography
    for (iX = 0; iX < iNumXCells; iX++)
      for (iY = 0; iY < iNumYCells; iY++) {
        //Calculate the index for each species
        for (k = 0; k < m_iNumBehaviorSpecies; k++) {
          //Mound favorabilities
          fScarifiedSoilFav
              = mp_fGapMoundScarSoilFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fForestFloorLitterFav
              = mp_fGapMoundForFlLitterFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fForestFloorMossFav
              = mp_fGapMoundForFlMossFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fTipUpFav = mp_fGapMoundTipUpFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fFreshLogFav
              = mp_fGapMoundFreshLogFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fDecayedLogFav
              = mp_fGapMoundDecLogFav[mp_iIndexes[mp_iWhatSpecies[k]]];

          //Get scarified soil
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iScarifiedSoilCode,
              &fCover);
          //Multiply it by scarified soil favorability for our cover
          fIndex = fCover * fScarifiedSoilFav;

          //Get forest floor litter, multiply it by the favorability, and add
          //it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iForestFloorLitterCode,
              &fCover);
          fIndex += fCover * fForestFloorLitterFav;

          //Get forest floor moss, multiply it by the favorability, and add
          //it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iForestFloorMossCode,
              &fCover);
          fIndex += fCover * fForestFloorMossFav;

          //Get tip-up, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iTipUpCode, &fCover);
          fIndex += fCover * fTipUpFav;

          //Get fresh log, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iFreshLogCode, &fCover);
          fIndex += fCover * fFreshLogFav;

          //Get decayed log, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iDecayedLogCode, &fCover);
          fIndex += fCover * fDecayedLogFav;

          //Multiply the index by the portion that is mound
          fIndex *= m_fMoundProp;

          //Ground favorabilities
          fScarifiedSoilFav
              = mp_fCanGroundScarSoilFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fForestFloorLitterFav
              = mp_fCanGroundForFlLitterFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fForestFloorMossFav
              = mp_fCanGroundForFlMossFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fTipUpFav = mp_fCanGroundTipUpFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fFreshLogFav
              = mp_fCanGroundFreshLogFav[mp_iIndexes[mp_iWhatSpecies[k]]];
          fDecayedLogFav
              = mp_fCanGroundDecLogFav[mp_iIndexes[mp_iWhatSpecies[k]]];

          //Get scarified soil
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iScarifiedSoilCode,
              &fCover);
          //Multiply it by scarified soil favorability for our cover
          fIndex2 = fCover * fScarifiedSoilFav;

          //Get forest floor litter, multiply it by the favorability, and add
          //it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iForestFloorLitterCode,
              &fCover);
          fIndex2 += fCover * fForestFloorLitterFav;

          //Get forest floor moss, multiply it by the favorability, and add
          //it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iForestFloorMossCode,
              &fCover);
          fIndex2 += fCover * fForestFloorMossFav;

          //Get tip-up, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iTipUpCode, &fCover);
          fIndex2 += fCover * fTipUpFav;

          //Get fresh log, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iFreshLogCode, &fCover);
          fIndex2 += fCover * fFreshLogFav;

          //Get decayed log, multiply it by the favorability, and add it in
          mp_oSubstrateGrid->GetValueOfCell(iX, iY, m_iDecayedLogCode, &fCover);
          fIndex2 += fCover * fDecayedLogFav;

          fIndex2 *= (1 - m_fMoundProp);
          fIndex += fIndex2;

          //Put the index in our grid
          mp_oSubstrateFavGrid->SetValueOfCell(iX, iY,
              mp_iSubstrFavCode[mp_iWhatSpecies[k]], fIndex);
        }
      } //end of for (j = 0; j < iNumYCells; j++)
  }

  //Calculate seed survival
  iNumXCells = mp_oSeedGrid->GetNumberXCells();
  iNumYCells = mp_oSeedGrid->GetNumberYCells();

  //Loop through each grid cell
  for (iX = 0; iX < iNumXCells; iX++) {
    for (iY = 0; iY < iNumYCells; iY++) {

      //Get the origin of this cell (min X and Y values)
      fXOrig = fXCellLength * iX;
      fYOrig = fYCellLength * iY;

      //Loop through each behavior species
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {

        iSp = mp_iWhatSpecies[i];
        fSurvivingSeeds = 0;

        //Get the number of seeds in this cell for this species
        mp_oSeedGrid->GetValueOfCell(iX, iY, mp_iSeedGridCode[iSp], &fNumSeeds);
        fNumSeeds = clModelMath::RandomRound(fNumSeeds);

        //Disperse the seeds produced
        for (j = 0; j < fNumSeeds; j++) {

          //Get a random X and Y value for this seed within the grid cell
          fSeedX = p_oPlot->CorrectX(fXOrig + (clModelMath::GetRand()
              * fXCellLength));
          fSeedY = p_oPlot->CorrectY(fYOrig + (clModelMath::GetRand()
              * fYCellLength));

          //Compute substrate germination probability - compare
          //substrate favorability index to a random number
          mp_oSubstrateFavGrid->GetValueAtPoint(fSeedX, fSeedY,
              mp_iSubstrFavCode[iSp], &fIndex);

          if (clModelMath::GetRand() < fIndex) {
            //seed survives
            fSurvivingSeeds++;
          }

          //Set the surviving seeds back into the grid cell
          mp_oSeedGrid->SetValueOfCell(iX, iY, mp_iSeedGridCode[iSp],
              fSurvivingSeeds);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// SetUpGrids()
/////////////////////////////////////////////////////////////////////////////
void clSubstrateDepSeedSurvival::SetupGrids() {
  try {

    clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject(
        "treepopulation");
    std::stringstream sLabel;
    short int iLengthXCells, iLengthYCells, //length of cells for grid
        iNumSpecies = p_oPop->GetNumberOfSpecies(), i; //loop counters


    //////////////////////////////////////////////
    // First the substrate grid
    //////////////////////////////////////////////
    mp_oSubstrateGrid = mp_oSimManager->GetGridObject("Substrate");
    if (!mp_oSubstrateGrid) {
      modelErr stcErr;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      stcErr.sFunction = "clSubstrateDepSeedSurvival::SetupGrids";
      stcErr.sMoreInfo = "Substrate behavior required.";
      throw(stcErr);
    }

    //Get the data member codes for the substrate grid
    m_iScarifiedSoilCode = mp_oSubstrateGrid->GetFloatDataCode("scarsoil");
    m_iForestFloorLitterCode = mp_oSubstrateGrid->GetFloatDataCode("fflitter");
    m_iForestFloorMossCode = mp_oSubstrateGrid->GetFloatDataCode("ffmoss");
    m_iTipUpCode = mp_oSubstrateGrid->GetFloatDataCode("tipup");
    m_iFreshLogCode = mp_oSubstrateGrid->GetFloatDataCode("freshlog");
    m_iDecayedLogCode = mp_oSubstrateGrid->GetFloatDataCode("declog");

    //If any of the codes are -1 throw an error
    if (-1 == m_iScarifiedSoilCode || -1 == m_iForestFloorLitterCode || -1
        == m_iTipUpCode || -1 == m_iFreshLogCode || -1 == m_iDecayedLogCode
        || -1 == m_iForestFloorMossCode) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSubstrateDepSeedSurvival::SetUpGrids";
      stcErr.sMoreInfo = "Unexpected grid data return code.";
      throw(stcErr);
    }

    if (NULL == mp_oSubstrateFavGrid) {
      //Get the length of the X cells and Y cells of substrate grid
      iLengthXCells = (int) mp_oSubstrateGrid->GetLengthXCells();
      iLengthYCells = (int) mp_oSubstrateGrid->GetLengthYCells();

      //Create the substrate favorability grid
      mp_oSubstrateFavGrid = mp_oSimManager->CreateGrid(
          "Substrate Favorability", //grid name
          0, //number of int data members
          iNumSpecies, //number of float data members
          0, //number of char data members
          0, //number of bool data members
          iLengthXCells, //X cell length
          iLengthYCells); //Y cell length

      mp_iSubstrFavCode = new short int[iNumSpecies];

      //Register the data members
      for (i = 0; i < iNumSpecies; i++) {
        sLabel << "Favorability Index" << i;
        mp_iSubstrFavCode[i] = mp_oSubstrateFavGrid->RegisterFloat(sLabel.str());
        sLabel.str("");
      }
    }

    //////////////////////////////////////////////
    // Now the dispersed seeds grid
    //////////////////////////////////////////////
    mp_oSeedGrid = mp_oSimManager->GetGridObject("Dispersed Seeds");
    if (NULL == mp_oSeedGrid) {
      modelErr stcErr;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      stcErr.sFunction = "clSubstrateDepSeedSurvival::SetupGrids";
      stcErr.sMoreInfo = "Disperse behaviors are required if establishment is to be used.";
      throw(stcErr);
    }
    mp_iSeedGridCode = new short int[iNumSpecies];
    for (i = 0; i < iNumSpecies; i++) {
      sLabel << "seeds_" << i;
      mp_iSeedGridCode[i] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iSeedGridCode[i]) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrateDepSeedSurvival::SetupGrids";
        stcErr.sMoreInfo = "Unexpected grid data return code.";
        throw(stcErr);
      }
    }

    if (m_bUsesGap) {
      m_iIsGapCode = mp_oSeedGrid->GetBoolDataCode("Is Gap");
      if (-1 == m_iIsGapCode) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrateDepSeedSurvival::SetupGrids";
        stcErr.sMoreInfo = "Unexpected grid data return code.";
        throw(stcErr);
      }
    }
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSubstrateDepSeedSurvival::SetupGrids";
    throw(stcErr);
  }
}

