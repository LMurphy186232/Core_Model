//---------------------------------------------------------------------------
#include "SpatialDisperse.h"
//---------------------------------------------------------------------------
#include "SimManager.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "Plot.h"
#include "ModelMath.h"
#include <math.h>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
clSpatialDispersal::clSpatialDispersal( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clDisperseBase( p_oSimManager)
{
  try
  {
    m_sNameString = "spatialdisperse";
    m_iNumFunctions = 2;
    m_iNumCovers = 2;

    mp_fDbhForReproduction = NULL;
    mp_fDispersalX0 = NULL;
    mp_fThetaXb = NULL;
    mp_fStr = NULL;
    mp_fBeta = NULL;
    mp_fFecundity = NULL;
    mp_fCumProb = NULL;
    mp_iIndexes = NULL;
    mp_iWhatFunction = NULL;
    mp_fStumpStr = NULL;
    mp_fStumpBeta = NULL;
    mp_fStumpFecundity = NULL;
    mp_bIsUsed = NULL;
    m_cQuery = NULL;

   m_iNumYearsPerTimestep = 0;
   m_iMaxDistance = 0;
   m_bStumps = false;
   m_bIsGap = false;
   m_iMaxGapDensity = 0;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

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
    stcErr.sFunction = "clSpatialDispersal::clSpatialDispersal" ;
    throw( stcErr );
  }
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
clSpatialDispersal::~clSpatialDispersal()
{
  short int i, j, k; //loop counters

  //Test to see if the arrays have been declared before deleting - since the
  //number of covers and functions will always be greater than 0 since they're
  //set in the constructor, the for loops would otherwise always execute and an
  //error would happen if the arrays hadn't been declared
  if ( mp_fDispersalX0 )
  {
    for ( i = 0; i < m_iNumFunctions; i++ )
    {
      for ( j = 0; j < m_iNumCovers; j++ )
      {
        delete[] mp_fDispersalX0[i] [j];
        delete[] mp_fThetaXb[i] [j];
        delete[] mp_fStr[i] [j];
        delete[] mp_fBeta[i] [j];
        delete[] mp_fFecundity[i] [j];
        if ( mp_fCumProb[i] [j] )
        {
          for ( k = 0; k < m_iNumBehaviorSpecies; k++ )
            delete[] mp_fCumProb[i] [j] [k];
        }
        delete[] mp_fCumProb[i] [j];
      }
      delete[] mp_fDispersalX0[i];
      delete[] mp_fThetaXb[i];
      delete[] mp_fStr[i];
      delete[] mp_fBeta[i];
      delete[] mp_fFecundity[i];
      delete[] mp_fCumProb[i];
    }
    for ( i = 0; i < m_iNumCovers; i++ )
      delete[] mp_iWhatFunction[i];
  }

  if ( mp_bIsUsed )
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_bIsUsed[i];

  delete[] mp_fDbhForReproduction;
  delete[] mp_fDispersalX0;
  delete[] mp_fThetaXb;
  delete[] mp_fStr;
  delete[] mp_fBeta;
  delete[] mp_fFecundity;
  delete[] mp_fCumProb;
  delete[] mp_iIndexes;
  delete[] mp_iWhatFunction;
  delete[] mp_fStumpStr;
  delete[] mp_fStumpBeta;
  delete[] mp_fStumpFecundity;
  delete[] mp_bIsUsed;
  delete[] m_cQuery;

  delete[] mp_iAllowedFileTypes; mp_iAllowedFileTypes = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// DeclareArrays
///////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::DeclareArrays()
{
  int i, j, k;
  //Declare our data arrays
  mp_fDbhForReproduction = new double[m_iTotalSpecies];

  mp_fDispersalX0 = new double * * [m_iNumFunctions];
  mp_fThetaXb = new double * * [m_iNumFunctions];
  mp_fBeta = new double * * [m_iNumFunctions];
  mp_fStr = new double * * [m_iNumFunctions];
  mp_fFecundity = new double * * [m_iNumFunctions];
  mp_fCumProb = new double * * * [m_iNumFunctions];
  if ( m_bStumps )
  {
    mp_fStumpStr = new double[m_iNumBehaviorSpecies];
    mp_fStumpBeta = new double[m_iNumBehaviorSpecies];
    mp_fStumpFecundity = new double[m_iNumBehaviorSpecies];
  }

  for ( i = 0; i < m_iNumFunctions; i++ )
  {
    mp_fDispersalX0[i] = new double * [m_iNumCovers];
    mp_fThetaXb[i] = new double * [m_iNumCovers];
    mp_fBeta[i] = new double * [m_iNumCovers];
    mp_fStr[i] = new double * [m_iNumCovers];
    mp_fFecundity[i] = new double * [m_iNumCovers];
    mp_fCumProb[i] = new double * * [m_iNumCovers];


    for ( j = 0; j < m_iNumCovers; j++ )
    {
      if ( canopy == j || m_bIsGap )
      {
        mp_fDispersalX0[i] [j] = new double[m_iNumBehaviorSpecies];
        mp_fThetaXb[i] [j] = new double[m_iNumBehaviorSpecies];
        mp_fBeta[i] [j] = new double[m_iNumBehaviorSpecies];
        mp_fStr[i] [j] = new double[m_iNumBehaviorSpecies];
        mp_fFecundity[i] [j] = new double[m_iNumBehaviorSpecies];
        mp_fCumProb[i] [j] = new double * [m_iNumBehaviorSpecies];

        //Declare these later - only those that are needed
        for ( k = 0; k < m_iNumBehaviorSpecies; k++ )
          mp_fCumProb[i] [j] [k] = NULL;
      }
      else
      {
        mp_fDispersalX0[i] [j] = NULL;
        mp_fThetaXb[i] [j] = NULL;
        mp_fBeta[i] [j] = NULL;
        mp_fStr[i] [j] = NULL;
        mp_fFecundity[i] [j] = NULL;
        mp_fCumProb[i] [j] = NULL;
      }
    }
  }

  mp_iWhatFunction = new function * [m_iNumCovers];
  for ( i = 0; i < m_iNumCovers; i++ )
    mp_iWhatFunction[i] = new function[m_iNumBehaviorSpecies];

  mp_iIndexes = new short int[m_iTotalSpecies];

  //Make the list of indexes
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_iIndexes[mp_iWhatSpecies[i]] = i;

}


///////////////////////////////////////////////////////////////////////////////
// DoShellSetup
///////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    int i;
    bool bAdults = false, bSaplings = false;

    m_iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    m_bStumps = false;

    //Find out what types this is being applied to
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType )
        bAdults = true;
      else if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType )
        bSaplings = true;
      else if ( clTreePopulation::stump == mp_whatSpeciesTypeCombos[i].iType )
        m_bStumps = true;
    }

    //Assemble the type query
    m_cQuery = new char[15];
    if ( bAdults && !bSaplings )
      sprintf( m_cQuery, "%s%d", "type=", clTreePopulation::adult );
    else if ( !bAdults && bSaplings )
      sprintf( m_cQuery, "%s%d", "type=", clTreePopulation::sapling );
    else
      sprintf( m_cQuery, "%s%d%s%d", "type=", clTreePopulation::sapling, ",", clTreePopulation::adult );

    DeclareArrays();

    GetParameterFileData( p_oDoc );

    PopulateUsedTable( p_oPop );

    CalcFecAndFunctions();
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
    stcErr.sFunction = "clSpatialDispersal::DoShellSetup" ;
    throw( stcErr );
  }
}


///////////////////////////////////////////////////////////////////////////////
// PopulateUsedTable()
///////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::PopulateUsedTable( clTreePopulation * p_oPop )
{
  short int iNumTypes = p_oPop->GetNumberOfTypes(), i, j; //loop counters

  //Declare the table - it's # species by # types
  mp_bIsUsed = new bool * [m_iTotalSpecies];
  for ( i = 0; i < m_iTotalSpecies; i++ )
  {
    mp_bIsUsed[i] = new bool[iNumTypes];
    for ( j = 0; j < iNumTypes; j++ )
      mp_bIsUsed[i] [j] = false;
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Make sure that the types are only sapling, adult, or stump
    if ( mp_whatSpeciesTypeCombos[i].iType != clTreePopulation::sapling
         && mp_whatSpeciesTypeCombos[i].iType != clTreePopulation::adult
         && mp_whatSpeciesTypeCombos[i].iType != clTreePopulation::stump )
         {
           modelErr stcErr;
           stcErr.sFunction = "clSpatialDisperse::PopulateUsedTable" ;
           stcErr.sMoreInfo = "Dispersal may only be applied to saplings and adults.";
           stcErr.iErrorCode = BAD_DATA;
           throw( stcErr );
    }
    mp_bIsUsed[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] = true;
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetParameterFileData
///////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::GetParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  intVal * p_iTemp = NULL; //for getting species-specific values
  try {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

    short int iNumFunctionSpecies, //number of species using a given function
         i; //loop counter

    //Start with which species use which function - influences the rest of
    //the file's reading
    //Set up our temp int array and pre-load with this behavior's species
    p_iTemp = new intVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_iTemp[i].code = mp_iWhatSpecies[i];

    p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTemp[i].code = mp_iWhatSpecies[i];

    //Minimum DBH for reproduction
    //Pre-load the minimum dbh for reproduction array with the minimum adult dbh
    //values.  Later we'll overwrite all values for those species which use
    //this behavior, leaving defaults for those that don't
    for ( i = 0; i < m_iTotalSpecies; i++ )
    {
      mp_fDbhForReproduction[i] = p_oPop->GetMinAdultDBH( i );
    }
    FillSpeciesSpecificValue( p_oElement, "di_minDbhForReproduction", "di_mdfrVal", p_fTemp,
       m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fDbhForReproduction[i] = p_fTemp[i].val;

    //Canopy functions
    FillSpeciesSpecificValue( p_oElement, "di_canopyFunction", "di_cfVal", p_iTemp, m_iNumBehaviorSpecies, p_oPop, true );

    //Transfer the values over - if any are not a valid function enum number,
    //throw an error
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( p_iTemp[i].val < 0 || p_iTemp[i].val >= m_iNumFunctions )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
        std::stringstream s;
        s << "Invalid disperse function code " << p_iTemp[i].val;
        stcErr.sMoreInfo = s.str();
        throw( stcErr );
      }
      else
      {
        mp_iWhatFunction[canopy] [mp_iIndexes[mp_iWhatSpecies[i]]] = (function)p_iTemp[i].val;
      }

    //**********************************
    //Weibull canopy parameters
    //**********************************
    //Start by getting a list of species that use weibull for canopy - we'll only
    //require parameters for those species
    //First count 'em
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( weibull == mp_iWhatFunction[canopy] [i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTemp; p_fTemp = NULL;
    p_fTemp = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( weibull == mp_iWhatFunction[canopy] [i] ) {
        p_fTemp[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 ) {

      //Dispersal
      FillSpeciesSpecificValue( p_oElement, "di_weibullCanopyDispersal", "di_wcdVal", p_fTemp,
         iNumFunctionSpecies, p_oPop, true );
      //Transfer to permanent array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        mp_fDispersalX0[weibull] [canopy] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Theta
      FillSpeciesSpecificValue( p_oElement, "di_weibullCanopyTheta", "di_wctVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to permanent array
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        mp_fThetaXb[weibull] [canopy] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
        if (p_fTemp[i].val >= 50) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
          stcErr.sMoreInfo = "One or more weibull theta values is too large and will cause math errors.";
          throw( stcErr );
        }
      }

      //Beta
      FillSpeciesSpecificValue( p_oElement, "di_weibullCanopyBeta", "di_wcbVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to permanent array
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        mp_fBeta[weibull] [canopy] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
        if (p_fTemp[i].val > 25) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
          stcErr.sMoreInfo = "One or more beta values is too large and will cause math errors.";
          throw( stcErr );
        }
      }

      //STR
      FillSpeciesSpecificValue( p_oElement, "di_weibullCanopySTR", "di_wcsVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to permanent array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        mp_fStr[weibull] [canopy] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
    } //end of if (iNumFunctionSpecies > 0) {

    //**********************************
    //Lognormal canopy parameters
    //**********************************
    //Start by getting a list of species that use lognormal for canopy - we'll
    //only require parameters for those species
    //First count 'em
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( lognormal == mp_iWhatFunction[canopy] [i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTemp; p_fTemp = NULL;
    p_fTemp = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      if ( lognormal == mp_iWhatFunction[canopy] [i] )
      {
        p_fTemp[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {

      //X0
      FillSpeciesSpecificValue( p_oElement, "di_lognormalCanopyX0", "di_lcx0Val", p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to permanent array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        mp_fDispersalX0[lognormal] [canopy] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Xb
      FillSpeciesSpecificValue( p_oElement, "di_lognormalCanopyXb", "di_lcxbVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to permanent array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        mp_fThetaXb[lognormal] [canopy] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Beta
      FillSpeciesSpecificValue( p_oElement, "di_lognormalCanopyBeta", "di_lcbVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to permanent array
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        mp_fBeta[lognormal] [canopy] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
        if (p_fTemp[i].val > 25) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
          stcErr.sMoreInfo = "One or more beta values is too large and will cause math errors.";
          throw( stcErr );
        }
      }

      //STR
      FillSpeciesSpecificValue( p_oElement, "di_lognormalCanopySTR", "di_lcsVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
      //Transfer to permanent array
      for ( i = 0; i < iNumFunctionSpecies; i++ )
        mp_fStr[lognormal] [canopy] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
    } //end of if (iNumFunctionSpecies > 0)

    if ( m_bIsGap )
    {

      //Maximum gap density
      FillSingleValue( p_oElement, "di_maxGapDensity", & m_iMaxGapDensity, true );

      //Load gap values
      //Gap functions
      FillSpeciesSpecificValue( p_oElement, "di_gapFunction", "di_gfVal", p_iTemp, m_iNumBehaviorSpecies, p_oPop, true );

      //Transfer the values over - if any are not a valid function enum number,
      //throw an error
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        if ( p_iTemp[i].val < 0 || p_iTemp[i].val >= m_iNumFunctions )
        {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
          std::stringstream s;
          s << "Invalid disperse function code " << p_iTemp[i].val;
          stcErr.sMoreInfo = s.str();
          throw( stcErr );
        }
        else
          mp_iWhatFunction[gap] [mp_iIndexes[mp_iWhatSpecies[i]]] = (function)p_iTemp[i].val;

      //**********************************
      //Weibull gap parameters
      //**********************************
      //Start by getting a list of species that use weibull for gap - we'll only
      //require parameters for those species
      //First count 'em
      iNumFunctionSpecies = 0;
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        if ( weibull == mp_iWhatFunction[gap] [i] )
          iNumFunctionSpecies++;

      //Now declare the float temp array to match and load with these species
      delete[] p_fTemp; p_fTemp = NULL;
      p_fTemp = new doubleVal[iNumFunctionSpecies];
      iNumFunctionSpecies = 0;
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      {
        if ( weibull == mp_iWhatFunction[gap] [i] )
        {
          p_fTemp[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
          iNumFunctionSpecies++;
        }
      }

      if ( iNumFunctionSpecies > 0 )
      {

        //Dispersal
        FillSpeciesSpecificValue( p_oElement, "di_weibullGapDispersal", "di_wgdVal", p_fTemp,
           iNumFunctionSpecies, p_oPop, true );
        //Transfer to permanent array
        for ( i = 0; i < iNumFunctionSpecies; i++ )
          mp_fDispersalX0[weibull] [gap] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

        //Theta
        FillSpeciesSpecificValue( p_oElement, "di_weibullGapTheta", "di_wgtVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
        //Transfer to permanent array
        for ( i = 0; i < iNumFunctionSpecies; i++ ) {
          mp_fThetaXb[weibull] [gap] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
          if (p_fTemp[i].val >= 50) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
            stcErr.sMoreInfo = "One or more weibull theta values is too large and will cause math errors.";
            throw( stcErr );
          }
        }

        //mp_fBeta
        FillSpeciesSpecificValue( p_oElement, "di_weibullGapBeta", "di_wgbVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
        //Transfer to permanent array
        for ( i = 0; i < iNumFunctionSpecies; i++ ) {
          mp_fBeta[weibull] [gap] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
          if (p_fTemp[i].val > 25) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
            stcErr.sMoreInfo = "One or more beta values is too large and will cause math errors.";
            throw( stcErr );
          }
        }

        //mp_fStr
        FillSpeciesSpecificValue( p_oElement, "di_weibullGapSTR", "di_wgsVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
        //Transfer to permanent array
        for ( i = 0; i < iNumFunctionSpecies; i++ )
          mp_fStr[weibull] [gap] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
      } //end of if (iNumFunctionSpecies > 0) {

      //**********************************
      //Lognormal gap parameters
      //**********************************
      //Start by getting a list of species that use lognormal for gap - we'll only
      //require parameters for those species
      //First count 'em
      iNumFunctionSpecies = 0;
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        if ( lognormal == mp_iWhatFunction[gap] [i] )
          iNumFunctionSpecies++;

      //Now declare the float temp array to match and load with these species
      delete[] p_fTemp; p_fTemp = NULL;
      p_fTemp = new doubleVal[iNumFunctionSpecies];
      iNumFunctionSpecies = 0;
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      {
        if ( lognormal == mp_iWhatFunction[gap] [i] )
        {
          p_fTemp[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
          iNumFunctionSpecies++;
        }
      }

      if ( iNumFunctionSpecies > 0 )
      {

        //X0
        FillSpeciesSpecificValue( p_oElement, "di_lognormalGapX0", "di_lgx0Val", p_fTemp, iNumFunctionSpecies, p_oPop, true );
        //Transfer to permanent array
        for ( i = 0; i < iNumFunctionSpecies; i++ )
          mp_fDispersalX0[lognormal] [gap] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

        //Xb
        FillSpeciesSpecificValue( p_oElement, "di_lognormalGapXb", "di_lgxbVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
        //Transfer to permanent array
        for ( i = 0; i < iNumFunctionSpecies; i++ )
          mp_fThetaXb[lognormal] [gap] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

        //Beta
        FillSpeciesSpecificValue( p_oElement, "di_lognormalGapBeta", "di_lgbVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
        //Transfer to permanent array
        for ( i = 0; i < iNumFunctionSpecies; i++ ) {
          mp_fBeta[lognormal] [gap] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
          if (p_fTemp[i].val > 25) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
            stcErr.sMoreInfo = "One or more beta values is too large and will cause math errors.";
            throw( stcErr );
          }
        }

        //STR
        FillSpeciesSpecificValue( p_oElement, "di_lognormalGapSTR", "di_lgsVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );
        //Transfer to permanent array
        for ( i = 0; i < iNumFunctionSpecies; i++ )
          mp_fStr[lognormal] [gap] [mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
      }
    }

    //**********************************
    //Stump values
    //**********************************
    if ( m_bStumps )
    {
      //Get a list of the species that have reproducing stumps
      iNumFunctionSpecies = 0;
      for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
        if ( clTreePopulation::stump == mp_whatSpeciesTypeCombos[i].iType )
          iNumFunctionSpecies++;

      //Now declare the float temp array to match and load with these species
      delete[] p_fTemp; p_fTemp = NULL;
      p_fTemp = new doubleVal[iNumFunctionSpecies];
      iNumFunctionSpecies = 0;
      for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
        if ( clTreePopulation::stump == mp_whatSpeciesTypeCombos[i].iType )
        {
          p_fTemp[iNumFunctionSpecies].code = mp_whatSpeciesTypeCombos[i].iSpecies;
          iNumFunctionSpecies++;
        }

      //Stump STR values
      FillSpeciesSpecificValue( p_oElement, "di_suckerSTR", "di_ssVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );

      for ( i = 0; i < iNumFunctionSpecies; i++ )
        mp_fStumpStr[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;

      //Stump Beta values
      FillSpeciesSpecificValue( p_oElement, "di_suckerBeta", "di_sbVal", p_fTemp, iNumFunctionSpecies, p_oPop, true );

      for ( i = 0; i < iNumFunctionSpecies; i++ )
        mp_fStumpBeta[mp_iIndexes[p_fTemp[i].code]] = p_fTemp[i].val;
    } //end of if (m_bStumps)

    delete[] p_iTemp;
    delete[] p_fTemp;
  }
  catch ( modelErr & err )
  {
    delete[] p_iTemp;
    delete[] p_fTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_iTemp;
    delete[] p_fTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_iTemp;
    delete[] p_fTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSpatialDispersal::GetParameterFileData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalcFecAndFunctions
///////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::CalcFecAndFunctions()
{
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  float fXPlotLength = p_oPlot->GetXPlotLength(), fYPlotLength = p_oPlot->GetYPlotLength();
  int i, j, k;
  clDisperseBase::function iFunction;
  clDisperseBase::cover iCover;

  //Calculate the maximum dispersal distance - which is the maximum dimension
  //of the grid with max of 1000 m
  if ( fXPlotLength > fYPlotLength )
    if ( fXPlotLength > 1000 )
      m_iMaxDistance = 1000;
    else
      m_iMaxDistance = (int)fXPlotLength;
  else if ( fYPlotLength > 1000 )
    m_iMaxDistance = 1000;
  else
    m_iMaxDistance = (int)fYPlotLength;

  //**********************************
  //Calculate fecundity and cumulative probability arrays
  //**********************************
  if ( m_bIsGap )
  {
    for ( i = 0; i < m_iNumFunctions; i++ ) {
      if (weibull == i) iFunction = weibull;
      else iFunction = lognormal;
      for ( j = 0; j < m_iNumCovers; j++ ) {
      	if (gap == j) iCover = gap;
      	else iCover = canopy;
        for ( k = 0; k < m_iNumBehaviorSpecies; k++ ) {
          if ( i == mp_iWhatFunction[j] [mp_iIndexes[mp_iWhatSpecies[k]]] )
          {

            mp_fFecundity[i] [j] [mp_iIndexes[mp_iWhatSpecies[k]]] =
                 mp_fStr[i] [j] [mp_iIndexes[mp_iWhatSpecies[k]]]
                 / pow( 30, mp_fBeta[i] [j] [mp_iIndexes[mp_iWhatSpecies[k]]] );

            mp_fCumProb[i] [j] [k] = new double[m_iMaxDistance];

            CalculateProbabilityDistribution( mp_fCumProb[i] [j] [k], m_iMaxDistance, mp_iWhatSpecies[k], iFunction, iCover );
          }
        }
      }
    }
  }
  else
  {
    for ( i = 0; i < m_iNumFunctions; i++ ) {
      if (weibull == i) iFunction = weibull;
      else iFunction = lognormal;
      for ( k = 0; k < m_iNumBehaviorSpecies; k++ ) {
        if ( i == mp_iWhatFunction[canopy] [mp_iIndexes[mp_iWhatSpecies[k]]] )
        {

          mp_fFecundity[i] [canopy] [mp_iIndexes[mp_iWhatSpecies[k]]] =
               mp_fStr[i] [canopy] [mp_iIndexes[mp_iWhatSpecies[k]]]
               / pow( 30, mp_fBeta[i] [canopy] [mp_iIndexes[mp_iWhatSpecies[k]]] );

          mp_fCumProb[i] [canopy] [k] = new double[m_iMaxDistance];

          CalculateProbabilityDistribution( mp_fCumProb[i] [canopy] [k], m_iMaxDistance, mp_iWhatSpecies[k], iFunction, canopy );
        }
      }
    }
  }

  if ( m_bStumps )
  {
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::stump == mp_whatSpeciesTypeCombos[i].iType )
        mp_fStumpFecundity[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]] =
             mp_fStumpStr[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
             / pow( 30, mp_fStumpBeta[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]] );
  }
}


//////////////////////////////////////////////////////////////////////////////
// CalculateProbabilityDistribution()
/////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::CalculateProbabilityDistribution( double * p_fProbArray, int iMaxDistance, int iSpecies,
     function iFunction, cover iCover )
     {
       try
       {
         float fCumProb = 0, //area under the probability distribution function curve -
              //sum of all the probabilities for each distance
              fInc, //incremental probability between adjacent units
              fNormalizer, //normalizing factor for cumulative probability array
              fErr = 0.0001; //prevents underflow errors
         int iDistance; //distance in meters - loop counter


         //Calculate cumulative probability for seed dispersal from 0 out to the
         //maximum dispersal distance
         for ( iDistance = 0; iDistance < iMaxDistance; iDistance++ )
         {
           fInc = M_PI * (2 * (iDistance + 0.5) ) * CalculateFunctionValue( (iDistance + 0.5), iSpecies, iFunction, iCover );

           //Avoid dealing with super-small float-underflow type numbers
           if ( fInc < fErr )
             fInc = 0.0;

           //Add up all the probabilities as they are calculated
           fCumProb += fInc;
         }

         //Normalize the cumulative probability array
         //Error trap - otherwise fNormalizer overflows
         if ( fCumProb < fErr ) fNormalizer = 0;
         else
           fNormalizer = 1.0 / fCumProb;
         fCumProb = 0.0;

         for ( iDistance = 0; iDistance < iMaxDistance; iDistance++ )
         {
           //Calculate each increment again, this time taking it times the normalizer
           fInc = fNormalizer * M_PI * (2 * (iDistance + 0.5) )
                * CalculateFunctionValue( iDistance + 0.5, iSpecies, iFunction, iCover );
           //Add up the cumulative probability to this point - that's the value that
           //goes into the array
           fCumProb += fInc;
           //Don't want the value to go over 1
           if ( fCumProb > 1 ) fCumProb = 1;

           //Special case - the last bucket in the array should have a value of 1
           if ( iDistance == iMaxDistance - 1 )
             p_fProbArray[iDistance] = 1;
           else
             p_fProbArray[iDistance] = fCumProb;

         }

         //Walk out one more time.  Each value should be greater than the value in the
         //bucket before it (unless 1 has been reached), or it's an error.
         for ( iDistance = 1; iDistance < iMaxDistance; iDistance++ )
         {
           if ( p_fProbArray[iDistance] < p_fProbArray[iDistance - 1] )
           {
             modelErr stcErr;
             stcErr.iErrorCode = BAD_DATA;
             stcErr.sFunction = "clDisperseBase::CalculateProb..." ;
             stcErr.sMoreInfo = "Probability array does not increase sequentially.";
             throw( stcErr );
           }
         }
       }
       catch ( modelErr & fErr )
       {
         throw( fErr );
       }
       catch ( modelMsg & msg )
       {
         throw( msg );
       } //non-fatal error
       catch ( ... )
       {
         modelErr stcErr;
         stcErr.iErrorCode = UNKNOWN;
         stcErr.sFunction = "clSpatialDisperse::CalculateProbability..." ;
         throw( stcErr );
       }
}


///////////////////////////////////////////////////////////////////////////////
// AddSeeds()
/////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::AddSeeds()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTreeSearch * p_oAllTrees; //search object for getting all trees
    clTree * p_oTree; //for working with a single tree
    float fDbh; //tree's dbh
    short int iSp, iType; //species and type of a given tree

    if ( m_bIsGap )
    {
      CalculateGapStatus( p_oPop );
    }

    //Ask the tree population to find the trees in our query
    p_oAllTrees = p_oPop->Find( m_cQuery );

    //Go through the trees one at a time
    p_oTree = p_oAllTrees->NextTree();
    while ( p_oTree )
    {
      //Get the tree's species and type
      iSp = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      //Check to see if this species/type combo is used
      if ( mp_bIsUsed[iSp] [iType] )
      {

        //Get the tree's dbh
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iType ), & fDbh );

        //If the tree is larger than the minimum size for reproduction,
        //let it reproduce
        if ( fDbh >= mp_fDbhForReproduction[mp_iIndexes[iSp]] ) {

          if ( m_bIsGap )
          {
            SpatialDisperse( p_oTree, p_oPop, p_oPlot, fDbh );
          }
          else
          {
            NonSpatialDisperse( p_oTree, p_oPop, p_oPlot, fDbh );
          }
        }
      }

      p_oTree = p_oAllTrees->NextTree();
    } //end of while (p_oTree)

    //If anybody's using stumps, repeat with the stumps list
    if ( m_bStumps )
    {
      p_oTree = p_oPop->GetFirstStump();
      while ( p_oTree )
      {
        //Get the tree's species and type
        iSp = p_oTree->GetSpecies();
        iType = p_oTree->GetType();

        if ( mp_bIsUsed[iSp] [iType] )
        {

          //Get the tree's dbh
          p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iType ), & fDbh );

          //If the tree is larger than the minimum size for reproduction,
          //let it reproduce
          if ( fDbh >= mp_fDbhForReproduction[mp_iIndexes[iSp]] ) {

            if ( m_bIsGap )
            {
              SpatialDisperse( p_oTree, p_oPop, p_oPlot, fDbh );
            }
            else
            {
              NonSpatialDisperse( p_oTree, p_oPop, p_oPlot, fDbh );
            }
          }
        }
        p_oTree = p_oTree->GetTaller();
      }
    } //end of if (m_bStumps)
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
    stcErr.sFunction = "clSpatialDisperse::AddSeeds" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// SpatialDisperse
///////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::SpatialDisperse( clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, float fDbh )
{
  try
  {
    float fRand, //random number for determining aspects of a seed's placement
         //and survival
         fDistance, //distance from the parent tree at which a seed lands
         fNewDistance, //we have to redo distance sometimes
         fPrevBucket, //value in the previous bucket of the probability array
         fParentX, fParentY, //coordinates of tree
         fSeedX, fSeedY, //coordinates of seed
         fStepX, fStepY, //coords of intermediate pts between 2 seed positions
         fStartX, fStartY, //starting point of seedling walk
         fAngle, //angle from the parent tree from which a seed lands
         fSurvivalChance, //ratio of gap to canopy fecundity
         fStepDistance, //distance we cover in a step between two seed positions
         fNumGridSeeds, //number of seeds in the seedling grid
         fWalkDistance, //distance to walk when adjusting seed position
         fNumSeeds, //number of seeds produced by one tree
         f, //loop counter for working with seeds
         fXCellLength = mp_oSeedGrid->GetLengthXCells(), fYCellLength = mp_oSeedGrid->GetLengthYCells(), fCellLength; //shorter of the two above
    int iDistanceCounter = 0; //used to walk out the cumulative probability array
    short int iSp = p_oTree->GetSpecies(), //reproducing tree's species
         iType = p_oTree->GetType(), //reproducing tree's type
         iSpIndex = mp_iIndexes[iSp]; //array index for this species
    function iCanFunc = mp_iWhatFunction[canopy] [iSpIndex], iGapFunc = mp_iWhatFunction[gap] [iSpIndex], iFunction; //variable
    // for when we need to work with function
    cover iCover; //variable for when we need to work with cover
    bool bIsParentInGap, //gap status of parent's location
         bIsSeedInGap, //gap status of seed's location
         bSurvives; //whether or not seed survives

    if ( fXCellLength <= fYCellLength )
      fCellLength = fXCellLength;
    else
      fCellLength = fYCellLength;

    //***************************************
    //Establish how many seeds this tree produces
    //***************************************

    //Get the X and Y coords of the tree
    p_oTree->GetValue( p_oPop->GetXCode( iSp, iType ), & fParentX );
    p_oTree->GetValue( p_oPop->GetYCode( iSp, iType ), & fParentY );

    //Get gap status at parent's location
    mp_oSeedGrid->GetValueAtPoint( fParentX, fParentY, m_iIsGapCode, & bIsParentInGap );

    //Calculate number of seeds produced per sq meter and the gap:canopy
    //fecundity ratio - seedling survival is based on this
    if ( clTreePopulation::stump == iType )
    {

      fNumSeeds = clModelMath::Round( mp_fStumpFecundity[iSpIndex] * pow( fDbh, mp_fStumpBeta[iSpIndex] ), 0 ) * m_iNumYearsPerTimestep;

      //For stumps - I'm kind of guessing here.  I don't have confirmation that
      //this is how this is supposed to be calculated.
      fSurvivalChance = ( mp_fStumpStr[iSpIndex] * ( pow( ( fDbh / 30 ), mp_fStumpBeta[iSpIndex] ) ) )
           / ( mp_fStr[iGapFunc] [gap] [iSpIndex] * ( pow( ( fDbh / 30 ), mp_fBeta[iGapFunc] [gap] [iSpIndex] ) ) );

    }
    else
    { //this is not a stump

      //To calculate the number of seeds for live trees, use the maximum of gap
      //STR and canopy STR.
      if ( mp_fStr[iCanFunc] [canopy] [iSpIndex] >= mp_fStr[iGapFunc] [gap] [iSpIndex] )
      {
        fNumSeeds = GetNumberOfSeeds( fDbh, iSp, canopy, iCanFunc );
      }
      else
      {
        fNumSeeds = GetNumberOfSeeds( fDbh, iSp, gap, iGapFunc );
      }

      fSurvivalChance = ( mp_fStr[iGapFunc] [gap] [iSpIndex] * ( pow( ( fDbh / 30 ), mp_fBeta[iGapFunc] [gap] [iSpIndex] ) ) )
           / ( mp_fStr[iCanFunc] [canopy] [iSpIndex] * ( pow( ( fDbh / 30 ), mp_fBeta[iCanFunc] [canopy] [iSpIndex] ) ) );
    }

    //***************************************
    //For each of these seeds, find its landing spot
    //***************************************
    for ( f = 0; f < fNumSeeds; f++ )
    {

      //***************************************
      //Get the seed's first position
      //***************************************

      //Use the cumulative probability array to determine how far away from the
      //parent tree the seed lands - use a random number and find the first
      //probability array bucket with a value greater than the random
      fRand = clModelMath::GetRand();
      if ( fRand == 0 )
        fRand = 0.0001; //prevent error when the probability at 1 meter is 0

      if ( bIsParentInGap )
      {
        iCover = gap;
        iFunction = iGapFunc;
      }
      else
      {
        iCover = canopy;
        iFunction = iCanFunc;
      }
      iDistanceCounter = 0;
      while ( fRand > mp_fCumProb[iFunction] [iCover] [iSpIndex] [iDistanceCounter] )
        iDistanceCounter++;

      //Get the value in the array bucket before the target one
      fPrevBucket = ( iDistanceCounter == 0 ) ? 0.0 : mp_fCumProb[iFunction] [iCover] [iSpIndex] [iDistanceCounter - 1];

      //Calculate the distance at which the seed will land
      fDistance = ( iDistanceCounter + ( fRand - fPrevBucket )
           / ( mp_fCumProb[iFunction] [iCover] [iSpIndex] [iDistanceCounter] - fPrevBucket ) );

      //Get a random direction to pitch the seed
      fRand = clModelMath::GetRand();
      fAngle = 2.0 * M_PI * ( fRand );

      //Using the angle and distance, get an X and Y value for the seed
      fSeedX = p_oPlot->CorrectX( p_oPlot->GetUncorrectedX( fParentX, fAngle, fDistance ) );
      fSeedY = p_oPlot->CorrectY( p_oPlot->GetUncorrectedY( fParentY, fAngle, fDistance ) );

      //Get the gap status for this location
      mp_oSeedGrid->GetValueAtPoint( fSeedX, fSeedY, m_iIsGapCode, & bIsSeedInGap );

      //Determine the likelihood of seedling survival based on ratio of gap STR
      //to canopy STR and the gap status of the location in which the seedling
      //seeks to establish itself.  If a seed is in the cover type with the
      //higher STR, it automatically survives; otherwise it has to go up against
      //a random number to see if it lives
      if ( fSurvivalChance < 1.0 )
      { //lower gap STR
        if ( bIsSeedInGap )
        {
          fRand = clModelMath::GetRand();
          if ( fRand <= fSurvivalChance ) bSurvives = true;
          else
            bSurvives = false;
        }
        else
          bSurvives = true; //if in canopy - automatic survival
      }
      else if ( fSurvivalChance > 1.0 )
      { //lower canopy STR
        if ( !bIsSeedInGap )
        {
          fRand = clModelMath::GetRand();
          if ( fRand <= 1 / fSurvivalChance ) bSurvives = true;
          else
            bSurvives = false;
        }
        else
          bSurvives = true; //if in gap - automatic survival
      }
      else
        bSurvives = true; //gap and canopy STRs are equal

      if ( !bSurvives ) goto nextSeed;

      //***************************************
      //Make any necessary corrections to seed position.  If both parent and seed
      //are in canopy, everything's fine the way it is.  If not, there may be
      //adjustments.
      //***************************************
      if ( !bIsParentInGap && !bIsSeedInGap ) goto plantSeed;

      //Next scenario - parent is in gap and seedling is in canopy.  Recalculate
      //seed location as though the parent was also in canopy.  The shortest
      //distance is where the seed lands.
      else if ( bIsParentInGap && !bIsSeedInGap )
      {
        //Recalc dispersal distance as if it dispersed from canopy
        iDistanceCounter = 0;
        fRand = clModelMath::GetRand();
        while ( fRand > mp_fCumProb[iCanFunc] [canopy] [iSpIndex] [iDistanceCounter] )
          iDistanceCounter++;
        fPrevBucket = ( iDistanceCounter == 0 ) ? 0.0 : mp_fCumProb[iCanFunc] [canopy] [iSpIndex] [iDistanceCounter - 1];
        fNewDistance = ( iDistanceCounter + ( fRand - fPrevBucket )
             / ( mp_fCumProb[iCanFunc] [canopy] [iSpIndex] [iDistanceCounter] - fPrevBucket ) );

        //If the new distance is smaller - redo landing coordinates
        if ( fNewDistance < fDistance )
        {
          fSeedX = p_oPlot->CorrectX( p_oPlot->GetUncorrectedX( fParentX, fAngle, fNewDistance ) );
          fSeedY = p_oPlot->CorrectY( p_oPlot->GetUncorrectedY( fParentY, fAngle, fNewDistance ) );
        }
      }
      else
      {

        //Next scenario - the seed lands in gap.  What we need to do is walk out
        //the line of the seed's path, checking each grid cell of the substrate/gap
        //status grid.  If any of the grid cells in the line are under canopy
        //cover, the seed drops in the first canopy cell it reaches

        //First thing:  prep the seed walk in both scenarios - parent in gap
        //or parent in canopy; each will set their starting and ending points
        if ( !bIsParentInGap )
        {

          //Calculate dispersal distance again as though parent were also in gap
          iDistanceCounter = 0;
          fRand = clModelMath::GetRand();
          while ( fRand > mp_fCumProb[iGapFunc] [gap] [iSpIndex] [iDistanceCounter] )
            iDistanceCounter++;
          fPrevBucket = ( iDistanceCounter == 0 ) ? 0.0 : mp_fCumProb[iGapFunc] [gap] [iSpIndex] [iDistanceCounter - 1];
          fNewDistance = ( iDistanceCounter + ( fRand - fPrevBucket )
               / ( mp_fCumProb[iGapFunc] [gap] [iSpIndex] [iDistanceCounter] - fPrevBucket ) );

          //If the canopy distance is greater than or equal to the gap distance,
          //no walk-out is necessary.  Skip out and go to planting.
          if ( fNewDistance <= fDistance ) goto plantSeed;

          //Nope, we've gotta do a seed walk-out.  The starting point is the
          //original seed canopy location, and the end is the gap location.
          fWalkDistance = fNewDistance - fDistance;
          fStartX = fSeedX;
          fStartY = fSeedY;

          //Get the new X and Y in case the seed isn't dropped on the way
          fSeedX = p_oPlot->CorrectX( p_oPlot->GetUncorrectedX( fParentX, fAngle, fNewDistance ) );
          fSeedY = p_oPlot->CorrectY( p_oPlot->GetUncorrectedY( fParentY, fAngle, fNewDistance ) );
        }
        else
        {
          //Parent's in gap - we know we've gotta walk out.  Starting point
          //is parent's location, end is seed's calculated gap location.
          fStartX = fParentX;
          fStartY = fParentY;
          fWalkDistance = fDistance;
        }

        //Walk-out time!  We've set up the loop - now go from the starting point
        //to the ending point and see if we hit canopy in the meantime
        for ( fStepDistance = fCellLength; fStepDistance <= fWalkDistance; fStepDistance += fCellLength )
        {
          //Compute the point location for this step
          fStepX = p_oPlot->CorrectX( p_oPlot->GetUncorrectedX( fStartX, fAngle, fStepDistance ) );
          fStepY = p_oPlot->CorrectY( p_oPlot->GetUncorrectedY( fStartY, fAngle, fStepDistance ) );

          //Under canopy?  If so, drop the seed here
          mp_oSeedGrid->GetValueAtPoint( fStepX, fStepY, m_iIsGapCode, & bIsSeedInGap );
          if ( !bIsSeedInGap )
          {
            fSeedX = fStepX;
            fSeedY = fStepY;
            break;
          }
        } //end of for (j = 1; j < iMaxSteps; j++)
      } //end of else - seed lands in gap

    plantSeed:

      //Increment the seed counter at the seed's location in the grid
      mp_oSeedGrid->GetValueAtPoint( fSeedX, fSeedY, mp_iNumSeedsCode[iSp], & fNumGridSeeds );
      fNumGridSeeds++;
      mp_oSeedGrid->SetValueAtPoint( fSeedX, fSeedY, mp_iNumSeedsCode[iSp], fNumGridSeeds );


    nextSeed:;
    } //end of for (i = 0; i < iNumSeeds; i++)
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
    stcErr.sFunction = "clSpatialDispersal::Disperse" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// NonSpatialDisperse
//////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::NonSpatialDisperse( clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, float fDbh )
{
  try
  {
    float fRand, //random number for determining distance and direction of
    //a seed's dispersal
    fDistance, //distance from the parent tree at which a seed lands
    fPrevBucket, //value in the previous bucket of the probability array
    fX, fY, //coordinates of tree
    fNumGridSeeds, //number of seeds in the seed grid
    fSeedX, fSeedY, //coordinates of seed
    fAngle, //angle from the parent tree from which a seed lands
    fNumSeeds, //number of seeds produced by the tree
    f; //loop counter
    int iDistanceCounter = 0; //used to walk out the cumulative probability array
    short int iSp = p_oTree->GetSpecies(), //reproducing tree's species
    iType = p_oTree->GetType(), //reproducing tree's type
    iSpIndex = mp_iIndexes[iSp];
    function iCanFunc = mp_iWhatFunction[canopy] [iSpIndex];

    //Get the X and Y coords of the tree
    p_oTree->GetValue( p_oPop->GetXCode( iSp, iType ), & fX );
    p_oTree->GetValue( p_oPop->GetYCode( iSp, iType ), & fY );

    //Calculate number of seeds produced, based on species
    //fecundity and the dbh of the tree
    fNumSeeds = GetNumberOfSeeds( fDbh, iSp, canopy, iCanFunc );

    //Disperse the seeds produced
    for ( f = 0; f < fNumSeeds; f++ )
    {
      //Use the cumulative probability array to determine how far away from the
      //parent tree the seed lands - use a random number and find the first
      //probability array bucket with a value greater than the random
      iDistanceCounter = 0;
      fRand = clModelMath::GetRand();
      while ( fRand > mp_fCumProb[iCanFunc] [canopy] [iSpIndex] [iDistanceCounter] )
        iDistanceCounter++;

      //Get the value in the array bucket before the target one
      fPrevBucket = ( iDistanceCounter == 0 ) ? 0.0 : mp_fCumProb[iCanFunc] [canopy] [iSpIndex] [iDistanceCounter - 1];

      //Calculate the distance at which the seed will land
      fDistance = ( iDistanceCounter + ( fRand - fPrevBucket )
           / ( mp_fCumProb[iCanFunc] [canopy] [iSpIndex] [iDistanceCounter] - fPrevBucket ) );

      //Get a random direction to pitch the seed
      fRand = clModelMath::GetRand();
      fAngle = 2.0 * M_PI * fRand;

      //Using the angle and distance, get an X and Y value for the seed
      fSeedX = p_oPlot->CorrectX( cos( fAngle ) * fDistance + fX );
      fSeedY = p_oPlot->CorrectY( sin( fAngle ) * fDistance + fY );

      //Increment the seed counter at the seed's location in the grid
      mp_oSeedGrid->GetValueAtPoint( fSeedX, fSeedY, mp_iNumSeedsCode[iSp], & fNumGridSeeds );
      fNumGridSeeds++;
      mp_oSeedGrid->SetValueAtPoint( fSeedX, fSeedY, mp_iNumSeedsCode[iSp], fNumGridSeeds );
    } //end of for (i = 0; i < iNumSeeds; i++)
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
    stcErr.sFunction = "clSpatialDisperse::NonSpatialDisperse" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// CalculateFunctionValue()
///////////////////////////////////////////////////////////////////////////*/
float clSpatialDispersal::CalculateFunctionValue( float fDistance, int iSpecies, function iFunction, cover iCover )
{
  if ( weibull == iFunction )
    return clModelMath::CalculateWeibullFunction( mp_fDispersalX0[iFunction] [iCover] [mp_iIndexes[iSpecies]],
        mp_fThetaXb[iFunction] [iCover] [mp_iIndexes[iSpecies]], fDistance );
  else
    return clModelMath::CalculateLognormalFunction( mp_fDispersalX0[iFunction] [iCover] [mp_iIndexes[iSpecies]],
        mp_fThetaXb[iFunction] [iCover] [mp_iIndexes[iSpecies]], fDistance );
}


///////////////////////////////////////////////////////////////////////////////
// CalculateGapStatus()
///////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::CalculateGapStatus( clTreePopulation * p_oPop )
{
  try
  {

    if ( m_bUpdatedGapStatus ) return;

    clTreeSearch * p_oAllTrees; //search object for getting all trees
    clTree * p_oTree; //for working with a single tree
    char cQuery[10];
    float fDbh, //tree's dbh
    fX, fY; //tree's X and Y coords
    int iCount; //for updating count value
    short int iSp, iType, //species and type of a given tree
    iNumXCells = mp_oSeedGrid->GetNumberXCells(), iNumYCells = mp_oSeedGrid->GetNumberYCells(), i, j; //loop counters

    //Set all current tree counts to 0
    iCount = 0;
    for ( i = 0; i < iNumXCells; i++ )
      for ( j = 0; j < iNumYCells; j++ )
        mp_oSeedGrid->SetValueOfCell( i, j, m_iGapCountCode, iCount );

    //Ask the tree population to find all saplings and adults.
    /** @todo make this more efficient by only asking for the trees needed */
    sprintf( cQuery, "%s%d%s%d", "type=", clTreePopulation::sapling, ",", clTreePopulation::adult );
    p_oAllTrees = p_oPop->Find( cQuery );

    //Go through the trees one at a time
    p_oTree = p_oAllTrees->NextTree();
    while ( p_oTree )
    {
      iSp = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      //Get the tree's dbh
      p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iType ), & fDbh );

      //If the tree is larger than the minimum size for reproduction,
      //lcount it
      if ( fDbh >= mp_fDbhForReproduction[iSp] )
      {

        //Get the tree's X and Y coordinates
        p_oTree->GetValue( p_oPop->GetXCode( iSp, iType ), & fX );
        p_oTree->GetValue( p_oPop->GetYCode( iSp, iType ), & fY );

        //Get the existing value of the tree count and increment
        mp_oSeedGrid->GetValueAtPoint( fX, fY, m_iGapCountCode, & iCount );
        iCount++;
        mp_oSeedGrid->SetValueAtPoint( fX, fY, m_iGapCountCode, iCount );
      }

      p_oTree = p_oAllTrees->NextTree();
    } //end of while (p_oTree)

    //Now go through each grid cell and access the count.  If it's larger than
    //the maximum acceptable density, set the "Is Gap" data member to false;
    //otherwise, set it to true
    for ( i = 0; i < iNumXCells; i++ )
      for ( j = 0; j < iNumYCells; j++ )
      {
        mp_oSeedGrid->GetValueOfCell( i, j, m_iGapCountCode, & iCount );
        if ( iCount <= m_iMaxGapDensity )
          mp_oSeedGrid->SetValueOfCell( i, j, m_iIsGapCode, true );
        else
          mp_oSeedGrid->SetValueOfCell( i, j, m_iIsGapCode, false );
      }

    m_bUpdatedGapStatus = true;
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
    stcErr.sFunction = "clSpatialDisperse::CalculateGapStatus" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// SetNameData()
////////////////////////////////////////////////////////////////////////////
void clSpatialDispersal::SetNameData(std::string sNameString)
{
  try
  {
    if (sNameString.compare("GapDisperse" ) == 0)
    {
      m_bIsGap = true;
      m_sXMLRoot = "GapDisperse";

    }
    else if (sNameString.compare("NonGapDisperse" ) == 0)
    {
      m_bIsGap = false;
      m_sXMLRoot = "NonGapDisperse";
    }
    else
    { //error!
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clSpatialDispersal::SetNameData" ;
      stcErr.sMoreInfo = "Unrecognized name ";
      stcErr.sMoreInfo += sNameString;
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
    stcErr.sFunction = "clSpatialDispersal::SetNameData" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// GetNumberOfSeeds()
////////////////////////////////////////////////////////////////////////////
float clSpatialDispersal::GetNumberOfSeeds(float fDbh, short int iSp, int iCover, int iFunc )
{
  return clModelMath::RandomRound( mp_fFecundity[iFunc] [iCover] [mp_iIndexes[iSp]]
       * pow( fDbh, mp_fBeta[iFunc] [iCover] [mp_iIndexes[iSp]] ) * m_iNumYearsPerTimestep );
}

/*/ ////////////////////////////////////////////////////////////////////////////
WriteCumProbArray
/////////////////////////////////////////////////////////////////////////////*/
/*void clSpatialDispersal::WriteCumProbArray() { fstream matrices; int iMaxDistance = mp_oDisperseOrg->GetMaxDistance();
short int n, iSp, iNumSpecies = mp_oDisperseOrg->GetNumberOfSpecies();

matrices.open("NewCumProb.xls", ios::out | ios::app); matrices << "Cumulative Probability Matrix - Weibull:\n";
matrices << "Canopy:\nSpecies:"; for (n = 0; n < iMaxDistance; n++) matrices << "\t" << n; matrices << "\n";
for (iSp = 0; iSp < iNumSpecies; iSp++) { if (mp_fCumProb[weibull][canopy][iSp]) {
matrices << "Species " << mp_iWhatSpecies[iSp] << "\t"; for (n = 0; n < iMaxDistance; n++)
matrices << mp_fCumProb[weibull][canopy][iSp][n] << "\t"; matrices << "\n"; } }

matrices << "\n\nGap:\nSpecies:"; for (n = 0; n < iMaxDistance; n++) matrices << "\t" << n; matrices << "\n";
for (iSp = 0; iSp < iNumSpecies; iSp++) { if (mp_fCumProb[weibull][gap][iSp]) {
matrices << "Species " << mp_iWhatSpecies[iSp] << "\t"; for (n = 0; n < iMaxDistance; n++)
matrices << mp_fCumProb[weibull][gap][iSp][n] << "\t"; matrices << "\n"; } }

matrices << "\n\nCumulative Probability Matrix - Lognormal:\nCanopy:\nSpecies:"; for (n = 0; n < iMaxDistance; n++)
matrices << "\t" << n; matrices << "\n"; for (iSp = 0; iSp < iNumSpecies; iSp++) { if (mp_fCumProb[lognormal][canopy][iSp]) {
matrices << "Species " << mp_iWhatSpecies[iSp] << "\t"; for (n = 0; n < iMaxDistance; n++)
matrices << mp_fCumProb[lognormal][canopy][iSp][n] << "\t"; matrices << "\n"; } }

matrices << "\n\nGap:\nSpecies:"; for (n = 0; n < iMaxDistance; n++) matrices << "\t" << n; matrices << "\n";
for (iSp = 0; iSp < iNumSpecies; iSp++) { if (mp_fCumProb[lognormal][gap][iSp]) {
matrices << "Species " << mp_iWhatSpecies[iSp] << "\t"; for (n = 0; n < iMaxDistance; n++)
matrices << mp_fCumProb[lognormal][gap][iSp][n] << "\t"; matrices << "\n"; } } matrices.close(); } */
//---------------------------------------------------------------------------
