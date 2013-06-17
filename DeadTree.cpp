//---------------------------------------------------------------------------
#include <stdio.h>
#include "DeadTree.h"
#include "TreePopulation.h"
//---------------------------------------------------------------------------
using namespace std;

////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////*/
clDeadTree::clDeadTree(int iType, int iSpecies, int iNumFloats, int iNumInts,
    int iNumStrings, int iNumBools) {
  int i;

  m_iSpecies = iSpecies;
  m_iType = iType;

  mp_oNext = NULL;

  //Null out the pointers
  mp_fFloatValues = NULL;
  mp_iIntValues = NULL;
  mp_sStringValues = NULL;
  mp_bBoolValues = NULL;

  if (iNumFloats > 0) {
    mp_fFloatValues = new float[iNumFloats];
    for (i = 0; i < iNumFloats; i++)
      mp_fFloatValues[i] = 0;
  }
  if (iNumInts > 0) {
    mp_iIntValues = new int[iNumInts];
    for (i = 0; i < iNumInts; i++)
      mp_iIntValues[i] = 0;
  }
  if (iNumStrings > 0) {
    mp_sStringValues = new string[iNumStrings];
  }
  if (iNumBools > 0) {
    mp_bBoolValues = new bool[iNumBools];
    for (i = 0; i < iNumBools; i++)
      mp_bBoolValues[i] = false;
  }

  m_iDeadCode = notdead;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clDeadTree::~clDeadTree() {
  //Delete all arrays
  delete[] mp_fFloatValues;
  delete[] mp_iIntValues;
  delete[] mp_sStringValues;
  delete[] mp_bBoolValues;
}

////////////////////////////////////////////////////////////////////////////
// GetValue
///////////////////////////////////////////////////////////////////////////*/
void clDeadTree::GetValue(short int iCode, int *p_iValHolder) {
  *p_iValHolder = mp_iIntValues[iCode];
}
//---------------------------------------------------------------------------
void clDeadTree::GetValue(short int iCode, float *p_fValHolder) {
  *p_fValHolder = mp_fFloatValues[iCode];
}
//---------------------------------------------------------------------------
void clDeadTree::GetValue(short int iCode, bool *p_bValHolder) {
  *p_bValHolder = mp_bBoolValues[iCode];
}
//---------------------------------------------------------------------------
void clDeadTree::GetValue(short int iCode, string *p_sValHolder) {
  *p_sValHolder = mp_sStringValues[iCode];
}
//---------------------------------------------------------------------------
