//---------------------------------------------------------------------------
#include "PostHarvestSkiddingMort.h"
#include "MortalityOrg.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ModelMath.h"
#include "Plot.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clPostHarvestSkiddingMort::clPostHarvestSkiddingMort(clSimManager *p_oSimManager) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clMortalityBase(p_oSimManager) {
  try {

    m_sNameString = "PostHarvestSkiddingmortshell";
    m_sXMLRoot = "PostHarvestSkiddingMortality";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_fPreHarvestBackgroundMort = NULL;
    mp_fWindthrowHarvestBasicProb = NULL;
    mp_fSnagRecruitHarvestBasicProb = NULL;
    mp_fWindthrowSizeEffect = NULL;
    mp_fWindthrowHarvestIntensityEffect = NULL;
    mp_fSnagRecruitHarvestIntensityEffect = NULL;
    mp_fWindthrowCrowdingEffect = NULL;
    mp_fSnagRecruitCrowdingEffect = NULL;
    mp_fWindthrowHarvestRateParam = NULL;
    mp_fSnagRecruitHarvestRateParam = NULL;
    mp_fWindthrowBackgroundProb = NULL;
    mp_fSnagRecruitBackgroundProb = NULL;
    mp_iHarvestIntensityCodes = NULL;
    mp_oHarvestResultsGrid = NULL;
    mp_oTimeSinceHarvestGrid = NULL;

    m_iHarvestTypeCode = -1;
    m_iTimeCode = -1;
    m_iNumYCells = 0;
    m_iLastUpdated = 0;
    m_fNumberYearsPerTimestep = 0;
    m_iNumXCells = 0;
    m_fCrowdingEffectRadius = 0;
    m_iNumSpecies = 0;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPostHarvestSkiddingMort::clPostHarvestSkiddingMort";
    throw(stcErr);
  }
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clPostHarvestSkiddingMort::~clPostHarvestSkiddingMort() {

  delete[] mp_fPreHarvestBackgroundMort;
  delete[] mp_fWindthrowHarvestBasicProb;
  delete[] mp_fSnagRecruitHarvestBasicProb;
  delete[] mp_fWindthrowSizeEffect;
  delete[] mp_fWindthrowHarvestIntensityEffect;
  delete[] mp_fSnagRecruitHarvestIntensityEffect;
  delete[] mp_fWindthrowCrowdingEffect;
  delete[] mp_fSnagRecruitCrowdingEffect;
  delete[] mp_fWindthrowHarvestRateParam;
  delete[] mp_fSnagRecruitHarvestRateParam;
  delete[] mp_fWindthrowBackgroundProb;
  delete[] mp_fSnagRecruitBackgroundProb;

  if (mp_iHarvestIntensityCodes) {
    for (int i = 0; i < m_iNumSpecies; i++) {
      delete[] mp_iHarvestIntensityCodes[i];
    }
    delete[] mp_iHarvestIntensityCodes;
  }
}


////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clPostHarvestSkiddingMort::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
  floatVal *p_fTempValues;  //for getting species-specific values
  short int i, j; //loop counter

  m_iNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the temp array and populate it with the species to which this
  //behavior applies
  p_fTempValues = new floatVal[m_iNumBehaviorSpecies];

  for (i = 0; i < m_iNumBehaviorSpecies; i++)
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Declare the arrays for holding the variables
  mp_fPreHarvestBackgroundMort = new float[m_iNumTotalSpecies];
  mp_fWindthrowHarvestBasicProb = new float[m_iNumTotalSpecies];
  mp_fSnagRecruitHarvestBasicProb = new float[m_iNumTotalSpecies];
  mp_fWindthrowSizeEffect = new float[m_iNumTotalSpecies];
  mp_fWindthrowHarvestIntensityEffect = new float[m_iNumTotalSpecies];
  mp_fSnagRecruitHarvestIntensityEffect = new float[m_iNumTotalSpecies];
  mp_fWindthrowCrowdingEffect = new float[m_iNumTotalSpecies];
  mp_fSnagRecruitCrowdingEffect = new float[m_iNumTotalSpecies];
  mp_fWindthrowHarvestRateParam = new float[m_iNumTotalSpecies];
  mp_fSnagRecruitHarvestRateParam = new float[m_iNumTotalSpecies];
  mp_fWindthrowBackgroundProb = new float[m_iNumTotalSpecies];
  mp_fSnagRecruitBackgroundProb = new float[m_iNumTotalSpecies];

  //Create and initialize time since harvest grid
  SetupTimeSinceHarvestGrid();


  //Make sure behavior is only applied to allowed types

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType &&
        clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType)
    {

      modelErr stcErr;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  }



  //Capture the values from the parameter file


  //Pre-harvest background mortality rate (beta)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvPreHarvestBackgroundMort",
      "mo_phphbmVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fPreHarvestBackgroundMort[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fPreHarvestBackgroundMort[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_preHarvestBackgroundMort must be non-negative.";
      throw( stcErr );
    }
  }


  //Windthrow harvest basic prob (rho w)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvWindthrowHarvestBasicProb",
      "mo_phwhbpVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fWindthrowHarvestBasicProb[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fWindthrowHarvestBasicProb[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_windthrowHarvestBasicProb must be non-negative.";
      throw( stcErr );
    }
  }

  //Snag recruitment harvest basic prob (rho s)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvSnagRecruitHarvestBasicProb",
      "mo_phsrhbpVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fSnagRecruitHarvestBasicProb[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fSnagRecruitHarvestBasicProb[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_snagRecruitHarvestBasicProb must be non-negative.";
      throw( stcErr );
    }
  }


  //Windthrow size effect (delta w)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvWindthrowSizeEffect",
      "mo_phwseVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fWindthrowSizeEffect[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fWindthrowSizeEffect[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_windthrowSizeEffect must be non-negative.";
      throw( stcErr );
    }
  }


  //Windthrow harvest intensity effect (kappa w)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvWindthrowHarvestIntensityEffect",
      "mo_phwhieVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fWindthrowHarvestIntensityEffect[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fWindthrowHarvestIntensityEffect[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_windthrowHarvestIntensityEffect must be non-negative.";
      throw( stcErr );
    }
  }


  //Snag recruitment skidding effect (kappa s)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvSnagRecruitHarvestIntensityEffect",
      "mo_phsrhieVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fSnagRecruitHarvestIntensityEffect[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fSnagRecruitHarvestIntensityEffect[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_snagRecruitHarvestIntensityEffect must be non-negative.";
      throw( stcErr );
    }
  }

  //Windthrow crowding effect (eta w)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvWindthrowCrowdingEffect",
      "mo_phwceVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fWindthrowCrowdingEffect[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fWindthrowCrowdingEffect[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_windthrowCrowdingEffect must be non-negative.";
      throw( stcErr );
    }
  }

  //Snag recruitment crowding effect (phi s)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvSnagRecruitCrowdingEffect",
      "mo_phsrceVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fSnagRecruitCrowdingEffect[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fSnagRecruitCrowdingEffect[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_snagRecruitCrowdingEffect must be non-negative.";
      throw( stcErr );
    }
  }


  //Windthrow harvest rate param (tau w)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvWindthrowHarvestRateParam",
      "mo_phwhrpVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fWindthrowHarvestRateParam[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fWindthrowHarvestRateParam[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_windthrowHarvestRateParam must be non-negative.";
      throw( stcErr );
    }
  }


  //Snag recruitment harvest rate param (tau s)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvSnagRecruitHarvestRateParam",
      "mo_phsrhrpVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fSnagRecruitHarvestRateParam[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fSnagRecruitHarvestRateParam[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_snagRecruitHarvestRateParam must be non-negative.";
      throw( stcErr );
    }
  }



  //windthrow Background Prob (omega)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvWindthrowBackgroundProb",
      "mo_phwbpVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fWindthrowBackgroundProb[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fWindthrowBackgroundProb[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_windthrowBackgroundProb must be non-negative.";
      throw( stcErr );
    }
  }


  //Snag recruitment background prob (zeta)
  FillSpeciesSpecificValue(p_oElement, "mo_postHarvSnagRecruitBackgroundProb",
      "mo_phsrbpVal",
      p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);

  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_fSnagRecruitBackgroundProb[p_fTempValues[i].code] = p_fTempValues[i].val;

    if ( mp_fSnagRecruitBackgroundProb[p_fTempValues[i].code] < 0 ) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
      stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_snagRecruitBackgroundProb must be non-negative.";
      throw( stcErr );
    }
  }

  //Crowding effect radius
  FillSingleValue(p_oElement, "mo_postHarvCrowdingEffectRadius",
      & m_fCrowdingEffectRadius, true);

  if ( m_fCrowdingEffectRadius <= 0 ) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clPostHarvestSkiddingMort::DoShellSetup" ;
    stcErr.sMoreInfo = "The Post-harvest skidding mortality parameter mo_crowdingEffectRadius must be greater than zero.";
    throw( stcErr );
  }

  //Gets the codes for the HarvInten float data members
  //that may have been saved by harvest interface

  mp_iHarvestIntensityCodes = new int * [m_iNumSpecies];
  for (i = 0; i < m_iNumSpecies; i++) {
    mp_iHarvestIntensityCodes[i] = new int [clTreePopulation::adult + 1];
    for (j = 0; j < (clTreePopulation::adult + 1); j++) {
      mp_iHarvestIntensityCodes[i][j] = -1;
    }
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {

    mp_iHarvestIntensityCodes[mp_whatSpeciesTypeCombos[i].iSpecies][mp_whatSpeciesTypeCombos[i].iType] =
        p_oPop->GetFloatDataCode("HarvInten", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType);

  }


  m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

  delete[] p_fTempValues; p_fTempValues = NULL;

}


////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clPostHarvestSkiddingMort::DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies) {
  float fRandom, //random number

  fLocalBasalArea, //basal area around target tree

  fImmediateWindthrowRisk, //Iw, windthrow risk at t=0 after harvest
  fImmediateSnagRecruitRisk, //Is, snag recruitment risk at t=0 after harvest

  fAnnualWindthrowRisk, //w, risk of windthrow t years after harvest
  fAnnualSnagRecruitRisk, //s, risk of snag recruitment t years after harvest

  fBackgroundMort, //pre-harvest background mortality rate, compounded for years per timestep

  fHarvestIntensity=0; //harvest intensity around tree

  deadCode iDead = notdead;
  int i,
  iType = p_oTree->GetType(),
  iTimeSinceHarvest; //time since last harvest for this grid cell


  //bool bFallen = false;


  iTimeSinceHarvest = GetTimeSinceHarvest(p_oTree);

  if (iTimeSinceHarvest < 1000) { //there has been harvesting in this grid cell to date

    if (mp_iHarvestIntensityCodes[iSpecies][iType] > -1) {
      p_oTree->GetValue(mp_iHarvestIntensityCodes[iSpecies][iType], &fHarvestIntensity);
    } else {
      fHarvestIntensity = 0;
    }

    fLocalBasalArea = LocalBasalAreaAroundTree(p_oTree);

    fImmediateWindthrowRisk = mp_fWindthrowHarvestBasicProb[iSpecies]
                                                            + mp_fWindthrowSizeEffect[iSpecies]*fDbh
                                                            + mp_fWindthrowHarvestIntensityEffect[iSpecies]*fHarvestIntensity
                                                            - mp_fWindthrowCrowdingEffect[iSpecies]*fLocalBasalArea;

    fImmediateSnagRecruitRisk = mp_fSnagRecruitHarvestBasicProb[iSpecies]
                                                                + mp_fSnagRecruitHarvestIntensityEffect[iSpecies]*fHarvestIntensity
                                                                + mp_fSnagRecruitCrowdingEffect[iSpecies]*fLocalBasalArea;

    for (i=0;i<m_fNumberYearsPerTimestep;i++) {

      iTimeSinceHarvest++;

      fAnnualWindthrowRisk = fImmediateWindthrowRisk
          * exp(-1.0*mp_fWindthrowHarvestRateParam[iSpecies]*iTimeSinceHarvest)
      + mp_fWindthrowBackgroundProb[iSpecies];

      fAnnualSnagRecruitRisk = fImmediateSnagRecruitRisk
          * exp(-1.0*mp_fSnagRecruitHarvestRateParam[iSpecies]*iTimeSinceHarvest)
      + mp_fSnagRecruitBackgroundProb[iSpecies];

      fRandom = clModelMath::GetRand();

      fRandom -= fAnnualWindthrowRisk;
      if (fRandom < 0.0) {
        iDead = natural;
        //    bFallen = true;
        break;
      }

      fRandom -= fAnnualSnagRecruitRisk;
      if (fRandom < 0.0) {
        iDead = natural;
        //   bFallen = false;
        break;
      }


    } //end for (years per timestep)


  } //end if (time since harvest < 1000)

  else {  //no harvesting in this cell yet

    fBackgroundMort = 1.0 - pow(1.0-mp_fPreHarvestBackgroundMort[iSpecies],m_fNumberYearsPerTimestep);

    fRandom = clModelMath::GetRand();
    if (fRandom < fBackgroundMort)
      iDead = natural;
    else
      iDead = notdead;
    //what about bFallen? -- is unknown whether tree fell or not
  }
  return iDead;
}


////////////////////////////////////////////////////////////////////////////
// LocalBasalAreaAroundTree()
////////////////////////////////////////////////////////////////////////////
float clPostHarvestSkiddingMort::LocalBasalAreaAroundTree( clTree *p_oTree ) {

  clTreePopulation *p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

  clTreeSearch * p_oCloseTrees; //search object for accessing neighboring trees
  clTree * p_oNeighborTree; //one individual tree to work with

  float fTreeX, fTreeY, //the target tree's location
  fDbh,
  fBasalArea = 0.0; //total basal area within crowding radius

  int iSp = p_oTree->GetSpecies(),
      iTp = p_oTree->GetType();

  char cQuery[70];

  p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fTreeX);
  p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fTreeY);


  sprintf(cQuery, "distance=%4.4f FROM x=%4.4f,y=%4.4f::height=1.35", m_fCrowdingEffectRadius, fTreeX, fTreeY);

  p_oCloseTrees = p_oPop->Find(cQuery);

  p_oNeighborTree = p_oCloseTrees->NextTree();

  while (p_oNeighborTree) { //loop through trees within crowding radius

    if ((p_oNeighborTree != p_oTree) && (p_oNeighborTree->GetType() == clTreePopulation::adult)) {
      //if the tree is an adult and isn't the focal tree itself

      //add it to the calculated basal area
      p_oNeighborTree->GetValue(p_oPop->GetDbhCode(p_oNeighborTree->GetSpecies(), p_oNeighborTree->GetType()), &fDbh);
      fBasalArea += clModelMath::CalculateBasalArea(fDbh);

    } //end if (

    p_oNeighborTree = p_oCloseTrees->NextTree();

  } //end while

  fBasalArea /= 1.0 * M_PI * pow(m_fCrowdingEffectRadius,2) / 10000.0;

  return fBasalArea;
}



////////////////////////////////////////////////////////////////////////////
// PreMortCalcs()
////////////////////////////////////////////////////////////////////////////
void clPostHarvestSkiddingMort::PreMortCalcs( clTreePopulation *p_oPop ) {

    CalcTimeSinceHarvest();

}


////////////////////////////////////////////////////////////////////////////
// CalcTimeSinceHarvest()
////////////////////////////////////////////////////////////////////////////
void clPostHarvestSkiddingMort::CalcTimeSinceHarvest() {
  try {


    int i, j,
    iHarvestedThisTimestep, //a value other than -1 if a grid cell was harvested this timestep
    iNewTimeSinceHarvest, //updated time since harvest
    iTimestepLength = (int)mp_oSimManager->GetNumberOfYearsPerTimestep(),
    iCurrentTimestep = (int)mp_oSimManager->GetCurrentTimestep(),
    iLastUpdated;

    //bool bAnyRecentHarvesting = false;


    if (mp_oHarvestResultsGrid) { //if there is cutting in this simulation

      mp_oTimeSinceHarvestGrid->GetValueOfCell( 0, 0, m_iLastUpdated, &iLastUpdated);

      if (iLastUpdated < iCurrentTimestep) { //if the grid hasn't yet been updated

        //traverse through cells of Harvest Results grid and update Harvest Type value as needed
        for(i=0; i<m_iNumXCells; i++)
          for(j=0; j<m_iNumYCells; j++)
          {
            //Get the harvest type value from the grid cell
            mp_oHarvestResultsGrid->GetValueOfCell(i, j, m_iHarvestTypeCode, &iHarvestedThisTimestep);

            //if the value is not -1, then harvesting occurred so set TimeSinceHarvest to 0
            //and set flag that recent harvesting has occurred
            if(iHarvestedThisTimestep != -1)
            {
              mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTimeCode, 0 );

             // bAnyRecentHarvesting = true;

            }//end if
            //Otherwise increment TimeSinceHarvest by the length of a timestep
            else
            {
              mp_oTimeSinceHarvestGrid->GetValueOfCell( i, j, m_iTimeCode, &iNewTimeSinceHarvest);
              iNewTimeSinceHarvest += iTimestepLength;
              mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTimeCode, iNewTimeSinceHarvest);
            } //end else
          } //end for (loop through grid cells)

        mp_oTimeSinceHarvestGrid->SetValueOfCell( 0, 0, m_iLastUpdated, iCurrentTimestep);

      } //end if grid not yet updated

    } //end if harvest results exists

  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPostHarvestSkiddingMort::CalcTimeSinceHarvest";
    throw(stcErr);
  }
}



////////////////////////////////////////////////////////////////////////////
// SetupTimeSinceHarvestGrid()
////////////////////////////////////////////////////////////////////////////
void clPostHarvestSkiddingMort::SetupTimeSinceHarvestGrid() {
  try {

    int i, j;

    clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;
    float fCellLength = p_oPop->GetGridCellSize();


    //check if the grid alread exists

    mp_oTimeSinceHarvestGrid = mp_oSimManager->GetGridObject( "Years Since Last Harvest" );

    if (!mp_oTimeSinceHarvestGrid) {
      //if it doesn't, then create it
      mp_oTimeSinceHarvestGrid = mp_oSimManager->CreateGrid("Years Since Last Harvest", //grid name
          2, //number of int data members
          0, //number of float data members
          0, //number of char data members
          0, //number of bool data members
          fCellLength, //Number of X cells
          fCellLength); //Number of Y cells


      //register the data members
      m_iTimeCode = mp_oTimeSinceHarvestGrid->RegisterInt( "Time" );
      m_iLastUpdated = mp_oTimeSinceHarvestGrid->RegisterInt( "LastUpdated" );

      //figure out its size
      m_iNumXCells = mp_oTimeSinceHarvestGrid->GetNumberXCells();
      m_iNumYCells = mp_oTimeSinceHarvestGrid->GetNumberYCells();


      //initialize the TimeSinceHarvestGrid with values of 1000 (an arbitrarily long period of time)
      for(i=0; i<m_iNumXCells; i++)
        for(j=0; j<m_iNumYCells; j++) {
          mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTimeCode, 1000);

        }
    }

    else {

      //the grid has already been created

      //get the data member codes
      m_iTimeCode = mp_oTimeSinceHarvestGrid->GetIntDataCode("Time");
      m_iLastUpdated = mp_oTimeSinceHarvestGrid->GetIntDataCode("LastUpdated");

      //figure out its size
      m_iNumXCells = mp_oTimeSinceHarvestGrid->GetNumberXCells(),
          m_iNumYCells = mp_oTimeSinceHarvestGrid->GetNumberYCells();

    }

    //Get a pointer to the harvest results grid
    mp_oHarvestResultsGrid = mp_oSimManager->GetGridObject("Harvest Results");

    //if harvest results grid exists, get the code for harvest type
    if (mp_oHarvestResultsGrid)
      m_iHarvestTypeCode = mp_oHarvestResultsGrid->GetPackageIntDataCode("Harvest Type");

  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPostHarvestSkiddingMort::SetupTimeSinceHarvestGrid";
    throw(stcErr);
  }
}



////////////////////////////////////////////////////////////////////////////
// GetTimeSinceHarvest()
////////////////////////////////////////////////////////////////////////////
int clPostHarvestSkiddingMort::GetTimeSinceHarvest( clTree *p_oTree ) {
  clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;

  float fTreeX, fTreeY; //the target tree's location

  int iSp = p_oTree->GetSpecies(),
      iTp = p_oTree->GetType(),
      iTime;

  p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fTreeX);
  p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fTreeY);


  if (!mp_oTimeSinceHarvestGrid)

    return 1000;

  else {

    mp_oTimeSinceHarvestGrid->GetValueAtPoint(fTreeX, fTreeY, m_iTimeCode, &iTime);
    return iTime;

  }
}
