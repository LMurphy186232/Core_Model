//---------------------------------------------------------------------------
// MerchValueCalculator.cpp
//---------------------------------------------------------------------------
#include "MerchValueCalculator.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "Plot.h"
#include "Constants.h"
#include <math.h>
#include <sstream>

//The amount of taper is found by multiplying the DBH by 0.4
#define MERCH_RATIO 0.4
#define MIN_DBH 10

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clMerchValueCalculator::clMerchValueCalculator( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "MerchValueCalculator";
    m_sXMLRoot = "MerchValueCalculator";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Indicate that this behavior intends to add one tree float data
    //member
    m_iNewTreeFloats = 1;

    m_iMaxLogs = 6;
    m_iDBHIncs = 16;
    m_iFormDBHIncs = 31;

    m_iNumTotalSpecies = 0;

    mp_fVal = NULL;
    mp_fFormClass = NULL;
    mp_fTaperTable = NULL;
    mp_fFormClass78Table = NULL;
    mp_fFormClass79Table = NULL;
    mp_fFormClass80Table = NULL;
    mp_fFormClass81Table = NULL;
    mp_fFormClass84Table = NULL;
    mp_fFormClass85Table = NULL;
    m_cQuery = NULL;
    mp_iMerchValCodes = NULL;
    mp_iValueCodes = NULL;
    mp_oValueGrid = NULL;
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
    stcErr.sFunction = "clMerchValueCalculator::clMerchValueCalculator" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clMerchValueCalculator::~clMerchValueCalculator()
{
  int i;
	delete[] mp_fVal;
  delete[] mp_fFormClass;
  delete[] m_cQuery;
  delete[] mp_iValueCodes;

  if ( mp_iMerchValCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iMerchValCodes[i];
  delete[] mp_iMerchValCodes;

  if ( mp_fTaperTable )
    for (i = 0; i < m_iDBHIncs; i++ )
      delete[] mp_fTaperTable[i];
  delete[] mp_fTaperTable;

  if ( mp_fFormClass78Table )
    for (i = 0; i < m_iFormDBHIncs; i++ )
      delete[] mp_fFormClass78Table[i];
  delete[] mp_fFormClass78Table;

  if ( mp_fFormClass79Table )
    for (i = 0; i < m_iFormDBHIncs; i++ )
      delete[] mp_fFormClass79Table[i];
  delete[] mp_fFormClass79Table;

  if ( mp_fFormClass80Table )
    for (i = 0; i < m_iFormDBHIncs; i++ )
      delete[] mp_fFormClass80Table[i];
  delete[] mp_fFormClass80Table;

  if ( mp_fFormClass81Table )
    for (i = 0; i < m_iFormDBHIncs; i++ )
      delete[] mp_fFormClass81Table[i];
  delete[] mp_fFormClass81Table;

  if ( mp_fFormClass84Table )
    for (i = 0; i < m_iFormDBHIncs; i++ )
      delete[] mp_fFormClass84Table[i];
  delete[] mp_fFormClass84Table;

  if ( mp_fFormClass85Table )
    for (i = 0; i < m_iFormDBHIncs; i++ )
      delete[] mp_fFormClass85Table[i];
  delete[] mp_fFormClass85Table;
}

/////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
/////////////////////////////////////////////////////////////////////////////
void clMerchValueCalculator::GetParameterFileData( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    int i, iFormClass;

    //Set up our doubleVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare our arrays and initialize to zeroes to avoid math problems
    mp_fVal = new double[m_iNumTotalSpecies];
    mp_fFormClass = new double[m_iNumTotalSpecies];
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      mp_fVal[i] = 0;
      mp_fFormClass[i] = 0;
    }

    //Get the parameter file values

    //Price per thousand board feet
    FillSpeciesSpecificValue( p_oElement, "an_merchValuePricePer1KFeet", "an_mvpp1kfVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets, and
    //convert to price per board foot
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fVal[p_fTempValues[i].code] = p_fTempValues[i].val / 1000.0;

    //Form classes
    FillSpeciesSpecificValue( p_oElement, "an_merchValueFormClasses", "an_mvfcVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets, and
    //validate that the form classes are one of the valid values
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      //Truncate down the form class to int - it's OK
      iFormClass = (int) p_fTempValues[i].val;
      if (78 == iFormClass ||
          79 == iFormClass ||
          80 == iFormClass ||
          81 == iFormClass ||
          84 == iFormClass ||
          85 == iFormClass)
          mp_fFormClass[p_fTempValues[i].code] = iFormClass;
      else {
        modelErr stcErr;
        stcErr.sFunction = "clMerchValueCalculator::GetParameterFileData" ;
        std::stringstream s;
        s << "Invalid form class value \"" << mp_fFormClass[i]
          << "\" for species " <<  mp_iWhatSpecies[i]
          << ".  Valid values are: 78, 79, 80, 81, 84, and 85.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Convert form class to proportions
    for ( i = 0; i < m_iNumTotalSpecies; i++ ) {
      mp_fFormClass[i] *= 0.01; //convert to proportion
      mp_fFormClass[i] = clModelMath::Round(1 - mp_fFormClass[i], 2); //invert
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
    stcErr.sFunction = "clMerchValueCalculator::GetParameterFileData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clMerchValueCalculator::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

    GetParameterFileData(p_oDoc, p_oPop);
    FormatQueryString(p_oPop);
    PopulateTables();
    SetupGrid();
    Action();
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
    stcErr.sFunction = "clMerchValueCalculator::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clMerchValueCalculator::SetupGrid()
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  std::stringstream sLabel;
  short int i; //loop counter

  //Create the grid with one float data member for each species
  mp_oValueGrid = mp_oSimManager->CreateGrid( "Merchantable Timber Value",
                        0,                  //number of ints
                        m_iNumTotalSpecies, //number of floats
                        0,                  //number of chars
                        0,                  //number of bools
                        p_oPlot->GetXPlotLength(), //1 cell for the plot
                        p_oPlot->GetYPlotLength() );

  mp_iValueCodes = new short int[m_iNumTotalSpecies];
  //Register the data member - called "value_x"
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    sLabel << "value_" << i;
    mp_iValueCodes[i] = mp_oValueGrid->RegisterFloat(sLabel.str());
sLabel.str("");
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetNumLogs()
/////////////////////////////////////////////////////////////////////////////
int clMerchValueCalculator::GetNumLogs(const float &fDBH, const int &iSpecies)
{
  float fTargetTaper = fDBH * MERCH_RATIO, //diameter at the merchantable height
        fTaperAmt; //keeps track of the amount of taper so far
  int iNumLogs = 1, iDBHIndex, iTemp, i;

  //Get the amount of taper left after subtracting that at the top of the first
  //log; get the amount of taper at the top of the first log from the form class
  fTaperAmt = fTargetTaper - (fDBH * mp_fFormClass[iSpecies]);

  //Get the index for the dbh in the taper table - round down to the nearest
  //multiple of 2
  iTemp = (int) fDBH; //truncate off the decimal portion
  if (iTemp % 2 == 0) {
    iDBHIndex = (iTemp - 10) / 2;
  }
  else {
    iDBHIndex = (iTemp - 11) / 2;
  }

  //If the value is greater than the biggest entry in our table, use that
  //entry instead
  iTemp = m_iDBHIncs - 1;
  if (iDBHIndex > iTemp) {
    iDBHIndex = iTemp;
  }

  //Find the number of logs to add from the taper table
  for (i = 0; i < m_iMaxLogs; i++) {
    if (mp_fTaperTable[iDBHIndex][i] <= fTaperAmt)
      iNumLogs++;
  }

  return iNumLogs;
}

/////////////////////////////////////////////////////////////////////////////
// GetTreeValue()
/////////////////////////////////////////////////////////////////////////////
float clMerchValueCalculator::GetTreeValue(const float &fDBH, const int &iSpecies)
{
  if (fDBH < MIN_DBH) return 0;

  int iNumLogs = GetNumLogs(fDBH, iSpecies) - 1,
      iDBHIndex, iTemp;

  //Get the index for the dbh in the form class value table - round down to the
  //nearest integer
  iDBHIndex = ((int) fDBH) - MIN_DBH;

  //If the value is greater than the biggest entry in our table, use that
  //entry instead
  iTemp = m_iFormDBHIncs - 1;
  if (iDBHIndex > iTemp) {
    iDBHIndex = iTemp;
  }

  //Use the appropriate form class table - remember that we turned the form
  //classes into proportions and flipped them
  //Form class 78
  if (fabs(0.22 - mp_fFormClass[iSpecies]) < 0.001) {
    return mp_fVal[iSpecies] * mp_fFormClass78Table[iDBHIndex][iNumLogs];
  }
  //Form class 79
  else if (fabs(0.21 - mp_fFormClass[iSpecies]) < 0.001) {
    return mp_fVal[iSpecies] * mp_fFormClass79Table[iDBHIndex][iNumLogs];
  }

  //Form class 80
  else if (fabs(0.20 - mp_fFormClass[iSpecies]) < 0.001) {
    return mp_fVal[iSpecies] * mp_fFormClass80Table[iDBHIndex][iNumLogs];
  }

  //Form class 81
  else if (fabs(0.19 - mp_fFormClass[iSpecies]) < 0.001) {
    return mp_fVal[iSpecies] * mp_fFormClass81Table[iDBHIndex][iNumLogs];
  }

  //Form class 84
  else if (fabs(0.16 - mp_fFormClass[iSpecies]) < 0.001) {
    return mp_fVal[iSpecies] * mp_fFormClass84Table[iDBHIndex][iNumLogs];
  }

  //Form class 85
  else if (fabs(0.15 - mp_fFormClass[iSpecies]) < 0.001) {
    return mp_fVal[iSpecies] * mp_fFormClass85Table[iDBHIndex][iNumLogs];
  }
  else {
    //Should never happen - but I've been surprised before
    modelErr stcErr;
    stcErr.sFunction = "clMerchValueCalculator::GetTreeValue" ;
    std::stringstream s;
    s << "Invalid form class value \"" << mp_fFormClass[iSpecies]
      << "\" for species " <<  iSpecies
      << ".  Valid values are: 78, 79, 80, 81, 84, and 85.";
    stcErr.sMoreInfo = s.str();
    stcErr.iErrorCode = BAD_DATA;
    throw( stcErr );
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clMerchValueCalculator::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    float *p_fSpeciesTotals = new float[m_iNumTotalSpecies];
    float fValue, //tree's value
         fDBH; //tree's dbhs
    int iSp, iTp;

    //Initialize our species totals arrays
    for (iSp = 0; iSp < m_iNumTotalSpecies; iSp++) {
      p_fSpeciesTotals[iSp] = 0;
    }

    //Reset the values in the species total grid to 0
    fValue = 0;
    for (iSp = 0; iSp < m_iNumTotalSpecies; iSp++) {
      mp_oValueGrid->SetValueOfCell(0, 0, mp_iValueCodes[iSp], fValue);
    }

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the merch val data member
      if ( -1 != mp_iMerchValCodes[iSp] [iTp - clTreePopulation::sapling] )
      {

        //Get this tree's dbh and convert it to a value in inches
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDBH );
        fDBH *= CONVERT_CM_TO_IN;

        //Get the tree's value
        fValue = GetTreeValue(fDBH, iSp);

        //Set its value
        p_oTree->SetValue( mp_iMerchValCodes[iSp] [iTp - clTreePopulation::sapling], fValue );

        //Add it to the species totals
        p_fSpeciesTotals[iSp] += fValue;
      }

      p_oTree = p_oBehaviorTrees->NextTree();
    }

    //Set the species totals into our grid
    for (iSp = 0; iSp < m_iNumTotalSpecies; iSp++) {
      mp_oValueGrid->SetValueOfCell(0, 0, mp_iValueCodes[iSp], p_fSpeciesTotals[iSp]);
    }

    delete[] p_fSpeciesTotals;
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
    stcErr.sFunction = "clMerchValueCalculator::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clMerchValueCalculator::FormatQueryString(clTreePopulation *p_oPop)
{
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
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clMerchValueCalculator::RegisterTreeDataMembers()
{

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int i, j;

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data member
  mp_iMerchValCodes = new short int * [m_iNumTotalSpecies];
  for ( i = 0; i < m_iNumTotalSpecies; i++ )
  {
    mp_iMerchValCodes[i] = new short int[3];
    for (j = 0; j < 3; j++) {
      mp_iMerchValCodes[i][j] = -1;
    }
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Make sure that only allowed types have been applied
    if ( clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::snag != mp_whatSpeciesTypeCombos[i].iType )
         {

           modelErr stcErr;
           stcErr.sFunction = "clMerchValueCalculator::RegisterTreeDataMembers" ;
           stcErr.sMoreInfo = "This behavior can only be applied to saplings, adults, and snags.";
           stcErr.iErrorCode = BAD_DATA;
           throw( stcErr );
    }

    //Register the code and capture it
    mp_iMerchValCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                     [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
         p_oPop->RegisterFloat( "Merch Val", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}

/////////////////////////////////////////////////////////////////////////////
// PopulateTables()
/////////////////////////////////////////////////////////////////////////////
void clMerchValueCalculator::PopulateTables()
{
  float fMAX = 10000;
  int i;

  //***********************************
  // Taper table
  //***********************************
  //Allocate memory array for the table
  mp_fTaperTable = new double*[m_iDBHIncs];
  for (i = 0; i < m_iDBHIncs; i++) {
    mp_fTaperTable[i] = new double[m_iMaxLogs];
  }

  //Fill in table values
  //10-inch DBH
  mp_fTaperTable[0][0] = 1.4;
  mp_fTaperTable[0][1] = 2.6;
  mp_fTaperTable[0][2] = fMAX;
  mp_fTaperTable[0][3] = fMAX;
  mp_fTaperTable[0][4] = fMAX;
  mp_fTaperTable[0][5] = fMAX;

  //12-inch DBH
  mp_fTaperTable[1][0] = 1.6;
  mp_fTaperTable[1][1] = 2.8;
  mp_fTaperTable[1][2] = 4.4;
  mp_fTaperTable[1][3] = fMAX;
  mp_fTaperTable[1][4] = fMAX;
  mp_fTaperTable[1][5] = fMAX;

  //14-inch DBH
  mp_fTaperTable[2][0] = 1.7;
  mp_fTaperTable[2][1] = 3;
  mp_fTaperTable[2][2] = 4.7;
  mp_fTaperTable[2][3] = fMAX;
  mp_fTaperTable[2][4] = fMAX;
  mp_fTaperTable[2][5] = fMAX;

  //16-inch DBH
  mp_fTaperTable[3][0] = 1.9;
  mp_fTaperTable[3][1] = 3.2;
  mp_fTaperTable[3][2] = 4.9;
  mp_fTaperTable[3][3] = fMAX;
  mp_fTaperTable[3][4] = fMAX;
  mp_fTaperTable[3][5] = fMAX;

  //18-inch DBH
  mp_fTaperTable[4][0] = 2;
  mp_fTaperTable[4][1] = 3.4;
  mp_fTaperTable[4][2] = 5.2;
  mp_fTaperTable[4][3] = fMAX;
  mp_fTaperTable[4][4] = fMAX;
  mp_fTaperTable[4][5] = fMAX;

  //20-inch DBH
  mp_fTaperTable[5][0] = 2.1;
  mp_fTaperTable[5][1] = 3.6;
  mp_fTaperTable[5][2] = 5.6;
  mp_fTaperTable[5][3] = 7.8;
  mp_fTaperTable[5][4] = fMAX;
  mp_fTaperTable[5][5] = fMAX;

  //22-inch DBH
  mp_fTaperTable[6][0] = 2.2;
  mp_fTaperTable[6][1] = 3.8;
  mp_fTaperTable[6][2] = 5.9;
  mp_fTaperTable[6][3] = 8;
  mp_fTaperTable[6][4] = fMAX;
  mp_fTaperTable[6][5] = fMAX;

  //24-inch DBH
  mp_fTaperTable[7][0] = 2.3;
  mp_fTaperTable[7][1] = 4;
  mp_fTaperTable[7][2] = 6.3;
  mp_fTaperTable[7][3] = 8.4;
  mp_fTaperTable[7][4] = fMAX;
  mp_fTaperTable[7][5] = fMAX;

  //26-inch DBH
  mp_fTaperTable[8][0] = 2.4;
  mp_fTaperTable[8][1] = 4.2;
  mp_fTaperTable[8][2] = 6.5;
  mp_fTaperTable[8][3] = 8.7;
  mp_fTaperTable[8][4] = fMAX;
  mp_fTaperTable[8][5] = fMAX;

  //28-inch DBH
  mp_fTaperTable[9][0] = 2.5;
  mp_fTaperTable[9][1] = 4.4;
  mp_fTaperTable[9][2] = 6.8;
  mp_fTaperTable[9][3] = 9;
  mp_fTaperTable[9][4] = 12;
  mp_fTaperTable[9][5] = fMAX;

  //30-inch DBH
  mp_fTaperTable[10][0] = 2.6;
  mp_fTaperTable[10][1] = 4.6;
  mp_fTaperTable[10][2] = 7.2;
  mp_fTaperTable[10][3] = 9.4;
  mp_fTaperTable[10][4] = 12.1;
  mp_fTaperTable[10][5] = fMAX;

  //32-inch DBH
  mp_fTaperTable[11][0] = 2.7;
  mp_fTaperTable[11][1] = 4.7;
  mp_fTaperTable[11][2] = 7.3;
  mp_fTaperTable[11][3] = 9.9;
  mp_fTaperTable[11][4] = 12.3;
  mp_fTaperTable[11][5] = fMAX;

  //34-inch DBH
  mp_fTaperTable[12][0] = 2.8;
  mp_fTaperTable[12][1] = 4.8;
  mp_fTaperTable[12][2] = 7.6;
  mp_fTaperTable[12][3] = 10.2;
  mp_fTaperTable[12][4] = 12.6;
  mp_fTaperTable[12][5] = fMAX;

  //36-inch DBH
  mp_fTaperTable[13][0] = 2.8;
  mp_fTaperTable[13][1] = 4.9;
  mp_fTaperTable[13][2] = 7.8;
  mp_fTaperTable[13][3] = 10.4;
  mp_fTaperTable[13][4] = 13;
  mp_fTaperTable[13][5] = fMAX;

  //38-inch DBH
  mp_fTaperTable[14][0] = 2.9;
  mp_fTaperTable[14][1] = 4.9;
  mp_fTaperTable[14][2] = 7.9;
  mp_fTaperTable[14][3] = 10.5;
  mp_fTaperTable[14][4] = 13.4;
  mp_fTaperTable[14][5] = fMAX;

  //40-inch DBH
  mp_fTaperTable[15][0] = 2.9;
  mp_fTaperTable[15][1] = 5;
  mp_fTaperTable[15][2] = 8;
  mp_fTaperTable[15][3] = 10.9;
  mp_fTaperTable[15][4] = 13.9;
  mp_fTaperTable[15][5] = fMAX;

  //***********************************
  // Form class 78 table
  //***********************************
  //Allocate memory array for the table
  mp_fFormClass78Table = new float*[m_iFormDBHIncs];
  for (i = 0; i < m_iFormDBHIncs; i++) {
    mp_fFormClass78Table[i] = new float[m_iMaxLogs];
  }

  //10-inch DBH
  mp_fFormClass78Table[0][0] = 36;
  mp_fFormClass78Table[0][1] = 59;
  mp_fFormClass78Table[0][2] = 73;
  mp_fFormClass78Table[0][3] = fMAX;
  mp_fFormClass78Table[0][4] = fMAX;
  mp_fFormClass78Table[0][5] = fMAX;

  //11-inch DBH
  mp_fFormClass78Table[1][0] = 46;
  mp_fFormClass78Table[1][1] = 76;
  mp_fFormClass78Table[1][2] = 96;
  mp_fFormClass78Table[1][3] = fMAX;
  mp_fFormClass78Table[1][4] = fMAX;
  mp_fFormClass78Table[1][5] = fMAX;

  //12-inch DBH
  mp_fFormClass78Table[2][0] = 56;
  mp_fFormClass78Table[2][1] = 92;
  mp_fFormClass78Table[2][2] = 120;
  mp_fFormClass78Table[2][3] = 137;
  mp_fFormClass78Table[2][4] = fMAX;
  mp_fFormClass78Table[2][5] = fMAX;

  //13-inch DBH
  mp_fFormClass78Table[3][0] = 67;
  mp_fFormClass78Table[3][1] = 112;
  mp_fFormClass78Table[3][2] = 147;
  mp_fFormClass78Table[3][3] = 168;
  mp_fFormClass78Table[3][4] = fMAX;
  mp_fFormClass78Table[3][5] = fMAX;

  //14-inch DBH
  mp_fFormClass78Table[4][0] = 78;
  mp_fFormClass78Table[4][1] = 132;
  mp_fFormClass78Table[4][2] = 174;
  mp_fFormClass78Table[4][3] = 200;
  mp_fFormClass78Table[4][4] = fMAX;
  mp_fFormClass78Table[4][5] = fMAX;

  //15-inch DBH
  mp_fFormClass78Table[5][0] = 92;
  mp_fFormClass78Table[5][1] = 156;
  mp_fFormClass78Table[5][2] = 208;
  mp_fFormClass78Table[5][3] = 242;
  mp_fFormClass78Table[5][4] = fMAX;
  mp_fFormClass78Table[5][5] = fMAX;

  //16-inch DBH
  mp_fFormClass78Table[6][0] = 106;
  mp_fFormClass78Table[6][1] = 180;
  mp_fFormClass78Table[6][2] = 241;
  mp_fFormClass78Table[6][3] = 285;
  mp_fFormClass78Table[6][4] = fMAX;
  mp_fFormClass78Table[6][5] = fMAX;

  //17-inch DBH
  mp_fFormClass78Table[7][0] = 121;
  mp_fFormClass78Table[7][1] = 206;
  mp_fFormClass78Table[7][2] = 278;
  mp_fFormClass78Table[7][3] = 330;
  mp_fFormClass78Table[7][4] = fMAX;
  mp_fFormClass78Table[7][5] = fMAX;

  //18-inch DBH
  mp_fFormClass78Table[8][0] = 136;
  mp_fFormClass78Table[8][1] = 233;
  mp_fFormClass78Table[8][2] = 314;
  mp_fFormClass78Table[8][3] = 374;
  mp_fFormClass78Table[8][4] = fMAX;
  mp_fFormClass78Table[8][5] = fMAX;

  //19-inch DBH
  mp_fFormClass78Table[9][0] = 154;
  mp_fFormClass78Table[9][1] = 264;
  mp_fFormClass78Table[9][2] = 358;
  mp_fFormClass78Table[9][3] = 427;
  mp_fFormClass78Table[9][4] = fMAX;
  mp_fFormClass78Table[9][5] = fMAX;

  //20-inch DBH
  mp_fFormClass78Table[10][0] = 171;
  mp_fFormClass78Table[10][1] = 296;
  mp_fFormClass78Table[10][2] = 401;
  mp_fFormClass78Table[10][3] = 480;
  mp_fFormClass78Table[10][4] = 542;
  mp_fFormClass78Table[10][5] = fMAX;

  //21-inch DBH
  mp_fFormClass78Table[11][0] = 191;
  mp_fFormClass78Table[11][1] = 332;
  mp_fFormClass78Table[11][2] = 450;
  mp_fFormClass78Table[11][3] = 542;
  mp_fFormClass78Table[11][4] = 616;
  mp_fFormClass78Table[11][5] = fMAX;

  //22-inch DBH
  mp_fFormClass78Table[12][0] = 211;
  mp_fFormClass78Table[12][1] = 368;
  mp_fFormClass78Table[12][2] = 500;
  mp_fFormClass78Table[12][3] = 603;
  mp_fFormClass78Table[12][4] = 691;
  mp_fFormClass78Table[12][5] = fMAX;

  //23-inch DBH
  mp_fFormClass78Table[13][0] = 231;
  mp_fFormClass78Table[13][1] = 404;
  mp_fFormClass78Table[13][2] = 552;
  mp_fFormClass78Table[13][3] = 663;
  mp_fFormClass78Table[13][4] = 714;
  mp_fFormClass78Table[13][5] = fMAX;

  //24-inch DBH
  mp_fFormClass78Table[14][0] = 251;
  mp_fFormClass78Table[14][1] = 441;
  mp_fFormClass78Table[14][2] = 605;
  mp_fFormClass78Table[14][3] = 723;
  mp_fFormClass78Table[14][4] = 782;
  mp_fFormClass78Table[14][5] = fMAX;

  //25-inch DBH
  mp_fFormClass78Table[15][0] = 275;
  mp_fFormClass78Table[15][1] = 484;
  mp_fFormClass78Table[15][2] = 665;
  mp_fFormClass78Table[15][3] = 800;
  mp_fFormClass78Table[15][4] = 865;
  mp_fFormClass78Table[15][5] = fMAX;

  //26-inch DBH
  mp_fFormClass78Table[16][0] = 299;
  mp_fFormClass78Table[16][1] = 528;
  mp_fFormClass78Table[16][2] = 725;
  mp_fFormClass78Table[16][3] = 877;
  mp_fFormClass78Table[16][4] = 1021;
  mp_fFormClass78Table[16][5] = fMAX;

  //27-inch DBH
  mp_fFormClass78Table[17][0] = 323;
  mp_fFormClass78Table[17][1] = 572;
  mp_fFormClass78Table[17][2] = 788;
  mp_fFormClass78Table[17][3] = 952;
  mp_fFormClass78Table[17][4] = 1111;
  mp_fFormClass78Table[17][5] = fMAX;

  //28-inch DBH
  mp_fFormClass78Table[18][0] = 347;
  mp_fFormClass78Table[18][1] = 616;
  mp_fFormClass78Table[18][2] = 850;
  mp_fFormClass78Table[18][3] = 1027;
  mp_fFormClass78Table[18][4] = 1201;
  mp_fFormClass78Table[18][5] = 1358;

  //29-inch DBH
  mp_fFormClass78Table[19][0] = 375;
  mp_fFormClass78Table[19][1] = 667;
  mp_fFormClass78Table[19][2] = 920;
  mp_fFormClass78Table[19][3] = 1112;
  mp_fFormClass78Table[19][4] = 1308;
  mp_fFormClass78Table[19][5] = 1488;

  //30-inch DBH
  mp_fFormClass78Table[20][0] = 403;
  mp_fFormClass78Table[20][1] = 718;
  mp_fFormClass78Table[20][2] = 991;
  mp_fFormClass78Table[20][3] = 1198;
  mp_fFormClass78Table[20][4] = 1415;
  mp_fFormClass78Table[20][5] = 1619;

  //31-inch DBH
  mp_fFormClass78Table[21][0] = 432;
  mp_fFormClass78Table[21][1] = 772;
  mp_fFormClass78Table[21][2] = 1070;
  mp_fFormClass78Table[21][3] = 1299;
  mp_fFormClass78Table[21][4] = 1526;
  mp_fFormClass78Table[21][5] = 1754;

  //32-inch DBH
  mp_fFormClass78Table[22][0] = 462;
  mp_fFormClass78Table[22][1] = 826;
  mp_fFormClass78Table[22][2] = 1149;
  mp_fFormClass78Table[22][3] = 1400;
  mp_fFormClass78Table[22][4] = 1637;
  mp_fFormClass78Table[22][5] = 1888;

  //33-inch DBH
  mp_fFormClass78Table[23][0] = 492;
  mp_fFormClass78Table[23][1] = 880;
  mp_fFormClass78Table[23][2] = 1226;
  mp_fFormClass78Table[23][3] = 1495;
  mp_fFormClass78Table[23][4] = 1750;
  mp_fFormClass78Table[23][5] = 2026;

  //34-inch DBH
  mp_fFormClass78Table[24][0] = 521;
  mp_fFormClass78Table[24][1] = 934;
  mp_fFormClass78Table[24][2] = 1304;
  mp_fFormClass78Table[24][3] = 1590;
  mp_fFormClass78Table[24][4] = 1864;
  mp_fFormClass78Table[24][5] = 2163;

  //35-inch DBH
  mp_fFormClass78Table[25][0] = 555;
  mp_fFormClass78Table[25][1] = 998;
  mp_fFormClass78Table[25][2] = 1394;
  mp_fFormClass78Table[25][3] = 1702;
  mp_fFormClass78Table[25][4] = 2000;
  mp_fFormClass78Table[25][5] = 2312;

  //36-inch DBH
  mp_fFormClass78Table[26][0] = 589;
  mp_fFormClass78Table[26][1] = 1063;
  mp_fFormClass78Table[26][2] = 1485;
  mp_fFormClass78Table[26][3] = 1814;
  mp_fFormClass78Table[26][4] = 2135;
  mp_fFormClass78Table[26][5] = 2461;

  //37-inch DBH
  mp_fFormClass78Table[27][0] = 622;
  mp_fFormClass78Table[27][1] = 1124;
  mp_fFormClass78Table[27][2] = 1578;
  mp_fFormClass78Table[27][3] = 1926;
  mp_fFormClass78Table[27][4] = 2272;
  mp_fFormClass78Table[27][5] = 2616;

  //38-inch DBH
  mp_fFormClass78Table[28][0] = 656;
  mp_fFormClass78Table[28][1] = 1186;
  mp_fFormClass78Table[28][2] = 1670;
  mp_fFormClass78Table[28][3] = 2038;
  mp_fFormClass78Table[28][4] = 2410;
  mp_fFormClass78Table[28][5] = 2771;

  //39-inch DBH
  mp_fFormClass78Table[29][0] = 694;
  mp_fFormClass78Table[29][1] = 1258;
  mp_fFormClass78Table[29][2] = 1769;
  mp_fFormClass78Table[29][3] = 2166;
  mp_fFormClass78Table[29][4] = 2552;
  mp_fFormClass78Table[29][5] = 2937;

  //40-inch DBH
  mp_fFormClass78Table[30][0] = 731;
  mp_fFormClass78Table[30][1] = 1329;
  mp_fFormClass78Table[30][2] = 1868;
  mp_fFormClass78Table[30][3] = 2294;
  mp_fFormClass78Table[30][4] = 2693;
  mp_fFormClass78Table[30][5] = 3103;

  //***********************************
  // Form class 79 table
  //***********************************
  //Allocate memory array for the table
  mp_fFormClass79Table = new float*[m_iFormDBHIncs];
  for (i = 0; i < m_iFormDBHIncs; i++) {
    mp_fFormClass79Table[i] = new float[m_iMaxLogs];
  }

  //10-inch DBH
  mp_fFormClass79Table[0][0] = 38;
  mp_fFormClass79Table[0][1] = 61;
  mp_fFormClass79Table[0][2] = 77;
  mp_fFormClass79Table[0][3] = fMAX;
  mp_fFormClass79Table[0][4] = fMAX;
  mp_fFormClass79Table[0][5] = fMAX;

  //11-inch DBH
  mp_fFormClass79Table[1][0] = 48;
  mp_fFormClass79Table[1][1] = 78;
  mp_fFormClass79Table[1][2] = 100;
  mp_fFormClass79Table[1][3] = fMAX;
  mp_fFormClass79Table[1][4] = fMAX;
  mp_fFormClass79Table[1][5] = fMAX;

  //12-inch DBH
  mp_fFormClass79Table[2][0] = 58;
  mp_fFormClass79Table[2][1] = 96;
  mp_fFormClass79Table[2][2] = 124;
  mp_fFormClass79Table[2][3] = 141;
  mp_fFormClass79Table[2][4] = fMAX;
  mp_fFormClass79Table[2][5] = fMAX;

  //13-inch DBH
  mp_fFormClass79Table[3][0] = 70;
  mp_fFormClass79Table[3][1] = 117;
  mp_fFormClass79Table[3][2] = 153;
  mp_fFormClass79Table[3][3] = 176;
  mp_fFormClass79Table[3][4] = fMAX;
  mp_fFormClass79Table[3][5] = fMAX;

  //14-inch DBH
  mp_fFormClass79Table[4][0] = 82;
  mp_fFormClass79Table[4][1] = 138;
  mp_fFormClass79Table[4][2] = 182;
  mp_fFormClass79Table[4][3] = 211;
  mp_fFormClass79Table[4][4] = fMAX;
  mp_fFormClass79Table[4][5] = fMAX;

  //15-inch DBH
  mp_fFormClass79Table[5][0] = 95;
  mp_fFormClass79Table[5][1] = 160;
  mp_fFormClass79Table[5][2] = 214;
  mp_fFormClass79Table[5][3] = 252;
  mp_fFormClass79Table[5][4] = fMAX;
  mp_fFormClass79Table[5][5] = fMAX;

  //16-inch DBH
  mp_fFormClass79Table[6][0] = 108;
  mp_fFormClass79Table[6][1] = 183;
  mp_fFormClass79Table[6][2] = 246;
  mp_fFormClass79Table[6][3] = 292;
  mp_fFormClass79Table[6][4] = fMAX;
  mp_fFormClass79Table[6][5] = fMAX;

  //17-inch DBH
  mp_fFormClass79Table[7][0] = 124;
  mp_fFormClass79Table[7][1] = 212;
  mp_fFormClass79Table[7][2] = 286;
  mp_fFormClass79Table[7][3] = 340;
  mp_fFormClass79Table[7][4] = fMAX;
  mp_fFormClass79Table[7][5] = fMAX;

  //18-inch DBH
  mp_fFormClass79Table[8][0] = 140;
  mp_fFormClass79Table[8][1] = 240;
  mp_fFormClass79Table[8][2] = 325;
  mp_fFormClass79Table[8][3] = 388;
  mp_fFormClass79Table[8][4] = fMAX;
  mp_fFormClass79Table[8][5] = fMAX;

  //19-inch DBH
  mp_fFormClass79Table[9][0] = 158;
  mp_fFormClass79Table[9][1] = 272;
  mp_fFormClass79Table[9][2] = 370;
  mp_fFormClass79Table[9][3] = 442;
  mp_fFormClass79Table[9][4] = fMAX;
  mp_fFormClass79Table[9][5] = fMAX;

  //20-inch DBH
  mp_fFormClass79Table[10][0] = 176;
  mp_fFormClass79Table[10][1] = 305;
  mp_fFormClass79Table[10][2] = 414;
  mp_fFormClass79Table[10][3] = 496;
  mp_fFormClass79Table[10][4] = 561;
  mp_fFormClass79Table[10][5] = fMAX;

  //21-inch DBH
  mp_fFormClass79Table[11][0] = 196;
  mp_fFormClass79Table[11][1] = 342;
  mp_fFormClass79Table[11][2] = 464;
  mp_fFormClass79Table[11][3] = 558;
  mp_fFormClass79Table[11][4] = 636;
  mp_fFormClass79Table[11][5] = fMAX;

  //22-inch DBH
  mp_fFormClass79Table[12][0] = 216;
  mp_fFormClass79Table[12][1] = 378;
  mp_fFormClass79Table[12][2] = 514;
  mp_fFormClass79Table[12][3] = 621;
  mp_fFormClass79Table[12][4] = 710;
  mp_fFormClass79Table[12][5] = fMAX;

  //23-inch DBH
  mp_fFormClass79Table[13][0] = 238;
  mp_fFormClass79Table[13][1] = 418;
  mp_fFormClass79Table[13][2] = 571;
  mp_fFormClass79Table[13][3] = 687;
  mp_fFormClass79Table[13][4] = 792;
  mp_fFormClass79Table[13][5] = fMAX;

  //24-inch DBH
  mp_fFormClass79Table[14][0] = 260;
  mp_fFormClass79Table[14][1] = 458;
  mp_fFormClass79Table[14][2] = 628;
  mp_fFormClass79Table[14][3] = 753;
  mp_fFormClass79Table[14][4] = 875;
  mp_fFormClass79Table[14][5] = fMAX;

  //25-inch DBH
  mp_fFormClass79Table[15][0] = 282;
  mp_fFormClass79Table[15][1] = 499;
  mp_fFormClass79Table[15][2] = 685;
  mp_fFormClass79Table[15][3] = 826;
  mp_fFormClass79Table[15][4] = 960;
  mp_fFormClass79Table[15][5] = fMAX;

  //26-inch DBH
  mp_fFormClass79Table[16][0] = 305;
  mp_fFormClass79Table[16][1] = 540;
  mp_fFormClass79Table[16][2] = 742;
  mp_fFormClass79Table[16][3] = 899;
  mp_fFormClass79Table[16][4] = 1046;
  mp_fFormClass79Table[16][5] = fMAX;

  //27-inch DBH
  mp_fFormClass79Table[17][0] = 331;
  mp_fFormClass79Table[17][1] = 588;
  mp_fFormClass79Table[17][2] = 810;
  mp_fFormClass79Table[17][3] = 980;
  mp_fFormClass79Table[17][4] = 1144;
  mp_fFormClass79Table[17][5] = fMAX;

  //28-inch DBH
  mp_fFormClass79Table[18][0] = 357;
  mp_fFormClass79Table[18][1] = 635;
  mp_fFormClass79Table[18][2] = 877;
  mp_fFormClass79Table[18][3] = 1061;
  mp_fFormClass79Table[18][4] = 1242;
  mp_fFormClass79Table[18][5] = fMAX;

  //29-inch DBH
  mp_fFormClass79Table[19][0] = 385;
  mp_fFormClass79Table[19][1] = 686;
  mp_fFormClass79Table[19][2] = 948;
  mp_fFormClass79Table[19][3] = 1148;
  mp_fFormClass79Table[19][4] = 1350;
  mp_fFormClass79Table[19][5] = 1537;

  //30-inch DBH
  mp_fFormClass79Table[20][0] = 413;
  mp_fFormClass79Table[20][1] = 737;
  mp_fFormClass79Table[20][2] = 1020;
  mp_fFormClass79Table[20][3] = 1235;
  mp_fFormClass79Table[20][4] = 1458;
  mp_fFormClass79Table[20][5] = 1670;

  //31-inch DBH
  mp_fFormClass79Table[21][0] = 444;
  mp_fFormClass79Table[21][1] = 792;
  mp_fFormClass79Table[21][2] = 1100;
  mp_fFormClass79Table[21][3] = 1338;
  mp_fFormClass79Table[21][4] = 1572;
  mp_fFormClass79Table[21][5] = 1808;

  //32-inch DBH
  mp_fFormClass79Table[22][0] = 474;
  mp_fFormClass79Table[22][1] = 848;
  mp_fFormClass79Table[22][2] = 1181;
  mp_fFormClass79Table[22][3] = 1440;
  mp_fFormClass79Table[22][4] = 1685;
  mp_fFormClass79Table[22][5] = 1945;

  //33-inch DBH
  mp_fFormClass79Table[23][0] = 506;
  mp_fFormClass79Table[23][1] = 907;
  mp_fFormClass79Table[23][2] = 1265;
  mp_fFormClass79Table[23][3] = 1544;
  mp_fFormClass79Table[23][4] = 1808;
  mp_fFormClass79Table[23][5] = 2094;

  //34-inch DBH
  mp_fFormClass79Table[24][0] = 538;
  mp_fFormClass79Table[24][1] = 966;
  mp_fFormClass79Table[24][2] = 1349;
  mp_fFormClass79Table[24][3] = 1647;
  mp_fFormClass79Table[24][4] = 1932;
  mp_fFormClass79Table[24][5] = 2244;

  //35-inch DBH
  mp_fFormClass79Table[25][0] = 570;
  mp_fFormClass79Table[25][1] = 1026;
  mp_fFormClass79Table[25][2] = 1435;
  mp_fFormClass79Table[25][3] = 1754;
  mp_fFormClass79Table[25][4] = 2000;
  mp_fFormClass79Table[25][5] = 2384;

  //36-inch DBH
  mp_fFormClass79Table[26][0] = 602;
  mp_fFormClass79Table[26][1] = 1087;
  mp_fFormClass79Table[26][2] = 1521;
  mp_fFormClass79Table[26][3] = 1860;
  mp_fFormClass79Table[26][4] = 2189;
  mp_fFormClass79Table[26][5] = 2525;

  //37-inch DBH
  mp_fFormClass79Table[27][0] = 638;
  mp_fFormClass79Table[27][1] = 1154;
  mp_fFormClass79Table[27][2] = 1620;
  mp_fFormClass79Table[27][3] = 1980;
  mp_fFormClass79Table[27][4] = 2338;
  mp_fFormClass79Table[27][5] = 2694;

  //38-inch DBH
  mp_fFormClass79Table[28][0] = 674;
  mp_fFormClass79Table[28][1] = 1220;
  mp_fFormClass79Table[28][2] = 1720;
  mp_fFormClass79Table[28][3] = 2101;
  mp_fFormClass79Table[28][4] = 2488;
  mp_fFormClass79Table[28][5] = 2862;

  //39-inch DBH
  mp_fFormClass79Table[29][0] = 712;
  mp_fFormClass79Table[29][1] = 1292;
  mp_fFormClass79Table[29][2] = 1822;
  mp_fFormClass79Table[29][3] = 2232;
  mp_fFormClass79Table[29][4] = 2632;
  mp_fFormClass79Table[29][5] = 3031;

  //40-inch DBH
  mp_fFormClass79Table[30][0] = 750;
  mp_fFormClass79Table[30][1] = 1365;
  mp_fFormClass79Table[30][2] = 1923;
  mp_fFormClass79Table[30][3] = 2362;
  mp_fFormClass79Table[30][4] = 2775;
  mp_fFormClass79Table[30][5] = 3200;

  //***********************************
  // Form class 80 table
  //***********************************
  //Allocate memory array for the table
  mp_fFormClass80Table = new float*[m_iFormDBHIncs];
  for (i = 0; i < m_iFormDBHIncs; i++) {
    mp_fFormClass80Table[i] = new float[m_iMaxLogs];
  }

  //10-inch DBH
  mp_fFormClass80Table[0][0] = 39;
  mp_fFormClass80Table[0][1] = 63;
  mp_fFormClass80Table[0][2] = 80;
  mp_fFormClass80Table[0][3] = fMAX;
  mp_fFormClass80Table[0][4] = fMAX;
  mp_fFormClass80Table[0][5] = fMAX;

  //11-inch DBH
  mp_fFormClass80Table[1][0] = 49;
  mp_fFormClass80Table[1][1] = 80;
  mp_fFormClass80Table[1][2] = 104;
  mp_fFormClass80Table[1][3] = fMAX;
  mp_fFormClass80Table[1][4] = fMAX;
  mp_fFormClass80Table[1][5] = fMAX;

  //12-inch DBH
  mp_fFormClass80Table[2][0] = 59;
  mp_fFormClass80Table[2][1] = 98;
  mp_fFormClass80Table[2][2] = 127;
  mp_fFormClass80Table[2][3] = 146;
  mp_fFormClass80Table[2][4] = fMAX;
  mp_fFormClass80Table[2][5] = fMAX;

  //13-inch DBH
  mp_fFormClass80Table[3][0] = 71;
  mp_fFormClass80Table[3][1] = 120;
  mp_fFormClass80Table[3][2] = 156;
  mp_fFormClass80Table[3][3] = 181;
  mp_fFormClass80Table[3][4] = fMAX;
  mp_fFormClass80Table[3][5] = fMAX;

  //14-inch DBH
  mp_fFormClass80Table[4][0] = 83;
  mp_fFormClass80Table[4][1] = 141;
  mp_fFormClass80Table[4][2] = 186;
  mp_fFormClass80Table[4][3] = 216;
  mp_fFormClass80Table[4][4] = fMAX;
  mp_fFormClass80Table[4][5] = fMAX;

  //15-inch DBH
  mp_fFormClass80Table[5][0] = 98;
  mp_fFormClass80Table[5][1] = 166;
  mp_fFormClass80Table[5][2] = 221;
  mp_fFormClass80Table[5][3] = 260;
  mp_fFormClass80Table[5][4] = fMAX;
  mp_fFormClass80Table[5][5] = fMAX;

  //16-inch DBH
  mp_fFormClass80Table[6][0] = 112;
  mp_fFormClass80Table[6][1] = 190;
  mp_fFormClass80Table[6][2] = 256;
  mp_fFormClass80Table[6][3] = 305;
  mp_fFormClass80Table[6][4] = fMAX;
  mp_fFormClass80Table[6][5] = fMAX;

  //17-inch DBH
  mp_fFormClass80Table[7][0] = 128;
  mp_fFormClass80Table[7][1] = 219;
  mp_fFormClass80Table[7][2] = 296;
  mp_fFormClass80Table[7][3] = 354;
  mp_fFormClass80Table[7][4] = fMAX;
  mp_fFormClass80Table[7][5] = fMAX;

  //18-inch DBH
  mp_fFormClass80Table[8][0] = 144;
  mp_fFormClass80Table[8][1] = 248;
  mp_fFormClass80Table[8][2] = 336;
  mp_fFormClass80Table[8][3] = 402;
  mp_fFormClass80Table[8][4] = fMAX;
  mp_fFormClass80Table[8][5] = fMAX;

  //19-inch DBH
  mp_fFormClass80Table[9][0] = 162;
  mp_fFormClass80Table[9][1] = 281;
  mp_fFormClass80Table[9][2] = 382;
  mp_fFormClass80Table[9][3] = 457;
  mp_fFormClass80Table[9][4] = fMAX;
  mp_fFormClass80Table[9][5] = fMAX;

  //20-inch DBH
  mp_fFormClass80Table[10][0] = 181;
  mp_fFormClass80Table[10][1] = 314;
  mp_fFormClass80Table[10][2] = 427;
  mp_fFormClass80Table[10][3] = 512;
  mp_fFormClass80Table[10][4] = 580;
  mp_fFormClass80Table[10][5] = fMAX;

  //21-inch DBH
  mp_fFormClass80Table[11][0] = 201;
  mp_fFormClass80Table[11][1] = 350;
  mp_fFormClass80Table[11][2] = 478;
  mp_fFormClass80Table[11][3] = 575;
  mp_fFormClass80Table[11][4] = 656;
  mp_fFormClass80Table[11][5] = fMAX;

  //22-inch DBH
  mp_fFormClass80Table[12][0] = 221;
  mp_fFormClass80Table[12][1] = 387;
  mp_fFormClass80Table[12][2] = 528;
  mp_fFormClass80Table[12][3] = 638;
  mp_fFormClass80Table[12][4] = 732;
  mp_fFormClass80Table[12][5] = fMAX;

  //23-inch DBH
  mp_fFormClass80Table[13][0] = 244;
  mp_fFormClass80Table[13][1] = 428;
  mp_fFormClass80Table[13][2] = 586;
  mp_fFormClass80Table[13][3] = 706;
  mp_fFormClass80Table[13][4] = 816;
  mp_fFormClass80Table[13][5] = fMAX;

  //24-inch DBH
  mp_fFormClass80Table[14][0] = 266;
  mp_fFormClass80Table[14][1] = 469;
  mp_fFormClass80Table[14][2] = 644;
  mp_fFormClass80Table[14][3] = 773;
  mp_fFormClass80Table[14][4] = 899;
  mp_fFormClass80Table[14][5] = fMAX;

  //25-inch DBH
  mp_fFormClass80Table[15][0] = 290;
  mp_fFormClass80Table[15][1] = 514;
  mp_fFormClass80Table[15][2] = 706;
  mp_fFormClass80Table[15][3] = 852;
  mp_fFormClass80Table[15][4] = 992;
  mp_fFormClass80Table[15][5] = fMAX;

  //26-inch DBH
  mp_fFormClass80Table[16][0] = 315;
  mp_fFormClass80Table[16][1] = 558;
  mp_fFormClass80Table[16][2] = 767;
  mp_fFormClass80Table[16][3] = 931;
  mp_fFormClass80Table[16][4] = 1086;
  mp_fFormClass80Table[16][5] = fMAX;

  //27-inch DBH
  mp_fFormClass80Table[17][0] = 341;
  mp_fFormClass80Table[17][1] = 606;
  mp_fFormClass80Table[17][2] = 836;
  mp_fFormClass80Table[17][3] = 1014;
  mp_fFormClass80Table[17][4] = 1185;
  mp_fFormClass80Table[17][5] = fMAX;

  //28-inch DBH
  mp_fFormClass80Table[18][0] = 367;
  mp_fFormClass80Table[18][1] = 654;
  mp_fFormClass80Table[18][2] = 904;
  mp_fFormClass80Table[18][3] = 1096;
  mp_fFormClass80Table[18][4] = 1284;
  mp_fFormClass80Table[18][5] = 1453;

  //29-inch DBH
  mp_fFormClass80Table[19][0] = 396;
  mp_fFormClass80Table[19][1] = 706;
  mp_fFormClass80Table[19][2] = 977;
  mp_fFormClass80Table[19][3] = 1184;
  mp_fFormClass80Table[19][4] = 1394;
  mp_fFormClass80Table[19][5] = 1588;

  //30-inch DBH
  mp_fFormClass80Table[20][0] = 424;
  mp_fFormClass80Table[20][1] = 758;
  mp_fFormClass80Table[20][2] = 1050;
  mp_fFormClass80Table[20][3] = 1272;
  mp_fFormClass80Table[20][4] = 1503;
  mp_fFormClass80Table[20][5] = 1723;

  //31-inch DBH
  mp_fFormClass80Table[21][0] = 454;
  mp_fFormClass80Table[21][1] = 814;
  mp_fFormClass80Table[21][2] = 1132;
  mp_fFormClass80Table[21][3] = 1376;
  mp_fFormClass80Table[21][4] = 1618;
  mp_fFormClass80Table[21][5] = 1862;

  //32-inch DBH
  mp_fFormClass80Table[22][0] = 485;
  mp_fFormClass80Table[22][1] = 870;
  mp_fFormClass80Table[22][2] = 1213;
  mp_fFormClass80Table[22][3] = 1480;
  mp_fFormClass80Table[22][4] = 1733;
  mp_fFormClass80Table[22][5] = 2001;

  //33-inch DBH
  mp_fFormClass80Table[23][0] = 518;
  mp_fFormClass80Table[23][1] = 930;
  mp_fFormClass80Table[23][2] = 1298;
  mp_fFormClass80Table[23][3] = 1586;
  mp_fFormClass80Table[23][4] = 1858;
  mp_fFormClass80Table[23][5] = 2152;

  //34-inch DBH
  mp_fFormClass80Table[24][0] = 550;
  mp_fFormClass80Table[24][1] = 989;
  mp_fFormClass80Table[24][2] = 1383;
  mp_fFormClass80Table[24][3] = 1691;
  mp_fFormClass80Table[24][4] = 1984;
  mp_fFormClass80Table[24][5] = 2304;

  //35-inch DBH
  mp_fFormClass80Table[25][0] = 585;
  mp_fFormClass80Table[25][1] = 1055;
  mp_fFormClass80Table[25][2] = 1477;
  mp_fFormClass80Table[25][3] = 1806;
  mp_fFormClass80Table[25][4] = 2124;
  mp_fFormClass80Table[25][5] = 2458;

  //36-inch DBH
  mp_fFormClass80Table[26][0] = 620;
  mp_fFormClass80Table[26][1] = 1121;
  mp_fFormClass80Table[26][2] = 1571;
  mp_fFormClass80Table[26][3] = 1922;
  mp_fFormClass80Table[26][4] = 2264;
  mp_fFormClass80Table[26][5] = 2612;

  //37-inch DBH
  mp_fFormClass80Table[27][0] = 656;
  mp_fFormClass80Table[27][1] = 1188;
  mp_fFormClass80Table[27][2] = 1672;
  mp_fFormClass80Table[27][3] = 2044;
  mp_fFormClass80Table[27][4] = 2416;
  mp_fFormClass80Table[27][5] = 2783;

  //38-inch DBH
  mp_fFormClass80Table[28][0] = 693;
  mp_fFormClass80Table[28][1] = 1256;
  mp_fFormClass80Table[28][2] = 1772;
  mp_fFormClass80Table[28][3] = 2167;
  mp_fFormClass80Table[28][4] = 2568;
  mp_fFormClass80Table[28][5] = 2954;

  //39-inch DBH
  mp_fFormClass80Table[29][0] = 732;
  mp_fFormClass80Table[29][1] = 1330;
  mp_fFormClass80Table[29][2] = 1874;
  mp_fFormClass80Table[29][3] = 2300;
  mp_fFormClass80Table[29][4] = 2714;
  mp_fFormClass80Table[29][5] = 3127;

  //40-inch DBH
  mp_fFormClass80Table[30][0] = 770;
  mp_fFormClass80Table[30][1] = 1403;
  mp_fFormClass80Table[30][2] = 1977;
  mp_fFormClass80Table[30][3] = 2432;
  mp_fFormClass80Table[30][4] = 2860;
  mp_fFormClass80Table[30][5] = 3300;

  //***********************************
  // Form class 81 table
  //***********************************
  //Allocate memory array for the table
  mp_fFormClass81Table = new float*[m_iFormDBHIncs];
  for (i = 0; i < m_iFormDBHIncs; i++) {
    mp_fFormClass81Table[i] = new float[m_iMaxLogs];
  }

  //10-inch DBH
  mp_fFormClass81Table[0][0] = 40;
  mp_fFormClass81Table[0][1] = 65;
  mp_fFormClass81Table[0][2] = 82;
  mp_fFormClass81Table[0][3] = fMAX;
  mp_fFormClass81Table[0][4] = fMAX;
  mp_fFormClass81Table[0][5] = fMAX;

  //11-inch DBH
  mp_fFormClass81Table[1][0] = 50;
  mp_fFormClass81Table[1][1] = 82;
  mp_fFormClass81Table[1][2] = 106;
  mp_fFormClass81Table[1][3] = fMAX;
  mp_fFormClass81Table[1][4] = fMAX;
  mp_fFormClass81Table[1][5] = fMAX;

  //12-inch DBH
  mp_fFormClass81Table[2][0] = 60;
  mp_fFormClass81Table[2][1] = 100;
  mp_fFormClass81Table[2][2] = 130;
  mp_fFormClass81Table[2][3] = 150;
  mp_fFormClass81Table[2][4] = fMAX;
  mp_fFormClass81Table[2][5] = fMAX;

  //13-inch DBH
  mp_fFormClass81Table[3][0] = 72;
  mp_fFormClass81Table[3][1] = 122;
  mp_fFormClass81Table[3][2] = 160;
  mp_fFormClass81Table[3][3] = 186;
  mp_fFormClass81Table[3][4] = fMAX;
  mp_fFormClass81Table[3][5] = fMAX;

  //14-inch DBH
  mp_fFormClass81Table[4][0] = 85;
  mp_fFormClass81Table[4][1] = 144;
  mp_fFormClass81Table[4][2] = 190;
  mp_fFormClass81Table[4][3] = 221;
  mp_fFormClass81Table[4][4] = fMAX;
  mp_fFormClass81Table[4][5] = fMAX;

  //15-inch DBH
  mp_fFormClass81Table[5][0] = 100;
  mp_fFormClass81Table[5][1] = 170;
  mp_fFormClass81Table[5][2] = 228;
  mp_fFormClass81Table[5][3] = 268;
  mp_fFormClass81Table[5][4] = fMAX;
  mp_fFormClass81Table[5][5] = fMAX;

  //16-inch DBH
  mp_fFormClass81Table[6][0] = 115;
  mp_fFormClass81Table[6][1] = 197;
  mp_fFormClass81Table[6][2] = 265;
  mp_fFormClass81Table[6][3] = 316;
  mp_fFormClass81Table[6][4] = fMAX;
  mp_fFormClass81Table[6][5] = fMAX;

  //17-inch DBH
  mp_fFormClass81Table[7][0] = 132;
  mp_fFormClass81Table[7][1] = 226;
  mp_fFormClass81Table[7][2] = 306;
  mp_fFormClass81Table[7][3] = 366;
  mp_fFormClass81Table[7][4] = fMAX;
  mp_fFormClass81Table[7][5] = fMAX;

  //18-inch DBH
  mp_fFormClass81Table[8][0] = 148;
  mp_fFormClass81Table[8][1] = 256;
  mp_fFormClass81Table[8][2] = 346;
  mp_fFormClass81Table[8][3] = 415;
  mp_fFormClass81Table[8][4] = fMAX;
  mp_fFormClass81Table[8][5] = fMAX;

  //19-inch DBH
  mp_fFormClass81Table[9][0] = 166;
  mp_fFormClass81Table[9][1] = 290;
  mp_fFormClass81Table[9][2] = 392;
  mp_fFormClass81Table[9][3] = 471;
  mp_fFormClass81Table[9][4] = fMAX;
  mp_fFormClass81Table[9][5] = fMAX;

  //20-inch DBH
  mp_fFormClass81Table[10][0] = 185;
  mp_fFormClass81Table[10][1] = 323;
  mp_fFormClass81Table[10][2] = 439;
  mp_fFormClass81Table[10][3] = 527;
  mp_fFormClass81Table[10][4] = 598;
  mp_fFormClass81Table[10][5] = fMAX;

  //21-inch DBH
  mp_fFormClass81Table[11][0] = 206;
  mp_fFormClass81Table[11][1] = 360;
  mp_fFormClass81Table[11][2] = 492;
  mp_fFormClass81Table[11][3] = 592;
  mp_fFormClass81Table[11][4] = 676;
  mp_fFormClass81Table[11][5] = fMAX;

  //22-inch DBH
  mp_fFormClass81Table[12][0] = 227;
  mp_fFormClass81Table[12][1] = 398;
  mp_fFormClass81Table[12][2] = 544;
  mp_fFormClass81Table[12][3] = 656;
  mp_fFormClass81Table[12][4] = 754;
  mp_fFormClass81Table[12][5] = fMAX;

  //23-inch DBH
  mp_fFormClass81Table[13][0] = 250;
  mp_fFormClass81Table[13][1] = 439;
  mp_fFormClass81Table[13][2] = 602;
  mp_fFormClass81Table[13][3] = 724;
  mp_fFormClass81Table[13][4] = 838;
  mp_fFormClass81Table[13][5] = fMAX;

  //24-inch DBH
  mp_fFormClass81Table[14][0] = 272;
  mp_fFormClass81Table[14][1] = 480;
  mp_fFormClass81Table[14][2] = 659;
  mp_fFormClass81Table[14][3] = 791;
  mp_fFormClass81Table[14][4] = 923;
  mp_fFormClass81Table[14][5] = fMAX;

  //25-inch DBH
  mp_fFormClass81Table[15][0] = 298;
  mp_fFormClass81Table[15][1] = 528;
  mp_fFormClass81Table[15][2] = 726;
  mp_fFormClass81Table[15][3] = 877;
  mp_fFormClass81Table[15][4] = 1024;
  mp_fFormClass81Table[15][5] = fMAX;

  //26-inch DBH
  mp_fFormClass81Table[16][0] = 324;
  mp_fFormClass81Table[16][1] = 575;
  mp_fFormClass81Table[16][2] = 793;
  mp_fFormClass81Table[16][3] = 963;
  mp_fFormClass81Table[16][4] = 1124;
  mp_fFormClass81Table[16][5] = fMAX;

  //27-inch DBH
  mp_fFormClass81Table[17][0] = 351;
  mp_fFormClass81Table[17][1] = 624;
  mp_fFormClass81Table[17][2] = 863;
  mp_fFormClass81Table[17][3] = 1047;
  mp_fFormClass81Table[17][4] = 1226;
  mp_fFormClass81Table[17][5] = fMAX;

  //28-inch DBH
  mp_fFormClass81Table[18][0] = 378;
  mp_fFormClass81Table[18][1] = 674;
  mp_fFormClass81Table[18][2] = 933;
  mp_fFormClass81Table[18][3] = 1131;
  mp_fFormClass81Table[18][4] = 1327;
  mp_fFormClass81Table[18][5] = 1502;

  //29-inch DBH
  mp_fFormClass81Table[19][0] = 406;
  mp_fFormClass81Table[19][1] = 726;
  mp_fFormClass81Table[19][2] = 1006;
  mp_fFormClass81Table[19][3] = 1220;
  mp_fFormClass81Table[19][4] = 1438;
  mp_fFormClass81Table[19][5] = 1640;

  //30-inch DBH
  mp_fFormClass81Table[20][0] = 435;
  mp_fFormClass81Table[20][1] = 779;
  mp_fFormClass81Table[20][2] = 1080;
  mp_fFormClass81Table[20][3] = 1310;
  mp_fFormClass81Table[20][4] = 1549;
  mp_fFormClass81Table[20][5] = 1777;

  //31-inch DBH
  mp_fFormClass81Table[21][0] = 466;
  mp_fFormClass81Table[21][1] = 836;
  mp_fFormClass81Table[21][2] = 1162;
  mp_fFormClass81Table[21][3] = 1416;
  mp_fFormClass81Table[21][4] = 1666;
  mp_fFormClass81Table[21][5] = 1918;

  //32-inch DBH
  mp_fFormClass81Table[22][0] = 497;
  mp_fFormClass81Table[22][1] = 892;
  mp_fFormClass81Table[22][2] = 1245;
  mp_fFormClass81Table[22][3] = 1522;
  mp_fFormClass81Table[22][4] = 1784;
  mp_fFormClass81Table[22][5] = 2059;

  //33-inch DBH
  mp_fFormClass81Table[23][0] = 530;
  mp_fFormClass81Table[23][1] = 953;
  mp_fFormClass81Table[23][2] = 1332;
  mp_fFormClass81Table[23][3] = 1628;
  mp_fFormClass81Table[23][4] = 1910;
  mp_fFormClass81Table[23][5] = 2214;

  //34-inch DBH
  mp_fFormClass81Table[24][0] = 563;
  mp_fFormClass81Table[24][1] = 1014;
  mp_fFormClass81Table[24][2] = 1419;
  mp_fFormClass81Table[24][3] = 1734;
  mp_fFormClass81Table[24][4] = 2037;
  mp_fFormClass81Table[24][5] = 2368;

  //35-inch DBH
  mp_fFormClass81Table[25][0] = 600;
  mp_fFormClass81Table[25][1] = 1084;
  mp_fFormClass81Table[25][2] = 1518;
  mp_fFormClass81Table[25][3] = 1859;
  mp_fFormClass81Table[25][4] = 2188;
  mp_fFormClass81Table[25][5] = 2534;

  //36-inch DBH
  mp_fFormClass81Table[26][0] = 637;
  mp_fFormClass81Table[26][1] = 1154;
  mp_fFormClass81Table[26][2] = 1618;
  mp_fFormClass81Table[26][3] = 1984;
  mp_fFormClass81Table[26][4] = 2338;
  mp_fFormClass81Table[26][5] = 2700;

  //37-inch DBH
  mp_fFormClass81Table[27][0] = 674;
  mp_fFormClass81Table[27][1] = 1223;
  mp_fFormClass81Table[27][2] = 1721;
  mp_fFormClass81Table[27][3] = 2109;
  mp_fFormClass81Table[27][4] = 2494;
  mp_fFormClass81Table[27][5] = 2874;

  //38-inch DBH
  mp_fFormClass81Table[28][0] = 712;
  mp_fFormClass81Table[28][1] = 1292;
  mp_fFormClass81Table[28][2] = 1824;
  mp_fFormClass81Table[28][3] = 2234;
  mp_fFormClass81Table[28][4] = 2649;
  mp_fFormClass81Table[28][5] = 3049;

  //39-inch DBH
  mp_fFormClass81Table[29][0] = 751;
  mp_fFormClass81Table[29][1] = 1366;
  mp_fFormClass81Table[29][2] = 1928;
  mp_fFormClass81Table[29][3] = 2368;
  mp_fFormClass81Table[29][4] = 2796;
  mp_fFormClass81Table[29][5] = 3224;

  //40-inch DBH
  mp_fFormClass81Table[30][0] = 790;
  mp_fFormClass81Table[30][1] = 1441;
  mp_fFormClass81Table[30][2] = 2032;
  mp_fFormClass81Table[30][3] = 2502;
  mp_fFormClass81Table[30][4] = 2944;
  mp_fFormClass81Table[30][5] = 3399;

  //***********************************
  // Form class 84 table
  //***********************************
  //Allocate memory array for the table
  mp_fFormClass84Table = new float*[m_iFormDBHIncs];
  for (i = 0; i < m_iFormDBHIncs; i++) {
    mp_fFormClass84Table[i] = new float[m_iMaxLogs];
  }

  //10-inch DBH
  mp_fFormClass84Table[0][0] = 43;
  mp_fFormClass84Table[0][1] = 71;
  mp_fFormClass84Table[0][2] = 91;
  mp_fFormClass84Table[0][3] = fMAX;
  mp_fFormClass84Table[0][4] = fMAX;
  mp_fFormClass84Table[0][5] = fMAX;

  //11-inch DBH
  mp_fFormClass84Table[1][0] = 54;
  mp_fFormClass84Table[1][1] = 91;
  mp_fFormClass84Table[1][2] = 118;
  mp_fFormClass84Table[1][3] = fMAX;
  mp_fFormClass84Table[1][4] = fMAX;
  mp_fFormClass84Table[1][5] = fMAX;

  //12-inch DBH
  mp_fFormClass84Table[2][0] = 66;
  mp_fFormClass84Table[2][1] = 111;
  mp_fFormClass84Table[2][2] = 145;
  mp_fFormClass84Table[2][3] = 168;
  mp_fFormClass84Table[2][4] = fMAX;
  mp_fFormClass84Table[2][5] = fMAX;

  //13-inch DBH
  mp_fFormClass84Table[3][0] = 80;
  mp_fFormClass84Table[3][1] = 135;
  mp_fFormClass84Table[3][2] = 178;
  mp_fFormClass84Table[3][3] = 208;
  mp_fFormClass84Table[3][4] = fMAX;
  mp_fFormClass84Table[3][5] = fMAX;

  //14-inch DBH
  mp_fFormClass84Table[4][0] = 93;
  mp_fFormClass84Table[4][1] = 159;
  mp_fFormClass84Table[4][2] = 212;
  mp_fFormClass84Table[4][3] = 248;
  mp_fFormClass84Table[4][4] = fMAX;
  mp_fFormClass84Table[4][5] = fMAX;

  //15-inch DBH
  mp_fFormClass84Table[5][0] = 108;
  mp_fFormClass84Table[5][1] = 185;
  mp_fFormClass84Table[5][2] = 249;
  mp_fFormClass84Table[5][3] = 295;
  mp_fFormClass84Table[5][4] = fMAX;
  mp_fFormClass84Table[5][5] = fMAX;

  //16-inch DBH
  mp_fFormClass84Table[6][0] = 123;
  mp_fFormClass84Table[6][1] = 211;
  mp_fFormClass84Table[6][2] = 286;
  mp_fFormClass84Table[6][3] = 342;
  mp_fFormClass84Table[6][4] = fMAX;
  mp_fFormClass84Table[6][5] = fMAX;

  //17-inch DBH
  mp_fFormClass84Table[7][0] = 142;
  mp_fFormClass84Table[7][1] = 244;
  mp_fFormClass84Table[7][2] = 332;
  mp_fFormClass84Table[7][3] = 398;
  mp_fFormClass84Table[7][4] = fMAX;
  mp_fFormClass84Table[7][5] = fMAX;

  //18-inch DBH
  mp_fFormClass84Table[8][0] = 160;
  mp_fFormClass84Table[8][1] = 277;
  mp_fFormClass84Table[8][2] = 377;
  mp_fFormClass84Table[8][3] = 453;
  mp_fFormClass84Table[8][4] = fMAX;
  mp_fFormClass84Table[8][5] = fMAX;

  //19-inch DBH
  mp_fFormClass84Table[9][0] = 180;
  mp_fFormClass84Table[9][1] = 314;
  mp_fFormClass84Table[9][2] = 428;
  mp_fFormClass84Table[9][3] = 524;
  mp_fFormClass84Table[9][4] = fMAX;
  mp_fFormClass84Table[9][5] = fMAX;

  //20-inch DBH
  mp_fFormClass84Table[10][0] = 200;
  mp_fFormClass84Table[10][1] = 351;
  mp_fFormClass84Table[10][2] = 479;
  mp_fFormClass84Table[10][3] = 576;
  mp_fFormClass84Table[10][4] = 657;
  mp_fFormClass84Table[10][5] = fMAX;

  //21-inch DBH
  mp_fFormClass84Table[11][0] = 223;
  mp_fFormClass84Table[11][1] = 392;
  mp_fFormClass84Table[11][2] = 537;
  mp_fFormClass84Table[11][3] = 649;
  mp_fFormClass84Table[11][4] = 744;
  mp_fFormClass84Table[11][5] = fMAX;

  //22-inch DBH
  mp_fFormClass84Table[12][0] = 246;
  mp_fFormClass84Table[12][1] = 434;
  mp_fFormClass84Table[12][2] = 595;
  mp_fFormClass84Table[12][3] = 722;
  mp_fFormClass84Table[12][4] = 830;
  mp_fFormClass84Table[12][5] = fMAX;

  //23-inch DBH
  mp_fFormClass84Table[13][0] = 271;
  mp_fFormClass84Table[13][1] = 480;
  mp_fFormClass84Table[13][2] = 660;
  mp_fFormClass84Table[13][3] = 798;
  mp_fFormClass84Table[13][4] = 925;
  mp_fFormClass84Table[13][5] = fMAX;

  //24-inch DBH
  mp_fFormClass84Table[14][0] = 296;
  mp_fFormClass84Table[14][1] = 525;
  mp_fFormClass84Table[14][2] = 724;
  mp_fFormClass84Table[14][3] = 873;
  mp_fFormClass84Table[14][4] = 1020;
  mp_fFormClass84Table[14][5] = fMAX;

  //25-inch DBH
  mp_fFormClass84Table[15][0] = 322;
  mp_fFormClass84Table[15][1] = 572;
  mp_fFormClass84Table[15][2] = 790;
  mp_fFormClass84Table[15][3] = 958;
  mp_fFormClass84Table[15][4] = 1118;
  mp_fFormClass84Table[15][5] = fMAX;

  //26-inch DBH
  mp_fFormClass84Table[16][0] = 347;
  mp_fFormClass84Table[16][1] = 619;
  mp_fFormClass84Table[16][2] = 855;
  mp_fFormClass84Table[16][3] = 1042;
  mp_fFormClass84Table[16][4] = 1217;
  mp_fFormClass84Table[16][5] = fMAX;

  //27-inch DBH
  mp_fFormClass84Table[17][0] = 376;
  mp_fFormClass84Table[17][1] = 673;
  mp_fFormClass84Table[17][2] = 932;
  mp_fFormClass84Table[17][3] = 1136;
  mp_fFormClass84Table[17][4] = 1331;
  mp_fFormClass84Table[17][5] = fMAX;

  //28-inch DBH
  mp_fFormClass84Table[18][0] = 406;
  mp_fFormClass84Table[18][1] = 727;
  mp_fFormClass84Table[18][2] = 1010;
  mp_fFormClass84Table[18][3] = 1230;
  mp_fFormClass84Table[18][4] = 1445;
  mp_fFormClass84Table[18][5] = 1636;

  //29-inch DBH
  mp_fFormClass84Table[19][0] = 438;
  mp_fFormClass84Table[19][1] = 786;
  mp_fFormClass84Table[19][2] = 1092;
  mp_fFormClass84Table[19][3] = 1330;
  mp_fFormClass84Table[19][4] = 1569;
  mp_fFormClass84Table[19][5] = 1790;

  //30-inch DBH
  mp_fFormClass84Table[20][0] = 470;
  mp_fFormClass84Table[20][1] = 844;
  mp_fFormClass84Table[20][2] = 1173;
  mp_fFormClass84Table[20][3] = 1429;
  mp_fFormClass84Table[20][4] = 1693;
  mp_fFormClass84Table[20][5] = 1943;

  //31-inch DBH
  mp_fFormClass84Table[21][0] = 504;
  mp_fFormClass84Table[21][1] = 907;
  mp_fFormClass84Table[21][2] = 1265;
  mp_fFormClass84Table[21][3] = 1546;
  mp_fFormClass84Table[21][4] = 1823;
  mp_fFormClass84Table[21][5] = 2101;

  //32-inch DBH
  mp_fFormClass84Table[22][0] = 538;
  mp_fFormClass84Table[22][1] = 970;
  mp_fFormClass84Table[22][2] = 1357;
  mp_fFormClass84Table[22][3] = 1664;
  mp_fFormClass84Table[22][4] = 1953;
  mp_fFormClass84Table[22][5] = 2259;

  //33-inch DBH
  mp_fFormClass84Table[23][0] = 574;
  mp_fFormClass84Table[23][1] = 1037;
  mp_fFormClass84Table[23][2] = 1453;
  mp_fFormClass84Table[23][3] = 1782;
  mp_fFormClass84Table[23][4] = 2096;
  mp_fFormClass84Table[23][5] = 2431;

  //34-inch DBH
  mp_fFormClass84Table[24][0] = 611;
  mp_fFormClass84Table[24][1] = 1104;
  mp_fFormClass84Table[24][2] = 1549;
  mp_fFormClass84Table[24][3] = 1901;
  mp_fFormClass84Table[24][4] = 2240;
  mp_fFormClass84Table[24][5] = 2603;

  //35-inch DBH
  mp_fFormClass84Table[25][0] = 647;
  mp_fFormClass84Table[25][1] = 1173;
  mp_fFormClass84Table[25][2] = 1648;
  mp_fFormClass84Table[25][3] = 2023;
  mp_fFormClass84Table[25][4] = 2387;
  mp_fFormClass84Table[25][5] = 2766;

  //36-inch DBH
  mp_fFormClass84Table[26][0] = 683;
  mp_fFormClass84Table[26][1] = 1242;
  mp_fFormClass84Table[26][2] = 1746;
  mp_fFormClass84Table[26][3] = 2145;
  mp_fFormClass84Table[26][4] = 2534;
  mp_fFormClass84Table[26][5] = 2929;

  //37-inch DBH
  mp_fFormClass84Table[27][0] = 724;
  mp_fFormClass84Table[27][1] = 1318;
  mp_fFormClass84Table[27][2] = 1859;
  mp_fFormClass84Table[27][3] = 2284;
  mp_fFormClass84Table[27][4] = 2706;
  mp_fFormClass84Table[27][5] = 3123;

  //38-inch DBH
  mp_fFormClass84Table[28][0] = 765;
  mp_fFormClass84Table[28][1] = 1393;
  mp_fFormClass84Table[28][2] = 1972;
  mp_fFormClass84Table[28][3] = 2422;
  mp_fFormClass84Table[28][4] = 2877;
  mp_fFormClass84Table[28][5] = 3317;

  //39-inch DBH
  mp_fFormClass84Table[29][0] = 808;
  mp_fFormClass84Table[29][1] = 1476;
  mp_fFormClass84Table[29][2] = 2088;
  mp_fFormClass84Table[29][3] = 2570;
  mp_fFormClass84Table[29][4] = 3042;
  mp_fFormClass84Table[29][5] = 3512;

  //40-inch DBH
  mp_fFormClass84Table[30][0] = 851;
  mp_fFormClass84Table[30][1] = 1558;
  mp_fFormClass84Table[30][2] = 2203;
  mp_fFormClass84Table[30][3] = 2719;
  mp_fFormClass84Table[30][4] = 3208;
  mp_fFormClass84Table[30][5] = 3706;

  //***********************************
  // Form class 85 table
  //***********************************
  //Allocate memory array for the table
  mp_fFormClass85Table = new float*[m_iFormDBHIncs];
  for (i = 0; i < m_iFormDBHIncs; i++) {
    mp_fFormClass85Table[i] = new float[m_iMaxLogs];
  }

  //10-inch DBH
  mp_fFormClass85Table[0][0] = 45;
  mp_fFormClass85Table[0][1] = 74;
  mp_fFormClass85Table[0][2] = 94;
  mp_fFormClass85Table[0][3] = fMAX;
  mp_fFormClass85Table[0][4] = fMAX;
  mp_fFormClass85Table[0][5] = fMAX;

  //11-inch DBH
  mp_fFormClass85Table[1][0] = 56;
  mp_fFormClass85Table[1][1] = 94;
  mp_fFormClass85Table[1][2] = 122;
  mp_fFormClass85Table[1][3] = fMAX;
  mp_fFormClass85Table[1][4] = fMAX;
  mp_fFormClass85Table[1][5] = fMAX;

  //12-inch DBH
  mp_fFormClass85Table[2][0] = 68;
  mp_fFormClass85Table[2][1] = 114;
  mp_fFormClass85Table[2][2] = 150;
  mp_fFormClass85Table[2][3] = 173;
  mp_fFormClass85Table[2][4] = fMAX;
  mp_fFormClass85Table[2][5] = fMAX;

  //13-inch DBH
  mp_fFormClass85Table[3][0] = 82;
  mp_fFormClass85Table[3][1] = 138;
  mp_fFormClass85Table[3][2] = 184;
  mp_fFormClass85Table[3][3] = 214;
  mp_fFormClass85Table[3][4] = fMAX;
  mp_fFormClass85Table[3][5] = fMAX;

  //14-inch DBH
  mp_fFormClass85Table[4][0] = 95;
  mp_fFormClass85Table[4][1] = 163;
  mp_fFormClass85Table[4][2] = 217;
  mp_fFormClass85Table[4][3] = 254;
  mp_fFormClass85Table[4][4] = fMAX;
  mp_fFormClass85Table[4][5] = fMAX;

  //15-inch DBH
  mp_fFormClass85Table[5][0] = 111;
  mp_fFormClass85Table[5][1] = 191;
  mp_fFormClass85Table[5][2] = 257;
  mp_fFormClass85Table[5][3] = 304;
  mp_fFormClass85Table[5][4] = fMAX;
  mp_fFormClass85Table[5][5] = fMAX;

  //16-inch DBH
  mp_fFormClass85Table[6][0] = 127;
  mp_fFormClass85Table[6][1] = 219;
  mp_fFormClass85Table[6][2] = 297;
  mp_fFormClass85Table[6][3] = 355;
  mp_fFormClass85Table[6][4] = fMAX;
  mp_fFormClass85Table[6][5] = fMAX;

  //17-inch DBH
  mp_fFormClass85Table[7][0] = 146;
  mp_fFormClass85Table[7][1] = 252;
  mp_fFormClass85Table[7][2] = 342;
  mp_fFormClass85Table[7][3] = 412;
  mp_fFormClass85Table[7][4] = fMAX;
  mp_fFormClass85Table[7][5] = fMAX;

  //18-inch DBH
  mp_fFormClass85Table[8][0] = 164;
  mp_fFormClass85Table[8][1] = 285;
  mp_fFormClass85Table[8][2] = 388;
  mp_fFormClass85Table[8][3] = 468;
  mp_fFormClass85Table[8][4] = fMAX;
  mp_fFormClass85Table[8][5] = fMAX;

  //19-inch DBH
  mp_fFormClass85Table[9][0] = 184;
  mp_fFormClass85Table[9][1] = 322;
  mp_fFormClass85Table[9][2] = 440;
  mp_fFormClass85Table[9][3] = 531;
  mp_fFormClass85Table[9][4] = fMAX;
  mp_fFormClass85Table[9][5] = fMAX;

  //20-inch DBH
  mp_fFormClass85Table[10][0] = 205;
  mp_fFormClass85Table[10][1] = 360;
  mp_fFormClass85Table[10][2] = 492;
  mp_fFormClass85Table[10][3] = 594;
  mp_fFormClass85Table[10][4] = 678;
  mp_fFormClass85Table[10][5] = fMAX;

  //21-inch DBH
  mp_fFormClass85Table[11][0] = 228;
  mp_fFormClass85Table[11][1] = 402;
  mp_fFormClass85Table[11][2] = 550;
  mp_fFormClass85Table[11][3] = 667;
  mp_fFormClass85Table[11][4] = 765;
  mp_fFormClass85Table[11][5] = fMAX;

  //22-inch DBH
  mp_fFormClass85Table[12][0] = 251;
  mp_fFormClass85Table[12][1] = 444;
  mp_fFormClass85Table[12][2] = 609;
  mp_fFormClass85Table[12][3] = 740;
  mp_fFormClass85Table[12][4] = 852;
  mp_fFormClass85Table[12][5] = fMAX;

  //23-inch DBH
  mp_fFormClass85Table[13][0] = 276;
  mp_fFormClass85Table[13][1] = 490;
  mp_fFormClass85Table[13][2] = 675;
  mp_fFormClass85Table[13][3] = 818;
  mp_fFormClass85Table[13][4] = 950;
  mp_fFormClass85Table[13][5] = fMAX;

  //24-inch DBH
  mp_fFormClass85Table[14][0] = 302;
  mp_fFormClass85Table[14][1] = 537;
  mp_fFormClass85Table[14][2] = 741;
  mp_fFormClass85Table[14][3] = 895;
  mp_fFormClass85Table[14][4] = 1047;
  mp_fFormClass85Table[14][5] = fMAX;

  //25-inch DBH
  mp_fFormClass85Table[15][0] = 330;
  mp_fFormClass85Table[15][1] = 588;
  mp_fFormClass85Table[15][2] = 812;
  mp_fFormClass85Table[15][3] = 986;
  mp_fFormClass85Table[15][4] = 1153;
  mp_fFormClass85Table[15][5] = fMAX;

  //26-inch DBH
  mp_fFormClass85Table[16][0] = 357;
  mp_fFormClass85Table[16][1] = 638;
  mp_fFormClass85Table[16][2] = 882;
  mp_fFormClass85Table[16][3] = 1076;
  mp_fFormClass85Table[16][4] = 1259;
  mp_fFormClass85Table[16][5] = fMAX;

  //27-inch DBH
  mp_fFormClass85Table[17][0] = 387;
  mp_fFormClass85Table[17][1] = 693;
  mp_fFormClass85Table[17][2] = 961;
  mp_fFormClass85Table[17][3] = 1172;
  mp_fFormClass85Table[17][4] = 1374;
  mp_fFormClass85Table[17][5] = fMAX;

  //28-inch DBH
  mp_fFormClass85Table[18][0] = 417;
  mp_fFormClass85Table[18][1] = 745;
  mp_fFormClass85Table[18][2] = 1040;
  mp_fFormClass85Table[18][3] = 1267;
  mp_fFormClass85Table[18][4] = 1490;
  mp_fFormClass85Table[18][5] = 1689;

  //29-inch DBH
  mp_fFormClass85Table[19][0] = 448;
  mp_fFormClass85Table[19][1] = 807;
  mp_fFormClass85Table[19][2] = 1122;
  mp_fFormClass85Table[19][3] = 1368;
  mp_fFormClass85Table[19][4] = 1616;
  mp_fFormClass85Table[19][5] = 1844;

  //30-inch DBH
  mp_fFormClass85Table[20][0] = 481;
  mp_fFormClass85Table[20][1] = 866;
  mp_fFormClass85Table[20][2] = 1205;
  mp_fFormClass85Table[20][3] = 1469;
  mp_fFormClass85Table[20][4] = 1741;
  mp_fFormClass85Table[20][5] = 1999;

  //31-inch DBH
  mp_fFormClass85Table[21][0] = 516;
  mp_fFormClass85Table[21][1] = 930;
  mp_fFormClass85Table[21][2] = 1298;
  mp_fFormClass85Table[21][3] = 1588;
  mp_fFormClass85Table[21][4] = 1874;
  mp_fFormClass85Table[21][5] = 2160;

  //32-inch DBH
  mp_fFormClass85Table[22][0] = 550;
  mp_fFormClass85Table[22][1] = 993;
  mp_fFormClass85Table[22][2] = 1391;
  mp_fFormClass85Table[22][3] = 1706;
  mp_fFormClass85Table[22][4] = 2006;
  mp_fFormClass85Table[22][5] = 2321;

  //33-inch DBH
  mp_fFormClass85Table[23][0] = 587;
  mp_fFormClass85Table[23][1] = 1061;
  mp_fFormClass85Table[23][2] = 1488;
  mp_fFormClass85Table[23][3] = 1827;
  mp_fFormClass85Table[23][4] = 2150;
  mp_fFormClass85Table[23][5] = 2495;

  //34-inch DBH
  mp_fFormClass85Table[24][0] = 624;
  mp_fFormClass85Table[24][1] = 1129;
  mp_fFormClass85Table[24][2] = 1586;
  mp_fFormClass85Table[24][3] = 1948;
  mp_fFormClass85Table[24][4] = 2294;
  mp_fFormClass85Table[24][5] = 2669;

  //35-inch DBH
  mp_fFormClass85Table[25][0] = 663;
  mp_fFormClass85Table[25][1] = 1204;
  mp_fFormClass85Table[25][2] = 1692;
  mp_fFormClass85Table[25][3] = 2080;
  mp_fFormClass85Table[25][4] = 2454;
  mp_fFormClass85Table[25][5] = 2846;

  //36-inch DBH
  mp_fFormClass85Table[26][0] = 702;
  mp_fFormClass85Table[26][1] = 1278;
  mp_fFormClass85Table[26][2] = 1797;
  mp_fFormClass85Table[26][3] = 2212;
  mp_fFormClass85Table[26][4] = 2614;
  mp_fFormClass85Table[26][5] = 3022;

  //37-inch DBH
  mp_fFormClass85Table[27][0] = 744;
  mp_fFormClass85Table[27][1] = 1355;
  mp_fFormClass85Table[27][2] = 1912;
  mp_fFormClass85Table[27][3] = 2352;
  mp_fFormClass85Table[27][4] = 2788;
  mp_fFormClass85Table[27][5] = 3219;

  //38-inch DBH
  mp_fFormClass85Table[28][0] = 785;
  mp_fFormClass85Table[28][1] = 1432;
  mp_fFormClass85Table[28][2] = 2027;
  mp_fFormClass85Table[28][3] = 2493;
  mp_fFormClass85Table[28][4] = 2962;
  mp_fFormClass85Table[28][5] = 3416;

  //39-inch DBH
  mp_fFormClass85Table[29][0] = 828;
  mp_fFormClass85Table[29][1] = 1515;
  mp_fFormClass85Table[29][2] = 2144;
  mp_fFormClass85Table[29][3] = 2644;
  mp_fFormClass85Table[29][4] = 3130;
  mp_fFormClass85Table[29][5] = 3614;

  //40-inch DBH
  mp_fFormClass85Table[30][0] = 872;
  mp_fFormClass85Table[30][1] = 1598;
  mp_fFormClass85Table[30][2] = 2260;
  mp_fFormClass85Table[30][3] = 2795;
  mp_fFormClass85Table[30][4] = 3298;
  mp_fFormClass85Table[30][5] = 3813;
}
