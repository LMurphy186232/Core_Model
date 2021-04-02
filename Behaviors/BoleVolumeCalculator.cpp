//---------------------------------------------------------------------------
// BoleVolumeCalculator.cpp
//---------------------------------------------------------------------------
#include "BoleVolumeCalculator.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Constants.h"
#include <math.h>
#include <sstream>

//The amount of taper is found by multiplying the DBH by 0.4
#define MERCH_RATIO 0.4
#define LOG_LENGTH 16
#define MIN_DBH 10

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clBoleVolumeCalculator::clBoleVolumeCalculator( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "TreeBoleVolumeCalculator";
    m_sXMLRoot = "TreeBoleVolumeCalculator";

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

    mp_fB0 = NULL;
    mp_fB1 = NULL;
    mp_fB2 = NULL;
    mp_fB3 = NULL;
    mp_fB4 = NULL;
    mp_fB5 = NULL;
    mp_fFormClass = NULL;
    mp_fTaperTable = NULL;
    mp_iIndexes = NULL;
    mp_iVolumeCodes = NULL;

  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBoleVolumeCalculator::clBoleVolumeCalculator" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clBoleVolumeCalculator::~clBoleVolumeCalculator()
{
  delete[] mp_fB0;
  delete[] mp_fB1;
  delete[] mp_fB2;
  delete[] mp_fB3;
  delete[] mp_fB4;
  delete[] mp_fB5;
  delete[] mp_fFormClass;
  delete[] mp_iIndexes;
  if ( mp_iVolumeCodes )
  {
    for ( int i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iVolumeCodes[i];
    }
  }
  delete[] mp_iVolumeCodes;

  if ( mp_fTaperTable )
  {
    for ( int i = 0; i < m_iDBHIncs; i++ )
    {
      delete[] mp_fTaperTable[i];
    }
  }
  delete[] mp_fTaperTable;
}

/////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
/////////////////////////////////////////////////////////////////////////////
void clBoleVolumeCalculator::GetParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL;
  try {

    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;
    int i;

    //Set up our doubleVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare our arrays
    mp_fB0 = new double[m_iNumBehaviorSpecies];
    mp_fB1 = new double[m_iNumBehaviorSpecies];
    mp_fB2 = new double[m_iNumBehaviorSpecies];
    mp_fB3 = new double[m_iNumBehaviorSpecies];
    mp_fB4 = new double[m_iNumBehaviorSpecies];
    mp_fB5 = new double[m_iNumBehaviorSpecies];
    mp_fFormClass = new double[m_iNumBehaviorSpecies];

    //Get the parameter file values

    //b0
    FillSpeciesSpecificValue( p_oElement, "an_boleB0", "an_bb0Val", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB0[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //b1
    FillSpeciesSpecificValue( p_oElement, "an_boleB1", "an_bb1Val", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB1[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //b2
    FillSpeciesSpecificValue( p_oElement, "an_boleB2", "an_bb2Val", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB2[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //b3
    FillSpeciesSpecificValue( p_oElement, "an_boleB3", "an_bb3Val", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB3[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //b4
    FillSpeciesSpecificValue( p_oElement, "an_boleB4", "an_bb4Val", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB4[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //b5
    FillSpeciesSpecificValue( p_oElement, "an_boleB5", "an_bb5Val", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB5[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Form classes
    FillSpeciesSpecificValue( p_oElement, "an_boleFormClasses", "an_bfcVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fFormClass[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Validate that the form classes are between 60 and 100%
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (mp_fFormClass[i] < 60 || mp_fFormClass[i] > 100) {
        modelErr stcErr;
        stcErr.sFunction = "clBoleVolumeCalculator::GetParameterFileData" ;
        std::stringstream s;
        s << "Invalid form class value \"" << mp_fFormClass[i]
          << "\" for species " << mp_iWhatSpecies[i]
          << ".  Form class values must be between 60 and 100.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Convert to proportions
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fFormClass[i] *= 0.01; //convert to proportion
      mp_fFormClass[i] = 1 - mp_fFormClass[i]; //invert
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
    stcErr.sFunction = "clBoleVolumeCalculator::GetParameterFileData" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clBoleVolumeCalculator::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    GetParameterFileData(p_oDoc);
    FormatQueryString();
    PopulateTaperTable();
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
    stcErr.sFunction = "clBoleVolumeCalculator::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// PopulateTaperTable()
/////////////////////////////////////////////////////////////////////////////
void clBoleVolumeCalculator::PopulateTaperTable()
{
  float fMAX = 10000;
  int i;

  //Allocate memory array for the table
  mp_fTaperTable = new float*[m_iDBHIncs];
  for (i = 0; i < m_iDBHIncs; i++) {
    mp_fTaperTable[i] = new float[m_iMaxLogs];
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
}

/////////////////////////////////////////////////////////////////////////////
// GetBoleHeight()
/////////////////////////////////////////////////////////////////////////////
int clBoleVolumeCalculator::GetBoleHeight(const float &fDBH, const int &iSpecies)
{
  float fTargetTaper = fDBH * MERCH_RATIO, //diameter at the merchantable height
        fTaperAmt; //keeps track of the amount of taper so far
  int iNumLogs = 1, iDBHIndex, iTemp, i;

  //Get the amount of taper left after subtracting that at the top of the first
  //log; get the amount of taper at the top of the first log from the form class
  fTaperAmt = fTargetTaper - (fDBH * mp_fFormClass[mp_iIndexes[iSpecies]]);

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

  return iNumLogs * LOG_LENGTH;
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clBoleVolumeCalculator::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    float fVolume, //tree's volume
         fDBH, //tree's dbh
         fTreeHeight; //tree's height
    int iSp, iTp;

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find(m_sQuery);

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the volume data member
      if ( -1 != mp_iVolumeCodes[mp_iIndexes[iSp]] [iTp - clTreePopulation::sapling] )
      {

        //Get this tree's dbh and convert it to a value in inches
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDBH );
        fDBH *= CONVERT_CM_TO_IN;

        //If it's below the minimum, give a volume of 0
        if (fDBH < MIN_DBH) {
          fVolume = 0;
          p_oTree->SetValue( mp_iVolumeCodes[mp_iIndexes[iSp]] [iTp - clTreePopulation::sapling], fVolume );
        } else {

          //Get the bole height
          fTreeHeight = GetBoleHeight(fDBH, iSp);

          //Calculate the volume
          fVolume = mp_fB0[mp_iIndexes[iSp]] +
                    (mp_fB1[mp_iIndexes[iSp]] * pow(fDBH, mp_fB2[mp_iIndexes[iSp]])) +
                    (mp_fB3[mp_iIndexes[iSp]] * pow(fDBH, mp_fB4[mp_iIndexes[iSp]]) * pow(fTreeHeight, mp_fB5[mp_iIndexes[iSp]]));
          p_oTree->SetValue( mp_iVolumeCodes[mp_iIndexes[iSp]] [iTp - clTreePopulation::sapling], fVolume );
        }
      }

      p_oTree = p_oBehaviorTrees->NextTree();
    }
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBoleVolumeCalculator::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clBoleVolumeCalculator::FormatQueryString()
{
  std::stringstream sQueryTemp;
  int i;
  bool bSapling = false, bAdult = false, bSnag = false;

  //Do a type/species search on all the types and species
  sQueryTemp << "species=";
  for (i = 0; i < m_iNumBehaviorSpecies - 1; i++) {
    sQueryTemp << mp_iWhatSpecies[i] << ",";
  }
  sQueryTemp << mp_iWhatSpecies[m_iNumBehaviorSpecies - 1];

  //Find all the types
  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType ) {
      bSapling = true;
    } else if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType ) {
      bAdult = true;
    } else if ( clTreePopulation::snag == mp_whatSpeciesTypeCombos[i].iType ) {
      bSnag = true;
    }
  }
  sQueryTemp << "::type=";
  if (bSapling) {
    sQueryTemp << clTreePopulation::sapling << ",";
  } if (bAdult) {
    sQueryTemp << clTreePopulation::adult << ",";
  } if (bSnag) {
    sQueryTemp << clTreePopulation::snag << ",";
  }

  //Remove the last comma and put it in m_sQuery
  m_sQuery = sQueryTemp.str().substr(0, sQueryTemp.str().length() - 1);
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clBoleVolumeCalculator::RegisterTreeDataMembers()
{

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int i, j;

  mp_iIndexes = new short int[p_oPop->GetNumberOfSpecies()];

  //Make the list of indexes
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_iIndexes[mp_iWhatSpecies[i]] = i;

  //Declare the array and register our new data member
  mp_iVolumeCodes = new short int * [m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    mp_iVolumeCodes[i] = new short int[3];
    for (j = 0; j < 3; j++)
      mp_iVolumeCodes[i][j] = -1;
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Make sure that only allowed types have been applied
    if ( clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::snag != mp_whatSpeciesTypeCombos[i].iType )
         {

           modelErr stcErr;
           stcErr.sFunction = "clBoleVolumeCalculator::RegisterTreeDataMembers" ;
           stcErr.sMoreInfo = "This behavior can only be applied to saplings, adults, and snags.";
           stcErr.iErrorCode = BAD_DATA;
           throw( stcErr );
    }

    //Register the code and capture it
    mp_iVolumeCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
         [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
         p_oPop->RegisterFloat( "Bole Vol", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}
