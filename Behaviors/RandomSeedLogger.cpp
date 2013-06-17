//---------------------------------------------------------------------------
#include "RandomSeedLogger.h"
#include "SimManager.h"
#include <fstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor()
// Edit history:
// -------------
// April 28, 2004 - Submitted in beta version (LEM)
//////////////////////////////////////////////////////////////////////////////
clRandomSeedLogger::clRandomSeedLogger(clSimManager *p_oSimManager) :
     clWorkerBase( p_oSimManager), clBehaviorBase(p_oSimManager) {

  //Allowed file types
  m_iNumAllowedTypes = 2;
  mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
  mp_iAllowedFileTypes[0] = parfile;
  mp_iAllowedFileTypes[1] = detailed_output;

  //Versions
  m_fVersionNumber = 1;
  m_fMinimumVersionNumber = 1;
}
//---------------------------------------------------------------------------

/*/////////////////////////////////////////////////////////////////////////////
GetData()
// Edit history:
// -------------
// April 28, 2004 - Submitted in beta version (LEM)

/////////////////////////////////////////////////////////////////////////////*/
void clRandomSeedLogger::GetData(DOMDocument *p_oDoc) {

  using namespace std;
  fstream log("RandomSeed.txt", ios::out | ios::app);
  log << "\n-----------------------------------------------------\n"
      << "Actual random seed for parameter file "
      << mp_oSimManager->GetParFilename() << " is "
      << *mp_oSimManager->GetRandomSeed() << ".\n";
  log.close();
}
//---------------------------------------------------------------------------
