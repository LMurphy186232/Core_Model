
#ifndef MESSAGESH
#define MESSAGESH
#include <string>
//---------------------------------------------------------------------------
/**
* @file Messages.h
* ERROR AND MESSAGE PASSING
* This defines a set of codes that can be used to pass errors and messages.
*
* The modelErr structure is used to pass fatal errors.  These errors stop
* processing and cause the erasure of all data.  This puts the model in a state
* ready to be initialized once again.
*
* The modelMsg structure is used to pass non-fatal errors and messages. These
* stop processing but do not cause data erasure.  An object passing one of
* these messages is responsible for internal data cleanup.
*
* Copyright 2003 Charles D. Canham.
*
* <br/>Edit history:
* <br/>------------------------
* <br/>10/26/2012: Changed to std::strings from c chars (LEM)
*/

/**This is the structure for sending and receiving messages*/
struct modelMsg {
  int iMessageCode;             /**<Integer message code.  See constants.*/
  std::string sMoreInfo, /**<String to hold additional information, if
                                needed, according to the message code*/
              sFunction; /**<Function in which the error occurred -
                                probably not for user consumption*/
};

/**Error reporting structure*/
struct modelErr {
  int iErrorCode;               /**<Error code.  See constants.*/
  std::string sMoreInfo, /**<String to hold additional information, if
                                needed, according to the error code*/
              sFunction; /**<Function in which the error occurred -
                                probably not for user consumption*/
};

//*****************************************
// Message and error codes - which these are
// depends on the structure they are passed in
//*****************************************

const int UNKNOWN =         0;  /**<Message of unknown type*/
const int NO_MESSAGE =      1;  /**<No message to pass*/

//Commands - these will probably never be used as errors
const int PAUSE_RUN =       2;  /**<Request to pause run*/
const int RUN =             3;  /**<Request to run model*/
const int INPUT_FILE =      4;  /**<Request to input file*/
const int QUIT =            5;  /**<Request to quit model*/

//For communicating model status
const int MODEL_READY =     6;  /**<Model is in ready state*/
const int MODEL_NOT_READY = 7;  /**<Model is not in ready state*/
const int MODEL_PAUSED =    8;  /**<Model is paused*/
const int RUN_COMPLETE =    9;  /**<Requested run is complete*/
const int COMMAND_DONE =    10; /**<Model is finished executing command*/
const int INFO =            11; /**<Message in the cMoreInfo string should be
                                passed to the user*/

//Runtime error codes
const int BAD_ARGUMENT =    12; /**<A bad or missing argument has been passed
                                from the interface for a command*/
const int BAD_COMMAND =     13; /**<A bad command has been passed from the
                                interface*/
const int CANT_FIND_OBJECT= 14; /**<An object could not be found*/
const int TREE_WRONG_TYPE = 15; /**<A tree was not of an expected type*/
const int ACCESS_VIOLATION= 16; /**<An access violation occurred*/

//File error codes
const int BAD_FILE =        17; /**<Bad file name or path - couldn't
                                open the file*/
const int BAD_FILE_TYPE =   18; /**<A file was not an expected type*/
const int BAD_XML_FILE =    19; /**<XML file is malformed or invalid*/
const int NEED_FILE =       20; /**<Expected file name and didn't get one*/

//Data errors - add'l data should have name of data piece
const int DATA_MISSING =    21; /**<Couldn't find needed data in file*/
const int BAD_DATA =        22; /**<Data was scrambled, of incorrect type, or
                                otherwise invalid*/
const int DATA_READ_ONLY =  23; /**<Data isn't accessible for writing*/
const int ILLEGAL_OP =      24; /**<Illegal operation*/

//---------------------------------------------------------------------------
#endif
