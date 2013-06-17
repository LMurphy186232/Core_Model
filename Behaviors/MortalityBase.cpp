//---------------------------------------------------------------------------
#include "MortalityBase.h"
#include "MortalityOrg.h"

clMortalityOrg *clMortalityBase::mp_oMortalityOrg = NULL;

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////*/
clMortalityBase::clMortalityBase(clSimManager *p_oSimManager) :
             clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ){
  //Set the hooked flag to false
  m_bHooked = false;

  //Now - see if the growth org object is NULL.  If it is, create it and pass
  //a self pointer so this object will be hooked
  if (mp_oMortalityOrg == NULL)
    mp_oMortalityOrg = new clMortalityOrg(this);

  //Allowed file types
  m_iNumAllowedTypes = 2;
  mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
  mp_iAllowedFileTypes[0] = parfile;
  mp_iAllowedFileTypes[1] = detailed_output;

  //Versions
  m_fVersionNumber = 1;
  m_fMinimumVersionNumber = 1;

  mp_bUsesThisMortality = NULL;
  m_iNumTotalSpecies = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clMortalityBase::~clMortalityBase() {
  if (m_bHooked) {
    delete mp_oMortalityOrg;
    mp_oMortalityOrg = NULL;
  }

  for (int i = 0; i < m_iNumTotalSpecies; i++)
    delete[] mp_bUsesThisMortality[i];

  delete[] mp_bUsesThisMortality;
}

//////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
/////////////////////////////////////////////////////////////////////////////*/
void clMortalityBase::RegisterTreeDataMembers() {
  if (m_bHooked)
    mp_oMortalityOrg->DoDataMemberRegistrations(this);
}


//////////////////////////////////////////////////////////////////////////////
// GetNewTreeInts()
//////////////////////////////////////////////////////////////////////////////
short int clMortalityBase::GetNewTreeInts() {
  if (m_bHooked) {
    mp_oMortalityOrg->UpdateDataMemberRegistrations(mp_oSimManager, this);
  }
  return m_iNewTreeInts;
}

//////////////////////////////////////////////////////////////////////////////
// PopulateUsesThisMortality()
/////////////////////////////////////////////////////////////////////////////*/
void clMortalityBase::PopulateUsesThisMortality() {

  int iNumberTypes = mp_oMortalityOrg->GetNumberOfTypes(), i, j;
  m_iNumTotalSpecies = mp_oMortalityOrg->GetNumberOfSpecies();

  //Populate the table for which species/type combos use this shell
  mp_bUsesThisMortality = new bool*[m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_bUsesThisMortality[i] = new bool[iNumberTypes];
    for (j = 0; j < iNumberTypes; j++)
      mp_bUsesThisMortality[i][j] = false;
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++)
    mp_bUsesThisMortality[mp_whatSpeciesTypeCombos[i].iSpecies]
                          [mp_whatSpeciesTypeCombos[i].iType] = true;
}

//////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////*/
void clMortalityBase::GetData(xercesc::DOMDocument *p_oDoc) {
  DoShellSetup(p_oDoc);
}

//////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////*/
void clMortalityBase::Action() {

  //If this is a hooked object, call the growth org object.  Otherwise, do
  //nothing.
  if (m_bHooked)
    mp_oMortalityOrg->DoMortality();
}
