//---------------------------------------------------------------------------
#include "PlatformFuncs.h"
#include <stdio.h>
#include "Constants.h"

#ifdef linux
//Linux defines
#include <stdlib.h>
#include <string.h>
#include <string>
#else
//Windows defines
#include <io.h>
#include "Messages.h"
#include <windows.h>
#include <shellapi.h>
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////
// TarballSetup
////////////////////////////////////////////////////////////////////////////
#ifdef linux
//Linux version
bool TarballSetup(string sAppPath) {
    return true;
}
#else
//Windows version
bool TarballSetup(string sAppPath) {
  string sPath = sAppPath + "gzip.exe";
  int iAvailable = access(sPath.c_str(), 0); //0 signifies checking for existence
  if (0 == iAvailable) {
    sPath = sAppPath + "tar.exe";
    if (0 == iAvailable) 
      return true;
    else 
      return false;
  } else
    return false;
} 
#endif

////////////////////////////////////////////////////////////////////////////
// ZipFile
////////////////////////////////////////////////////////////////////////////
void ZipFile(string sFileName, string sAppPath) {
  FILE *filetest;  //only this kind of file can be tested for existence of a
                   //file without creating

  //Does the file exist?  If not, exit without error
  filetest = fopen(sFileName.c_str(), "r");
  if (filetest == NULL) return;
  else fclose(filetest);

  // gzip flags - -f is force, which will overwrite
  string sArgs = " -f -q \"" + sFileName + "\"";

  LaunchProcess("gzip", sArgs, sAppPath);
  /// @todo check error
}

////////////////////////////////////////////////////////////////////////////
// UnzipFile
////////////////////////////////////////////////////////////////////////////
void UnzipFile(string sFileName) {
  FILE *filetest;  //only this kind of file can be tested for existence of a
                   //file without creating
  //Does the file exist?  If not, exit without error
  filetest = fopen(sFileName.c_str(), "r");
  if (filetest == NULL) return;
  else fclose(filetest);

  //gzip flags - -f is force, which will overwrite existing files, -q is 
  //quiet, which suppresses warnings, and -d is the uncompress flag
  string sArgs = "-f -q -d \"" + sFileName + "\"";

  LaunchProcess("gzip", sArgs, "");
}

////////////////////////////////////////////////////////////////////////////
// AddFileToTarball
////////////////////////////////////////////////////////////////////////////
void AddFileToTarball(string sTarball, string sFileToAdd, string sAppPath) {
  FILE *filetest;  //only this kind of file can be tested for existence of a
                   //file without creating
  string sFlags;

  //Does the file to add exist?  If not, exit without error
  filetest = fopen(sFileToAdd.c_str(), "r");
  if (filetest == NULL) return;
  else fclose(filetest);
  
  //Make sure we feed only forward slashes to tar
  ReplaceAll(sTarball, "\\", "/");
  ReplaceAll(sFileToAdd, "\\", "/");

  //Set up the parameters for tar based on whether or not the tarball already
  //exists
  filetest = fopen(sTarball.c_str(), "r");
  if (filetest == NULL) {

    //The tarball doesn't exist - set up tar's flags to create
    //strcpy(cFlags, "--create --no-recursion --file ");   //-c means create - -f for file name
    sFlags = " -cf ";

  } else {
    //The tarball does exist - set up tar's flags to create
    //strcpy(cFlags, "--append --file ");  //-r means append - -f for file name
    sFlags = " -rf ";
    fclose(filetest);
  }

  string sArgs = sFlags + "\"" + sTarball + "\" \"" + sFileToAdd + "\"";

  LaunchProcess("tar", sArgs, sAppPath);
  /// @todo check error
}


////////////////////////////////////////////////////////////////////////////
// AddFileToTarball
////////////////////////////////////////////////////////////////////////////
void AddFileToNewTarball(string sTarball, string sFileToAdd, string sAppPath) {
  FILE *filetest;  //only this kind of file can be tested for existence of a
                   //file without creating

  //Make sure we feed only forward slashes to tar
  ReplaceAll(sTarball, "\\", "/");
  ReplaceAll(sFileToAdd, "\\", "/");

  //Does the file to add exist?  If not, exit without error
  filetest = fopen(sFileToAdd.c_str(), "r");
  if (filetest == NULL) return;
  else fclose(filetest);

  //Set up the parameters for tar to create the tarball
  string sArgs = " -cf\"" + sTarball + "\" \"" + sFileToAdd + "\"";

  LaunchProcess("tar", sArgs, sAppPath);
  /// @todo check error
}

////////////////////////////////////////////////////////////////////////////
// DeleteThisFile
////////////////////////////////////////////////////////////////////////////
#ifdef linux
//Linux version
void DeleteThisFile(string sFile) {
  //unlink(sFile.c_str());
  string sCmd;
  sCmd = "rm -f " + sFile;
  system(sCmd.c_str());
  /// @todo check error
}

#else
//Windows version
void DeleteThisFile(string sFile) {

  //DeleteFile(cFile);
  //While using the above code is preferable, for some reason Java crashes when
  //loading the DLL.  So we have to do things the hard way - through the shell.

  //This next part I'm copying from http://www.bridgespublishing.com/articles/
  //issues/9806/File_operations.htm
  // Create a char array and fill it with nulls.  This is because pFrom and
  //pTo have to be double-null terminated, so this will make sure of it.
  char src[MAX_PATH], dst[MAX_PATH];
  memset(src, 0, sizeof(src));
  memset(dst, 0, sizeof(dst));
  // Create a SHFILOPSTRUCT and zero it, too.
  SHFILEOPSTRUCT fos;
  memset(&fos, 0, sizeof(fos));

  //Set the fields of SHFILOPSTRUCT
  strcpy(src, sFile.c_str());


  fos.hwnd = NULL; //handle of parent window
  fos.wFunc = FO_DELETE; //operation to perform
  fos.pFrom = src; //pFrom - file to work on
  fos.pTo = dst; //pTo - destination
  fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION; //no dialog box and no asking
       //the user

  //Now perform the operation
  SHFileOperation(&fos); 

} 
#endif

////////////////////////////////////////////////////////////////////////////
// DoesFileExist
////////////////////////////////////////////////////////////////////////////
#ifdef linux
//Linux version
bool DoesFileExist(string sFile) {
  FILE *filetest; 
  filetest = fopen(sFile.c_str(), "r");
  if (filetest == NULL) return false;
  else fclose(filetest);
  return true;
}
#else
//Windows version
bool DoesFileExist(string sFile) {
  int iCode = access(sFile.c_str(), 0); //0 signifies checking for existence
  if (0 == iCode) 
    return true;
  else  
    return false;
} 
#endif

////////////////////////////////////////////////////////////////////////////
// LaunchProcess
////////////////////////////////////////////////////////////////////////////
#ifdef linux
//Linux version
void LaunchProcess(string sFile, string sArgs, string sDir) {
  string sCmd;
  
  if (sDir.length() > 0)  
    sCmd = sDir + "/" + sFile + " " + sArgs;
  else
    sCmd = sFile + " " + sArgs;

  system(sCmd.c_str());
}
#else
//Windows version
void LaunchProcess(string sFile, string sArgs, string sDir) {
  SHELLEXECUTEINFO *params;       //the data structure for running ShellExecuteEx
  
  params = new SHELLEXECUTEINFO;
  params->cbSize = sizeof(SHELLEXECUTEINFO); //size of this structure
  params->fMask = SEE_MASK_NOCLOSEPROCESS;  //flag to allow us to find out when
    //the process terminates
  params->lpVerb = 0; //what the executable should do - open (run)
  params->lpFile = sFile.c_str(); //path to executable
  params->lpParameters = sArgs.c_str(); //parameters passed
  params->lpDirectory = sDir.c_str();
  
  params->nShow = SW_HIDE;
  //params->nShow = SW_SHOW;
        
  //Run the executable
  if (ShellExecuteEx(params)) {
    
    //int iVal = (int)params->hInstApp;
    //if (SE_ERR_FNF == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_PNF == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_ACCESSDENIED == iVal) {
    // int i = 0;  
    //} else if (SE_ERR_OOM == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_DLLNOTFOUND == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_SHARE == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_ASSOCINCOMPLETE == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_DDETIMEOUT == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_DDEFAIL == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_DDEBUSY == iVal) {
    // int i = 0; 
    //} else if (SE_ERR_NOASSOC == iVal) {
    // int i = 0; 
    //}
    
    //DWORD dw = GetLastError();
    //force the process to complete before moving on
    WaitForSingleObject(params->hProcess, INFINITE);
  }
  delete params;
}   
#endif

////////////////////////////////////////////////////////////////////////////
// ReplaceAll
////////////////////////////////////////////////////////////////////////////
void ReplaceAll( std::string &source, const std::string &find, 
       const std::string& replace ) {
  size_t j;
  while ((j = source.find( find )) != std::string::npos)
  {
    source.replace( j, find.length(), replace );
  }
}
