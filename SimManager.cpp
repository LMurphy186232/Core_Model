//---------------------------------------------------------------------------
//#define DEBUG


#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <sstream>

#include "SimManager.h"
#include "Plot.h"
#include "BehaviorBase.h"
#include "TreePopulation.h"
#include "Output.h"
#include "Messages.h"
#include "Behaviors.h"
#include "Constants.h"
#include "Populations.h"
#include "Grids.h"
#include "Grid.h"
#include "PopulationBase.h"
#include "ParsingFunctions.h"
#include "ModelMath.h"

//XML includes
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>


using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Constructor()
//////////////////////////////////////////////////////////////////////////////
clSimManager::clSimManager( int iMajorVersion, int iMinorVersion, const string sAppPath)
{
  try
  {
    m_iMajorVersion = iMajorVersion;
    m_iMinorVersion = iMinorVersion;
    m_sAppPath = sAppPath;

    mp_oBehaviorManager = NULL;
    mp_oPopulationManager = NULL;
    mp_oGridManager = NULL;
    mp_oPlot = NULL;

    m_eSimState = No_Data;

    m_iBatchNumber = 0;
    m_iBatchGroup = 0;

    //Prepare XML parsing tools
    try {
      XMLPlatformUtils::Initialize();
    } catch (const XMLException& toCatch) {
      char* message = XMLString::transcode(toCatch.getMessage());
      modelErr stcErr;
      stcErr.iErrorCode = MODEL_NOT_READY;
      stcErr.sFunction = "clSimManager::clSimManager";
      stcErr.sMoreInfo = "Couldn't initialize XML parsing. Message: ";
      stcErr.sMoreInfo += message;
      XMLString::release(&message);
      throw( stcErr );
    }

    mp_oXMLParser = new xercesc::XercesDOMParser();
    ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
    mp_oXMLParser->setErrorHandler( errHandler );

    GoToNoDataState();
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::clSimManager" ;
    GoToNoDataState();
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Destructor()
//////////////////////////////////////////////////////////////////////////////
clSimManager::~clSimManager()
{
  if ( No_Data != m_eSimState )
    GoToNoDataState();

  delete mp_oPopulationManager; mp_oPopulationManager = NULL;
  delete mp_oGridManager; mp_oGridManager = NULL;
  delete mp_oBehaviorManager; mp_oBehaviorManager = NULL;
  delete mp_oPlot; mp_oPlot = NULL;

  //Shut down the XML tools
  delete mp_oXMLParser;
  XMLPlatformUtils::Terminate();
}

/////////////////////////////////////////////////////////////////////////////
// GoToNoDataState()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::GoToNoDataState()
{
  try
  {
    //Create the object managers, if they haven't been already
    if ( !mp_oPopulationManager )
      mp_oPopulationManager = new clPopulationManager( this );
    if ( !mp_oGridManager )
      mp_oGridManager = new clGridManager( this );
    if ( !mp_oBehaviorManager )
      mp_oBehaviorManager = new clBehaviorManager( this );

    //Set up the plot, if it hasn't been already
    //if ( !mp_oPlot )
    delete mp_oPlot;  mp_oPlot = new clPlot( this );

    //Have the object managers and plot clear their memory, in case we're coming
    //from a data defined state
    if ( m_eSimState > No_Data )
    {
      mp_oGridManager->FreeMemory();
      mp_oPopulationManager->FreeMemory();
      mp_oBehaviorManager->FreeMemory();
      //delete mp_oPlot;
      //mp_oPlot = new clPlot( this );
    }

    //Re-initialize variables that come from input files
    m_iNumTimesteps = 0;
    m_iRandomSeed = 0;

    m_iCurrentTimestep = 0;
    m_iTargetTimestep = 0;

    m_iBatchNumber = 0;
    //No - don't do batch group here
    //m_iBatchGroup = 0;

    m_eSimState = No_Data;
  } //end of try block
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GoToNoDataState" ;
    GoToNoDataState();
    throw( stcErr );
  }
}



/////////////////////////////////////////////////////////////////////////////
// ReadFile()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::ReadFile(string sFileName )
{
  try
  {
    DOMDocument * p_oDoc;
    fileType iFileType;

    //Check the file type
    iFileType = GetFileType( sFileName );

    //Parameter file - load it
    if ( parfile == iFileType )
    {
      mp_oXMLParser->parse(sFileName.c_str());
      p_oDoc = mp_oXMLParser->getDocument();
      m_sParFilename = sFileName;
      ReadParameterFile( p_oDoc, iFileType );

      //Batch file - erase memory and run the batch
    }
    else if ( batchfile == iFileType )
    {
      GoToNoDataState();
      RunBatch( sFileName );

      //Detailed output file - load it like a par file
    }
    else if ( detailed_output == iFileType )
    {
      mp_oXMLParser->parse(sFileName.c_str());
      p_oDoc = mp_oXMLParser->getDocument();
      m_sParFilename = sFileName;
      ReadParameterFile( p_oDoc, iFileType );

      //Tree file - pass it to the tree population
    }
    else if ( tree == iFileType )
    {
      //This is only allowed if a parameter file is loaded (i.e. the model is
      //ready to run)
      if ( Initialized != m_eSimState )
      {
        modelErr stcErr;
        stcErr.iErrorCode = MODEL_NOT_READY;
        stcErr.sFunction = "clSimManager::ReadFile" ;
        stcErr.sMoreInfo = "Parameter file must be loaded";
        throw( stcErr );
      }
      else
      {
        mp_oXMLParser->parse(sFileName.c_str());
        p_oDoc = mp_oXMLParser->getDocument();
        ReadTreeFile( p_oDoc, iFileType );
      }

      //Tree map file - pass it to the tree population
    }
    else if ( treemap == iFileType )
    {
      //This is only allowed if a parameter file is loaded (i.e. the model is
      //ready to run)
      if ( Initialized != m_eSimState )
      {
        modelErr stcErr;
        stcErr.iErrorCode = MODEL_NOT_READY;
        stcErr.sFunction = "clSimManager::ReadFile" ;
        stcErr.sMoreInfo = "Parameter file must be loaded";
        throw( stcErr );
      }
      else
      {
        mp_oXMLParser->parse(sFileName.c_str());
        p_oDoc = mp_oXMLParser->getDocument();
        ReadTreeFile( p_oDoc, iFileType );
      }

      //Grid map file - pass it to the grid manager
    }
    else if ( map == iFileType )
    {
      //This is only allowed if a parameter file is loaded (i.e. the model is
      //ready to run)
      if ( Initialized != m_eSimState )
      {
        modelErr stcErr;
        stcErr.iErrorCode = MODEL_NOT_READY;
        stcErr.sFunction = "clSimManager::ReadFile" ;
        stcErr.sMoreInfo = "Parameter file must be loaded";
        throw( stcErr );
      }
      else
      {
        mp_oXMLParser->parse(sFileName.c_str());
        p_oDoc = mp_oXMLParser->getDocument();
        ReadMapFile( p_oDoc, iFileType );
      }

      //Detailed output timestep file - read it like tree and grid maps
    }
    else if ( detailed_output_timestep == iFileType )
    {
      //This is only allowed if a parameter file is loaded (i.e. the model is
      //ready to run)
      if ( Initialized != m_eSimState )
      {
        modelErr stcErr;
        stcErr.iErrorCode = MODEL_NOT_READY;
        stcErr.sFunction = "clSimManager::ReadFile" ;
        stcErr.sMoreInfo = "Parameter file must be loaded";
        throw( stcErr );
      }
      else
      {
        mp_oXMLParser->parse(sFileName.c_str());
        p_oDoc = mp_oXMLParser->getDocument();
        ReadMapFile( p_oDoc, iFileType );
        ReadTreeFile( p_oDoc, iFileType );
      }
    }
    else
    {
      //This is not a type we can handle - throw an error
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE_TYPE;
      stcErr.sFunction = "clSimManager::ReadFile" ;
      stcErr.sMoreInfo = sFileName;
      throw( stcErr );
    }

    //Free the DOM document's memory
    mp_oXMLParser->resetDocumentPool();
  }
  catch (modelErr &e) {throw (e);}
  catch (SAXParseException & err) {
    modelErr stcErr;
    char* message = XMLString::transcode(err.getMessage());
    stcErr.iErrorCode = BAD_XML_FILE;
    stcErr.sFunction = "Xerces parser";
    stcErr.sMoreInfo = message;
    XMLString::release(&message);
    throw(stcErr);
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::ReadFile" ;
    GoToNoDataState();
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// GetFileType()
/////////////////////////////////////////////////////////////////////////////
fileType clSimManager::GetFileType(string sFileName)
{
  try
  {
    using namespace std;
    string strFileExtension, strFileNameCopy, strPlaybackExtension, strFileCodeTag = "fileCode", //the XML tag for the SORTIE file
        //code
        strLine, //for working with lines of a text file after converting
        //them from char*
        strFileCode; //for holding the text of the file code from an XML file
    string::size_type pos; //for string parsing - holds string positions
    fstream file;
    FILE * filetest; //only this kind of file can be tested for existence of a
    //file without creating
    char cXMLTest[5], //for extracting the first 4 characters of a text file
    cLine[200], //for extracting text from file and converting to string
    cXMLDec[] = "<?xm", cXMLCom[] = "<!--";
    int iFileType = 0; //for an int representation of the file type code
    modelErr stcErr;
    stcErr.sFunction = "clSimManager::GetFileType" ;

    //Does this file exist and can it be opened?
    filetest = fopen(sFileName.c_str(), "r");
    if ( filetest == NULL )
    { //file does not exist
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sMoreInfo = sFileName;
      throw( stcErr );
    }
    else
      fclose( filetest );

    strFileNameCopy = sFileName;

    //Find this file's extension
    pos = strFileNameCopy.find_last_of( "." );
    if ( pos == string::npos ) //no extension - return "not sortie"
      return notrecognized;

    strFileExtension = strFileNameCopy.substr( pos, strFileNameCopy.length() );

    // IS THIS XML?
    //Look at the first four characters of the file - should be either <!--
    //(a comment) or <?xm (the xml declaration)
    file.open(sFileName.c_str(), ios::in);
    file.get( cXMLTest, sizeof( cXMLTest ) );
    strcat( cXMLTest, "\0" );
    if ( strcmp( cXMLTest, cXMLDec ) == 0 || strcmp( cXMLTest, cXMLCom ) == 0 )
    {
      //Yes - this is an xml file - so search the file text for the fileCode tag
      while ( 1 )
      { //read all the file text until we find what we want
        //search the line from the file for the file code tag
        file.getline( cLine, sizeof( cLine ) );
        strLine = cLine;
        pos = strLine.find( strFileCodeTag );
        if ( pos != string::npos )
        {
          //we found the file code tag - now extract the 8 char code after it
          strFileCode = strLine.substr( pos + strFileCodeTag.length() + 2, 8 );
          //the file type is the third pair of numbers
          strFileCode = strFileCode.substr( 4, 2 );
          iFileType = atoi( strFileCode.c_str() );
          break;
        }
        if ( file.eof() ) break;
      }
      //if we don't have a file code this isn't an XML file we can read - exit
      if ( 0 == iFileType ) return notrecognized;

      //this is an XML file and we have a file type code - see if it's defined
      if ( iFileType > 0 && iFileType < lastfile )
        return (fileType) iFileType; //yep - it was found
      else
        return notrecognized; //we couldn't tell what file type it was
    } //end of if (strcmp(cXMLTest, "<?xm") == 0 || strcmp(cXMLTest, "<!--") == 0)

    /** IS THIS OLD SORTIE? This isn't an XML file so see if it's an old SORTIE text file */
    //Check the file extension
    if ( strFileExtension == ".sgli" || strFileExtension == ".par" || strFileExtension == ".out"
        || strFileExtension == ".stmf" || strFileExtension == ".sll" || strFileExtension == ".hvr"
            || strFileExtension == ".spb" || strFileExtension == ".hvs" ) return oldsortie;

    return notrecognized;
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GetFileType" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return notrecognized;
}

/////////////////////////////////////////////////////////////////////////////
// GetFileType()
/////////////////////////////////////////////////////////////////////////////
fileType clSimManager::GetFileType( DOMDocument * p_oDoc ) {
  try {
    DOMElement * p_oRoot; //for getting the root element
    DOMAttr * p_oAttribute; //for getting the filecode node
    XMLCh *sVal;
    std::string strData; //for extracting the filecode text
    char *cData;
    int iFileType; //for storing the extracted file type

    p_oRoot = p_oDoc->getDocumentElement();
    sVal = XMLString::transcode( "fileCode" );
    p_oAttribute = p_oRoot->getAttributeNode( sVal );
    XMLString::release(&sVal);
    if ( NULL == p_oAttribute ) return notrecognized;
    cData = XMLString::transcode(p_oAttribute->getValue());
    strData = cData;
    delete[] cData;
    strData = strData.substr( 4, 2 );
    iFileType = atoi( strData.c_str() );

    //if we don't have a file code this isn't an XML file we can read - exit
    if ( 0 == iFileType ) return notrecognized;

    //See if the file type code is defined
    if ( iFileType > 0 && iFileType < lastfile )
      return (fileType)iFileType; //yep - it was found
    else
      return notrecognized; //we couldn't tell what file type it was
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GetFileType" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return notrecognized;
}


/////////////////////////////////////////////////////////////////////////////
// RunSim()
/////////////////////////////////////////////////////////////////////////////
unsigned long clSimManager::RunSim( int iNumStepsToRun )
{
  try
  {
    using namespace std;
    DOMDocument * p_oDoc; //in case we need to parse and reload the par file
    clWorkerBase * p_oTempObject = NULL;
    clBehaviorBase * p_oBehavior = NULL;
    clPopulationBase * p_oPopulation = NULL;
    modelMsg stcMsg; //for checking for messages
    int iNumBehaviors = 0, //number of behaviors - for looping through
        //and calling their Action() functions
        iNumPopulations = 0, //number of populations
        iTimestep = 0, //current timestep - loop counter
        iStartTimestep = 0, //the timestep to start on this time
        iEndTimestep = 0, //the timestep to end on this time
        iHrs,iMin;
    clock_t startRun, endRun, now;
    unsigned long iRunTime;

    m_bUserQuit = false;
    m_bUserPaused = false;

    //Make sure that the number of timesteps is not less than zero
    if ( 0 > iNumStepsToRun )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::RunSim" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Number of timesteps to run must be greater than zero.";
      throw( stcErr );
    }

    //If the status is "Run_Complete" - reset the model
    if ( Run_Complete == m_eSimState )
    {
      mp_oXMLParser->parse(m_sParFilename.c_str());
      p_oDoc = mp_oXMLParser->getDocument();
      ReadParameterFile( p_oDoc, GetFileType( p_oDoc ) );
      //Free DOM doc memory
      mp_oXMLParser->resetDocumentPool();
    }

    //Only continue if the Simulation Manager state is Initialized or Paused
    if ( m_eSimState != Initialized && m_eSimState != Paused )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::RunSim" ;
      stcErr.iErrorCode = MODEL_NOT_READY;
      stcErr.sMoreInfo = "Valid parameter file not loaded";
      throw( stcErr );
    }

    startRun = clock();

    //If the model status is paused and no timesteps are entered, pick up where
    //we left off
    if ( Paused == m_eSimState && 0 == iNumStepsToRun )
    {
      //If the current timestep is the same as the target timestep (minus 1),
      //run to the end
      if ( m_iCurrentTimestep == m_iTargetTimestep - 1 )
      {
        m_iTargetTimestep = 0;
        iEndTimestep = m_iNumTimesteps + 1;
      }
      else
      {
        iEndTimestep = m_iTargetTimestep;
      }
      iStartTimestep = m_iCurrentTimestep + 1;
      m_eSimState = Initialized; //no longer paused - running again
    } //end of if (Paused == m_eSimState && 0 != iNumStepsToRun)

    //Not paused, or we are supposed to start over - so figure out the start and
    //stop timesteps
    else
    {
      m_eSimState = Initialized; //erase paused status if present
      iStartTimestep = m_iCurrentTimestep + 1;
      if ( iNumStepsToRun < 1 || iNumStepsToRun >= ( m_iNumTimesteps - m_iCurrentTimestep ) )
        iEndTimestep = m_iNumTimesteps + 1;
      else
        iEndTimestep = iStartTimestep + iNumStepsToRun;

      m_iTargetTimestep = iEndTimestep;
    } //end of else - not paused

    //Query the behavior manager for the number of behaviors to run
    //each timestep
    iNumBehaviors = mp_oBehaviorManager->GetNumberOfObjects();
    iNumPopulations = mp_oPopulationManager->GetNumberOfObjects();

    //TIMESTEP LOOP
    for ( iTimestep = iStartTimestep; iTimestep < iEndTimestep; iTimestep++ )
    {

      //Pass a message indicating the timestep we just started, and the elapsed time
      now = clock();
      iRunTime = (now - startRun) / CLOCKS_PER_SEC;
      iHrs = (int)floor( iRunTime / 3600 );
      iRunTime -= ( iHrs * 3600 );

      //Calc number of minutes
      iMin = (int)floor( iRunTime / 60 );
      iRunTime -= ( iMin * 60 );

      stcMsg.iMessageCode = INFO;
      stringstream msg;
      msg << "Starting timestep " << iTimestep << " of " << m_iNumTimesteps
          << ". Elapsed time: " << iHrs << ":" << iMin << ":" << iRunTime;
      stcMsg.sMoreInfo = msg.str();
      SendMessage( stcMsg );

      m_iCurrentTimestep = iTimestep;

      //Run each behavior object
      for ( int i = 0; i < iNumBehaviors; i++ )
      {
        p_oTempObject = mp_oBehaviorManager->PassObjectPointer( i );
        p_oBehavior = dynamic_cast < clBehaviorBase * > ( p_oTempObject );

#ifdef DEBUG
        stcMsg.iMessageCode = INFO;
        stringstream msg;
        fstream out( "debug log.txt", ios::out | ios::app );
        msg << "Timestep " << iTimestep << ": Starting behavior " << p_oBehavior->GetName() << " at " << clock();
        out << "Timestep " << iTimestep << ": Starting behavior " << p_oBehavior->GetName() << " at " << clock() << "\n";
        out.close();
        strcpy( stcMsg.cMoreInfo, msg.str().c_str() );
        SendMessage( stcMsg );
#endif

        p_oBehavior->Action();
        for ( int j = 0; j < iNumPopulations; j++ )
        {
          p_oTempObject = mp_oPopulationManager->PassObjectPointer( j );
          p_oPopulation = dynamic_cast < clPopulationBase * > ( p_oTempObject );
          p_oPopulation->DoDataUpdates();
        }
      }

      //Run the cleanup operations
      TimestepCleanup();

      //Check to see if we need to process any messages while we were waiting
      stcMsg = CheckForMessage(m_sAppPath);
      if ( NO_MESSAGE != stcMsg.iMessageCode )
      {
        if ( MODEL_PAUSED == stcMsg.iMessageCode )
        {
          //"pause"; set the timestep we were interrupted on so we know where
          //to pick up
          m_eSimState = Paused;
          m_iTargetTimestep = iEndTimestep;
          stcMsg.iMessageCode = MODEL_PAUSED;
          stringstream s;
          s << "Finished timestep " << m_iCurrentTimestep;
          stcMsg.sMoreInfo = s.str();
          SendMessage( stcMsg );
          endRun = clock();
          iRunTime = endRun - startRun;
          m_bUserPaused = true;
          return ( iRunTime / CLOCKS_PER_SEC ); //convert to seconds

        }
        else if ( QUIT == stcMsg.iMessageCode )
        {
          //quit - shut down everything
          GoToNoDataState();
          m_bUserQuit = true;
          endRun = clock();
          iRunTime = endRun - startRun;
          return ( iRunTime / CLOCKS_PER_SEC ); //convert to seconds
        }
      }
    } //end of timestep loop

    //Set the sim state to run complete if we've run all the timesteps
    if ( m_iCurrentTimestep == m_iNumTimesteps )
    {
      //Perform end-of-run cleanup
      EndOfRunCleanup();
      m_eSimState = Run_Complete;
      m_iTargetTimestep = 0;
    }
    //Otherwise the status is paused
    else {
      m_eSimState = Paused;
      stcMsg.iMessageCode = MODEL_PAUSED;
      SendMessage( stcMsg );
    }

    endRun = clock();
    iRunTime = endRun - startRun;
    return ( iRunTime / CLOCKS_PER_SEC ); //convert to seconds

  } //end of try block
  catch ( modelErr & err )
  {
    char sMsg[30];
    sprintf(sMsg, "%s%ld", " Random seed: ", m_iActualSeed);
    err.sMoreInfo += sMsg;
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::RunSim" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return -1;
}


/////////////////////////////////////////////////////////////////////////////
// TimestepCleanup()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::TimestepCleanup()
{
  try
  {
    //Run the timestep cleanup functions for each of the object managers
    //and the plot
    mp_oPlot->TimestepCleanup();
    mp_oBehaviorManager->TimestepCleanup();
    mp_oGridManager->TimestepCleanup();
    mp_oPopulationManager->TimestepCleanup();
  } //end of try block
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::TimestepCleanup" ;
    GoToNoDataState();
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// EndOfRunCleanup()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::EndOfRunCleanup()
{
  try
  {
    //Run the end-of-run cleanup functions for each of the object managers
    //and the plot
    mp_oPlot->EndOfRunCleanup();
    mp_oBehaviorManager->EndOfRunCleanup();
    mp_oGridManager->EndOfRunCleanup();
    mp_oPopulationManager->EndOfRunCleanup();
  } //end of try block
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::EndOfRunCleanup" ;
    GoToNoDataState();
    throw( stcErr );
  }
}



/////////////////////////////////////////////////////////////////////////////
// GetPopulationObject()
/////////////////////////////////////////////////////////////////////////////
clPopulationBase *clSimManager::GetPopulationObject(string sPopName) {
  try
  {
    clWorkerBase * p_oTempObject = NULL;
    clPopulationBase * p_oPop = NULL;

    //Make sure that the population manager has been created - if not, just
    //return NULL
    if ( !mp_oPopulationManager ) return p_oPop;

    //Request the population from the Population Manager and return it
    p_oTempObject = mp_oPopulationManager->PassObjectPointer(sPopName);
    //Recast the pointer as a population pointer
    if ( p_oTempObject )
      p_oPop = dynamic_cast < clPopulationBase * > ( p_oTempObject );

    return p_oPop;
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GetPopulationObject" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// GetPopulationObject()
/////////////////////////////////////////////////////////////////////////////
clPopulationBase * clSimManager::GetPopulationObject( int iIndex )
{
  try
  {
    clWorkerBase * p_oTempObject = NULL;
    clPopulationBase * p_oPop = NULL;

    //Make sure that the population manager has been created - if not, just
    //return NULL
    if ( !mp_oPopulationManager ) return p_oPop;

    //Request the population from the Population Manager and return it
    p_oTempObject = mp_oPopulationManager->PassObjectPointer( iIndex );
    //Recast the pointer as a population pointer
    if ( p_oTempObject )
      p_oPop = dynamic_cast < clPopulationBase * > ( p_oTempObject );

    return p_oPop;
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GetPopulationObject" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// GetGridObject()
/////////////////////////////////////////////////////////////////////////////
clGrid * clSimManager::GetGridObject( const char * cGridName )
{
  try
  {
    clWorkerBase * p_oTempObject = NULL;
    clGrid * p_oGrid = NULL;

    //Make sure the Grid Manager has been created - if not, just return NULL
    if ( !mp_oGridManager ) return p_oGrid;

    //Request the grid from the Grid Manager and return it
    p_oTempObject = mp_oGridManager->PassObjectPointer( cGridName );
    //Recast the pointer as a population pointer
    if ( p_oTempObject )
      p_oGrid = ( clGrid * ) p_oTempObject;

    return p_oGrid;
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GetGridObject" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// GetGridObject()
/////////////////////////////////////////////////////////////////////////////
clGrid * clSimManager::GetGridObject( int iIndex ) {
  try {
    clWorkerBase * p_oTempObject = NULL;
    clGrid * p_oGrid = NULL;

    //Make sure the Grid Manager has been created - if not, just return NULL
    if ( !mp_oGridManager ) return p_oGrid;

    //Request the grid from the Grid Manager and return it
    p_oTempObject = mp_oGridManager->PassObjectPointer( iIndex );
    //Recast the pointer as a population pointer
    if ( p_oTempObject )
      p_oGrid = ( clGrid * ) p_oTempObject;

    return p_oGrid;
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GetGridObject" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// GetBehaviorObject()
/////////////////////////////////////////////////////////////////////////////
clBehaviorBase *clSimManager::GetBehaviorObject(string sBehaviorName)
{
  try
  {
    clWorkerBase * p_oTempObject = NULL;
    clBehaviorBase * p_oBehavior = NULL;

    //Make sure the Behavior Manager has been created - if not, just return NULL
    if ( !mp_oBehaviorManager ) return p_oBehavior;

    //Request the behavior from the Behavior Manager and return it
    p_oTempObject = mp_oBehaviorManager->PassObjectPointer(sBehaviorName);
    //Recast the pointer as a population pointer
    if ( p_oTempObject )
      p_oBehavior = dynamic_cast < clBehaviorBase * > ( p_oTempObject );

    return p_oBehavior;
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GetBehaviorObject" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// GetBehaviorObject()
/////////////////////////////////////////////////////////////////////////////
clBehaviorBase * clSimManager::GetBehaviorObject( int iIndex )
{
  try
  {
    clWorkerBase * p_oTempObject = NULL;
    clBehaviorBase * p_oBehavior = NULL;

    //Make sure the Behavior Manager has been created - if not, just return NULL
    if ( !mp_oBehaviorManager ) return p_oBehavior;

    //Request the behavior from the Behavior Manager and return it
    p_oTempObject = mp_oBehaviorManager->PassObjectPointer( iIndex );
    //Recast the pointer as a population pointer
    if ( p_oTempObject )
      p_oBehavior = dynamic_cast < clBehaviorBase * > ( p_oTempObject );

    return p_oBehavior;
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::GetGridObject" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::ReadParameterFile( DOMDocument * p_oDoc, fileType iFileType )
{
  try
  {
    //Make sure our pointer is good
    if ( NULL == p_oDoc )
    { //nope - throw error
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::ReadParameterFile" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Document pointer is NULL.";
      throw( stcErr );
    }

    //If there's currently data read in, erase it all
    if ( No_Data != m_eSimState ) GoToNoDataState();

    //Call the simulation manager's own DoSetup, which will read general params
    DoSetup( p_oDoc );

    //Set up the plot object
    mp_oPlot->DoObjectSetup( p_oDoc, iFileType );

    // Start by creating the objects
    //Do populations
    mp_oPopulationManager->CreateObjects( p_oDoc );

    //Do grids
    mp_oGridManager->CreateObjects( p_oDoc );

    //Do behaviors
    mp_oBehaviorManager->CreateObjects( p_oDoc );

    // Phase 2 setup
    //Do populations
    mp_oPopulationManager->DoObjectSetup( p_oDoc, iFileType );

    //Do grids
    mp_oGridManager->DoObjectSetup( p_oDoc, iFileType );

    //Do behaviors
    mp_oBehaviorManager->DoObjectSetup( p_oDoc, iFileType );

    //Set the sim state - if we were paused mid-run, keep the status as
    //paused
    if ( Paused != m_eSimState )
      m_eSimState = Initialized;
  } //end of try block
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::ReadParameterFile" ;
    GoToNoDataState();
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// ReadTreeFile()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::ReadTreeFile( DOMDocument * p_oDoc, fileType iFileType )
{
  try
  {
    clWorkerBase * p_oTreePop = NULL;

    //Make sure that we are not in the No_Data state
    if ( No_Data == m_eSimState )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::ReadTreeFile" ;
      stcErr.iErrorCode = MODEL_NOT_READY;
      throw( stcErr );
    }

    //Make sure our pointer is good
    if ( NULL == p_oDoc )
    { //nope - throw error
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::ReadTreeFile" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Document pointer is NULL.";
      throw( stcErr );
    }

    //Request the tree population from the Population Manager
    p_oTreePop = mp_oPopulationManager->PassObjectPointer( "treepopulation" );

    //Make sure we got a valid pointer - if not throw error
    if ( NULL == p_oTreePop )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::ReadTreeFile" ;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw( stcErr );
    }

    //Pass it to the tree population for setup
    p_oTreePop->DoObjectSetup( p_oDoc, iFileType );
  }
  catch ( modelErr & err )
  {
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::ReadTreeFile" ;
    GoToNoDataState();
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// ReadMapFile()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::ReadMapFile( DOMDocument * p_oDoc, fileType iFileType )
{
  try
  {
    //Make sure that we are not in the No_Data state
    if ( No_Data == m_eSimState )
    {
      modelErr stcErr;
      stcErr.iErrorCode = MODEL_NOT_READY;
      stcErr.sFunction = "clSimManager::ReadMapFile" ;
      throw( stcErr );
    }

    //Make sure our pointer is good
    if ( NULL == p_oDoc )
    { //nope - throw error
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::ReadMapFile" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Document pointer is NULL.";
      throw( stcErr );
    }

    //Pass it to the grid manager for setup
    mp_oGridManager->DoObjectSetup( p_oDoc, iFileType );
  }
  catch ( modelErr & err )
  {
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::ReadMapFile" ;
    GoToNoDataState();
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// DoSetup()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::DoSetup( DOMDocument * p_oDoc )
{
  try
  {
    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    int iTemp;

    //Get number of timesteps
    FillSingleValue( p_oElement, "timesteps", & iTemp, true );
    if ( 0 >= iTemp )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::DoSetup" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Timesteps must be a positive number.";
      throw( stcErr );
    }
    m_iNumTimesteps = iTemp;

    //Get number of years per timestep
    FillSingleValue( p_oElement, "yearsPerTimestep", & m_iNumYearsPerTimestep, true );

    //Throw an error if the value is less than 0
    if ( 0 >= m_iNumYearsPerTimestep )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::DoSetup" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Number of years per timestep must be a positive number.";
      throw( stcErr );
    }

    //Get current timestep, if present
    iTemp = 0;
    FillSingleValue( p_oElement, "currentTimestep", & iTemp, false );

    //Make sure that the current timestep is not a negative number or
    //greater than the number of timesteps
    if ( iTemp < 0 || iTemp >= m_iNumTimesteps )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::DoSetup" ;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Current timestep is not within run - value is " << iTemp;
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }
    m_iCurrentTimestep = iTemp;

    //Get random number seed
    FillSingleValue( p_oElement, "randomSeed", & m_iRandomSeed, true );
    //Initialize random number generator - if a seed was provided, use it; if
    //the seed was 0, use a random seed
    if ( 0 == m_iRandomSeed )
    {
      time(&m_iActualSeed);
      m_iActualSeed = -m_iActualSeed;
    }
    else if ( m_iRandomSeed > 0 )
    {
      m_iActualSeed = -m_iRandomSeed;
    }
    else
    {
      m_iActualSeed = m_iRandomSeed;
    }

    clModelMath::SetRandomSeed(m_iActualSeed);

    //log.open(logFileName, ios::out | ios::app);
    //log << "Beginning of run for par file " << m_strParFilename << "\n";
    //log.close();

  } //end of the try block
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::DoSetup" ;
    GoToNoDataState();
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// RunBatch()
/////////////////////////////////////////////////////////////////////////////
void clSimManager::RunBatch(string sBatchFile) {
  using namespace std;
  DOMDocument * p_oBatchFileDoc, //Document object model of the batch file
  * p_oParFileDoc; //DOM of par file
  DOMElement * p_oBatchElement; //Element variable for batch group
  DOMNode * p_oBatchNode, //Node variable for batch group
  * p_oFilenameNode; //Node for detailed output filename
  DOMNodeList * p_oBatchGroups, //List of all batch group
  * p_oNodeList;
  fileType iFileType;

  //This is the list of parameter file names and number of times to run
  //each
  struct stcParFiles
  {
    string sParFileName;
    short int iNumRuns;
  }
  * p_parFileList;

  fstream file; //for checking the validity of file names
  stringstream strNewFileName; //for assembling the new output filenames
  char * cNumRunsText; //text for number of times to run
  std::string strDetailedOutputFileName, //file name of detailed output file, if any
  strShortOutFileName; //file name of short output file, if any
  int iNumBatchGroups = 0, //number of par file / run number pairs
      iNumNodes = 0, //used for counting search results
      iStart,
      i, j; //loop counter
  bool bPausedFlag = (0 != m_iBatchNumber);

  if (sBatchFile.length() == 0 && 0 == m_sBatchFilename.length())
    return;
  if (sBatchFile.length() != 0) {
    m_sBatchFilename = sBatchFile;
  }

  //***********************************
  //Validation
  //***********************************
  //Make sure the filename passed is not empty
  if (m_sBatchFilename.length() == 0) {
    modelErr stcErr;
    stcErr.sFunction = "clSimManager::RunBatch" ;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sMoreInfo = "Need batch file path.";
    throw( stcErr );
  }

  //Make sure the file is a batch file - if not throw error
  if ( batchfile != GetFileType(m_sBatchFilename)) {
    modelErr stcErr;
    stcErr.sFunction = "clSimManager::RunBatch" ;
    stcErr.iErrorCode = BAD_FILE_TYPE;
    throw( stcErr );
  }

  //***********************************
  //Parse the batch document, and put its info into the filenames array.  Each
  //parameter file has an array bucket, with its associated number of runs to
  //complete
  //***********************************
  //Parse the document
  mp_oXMLParser->parse(m_sBatchFilename.c_str());
  p_oBatchFileDoc = mp_oXMLParser->getDocument();

  //Count the number of batch groups
  p_oBatchGroups = p_oBatchFileDoc->getElementsByTagName( XMLString::transcode( "ba_parFile" ) );
  iNumBatchGroups = p_oBatchGroups->getLength();

  //Declare the filename array to match
  p_parFileList = new struct stcParFiles[iNumBatchGroups];

  for ( i = 0; i < iNumBatchGroups; i++ )
  {
    //Cast the batch group node to an element for extracting the data
    p_oBatchNode = p_oBatchGroups->item( i );
    p_oBatchElement = ( DOMElement * ) p_oBatchNode;


    //Get the par file name
    p_oNodeList = p_oBatchElement->getElementsByTagName(XMLString::transcode("ba_fileName"));
    iNumNodes = p_oNodeList->getLength();
    if ( 0 == iNumNodes )
    { //no par filename - throw error
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::RunBatch" ;
      stcErr.iErrorCode = DATA_MISSING;
      std::stringstream s;
      s << "ba_parFile for batch group " << i;
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    } //end of if (0 == iNumNodes)


    //Assign the par filename into its bucket
    p_parFileList[i].sParFileName = XMLString::transcode(p_oNodeList->item(0)->getFirstChild()->getNodeValue());
    //p_parFileList[i].sParFileName = cNumRunsText;


    //Check to see if we can open the par file
    file.open(p_parFileList[i].sParFileName.c_str());
    if (file.bad()) {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::RunBatch" ;
      stcErr.iErrorCode = BAD_FILE;
      std::stringstream s;
      s << "Couldn't open parameter file " << p_parFileList[i].sParFileName;
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    } //end of if (file.bad())
    file.close();

    //Get the number of times to run the par file
    p_oNodeList = p_oBatchElement->getElementsByTagName( XMLString::transcode( "ba_numTimesToRun" ) );
    if ( 0 == p_oNodeList->getLength()) {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::RunBatch" ;
      stcErr.iErrorCode = DATA_MISSING;
      std::stringstream s;
      s << "ba_numTimesToRun for parameter file " << p_parFileList[i].sParFileName;
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    } //end of if (0 == p_oNodeList->getLength())
    cNumRunsText = XMLString::transcode( p_oNodeList->item( 0 )->getFirstChild()->getNodeValue() );

    //Assign number of runs to its place in the array
    p_parFileList[i].iNumRuns = atoi( cNumRunsText );

    //Make sure the number of runs was a valid, positive integer
    if ( 0 == p_parFileList[i].iNumRuns && strcmp( cNumRunsText, "0" ) != 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::RunBatch" ;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Number of runs is not a number:  " << cNumRunsText;
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
    }
    else if ( 0 > p_parFileList[i].iNumRuns )
    {
      modelErr stcErr;
      stcErr.sFunction = "clSimManager::RunBatch" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Number of runs cannot be less than zero.";
      throw( stcErr );
    }
  } //end of for (i = 0; i < iNumBatchGroups; i++)


  //****************************************
  //Main Loop
  //Go through each batch group in the array and run each par file for the
  //requested number of times
  //****************************************

  //If we were midway in a batch group, start it again at the spot where
  //we left off
  iStart = m_iBatchGroup == 0 ? 0 : m_iBatchGroup - 1;
  for ( i = iStart; i < iNumBatchGroups; i++ )
  {
    try
    {
      m_iBatchGroup = i+1;

      //Set the filename as the current parameter filename
      m_sParFilename = p_parFileList[i].sParFileName;
      iFileType = GetFileType(m_sParFilename);

      mp_oXMLParser->parse(p_parFileList[i].sParFileName.c_str());
      p_oParFileDoc = mp_oXMLParser->getDocument();
      //In batch mode - we need to check for the presence of output settings.
      //If it exists, grab the file names - we'll rename them for each run
      //so they're not overwritten
      p_oNodeList = p_oParFileDoc->getElementsByTagName( XMLString::transcode( "ou_filename" ) );
      if ( 0 != p_oNodeList->getLength() )
      {
        strDetailedOutputFileName = XMLString::transcode( p_oNodeList->item( 0 )->getFirstChild()->getNodeValue() );
        //trim off the file extension, if it exists
        if ( "" != strDetailedOutputFileName )
        {
          strDetailedOutputFileName = strDetailedOutputFileName.substr( 0, strDetailedOutputFileName.find( TARBALL_FILE_EXT ) );
        }
      }
      else
        strDetailedOutputFileName = "";
      //Short output
      p_oNodeList = p_oParFileDoc->getElementsByTagName( XMLString::transcode( "so_filename" ) );
      if ( 0 != p_oNodeList->getLength() )
      {
        strShortOutFileName = XMLString::transcode( p_oNodeList->item( 0 )->getFirstChild()->getNodeValue() );
        //trim off the file extension, if it exists
        if ( "" != strShortOutFileName )
        {
          strShortOutFileName = strShortOutFileName.substr( 0, strShortOutFileName.find( SHORT_OUTPUT_FILE_EXT ) );
        }
      }
      else
        strShortOutFileName = "";

      //Perform the number of runs specified
      cout << "\nBeginning runs for file " << p_parFileList[i].sParFileName << ".\n" << flush;

      //See whether we are picking up from an uncompleted batch; if so,
      //start there
      if (bPausedFlag) {
        iStart = m_iBatchNumber;
        bPausedFlag = false;
      } else iStart = 0;
      for ( j = iStart; j < p_parFileList[i].iNumRuns; j++ )
      {
        //Clean the memory
        GoToNoDataState();

        m_iBatchNumber = j+1;

        //Re-parse the parameter file if this isn't the first timestep -
        //otherwise we can just keep the parsed doc from above
        if ( j > 0 ) {
          mp_oXMLParser->parse(p_parFileList[i].sParFileName.c_str());
          p_oParFileDoc = mp_oXMLParser->getDocument();
        }
        //Rename the output files in the DOM
        if ( strDetailedOutputFileName != "" )
        {
          strNewFileName.str( "" );
          strNewFileName << strDetailedOutputFileName << "_" << j + 1;
          p_oNodeList = p_oParFileDoc->getElementsByTagName( XMLString::transcode( "ou_filename" ) );
          p_oFilenameNode = p_oNodeList->item( 0 )->getFirstChild();
          p_oFilenameNode->setNodeValue( XMLString::transcode( strNewFileName.str().c_str() ) );
        }
        if ( strShortOutFileName != "" )
        {
          strNewFileName.str( "" );
          strNewFileName << strShortOutFileName << "_" << j + 1;
          p_oNodeList = p_oParFileDoc->getElementsByTagName( XMLString::transcode( "so_filename" ) );
          p_oFilenameNode = p_oNodeList->item( 0 )->getFirstChild();
          p_oFilenameNode->setNodeValue( XMLString::transcode( strNewFileName.str().c_str() ) );
        }

        //Send in the par file
        ReadParameterFile( p_oParFileDoc, iFileType );

        //Run
        RunSim();

        //If the user quit this run, then quit the batch
        if (m_bUserQuit || m_bUserPaused) {
          return;
        }
        cout << "Run " << ( j + 1 ) << " of " << p_parFileList[i].iNumRuns << " completed.\n" << flush;

        //Free the DOM document's memory
        mp_oXMLParser->resetDocumentPool();
      } //end of for (j = 0; j < iNumRuns; j++)

      m_iBatchNumber = 0;
    } //end of try block
    catch (SAXParseException & err) {
      modelErr stcErr;
      char* message = XMLString::transcode(err.getMessage());
      stcErr.iErrorCode = BAD_XML_FILE;
      stcErr.sFunction = "Xerces parser";
      stcErr.sMoreInfo = message;
      XMLString::release(&message);
      throw(stcErr);
    }
    catch ( modelErr & err )
    {
      if (err.sMoreInfo.compare("Random seed") != 0) {
        stringstream s;
        s << "Random seed: " << m_iActualSeed;
        err.sMoreInfo += s.str();
      }
      GoToNoDataState();
      ExternalErrorHandler( err, m_eSimState, true );
      goto NextBatchGroup;
    }
    catch ( ... )
    {
      modelErr stcErr;
      stcErr.iErrorCode = UNKNOWN;
      stcErr.sFunction = "clSimManager::RunBatch" ;
      GoToNoDataState();
      ExternalErrorHandler( stcErr, m_eSimState, true );
      goto NextBatchGroup;
    }
    //Label for allowing code in the loop to jump to the next batch group
    NextBatchGroup:;
  } //end of for (i = 0; i < iNumBatchGroups; i++)
  m_iBatchGroup = 0;
  //Delete the par file array
  delete[] p_parFileList;
}


/////////////////////////////////////////////////////////////////////////////
// CreateGrid()
/////////////////////////////////////////////////////////////////////////////
clGrid * clSimManager::CreateGrid(string sGridName, short int iNumIntVals, short int iNumdoubleVals,
    short int iNumCharVals, short int iNumBoolVals, float fXCellLength, float fYCellLength )
{
  try
  {
    //Make sure the grid manager has been created - if not, return NULL
    if ( !mp_oGridManager ) return NULL;
    return mp_oGridManager->CreateGrid(sGridName, iNumIntVals, iNumdoubleVals, iNumCharVals, iNumBoolVals,
        fXCellLength, fYCellLength );
  }
  catch ( modelErr & err )
  {
    GoToNoDataState();
    throw( err );
  }
  catch ( modelMsg & msg )
  { //non-fatal error
    throw( msg );
  }
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clSimManager::CreateGrid" ;
    GoToNoDataState();
    throw( stcErr );
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// GetNumberOfBehaviors
/////////////////////////////////////////////////////////////////////////////
int clSimManager::GetNumberOfBehaviors()
{
  if ( NULL == mp_oBehaviorManager ) return 0;
  else
    return mp_oBehaviorManager->GetNumberOfObjects();
}

/////////////////////////////////////////////////////////////////////////////
// GetNumberOfGrids
/////////////////////////////////////////////////////////////////////////////
int clSimManager::GetNumberOfGrids()
{
  if ( NULL == mp_oGridManager ) return 0;
  else
    return mp_oGridManager->GetNumberOfObjects();
}

/////////////////////////////////////////////////////////////////////////////
// GetNumberOfPopulations
/////////////////////////////////////////////////////////////////////////////
int clSimManager::GetNumberOfPopulations()
{
  if ( NULL == mp_oPopulationManager ) return 0;
  else
    return mp_oPopulationManager->GetNumberOfObjects();
}
