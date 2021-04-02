//---------------------------------------------------------------------------
// AggregatedMortality.cpp
//---------------------------------------------------------------------------
#include "AggregatedMortality.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "ModelMath.h"
#include "TreePopulation.h"
#include "TreeSearch.h"
#include "Plot.h"
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clAggregatedMortality::clAggregatedMortality( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clMortalityBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "aggregatedmortshell";
    m_sXMLRoot = "AggregatedMortality";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_iDeadCodes = NULL;
    mp_bAppliesTo = NULL;

    m_fMortalityProb = 0;
    m_fClumpingParameter = 0;
    m_iTotalNumSpecies = 0;
    m_fMortalityEpisodeProbability = 0;
    m_iNumTreesToClump = 0;
    m_bClumpSizeDeterministic = true;

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
    stcErr.sFunction = "clAggregatedMortality::clAggregatedMortality" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clAggregatedMortality::~clAggregatedMortality()
{
  int i;
  if (mp_iDeadCodes) {
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_iDeadCodes[i];
  }
  if (mp_bAppliesTo) {
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_bAppliesTo[i];
  }
  delete[] mp_iDeadCodes;
  delete[] mp_bAppliesTo;
}

/////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////

void clAggregatedMortality::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation *p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
    m_iTotalNumSpecies = p_oPop->GetNumberOfSpecies();


    ReadParFile( p_oDoc );
    GetDeadCodes( p_oPop );
    SetUpAppliesTo( p_oPop );

    //Calculate probability of a mortality episode
    int iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    m_fMortalityEpisodeProbability = iNumYearsPerTimestep / m_fMortalityEpisodeProbability;

    //Calculate the per-timestep amount to kill
    m_fMortalityProb = 1 - pow(1 - m_fMortalityProb, iNumYearsPerTimestep);
    m_fMortalityProb /= m_iNumTreesToClump;

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
    stcErr.sFunction = "clAggregatedMortality::DoShellSetup" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// ReadParFile()
/////////////////////////////////////////////////////////////////////////////
void clAggregatedMortality::ReadParFile( xercesc::DOMDocument * p_oDoc )
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

  //Mortality episode return interval in years
  FillSingleValue( p_oElement, "mo_aggReturnInterval", &m_fMortalityEpisodeProbability, true );
  if (m_fMortalityEpisodeProbability < 0) {
    modelErr stcErr;
    stcErr.sFunction = "clAggregatedMortality::ReadParFile" ;
    stcErr.sMoreInfo = "Return interval cannot be negative.";
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }

  //Mortality probability per year of a mortality episode
  //Make sure each value is between 0 and 1
  FillSingleValue( p_oElement, "mo_aggPropTreesToKill", &m_fMortalityProb, true );
  if (m_fMortalityProb < 0 || m_fMortalityProb > 1) {
    modelErr stcErr;
    stcErr.sFunction = "clAggregatedMortality::ReadParFile" ;
    stcErr.sMoreInfo = "Mortality probability must be between 0 and 1.";
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }

  //Number of trees to aggregate
  FillSingleValue( p_oElement, "mo_aggNumTreesToClump", &m_iNumTreesToClump, true );
  //Make sure each value is greater than 0
  if (m_iNumTreesToClump < 1) {
    modelErr stcErr;
    stcErr.sFunction = "clAggregatedMortality::ReadParFile" ;
    stcErr.sMoreInfo = "Number of trees to aggregate must be greater than 0.";
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }

  //Whether clump size is deterministic (true) or from the
  //negative binomial probability distribution (false)
  FillSingleValue( p_oElement, "mo_aggClumpSizeDeterministic", &m_bClumpSizeDeterministic, true );

  //Clumping parameter for negative binomial distribution, if required
  if (!m_bClumpSizeDeterministic)
    FillSingleValue( p_oElement, "mo_aggClumpingParameter", &m_fClumpingParameter, true );
}

/////////////////////////////////////////////////////////////////////////////
// PreMortCalcs()
/////////////////////////////////////////////////////////////////////////////
void clAggregatedMortality::PreMortCalcs( clTreePopulation *p_oPop )
{
  //Decide if a mortality episode occurs
  if (clModelMath::GetRand() > m_fMortalityEpisodeProbability) return;

  clTreeSearch *p_oTrees;
  clTree *p_oTree;
  int iClumpSize; //size of a clump

  //Go through the trees and find each one to this behavior applies
  p_oTrees = p_oPop->Find("all");
  p_oTree = p_oTrees->NextTree();

  //How big is our clump?
  if (m_bClumpSizeDeterministic) iClumpSize = m_iNumTreesToClump;

  while (p_oTree) {
    if (IsValid(p_oTree) && clModelMath::GetRand() < m_fMortalityProb) {

      //Start a clump
      if (!m_bClumpSizeDeterministic) {
        //Do a random draw to find out the size of our clump
        iClumpSize = clModelMath::NegBinomialRandomDraw ((float) m_iNumTreesToClump, m_fClumpingParameter);
      }

      //Kill a clump
      KillClump(p_oPop, p_oTree, iClumpSize);
    }
    p_oTree = p_oTrees->NextTree();
  }
}

////////////////////////////////////////////////////////////////////////////
// KillClump()
////////////////////////////////////////////////////////////////////////////
void clAggregatedMortality::KillClump(clTreePopulation *p_oPop, clTree *p_oTree, int iNumToKill) {
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  clTreeSearch *p_oTrees;
  clTree **p_oClumpTrees; //list to keep track of trees to kill
  std::stringstream sQuery;
  float *p_fDistances;
  float fTargetX, fTargetY, //coordinates of the first tree in a clump
        fX, fY, //coordinates of other trees
        fDistance = 0, fDistanceInc = 5, fHeight = 0;
  int iNumTrees = 0, iSp, iTp, iTemp, j,
      iDead = natural;

  //Get the coordinates of the first clump tree
  iSp = p_oTree->GetSpecies();
  iTp = p_oTree->GetType();
  p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fTargetX);
  p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fTargetY);

  //Kill the tree
  p_oTree->SetValue(mp_iDeadCodes[iSp][iTp], iDead);

  //Set up a search - widen the search until we've got enough trees to
  //make a clump
  do {
    iNumTrees = 0;
    fDistance += fDistanceInc;
    sQuery << "distance=" << fDistance << "FROM x=" << fTargetX
           << "y=" << fTargetY << "::height=" << fHeight;
    p_oTrees = p_oPop->Find(sQuery.str());
    sQuery.str("");
    p_oTree = p_oTrees->NextTree();
    while (p_oTree) {
      if (IsValid(p_oTree)) iNumTrees++;
      p_oTree = p_oTrees->NextTree();
    }
  } while (iNumTrees < iNumToKill);

  //Now p_oTrees contains all the trees we need to kill for our clump.
  //We need to sort them into distance order

  //Create an array to hold tree pointers
  p_oClumpTrees = new clTree*[iNumTrees];

  //Create an array for the distance to the ith tree
  p_fDistances = new float[iNumTrees];

  //Put the trees into the array
  p_oTrees->StartOver();

  //Find the first tree that counts
  p_oTree = p_oTrees->NextTree();
  while (p_oTree) {
    if (IsValid(p_oTree)) break;
    p_oTree = p_oTrees->NextTree();
  }
  p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
  p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);
  fDistance = p_oPlot->GetDistance(fTargetX, fTargetY, fX, fY);
  p_oClumpTrees[0] = p_oTree;
  p_fDistances[0] = fDistance;
  iTemp = 0;
  p_oTree = p_oTrees->NextTree();

  //Put the other trees in order - iTemp holds the index of the last tree
  while (p_oTree) {
    if (IsValid(p_oTree)) {
      p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
      p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);
      fDistance = p_oPlot->GetDistance(fTargetX, fTargetY, fX, fY);
      if (fDistance >= p_fDistances[iTemp]) {
        //This tree goes at the end
        iTemp++;
        p_oClumpTrees[iTemp] = p_oTree;
        p_fDistances[iTemp] = fDistance;
      } else {
        //Figure out where to insert it - move the elements back to open
        //a space
        iTemp++;
        for (j = iTemp; (j > 0) && (p_fDistances[j - 1] > fDistance); j--) {
          p_fDistances[j] = p_fDistances[j-1];
          p_oClumpTrees[j] = p_oClumpTrees[j-1];
        }
        p_fDistances[j] = fDistance;
        p_oClumpTrees[j] = p_oTree;
      }
    }
    p_oTree = p_oTrees->NextTree();
  }

  //Kill the trees in the clump
  iNumToKill--; //we already killed one tree
  for (j = 0; j < iNumToKill; j++)
    p_oClumpTrees[j]->SetValue(mp_iDeadCodes[p_oClumpTrees[j]->GetSpecies()]
                                            [p_oClumpTrees[j]->GetType()], iDead);

  delete[] p_oClumpTrees;
  delete[] p_fDistances;
}

////////////////////////////////////////////////////////////////////////////
// IsValid()
////////////////////////////////////////////////////////////////////////////
bool clAggregatedMortality::IsValid(clTree *p_oTree) {
  int iSp = p_oTree->GetSpecies(),
      iTp = p_oTree->GetType(),
      iIsDead;
  if (mp_bAppliesTo[iSp][iTp]) {
    p_oTree->GetValue(mp_iDeadCodes[iSp][iTp], &iIsDead);
    if (notdead == iIsDead) return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////
// GetDeadCodes()
////////////////////////////////////////////////////////////////////////////
void clAggregatedMortality::GetDeadCodes(clTreePopulation *p_oPop)
{
  int i, j,
      iNumTypes = p_oPop->GetNumberOfTypes();

  mp_iDeadCodes = new int * [m_iTotalNumSpecies];
  for ( i = 0; i < m_iTotalNumSpecies; i++ )
  {
    mp_iDeadCodes[i] = new int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iDeadCodes[i][j] = -1;
    }
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {

    //Get the code
    mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                 [mp_whatSpeciesTypeCombos[i].iType] =
         p_oPop->GetIntDataCode( "dead", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
    if (-1 == mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                           [mp_whatSpeciesTypeCombos[i].iType]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clAggregatedMortality::GetDeadCodes" ;
      stcErr.sMoreInfo = "Panic!  Can't get dead data member.";
      throw( stcErr );
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// SetUpAppliesTo()
////////////////////////////////////////////////////////////////////////////
void clAggregatedMortality::SetUpAppliesTo(clTreePopulation *p_oPop)
{
  int i, j,
      iNumTypes = p_oPop->GetNumberOfTypes();

  mp_bAppliesTo = new bool * [m_iTotalNumSpecies];
  for ( i = 0; i < m_iTotalNumSpecies; i++ )
  {
    mp_bAppliesTo[i] = new bool[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_bAppliesTo[i][j] = false;
    }
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {

    mp_bAppliesTo[mp_whatSpeciesTypeCombos[i].iSpecies]
                 [mp_whatSpeciesTypeCombos[i].iType] = true;
  }
}

////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clAggregatedMortality::DoMort (clTree *p_oTree, const float &fDbh, const short int &iSpecies)
{
  int iIsDead;
  p_oTree->GetValue(mp_iDeadCodes[p_oTree->GetSpecies()][p_oTree->GetType()], &iIsDead);
  return (deadCode)iIsDead;
}

