//---------------------------------------------------------------------------
// StormKiller.cpp
//---------------------------------------------------------------------------
#include "StormKiller.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "ModelMath.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clStormKiller::clStormKiller( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "StormKiller";
    m_sXMLRoot = "StormKiller";

    m_cQuery = NULL;
    mp_fMinStormDBH = NULL;
    mp_iStmDmgCodes = NULL;
    mp_iDeadCodes = NULL;
    mp_fPropTipUp = NULL;
    mp_fStmDmgMedA = NULL;
    mp_fStmDmgHeavyA = NULL;
    mp_fStmDmgMedB = NULL;
    mp_fStmDmgHeavyB = NULL;

    m_iSnagYears = 0;
    m_iNumTypes = 0;

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
    stcErr.sFunction = "clStormKiller::clStormKiller" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clStormKiller::~clStormKiller()
{
  delete[] m_cQuery;
  delete[] mp_fMinStormDBH;
  delete[] mp_fStmDmgMedA;
  delete[] mp_fStmDmgHeavyA;
  delete[] mp_fStmDmgMedB;
  delete[] mp_fStmDmgHeavyB;
  delete[] mp_fPropTipUp;
  if ( mp_iStmDmgCodes )
  {
    for ( int i = 0; i < m_iNumTypes; i++ )
      delete[] mp_iStmDmgCodes[i];
  }
  delete[] mp_iStmDmgCodes;
  if ( mp_iDeadCodes )
  {
    for ( int i = 0; i < m_iNumTypes; i++ )
      delete[] mp_iDeadCodes[i];
  }
  delete[] mp_iDeadCodes;
}

////////////////////////////////////////////////////////////////////////////
// GetDeadCodes()
////////////////////////////////////////////////////////////////////////////
void clStormKiller::GetDeadCodes(clTreePopulation *p_oPop)
{
  int i, j, iNumSpecies = p_oPop->GetNumberOfSpecies();

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
      stcErr.sFunction = "clStormKiller::GetDeadCodes" ;
      stcErr.sMoreInfo = "You must use tree mortality behaviors along with the storm killer.";
      throw( stcErr );
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// GetStmDmgCodes()
////////////////////////////////////////////////////////////////////////////
void clStormKiller::GetStmDmgCodes()
{
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
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
      stcErr.sFunction = "clStormKiller::GetStmDmgCodes" ;
      stcErr.sMoreInfo = "The storm killer behavior cannot be applied to seedlings.";
      throw( stcErr );
    }

    mp_iStmDmgCodes[mp_whatSpeciesTypeCombos[i].iType] [mp_whatSpeciesTypeCombos[i].iSpecies] =
         p_oPop->GetIntDataCode( "stm_dmg", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

    if (-1 == mp_iStmDmgCodes[mp_whatSpeciesTypeCombos[i].iType] [mp_whatSpeciesTypeCombos[i].iSpecies]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clStormKiller::GetStmDmgCodes" ;
      stcErr.sMoreInfo = "You must use the storm damage applier behavior along with the storm killer.";
      throw( stcErr );
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clStormKiller::GetParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    int i, iNumSpecies = p_oPop->GetNumberOfSpecies();

    //Declare our arrays
    mp_fMinStormDBH = new double[iNumSpecies];
    mp_fStmDmgMedA = new double[iNumSpecies];
    mp_fStmDmgHeavyA = new double[iNumSpecies];
    mp_fStmDmgMedB = new double[iNumSpecies];
    mp_fStmDmgHeavyB = new double[iNumSpecies];
    mp_fPropTipUp = new double[iNumSpecies];

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

    //Medium damage probability of survival "a"
    FillSpeciesSpecificValue( p_oElement, "st_stmMedDmgSurvProbA", "st_smdspaVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStmDmgMedA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Medium damage probability of survival "b"
    FillSpeciesSpecificValue( p_oElement, "st_stmMedDmgSurvProbB", "st_smdspbVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStmDmgMedB[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Heavy damage probability of survival "a"
    FillSpeciesSpecificValue( p_oElement, "st_stmFullDmgSurvProbA", "st_sfdspaVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStmDmgHeavyA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Heavy damage probability of survival "b"
    FillSpeciesSpecificValue( p_oElement, "st_stmFullDmgSurvProbB", "st_sfdspbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fStmDmgHeavyB[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Proportion of dead trees with full damage that tip-up
    FillSpeciesSpecificValue( p_oElement, "st_stmPropTipUpFullDmg", "st_sptufdVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //between 0 and 1
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( p_fTempValues[i].val < 0 || p_fTempValues[i].val > 1 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clStormKiller::GetParameterFileData" ;
        stcErr.sMoreInfo = "All values for proportion of fully-damaged trees that tip-up must be between 0 and 1.";
        throw( stcErr );
      }
      mp_fPropTipUp[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Number of years storm-damaged snags hang on
    FillSingleValue( p_oElement, "st_numYearsStormSnags", & m_iSnagYears, true );
    if ( m_iSnagYears < 0 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clStormKiller::GetParameterFileData" ;
      stcErr.sMoreInfo = "The number of years snags last parameter cannot be less than 0.";
      throw( stcErr );
    }

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
    stcErr.sFunction = "clStormKiller::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clStormKiller::FormatQueryString()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  char * cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];
  char cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSapling = false, bAdult = false, bSnag = false;

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
    else if ( clTreePopulation::snag == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSnag = true;
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
void clStormKiller::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {

    //Read in the parameter file data
    GetParameterFileData( p_oDoc );

    //Format our query string
    FormatQueryString();

    GetStmDmgCodes();

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
    stcErr.sFunction = "clStormKiller::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clStormKiller::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    float fDbh, //tree's dbh
          fSurvivalProb; //probability of survival
    int iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(), iTreeDamage, iSnagYears,
        iSp, iTp;

    //Get the dead codes if we haven't already - have to do this here because
    //they wouldn't be available at setup time
    if ( !mp_iDeadCodes ) GetDeadCodes(p_oPop);

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

      //If this is a snag, check to see if it needs its counter incremented or
      //if it wants removal
      if ( clTreePopulation::snag == iTp )
      {

        //Get the counter
        p_oTree->GetValue( mp_iStmDmgCodes[iTp] [iSp], & iTreeDamage );

        //If the counter is 0, it's not storm-killed; so get out
        if ( 0 == iTreeDamage ) goto nextTree;

        //Get the time since damage counter and see if it's equal to the
        //snag lifespan
        iSnagYears = iTreeDamage > 2000 ? iTreeDamage % 2000 : iTreeDamage % 1000;
        iSnagYears += iNumYearsPerTimestep;
        if ( iSnagYears >= m_iSnagYears )
        {
          //Kill it!
          p_oPop->KillTree( p_oTree, storm );
        }
        else
        {
          //Increment the counter and put it back
          iTreeDamage += iNumYearsPerTimestep;
          p_oTree->SetValue( mp_iStmDmgCodes[iTp] [iSp], iTreeDamage );
        }

        //Nothing else to do with this tree - skip out to the next tree
        goto nextTree;
      }

      //If this tree is too small, skip to next tree
      p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDbh );
      if ( fDbh < mp_fMinStormDBH[iSp] )
        goto nextTree;

      //Get the tree's existing damage
      p_oTree->GetValue( mp_iStmDmgCodes[iTp] [iSp], & iTreeDamage );

      //If no damage, skip to next tree
      if ( 0 == iTreeDamage ) goto nextTree;

      //If not damaged this timestep, skip to next tree
      if (0 != (iTreeDamage > 2000) ? iTreeDamage % 2000 : iTreeDamage % 1000)
        goto nextTree;

      //Assess probability of survival based on damage level
      if (iTreeDamage < 2000) {
        //Medium damage
        fSurvivalProb = exp(mp_fStmDmgMedA[iSp] + mp_fStmDmgMedB[iSp] * fDbh) /
           (1 + exp(mp_fStmDmgMedA[iSp] + mp_fStmDmgMedB[iSp] * fDbh));
      }
      else {
        //Heavy damage
        fSurvivalProb = exp(mp_fStmDmgHeavyA[iSp] + mp_fStmDmgHeavyB[iSp] * fDbh) /
           (1 + exp(mp_fStmDmgHeavyA[iSp] + mp_fStmDmgHeavyB[iSp] * fDbh));
      }

      //Does this tree die?
      if (fSurvivalProb < clModelMath::GetRand()) {
        //Yes it dies - if this is an adult and this behavior applies to
        //snags, create one
        if (-1 != mp_iStmDmgCodes[clTreePopulation::snag] [iSp] && clTreePopulation::adult == iTp) {
          p_oTree = p_oPop->KillTree(p_oTree, storm);

          //If this is a heavily damaged tree, see if it tips up
          if (iTreeDamage >= 2000) {
            if (clModelMath::GetRand() <= mp_fPropTipUp[iSp] ) {
              //Yes - "kill" the snag
              p_oTree->SetValue( p_oPop->GetIntDataCode( "dead", iSp, clTreePopulation::snag ), storm );
            }

            //Set the new value of the damage counter, since we're here in
            //this "if"
            iTreeDamage = 2000;
          }
          else
            iTreeDamage = 1000;

          //Reset the damage counter
          p_oTree->SetValue(mp_iStmDmgCodes[clTreePopulation::snag] [iSp], iTreeDamage );
          goto nextTree;
        }
        else {
          //No snags - just kill this tree
          p_oTree->SetValue( p_oPop->GetIntDataCode( "dead", iSp, iTp ), storm );
          goto nextTree;
        }
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
    stcErr.sFunction = "clStormKiller::Action" ;
    throw( stcErr );
  }
}
