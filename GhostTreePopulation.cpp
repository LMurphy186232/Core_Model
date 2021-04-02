#include <stddef.h>
#include <math.h>
#include "GhostTreePopulation.h"
#include "Tree.h"
#include "DeadTree.h"
#include "SimManager.h"
#include "ModelMath.h"


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clGhostTreePopulation::~clGhostTreePopulation() {
  TimestepCleanup();
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clGhostTreePopulation::clGhostTreePopulation(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clPopulationBase(p_oSimManager) {

  m_sNameString = "GhostTreePopulation";

  mp_oTrees = NULL;

  //Allowed file types
  m_iNumAllowedTypes = 5;
  mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
  mp_iAllowedFileTypes[0] = parfile;
  mp_iAllowedFileTypes[1] = tree;
  mp_iAllowedFileTypes[2] = treemap;
  mp_iAllowedFileTypes[3] = detailed_output_timestep;
  mp_iAllowedFileTypes[4] = detailed_output;
}

///////////////////////////////////////////////////////////////////////////
// AddTree()
///////////////////////////////////////////////////////////////////////////
void clGhostTreePopulation::AddTree(clTree * p_oTree, deadCode iDeadReasonCode) {
  clDeadTree *p_oDeadClone = p_oTree->MakeDeadClone();
             //*p_oNextTree = NULL, *p_oTreeHolder = NULL;

  p_oDeadClone->SetDeadReasonCode(iDeadReasonCode);
  p_oDeadClone->mp_oNext = mp_oTrees;
  mp_oTrees = p_oDeadClone;

  //Go through that grid cell and insert the tree in the right place
  /*if (NULL == mp_oTrees) {
    mp_oTrees = p_oDeadClone;
  } else {
    p_oNextTree = mp_oTrees;
    p_oTreeHolder = p_oNextTree->GetNext();
    while (p_oTreeHolder != NULL) {
      p_oNextTree = p_oTreeHolder;
      p_oTreeHolder = p_oNextTree->GetNext();
    }
    p_oNextTree->mp_oNext = p_oDeadClone;
    p_oDeadClone->mp_oNext = NULL;
  }*/
}

//////////////////////////////////////////////////////////////////////////////
// TimestepCleanup()
//////////////////////////////////////////////////////////////////////////////
void clGhostTreePopulation::TimestepCleanup() {

  clDeadTree * p_oTree = NULL, *p_oNextTree = NULL;

  p_oTree = mp_oTrees;
  while (p_oTree != NULL) {
    p_oNextTree = p_oTree->GetNext();
    delete p_oTree;
    p_oTree = p_oNextTree;
  }

  mp_oTrees = NULL;
}
