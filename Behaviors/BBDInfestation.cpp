//---------------------------------------------------------------------------
// BBDInfestation.cpp
//---------------------------------------------------------------------------
#include "BBDInfestation.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "ModelMath.h"
#include "TreePopulation.h"
#include "Plot.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clBBDInfestation::clBBDInfestation( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "BBDInfestation";
    m_sXMLRoot = "BBDInfestation";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_iYearsInfestedCodes = NULL;
    mp_iBBDResistanceStatusCodes = NULL;
    mp_fMinDBH = NULL;
    mp_fCohortDBH = NULL;
    mp_fProbResistant = NULL;
    mp_fProbConditionallySusceptible = NULL;
    m_cQuery = NULL;
    m_iTotalNumSpecies = 0;
    m_fBx = 0;
    m_fBy = 0;
    m_fA = 0;
    m_fMax = 1;
    m_fPlotBA = 0;
    m_fPoolBA = 0;
    m_iNumSmallCohortPoolTrees = 0;
    m_iNumLargeCohortPoolTrees = 0;
    m_iNumSmallCohortInfestedTrees = 0;
    m_iNumLargeCohortInfestedTrees = 0;
    m_iNumSmallInfestibleTrees = 0;
    m_iNumLargeInfestibleTrees = 0;
    m_iFirstTimestep = 0;
    m_iYearsOfInfestation = 0;
    m_fSmallCohortProb = 0;
    m_fLargeCohortProb = 0;

    m_iNewTreeInts = 2;
  }
  catch ( modelErr & err )
  {
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBBDInfestation::clBBDInfestation" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clBBDInfestation::~clBBDInfestation()
{
  if (mp_iYearsInfestedCodes) {
    int i;
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_iYearsInfestedCodes[i];
  }
  delete[] mp_iYearsInfestedCodes;

  if (mp_iBBDResistanceStatusCodes) {
    int i;
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_iBBDResistanceStatusCodes[i];
  }
  delete[] mp_iBBDResistanceStatusCodes;

  delete[] mp_fMinDBH;
  delete[] mp_fCohortDBH;
  delete[] m_cQuery;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////

void clBBDInfestation::GetData( xercesc::DOMDocument * p_oDoc )
{
  clTreePopulation *p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");

  ReadParFile( p_oDoc, p_oPop );
  FormatQueryString( p_oPop );

  if (m_iFirstTimestep <= 0) InfestInitialConditionsTrees();
}

/////////////////////////////////////////////////////////////////////////////
// ReadParFile()
////////////////////////////////////////////////////////////////////////////

void clBBDInfestation::ReadParFile( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  floatVal * p_fTempValues = NULL; //for getting species-specific values
  try {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    int i;

    //Declare our arrays
    mp_fMinDBH = new float[m_iTotalNumSpecies];
    mp_fCohortDBH = new float[m_iTotalNumSpecies];
    mp_fProbResistant = new float[m_iTotalNumSpecies];
    mp_fProbConditionallySusceptible = new float[m_iTotalNumSpecies];

    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];


    //Maximum infestation rate
    FillSingleValue( p_oElement, "di_BBDMaxInfestation", &m_fMax, true );
    //Make sure all values are between 0 and 1
    if (m_fMax < 0 || m_fMax > 1) {
      modelErr stcErr;
      stcErr.sFunction = "clBBDInfestation::ReadParFile" ;
      stcErr.sMoreInfo = "Max infestation value must be between 0 and 1.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }


    // a
    FillSingleValue( p_oElement, "di_BBDA", &m_fA, true );


    // bx
    FillSingleValue( p_oElement, "di_BBDBx", &m_fBx, true );

    // by
    FillSingleValue( p_oElement, "di_BBDBy", &m_fBy, true );

    //Minimum DBH to infest
    FillSpeciesSpecificValue( p_oElement, "di_BBDMinDBH", "di_bmdVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //greater than 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fMinDBH[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fMinDBH[p_fTempValues[i].code] < 0) {
        modelErr stcErr;
        stcErr.sFunction = "clBBDInfestation::ReadParFile" ;
        stcErr.sMoreInfo = "Minimum DBH for insect infestation cannot be negative.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Cohort cutoff DBH
    FillSpeciesSpecificValue( p_oElement, "di_BBDMCohortDBH", "di_bcdVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //greater than 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fCohortDBH[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fCohortDBH[p_fTempValues[i].code] < 0) {
        modelErr stcErr;
        stcErr.sFunction = "clBBDInfestation::ReadParFile" ;
        stcErr.sMoreInfo = "Minimum cohort DBH for insect infestation cannot be negative.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Proportion of trees that are resistant
    FillSpeciesSpecificValue( p_oElement, "di_BBDPropResistant", "di_bprVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //greater than 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fProbResistant[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fProbResistant[p_fTempValues[i].code] < 0 ||
          mp_fProbResistant[p_fTempValues[i].code] > 1) {
        modelErr stcErr;
        stcErr.sFunction = "clBBDInfestation::ReadParFile" ;
        stcErr.sMoreInfo = "Proportion of trees that are resistant must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Proportion of trees that are conditionally susceptible
    FillSpeciesSpecificValue( p_oElement, "di_BBDPropCondSusceptible", "di_bpcsVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //greater than 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fProbConditionallySusceptible[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fProbConditionallySusceptible[p_fTempValues[i].code] < 0 ||
          mp_fProbConditionallySusceptible[p_fTempValues[i].code] > 1) {
        modelErr stcErr;
        stcErr.sFunction = "clBBDInfestation::ReadParFile" ;
        stcErr.sMoreInfo = "Proportion of trees that are conditionally susceptible  must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Year to begin infestation
    FillSingleValue( p_oElement, "di_BBDStartYear", &m_iFirstTimestep, true );
    //Transform to timestep
    m_iFirstTimestep /= mp_oSimManager->GetNumberOfYearsPerTimestep();

    delete[] p_fTempValues;
  }
  catch ( modelErr & err )
  {
    delete[] p_fTempValues;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTempValues;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBBDInfestation::ReadParFile" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clBBDInfestation::Action()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTreeSearch *p_oBehaviorTrees = p_oPop->Find( m_cQuery );
  clTree * p_oTree = p_oBehaviorTrees->NextTree();
  float fDbh, fB, fTargetRate;
  int iSp, iTp, iInf, iStatus,
  iNumYrsTimestep = (int)mp_oSimManager->GetNumberOfYearsPerTimestep();

  //If we haven't reached the first timestep of infestation, exit
  if (m_iFirstTimestep > mp_oSimManager->GetCurrentTimestep()) return;

  //Do current inventory
  TreeInventory();

  //Calculate the value of B
  fB = m_fBx * (m_fPoolBA / m_fPlotBA) + m_fBy;

  //Calculate the proportion of trees infested
  fTargetRate = m_fMax * exp(-exp(m_fA - fB * m_iYearsOfInfestation));

  DetermineCohortInfestationProbability(fTargetRate);

  //Go through trees - increment counters and assess for new infestation
  while (p_oTree) {
    iSp = p_oTree->GetSpecies();
    iTp = p_oTree->GetType();

    if (-1 < mp_iYearsInfestedCodes[iSp][iTp]) {
      p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);

      //If the tree is over the minimum infestible size, check its status
      if (fDbh >= mp_fMinDBH[iSp]) {
        p_oTree->GetValue(mp_iYearsInfestedCodes[iSp][iTp], &iInf);

        if (iInf > 0) {

          //This tree is infested - increment time infested
          iInf += iNumYrsTimestep;
          p_oTree->SetValue(mp_iYearsInfestedCodes[iSp][iTp], iInf);

        } else {

          //Tree is not infested - check its resistance status
          p_oTree->GetValue(mp_iBBDResistanceStatusCodes[iSp][iTp], &iStatus);

          //Only continue if it's not resistant
          if (iStatus > resistant) {

            //Check for resistance based on the tree's cohort
            if (fDbh < mp_fCohortDBH[iSp]) {
              if (clModelMath::GetRand() <= m_fSmallCohortProb) {
                iInf = iNumYrsTimestep;
                p_oTree->SetValue(mp_iYearsInfestedCodes[iSp][iTp], iInf);
              }
            } else {
              if (clModelMath::GetRand() <= m_fLargeCohortProb) {
                iInf = iNumYrsTimestep;
                p_oTree->SetValue(mp_iYearsInfestedCodes[iSp][iTp], iInf);
              }
            }
          }
        }
      }
    }
    p_oTree = p_oBehaviorTrees->NextTree();
  }

  m_iYearsOfInfestation += iNumYrsTimestep;
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clBBDInfestation::FormatQueryString(clTreePopulation *p_oPop)
{
  char * cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];
  char cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSapling = false, bAdult = false;

  //Do a type/species search on all the types and species
  strcpy( cQueryTemp, "species=" );
  for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
  {
    sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
    strcat( cQueryTemp, cQueryPiece );
  }
  sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
  strcat( cQueryTemp, cQueryPiece );

  //Find all the types
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSapling = true;
    }
    else if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType )
    {
      bAdult = true;
    }
  }
  strcat( cQueryTemp, "::type=" );
  if ( bSapling )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::sapling, "," );
    strcat( cQueryTemp, cQueryPiece );
  }
  if ( bAdult )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::adult, "," );
    strcat( cQueryTemp, cQueryPiece );
  }

  //Remove the last comma
  cQueryTemp[strlen( cQueryTemp ) - 1] = '\0';

  //Now put it in m_cQuery, sized correctly
  m_cQuery = new char[strlen( cQueryTemp ) + 1];
  strcpy( m_cQuery, cQueryTemp );
  delete[] cQueryTemp;
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clBBDInfestation::RegisterTreeDataMembers()
{

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int i, j,
    iNumTypes = p_oPop->GetNumberOfTypes();

  m_iTotalNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data member
  mp_iYearsInfestedCodes = new short int * [m_iTotalNumSpecies];
  mp_iBBDResistanceStatusCodes = new short int * [m_iTotalNumSpecies];
  for ( i = 0; i < m_iTotalNumSpecies; i++ ) {
    mp_iYearsInfestedCodes[i] = new short int[iNumTypes];
    mp_iBBDResistanceStatusCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iYearsInfestedCodes[i][j] = -1;
      mp_iBBDResistanceStatusCodes[i][j] = -1;
    }
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ ) {
    //Make sure that only allowed types have been applied
    if (clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType) {
      modelErr stcErr;
      stcErr.sFunction = "clBBDInfestation::RegisterTreeDataMembers" ;
      stcErr.sMoreInfo = "This behavior cannot be applied to seedlings.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Register the code and capture it
    mp_iYearsInfestedCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                 [mp_whatSpeciesTypeCombos[i].iType] =
         p_oPop->RegisterInt("YearsInfested", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
    mp_iBBDResistanceStatusCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                 [mp_whatSpeciesTypeCombos[i].iType] =
         p_oPop->RegisterInt("BBDResistanceStatus", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}


////////////////////////////////////////////////////////////////////////////
// TreeInventory()
////////////////////////////////////////////////////////////////////////////
void clBBDInfestation::TreeInventory() {
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

  //Query for sapling and adult search - all species
  std::stringstream s;
  s << "type=" << clTreePopulation::sapling << "," << clTreePopulation::adult;
  clTreeSearch *p_oBehaviorTrees = p_oPop->Find( s.str() );
  clTree * p_oTree = p_oBehaviorTrees->NextTree();
  float fDbh, fBA;
  int iSp, iTp, iInf, iStatus;

  //Zero out counting variables
  m_fPlotBA = 0;
  m_fPoolBA = 0;
  m_iNumSmallCohortPoolTrees = 0;
  m_iNumLargeCohortPoolTrees = 0;
  m_iNumSmallCohortInfestedTrees = 0;
  m_iNumLargeCohortInfestedTrees = 0;
  m_iNumSmallInfestibleTrees = 0;
  m_iNumLargeInfestibleTrees = 0;

  //Go through all the trees
  while (p_oTree) {

    //Get the DBH
    iSp = p_oTree->GetSpecies();
    iTp = p_oTree->GetType();
    p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);

    //Get the basal area
    fBA = clModelMath::CalculateBasalArea(fDbh);

    //Add this to the all-plot BA total
    m_fPlotBA += fBA;

    //Is this one of our pool trees? Use the BBDResistanceStatus code to check
    if (mp_iBBDResistanceStatusCodes[iSp][iTp] > -1) {

      //To count in the pool stats, the tree must be above the minimum
      //infestible DBH
      if (fDbh >= mp_fMinDBH[iSp]) {

        //Add the BA to the pool BA total
        m_fPoolBA += fBA;

        //Count it into the total number of pool trees depending on whether
        //it's above or below the cohort cutoff
        if (fDbh < mp_fCohortDBH[iSp]) m_iNumSmallCohortPoolTrees++;
        else m_iNumLargeCohortPoolTrees++;
      }

      //Is this tree infested? If so, count it into the number of infested trees
      //in the appropriate cohort
      p_oTree->GetValue(mp_iYearsInfestedCodes[iSp][iTp], &iInf);
      if (iInf > 0) {
        if (fDbh < mp_fCohortDBH[iSp]) m_iNumSmallCohortInfestedTrees++;
        else m_iNumLargeCohortInfestedTrees++;
      }

      //Does this tree have a resistance status?
      p_oTree->GetValue(mp_iBBDResistanceStatusCodes[iSp][iTp], &iStatus);

      if (iStatus == 0) {

        //No status - assign one
        if (clModelMath::GetRand() <= mp_fProbResistant[iSp]) {
          iStatus = resistant;
        } else if (clModelMath::GetRand() <= mp_fProbConditionallySusceptible[iSp]) {
          iStatus = cond_susceptible;
        } else {
          iStatus = susceptible;
        }
        p_oTree->SetValue(mp_iBBDResistanceStatusCodes[iSp][iTp], iStatus);

      }

      //If it's susceptible, count it based on size
      if (fDbh >= mp_fMinDBH[iSp]) {
        if (iStatus > resistant) {
          if (fDbh < mp_fCohortDBH[iSp]) m_iNumSmallInfestibleTrees++;
          else m_iNumLargeInfestibleTrees++;

        }
      }
    }

    p_oTree = p_oBehaviorTrees->NextTree();
  }
}


////////////////////////////////////////////////////////////////////////////
// InfestInitialConditionsTrees()
////////////////////////////////////////////////////////////////////////////
void clBBDInfestation::InfestInitialConditionsTrees() {

  //Double check - if the first timestep isn't negative, this doesn't apply
  if (m_iFirstTimestep >= 0) return;

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTreeSearch *p_oBehaviorTrees = p_oPop->Find( m_cQuery );
  clTree * p_oTree = p_oBehaviorTrees->NextTree();
  float *p_fProb = new float[-m_iFirstTimestep]; //Cumulative probability of infestation
  float fB, fDbh,
        fTargetRate, fRand;
  int i, iSp, iTp, iInf, iStatus,
      iNumYrsTimestep = (int)mp_oSimManager->GetNumberOfYearsPerTimestep();

  //Do the tree inventory that allows us to calculate B
  TreeInventory();

  if (m_fPlotBA == 0) return;

  //Calculate the value of B
  fB = m_fBx * (m_fPoolBA / m_fPlotBA) + m_fBy;

  // Make a cumulative probability of infection in each year for assigning
  // time infected
  //------------------------------------------------------------
  m_iYearsOfInfestation = 0;
  for (i = 0; i < (-m_iFirstTimestep); i++) {

    //Calculate the proportion of trees infested
    p_fProb[i] = m_fMax * exp(-exp(m_fA - fB * m_iYearsOfInfestation));

    //Keep a running tally of number of years since first infestation
    m_iYearsOfInfestation += iNumYrsTimestep;
  }

  //Stash the current probability of infestation
  fTargetRate = p_fProb[(-m_iFirstTimestep) - 1];

  //Loop through again - relativize and make cumulative
  for (i = 0; i < (-m_iFirstTimestep); i++) {
    p_fProb[i] /= fTargetRate;
  }

  // Figure out how to split infection probability between cohorts
  //------------------------------------------------------------
  DetermineCohortInfestationProbability(fTargetRate);

  //Infect the trees and assign resistance status
  //------------------------------------------------------------
  while (p_oTree) {
    iSp = p_oTree->GetSpecies();
    iTp = p_oTree->GetType();

    if (-1 < mp_iYearsInfestedCodes[iSp][iTp]) {
      p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);

      //Make sure the tree is over the minimum infestible size
      if (fDbh >= mp_fMinDBH[iSp]) {

        //Check its resistance status
        p_oTree->GetValue(mp_iBBDResistanceStatusCodes[iSp][iTp], &iStatus);

        //Only continue if it's not resistant
        if (iStatus > resistant) {

          //Check for resistance based on the tree's cohort
          if (fDbh < mp_fCohortDBH[iSp]) {
            if (clModelMath::GetRand() <= m_fSmallCohortProb) {

              //Walk a random number out the cumulative probability array
              //until we pick the timestep infested
              fRand = clModelMath::GetRand();
              for (i = 0; i < -(m_iFirstTimestep); i++) {
                if (fRand < p_fProb[i]) break;
              }
              iInf = iNumYrsTimestep * i;
              p_oTree->SetValue(mp_iYearsInfestedCodes[iSp][iTp], iInf);
            }
          } else {
            if (clModelMath::GetRand() <= m_fLargeCohortProb) {

              //Walk a random number out the cumulative probability array
              //until we pick the timestep infested
              fRand = clModelMath::GetRand();
              for (i = 0; i < -(m_iFirstTimestep); i++) {
                if (fRand < p_fProb[i]) break;
              }
              iInf = iNumYrsTimestep * (-(m_iFirstTimestep) - i);
              p_oTree->SetValue(mp_iYearsInfestedCodes[iSp][iTp], iInf);
            }
          }
        }
      }

    }
    p_oTree = p_oBehaviorTrees->NextTree();
  }

  delete[] p_fProb;

}


/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clBBDInfestation::DetermineCohortInfestationProbability(double fTargetRate) {
  //Figure out how many total trees must be infested
  long iNumToInfest = fTargetRate * (m_iNumSmallCohortPoolTrees + m_iNumLargeCohortPoolTrees);

  if (m_iNumLargeCohortPoolTrees == 0 && m_iNumSmallCohortPoolTrees == 0) return;

  if (iNumToInfest <= (m_iNumSmallCohortInfestedTrees + m_iNumLargeCohortInfestedTrees)) {

    //We've already got enough infested trees
    m_fSmallCohortProb = 0;
    m_fLargeCohortProb = 0;
    return;

  }

  //Change to how many NEW trees to infest
  iNumToInfest -= (m_iNumSmallCohortInfestedTrees + m_iNumLargeCohortInfestedTrees);

  //Case: we don't have any trees of one cohort or the other
  if (m_iNumLargeCohortPoolTrees == 0) {
    m_fLargeCohortProb = 0;
    m_fSmallCohortProb = (double)iNumToInfest / (double)(m_iNumSmallInfestibleTrees - m_iNumSmallCohortInfestedTrees);
    return;
  }

  if (m_iNumSmallCohortPoolTrees == 0) {
    m_fSmallCohortProb = 0;
    m_fLargeCohortProb = (double)iNumToInfest / (double)(m_iNumLargeInfestibleTrees - m_iNumLargeCohortInfestedTrees);
    return;
  }

  //Do we have enough large cohort trees to cover everybody?
  if (iNumToInfest < (m_iNumLargeInfestibleTrees - m_iNumLargeCohortInfestedTrees)) {

    //Yep - don't infest any little ones
    m_fSmallCohortProb = 0;

    //Figure out how many of the large ones to infest
    m_fLargeCohortProb = (double)iNumToInfest / (double)(m_iNumLargeInfestibleTrees - m_iNumLargeCohortInfestedTrees);

  } else {

    //No - we don't have enough large trees. So infest them all, then
    //figure out how many little trees to infest
    m_fLargeCohortProb = 1;

    long iLargeToInfest = m_iNumLargeInfestibleTrees - m_iNumLargeCohortInfestedTrees;
    m_fSmallCohortProb = (double)(iNumToInfest - iLargeToInfest) / (double)(m_iNumSmallInfestibleTrees - m_iNumSmallCohortInfestedTrees);
  }


}
