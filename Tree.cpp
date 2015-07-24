//---------------------------------------------------------------------------
#include <stdio.h>
#include <sstream>
#include "Tree.h"
#include "TreePopulation.h"
#include "DeadTree.h"
//---------------------------------------------------------------------------

clTreePopulation *clTree::mp_oTreePop = NULL;
using namespace std;

////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////*/
clTree::clTree(int iType, int iSpecies, int iNumFloats, int iNumInts,
    int iNumStrings, int iNumBools, clTreePopulation *p_oTreePop) {
  try {
    int i;

    //mp_oTreePop is static - it may already be initialized
    if (NULL == mp_oTreePop) mp_oTreePop = p_oTreePop;

    mp_oPrevious = NULL;
    mp_oNext = NULL;

    //Verify that type and species is okay
    if (iType<clTreePopulation::seed || iType>clTreePopulation::woody_debris) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTree::clTree";
      std::stringstream s;
      s << "Unrecognized tree type:  " << iType;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    if (iSpecies < 0 || iSpecies >= mp_oTreePop->m_iNumSpecies) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTree::clTree";
      std::stringstream s;
      s << "Unrecognized tree type:  " << iType;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    if (iNumFloats < 0 || iNumInts < 0 || iNumStrings < 0 || iNumBools < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTree::clTree";
      stcErr.sMoreInfo = "Data member numbers cannot be less than zero.";
      throw(stcErr);
    }

    m_iSpecies = iSpecies;
    m_iType = iType;

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
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTree::clTree";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clTree::~clTree() {
  //Delete all arrays
  delete[] mp_fFloatValues;
  delete[] mp_iIntValues;
  delete[] mp_sStringValues;
  delete[] mp_bBoolValues;
}

////////////////////////////////////////////////////////////////////////////
// SetValue
///////////////////////////////////////////////////////////////////////////*/
void clTree::SetValue(short int iCode, int iValue) {
  mp_oTreePop->UpdateTree(this, iCode, iValue);
}

////////////////////////////////////////////////////////////////////////////
// SetValue
///////////////////////////////////////////////////////////////////////////*/
void clTree::SetValue(short int iCode, float fValue, bool bUpdateNow, bool bUpdateAllometry) {
  mp_oTreePop->UpdateTree(this, iCode, fValue, bUpdateNow, bUpdateAllometry);
}

////////////////////////////////////////////////////////////////////////////
// SetValue
///////////////////////////////////////////////////////////////////////////*/
void clTree::SetValue(short int iCode, bool bValue) {
  mp_oTreePop->UpdateTree(this, iCode, bValue);
}

////////////////////////////////////////////////////////////////////////////
// SetValue
///////////////////////////////////////////////////////////////////////////*/
void clTree::SetValue(short int iCode, string sValue) {
  mp_oTreePop->UpdateTree(this, iCode, sValue);
}


////////////////////////////////////////////////////////////////////////////
// GetValue
///////////////////////////////////////////////////////////////////////////*/
void clTree::GetValue(short int iCode, int *p_iValHolder) {
  if (iCode < 0 || iCode > mp_oTreePop->mp_iNumTreeIntVals[m_iSpecies] [m_iType]) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTree::clTree";
    stcErr.sMoreInfo = "Bad tree int data code.";
    throw(stcErr);
  }
  *p_iValHolder = mp_iIntValues[iCode];
}

////////////////////////////////////////////////////////////////////////////
// GetValue
///////////////////////////////////////////////////////////////////////////*/
void clTree::GetValue(short int iCode, float *p_fValHolder) {
  *p_fValHolder = mp_fFloatValues[iCode];
}

////////////////////////////////////////////////////////////////////////////
// GetValue
///////////////////////////////////////////////////////////////////////////*/
void clTree::GetValue(short int iCode, bool *p_bValHolder) {
  *p_bValHolder = mp_bBoolValues[iCode];
}

////////////////////////////////////////////////////////////////////////////
// GetValue
///////////////////////////////////////////////////////////////////////////*/
void clTree::GetValue(short int iCode, string *p_sValHolder) {
  *p_sValHolder = mp_sStringValues[iCode];
}


////////////////////////////////////////////////////////////////////////////
// MakeDeadClone
////////////////////////////////////////////////////////////////////////////
clDeadTree* clTree::MakeDeadClone() {
  clDeadTree *p_oClone = new clDeadTree(m_iType, m_iSpecies,
      mp_oTreePop->mp_iNumTreeFloatVals[m_iSpecies] [m_iType],
      mp_oTreePop->mp_iNumTreeIntVals[m_iSpecies] [m_iType],
      mp_oTreePop->mp_iNumTreeStringVals[m_iSpecies] [m_iType],
      mp_oTreePop->mp_iNumTreeBoolVals[m_iSpecies] [m_iType]);

  int i;

  for (i = 0; i < mp_oTreePop->mp_iNumTreeFloatVals[m_iSpecies][m_iType]; i++) {
    p_oClone->mp_fFloatValues[i] = mp_fFloatValues[i];
  }

  for (i = 0; i < mp_oTreePop->mp_iNumTreeIntVals[m_iSpecies][m_iType]; i++) {
    p_oClone->mp_iIntValues[i] = mp_iIntValues[i];
  }

  for (i = 0; i < mp_oTreePop->mp_iNumTreeBoolVals[m_iSpecies][m_iType]; i++) {
    p_oClone->mp_bBoolValues[i] = mp_bBoolValues[i];
  }

  for (i = 0; i < mp_oTreePop->mp_iNumTreeStringVals[m_iSpecies][m_iType]; i++) {
    p_oClone->mp_sStringValues[i] = mp_sStringValues[i];
  }

  return p_oClone;
}
