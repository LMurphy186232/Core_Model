//---------------------------------------------------------------------------
// StormDirectKiller.cpp
//---------------------------------------------------------------------------
#include "StormDirectKiller.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "ModelMath.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clStormDirectKiller::clStormDirectKiller( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "StormDirectKiller";
    m_sXMLRoot = "StormDirectKiller";

    m_cQuery = NULL;
    mp_fA = NULL;
    mp_fB = NULL;
    mp_iDeadCodes = NULL;
    mp_oStormDamageGrid = NULL;
    m_iNumTypes = 0;
    m_iDmgIndexCode = -1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1.0;
    m_fMinimumVersionNumber = 1.0;

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
    stcErr.sFunction = "clStormDirectKiller::clStormDirectKiller" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clStormDirectKiller::~clStormDirectKiller()
{
  delete[] m_cQuery;
  delete[] mp_fA;
  delete[] mp_fB;
  if ( mp_iDeadCodes )
  {
    for ( int i = 0; i < m_iNumTypes; i++ )
      delete[] mp_iDeadCodes[i];
  }
  delete[] mp_iDeadCodes;
}

////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clStormDirectKiller::GetParameterFileData( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  doubleVal * p_fTempValues; //for getting species-specific values
  int i, iNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare our arrays
  mp_fA = new double[iNumSpecies];
  mp_fB = new double[iNumSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //"a"
  FillSpeciesSpecificValue( p_oElement, "st_stormDirectKillerA", "st_sdkaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;

  //"b"
  FillSpeciesSpecificValue( p_oElement, "st_stormDirectKillerB", "st_sdkbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clStormDirectKiller::FormatQueryString()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  char * cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];
  char cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSeedling = false, bSapling = false, bAdult = false, bSnag = false;

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
    if ( clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSeedling = true;
    }
    else if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSapling = true;
    }
    else if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType )
    {
      bAdult = true;
    }
    else if ( clTreePopulation::snag == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSnag = true;
    }
  }
  strcat( cQueryTemp, "::type=" );
  if ( bSeedling )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::seedling, "," );
    strcat( cQueryTemp, cQueryPiece );
  }
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
  if ( bSnag )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::snag, "," );
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
// GetData()
////////////////////////////////////////////////////////////////////////////
void clStormDirectKiller::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

    GetParameterFileData( p_oDoc, p_oPop );
    FormatQueryString();
    GetDeadCodes(p_oPop);
    GetGridInfo();
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
    stcErr.sFunction = "clStormDirectKiller::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clStormDirectKiller::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clPackage *p_oPkg;
    clTree * p_oTree;
    float fStormSeverity, //storm severity in a tree's grid cell
         fX, fY, //tree's coordinates
         fMortProb; //probability of mortality
    int iSp, iTp;

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by seeing if we have an
      //appropriate value for its dead code
      if ( -1 == mp_iDeadCodes[iTp] [iSp] )
        goto nextTree;

      //Get the storm severity for this tree's location
      p_oTree->GetValue( p_oPop->GetXCode( iSp, iTp ), & fX );
      p_oTree->GetValue( p_oPop->GetYCode( iSp, iTp ), & fY );
      p_oPkg = mp_oStormDamageGrid->GetFirstPackageAtPoint(fX, fY);

      //Go through the storms and allow each a chance to kill this tree
      while (p_oPkg)
      {
        p_oPkg->GetValue( m_iDmgIndexCode, & fStormSeverity );
        if (fStormSeverity > 0) {

          //Get the probability of mortality
          fMortProb = exp( mp_fA[iSp] + ( mp_fB[iSp] * fStormSeverity ) )
             / ( 1 + exp(  mp_fA[iSp] + ( mp_fB[iSp] * fStormSeverity ) ) );

          //Get a random number
          if ( clModelMath::GetRand() <= fMortProb ) {
            p_oTree->SetValue( p_oPop->GetIntDataCode( "dead", iSp, iTp ), storm );
            goto nextTree;

          }
        }
        p_oPkg = p_oPkg->GetNextPackage();
      }


    nextTree:
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
    stcErr.sFunction = "clStormDirectKiller::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetDeadCodes()
////////////////////////////////////////////////////////////////////////////
void clStormDirectKiller::GetDeadCodes(clTreePopulation *p_oPop)
{
  int i, j, iNumSpecies = p_oPop->GetNumberOfSpecies();
  m_iNumTypes = p_oPop->GetNumberOfTypes();

  mp_iDeadCodes = new int * [m_iNumTypes];
  for ( i = 0; i < m_iNumTypes; i++ )
  {
    mp_iDeadCodes[i] = new int[iNumSpecies];
    for (j = 0; j < iNumSpecies; j++) {
      mp_iDeadCodes[i][j] = -1;
    }
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Get the code
    mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iType] [mp_whatSpeciesTypeCombos[i].iSpecies] =
         p_oPop->GetIntDataCode( "dead", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
    if (-1 == mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iType] [mp_whatSpeciesTypeCombos[i].iSpecies]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clStormDirectKiller::GetDeadCodes" ;
      stcErr.sMoreInfo = "You must use tree mortality behaviors along with the storm killer.";
      throw( stcErr );
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// GetGridInfo()
////////////////////////////////////////////////////////////////////////////
void clStormDirectKiller::GetGridInfo()
{
  //Get the pointer to the storm damage grid
  mp_oStormDamageGrid = mp_oSimManager->GetGridObject( "Storm Damage" );
  if ( NULL == mp_oStormDamageGrid )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clStormDirectKiller::GetGridInfo" ;
    stcErr.sMoreInfo = "The storm behavior must be used with the storm behavior.";
    throw( stcErr );
  }

  m_iDmgIndexCode = mp_oStormDamageGrid->GetPackageFloatDataCode( "1dmg_index" );
  if ( -1 == m_iDmgIndexCode )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clStormDirectKiller::GetGridInfo" ;
    stcErr.sMoreInfo = "The grid \"Storm Damage\" is set up in an unexpected way.";
    throw( stcErr );
  }
}

