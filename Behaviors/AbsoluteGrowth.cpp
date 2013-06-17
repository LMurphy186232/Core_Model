//---------------------------------------------------------------------------
#include "AbsoluteGrowth.h"
#include "GrowthOrg.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clAbsoluteGrowth::clAbsoluteGrowth(clSimManager *p_oSimManager) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clGrowthBase( p_oSimManager), clMichMenBase(p_oSimManager) {

  try {
    mp_fLengthLastSuppFactor = NULL;
    mp_fLengthCurrReleaseFactor = NULL;
    mp_fGliThreshold = NULL;
    mp_ylrCodes = NULL;
    mp_ylsCodes = NULL;

    m_iMaxYears = 20;

    m_iNumBehaviorTypes = 0;
    m_iYrsExceedThresholdBeforeSup = 0;
    m_fMortRateAtSuppression = 0;
    m_fNumberYearsPerTimestep = 0;

    m_sNameString = "absolutegrowthshell";
    m_iNewTreeInts = 2;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clAbsoluteGrowth::clAbsoluteGrowth";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clAbsoluteGrowth::~clAbsoluteGrowth() {
  delete[] mp_fLengthLastSuppFactor;
  delete[] mp_fLengthCurrReleaseFactor;
  delete[] mp_fGliThreshold;

  if (mp_ylrCodes) {
    for (int i = 0; i < m_iNumBehaviorTypes; i++) {
      delete[] mp_ylrCodes[i].p_iCodes;
    }
  }
  if (mp_ylsCodes) {
    for (int i = 0; i < m_iNumBehaviorTypes; i++) {
      delete[] mp_ylsCodes[i].p_iCodes;
    }
  }
  delete[] mp_ylrCodes;
  delete[] mp_ylsCodes;
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
/////////////////////////////////////////////////////////////////////////////*/
void clAbsoluteGrowth::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("AbsRadialGrowth") == 0) {
      m_bConstRadialLimited = true;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = diameter_auto;
      m_sXMLRoot = "AbsRadialGrowth";
    } else if (sNameString.compare("AbsBAGrowth") == 0) {
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = true;
      m_iGrowthMethod = diameter_auto;
      m_sXMLRoot = "AbsBAGrowth";
    } else if (sNameString.compare("AbsUnlimGrowth") == 0){
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = diameter_auto;
      m_sXMLRoot = "AbsUnlimGrowth";
    } else if (sNameString.compare("AbsRadialGrowth diam only") == 0) {
      m_bConstRadialLimited = true;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = diameter_only;
      m_sXMLRoot = "AbsRadialGrowth";
    } else if (sNameString.compare("AbsBAGrowth diam only") == 0) {
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = true;
      m_iGrowthMethod = diameter_only;
      m_sXMLRoot = "AbsBAGrowth";
    } else if (sNameString.compare("AbsUnlimGrowth diam only") == 0){
      m_bConstRadialLimited = false;
      m_bConstBasalAreaLimited = false;
      m_iGrowthMethod = diameter_only;
      m_sXMLRoot = "AbsUnlimGrowth";
    } else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sFunction = "clAbsoluteGrowth::SetNameData";
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clAbsoluteGrowth::SetNameData";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
//////////////////////////////////////////////////////////////////////////////
void clAbsoluteGrowth::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  try {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    xercesc::DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
    floatVal *p_fTempValues;  //for getting species-specific values
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(),
        i;

    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    mp_fAsympDiamGrowth = new float[iNumSpecies];
    mp_fSlopeDiamGrowthResponse = new float[iNumSpecies];

    if (m_bConstRadialLimited) {
      mp_fAdultConstRadInc = new float[iNumSpecies];
    }

    if (m_bConstBasalAreaLimited) {
      mp_fAdultConstBAInc = new float[iNumSpecies];
    }

    //Read the base variables
    GetParameterFileData(p_oDoc);

    //Declare the suppression arrays - we'll do them as total number of species
    //even though we may not be applying suppression to all species, since the
    //couple extra floats of space is more than compensated for by array access
    //speed
    mp_fLengthLastSuppFactor = new float[iNumSpecies];
    mp_fLengthCurrReleaseFactor = new float[iNumSpecies];


    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Length of last suppression factor
    FillSpeciesSpecificValue(p_oElement, "gr_lengthLastSuppFactor", "gr_llsfVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

    //Transfer values to our permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fLengthLastSuppFactor[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Length of current release factor
    FillSpeciesSpecificValue(p_oElement, "gr_lengthCurrReleaseFactor",
        "gr_lcrfVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

    //Transfer values to our permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fLengthCurrReleaseFactor[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Years exceeding suppression threshold...
    FillSingleValue(p_oElement, "gr_yrsExceedThresholdBeforeSupp",
        &m_iYrsExceedThresholdBeforeSup, true);

    //Mortality rate at suppression
    FillSingleValue(p_oElement, "gr_mortRateAtSuppression",
        &m_fMortRateAtSuppression, true);
    //If the mortality rate is 1, shave off a bit to avoid a log domain error
    //later
    if (1 == m_fMortRateAtSuppression) m_fMortRateAtSuppression -= 0.01;

    //Calculate the suppression thresholds
    CalculateSuppressionThresholds(p_oDoc, p_oPop);

    //Make sure all species/type combos have "Light" registered
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      if (-1 == mp_oGrowthOrg->GetLightCode(mp_whatSpeciesTypeCombos[i].iSpecies,
          mp_whatSpeciesTypeCombos[i].iType)) {
        modelErr stcErr;
        stcErr.sFunction = "clAbsoluteGrowth::DoShellSetup" ;
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " does not have a required light behavior.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    delete[] p_fTempValues;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clAbsoluteGrowth::DoShellSetup";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalculateSuppressionThresholds()
//////////////////////////////////////////////////////////////////////////////
void clAbsoluteGrowth::CalculateSuppressionThresholds(xercesc::DOMDocument *p_oDoc,
    clTreePopulation *p_oPop) {
  try {
    floatVal *p_fMortAtZeroGrowth, //mortality at zero growth - from par file
    *p_fLightDepMort;     //light dependent mortality - from par file
    DOMElement *p_oElement = p_oDoc->getDocumentElement();
    float fGrowthAtMortThreshold,  //growth value at mortality threshold
    fLogGrowth;              //log10 of growth at mort threshold
    short int iNumSpecies,         //total number of species
    i;                   //loop counter

    //Declare the mortality values array and pre-load with species codes
    p_fMortAtZeroGrowth = new floatVal[m_iNumBehaviorSpecies];
    p_fLightDepMort = new floatVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      p_fMortAtZeroGrowth[i].code = mp_iWhatSpecies[i];
      p_fLightDepMort[i].code = mp_iWhatSpecies[i];
    }

    //Capture the applicable mortality values - they are required
    FillSpeciesSpecificValue(p_oElement, "mo_mortAtZeroGrowth", "mo_mazgVal",
        p_fMortAtZeroGrowth, m_iNumBehaviorSpecies, p_oPop, true);

    FillSpeciesSpecificValue(p_oElement, "mo_lightDependentMortality",
        "mo_ldmVal", p_fLightDepMort, m_iNumBehaviorSpecies, p_oPop, true);

    //Declare the thresholds array - make it of size total # species
    iNumSpecies = p_oPop->GetNumberOfSpecies();
    mp_fGliThreshold = new float[iNumSpecies];
    for (i = 0; i < iNumSpecies; i++)
      mp_fGliThreshold[i] = 0;

    //Now for each behavior species, calculate the threshold for that species
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {

      //Calculate the growth at the mortality threshold
      if (0 < m_iYrsExceedThresholdBeforeSup * p_fMortAtZeroGrowth[i].val &&
          p_fLightDepMort[i].val > 0) {
        fGrowthAtMortThreshold =
            log(m_iYrsExceedThresholdBeforeSup * p_fMortAtZeroGrowth[i].val /
                log(1/(1-m_fMortRateAtSuppression))) / p_fLightDepMort[i].val + 1;
      } else {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Cannot calculate suppression thresholds.";
        stcErr.sFunction = "clAbsoluteGrowth::CalculateSuppressionThresholds";
        throw(stcErr);
      }

      if (fGrowthAtMortThreshold >= 0) {
        fLogGrowth = log10(fGrowthAtMortThreshold);
        if (mp_fAsympDiamGrowth[mp_iWhatSpecies[i]] != fLogGrowth)

          mp_fGliThreshold[mp_iWhatSpecies[i]] =
              mp_fSlopeDiamGrowthResponse[mp_iWhatSpecies[i]] * fLogGrowth /
              (mp_fAsympDiamGrowth[mp_iWhatSpecies[i]] - fLogGrowth);

        else
          ; //figure out what is supposed to happen and install correction
        //or error message

      } else
        //the parameter file did not include parameters
        //or there is an error condition and growth is negative
        mp_fGliThreshold[mp_iWhatSpecies[i]] = 0.0;

    } //end of for (i = 0; i < m_iNumBehaviorSpecies; i++)

    delete[] p_fMortAtZeroGrowth;
    delete[] p_fLightDepMort;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clAbsoluteGrowth::CalculateSuppressionThresholds";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
/////////////////////////////////////////////////////////////////////////////*/
void clAbsoluteGrowth::RegisterTreeDataMembers() {
  try {
    clPopulationBase *p_oTemp = mp_oSimManager->GetPopulationObject("treepopulation");
    clTreePopulation *p_oPop = (clTreePopulation *)p_oTemp;
    short int *p_iTypesList,  //array holding unique types
    iNumTypes,      //number of unique types
    iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
    i, j;           //loop counters
    bool bFound;              //flag for whether a type under consideration has
    //already been added to the list

    //Call the base class version as well
    clGrowthBase::RegisterTreeDataMembers();

    //************************************
    //Assemble the list of unique types
    //************************************

    //Declare the temp. types array to be as big as the combo list to make
    //sure we have space for everything, and initialize values to -1
    p_iTypesList = new short int[m_iNumSpeciesTypeCombos];
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++)
      p_iTypesList[i] = -1;

    //Go through each combo, and for the type for that combo, if it's not
    //already on the temp list, add it
    iNumTypes = 0;
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      bFound = false;
      //Test to see if this type is already on the list
      for (j = 0; j < iNumTypes; j++) {
        if (mp_whatSpeciesTypeCombos[i].iType == p_iTypesList[j])
        {bFound = true; break;}
      }
      if (!bFound) {
        //Add the type to the list and increment the number of found types
        //by one
        p_iTypesList[iNumTypes] = mp_whatSpeciesTypeCombos[i].iType;
        iNumTypes++;
      }
    } //end of for (i = 0; i < m_iNumSpeciesTypeCombos; i++)

    //************************************
    //Declare the return codes arrays and populate their type codes
    //************************************
    m_iNumBehaviorTypes = iNumTypes;
    mp_ylrCodes = new stcCodes[m_iNumBehaviorTypes];
    mp_ylsCodes = new stcCodes[m_iNumBehaviorTypes];

    for (i = 0; i < m_iNumBehaviorTypes; i++) {
      mp_ylrCodes[i].iType = p_iTypesList[i];
      mp_ylsCodes[i].iType = p_iTypesList[i];

      mp_ylrCodes[i].p_iCodes = new short int[iNumTotalSpecies];
      mp_ylsCodes[i].p_iCodes = new short int[iNumTotalSpecies];

      for (j = 0; j < iNumTotalSpecies; j++) {
        mp_ylrCodes[i].p_iCodes[j] = -1;
        mp_ylsCodes[i].p_iCodes[j] = -1;
      }
    }

    //************************************
    //Register the variables for what's actually in our type/species
    //combos
    //************************************
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      //Find the index of the matching type
      for (j = 0; j < m_iNumBehaviorTypes; j++) {
        if (mp_ylrCodes[j].iType == mp_whatSpeciesTypeCombos[i].iType)
          break;

      }

      //Register the code and capture it
      mp_ylrCodes[j].p_iCodes[mp_whatSpeciesTypeCombos[i].iSpecies] =
          p_oPop->RegisterInt("ylr", mp_whatSpeciesTypeCombos[i].iSpecies,
              mp_whatSpeciesTypeCombos[i].iType);

      mp_ylsCodes[j].p_iCodes[mp_whatSpeciesTypeCombos[i].iSpecies] =
          p_oPop->RegisterInt("yls", mp_whatSpeciesTypeCombos[i].iSpecies,
              mp_whatSpeciesTypeCombos[i].iType);
    }

    delete[] p_iTypesList;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clAbsoluteGrowth::RegisterTreeDataMembers";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue()
//////////////////////////////////////////////////////////////////////////////
float clAbsoluteGrowth::CalcDiameterGrowthValue(clTree *p_oTree,
    clTreePopulation *p_oPop, float fHeightGrowth) {
  float fLogRadialGrowth,        //log10 of annual radial growth
  fGli,                    //tree's gli value
  fDiam,                   //tree's diameter value
  fSuppressionFactor,      //suppression factor
  fAmountDiamIncrease;     //amount by which the tree's diameter will
  //increase
  short int iSpecies = p_oTree->GetSpecies(),
      iType = p_oTree->GetType(),
      iLightCode = mp_oGrowthOrg->GetLightCode(iSpecies, iType);

  //***************************************
  // Calculate the amount of growth
  //***************************************

  //Get the gli value for this tree
  p_oTree->GetValue(iLightCode, &fGli);

  //Get the appropriate diameter for this tree
  p_oTree->GetValue(mp_oGrowthOrg->GetDiamCode(iSpecies, iType), &fDiam);

  //Get the suppression factor for this tree
  fSuppressionFactor = CalculateSuppressionFactor(p_oTree, fGli);

  //Get the value of the Michaelis-Menton function - this gives log of
  //annual growth in mm/yr
  fLogRadialGrowth = fSuppressionFactor *
      CalculateMichaelisMentonDiam(iSpecies, fGli);

  //Get the amount of diameter by undoing log
  fAmountDiamIncrease = pow(10, fLogRadialGrowth) - 1;

  //The amount of increase is currently annual radial increase in mm/yr.
  //We need to convert to diameter increase per timestep in cm.
  fAmountDiamIncrease *= m_fConvertMmPerYearToCmPerTS;

  fAmountDiamIncrease = ApplyGrowthLimits(iSpecies,fAmountDiamIncrease, fDiam);

  return fAmountDiamIncrease;
}


//////////////////////////////////////////////////////////////////////////////
// CalculateSuppressionFactor()
/////////////////////////////////////////////////////////////////////////////*/
float clAbsoluteGrowth::CalculateSuppressionFactor(clTree *p_oTree,
    const float &fGli){
  float fFactor = 1.0;        //suppression factor being returned
  int iYls, iYlr;             //values in yls and ylr data members
  short int iIndex = 0,       //array index for species/type combo
      iSpecies = p_oTree->GetSpecies(),
      iType = p_oTree->GetType(),
      i;                //loop counter
  bool bIsSuppressed = false; //whether or not the tree is suppressed this
  //timestep

  //Was the tree's gli this timestep below the suppression threshold?
  if(fGli <= mp_fGliThreshold[iSpecies])
    bIsSuppressed = true;

  //Get the type index for accessing yls and ylr
  for (i = 0; i < m_iNumBehaviorTypes; i++) {
    if (iType == mp_ylsCodes[i].iType) {
      iIndex = i;
      break;
    }
  }

  //Get the tree's yls and ylr values
  p_oTree->GetValue(mp_ylsCodes[iIndex].p_iCodes[iSpecies], &iYls);
  p_oTree->GetValue(mp_ylrCodes[iIndex].p_iCodes[iSpecies], &iYlr);

  if (bIsSuppressed) {

    //Manage the tree's suppression/release length tracking variables
    if (iYlr > 0) {

      //Was released but is now suppressed - record this last timestep as
      //years suppressed, and set the release counter to 0
      iYls = (int)m_fNumberYearsPerTimestep;
      iYlr = 0;

    } else if (iYlr == 0)

      //Was suppressed and is suppressed - tack on this timestep to the tally
      iYls += (int)m_fNumberYearsPerTimestep;

  } else  //is released
    iYlr += (int)m_fNumberYearsPerTimestep;

  //Limit tracking to maximum number of years
  if (iYlr >= m_iMaxYears)  iYlr = m_iMaxYears;
  if (iYls >= m_iMaxYears)  iYls = m_iMaxYears;

  //Reassign these variables to the tree
  p_oTree->SetValue(mp_ylsCodes[iIndex].p_iCodes[iSpecies], iYls);
  p_oTree->SetValue(mp_ylrCodes[iIndex].p_iCodes[iSpecies], iYlr);

  //Calculate the suppression factor
  fFactor = exp((mp_fLengthCurrReleaseFactor[iSpecies] * iYlr) -
      (mp_fLengthLastSuppFactor[iSpecies] * iYls));

  if (fFactor > 1) fFactor = 1;
  if (fFactor < 0) fFactor = 0;

  return fFactor;
}
