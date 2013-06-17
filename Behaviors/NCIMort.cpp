//---------------------------------------------------------------------------
// NCIMort.cpp
//---------------------------------------------------------------------------
#include "NCIMort.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include "Allometry.h"
#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIMort::clNCIMort( clSimManager * p_oSimManager ) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clMortalityBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "ncimortshell";
    m_sXMLRoot = "NCIMortality";

    //Version 2
    m_fVersionNumber = 2.0;
    m_fMinimumVersionNumber = 1.0;

    mp_iDeadCodes = NULL;
    m_cQuery = NULL;
    m_fNumberYearsPerTimestep = 0;

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
    stcErr.sFunction = "clNCIMort::clNCIMort" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clNCIMort::~clNCIMort()
{
  int i;
  if ( mp_iDeadCodes )
  {
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iDeadCodes[i];
    }
  }
  delete[] mp_iDeadCodes;
  delete[] m_cQuery;
}

//////////////////////////////////////////////////////////////////////////////
// FormatQuery()
//////////////////////////////////////////////////////////////////////////////
void clNCIMort::FormatQuery()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  char * cQuery = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50],
         cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSapling = false, bAdult = false;

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    if (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::sapling)
      bSapling = true;
    else if (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::adult)
      bAdult = true;
  }

  //Do a type/species search on all the types and species
  strcpy( cQuery, "species=" );
  for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
  {
    sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
    strcat( cQuery, cQueryPiece );
  }
  sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
  strcat( cQuery, cQueryPiece );

  //Now type
  strcat( cQuery, "::type=" );
  if ( bSapling )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::sapling, "," );
    strcat( cQuery, cQueryPiece );
  }
  if ( bAdult )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::adult, "," );
    strcat( cQuery, cQueryPiece );
  }

  //Remove the last comma
  cQuery[strlen( cQuery ) - 1] = '\0';

  //Now put it in m_cQuery, sized correctly
  m_cQuery = new char[strlen( cQuery ) + 1];
  strcpy( m_cQuery, cQuery );
  delete[] cQuery;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clNCIMort::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

    DoNCISetup( p_oPop, m_iNumBehaviorSpecies);
    ReadParameterFile( p_oDoc );
    ValidateData();
    GetTreeMemberCodes();
    SetFunctionPointers();
    FormatQuery();

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
    stcErr.sFunction = "clNCIMort::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clNCIMort::ReadParameterFile( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values
    std::stringstream sLabel;
    short int i, j; //loop counters

    //If any of the types is seedling, error out
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
           && clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType )
           {
             modelErr stcErr;
             stcErr.iErrorCode = BAD_DATA;
             stcErr.sFunction = "clNCIMort::ReadParameterFile" ;
             stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
             throw( stcErr );
      }

    //Make the list of indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;


    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Fill the variables

    //*************************************
    // General parameters
    //*************************************
    //Maximum survival probability
    FillSpeciesSpecificValue( p_oElement, "mo_nciMaxPotentialSurvival",
        "mo_nmpsVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxPotentialValue[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;


    //*************************************
    // Crowding effect parameters
    //*************************************
    //Max crowding radius
    FillSpeciesSpecificValue( p_oElement, "mo_nciMaxCrowdingRadius",
        "mo_nmcrVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxCrowdingRadius[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Neighbor DBH effect (alpha)
    FillSpeciesSpecificValue( p_oElement, "mo_nciNeighDBHEff", "mo_nndeVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fAlpha[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Neighbor distance effect (beta)
    FillSpeciesSpecificValue( p_oElement, "mo_nciNeighDistEff", "mo_nndseVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBeta[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Size sensitivity to NCI parameter (gamma)
    FillSpeciesSpecificValue( p_oElement, "mo_nciSizeSensNCI", "mo_nssnVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fGamma[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //NCI Slope (C)
    FillSpeciesSpecificValue( p_oElement, "mo_nciNCISlope", "mo_nnslVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCrowdingSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //NCI Steepness (D)
    FillSpeciesSpecificValue( p_oElement, "mo_nciNCISteepness", "mo_nnstVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCrowdingSteepness[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Neighbor storm effect - medium damage - not required if not using
    //storms; pre-fill with 1 (no damage) in case it's not in the parameter file
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fMedDamageEta[i] = 1.0;
    }
    FillSpeciesSpecificValue( p_oElement, "mo_nciNeighStormEffMedDmg", "mo_nnsemdVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, false );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMedDamageEta[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Neighbor storm effect - full damage - not required if not using
    //storms; pre-fill with 1 (no damage) in case it's not in the parameter file
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fFullDamageEta[i] = 1.0;
    }
    FillSpeciesSpecificValue( p_oElement, "mo_nciNeighStormEffFullDmg",
        "mo_nnsefdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fFullDamageEta[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //NCI DBH divisor
    FillSingleValue( p_oElement, "mo_nciDbhDivisor", & m_fDbhDivisor, true );

    //Whether to include snags
    FillSingleValue( p_oElement, "mo_nciIncludeSnagsInNCI", & m_bIncludeSnags, true );

    //Minimum neighbor DBH
    FillSpeciesSpecificValue( p_oElement, "mo_nciMinNeighborDBH", "mo_nmndVal", mp_fMinimumNeighborDBH, p_oPop, true );

    //Lambda
    for ( i = 0; i < clNCIBase::m_iNumTotalSpecies; i++ )
    {
      sLabel << "mo_nci" << p_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambda";
      FillSpeciesSpecificValue( p_oElement, sLabel.str(), "mo_nlVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      sLabel.str("");
      for ( j = 0; j < m_iNumBehaviorSpecies; j++ )
      {
        mp_fLambda[mp_iIndexes[p_fTempValues[j].code]] [i] = p_fTempValues[j].val;
      }
    }

    //*************************************
    // Size effect parameters
    //*************************************
    //Size effect mode (X0)
    FillSpeciesSpecificValue( p_oElement, "mo_nciSizeEffectMode", "mo_nsemVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fX0[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Size effect variance (Xb)
    FillSpeciesSpecificValue( p_oElement, "mo_nciSizeEffectVar", "mo_nsevVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fXb[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //*************************************
    // Shading effect parameters - not required
    //*************************************
    //Shading coefficient (m) - initialize all values to 0 in case they are
    //not in the parameter file - this turns off shading
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fShadingCoefficient[i] = 0.0;
      p_fTempValues[i].val = 0.0;
    }
    FillSpeciesSpecificValue( p_oElement, "mo_nciShadingCoefficient", "mo_nscVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, false );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fShadingCoefficient[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Shading exponent (n)
    FillSpeciesSpecificValue( p_oElement, "mo_nciShadingExponent", "mo_nseVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, false );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fShadingExponent[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;


    //*************************************
    // Damage effect parameters - not required
    //*************************************
    //Initialize all values to 1 (no damage) in case they aren't in the
    //parameter file
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fMedDamageStormEff[i] = 1.0;
      mp_fFullDamageStormEff[i] = 1.0;
      p_fTempValues[i].val = 1.0;
    }

    //Storm effect - medium damage
    FillSpeciesSpecificValue( p_oElement, "mo_nciStormEffMedDmg", "mo_nsemdVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMedDamageStormEff[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Storm effect - full damage
    FillSpeciesSpecificValue( p_oElement, "mo_nciStormEffFullDmg",
        "mo_nsefdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fFullDamageStormEff[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;


    delete[] p_fTempValues;
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
    stcErr.sFunction = "clNCIMort::ReadParameterFile" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetTreeMemberCodes()
////////////////////////////////////////////////////////////////////////////
void clNCIMort::GetTreeMemberCodes()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  int i, j, iNumTypes = p_oPop->GetNumberOfTypes();
  bool bUsed = false;

  //Declare the dead codes array
  mp_iDeadCodes = new short int * [m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    mp_iDeadCodes[i] = new short int[2];
  }
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    mp_iDeadCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]] [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
        p_oPop->GetIntDataCode( "dead", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

    if ( -1 == mp_iDeadCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
               [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] )
    {
      modelErr stcErr;
      stcErr.sFunction = "clNCIMort::GetTreeMemberCodes" ;
      stcErr.sMoreInfo = "Can't find the \"dead\" data member.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  }

  //See if damage is used at all
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    if ( mp_fMedDamageStormEff[i] < 1 || mp_fFullDamageStormEff[i] < 1 ||
         mp_fMedDamageEta[i] < 1 || mp_fFullDamageEta[i] < 1 )
    {
      bUsed = true;
      break;
    }
  }


  if ( true == bUsed )
  {
    mp_iDamageCodes = new short int * [clNCIBase::m_iNumTotalSpecies];
    for ( i = 0; i < clNCIBase::m_iNumTotalSpecies; i++ )
    {
      mp_iDamageCodes[i] = new short int[iNumTypes];
      for (j = 0; j < iNumTypes; j++)
        mp_iDamageCodes[i][j] = -1;

      mp_iDamageCodes[i] [clTreePopulation::sapling] = p_oPop->GetIntDataCode( "stm_dmg", i, clTreePopulation::sapling );
      mp_iDamageCodes[i] [clTreePopulation::adult] = p_oPop->GetIntDataCode( "stm_dmg", i, clTreePopulation::adult );
      mp_iDamageCodes[i] [clTreePopulation::snag] = p_oPop->GetIntDataCode( "stm_dmg", i, clTreePopulation::snag );
    }
  }

  //Is light used at all?
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    if ( fabs( mp_fShadingCoefficient[i] ) >= 0.001 )
    {
      bUsed = true;
      break;
    }
  }

  if ( bUsed )
  {

    //Declare the light codes array
    mp_iLightCodes = new short int * [m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_iLightCodes[i] = new short int[2];
    }

    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {

      //Get the code if this species uses light
      if ( fabs( mp_fShadingCoefficient[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]] ) >= 0.001 )
      {
        mp_iLightCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]] [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
             p_oPop->GetFloatDataCode( "Light", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

        if ( -1 == mp_iLightCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
             [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] )
             {
               modelErr stcErr;
               stcErr.sFunction = "clNCIMort::GetTreeMemberCodes" ;
               stcErr.sMoreInfo = "All trees to which the shading effect of NCI mortality is used must have a light behavior applied.";
               stcErr.iErrorCode = BAD_DATA;
               throw( stcErr );
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clNCIMort::ValidateData()
{
  try
  {

    int i;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {

      //Make sure that the max radius of neighbor effects is > 0
      if ( mp_fMaxCrowdingRadius[i] < 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max crowding radius must be greater than 0.";
        throw( stcErr );
      }

      //Make sure that the maximum growth for each species is between 0 and 1
      if ( mp_fMaxPotentialValue[i] < 0 || mp_fMaxPotentialValue[i] > 1 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max survival probability must be between 0 and 1.";
        throw( stcErr );
      }

      //Make sure that the size effect mode is not 0
      if ( 0 == mp_fX0[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Size effect mode values cannot be 0.";
        throw( stcErr );
      }

      //Make sure that the size effect variance is not 0
      if ( 0 == mp_fXb[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Size effect variance values cannot be 0.";
        throw( stcErr );
      }

      //Make sure that the storm effect values are between 0 and 1
      if ( 0 > mp_fMedDamageStormEff[i] || mp_fMedDamageStormEff[i] > 1 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Storm effect values must be between 0 and 1.";
        throw( stcErr );
      }

      if ( 0 > mp_fFullDamageStormEff[i] || mp_fFullDamageStormEff[i] > 1 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Storm effect values must be between 0 and 1.";
        throw( stcErr );
      }
      //Make sure that the neighbor storm effect values are between 0 and 1
      if ( 0 > mp_fMedDamageEta[i] || mp_fMedDamageEta[i] > 1 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Neighbor storm effect values must be between 0 and 1.";
        throw( stcErr );
      }

      if ( 0 > mp_fFullDamageEta[i] || mp_fFullDamageEta[i] > 1 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Neighbor storm effect values must be between 0 and 1.";
        throw( stcErr );
      }
    }

    for ( i = 0; i < clNCIBase::m_iNumTotalSpecies; i++ )
    {
      //Make sure that the minimum neighbor DBH is not negative
      if ( 0 > mp_fMinimumNeighborDBH[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Minimum neighbor DBH for NCI cannot be less than 0.";
        throw( stcErr );
      }
    }
  }
  catch ( modelErr & err )
  {
    err.sFunction = "clNCIMort::ValidateData";
    err.iErrorCode = BAD_DATA;
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
    stcErr.sFunction = "clNCIMort::ValidateData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoMort
////////////////////////////////////////////////////////////////////////////
deadCode clNCIMort::DoMort( clTree * p_oTree, const float & fDbh, const short int & iSpecies )
{
  int iDead;
  p_oTree->GetValue(mp_iDeadCodes[mp_iIndexes[iSpecies]][p_oTree->GetType() - clTreePopulation::sapling], &iDead);
  return (deadCode)iDead;
}

////////////////////////////////////////////////////////////////////////////
// PreMortCalcs
////////////////////////////////////////////////////////////////////////////
void clNCIMort::PreMortCalcs( clTreePopulation * p_oPop )
{
  try
  {

#ifdef NCI_WRITER
    using namespace std;
    float fX, fY;
    int iTS = mp_oSimManager->GetCurrentTimestep();
    char cFilename[100];
    sprintf(cFilename, "%s%d%s", "MortNCI", iTS, ".txt");
    fstream out( cFilename, ios::trunc | ios::out );
    out << "Timestep\tSpecies\tDBH\tNCI\tSize Effect\tCrowding Effect\tDamage Effect\tSurv Prob\tDead?\n";
#endif

    clTreeSearch * p_oNCITrees; //trees that this growth behavior applies to
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTree * p_oTree; //a single tree we're working with
    float fDamageEffect, //this tree's damage effect
         fCrowdingEffect, //tree's crowding effect
         fNCI, //the NCI
         fSizeEffect, //tree's size effect
         fShadingEffect, //tree's shading effect
         fDbh, //tree's dbh
         fSurvivalProb; //amount diameter increase - intermediate
    int iDead;
    short int iSpecies, iType; //type and species of a tree
    bool bIsDead;

    p_oNCITrees = p_oPop->Find( m_cQuery );
    p_oTree = p_oNCITrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      if ( mp_bUsesThisMortality[iSpecies][iType] )
      {

        //Make sure tree's not dead
        p_oTree->GetValue( mp_iDeadCodes[mp_iIndexes[iSpecies]] [iType - clTreePopulation::sapling], & iDead );

        if ( iDead <= natural )
        {

          p_oTree->GetValue( p_oPop->GetDbhCode( iSpecies, iType ), & fDbh );

          //Get the tree's crowding effect
          //Get this tree's NCI value using the function appropriate to
          //this species
          fNCI = ( * this.*mp_NCI[mp_iIndexes[iSpecies]] ) ( p_oTree, p_oPop, p_oPlot );
          fCrowdingEffect = ( * this.*mp_CrowdingEffect[mp_iIndexes[iSpecies]] ) ( fDbh, fNCI, iSpecies );
          //Make sure it's between 0 and 1
          if ( fCrowdingEffect < 0 ) fCrowdingEffect = 0;
          if ( fCrowdingEffect > 1 ) fCrowdingEffect = 1;

          //Get the tree's shading effect
          fShadingEffect = ( * this.*mp_ShadingEffect[mp_iIndexes[iSpecies]] ) ( p_oTree );
          //Make sure it's between 0 and 1
          if ( fShadingEffect < 0 ) fShadingEffect = 0;
          if ( fShadingEffect > 1 ) fShadingEffect = 1;

          //Get the tree's damage effect
          fDamageEffect = ( * this.*DamageEffect ) ( p_oTree );

          //Get the tree's size effect
          fSizeEffect = exp( -0.5 * pow( log( fDbh / mp_fX0[mp_iIndexes[iSpecies]] ) / mp_fXb[mp_iIndexes[iSpecies]], 2 ) );

          //Make sure it's bounded between 0 and 1
          if ( fSizeEffect < 0 ) fSizeEffect = 0;
          if ( fSizeEffect > 1 ) fSizeEffect = 1;

          //Determine whether tree survives
          fSurvivalProb = mp_fMaxPotentialValue[mp_iIndexes[iSpecies]] * fSizeEffect * fCrowdingEffect
               * fDamageEffect * fShadingEffect;
          fSurvivalProb = pow( fSurvivalProb, m_fNumberYearsPerTimestep );

          bIsDead = clModelMath::GetRand() >= fSurvivalProb;

          //Assign the value back to the dead data member
          if (bIsDead) iDead = natural;
          else iDead = notdead;
          p_oTree->SetValue( mp_iDeadCodes[mp_iIndexes[iSpecies]] [iType - clTreePopulation::sapling], iDead );

#ifdef NCI_WRITER
          //if (8 == p_oTree->GetSpecies()) {
            out << iTS << "\t"  << p_oTree->GetSpecies() << "\t" << fDbh
                << "\t" << fNCI  << "\t" << fSizeEffect << "\t"
                << fCrowdingEffect << "\t" << fDamageEffect << "\t"
                << fSurvivalProb << "\t" << bIsDead << "\n";
          //}
#endif

        } //end of if ( notdead == iDead )
      }

      p_oTree = p_oNCITrees->NextTree();
    }

#ifdef NCI_WRITER
    out.close();
#endif

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
    stcErr.sFunction = "clNCIMort::PreMortGrowth" ;
    throw( stcErr );
  }
}
