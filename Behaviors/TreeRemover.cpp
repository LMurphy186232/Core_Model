//---------------------------------------------------------------------------
#include "TreeRemover.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include <stdio.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clTreeRemover::clTreeRemover(clSimManager *p_oSimManager) :
         clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ) {
 try {

   m_sNameString = "removedead";

   mp_iDeadCodes = NULL;
   m_iNumTotalSpecies = 0;
   mp_oPop = NULL;

   m_fVersionNumber = 3;
   m_fMinimumVersionNumber = 1;

   //Allowed file types
   m_iNumAllowedTypes = 2;
   mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
   mp_iAllowedFileTypes[0] = parfile;
   mp_iAllowedFileTypes[1] = detailed_output;
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clTreeRemover::clTreeRemover";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clTreeRemover::~clTreeRemover() {
  for (int i = 0; i < m_iNumTotalSpecies; i++)
    delete[] mp_iDeadCodes[i];

  delete[] mp_iDeadCodes;
}

////////////////////////////////////////////////////////////////////////////
// GetData()
////////////////////////////////////////////////////////////////////////////
void clTreeRemover::GetData(xercesc::DOMDocument *p_oDoc) {
 try {
   char cLabel[] = "dead";
   short int iNumTypes, //total number of types
             i, j;      //loop counters

   mp_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");

   m_iNumTotalSpecies = mp_oPop->GetNumberOfSpecies();
   iNumTypes = mp_oPop->GetNumberOfTypes();

   //Declare and initialize the return codes array
   mp_iDeadCodes = new short int*[m_iNumTotalSpecies];
   for (i = 0; i < m_iNumTotalSpecies; i++) {
     mp_iDeadCodes[i] = new short int[iNumTypes];
     for (j = 0; j < iNumTypes; j++)
       mp_iDeadCodes[i][j] = -1;
   }

   //Now get the code for each species/type combo this behavior applies to
   for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
     mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                 [mp_whatSpeciesTypeCombos[i].iType] =
       mp_oPop->GetIntDataCode(cLabel,
       mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType);

     //If the return code is -1, throw an error
     if (-1 == mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                           [mp_whatSpeciesTypeCombos[i].iType]) {
       modelErr stcErr;
       stcErr.sFunction = "clTreeRemover::GetData";
       std::stringstream s;
       s << "Type/species combo species="
         << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
         << mp_whatSpeciesTypeCombos[i].iType
         << " cannot be used with this behavior.";
       stcErr.iErrorCode = BAD_DATA;
       stcErr.sMoreInfo = s.str();
       throw(stcErr);
     }
   }
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clTreeRemover::GetData";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clTreeRemover::Action() {
 try {
   clTreeSearch *p_oAllTrees;  //for retrieving all trees
   clTree *p_oTree;            //a single tree to work with
   int iDeadValue;             //value in the "dead" data member
   short int iDeadCode;        //"dead" code for a single tree

   //Get all the trees
   p_oAllTrees = mp_oPop->Find("all");
   p_oTree = p_oAllTrees->NextTree();
   while (p_oTree) {

     //Use the existence of a valid dead code as a proxy for whether or not
     //to test for this tree's death
     iDeadCode = mp_iDeadCodes[p_oTree->GetSpecies()][p_oTree->GetType()];
     if (iDeadCode != -1) {

       p_oTree->GetValue(iDeadCode, &iDeadValue);

       if (iDeadValue != notdead) {
         //Ask the tree population to kill the tree
         p_oTree = mp_oPop->KillTree(p_oTree, (deadCode) iDeadValue);
         //If it became a snag, set it to notdead
         if (p_oTree && -1 != mp_oPop->GetIntDataCode("dead", p_oTree->GetSpecies(), p_oTree->GetType()))
           p_oTree->SetValue(mp_oPop->GetIntDataCode("dead", p_oTree->GetSpecies(), p_oTree->GetType()), notdead);
       }
     }

     p_oTree = p_oAllTrees->NextTree();
   }
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clTreeRemover::Action";
   throw(stcErr);
 }
}
