//---------------------------------------------------------------------------
// TreeAgeCalculator.cpp
//---------------------------------------------------------------------------
#include "TreeAgeCalculator.h"
#include "MortalityOrg.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ModelMath.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clTreeAgeCalculator::clTreeAgeCalculator(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try
  {

    m_sNameString = "TreeAgeCalculator";
    m_sXMLRoot = "TreeAgeCalculator";

    mp_iAgeCodes = NULL;
    m_cQuery = NULL;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Indicate that this behavior intends to add one tree int data
    //member
    m_iNewTreeInts = 1;
    m_iNumSpecies = 0;

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
    stcErr.sFunction = "clTreeAgeCalculator::clTreeAgeCalculator" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clTreeAgeCalculator::~clTreeAgeCalculator() {
  if (mp_iAgeCodes) {
    for (int i = 0; i < m_iNumSpecies; i++) {
      delete[] mp_iAgeCodes[i];
    }
    delete[] mp_iAgeCodes;
  }

  delete[] m_cQuery;
}

////////////////////////////////////////////////////////////////////////////
// GetData()
////////////////////////////////////////////////////////////////////////////
void clTreeAgeCalculator::GetData(xercesc::DOMDocument * p_oDoc) {
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch *p_oAllTrees;
    clTree *p_oTree;
    char * cQueryTemp = new char[( p_oPop->GetNumberOfSpecies() * 4 ) + 50];
    char cQueryPiece[5]; //for assembling the search query
    int i, iInitialAge = 10000, iAge, iSp, iTp;
    bool bSapling = false, bAdult = false, bSeedling = false;

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
      else if ( clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType )
      {
        bSeedling = true;
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
    if ( bSeedling )
    {
      sprintf( cQueryPiece, "%d%s", clTreePopulation::seedling, "," );
      strcat( cQueryTemp, cQueryPiece );
    }

    //Remove the last comma
    cQueryTemp[strlen( cQueryTemp ) - 1] = '\0';

    //Now put it in m_cQuery, sized correctly
    m_cQuery = new char[strlen( cQueryTemp ) + 1];
    strcpy( m_cQuery, cQueryTemp );
    delete[] cQueryTemp;

    //Now set all initial conditions trees to an age of 10000
    p_oAllTrees = p_oPop->Find("all");
    p_oTree = p_oAllTrees->NextTree();
    while (p_oTree) {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the age data member
      if ( -1 != mp_iAgeCodes[iSp][iTp] )
      {
        //Get this tree's existing age, if any
        p_oTree->GetValue( mp_iAgeCodes[iSp][iTp], & iAge );

        if (0 == iAge) {
          p_oTree->SetValue( mp_iAgeCodes[iSp][iTp], iInitialAge );
        }
      }
      p_oTree = p_oAllTrees->NextTree();
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
    stcErr.sFunction = "clTreeAgeCalculator::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clTreeAgeCalculator::RegisterTreeDataMembers() {

  clTreePopulation
      * p_oPop =
          ( clTreePopulation * ) mp_oSimManager->GetPopulationObject("treepopulation");
  short int i, j, iNumTypes = p_oPop->GetNumberOfTypes();

  m_iNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data member
  mp_iAgeCodes = new short int * [m_iNumSpecies];
  for (i = 0; i < m_iNumSpecies; i++) {
    mp_iAgeCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iAgeCodes[i][j] = -1;
    }
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    //Make sure that only allowed types have been applied
    if (clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType
        && clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
        && clTreePopulation::seedling != mp_whatSpeciesTypeCombos[i].iType) {

      modelErr stcErr;
      stcErr.sFunction = "clTreeAgeCalculator::RegisterTreeDataMembers";
      stcErr.sMoreInfo = "This behavior can only be applied to seedlings, saplings, and adults.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Register the code and capture it
    mp_iAgeCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
        = p_oPop->RegisterInt("Tree Age", mp_whatSpeciesTypeCombos[i].iSpecies,
            mp_whatSpeciesTypeCombos[i].iType);
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clTreeAgeCalculator::Action() {
  clTreePopulation
      * p_oPop =
          ( clTreePopulation * ) mp_oSimManager->GetPopulationObject("treepopulation");
  clTreeSearch * p_oBehaviorTrees = p_oPop->Find(m_cQuery);
  clTree * p_oTree;
  int iNumYearsPerTimestep =
      ( int )mp_oSimManager->GetNumberOfYearsPerTimestep(), iSp, iTp, iAge;

  try
  {

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the age data member
      if ( -1 != mp_iAgeCodes[iSp][iTp] )
      {

        //Get this tree's existing age
        p_oTree->GetValue( mp_iAgeCodes[iSp][iTp], & iAge );

        //Increment and reassign
        iAge += iNumYearsPerTimestep;
        p_oTree->SetValue( mp_iAgeCodes[iSp][iTp], iAge );

      }

      p_oTree = p_oBehaviorTrees->NextTree();
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
    stcErr.sFunction = "clTreeAgeCalculator::Action" ;
    throw( stcErr );
  }
}
