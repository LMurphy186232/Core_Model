//---------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h> //for getting dates
#include <queue>
#include "SimManager.h"

//Interface stuff
#include "Interface.h"
#include "Messages.h"

#include "ParsingFunctions.h"
//---------------------------------------------------------------------------

//***************************************
// MAIN MODEL CONTROL CODE
// This calls the sim manager and gets things rockin', also acting as the
// liaison with the appropriate interface.  We expect two possible argument
// structures coming into main:  in the first, there are no arguments after
// the application name.  This starts the application using the interactive
// text interface.  In the second, there is one argument, a filename, after the
// application name.  This causes the application to start and immediately
// begin processing that file (i.e., command-line style).
//
// In either mode, you can pause an in-process run.  The method depends on the
// active interface DLL (with the DOS one, hit any key; with the Java one,
// create a file called "messages.txt" with a 'p' in the first character spot).
// In interactive mode, you are prompted for your next steps.  In command line
// mode, the model pauses until you type either "run" or "quit" (although it
// may not tell you this).  It will finish its run and exit as normal, assuming
// it is not interrupted again.
//
// So to be clear:  in command-line mode, the model seeks to finish its
// assigned run and will quit as soon as it does so, or as soon as it runs into
// a fatal error.  In interactive mode, the model will remain active until the
// user explicitly tells it to quit by typing "quit".
//
// Copyright 2003 Charles D. Canham.
//***************************************

int main( int argc, char * argv[] )
{
  using namespace std;
  modelMsg stcMsg; //for capturing messages
  modelErr stcError; //for sending errors

  string strArg1, //this represents the first argument to the command
       strArg2; //this represents the second argument to the command
  string::size_type pos;

  time_t timer;
  struct tm * stcTime;

  unsigned long iRunTime = 0; //time of run in seconds
  int iNumTimesteps; //for figuring out the number of timesteps

  //Capture the application path
  string sPath = argv[0];


  //cout << argc << " arguments.\n";
  //for (int i = 0; i < argc; i++)
  //cout << "Argument " << (i+1) << ": " << argv[i] << "\n";

  //if (sPath.find("coremodel.exe") != string::npos) {
  //  sPath = sPath.substr(0, sPath.find("coremodel.exe"));
  //}
  //The above won't work if launched with just the string "coremodel"

  if (sPath.find("coremodel") != string::npos) {
    sPath = sPath.substr(0, sPath.find("coremodel"));
  }

  //***************************************
  // Version numbers
  //***************************************
  int iMajorVersion = 7, iMinorVersion = 5;
  clSimManager p_oSimManager( iMajorVersion, iMinorVersion, sPath);

  //***************************************
  // Command line mode or text interface mode?
  // If the model was started with an argument, assume it is
  // a parameter file string and this should run in pure command line mode.
  //***************************************

  //No additional arguments beyond the application name - start the model in
  //text interface mode
  if ( 1 == argc )
  {
    DoIntroduction();

  }
  //One or two additional argument beyond the application name - we're in
  //command line mode with a file name and possibly a number of timesteps
  else if ( 2 == argc || 3 == argc )
  {
    try
    {
      //Redirect cout to a log file - use a local object to do this to make
      //sure that we direct it back in the destructor (otherwise an error can
      //cause this to crash)

      //Update - I'm not redirecting to a log file anymore.  This is for running
      //this as a job on Condor.

      timer = time( NULL );
      stcTime = localtime( & timer );

      //Write the log header
      cout << "\n\nFile:  " << argv[1] << "\nRun started on:  " << asctime( stcTime ) << "\n\n";

      //Get the file type
      fileType iFileType = p_oSimManager.GetFileType( argv[1] );
      if ( batchfile == iFileType )
      {
        p_oSimManager.RunBatch( argv[1] );
        if ( p_oSimManager.GetSimState() == clSimManager::Run_Complete
             || p_oSimManager.GetSimState() == clSimManager::No_Data )
             {
               timer = time( NULL );
               stcTime = localtime( & timer );
               cout << "\n\nRun ended on:  " << asctime( stcTime ) << "\n\n";
               return 0;
        }
      }
      else if ( parfile == iFileType || detailed_output == iFileType )
      {
        p_oSimManager.ReadFile( argv[1] );
        if ( p_oSimManager.GetSimState() != clSimManager::Initialized )
        {
          //An error occurred - exit
          return 0;
        }

        //If there's an extra argument, get the number of timesteps
        int iNumTimesteps = 0;
        if ( 3 == argc )
        {
          iNumTimesteps = atoi( argv[2] );
          if ( 0 == iNumTimesteps && argv[2] [0] != '0' )
          {
            cerr << "Couldn't read number of timesteps.\n";
            return 0;
          }
        }

        iRunTime = p_oSimManager.RunSim( iNumTimesteps );
        //If the run was finished, exit; this way, if the user is treating
        //this strictly as command-line, SORTIE will do its run and finish.
        //If the user broke in and paused, then they will enter interactive
        //mode in the while loop below until they hit "run" some more
        if ( ( p_oSimManager.GetSimState() == clSimManager::Run_Complete
             && p_oSimManager.GetCurrentTimestep() == p_oSimManager.GetNumberOfTimesteps() )
             || p_oSimManager.GetSimState() == clSimManager::No_Data )
             {
               stringstream msg;
               msg << p_oSimManager.GetCurrentTimestep() << "," << p_oSimManager.GetNumberOfTimesteps() << "," << iRunTime;
               stcMsg.iMessageCode = RUN_COMPLETE;
               stcMsg.sMoreInfo = msg.str();
               SendMessage( stcMsg );

               return 0;
        }
      }
      else
      {
        cout << "The file passed to SORTIE is not of a valid type.  Exiting.\n";
        //An error occurred - exit
        return 0;
      }
    } //end of try block
    catch ( modelErr & err )
    {
      ExternalErrorHandler( err, p_oSimManager.GetSimState(), false );
      //An error occurred - exit
      return 0;
    }

  } //end of if (2 == argc || 3 == argc)
  else
  {
    //More arguments - quit 'cause we don't know what's going on
    cerr << "Too many arguments.\n";
    return 0;
  }


  ////////////////////////////////////////////////////////////
  // Main loop - this will accept and process commands
  // until the user wants to quit; this is automatically entered
  // in interface mode, and can be entered in command-line
  // mode if the user paused
  ////////////////////////////////////////////////////////////
  while ( 1 )
  {
    stcMsg.iMessageCode = UNKNOWN;


    try
    {
      stcMsg = GetMessage();

      if ( QUIT == stcMsg.iMessageCode ) {
        stringstream msg;
        msg << p_oSimManager.GetCurrentTimestep() << "," << p_oSimManager.GetNumberOfTimesteps() << "," << iRunTime;
        stcMsg.iMessageCode = RUN_COMPLETE;
        stcMsg.sMoreInfo = msg.str();
        SendMessage( stcMsg );
        return 0;
      }

      //parse out any additional arguments if not "input" - skip input because
      //filenames may have spaces and won't have any additional args
      if ( INPUT_FILE == stcMsg.iMessageCode )
      {
        strArg1 = stcMsg.sMoreInfo;
        strArg2 = "";
      }
      else
      {
        strArg1 = stcMsg.sMoreInfo;
        pos = strArg1.find_first_of( " " );
        if ( string::npos != pos )
        {
          strArg2 = strArg1.substr( pos + 1 );
          strArg1 = strArg1.substr( 0, pos );
        }
        else
          strArg2 = "";
      }

      /** Input command */
      if ( INPUT_FILE == stcMsg.iMessageCode )
      {

        //Make sure we got a file name
        if ( "" == strArg1 )
        {
          stcError.iErrorCode = NEED_FILE;
          stcError.sFunction = "main";
          ExternalErrorHandler( stcError, p_oSimManager.GetSimState(), false );
        }
        else
        {
          p_oSimManager.ReadFile( ( char * ) strArg1.c_str() );

          //Let the user know the status of the model if ready
          if ( clSimManager::Initialized == p_oSimManager.GetSimState() )
          {
            stcMsg.iMessageCode = MODEL_READY;
            SendMessage( stcMsg );
          }
          else
          {
            stcMsg.iMessageCode = MODEL_NOT_READY;
            SendMessage( stcMsg );
          }
        } //end of else
      } //end of if ("input" = strCommand)

      /** Run command */
      else if ( RUN == stcMsg.iMessageCode )
      {
        //Make sure we're ready
        if ( clSimManager::No_Data == p_oSimManager.GetSimState() )
        {
          stcError.iErrorCode = MODEL_NOT_READY;
          ExternalErrorHandler( stcError, p_oSimManager.GetSimState(), false );
        }
        else
        {

          //Are there any timesteps?  If not don't pass anything
          if ( "" == strArg1 )
          {
            iRunTime = p_oSimManager.RunSim();
            if (p_oSimManager.GetBatchNumber() != 0) {
              p_oSimManager.RunBatch(NULL);
            }
            if ( 2 == argc || 3 == argc )
            {
              //If we're in batch mode and the run is finished, exit
              if ( ( p_oSimManager.GetSimState() == clSimManager::Run_Complete
                   && p_oSimManager.GetCurrentTimestep() == p_oSimManager.GetNumberOfTimesteps()
                   && p_oSimManager.GetBatchNumber() == 0)
                   || p_oSimManager.GetSimState() == clSimManager::No_Data )
                   {
                     timer = time( NULL );
                     stcTime = localtime( & timer );
                     cout << "\n\nRun ended on:  " << asctime( stcTime ) << "\n\n";
                     //flushall();
                     return 0;
              }
            }
            if ( p_oSimManager.GetSimState() == clSimManager::Run_Complete )
            {
              stringstream msg;
              msg << p_oSimManager.GetCurrentTimestep() << "," << p_oSimManager.GetNumberOfTimesteps() << "," << iRunTime;
              stcMsg.iMessageCode = RUN_COMPLETE;
              stcMsg.sMoreInfo = msg.str();
              SendMessage( stcMsg );
            }
          }
          else
          {
            iNumTimesteps = atoi( strArg1.c_str() );
            //If it equals 0 make sure that's what the user meant - else
            //return an error
            if ( 0 == iNumTimesteps && strArg1.length() > 1 )
            {
              stcMsg.iMessageCode = BAD_ARGUMENT;
              stcMsg.sMoreInfo = "Couldn't read number of timesteps";
              SendMessage( stcMsg );
            }
            else
            {
              iRunTime = p_oSimManager.RunSim( iNumTimesteps );
              if ( 2 == argc || 3 == argc )
              {
                //If we're in batch mode and the run is finished, exit
                if ( ( p_oSimManager.GetSimState() == clSimManager::Run_Complete
                     && p_oSimManager.GetCurrentTimestep() == p_oSimManager.GetNumberOfTimesteps() )
                     || p_oSimManager.GetSimState() == clSimManager::No_Data )
                     {
                       timer = time( NULL );
                       stcTime = localtime( & timer );
                       cout << "\n\nRun ended on:  " << asctime( stcTime ) << "\n\n";
                       //flushall();
                       return 0;
                }
              }
              if ( p_oSimManager.GetSimState() == clSimManager::Run_Complete )
              {
                stringstream msg;
                msg << p_oSimManager.GetCurrentTimestep() << "," << p_oSimManager.GetNumberOfTimesteps() << "," << iRunTime;
                stcMsg.iMessageCode = RUN_COMPLETE;
                stcMsg.sMoreInfo = msg.str();
                SendMessage( stcMsg );
              }
            }
          }
        }
      }
      /** Other (hopefully registered) command */
      else
      {
        stcMsg.iMessageCode = BAD_COMMAND;
        SendMessage( stcMsg );
      } //end of else
    } //end of try block
    catch ( modelErr & err )
    {
      ExternalErrorHandler( err, p_oSimManager.GetSimState(), false );
    }
    catch ( modelMsg & msg )
    {
      SendMessage( msg );
    }

  } //end of while (1)
  return 0;
}
//---------------------------------------------------------------------------
