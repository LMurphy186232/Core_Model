//---------------------------------------------------------------------------
#include "WeibullClimateQuadratGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include "Allometry.h"
#include "Grid.h"
#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clWeibullClimateQuadratGrowth::clWeibullClimateQuadratGrowth( clSimManager * p_oSimManager ) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clGrowthBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "WeibullClimateQuadratgrowthshell";
    m_sXMLRoot = "WeibullClimateQuadratGrowth";

    //Null out our pointers
    mp_fCompC = NULL;
    mp_fCompD = NULL;
    mp_fMinimumNeighborDBH = NULL;
    mp_fPrecipA = NULL;
    mp_fPrecipB = NULL;
    mp_fPrecipC = NULL;
    mp_fTempA = NULL;
    mp_fTempB = NULL;
    mp_fTempC = NULL;
    mp_fMaxRG = NULL;
    mp_iIndexes = NULL;
    m_cQuery = NULL;
    mp_iGridGrowthCodes = NULL;

    mp_oGrid = NULL;
    m_fMaxCrowdingRadius = 0;
    m_fMinSaplingHeight = 0;
    m_iNumNeighCode = -1;

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
    stcErr.sFunction = "clWeibullClimateQuadratGrowth::clWeibullClimateQuadratGrowth" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clWeibullClimateQuadratGrowth::~clWeibullClimateQuadratGrowth()
{
  delete[] mp_fCompC;
  delete[] mp_fCompD;
  delete[] mp_fMinimumNeighborDBH;
  delete[] mp_fPrecipA;
  delete[] mp_fPrecipB;
  delete[] mp_fPrecipC;
  delete[] mp_fTempA;
  delete[] mp_fTempB;
  delete[] mp_fTempC;
  delete[] mp_fMaxRG;
  delete[] mp_iIndexes;
  delete[] m_cQuery;
  delete[] mp_iGridGrowthCodes;
}


////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateQuadratGrowth::ReadParameterFile(
    xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values
    short int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
        i; //loop counter

    m_fMinSaplingHeight = 50;

    //Get the minimum sapling height
    for ( i = 0; i < iNumTotalSpecies; i++ )
    {
      if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
      {
        m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
      }
    }

    //Make the list of indexes
    mp_iIndexes = new short int[iNumTotalSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //This alone is sized total number of species
    mp_fMinimumNeighborDBH = new float[iNumTotalSpecies];

    //The rest are sized number of species to which this behavior applies
    mp_fCompC = new float[m_iNumBehaviorSpecies];
    mp_fCompD = new float[m_iNumBehaviorSpecies];
    mp_fPrecipA = new float[m_iNumBehaviorSpecies];
    mp_fPrecipB = new float[m_iNumBehaviorSpecies];
    mp_fPrecipC = new float[m_iNumBehaviorSpecies];
    mp_fTempA = new float[m_iNumBehaviorSpecies];
    mp_fTempB = new float[m_iNumBehaviorSpecies];
    mp_fTempC = new float[m_iNumBehaviorSpecies];
    mp_fMaxRG = new float[m_iNumBehaviorSpecies];

    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Fill the variables

    //Maximum potential growth
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadMaxGrowth", "gr_wcqmgVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxRG[i] = p_fTempValues[i].val;

    //Max crowding radius
    FillSingleValue( p_oElement, "gr_weibClimQuadMaxNeighRad", &m_fMaxCrowdingRadius, true );

    //Minimum neighbor DBH
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadMinNeighDBH",
        "gr_wcqmndVal", mp_fMinimumNeighborDBH, p_oPop, true );

    //C
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadCompEffC",
        "gr_wcqcecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCompC[i] = p_fTempValues[i].val;

    //D
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadCompEffD",
        "gr_wcqcedVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCompD[i] = p_fTempValues[i].val;

    //temperature effect a
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadTempEffA",
        "gr_wcqteaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempA[i] = p_fTempValues[i].val;

    //temperature effect b
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadTempEffB",
        "gr_wcqtebVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempB[i] = p_fTempValues[i].val;

    //temperature effect c
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadTempEffC",
        "gr_wcqtecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempC[i] = p_fTempValues[i].val;

    //precipitation effect a
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadPrecipEffA",
        "gr_wcqpeaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipA[i] = p_fTempValues[i].val;

    //precipitation effect b
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadPrecipEffB",
        "gr_wcqpebVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipB[i] = p_fTempValues[i].val;

    //precipitation effect c
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimQuadPrecipEffC",
        "gr_wcqpecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipC[i] = p_fTempValues[i].val;

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
    stcErr.sFunction = "clWeibullClimateQuadratGrowth::ReadParameterFile" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateQuadratGrowth::ValidateData(clTreePopulation *p_oPop)
{
  try
  {
    int i, iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    //Make sure that the max radius of neighbor effects is > 0
    if ( m_fMaxCrowdingRadius <= 0 )
    {
      modelErr stcErr;
      stcErr.sMoreInfo = "All values for max crowding radius must be greater than 0.";
      throw( stcErr );
    }
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {

      //Make sure that the maximum growth for each species is > 0
      if ( mp_fMaxRG[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max potential growth must be greater than 0.";
        throw( stcErr );
      }

      //Make sure that the temp a is not 0
      if ( 0 == mp_fTempA[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Temperature effect A values cannot be 0.";
        throw( stcErr );
      }

      //Make sure that the precip a is not 0
      if ( 0 == mp_fPrecipA[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Precipitation effect A values cannot be 0.";
        throw( stcErr );
      }
    }

    for ( i = 0; i < iNumTotalSpecies; i++ )
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
    err.sFunction = "clWeibullClimateQuadratGrowth::ValidateData";
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
    stcErr.sFunction = "clWeibullClimateQuadratGrowth::ValidateData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateQuadratGrowth::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    FormatQuery( p_oPop );
    ReadParameterFile( p_oDoc, p_oPop );
    ValidateData( p_oPop );
    SetupGrid( p_oPop );
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
    stcErr.sFunction = "clWeibullClimateQuadratGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clWeibullClimateQuadratGrowth::SetNameData(std::string sNameString)
{
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("WeibullClimateQuadratGrowth") == 0 )
    {
      m_iGrowthMethod = diameter_auto;
    }
    else if (sNameString.compare("WeibullClimateQuadratGrowth diam only") == 0 )
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
      stcErr.sFunction = "clWeibullClimateQuadratGrowth::SetNameData" ;
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
    stcErr.sFunction = "clWeibullClimateQuadratGrowth::SetNameData" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// CalcGrowthValue
////////////////////////////////////////////////////////////////////////////
float clWeibullClimateQuadratGrowth::CalcDiameterGrowthValue( clTree * p_oTree, clTreePopulation * p_oPop, float fHeightGrowth )
{
  float fX, fY, fGrowth;

  //Get this tree's location
  p_oTree->GetValue(p_oPop->GetXCode(p_oTree->GetSpecies(), p_oTree->GetType()), &fX);
  p_oTree->GetValue(p_oPop->GetYCode(p_oTree->GetSpecies(), p_oTree->GetType()), &fY);
  mp_oGrid->GetValueAtPoint(fX, fY, mp_iGridGrowthCodes[p_oTree->GetSpecies()], &fGrowth);

  if (fGrowth < -1 ) {
    modelErr stcErr;
    stcErr.sMoreInfo = "Panic: expected growth not calculated.";
    stcErr.sFunction = "clWeibullClimateQuadratGrowth::CalcDiameterGrowthValue" ;
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }

  return fGrowth;
}

////////////////////////////////////////////////////////////////////////////
// PreGrowthCalcs
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateQuadratGrowth::PreGrowthCalcs( clTreePopulation * p_oPop )
{
  float *p_fTempEffect = new float[m_iNumBehaviorSpecies],
      *p_fPrecipEffect = new float[m_iNumBehaviorSpecies];
  try
  {
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    clTreeSearch * p_oNCITrees; //trees that this growth behavior applies to
    clTree * p_oTree; //a single tree we're working with
    float fCrowdingEffect, //tree's crowding effect
    fPlotTemp = p_oPlot->GetMeanAnnualTemp(), //Current plot temperature
    fPlotPrecip = p_oPlot->GetMeanAnnualPrecip(), //Current plot precipitation
    fGrowth,
    fX, fY,
    fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    int iIsDead, //whether a tree is dead
    iNumNeighbors;
    short int iSpecies, iType, //type and species of a tree
    iSpInd,
    iNumXCells = mp_oGrid->GetNumberXCells(),
    iNumYCells = mp_oGrid->GetNumberYCells(),
    iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
    i, iX, iY, iSp,
    iDeadCode; //tree's dead code

    //Calculate precipitation and temperature effect for all species
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fPrecipEffect[i] = exp(-0.5*pow(fabs(fPlotPrecip - mp_fPrecipC[i])/mp_fPrecipA[i], mp_fPrecipB[i]));
      p_fTempEffect[i] = exp(-0.5*pow(fabs(fPlotTemp - mp_fTempC[i])/mp_fTempA[i], mp_fTempB[i]));
    }

    //Reset the values in the growth grid to -1
    fGrowth = -1;
    iNumNeighbors = -1;
    for (iX = 0; iX < iNumXCells; iX++)
      for (iY = 0; iY < iNumYCells; iY++) {
        mp_oGrid->SetValueOfCell(iX, iY, m_iNumNeighCode, iNumNeighbors);
        for (iSp = 0; iSp < iNumTotalSpecies; iSp++)
          mp_oGrid->SetValueOfCell(iX, iY, mp_iGridGrowthCodes[iSp], fGrowth);
      }


    p_oNCITrees = p_oPop->Find( m_cQuery );
    p_oTree = p_oNCITrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      //Make sure tree's not dead
      iDeadCode = p_oPop->GetIntDataCode( "dead", iSpecies, iType );
      if ( -1 != iDeadCode ) {
        p_oTree->GetValue( iDeadCode, & iIsDead );
      } else
        iIsDead = notdead;

      if ( notdead == iIsDead )
      {

        //Has growth been calculated for this tree's cell?
        p_oTree->GetValue( p_oPop->GetXCode( iSpecies, iType ), & fX );
        p_oTree->GetValue( p_oPop->GetYCode( iSpecies, iType ), & fY );
        mp_oGrid->GetValueAtPoint( fX, fY, mp_iGridGrowthCodes[iSpecies], & fGrowth );
        if ( -1 == fGrowth ) {

          //Get the number of neighbors, if it has not already been gotten
          mp_oGrid->GetCellOfPoint(fX, fY, &iX, &iY);
          mp_oGrid->GetPointOfCell( iX, iY, & fX, & fY );
          mp_oGrid->GetValueOfCell(iX, iY, m_iNumNeighCode, &iNumNeighbors);
          if (iNumNeighbors < 0) {
            iNumNeighbors = GetNumNeighbors(fX, fY, p_oPop);
            mp_oGrid->SetValueOfCell(iX, iY, m_iNumNeighCode, iNumNeighbors);
          }

          iSpInd = mp_iIndexes[iSpecies];

          fCrowdingEffect = exp(-mp_fCompC[iSpInd] *
              pow(iNumNeighbors, mp_fCompD[iSpInd]));
          //Make sure it's between 0 and 1
          if ( fCrowdingEffect < 0 ) fCrowdingEffect = 0;
          if ( fCrowdingEffect > 1 ) fCrowdingEffect = 1;

          //Calculate actual growth in cm/yr
          fGrowth = mp_fMaxRG[iSpInd] *
              fCrowdingEffect *
              p_fPrecipEffect[iSpInd] *
              p_fTempEffect[iSpInd];

          //Make it time step growth
          fGrowth *= fNumberYearsPerTimestep;

          //Assign the growth back to the grid
          mp_oGrid->SetValueOfCell(iX, iY, mp_iGridGrowthCodes[iSpecies], fGrowth  );
        }
      }
      p_oTree = p_oNCITrees->NextTree();
    }

    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;

  }
  catch ( modelErr & err )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clWeibullClimateQuadratGrowth::PreCalcGrowth" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetNumNeighbors
////////////////////////////////////////////////////////////////////////////
int clWeibullClimateQuadratGrowth::GetNumNeighbors(float &fX, float &fY, clTreePopulation *p_oPop)
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  char cQuery[75]; //format search strings into this
  float fNeighDbh; //neighbor's dbh
  int iIsDead, //whether a neighbor is dead
  iNumNeighbors = 0;
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iDeadCode; //neighbor's dead code

  //Format the query to get all competing neighbors
  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=",
      m_fMaxCrowdingRadius, "FROM x=", fX,
      "y=", fY, "::height=", m_fMinSaplingHeight );
  p_oAllNeighbors = p_oPop->Find( cQuery );

  //Loop through and find all the bigger neighbors
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor ) {

    iNeighSpecies = p_oNeighbor->GetSpecies();
    iNeighType = p_oNeighbor->GetType();

    if ( clTreePopulation::seedling != iNeighType &&
        clTreePopulation::snag != iNeighType) {

      //Get the neighbor's dbh
      p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fNeighDbh );
      if ( fNeighDbh >= mp_fMinimumNeighborDBH[iNeighSpecies]) {

        //Make sure the neighbor's not dead
        iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
        if ( -1 != iDeadCode )  p_oNeighbor->GetValue( iDeadCode, & iIsDead );
        else iIsDead = notdead;

        if ( notdead == iIsDead ) iNumNeighbors++;
      }
    }

    p_oNeighbor = p_oAllNeighbors->NextTree();
  }
  return iNumNeighbors;
}

////////////////////////////////////////////////////////////////////////////
// FormatQuery
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateQuadratGrowth::FormatQuery(clTreePopulation *p_oPop)
{
  char * cQuery = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50],
      cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSeedling = false, bSapling = false, bAdult = false;

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    if (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::sapling)
      bSapling = true;
    else if (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::seedling)
      bSeedling = true;
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
  if ( bSeedling )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::seedling, "," );
    strcat( cQuery, cQueryPiece );
  }
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

/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clWeibullClimateQuadratGrowth::SetupGrid(clTreePopulation *p_oPop)
{
  std::stringstream sLabel;
  short int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
      i; //loop counter

  //Declare the arrays for our grid codes
  mp_iGridGrowthCodes = new short int[iNumTotalSpecies];

  //Check to see if this grid has already been created
  mp_oGrid = mp_oSimManager->GetGridObject("Weibull Climate Quadrat Growth");

  if (NULL == mp_oGrid) {

    //Create the grid with five float data members for each species
    mp_oGrid = mp_oSimManager->CreateGrid( "Weibull Climate Quadrat Growth",
        1,                //number of ints
        iNumTotalSpecies, //number of floats
        0,                //number of chars
        0);               //number of bools

    //Register the data member called "growth_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "growth_" << i;
      mp_iGridGrowthCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }

    //Register the data member called "num_neigh"
    m_iNumNeighCode = mp_oGrid->RegisterInt("num_neigh");
  }
  else {
    //Grid already exists - get the codes
    //Get the data member called "growth_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "growth_" << i;
      mp_iGridGrowthCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iGridGrowthCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clWeibullClimateQuadratGrowth::SetupGrid";
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel
            << "\" member of the \"Weibull Climate Quadrat Growth\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      m_iNumNeighCode = mp_oGrid->GetIntDataCode( "num_neigh" );
      if (-1 == m_iNumNeighCode) {
        modelErr stcErr;
        stcErr.sFunction = "clWeibullClimateQuadratGrowth::SetupGrid" ;
        stcErr.sMoreInfo = "Couldn't find the \"num_neigh\" member of the \"Weibull Climate Quadrat Growth\" grid.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }
  }

}
