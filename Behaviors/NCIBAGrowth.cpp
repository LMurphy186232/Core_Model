//---------------------------------------------------------------------------
#include "NCIBAGrowth.h"
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
clNCIBAGrowth::clNCIBAGrowth( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
  clGrowthBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "ncibagrowthshell";
    m_sXMLRoot = "NCIBAGrowth";

    //Null out our pointers
    mp_iGrowthCodes = NULL;
    mp_fCrowdingSlope = NULL;
    mp_fCrowdingSteepness = NULL;
    mp_fMinimumNeighborDBH = NULL;
    mp_fXb = NULL;
    mp_fMaxPotentialValue = NULL;
    mp_fX0 = NULL;
    mp_iIndexes = NULL;
    mp_iWhatBehaviorTypes = NULL;
    mp_fGamma = NULL;
    mp_fMaxCrowdingRadius = NULL;

    m_bUseOnlyLargerNeighbors = false;
    m_iNumBehaviorTypes = 0;
    m_fMinSaplingHeight = 0;
    m_iNumTotalSpecies = 0;
    m_fBADivisor = 0;

    //Version 1
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
    stcErr.sFunction = "clNCIBAGrowth::clNCIBAGrowth" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clNCIBAGrowth::~clNCIBAGrowth()
{
  if ( mp_iGrowthCodes )
  {
    for ( int i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iGrowthCodes[i];
    }
    delete[] mp_iGrowthCodes;
  }
  delete[] mp_fCrowdingSlope;
  delete[] mp_fCrowdingSteepness;
  delete[] mp_fMinimumNeighborDBH;
  delete[] mp_fXb;
  delete[] mp_fMaxPotentialValue;
  delete[] mp_fX0;
  delete[] mp_iIndexes;
  delete[] mp_iWhatBehaviorTypes;
  delete[] mp_fMaxCrowdingRadius;
  delete[] mp_fGamma;
}


////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clNCIBAGrowth::ReadParameterFile( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values
    short int i; //loop counter

    m_fMinSaplingHeight = 50;

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    //Get the minimum sapling height
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
      {
        m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
      }
    }

    //If any of the types is seedling, error out
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
           && clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType )
           {
             modelErr stcErr;
             stcErr.iErrorCode = BAD_DATA;
             stcErr.sFunction = "clNCIBAGrowth::ReadParameterFile" ;
             stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
             throw( stcErr );
      }

    //Make the list of indexes
    mp_iIndexes = new short int[m_iNumTotalSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //This alone is sized total number of species
    mp_fMinimumNeighborDBH = new float[m_iNumTotalSpecies];

    //The rest are sized number of species to which this behavior applies
    mp_fCrowdingSlope = new float[m_iNumBehaviorSpecies];
    mp_fCrowdingSteepness = new float[m_iNumBehaviorSpecies];
    mp_fXb = new float[m_iNumBehaviorSpecies];
    mp_fMaxPotentialValue = new float[m_iNumBehaviorSpecies];
    mp_fX0 = new float[m_iNumBehaviorSpecies];
    mp_fGamma = new float[m_iNumBehaviorSpecies];
    mp_fMaxCrowdingRadius = new float[m_iNumBehaviorSpecies];

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
    FillSpeciesSpecificValue( p_oElement, "gr_nciMaxPotentialGrowth", "gr_nmpgVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxPotentialValue[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //*************************************
    // Crowding effect parameters
    //*************************************
    //Max crowding radius
    FillSpeciesSpecificValue( p_oElement, "gr_nciMaxCrowdingRadius", "gr_nmcrVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxCrowdingRadius[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Size sensitivity to NCI parameter (gamma)
    FillSpeciesSpecificValue( p_oElement, "gr_nciSizeSensNCI", "gr_nssnVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fGamma[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Minimum neighbor DBH
    FillSpeciesSpecificValue( p_oElement, "gr_nciMinNeighborDBH", "gr_nmndVal", mp_fMinimumNeighborDBH, p_oPop, true );

    //Crowding Slope (C)
    FillSpeciesSpecificValue( p_oElement, "gr_nciCrowdingSlope", "gr_ncslVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCrowdingSlope[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Crowding Steepness (D)
    FillSpeciesSpecificValue( p_oElement, "gr_nciCrowdingSteepness", "gr_ncstVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCrowdingSteepness[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Size effect mode (X0)
    FillSpeciesSpecificValue( p_oElement, "gr_nciSizeEffectMode", "gr_nsemVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fX0[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Size effect variance (Xb)
    FillSpeciesSpecificValue( p_oElement, "gr_nciSizeEffectVariance", "gr_nsevVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fXb[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Whether to use only larger neighbors
    FillSingleValue( p_oElement, "gr_banciOnlyLargerNeighbors", & m_bUseOnlyLargerNeighbors, true );

    //Basal area divisor
    FillSingleValue( p_oElement, "gr_banciBADivisor", & m_fBADivisor, true );

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
    stcErr.sFunction = "clNCIBAGrowth::ReadParameterFile" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clNCIBAGrowth::ValidateData()
{
  try
  {

    int i;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {

      //Make sure that the max radius of neighbor effects is > 0
      if ( mp_fMaxCrowdingRadius[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max crowding radius must be greater than 0.";
        throw( stcErr );
      }

      //Make sure that the maximum growth for each species is > 0
      if ( mp_fMaxPotentialValue[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max potential growth must be greater than 0.";
        throw( stcErr );
      }

      //Make sure that the size effect mode is not 0
      if ( 0 >= mp_fX0[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Size effect mode values must be greater than 0.";
        throw( stcErr );
      }

      //Make sure that the size effect variance is not 0
      if ( 0 == mp_fXb[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Size effect variance values cannot be 0.";
        throw( stcErr );
      }
    }

    for ( i = 0; i < m_iNumTotalSpecies; i++ )
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
    err.sFunction = "clNCIBAGrowth::ValidateData";
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
    stcErr.sFunction = "clNCIBAGrowth::ValidateData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetGrowthCodes()
////////////////////////////////////////////////////////////////////////////
void clNCIBAGrowth::GetGrowthCodes()
{
  int i;

  //Get codes for growth
  mp_iGrowthCodes = new short int * [m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    mp_iGrowthCodes[i] = new short int[2];
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {

    //Get the code from growth org
    mp_iGrowthCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
         [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
         mp_oGrowthOrg->GetGrowthCode( mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clNCIBAGrowth::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    AssembleUniqueTypes();
    ReadParameterFile( p_oDoc );
    ValidateData();
    GetGrowthCodes();
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
    stcErr.sFunction = "clNCIBAGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// AssembleUniqueTypes
////////////////////////////////////////////////////////////////////////////
void clNCIBAGrowth::AssembleUniqueTypes()
{
  short int * p_iTypesList, //for assembling the list of unique types
       i, j; //loop counters
  bool bFound; //flag used in assembling list of unique types

  //Declare the temp. types array to be as big as the combo list to make
  //sure we have space for everything, and initialize values to -1
  p_iTypesList = new short int[m_iNumSpeciesTypeCombos];
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    p_iTypesList[i] = -1;

  m_iNumBehaviorTypes = 0;
  //Go through each combo, and for the type for that combo, if it's not
  //already on the temp list, add it
  m_iNumBehaviorTypes = 0;
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    bFound = false;
    //Test to see if this type is already on the list
    for ( j = 0; j < m_iNumBehaviorTypes; j++ )
    {
      if ( mp_whatSpeciesTypeCombos[i].iType == p_iTypesList[j] )
      {
        bFound = true; break;
      }
    }
    if ( !bFound )
    {
      //Add the type to the list and increment the number of found species
      //by one
      p_iTypesList[m_iNumBehaviorTypes] = mp_whatSpeciesTypeCombos[i].iType;
      m_iNumBehaviorTypes++;
    }
  } //end of for (i = 0; i < m_iNumSpeciesTypeCombos; i++)

  mp_iWhatBehaviorTypes = new short int[m_iNumBehaviorTypes];
  for ( i = 0; i < m_iNumBehaviorTypes; i++ )
    mp_iWhatBehaviorTypes[i] = p_iTypesList[i];

  delete[] p_iTypesList;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clNCIBAGrowth::SetNameData(std::string sNameString)
{
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("NCIBAGrowth") == 0 )
    {
      m_iGrowthMethod = diameter_auto;
    }
    else if (sNameString.compare("NCIBAGrowth diam only") == 0 )
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
      stcErr.sFunction = "clNCIBAGrowth::SetNameData" ;
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
    stcErr.sFunction = "clNCIBAGrowth::SetNameData" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// CalcGrowthValue
////////////////////////////////////////////////////////////////////////////
float clNCIBAGrowth::CalcDiameterGrowthValue( clTree * p_oTree, clTreePopulation * p_oPop, float fHeightGrowth )
{
  float fAmountDiamIncrease; //amount diameter increase

  //Get the tree's growth - it's already calculated
  p_oTree->GetValue( mp_iGrowthCodes[mp_iIndexes[p_oTree->GetSpecies()]] [p_oTree->GetType() - clTreePopulation::sapling],
      & fAmountDiamIncrease );

  return fAmountDiamIncrease;
}

////////////////////////////////////////////////////////////////////////////
// PreGrowthCalcs
////////////////////////////////////////////////////////////////////////////
void clNCIBAGrowth::PreGrowthCalcs( clTreePopulation * p_oPop )
{
  try
  {
    clTreeSearch * p_oNCITrees; //trees that this growth behavior applies to
    clAllometry * p_oAllom = p_oPop->GetAllometryObject();
    clTree * p_oTree; //a single tree we're working with
    char cQuery[75], //for searching for trees
         cQueryPiece[5]; //for assembling the search query
    float fCrowdingEffect, //tree's crowding effect
         fBA, //the basal area sum
         fSizeEffect, //tree's size effect
         fDbh, //tree's dbh
         fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(), fAmountDiamIncrease, //amount diameter increase
         fTempDiamIncrease; //amount diameter increase - intermediate
    int iIsDead; //whether a tree is dead
    short int iSpecies, iType, //type and species of a tree
         i, //loop counter
         iDeadCode; //tree's dead code

    //Do a type/species search on all the types and species
    strcpy( cQuery, "species=" );
    for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
    {
      sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
      strcat( cQuery, cQueryPiece );
    }
    sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
    strcat( cQuery, cQueryPiece );

    strcat( cQuery, "::type=" );
    for ( i = 0; i < m_iNumBehaviorTypes - 1; i++ )
    {
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

      if ( -1 != mp_iGrowthCodes[mp_iIndexes[iSpecies]] [iType - clTreePopulation::sapling] )
      {

        //Make sure tree's not dead
        iDeadCode = p_oPop->GetIntDataCode( "dead", p_oTree->GetSpecies(), p_oTree->GetType() );
        if ( -1 != iDeadCode )
        {
          p_oTree->GetValue( iDeadCode, & iIsDead );
        }
        else
          iIsDead = notdead;

        if ( notdead == iIsDead )
        {

          p_oTree->GetValue( p_oPop->GetDbhCode( iSpecies, iType ), & fDbh );

          //First calculate the pieces that have no DBH component and thus will
          //not change in our loop

          //Get basal area
          if (m_bUseOnlyLargerNeighbors)
            fBA = GetBasalAreasLargerNeighbors(p_oTree, p_oPop);
          else
            fBA = GetBasalAreasAllNeighbors(p_oTree, p_oPop);

          //Divide by divisor amount
          fBA /= m_fBADivisor;

          //To correctly compound growth over the number of years per timestep,
          //we have to loop over the number of years, re-calculating the parts
          //with DBH and incrementing the DBH each time
          fAmountDiamIncrease = 0;
          for ( i = 0; i < fNumberYearsPerTimestep; i++ )
          {

            fCrowdingEffect = exp(-mp_fCrowdingSlope[mp_iIndexes[iSpecies]] *
                              pow(pow(fDbh, mp_fGamma[mp_iIndexes[iSpecies]]) * fBA, mp_fCrowdingSteepness[mp_iIndexes[iSpecies]]));
            //Make sure it's between 0 and 1
            if ( fCrowdingEffect < 0 ) fCrowdingEffect = 0;
            if ( fCrowdingEffect > 1 ) fCrowdingEffect = 1;

            //Get the tree's size effect
            fSizeEffect = exp( -0.5 * pow( log( fDbh / mp_fX0[mp_iIndexes[iSpecies]] ) / mp_fXb[mp_iIndexes[iSpecies]], 2 ) );

            //Make sure it's bounded between 0 and 1
            if ( fSizeEffect < 0 ) fSizeEffect = 0;
            if ( fSizeEffect > 1 ) fSizeEffect = 1;

            //Calculate actual growth in cm/yr
            fTempDiamIncrease = mp_fMaxPotentialValue[mp_iIndexes[iSpecies]] * fSizeEffect * fCrowdingEffect;

            //Add it to the running total of diameter increase
            fAmountDiamIncrease += fTempDiamIncrease;

            //Increase the DBH for the next loop.  If this is a sapling,
            //convert to a dbh value from what would be a diam10 increase
            if ( clTreePopulation::sapling == iType )
            {
              fDbh += p_oAllom->ConvertDiam10ToDbh( fTempDiamIncrease, iSpecies );
            }
            else
            {
              fDbh += fTempDiamIncrease;
            }
          }

          //Assign the growth back to "Growth" and hold it
          p_oTree->SetValue( mp_iGrowthCodes[mp_iIndexes[iSpecies]] [iType - clTreePopulation::sapling], fAmountDiamIncrease );

        } //end of if (bIsNCITree)
      }

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
    stcErr.sFunction = "clNCIBAGrowth::PreCalcGrowth" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetBasalAreasLargerNeighbors
////////////////////////////////////////////////////////////////////////////
float clNCIBAGrowth::GetBasalAreasLargerNeighbors(clTree *p_oTarget, clTreePopulation *p_oPop)
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
    clTree * p_oNeighbor; //competing neighbor
    char cQuery[75]; //format search strings into this
    float fBASum = 0, //sum of all qualifying neighbor basal area
         fNeighDbh, //neighbor's dbh
         fTargetDbh, //target's dbh
         fTargetX, fTargetY; //holders for the target tree's X and Y location
    int iIsDead; //whether a neighbor is dead
    short int iNeighSpecies, iNeighType, //species and type for neighbor
         iTargetSpecies = p_oTarget->GetSpecies(), //target tree's species
         iTargetType = p_oTarget->GetType(), //target tree's type
         iDeadCode; //neighbor's dead code

    //Get the target tree's DBH
    p_oTarget->GetValue( p_oPop->GetDbhCode( iTargetSpecies, iTargetType ), & fTargetDbh );

    //Format the query to get all competing neighbors
    p_oTarget->GetValue( p_oPop->GetXCode( iTargetSpecies, iTargetType ), & fTargetX );
    p_oTarget->GetValue( p_oPop->GetYCode( iTargetSpecies, iTargetType ), & fTargetY );

    //Get all trees taller than seedlings within the max crowding radius -
    //seedlings don't compete
    sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", mp_fMaxCrowdingRadius[mp_iIndexes[iTargetSpecies]], "FROM x=", fTargetX,
         "y=", fTargetY, "::height=", m_fMinSaplingHeight );
    p_oAllNeighbors = p_oPop->Find( cQuery );

    //Loop through and get the basal area of each
    p_oNeighbor = p_oAllNeighbors->NextTree();

    while ( p_oNeighbor )
    {
      if ( p_oNeighbor != p_oTarget )
      {

        iNeighSpecies = p_oNeighbor->GetSpecies();
        iNeighType = p_oNeighbor->GetType();

        if ( clTreePopulation::seedling != iNeighType && clTreePopulation::snag != iNeighType)
        {

          //Get the neighbor's dbh
          p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fNeighDbh );

          if ( fNeighDbh >= mp_fMinimumNeighborDBH[iNeighSpecies] && fNeighDbh > fTargetDbh)
          {

            //Make sure the neighbor's not dead
            iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
            if ( -1 != iDeadCode )
            {
              p_oNeighbor->GetValue( iDeadCode, & iIsDead );
            }
            else
              iIsDead = notdead;

            if ( notdead == iIsDead )
            {

              //Add in basal area in square centimeters
              fBASum += pow((fNeighDbh * 0.5), 2) * M_PI;

            }
          }
        }
      }
      p_oNeighbor = p_oAllNeighbors->NextTree();
    }
    return fBASum;
}

////////////////////////////////////////////////////////////////////////////
// GetBasalAreasAllNeighbors
////////////////////////////////////////////////////////////////////////////
float clNCIBAGrowth::GetBasalAreasAllNeighbors(clTree *p_oTarget, clTreePopulation *p_oPop)
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
    clTree * p_oNeighbor; //competing neighbor
    char cQuery[75]; //format search strings into this
    float fBASum = 0, //sum of all qualifying neighbor basal area
         fNeighDbh, //neighbor's dbh
         fTargetDbh, //target's dbh
         fTargetX, fTargetY; //holders for the target tree's X and Y location
    int iIsDead; //whether a neighbor is dead
    short int iNeighSpecies, iNeighType, //species and type for neighbor
         iTargetSpecies = p_oTarget->GetSpecies(), //target tree's species
         iTargetType = p_oTarget->GetType(), //target tree's type
         iDeadCode; //neighbor's dead code

    //Get the target tree's DBH
    p_oTarget->GetValue( p_oPop->GetDbhCode( iTargetSpecies, iTargetType ), & fTargetDbh );

    //Format the query to get all competing neighbors
    p_oTarget->GetValue( p_oPop->GetXCode( iTargetSpecies, iTargetType ), & fTargetX );
    p_oTarget->GetValue( p_oPop->GetYCode( iTargetSpecies, iTargetType ), & fTargetY );

    //Get all trees taller than seedlings within the max crowding radius -
    //seedlings don't compete
    sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", mp_fMaxCrowdingRadius[mp_iIndexes[iTargetSpecies]], "FROM x=", fTargetX,
         "y=", fTargetY, "::height=", m_fMinSaplingHeight );
    p_oAllNeighbors = p_oPop->Find( cQuery );

    //Loop through and get the basal area of each
    p_oNeighbor = p_oAllNeighbors->NextTree();

    while ( p_oNeighbor )
    {
      if ( p_oNeighbor != p_oTarget )
      {

        iNeighSpecies = p_oNeighbor->GetSpecies();
        iNeighType = p_oNeighbor->GetType();

        if ( clTreePopulation::seedling != iNeighType && clTreePopulation::snag != iNeighType)
        {

          //Get the neighbor's dbh
          p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fNeighDbh );

          if ( fNeighDbh >= mp_fMinimumNeighborDBH[iNeighSpecies])
          {

            //Make sure the neighbor's not dead
            iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
            if ( -1 != iDeadCode )
            {
              p_oNeighbor->GetValue( iDeadCode, & iIsDead );
            }
            else
              iIsDead = notdead;

            if ( notdead == iIsDead )
            {

              //Add in basal area in square centimeters
              fBASum += pow((fNeighDbh * 0.5), 2) * M_PI;

            }
          }
        }
      }
      p_oNeighbor = p_oAllNeighbors->NextTree();
    }
    return fBASum;
}

