//---------------------------------------------------------------------------
#include "LaggedPostHarvestGrowth.h"
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
clLaggedPostHarvestGrowth::clLaggedPostHarvestGrowth( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
  clGrowthBase( p_oSimManager )
  {
  try
  {
    //Set namestring
    m_sNameString = "laggedpostharvestgrowthshell";
    m_sXMLRoot = "LaggedPostHarvestGrowth";

    //Indicate that this behavior intends to add 2 float tree data members
    //(overwritten from GrowthBase)
    m_iNewTreeFloats = 2; //pre-harvest growth + current growth

    //Null out our pointers
    mp_iGrowthCodes = NULL;
    mp_iPreHarvGrowthCodes = NULL;
    mp_iWhatBehaviorTypes = NULL;
    mp_fMaxGrowthConstant = NULL;
    mp_fMaxGrowthDbhEffect = NULL;
    mp_fNciConstant = NULL;
    mp_fNciDbhEffect = NULL;
    mp_fTimeSinceHarvestRateParam = NULL;
    mp_iIndexes = NULL;

    //Version 1
    m_fVersionNumber = 1.0;
    m_fMinimumVersionNumber = 1.0;

   mp_oTimeSinceHarvestGrid = NULL;
   mp_oHarvestResultsGrid = NULL;
   m_iNumBehaviorTypes = 0;
   m_iNumXCells = 0;
   m_iNumYCells = 0;
   m_iNumTotalSpecies = 0;
   m_iHarvestTypeCode = 0;
   m_iTimeCode = 0;
   m_iLastUpdated = 0;
   m_fNumberYearsPerTimestep = 0;
   m_fNciDistanceRadius = 0;

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
    stcErr.sFunction = "clLaggedPostHarvestGrowth::clLaggedPostHarvestGrowth" ;
    throw( stcErr );
  }
}



////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clLaggedPostHarvestGrowth::~clLaggedPostHarvestGrowth()
{

  int i;

  if ( mp_iGrowthCodes )
  {
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iGrowthCodes[i];
    }
    delete[] mp_iGrowthCodes;

  }

  if (mp_iPreHarvGrowthCodes) {
    for (i = 0; i < m_iNumTotalSpecies; i++)
    {
      delete[] mp_iPreHarvGrowthCodes[i];
    }
    delete[] mp_iPreHarvGrowthCodes;
  }


  delete[] mp_iWhatBehaviorTypes;
  delete[] mp_fMaxGrowthConstant;
  delete[] mp_fMaxGrowthDbhEffect;
  delete[] mp_fNciConstant;
  delete[] mp_fNciDbhEffect;
  delete[] mp_fTimeSinceHarvestRateParam;
  delete[] mp_iIndexes;

}



////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clLaggedPostHarvestGrowth::ReadParameterFile( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values

    short int i; //loop counters

    mp_iIndexes = new short int[m_iNumTotalSpecies];

    //The rest are sized number of species to which this behavior applies
    mp_fMaxGrowthConstant = new float[m_iNumBehaviorSpecies];
    mp_fMaxGrowthDbhEffect = new float[m_iNumBehaviorSpecies];
    mp_fNciConstant = new float[m_iNumBehaviorSpecies];
    mp_fNciDbhEffect = new float[m_iNumBehaviorSpecies];
    mp_fTimeSinceHarvestRateParam = new float[m_iNumBehaviorSpecies];


    //If any of the types is seedling, error out
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
          && clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clLaggedPostHarvestGrowth::ReadParameterFile" ;
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

    //Multiplier constant in Max potential growth term
    FillSpeciesSpecificValue( p_oElement, "gr_lagMaxGrowthConstant", "gr_lmgcVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxGrowthConstant[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Effect of DBH on max potential growth term
    FillSpeciesSpecificValue( p_oElement, "gr_lagMaxGrowthDbhEffect", "gr_lmgdbheVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxGrowthDbhEffect[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Effect of NCI on growth
    FillSpeciesSpecificValue( p_oElement, "gr_lagNciConstant", "gr_lncicVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fNciConstant[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Effect of DBH on NCI term
    FillSpeciesSpecificValue( p_oElement, "gr_lagNciDbhEffect", "gr_lncidbheVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fNciDbhEffect[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Rate parameter for time since harvest lag effect
    FillSpeciesSpecificValue( p_oElement, "gr_lagTimeSinceHarvestRateParam", "gr_ltshrpVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTimeSinceHarvestRateParam[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //NCI radius
    FillSingleValue( p_oElement, "gr_lagNciDistanceRadius", & m_fNciDistanceRadius, true );


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
    stcErr.sFunction = "clLaggedPostHarvestGrowth::ReadParameterFile" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clLaggedPostHarvestGrowth::ValidateData()
{
  try
  {

    int i;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {

      //Make sure that the max growth constant is >= 0
      if ( mp_fMaxGrowthConstant[i] < 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for maximum growth constant must be non-negative.";
        throw( stcErr );
      }

      //Make sure that the max growth dbh effect is >=0
      if ( mp_fMaxGrowthDbhEffect[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for maximum growth dbh effect must be non-negative.";
        throw( stcErr );
      }

      //Make sure that the NCI constant is >=0
      if ( mp_fNciConstant[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for NCI constant must be non-negative.";
        throw( stcErr );
      }

      //Make sure that the NCI dbh effect is >=0
      if ( mp_fNciDbhEffect[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for NCI dbh effect must be non-negative.";
        throw( stcErr );
      }

      //Make sure that the time since harvest decay effect is >=0
      if ( mp_fTimeSinceHarvestRateParam[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Time since harvest rate parameter must be greater than 0.";
        throw( stcErr );
      }

    } //end for

    //Make sure that NCI distance radius is > 0
    if ( m_fNciDistanceRadius <= 0)
    {
      modelErr stcErr;
      stcErr.sMoreInfo = "The NCI distance radius must be greater than 0.";
      throw( stcErr );
    }


  }
  catch ( modelErr & err )
  {
    err.sFunction = "clLaggedPostHarvestGrowth::ValidateData";
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
    stcErr.sFunction = "clLaggedPostHarvestGrowth::ValidateData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetTreeMemberCodes()
////////////////////////////////////////////////////////////////////////////
void clLaggedPostHarvestGrowth::GetTreeMemberCodes()
{

  int i, j; //loop counters

  //Get codes for growth
  mp_iGrowthCodes = new short int * [m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    mp_iGrowthCodes[i] = new short int[2];
    for (j = 0; j < 2; j++) {
      mp_iGrowthCodes[i][j] = -1;
    }
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
void clLaggedPostHarvestGrowth::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {

    clTreePopulation *p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    AssembleUniqueTypes();

    ReadParameterFile( p_oDoc );
    ValidateData();

    GetTreeMemberCodes();

    SetupTimeSinceHarvestGrid();

    //Get the expected growth values at the start in case there's harvesting in the first timestep
    PreGrowthCalcs(p_oPop);

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
    stcErr.sFunction = "clLaggedPostHarvestGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}




////////////////////////////////////////////////////////////////////////////
// AssembleUniqueTypes
////////////////////////////////////////////////////////////////////////////
void clLaggedPostHarvestGrowth::AssembleUniqueTypes()
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
void clLaggedPostHarvestGrowth::SetNameData(std::string sNameString)
{
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("LaggedPostHarvestGrowth") == 0 )
    {
      m_iGrowthMethod = diameter_auto;
    }
    else if (sNameString.compare("LaggedPostHarvestGrowth diam only") == 0 )
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
      stcErr.sFunction = "clLaggedPostHarvestGrowth::SetNameData" ;
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
    stcErr.sFunction = "clLaggedPostHarvestGrowth::SetNameData" ;
    throw( stcErr );
  }
}



////////////////////////////////////////////////////////////////////////////
// CalcDiameterGrowthValue
////////////////////////////////////////////////////////////////////////////
float clLaggedPostHarvestGrowth::CalcDiameterGrowthValue( clTree * p_oTree, clTreePopulation * p_oPop, float fHeightGrowth ) {
  float fAmountDiamIncrease; //amount diameter increase

  //Get the tree's growth - it's already calculated
  p_oTree->GetValue( mp_iGrowthCodes[mp_iIndexes[p_oTree->GetSpecies()]] [p_oTree->GetType() - clTreePopulation::sapling],
      & fAmountDiamIncrease );

  return fAmountDiamIncrease;
}

////////////////////////////////////////////////////////////////////////////
// PreGrowthCalcs
////////////////////////////////////////////////////////////////////////////
void clLaggedPostHarvestGrowth::PreGrowthCalcs( clTreePopulation * p_oPop )
{
  try
  {
    clTreeSearch * p_oTrees; //trees that this growth behavior applies to
    clTree * p_oTree; //a single tree we're working with

    char cQuery[75], //for searching for trees
    cQueryPiece[5]; //for assembling the search query
    float

    fLocalNCI, //local NCI
    fDbh, //tree's DBH
    fMaxGrowth, //tree's max potential growth value
    fPreHarvestGrowth, //last growth before harvest
    fPostHarvestGrowth, //asymptotic growth after harvest
    fAmountDiamIncrease; //amount diameter increase

    short int iSpecies, iType, //type and species of a tree
    i, //loop counter
    iDeadCode, //tree's dead code
    iTimeSinceHarvest; //years since last harvest at a tree's location

    bool bIsDead; //whether a tree is dead


    //Update the Years Since Harvest grid, if it hasn't already been updated by another behavior.
    CalcTimeSinceHarvest();


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

    p_oTrees = p_oPop->Find( cQuery );

    //************************************
    // Loop through and calculate growth for each tree
    //************************************
    p_oTree = p_oTrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      //if this growth behavior applies to the species and type of the current tree
      if ( -1 != mp_iGrowthCodes[mp_iIndexes[iSpecies]] [iType - clTreePopulation::sapling] )
      {

        //Make sure tree's not dead
        iDeadCode = p_oPop->GetBoolDataCode( "dead", p_oTree->GetSpecies(), p_oTree->GetType() );
        if ( -1 != iDeadCode )
        {
          p_oTree->GetValue( iDeadCode, & bIsDead );
        }
        else
          bIsDead = false;

        if ( !bIsDead )
        {

          //Get values for DBH, NCI and TSH state variables.

          p_oTree->GetValue(p_oPop->GetDbhCode(iSpecies, iType), &fDbh);

          fLocalNCI = LocalBasalAreaAroundTree(p_oTree) * exp(-1.0 * mp_fNciDbhEffect[mp_iIndexes[iSpecies]] * fDbh);

          iTimeSinceHarvest = GetTimeSinceHarvest(p_oTree);

          fMaxGrowth = mp_fMaxGrowthConstant[mp_iIndexes[iSpecies]] *
          exp(-1.0 * mp_fMaxGrowthDbhEffect[mp_iIndexes[iSpecies]] * fDbh);


          //post-harvest growth model equation
          fPostHarvestGrowth = fMaxGrowth * exp(-1.0 * mp_fNciConstant[mp_iIndexes[iSpecies]] * fLocalNCI);

          if (iTimeSinceHarvest >= 1000) { //no harvest yet

            fAmountDiamIncrease = fPostHarvestGrowth;
          } // end if (no harvest)

          else if (iTimeSinceHarvest == 0) { //harvest was this timestep
            //Assign the previous timestep's "Growth" value to pre-harvest growth
            p_oTree->GetValue( mp_iGrowthCodes[mp_iIndexes[iSpecies]][iType - clTreePopulation::sapling], &fPreHarvestGrowth );
            p_oTree->SetValue(mp_iPreHarvGrowthCodes[iSpecies][iType], fPreHarvestGrowth);

            //immediately after harvest, we apply the pre-harvest growth rate (same as formula below for tsh=0)
            fAmountDiamIncrease = fPreHarvestGrowth;

          } //end else if (harvest this timestep)

          else { //harvest was in an earlier timestep

            //retrieve the pre-harvest growth state variable
            p_oTree->GetValue(mp_iPreHarvGrowthCodes[iSpecies][iType], &fPreHarvestGrowth);
            //ramp up to post-harvest growth slowly
            fAmountDiamIncrease = fPreHarvestGrowth + (fPostHarvestGrowth - fPreHarvestGrowth) *
            (1.0 - exp(-1.0 * mp_fTimeSinceHarvestRateParam[mp_iIndexes[iSpecies]] * iTimeSinceHarvest));

          } //end else (harvest earlier)

          //convert from mm radius to cm diameter
          fAmountDiamIncrease *= 2.0/10.0;
          //Multiply by the number of years per timestep
          fAmountDiamIncrease *= m_fNumberYearsPerTimestep;

          //Prevent negative growth
          if (fAmountDiamIncrease < 0.0)
            fAmountDiamIncrease = 0.0;

          //Assign the growth back to "Growth" and hold it
          p_oTree->SetValue( mp_iGrowthCodes[mp_iIndexes[iSpecies]][iType - clTreePopulation::sapling], fAmountDiamIncrease );

        } //end of if (!bIsDead)
      }

      p_oTree = p_oTrees->NextTree();
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
    stcErr.sFunction = "clLaggedPostHarvestGrowth::PreCalcGrowth" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// LocalBasalAreaAroundTree()
////////////////////////////////////////////////////////////////////////////
float clLaggedPostHarvestGrowth::LocalBasalAreaAroundTree( clTree *p_oTree ) {

  clTreePopulation *p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

  clTreeSearch * p_oCloseTrees; //search object for accessing neighboring trees
  clTree * p_oNeighborTree; //one individual tree to work with

  float fTreeX, fTreeY, //the target tree's location
  fDbh,
  fBasalArea= 0.0; //NCI within crowding radius

  int iSp = p_oTree->GetSpecies(),
      iTp = p_oTree->GetType();

  char cQuery[70];

  p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fTreeX);
  p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fTreeY);


  sprintf(cQuery, "distance=%4.4f FROM x=%4.4f,y=%4.4f::height=1.35", m_fNciDistanceRadius, fTreeX, fTreeY);

  p_oCloseTrees = p_oPop->Find(cQuery);

  p_oNeighborTree = p_oCloseTrees->NextTree();

  while (p_oNeighborTree) { //loop through trees within crowding radius

    if ((p_oNeighborTree != p_oTree) && (p_oNeighborTree->GetType() == clTreePopulation::adult)) {
      //if the tree is an adult and isn't the focal tree itself

      //add it to the calculated NCI
      p_oNeighborTree->GetValue(p_oPop->GetDbhCode(p_oNeighborTree->GetSpecies(), p_oNeighborTree->GetType()), &fDbh);
      fBasalArea += clModelMath::CalculateBasalArea(fDbh);

    } //end if (

    p_oNeighborTree = p_oCloseTrees->NextTree();

  } //end while

  return fBasalArea;
}


////////////////////////////////////////////////////////////////////////////
// CalcTimeSinceHarvest()
////////////////////////////////////////////////////////////////////////////
void clLaggedPostHarvestGrowth::CalcTimeSinceHarvest() {
  try {


    int i, j,
    iHarvestedThisTimestep, //a value other than -1 if a grid cell was harvested this timestep
    iNewTimeSinceHarvest, //updated time since harvest
    iTimestepLength = (int)mp_oSimManager->GetNumberOfYearsPerTimestep(),
    iCurrentTimestep = (int)mp_oSimManager->GetCurrentTimestep(),
    iLastUpdated;


    if (mp_oHarvestResultsGrid) { //if there is cutting in this simulation

      mp_oTimeSinceHarvestGrid->GetValueOfCell( 0, 0, m_iLastUpdated, &iLastUpdated);

      if (iLastUpdated < iCurrentTimestep) { //if the grid hasn't yet been updated

        //traverse through cells of Harvest Results grid and update Harvest Type value as needed
        for(i=0; i<m_iNumXCells; i++)
          for(j=0; j<m_iNumYCells; j++)
          {
            //Get the harvest type value from the grid cell
            mp_oHarvestResultsGrid->GetValueOfCell(i, j, m_iHarvestTypeCode, &iHarvestedThisTimestep);

            //if the value is not -1, then harvesting occurred so set TimeSinceHarvest to 0
            if(iHarvestedThisTimestep != -1)
            {
              mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTimeCode, 0 );

            }//end if
            //Otherwise increment TimeSinceHarvest by the length of a timestep
            else
            {
              mp_oTimeSinceHarvestGrid->GetValueOfCell( i, j, m_iTimeCode, &iNewTimeSinceHarvest);
              iNewTimeSinceHarvest += iTimestepLength;
              mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTimeCode, iNewTimeSinceHarvest);
            } //end else
          } //end for (loop through grid cells)

        mp_oTimeSinceHarvestGrid->SetValueOfCell( 0, 0, m_iLastUpdated, iCurrentTimestep);

      } //end if grid not yet updated

    } //end if harvest results exists

  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clLaggedPostHarvestGrowth::CalcTimeSinceHarvest";
    throw(stcErr);
  }
}



////////////////////////////////////////////////////////////////////////////
// SetupTimeSinceHarvestGrid()
////////////////////////////////////////////////////////////////////////////

void clLaggedPostHarvestGrowth::SetupTimeSinceHarvestGrid() {
  try {

    int i, j;

    clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;
    float fCellLength = p_oPop->GetGridCellSize();


    //check if the grid alread exists

    mp_oTimeSinceHarvestGrid = mp_oSimManager->GetGridObject( "Years Since Last Harvest" );

    if (!mp_oTimeSinceHarvestGrid) {
      //if it doesn't, then create it
      mp_oTimeSinceHarvestGrid = mp_oSimManager->CreateGrid("Years Since Last Harvest", //grid name
          2, //number of int data members
          0, //number of float data members
          0, //number of char data members
          0, //number of bool data members
          fCellLength, //Number of X cells
          fCellLength); //Number of Y cells


      //register the data members
      m_iTimeCode = mp_oTimeSinceHarvestGrid->RegisterInt( "Time" );
      m_iLastUpdated = mp_oTimeSinceHarvestGrid->RegisterInt( "LastUpdated" );

      //figure out its size
      m_iNumXCells = mp_oTimeSinceHarvestGrid->GetNumberXCells(),
      m_iNumYCells = mp_oTimeSinceHarvestGrid->GetNumberYCells();


      //initialize the TimeSinceHarvestGrid with values of 1000 (an arbitrarily long period of time)
      for(i=0; i<m_iNumXCells; i++)
        for(j=0; j<m_iNumYCells; j++) {
          mp_oTimeSinceHarvestGrid->SetValueOfCell( i, j, m_iTimeCode, 1000);

        }
    }

    else {

      //the grid has already been created

      //get the data member codes
      m_iTimeCode = mp_oTimeSinceHarvestGrid->GetIntDataCode("Time");
      m_iLastUpdated = mp_oTimeSinceHarvestGrid->GetIntDataCode("LastUpdated");

      //figure out its size
      m_iNumXCells = mp_oTimeSinceHarvestGrid->GetNumberXCells(),
      m_iNumYCells = mp_oTimeSinceHarvestGrid->GetNumberYCells();

    }

    //Get a pointer to the harvest results grid
    mp_oHarvestResultsGrid = mp_oSimManager->GetGridObject("Harvest Results");

    //if harvest results grid exists, get the code for harvest type
    if (mp_oHarvestResultsGrid)
      m_iHarvestTypeCode = mp_oHarvestResultsGrid->GetPackageIntDataCode("Harvest Type");

  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clLaggedPostHarvestGrowth::SetupTimeSinceHarvestGrid";
    throw(stcErr);
  }
}



////////////////////////////////////////////////////////////////////////////
// GetTimeSinceHarvest()
////////////////////////////////////////////////////////////////////////////

int clLaggedPostHarvestGrowth::GetTimeSinceHarvest( clTree *p_oTree ) {

  clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;

  float fTreeX, fTreeY; //the target tree's location

  int iSp = p_oTree->GetSpecies(),
      iTp = p_oTree->GetType(),
      iTime;

  p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fTreeX);
  p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fTreeY);


  if (!mp_oTimeSinceHarvestGrid)

    return 1000;

  else {

    mp_oTimeSinceHarvestGrid->GetValueAtPoint(fTreeX, fTreeY, m_iTimeCode, &iTime);

    return iTime;

  }
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clLaggedPostHarvestGrowth::RegisterTreeDataMembers()
{

  clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;
  char cLabel[] = "PreHarvGr";
  short int i,j;

  //Call the base class version as well
  clGrowthBase::RegisterTreeDataMembers();

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data members
  mp_iPreHarvGrowthCodes = new short int * [m_iNumTotalSpecies];

  for (i=0;i<m_iNumTotalSpecies;i++) {
    mp_iPreHarvGrowthCodes[i] = new short int [clTreePopulation::adult+1];

    for (j = 0; j < clTreePopulation::adult+1; j++) {
      mp_iPreHarvGrowthCodes[i][j] = -1;
    }

  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {


    //Make sure that only allowed types have been applied
    if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType &&
        clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType)
    {

      modelErr stcErr;
      stcErr.sFunction = "clSnagDecomp::RegisterTreeDataMembers" ;
      stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Register the pre harvest growth data member

    mp_iPreHarvGrowthCodes[mp_whatSpeciesTypeCombos[i].iSpecies][mp_whatSpeciesTypeCombos[i].iType] =
      p_oPop->RegisterFloat( cLabel, mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

  }  //end for combos
}
