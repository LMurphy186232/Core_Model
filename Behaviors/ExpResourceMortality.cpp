//---------------------------------------------------------------------------
// ExpResourceMortality.cpp
//---------------------------------------------------------------------------
#include "ExpResourceMortality.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "Grid.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clExpResourceMortality::clExpResourceMortality( clSimManager * p_oSimManager ) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clMortalityBase( p_oSimManager )
{
  try
  {

    m_sNameString = "expresourcemortshell";
    m_sXMLRoot = "ExponentialGrowthResourceMortality";

    mp_fA = NULL;
    mp_fB = NULL;
    mp_iIndexes = NULL;
    mp_fC = NULL;
    mp_fD = NULL;
    mp_iGrowthCodes = NULL;
    mp_iXCodes = NULL;
    mp_iYCodes = NULL;

    m_fNumberYearsPerTimestep = 0;
    mp_oResourceGrid = NULL;
    m_iResourceCode = -1;
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
    stcErr.sFunction = "clExpResourceMortality::clExpResourceMortality" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clExpResourceMortality::~clExpResourceMortality()
{
  int i; //loop counter

  if ( mp_iGrowthCodes )
  {
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iGrowthCodes[i];
    }
  }
  if ( mp_iXCodes )
  {
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iXCodes[i];
    }
  }
  if ( mp_iYCodes )
  {
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iYCodes[i];
    }
  }

  delete[] mp_iGrowthCodes;
  delete[] mp_iXCodes;
  delete[] mp_iYCodes;
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_iIndexes;
  delete[] mp_fC;
  delete[] mp_fD;
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clExpResourceMortality::ReadParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  floatVal * p_fTempValues; //for getting species-specific values
  short int i; //loop counter

  //Declare the temp array and populate it with the species to which this
  //behavior applies
  p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Declare the index array and populate it
  mp_iIndexes = new short int[p_oPop->GetNumberOfSpecies()];

  //Make the list of indexes
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_iIndexes[mp_iWhatSpecies[i]] = i;

  //Declare the arrays for holding the variables
  mp_fA = new float[m_iNumBehaviorSpecies];
  mp_fB = new float[m_iNumBehaviorSpecies];
  mp_fC = new float[m_iNumBehaviorSpecies];
  mp_fD = new float[m_iNumBehaviorSpecies];

  //Capture the values from the parameter file

  //Exponential resource mortality - a
  FillSpeciesSpecificValue( p_oElement, "mo_expResMortA", "mo_ermaVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fA[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

  //Exponential resource mortality - b
  FillSpeciesSpecificValue( p_oElement, "mo_expResMortB", "mo_ermbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

  //Exponential resource mortality - c
  FillSpeciesSpecificValue( p_oElement, "mo_expResMortC", "mo_ermcVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fC[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

  //Exponential resource mortality - d
  FillSpeciesSpecificValue( p_oElement, "mo_expResMortD", "mo_ermdVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fD[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

  delete[] p_fTempValues; p_fTempValues = NULL;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clExpResourceMortality::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    ReadParameterFileData( p_oDoc );
    GetTreeDataMemberCodes();
    GetResourceGrid();

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
    stcErr.sFunction = "clExpResourceMortality::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetResourceGrid()
/////////////////////////////////////////////////////////////////////////////
void clExpResourceMortality::GetResourceGrid()
{

  //Get the "Resource" grid
  mp_oResourceGrid = mp_oSimManager->GetGridObject( "Resource" );
  if ( NULL == mp_oResourceGrid )
  {
    modelErr stcErr;
    stcErr.sFunction = "clExpResourceMortality::GetResourceGrid" ;
    stcErr.sMoreInfo = "Can't find required grid object \"Resources\".";
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    throw( stcErr );
  }

  m_iResourceCode = mp_oResourceGrid->GetFloatDataCode( "Resource" );
  if ( -1 == m_iResourceCode )
  {
    modelErr stcErr;
    stcErr.sFunction = "clExpResourceMortality::GetResourceGrid" ;
    stcErr.sMoreInfo = "Grid object \"Resources\" is set up incorrectly.";
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetTreeDataMemberCodes()
//////////////////////////////////////////////////////////////////////////////
void clExpResourceMortality::GetTreeDataMemberCodes()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    char cLabel[] = "Growth";
    short int i, j, //loop counters
    iTotalTypes = p_oPop->GetNumberOfTypes(); //number of tree types

    //Declare and initialize the return codes arrays
    mp_iGrowthCodes = new short int * [m_iNumBehaviorSpecies];
    mp_iXCodes = new short int * [m_iNumBehaviorSpecies];
    mp_iYCodes = new short int * [m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_iGrowthCodes[i] = new short int[iTotalTypes];
      mp_iXCodes[i] = new short int[iTotalTypes];
      mp_iYCodes[i] = new short int[iTotalTypes];
      for ( j = 0; j < iTotalTypes; j++ ) {
        mp_iGrowthCodes[i] [j] = -1;
        mp_iXCodes[i] [j] = -1;
        mp_iYCodes[i] [j] = -1;
      }
    }

    //Now go through the growth functions table and get the code for
    //each species/type combo with a valid pointer
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      for ( j = 0; j < iTotalTypes; j++ )
        if ( mp_bUsesThisMortality[i] [j] )
        {
          mp_iGrowthCodes[mp_iIndexes[i]] [j] = p_oPop->GetFloatDataCode( cLabel, i, j );
          mp_iXCodes[mp_iIndexes[i]][j] = p_oPop->GetXCode(i, j);
          mp_iYCodes[mp_iIndexes[i]][j] = p_oPop->GetYCode(i, j);

          //If the return code is -1, throw an error
          if ( -1 == mp_iGrowthCodes[mp_iIndexes[i]] [j] )
          {
            modelErr stcErr;
            stcErr.sFunction = "clExpResourceMortality::GetTreeDataMemberCodes" ;
            std::stringstream s;
            s << "Type/species combo species=" << i << " type=" << j
                << " does not have a growth behavior compatible with its mortality behavior.";
            stcErr.sMoreInfo = s.str();
            stcErr.iErrorCode = BAD_DATA;
            throw( stcErr );
          }
        }

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
    stcErr.sFunction = "clExpResourceMortality::GetTreeDataMemberCodes" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clExpResourceMortality::DoMort(clTree * p_oTree, const float & fDbh, const short int & iSpecies )
{
  float fRandom = clModelMath::GetRand(), //random number
      fGrowth, //growth value
      fResource, //resource amount
      fX, fY, //tree coordinates
      fMortProb; //probability of mortality for this tree
  int iType = p_oTree->GetType();

  //Get the proper resource level from the tree's coordinates
  p_oTree->GetValue( mp_iXCodes[mp_iIndexes[iSpecies]][iType], & fX );
  p_oTree->GetValue( mp_iYCodes[mp_iIndexes[iSpecies]][iType], & fY );
  mp_oResourceGrid->GetValueAtPoint( fX, fY, m_iResourceCode, & fResource );

  p_oTree->GetValue( mp_iGrowthCodes[mp_iIndexes[iSpecies]] [iType], & fGrowth );

  fMortProb = (mp_fD[mp_iIndexes[iSpecies]] + (mp_fA[mp_iIndexes[iSpecies]] * fResource)) *
      exp(-(mp_fB[mp_iIndexes[iSpecies]] +
          (mp_fC[mp_iIndexes[iSpecies]] * fResource)) * fGrowth);

  //Compound over the number of years per timestep
  fMortProb = 1 - pow(1 - fMortProb, m_fNumberYearsPerTimestep);

  //If the probability of death is greater than the random number, kill the
  //tree
  if ( fRandom > fMortProb )
    return notdead;
  else
    return natural;
}
