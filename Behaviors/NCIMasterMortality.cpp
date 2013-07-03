//---------------------------------------------------------------------------
// NCIMort.cpp
//---------------------------------------------------------------------------
#include "NCIMasterMortality.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include "Allometry.h"
#include <stdio.h>
#include <sstream>

#include "NCI/DefaultCrowdingEffect.h"
#include "NCI/DefaultDamageEffect.h"
#include "NCI/DefaultNCITerm.h"
#include "NCI/DefaultShadingEffect.h"
#include "NCI/DefaultSizeEffect.h"
#include "NCI/NCITermWithNeighborDamage.h"
#include "NCI/NCILargerNeighbors.h"
#include "NCI/NoCrowdingEffect.h"
#include "NCI/NoDamageEffect.h"
#include "NCI/NoNCITerm.h"
#include "NCI/NoShadingEffect.h"
#include "NCI/NoSizeEffect.h"
#include "NCI/NoTemperatureEffect.h"
#include "NCI/WeibullTemperatureEffect.h"
#include "NCI/NoPrecipitationEffect.h"
#include "NCI/WeibullPrecipitationEffect.h"
#include "NCI/SizeEffectLowerBounded.h"
#include "NCI/NoNitrogenEffect.h"
#include "NCI/GaussianNitrogenEffect.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIMasterMortality::clNCIMasterMortality( clSimManager * p_oSimManager ) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clMortalityBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "ncimortshell";
    m_sXMLRoot = "NCIMasterMortality";

    //Version 3
    m_fVersionNumber = 3.0;
    m_fMinimumVersionNumber = 3.0;

    mp_iDeadCodes = NULL;
    m_cQuery = NULL;
    m_fNumberYearsPerTimestep = 0;

    mp_fMaxPotentialValue = NULL;

    mp_oCrowdingEffect = NULL;
    mp_oDamageEffect = NULL;
    mp_oNCITerm = NULL;
    mp_oShadingEffect = NULL;
    mp_oSizeEffect = NULL;
    mp_oPrecipEffect = NULL;
    mp_oTempEffect = NULL;
    mp_oNEffect = NULL;

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
    stcErr.sFunction = "clNCIMasterMortality::clNCIMasterMortality" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clNCIMasterMortality::~clNCIMasterMortality()
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

  delete mp_oCrowdingEffect;
  delete mp_oDamageEffect;
  delete mp_oNCITerm;
  delete mp_oShadingEffect;
  delete mp_oSizeEffect;
  delete mp_oPrecipEffect;
  delete mp_oTempEffect;
  delete mp_oNEffect;
}

//////////////////////////////////////////////////////////////////////////////
// FormatQuery()
//////////////////////////////////////////////////////////////////////////////
void clNCIMasterMortality::FormatQuery()
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
void clNCIMasterMortality::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    ReadParameterFile( p_oDoc );
    ValidateData();
    GetTreeMemberCodes();
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
    stcErr.sFunction = "clNCIMasterMortality::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterMortality::ReadParameterFile( xercesc::DOMDocument * p_oDoc )
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  floatVal * p_fTempValues; //for getting species-specific values
  std::stringstream sLabel;
  int iVal;
  short int i; //loop counters

  mp_fMaxPotentialValue = new float[p_oPop->GetNumberOfSpecies()];

  //If any of the types is seedling, error out
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
        && clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCIMasterMortality::ReadParameterFile" ;
      stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
      throw( stcErr );
    }

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Fill the variables

  //Maximum survival probability
  FillSpeciesSpecificValue( p_oElement, "mo_nciMaxPotentialSurvival",
      "mo_nmpsVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fMaxPotentialValue[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;

  //Which shading term?
  FillSingleValue(p_oElement, "nciWhichShadingEffect", &iVal, true);
  if (iVal == no_shading) {
    mp_oShadingEffect = new clNoShadingEffect();
  } else if (iVal == default_shading) {
    mp_oShadingEffect = new clDefaultShadingEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterMortality::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized shading term.";
    throw(err);
  }

  //Which crowding term?
  FillSingleValue(p_oElement, "nciWhichCrowdingEffect", &iVal, true);
  if (iVal == no_crowding_effect) {
    mp_oCrowdingEffect = new clNoCrowdingEffect();
  } else if (iVal == default_crowding_effect) {
    mp_oCrowdingEffect = new clDefaultCrowdingEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterMortality::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized crowding term.";
    throw(err);
  }

  //Which NCI term?
  FillSingleValue(p_oElement, "nciWhichNCITerm", &iVal, true);
  if (iVal == no_nci_term) {
    mp_oNCITerm = new clNoNCITerm();
  } else if (iVal == default_nci_term) {
    mp_oNCITerm = new clDefaultNCITerm();
  } else if (iVal == nci_with_neighbor_damage) {
    mp_oNCITerm = new clNCITermWithNeighborDamage();
  } else if (iVal == larger_neighbors) {
    mp_oNCITerm = new clNCILargerNeighbors();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterMortality::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized NCI term.";
    throw(err);
  }

  //Which size effect term?
  FillSingleValue(p_oElement, "nciWhichSizeEffect", &iVal, true);
  if (iVal == no_size_effect) {
    mp_oSizeEffect = new clNoSizeEffect();
  } else if (iVal == default_size_effect) {
    mp_oSizeEffect = new clDefaultSizeEffect();
  } else if (iVal == size_effect_bounded) {
    mp_oSizeEffect = new clSizeEffectLowerBounded();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterMortality::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized size effect term.";
    throw(err);
  }

  //Which damage effect term?
  FillSingleValue(p_oElement, "nciWhichDamageEffect", &iVal, true);
  if (iVal == no_damage_effect) {
    mp_oDamageEffect = new clNoDamageEffect();
  } else if (iVal == default_damage_effect) {
    mp_oDamageEffect = new clDefaultDamageEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterMortality::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized damage effect term.";
    throw(err);
  }

  //Which precipitation effect term?
  FillSingleValue(p_oElement, "nciWhichPrecipitationEffect", &iVal, true);
  if (iVal == no_precip_effect) {
    mp_oPrecipEffect = new clNoPrecipitationEffect();
  } else if (iVal == weibull_precip_effect) {
    mp_oPrecipEffect = new clWeibullPrecipitationEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterMortality::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized precipitation effect term.";
    throw(err);
  }

  //Which temperature effect term?
  FillSingleValue(p_oElement, "nciWhichTemperatureEffect", &iVal, true);
  if (iVal == no_temp_effect) {
    mp_oTempEffect = new clNoTemperatureEffect();
  } else if (iVal == weibull_temp_effect) {
    mp_oTempEffect = new clWeibullTemperatureEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterMortality::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized temperature effect term.";
    throw(err);
  }

  //Which nitrogen effect term?
  FillSingleValue(p_oElement, "nciWhichNitrogenEffect", &iVal, true);
  if (iVal == no_nitrogen_effect) {
    mp_oNEffect = new clNoNitrogenEffect();
  } else if (iVal == gauss_nitrogen_effect) {
    mp_oNEffect = new clGaussianNitrogenEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterMortality::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized nitrogen effect term.";
    throw(err);
  }

  mp_oCrowdingEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oDamageEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oNCITerm->DoSetup(p_oPop, this, p_oElement);
  mp_oShadingEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oSizeEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oPrecipEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oTempEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oNEffect->DoSetup(p_oPop, this, p_oElement);
}

////////////////////////////////////////////////////////////////////////////
// GetTreeMemberCodes()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterMortality::GetTreeMemberCodes()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  int i, j, iNumTypes = p_oPop->GetNumberOfTypes();

  //Declare the dead codes array
  mp_iDeadCodes = new short int * [m_iNumTotalSpecies];
  for ( i = 0; i < m_iNumTotalSpecies; i++ ) {
    mp_iDeadCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iDeadCodes[i][j] = -1;
    }
  }
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ ) {
    mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] =
        p_oPop->GetIntDataCode( "dead", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

    if ( -1 == mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                             [mp_whatSpeciesTypeCombos[i].iType] )
    {
      modelErr stcErr;
      stcErr.sFunction = "clNCIMasterMortality::GetTreeMemberCodes" ;
      stcErr.sMoreInfo = "Can't find the \"dead\" data member.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clNCIMasterMortality::ValidateData()
{
  int i;
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {

    //Make sure that the maximum survival for each species is between 0 and 1
    if ( mp_fMaxPotentialValue[mp_iWhatSpecies[i]] < 0 || mp_fMaxPotentialValue[mp_iWhatSpecies[i]] > 1 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clNCIMasterMortality::ValidateData";
      stcErr.sMoreInfo = "All values for max survival probability must be between 0 and 1.";
      throw( stcErr );
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// DoMort
////////////////////////////////////////////////////////////////////////////
deadCode clNCIMasterMortality::DoMort( clTree * p_oTree, const float & fDbh, const short int & iSpecies )
{
  int iDead;
  p_oTree->GetValue(mp_iDeadCodes[iSpecies][p_oTree->GetType()], &iDead);
  return (deadCode)iDead;
}

////////////////////////////////////////////////////////////////////////////
// PreMortCalcs
////////////////////////////////////////////////////////////////////////////
void clNCIMasterMortality::PreMortCalcs( clTreePopulation * p_oPop )
{
  float *p_fTempEffect,
        *p_fPrecipEffect,
        *p_fNEffect;
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
    short int iSpecies, iType, //type and species of a tree
    i; //loop counter
    bool bIsDead;

    p_fPrecipEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fTempEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fNEffect = new float[p_oPop->GetNumberOfSpecies()];
    //Calculate climate effects for all species
    for (i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fPrecipEffect[mp_iWhatSpecies[i]] = mp_oPrecipEffect->CalculatePrecipitationEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fTempEffect[mp_iWhatSpecies[i]] = mp_oTempEffect->CalculateTemperatureEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fNEffect[mp_iWhatSpecies[i]] = mp_oNEffect->CalculateNitrogenEffect(p_oPlot, mp_iWhatSpecies[i]);
    }

    p_oNCITrees = p_oPop->Find( m_cQuery );
    p_oTree = p_oNCITrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      if ( mp_bUsesThisMortality[iSpecies][iType] )
      {

        //Make sure tree's not dead
        p_oTree->GetValue( mp_iDeadCodes[iSpecies][iType], & iDead );

        if ( iDead <= natural )
        {

          p_oTree->GetValue( p_oPop->GetDbhCode( iSpecies, iType ), & fDbh );

          //Get NCI
          fNCI = mp_oNCITerm->CalculateNCITerm(p_oTree, p_oPop, p_oPlot);

          //Get crowding effect
          fCrowdingEffect = mp_oCrowdingEffect->CalculateCrowdingEffect(p_oTree, fDbh, fNCI);

          //Get the tree's damage effect
          fDamageEffect = mp_oDamageEffect->CalculateDamageEffect(p_oTree);

          //Get the tree's shading effect
          fShadingEffect = mp_oShadingEffect->CalculateShadingEffect(p_oTree);

          //Get the tree's size effect
          fSizeEffect = mp_oSizeEffect->CalculateSizeEffect(iSpecies, fDbh);

          //Determine whether tree survives
          fSurvivalProb = mp_fMaxPotentialValue[iSpecies] * fSizeEffect *
                fCrowdingEffect * fShadingEffect * fDamageEffect *
                p_fPrecipEffect[iSpecies] * p_fTempEffect[iSpecies] *
                p_fNEffect[iSpecies];
          fSurvivalProb = pow( fSurvivalProb, m_fNumberYearsPerTimestep );

          bIsDead = clModelMath::GetRand() >= fSurvivalProb;

          //Assign the value back to the dead data member
          if (bIsDead) iDead = natural;
          else iDead = notdead;
          p_oTree->SetValue( mp_iDeadCodes[iSpecies][iType], iDead );

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
    stcErr.sFunction = "clNCIMasterMortality::PreMortGrowth" ;
    throw( stcErr );
  }
}
