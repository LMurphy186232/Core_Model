//---------------------------------------------------------------------------
// MicroEstablishment.cpp
//---------------------------------------------------------------------------
#include "MicroEstablishment.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include <math.h>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clMicroEstablishment::clMicroEstablishment(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try {
    m_sNameString = "MicroEstablishment";
    m_sXMLRoot = "MicroEstablishment";

    mp_iSeedGridCode = NULL;
    mp_iCounterCodes = NULL;
    mp_iZCodes = NULL;
    mp_iIndexes = NULL;
    mp_iSubstrateCodes = NULL;
    mp_oSubstrateGrid = NULL;
    mp_oSeedGrid = NULL;

    m_fFreshLogB = 0;
    m_fFreshLogA = 0;
    m_fFreshLogHeightMean = 0;
    m_iMaxRespiteYears = 0;
    m_fMoundProportion = 0;
    m_fMoundHeightMean = 0;
    m_iCohAgeCode = -1;
    m_iCohFreshLogCode = -1;
    m_fFreshLogStandardDeviation = 0;
    m_fMoundStandardDeviation = 0;
    m_iMaxRespiteTimesteps = 0;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMicroEstablishment::clMicroEstablishment";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clMicroEstablishment::~clMicroEstablishment() {
  delete[] mp_iSeedGridCode;
  delete[] mp_iCounterCodes;
  delete[] mp_iZCodes;
  delete[] mp_iIndexes;
  delete[] mp_iSubstrateCodes;
}

///////////////////////////////////////////////////////////////////////////////
// GetParameterFileData
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::GetParameterFileData(xercesc::DOMDocument * p_oDoc) {
  clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  short int i;

  mp_iIndexes = new short int[p_oPop->GetNumberOfSpecies()];
  //Load the indexes array
  for (i = 0; i < m_iNumBehaviorSpecies; i++)
    mp_iIndexes[mp_iWhatSpecies[i]] = i;

  //Proportion of the plot that is mound
  FillSingleValue(p_oElement, "es_moundProportion", &m_fMoundProportion, true);

  //Maximum number of years of respite
  FillSingleValue(p_oElement, "es_maxRespite", &m_iMaxRespiteYears, true);

  //Calculate the number of timesteps of respite
  m_iMaxRespiteTimesteps = (int) ceil(m_iMaxRespiteYears
      / mp_oSimManager->GetNumberOfYearsPerTimestep());

  //Mean mound height in m
  FillSingleValue(p_oElement, "es_meanMoundHeight", &m_fMoundHeightMean, true);
  //Convert to mm
  m_fMoundHeightMean *= 1000;

  //Mound height standard deviation in m
  FillSingleValue(p_oElement, "es_moundStdDev", &m_fMoundStandardDeviation,
      true);
  //Convert to mm
  m_fMoundStandardDeviation *= 1000;

  //Mean fresh log height in m
  FillSingleValue(p_oElement, "es_meanFreshLogHeight", &m_fFreshLogHeightMean,
      true);
  //Convert to mm
  m_fFreshLogHeightMean *= 1000;

  //Fresh log height standard deviation in m
  FillSingleValue(p_oElement, "es_freshLogStdDev",
      &m_fFreshLogStandardDeviation, true);
  //Convert to mm
  m_fFreshLogStandardDeviation *= 1000;

  //Substrate parameters - get first available in the parameter file
  p_oElement = p_oDoc->getDocumentElement();
  //Fresh log decay alpha
  FillSingleValue(p_oElement, "su_freshLogDecayAlpha", &m_fFreshLogA, true);
  //Fresh log decay beta
  FillSingleValue(p_oElement, "su_freshLogDecayBeta", &m_fFreshLogB, true);

  if (m_fMoundProportion < 0 || m_fMoundProportion > 1) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clMicroEstablishment::GetParameterFileData";
    stcErr.sMoreInfo = "Proportion of plot that is mound must be between 0 and 1.";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::GetData(xercesc::DOMDocument * p_oDoc) {
  try {
    GetParameterFileData(p_oDoc);
    SetupGrids();
    GetTreeDataMemberCodes();
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMicroEstablishment::GetData";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// SetupGrids()
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::SetupGrids() {
  std::stringstream sLabel;
  int i;

  //************************************
  //Get the "Substrate" grid and its data members
  //************************************
  mp_oSubstrateGrid = mp_oSimManager->GetGridObject("Substrate");
  if (!mp_oSubstrateGrid) {
    modelErr stcErr;
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    stcErr.sFunction = "clMicroEstablishment::SetUpGrids";
    stcErr.sMoreInfo = "Substrate behavior required.";
    throw(stcErr);
  }

  mp_iSubstrateCodes = new short int[number_substrates];
  mp_iSubstrateCodes[mound_ffmoss] = mp_oSubstrateGrid->GetFloatDataCode(
      "ffmoss");
  mp_iSubstrateCodes[mound_tipup]
      = mp_oSubstrateGrid->GetFloatDataCode("tipup");
  mp_iSubstrateCodes[mound_freshlogs] = mp_oSubstrateGrid->GetFloatDataCode(
      "freshlog");
  mp_iSubstrateCodes[mound_decayedlogs] = mp_oSubstrateGrid->GetFloatDataCode(
      "declog");
  mp_iSubstrateCodes[mound_fflitter] = mp_oSubstrateGrid->GetFloatDataCode(
      "fflitter");
  mp_iSubstrateCodes[mound_scarsoil] = mp_oSubstrateGrid->GetFloatDataCode(
      "scarsoil");
  mp_iSubstrateCodes[ground_ffmoss] = mp_oSubstrateGrid->GetFloatDataCode(
      "ffmoss");
  mp_iSubstrateCodes[ground_tipup] = mp_oSubstrateGrid->GetFloatDataCode(
      "tipup");
  mp_iSubstrateCodes[ground_freshlogs] = mp_oSubstrateGrid->GetFloatDataCode(
      "freshlog");
  mp_iSubstrateCodes[ground_decayedlogs] = mp_oSubstrateGrid->GetFloatDataCode(
      "declog");
  mp_iSubstrateCodes[ground_fflitter] = mp_oSubstrateGrid->GetFloatDataCode(
      "fflitter");
  mp_iSubstrateCodes[ground_scarsoil] = mp_oSubstrateGrid->GetFloatDataCode(
      "scarsoil");

  //Make sure that all the codes are valid
  for (i = 0; i < number_substrates; i++) {
    if (-1 == mp_iSubstrateCodes[i]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clMicroEstablishment::SetUpGrids";
      stcErr.sMoreInfo = "Unexpected grid data return code for Substrate.";
      throw(stcErr);
    }
  }

  //Package codes
  m_iCohFreshLogCode = mp_oSubstrateGrid->GetPackageFloatDataCode("freshlog");
  m_iCohAgeCode = mp_oSubstrateGrid->GetPackageIntDataCode("age");

  //Make sure that all the codes are valid
  if (-1 == m_iCohFreshLogCode || -1 == m_iCohAgeCode) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clMicroEstablishment::SetUpGrids";
    stcErr.sMoreInfo = "Unexpected grid data return code for Substrate.";
    throw(stcErr);
  }

  //************************************
  //Get the "Dispersed Seeds" grid and its data members
  //************************************
  mp_oSeedGrid = mp_oSimManager->GetGridObject("Dispersed Seeds");
  if (NULL == mp_oSeedGrid) {
    modelErr stcErr;
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    stcErr.sFunction = "clMicroEstablishment::SetupGrids";
    stcErr.sMoreInfo = "Disperse behaviors are required if establishment is to be used.";
    throw(stcErr);
  }
  mp_iSeedGridCode = new short int[m_iNumBehaviorSpecies];
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    sLabel << "seeds_" << mp_iWhatSpecies[i];
    mp_iSeedGridCode[i] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
    if (-1 == mp_iSeedGridCode[i]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clMicroEstablishment::SetupGrids";
      stcErr.sMoreInfo = "Unexpected grid data return code for Dispersed Seeds.";
      throw(stcErr);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetTreeDataMemberCodes()
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::GetTreeDataMemberCodes() {
  clTreePopulation
      * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject(
          "treepopulation");
  char cCounter[] = "lf_count", cZ[] = "z";
  int i;

  mp_iCounterCodes = new short int[m_iNumBehaviorSpecies];
  mp_iZCodes = new short int[m_iNumBehaviorSpecies];

  //Get the values for each species for seedlings
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_iCounterCodes[i] = p_oPop->GetIntDataCode(cCounter, mp_iWhatSpecies[i],
        clTreePopulation::seedling);
    mp_iZCodes[i] = p_oPop->GetIntDataCode(cZ, mp_iWhatSpecies[i],
        clTreePopulation::seedling);

    //If the return code is -1, throw an error
    if (-1 == mp_iCounterCodes[i] || -1 == mp_iZCodes[i]) {
      modelErr stcErr;
      stcErr.sFunction = "clMicroEstablishment::GetTreeDataMemberCodes";
      stcErr.sMoreInfo = "Microtopographic Establishment must be used with the light filter behavior.";
      stcErr.iErrorCode = BAD_DATA;
      throw(stcErr);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::Action() {
  try {
    clTreePopulation * p_oPop =
        (clTreePopulation *) mp_oSimManager->GetPopulationObject(
            "treepopulation");
    ;
    clTree * p_oSeedling; //newly created seedling
    float * p_fSubstrateProportions = new float[number_substrates], //proportions of each type of substrate
        * p_fCumSubstrate = new float[number_substrates], //cumulative proportion - helps us know how
        //much substrate we have left
        * p_fFreshLogProportions = new float[m_iMaxRespiteTimesteps + 1], //proportions of different age fresh logs
        fSeedX, fSeedY, //coordinates of seed
        fXCellLength = mp_oSeedGrid->GetLengthXCells(), //length of seed grid x
        fYCellLength = mp_oSeedGrid->GetLengthYCells(), //length of seed grid y
        fXOrig, fYOrig, //coordinates of origin of a grid cell
        fXEnd, fYEnd, //coordinates of end of a grid cell
        fNumOrigSeeds, //number of total seeds for a species for a grid
        fNumSeedsLeft, //number of seeds per grid - managing subtraction
        fNumSubstrateSeeds; //number of seeds on an individual substrate
    int iNumXCells = mp_oSeedGrid->GetNumberXCells(), //number X cells
        iNumYCells = mp_oSeedGrid->GetNumberYCells(), //number Y cells
        iX, iY, //loop counters for moving through grid cells
        iSpecies, //species of seed being dispersed
        i, j, iSubstr; //loop counter for substrate types

    //Cycle through the seeds grid and deal with each seed in each one
    for (iX = 0; iX < iNumXCells; iX++) {
      for (iY = 0; iY < iNumYCells; iY++) {

        //Get the origin and end of this cell of the "Dispersed Seeds" grid
        //(min and max X and Y values)
        fXOrig = mp_oSeedGrid->GetOriginXOfCell(iX);
        fYOrig = mp_oSeedGrid->GetOriginYOfCell(iY);
        fXEnd = mp_oSeedGrid->GetEndXOfCell(iX);
        fYEnd = mp_oSeedGrid->GetEndXOfCell(iY);

        //If these are the last cells in the plot, reduce them down 1 cm to
        //make sure no trees get too close to the edge of the plot - this
        //ensures that output writing won't round them up to the plot length
        if (iX == (iNumXCells - 1))
          fXEnd -= 0.01;
        if (iY == (iNumYCells - 1))
          fYEnd -= 0.01;

        //Get the substrate proportions for this cell
        GetSubstrateProportions(p_fSubstrateProportions, fXOrig, fYOrig, fXEnd,
            fYEnd);

        //Get cumulative substrate
        for (i = 0; i < number_substrates; i++)
          p_fCumSubstrate[i] = p_fSubstrateProportions[i];
        for (i = 1; i < number_substrates; i++)
          p_fCumSubstrate[i] += p_fCumSubstrate[i - 1];
        p_fCumSubstrate[number_substrates - 1] = 1;

        //Get the fresh log proportions for this cell
        GetFreshLogProportions(p_fFreshLogProportions, fXOrig, fYOrig, fXEnd,
            fYEnd);

        //Loop through each behavior species
        for (i = 0; i < m_iNumBehaviorSpecies; i++) {

          iSpecies = mp_iWhatSpecies[i];

          //Get the number of seeds in this cell for this species
          mp_oSeedGrid->GetValueOfCell(iX, iY,
              mp_iSeedGridCode[mp_iIndexes[iSpecies]], &fNumOrigSeeds);
          fNumSeedsLeft = fNumOrigSeeds;

          //Loop through the substrate types
          for (iSubstr = 0; iSubstr < number_substrates; iSubstr++) {

            //How many seeds are for this substrate?
            if (0 == fNumSeedsLeft)
              break; //goto nextSpecies;
            fNumSubstrateSeeds = clModelMath::RandomRound(fNumOrigSeeds
                * p_fSubstrateProportions[iSubstr]);
            if (fNumSubstrateSeeds > fNumSeedsLeft)
              fNumSubstrateSeeds = fNumSeedsLeft;
            fNumSeedsLeft -= fNumSubstrateSeeds;
            //Make sure we use up all the seeds - if this is the last
            //substrate then add any leftover seeds (should be no more than 1
            //or 2)
            if (1 - p_fCumSubstrate[iSubstr] < 0.01) {
              fNumSubstrateSeeds += fNumSeedsLeft;
              fNumSeedsLeft = 0;
            }

            //Split up the substrate types - this saves us if statements
            //for each seed
            if (mound_freshlogs == iSubstr || ground_freshlogs == iSubstr) {

              //This is fresh log

              //Disperse each of the seeds
              for (j = 0; j < fNumSubstrateSeeds; j++) {

                //Make this seed into a seedling.
                //Get a random X and Y value for this seed within the grid cell
                fSeedX = fXOrig + (clModelMath::GetRand() * fXCellLength);
                fSeedY = fYOrig + (clModelMath::GetRand() * fYCellLength);

                //Create the new seedling - allow a random diameter at 10 cm
                p_oSeedling = p_oPop->CreateTree(fSeedX, fSeedY, iSpecies,
                    clTreePopulation::seedling, 0);

                //Give it its rooting height and fern respite counters
                SetFreshLogZAndRespite(p_oSeedling, p_fFreshLogProportions);
              }
            } else if (iSubstr >= mound_scarsoil && iSubstr < ground_scarsoil) {

              //This is a mound substrate
              //Disperse each of the seeds
              for (j = 0; j < fNumSubstrateSeeds; j++) {
                //Make this seed into a seedling.
                //Get a random X and Y value for this seed within the grid cell
                fSeedX = fXOrig + (clModelMath::GetRand() * fXCellLength);
                fSeedY = fYOrig + (clModelMath::GetRand() * fYCellLength);

                //Create the new seedling - allow a random diameter at 10 cm
                p_oSeedling = p_oPop->CreateTree(fSeedX, fSeedY, iSpecies,
                    clTreePopulation::seedling, 0);

                //Give it its rooting height and fern respite counters
                SetMoundZAndRespite(p_oSeedling);
              }

            } else {
              //This is a ground substrate
              //Disperse each of the seeds
              for (j = 0; j < fNumSubstrateSeeds; j++) {
                //Make this seed into a seedling.
                //Get a random X and Y value for this seed within the grid cell
                fSeedX = fXOrig + (clModelMath::GetRand() * fXCellLength);
                fSeedY = fYOrig + (clModelMath::GetRand() * fYCellLength);

                //Create the new seedling - allow a random diameter at 10 cm
                p_oSeedling = p_oPop->CreateTree(fSeedX, fSeedY, iSpecies,
                    clTreePopulation::seedling, 0);

                //Give it its rooting height and fern respite counters
                SetGroundZAndRespite(p_oSeedling);
              }
            }
          }
          //nextSpecies: iSpecies = iSpecies;
        }
      }
    }
    delete[] p_fSubstrateProportions;
    delete[] p_fCumSubstrate;
    delete[] p_fFreshLogProportions;
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMicroEstablishment::Action";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// SetGroundZAndRespite()
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::SetGroundZAndRespite(clTree * p_oSeedling) {
  int iValue = 0, //value to be set
      iSp = p_oSeedling->GetSpecies(); //seedling's species
  //Set both z and lf_count to 0
  p_oSeedling->SetValue(mp_iZCodes[mp_iIndexes[iSp]], iValue);
  p_oSeedling->SetValue(mp_iCounterCodes[mp_iIndexes[iSp]], iValue);
}

///////////////////////////////////////////////////////////////////////////////
// SetMoundZAndRespite()
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::SetMoundZAndRespite(clTree * p_oSeedling) {
  int iValue = 0, //value to be set
      iSp = p_oSeedling->GetSpecies(); //seedling's species
  //Set lf_count to 0
  p_oSeedling->SetValue(mp_iCounterCodes[mp_iIndexes[iSp]], iValue);

  //Get a lognormally distributed number for z
  iValue = (int) floor(m_fMoundHeightMean + clModelMath::NormalRandomDraw(
      m_fMoundStandardDeviation));
  if (iValue < 0)
    iValue = 0;
  p_oSeedling->SetValue(mp_iZCodes[mp_iIndexes[iSp]], iValue);
}

///////////////////////////////////////////////////////////////////////////////
// SetFreshLogZAndRespite()
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::SetFreshLogZAndRespite(clTree * p_oSeedling,
    float * p_fFreshLogProportions) {
  float fRandom = clModelMath::GetRand(); //random number
  int iValue = 0, i, //value to be set
      iArraySize = m_iMaxRespiteTimesteps + 1, iSp = p_oSeedling->GetSpecies(); //seedling's species

  //Find the age of the fresh log cohort onto which the seed has landed
  for (i = 0; i < iArraySize; i++) {
    if (fRandom < p_fFreshLogProportions[i]) {
      iValue = i;
      break;
    }
  }

  //Subtract the age of the cohort from the max years of respite and set it
  //to the counter
  iValue = m_iMaxRespiteYears - (int) ceil(iValue
      * mp_oSimManager->GetNumberOfYearsPerTimestep());
  p_oSeedling->SetValue(mp_iCounterCodes[mp_iIndexes[iSp]], iValue);

  //Get a lognormally distributed number for z
  iValue = (int) floor(m_fFreshLogHeightMean + clModelMath::NormalRandomDraw(
      m_fFreshLogStandardDeviation));
  if (iValue < 0)
    iValue = 0;
  p_oSeedling->SetValue(mp_iZCodes[mp_iIndexes[iSp]], iValue);
}

/////////////////////////////////////////////////////////////////////////////
// GetSubstrateProportions()
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::GetSubstrateProportions(float *p_fProportions,
    const float & fFromX, const float & fFromY, const float & fToX,
    const float & fToY) {
  float fGroundProportion = 1 - m_fMoundProportion;
  int i; //loop counter

  //Place the substrate proportions in the array
  for (i = 0; i < number_substrates; i++) {
    p_fProportions[i] = mp_oSubstrateGrid->GetAverageFloatValue(fFromX, fFromY,
        fToX, fToY, mp_iSubstrateCodes[i]);
  }

  //Now reduce them by the mound/ground proportion
  for (i = mound_scarsoil; i <= mound_ffmoss; i++) {
    p_fProportions[i] *= m_fMoundProportion;
  }

  for (i = ground_scarsoil; i <= ground_ffmoss; i++) {
    p_fProportions[i] *= fGroundProportion;
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetFreshLogProportions()
/////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::GetFreshLogProportions(float *p_fProportions,
    const float & fFromX, const float & fFromY, const float & fToX,
    const float & fToY) {
  clPackage * p_oCohort;
  float fTotal = 0, //total fresh log proportion
      fValue = 0, //float-of-all-trades
      fXCellLength = mp_oSubstrateGrid->GetLengthXCells(), fYCellLength =
          mp_oSubstrateGrid->GetLengthYCells();
  int iMinXGridCell = (int) floor(fFromX / fXCellLength), //minimum X grid
      iMinYGridCell = (int) floor(fFromY / fYCellLength), //minimum Y grid
      iMaxXGridCell = (int) floor(fToX / fXCellLength), //maximum X grid
      iMaxYGridCell = (int) floor(fToY / fYCellLength), //maximum Y grid
      iYGrid, iXGrid, i, //loop counters
      iAge, //age of substrate cohort
      iNumTimesteps = m_iMaxRespiteTimesteps + 1;

  for (i = 0; i < iNumTimesteps; i++) {
    p_fProportions[i] = 0;
  }

  //Loop through each substrate cell in this seed cell's area.
  for (iXGrid = iMinXGridCell; iXGrid <= iMaxXGridCell; iXGrid++) {
    for (iYGrid = iMinYGridCell; iYGrid <= iMaxYGridCell; iYGrid++) {

      p_oCohort = mp_oSubstrateGrid->GetFirstPackageOfCell(iXGrid, iYGrid);
      while (p_oCohort) {

        //Get this cohort's age
        p_oCohort->GetValue(m_iCohAgeCode, &iAge);
        //Decrement the age 1 since substrate will have aged it in prep for
        //the next timestep and we want this timestep's age.
        iAge--;

        //Get the decayed logs value
        p_oCohort->GetValue(m_iCohFreshLogCode, &fValue);
        fValue *= exp(m_fFreshLogA * pow(iAge, m_fFreshLogB));

        if (iAge < m_iMaxRespiteTimesteps) {
          //Add to the proper array place if young enough
          p_fProportions[iAge] += fValue;
        }

        //Add it to the total of all freshlog
        fTotal += fValue;

        p_oCohort = p_oCohort->GetNextPackage();
      }
    }
  }
  //Proportions:
  //Now that we have a running total of all values, divide by that
  if (fTotal > 0) {
    for (i = 0; i < iNumTimesteps; i++) {
      p_fProportions[i] /= fTotal;
    }
  }

  //Now make it a cumulative total - divide each by the total of all values
  //and add to the previous value
  for (i = 1; i < m_iMaxRespiteTimesteps; i++) {
    p_fProportions[i] += p_fProportions[i - 1];
  }
  p_fProportions[m_iMaxRespiteTimesteps] = 1;
}

///////////////////////////////////////////////////////////////////////////////
// TimestepCleanup
///////////////////////////////////////////////////////////////////////////////
void clMicroEstablishment::TimestepCleanup() {
  float fNumSeeds = 0;
  int iNumXCells = mp_oSeedGrid->GetNumberXCells(), iNumYCells =
      mp_oSeedGrid->GetNumberYCells(), iSp, iX, iY; //loop counters

  for (iX = 0; iX < iNumXCells; iX++) {
    for (iY = 0; iY < iNumYCells; iY++) {
      for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
        mp_oSeedGrid->SetValueOfCell(iX, iY,
            mp_iSeedGridCode[mp_iIndexes[iSp]], fNumSeeds);
      }
    }
  }
}
