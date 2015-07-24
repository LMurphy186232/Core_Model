//---------------------------------------------------------------------------
#include "Output.h"
#include "Grid.h"
#include "SimManager.h"
#include "Plot.h"
#include "TreePopulation.h"
#include "GhostTreePopulation.h"
#include "ParsingFunctions.h"
#include "PlatformFuncs.h"
#include <sstream>

const int BUFFER_SIZE = 1023;

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clOutput::clOutput( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "Output";

    //Versions
    m_fVersionNumber = 1.3;
    m_fMinimumVersionNumber = 1;

    //Null pointers, initialize variables to empty
    mp_treeSettings = NULL;
    mp_gridSettings = NULL;
    mp_deadTreeSettings = NULL;
    mp_masterTreeSettings = NULL;
    mp_subplots = NULL;
    m_iNumGridsToSave = 0;

    m_iNumYCells = 0;
    m_iNumXCells = 0;
    m_fYCellLength = 0;
    m_iNumTypes = 0;
    m_iNumSubplotsToSave = 0;
    m_iNumSpecies = 0;
    m_fXCellLength = 0;

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
    stcErr.sFunction = "clOutput::clOutput" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clOutput::~clOutput()
{
  int i, j, k; //loop counter

  //Delete arrays
  if ( mp_treeSettings ) {
    for ( i = 0; i < m_iNumSpecies; i++ ) {
      for ( j = 0; j < m_iNumTypes; j++ ) {
        delete[] mp_treeSettings[i][j].p_iIntCodes;
        delete[] mp_treeSettings[i][j].p_iFloatCodes;
        delete[] mp_treeSettings[i][j].p_iStringCodes;
        delete[] mp_treeSettings[i][j].p_iBoolCodes;
      }
      delete[] mp_treeSettings[i];
    }
    delete[] mp_treeSettings; mp_treeSettings = NULL;
  }

  if ( mp_deadTreeSettings ) {
    for ( i = 0; i < m_iNumSpecies; i++ ) {
      for ( j = 0; j < m_iNumTypes; j++ ) {
        for ( k = 0; k <= remove_tree; k++) {
          delete[] mp_deadTreeSettings[i][j][k].p_iIntCodes;
          delete[] mp_deadTreeSettings[i][j][k].p_iFloatCodes;
          delete[] mp_deadTreeSettings[i][j][k].p_iStringCodes;
          delete[] mp_deadTreeSettings[i][j][k].p_iBoolCodes;
        }
        delete[] mp_deadTreeSettings[i][j];
      }
      delete[] mp_deadTreeSettings[i];
    }
    delete[] mp_deadTreeSettings; mp_deadTreeSettings = NULL;
  }

  if ( mp_gridSettings ) {
    for ( i = 0; i < m_iNumGridsToSave; i++ ) {
      delete[] mp_gridSettings[i].p_iIntCodes;
      delete[] mp_gridSettings[i].p_iFloatCodes;
      delete[] mp_gridSettings[i].p_iStringCodes;
      delete[] mp_gridSettings[i].p_iBoolCodes;
      delete[] mp_gridSettings[i].p_iPackageIntCodes;
      delete[] mp_gridSettings[i].p_iPackageFloatCodes;
      delete[] mp_gridSettings[i].p_iPackageStringCodes;
      delete[] mp_gridSettings[i].p_iPackageBoolCodes;
    }
    delete[] mp_gridSettings;
  }
  if (mp_masterTreeSettings) {
    for ( i = 0; i < m_iNumSpecies; i++ ) {
      for ( j = 0; j < m_iNumTypes; j++ ) {
        delete[] mp_masterTreeSettings[i][j].p_iIntCodes;
        delete[] mp_masterTreeSettings[i][j].p_iFloatCodes;
        delete[] mp_masterTreeSettings[i][j].p_iStringCodes;
        delete[] mp_masterTreeSettings[i][j].p_iBoolCodes;
      }
      delete[] mp_masterTreeSettings[i];
    }
    delete[] mp_masterTreeSettings; mp_masterTreeSettings = NULL;
  }
  if (mp_subplots) {
    for (i = 0; i < m_iNumSubplotsToSave; i++) {
      for (j = 0; j < m_iNumXCells; j++) {
        delete[] mp_subplots[i].p_bUseCell[j];
      }
      delete[] mp_subplots[i].p_bUseCell;
    }
    delete[] mp_subplots;
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clOutput::GetData( DOMDocument * p_oDoc )
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );;
  DOMElement * p_oElement; //for casting to DOM_Element
  string sTemp;
  int i, j;
  bool bTest;

  //Get the number of cells in each direction
  m_iNumXCells = p_oPop->GetNumXCells();
  m_iNumYCells = p_oPop->GetNumYCells();
  m_fXCellLength = p_oPop->GetGridCellSize();
  m_fYCellLength = p_oPop->GetGridCellSize();

  ExtractLiveTreeInfo( p_oDoc );
  ExtractDeadTreeInfo( p_oDoc );
  ExtractGridInfo( p_oDoc );
  ExtractSubplotInfo( p_oDoc );

  MakeMasterTreeSettings();

  //General data and flags

  //Filenames
  //get the doc root element
  p_oElement = p_oDoc->getDocumentElement();
  FillSingleValue( p_oElement, "ou_filename", &m_sFileRoot, true );

  //Strip the filename of the tarball extension, if it's there, to get the
  //file root

  //Work backwards the length of the file extension and compare
  if ( m_sFileRoot.length() > TARBALL_FILE_EXT.length()) {

    if(m_sFileRoot.substr(m_sFileRoot.length() - TARBALL_FILE_EXT.length()).compare(TARBALL_FILE_EXT) == 0) {

      //The file extension's already on there - so strip the extension
      //off of root
      m_sFileRoot = m_sFileRoot.substr(0,
          m_sFileRoot.length() - TARBALL_FILE_EXT.length());
    }
  }
  m_sTarball = m_sFileRoot + TARBALL_FILE_EXT;

  //Make the tarball names of all the subplots
  for (i = 0; i < m_iNumSubplotsToSave; i++) {
    mp_subplots[i].sSubplotTarball = m_sFileRoot + "_" +
        mp_subplots[i].sSubplotName + TARBALL_FILE_EXT;
  }

  //Now take off the directory structure so that TAR won't preserve a
  //directory structure for files - which is good since very long filenames
  //confound TAR otherwise
  if (m_sFileRoot.find('\\') != string::npos) {
    m_sFileRoot.substr(m_sFileRoot.rfind('\\', m_sFileRoot.length()) + 1);
  }
  if (m_sFileRoot.find('/') != string::npos) {
    m_sFileRoot.substr(m_sFileRoot.rfind('/', m_sFileRoot.length()) + 1);
  }

  //Just double-check that we have settings to save - if not, erase the
  //filename
  bTest = false;
  for ( i = 0; i < m_iNumSpecies; i++ )
    for ( j = 0; j < m_iNumTypes; j++ )
      if ( mp_treeSettings[i][j].iSaveFreq > -1 )
      {
        bTest = true; break;
      }
  if ( false == bTest && 0 == m_iNumGridsToSave )
    m_sFileRoot = "";

  //Make sure that everything's in place for zipping - if not throw error
  sTemp = mp_oSimManager->GetAppPath();
  if ( !TarballSetup( sTemp ) )
  {
    modelErr stcErr;
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    stcErr.sMoreInfo = "Missing file gzip.exe and/or tar.exe in path ";
    stcErr.sMoreInfo += mp_oSimManager->GetAppPath();
    stcErr.sFunction = "clOutput::GetData" ;
    throw( stcErr );
  }

  //Write the output file headers
  WriteDetailedOutputHeader();

  //Write the initial conditions timestep
  Action();
}


/////////////////////////////////////////////////////////////////////////////
// ExtractLiveTreeInfo()
/////////////////////////////////////////////////////////////////////////////
void clOutput::ExtractLiveTreeInfo( DOMDocument * p_oDoc )
{
  char * cData = NULL; //for extracting data from the file
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );;
    DOMNodeList * p_oSettingsList, //holds the list of tree info structures
    * p_oDataMemberList; //holds list of tree data members
    DOMNode * p_oDocNode;
    DOMElement * p_oTreeInfo, //one tree type's info
    * p_oDataMember; //one data member
    XMLCh *sVal;
    short int iNumSettings, //how many types we have output data for in the file
    iSpecies, iType, //species number
    iDataMemberCode, //data member code for a label
    i, j; //loop counters

    //Get the tree population
    m_iNumSpecies = p_oPop->GetNumberOfSpecies();
    m_iNumTypes = p_oPop->GetNumberOfTypes();

    // Look for tree info
    //Search for all tree type info sets
    sVal = XMLString::transcode( "ou_treeInfo" );
    p_oSettingsList = p_oDoc->getElementsByTagName( sVal );
    XMLString::release(&sVal);
    iNumSettings = p_oSettingsList->getLength();

    //Allocate the tree memory and initialize - set frequencies to -1 so we
    //can distinguish absence of data from the user setting 0
    mp_treeSettings = new stcTreeOutputInfo * [m_iNumSpecies];
    for ( i = 0; i < m_iNumSpecies; i++ ) {
      mp_treeSettings[i] = new stcTreeOutputInfo[m_iNumTypes];
      for ( j = 0; j < m_iNumTypes; j++ ) {
        mp_treeSettings[i][j].iSaveFreq = -1;
        mp_treeSettings[i][j].iSumTimestep = -1;
        mp_treeSettings[i][j].iNumInts = 0;
        mp_treeSettings[i][j].iNumFloats = 0;
        mp_treeSettings[i][j].iNumStrings = 0;
        mp_treeSettings[i][j].iNumBools = 0;
        mp_treeSettings[i][j].bSaveThisTimeStep = false;
        mp_treeSettings[i][j].p_iIntCodes = NULL;
        mp_treeSettings[i][j].p_iFloatCodes = NULL;
        mp_treeSettings[i][j].p_iStringCodes = NULL;
        mp_treeSettings[i][j].p_iBoolCodes = NULL;
      }
    }

    //If we found nothing, exit with no error
    if ( iNumSettings == 0 ) return;

    //go through each type and break out its info
    for ( i = 0; i < iNumSettings; i++ )
    {
      p_oDocNode = p_oSettingsList->item( i );
      p_oTreeInfo = ( DOMElement * ) p_oDocNode;

      //get the type name - if something goes wrong throw an error
      sVal = XMLString::transcode( "type" );
      cData = XMLString::transcode( p_oTreeInfo->getAttributeNode( sVal )->getNodeValue() );
      XMLString::release(&sVal);
      if ( strcmp( "Seed", cData ) == 0 )
        iType = clTreePopulation::seed;
      else if ( strcmp( "Seedling", cData ) == 0 )
        iType = clTreePopulation::seedling;
      else if ( strcmp( "Sapling", cData ) == 0 )
        iType = clTreePopulation::sapling;
      else if ( strcmp( "Adult", cData ) == 0 )
        iType = clTreePopulation::adult;
      else if ( strcmp( "Snag", cData ) == 0 )
        iType = clTreePopulation::snag;
      else if ( strcmp( "Woody_debris", cData ) == 0 )
        iType = clTreePopulation::woody_debris;
      else
      {
        modelErr stcErr;
        stcErr.sFunction = "clOutput::ExtractLiveTreeInfo" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "unrecognized type - ";
        stcErr.sMoreInfo += cData;
        throw( stcErr );
      }
      delete[] cData; cData = NULL;

      //Species
      sVal = XMLString::transcode( "species" );
      cData = XMLString::transcode( p_oTreeInfo->getAttributeNode( sVal )->getNodeValue() );
      XMLString::release(&sVal);
      iSpecies = p_oPop->TranslateSpeciesNameToCode( cData );
      if ( -1 == iSpecies )
      { //unrecognized species - throw error
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clOutput::ExtractLiveTreeInfo" ;
        stcErr.sMoreInfo = "Unrecognized species - ";
        stcErr.sMoreInfo += cData;
        throw( stcErr );
      }
      delete[] cData; cData = NULL;

      //Frequency data
      FillSingleValue( p_oTreeInfo, "ou_saveFreq", ( int * ) & mp_treeSettings[iSpecies] [iType].iSaveFreq, true );

      //Make sure that the frequency is not negative - if so, throw error
      if ( mp_treeSettings[iSpecies][iType].iSaveFreq < 0 ) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clOutput::ExtractLiveTreeInfo" ;
        stcErr.sMoreInfo = "Save frequency is negative";
        throw( stcErr );
      }

      //Number to save for each data member type
      //****************Ints*******************
      sVal = XMLString::transcode( "ou_int" );
      p_oDataMemberList = p_oTreeInfo->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() ) {
        mp_treeSettings[iSpecies] [iType].iNumInts = p_oDataMemberList->getLength();
        //Declare the int array
        mp_treeSettings[iSpecies] [iType].p_iIntCodes = new short int[mp_treeSettings[iSpecies] [iType].iNumInts];
        for ( j = 0; j < mp_treeSettings[iSpecies] [iType].iNumInts; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = p_oPop->GetIntDataCode( cData, iSpecies, iType );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractLiveTreeInfo" ;
            stcErr.sMoreInfo = "Unrecognized int data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_treeSettings[iSpecies] [iType].p_iIntCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Floats*******************
      sVal = XMLString::transcode( "ou_float" );
      p_oDataMemberList = p_oTreeInfo->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_treeSettings[iSpecies] [iType].iNumFloats = p_oDataMemberList->getLength();
        //Declare the float array
        mp_treeSettings[iSpecies] [iType].p_iFloatCodes = new short int[mp_treeSettings[iSpecies] [iType].iNumFloats];
        for ( j = 0; j < mp_treeSettings[iSpecies] [iType].iNumFloats; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = p_oPop->GetFloatDataCode( cData, iSpecies, iType );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractLiveTreeInfo" ;
            stcErr.sMoreInfo = "Unrecognized float data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_treeSettings[iSpecies] [iType].p_iFloatCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Chars*******************
      sVal = XMLString::transcode( "ou_char" );
      p_oDataMemberList = p_oTreeInfo->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_treeSettings[iSpecies] [iType].iNumStrings = p_oDataMemberList->getLength();
        //Declare the int array
        mp_treeSettings[iSpecies] [iType].p_iStringCodes = new short int[mp_treeSettings[iSpecies] [iType].iNumStrings];
        for ( j = 0; j < mp_treeSettings[iSpecies] [iType].iNumStrings; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = p_oPop->GetStringDataCode( cData, iSpecies, iType );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractLiveTreeInfo" ;
            stcErr.sMoreInfo = "Unrecognized char data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_treeSettings[iSpecies] [iType].p_iStringCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Bools*******************
      sVal = XMLString::transcode( "ou_bool" );
      p_oDataMemberList = p_oTreeInfo->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_treeSettings[iSpecies] [iType].iNumBools = p_oDataMemberList->getLength();
        //Declare the int array
        mp_treeSettings[iSpecies] [iType].p_iBoolCodes = new short int[mp_treeSettings[iSpecies] [iType].iNumBools];
        for ( j = 0; j < mp_treeSettings[iSpecies] [iType].iNumBools; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = p_oPop->GetBoolDataCode( cData, iSpecies, iType );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractLiveTreeInfo" ;
            stcErr.sMoreInfo = "Unrecognized bool data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_treeSettings[iSpecies] [iType].p_iBoolCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
    } //end of for (i = 0; i < m_iNumTypeToSave; i++)
  } //end of try block
  catch ( modelErr & err )
  {
    delete[] cData;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] cData;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] cData;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clOutput::ExtractLiveTreeInfo" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// ExtractDeadTreeInfo()
/////////////////////////////////////////////////////////////////////////////
void clOutput::ExtractDeadTreeInfo( DOMDocument * p_oDoc )
{
  char * cData = NULL; //for extracting data from the file
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );;
    DOMNodeList * p_oSettingsList, //holds the list of tree info structures
    * p_oDataMemberList; //holds list of tree data members
    DOMNode * p_oDocNode;
    DOMElement * p_oTreeInfo, //one tree type's info
    * p_oDataMember; //one data member
    XMLCh *sVal;
    short int iNumSettings, //how many types we have output data for in the file
    iSpecies, iType, //species number
    iDeadReason,
    iDataMemberCode, //data member code for a label
    i, j, k; //loop counters

    //Get the tree population
    m_iNumSpecies = p_oPop->GetNumberOfSpecies();
    m_iNumTypes = p_oPop->GetNumberOfTypes();

    // Look for tree info
    //Search for all tree type info sets
    sVal = XMLString::transcode( "ou_deadTreeInfo" );
    p_oSettingsList = p_oDoc->getElementsByTagName( sVal );
    XMLString::release(&sVal);
    iNumSettings = p_oSettingsList->getLength();

    //Allocate the tree memory and initialize - set frequencies to -1 so we
    //can distinguish absence of data from the user setting 0
    mp_deadTreeSettings = new stcTreeOutputInfo ** [m_iNumSpecies];
    for ( i = 0; i < m_iNumSpecies; i++ ) {
      mp_deadTreeSettings[i] = new stcTreeOutputInfo*[m_iNumTypes];
      for ( j = 0; j < m_iNumTypes; j++ ) {
        mp_deadTreeSettings[i][j] = new stcTreeOutputInfo[remove_tree+1];
        for ( k = 0; k <= remove_tree; k++ ) {
          mp_deadTreeSettings[i][j][k].iSaveFreq = -1;
          mp_deadTreeSettings[i][j][k].iSumTimestep = -1;
          mp_deadTreeSettings[i][j][k].iNumInts = 0;
          mp_deadTreeSettings[i][j][k].iNumFloats = 0;
          mp_deadTreeSettings[i][j][k].iNumStrings = 0;
          mp_deadTreeSettings[i][j][k].iNumBools = 0;
          mp_deadTreeSettings[i][j][k].bSaveThisTimeStep = false;
          mp_deadTreeSettings[i][j][k].p_iIntCodes = NULL;
          mp_deadTreeSettings[i][j][k].p_iFloatCodes = NULL;
          mp_deadTreeSettings[i][j][k].p_iStringCodes = NULL;
          mp_deadTreeSettings[i][j][k].p_iBoolCodes = NULL;
        }
      }
    }

    //If we found nothing, exit with no error
    if ( iNumSettings == 0 ) return;

    //go through each type and break out its info
    for ( i = 0; i < iNumSettings; i++ )
    {
      p_oDocNode = p_oSettingsList->item( i );
      p_oTreeInfo = ( DOMElement * ) p_oDocNode;

      //get the type name - if something goes wrong throw an error
      sVal = XMLString::transcode( "type" );
      cData = XMLString::transcode( p_oTreeInfo->getAttributeNode( sVal )->getNodeValue() );
      XMLString::release(&sVal);
      if ( strcmp( "Seed", cData ) == 0 )
        iType = clTreePopulation::seed;
      else if ( strcmp( "Seedling", cData ) == 0 )
        iType = clTreePopulation::seedling;
      else if ( strcmp( "Sapling", cData ) == 0 )
        iType = clTreePopulation::sapling;
      else if ( strcmp( "Adult", cData ) == 0 )
        iType = clTreePopulation::adult;
      else if ( strcmp( "Snag", cData ) == 0 )
        iType = clTreePopulation::snag;
      else if ( strcmp( "Woody_debris", cData ) == 0 )
        iType = clTreePopulation::woody_debris;
      else
      {
        modelErr stcErr;
        stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "unrecognized type - ";
        stcErr.sMoreInfo += cData;
        throw( stcErr );
      }
      delete[] cData; cData = NULL;

      //Species
      sVal = XMLString::transcode( "species" );
      cData = XMLString::transcode( p_oTreeInfo->getAttributeNode( sVal )->getNodeValue() );
      XMLString::release(&sVal);
      iSpecies = p_oPop->TranslateSpeciesNameToCode( cData );
      if ( -1 == iSpecies )
      { //unrecognized species - throw error
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
        stcErr.sMoreInfo = "Unrecognized species - ";
        stcErr.sMoreInfo += cData;
        throw( stcErr );
      }
      delete[] cData; cData = NULL;

      //Dead reason code
      sVal = XMLString::transcode( "code" );
      cData = XMLString::transcode( p_oTreeInfo->getAttributeNode( sVal )->getNodeValue() );
      XMLString::release(&sVal);
      if ( strcmp( "harvest", cData ) == 0 )
        iDeadReason = harvest;
      else if ( strcmp( "natural", cData ) == 0 )
        iDeadReason = natural;
      else if ( strcmp( "disease", cData ) == 0 )
        iDeadReason = disease;
      else if ( strcmp( "fire", cData ) == 0 )
        iDeadReason = fire;
      else if ( strcmp( "insects", cData ) == 0 )
        iDeadReason = insects;
      else if ( strcmp( "storm", cData ) == 0 )
        iDeadReason = storm;
      else if ( strcmp( "remove_tree", cData ) == 0 )
        iDeadReason = remove_tree;
      else
      {
        modelErr stcErr;
        stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Unrecognized or unsupported dead reason code - ";
        stcErr.sMoreInfo += cData;
        throw( stcErr );
      }
      delete[] cData; cData = NULL;

      //Frequency data
      FillSingleValue( p_oTreeInfo, "ou_saveFreq", ( int * ) & mp_deadTreeSettings[iSpecies][iType][iDeadReason].iSaveFreq, true );

      //Make sure that the frequency is not negative - if so, throw error
      if ( mp_deadTreeSettings[iSpecies][iType][iDeadReason].iSaveFreq < 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
        stcErr.sMoreInfo = "Save frequency is negative";
        throw( stcErr );
      }

      //Number to save for each data member type
      //****************Ints*******************
      sVal = XMLString::transcode( "ou_int" );
      p_oDataMemberList = p_oTreeInfo->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumInts = p_oDataMemberList->getLength();
        //Declare the int array
        mp_deadTreeSettings[iSpecies][iType][iDeadReason].p_iIntCodes = new short int[mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumInts];
        for ( j = 0; j < mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumInts; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = p_oPop->GetIntDataCode( cData, iSpecies, iType );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
            stcErr.sMoreInfo = "Unrecognized int data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_deadTreeSettings[iSpecies][iType][iDeadReason].p_iIntCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Floats*******************
      sVal = XMLString::transcode( "ou_float" );
      p_oDataMemberList = p_oTreeInfo->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumFloats = p_oDataMemberList->getLength();
        //Declare the float array
        mp_deadTreeSettings[iSpecies][iType][iDeadReason].p_iFloatCodes = new short int[mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumFloats];
        for ( j = 0; j < mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumFloats; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = p_oPop->GetFloatDataCode( cData, iSpecies, iType );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
            stcErr.sMoreInfo = "Unrecognized float data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_deadTreeSettings[iSpecies][iType][iDeadReason].p_iFloatCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Chars*******************
      sVal = XMLString::transcode( "ou_char" );
      p_oDataMemberList = p_oTreeInfo->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumStrings = p_oDataMemberList->getLength();
        //Declare the int array
        mp_deadTreeSettings[iSpecies][iType][iDeadReason].p_iStringCodes = new short int[mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumStrings];
        for ( j = 0; j < mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumStrings; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = p_oPop->GetStringDataCode( cData, iSpecies, iType );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
            stcErr.sMoreInfo = "Unrecognized char data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_deadTreeSettings[iSpecies][iType][iDeadReason].p_iStringCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Bools*******************
      sVal = XMLString::transcode( "ou_bool" );
      p_oDataMemberList = p_oTreeInfo->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumBools = p_oDataMemberList->getLength();
        //Declare the int array
        mp_deadTreeSettings[iSpecies][iType][iDeadReason].p_iBoolCodes = new short int[mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumBools];
        for ( j = 0; j < mp_deadTreeSettings[iSpecies][iType][iDeadReason].iNumBools; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = p_oPop->GetBoolDataCode( cData, iSpecies, iType );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
            stcErr.sMoreInfo = "Unrecognized bool data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_deadTreeSettings[iSpecies][iType][iDeadReason].p_iBoolCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
    } //end of for (i = 0; i < m_iNumTypeToSave; i++)
  } //end of try block
  catch ( modelErr & err )
  {
    delete[] cData;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] cData;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] cData;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clOutput::ExtractDeadTreeInfo" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// ExtractGridInfo()
/////////////////////////////////////////////////////////////////////////////
void clOutput::ExtractGridInfo( DOMDocument * p_oDoc )
{
  char * cData = NULL; //for extracting text data
  try
  {
    DOMNodeList * p_oGridList, //list of grids
    * p_oDataMemberList; //list of data members
    DOMNode * p_oDocNode; //for retrieving data
    DOMElement * p_oGrid, //one grid element
    * p_oDataMember; //data member element
    XMLCh *sVal;
    short int iNumGrids, //number of grids found
    iDataMemberCode, //data member code
    i, j; //loop counters

    //Get all the grid info structures out of the file, if any
    sVal = XMLString::transcode( "ou_gridInfo" );
    p_oGridList = p_oDoc->getElementsByTagName( sVal );
    XMLString::release(&sVal);
    iNumGrids = p_oGridList->getLength();
    if ( 0 == iNumGrids ) return; //if nothing found exit without error

    //Allocate memory - set frequencies to -1 so we
    //can distinguish absence of data from the user setting 0
    m_iNumGridsToSave = iNumGrids;
    mp_gridSettings = new stcGridOutputInfo[m_iNumGridsToSave];
    for ( i = 0; i < m_iNumGridsToSave; i++ )
    {
      mp_gridSettings[i].iSaveFreq = -1;
      mp_gridSettings[i].iSumTimestep = 0;
      mp_gridSettings[i].iNumInts = 0;
      mp_gridSettings[i].iNumFloats = 0;
      mp_gridSettings[i].iNumStrings = 0;
      mp_gridSettings[i].iNumBools = 0;
      mp_gridSettings[i].p_iIntCodes = NULL;
      mp_gridSettings[i].p_iFloatCodes = NULL;
      mp_gridSettings[i].p_iStringCodes = NULL;
      mp_gridSettings[i].p_iBoolCodes = NULL;
      mp_gridSettings[i].iNumPackageInts = 0;
      mp_gridSettings[i].iNumPackageFloats = 0;
      mp_gridSettings[i].iNumPackageStrings = 0;
      mp_gridSettings[i].iNumPackageBools = 0;
      mp_gridSettings[i].p_iPackageIntCodes = NULL;
      mp_gridSettings[i].p_iPackageFloatCodes = NULL;
      mp_gridSettings[i].p_iPackageStringCodes = NULL;
      mp_gridSettings[i].p_iPackageBoolCodes = NULL;
    } //end of for (i = 0; i < m_iNumGridsToSave; i++)

    //Go through the grid objects in the file one at a time and extract the info
    for ( i = 0; i < m_iNumGridsToSave; i++ )
    {
      p_oDocNode = p_oGridList->item( i );
      p_oGrid = ( DOMElement * ) p_oDocNode;

      //First make sure that this is for a valid grid object
      sVal = XMLString::transcode( "gridName" );
      cData = XMLString::transcode( p_oGrid->getAttributeNode( sVal )->getNodeValue() );
      XMLString::release(&sVal);
      mp_gridSettings[i].p_oGridPointer = mp_oSimManager->GetGridObject( cData );
      if ( NULL == mp_gridSettings[i].p_oGridPointer )
      {
        modelErr stcErr;
        stcErr.sFunction = "clOutput::ExtractGridInfo" ;
        stcErr.iErrorCode = CANT_FIND_OBJECT;
        stcErr.sMoreInfo = cData;
        throw( stcErr );
      }
      delete[] cData; cData = NULL;

      //Get frequency
      FillSingleValue( p_oGrid, "ou_saveFreq", ( int * ) & mp_gridSettings[i].iSaveFreq, true );

      //Number to save for each data member type
      //****************Ints*******************
      sVal = XMLString::transcode( "ou_int" );
      p_oDataMemberList = p_oGrid->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_gridSettings[i].iNumInts = p_oDataMemberList->getLength();
        //Declare the int array
        mp_gridSettings[i].p_iIntCodes = new short int[mp_gridSettings[i].iNumInts];
        for ( j = 0; j < mp_gridSettings[i].iNumInts; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = mp_gridSettings[i].p_oGridPointer->GetIntDataCode( cData );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractGridInfo" ;
            stcErr.sMoreInfo = "Unrecognized int data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_gridSettings[i].p_iIntCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Floats*******************
      sVal = XMLString::transcode( "ou_float" );
      p_oDataMemberList = p_oGrid->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_gridSettings[i].iNumFloats = p_oDataMemberList->getLength();
        //Declare the float array
        mp_gridSettings[i].p_iFloatCodes = new short int[mp_gridSettings[i].iNumFloats];
        for ( j = 0; j < mp_gridSettings[i].iNumFloats; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = mp_gridSettings[i].p_oGridPointer->GetFloatDataCode( cData );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractGridInfo" ;
            stcErr.sMoreInfo = "Unrecognized float data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          delete[] cData; cData = NULL;
          mp_gridSettings[i].p_iFloatCodes[j] = iDataMemberCode;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Chars*******************
      sVal = XMLString::transcode( "ou_char" );
      p_oDataMemberList = p_oGrid->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_gridSettings[i].iNumStrings = p_oDataMemberList->getLength();
        //Declare the char array
        mp_gridSettings[i].p_iStringCodes = new short int[mp_gridSettings[i].iNumStrings];
        for ( j = 0; j < mp_gridSettings[i].iNumStrings; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = mp_gridSettings[i].p_oGridPointer->GetStringDataCode( cData );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractGridInfo" ;
            stcErr.sMoreInfo = "Unrecognized char data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          mp_gridSettings[i].p_iStringCodes[j] = iDataMemberCode;
          delete[] cData; cData = NULL;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Bools*******************
      sVal = XMLString::transcode( "ou_bool" );
      p_oDataMemberList = p_oGrid->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_gridSettings[i].iNumBools = p_oDataMemberList->getLength();
        //Declare the bool array
        mp_gridSettings[i].p_iBoolCodes = new short int[mp_gridSettings[i].iNumBools];
        for ( j = 0; j < mp_gridSettings[i].iNumBools; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = mp_gridSettings[i].p_oGridPointer->GetBoolDataCode( cData );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractGridInfo" ;
            stcErr.sMoreInfo = "Unrecognized bool data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          mp_gridSettings[i].p_iBoolCodes[j] = iDataMemberCode;
          delete[] cData; cData = NULL;
        }
      } //end of if (0 != p_oDataMemberList->getLength())


      //Packages
      //****************Package Ints*******************
      sVal = XMLString::transcode( "ou_packageInt" );
      p_oDataMemberList = p_oGrid->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_gridSettings[i].iNumPackageInts = p_oDataMemberList->getLength();
        //Declare the int array
        mp_gridSettings[i].p_iPackageIntCodes = new short int[mp_gridSettings[i].iNumPackageInts];
        for ( j = 0; j < mp_gridSettings[i].iNumPackageInts; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = mp_gridSettings[i].p_oGridPointer->GetPackageIntDataCode( cData );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractGridInfo" ;
            stcErr.sMoreInfo = "Unrecognized package int data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          mp_gridSettings[i].p_iPackageIntCodes[j] = iDataMemberCode;
          delete[] cData; cData = NULL;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Package Floats*******************
      sVal = XMLString::transcode( "ou_packageFloat" );
      p_oDataMemberList = p_oGrid->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_gridSettings[i].iNumPackageFloats = p_oDataMemberList->getLength();
        //Declare the float array
        mp_gridSettings[i].p_iPackageFloatCodes = new short int[mp_gridSettings[i].iNumPackageFloats];
        for ( j = 0; j < mp_gridSettings[i].iNumPackageFloats; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = mp_gridSettings[i].p_oGridPointer->GetPackageFloatDataCode( cData );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractGridInfo" ;
            stcErr.sMoreInfo = "Unrecognized package float data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          mp_gridSettings[i].p_iPackageFloatCodes[j] = iDataMemberCode;
          delete[] cData; cData = NULL;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Package Chars*******************
      sVal = XMLString::transcode( "ou_packageChar" );
      p_oDataMemberList = p_oGrid->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_gridSettings[i].iNumPackageStrings = p_oDataMemberList->getLength();
        //Declare the char array
        mp_gridSettings[i].p_iPackageStringCodes = new short int[mp_gridSettings[i].iNumPackageStrings];
        for ( j = 0; j < mp_gridSettings[i].iNumPackageStrings; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = mp_gridSettings[i].p_oGridPointer->GetPackageStringDataCode( cData );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractGridInfo" ;
            stcErr.sMoreInfo = "Unrecognized package char data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          mp_gridSettings[i].p_iPackageStringCodes[j] = iDataMemberCode;
          delete[] cData; cData = NULL;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
      //****************Package Bools*******************
      sVal = XMLString::transcode( "ou_packageBool" );
      p_oDataMemberList = p_oGrid->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      if ( 0 != p_oDataMemberList->getLength() )
      {
        mp_gridSettings[i].iNumPackageBools = p_oDataMemberList->getLength();
        //Declare the bool array
        mp_gridSettings[i].p_iPackageBoolCodes = new short int[mp_gridSettings[i].iNumPackageBools];
        for ( j = 0; j < mp_gridSettings[i].iNumPackageBools; j++ )
        {
          p_oDocNode = p_oDataMemberList->item( j );
          p_oDataMember = ( DOMElement * ) p_oDocNode;
          cData = XMLString::transcode( p_oDataMember->getFirstChild()->getNodeValue() );
          iDataMemberCode = mp_gridSettings[i].p_oGridPointer->GetPackageBoolDataCode( cData );
          if ( -1 == iDataMemberCode )
          { //unrecognized data code - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clOutput::ExtractGridInfo" ;
            stcErr.sMoreInfo = "Unrecognized package bool data member label - ";
            stcErr.sMoreInfo += cData;
            throw( stcErr );
          }
          mp_gridSettings[i].p_iPackageBoolCodes[j] = iDataMemberCode;
          delete[] cData; cData = NULL;
        }
      } //end of if (0 != p_oDataMemberList->getLength())
    } //end of (i = 0; i < m_iNumGridsToSave; i++)
  } //end of try block
  catch ( modelErr & err )
  {
    delete[] cData;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] cData;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] cData;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clOutput::ExtractGridInfo" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ExtractSubplotInfo()
////////////////////////////////////////////////////////////////////////////
void clOutput::ExtractSubplotInfo( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    DOMNodeList * p_oGridCellList, //for getting the list of grid cells to save for
    * p_oSubplotList; //list of subplots
    DOMNode * p_oDocNode; //for retrieving data
    DOMElement * p_oSubplot, //one subplot element
    * p_oElement; //generic element object
    XMLCh *sVal;
    char *cData;
    float fTemp;
    int iX, iY; //grid cell X and Y coordinate numbers
    short int iNumSubplots, //number of subplots found
    iNumCells, //number of grid cells found for one subplot
    i, j; //loop counters
    bool bTest;

    m_iNumSubplotsToSave = 0;

    //If there was no tree data saved, we will ignore subplots anyway, so
    //exit
    bTest = false;
    for ( i = 0; i < m_iNumSpecies; i++ )
      for ( j = 0; j < m_iNumTypes; j++ )
        if ( mp_treeSettings[i][j].iSaveFreq > -1 )
        {
          bTest = true; break;
        }
    if ( false == bTest ) return;

    //Check to see if there is user-specified grid cell size information - we've
    //already collected the default info in case there isn't
    p_oElement = p_oDoc->getDocumentElement();
    fTemp = -1;
    FillSingleValue( p_oElement, "ou_subplotXLength", &fTemp, false );
    if (fTemp > 0) {
      m_fXCellLength = fTemp;
      m_iNumXCells = (int)ceil( mp_oSimManager->GetPlotObject()->GetXPlotLength() / m_fXCellLength );
    }
    fTemp = -1;
    FillSingleValue( p_oElement, "ou_subplotYLength", &fTemp, false );
    if (fTemp > 0) {
      m_fYCellLength = fTemp;
      m_iNumYCells = (int)ceil( mp_oSimManager->GetPlotObject()->GetYPlotLength() / m_fYCellLength );
    }

    //Get all the subplot info structures out of the file, if any
    sVal = XMLString::transcode( "ou_subplot" );
    p_oSubplotList = p_oDoc->getElementsByTagName( sVal );
    XMLString::release(&sVal);
    iNumSubplots = p_oSubplotList->getLength();
    if ( 0 == iNumSubplots ) return; //if nothing found exit without error

    //Allocate memory
    m_iNumSubplotsToSave = iNumSubplots;
    mp_subplots = new stcSubplotInfo[m_iNumSubplotsToSave];
    for ( i = 0; i < m_iNumSubplotsToSave; i++ )
    {
      mp_subplots[i].p_bUseCell = new bool*[m_iNumXCells];
      for (iX = 0; iX < m_iNumXCells; iX++) {
        mp_subplots[i].p_bUseCell[iX] = new bool[m_iNumYCells];
        for (iY = 0; iY < m_iNumYCells; iY++) {
          mp_subplots[i].p_bUseCell[iX][iY] = false;
        }
      }
    } //end of for (i = 0; i < m_iNumSubplotsToSave; i++)

    //Go through the subplots in the file one at a time and extract the info
    for ( i = 0; i < m_iNumSubplotsToSave; i++ )
    {
      p_oDocNode = p_oSubplotList->item( i );
      p_oSubplot = ( DOMElement * ) p_oDocNode;

      //Get subplot name
      FillSingleValue( p_oSubplot, "ou_subplotName", &mp_subplots[i].sSubplotName, true );

      //Get grid coords
      p_oGridCellList = p_oSubplot->getElementsByTagName( XMLString::transcode( "pointSet" ) );
      if ( 0 == p_oGridCellList->getLength() )
      {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clOutput::ExtractSubplotInfo" ;
        stcErr.sMoreInfo = "pointSet";
        throw( stcErr );
      }
      p_oDocNode = p_oGridCellList->item( 0 );
      p_oElement = ( DOMElement * ) p_oDocNode;
      sVal = XMLString::transcode( "po_point" );
      p_oGridCellList = p_oElement->getElementsByTagName( sVal );
      XMLString::release(&sVal);
      iNumCells = p_oGridCellList->getLength();
      if ( 0 == iNumCells )
      {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clOutput::ExtractSubplotInfo" ;
        stcErr.sMoreInfo = "po_point";
        throw( stcErr );
      }

      //Set each cell in the list to "true" in this subplot's cell array
      for ( j = 0; j < iNumCells; j++ )
      {
        p_oDocNode = p_oGridCellList->item( j );
        p_oElement = ( DOMElement * ) p_oDocNode;
        sVal = XMLString::transcode( "x" );
        cData =  XMLString::transcode( p_oElement->getAttributeNode( sVal )->getNodeValue() );
        iX = atoi( cData );
        XMLString::release(&sVal);
        delete[] cData; cData = NULL;
        sVal = XMLString::transcode( "y" );
        cData = XMLString::transcode( p_oElement->getAttributeNode( sVal )->getNodeValue() );
        iY = atoi( cData );
        delete[] cData; cData = NULL;
        XMLString::release(&sVal);
        //Make sure that the values are within the plot
        if ( iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0 )
        {
          modelErr stcErr;
          stcErr.sFunction = "clOutput::ExtractSubplotInfo" ;
          std::stringstream s;
          s << "Point value outside plot - " << iX << ", " << iY;
          stcErr.sMoreInfo = s.str();
          stcErr.iErrorCode = BAD_DATA;
        }

        mp_subplots[i].p_bUseCell[iX][iY] = true;

      } //end of for (j = 0; j < iNumCells; j++)
    } //end of for (i = 0; i < m_iNumSubplotsToSave; i++)
  } //end of try block
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
    stcErr.sFunction = "clOutput::ExtractSubplotInfo" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// WriteDetailedOutputHeader()
/////////////////////////////////////////////////////////////////////////////
void clOutput::WriteDetailedOutputHeader()
{
  const int BUFFER_SIZE = 128;

  FILE ** p_outFile = NULL, //array of detailed output file streams, one for each
      //subplot plus one for the whole plot
      * inFile; //parameter file
  string * p_sFilename = NULL,
      sTemp,
      sPath = mp_oSimManager->GetAppPath();
  char cFileCode[9], //holds the file code
  //** p_cFilename = NULL,
  cBuf[BUFFER_SIZE]; //for transferring between files
  char * cPos;
  int iNumRead, //for testing for end-of-file
  iStrLen,
  iNumOutFiles = m_iNumSubplotsToSave + 1,
  iPos, i;
  bool bReading; //whether or not we found the <paramFile> tag

  try
  {

    if ( m_sFileRoot.length() == 0 ) return;

    //Declare the array of out files and filenames
    p_outFile = new FILE*[iNumOutFiles];
    p_sFilename = new string[iNumOutFiles];

    //Tack on the file extension to the root for the whole-plot file
    p_sFilename[0] = m_sFileRoot + DETAILED_OUTPUT_FILE_EXT;

    //There is a possibility that the detailed output filename and the
    //parameter filename are the same - especially if a detailed output file
    //was loaded as a parameter file.  In this case, just skip out
    sTemp = mp_oSimManager->GetParFilename();
    if (p_sFilename[0] == sTemp )
      return;

    //Open detailed output file
    p_outFile[0] = fopen( p_sFilename[0].c_str(), "wt" ); //overwrite existing

    //If we can't open the file throw an error
    if ( NULL == p_outFile[0] )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sFunction = "clOutput::WriteDetailedOutputHeader";
      std::stringstream s;
      s << "Couldn't save the output file \"" << p_sFilename[0]
                                                             << "\". Check that the path exists.";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }

    //Open the subplot streams
    for (i = 1; i < iNumOutFiles; i++) {
      //Tack on the file extension to the root for the whole-plot file
      p_sFilename[i] = m_sFileRoot + "_" + mp_subplots[i - 1].sSubplotName
          + DETAILED_OUTPUT_FILE_EXT;
      p_outFile[i] = fopen( p_sFilename[i].c_str(), "wt" ); //overwrite existing
      //If we can't open the file throw an error
      if ( NULL == p_outFile[i] )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_FILE;
        stcErr.sFunction = "clOutput::WriteDetailedOutputHeader" ;
        std::stringstream s;
        s << "Couldn't open file \"" << p_sFilename[i] << "\".";
        stcErr.sMoreInfo = s.str();
        throw( stcErr );
      }
    }

    //Open parameter file
    inFile = fopen( mp_oSimManager->GetParFilename().c_str(), "rt" );

    //If we can't open the file throw an error
    if ( NULL == inFile )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sFunction = "clOutput::WriteDetailedOutputHeader" ;
      std::stringstream s;
      s << "Couldn't open file \"" << mp_oSimManager->GetParFilename() << "\".";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }

    //Get the file code string
    AssembleFileCode( detailed_output, DETAILED_OUTPUT_FILE_VERSION, cFileCode );

    //Write the detailed output opening tag
    sprintf( cBuf, "%s%s%s", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><rundata fileCode=\"", cFileCode, "\">" );
    for (i = 0; i < iNumOutFiles; i++) {
      fwrite( cBuf, strlen( cBuf ), 1, p_outFile[i] );
    }

    //Now start the data feed from parameter file to detailed output file.  We
    //want to start at the <paramFile> tag.
    bReading = false;
    iNumRead = BUFFER_SIZE - 1;

    while ( iNumRead == BUFFER_SIZE - 1 )
    { //this is our end-of-file test
      iNumRead = fread( cBuf, sizeof( char ), BUFFER_SIZE - 1, inFile );
      cBuf[iNumRead] = '\0';

      if ( !bReading )
      {

        //We haven't found the parameter file opener tag yet - so find it first
        cPos = strstr( cBuf, "<paramFile" );
        if ( cPos != NULL )
        {

          //It's in this string - find the index in the string where cPos is
          iPos = -1;
          iStrLen = strlen( cBuf );
          for ( i = 0; i < iStrLen; i++ )
          {
            if ( cPos == cBuf + i )
            {
              iPos = i;
              break;
            }
          }

          if ( iPos > -1 )
          {
            for (i = 0; i < iNumOutFiles; i++) {
              fwrite( cPos, strlen( cBuf ) - iPos, 1, p_outFile[i] );
            }
            bReading = true;
          }
        }

      }
      else
      {

        //We've already found the parameter string - so it's a straight
        //read/write
        for (i = 0; i < iNumOutFiles; i++) {
          fwrite( cBuf, strlen( cBuf ), 1, p_outFile[i] );
        }
      }
    }


    //Write detailed output closing tag

    strcpy( cBuf, "</rundata>" );
    for (i = 0; i < iNumOutFiles; i++) {
      fwrite( cBuf, strlen( cBuf ), 1, p_outFile[i] );
    }

    fclose( inFile );

    for (i = 0; i < iNumOutFiles; i++) {
      fclose( p_outFile[i] );
    }

    //Zip and tar the whole-plot file
    ZipFile( p_sFilename[0], sPath );
    p_sFilename[0] += GZIP_EXT;
    sTemp = m_sTarball;
    AddFileToNewTarball( sTemp, p_sFilename[0], sPath );
    DeleteThisFile( p_sFilename[0] ); //it's copied to tarball - so remove original

    //Repeat with the subplot files
    for (i = 1; i < iNumOutFiles; i++) {
      ZipFile( p_sFilename[i], sPath );
      p_sFilename[i] += GZIP_EXT;
      AddFileToNewTarball( mp_subplots[i - 1].sSubplotTarball, p_sFilename[i], sPath );
      DeleteThisFile( p_sFilename[i] ); //it's copied to tarball - so remove original
    }

    delete[] p_outFile;
    delete[] p_sFilename;

  } //end of try block
  catch ( modelErr & err )
  {
    delete[] p_sFilename;
    delete[] p_outFile;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_sFilename;
    delete[] p_outFile;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clOutput::WriteDetailedOutputHeader" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clOutput::Action()
{
  try
  {
    if ( m_sFileRoot.length() == 0 ) return;

    string *p_sTimestepFilename,
    sPath = mp_oSimManager->GetAppPath();
    //Get the filename of the detailed output timestep file
    int iTimestep = mp_oSimManager->GetCurrentTimestep(),
        iNumOutputFiles = m_iNumSubplotsToSave + 1,
        i;

    p_sTimestepFilename = new string[iNumOutputFiles];
    for (i = 0; i < iNumOutputFiles; i++) {
      p_sTimestepFilename[i] = GetTimestepFilename( iTimestep, i - 1);
    }

    //Write the header for each file
    for (i = 0; i < iNumOutputFiles; i++) {
      WriteTimestepHeader( p_sTimestepFilename[i] );
    }

    //Write the tree data - WriteTreeData will take care of file juggling
    WriteTreeData( p_sTimestepFilename );

    //Write grid data to the whole plot file only
    WriteGridData( p_sTimestepFilename[0] );

    //Write the footer for each file
    for (i = 0; i < iNumOutputFiles; i++) {
      WriteTimestepFooter( p_sTimestepFilename[i] );
    }

    //Zip each file and add it to the tarball
    ZipFile( p_sTimestepFilename[0], sPath );
    p_sTimestepFilename[0] += GZIP_EXT; //'cause that's the new filename after
    AddFileToTarball( m_sTarball, p_sTimestepFilename[0], sPath );
    DeleteThisFile( p_sTimestepFilename[0] );

    //Repeat with the subplot files
    for (i = 1; i < iNumOutputFiles; i++) {
      ZipFile( p_sTimestepFilename[i], sPath );
      p_sTimestepFilename[i] += GZIP_EXT;
      AddFileToTarball( mp_subplots[i - 1].sSubplotTarball, p_sTimestepFilename[i], sPath );
      DeleteThisFile( p_sTimestepFilename[i] ); //it's copied to tarball - so remove original
    }

    delete[] p_sTimestepFilename;

  } //end of try block
  catch ( modelErr & err )
  {
    throw( err );
  } //throw without clearing this object
  catch ( modelMsg & msg )
  {
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clOutput::Action" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// WriteTreeData()
/////////////////////////////////////////////////////////////////////////////
void clOutput::WriteTreeData( string * p_sFilename )
{
  clTreePopulation * p_oTrees = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" ); //pointer to tree population
  clGhostTreePopulation *p_oGhosts = ( clGhostTreePopulation * ) mp_oSimManager->GetPopulationObject( "GhostTreePopulation" );
  clTreeSearch * p_oTreesToWrite; //pointer to search results
  clTree * p_oTree;
  clDeadTree *p_oGhost;
  FILE ** p_out = NULL; //out file to write to
  char cTemp[100], //temp for string formatting
  ** p_cBuf = NULL; //file writing buffer for each output file
  float fX, fY; //tree coordinates
  int iTimestep = mp_oSimManager->GetCurrentTimestep(), //current timestep
      iNumTimesteps = mp_oSimManager->GetNumberOfTimesteps(), //total number of timesteps
      iX, iY, //tree grid cell
      iNumOutFiles = m_iNumSubplotsToSave + 1,
      iSp, iTp, iBuf, iCd, j; //loop counters
  bool bTemp;

  try
  {

    //Open the out file(s)
    if ( m_sFileRoot.length() == 0 ) return;

    p_out = new FILE*[iNumOutFiles];
    p_cBuf = new char*[iNumOutFiles];
    for (iBuf = 0; iBuf < iNumOutFiles; iBuf++) {
      p_out[iBuf] = fopen( p_sFilename[iBuf].c_str(), "at" );
      //If the file could not be opened - throw error
      if ( NULL == p_out[iBuf] )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_FILE;
        stcErr.sFunction = "clOutputCFile::WriteTreeData" ;
        std::stringstream s;
        s << "Could not open output file \"" << p_sFilename[iBuf] << "\"";
        stcErr.sMoreInfo = s.str();
        throw( stcErr );
      }

      p_cBuf[iBuf] = new char[BUFFER_SIZE + 1];
      //Clear the buffer
      p_cBuf[iBuf][0] = '\0';
    }

    //Figure out which species/types to write this timestep
    bTemp = false;
    for ( iSp = 0; iSp < m_iNumSpecies; iSp++ ) {
      for ( iTp = 0; iTp < m_iNumTypes; iTp++ ) {
        if ( mp_treeSettings[iSp][iTp].iSaveFreq > -1 ) {
          mp_treeSettings[iSp][iTp].iSumTimestep++;
          if ( 0 == iTimestep || iNumTimesteps == iTimestep ||
              ( mp_treeSettings[iSp][iTp].iSaveFreq ==
                  mp_treeSettings[iSp][iTp].iSumTimestep ) ) {
            mp_treeSettings[iSp][iTp].bSaveThisTimeStep = true;
            bTemp = true;
            //Reset the counter
            mp_treeSettings[iSp][iTp].iSumTimestep = 0;
          } else mp_treeSettings[iSp][iTp].bSaveThisTimeStep = false;
        }
        for (iCd = 0; iCd <= remove_tree; iCd++) {
          if ( mp_deadTreeSettings[iSp][iTp][iCd].iSaveFreq > -1 ) {
            mp_deadTreeSettings[iSp][iTp][iCd].iSumTimestep++;
            if ( 0 == iTimestep || iNumTimesteps == iTimestep ||
                ( mp_deadTreeSettings[iSp][iTp][iCd].iSaveFreq ==
                    mp_deadTreeSettings[iSp][iTp][iCd].iSumTimestep ) ) {
              mp_deadTreeSettings[iSp][iTp][iCd].bSaveThisTimeStep = true;
              bTemp = true;
              //Reset the counter
              mp_deadTreeSettings[iSp][iTp][iCd].iSumTimestep = 0;
            }
            else mp_deadTreeSettings[iSp][iTp][iCd].bSaveThisTimeStep = false;
          }
        }
      }
    }


    //******************************
    // Write the tree map header from the master settings
    //******************************
    if ( false == bTemp ) {
      for (iBuf = 0; iBuf < iNumOutFiles; iBuf++) {
        fclose( p_out[iBuf] );
      }
      return;
    }

    for (iBuf = 0; iBuf < iNumOutFiles; iBuf++) {
      strcpy( p_cBuf[iBuf], "<tr_treemap>" );

      //Write a species list
      strcpy( cTemp, "<tm_speciesList>" );
      AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
      for ( j = 0; j < m_iNumSpecies; j++ ) {
        sprintf( cTemp, "%s%s%s", "<tm_species speciesName=\"",
            p_oTrees->TranslateSpeciesCodeToName(j).c_str(), "\"/>" );
        AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
      }
      strcpy( cTemp, "</tm_speciesList>" );
      AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );

      //Tree settings for each species/type combo
      for ( iSp = 0; iSp < m_iNumSpecies; iSp++ ) {
        for ( iTp = 0; iTp < m_iNumTypes; iTp++ ) {
          if ( mp_masterTreeSettings[iSp][iTp].bSaveThisTimeStep ) {
            //Tree settings opening tag
            sprintf( cTemp, "%s%s%s%d%s", "<tm_treeSettings sp=\"",
                p_oTrees->TranslateSpeciesCodeToName(iSp).c_str(),
                "\" tp=\"", iTp, "\">" );
            AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );

            //Data member labels and codes
            if ( mp_masterTreeSettings[iSp][iTp].iNumInts > 0 ) {
              strcpy( cTemp, "<tm_intCodes>" );
              AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
              for ( j = 0; j < mp_masterTreeSettings[iSp][iTp].iNumInts; j++ ) {
                sprintf( cTemp, "%s%s%s%d%s", "<tm_intCode label=\"",
                    p_oTrees->GetIntDataLabel(mp_masterTreeSettings[iSp][iTp].p_iIntCodes[j], iSp, iTp ).c_str(),
                    "\">", mp_masterTreeSettings[iSp][iTp].p_iIntCodes[j], "</tm_intCode>" );
                AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
              }
              strcpy( cTemp, "</tm_intCodes>" );
              AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
            } //end of if (mp_masterTreeSettings[i].iNumInts > 0)



            if ( mp_masterTreeSettings[iSp][iTp].iNumFloats > 0 ) { //floats
              strcpy( cTemp, "<tm_floatCodes>" );
              AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
              for ( j = 0; j < mp_masterTreeSettings[iSp][iTp].iNumFloats; j++ ) {
                sprintf( cTemp, "%s%s%s%d%s", "<tm_floatCode label=\"",
                    p_oTrees->GetFloatDataLabel( mp_masterTreeSettings[iSp][iTp].p_iFloatCodes[j], iSp, iTp ).c_str(),
                    "\">", mp_masterTreeSettings[iSp][iTp].p_iFloatCodes[j], "</tm_floatCode>" );
                AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
              }
              strcpy( cTemp, "</tm_floatCodes>" );
              AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
            } //end of if (mp_masterTreeSettings[iSp][iTp].iNumFloats > 0)



            if ( mp_masterTreeSettings[iSp][iTp].iNumStrings > 0 ) { //chars
              strcpy( cTemp, "<tm_charCodes>" );
              AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
              for ( j = 0; j < mp_masterTreeSettings[iSp][iTp].iNumStrings; j++ ) {
                sprintf( cTemp, "%s%s%s%d%s", "<tm_charCode label=\"",
                    p_oTrees->GetStringDataLabel( mp_masterTreeSettings[iSp][iTp].p_iStringCodes[j], iSp, iTp ).c_str(),
                    "\">", mp_masterTreeSettings[iSp][iTp].p_iStringCodes[j], "</tm_charCode>" );
                AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
              }
              strcpy( cTemp, "</tm_charCodes>" );
              AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
            } //end of if (mp_masterTreeSettings[iSp][iTp].iNumChars > 0)



            if ( mp_masterTreeSettings[iSp][iTp].iNumBools > 0 ) { //bools
              strcpy( cTemp, "<tm_boolCodes>" );
              AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
              for ( j = 0; j < mp_masterTreeSettings[iSp][iTp].iNumBools; j++ ) {
                sprintf( cTemp, "%s%s%s%d%s", "<tm_boolCode label=\"",
                    p_oTrees->GetBoolDataLabel( mp_masterTreeSettings[iSp][iTp].p_iBoolCodes[j], iSp, iTp ).c_str(),
                    "\">", mp_masterTreeSettings[iSp][iTp].p_iBoolCodes[j], "</tm_boolCode>" );
                AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
              }
              strcpy( cTemp, "</tm_boolCodes>" );
              AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
            } //end of if (mp_masterTreeSettings[iSp][iTp].iNumBools > 0)

            //Tree settings closing tag

            strcpy( cTemp, "</tm_treeSettings>" );
            AddToBuffer( p_cBuf[iBuf], cTemp, p_out[iBuf], BUFFER_SIZE );
          }
        }
      }
    }

    //******************************
    // Tree data saving - go through all trees and write the ones being saved
    //******************************
    if (iNumOutFiles == 1) { //No subplots!
      p_oTreesToWrite = p_oTrees->Find( "all" );
      p_oTree = p_oTreesToWrite->NextTree();
      while ( p_oTree ) {
        iSp = p_oTree->GetSpecies();
        iTp = p_oTree->GetType();
        if ( mp_treeSettings[iSp][iTp].bSaveThisTimeStep )
          WriteTree(p_oTree, p_cBuf[0], cTemp, p_out[0]);
        p_oTree = p_oTreesToWrite->NextTree();
      }


    } else { //Subplots!

      //Find the trees
      p_oTreesToWrite = p_oTrees->Find( "all" );
      p_oTree = p_oTreesToWrite->NextTree();
      while ( p_oTree ) {
        iSp = p_oTree->GetSpecies();
        iTp = p_oTree->GetType();

        if ( mp_treeSettings[iSp][iTp].bSaveThisTimeStep ) {
          WriteTree(p_oTree, p_cBuf[0], cTemp, p_out[0]);
          //Get the tree's grid cell counters
          p_oTree->GetValue(p_oTrees->GetXCode(iSp, iTp), &fX);
          p_oTree->GetValue(p_oTrees->GetYCode(iSp, iTp), &fY);
          iX = (int)floor( fX / m_fXCellLength );
          iY = (int)floor( fY / m_fYCellLength );

          //Now subplots, if there are any
          for (iBuf = 1; iBuf < iNumOutFiles; iBuf++) {
            if (mp_subplots[iBuf-1].p_bUseCell[iX][iY]) {
              WriteTree(p_oTree, p_cBuf[iBuf], cTemp, p_out[iBuf]);
            }
          }
        }

        p_oTree = p_oTreesToWrite->NextTree();
      }
    }

    //Write the dead trees
    bTemp = false;
    for ( iSp = 0; iSp < m_iNumSpecies; iSp++ ) {
      for ( iTp = 0; iTp < m_iNumTypes; iTp++ ) {
        for (iCd = 0; iCd <= remove_tree; iCd++) {
          if ( mp_deadTreeSettings[iSp][iTp][iCd].bSaveThisTimeStep) {
            bTemp = true;
            break;
          }
        }
      }
    }
    if (bTemp) {
      p_oGhost = p_oGhosts->GetFirstTree();
      while ( p_oGhost ) {
        iSp = p_oGhost->GetSpecies();
        iTp = p_oGhost->GetType();
        iCd = p_oGhost->GetDeadReasonCode();
        if ( mp_deadTreeSettings[iSp][iTp][iCd].bSaveThisTimeStep )
          WriteGhost(p_oGhost, p_cBuf[0], cTemp, p_out[0]);
        p_oGhost = p_oGhost->GetNext();
      }
    }

    //Write the end tag for the tree map
    //Flush all buffers one last time and close them
    for (iBuf = 0; iBuf < iNumOutFiles; iBuf++) {
      fwrite( p_cBuf[iBuf], strlen( p_cBuf[iBuf] ), 1, p_out[iBuf] );
      strcpy( p_cBuf[iBuf], "</tr_treemap>" );
      fwrite( p_cBuf[iBuf], strlen( p_cBuf[iBuf] ), 1, p_out[iBuf] );
      fclose( p_out[iBuf] );
    }

    delete[] p_out;
    for (iBuf = 0; iBuf < iNumOutFiles; iBuf++) {
      delete[] p_cBuf[iBuf];
    }
    delete[] p_cBuf;

  } //end of try block
  catch ( modelErr & err )
  {
    delete[] p_out;
    for (iBuf = 0; iBuf < iNumOutFiles; iBuf++) {
      delete[] p_cBuf[iBuf];
    }
    delete[] p_cBuf;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_out;
    for (iBuf = 0; iBuf < iNumOutFiles; iBuf++) {
      delete[] p_cBuf[iBuf];
    }
    delete[] p_cBuf;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clOutput::WriteTreeData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// WriteTimestepHeader()
/////////////////////////////////////////////////////////////////////////////
void clOutput::WriteTimestepHeader( string sFilename )
{
  try
  {
    FILE * out; //file writing stream
    char cTemp[50], cFileCode[9]; //file code
    int iFileVersion = 1; //for the filecode

    //If the filename is empty return
    if ( m_sFileRoot.length() == 0 ) return;

    //Open the out file
    out = fopen( sFilename.c_str(), "wt" );

    //If the file could not be opened - throw error
    if ( NULL == out )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sFunction = "clOutput::WriteTimestepHeader" ;
      std::stringstream s;
      s << "Could not open output file \"" << sFilename << "\"";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }

    AssembleFileCode( detailed_output_timestep, iFileVersion, cFileCode );
    //***************************
    //Write the header of the file
    //***************************
    //Root element
    strcpy( cTemp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" );
    fwrite( cTemp, strlen( cTemp ), 1, out );

    //  << "<!DOCTYPE timestepRundata SYSTEM \"xml\\RundataTimestep.dtd\">\n"
    sprintf( cTemp, "%s%s%s", "<timestepRundata fileCode=\"", cFileCode, "\">" );
    fwrite( cTemp, strlen( cTemp ), 1, out );

    //Timestep
    sprintf( cTemp, "%s%d%s", "<rt_timestep>", mp_oSimManager->GetCurrentTimestep(), "</rt_timestep>" );
    fwrite( cTemp, strlen( cTemp ), 1, out );

    fclose( out );
  } //end of try block
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
    stcErr.sFunction = "clOutput::WriteTimestepHeader" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// WriteTimestepFooter()
/////////////////////////////////////////////////////////////////////////////
void clOutput::WriteTimestepFooter( string sFilename )
{
  try
  {
    char cCloser[] = "</timestepRundata>";
    FILE * out; //file writing stream

    //If the filename is empty return
    if ( sFilename.length() == 0 ) return;

    //Open the out file
    out = fopen( sFilename.c_str(), "at" );

    //If we can't open the file throw an error
    if ( NULL == out )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sFunction = "clOutput::WriteTimestepFooter" ;
      std::stringstream s;
      s << "Couldn't open file \"" << sFilename << "\".";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }

    fwrite( cCloser, strlen( cCloser ), 1, out );
    fclose( out );

  } //end of try block
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
    stcErr.sFunction = "clOutput::WriteTimestepFooter" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// WriteGridData()
/////////////////////////////////////////////////////////////////////////////
void clOutput::WriteGridData( string sFilename )
{
  try
  {
    std::string sTemp;
    char cBuf[BUFFER_SIZE + 1], //buffer for file writing
    cTemp[100]; //for formatting data into buffer
    clPlot * p_oPlot; //plot object - for grid lengths
    clPackage * p_oPackage; //for writing the grid's packages
    FILE * out; //file to write grid file
    float fPlotLenX, fPlotLenY, //plot dimensions
    fTemp; //for getting float values
    int iTimestep, //current timestep
    iNumTimesteps, //total number of timesteps
    iNumXCells, iNumYCells, //number of X and Y cells in the grid
    iNumDataMembers, //number of a kind of data member
    iTemp, //for getting int values
    i, j, iX, iY; //loop counters
    bool bSavePackages, //for whether or not packages are to be saved
    bTemp; //for getting bool values

    //If the filename is empty return
    if (sFilename.length() == 0 ) return;

    //Open the out file
    out = fopen( sFilename.c_str(), "at" );

    //If the file could not be opened - throw error
    if ( NULL == out )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sFunction = "clOutput::WriteGridData" ;
      std::stringstream s;
      s << "Could not open output file \"" << sFilename << "\"";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }

    //Get the timestep data
    iTimestep = mp_oSimManager->GetCurrentTimestep();
    iNumTimesteps = mp_oSimManager->GetNumberOfTimesteps();

    //Get the plot dimensions
    p_oPlot = mp_oSimManager->GetPlotObject();
    fPlotLenX = p_oPlot->GetXPlotLength();
    fPlotLenY = p_oPlot->GetYPlotLength();

    //Loop through the grids to be written
    for ( i = 0; i < m_iNumGridsToSave; i++ )
    {
      //Clear the buffer
      cBuf[0] = '\0';

      //Is this a saving timestep?
      //Increment the timestep counter and compare to frequency - always
      //write if this is timestep 0 or the last timestep
      mp_gridSettings[i].iSumTimestep++;
      if ( 0 == iTimestep || iNumTimesteps == iTimestep
          || ( mp_gridSettings[i].iSaveFreq == mp_gridSettings[i].iSumTimestep ) )
      {
        //Reset counter
        mp_gridSettings[i].iSumTimestep = 0;

        //Figure out if there are any package data members to save
        if ( mp_gridSettings[i].iNumPackageInts > 0 || mp_gridSettings[i].iNumPackageFloats > 0
            || mp_gridSettings[i].iNumPackageStrings > 0 || mp_gridSettings[i].iNumPackageBools > 0 )
          bSavePackages = true;
        else
          bSavePackages = false;

        //********************************
        // Grid header
        //********************************

        //Open tag for grid
        sprintf( cTemp, "%s%s%s", "<grid gridName=\"", mp_gridSettings[i].p_oGridPointer->GetName().c_str(), "\">" );
        AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

        //Data labels - write all even if all are not being saved
        //Ints
        iNumDataMembers = mp_gridSettings[i].p_oGridPointer->GetNumberIntDataMembers();
        if ( iNumDataMembers > 0 )
        {
          strcpy( cTemp, "<ma_intCodes>" );
          AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

          for ( j = 0; j < iNumDataMembers; j++ )
          {
            sprintf( cTemp, "%s%s%s%d%s", "<ma_intCode label=\"", mp_gridSettings[i].p_oGridPointer->GetIntDataLabel( j ).c_str(),
                "\">", j, "</ma_intCode>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          }

          strcpy( cTemp, "</ma_intCodes>" );
          AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
        } //end of if (iNumDataMembers > 0)

        //floats
        iNumDataMembers = mp_gridSettings[i].p_oGridPointer->GetNumberFloatDataMembers();
        if ( iNumDataMembers > 0 )
        {
          strcpy( cTemp, "<ma_floatCodes>" );
          AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

          for ( j = 0; j < iNumDataMembers; j++ )
          {
            sprintf( cTemp, "%s%s%s%d%s", "<ma_floatCode label=\"", mp_gridSettings[i].p_oGridPointer->GetFloatDataLabel( j ).c_str(),
                "\">", j, "</ma_floatCode>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          }

          strcpy( cTemp, "</ma_floatCodes>" );
          AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
        } //end of if (iNumDataMembers > 0)

        //chars
        iNumDataMembers = mp_gridSettings[i].p_oGridPointer->GetNumberStringDataMembers();
        if ( iNumDataMembers > 0 )
        {
          strcpy( cTemp, "<ma_charCodes>" );
          AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

          for ( j = 0; j < iNumDataMembers; j++ )
          {
            sprintf( cTemp, "%s%s%s%d%s", "<ma_charCode label=\"", mp_gridSettings[i].p_oGridPointer->GetStringDataLabel( j ).c_str(),
                "\">", j, "</ma_charCode>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          }

          strcpy( cTemp, "</ma_charCodes>" );
          AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
        } //end of if (iNumDataMembers > 0)

        //bools
        iNumDataMembers = mp_gridSettings[i].p_oGridPointer->GetNumberBoolDataMembers();
        if ( iNumDataMembers > 0 )
        {
          strcpy( cTemp, "<ma_boolCodes>" );
          AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

          for ( j = 0; j < iNumDataMembers; j++ )
          {
            sprintf( cTemp, "%s%s%s%d%s", "<ma_boolCode label=\"", mp_gridSettings[i].p_oGridPointer->GetBoolDataLabel( j ).c_str(),
                "\">", j, "</ma_boolCode>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          }

          strcpy( cTemp, "</ma_boolCodes>" );
          AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
        } //end of if (iNumDataMembers > 0)

        //Package data labels
        if ( mp_gridSettings[i].p_oGridPointer->GetPackageDataChanged() )
        {
          //Ints
          iNumDataMembers = mp_gridSettings[i].p_oGridPointer->GetNumberIntPackageDataMembers();
          if ( iNumDataMembers > 0 )
          {
            strcpy( cTemp, "<ma_packageIntCodes>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

            for ( j = 0; j < iNumDataMembers; j++ )
            {
              sprintf( cTemp, "%s%s%s%d%s", "<ma_intCode label=\"", mp_gridSettings[i].p_oGridPointer->GetPackageIntDataLabel( j ).c_str(),
                  "\">", j, "</ma_intCode>" );
              AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
            }

            strcpy( cTemp, "</ma_packageIntCodes>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          } //end of if (iNumDataMembers > 0)

          //Floats
          iNumDataMembers = mp_gridSettings[i].p_oGridPointer->GetNumberFloatPackageDataMembers();
          if ( iNumDataMembers > 0 )
          {
            strcpy( cTemp, "<ma_packageFloatCodes>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

            for ( j = 0; j < iNumDataMembers; j++ )
            {
              sprintf( cTemp, "%s%s%s%d%s", "<ma_floatCode label=\"", mp_gridSettings[i].p_oGridPointer->GetPackageFloatDataLabel( j ).c_str(),
                  "\">", j, "</ma_floatCode>" );
              AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
            }

            strcpy( cTemp, "</ma_packageFloatCodes>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          } //end of if (iNumDataMembers > 0)

          //Strings
          iNumDataMembers = mp_gridSettings[i].p_oGridPointer->GetNumberStringPackageDataMembers();
          if ( iNumDataMembers > 0 )
          {
            strcpy( cTemp, "<ma_packageCharCodes>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

            for ( j = 0; j < iNumDataMembers; j++ )
            {
              sprintf( cTemp, "%s%s%s%d%s", "<ma_charCode label=\"", mp_gridSettings[i].p_oGridPointer->GetPackageStringDataLabel( j ).c_str(),
                  "\">", j, "</ma_charCode>" );
              AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
            }

            strcpy( cTemp, "</ma_packageCharCodes>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          } //end of if (iNumDataMembers > 0)

          //Bools
          iNumDataMembers = mp_gridSettings[i].p_oGridPointer->GetNumberBoolPackageDataMembers();
          if ( iNumDataMembers > 0 )
          {
            strcpy( cTemp, "<ma_packageBoolCodes>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

            for ( j = 0; j < iNumDataMembers; j++ )
            {
              sprintf( cTemp, "%s%s%s%d%s", "<ma_boolCode label=\"", mp_gridSettings[i].p_oGridPointer->GetPackageBoolDataLabel( j ).c_str(),
                  "\">", j, "</ma_boolCode>" );
              AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
            }

            strcpy( cTemp, "</ma_packageBoolCodes>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          } //end of if (iNumDataMembers > 0)
        }

        //Plot size values
        sprintf( cTemp, "%s%g%s%g%s", "<ma_plotLenX>", fPlotLenX, "</ma_plotLenX><ma_plotLenY>",
            fPlotLenY, "</ma_plotLenY>" );
        AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

        //Length of grids
        sprintf( cTemp, "%s%g%s%g%s", "<ma_lengthXCells>", mp_gridSettings[i].p_oGridPointer->GetLengthXCells(),
            "</ma_lengthXCells><ma_lengthYCells>", mp_gridSettings[i].p_oGridPointer->GetLengthYCells(),
            "</ma_lengthYCells>" );
        AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

        //********************************
        // Map Values
        //********************************
        //Escape out if there's nothing to save
        if ( mp_gridSettings[i].iNumInts == 0 && mp_gridSettings[i].iNumFloats == 0 && mp_gridSettings[i].iNumStrings == 0
            && mp_gridSettings[i].iNumBools == 0 && mp_gridSettings[i].iNumPackageInts == 0
            && mp_gridSettings[i].iNumPackageFloats == 0 && mp_gridSettings[i].iNumPackageStrings == 0
            && mp_gridSettings[i].iNumPackageBools == 0 )
          goto EndTag;

        iNumXCells = mp_gridSettings[i].p_oGridPointer->GetNumberXCells();
        iNumYCells = mp_gridSettings[i].p_oGridPointer->GetNumberYCells();
        for ( iX = 0; iX < iNumXCells; iX++ )
          for ( iY = 0; iY < iNumYCells; iY++ )
          {

            //opening grid tag
            sprintf( cTemp, "%s%d%s%d%s", "<ma_v x=\"", iX, "\" y=\"", iY, "\">" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

            //Write int values for this cell
            for ( j = 0; j < mp_gridSettings[i].iNumInts; j++ )
            {

              mp_gridSettings[i].p_oGridPointer->GetValueOfCell( iX, iY, mp_gridSettings[i].p_iIntCodes[j], & iTemp );

              sprintf( cTemp, "%s%d%s%d%s", "<int c=\"", mp_gridSettings[i].p_iIntCodes[j], "\">", iTemp, "</int>" );

              AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
            }
            //Write float values for this cell
            for ( j = 0; j < mp_gridSettings[i].iNumFloats; j++ )
            {

              mp_gridSettings[i].p_oGridPointer->GetValueOfCell( iX, iY, mp_gridSettings[i].p_iFloatCodes[j], & fTemp );

              sprintf( cTemp, "%s%d%s%g%s", "<fl c=\"", mp_gridSettings[i].p_iFloatCodes[j], "\">", fTemp, "</fl>" );

              AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
            }
            //Write string values for this cell - but don't write empty strings
            //map reading chokes on empty strings
            for ( j = 0; j < mp_gridSettings[i].iNumStrings; j++ )
            {

              mp_gridSettings[i].p_oGridPointer->GetValueOfCell( iX, iY,
                  mp_gridSettings[i].p_iStringCodes[j], &sTemp);

              if (sTemp.length() > 0) {
                sprintf( cTemp, "%s%d%s%s%s", "<ch c=\"", mp_gridSettings[i].p_iStringCodes[j], "\">",
                    sTemp.c_str(), "</ch>" );

                AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
              }
            }
            //Write bool values for this cell
            for ( j = 0; j < mp_gridSettings[i].iNumBools; j++ )
            {

              mp_gridSettings[i].p_oGridPointer->GetValueOfCell( iX, iY, mp_gridSettings[i].p_iBoolCodes[j], & bTemp );

              if ( true == bTemp )
                sprintf( cTemp, "%s%d%s", "<bl c=\"", mp_gridSettings[i].p_iBoolCodes[j], "\">true</bl>" );

              else
                sprintf( cTemp, "%s%d%s", "<bl c=\"", mp_gridSettings[i].p_iBoolCodes[j], "\">false</bl>" );

              AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
            }

            //Packages
            if ( bSavePackages )
            {
              p_oPackage = mp_gridSettings[i].p_oGridPointer->GetFirstPackageOfCell( iX, iY );
              while ( p_oPackage )
              {

                strcpy( cTemp, "<pkg>" );
                AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

                //Write int values for this package
                for ( j = 0; j < mp_gridSettings[i].iNumPackageInts; j++ )
                {

                  p_oPackage->GetValue( mp_gridSettings[i].p_iPackageIntCodes[j], & iTemp );

                  sprintf( cTemp, "%s%d%s%d%s", "<pint c=\"", mp_gridSettings[i].p_iPackageIntCodes[j],
                      "\">", iTemp, "</pint>" );

                  AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
                }
                //Write float values for this package
                for ( j = 0; j < mp_gridSettings[i].iNumPackageFloats; j++ )
                {

                  p_oPackage->GetValue( mp_gridSettings[i].p_iPackageFloatCodes[j], & fTemp );

                  sprintf( cTemp, "%s%d%s%g%s", "<pfl c=\"", mp_gridSettings[i].p_iPackageFloatCodes[j],
                      "\">", fTemp, "</pfl>" );

                  AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
                }
                //Write char values for this package
                for ( j = 0; j < mp_gridSettings[i].iNumPackageStrings; j++ )
                {

                  p_oPackage->GetValue( mp_gridSettings[i].p_iPackageStringCodes[j], &sTemp );

                  if (sTemp.length() > 0) {

                    sprintf( cTemp, "%s%d%s%s%s", "<pch c=\"", mp_gridSettings[i].p_iPackageStringCodes[j], "\">",
                        sTemp.c_str(), "</pch>" );

                    AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
                  }
                }
                //Write bool values for this package
                for ( j = 0; j < mp_gridSettings[i].iNumPackageBools; j++ )
                {

                  p_oPackage->GetValue( mp_gridSettings[i].p_iPackageBoolCodes[j], & bTemp );

                  if ( true == bTemp )
                    sprintf( cTemp, "%s%d%s", "<pbl c=\"", mp_gridSettings[i].p_iPackageBoolCodes[j], "\">true</pbl>" );
                  else
                    sprintf( cTemp, "%s%d%s", "<pbl c=\"", mp_gridSettings[i].p_iPackageBoolCodes[j], "\">false</pbl>" );

                  AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
                }
                strcpy( cTemp, "</pkg>" );
                AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );

                p_oPackage = p_oPackage->GetNextPackage();
              } //end of while (p_oPackage)
            } //end of if (bSavePackages)

            //closing value tag
            strcpy( cTemp, "</ma_v>" );
            AddToBuffer( cBuf, cTemp, out, BUFFER_SIZE );
          }
        //closing grid tag
        EndTag:
        //Flush buffer for the last time
        fwrite( cBuf, strlen( cBuf ), 1, out );
        strcpy( cBuf, "</grid>" );
        fwrite( cBuf, strlen( cBuf ), 1, out );
      } //end of if this is a saving timestep
    } //end of for (i = 0; i < m_iNumGridsToSave; i++)

    fclose( out );
  } //end of try block
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
    stcErr.sFunction = "clOutput::WriteGridData" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// GetTimestepFilename()
/////////////////////////////////////////////////////////////////////////////
string clOutput::GetTimestepFilename( int iTimestep, int iSubplot )
{
  //If the filename is empty return empty string
  if ( m_sFileRoot.length() == 0 )
  {
    //Return empty string
    return "";
  }

  char cTS[10];
  sprintf(cTS, "%d", iTimestep);

  if (-1 == iSubplot) {
    return m_sFileRoot + "_" + cTS + DETAILED_OUTPUT_FILE_EXT;
  } else if (iSubplot > -1 && iSubplot < m_iNumSubplotsToSave) {
    return m_sFileRoot + "_" + mp_subplots[iSubplot].sSubplotName +
        "_" + cTS + DETAILED_OUTPUT_FILE_EXT;
  } else {
    //Error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Invalid subplot number:" << iSubplot;
    stcErr.sMoreInfo = s.str();
    stcErr.sFunction = "clOutput::GetTimestepFilename" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// MakeMasterTreeSettings()
/////////////////////////////////////////////////////////////////////////////
void clOutput::MakeMasterTreeSettings() {
  int iTp, iSp, iCd, i, iMaxCode, iNumCodes;
  bool *p_bUsed = NULL;

  mp_masterTreeSettings = new stcTreeOutputInfo * [m_iNumSpecies];
  for ( iSp = 0; iSp < m_iNumSpecies; iSp++ ) {
    mp_masterTreeSettings[iSp] = new stcTreeOutputInfo[m_iNumTypes];
    for ( iTp = 0; iTp < m_iNumTypes; iTp++ ) {
      mp_masterTreeSettings[iSp][iTp].iSaveFreq = -1;
      mp_masterTreeSettings[iSp][iTp].iSumTimestep = -1;
      mp_masterTreeSettings[iSp][iTp].iNumInts = 0;
      mp_masterTreeSettings[iSp][iTp].iNumFloats = 0;
      mp_masterTreeSettings[iSp][iTp].iNumStrings = 0;
      mp_masterTreeSettings[iSp][iTp].iNumBools = 0;
      mp_masterTreeSettings[iSp][iTp].bSaveThisTimeStep = false;
      mp_masterTreeSettings[iSp][iTp].p_iIntCodes = NULL;
      mp_masterTreeSettings[iSp][iTp].p_iFloatCodes = NULL;
      mp_masterTreeSettings[iSp][iTp].p_iStringCodes = NULL;
      mp_masterTreeSettings[iSp][iTp].p_iBoolCodes = NULL;

      //******************************************************
      //Ints
      //******************************************************
      //Get the maximum code value
      iMaxCode = -1;
      for (i = 0; i < mp_treeSettings[iSp][iTp].iNumInts; i++) {
        iMaxCode = (iMaxCode < mp_treeSettings[iSp][iTp].p_iIntCodes[i]) ?
            mp_treeSettings[iSp][iTp].p_iIntCodes[i] : iMaxCode;
      }
      for (iCd = 0; iCd <= remove_tree; iCd++) {
        for (i = 0; i < mp_deadTreeSettings[iSp][iTp][iCd].iNumInts; i++) {
          iMaxCode = (iMaxCode < mp_deadTreeSettings[iSp][iTp][iCd].p_iIntCodes[i]) ?
              mp_deadTreeSettings[iSp][iTp][iCd].p_iIntCodes[i] : iMaxCode;
        }
      }
      //Set up a place for the unique code list
      if (iMaxCode > -1) {
        p_bUsed = new bool[iMaxCode+1];
        for (i = 0; i <= iMaxCode; i++) p_bUsed[i] = false;
        for (i = 0; i < mp_treeSettings[iSp][iTp].iNumInts; i++) {
          p_bUsed[mp_treeSettings[iSp][iTp].p_iIntCodes[i]] = true;
        }
        for (iCd = 0; iCd <= remove_tree; iCd++) {
          for (i = 0; i < mp_deadTreeSettings[iSp][iTp][iCd].iNumInts; i++) {
            p_bUsed[mp_deadTreeSettings[iSp][iTp][iCd].p_iIntCodes[i]] = true;
          }
        }

        //How many codes?
        iNumCodes = 0;
        for (i = 0; i <= iMaxCode; i++) if (p_bUsed[i]) iNumCodes++;
        //Put them in the array
        mp_masterTreeSettings[iSp][iTp].iNumInts = iNumCodes;
        mp_masterTreeSettings[iSp][iTp].p_iIntCodes = new short int[iNumCodes];
        iNumCodes = 0;
        for (i = 0; i <= iMaxCode; i++) if (p_bUsed[i]) {
          mp_masterTreeSettings[iSp][iTp].p_iIntCodes[iNumCodes] = i;
          iNumCodes++;
        }
        delete[] p_bUsed;
      }


      //******************************************************
      //Floats
      //******************************************************
      //Get the maximum code value
      iMaxCode = -1;
      for (i = 0; i < mp_treeSettings[iSp][iTp].iNumFloats; i++) {
        iMaxCode = (iMaxCode < mp_treeSettings[iSp][iTp].p_iFloatCodes[i]) ?
            mp_treeSettings[iSp][iTp].p_iFloatCodes[i] : iMaxCode;
      }
      for (iCd = 0; iCd <= remove_tree; iCd++) {
        for (i = 0; i < mp_deadTreeSettings[iSp][iTp][iCd].iNumFloats; i++) {
          iMaxCode = (iMaxCode < mp_deadTreeSettings[iSp][iTp][iCd].p_iFloatCodes[i]) ?
              mp_deadTreeSettings[iSp][iTp][iCd].p_iFloatCodes[i] : iMaxCode;
        }
      }
      //Set up a place for the unique code list
      if (iMaxCode > -1) {
        p_bUsed = new bool[iMaxCode+1];
        for (i = 0; i <= iMaxCode; i++) p_bUsed[i] = false;
        for (i = 0; i < mp_treeSettings[iSp][iTp].iNumFloats; i++) {
          p_bUsed[mp_treeSettings[iSp][iTp].p_iFloatCodes[i]] = true;
        }
        for (iCd = 0; iCd <= remove_tree; iCd++) {
          for (i = 0; i < mp_deadTreeSettings[iSp][iTp][iCd].iNumFloats; i++) {
            p_bUsed[mp_deadTreeSettings[iSp][iTp][iCd].p_iFloatCodes[i]] = true;
          }
        }

        //How many codes?
        iNumCodes = 0;
        for (i = 0; i <= iMaxCode; i++) if (p_bUsed[i]) iNumCodes++;
        //Put them in the array
        mp_masterTreeSettings[iSp][iTp].iNumFloats = iNumCodes;
        mp_masterTreeSettings[iSp][iTp].p_iFloatCodes = new short int[iNumCodes];
        iNumCodes = 0;
        for (i = 0; i <= iMaxCode; i++) if (p_bUsed[i]) {
          mp_masterTreeSettings[iSp][iTp].p_iFloatCodes[iNumCodes] = i;
          iNumCodes++;
        }
        delete[] p_bUsed;
      }

      //******************************************************
      //Chars
      //******************************************************
      //Get the maximum code value
      iMaxCode = -1;
      for (i = 0; i < mp_treeSettings[iSp][iTp].iNumStrings; i++) {
        iMaxCode = (iMaxCode < mp_treeSettings[iSp][iTp].p_iStringCodes[i]) ?
            mp_treeSettings[iSp][iTp].p_iStringCodes[i] : iMaxCode;
      }
      for (iCd = 0; iCd <= remove_tree; iCd++) {
        for (i = 0; i < mp_deadTreeSettings[iSp][iTp][iCd].iNumStrings; i++) {
          iMaxCode = (iMaxCode < mp_deadTreeSettings[iSp][iTp][iCd].p_iStringCodes[i]) ?
              mp_deadTreeSettings[iSp][iTp][iCd].p_iStringCodes[i] : iMaxCode;
        }
      }
      //Set up a place for the unique code list
      if (iMaxCode > -1) {
        p_bUsed = new bool[iMaxCode+1];
        for (i = 0; i <= iMaxCode; i++) p_bUsed[i] = false;
        for (i = 0; i < mp_treeSettings[iSp][iTp].iNumStrings; i++) {
          p_bUsed[mp_treeSettings[iSp][iTp].p_iStringCodes[i]] = true;
        }
        for (iCd = 0; iCd <= remove_tree; iCd++) {
          for (i = 0; i < mp_deadTreeSettings[iSp][iTp][iCd].iNumStrings; i++) {
            p_bUsed[mp_deadTreeSettings[iSp][iTp][iCd].p_iStringCodes[i]] = true;
          }
        }

        //How many codes?
        iNumCodes = 0;
        for (i = 0; i <= iMaxCode; i++) if (p_bUsed[i]) iNumCodes++;
        //Put them in the array
        mp_masterTreeSettings[iSp][iTp].iNumStrings = iNumCodes;
        mp_masterTreeSettings[iSp][iTp].p_iStringCodes = new short int[iNumCodes];
        iNumCodes = 0;
        for (i = 0; i <= iMaxCode; i++) if (p_bUsed[i]) {
          mp_masterTreeSettings[iSp][iTp].p_iStringCodes[iNumCodes] = i;
          iNumCodes++;
        }
        delete[] p_bUsed;
      }

      //******************************************************
      //Bools
      //******************************************************
      //Get the maximum code value
      iMaxCode = -1;
      for (i = 0; i < mp_treeSettings[iSp][iTp].iNumBools; i++) {
        iMaxCode = (iMaxCode < mp_treeSettings[iSp][iTp].p_iBoolCodes[i]) ?
            mp_treeSettings[iSp][iTp].p_iBoolCodes[i] : iMaxCode;
      }
      for (iCd = 0; iCd <= remove_tree; iCd++) {
        for (i = 0; i < mp_deadTreeSettings[iSp][iTp][iCd].iNumBools; i++) {
          iMaxCode = (iMaxCode < mp_deadTreeSettings[iSp][iTp][iCd].p_iBoolCodes[i]) ?
              mp_deadTreeSettings[iSp][iTp][iCd].p_iBoolCodes[i] : iMaxCode;
        }
      }
      //Set up a place for the unique code list
      if (iMaxCode > -1) {
        p_bUsed = new bool[iMaxCode+1];
        for (i = 0; i <= iMaxCode; i++) p_bUsed[i] = false;
        for (i = 0; i < mp_treeSettings[iSp][iTp].iNumBools; i++) {
          p_bUsed[mp_treeSettings[iSp][iTp].p_iBoolCodes[i]] = true;
        }
        for (iCd = 0; iCd <= remove_tree; iCd++) {
          for (i = 0; i < mp_deadTreeSettings[iSp][iTp][iCd].iNumBools; i++) {
            p_bUsed[mp_deadTreeSettings[iSp][iTp][iCd].p_iBoolCodes[i]] = true;
          }
        }

        //How many codes?
        iNumCodes = 0;
        for (i = 0; i <= iMaxCode; i++) if (p_bUsed[i]) iNumCodes++;
        //Put them in the array
        mp_masterTreeSettings[iSp][iTp].iNumBools = iNumCodes;
        mp_masterTreeSettings[iSp][iTp].p_iBoolCodes = new short int[iNumCodes];
        iNumCodes = 0;
        for (i = 0; i <= iMaxCode; i++) if (p_bUsed[i]) {
          mp_masterTreeSettings[iSp][iTp].p_iBoolCodes[iNumCodes] = i;
          iNumCodes++;
        }
        delete[] p_bUsed;
      }

      if (mp_masterTreeSettings[iSp][iTp].iNumFloats >  0 ||
          mp_masterTreeSettings[iSp][iTp].iNumInts >  0 ||
          mp_masterTreeSettings[iSp][iTp].iNumBools >  0 ||
          mp_masterTreeSettings[iSp][iTp].iNumStrings >  0) {
        mp_masterTreeSettings[iSp][iTp].bSaveThisTimeStep = true;
      }
    }
  }
}


/////////////////////////////////////////////////////////////////////////////
// WriteTree()
/////////////////////////////////////////////////////////////////////////////
void clOutput::WriteTree(clTree *p_oTree, char *cBuf, char *cTemp, FILE *oOut) {

  std::string sTempTreeVal;
  float fTemp;
  bool bTemp; //for getting bool values
  int iTemp, j,
  iSp = p_oTree->GetSpecies(),
  iTp = p_oTree->GetType();

  //Write to the whole timestep, and to each subplot of which this
  //tree is a part
  sprintf( cTemp, "%s%d%s%d%s", "<tree sp=\"", iSp, "\" tp=\"", iTp, "\">" );
  AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );

  //Write int values for this tree
  for ( j = 0; j < mp_treeSettings[iSp][iTp].iNumInts; j++ ) {
    p_oTree->GetValue( mp_treeSettings[iSp][iTp].p_iIntCodes[j], & iTemp );
    sprintf( cTemp, "%s%d%s%d%s", "<int c=\"", mp_treeSettings[iSp][iTp].p_iIntCodes[j], "\">", iTemp, "</int>" );
    AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );
  }
  //Write float values for this tree
  for ( j = 0; j < mp_treeSettings[iSp][iTp].iNumFloats; j++ )
  {
    p_oTree->GetValue( mp_treeSettings[iSp][iTp].p_iFloatCodes[j], & fTemp );

    sprintf( cTemp, "%s%d%s%g%s", "<fl c=\"", mp_treeSettings[iSp][iTp].p_iFloatCodes[j], "\">", fTemp, "</fl>" );
    AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );
  }
  //Write char values for this tree
  for ( j = 0; j < mp_treeSettings[iSp][iTp].iNumStrings; j++ )
  {
    p_oTree->GetValue( mp_treeSettings[iSp][iTp].p_iStringCodes[j], &sTempTreeVal );
    if ( sTempTreeVal.length() != 0 )
    {
      sprintf( cTemp, "%s%d%s%s%s", "<ch c=\"", mp_treeSettings[iSp][iTp].p_iStringCodes[j], "\">",
          sTempTreeVal.c_str(), "</ch>" );

      AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );
    }
  }
  //Write bool values for this tree
  for ( j = 0; j < mp_treeSettings[iSp][iTp].iNumBools; j++ )
  {
    p_oTree->GetValue( mp_treeSettings[iSp][iTp].p_iBoolCodes[j], & bTemp );
    if ( true == bTemp )
      sprintf( cTemp, "%s%d%s", "<bl c=\"", mp_treeSettings[iSp][iTp].p_iBoolCodes[j], "\">true</bl>" );
    else
      sprintf( cTemp, "%s%d%s", "<bl c=\"", mp_treeSettings[iSp][iTp].p_iBoolCodes[j], "\">false</bl>" );

    AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );
  }

  strcpy( cTemp, "</tree>" );
  AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );


}


/////////////////////////////////////////////////////////////////////////////
// WriteGhost()
/////////////////////////////////////////////////////////////////////////////
void clOutput::WriteGhost(clDeadTree *p_oTree, char *cBuf, char *cTemp, FILE *oOut) {

  std::string sTempTreeVal;
  float fTemp;
  bool bTemp; //for getting bool values
  int iTemp, j,
  iSp = p_oTree->GetSpecies(),
  iTp = p_oTree->GetType(),
  iRs = p_oTree->GetDeadReasonCode();

  //Write to the whole timestep, and to each subplot of which this
  //tree is a part
  sprintf( cTemp, "%s%d%s%d%s%d%s", "<ghost sp=\"", iSp, "\" tp=\"", iTp,
      "\" rs=\"", iRs, "\">" );
  AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );

  //Write int values for this tree
  for ( j = 0; j < mp_deadTreeSettings[iSp][iTp][iRs].iNumInts; j++ ) {
    p_oTree->GetValue( mp_deadTreeSettings[iSp][iTp][iRs].p_iIntCodes[j], & iTemp );
    sprintf( cTemp, "%s%d%s%d%s", "<gint c=\"", mp_deadTreeSettings[iSp][iTp][iRs].p_iIntCodes[j], "\">", iTemp, "</gint>" );
    AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );
  }
  //Write float values for this tree
  for ( j = 0; j < mp_deadTreeSettings[iSp][iTp][iRs].iNumFloats; j++ )
  {
    p_oTree->GetValue( mp_deadTreeSettings[iSp][iTp][iRs].p_iFloatCodes[j], & fTemp );

    sprintf( cTemp, "%s%d%s%g%s", "<gfl c=\"", mp_deadTreeSettings[iSp][iTp][iRs].p_iFloatCodes[j], "\">", fTemp, "</gfl>" );
    AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );
  }
  //Write char values for this tree
  for ( j = 0; j < mp_deadTreeSettings[iSp][iTp][iRs].iNumStrings; j++ )
  {
    p_oTree->GetValue( mp_deadTreeSettings[iSp][iTp][iRs].p_iStringCodes[j], &sTempTreeVal );
    if ( sTempTreeVal.length() != 0 )
    {
      sprintf( cTemp, "%s%d%s%s%s", "<gch c=\"", mp_deadTreeSettings[iSp][iTp][iRs].p_iStringCodes[j], "\">",
          sTempTreeVal.c_str(), "</gch>" );

      AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );
    }
  }
  //Write bool values for this tree
  for ( j = 0; j < mp_deadTreeSettings[iSp][iTp][iRs].iNumBools; j++ )
  {
    p_oTree->GetValue( mp_deadTreeSettings[iSp][iTp][iRs].p_iBoolCodes[j], & bTemp );
    if ( true == bTemp )
      sprintf( cTemp, "%s%d%s", "<gbl c=\"", mp_deadTreeSettings[iSp][iTp][iRs].p_iBoolCodes[j], "\">true</gbl>" );
    else
      sprintf( cTemp, "%s%d%s", "<gbl c=\"", mp_deadTreeSettings[iSp][iTp][iRs].p_iBoolCodes[j], "\">false</gbl>" );

    AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );
  }

  strcpy( cTemp, "</ghost>" );
  AddToBuffer( cBuf, cTemp, oOut, BUFFER_SIZE );


}
