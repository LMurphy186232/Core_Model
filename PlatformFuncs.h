//---------------------------------------------------------------------------

#ifndef PlatformFuncsH
#define PlatformFuncsH
//---------------------------------------------------------------------------
#include <string>
using namespace std;

/**
* @file PlatformFuncs.h
* These are functions which use platform-specific code.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>April 28, 2004 - Submitted as beta (LEM)
* <br>June 20, 2007 - New Linux versions of the functions, based on code
* originally written by Daniel Lipsitt, to whom great thanks is owed (LEM)
*/

 /**
 * Verifies that gzip and tar are where we expect them to be (in the same
 * directory as the executable).  This should be called before either of the
 * functions below and a fatal error thrown if it returns false.
 * For Linux, this function always returns true.
 *
 * @param sAppPath Path to the main application, ending with the path separator.
 * The executables are expected to be in the same path.
 * @return true if the executables are where they should be. Otherwise, false.
 * (For Linux always true.)
 */
 bool TarballSetup(string sAppPath);

 /**
 * Zips a file with GZIP.  If the file does not exist, the function does
 * nothing.
 *
 * @param sFileName File to zip.
 * @param sAppPath Path to the main application, ending with the path separator.
 * For Windows, the GZIP executable is expected to be in the same path. For
 * Linux, gzip is expected to be somewhere in the user's PATH.
 */
 void ZipFile(string sFileName, string sAppPath);

 /**
 * Adds a file to the end of a tarball.  If the tarball does not exist, it will
 * be created.  The file to add can be either zipped or not zipped.
 *
 * @param sTarball File name of tarball
 * @param sFileToAdd File name of the file to add to the tarball
 * @param sAppPath Path to the main application, ending with the path separator.
 * The TAR executable is expected to be in the same path. For
 * Linux, tar is expected to be somewhere in the user's PATH.
 */
 void AddFileToTarball(string sTarball, string sFileToAdd, string sAppPath);

 /**
 * Creates a new tarball and adds a file to it.  Any existing tarball of that
 * name will be overwritten.  If the tarball does not exist, it will
 * be created.  The file to add can be either zipped or not zipped.
 *
 * @param sTarball File name of tarball
 * @param sFileToAdd File name of the file to add to the tarball
 * @param sAppPath Path to the main application, ending with the path separator.
 * The TAR executable is expected to be in the same path. For
 * Linux, tar is expected to be somewhere in the user's PATH.
 */
 void AddFileToNewTarball(string sTarball, string sFileToAdd, string sAppPath);


 /**
  * Deletes a file.
  * @param sFile File name of the file to delete
  */
 void DeleteThisFile(string sFile);

 /**
  * Checks for the existence of a file
  * @param sFile File name of the file to check for existence
  * @return True if the file exists, false if it doesn't
  */
 bool DoesFileExist(string sFile);

 /**
  * Launches a process and waits for it to finish.
  * @param sFile Executable to launch
  * @param sArgs Arguments to pass to process
  * @param sDir Working directory
  */
 void LaunchProcess(string sFile, string sArgs, string sDir);

 /**
  * Finds all occurrences of one string within another and replaces them.
  * @param source String to search and replace within
  * @param find String to find
  * @param replace String to replace with
  */
 void ReplaceAll( std::string &source, const std::string &find,
       const std::string& replace );

//---------------------------------------------------------------------------
#endif
