//---------------------------------------------------------------------------
// LightDepSeedSurvival.cpp
//---------------------------------------------------------------------------
#include "LightDepSeedSurvival.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "LightOrg.h"
#include "Plot.h"
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clLightDepSeedSurvival::clLightDepSeedSurvival( clSimManager * p_oSimManager ) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
     clLightBase( p_oSimManager), clGLIBase( p_oSimManager )
{
  try
  {
    m_sNameString = "LightDependentSeedSurvival";
    m_sXMLRoot = "LightDependentSeedSurvival";

    mp_fLightExtCoeff = NULL;
    mp_fOptimumGLI = NULL;
    mp_fLowGLISlope = NULL;
    mp_fHighGLISlope = NULL;
    mp_iSeedGridCode = NULL;
    mp_iDamageCodes = NULL;
    mp_iIndexes = NULL;

    mp_oStormLightGrid = NULL;

    //Versions
    m_fVersionNumber = 1.1;
    m_fMinimumVersionNumber = 1;

    m_fMaxSearchDistance = 0;
    m_fLightHeight = 0;
    mp_oSeedGrid = NULL;
    m_bUseStormLight = false;
    m_iNumTotalSpecies = 0;
    m_iLightCode = -1;
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
    stcErr.sFunction = "clLightDepSeedSurvival::clLightDepSeedSurvival" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clLightDepSeedSurvival::~clLightDepSeedSurvival()
{
  int i;
  if ( mp_fLightExtCoeff )
  {
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      delete[] mp_fLightExtCoeff[i];
    }
  }
  if ( mp_iDamageCodes )
  {
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iDamageCodes[i];
  }
  delete[] mp_iDamageCodes;
  delete[] mp_fLightExtCoeff;
  delete[] mp_fOptimumGLI;
  delete[] mp_fLowGLISlope;
  delete[] mp_fHighGLISlope;
  delete[] mp_iSeedGridCode;
  delete[] mp_iIndexes;
}

///////////////////////////////////////////////////////////////////////////////
// GetParameterFileData
/////////////////////////////////////////////////////////////////////////////
void clLightDepSeedSurvival::GetParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  floatVal * p_fTemp = NULL; //for getting species-specific values
  float * p_fTempAll = NULL;;
  try {
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int i, j;

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  //Set up our temp array - pre-load with this behavior's species
  p_fTemp = new floatVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTemp[i].code = mp_iWhatSpecies[i];

  //Set up our indexes array and load
  mp_iIndexes = new short int[p_oPop->GetNumberOfSpecies()];
  //Load the indexes array
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_iIndexes[mp_iWhatSpecies[i]] = i;

  //*******************
  // Parameters used for both GLI situations
  //*******************
  //Declare arrays
  mp_fOptimumGLI = new float[m_iNumBehaviorSpecies];
  mp_fLowGLISlope = new float[m_iNumBehaviorSpecies];
  mp_fHighGLISlope = new float[m_iNumBehaviorSpecies];

  //Optimum GLI
  FillSpeciesSpecificValue( p_oElement, "es_optimumGLI", "es_ogVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
  //Now transfer the values to the permanent array
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fOptimumGLI[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

  //Slope of decreased light favorability below optimum GLI
  FillSpeciesSpecificValue( p_oElement, "es_lowSlope", "es_lsVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
  //Now transfer the values to the permanent array
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fLowGLISlope[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

  //Slope of decreased light favorability above optimum GLI
  FillSpeciesSpecificValue( p_oElement, "es_highSlope", "es_hsVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
  //Now transfer the values to the permanent array
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fHighGLISlope[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

  //Make sure the optimum GLI values are between 0 and 100
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    if ( mp_fOptimumGLI[i] < 0 || mp_fOptimumGLI[i] > 100 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clLightDepSeedSurvival::GetParameterFileData" ;
      stcErr.sMoreInfo = "Optimum GLI must be between 0 and 100.";
      throw( stcErr );
    }
  }


  //*******************
  // Parameters used if this behavior is doing GLI calculations
  //*******************
  if ( !m_bUseStormLight )
  {

    p_fTempAll = new float[m_iNumTotalSpecies];

    //Declare arrays
    mp_fLightExtCoeff = new float * [m_iNumTotalSpecies];
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      mp_fLightExtCoeff[i] = new float[3];
    }

    FillSingleValue( p_oElement, "li_numAltGrids", & m_iNumAltAng, true );
    //Number of azimuth angles
    FillSingleValue( p_oElement, "li_numAziGrids", & m_iNumAziAng, true );
    //Minimum sun angle
    FillSingleValue( p_oElement, "li_minSunAngle", & m_fMinSunAngle, true );

    //Light extinction coefficient for undamaged trees
    //Use the standard light extinction coefficient tags - this lets clLightOrg
    //find this tag as its required light extinction coefficient stuff so it can
    //perform its setup
    FillSpeciesSpecificValue( p_oElement, "li_lightExtinctionCoefficient", "li_lecVal", p_fTempAll, p_oPop, true );
    //Transfer to array
    //The other two light extinction coefficients aren't required - default them
    //to the undamaged light extinction coefficients
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      mp_fLightExtCoeff[i] [0] = p_fTempAll[i];
      mp_fLightExtCoeff[i] [1] = p_fTempAll[i];
      mp_fLightExtCoeff[i] [2] = p_fTempAll[i];
    }

    //Light extinction coefficient for fully damaged trees - not required
    FillSpeciesSpecificValue( p_oElement, "es_lightExtCoeffFullDmg", "es_lecfdVal", p_fTempAll, p_oPop, false );
    //Transfer to array
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      mp_fLightExtCoeff[i] [2] = p_fTempAll[i];
    }

    //Light extinction coefficient for partially damaged trees - not required
    FillSpeciesSpecificValue( p_oElement, "es_lightExtCoeffPartDmg", "es_lecpdVal", p_fTempAll, p_oPop, false );
    //Transfer to array
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      mp_fLightExtCoeff[i] [1] = p_fTempAll[i];
    }

    //Height of light calculations
    FillSingleValue( p_oElement, "es_lightHeight", & m_fLightHeight, true );

    //Validation

    //Make sure all of the light extinction coefficient values are between 0 and 1
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      for ( j = 0; j < 3; j++ )
      {
        if ( mp_fLightExtCoeff[i] [j] < 0 || mp_fLightExtCoeff[i] [j] > 1 )
        {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clLightDepSeedSurvival::GetParameterFileData" ;
          stcErr.sMoreInfo = "All light extinction coefficients must be between 0 and 1.";
          throw( stcErr );
        }
      }
    }

    //Make sure the height of the light calculation is not negative
    if ( m_fLightHeight < 0 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clLightDepSeedSurvival::GetParameterFileData" ;
      stcErr.sMoreInfo = "Height of light calculations must be greater than 0.";
      throw( stcErr );
    }

    //Make sure the value for m_iNumAltAng and m_iNumAziAng is greater than 0
    if ( m_iNumAltAng < 1 || m_iNumAziAng < 1 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clLightDepSeedSurvival::GetParameterFileData" ;
      stcErr.sMoreInfo = "Number of altitude and azimuth sky divisions must be at least 1.";
      throw( stcErr );

    }

    delete[] p_fTempAll;
  }

  delete[] p_fTemp;

  }
  catch ( modelErr & err )
  {
    delete[] p_fTempAll;
    delete[] p_fTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTempAll;
    delete[] p_fTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempAll;
    delete[] p_fTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clLightDepSeedSurvival::GetParameterFileData" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clLightDepSeedSurvival::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {

    //Make sure the light org object got a chance to do setup, which it
    //wouldn't have if this is the only light behavior around
    if (!m_bUseStormLight) {
      if ( mp_oLightOrg->GetMaxTreeHeight() <= 0 )
      {
        mp_oLightOrg->DoSetup( mp_oSimManager, p_oDoc );
      }
    }

    GetParameterFileData( p_oDoc );
    GetTreeDataMemberCodes();
    SetupGrid();
    DoLightSetup();
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
    stcErr.sFunction = "clLightDepSeedSurvival::GetData" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// DoLightSetup
/////////////////////////////////////////////////////////////////////////////
void clLightDepSeedSurvival::DoLightSetup()
{
  if (m_bUseStormLight) return;

  try
  {
    float fAngChunk;
    int i, j, iHalfAzi; //for partitioning the sky hemisphere

    //Populate the sky brightness and photo arrays

    //Declare 'em
    mp_fBrightness = new float * [m_iNumAltAng];
    mp_fPhoto = new float * [m_iNumAltAng];
    for ( i = 0; i < m_iNumAltAng; i++ )
    {
      mp_fBrightness[i] = new float[m_iNumAziAng];
      mp_fPhoto[i] = new float[m_iNumAziAng];
      for ( j = 0; j < m_iNumAziAng; j++ )
        mp_fBrightness[i] [j] = 0;
    }

    m_fSinMinSunAng = sin( m_fMinSunAngle );
    m_iMinAngRow = (int)floor( m_fSinMinSunAng * m_iNumAltAng );

    PopulateGLIBrightnessArray();

    m_fAziChunkConverter = m_iNumAziAng / 360.0; //force float conversion
    m_fRcpTanMinAng = 1 / ( tan( m_fMinSunAngle ) );

    mp_fAziSlope = new float[m_iNumAziAng];
    iHalfAzi = m_iNumAziAng / 2;
    //Get the size of each azimuth chunk in radians
    fAngChunk = ( 2.0 * M_PI ) / m_iNumAziAng;
    for ( i = 0; i < iHalfAzi; i++ )
    {
      //slope = tan of azimuth angle
      mp_fAziSlope[i] = 1 / ( tan( fAngChunk * ( i + 0.5 ) ) );
      mp_fAziSlope[i + iHalfAzi] = mp_fAziSlope[i];
    }

    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    m_fMaxSearchDistance = ( mp_oLightOrg->GetMaxTreeHeight() - m_fLightHeight ) * m_fRcpTanMinAng
         + p_oPop->GetAllometryObject()->GetMaxCrownRadius();
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
    stcErr.sFunction = "clLightDepSeedSurvival::DoLightSetup" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetGLI()
/////////////////////////////////////////////////////////////////////////////
float clLightDepSeedSurvival::GetGLI( clTreePopulation * p_oPop, const float & fX, const float & fY )
{
  float fGli; //global light index - end result of all this math

  if (m_bUseStormLight) {
    mp_oStormLightGrid->GetValueAtPoint(fX, fY, m_iLightCode, &fGli);
    return fGli;
  }

  clTreeSearch * p_oShaders; //neighborhood trees who can shade each target
  clTree * p_oNeighbor; //shading neighbors
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  clAllometry * p_oAllom = p_oPop->GetAllometryObject();
  char cQuery[75]; //format search strings into this

  int i, j;

  //Initialize photo array
  for ( i = 0; i < m_iNumAltAng; i++ )
    for ( j = 0; j < m_iNumAziAng; j++ )
      mp_fPhoto[i] [j] = 1.0;

  //Get a list of all trees that are within the search radius and taller than
  //the fish-eye photo height
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fMaxSearchDistance, " FROM x=", fX, "y=", fY,
      "::height=", m_fLightHeight );
  p_oShaders = p_oPop->Find( cQuery );

  p_oNeighbor = p_oShaders->NextTree();
  while ( p_oNeighbor != NULL )
  {

    //Skip seedlings, which don't shade
    if ( clTreePopulation::seedling != p_oNeighbor->GetType() )

      //Add the effect of the neighbor to the simulated fisheye photo
      AddTreeToGliFishEye( fX, fY, m_fLightHeight, p_oNeighbor, p_oPlot, p_oPop, p_oAllom );

    p_oNeighbor = p_oShaders->NextTree();
  } //end of while (neighbor != NULL)

  //Calculate GLI
  fGli = 0.0;
  for ( i = 0; i < m_iNumAltAng; i++ )
    for ( j = 0; j < m_iNumAziAng; j++ )
      fGli += ( mp_fPhoto[i] [j] * mp_fBrightness[i] [j] );
  fGli *= 100;

  return fGli;
}

///////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clLightDepSeedSurvival::SetupGrid()
{
  std::stringstream sLabel;
  int i;

  //Get the "Dispersed Seeds" grid and its data members
  mp_oSeedGrid = mp_oSimManager->GetGridObject( "Dispersed Seeds" );
  if ( NULL == mp_oSeedGrid )
  {
    modelErr stcErr;
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    stcErr.sFunction = "clLightDepSeedSurvival::SetupGrids" ;
    stcErr.sMoreInfo = "Disperse behaviors are required if establishment is to be used.";
    throw( stcErr );
  }
  mp_iSeedGridCode = new short int[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    sLabel << "seeds_" << mp_iWhatSpecies[i];
    mp_iSeedGridCode[i] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
    if ( -1 == mp_iSeedGridCode[i] )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clLightDepSeedSurvival::SetupGrids" ;
      stcErr.sMoreInfo = "Unexpected grid data return code for Dispersed Seeds.";
      throw( stcErr );
    }
    sLabel.str("");
  }

  //If this is using the Storm Light grid, get that grid and its data member
  if ( m_bUseStormLight )
  {
    mp_oStormLightGrid = mp_oSimManager->GetGridObject( "Storm Light" );
    if ( NULL == mp_oStormLightGrid )
    {
      modelErr stcErr;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      stcErr.sFunction = "clLightDepSeedSurvival::SetupGrids" ;
      stcErr.sMoreInfo = "The Storm Light behavior is required for Storm Light Seed Dependent Survival.";
      throw( stcErr );
    }
    m_iLightCode = mp_oStormLightGrid->GetFloatDataCode( "Light" );
    if ( -1 == m_iLightCode )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clLightDepSeedSurvival::SetupGrids" ;
      stcErr.sMoreInfo = "Unexpected grid data return code for Storm Light.";
      throw( stcErr );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetTreeDataMemberCodes()
/////////////////////////////////////////////////////////////////////////////
void clLightDepSeedSurvival::GetTreeDataMemberCodes()
{
  if (m_bUseStormLight) return;

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  int i;

  mp_iDamageCodes = new short int * [m_iNumTotalSpecies];
  for ( i = 0; i < m_iNumTotalSpecies; i++ )
  {
    mp_iDamageCodes[i] = new short int[2];
    mp_iDamageCodes[i] [0] = p_oPop->GetIntDataCode( "stm_dmg", i, clTreePopulation::sapling );
    mp_iDamageCodes[i] [1] = p_oPop->GetIntDataCode( "stm_dmg", i, clTreePopulation::adult );
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetLightFavorability()
/////////////////////////////////////////////////////////////////////////////
float clLightDepSeedSurvival::GetLightFavorability( const int & iSpecies, const float & fGLI )
{
  float fFavorability; //favorability
  int iSpIndex = mp_iIndexes[iSpecies]; //species index

  //Calculate the favorability

  if ( fGLI > mp_fOptimumGLI[iSpIndex] )
  {
    //GLI is greater than optimum - calculate using above-optimum slope
    fFavorability = 1 - ( mp_fHighGLISlope[iSpIndex] * ( fGLI - mp_fOptimumGLI[iSpIndex] ) );
  }
  else if ( fGLI < mp_fOptimumGLI[iSpIndex] )
  {
    //GLI is less than optimum - calculate using below-optimum slope
    fFavorability = 1 - ( mp_fLowGLISlope[iSpIndex] * ( mp_fOptimumGLI[iSpIndex] - fGLI ) );
  }
  else
  {
    //GLI is optimum - favorability is 1
    fFavorability = 1;
  }
  return fFavorability;
}

///////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clLightDepSeedSurvival::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    float fGLIX, fGLIY, //coordinates for calculating GLI
         fGLI, //GLI at a point in the cell
         fFavorability, //light favorability for a species
         fNewSeeds, //number of seeds that survive
         fNumSeeds; //number of total seeds for a species for a grid
    int iNumXCells = mp_oSeedGrid->GetNumberXCells(), //number X cells
         iNumYCells = mp_oSeedGrid->GetNumberYCells(), //number Y cells
         iX, iY, //loop counters for moving through grid cells
         iSpecies, //species of seed being dispersed
         i;

    //Cycle through the seeds grid and deal with each seed in each one
    for ( iX = 0; iX < iNumXCells; iX++ )
    {
      for ( iY = 0; iY < iNumYCells; iY++ )
      {

        //Find the point at which to calculate gli - center of the cell
        mp_oSeedGrid->GetPointOfCell( iX, iY, & fGLIX, & fGLIY );

        //Get the GLI at that point
        fGLI = GetGLI( p_oPop, fGLIX, fGLIY );

        //Loop through each behavior species
        for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        {

          iSpecies = mp_iWhatSpecies[i];

          //Get the number of seeds in this cell for this species
          mp_oSeedGrid->GetValueOfCell( iX, iY, mp_iSeedGridCode[mp_iIndexes[iSpecies]], & fNumSeeds );

          //Get the light favorability for this species
          fFavorability = GetLightFavorability( iSpecies, fGLI );

          //Get the number of seeds that survive
          if ( fNumSeeds > 0 )
          {
            fNewSeeds = fNumSeeds * fFavorability;
            if ( fNewSeeds < 0 ) fNewSeeds = 0;
            if ( fNewSeeds > fNumSeeds ) fNewSeeds = fNumSeeds;
          }
          else
          {
            fNewSeeds = 0;
          }

          //Set the number of seeds in this cell for this species
          mp_oSeedGrid->SetValueOfCell( iX, iY, mp_iSeedGridCode[mp_iIndexes[iSpecies]], fNewSeeds );
        }
      }
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
    stcErr.sFunction = "clLightDepSeedSurvival::Action" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetLightExtinctionCoefficient()
/////////////////////////////////////////////////////////////////////////////
float clLightDepSeedSurvival::GetLightExtinctionCoefficient( clTree * p_oTree )
{
  int iDamage, iTp = p_oTree->GetType();

  //If this tree has no storm damage member, or is a snag, use default behavior
  if (clTreePopulation::snag == iTp || -1 == mp_iDamageCodes[p_oTree->GetSpecies()] [iTp - clTreePopulation::sapling] )
  {
    return clLightBase::GetLightExtinctionCoefficient( p_oTree );
  }

  //Get the tree's storm damage
  p_oTree->GetValue( mp_iDamageCodes[p_oTree->GetSpecies()] [p_oTree->GetType() - clTreePopulation::sapling], & iDamage );
  //What damage category is the neighbor in?
  if ( iDamage > 2000 )
  {
    iDamage = 2;
  }
  else if ( iDamage > 1000 )
  {
    iDamage = 1;
  }
  else
  {
    iDamage = 0;
  }
  return mp_fLightExtCoeff[p_oTree->GetSpecies()] [iDamage];
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clLightDepSeedSurvival::SetNameData(std::string sNameString)
{
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("LightDependentSeedSurvival") == 0 )
    {
      m_bUseStormLight = false;
    }
    else if (sNameString.compare("StormLightDependentSeedSurvival") == 0 )
    {
      m_bUseStormLight = true;
    }
    else
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sMoreInfo = s.str();
      stcErr.sFunction = "clLightDepSeedSurvival::SetNameData" ;
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
    stcErr.sFunction = "clLightDepSeedSurvival::SetNameData" ;
    throw( stcErr );
  }
}
