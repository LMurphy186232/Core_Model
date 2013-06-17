//---------------------------------------------------------------------------
#include "MortalityOrg.h"
#include "SimManager.h"
#include "MortalityBase.h"
#include "TreePopulation.h"
#include <stdio.h>
#include <fstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clMortalityOrg::clMortalityOrg(clMortalityBase *p_oHookedShell) {
  //Initialize variables to 0, NULL, etc.
  mp_oMortShellList = NULL;
  mp_iDeadCodes = NULL;
  mp_bUsesMortality = NULL;
  mp_oPop = NULL;

  m_iTotalSpecies = 0;
  m_iTotalTypes = 0;
  m_iNumMortShells = 0;

  //Make sure the hooked shell object pointer isn't NULL - if it is throw
  //error
  if (NULL == p_oHookedShell) {
    modelErr stcErr;
    stcErr.sFunction = "clMortalityOrg::clMortalityOrg";
    stcErr.sMoreInfo = "Null pointer was passed to the function.";
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    throw(stcErr);
  }

  //Otherwise - set the flag on the shell object that it's hooked
  p_oHookedShell->m_bHooked = true;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clMortalityOrg::~clMortalityOrg() {
  int i; //loop counter

  if (mp_bUsesMortality) {
    for (i = 0; i < m_iTotalSpecies; i++) {
      delete[] mp_bUsesMortality[i];
    }
  }
  if (mp_iDeadCodes) {
    for (i = 0; i < m_iTotalSpecies; i++) {
      delete[] mp_iDeadCodes[i];
    }
  }
  delete[] mp_bUsesMortality;
  delete[] mp_iDeadCodes;
  delete[] mp_oMortShellList;
}

//////////////////////////////////////////////////////////////////////////////
// UpdateDataMemberRegistrations()
//////////////////////////////////////////////////////////////////////////////
void clMortalityOrg::UpdateDataMemberRegistrations(clSimManager *p_oSimManager, clMortalityBase *p_oHooked) {
  stcSpeciesTypeCombo *p_whatCombos;
  short int i, j, iNumCombos = 0;

  //Get the tree population object, and from it the numbers of species and types
  mp_oPop = (clTreePopulation*) p_oSimManager->GetPopulationObject("treepopulation");

  m_iTotalSpecies = mp_oPop->GetNumberOfSpecies();
  m_iTotalTypes = mp_oPop->GetNumberOfTypes();

  //Assemble the list of mortality shells
  AssembleMortShellList(p_oSimManager);

  //Cause each shell to populate its own mortality use table
  for (i = 0; i < m_iNumMortShells; i++) {
    mp_oMortShellList[i]->PopulateUsesThisMortality();
  }

  //Populate the table of species/type combos that use any kind of mortality
  PopulateUsesMortality();

  //Clear the existing list of species/type combos from the hooked mortality object
  delete[] p_oHooked->mp_whatSpeciesTypeCombos;
  p_oHooked->mp_whatSpeciesTypeCombos = NULL;
  delete[] p_oHooked->mp_iWhatSpecies;
  p_oHooked->mp_iWhatSpecies = NULL;

  //Assemble the species/type combo array
  for (i = 0; i < m_iTotalSpecies; i++) {
    for (j = 0; j < m_iTotalTypes; j++)
      if(mp_bUsesMortality[i][j]) iNumCombos++;
  }

  p_whatCombos = new stcSpeciesTypeCombo[iNumCombos];
  iNumCombos = 0;
  for (i = 0; i < m_iTotalSpecies; i++) {
    for (j = 0; j < m_iTotalTypes; j++)
      if(mp_bUsesMortality[i][j]) {
        p_whatCombos[iNumCombos].iSpecies = i;
        p_whatCombos[iNumCombos].iType = j;
        iNumCombos++;
      }
  }
  p_oHooked->SetSpeciesTypeCombos(iNumCombos, p_whatCombos);
  p_oHooked->m_iNewTreeInts = 1;
  delete[] p_whatCombos;

}


//////////////////////////////////////////////////////////////////////////////
// DoDataMemberRegistrations
//////////////////////////////////////////////////////////////////////////////
void clMortalityOrg::DoDataMemberRegistrations(clMortalityBase *p_oHooked) {
  stcSpeciesTypeCombo *p_whatCombos;
  char cLabel[] = "dead";
  short int i, j, //loop counters
  iNumCombos = 0;

  //Declare and initialize the return codes array
  mp_iDeadCodes = new short int*[m_iTotalSpecies];
  for (i = 0; i < m_iTotalSpecies; i++) {
    mp_iDeadCodes[i] = new short int[m_iTotalTypes];
    for (j = 0; j < m_iTotalTypes; j++)
      mp_iDeadCodes[i][j] = -1;
  }

  //Now go through the growth functions table and get the code for
  //each species/type combo with a valid pointer
  for (i = 0; i < m_iTotalSpecies; i++)
    for (j = 0; j < m_iTotalTypes; j++)
      if (mp_bUsesMortality[i][j]) {
        mp_iDeadCodes[i][j] = mp_oPop->RegisterInt(cLabel, i, j);
      }

  //Now reset the hooked object's species/type combos
  //Clear the existing list of species/type combos from the hooked mortality object
  delete[] p_oHooked->mp_whatSpeciesTypeCombos;
  p_oHooked->mp_whatSpeciesTypeCombos = NULL;
  delete[] p_oHooked->mp_iWhatSpecies;
  p_oHooked->mp_iWhatSpecies = NULL;

  //Assemble the species/type combo array
  for (i = 0; i < m_iTotalSpecies; i++) {
    for (j = 0; j < m_iTotalTypes; j++)
      if(p_oHooked->mp_bUsesThisMortality[i][j]) iNumCombos++;
  }

  p_whatCombos = new stcSpeciesTypeCombo[iNumCombos];
  iNumCombos = 0;
  for (i = 0; i < m_iTotalSpecies; i++) {
    for (j = 0; j < m_iTotalTypes; j++)
      if(p_oHooked->mp_bUsesThisMortality[i][j]) {
        p_whatCombos[iNumCombos].iSpecies = i;
        p_whatCombos[iNumCombos].iType = j;
        iNumCombos++;
      }
  }
  p_oHooked->SetSpeciesTypeCombos(iNumCombos, p_whatCombos);
  delete[] p_whatCombos;

}

//////////////////////////////////////////////////////////////////////////////
// AssembleMortShellList()
//////////////////////////////////////////////////////////////////////////////
void clMortalityOrg::AssembleMortShellList(clSimManager *p_oSimManager) {
  clBehaviorBase *p_oTempBehavior; //for going through behaviors looking for
  //mortality shells
  clMortalityBase *p_oMortShell;   //mortality shell object
  std::string sBehaviorName, //behavior namestrings
              sShellMarker = "mortshell";
  short int iNumBehaviors,     //total number of shell behaviors
  i, j;              //loop counters

  m_iNumMortShells = 0;

  //Go through and count all the mortality shell behaviors first
  iNumBehaviors = p_oSimManager->GetNumberOfBehaviors();
  for (i = 0; i < iNumBehaviors; i++) {
    p_oTempBehavior = p_oSimManager->GetBehaviorObject(i);
    sBehaviorName = p_oTempBehavior->GetName();
    if (std::string::npos != sBehaviorName.find(sShellMarker))
      m_iNumMortShells++;
  }

  //Declare the list array
  mp_oMortShellList = new clMortalityBase*[m_iNumMortShells];
  for (i = 0; i < m_iNumMortShells; i++)
    mp_oMortShellList[i] = NULL;

  //Now go through all the behaviors again and get pointers to the shells now
  j = 0;
  for (i = 0; i < iNumBehaviors; i++) {
    p_oTempBehavior = p_oSimManager->GetBehaviorObject(i);
    sBehaviorName = p_oTempBehavior->GetName();
    if (std::string::npos != sBehaviorName.find(sShellMarker)) {

      //This is a mortality shell object - cast its pointer as such
      p_oMortShell = dynamic_cast<clMortalityBase*>(p_oTempBehavior);

      mp_oMortShellList[j] = p_oMortShell;
      j++;
    } //end of if (NULL != strstr(cBehaviorName, cShellMarker))
  } //end of for (i = 0; i < iNumBehaviors; i++)
}

//////////////////////////////////////////////////////////////////////////////
// PopulateUsesMortality()
//////////////////////////////////////////////////////////////////////////////
void clMortalityOrg::PopulateUsesMortality() {
  stcSpeciesTypeCombo combo; //species/type combo from a behavior
  short int iNumCombos,      //number of species/type combos for a behavior
  i, j; //loop counters

  //Declare the uses mortality table and set all flags to false
  mp_bUsesMortality = new bool*[m_iTotalSpecies];
  for (i = 0; i < m_iTotalSpecies; i++) {
    mp_bUsesMortality[i] = new bool[m_iTotalTypes];
    for (j = 0; j < m_iTotalTypes; j++)
      mp_bUsesMortality[i][j] = false;
  }

  //Go through the mortality shells and for each species/type combo that
  //the shell claims, set the flag to true
  for (i = 0; i < m_iNumMortShells; i++) {
    iNumCombos = mp_oMortShellList[i]->GetNumSpeciesTypeCombos();
    for (j = 0; j < iNumCombos; j++) {
      combo = mp_oMortShellList[i]->GetSpeciesTypeCombo(j);
      mp_bUsesMortality[combo.iSpecies][combo.iType] = true;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// DoMortality()
//////////////////////////////////////////////////////////////////////////////
void clMortalityOrg::DoMortality() {
  clTreeSearch *p_oAllTrees;    //search object for getting all trees
  clTree *p_oTree;              //for working with a single tree
  float fDbh;                   //dbh of tree (0 for seedlings)
  int iDeadCode;                //value for "dead" data member
  short int iSp, iType,         //species and type of a given tree
  i;                  //loop counter

  //Call the PreMortCalcs() function for all shells
  for (int i = 0; i < m_iNumMortShells; i++) {
    mp_oMortShellList[i]->PreMortCalcs( mp_oPop );
  }

  //Ask the tree population to find all trees
  p_oAllTrees = mp_oPop->Find("all");

  //Go through the trees one at a time
  p_oTree = p_oAllTrees->NextTree();
  while (p_oTree) {

    //Get the tree's species and type
    iSp = p_oTree->GetSpecies();
    iType = p_oTree->GetType();

    if (iType == clTreePopulation::seedling)
      fDbh = 0;
    else
      p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, iType), &fDbh);

    //If this species/type combo gets mortality applied, put it through the
    //mortality shell gauntlet of each shell that applies to it
    if (mp_bUsesMortality[iSp][iType]) {
      p_oTree->GetValue(mp_iDeadCodes[iSp][iType], &iDeadCode);
      for (i = 0; i < m_iNumMortShells; i++) {
        if (iDeadCode == notdead &&
            mp_oMortShellList[i]->mp_bUsesThisMortality[iSp][iType]) {
          iDeadCode = mp_oMortShellList[i]->DoMort(p_oTree, fDbh, iSp);
          p_oTree->SetValue(mp_iDeadCodes[iSp][iType], iDeadCode);
        }
      }
    }

    p_oTree = p_oAllTrees->NextTree();
  } //end of while (p_oTree)
}
