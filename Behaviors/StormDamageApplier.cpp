//---------------------------------------------------------------------------
// StormDamageApplier.cpp
//---------------------------------------------------------------------------
#include "StormDamageApplier.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "ModelMath.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clStormDamageApplier::clStormDamageApplier( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "StormDamageApplier";
    m_sXMLRoot = "StormDamageApplier";

    m_cQuery = NULL;
    mp_fMinStormDBH = NULL;
    mp_fStmDmgInterceptMed = NULL;
    mp_fStmDmgInterceptFull = NULL;
    mp_fStmIntensityCoeff = NULL;
    mp_fStmDBHCoeff = NULL;
    mp_iStmDmgCodes = NULL;
    mp_oStormDamageGrid = NULL;
    m_iNumYearsToHeal = 0;
    m_iNumTypes = 0;
    m_iDmgIndexCode = -1;

    //Set that we want to add one int value
    m_iNewTreeInts = 1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1.1;
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
    stcErr.sFunction = "clStormDamageApplier::clStormDamageApplier" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clStormDamageApplier::~clStormDamageApplier()
{
  delete[] m_cQuery;
  delete[] mp_fMinStormDBH;
  delete[] mp_fStmDmgInterceptMed;
  delete[] mp_fStmDmgInterceptFull;
  delete[] mp_fStmIntensityCoeff;
  delete[] mp_fStmDBHCoeff;
  if ( mp_iStmDmgCodes )
  {
    for ( int i = 0; i < m_iNumTypes; i++ )
      delete[] mp_iStmDmgCodes[i];
  }
  delete[] mp_iStmDmgCodes;
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clStormDamageApplier::RegisterTreeDataMembers()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  int i, j, iNumSpecies = p_oPop->GetNumberOfSpecies();
  m_iNumTypes = p_oPop->GetNumberOfTypes();

  mp_iStmDmgCodes = new int * [m_iNumTypes];
  for ( i = 0; i < m_iNumTypes; i++ )
  {
    mp_iStmDmgCodes[i] = new int[iNumSpecies];
    for (j = 0; j < iNumSpecies; j++) {
      mp_iStmDmgCodes[i][j] = -1;
    }
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    if ( clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clStormDamageApplier::RegisterTreeDataMembers" ;
      stcErr.sMoreInfo = "The storm damage applier behavior cannot be applied to seedlings.";
      throw( stcErr );
    }

    //Register the code and capture it
    mp_iStmDmgCodes[mp_whatSpeciesTypeCombos[i].iType] [mp_whatSpeciesTypeCombos[i].iSpecies] =
         p_oPop->RegisterInt( "stm_dmg", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clStormDamageApplier::GetParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  doubleVal * p_fTempValues; //for getting species-specific values
  int i, iNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare our arrays
  mp_fMinStormDBH = new double[iNumSpecies];
  mp_fStmDmgInterceptMed = new double[iNumSpecies];
  mp_fStmDmgInterceptFull = new double[iNumSpecies];
  mp_fStmIntensityCoeff = new double[iNumSpecies];
  mp_fStmDBHCoeff = new double[iNumSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Minimum DBH for storm damage
  FillSpeciesSpecificValue( p_oElement, "st_minDBH", "st_mdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fMinStormDBH[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Damage intercept for medium damage
  FillSpeciesSpecificValue( p_oElement, "st_stmDmgInterceptMed", "st_sdimVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fStmDmgInterceptMed[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Damage intercept for full damage
  FillSpeciesSpecificValue( p_oElement, "st_stmDmgInterceptFull", "st_sdifVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fStmDmgInterceptFull[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Storm intensity coefficient
  FillSpeciesSpecificValue( p_oElement, "st_stmIntensityCoeff", "st_sicVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fStmIntensityCoeff[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Storm DBH coefficient
  FillSpeciesSpecificValue( p_oElement, "st_stmDBHCoeff", "st_sdcVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fStmDBHCoeff[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Number of years damaged trees take to heal
  FillSingleValue( p_oElement, "st_numYearsToHeal", & m_iNumYearsToHeal, true );
  if ( m_iNumYearsToHeal <= 0 )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clStormDamageApplier::GetParameterFileData" ;
    stcErr.sMoreInfo = "The number of years to heal parameter must be greater than 0.";
    delete[] p_fTempValues;
    throw( stcErr );
  }

  delete[] p_fTempValues;
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clStormDamageApplier::FormatQueryString()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
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
// GetData()
////////////////////////////////////////////////////////////////////////////
void clStormDamageApplier::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {

    //Read in the parameter file data
    GetParameterFileData( p_oDoc );

    //Format our query string
    FormatQueryString();

    //Get the pointer to the storm damage grid
    mp_oStormDamageGrid = mp_oSimManager->GetGridObject( "Storm Damage" );
    if ( NULL == mp_oStormDamageGrid )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clStormDamageApplier::GetData" ;
      stcErr.sMoreInfo = "The storm damage applier behavior must be used with the storm behavior.";
      throw( stcErr );
    }

    m_iDmgIndexCode = mp_oStormDamageGrid->GetPackageFloatDataCode( "1dmg_index" );
    if ( -1 == m_iDmgIndexCode )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clStormDamageApplier::GetData" ;
      stcErr.sMoreInfo = "The grid \"Storm Damage\" is set up in an unexpected way.";
      throw( stcErr );
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
    stcErr.sFunction = "clStormDamageApplier::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clStormDamageApplier::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clPackage *p_oPkg;
    clTree * p_oTree;
    float fStormSeverity, //storm severity in a tree's grid cell
         fX, fY, //tree's coordinates
         fDbh, //tree's dbh
         fRandom, //random number
         fMediumDmgProb, //probability of medium tree damage
         fFullDmgProb; //probability of full tree damage
    int iTreeDamage, iYearsToHeal, iThisDamage,
        iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    int iSp, iTp;

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the stm_dmg data member
      if ( -1 == mp_iStmDmgCodes[iTp] [iSp] )
        goto nextTree;

      //If this tree is too small, skip out
      p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDbh );
      if ( fDbh < mp_fMinStormDBH[iSp] )
        goto nextTree;

      //Get the tree's existing damage
      p_oTree->GetValue( mp_iStmDmgCodes[iTp] [iSp], & iTreeDamage );

      //Get the storm severity for this tree's location
      p_oTree->GetValue( p_oPop->GetXCode( iSp, iTp ), & fX );
      p_oTree->GetValue( p_oPop->GetYCode( iSp, iTp ), & fY );
      p_oPkg = mp_oStormDamageGrid->GetFirstPackageAtPoint(fX, fY);

      //Were there any storms this timestep?
      if ( NULL != p_oPkg)
      {
        //Go through the storms and determine the most severe damage
        //that this tree will get - 0, 1, or 2
        iThisDamage = 0;

        while (p_oPkg && iThisDamage < 2) {
          p_oPkg->GetValue( m_iDmgIndexCode, & fStormSeverity );
          if (fStormSeverity > 0) {

            //Get the probability of medium damage
            fMediumDmgProb = exp( mp_fStmDmgInterceptMed[iSp]
             + ( mp_fStmIntensityCoeff[iSp] * fStormSeverity * pow( fDbh, mp_fStmDBHCoeff[iSp] ) ) )
             / ( 1 + exp( mp_fStmDmgInterceptMed[iSp]
             + ( mp_fStmIntensityCoeff[iSp] * fStormSeverity * pow( fDbh, mp_fStmDBHCoeff[iSp] ) ) ) );

            //Get a random number
            fRandom = clModelMath::GetRand();

            if ( fRandom > fMediumDmgProb ) {
              //Damage is medium or heavy
              fFullDmgProb = exp( mp_fStmDmgInterceptFull[iSp]
               + ( mp_fStmIntensityCoeff[iSp] * fStormSeverity * pow( fDbh, mp_fStmDBHCoeff[iSp] ) ) )
               / ( 1 + exp( mp_fStmDmgInterceptFull[iSp]
               + ( mp_fStmIntensityCoeff[iSp] * fStormSeverity * pow( fDbh, mp_fStmDBHCoeff[iSp] ) ) ) );

              if ( fRandom <= fFullDmgProb )
                iThisDamage = 1;
              else
                iThisDamage = 2;

            }
          }
          p_oPkg = p_oPkg->GetNextPackage();
        }

        //Now update the tree based on the damage that occurred
        if ( 0 == iThisDamage )
        {
          //The tree is undamaged, but may need its counter incremented
          if ( iTreeDamage > 0 )
          {
            iYearsToHeal = iTreeDamage > 2000 ? iTreeDamage % 2000 : iTreeDamage % 1000;
            iYearsToHeal += iNumYearsPerTimestep;
            if ( iYearsToHeal >= m_iNumYearsToHeal )
              iTreeDamage = 0;
            else
              iTreeDamage += iNumYearsPerTimestep;
          }
        }
        else if ( 1 == iThisDamage )
        {

          //Tree gets medium damage
          if ( iTreeDamage > 2000 )
          {
            //Tree already had full damage; keep that damage status
            //and reset the counter
            iTreeDamage = 2000;
          }
          else
          {
            //Tree already had no more than medium damage; so just
            //reset the counter
            iTreeDamage = 1000;
          }
        }
        else
        {
          //Tree gets full damage
          iTreeDamage = 2000;
        }
      } //end of a storm did occur
      else
      {

        //There was no storm, but the tree may need decrementing
        if ( iTreeDamage > 0 )
        {
          iYearsToHeal = iTreeDamage > 2000 ? iTreeDamage % 2000 : iTreeDamage % 1000;

          iYearsToHeal += iNumYearsPerTimestep;
          if ( iYearsToHeal >= m_iNumYearsToHeal )
            iTreeDamage = 0;
          else
            iTreeDamage += iNumYearsPerTimestep;
        } else goto nextTree;
      }

      p_oTree->SetValue( mp_iStmDmgCodes[iTp] [iSp], iTreeDamage );

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
    stcErr.sFunction = "clStormDamageApplier::Action" ;
    throw( stcErr );
  }
}
