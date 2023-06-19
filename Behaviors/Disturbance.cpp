//---------------------------------------------------------------------------
#include "Disturbance.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "Plot.h"
#include <stdio.h>
#include <fstream>
#include <sstream>

#define test

short int clDisturbance::m_iNumAllowedCutRanges = 4;
short int clDisturbance::m_iNumAllowedPriorities = 3;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clDisturbance::clDisturbance(clSimManager * p_oSimManager) :
              clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try {
    m_sNameString = "Harvest";

    //Versions
    m_fVersionNumber = 2.1;
    m_fMinimumVersionNumber = 1;

    //Null pointers, initialize variables to empty
    mp_oMasterCutsGrid = NULL;
    mp_oCutEventsGrid = NULL;
    mp_oResultsGrid = NULL;
    mp_iSpeciesCodes = NULL;
    mp_iRangeMinCodes = NULL;
    mp_iRangeMaxCodes = NULL;
    mp_iRangeAmountCodes = NULL;
    mp_iDenCutCodes = NULL;
    mp_iBaCutCodes = NULL;
    mp_iSeedlingCodes = NULL;
    mp_iSeedlingCutCodes = NULL;
    mp_iPriorityMaxCodes = NULL;
    mp_iPriorityMinCodes = NULL;
    mp_iPriorityNameCodes = NULL;
    mp_iPriorityTypeCodes = NULL;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Number of allowed cut ranges
    m_iNumAllowedCutRanges = 4;

    m_iMasterTimestepCode = -1;
    m_iMasterIDCode = -1;
    m_fDistYCellLen = 0;
    m_iCutTimestepCode = -1;
    m_iCutTypeCode = -1;
    m_iAmountTypeCode = -1;
    m_iTallestFirstCode = -1;
    m_iCutIDCode = -1;
    m_iHarvestTypeCode = -1;
    m_iMaxSnagClassCode = -1;
    mp_oPop = NULL;
    m_fPopCellLen = 0;
    m_iReasonCode = harvest;
    m_fDistXCellLen = 0;
    m_bIsHarvest = true;

  } catch (modelErr& err) {
    throw(err);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisturbance::clDisturbance";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clDisturbance::~clDisturbance() {
  short int i;
  delete[] mp_iSpeciesCodes;
  delete[] mp_iRangeMinCodes;
  delete[] mp_iRangeMaxCodes;
  delete[] mp_iRangeAmountCodes;
  delete[] mp_iSeedlingCodes;
  delete[] mp_iSeedlingCutCodes;
  if (mp_iDenCutCodes)
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      delete[] mp_iDenCutCodes[i];
      delete[] mp_iBaCutCodes[i];
    }
  delete[] mp_iDenCutCodes;
  delete[] mp_iBaCutCodes;
  delete[] mp_iPriorityMaxCodes;
  delete[] mp_iPriorityMinCodes;
  delete[] mp_iPriorityNameCodes;
  delete[] mp_iPriorityTypeCodes;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("Harvest") == 0) {
      m_iReasonCode = harvest;
      m_bIsHarvest = true;
      m_sXMLRoot = "Harvest";
    } else if (sNameString.compare("EpisodicMortality") == 0) {
      m_iReasonCode = disease;
      m_bIsHarvest = false;
      m_sXMLRoot = "EpisodicMortality";

    } else {
      //Throw an error
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized namestring \"" << sNameString << "\".";
      stcErr.sFunction = "clDisturbance::SetNameData";
      stcErr.sMoreInfo = s.str();
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
    stcErr.sFunction = "clDisturbance::SetNameData";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// ReadHarvestParameterFileData()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::ReadHarvestParameterFileData(xercesc::DOMDocument * p_oDoc) {
  clPackage * p_oOldPackage = NULL, //for finding where to place a new package
      *p_oNewPackage = NULL; //a newly created package
  DOMNodeList * p_oCutEventsList, //list of cut events
  *p_oCutSpecificsList; //sub-cut-event - species, cut range, cell
  DOMNode * p_oNode;
  DOMElement * p_oElement, *p_oChildElement;
  XMLCh *sVal;
  std::stringstream sTempStream;
  std::string sTemp;
  double fTemp, //for extracting float data from parameter file
  fMaxVal; //maximum cut range value
  double *p_fRanges; //for sorting cut ranges
  doubleVal * p_fSeedlingValues; //for getting species-specific values
  int iNumHarvestEvents, //total number of cut events to extract
  iX, iY, //X and Y grid values for package
  iNumberTimesteps = mp_oSimManager->GetNumberOfTimesteps(), iNumSpecies =
      mp_oPop->GetNumberOfSpecies(), iTimestep, //timestep of harvest cut event
      iMinIndex, //index of the smallest value on a list
      iNumChildren, //number of sub-cut-events
      iCutType, //cut type - value matches enum cutType
      iTemp; //for extracting integer data from parameter file
  short int i, j, k; //loop counter
  bool bTemp;

  //See if there's any harvest data in the parameter file
  sTempStream << "Harvest" << m_iBehaviorListNumber;
  sVal = XMLString::transcode(sTempStream.str().c_str());
  p_oCutEventsList = p_oDoc->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  if (0 == p_oCutEventsList->getLength())
    return;

  //Now get the list of all cut events
  //p_oNode = p_oCutEventsList->item(0);
  //p_oElement = (DOMElement *) p_oNode;
  p_oElement = GetParentParametersElement(p_oDoc);
  sVal = XMLString::transcode("ha_cutEvent");
  p_oCutEventsList = p_oElement->getElementsByTagName(sVal);
  XMLString::release(&sVal);

  //If there's no cut events, don't do anything else
  iNumHarvestEvents = p_oCutEventsList->getLength();
  if (0 == iNumHarvestEvents)
    return;

  p_fSeedlingValues = new doubleVal[iNumSpecies];
  for (i = 0; i < iNumSpecies; i++)
    p_fSeedlingValues[i].code = i;

  //*******************************************
  //Go through each cut event and add it as packages to the grids
  //*******************************************
  for (i = 0; i < iNumHarvestEvents; i++) {
    p_oNode = p_oCutEventsList->item(i);
    p_oElement = (DOMElement *) p_oNode;

    //Timestep first
    FillSingleValue(p_oElement, "ha_timestep", &iTimestep, true);

    //Validate it - needs to be between 0 and number of timesteps
    if (iTimestep <= 0 || iTimestep > iNumberTimesteps) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
      std::stringstream s;
      s << "Invalid harvest cut timestep " << iTimestep;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Add a package to the master cut events grid - they're in timestep order,
    //so keep it that way
    p_oOldPackage = NULL;
    p_oNewPackage = NULL;
    p_oOldPackage = mp_oMasterCutsGrid->GetFirstPackageOfCell(0, 0);
    while (p_oOldPackage) {
      //Get this package's timestep
      p_oOldPackage->GetValue(m_iMasterTimestepCode, &iTemp);

      if (iTemp >= iTimestep) //here's where we'll drop it
        break;

      p_oNewPackage = p_oOldPackage; //use temporarily as a holder
      p_oOldPackage = p_oOldPackage->GetNextPackage();
    }

    //Create the new package
    if (!p_oNewPackage) //either there's no packages in grid, or new is first
      p_oNewPackage = mp_oMasterCutsGrid->CreatePackageOfCell(0, 0);
    else
      //make the new one after the one in p_oNewPackage
      p_oNewPackage = mp_oMasterCutsGrid->CreatePackage(p_oNewPackage);

    //Load up the new package with the timestep value
    p_oNewPackage->SetValue(m_iMasterTimestepCode, iTimestep);

    //Get species values and assign them
    sVal = XMLString::transcode("ha_applyToSpecies");
    p_oCutSpecificsList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumChildren = p_oCutSpecificsList->getLength();
    for (j = 0; j < iNumChildren; j++) {
      p_oNode = p_oCutSpecificsList->item(j);
      p_oChildElement = (DOMElement *) p_oNode;
      iTemp = GetNodeSpeciesCode(p_oChildElement, mp_oPop);
      if (-1 == iTemp) { //throw an error
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
        stcErr.sMoreInfo = "Unrecognized species in harvest.";
        throw(stcErr);
      }
      p_oNewPackage->SetValue(mp_iSpeciesCodes[iTemp], true);
    }

    //Get the cut type
    FillSingleValue(p_oElement, "ha_cutType", &sTemp, true);
    //See if it's a value we recognize
    if (sTemp.compare("gap") == 0)
      iCutType = gap;
    else if (sTemp.compare("partial") == 0)
      iCutType = partial;
    else if (sTemp.compare("clear") == 0)
      iCutType = clear;
    else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
      stcErr.sMoreInfo = "Unrecognized value in ha_cutType.";
      throw(stcErr);
    }
    p_oNewPackage->SetValue(m_iCutTypeCode, iCutType);

    //Get the amount type if this is a partial cut
    if (partial == iCutType) {
      FillSingleValue(p_oElement, "ha_cutAmountType", &sTemp, true);
      //See if it's a value we recognize
      if (sTemp.compare("percent of basal area") == 0)
        p_oNewPackage->SetValue(m_iAmountTypeCode, percentBA);
      else if (sTemp.compare("percent of density") == 0)
        p_oNewPackage->SetValue(m_iAmountTypeCode, percentDen);
      else if (sTemp.compare("absolute basal area") == 0)
        p_oNewPackage->SetValue(m_iAmountTypeCode, absBA);
      else if (sTemp.compare("absolute density") == 0)
        p_oNewPackage->SetValue(m_iAmountTypeCode, absDen);
      else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
        stcErr.sMoreInfo = "Unrecognized value in ha_cutAmountType.";
        throw(stcErr);
      }
    } else {
      //Set the amount type to -1
      iTemp = -1;
      p_oNewPackage->SetValue(m_iAmountTypeCode, iTemp);
    }

    //-----------------------------------------------------------------------//
    //Get the tallest first flag type
    //-----------------------------------------------------------------------//
    bTemp = true;
    FillSingleValue(p_oElement, "ha_tallestFirst", &bTemp, false);
    p_oNewPackage->SetValue(m_iTallestFirstCode, bTemp);


    //-----------------------------------------------------------------------//
    //Get the max snag decay class to consider - not required; set to -1
    //by default for backwards compatibility
    //-----------------------------------------------------------------------//
    iTemp = -1;
    FillSingleValue(p_oElement, "ha_maxSnagDecayClass", &iTemp, false);
    p_oNewPackage->SetValue(m_iMaxSnagClassCode, iTemp);


    //-----------------------------------------------------------------------//
    //Get the cut ranges
    //-----------------------------------------------------------------------//
    sVal = XMLString::transcode("ha_dbhRange");
    p_oCutSpecificsList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);

    iNumChildren = p_oCutSpecificsList->getLength();
    //If there are more ranges than allowed, throw an error
    if (iNumChildren > m_iNumAllowedCutRanges) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
      stcErr.sMoreInfo = "Too many dbh cut ranges.";
      throw(stcErr);
    }
    //Or, if there aren't any and this is a partial cut, throw an error
    if (partial == iCutType && 0 == iNumChildren) {
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
      stcErr.sMoreInfo = "ha_dbhRange";
      throw(stcErr);
    }

    //Get the minimum dbh in each cut range
    p_fRanges = new double[iNumChildren];
    for (j = 0; j < iNumChildren; j++) {
      p_oNode = p_oCutSpecificsList->item(j);
      p_oChildElement = (DOMElement *) p_oNode;
      FillSingleValue(p_oChildElement, "ha_low", &fTemp, true);
      p_fRanges[j] = fTemp;

      //If the value is negative, throw an error
      if (fTemp < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
        stcErr.sMoreInfo = "Harvest cut range value cannot be negative.";
        throw(stcErr);
      }
    }

    //Sort the values and place them in the package
    fMaxVal = 0;
    for (j = 0; j < iNumChildren; j++)
      if (fMaxVal < p_fRanges[j])
        fMaxVal = p_fRanges[j];

    fMaxVal += 1;

    for (j = 0; j < iNumChildren; j++) {
      fTemp = fMaxVal;
      for (k = 0; k < iNumChildren; k++)
        if (p_fRanges[k] < fTemp) {
          iMinIndex = k;
          fTemp = p_fRanges[k];
        }

      p_oNode = p_oCutSpecificsList->item(iMinIndex);
      p_oChildElement = (DOMElement *) p_oNode;

      p_oNewPackage->SetValue(mp_iRangeMinCodes[j], (float)p_fRanges[iMinIndex]);
      FillSingleValue(p_oChildElement, "ha_high", &fTemp, true);
      p_oNewPackage->SetValue(mp_iRangeMaxCodes[j], (float)fTemp);
      FillSingleValue(p_oChildElement, "ha_amountToCut", &fTemp, true);
      p_oNewPackage->SetValue(mp_iRangeAmountCodes[j], (float)fTemp);

      p_fRanges[iMinIndex] = fMaxVal;
    }

    //Get the seedling proportion to kill - not required
    for (j = 0; j < iNumSpecies; j++)
      p_fSeedlingValues[j].val = 0;
    FillSpeciesSpecificValue(p_oElement, "ha_percentSeedlingsDie", "ha_psdVal",
        p_fSeedlingValues, iNumSpecies, mp_oPop, false);
    for (j = 0; j < iNumSpecies; j++) {
      p_oNewPackage->SetValue(mp_iSeedlingCodes[j],
          (float) (p_fSeedlingValues[j].val / 100.0));
    }

    //Get any priorites - not required
    sVal = XMLString::transcode("ha_priority");
    p_oCutSpecificsList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);

    iNumChildren = p_oCutSpecificsList->getLength();
    //If there are more ranges than allowed, throw an error
    if (iNumChildren > m_iNumAllowedPriorities) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
      stcErr.sMoreInfo = "Too many cut priorities.";
      throw(stcErr);
    }
    //Disallow for percent of density cuts
    if (iNumChildren > 0 && iCutType == percentDen) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
      stcErr.sMoreInfo = "Cut priorities cannot be used with percent density cuts.";
      throw(stcErr);
    }
    for (j = 0; j < iNumChildren; j++) {
      p_oNode = p_oCutSpecificsList->item(j);
      p_oChildElement = (DOMElement *) p_oNode;
      FillSingleValue(p_oChildElement, "ha_name", &sTemp, true);
      if (sTemp.length() == 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
        stcErr.sMoreInfo = "Priority name is missing.";
        throw(stcErr);
      }
      p_oNewPackage->SetValue(mp_iPriorityNameCodes[j], sTemp);

      FillSingleValue(p_oChildElement, "ha_type", &sTemp, true);
      //See if it's a value we recognize
      if (sTemp.compare("float") == 0)
        iTemp = floatType;
      else if (sTemp.compare("int") == 0)
        iTemp = intType;
      else if (sTemp.compare("bool") == 0)
        iTemp = boolType;
      else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
        stcErr.sMoreInfo = "Unrecognized value in ha_type.";
        throw(stcErr);
      }
      p_oNewPackage->SetValue(mp_iPriorityTypeCodes[j], iTemp);

      FillSingleValue(p_oChildElement, "ha_min", &fTemp, true);
      p_oNewPackage->SetValue(mp_iPriorityMinCodes[j], (float)fTemp);
      FillSingleValue(p_oChildElement, "ha_max", &fTemp, false);
      p_oNewPackage->SetValue(mp_iPriorityMaxCodes[j], (float)fTemp);
    }

    //Create an ID number - just use the loop counter
    p_oNewPackage->SetValue(m_iMasterIDCode, (int) i);

    //*******************************************
    //Now:  Get the list of cells to which to apply this cut and make a
    //package for each
    //*******************************************
    sVal = XMLString::transcode("ha_applyToCell");
    p_oCutSpecificsList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumChildren = p_oCutSpecificsList->getLength();
    for (j = 0; j < iNumChildren; j++) {
      p_oNode = p_oCutSpecificsList->item(j);
      p_oChildElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("x");
      sTemp = XMLString::transcode(p_oChildElement->getAttributeNode(sVal)->getNodeValue());
      iX = atoi(sTemp.c_str());
      XMLString::release(&sVal);
      sVal = XMLString::transcode("y");
      sTemp = XMLString::transcode(p_oChildElement->getAttributeNode(sVal)->getNodeValue());
      iY = atoi(sTemp.c_str());
      XMLString::release(&sVal);

      p_oOldPackage = NULL;
      p_oNewPackage = NULL;
      p_oOldPackage = mp_oCutEventsGrid->GetFirstPackageOfCell(iX, iY);
      while (p_oOldPackage) {
        //Get this package's timestep
        p_oOldPackage->GetValue(m_iCutTimestepCode, &iTemp);

        if (iTemp >= iTimestep) //here's where we'll drop it
          break;

        p_oNewPackage = p_oOldPackage; //use temporarily as a holder
        p_oOldPackage = p_oOldPackage->GetNextPackage();
      }

      //Create the new package
      if (!p_oNewPackage) //either there's no packages in grid, or new is first
        p_oNewPackage = mp_oCutEventsGrid->CreatePackageOfCell(iX, iY);
      else
        //make the new one after the one in p_oNewPackage
        p_oNewPackage = mp_oCutEventsGrid->CreatePackage(p_oNewPackage);

      //Load up the new package with the timestep and id values
      p_oNewPackage->SetValue(m_iCutTimestepCode, iTimestep);
      p_oNewPackage->SetValue(m_iCutIDCode, (int) i);
    } //end of for (j = 0; j < iNumChildren; j++)

    delete[] p_fRanges;
  } //end of for (i = 0; i < iNumHarvestEvents; i++)

  delete[] p_fSeedlingValues;
}

//////////////////////////////////////////////////////////////////////////////
// ReadMortEpParameterFileData()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::ReadMortEpParameterFileData(xercesc::DOMDocument * p_oDoc) {
  clPackage * p_oOldPackage = NULL, //for finding where to place a new package
      *p_oNewPackage = NULL; //a newly created package
  DOMNodeList * p_oCutEventsList, //list of cut events
  *p_oCutSpecificsList; //sub-cut-event - species, cut range, cell
  DOMNode * p_oNode;
  DOMElement * p_oElement, *p_oChildElement;
  XMLCh *sVal;
  std::stringstream sTempStream;
  std::string sTemp;
  double fTemp, //for extracting float data from parameter file
  fMaxVal, //maximum cut range value
  *p_fRanges; //for sorting cut ranges
  doubleVal * p_fSeedlingValues; //for getting species-specific values
  int iNumDisturbanceEvents, //total number of cut events to extract
  iX, iY, //X and Y grid values for package
  iNumberTimesteps = mp_oSimManager->GetNumberOfTimesteps(), iNumSpecies =
      mp_oPop->GetNumberOfSpecies(), iTimestep, //timestep of mortality episode
      iMinIndex, //index of the smallest value on a list
      iNumChildren, //number of sub-cut-events
      iTemp; //for extracting integer data from parameter file
  short int i, j, k; //loop counters

  //See if there's any mortality episode data in the parameter file
  sTempStream << "EpisodicMortality" << m_iBehaviorListNumber;
  sVal = XMLString::transcode(sTempStream.str().c_str());
  p_oCutEventsList = p_oDoc->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  if (0 == p_oCutEventsList->getLength())
    return;

  //Now get the list of all cut events
  //p_oNode = p_oCutEventsList->item(0);
  //p_oElement = (DOMElement *) p_oNode;
  p_oElement = GetParentParametersElement(p_oDoc);
  sVal = XMLString::transcode("ds_deathEvent");
  p_oCutEventsList = p_oElement->getElementsByTagName(sVal);
  XMLString::release(&sVal);

  //If there's no mortality episodes, don't do anything else
  iNumDisturbanceEvents = p_oCutEventsList->getLength();
  if (0 == iNumDisturbanceEvents)
    return;

  p_fSeedlingValues = new doubleVal[iNumSpecies];
  for (i = 0; i < iNumSpecies; i++)
    p_fSeedlingValues[i].code = i;

  //*******************************************
  //Go through each mortality episode and add it as packages to the grids
  //*******************************************
  for (i = 0; i < iNumDisturbanceEvents; i++) {
    p_oNode = p_oCutEventsList->item(i);
    p_oElement = (DOMElement *) p_oNode;

    //Timestep first
    FillSingleValue(p_oElement, "ds_timestep", &iTimestep, true);

    //Validate it - needs to be between 0 and number of timesteps
    if (iTimestep <= 0 || iTimestep > iNumberTimesteps) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisturbance::ReadMortEpParameterFileData";
      std::stringstream s;
      s << "Invalid mortality episode cut timestep " << iTimestep;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Add a package to the master cut events grid - they're in timestep order,
    //so keep it that way
    p_oOldPackage = NULL;
    p_oNewPackage = NULL;
    p_oOldPackage = mp_oMasterCutsGrid->GetFirstPackageOfCell(0, 0);
    while (p_oOldPackage) {
      //Get this package's timestep
      p_oOldPackage->GetValue(m_iMasterTimestepCode, &iTemp);

      if (iTemp >= iTimestep) //here's where we'll drop it
        break;

      p_oNewPackage = p_oOldPackage; //use temporarily as a holder
      p_oOldPackage = p_oOldPackage->GetNextPackage();
    }

    //Create the new package
    if (!p_oNewPackage) //either there's no packages in grid, or new is first
      p_oNewPackage = mp_oMasterCutsGrid->CreatePackageOfCell(0, 0);
    else
      //make the new one after the one in p_oNewPackage
      p_oNewPackage = mp_oMasterCutsGrid->CreatePackage(p_oNewPackage);

    //Load up the new package with the timestep value
    p_oNewPackage->SetValue(m_iMasterTimestepCode, iTimestep);

    //Make tallest-first always true - I should come back and change
    //this at some point
    p_oNewPackage->SetValue(m_iTallestFirstCode, true);

    //Get the max snag decay class to consider - not required; set to -1
    //by default for backwards compatibility
    iTemp = -1;
    FillSingleValue(p_oElement, "ds_maxSnagDecayClass", &iTemp, false);
    p_oNewPackage->SetValue(m_iMaxSnagClassCode, iTemp);

    //Get species values and assign them
    sVal = XMLString::transcode("ds_applyToSpecies");
    p_oCutSpecificsList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumChildren = p_oCutSpecificsList->getLength();
    for (j = 0; j < iNumChildren; j++) {
      p_oNode = p_oCutSpecificsList->item(j);
      p_oChildElement = (DOMElement *) p_oNode;
      iTemp = GetNodeSpeciesCode(p_oChildElement, mp_oPop);
      if (-1 == iTemp) { //throw an error
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDisturbance::ReadMortEpParameterFileData";
        stcErr.sMoreInfo = "Unrecognized species in mortality episode.";
        throw(stcErr);
      }
      p_oNewPackage->SetValue(mp_iSpeciesCodes[iTemp], true);
    }

    //Get the amount type
    FillSingleValue(p_oElement, "ds_cutAmountType", &sTemp, true);
    //See if it's a value we recognize
    if (sTemp.compare("percent of basal area") == 0)
      p_oNewPackage->SetValue(m_iAmountTypeCode, percentBA);
    else if (sTemp.compare("percent of density") == 0)
      p_oNewPackage->SetValue(m_iAmountTypeCode, percentDen);
    else if (sTemp.compare("absolute basal area") == 0)
      p_oNewPackage->SetValue(m_iAmountTypeCode, absBA);
    else if (sTemp.compare("absolute density") == 0)
      p_oNewPackage->SetValue(m_iAmountTypeCode, absDen);
    else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisturbance::ReadMortEpParameterFileData";
      stcErr.sMoreInfo = "Unrecognized value in ds_cutAmountType.";
      throw(stcErr);
    }

    //Get the cut ranges
    sVal = XMLString::transcode("ds_dbhRange");
    p_oCutSpecificsList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);

    iNumChildren = p_oCutSpecificsList->getLength();
    //If there are more ranges than allowed, throw an error
    if (iNumChildren > m_iNumAllowedCutRanges) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisturbance::ReadMortEpParameterFileData";
      stcErr.sMoreInfo = "Too many dbh cut ranges.";
      throw(stcErr);
    }
    //Or, if there aren't any and this is a partial cut, throw an error
    if (0 == iNumChildren) {
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sFunction = "clDisturbance::ReadMortEpParameterFileData";
      stcErr.sMoreInfo = "ds_dbhRange";
      throw(stcErr);
    }

    //Get the minimum dbh in each cut range
    p_fRanges = new double[iNumChildren];
    for (j = 0; j < iNumChildren; j++) {
      p_oNode = p_oCutSpecificsList->item(j);
      p_oChildElement = (DOMElement *) p_oNode;
      FillSingleValue(p_oChildElement, "ds_low", &fTemp, true);
      p_fRanges[j] = fTemp;

      //If the value is negative, throw an error
      if (fTemp < 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDisturbance::ReadMortEpParameterFileData";
        stcErr.sMoreInfo = "Mortality episode cut range value cannot be negative.";
        throw(stcErr);
      }
    }

    //Sort the values and place them in the package
    fMaxVal = 0;
    for (j = 0; j < iNumChildren; j++)
      if (fMaxVal < p_fRanges[j])
        fMaxVal = p_fRanges[j];

    fMaxVal += 1;

    for (j = 0; j < iNumChildren; j++) {
      fTemp = fMaxVal;
      for (k = 0; k < iNumChildren; k++)
        if (p_fRanges[k] < fTemp) {
          iMinIndex = k;
          fTemp = p_fRanges[k];
        }

      p_oNode = p_oCutSpecificsList->item(iMinIndex);
      p_oChildElement = (DOMElement *) p_oNode;

      p_oNewPackage->SetValue(mp_iRangeMinCodes[j], (float)p_fRanges[iMinIndex]);
      FillSingleValue(p_oChildElement, "ds_high", &fTemp, true);
      p_oNewPackage->SetValue(mp_iRangeMaxCodes[j], (float)fTemp);
      FillSingleValue(p_oChildElement, "ds_amountToCut", &fTemp, true);
      p_oNewPackage->SetValue(mp_iRangeAmountCodes[j], (float)fTemp);

      p_fRanges[iMinIndex] = fMaxVal;
    }

    //Get the seedling proportion to kill - not required
    for (j = 0; j < iNumSpecies; j++)
      p_fSeedlingValues[j].val = 0;
    FillSpeciesSpecificValue(p_oElement, "ds_percentSeedlingsDie", "ds_psdVal",
        p_fSeedlingValues, iNumSpecies, mp_oPop, false);
    for (j = 0; j < iNumSpecies; j++) {
      p_oNewPackage->SetValue(mp_iSeedlingCodes[j],
          (float) (p_fSeedlingValues[j].val / 100.0));
    }

    //Create an ID number - just use the loop counter
    p_oNewPackage->SetValue(m_iMasterIDCode, (int) i);

    //*******************************************
    //Now:  Get the list of cells to which to apply this mortality episode
    //and make a package for each
    //*******************************************
    sVal = XMLString::transcode("ds_applyToCell");
    p_oCutSpecificsList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumChildren = p_oCutSpecificsList->getLength();
    for (j = 0; j < iNumChildren; j++) {
      p_oNode = p_oCutSpecificsList->item(j);
      p_oChildElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("x");
      sTemp = XMLString::transcode(p_oChildElement->getAttributeNode(sVal)->getNodeValue());
      iX = atoi(sTemp.c_str());
      XMLString::release(&sVal);
      sVal = XMLString::transcode("y");
      sTemp = XMLString::transcode(p_oChildElement->getAttributeNode(sVal)->getNodeValue());
      iY = atoi(sTemp.c_str());
      XMLString::release(&sVal);

      p_oOldPackage = NULL;
      p_oNewPackage = NULL;
      p_oOldPackage = mp_oCutEventsGrid->GetFirstPackageOfCell(iX, iY);
      while (p_oOldPackage) {
        //Get this package's timestep
        p_oOldPackage->GetValue(m_iCutTimestepCode, &iTemp);

        if (iTemp >= iTimestep) //here's where we'll drop it
          break;

        p_oNewPackage = p_oOldPackage; //use temporarily as a holder
        p_oOldPackage = p_oOldPackage->GetNextPackage();
      }

      //Create the new package
      if (!p_oNewPackage) //either there's no packages in grid, or new is first
        p_oNewPackage = mp_oCutEventsGrid->CreatePackageOfCell(iX, iY);
      else
        //make the new one after the one in p_oNewPackage
        p_oNewPackage = mp_oCutEventsGrid->CreatePackage(p_oNewPackage);

      //Load up the new package with the timestep and id values
      p_oNewPackage->SetValue(m_iCutTimestepCode, iTimestep);
      p_oNewPackage->SetValue(m_iCutIDCode, (int) i);
    } //end of for (j = 0; j < iNumChildren; j++)

    delete[] p_fRanges;
  } //end of for (i = 0; i < iNumDisturbanceEvents; i++)

  delete[] p_fSeedlingValues;
}

//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::GetData(xercesc::DOMDocument * p_oDoc) {
  try {
    mp_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject(
        "treepopulation");

    m_fPopCellLen = mp_oPop->GetGridCellSize();

    //Set up the harvest info grid
    SetupGrids();

    //Read the appropriate parameter file data
    if (m_bIsHarvest) {
      ReadHarvestParameterFileData(p_oDoc);
    } else {
      ReadMortEpParameterFileData(p_oDoc);
    }

    //Validate the package data
    ValidatePackages();

  } //end of try block
  catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisturbance::GetData";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// ValidatePackages()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::ValidatePackages() {
  try {
    clPackage * p_oPackage, //a package to validate
    *p_oNextPackage; //for comparing other packages to this one
    float fLowDbh, //low dbh in a cut range
    fHiDbh, //high dbh in a cut range
    fPrevHiDbh, //high dbh in the previous cut range
    fAmount; //cut amount in a cut range
    int iAmountType, //amount type value
    iCutType, //cut type value
    iNumSpecies = mp_oPop->GetNumberOfSpecies(), iID, //package ID value
    iNextID; //for comparing other packages' ID values
    short int iNumXCells = mp_oCutEventsGrid->GetNumberXCells(), iNumYCells =
        mp_oCutEventsGrid->GetNumberYCells(), i, j, k;

    //****************************************************
    // Validate packages in master cuts grid
    //****************************************************
    p_oPackage = mp_oMasterCutsGrid->GetFirstPackageOfCell(0, 0);
    while (p_oPackage) {
      fPrevHiDbh = -1;
      if (m_bIsHarvest) {
        p_oPackage->GetValue(m_iCutTypeCode, &iCutType);
      } else {
        iCutType = partial;
      }

      //Check each cut range to make sure that the high dbh value is bigger
      //than the low value
      for (k = 0; k < m_iNumAllowedCutRanges; k++) {
        p_oPackage->GetValue(mp_iRangeMinCodes[k], &fLowDbh);
        p_oPackage->GetValue(mp_iRangeMaxCodes[k], &fHiDbh);
        p_oPackage->GetValue(mp_iRangeAmountCodes[k], &fAmount);
        p_oPackage->GetValue(m_iAmountTypeCode, &iAmountType);

        //The range may not have been filled - check
        if (fLowDbh != 0 && fHiDbh != 0 && fAmount != 0) {

          //Only partial cuts can have more than one cut range defined
          if (partial != iCutType && k > 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clDisturbance::ValidatePackages";
            stcErr.sMoreInfo = "Cut ranges may not be defined for harvest gap or clear cuts.";
            throw(stcErr);
          }

          //Make sure the range high dbh value is greater than the low dbh
          if (fHiDbh <= fLowDbh) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clDisturbance::ValidatePackages";
            stcErr.sMoreInfo = "The low dbh range value in a harvest cut must be smaller than the high.";
            throw(stcErr);
          }

          //Make sure no value is negative
          if (fLowDbh < 0 || fHiDbh < 0 || fAmount < 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clDisturbance::ValidatePackages";
            stcErr.sMoreInfo = "No value in a harvest cut range may be negative.";
            throw(stcErr);
          }

          //Make sure that the ranges don't overlap
          if (fPrevHiDbh > fLowDbh) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clDisturbance::ValidatePackages";
            stcErr.sMoreInfo = "Harvest cut ranges may not overlap.";
            throw(stcErr);
          }

          //Make sure that the amount value, if a percentage, is not greater
          //than 100
          if ((percentBA == iAmountType || percentDen == iAmountType)
              && fAmount > 100) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clDisturbance::ValidatePackages";
            stcErr.sMoreInfo = "Harvest cut percentage amounts must be less than 100.";
            throw(stcErr);
          }

          fPrevHiDbh = fHiDbh;
        } //end of if (fLowDbh != 0 && fHiDbh != 0 && fAmount != 0)
      } //end of for (k = 0; k < m_iNumAllowedCutRanges; k++)

      //If this was a gap or clear cut, set up the first cut range for the
      //sake of CutTrees(), if it wasn't passed in.  Set it up to remove all trees
      if (clear == iCutType || gap == iCutType) {

        p_oPackage->GetValue(mp_iRangeAmountCodes[0], &fAmount);
        if (fAmount == 0)
          fAmount = 100;
        fHiDbh = 3000;
        iAmountType = percentDen;
        p_oPackage->SetValue(mp_iRangeMaxCodes[0], fHiDbh);
        p_oPackage->SetValue(mp_iRangeAmountCodes[0], fAmount);
        p_oPackage->SetValue(m_iAmountTypeCode, iAmountType);
      }

      //Make sure all seedling death rates are between zero and one
      for (i = 0; i < iNumSpecies; i++) {
        p_oPackage->GetValue(mp_iSeedlingCodes[i], &fAmount);
        if (fAmount < 0 || fAmount > 1) {
          //throw an error
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clDisturbance::ReadHarvestParameterFileData";
          stcErr.sMoreInfo = "Seedling death rate must be between 0 and 100.";
          throw(stcErr);
        }
      }

      p_oPackage = p_oPackage->GetNextPackage();
    } //end of while (p_oPackage)

    //****************************************************
    // Validate packages in cut events grid
    //****************************************************
    for (i = 0; i < iNumXCells; i++)
      for (j = 0; j < iNumYCells; j++) {
        p_oPackage = mp_oCutEventsGrid->GetFirstPackageOfCell(i, j);
        while (p_oPackage) {

          //Go through all the other packages in this grid cell and make sure that
          //there is no other with the same ID number
          p_oPackage->GetValue(m_iCutIDCode, &iID);

          p_oNextPackage = p_oPackage->GetNextPackage();
          while (p_oNextPackage) {

            p_oNextPackage->GetValue(m_iCutIDCode, &iNextID);
            if (iID == iNextID) {
              //throw an error
              modelErr stcErr;
              stcErr.iErrorCode = BAD_DATA;
              stcErr.sFunction = "clDisturbance::ValidatePackages";
              stcErr.sMoreInfo = "Disturbance event may not be applied twice to the same grid cell.";
              throw(stcErr);
            }
            p_oNextPackage = p_oNextPackage->GetNextPackage();
          } //end of while (p_oNextPackage)
          p_oPackage = p_oPackage->GetNextPackage();
        } //end of while (p_oPackage)
      } //end of for (j = 0; j < iNumYCells; j++)
  } //end of try block
  catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisturbance::ValidatePackages";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// ResetResultsGrid()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::ResetResultsGrid() {
  try {
    float fValue = 0;
    int iValue;
    short int iNumXCells = mp_oResultsGrid->GetNumberXCells(), iNumYCells =
        mp_oResultsGrid->GetNumberYCells(), iNumSpecies =
            mp_oPop->GetNumberOfSpecies(), i, j, iSp, iRange; //loop counters

    //For the harvest results grid - reset all values
    for (i = 0; i < iNumXCells; i++)
      for (j = 0; j < iNumYCells; j++) {

        iValue = -1;

        if (m_bIsHarvest) {
          //Harvest Type - set values to -1 (no harvest event has occurred)
          mp_oResultsGrid->SetValueOfCell(i, j, m_iHarvestTypeCode, iValue);
        }

        iValue = 0;
        //Cut Density and Cut Basal Area - set values to 0
        for (iSp = 0; iSp < iNumSpecies; iSp++) {
          mp_oResultsGrid->SetValueOfCell(i, j, mp_iSeedlingCutCodes[iSp],
              iValue);
          for (iRange = 0; iRange < m_iNumAllowedCutRanges; iRange++) {
            mp_oResultsGrid->SetValueOfCell(i, j, mp_iDenCutCodes[iRange][iSp],
                iValue);
            mp_oResultsGrid->SetValueOfCell(i, j, mp_iBaCutCodes[iRange][iSp],
                fValue);
          }
        }
      }

  } //end of try block
  catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisturbance::ResetResultsGrid";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetupGrids()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::SetupGrids() {
  try {
    std::stringstream sLabel;
    short int iNumSpecies = mp_oPop->GetNumberOfSpecies(), i, j; //loop counters
    //Declare the arrays for holding data member codes
    mp_iRangeMinCodes = new short int[m_iNumAllowedCutRanges];
    mp_iRangeMaxCodes = new short int[m_iNumAllowedCutRanges];
    mp_iRangeAmountCodes = new short int[m_iNumAllowedCutRanges];
    mp_iDenCutCodes = new short int *[m_iNumAllowedCutRanges];
    mp_iBaCutCodes = new short int *[m_iNumAllowedCutRanges];
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      mp_iDenCutCodes[i] = new short int[iNumSpecies];
      mp_iBaCutCodes[i] = new short int[iNumSpecies];
    }
    mp_iSpeciesCodes = new short int[iNumSpecies];
    mp_iSeedlingCodes = new short int[iNumSpecies];
    mp_iSeedlingCutCodes = new short int[iNumSpecies];
    mp_iPriorityMaxCodes = new short int[m_iNumAllowedPriorities];
    mp_iPriorityMinCodes = new short int[m_iNumAllowedPriorities];
    mp_iPriorityNameCodes = new short int[m_iNumAllowedPriorities];
    mp_iPriorityTypeCodes = new short int[m_iNumAllowedPriorities];
    for (i = 0; i < m_iNumAllowedPriorities; i++) {
      mp_iPriorityMaxCodes[i] = -1;
      mp_iPriorityMinCodes[i] = -1;
      mp_iPriorityNameCodes[i] = -1;
      mp_iPriorityTypeCodes[i] = -1;
    }

    if (m_bIsHarvest) {
      //*************************************
      //harvestmastercuts grid
      //*************************************
      mp_oMasterCutsGrid = mp_oSimManager->GetGridObject("harvestmastercuts");
      if (NULL == mp_oMasterCutsGrid) {
        m_fDistXCellLen = mp_oPop->GetGridCellSize();
        m_fDistYCellLen = m_fDistXCellLen;
        mp_oMasterCutsGrid = mp_oSimManager->CreateGrid("harvestmastercuts", //grid name
            0, //number of int data members
            0, //number of float data members
            0, //number of char data members
            0, //number of bool data members
            m_fDistXCellLen, //X cell length
            m_fDistYCellLen); //Y cell length

      } else {
        //Grid was already created
        m_fDistXCellLen = mp_oMasterCutsGrid->GetLengthXCells();
        m_fDistYCellLen = mp_oMasterCutsGrid->GetLengthYCells();
      }

      //Now adjust for the package data members
      mp_oMasterCutsGrid->ChangePackageDataStructure(
          5 + m_iNumAllowedPriorities, //int data members
          (3 * m_iNumAllowedCutRanges) + iNumSpecies + (2 * m_iNumAllowedPriorities), //float data members
          m_iNumAllowedPriorities, //char data members
          iNumSpecies+1); //bool data members

      //Register the data members
      m_iMasterTimestepCode
      = mp_oMasterCutsGrid->RegisterPackageInt("timestep");
      m_iMasterIDCode = mp_oMasterCutsGrid->RegisterPackageInt("id");
      m_iCutTypeCode = mp_oMasterCutsGrid->RegisterPackageInt("cuttype");
      m_iAmountTypeCode = mp_oMasterCutsGrid->RegisterPackageInt("amttype");
      m_iTallestFirstCode = mp_oMasterCutsGrid->RegisterPackageBool("tallestfirst");
      m_iMaxSnagClassCode = mp_oMasterCutsGrid->RegisterPackageInt("maxsnagclass");

      for (i = 0; i < m_iNumAllowedCutRanges; i++) {
        sLabel << "rangemin" << i;
        mp_iRangeMinCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
        sLabel << "rangemax" << i;
        mp_iRangeMaxCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
        sLabel << "rangeamt" << i;
        mp_iRangeAmountCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
      }

      for (i = 0; i < iNumSpecies; i++) {
        sLabel << "species" << i;
        mp_iSpeciesCodes[i] = mp_oMasterCutsGrid->RegisterPackageBool(sLabel.str());
        sLabel.str("");
        sLabel << "seedling" << i;
        mp_iSeedlingCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
      }

      for (i = 0; i < m_iNumAllowedPriorities; i++) {
        sLabel << "priorityname" << i;
        mp_iPriorityNameCodes[i] = mp_oMasterCutsGrid->RegisterPackageString(sLabel.str());
        sLabel.str("");
        sLabel << "prioritytype" << i;
        mp_iPriorityTypeCodes[i] = mp_oMasterCutsGrid->RegisterPackageInt(sLabel.str());
        sLabel.str("");
        sLabel << "prioritymin" << i;
        mp_iPriorityMinCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
        sLabel << "prioritymax" << i;
        mp_iPriorityMaxCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
      }

      //*************************************
      //harvestcutevents grid
      //*************************************
      mp_oCutEventsGrid = mp_oSimManager->CreateGrid("harvestcutevents", //grid name
          0, //number of int data members
          0, //number of float data members
          0, //number of char data members
          0, //number of bool data members
          m_fDistXCellLen, //X cell length
          m_fDistYCellLen); //Y cell length

      //Now adjust for the package data members
      mp_oCutEventsGrid->ChangePackageDataStructure(2, //number of package int data members
          0, //number of package float data members
          0, //number of package char data members
          0); //number of package bool data members

      //Register the data members
      m_iCutTimestepCode = mp_oCutEventsGrid->RegisterPackageInt("timestep");
      m_iCutIDCode = mp_oCutEventsGrid->RegisterPackageInt("id");

      //*************************************
      //Harvest results grid
      //*************************************
      mp_oResultsGrid = mp_oSimManager->CreateGrid("Harvest Results", //grid name
          1 + iNumSpecies + (m_iNumAllowedCutRanges * iNumSpecies), //number of int data members
          (m_iNumAllowedCutRanges * iNumSpecies), //num float data members
          0, //number of char data members
          0, //number of bool data members
          m_fDistXCellLen, //X cell length
          m_fDistYCellLen); //Y cell length

      //Register the data members
      m_iHarvestTypeCode = mp_oResultsGrid->RegisterInt("Harvest Type");

      for (i = 0; i < m_iNumAllowedCutRanges; i++)
        for (j = 0; j < iNumSpecies; j++) {
          sLabel << "Cut Density_" << i << "_" << j;
          mp_iDenCutCodes[i][j] = mp_oResultsGrid->RegisterInt(sLabel.str());
          sLabel.str("");
          sLabel << "Cut Basal Area_" << i << "_" << j;
          mp_iBaCutCodes[i][j] = mp_oResultsGrid->RegisterFloat(sLabel.str());
          sLabel.str("");
        }

      for (i = 0; i < iNumSpecies; i++) {
        sLabel << "Cut Seedlings_" << i;
        mp_iSeedlingCutCodes[i] = mp_oResultsGrid->RegisterInt(sLabel.str());
        sLabel.str("");
      }
    } else {

      //*************************************
      //mortepisodemastercuts grid
      //*************************************
      mp_oMasterCutsGrid = mp_oSimManager->GetGridObject(
          "mortepisodemastercuts");
      if (NULL == mp_oMasterCutsGrid) {
        m_fDistXCellLen = mp_oPop->GetGridCellSize();
        m_fDistYCellLen = m_fDistXCellLen;
        mp_oMasterCutsGrid = mp_oSimManager->CreateGrid(
            "mortepisodemastercuts", //grid name
            0, //number of int data members
            0, //number of float data members
            0, //number of char data members
            0, //number of bool data members
            m_fDistXCellLen, //X cell length
            m_fDistYCellLen); //Y cell length
      } else {
        //Grid was already created
        m_fDistXCellLen = mp_oMasterCutsGrid->GetLengthXCells();
        m_fDistYCellLen = mp_oMasterCutsGrid->GetLengthYCells();
      }

      //Now adjust for the package data members
      mp_oMasterCutsGrid->ChangePackageDataStructure(4, //number of package int data members
          (3 * m_iNumAllowedCutRanges) + iNumSpecies, //number of package float data members
          0, //number of package char data members
          iNumSpecies+1); //number of package bool data members

      //Register the data members
      m_iMasterTimestepCode
      = mp_oMasterCutsGrid->RegisterPackageInt("timestep");
      m_iMasterIDCode = mp_oMasterCutsGrid->RegisterPackageInt("id");
      m_iAmountTypeCode = mp_oMasterCutsGrid->RegisterPackageInt("amttype");
      m_iTallestFirstCode = mp_oMasterCutsGrid->RegisterPackageBool("tallestfirst");
      m_iMaxSnagClassCode = mp_oMasterCutsGrid->RegisterPackageInt("maxsnagclass");

      for (i = 0; i < m_iNumAllowedCutRanges; i++) {
        sLabel << "rangemin" << i;
        mp_iRangeMinCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
        sLabel << "rangemax" << i;
        mp_iRangeMaxCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
        sLabel << "rangeamt" << i;
        mp_iRangeAmountCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
      }

      for (i = 0; i < iNumSpecies; i++) {
        sLabel << "species" << i;
        mp_iSpeciesCodes[i] = mp_oMasterCutsGrid->RegisterPackageBool(sLabel.str());
        sLabel.str("");
        sLabel << "seedling" << i;
        mp_iSeedlingCodes[i] = mp_oMasterCutsGrid->RegisterPackageFloat(sLabel.str());
        sLabel.str("");
      }

      //*************************************
      //mortepisodecutevents grid
      //*************************************
      mp_oCutEventsGrid = mp_oSimManager->CreateGrid("mortepisodecutevents", //grid name
          0, //number of int data members
          0, //number of float data members
          0, //number of char data members
          0, //number of bool data members
          m_fDistXCellLen, //X cell length
          m_fDistYCellLen); //Y cell length

      //Now adjust for the package data members
      mp_oCutEventsGrid->ChangePackageDataStructure(2, //number of package int data members
          0, //number of package float data members
          0, //number of package char data members
          0); //number of package bool data members

      //Register the data members
      m_iCutTimestepCode = mp_oCutEventsGrid->RegisterPackageInt("timestep");
      m_iCutIDCode = mp_oCutEventsGrid->RegisterPackageInt("id");

      //*************************************
      //Mortality Episode Results grid
      //*************************************
      mp_oResultsGrid = mp_oSimManager->CreateGrid("Mortality Episode Results", //grid name
          iNumSpecies + (m_iNumAllowedCutRanges * iNumSpecies), //number of int data members
          (m_iNumAllowedCutRanges * iNumSpecies), //num float data members
          0, //number of char data members
          0, //number of bool data members
          m_fDistXCellLen, //X cell length
          m_fDistYCellLen); //Y cell length

      //Register the data members
      for (i = 0; i < m_iNumAllowedCutRanges; i++)
        for (j = 0; j < iNumSpecies; j++) {
          sLabel << "Cut Density_" << i << "_" << j;
          mp_iDenCutCodes[i][j] = mp_oResultsGrid->RegisterInt(sLabel.str());
          sLabel.str("");
          sLabel << "Cut Basal Area_" << i << "_" << j;
          mp_iBaCutCodes[i][j] = mp_oResultsGrid->RegisterFloat(sLabel.str());
          sLabel.str("");
        }

      for (i = 0; i < iNumSpecies; i++) {
        sLabel << "Cut Seedlings_" << i;
        mp_iSeedlingCutCodes[i] = mp_oResultsGrid->RegisterInt(sLabel.str());
        sLabel.str("");
      }
    }

    //Use the ResetResultsGrid function to initialize the values
    ResetResultsGrid();

  } //end of try block
  catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisturbance::SetupGrids";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Action()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::Action() {
  try {
    clPackage * p_oMasterPackage, *p_oPackageHolder;
    int iTimestep = mp_oSimManager->GetCurrentTimestep(), iPackageTimestep;

    ResetResultsGrid();

    //Get all the master cut packages for this timestep and execute them
    p_oMasterPackage = mp_oMasterCutsGrid->GetFirstPackageOfCell(0, 0);

    while (p_oMasterPackage) {
      //Get the next package - if this is a cut package, it will be deleted during
      //the course of the cut process
      p_oPackageHolder = p_oMasterPackage->GetNextPackage();

      //Get this package's timestep
      p_oMasterPackage->GetValue(m_iMasterTimestepCode, &iPackageTimestep);
      if (iTimestep == iPackageTimestep)
        CutTrees(p_oMasterPackage);

      //I commented this out as a quick-and-dirty way to make this work if we
      //start on a timestep other than 0 and there are defined events that
      //were supposed to happen before the start timestep.
      //      else
      //        break; //we've done them all this timestep

      p_oMasterPackage = p_oPackageHolder;
    }
  } //end of try block
  catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisturbance::Action";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// CutTrees()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::CutTrees(clPackage * p_oMasterPackage) {
  float *p_fLoDbh = NULL, //low dbh of cut ranges - sized m_iNumAllowedCutRanges
      *p_fHiDbh = NULL, //high dbh of cut ranges - sized m_iNumAllowedCutRanges
      *p_fAmountToRemove = NULL, //amount to cut - sized m_iNumAllowedCutRanges
      *p_fAmountRemoved = NULL, //amount cut - either in basal area or density -
      //sized number of cut ranges
      *p_fBABackup = NULL, //this is to preserve the amt basal area to remove
      *p_fTotalBasalArea = NULL, //amount of total basal area - sized number
      //of cut ranges
      *p_fKillProb = NULL; //seedling kill probs - sized number of species
  bool * p_bSpeciesCut = NULL; //array of size # species of whether each species is in
  //this cut
  try {
    stcGridList * p_cutArea = NULL, //pointer to list of applicable grid cells
        *p_cellRecord = NULL; //for working with cell records
    stcTreeGridList * p_treeArea = NULL, *p_treeRecord = NULL;
    std::string sPriorityName;
    float fPriorityMin, fPriorityMax,
    fTemp; //for doing grid values
    int iNumXCells = mp_oCutEventsGrid->GetNumberXCells(),
        iNumYCells = mp_oCutEventsGrid->GetNumberYCells(),
        iNumSpecies = mp_oPop->GetNumberOfSpecies(),
        iAmountType, //in what terms the cut is expressed
        iCutType, //type of cut to be performed
        iPriorityType,
        iMaxSnagDecayClass, //max allowed snag decay class - -1 if no snags
        iNumCellsInCutArea; //number of cells in cut area
    short int iSp, i, j;
    bool bTallestFirst, bKeepCuttingAfterPriority = true;


#ifdef test
    std::fstream harv( "HarvestTrees.txt", std::ios::out | std::ios::app );
    harv << "Timestep: " << mp_oSimManager->GetCurrentTimestep() << "\n" << "X\tY\tSpecies\tDbh\n";
    harv.close();
#endif

    //Declare our arrays and set all the values to 0
    p_fLoDbh = new float[m_iNumAllowedCutRanges];
    p_fHiDbh = new float[m_iNumAllowedCutRanges];
    p_fAmountToRemove = new float[m_iNumAllowedCutRanges];
    p_fTotalBasalArea = new float[m_iNumAllowedCutRanges];
    p_fAmountRemoved = new float[m_iNumAllowedCutRanges];
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      p_fLoDbh[i] = 0;
      p_fHiDbh[i] = 0;
      p_fAmountToRemove[i] = 0;
      p_fAmountRemoved[i] = 0;
    }

    p_bSpeciesCut = new bool[iNumSpecies];
    p_fKillProb = new float[iNumSpecies];
    for (i = 0; i < iNumSpecies; i++) {
      p_bSpeciesCut[i] = false;
      p_fKillProb[i] = 0;
    }

    //Get the cut type and cut amount values for this package
    if (m_bIsHarvest) {
      p_oMasterPackage->GetValue(m_iCutTypeCode, &iCutType);
    } else {
      iCutType = partial;
    }
    p_oMasterPackage->GetValue(m_iAmountTypeCode, &iAmountType);

    //Get the "tallest first" flag
    p_oMasterPackage->GetValue(m_iTallestFirstCode, &bTallestFirst);

    //Get the max allowed snag decay class
    p_oMasterPackage->GetValue(m_iMaxSnagClassCode, &iMaxSnagDecayClass);

    //If the cut type is percent of basal area - declare the total basal area
    //arrays
    if (partial == iCutType && percentBA == iAmountType) {
      p_fBABackup = new float[m_iNumAllowedCutRanges];
    }

    //Get the seedling kill probabilities
    for (i = 0; i < iNumSpecies; i++) {
      p_oMasterPackage->GetValue(mp_iSeedlingCodes[i], &fTemp);
      p_fKillProb[i] = fTemp;
    }

    //Get the cut area data - the cut information extracted from the master
    //package and the linked list of grid cells to which to apply the cut
    iNumCellsInCutArea = AssembleCutArea(p_oMasterPackage, iNumXCells,
        iNumYCells, p_cutArea, p_treeArea, p_fLoDbh, p_fHiDbh, p_fAmountToRemove,
        p_bSpeciesCut);
    if (iNumCellsInCutArea == 0) {
      delete[] p_fLoDbh;
      delete[] p_fHiDbh;
      delete[] p_fAmountToRemove;
      delete[] p_fAmountRemoved;
      delete[] p_fTotalBasalArea;
      delete[] p_bSpeciesCut;
      delete[] p_fBABackup;
      delete[] p_fKillProb;
      while (p_cutArea) {
        p_cellRecord = p_cutArea->next;
        delete p_cutArea;
        p_cutArea = p_cellRecord;
      }
      while (p_treeArea) {
        p_treeRecord = p_treeArea->next;
        delete p_treeArea;
        p_treeArea = p_treeRecord;
      }
      //Delete the master package - we're done with it
      mp_oMasterCutsGrid->DeletePackage(p_oMasterPackage);
      return;
    }

    //Set the cut event that's occurring for each cell in the cut area in the
    //harvest results grid
    if (m_bIsHarvest)
      SetCutFlags(p_cutArea, iCutType);

    //Convert the amount to cut - if it's in percentage, convert it to a
    //proportion; if it's an absolute value in amounts per hectare, scale it to
    //a total amount
    if (percentDen == iAmountType || percentBA == iAmountType) //percentage value
      for (i = 0; i < m_iNumAllowedCutRanges; i++)
        p_fAmountToRemove[i] *= 0.01;
    else if (absDen == iAmountType || absBA == iAmountType) {
      //Figure out the area of the cut in hectares and multiply
      fTemp = iNumCellsInCutArea * mp_oCutEventsGrid->GetLengthXCells()
                      * mp_oCutEventsGrid->GetLengthYCells() / 10000;
      for (i = 0; i < m_iNumAllowedCutRanges; i++)
        p_fAmountToRemove[i] *= fTemp;
    }
    if (absDen == iAmountType) //Round to nearest whole number
      for (i = 0; i < m_iNumAllowedCutRanges; i++)
        p_fAmountToRemove[i] = clModelMath::Round(p_fAmountToRemove[i], 0);

    //Cut each species in turn for this package
    for (iSp = 0; iSp < iNumSpecies; iSp++)
      if (p_bSpeciesCut[iSp]) {

        if (percentBA == iAmountType) {
          //If the cut type is percentage of basal area, we have to know how
          //much basal area is in the cut ranges.  So get it.
          for (i = 0; i < m_iNumAllowedCutRanges; i++)
            p_fTotalBasalArea[i] = 0;

          GetBasalArea(p_cutArea, p_treeArea, iSp, p_fTotalBasalArea,
              p_fLoDbh, p_fHiDbh, iMaxSnagDecayClass);
        }

        //Reset the amount removed array
        for (i = 0; i < m_iNumAllowedCutRanges; i++)
          p_fAmountRemoved[i] = 0;

        //Check for priorities
        for (j = 0; j < m_iNumAllowedPriorities; j++) {
          if (mp_iPriorityNameCodes[j] > -1 && bKeepCuttingAfterPriority) {
            p_oMasterPackage->GetValue(mp_iPriorityNameCodes[j], &sPriorityName);
            if (sPriorityName.length() > 0) {
              p_oMasterPackage->GetValue(mp_iPriorityTypeCodes[j], &iPriorityType);
              p_oMasterPackage->GetValue(mp_iPriorityMinCodes[j], &fPriorityMin);
              p_oMasterPackage->GetValue(mp_iPriorityMaxCodes[j], &fPriorityMax);

              if (percentBA == iAmountType)
                bKeepCuttingAfterPriority = CutSpeciesPercentBAPriority(iSp, p_cutArea,
                    p_treeArea, p_fLoDbh, p_fHiDbh,
                    p_fAmountToRemove, p_fAmountRemoved, p_fTotalBasalArea,
                    sPriorityName, iPriorityType, fPriorityMin, fPriorityMax,
                    bTallestFirst, iMaxSnagDecayClass);
              else if (absDen == iAmountType)
                CutSpeciesAbsDenPriority(iSp, p_cutArea, p_treeArea, p_fLoDbh, p_fHiDbh,
                    p_fAmountToRemove, p_fAmountRemoved, sPriorityName,
                    iPriorityType, fPriorityMin, fPriorityMax, bTallestFirst,
                    iMaxSnagDecayClass);
              else if (absBA == iAmountType)
                bKeepCuttingAfterPriority = CutSpeciesAbsBAPriority(iSp, p_cutArea,
                    p_treeArea, p_fLoDbh, p_fHiDbh,
                    p_fAmountToRemove, p_fAmountRemoved, sPriorityName,
                    iPriorityType, fPriorityMin, fPriorityMax, bTallestFirst,
                    iMaxSnagDecayClass);
            }
          }
        }

        //Cut beyond priorities
        if (bKeepCuttingAfterPriority) {
          if (percentDen == iAmountType)
            CutSpeciesPercentDen(iSp, p_cutArea, p_treeArea, p_fLoDbh, p_fHiDbh,
                p_fAmountToRemove, p_fAmountRemoved, iMaxSnagDecayClass);
          else if (percentBA == iAmountType)
            CutSpeciesPercentBA(iSp, p_cutArea, p_treeArea, p_fLoDbh, p_fHiDbh,
                p_fAmountToRemove, p_fAmountRemoved, p_fTotalBasalArea,
                bTallestFirst, iMaxSnagDecayClass);
          else if (absDen == iAmountType)
            CutSpeciesAbsDen(iSp, p_cutArea, p_treeArea, p_fLoDbh, p_fHiDbh,
                p_fAmountToRemove, p_fAmountRemoved, bTallestFirst,
                iMaxSnagDecayClass);
          else // absBA == iAmountType)
            CutSpeciesAbsBA(iSp, p_cutArea, p_treeArea, p_fLoDbh, p_fHiDbh,
                p_fAmountToRemove, p_fAmountRemoved, bTallestFirst,
                iMaxSnagDecayClass);
        }

      } //end of if (p_bSpeciesCut[iSp])

    KillSeedlings(p_cutArea, p_treeArea, p_fKillProb);

    //Delete the master package - we're done with it
    mp_oMasterCutsGrid->DeletePackage(p_oMasterPackage);

    delete[] p_fLoDbh;
    delete[] p_fHiDbh;
    delete[] p_fAmountToRemove;
    delete[] p_fAmountRemoved;
    delete[] p_fTotalBasalArea;
    delete[] p_bSpeciesCut;
    delete[] p_fBABackup;
    delete[] p_fKillProb;

    //Delete the list of grid cell records in this cut area
    while (p_cutArea) {
      p_cellRecord = p_cutArea->next;
      delete p_cutArea;
      p_cutArea = p_cellRecord;
    }

    while (p_treeArea) {
      p_treeRecord = p_treeArea->next;
      delete p_treeArea;
      p_treeArea = p_treeRecord;
    }

  } //end of try block
  catch (modelErr& err) {
    delete[] p_fLoDbh;
    delete[] p_fHiDbh;
    delete[] p_fAmountToRemove;
    delete[] p_fAmountRemoved;
    delete[] p_fTotalBasalArea;
    delete[] p_bSpeciesCut;
    delete[] p_fBABackup;
    delete[] p_fKillProb;
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisturbance::CutTrees";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// CutSpeciesPercentBA()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::CutSpeciesPercentBA(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, float *p_fTotalBasalArea, bool bTallestFirst,
    const int &iMaxSnagDecayClass) {

  clTree * p_oTree, //tree we're working with
  *p_oNextTree; //placeholder for getting the next tree
  float *p_fAdjustedToRemove = new float[m_iNumAllowedCutRanges];
  float fDbh,
  fThisBA; //basal area of a single tree;
  int i;
  bool bTreeIsDead; //whether or not a particular tree dies

  //Multiply the percentage to cut by the total basal area and then treat
  //the cut like an amount of absolute basal area
  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    p_fAdjustedToRemove[i] = p_fAmountToRemove[i] * p_fTotalBasalArea[i];

  //See if there's any cutting to do
  bTreeIsDead = true; //grabbing a handy bool variable as a temp
  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    bTreeIsDead = bTreeIsDead && p_fAmountRemoved[i] >= p_fAdjustedToRemove[i];
  if (bTreeIsDead) return;

  //Now remove the trees
  p_oTree = GetFirstTreeInCutArea(p_cutArea, p_treeArea, iSp, bTallestFirst,
      iMaxSnagDecayClass);
  while (p_oTree) {
    bTreeIsDead = false;
    fThisBA = 0;

    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);

    //Go through each cut range and see if a tree is in it
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i]) {

        //Absolute amount of basal area (and percent of basal area converted
        //to absolute) - cut until we've reached the goal
        if (p_fAmountRemoved[i] < p_fAdjustedToRemove[i]) {
          fThisBA = clModelMath::CalculateBasalArea(fDbh);
          p_fAmountRemoved[i] += fThisBA;
          //If we're still below the limit, cut the tree
          if (p_fAmountRemoved[i] <= p_fAdjustedToRemove[i])
            bTreeIsDead = true;
          else {
            //This tree takes us over the limit. If this is a large-to-small
            //size cut, don't cut it. A smaller tree might bring us closer.
            //If this is a small-to-large cut, do the thing that brings us
            //closest to the goal.
            if (bTallestFirst) {
              p_fAmountRemoved[i] -= fThisBA;
            } else {
              if (p_fAmountRemoved[i] - p_fAdjustedToRemove[i] >
                  p_fAdjustedToRemove[i] - (p_fAmountRemoved[i] - fThisBA)) {
                //Not cutting the tree gets us closest to the goal
                p_fAmountRemoved[i] -= fThisBA;
              } else {
                //Cutting the tree gets us closest to the goal
                bTreeIsDead = true;
              }
            }
          }
        } //end of if (p_fAmountRemoved[i] < p_fAdjustedToRemove[i])
      } //end of (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
    } //end of for (i = 0; i < m_iNumAllowedCutRanges; i++)

    p_oNextTree = GetNextTreeInCutArea(p_cutArea, p_treeArea, iSp,
        bTallestFirst, iMaxSnagDecayClass);
    if (bTreeIsDead) {

      AddTreeStatsToResults(p_oTree, p_fLoDbh, p_fHiDbh, fDbh);
      mp_oPop->KillTree(p_oTree, m_iReasonCode);
    }

    p_oTree = p_oNextTree;
  } //end of while (p_oTree)

  delete[] p_fAdjustedToRemove;
}

//////////////////////////////////////////////////////////////////////////////
// CutSpeciesAbsBA()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::CutSpeciesAbsBA(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, bool bTallestFirst, const int &iMaxSnagDecayClass) {

  clTree * p_oTree, //tree we're working with
  *p_oNextTree; //placeholder for getting the next tree
  float fDbh,
  fThisBA; //basal area of a single tree;
  int i;
  bool bTreeIsDead; //whether or not a particular tree dies

  //See if there's any cutting to do
  bTreeIsDead = true; //grabbing a handy bool variable as a temp
  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    bTreeIsDead = bTreeIsDead && p_fAmountRemoved[i] >= p_fAmountToRemove[i];
  if (bTreeIsDead) return;

  //Now remove the trees
  p_oTree = GetFirstTreeInCutArea(p_cutArea, p_treeArea, iSp, bTallestFirst,
      iMaxSnagDecayClass);
  while (p_oTree) {
    bTreeIsDead = false;
    fThisBA = 0;

    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()),
        &fDbh);

    //Go through each cut range and see if a tree is in it
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i]) {

        //Absolute amount of basal area - cut until we've reached the goal
        if (p_fAmountRemoved[i] < p_fAmountToRemove[i]) {
          fThisBA = clModelMath::CalculateBasalArea(fDbh);
          p_fAmountRemoved[i] += fThisBA;

          if (p_fAmountRemoved[i] <= p_fAmountToRemove[i])
            bTreeIsDead = true;
          else {
            //This tree takes us over the limit. If this is a large-to-small
            //size cut, don't cut it. A smaller tree might bring us closer.
            //If this is a small-to-large cut, do the thing that brings us
            //closest to the goal.
            if (bTallestFirst) {
              p_fAmountRemoved[i] -= fThisBA;
            } else {
              if (p_fAmountRemoved[i] - p_fAmountToRemove[i] >
                  p_fAmountToRemove[i] - (p_fAmountRemoved[i] - fThisBA)) {
                //Not cutting the tree gets us closest to the goal
                p_fAmountRemoved[i] -= fThisBA;
              } else {
                //Cutting the tree gets us closest to the goal
                bTreeIsDead = true;
              }
            }
          }
        } //end of if (p_fAmountRemoved[i] < p_fAmountToRemove[i])
      } //end of (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
    } //end of for (i = 0; i < m_iNumAllowedCutRanges; i++)

    p_oNextTree = GetNextTreeInCutArea(p_cutArea, p_treeArea, iSp,
        bTallestFirst, iMaxSnagDecayClass);
    if (bTreeIsDead) {

      AddTreeStatsToResults(p_oTree, p_fLoDbh, p_fHiDbh, fDbh);
      mp_oPop->KillTree(p_oTree, m_iReasonCode);
    }

    p_oTree = p_oNextTree;
  } //end of while (p_oTree)
}

//////////////////////////////////////////////////////////////////////////////
// CutSpeciesPercentDen()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::CutSpeciesPercentDen(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, const int &iMaxSnagDecayClass) {

  clTree * p_oTree, //tree we're working with
  *p_oNextTree; //placeholder for getting the next tree
  float fDbh,
  fRand; //random number
  //fThisBA; //basal area of a single tree;
  int i;
  bool bTreeIsDead; //whether or not a particular tree dies

  //Now remove the trees
  p_oTree = GetFirstTreeInCutArea(p_cutArea, p_treeArea, iSp, true,
      iMaxSnagDecayClass);
  while (p_oTree) {
    bTreeIsDead = false;
    //fThisBA = 0;

    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()),
        &fDbh);


    //Go through each cut range and see if a tree is in it
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i]) {

        //Percent of density - compare a random number to the desired
        //percentage to remove
        fRand = clModelMath::GetRand();
        if (fRand <= p_fAmountToRemove[i])
          bTreeIsDead = true;
      } //end of (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
    } //end of for (i = 0; i < m_iNumAllowedCutRanges; i++)

    p_oNextTree = GetNextTreeInCutArea(p_cutArea, p_treeArea, iSp, true,
        iMaxSnagDecayClass);
    if (bTreeIsDead) {

      AddTreeStatsToResults(p_oTree, p_fLoDbh, p_fHiDbh, fDbh);
      mp_oPop->KillTree(p_oTree, m_iReasonCode);
    }

    p_oTree = p_oNextTree;
  } //end of while (p_oTree)
}

//////////////////////////////////////////////////////////////////////////////
// CutSpeciesAbsDen()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::CutSpeciesAbsDen(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, bool bTallestFirst, const int &iMaxSnagDecayClass) {
  clTree * p_oTree, //tree we're working with
  *p_oNextTree; //placeholder for getting the next tree
  float fDbh;
  //fThisBA; //basal area of a single tree;
  int i;
  bool bTreeIsDead; //whether or not a particular tree dies

  //See if there's any cutting to do
  bTreeIsDead = true; //grabbing a handy bool variable as a temp
  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    bTreeIsDead = bTreeIsDead && p_fAmountRemoved[i] >= p_fAmountToRemove[i];
  if (bTreeIsDead) return;

  //Now remove the trees
  p_oTree = GetFirstTreeInCutArea(p_cutArea, p_treeArea, iSp, bTallestFirst,
      iMaxSnagDecayClass);
  while (p_oTree) {
    bTreeIsDead = false;
    //fThisBA = 0;

    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);

    //Go through each cut range and see if a tree is in it
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i]) {

        if (p_fAmountRemoved[i] < p_fAmountToRemove[i]) {
          bTreeIsDead = true;
          p_fAmountRemoved[i] += 1.0;
        } else
          p_oTree = NULL; //break the loop

      } //end of (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
    } //end of for (i = 0; i < m_iNumAllowedCutRanges; i++)

    p_oNextTree = GetNextTreeInCutArea(p_cutArea, p_treeArea, iSp,
        bTallestFirst, iMaxSnagDecayClass);
    if (bTreeIsDead) {

      AddTreeStatsToResults(p_oTree, p_fLoDbh, p_fHiDbh, fDbh);
      mp_oPop->KillTree(p_oTree, m_iReasonCode);
    }

    p_oTree = p_oNextTree;
  } //end of while (p_oTree)
}

//////////////////////////////////////////////////////////////////////////////
// CutSpeciesPercentBAPriority()
//////////////////////////////////////////////////////////////////////////////
bool clDisturbance::CutSpeciesPercentBAPriority(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, float *p_fTotalBasalArea, std::string sPriorityName,
    int &iPriorityType, float &fPriorityMin, float &fPriorityMax,
    bool bTallestFirst, const int &iMaxSnagDecayClass) {

  clTree * p_oTree, //tree we're working with
  *p_oNextTree; //placeholder for getting the next tree
  float *p_fAdjustedToRemove = new float[m_iNumAllowedCutRanges];
  float fDbh,
  fTemp,
  fThisBA; //basal area of a single tree;
  int i, iTemp, iCode;
  bool bTreeIsDead, bTreeIsPriority, bTemp, bKeepCuttingAfterPriority = true;

  //Multiply the percentage to cut by the total basal area and then treat
  //the cut like an amount of absolute basal area
  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    p_fAdjustedToRemove[i] = p_fAmountToRemove[i] * p_fTotalBasalArea[i];

  //See if there's any cutting to do
  bTreeIsDead = true; //grabbing a handy bool variable as a temp
  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    bTreeIsDead = bTreeIsDead && p_fAmountRemoved[i] >= p_fAdjustedToRemove[i];
  if (bTreeIsDead) return false; //Don't need to look for more trees in the non-priority section

  //Now remove the trees
  p_oTree = GetFirstTreeInCutArea(p_cutArea, p_treeArea, iSp, bTallestFirst,
      iMaxSnagDecayClass);
  while (p_oTree) {
    bTreeIsDead = false;
    fThisBA = 0;

    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);

    //Go through each cut range and see if a tree is in it
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i]) {

        //See if it is a priority tree
        bTreeIsPriority = false;
        if (floatType == iPriorityType) {
          iCode = mp_oPop->GetFloatDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &fTemp);
            if (fTemp >= fPriorityMin && fTemp <= fPriorityMax)
              bTreeIsPriority = true;
          }
        } else if (intType == iPriorityType) {
          iCode = mp_oPop->GetIntDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &iTemp);
            if (iTemp >= fPriorityMin && iTemp <= fPriorityMax)
              bTreeIsPriority = true;
          }
        } else {
          iCode = mp_oPop->GetBoolDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &bTemp);
            if ((bTemp && fPriorityMin == 1) || (!bTemp && fPriorityMin == 0))
              bTreeIsPriority = true;
          }
        }

        //Absolute amount of basal area (and percent of basal area converted
        //to absolute) - cut until we've reached the goal
        if (bTreeIsPriority && p_fAmountRemoved[i] < p_fAdjustedToRemove[i]) {
          fThisBA = clModelMath::CalculateBasalArea(fDbh);
          p_fAmountRemoved[i] += fThisBA;
          if (p_fAmountRemoved[i] <= p_fAdjustedToRemove[i])
            bTreeIsDead = true;
          else {
            //This tree takes us over the limit. If this is a large-to-small
            //size cut, don't cut it. A smaller tree might bring us closer.
            //If this is a small-to-large cut, do the thing that brings us
            //closest to the goal.
            if (bTallestFirst) {
              p_fAmountRemoved[i] -= fThisBA;
            } else {
              if (p_fAmountRemoved[i] - p_fAdjustedToRemove[i] >
                  p_fAdjustedToRemove[i] - (p_fAmountRemoved[i] - fThisBA)) {
                //Not cutting the tree gets us closest to the goal
                p_fAmountRemoved[i] -= fThisBA;
              } else {
                //Cutting the tree gets us closest to the goal
                bTreeIsDead = true;
              }
            }
            //Since we reached the limit with this tree, whether or not we
            //decided to cut it, it makes no sense to cut other trees.
            //Trigger a stop to the harvest.
            bKeepCuttingAfterPriority = false;
          }
        } //end of if (p_fAmountRemoved[i] < p_fAdjustedToRemove[i])
      } //end of (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
    } //end of for (i = 0; i < m_iNumAllowedCutRanges; i++)

    p_oNextTree = GetNextTreeInCutArea(p_cutArea, p_treeArea, iSp,
        bTallestFirst, iMaxSnagDecayClass);
    if (bTreeIsDead) {

      AddTreeStatsToResults(p_oTree, p_fLoDbh, p_fHiDbh, fDbh);
      mp_oPop->KillTree(p_oTree, m_iReasonCode);
    }

    p_oTree = p_oNextTree;
  } //end of while (p_oTree)
  delete[] p_fAdjustedToRemove;
  return bKeepCuttingAfterPriority;
}

//////////////////////////////////////////////////////////////////////////////
// CutSpeciesAbsBAPriority()
//////////////////////////////////////////////////////////////////////////////
bool clDisturbance::CutSpeciesAbsBAPriority(int iSp, stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, std::string sPriorityName, int &iPriorityType,
    float &fPriorityMin, float &fPriorityMax, bool bTallestFirst,
    const int &iMaxSnagDecayClass) {

  clTree * p_oTree, //tree we're working with
  *p_oNextTree; //placeholder for getting the next tree
  float fDbh,
  fTemp,
  fThisBA; //basal area of a single tree;
  int i, iTemp, iCode;
  bool bTreeIsDead, bTreeIsPriority, bTemp, bKeepCuttingAfterPriority = true;

  //See if there's any cutting to do
  bTreeIsDead = true; //grabbing a handy bool variable as a temp
  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    bTreeIsDead = bTreeIsDead && p_fAmountRemoved[i] >= p_fAmountToRemove[i];
  if (bTreeIsDead) return false; //Don't need to look for more trees in the non-priority section

  //Now remove the trees
  p_oTree = GetFirstTreeInCutArea(p_cutArea, p_treeArea, iSp, bTallestFirst,
      iMaxSnagDecayClass);
  while (p_oTree) {
    bTreeIsDead = false;
    fThisBA = 0;

    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()),
        &fDbh);

    //Go through each cut range and see if a tree is in it
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i]) {

        //See if it is a priority tree
        bTreeIsPriority = false;
        if (floatType == iPriorityType) {
          iCode = mp_oPop->GetFloatDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &fTemp);
            if (fTemp >= fPriorityMin && fTemp <= fPriorityMax)
              bTreeIsPriority = true;
          }
        } else if (intType == iPriorityType) {
          iCode = mp_oPop->GetIntDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &iTemp);
            if (iTemp >= fPriorityMin && iTemp <= fPriorityMax)
              bTreeIsPriority = true;
          }
        } else {
          iCode = mp_oPop->GetBoolDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &bTemp);
            if ((bTemp && fPriorityMin == 1) || (!bTemp && fPriorityMin == 0))
              bTreeIsPriority = true;
          }
        }

        //Absolute amount of basal area - cut until we've reached the goal
        if (bTreeIsPriority && p_fAmountRemoved[i] < p_fAmountToRemove[i]) {
          fThisBA = clModelMath::CalculateBasalArea(fDbh);
          p_fAmountRemoved[i] += fThisBA;
          if (p_fAmountRemoved[i] <= p_fAmountToRemove[i])
            bTreeIsDead = true;
          else {
            //This tree takes us over the limit. If this is a large-to-small
            //size cut, don't cut it. A smaller tree might bring us closer.
            //If this is a small-to-large cut, do the thing that brings us
            //closest to the goal.
            if (bTallestFirst) {
              p_fAmountRemoved[i] -= fThisBA;
            } else {
              if (p_fAmountRemoved[i] - p_fAmountToRemove[i] >
                  p_fAmountToRemove[i] - (p_fAmountRemoved[i] - fThisBA)) {
                //Not cutting the tree gets us closest to the goal
                p_fAmountRemoved[i] -= fThisBA;
              } else {
                //Cutting the tree gets us closest to the goal
                bTreeIsDead = true;
              }
            }
            //Since we reached the limit with this tree, whether or not we
            //decided to cut it, it makes no sense to cut other trees.
            //Trigger a stop to the harvest.
            bKeepCuttingAfterPriority = false;
          }
        } //end of if (p_fAmountRemoved[i] < p_fAmountToRemove[i])
      } //end of (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
    } //end of for (i = 0; i < m_iNumAllowedCutRanges; i++)

    p_oNextTree = GetNextTreeInCutArea(p_cutArea, p_treeArea, iSp,
        bTallestFirst, iMaxSnagDecayClass);
    if (bTreeIsDead) {

      AddTreeStatsToResults(p_oTree, p_fLoDbh, p_fHiDbh, fDbh);
      mp_oPop->KillTree(p_oTree, m_iReasonCode);
    }

    p_oTree = p_oNextTree;
  } //end of while (p_oTree)
  return bKeepCuttingAfterPriority;
}

//////////////////////////////////////////////////////////////////////////////
// CutSpeciesAbsDenPriority()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::CutSpeciesAbsDenPriority(int iSp, stcGridList * & p_cutArea,
    stcTreeGridList * & p_treeArea,
    float *p_fLoDbh, float *p_fHiDbh, float *p_fAmountToRemove,
    float *p_fAmountRemoved, std::string sPriorityName, int &iPriorityType,
    float &fPriorityMin, float &fPriorityMax, bool bTallestFirst,
    const int &iMaxSnagDecayClass) {
  clTree * p_oTree, //tree we're working with
  *p_oNextTree; //placeholder for getting the next tree
  float fDbh,
  fTemp;
  //fThisBA; //basal area of a single tree;
  int i, iTemp, iCode;
  bool bTreeIsDead, bTreeIsPriority, bTemp;

  //See if there's any cutting to do
  bTreeIsDead = true; //grabbing a handy bool variable as a temp
  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    bTreeIsDead = bTreeIsDead && p_fAmountRemoved[i] >= p_fAmountToRemove[i];
  if (bTreeIsDead) return;

  //Now remove the trees
  p_oTree = GetFirstTreeInCutArea(p_cutArea, p_treeArea, iSp, bTallestFirst,
      iMaxSnagDecayClass);
  while (p_oTree) {
    bTreeIsDead = false;
    //fThisBA = 0;

    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);

    //Go through each cut range and see if a tree is in it
    for (i = 0; i < m_iNumAllowedCutRanges; i++) {
      if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i]) {

        //See if it is a priority tree
        bTreeIsPriority = false;
        if (floatType == iPriorityType) {
          iCode = mp_oPop->GetFloatDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &fTemp);
            if (fTemp >= fPriorityMin && fTemp <= fPriorityMax)
              bTreeIsPriority = true;
          }
        } else if (intType == iPriorityType) {
          iCode = mp_oPop->GetIntDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &iTemp);
            if (iTemp >= fPriorityMin && iTemp <= fPriorityMax)
              bTreeIsPriority = true;
          }
        } else {
          iCode = mp_oPop->GetBoolDataCode(sPriorityName, iSp, p_oTree->GetType());
          if (iCode > -1) {
            p_oTree->GetValue(iCode, &bTemp);
            if ((bTemp && fPriorityMin == 1) || (!bTemp && fPriorityMin == 0))
              bTreeIsPriority = true;
          }
        }

        //Absolute density - keep cutting until we've cut down all that
        //we're supposed to
        if (bTreeIsPriority && p_fAmountRemoved[i] < p_fAmountToRemove[i]) {
          bTreeIsDead = true;
          p_fAmountRemoved[i] += 1.0;
        } else
          p_oTree = NULL; //break the loop

      } //end of (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
    } //end of for (i = 0; i < m_iNumAllowedCutRanges; i++)

    p_oNextTree = GetNextTreeInCutArea(p_cutArea, p_treeArea, iSp,
        bTallestFirst, iMaxSnagDecayClass);
    if (bTreeIsDead) {

      AddTreeStatsToResults(p_oTree, p_fLoDbh, p_fHiDbh, fDbh);
      mp_oPop->KillTree(p_oTree, m_iReasonCode);
    }

    p_oTree = p_oNextTree;
  } //end of while (p_oTree)
}

//////////////////////////////////////////////////////////////////////////////
// AddTreeStatsToResults()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::AddTreeStatsToResults(clTree *p_oTree, float *p_fLoDbh,
    float *p_fHiDbh, float &fDbh) {
  float fX, fY, fTemp, fThisBA = 0;
  int i, iSp = p_oTree->GetSpecies(),
      iTemp;

  //Add the dead tree's stats to the grid keeping track of harvest cuts
  p_oTree->GetValue(mp_oPop->GetXCode(iSp, p_oTree->GetType()), &fX);
  p_oTree->GetValue(mp_oPop->GetYCode(iSp, p_oTree->GetType()), &fY);

  for (i = 0; i < m_iNumAllowedCutRanges; i++)
    if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i]) {
      mp_oResultsGrid->GetValueAtPoint(fX, fY,
          mp_iDenCutCodes[i][iSp], &iTemp);
      iTemp += 1;
      mp_oResultsGrid->SetValueAtPoint(fX, fY,
          mp_iDenCutCodes[i][iSp], iTemp);

      if (0 == fThisBA)
        fThisBA = clModelMath::CalculateBasalArea(fDbh);

      mp_oResultsGrid->GetValueAtPoint(fX, fY,
          mp_iBaCutCodes[i][iSp], &fTemp);
      fTemp += fThisBA;
      mp_oResultsGrid->SetValueAtPoint(fX, fY,
          mp_iBaCutCodes[i][iSp], fTemp);
    } //end of if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])


#ifdef test
  std::fstream harv( "HarvestTrees.txt", std::ios::out | std::ios::app );
  harv << fX << "\t" << fY << "\t" << iSp << "\t" << fDbh << "\n";
  harv.close();
#endif
}

//////////////////////////////////////////////////////////////////////////////
// AssembleCutArea()
//////////////////////////////////////////////////////////////////////////////
int clDisturbance::AssembleCutArea(clPackage * p_oMasterPackage,
    const int & iNumXCells, const int & iNumYCells, stcGridList * & p_cutArea,
    stcTreeGridList * &p_treeArea,
    float * p_fLoDbh, float * p_fHiDbh, float * p_fAmountToRemove,
    bool * p_bSpeciesCut) {
  struct stcGridList * p_cellRecord = NULL; //for creating cut area
  struct stcTreeGridList * p_treeRecord = NULL;
  float fTemp; //for extracting package values
  int iMasterID, //ID of master cell
  iTemp, //for getting package integer values
  iMinX, iMaxX, iMinY, iMaxY, iX, iY,
  iNumSpecies = mp_oPop->GetNumberOfSpecies(), iTimestep =
      mp_oSimManager->GetCurrentTimestep(), iNumCellsInCutArea = 0; //number of cells in the cut area
  short int i, j; //loop counters
  bool bTemp; //for getting package bool values

  //********************************************
  // Extract the data from the master package
  //********************************************

  //Get the master package's ID number
  p_oMasterPackage->GetValue(m_iMasterIDCode, &iMasterID);

  //Get the package's cut ranges and amounts
  for (i = 0; i < m_iNumAllowedCutRanges; i++) {
    //Low dbh
    p_oMasterPackage->GetValue(mp_iRangeMinCodes[i], &fTemp);
    p_fLoDbh[i] = fTemp;
    //High dbh
    p_oMasterPackage->GetValue(mp_iRangeMaxCodes[i], &fTemp);
    p_fHiDbh[i] = fTemp;
    //Amount
    p_oMasterPackage->GetValue(mp_iRangeAmountCodes[i], &fTemp);
    p_fAmountToRemove[i] = fTemp;
  }

  //Get the species list
  for (i = 0; i < iNumSpecies; i++) {
    p_oMasterPackage->GetValue(mp_iSpeciesCodes[i], &bTemp);
    p_bSpeciesCut[i] = bTemp;
  }

  //********************************************
  // Find the matching cut packages and assemble them into
  // our linked list
  //********************************************

  for (i = 0; i < iNumXCells; i++)
    for (j = 0; j < iNumYCells; j++) {

      //Reusing the p_oMasterPackage pointer
      p_oMasterPackage = mp_oCutEventsGrid->GetFirstPackageOfCell(i, j);
      while (p_oMasterPackage) {

        //Get this package's timestep to see if it's for this timestep
        p_oMasterPackage->GetValue(m_iCutTimestepCode, &iTemp);
        if (iTimestep != iTemp) {
          //Null out the pointer - this will break the loop
          p_oMasterPackage = NULL;
          goto nextPackage;
        }

        //Check the ID number
        p_oMasterPackage->GetValue(m_iCutIDCode, &iTemp);
        if (iTemp != iMasterID)
          goto nextPackage;

        //Make this a record in our cut list - stick it at the beginning
        p_cellRecord = new stcGridList;
        p_cellRecord->iDistX = i;
        p_cellRecord->iDistY = j;
        p_cellRecord->next = p_cutArea;
        p_cutArea = p_cellRecord;

        //Get all the tree cells that this could be part of and add them
        //to the list
        iMinX = (int) floor((i * m_fDistXCellLen) / m_fPopCellLen);
        iMaxX = (int) floor((((i+1) * m_fDistXCellLen)-0.001) / m_fPopCellLen);
        iMinY = (int) floor((j * m_fDistYCellLen) / m_fPopCellLen);
        iMaxY = (int) floor((((j+1) * m_fDistYCellLen)-0.001) / m_fPopCellLen);

        if (iMinX >= mp_oPop->GetNumXCells()) iMinX = mp_oPop->GetNumXCells() - 1;
        if (iMaxX >= mp_oPop->GetNumXCells()) iMaxX = mp_oPop->GetNumXCells() - 1;
        if (iMinY >= mp_oPop->GetNumYCells()) iMinY = mp_oPop->GetNumYCells() - 1;
        if (iMaxY >= mp_oPop->GetNumYCells()) iMaxY = mp_oPop->GetNumYCells() - 1;

        //Add them to the list, making sure the cell records are unique
        for (iX = iMinX; iX <= iMaxX; iX++) {
          for (iY = iMinY; iY <= iMaxY; iY++) {
            bTemp = false;
            p_treeRecord = p_treeArea;
            while (p_treeRecord) {
              if (p_treeRecord->iTreeX == iX &&
                  p_treeRecord->iTreeY == iY) {
                bTemp = true;
                break;
              }
              p_treeRecord = p_treeRecord->next;
            }
            if (!bTemp) {
              p_treeRecord = new stcTreeGridList;
              p_treeRecord->iTreeX = iX;
              p_treeRecord->iTreeY = iY;
              p_treeRecord->p_oTree = NULL;
              p_treeRecord->fDbh = 0;
              p_treeRecord->next = p_treeArea;
              p_treeArea = p_treeRecord;
            }
          }
        }

        iNumCellsInCutArea++;

        //We're done with this package - delete it
        mp_oCutEventsGrid->DeletePackage(p_oMasterPackage);
        p_oMasterPackage = NULL;

        nextPackage: if (p_oMasterPackage)
          p_oMasterPackage = p_oMasterPackage->GetNextPackage();
      } //end of while (p_oMasterPackage)
    } //end of for (j = 0; j < iNumYCells; j++)

  return iNumCellsInCutArea;
}

//////////////////////////////////////////////////////////////////////////////
// GetBasalArea()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::GetBasalArea(stcGridList * p_cutArea, stcTreeGridList * & p_treeArea,
    const short int & iSpecies, float * p_fTotalBasalArea, float * p_fLoDbh,
    float * p_fHiDbh, const int &iMaxSnagDecayClass) {
  clTree * p_oTree; //tree whose basal area we're getting
  stcTreeGridList * p_treeRecord = NULL; //grid cell
  float fDbh; //tree's dbh
  short int iTreeType, iTreeSpecies, //tree's type and species
  i; //loop counter

  p_treeRecord = p_treeArea;
  while (p_treeRecord) {
    p_oTree = mp_oPop->GetTallestTreeInCell(p_treeRecord->iTreeX, p_treeRecord->iTreeY);
    while (p_oTree) {
      iTreeSpecies = p_oTree->GetSpecies();
      iTreeType = p_oTree->GetType();
      if (iSpecies == iTreeSpecies            &&
          IsTreeInCutArea(p_oTree, p_cutArea) &&
          IsTreeOkay(p_oTree, iMaxSnagDecayClass)) {


        //----- If everything else is good, validate size -------------------//
        p_oTree->GetValue(mp_oPop->GetDbhCode(iTreeSpecies, iTreeType), &fDbh);
        for (i = 0; i < m_iNumAllowedCutRanges; i++)
          if (fDbh >= p_fLoDbh[i] && fDbh <= p_fHiDbh[i])
            p_fTotalBasalArea[i] += clModelMath::CalculateBasalArea(fDbh);
      } //end of if (iSpecies == iTreeSpecies...)
      p_oTree = p_oTree->GetShorter();
    } //end of while (p_oTree)
    p_treeRecord = p_treeRecord->next;
  } //end of while (p_treeRecord)
}

//////////////////////////////////////////////////////////////////////////////
// GetFirstTreeInCutArea()
//////////////////////////////////////////////////////////////////////////////
clTree* clDisturbance::GetFirstTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
   const short int &iSpecies, const bool bTallestFirst, const int &iMaxSnagDecayClass) {
  if (bTallestFirst) {
    return GetTallestTreeInCutArea(p_cutArea, p_treeArea, iSpecies, iMaxSnagDecayClass);
  }
  return GetShortestTreeInCutArea(p_cutArea, p_treeArea, iSpecies, iMaxSnagDecayClass);
}

//////////////////////////////////////////////////////////////////////////////
// GetTallestTreeInCutArea()
//////////////////////////////////////////////////////////////////////////////
clTree * clDisturbance::GetTallestTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    const short int & iSpecies, const int &iMaxSnagDecayClass) {
  clTree * p_oTree; //a tree we're working with
  stcTreeGridList * p_sortedCutArea = NULL, //where to put the sorted list
      *p_cellRecord, //one record in the cut area list
      *p_cellHolder, //for holding our place in the linked list
      *p_sorter; //for sorting the linked list
  float fDbh; //tree's dbh

  //Go through each of the cell list records and find the tallest tree of
  //the given species
  p_cellRecord = p_treeArea;
  while (p_cellRecord) {
    p_cellHolder = p_cellRecord->next;
    p_cellRecord->p_oTree = NULL;
    p_cellRecord->fDbh = 0;
    p_cellRecord->next = NULL;
    fDbh = 0;

    //Go through the trees to find the tallest
    p_oTree = mp_oPop->GetTallestTreeInCell(p_cellRecord->iTreeX,
        p_cellRecord->iTreeY);
    while (p_oTree) {
      if (p_oTree->GetSpecies() == iSpecies       &&
          IsTreeOkay(p_oTree, iMaxSnagDecayClass) &&
          IsTreeInCutArea(p_oTree, p_cutArea)) {

          //Found one that will do!
          break;
      } else {
        if (clTreePopulation::seedling == p_oTree->GetType()) {
          //No more trees to bother with here - null the pointer and break
          //the loop
          p_oTree = NULL;
          break;
        }
      }

      p_oTree = p_oTree->GetShorter();
    } //end of while (p_oTree)

  //Get the dbh of our tree, if it exists
  if (p_oTree)
    p_oTree->GetValue(mp_oPop->GetDbhCode(iSpecies, p_oTree->GetType()),
        &fDbh);
  else
    fDbh = 0;

  //Assign the values
  p_cellRecord->p_oTree = p_oTree;
  p_cellRecord->fDbh = fDbh;

  //Sort our record (tallest tree or not) into the new list
  p_sorter = p_sortedCutArea;
  if (NULL != p_sorter) {
    while (p_sorter) {

      //Does our record come first?
      if (p_sorter == p_sortedCutArea && p_sorter->fDbh <= fDbh) {
        p_cellRecord->next = p_sorter;
        p_sortedCutArea = p_cellRecord;
        break;
      }

      //Evaluate the other cases independently of the first - no else if here
      //Does our record come last?
      if (NULL == p_sorter->next) {
        p_sorter->next = p_cellRecord;
        break;

        //Our record comes somewhere in the middle
      } else if (p_sorter->next->fDbh <= fDbh) {

        //The new record goes after p_sorter
        p_cellRecord->next = p_sorter->next;
        p_sorter->next = p_cellRecord;
        break;
      }

      p_sorter = p_sorter->next;
    } //end of while (p_sorter)
  } else
    //this is first one to be sorted
    p_sortedCutArea = p_cellRecord;

  p_cellRecord = p_cellHolder;
} //end of while (p_cellRecord)

//OK, we're all sorted - return the tree in the first block and re-assign
//the linked list
p_treeArea = p_sortedCutArea;
return (p_treeArea->p_oTree);
}

//////////////////////////////////////////////////////////////////////////////
// GetShortestTreeInCutArea()
//////////////////////////////////////////////////////////////////////////////
clTree * clDisturbance::GetShortestTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    const short int & iSpecies, const int &iMaxSnagDecayClass) {
  clTree * p_oTree; //a tree we're working with
  stcTreeGridList * p_sortedCutArea = NULL, //where to put the sorted list
      *p_cellRecord, //one record in the cut area list
      *p_cellHolder, //for holding our place in the linked list
      *p_sorter; //for sorting the linked list
  float fDbh; //tree's dbh

  //Go through each of the cell list records and find the shortest tree of
  //the given species
  p_cellRecord = p_treeArea;
  while (p_cellRecord) {
    p_cellHolder = p_cellRecord->next;
    p_cellRecord->p_oTree = NULL;
    p_cellRecord->fDbh = 0;
    p_cellRecord->next = NULL;
    fDbh = 0;

    //Go through the trees to find the shortest
    p_oTree = mp_oPop->GetShortestTreeInCell(p_cellRecord->iTreeX,
        p_cellRecord->iTreeY);
    while (p_oTree) {
      if (p_oTree->GetSpecies() == iSpecies       &&
          IsTreeOkay(p_oTree, iMaxSnagDecayClass) &&
          IsTreeInCutArea(p_oTree, p_cutArea)) {

        //Found one that will do!
        break;
      }
      p_oTree = p_oTree->GetTaller();
    } //end of while (p_oTree)

    //Get the dbh of our tree, if it exists
    if (p_oTree)
      p_oTree->GetValue(mp_oPop->GetDbhCode(iSpecies, p_oTree->GetType()), &fDbh);
    else
      fDbh = 5000;

    //Assign the values
    p_cellRecord->p_oTree = p_oTree;
    p_cellRecord->fDbh = fDbh;

    //Sort our record (shortest tree or not) into the new list
    p_sorter = p_sortedCutArea;
    if (NULL != p_sorter) {
      while (p_sorter) {

        //Does our record come first?
        if (p_sorter == p_sortedCutArea && p_sorter->fDbh >= fDbh) {
          p_cellRecord->next = p_sorter;
          p_sortedCutArea = p_cellRecord;
          break;
        }

        //Evaluate the other cases independently of the first - no else if here
        //Does our record come last?
        if (NULL == p_sorter->next) {
          p_sorter->next = p_cellRecord;
          break;

          //Our record comes somewhere in the middle
        } else if (p_sorter->next->fDbh >= fDbh) {

          //The new record goes after p_sorter
          p_cellRecord->next = p_sorter->next;
          p_sorter->next = p_cellRecord;
          break;
        }

        p_sorter = p_sorter->next;
      } //end of while (p_sorter)
    } else
      //this is first one to be sorted
      p_sortedCutArea = p_cellRecord;

    p_cellRecord = p_cellHolder;
  } //end of while (p_cellRecord)

  //OK, we're all sorted - return the tree in the first block and re-assign
  //the linked list
  p_treeArea = p_sortedCutArea;
  return (p_treeArea->p_oTree);
}


//////////////////////////////////////////////////////////////////////////////
// GetNextTreeInCutArea()
//////////////////////////////////////////////////////////////////////////////
clTree * clDisturbance::GetNextTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    const short int & iSpecies, const bool bTallestFirst, const int &iMaxSnagDecayClass) {

  if (bTallestFirst) {
    return GetNextTallestTreeInCutArea(p_cutArea, p_treeArea,
        iSpecies, iMaxSnagDecayClass);
  }
  return GetNextShortestTreeInCutArea(p_cutArea, p_treeArea,
      iSpecies, iMaxSnagDecayClass);
}

//////////////////////////////////////////////////////////////////////////////
// GetNextTallestTreeInCutArea()
//////////////////////////////////////////////////////////////////////////////
clTree * clDisturbance::GetNextTallestTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    const short int & iSpecies, const int &iMaxSnagDecayClass) {
  clTree * p_oTree; //for finding the next tallest tree
  stcTreeGridList * p_cellRecord, //an individual record
  *p_sorter; //for sorting the list
  float fDbh; //tree's dbh

  //If we've already returned all the trees return NULL
  if (NULL == p_treeArea->p_oTree)
    return NULL;

  //The tallest tree is the one in the first cut record, and it's the one
  //that's no longer wanted. So update that cell with the next tallest one
  //of that species
  p_oTree = p_treeArea->p_oTree->GetShorter();
  while (p_oTree) {

    if (p_oTree->GetSpecies() == iSpecies       &&
        IsTreeOkay(p_oTree, iMaxSnagDecayClass) &&
        IsTreeInCutArea(p_oTree, p_cutArea)) {

      //Found one that will do!
      break;
    } else {
      if (clTreePopulation::seedling == p_oTree->GetType()) {
        //No more trees to bother with here - null the pointer and break
        //the loop
        p_oTree = NULL;
        break;
      }
    }

    p_oTree = p_oTree->GetShorter();
  } //end of while (p_oTree)

  p_cellRecord = p_treeArea;
  if (p_oTree)
    p_oTree->GetValue(mp_oPop->GetDbhCode(iSpecies, p_oTree->GetType()),
        &fDbh);
  else
    fDbh = 0;

  p_cellRecord->p_oTree = p_oTree;
  p_cellRecord->fDbh = fDbh;

  //Now move this record down in the list if necessary
  if (p_treeArea->next && fDbh < p_treeArea->next->fDbh) { //yep - needs to move
    p_sorter = p_treeArea->next;
    p_treeArea = p_sorter;
    while (p_sorter) {
      //Does our record come last?
      if (NULL == p_sorter->next) {
        p_sorter->next = p_cellRecord;
        p_cellRecord->next = NULL;
        break;

        //Our record comes somewhere in the middle
      } else if (p_sorter->next->fDbh <= fDbh) {

        //The new record goes after p_sorter
        p_cellRecord->next = p_sorter->next;
        p_sorter->next = p_cellRecord;
        break;
      }

      p_sorter = p_sorter->next;
    } //end of while (p_sorter)
  } //end of if (fDbh < p_oCutArea->next->fDbh)

  return (p_treeArea->p_oTree);
}

//////////////////////////////////////////////////////////////////////////////
// GetNextShortestTreeInCutArea()
//////////////////////////////////////////////////////////////////////////////
clTree * clDisturbance::GetNextShortestTreeInCutArea(stcGridList * & p_cutArea, stcTreeGridList * & p_treeArea,
    const short int & iSpecies, const int &iMaxSnagDecayClass) {
  clTree * p_oTree; //for finding the next shortest tree
  stcTreeGridList * p_cellRecord, //an individual record
  *p_sorter; //for sorting the list
  float fDbh; //tree's dbh

  //If we've already returned all the trees return NULL
  if (NULL == p_treeArea->p_oTree)
    return NULL;

  //The shortest tree is the one in the first cut record, and it's the one
  //that's no longer wanted. So update that cell with the next shortest one
  //of that species
  p_oTree = p_treeArea->p_oTree->GetTaller();
  while (p_oTree) {
    if (p_oTree->GetSpecies() == iSpecies       &&
        IsTreeOkay(p_oTree, iMaxSnagDecayClass) &&
        IsTreeInCutArea(p_oTree, p_cutArea)) {

      //Found one that will do!
      break;
    }

    p_oTree = p_oTree->GetTaller();
  } //end of while (p_oTree)

  p_cellRecord = p_treeArea;
  if (p_oTree)
    p_oTree->GetValue(mp_oPop->GetDbhCode(iSpecies, p_oTree->GetType()),
        &fDbh);
  else
    fDbh = 5000;

  p_cellRecord->p_oTree = p_oTree;
  p_cellRecord->fDbh = fDbh;

  //Now move this record down in the list if necessary
  if (p_treeArea->next && fDbh > p_treeArea->next->fDbh) { //yep - needs to move
    p_sorter = p_treeArea->next;
    p_treeArea = p_sorter;
    while (p_sorter) {
      //Does our record come last?
      if (NULL == p_sorter->next) {
        p_sorter->next = p_cellRecord;
        p_cellRecord->next = NULL;
        break;

        //Our record comes somewhere in the middle
      } else if (p_sorter->next->fDbh >= fDbh) {

        //The new record goes after p_sorter
        p_cellRecord->next = p_sorter->next;
        p_sorter->next = p_cellRecord;
        break;
      }

      p_sorter = p_sorter->next;
    } //end of while (p_sorter)
  } //end of if (fDbh < p_oCutArea->next->fDbh)

  return (p_treeArea->p_oTree);
}

//////////////////////////////////////////////////////////////////////////////
// SetCutFlags()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::SetCutFlags(stcGridList * p_cutArea, const int & iCutType) {
  stcGridList * p_record;
  int iTemp;

  p_record = p_cutArea;
  while (p_record) {

    //Make sure the cut severity type is set to the most severe type for
    //this grid cell
    mp_oResultsGrid->GetValueOfCell(p_record->iDistX, p_record->iDistY,
        m_iHarvestTypeCode, &iTemp);

    if (iTemp < iCutType)
      mp_oResultsGrid->SetValueOfCell(p_record->iDistX, p_record->iDistY,
          m_iHarvestTypeCode, iCutType);

    p_record = p_record->next;
  }
}

//////////////////////////////////////////////////////////////////////////////
// IsTreeInCutArea()
//////////////////////////////////////////////////////////////////////////////
bool clDisturbance::IsTreeInCutArea(clTree *p_oTree, stcGridList *p_cutArea) {
  stcGridList *p_cellRecord;
  float fX, fY;
  int iSp = p_oTree->GetSpecies(), iTp = p_oTree->GetType();

  p_oTree->GetValue(mp_oPop->GetXCode(iSp, iTp), &fX);
  p_oTree->GetValue(mp_oPop->GetYCode(iSp, iTp), &fY);

  p_cellRecord = p_cutArea;
  while (p_cellRecord) {

    if ((int) floor(fX / m_fDistXCellLen) == p_cellRecord->iDistX && (int) floor(fY
        / m_fDistYCellLen) == p_cellRecord->iDistY) {
      return true;
    }
    p_cellRecord = p_cellRecord->next;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// KillSeedlings()
//////////////////////////////////////////////////////////////////////////////
void clDisturbance::KillSeedlings(stcGridList *p_cutArea, stcTreeGridList * & p_treeArea, float *p_fKillProb) {

  clTree * p_oTree, *p_oNextTree;
  stcTreeGridList * p_cellRecord = NULL; //grid cell
  float fMaxSeedlingHeight = 0, fHeight = 0, fX, fY;
  int iNumSpecies = mp_oPop->GetNumberOfSpecies(), iSp, iTp, iTemp, iX, iY;

  //Quick check: see if any of the species actually need killing. If not, exit
  //Grabbing the handy fHeight variable as a temp
  for (iSp = 0; iSp < iNumSpecies; iSp++) {
    fHeight = fHeight > p_fKillProb[iSp] ? fHeight : p_fKillProb[iSp];
  }
  if (fHeight < 0.0001) return;

  //Get the maximum possible seedling height
  for (iSp = 0; iSp < iNumSpecies; iSp++) {
    fHeight = mp_oPop->GetMaxSeedlingHeight(iSp);
    fMaxSeedlingHeight = fHeight > fMaxSeedlingHeight ? fHeight
        : fMaxSeedlingHeight;
  }

  p_cellRecord = p_treeArea;
  while (p_cellRecord) {
    p_oTree = mp_oPop->GetShortestTreeInCell(p_cellRecord->iTreeX,
        p_cellRecord->iTreeY);
    while (p_oTree) {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();
      p_oTree->GetValue(mp_oPop->GetHeightCode(iSp, iTp), &fHeight);
      p_oNextTree = p_oTree->GetTaller();

      if ( clTreePopulation::seedling == iTp &&
          p_fKillProb[iSp] > 0 &&
          IsTreeInCutArea(p_oTree, p_cutArea)) {

        if (clModelMath::GetRand() <= p_fKillProb[iSp]) {
          p_oTree->GetValue(mp_oPop->GetXCode(iSp, iTp), &fX);
          p_oTree->GetValue(mp_oPop->GetYCode(iSp, iTp), &fY);
          iX = (int) floor(fX / m_fDistXCellLen);
          iY = (int) floor(fY / m_fDistYCellLen);
          mp_oPop->KillTree(p_oTree, m_iReasonCode);

          mp_oResultsGrid->GetValueOfCell(iX, iY, mp_iSeedlingCutCodes[iSp], &iTemp);
          iTemp++;
          mp_oResultsGrid->SetValueOfCell(iX, iY, mp_iSeedlingCutCodes[iSp], iTemp);
        }

      }
      p_oTree = fHeight > fMaxSeedlingHeight ? NULL : p_oNextTree;

    } //end of while (p_oTree)
    p_cellRecord = p_cellRecord->next;
  } //end of while (p_cellRecord)


}

//////////////////////////////////////////////////////////////////////////////
// IsTreeOkay
//////////////////////////////////////////////////////////////////////////////
bool clDisturbance::IsTreeOkay(clTree *p_oTree, const int &iMaxSnagDecayClass) {
  int iCode, iValue, iSpecies = p_oTree->GetSpecies(),
      iType = p_oTree->GetType();

  //----- Is this tree dead? ------------------------------------------------//
  iCode = mp_oPop->GetIntDataCode("dead", iSpecies, iType);
  if (-1 != iCode) {
    p_oTree->GetValue(iCode, &iValue);
  } else {
    // No trees are ever dead - unlikely but we'll roll with it
    iValue = notdead;
  }

  if (iValue != notdead) {
    //This tree is dead and therefore not okay!
    return false;
  }

  //----- What is this tree's life history stage? ---------------------------//
  if (clTreePopulation::adult   == iType ||
      clTreePopulation::sapling == iType) {

    //This is a tree type we will always consider. It's okay!
    return true;
  }

  if (clTreePopulation::seedling == iType) {

    //Seedlings are never okay!
    return false;
  }

  //Check to see if this is snag AND snags are okay
  if (clTreePopulation::snag == iType) {

    if (iMaxSnagDecayClass < 0) {
      //For this run, snags are never okay
      return false;
    }

    //Get decay class, allowing for it not to exist
    iCode = mp_oPop->GetIntDataCode("SnagDecayClass", iSpecies, iType);
    if (iCode == -1) {
      //Snag decay classes don't exist, but snags should be considered
      return true;

    } else {
      //Retrieve the current decay class
      p_oTree->GetValue(iCode, &iValue);
      if (iValue <= iMaxSnagDecayClass) {
        //Snag decay class exists, and it's less than the max; okay
        return true;
      } else {
        //Snag decay class exists, but it's too high; not okay
        return false;
      }
    }
  }

  //Whatever this tree is, we have no idea what to do with it. It's probably
  //not okay
  return false;
}
