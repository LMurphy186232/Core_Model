//---------------------------------------------------------------------------
#include "NCIJuvenileGrowth.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIJuvenileGrowth::clNCIJuvenileGrowth(clSimManager * p_oSimManager) :
clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
clGrowthBase(p_oSimManager) {
  try
  {
    //Set namestring
    m_sNameString = "ncijuvenilegrowthshell";
    m_sXMLRoot = "NCIJuvenileGrowth";

    //Null out our pointers
    mp_iGrowthCodes = NULL;
    mp_fAlpha = NULL;
    mp_fBeta = NULL;
    mp_fMaxGrowth = NULL;
    mp_fLambda = NULL;
    mp_fMaxCrowdingRadius = NULL;
    mp_iIndexes = NULL;
    mp_fCrowdingSlope = NULL;
    mp_fCrowdingSteepness = NULL;
    mp_fSizeEffectB = NULL;
    mp_fSizeEffectA = NULL;
    mp_iWhatBehaviorTypes = NULL;
    mp_fMinimumNeighborDiam10 = NULL;

    m_iNumBehaviorTypes = 0;
    m_bIncludeSnags = false;
    m_fDiam10Divisor = 0;

    //Version 2
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
    stcErr.sFunction = "clNCIJuvenileGrowth::clNCIJuvenileGrowth" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clNCIJuvenileGrowth::~clNCIJuvenileGrowth() {
  if (mp_iGrowthCodes) {
    for (int i = 0; i < m_iNumBehaviorSpecies; i++) {
      delete[] mp_iGrowthCodes[i];
    }
    delete[] mp_iGrowthCodes;
  }
  delete[] mp_iWhatBehaviorTypes;
  delete[] mp_fAlpha;
  delete[] mp_fBeta;
  delete[] mp_fMaxGrowth;
  delete[] mp_fMaxCrowdingRadius;
  if ( mp_fLambda )
    for ( int i = 0; i < m_iNumBehaviorSpecies; i++ )
      delete[] mp_fLambda[i];
  delete[] mp_fLambda;
  delete[] mp_iIndexes;
  delete[] mp_fCrowdingSlope;
  delete[] mp_fCrowdingSteepness;
  delete[] mp_fSizeEffectB;
  delete[] mp_fSizeEffectA;
  delete[] mp_fMinimumNeighborDiam10;
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clNCIJuvenileGrowth::ReadParameterFile(xercesc::DOMDocument * p_oDoc) {
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values
    std::stringstream sLabel;
    short int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i, j; //loop counters

    //If any of the types is not juvenile, error out
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
          && clTreePopulation::seedling != mp_whatSpeciesTypeCombos[i].iType )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clNCIJuvenileGrowth::ReadParameterFile" ;
        stcErr.sMoreInfo = "This behavior can only be applied to seedlings and saplings.";
        throw( stcErr );
      }

    mp_fMinimumNeighborDiam10 = new float[iNumTotalSpecies];
    mp_fAlpha = new float[m_iNumBehaviorSpecies];
    mp_fBeta = new float[m_iNumBehaviorSpecies];
    mp_fCrowdingSlope = new float[m_iNumBehaviorSpecies];
    mp_fCrowdingSteepness = new float[m_iNumBehaviorSpecies];
    mp_fSizeEffectA = new float[m_iNumBehaviorSpecies];
    mp_fMaxGrowth = new float[m_iNumBehaviorSpecies];
    mp_fSizeEffectB = new float[m_iNumBehaviorSpecies];
    mp_fMaxCrowdingRadius = new float[m_iNumBehaviorSpecies];
    mp_fLambda = new float * [m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fLambda[i] = new float[iNumTotalSpecies];
      for ( j = 0; j < iNumTotalSpecies; j++ )
        mp_fLambda[i] [j] = 0;
    }

    //Make the list of indexes
    mp_iIndexes = new short int[iNumTotalSpecies];
    for ( i = 0; i < iNumTotalSpecies; i++ ) mp_iIndexes[i] = -1;
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
    //Maximum potential growth
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCIMaxPotentialGrowth",
        "gr_jnmpgVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxGrowth[i] = p_fTempValues[i].val;

    //*************************************
    // Crowding effect parameters
    //*************************************
    //Max crowding radius
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCIMaxCrowdingRadius",
        "gr_jnmcrVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxCrowdingRadius[i] = p_fTempValues[i].val;

    //Neighbor DBH effect (alpha)
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCIAlpha", "gr_jnaVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fAlpha[i] = p_fTempValues[i].val;

    //Neighbor distance effect (beta)
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCIBeta", "gr_jnbVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBeta[i] = p_fTempValues[i].val;

    //Lambda
    for ( i = 0; i < iNumTotalSpecies; i++ )
    {
      sLabel << "gr_juvNCI" << p_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambda";
      FillSpeciesSpecificValue( p_oElement, sLabel.str(), "gr_jnlVal", p_fTempValues,
          m_iNumBehaviorSpecies, p_oPop, true );
      sLabel.str("");
      for ( j = 0; j < m_iNumBehaviorSpecies; j++ )
        mp_fLambda[j] [i] = p_fTempValues[j].val;
    }

    //Minimum neighbor Diam10
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCIMinNeighborDiam10",
        "gr_jnmndVal", mp_fMinimumNeighborDiam10, p_oPop, true );

    //Crowding Slope (C)
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCICrowdingSlope",
        "gr_jncslVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCrowdingSlope[i] = p_fTempValues[i].val;

    //Crowding Steepness (D)
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCICrowdingSteepness",
        "gr_jncstVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCrowdingSteepness[i] = p_fTempValues[i].val;

    //NCI DBH divisor
    FillSingleValue( p_oElement, "gr_juvNCIDiam10Divisor", & m_fDiam10Divisor, true );

    //Whether to include snags
    FillSingleValue( p_oElement, "gr_juvNCIIncludeSnagsInNCI", & m_bIncludeSnags, true );

    //*************************************
    // Size effect parameters
    //*************************************
    //Size effect a
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCISizeEffectA", "gr_jnseaVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSizeEffectA[i] = p_fTempValues[i].val;

    //Size effect b
    FillSpeciesSpecificValue( p_oElement, "gr_juvNCISizeEffectB", "gr_jnsebVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSizeEffectB[i] = p_fTempValues[i].val;

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
    stcErr.sFunction = "clNCIJuvenileGrowth::ReadParameterFile" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clNCIJuvenileGrowth::ValidateData() {
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {

      //Make sure that the max radius of neighbor effects is > 0
      if ( mp_fMaxCrowdingRadius[i] < 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max crowding radius must be greater than 0.";
        throw( stcErr );
      }

      //Make sure that the maximum growth for each species is > 0
      if ( mp_fMaxGrowth[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max potential growth must be greater than 0.";
        throw( stcErr );
      }
    }

    for ( i = 0; i < iNumTotalSpecies; i++ )
    {
      //Make sure that the minimum neighbor Diam10 is not negative
      if ( 0 > mp_fMinimumNeighborDiam10[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Minimum neighbor Diam10 for NCI cannot be less than 0.";
        throw( stcErr );
      }
    }

    //Make sure the DBH divisor is greater than 0
    if ( m_fDiam10Divisor <= 0 )
    {
      modelErr stcErr;
      stcErr.sMoreInfo = "The NCI Diam10 divisor must be greater than 0.";
      throw( stcErr );
    }
  }
  catch ( modelErr & err )
  {
    err.sFunction = "clNCIJuvenileGrowth::ValidateData";
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
    stcErr.sFunction = "clNCIJuvenileGrowth::ValidateData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetTreeMemberCodes()
////////////////////////////////////////////////////////////////////////////
void clNCIJuvenileGrowth::GetTreeMemberCodes() {
  int i, j;

  //Get codes for growth
  mp_iGrowthCodes = new short int * [m_iNumBehaviorSpecies];
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_iGrowthCodes[i] = new short int[2];
    for (j = 0; j < 2; j++) {
      mp_iGrowthCodes[i][j] = -1;
    }
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {

    //Get the code from growth org
    mp_iGrowthCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
                    [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::seedling]
                     = mp_oGrowthOrg->GetGrowthCode(mp_whatSpeciesTypeCombos[i].iSpecies,
                         mp_whatSpeciesTypeCombos[i].iType);
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clNCIJuvenileGrowth::DoShellSetup(xercesc::DOMDocument * p_oDoc) {
  AssembleUniqueTypes();
  ReadParameterFile( p_oDoc );
  ValidateData();
  GetTreeMemberCodes();
}

////////////////////////////////////////////////////////////////////////////
// AssembleUniqueTypes
////////////////////////////////////////////////////////////////////////////
void clNCIJuvenileGrowth::AssembleUniqueTypes() {
  short int * p_iTypesList, //for assembling the list of unique types
  i, j; //loop counters
  bool bFound; //flag used in assembling list of unique types

  //Declare the temp. types array to be as big as the combo list to make
  //sure we have space for everything, and initialize values to -1
  p_iTypesList = new short int[m_iNumSpeciesTypeCombos];
  for (i = 0; i < m_iNumSpeciesTypeCombos; i++)
    p_iTypesList[i] = -1;

  m_iNumBehaviorTypes = 0;
  //Go through each combo, and for the type for that combo, if it's not
  //already on the temp list, add it
  m_iNumBehaviorTypes = 0;
  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    bFound = false;
    //Test to see if this type is already on the list
    for (j = 0; j < m_iNumBehaviorTypes; j++) {
      if (mp_whatSpeciesTypeCombos[i].iType == p_iTypesList[j]) {
        bFound = true;
        break;
      }
    }
    if ( !bFound) {
      //Add the type to the list and increment the number of found species
      //by one
      p_iTypesList[m_iNumBehaviorTypes] = mp_whatSpeciesTypeCombos[i].iType;
      m_iNumBehaviorTypes++;
    }
  } //end of for (i = 0; i < m_iNumSpeciesTypeCombos; i++)

  mp_iWhatBehaviorTypes = new short int[m_iNumBehaviorTypes];
  for (i = 0; i < m_iNumBehaviorTypes; i++)
    mp_iWhatBehaviorTypes[i] = p_iTypesList[i];

  delete[] p_iTypesList;
}

////////////////////////////////////////////////////////////////////////////
// CalcGrowthValue
////////////////////////////////////////////////////////////////////////////
float clNCIJuvenileGrowth::CalcDiameterGrowthValue(clTree * p_oTree,
    clTreePopulation * p_oPop, float fHeightGrowth) {
  float fAmountDiamIncrease; //amount diameter increase

  //Get the tree's growth - it's already calculated
  p_oTree->GetValue( mp_iGrowthCodes[mp_iIndexes[p_oTree->GetSpecies()]]
                                     [p_oTree->GetType() - clTreePopulation::seedling],
                                     & fAmountDiamIncrease );

  return fAmountDiamIncrease;
}

////////////////////////////////////////////////////////////////////////////
// PreGrowthCalcs
////////////////////////////////////////////////////////////////////////////
void clNCIJuvenileGrowth::PreGrowthCalcs(clTreePopulation * p_oPop) {
  try
  {
    clTreeSearch * p_oNCITrees; //trees that this growth behavior applies to
    clAllometry *p_oAllom = p_oPop->GetAllometryObject();
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTree * p_oTree; //a single tree we're working with
    char cQuery[75], //for searching for trees
    cQueryPiece[5]; //for assembling the search query
    float fCrowdingEffect, //tree's crowding effect
    fNCI, //the NCI
    fSizeEffect, //tree's size effect
    fDiam10, //tree's diam10
    fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(),
    fAmountDiamIncrease, //amount diameter increase
    fTempDiamIncrease; //amount diameter increase - intermediate
    int iIsDead;
    short int iSpecies, iType, //type and species of a tree
    i, //loop counter
    iDeadCode; //tree's dead code

    //Do a type/species search on all the types and species
    strcpy( cQuery, "species=" );
    for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ ) {
      sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
      strcat( cQuery, cQueryPiece );
    }
    sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
    strcat( cQuery, cQueryPiece );

    strcat( cQuery, "::type=" );
    for ( i = 0; i < m_iNumBehaviorTypes - 1; i++ ) {
      sprintf( cQueryPiece, "%d%s", mp_iWhatBehaviorTypes[i], "," );
      strcat( cQuery, cQueryPiece );
    }
    sprintf( cQueryPiece, "%d", mp_iWhatBehaviorTypes[m_iNumBehaviorTypes - 1] );
    strcat( cQuery, cQueryPiece );

    p_oNCITrees = p_oPop->Find( cQuery );

    //************************************
    // Loop through and to calculate growth for each tree
    //************************************
    p_oTree = p_oNCITrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      if ( -1 == mp_iGrowthCodes[mp_iIndexes[iSpecies]] [iType - clTreePopulation::seedling] )
        goto nextTree;

      //Make sure tree's not dead
      iDeadCode = p_oPop->GetIntDataCode( "dead", p_oTree->GetSpecies(), p_oTree->GetType() );
      if ( -1 != iDeadCode ) {
        p_oTree->GetValue( iDeadCode, & iIsDead );
        if ( notdead != iIsDead ) goto nextTree;
      }


      p_oTree->GetValue( p_oPop->GetDiam10Code( iSpecies, iType ), & fDiam10 );

      //First calculate the pieces that have no diameter component and thus
      //will not change in our loop

      //Get NCI
      fNCI = CalculateNCI( p_oTree, p_oPop, p_oAllom, p_oPlot );
      if (fNCI > 0)
        fCrowdingEffect = exp( -mp_fCrowdingSlope[mp_iIndexes[iSpecies]] *
            pow( fNCI, mp_fCrowdingSteepness[mp_iIndexes[iSpecies]] ) );
      else fCrowdingEffect = 1;
      //Make sure it's between 0 and 1
      if ( fCrowdingEffect < 0 ) fCrowdingEffect = 0;
      if ( fCrowdingEffect > 1 ) fCrowdingEffect = 1;

      //To correctly compound growth over the number of years per timestep,
      //we have to loop over the number of years, re-calculating the parts
      //with diam10 and incrementing the diam10 each time
      fAmountDiamIncrease = 0;
      for ( i = 0; i < fNumberYearsPerTimestep; i++ )
      {
        //Get the tree's size effect
        fSizeEffect = mp_fSizeEffectA[mp_iIndexes[iSpecies]] *
            pow(fDiam10, mp_fSizeEffectB[mp_iIndexes[iSpecies]]);

        //Make sure it's bounded between 0 and 1
        if ( fSizeEffect < 0 ) fSizeEffect = 0;
        if ( fSizeEffect > 1 ) fSizeEffect = 1;

        //Calculate actual growth in cm/yr
        fTempDiamIncrease = mp_fMaxGrowth[mp_iIndexes[iSpecies]] *
            fSizeEffect * fCrowdingEffect;

        //Add it to the running total of diameter increase
        fAmountDiamIncrease += fTempDiamIncrease;

        //Increase the diameter for the next loop
        fDiam10 += fTempDiamIncrease;

      }

      //Assign the growth back to "Growth" and hold it
      p_oTree->SetValue( mp_iGrowthCodes[mp_iIndexes[iSpecies]]
                                         [iType - clTreePopulation::seedling], fAmountDiamIncrease );

      nextTree:
      p_oTree = p_oNCITrees->NextTree();
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
    stcErr.sFunction = "clNCIJuvenileGrowth::PreCalcGrowth" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalculateNCI
////////////////////////////////////////////////////////////////////////////
float clNCIJuvenileGrowth::CalculateNCI( clTree * p_oTree, clTreePopulation * p_oPop, clAllometry *p_oAllom, clPlot * p_oPlot )
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  char cQuery[75]; //format search strings into this
  float fNCI = 0, //nci - the end result of all this math
      fDistance, //distance between target and neighbor
      fDiam10, //neighbor's dbh
      fTemp,
      fNeighX, fNeighY, //holders for the neighbor tree's X and Y
      fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iIsDead; //whether a neighbor is dead
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iTargetSpecies, //target tree's species
  iDeadCode; //neighbor's dead code

  iTargetSpecies = p_oTree->GetSpecies();

  //Format the query to get all competing neighbors
  p_oTree->GetValue( p_oPop->GetXCode( iTargetSpecies, p_oTree->GetType() ), & fTargetX );
  p_oTree->GetValue( p_oPop->GetYCode( iTargetSpecies, p_oTree->GetType() ), & fTargetY );

  //Get all trees within the max crowding radius - seedlings don't compete
  sprintf( cQuery, "%s%f%s%f%s%f%s", "distance=", mp_fMaxCrowdingRadius[mp_iIndexes[iTargetSpecies]], "FROM x=", fTargetX,
      "y=", fTargetY, "::height=0");
  p_oAllNeighbors = p_oPop->Find( cQuery );

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor ) {
    if ( p_oNeighbor == p_oTree ) goto nextTree;

    iNeighSpecies = p_oNeighbor->GetSpecies();
    iNeighType = p_oNeighbor->GetType();

    if (clTreePopulation::snag == iNeighType && !m_bIncludeSnags) goto nextTree;

    //Make sure the neighbor's not dead
    iDeadCode = p_oPop->GetIntDataCode( "dead", p_oNeighbor->GetSpecies(), p_oNeighbor->GetType() );
    if ( -1 != iDeadCode ) {
      p_oNeighbor->GetValue( iDeadCode, & iIsDead );
      if (iIsDead != notdead && iIsDead != natural) goto nextTree;
    }

    //Get diam10 - if it's an adult, use the sapling allometry from DBH
    if (clTreePopulation::seedling == iNeighType ||
        clTreePopulation::sapling == iNeighType) {
      //Get the neighbor's diam10
      p_oNeighbor->GetValue( p_oPop->GetDiam10Code( iNeighSpecies, iNeighType ), & fDiam10 );
    } else {
      p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fTemp );
      fDiam10 = p_oAllom->ConvertDbhToDiam10(fTemp, iNeighSpecies);
    }

    if ( fDiam10 < mp_fMinimumNeighborDiam10[iNeighSpecies] ) goto nextTree;

    //Get the neighbor's X and Y values
    p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
    p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

    //Get the distance between the two trees
    fDistance = p_oPlot->GetDistance( fTargetX, fTargetY, fNeighX, fNeighY );


    //Only goto nextTree if distance is not 0 - it will be a fluke condition to
    //allow a tree that is literally standing on top of another one not to
    //affect it competitively, but there it is
    if ( fDistance < VERY_SMALL_VALUE) goto nextTree;

    //Add competitive effect to NCI
    fNCI += mp_fLambda[mp_iIndexes[iTargetSpecies]] [iNeighSpecies]
                                                     * ( pow( ( fDiam10 / m_fDiam10Divisor ), mp_fAlpha[mp_iIndexes[iTargetSpecies]] )
                                                         / pow( fDistance, mp_fBeta[mp_iIndexes[iTargetSpecies]] ) );

    nextTree:
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  return fNCI;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clNCIJuvenileGrowth::SetNameData(std::string sNameString) {

  //Check the string passed and set the flags accordingly
  if (sNameString.compare("NCIJuvenileGrowth") == 0 )
  {
    m_iGrowthMethod = diameter_auto;
  }
  else if (sNameString.compare("NCIJuvenileGrowth diam only") == 0 )
  {
    m_iGrowthMethod = diameter_only;
  }
  else
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Unrecognized behavior name \"" << sNameString << "\".";
    stcErr.sMoreInfo = s.str();
    stcErr.sFunction = "clNciGrowth::SetNameData" ;
    throw( stcErr );
  }
}
