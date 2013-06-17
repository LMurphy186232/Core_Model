//---------------------------------------------------------------------------
#include "Substrate.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "ModelMath.h"
#include "Disturbance.h"
#include "Plot.h"
#include <stdio.h>
#include <math.h>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clSubstrate::clSubstrate( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "Substrate";
    m_sXMLRoot = "Substrate";

    //NULL our pointers
    mp_iDeadCodes = NULL;
    mp_bSubstrateApplies = NULL;
    mp_fPropOfDeadThatFall = NULL;
    mp_fPropOfFallenThatUproot = NULL;
    mp_fPropOfSnagsThatUproot = NULL;
    mp_fFLogDecayProp = NULL;
    mp_fDecLogDecayProp = NULL;
    mp_fScarSoilDecayProp = NULL;
    mp_fTipupDecayProp = NULL;
    mp_oCalcGrid = NULL;
    mp_oSubstrateGrid = NULL;
    mp_oHarvestGrid = NULL;

    mp_iFLogCalcsCode = NULL;
    mp_iDecLogCalcsCode = NULL;
    m_iNumTotalSpecies = 0;

    m_iTipupCode = -1;
    m_bUseDirectionalTreeFall = false;
    m_fScarifiedSoilA = 0;
    m_fGapCutTipUp = 0;
    m_fPartCutTipUp = 0;
    m_iMaxDecayTimesteps = 0;
    m_fGapCutFreshLog = 0;
    m_iFFLitterCode = -1;
    m_iDecLogCode = -1;
    m_iPkgFreshLogCode = -1;
    m_iPkgTipupCode = -1;
    m_iFreshLogCode = -1;
    m_fInitTipUp = 0;
    m_iScarSoilCode = -1;
    m_fInitFreshLog = 0;
    m_iTipupCalcsCode = -1;
    m_fTipUpB = 0;
    m_fScarifiedSoilB = 0;
    m_fGridCellArea = 0;
    m_iPkgScarSoilCode = -1;
    m_fGapCutDecayedLog = 0;
    m_fClearCutFreshLog = 0;
    m_fTipUpA = 0;
    m_fDecayedLogA = 0;
    m_iMaxDecayYears = 0;
    m_fFreshLogB = 0;
    m_fMossProportion = 0;
    m_fRecipOfGridCellArea = 0;
    m_fClearCutScarifiedSoil = 0;
    m_fInitDecayedLog = 0;
    m_fGapCutScarifiedSoil = 0;
    m_iHarvestTypeCode = -1;
    m_iFFMossCode = -1;
    m_fClearCutTipUp = 0;
    m_fFreshLogA = 0;
    m_fInitScarifiedSoil = 0;
    m_fPartCutDecayedLog = 0;
    m_fClearCutDecayedLog = 0;
    m_fPartCutFreshLog = 0;
    m_bParFileHasSubstrateMap = false;
    m_fDecayedLogB = 0;
    m_fPartCutScarifiedSoil = 0;
    m_iPkgAgeCode = -1;
    m_fRootTipupFactor = 0;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 2.1;
    m_fMinimumVersionNumber = 2.1;
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
    stcErr.sFunction = "clSubstrate::clSubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clSubstrate::~clSubstrate()
{
  if ( mp_iDeadCodes && mp_bSubstrateApplies )
  {
    for ( int i = 0; i < m_iNumTotalSpecies; i++ )
    {
      delete[] mp_iDeadCodes[i];
      delete[] mp_bSubstrateApplies[i];
    }
  }

  delete[] mp_iDeadCodes;
  delete[] mp_bSubstrateApplies;
  delete[] mp_fPropOfDeadThatFall;
  delete[] mp_fPropOfFallenThatUproot;
  delete[] mp_fPropOfSnagsThatUproot;
  delete[] mp_fFLogDecayProp;
  delete[] mp_fDecLogDecayProp;
  delete[] mp_iDecLogCalcsCode;
  delete[] mp_fScarSoilDecayProp;
  delete[] mp_fTipupDecayProp;
  delete[] mp_iFLogCalcsCode;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    //Do this now - the presence of the Harvest grid is the proxy for
    //whether Harvest is in the behavior list
    mp_oHarvestGrid = mp_oSimManager->GetGridObject( "Harvest Results" );

    //Read in the values from the parameter file
    GetParameterFileData( p_oDoc );

    //Pre-calculate decay proportions
    CalculateDecayProportions();

    //Set up the substrate grids
    SetupSubstrateGrids();

    //Put the initial conditions into the substrate grid
    PopulateInitialConditions();

    //Get the codes for the "dead" data member
    GetDeadCodes();
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
    stcErr.sFunction = "clSubstrate::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::GetParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    floatVal * p_fTempValues; //for getting species-specific values
    float fTemp;
    short int i; //loop counter

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

    //Declare the permanent arrays
    mp_fPropOfDeadThatFall = new float[m_iNumTotalSpecies];
    mp_fPropOfFallenThatUproot = new float[m_iNumTotalSpecies];
    mp_fPropOfSnagsThatUproot = new float[m_iNumTotalSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //*****************************************
    // Read in the values from the parameter file
    //*****************************************

    //Area tipup matter exposed
    FillSingleValue( p_oElement, "su_rootTipupFactor", & m_fRootTipupFactor, true );

    //Proportion of dead that fall
    FillSpeciesSpecificValue( p_oElement, "su_propOfDeadFall", "su_podfVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPropOfDeadThatFall[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Proportion of fallen that uproot
    FillSpeciesSpecificValue( p_oElement, "su_propOfFallUproot", "su_pofuVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, turning each value into a
    //per-timestep proportion on the way
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPropOfFallenThatUproot[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Proportion of snags that uproot
    FillSpeciesSpecificValue( p_oElement, "su_propOfSnagsUproot", "su_posuVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, turning each value into a
    //per-timestep proportion on the way
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPropOfSnagsThatUproot[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Scarified soil decay alpha
    FillSingleValue( p_oElement, "su_scarSoilDecayAlpha", & m_fScarifiedSoilA, true );
    //Scarified soil decay beta
    FillSingleValue( p_oElement, "su_scarSoilDecayBeta", & m_fScarifiedSoilB, true );
    //Tipup decay alpha
    FillSingleValue( p_oElement, "su_tipupDecayAlpha", & m_fTipUpA, true );
    //Tipup decay beta
    FillSingleValue( p_oElement, "su_tipupDecayBeta", & m_fTipUpB, true );
    //Fresh log decay alpha
    FillSingleValue( p_oElement, "su_freshLogDecayAlpha", & m_fFreshLogA, true );
    //Fresh log decay beta
    FillSingleValue( p_oElement, "su_freshLogDecayBeta", & m_fFreshLogB, true );
    //Decayed log decay alpha
    FillSingleValue( p_oElement, "su_decayedLogDecayAlpha", & m_fDecayedLogA, true );
    //Decayed log decay beta
    FillSingleValue( p_oElement, "su_decayedLogDecayBeta", & m_fDecayedLogB, true );

    //Maximum number of decay years
    FillSingleValue( p_oElement, "su_maxNumberDecayYears", & m_iMaxDecayYears, true );

    //Proportion of forest floor that is moss
    FillSingleValue( p_oElement, "su_mossProportion", & m_fMossProportion, true );

    //Directional tree fall?
    FillSingleValue( p_oElement, "su_directionalTreeFall", & m_bUseDirectionalTreeFall, true );

    //Initial conditions - scarified soil
    FillSingleValue( p_oElement, "su_initialScarSoil", & m_fInitScarifiedSoil, true );
    //Initial conditions - tipup
    FillSingleValue( p_oElement, "su_initialTipup", & m_fInitTipUp, true );
    //Initial conditions - fresh logs
    FillSingleValue( p_oElement, "su_initialFreshLog", & m_fInitFreshLog, true );
    //Initial conditions - decayed logs
    FillSingleValue( p_oElement, "su_initialDecayedLog", & m_fInitDecayedLog, true );

    //Only do the harvest conditions if harvest is in the behavior list
    if ( mp_oHarvestGrid )
    {
      //Partial cut conditions - scarified soil
      FillSingleValue( p_oElement, "su_partialCutScarSoil", & m_fPartCutScarifiedSoil, true );
      //Partial cut conditions - tipup
      FillSingleValue( p_oElement, "su_partialCutTipup", & m_fPartCutTipUp, true );
      //Partial cut conditions - fresh logs
      FillSingleValue( p_oElement, "su_partialCutFreshLog", & m_fPartCutFreshLog, true );
      //Partial cut conditions - decayed logs
      FillSingleValue( p_oElement, "su_partialCutDecayedLog", & m_fPartCutDecayedLog, true );

      //Gap cut conditions - scarified soil
      FillSingleValue( p_oElement, "su_gapCutScarSoil", & m_fGapCutScarifiedSoil, true );
      //Gap cut conditions - tipup
      FillSingleValue( p_oElement, "su_gapCutTipup", & m_fGapCutTipUp, true );
      //Gap cut conditions - fresh logs
      FillSingleValue( p_oElement, "su_gapCutFreshLog", & m_fGapCutFreshLog, true );
      //Gap cut conditions - decayed logs
      FillSingleValue( p_oElement, "su_gapCutDecayedLog", & m_fGapCutDecayedLog, true );

      //Clear cut conditions - scarified soil
      FillSingleValue( p_oElement, "su_clearCutScarSoil", & m_fClearCutScarifiedSoil, true );
      //Clear cut conditions - tipup
      FillSingleValue( p_oElement, "su_clearCutTipup", & m_fClearCutTipUp, true );
      //Clear cut conditions - fresh logs
      FillSingleValue( p_oElement, "su_clearCutFreshLog", & m_fClearCutFreshLog, true );
      //Clear cut conditions - decayed logs
      FillSingleValue( p_oElement, "su_clearCutDecayedLog", & m_fClearCutDecayedLog, true );
    }

    //*****************************************
    // Validation and subsequent calculations - all of the proportions must add
    // up to less than or equal to 1.  If the don't, throw error.  If they do,
    // forest floor pool is the remainder needed to make it all add up to 1.
    //*****************************************

    //Max number of timesteps
    m_iMaxDecayTimesteps = (int)ceil( m_iMaxDecayYears / mp_oSimManager->GetNumberOfYearsPerTimestep() );

    //Initial conditions substrate proportions
    fTemp = 1 - ( m_fInitScarifiedSoil + m_fInitTipUp + m_fInitFreshLog + m_fInitDecayedLog );
    if ( fTemp < 0 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSubstrate::GetParameterFileData" ;
      stcErr.sMoreInfo = "Initial conditions substrate proportions must add up to be less than 1";
      throw( stcErr );
    }

    if ( mp_oHarvestGrid )
    {

      //Partial cut substrate proportions
      fTemp = 1 - ( m_fPartCutScarifiedSoil + m_fPartCutTipUp + m_fPartCutFreshLog + m_fPartCutDecayedLog );
      if ( fTemp < 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "Partial cut substrate proportions must add up to be less than 1";
        throw( stcErr );
      }


      //Gap cut substrate proportions
      fTemp = 1 - ( m_fGapCutScarifiedSoil + m_fGapCutTipUp + m_fGapCutFreshLog + m_fGapCutDecayedLog );
      if ( fTemp < 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "Gap cut substrate proportions must add up to be less than 1";
        throw( stcErr );
      }

      //Clear cut substrate proportions
      fTemp = 1 - ( m_fClearCutScarifiedSoil + m_fClearCutTipUp + m_fClearCutFreshLog + m_fClearCutDecayedLog );
      if ( fTemp < 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrate::GetParameterFileData" ;
        stcErr.sMoreInfo = "Clear cut substrate proportions must add up to be less than 1";
        throw( stcErr );
      }
    }

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
    stcErr.sFunction = "clSubstrate::GetParameterFileData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetupSubstrateGrids()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::SetupSubstrateGrids()
{
  try
  {
    std::stringstream sLabel;
    float fXCellLength = 0, fYCellLength = 0;
    int i;

    //See if the harvest results grid exists.  If it does, change the cell
    //length to match
    if ( mp_oHarvestGrid )
    {
      fXCellLength = mp_oHarvestGrid->GetLengthXCells();
      fYCellLength = mp_oHarvestGrid->GetLengthYCells();
    }

    //**************************************
    //Create the main substrate grid
    //**************************************
    //Check for its pre-existence
    mp_oSubstrateGrid = mp_oSimManager->GetGridObject( "Substrate" );

    if ( NULL == mp_oSubstrateGrid )
    {

      m_bParFileHasSubstrateMap = false;

      //There was no map of this grid, so create it
      //If there's no harvest grid, the cell lengths will be 0 and therefore
      //the grid defaults will be used
      mp_oSubstrateGrid = mp_oSimManager->CreateGrid( "Substrate", //grid name
          0, //number of int data members
          6, //number of float data members
          0, //number of char data members
          0, //number of bool data members
          fXCellLength, //X cell length
          fYCellLength ); //Y cell length

      //Package data structure
      mp_oSubstrateGrid->ChangePackageDataStructure( 1, //number of package int data members
          3, //number of package float data members
          0, //number of package char data members
          0 ); //number of package bool data members

      //Register data members
      m_iScarSoilCode = mp_oSubstrateGrid->RegisterFloat( "scarsoil" );
      m_iFFMossCode = mp_oSubstrateGrid->RegisterFloat( "ffmoss" );
      m_iFFLitterCode = mp_oSubstrateGrid->RegisterFloat( "fflitter" );
      m_iTipupCode = mp_oSubstrateGrid->RegisterFloat( "tipup" );
      m_iFreshLogCode = mp_oSubstrateGrid->RegisterFloat( "freshlog" );
      m_iDecLogCode = mp_oSubstrateGrid->RegisterFloat( "declog" );
      m_iPkgAgeCode = mp_oSubstrateGrid->RegisterPackageInt( "age" );
      m_iPkgScarSoilCode = mp_oSubstrateGrid->RegisterPackageFloat( "scarsoil" );
      m_iPkgTipupCode = mp_oSubstrateGrid->RegisterPackageFloat( "tipup" );
      m_iPkgFreshLogCode = mp_oSubstrateGrid->RegisterPackageFloat( "freshlog" );

    }
    else
    {

      //The grid was created from a map file.  Validate it.
      m_bParFileHasSubstrateMap = true;

      //Make sure the cell lengths are right
      if ( ( fXCellLength > 0 && mp_oSubstrateGrid->GetLengthXCells() != fXCellLength )
          || ( fYCellLength > 0 && mp_oSubstrateGrid->GetLengthYCells() != fYCellLength ) ) {

        //Throw an eror
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Grid map values for grid \"Substrate\" contain cell lengths which do not match the allowed values.";
        throw( stcErr );
      }

      if (fXCellLength == 0) {
        fXCellLength = mp_oSubstrateGrid->GetLengthXCells();
      }
      if (fYCellLength == 0) {
        fYCellLength = mp_oSubstrateGrid->GetLengthYCells();
      }

      //Get the data member codes
      m_iScarSoilCode = mp_oSubstrateGrid->GetFloatDataCode( "scarsoil" );
      m_iFFMossCode = mp_oSubstrateGrid->GetFloatDataCode( "ffmoss" );
      m_iFFLitterCode = mp_oSubstrateGrid->GetFloatDataCode( "fflitter" );
      m_iTipupCode = mp_oSubstrateGrid->GetFloatDataCode( "tipup" );
      m_iFreshLogCode = mp_oSubstrateGrid->GetFloatDataCode( "freshlog" );
      m_iDecLogCode = mp_oSubstrateGrid->GetFloatDataCode( "declog" );

      //Throw an error if any of them do not exist - we can't register them at
      //this point
      if ( -1 == m_iScarSoilCode || -1 == m_iFFMossCode || -1 == m_iFFLitterCode
          || -1 == m_iTipupCode || -1 == m_iFreshLogCode || -1 == m_iDecLogCode ) {

        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Grid map setup values for grid \"Substrate\" did not contain all required grid data members.";
        throw( stcErr );
      }

      //The packages - try to find age, which would not be automatically present
      m_iPkgAgeCode = mp_oSubstrateGrid->GetPackageIntDataCode( "age" );

      if ( -1 == m_iPkgAgeCode )
      {

        //Packages were not included in the map - declare them now
        mp_oSubstrateGrid->ChangePackageDataStructure( 1, //number of package int data members
            3, //number of package float data members
            0, //number of package char data members
            0 ); //number of package bool data members

        m_iPkgAgeCode = mp_oSubstrateGrid->RegisterPackageInt( "age" );
        m_iPkgScarSoilCode = mp_oSubstrateGrid->RegisterPackageFloat( "scarsoil" );
        m_iPkgTipupCode = mp_oSubstrateGrid->RegisterPackageFloat( "tipup" );
        m_iPkgFreshLogCode = mp_oSubstrateGrid->RegisterPackageFloat( "freshlog" );

      }
      else
      {

        //Packages should have been in the map - get their codes
        m_iPkgScarSoilCode = mp_oSubstrateGrid->GetPackageFloatDataCode( "scarsoil" );
        m_iPkgTipupCode = mp_oSubstrateGrid->GetPackageFloatDataCode( "tipup" );
        m_iPkgFreshLogCode = mp_oSubstrateGrid->GetPackageFloatDataCode( "freshlog" );

        //Throw an error if any are missing
        if ( -1 == m_iPkgScarSoilCode || -1 == m_iPkgTipupCode || -1 == m_iPkgFreshLogCode ) {

          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
          stcErr.sMoreInfo = "Grid map setup values for grid \"Substrate\" did not contain all required grid data members.";
          throw( stcErr );
        }
      }
    }

    //**************************************
    //Create the calculations grid
    //**************************************
    //Check for its pre-existence
    mp_oCalcGrid = mp_oSimManager->GetGridObject( "substratecalcs" );

    if ( NULL == mp_oCalcGrid )
    {

      mp_oCalcGrid = mp_oSimManager->CreateGrid( "substratecalcs",
          0, //number of ints
          1 + (2 * m_iMaxDecayTimesteps), //number of floats
          0, //number of chars
          0, //number of bools
          fXCellLength, //X cell length - must match the grid above!
          fYCellLength ); //Y cell length - must match the grid above!

      //Register data members
      m_iTipupCalcsCode = mp_oCalcGrid->RegisterFloat( "newtipup" );
      mp_iFLogCalcsCode = new short int[m_iMaxDecayTimesteps];
      mp_iDecLogCalcsCode = new short int[m_iMaxDecayTimesteps];
      for ( i = 0; i < m_iMaxDecayTimesteps; i++ )
      {
        sLabel << "freshlog_" << i;
        mp_iFLogCalcsCode[i] = mp_oCalcGrid->RegisterFloat(sLabel.str());
        sLabel.str("");
        sLabel << "declog_" << i;
        mp_iDecLogCalcsCode[i] = mp_oCalcGrid->RegisterFloat(sLabel.str());
        sLabel.str("");
      }
    }
    else
    {
      //It was already created - make sure it was set up right
      //Make sure the cell lengths are right
      if ( (mp_oCalcGrid->GetLengthXCells() != mp_oSubstrateGrid->GetLengthXCells())
          || ( mp_oCalcGrid->GetLengthYCells() != mp_oSubstrateGrid->GetLengthYCells())) {

        //Throw an eror
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Grid map values for grid \"substratecalcs\" contain cell lengths which do not match the allowed values.";
        throw( stcErr );
      }

      //Get the data member codes
      m_iTipupCalcsCode = mp_oCalcGrid->GetFloatDataCode( "newtipup" );
      if (-1 == m_iTipupCalcsCode) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Grid map setup values for grid \"substratecalcs\" did not contain all required grid data members.";
        throw( stcErr );
      }
      mp_iFLogCalcsCode = new short int[m_iMaxDecayTimesteps];
      mp_iDecLogCalcsCode = new short int[m_iMaxDecayTimesteps];
      for ( i = 0; i < m_iMaxDecayTimesteps; i++ ) {
        sLabel << "freshlog_" << i;
        mp_iFLogCalcsCode[i] = mp_oCalcGrid->GetFloatDataCode(sLabel.str());
        sLabel.str("");
        if (-1 == mp_iFLogCalcsCode[i]) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
          stcErr.sMoreInfo = "Grid map setup values for grid \"substratecalcs\" did not contain all required grid data members.";
          throw( stcErr );
        }

        sLabel << "declog_" << i;
        mp_iDecLogCalcsCode[i] = mp_oCalcGrid->GetFloatDataCode(sLabel.str());
        sLabel.str("");
        if (-1 == mp_iDecLogCalcsCode[i]) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
          stcErr.sMoreInfo = "Grid map setup values for grid \"substratecalcs\" did not contain all required grid data members.";
          throw( stcErr );
        }
      }
    }

    //Get the area and the reciprocal of the area of a grid cell
    m_fGridCellArea = mp_oSubstrateGrid->GetLengthXCells() * mp_oSubstrateGrid->GetLengthYCells();
    m_fRecipOfGridCellArea = 1.0 / m_fGridCellArea;

    //**************************************
    //If there's a harvest grid, get the data code for harvest type
    //**************************************
    if ( mp_oHarvestGrid )
    {
      m_iHarvestTypeCode = mp_oHarvestGrid->GetIntDataCode( "Harvest Type" );
      //If the code is -1 throw an error
      if ( m_iHarvestTypeCode == -1 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
        stcErr.sMoreInfo = "Harvest grid set up in unexpected way.";
        throw( stcErr );
      }
    }
    else
      m_iHarvestTypeCode = -1;
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
    stcErr.sFunction = "clSubstrate::SetupSubstrateGrid" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// PopulateInitialConditions()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::PopulateInitialConditions()
{
  try
  {

    if ( m_bParFileHasSubstrateMap ) return;

    clPackage * p_oPackage; //new package
    float fInitForestFloor = 1 - ( m_fInitScarifiedSoil + m_fInitTipUp + m_fInitFreshLog + m_fInitDecayedLog );
    short int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(), iNumYCells = mp_oSubstrateGrid->GetNumberYCells(), i, j; //loop counters

    //Go through each grid cell and give it the proper initial conditions
    for ( i = 0; i < iNumXCells; i++ )
      for ( j = 0; j < iNumYCells; j++ )
      {

        mp_oSubstrateGrid->SetValueOfCell( i, j, m_iScarSoilCode, m_fInitScarifiedSoil );
        mp_oSubstrateGrid->SetValueOfCell( i, j, m_iFFMossCode, fInitForestFloor * m_fMossProportion );
        mp_oSubstrateGrid->SetValueOfCell( i, j, m_iFFLitterCode, fInitForestFloor * (1 - m_fMossProportion) );
        mp_oSubstrateGrid->SetValueOfCell( i, j, m_iTipupCode, m_fInitTipUp );
        mp_oSubstrateGrid->SetValueOfCell( i, j, m_iFreshLogCode, m_fInitFreshLog );
        mp_oSubstrateGrid->SetValueOfCell( i, j, m_iDecLogCode, m_fInitDecayedLog );

        //Create a new package with the initial substrate proportions
        p_oPackage = mp_oSubstrateGrid->CreatePackageOfCell( i, j );

        //Package will be incremented in DecaySubstrate, first timestep
        p_oPackage->SetValue( m_iPkgAgeCode, 0 );
        p_oPackage->SetValue( m_iPkgScarSoilCode, m_fInitScarifiedSoil );
        p_oPackage->SetValue( m_iPkgTipupCode, m_fInitTipUp );
        p_oPackage->SetValue( m_iPkgFreshLogCode, m_fInitFreshLog );
      } //end of for (j = 0; j < iNumYCells; j++)
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
    stcErr.sFunction = "clSubstrate::PopulateInitialConditions" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetDeadCodes()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::GetDeadCodes()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    char cLabel[] = "dead";
    short int iTotalTypes = p_oPop->GetNumberOfTypes(), i, j; //loop counters

    //Declare and initialize the return codes and substrate applies arrays
    mp_iDeadCodes = new short int * [m_iNumTotalSpecies];
    mp_bSubstrateApplies = new bool * [m_iNumTotalSpecies];
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      mp_iDeadCodes[i] = new short int[iTotalTypes];
      mp_bSubstrateApplies[i] = new bool[iTotalTypes];
      for ( j = 0; j < iTotalTypes; j++ )
      {
        mp_iDeadCodes[i] [j] = -1;
        mp_bSubstrateApplies[i] [j] = false;
      }
    }

    //Now go through the species/type combos and get the code for each
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {

      //If any type is seedling, throw an error
      if ( mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::seedling )
      {
        modelErr stcErr;
        stcErr.sFunction = "clSubstrate::GetDeadCodes" ;
        stcErr.sMoreInfo = "Substrate cannot be applied to seedlings.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] =
          p_oPop->GetIntDataCode( cLabel, mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

      mp_bSubstrateApplies[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] = true;

      //If the return code is -1, throw an error
      if ( -1 == mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] )
      {
        modelErr stcErr;
        stcErr.sFunction = "clSubstrate::GetDeadCodes";
        std::stringstream s;
        s << "Type/species combo species="
            << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
            << mp_whatSpeciesTypeCombos[i].iType
            << " must have a mortality behavior assigned to be compatible with substrate.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
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
    stcErr.sFunction = "clSubstrate::GetDeadCodes" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::Action()
{
  try
  {
    //Update the ages in the calcs array
    UpdateSubstrateAges();

    //Add new substrate due to harvest
    HarvestSubstrate();

    //Add new substrate due to mortality
    MortalitySubstrate();

    //Add in any new substrate created by harvest
    AddHarvestAndInitialNewSubstrate();

    //Decay substrate packages
    DecaySubstrate();
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
    stcErr.sFunction = "clSubstrate::Action" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// HarvestSubstrate()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::HarvestSubstrate()
{
  try
  {

    //If there's no harvest, skip out
    if ( !mp_oHarvestGrid ) return;

    clPackage * p_oPackage; //for working with substrate grid packages
    float fNewTipup, //amount of new tipup for harvest
    fNewScarSoil, //amount of new scarified soil for harvest
    fNewFreshLog, //amount of new fresh log for harvest
    fNewDecayedLog; //amount of new decayed log for harvest
    int iCutType; //the harvest cut type that occurred in a grid cell
    short int iNumXCells = mp_oHarvestGrid->GetNumberXCells(), iNumYCells = mp_oHarvestGrid->GetNumberYCells(), iX, iY, i; //loop counters

    for ( iX = 0; iX < iNumXCells; iX++ )
      for ( iY = 0; iY < iNumYCells; iY++ )
      {

        //Get the cut type for this grid cell
        mp_oHarvestGrid->GetValueOfCell( iX, iY, m_iHarvestTypeCode, & iCutType );

        //If there was no cut, skip this cell
        if ( -1 == iCutType ) goto nextCell;

        //Delete all existing packages
        p_oPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );
        while ( p_oPackage )
        {
          mp_oSubstrateGrid->DeletePackage( p_oPackage );
          p_oPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );
        }

        //Clear all existing fresh log data
        fNewFreshLog = 0;
        for ( i = 1; i < m_iMaxDecayTimesteps; i++ )
        {
          mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iFLogCalcsCode[i], fNewFreshLog );
        }

        //Clear all existing decayed log data - LEM 11-10-05
        fNewDecayedLog = 0;
        for ( i = 1; i < m_iMaxDecayTimesteps; i++ )
        {
          mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iDecLogCalcsCode[i], fNewDecayedLog );
        }

        //Get the new substrate amounts depending on the cut type
        if ( clDisturbance::partial == iCutType )
        { //partial cut
          fNewTipup = m_fPartCutTipUp;
          fNewScarSoil = m_fPartCutScarifiedSoil;
          fNewFreshLog = m_fPartCutFreshLog;
          fNewDecayedLog = m_fPartCutDecayedLog;
        }
        else if ( clDisturbance::gap == iCutType )
        { //gap cut
          fNewTipup = m_fGapCutTipUp;
          fNewScarSoil = m_fGapCutScarifiedSoil;
          fNewFreshLog = m_fGapCutFreshLog;
          fNewDecayedLog = m_fGapCutDecayedLog;
        }
        else
        { //clear cut
          fNewTipup = m_fClearCutTipUp;
          fNewScarSoil = m_fClearCutScarifiedSoil;
          fNewFreshLog = m_fClearCutFreshLog;
          fNewDecayedLog = m_fClearCutDecayedLog;
        }

        //Set the package proportions to match the harvest
        p_oPackage = mp_oSubstrateGrid->CreatePackageOfCell( iX, iY );
        p_oPackage->SetValue( m_iPkgScarSoilCode, fNewScarSoil );
        p_oPackage->SetValue( m_iPkgTipupCode, fNewTipup );
        p_oPackage->SetValue( m_iPkgFreshLogCode, fNewFreshLog );
        //By setting the age to zero, the proportions will pass through the
        //decay equations in DecaySubstrate() unchanged; that function will
        //then set the values in the grid to the same proportions
        p_oPackage->SetValue( m_iPkgAgeCode, 0 );

        //Set the value for the main cell's decayed logs - since they don't have
        //a place in the cohort package, we have to stash them like this.
        //DecaySubstrate() will watch out for them.
        mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iDecLogCode, fNewDecayedLog );

        nextCell:;
      } //end of for (iY = 0; iY < iNumYCells; iY++)
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
    stcErr.sFunction = "clSubstrate::HarvestSubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// AddHarvestAndInitialNewSubstrate()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::AddHarvestAndInitialNewSubstrate()
{
  try
  {
    float fNewTipup, //amount of new tipup for harvest
    fNewFreshLog, //amount of new fresh log for harvest
    fNewDecayedLog, //amount of new decayed log for harvest
    fTemp;
    int iCutType; //the harvest cut type that occurred in a grid cell
    short int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(), iNumYCells = mp_oSubstrateGrid->GetNumberYCells(), iX, iY; //loop counters

    //Do two loops to cut down the number of if statements
    if ( mp_oSimManager->GetCurrentTimestep() > 1 )
    {
      //If there's no harvest, skip out
      if ( !mp_oHarvestGrid ) return;

      for ( iX = 0; iX < iNumXCells; iX++ )
        for ( iY = 0; iY < iNumYCells; iY++ )
        {

          //Was there a harvest?
          mp_oHarvestGrid->GetValueOfCell( iX, iY, m_iHarvestTypeCode, & iCutType );

          //If there was no cut, skip this cell
          if ( -1 == iCutType ) goto nextCell;

          //Get the new substrate amounts depending on the cut type
          if ( clDisturbance::partial == iCutType )
          { //partial cut
            fNewTipup = m_fPartCutTipUp;
            fNewFreshLog = m_fPartCutFreshLog;
            fNewDecayedLog = m_fPartCutDecayedLog;
          }
          else if ( clDisturbance::gap == iCutType )
          { //gap cut
            fNewTipup = m_fGapCutTipUp;
            fNewFreshLog = m_fGapCutFreshLog;
            fNewDecayedLog = m_fGapCutDecayedLog;
          }
          else
          { //clear cut
            fNewTipup = m_fClearCutTipUp;
            fNewFreshLog = m_fClearCutFreshLog;
            fNewDecayedLog = m_fClearCutDecayedLog;
          }
          //Add the amount of fresh logs and tipup
          mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iFLogCalcsCode[0], & fTemp );
          mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iFLogCalcsCode[0], fNewFreshLog + fTemp );

          mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iDecLogCalcsCode[0], & fTemp );
          mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iDecLogCalcsCode[0], fNewDecayedLog + fTemp );

          mp_oCalcGrid->GetValueOfCell( iX, iY, m_iTipupCalcsCode, & fTemp );
          mp_oCalcGrid->SetValueOfCell( iX, iY, m_iTipupCalcsCode, fNewTipup + fTemp );

          nextCell:;
        } //end of for (iY = 0; iY < iNumYCells; iY++)
    }
    else
    {
      for ( iX = 0; iX < iNumXCells; iX++ )
        for ( iY = 0; iY < iNumYCells; iY++ )
        {

          //This is the first timestep
          iCutType = -1;
          if ( mp_oHarvestGrid )
          {
            mp_oHarvestGrid->GetValueOfCell( iX, iY, m_iHarvestTypeCode, & iCutType );
          }

          if ( -1 == iCutType )
          {
            fNewTipup = m_fInitTipUp;
            fNewFreshLog = m_fInitFreshLog;
            fNewDecayedLog = m_fInitDecayedLog;
          }
          else if ( clDisturbance::partial == iCutType )
          { //partial cut
            fNewTipup = m_fPartCutTipUp;
            fNewFreshLog = m_fPartCutFreshLog;
            fNewDecayedLog = m_fPartCutDecayedLog;
          }
          else if ( clDisturbance::gap == iCutType )
          { //gap cut
            fNewTipup = m_fGapCutTipUp;
            fNewFreshLog = m_fGapCutFreshLog;
            fNewDecayedLog = m_fGapCutDecayedLog;
          }
          else
          { //clear cut
            fNewTipup = m_fClearCutTipUp;
            fNewFreshLog = m_fClearCutFreshLog;
            fNewDecayedLog = m_fClearCutDecayedLog;
          }

          //Add the amount of fresh logs, decayed lots, and tipup
          mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iFLogCalcsCode[0], & fTemp );
          mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iFLogCalcsCode[0], fNewFreshLog + fTemp );

          mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iDecLogCalcsCode[0], & fTemp );
          mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iDecLogCalcsCode[0], fNewDecayedLog + fTemp );

          mp_oCalcGrid->GetValueOfCell( iX, iY, m_iTipupCalcsCode, & fTemp );
          mp_oCalcGrid->SetValueOfCell( iX, iY, m_iTipupCalcsCode, fNewTipup + fTemp );
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
    stcErr.sFunction = "clSubstrate::HarvestSubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// MortalitySubstrate()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::MortalitySubstrate()
{
  try
  {
    AddNewDeadTrees();
    AdjustSubstrateForMortality();
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
    stcErr.sFunction = "clSubstrate::MortalitySubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// AddNewDeadTrees()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::AddNewDeadTrees()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    clTreeSearch * p_oAllTrees; //search object for accessing all trees
    clTree * p_oTree; //one individual tree to work with
    float fRandom, //random number
    fRadius, //tree's radius in meters
    fX, fY, //tree's X and Y coordinates
    fTreeLogArea, //tree's new fresh log area
    fDbh, //tree's dbh
    fAngle, //angle of tree fall, in the case of directional tree fall
    fTrunkInterval = 0.5, fStep, //to help apportion fresh log area in the case
    //of directional tree fall
    fHeight, //tree's height
    fTrunkChunk, //one chunk of fresh log in the case of directional tree fall
    fSubstrate; //for getting and setting substrate amounts in
    //the calculations grid
    int iNumTrunkChunks, //number of chunks into which to divide the tree in case of
    //directional tree fall
    iDead; //value in the "dead" data member
    short int iSp, iTp,  //species and type of a tree
    i, iStepX, iStepY; //loop counters

    //Access all trees one at a time and see who's dead - only those who
    //qualify for substrate will be considered.  We could also do a species/type
    //search, but I decided to do it this way because I think it will be
    //marginally faster.
    p_oAllTrees = p_oPop->Find( "all" );

    p_oTree = p_oAllTrees->NextTree();
    while ( p_oTree )
    {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();
      if ( !mp_bSubstrateApplies[iSp] [iTp] )
        goto nextTree;
      p_oTree->GetValue( mp_iDeadCodes[iSp] [iTp], & iDead );
      if ( notdead == iDead  )
        goto nextTree;

      //If we're still here, this tree's dead.  Get a random number to see if
      //it falls over.
      if ( iTp != clTreePopulation::snag )
      {
        fRandom = clModelMath::GetRand();
        if ( fRandom > mp_fPropOfDeadThatFall[iSp] )
          goto nextTree; //didn't fall
      }

      p_oTree->GetValue( p_oPop->GetXCode( iSp, iTp ), & fX );
      p_oTree->GetValue( p_oPop->GetYCode( iSp, iTp ), & fY );
      p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDbh );
      p_oTree->GetValue( p_oPop->GetHeightCode( iSp, iTp ), & fHeight );

      //Kill this tree and force its removal
      p_oPop->KillTree( p_oTree, remove_tree );

      //Calculate the new fresh log area for this tree.  The trunk area is
      //modeled as a cone and calculated in sq m.
      fRadius = fDbh * 0.005; //*0.01 converts from cm to m, *0.5 converts from diameter to radius
      fTreeLogArea = fRadius * fHeight;

      if (!m_bUseDirectionalTreeFall) {
        mp_oCalcGrid->GetValueAtPoint( fX, fY, mp_iFLogCalcsCode[0], & fSubstrate );
        fSubstrate += fTreeLogArea;
        mp_oCalcGrid->SetValueAtPoint( fX, fY, mp_iFLogCalcsCode[0], fSubstrate );
      }
      else {
        //Divide the trunk into even chunks based on the number of 0.5 meter
        //sections of trunk
        iNumTrunkChunks = (int)ceil( fHeight * 2 );
        fTrunkChunk = fTreeLogArea / iNumTrunkChunks;

        //Get a random azimuth direction for the tree to fall
        fAngle = 2.0 * M_PI * ( clModelMath::GetRand() );

        //Walk out the length of the trunk in 0.5 m intervals and put a chunk of
        //fresh log in each cell we hit
        //It's possible to go a little higher than the top of the tree, and
        //hit a grid cell that the actual trunk couldn't have reached.  I am
        //accepting that risk since I think this is a good way to portion up
        //the fallen trunk.
        for ( i = 0; i < iNumTrunkChunks; i++ )
        {

          //Find the X and Y grid cells of the point at the end of this 0.5 m
          //interval
          fStep = i * fTrunkInterval;
          mp_oCalcGrid->GetCellOfPoint( p_oPlot->CorrectX( p_oPlot->GetUncorrectedX( fX, fAngle, fStep ) ),
              p_oPlot->CorrectY( p_oPlot->GetUncorrectedY( fY, fAngle, fStep ) ), & iStepX, & iStepY );

          //Drop a trunk chunk in this cell
          //Get the amount of fresh log currently in this tree's grid cell from
          //the calculation grid
          mp_oCalcGrid->GetValueOfCell( iStepX, iStepY, mp_iFLogCalcsCode[0], & fSubstrate );
          fSubstrate += fTrunkChunk;
          mp_oCalcGrid->SetValueOfCell( iStepX, iStepY, mp_iFLogCalcsCode[0], fSubstrate );
        }
      }


      //Now see if this fallen tree uprooted and left a tip-up mound - in which
      //case we'll add the amount of new tipup matter to the tree's grid cell
      //in the calculation grid.
      if ( iTp == clTreePopulation::adult)
      {
        fRandom = clModelMath::GetRand();
        if ( fRandom <= mp_fPropOfFallenThatUproot[iSp] )
        {
          //Yes, it uprooted - add the new tipup exposed to this grid's new
          //tipup variable
          mp_oCalcGrid->GetValueAtPoint( fX, fY, m_iTipupCalcsCode, & fSubstrate );
          fSubstrate += M_PI * pow( fRadius * m_fRootTipupFactor, 2 );
          mp_oCalcGrid->SetValueAtPoint( fX, fY, m_iTipupCalcsCode, fSubstrate );
        }
      }
      else if ( iTp == clTreePopulation::snag) //LEM 04/22/05
      {
        fRandom = clModelMath::GetRand();
        if ( fRandom <= mp_fPropOfSnagsThatUproot[iSp] )
        {
          //Yes, it uprooted - add the new tipup exposed to this grid's new
          //tipup variable
          mp_oCalcGrid->GetValueAtPoint( fX, fY, m_iTipupCalcsCode, & fSubstrate );
          fSubstrate += M_PI * pow( fRadius * m_fRootTipupFactor, 2 );
          mp_oCalcGrid->SetValueAtPoint( fX, fY, m_iTipupCalcsCode, fSubstrate );
        }
      }

      nextTree:
      p_oTree = p_oAllTrees->NextTree();
    } //end of while (p_oTree)
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
    stcErr.sFunction = "clSubstrate::AddNewDeadTrees" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// AdjustSubstrateForMortality()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::AdjustSubstrateForMortality()
{
  try
  {
    clPackage * p_oPackage, //for working with substrate grid packages
    * p_oNextToLastPackage; //for getting the last package
    float fNewSubstrateCreated, //amount of new substrate created as a proportion
    fNewFreshLog, //new fresh log area created
    fNewTipup, //new tipup matter exposed through tip-ups
    fTemp; //temp variable for working with grid vals
    int iTemp; //temp variable for working with grid vals
    short int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(), iNumYCells = mp_oSubstrateGrid->GetNumberYCells(), i, j; //loop counters
    bool bSubstrateAdded; //for whether or not new substrate proportions
    //were added to existing package

    //All the dead trees have been added to substrate.  Now go through each
    //grid cell in the substrate grid and update it with the new substrate from
    //the calculations grid

    for ( i = 0; i < iNumXCells; i++ )
      for ( j = 0; j < iNumYCells; j++ )
      {

        //Get the amount of fresh log area added for this grid cell
        mp_oCalcGrid->GetValueOfCell( i, j, mp_iFLogCalcsCode[0], & fNewFreshLog );
        mp_oCalcGrid->GetValueOfCell( i, j, m_iTipupCalcsCode, & fNewTipup );

        if ( fNewFreshLog == 0.0 ) //no new fresh log added - nothing to do
          goto nextCell;

        //Was there more substrate area added than there is area in the grid?
        //If not, make the new values a proportion of the total area.  If so,
        //make the new values a proportion of total new substrate
        if ( fNewFreshLog + fNewTipup <= m_fGridCellArea )
        {
          fNewFreshLog = fNewFreshLog * m_fRecipOfGridCellArea;
          fNewTipup = fNewTipup * m_fRecipOfGridCellArea;
        }
        else
        {
          fTemp = fNewFreshLog + fNewTipup;
          fNewFreshLog = fNewFreshLog / fTemp;
          fNewTipup = fNewTipup / fTemp;
        }
        //Add the fresh log and tipup back to the grid
        mp_oCalcGrid->SetValueOfCell( i, j, mp_iFLogCalcsCode[0], fNewFreshLog );
        mp_oCalcGrid->SetValueOfCell( i, j, m_iTipupCalcsCode, fNewTipup );
        //Calculate total new substrate proportion created
        fNewSubstrateCreated = fNewFreshLog + fNewTipup;

        //Now reduce each package's substrate proportions by the amount of new
        //substrate created
        bSubstrateAdded = false;
        p_oPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( i, j );
        while ( p_oPackage )
        {
          //Decrement the initial package according the new proportions created
          p_oPackage->GetValue( m_iPkgScarSoilCode, & fTemp );
          fTemp *= ( 1 - fNewSubstrateCreated );
          p_oPackage->SetValue( m_iPkgScarSoilCode, fTemp );

          p_oPackage->GetValue( m_iPkgTipupCode, & fTemp );
          fTemp *= ( 1 - fNewSubstrateCreated );
          p_oPackage->SetValue( m_iPkgTipupCode, fTemp );

          p_oPackage->GetValue( m_iPkgFreshLogCode, & fTemp );
          fTemp *= ( 1 - fNewSubstrateCreated );
          p_oPackage->SetValue( m_iPkgFreshLogCode, fTemp );

          //If the package has an age of 0 (created this timestep), it's either
          //harvest or initial conditions.  In this case, add the new substrates
          //to this package
          p_oPackage->GetValue( m_iPkgAgeCode, & iTemp );
          if ( iTemp == 0 )
          {

            p_oPackage->GetValue( m_iPkgTipupCode, & fTemp );
            fTemp += fNewTipup;
            p_oPackage->SetValue( m_iPkgTipupCode, fTemp );

            p_oPackage->GetValue( m_iPkgFreshLogCode, & fTemp );
            fNewFreshLog += fTemp;
            p_oPackage->SetValue( m_iPkgFreshLogCode, fNewFreshLog );

            //Main value should also match for scarified soil
            p_oPackage->GetValue( m_iPkgScarSoilCode, & fTemp );
            mp_oSubstrateGrid->SetValueOfCell( i, j, m_iScarSoilCode, fTemp );

            bSubstrateAdded = true;

          }

          p_oPackage = p_oPackage->GetNextPackage();
        } //end of while (p_oPackage)

        //If there was no package already created for this timestep, create one
        //now and put it last; adjust all previous packages' proportions
        if ( !bSubstrateAdded )
        {
          p_oNextToLastPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( i, j );

          while ( p_oNextToLastPackage )
          {

            p_oPackage = p_oNextToLastPackage->GetNextPackage();
            if ( !p_oPackage )
            {
              p_oPackage = mp_oSubstrateGrid->CreatePackage( p_oNextToLastPackage );
              //Age will be incremented in DecaySubstrate() so set to 0
              p_oPackage->SetValue( m_iPkgAgeCode, ( int )0 );
              p_oPackage->SetValue( m_iPkgFreshLogCode, fNewFreshLog );
              p_oPackage->SetValue( m_iPkgTipupCode, fNewTipup );
              //Leave scarified soil at zero
              //Now break the loop
              p_oNextToLastPackage = NULL;
            }
            else
              p_oNextToLastPackage = p_oPackage;
          }
        } //end of if (!bSubstrateAdded)

        nextCell:;
      } //end of for (j = 0; j < iNumYCells; j++)
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
    stcErr.sFunction = "clSubstrate::AdjustSubstrateForMortality" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// DecaySubstrate()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::DecaySubstrate()
{
  try
  {
    clPackage * p_oPackage, //next two for working with packages
    * p_oNextPackage;
    float fScarSoil, //adjusted scarified soil substrate
    fTipup, //adjusted tipup substrate
    fFreshLogs, //adjusted fresh log substrate
    fDecLogs, //amount of decayed log substrate
    fTotalSubstrate, //all the substrates added together
    fTemp; //for getting/setting grid values
    int iAge, //package age
    iTemp;
    short int iNumXCells = mp_oSubstrateGrid->GetNumberXCells(), iNumYCells = mp_oSubstrateGrid->GetNumberYCells(), iX, iY, k; //loop counters


    for ( iX = 0; iX < iNumXCells; iX++ )
      for ( iY = 0; iY < iNumYCells; iY++ )
      {

        //***************************************
        // Decayed logs
        //***************************************
        //If a harvest occurred in the past timestep, the decayed log value is
        //already correct.  So only update if there was no harvest AND it's not
        //the first timestep
        iTemp = -1;
        if ( mp_oHarvestGrid )
          mp_oHarvestGrid->GetValueOfCell( iX, iY, m_iHarvestTypeCode, & iTemp );
        if ( iTemp == -1 && 1 != mp_oSimManager->GetCurrentTimestep())
        {
          fDecLogs = 0;
          //Get the amount by which all previous timesteps' fresh logs
          //have decayed - this is this timestep's new decayed logs
          for ( k = 1; k < m_iMaxDecayTimesteps; k++ )
          {
            mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iFLogCalcsCode[k], & fTemp );
            fDecLogs += fTemp * (mp_fFLogDecayProp[k-1] - mp_fFLogDecayProp[k]);
          }

          //Set this value in the first portion of the decayed log array
          mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iDecLogCalcsCode[0], fDecLogs );

          //Add in the decay amounts of all the other positions in the array
          for ( k = 1; k < m_iMaxDecayTimesteps; k++ )
          {
            mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iDecLogCalcsCode[k], & fTemp );
            fDecLogs += fTemp * mp_fDecLogDecayProp[k];
          }

          //Set our finished decayed log value
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iDecLogCode, fDecLogs );
        }
        else {
          //Get the decayed log value for calculating litter and moss
          mp_oSubstrateGrid->GetValueOfCell( iX, iY, m_iDecLogCode, &fDecLogs );
        }

        p_oPackage = mp_oSubstrateGrid->GetFirstPackageOfCell( iX, iY );
        if ( p_oPackage )
        {

          //***************************************
          // Decay packages to get tipup, fresh logs, and scarified soil
          //***************************************
          fScarSoil = 0;
          fTipup = 0;
          fFreshLogs = 0;

          while ( p_oPackage )
          {
            p_oNextPackage = p_oPackage->GetNextPackage();

            p_oPackage->GetValue( m_iPkgAgeCode, & iAge );

            //Scarified soil
            p_oPackage->GetValue( m_iPkgScarSoilCode, & fTemp );
            fScarSoil += fTemp * mp_fScarSoilDecayProp[iAge];

            //Tipup
            p_oPackage->GetValue( m_iPkgTipupCode, & fTemp );
            fTipup += fTemp * mp_fTipupDecayProp[iAge];

            //Fresh logs
            p_oPackage->GetValue( m_iPkgFreshLogCode, & fTemp );
            fFreshLogs += fTemp * mp_fFLogDecayProp[iAge];

            //Increment the package's age
            iAge++;
            //If the package has reached the max decay age, delete it
            if ( iAge >= m_iMaxDecayTimesteps )
            {
              mp_oSubstrateGrid->DeletePackage( p_oPackage );
            }
            else
            {
              p_oPackage->SetValue( m_iPkgAgeCode, iAge );
            }
            p_oPackage = p_oNextPackage;
          } //end of while (p_oPackage)

          //Assign the package totals back to the main substrate
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iScarSoilCode, fScarSoil );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iTipupCode, fTipup );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFreshLogCode, fFreshLogs );
        }
        else
        {

          //There were no substrate cohort packages.  Retrieve the proportions
          //for the other substrate types so forest floor litter and moss can
          //be re-calculated.
          mp_oSubstrateGrid->GetValueOfCell( iX, iY, m_iScarSoilCode, & fScarSoil );
          mp_oSubstrateGrid->GetValueOfCell( iX, iY, m_iTipupCode, & fTipup );
          mp_oSubstrateGrid->GetValueOfCell( iX, iY, m_iFreshLogCode, & fFreshLogs );
        }

        //***************************************
        // Forest floor and error correction
        //***************************************

        fTotalSubstrate = fScarSoil + fTipup + fFreshLogs + fDecLogs;

        //Check that the proportions are less than 1
        if ( fTotalSubstrate <= 1 ) {

          //Calculate the forest floor litter and moss values as the remainder
          //needed to make the total substrate proportions add up to 1 and
          //assign
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFFMossCode, (1 - fTotalSubstrate) * m_fMossProportion );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFFLitterCode, (1 - fTotalSubstrate) * (1 - m_fMossProportion) );

        } else
        {
          //The proportions add up to more than 1 - re-proportion them and
          //re-assign to the grid cell
          fScarSoil /= fTotalSubstrate;
          fTipup /= fTotalSubstrate;
          fFreshLogs /= fTotalSubstrate;
          fDecLogs /= fTotalSubstrate;
          fTotalSubstrate = fScarSoil + fTipup + fFreshLogs + fDecLogs;
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iScarSoilCode, fScarSoil );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iTipupCode, fTipup );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFreshLogCode, fFreshLogs );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFFMossCode, (1 - fTotalSubstrate) * m_fMossProportion );
          mp_oSubstrateGrid->SetValueOfCell( iX, iY, m_iFFLitterCode, (1 - fTotalSubstrate) * (1 - m_fMossProportion) );
        }
      } //end of for (iY = 0; iY < iNumYCells; iY++)
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
    stcErr.sFunction = "clSubstrate::DecaySubstrate" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// UpdateSubstrateAges()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::UpdateSubstrateAges()
{
  if ( 1 == mp_oSimManager->GetCurrentTimestep() ) return;

  float fVal;
  int iNumXCells = mp_oCalcGrid->GetNumberXCells(), iNumYCells = mp_oCalcGrid->GetNumberYCells(), iX, iY, i;

  for ( iX = 0; iX < iNumXCells; iX++ )
  {
    for ( iY = 0; iY < iNumYCells; iY++ )
    {
      for ( i = m_iMaxDecayTimesteps - 2; i >= 0; i-- )
      {
        mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iFLogCalcsCode[i], & fVal );
        mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iFLogCalcsCode[i + 1], fVal );
        mp_oCalcGrid->GetValueOfCell( iX, iY, mp_iDecLogCalcsCode[i], & fVal );
        mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iDecLogCalcsCode[i + 1], fVal );
      }
      //Zero out this timestep's value
      fVal = 0;
      mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iFLogCalcsCode[0], fVal );
      mp_oCalcGrid->SetValueOfCell( iX, iY, mp_iDecLogCalcsCode[0], fVal );

      //Zero out the tipup value
      mp_oCalcGrid->SetValueOfCell( iX, iY, m_iTipupCalcsCode, fVal );
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CalculateDecayProportions()
////////////////////////////////////////////////////////////////////////////
void clSubstrate::CalculateDecayProportions()
{
  float fNumYearsPerTS = mp_oSimManager->GetNumberOfYearsPerTimestep(),
      fTemp;
  int i;

  mp_fFLogDecayProp = new float[m_iMaxDecayTimesteps + 1];
  mp_fScarSoilDecayProp = new float[m_iMaxDecayTimesteps + 1];
  mp_fTipupDecayProp = new float[m_iMaxDecayTimesteps + 1];
  mp_fDecLogDecayProp = new float[m_iMaxDecayTimesteps + 1];

  for (i = 0; i < m_iMaxDecayTimesteps + 1; i++) {
    //Make sure we don't crash if beta = 0 when t = 0 - we know the intention
    if (0 == m_fFreshLogB) fTemp = 1;
    else fTemp = pow((i * fNumYearsPerTS), m_fFreshLogB);
    mp_fFLogDecayProp[i] = exp(m_fFreshLogA * fTemp);

    if (0 == m_fScarifiedSoilB) fTemp = 1;
    else fTemp = pow((i * fNumYearsPerTS), m_fScarifiedSoilB);
    mp_fScarSoilDecayProp[i] = exp(m_fScarifiedSoilA * fTemp);

    if (0 == m_fTipUpB) fTemp = 1;
    else fTemp = pow((i * fNumYearsPerTS), m_fTipUpB);
    mp_fTipupDecayProp[i] = exp(m_fTipUpA * fTemp);

    if (0 == m_fDecayedLogB) fTemp = 1;
    else fTemp = pow((i * fNumYearsPerTS), m_fDecayedLogB);
    mp_fDecLogDecayProp[i] = exp(m_fDecayedLogA * fTemp);
  }
}
