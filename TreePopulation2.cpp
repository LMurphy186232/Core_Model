#include <stddef.h>
#include <math.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <string>
#include "TreePopulation.h"
#include "Plot.h"
#include "Allometry.h"
#include "Tree.h"
#include "SimManager.h"
#include "ModelMath.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "PlatformFuncs.h"
#include "GhostTreePopulation.h"

/////////////////////////////////////////////////////////////////////////////
// Holds more tree functions. Getters, setters, tree data member stuff
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
// GetRandomDiam10Value
/////////////////////////////////////////////////////////////////////////////
float clTreePopulation::GetRandomDiam10Value(float fDiam10Seed) {
  float fRandom = clModelMath::GetRand();
  //I couldn't make the default assignment to m_fNewSeedlingDiam10 in the
  //arguments list without compiler errors so I'm doing it here.
  if (fDiam10Seed == 0)
    fDiam10Seed = m_fNewSeedlingDiam10;
  return (fDiam10Seed * (0.9995 + 0.001 * fRandom));
}


//////////////////////////////////////////////////////////////////////////////
// UpdateTree()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTree(clTree * p_oTree, short int iCode, int iValue) {
  //For ints - no allometry - so just update the value
  try
  {
    //Make sure the code is a good one - if not, throw error
    if (iCode < 0 || iCode >= mp_iNumTreeIntVals[p_oTree->m_iSpecies][p_oTree->m_iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::UpdateTree(int)" ;
      std::stringstream s;
      s << "Invalid data code: " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    p_oTree->mp_iIntValues[iCode] = iValue;
  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree(int)" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// UpdateTree()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTree(clTree *p_oTree, short int iCode, string sValue) {
  //For strings - no allometry - so just update the value
  try
  {
    //Make sure the code is a good one - if not, throw error
    if (iCode < 0 || iCode >= mp_iNumTreeStringVals[p_oTree->m_iSpecies][p_oTree->m_iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::UpdateTree(string)" ;
      std::stringstream s;
      s << "Invalid data code: " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    p_oTree->mp_sStringValues[iCode] = sValue;
  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree(char)" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// UpdateTree()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTree(clTree * p_oTree, short int iCode, bool bValue) {
  //For bools - no allometry - so just update the value
  try
  {
    //Make sure the code is a good one - if not, throw error
    if (iCode < 0 || iCode >= mp_iNumTreeBoolVals[p_oTree->m_iSpecies][p_oTree->m_iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::UpdateTree(bool)" ;
      std::stringstream s;
      s << "Invalid data code: " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    p_oTree->mp_bBoolValues[iCode] = bValue;
  }
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree(bool)" ;
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// UpdateTree - float()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::UpdateTree(clTree * p_oTree, short int iCode,
    float fValue, bool bUpdateNow, bool bUpdateAllometry) {
  try
  {
    float fAllomValue; //for calculating allometric values
    unsigned short int iSp = p_oTree->GetSpecies(), iType = p_oTree->GetType();
    bool bAllometryUpdated = false;

    //Make sure the code is a good one - if not, throw error
    if (iCode < 0 || iCode >= mp_iNumTreeFloatVals[iSp][iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clTreePopulation::UpdateTree(float)" ;
      std::stringstream s;
      s << "Invalid data code: " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Verify that we're not trying to change X or Y - if so, throw an error.
    if (iCode == mp_iXCode[iSp][iType] || iCode == mp_iYCode[iSp][iType])
    {
      modelErr stcErr;
      stcErr.iErrorCode = ILLEGAL_OP;
      stcErr.sFunction = "clTreePopulation::UpdateTree" ;
      stcErr.sMoreInfo = "X and Y cannot be changed.";
      throw(stcErr);
    }

    if (bUpdateAllometry)
    {

      //*******************************
      // Update the tree's allometry
      //*******************************
      //>>>>>>>>>>> Tree is a seedling <<<<<<<<<<<
      if (seedling == p_oTree->m_iType)
      {
        if (mp_iDiam10Code[iSp][iType] == iCode)
        { //update diam10

          //reject a value below the minimum
          if (fValue < MINDIAM) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set seedling diameter below minimum.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[iCode] = fValue;
          //calculate height based on diam10
          fAllomValue = mp_oAllom->CalcSeedlingHeight(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          bAllometryUpdated = true;
        }
        else if (mp_iHeightCode[iSp][iType] == iCode)
        {
          //Do not allow negative values
          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set seedling height to negative value.";
            throw(stcErr);
          }

          p_oTree->mp_fFloatValues[iCode] = fValue;
          //calculate diam10 based on height
          fAllomValue = mp_oAllom->CalcSeedlingDiam10(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
          bAllometryUpdated = true;
        }
        else
        { //this is not an allometric value - just assign it
          p_oTree->mp_fFloatValues[iCode] = fValue;
          return;
        }
        //Check to see if this seedling is big enough to be a sapling
        if (p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] > mp_fMaxSeedlingHeight[iSp])
        {
          //Re-do the allometry based on sapling equations, keeping constant the
          //thing that was assigned
          if (mp_iDiam10Code[iSp][iType] == iCode)
          { //update diam10
            //convert diam10 to dbh
            fAllomValue = mp_oAllom->ConvertDiam10ToDbh(fValue, iSp);
            //calculate height based on diam10
            fAllomValue = mp_oAllom->CalcSaplingHeight(fAllomValue, iSp);
            p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;

          }
          else if (mp_iHeightCode[iSp][iType] == iCode)
          { //update height
            //calculate dbh based on height
            fAllomValue = mp_oAllom->CalcSaplingDbh(fValue, iSp);
            //Convert dbh to diam10 and assign
            fAllomValue = mp_oAllom->ConvertDbhToDiam10(fAllomValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
          }
          ChangeTreeType(p_oTree, sapling);

          //Just in case - is the sapling big enough to be an adult?  If so,
          //convert it a second time
          if (p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][sapling]] >= mp_fMinAdultDbh[iSp])
          {

            //Convert the type and species codes to sapling
            if (mp_iDiam10Code[iSp][iType] == iCode) //diam10 was updated
              iCode = mp_iDbhCode[iSp][sapling]; //make it the dbh code
            else
              iCode = mp_iHeightCode[iSp][sapling]; //height code instead
            iType = sapling;

            //Re-do the allometry based on adult equations, keeping constant the
            //thing that was assigned
            if (mp_iDiam10Code[iSp][iType] == iCode)
            { //update diam10
              //calculate height based on dbh
              fAllomValue = mp_oAllom->CalcAdultHeight(p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]], iSp);
              p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
            }
            else if (mp_iHeightCode[iSp][iType] == iCode)
            { //update height
              //calculate dbh based on height
              fAllomValue = mp_oAllom->CalcAdultDbh(fValue, iSp);
              p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
            }
            else if (mp_iDbhCode[iSp][iType] == iCode)
            { //update dbh
              //calculate height based on dbh
              fAllomValue = mp_oAllom->CalcAdultHeight(fValue, iSp);
              p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
            }
            ChangeTreeType(p_oTree, adult);
          }
        }

      } //end of if (seedling == p_oTree->m_iTreeType)


      //>>>>>>>>>>> Tree is a sapling <<<<<<<<<<<
      else if (sapling == p_oTree->m_iType)
      {
        if (mp_iDiam10Code[iSp][iType] == iCode)
        { //update diam10

          //reject a value below the minimum
          if (fValue < m_fNewSeedlingDiam10) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set sapling diameter below minimum.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[iCode] = fValue;
          //calculate dbh based on diam10
          fAllomValue = mp_oAllom->ConvertDiam10ToDbh(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
          //calculate height based on dbh
          fAllomValue = mp_oAllom->CalcSaplingHeight(fAllomValue, iSp);
          p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          bAllometryUpdated = true;

        }
        else if (mp_iHeightCode[iSp][iType] == iCode)
        { //update height

          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set sapling height to negative value.";
            throw(stcErr);
          }
          //Does this height make the tree shorter than the max seedling height?
          if (fValue < mp_fMaxSeedlingHeight[iSp])
          {
            //Assign height
            p_oTree->mp_fFloatValues[iCode] = fValue;
            //Re-do the allometry based on seedling equations, keeping height
            //constant
            //calculate diam10 based on height and assign
            fAllomValue = mp_oAllom->CalcSeedlingDiam10(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
            ChangeTreeType(p_oTree, seedling);
            bAllometryUpdated = true;
          }
          else
          { //normal - no shrinking back to seedling - update
            p_oTree->mp_fFloatValues[iCode] = fValue;
            //calculate dbh based on height
            fAllomValue = mp_oAllom->CalcSaplingDbh(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
            //calculate diam10 based on dbh
            fAllomValue = mp_oAllom->ConvertDbhToDiam10(fAllomValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
            bAllometryUpdated = true;
          }

        }
        else if (mp_iDbhCode[iSp][iType] == iCode)
        { //update dbh
          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set sapling DBH to negative value.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fValue;
          //calculate diam10 based on dbh
          fAllomValue = mp_oAllom->ConvertDbhToDiam10(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
          //calculate height based on dbh
          fAllomValue = mp_oAllom->CalcSaplingHeight(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          bAllometryUpdated = true;
        }
        else
        { //this is not an allometric value - just assign it
          p_oTree->mp_fFloatValues[iCode] = fValue;
          return;
        }
        //Check to see if this sapling is big enough to be an adult
        if (p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] >= mp_fMinAdultDbh[iSp])
        {
          //Re-do the allometry based on adult equations, keeping constant the
          //thing that was assigned
          if (mp_iDiam10Code[iSp][iType] == iCode)
          { //update diam10
            //calculate height based on dbh
            fAllomValue = mp_oAllom->CalcAdultHeight(p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]], iSp);
            p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          }
          else if (mp_iHeightCode[iSp][iType] == iCode)
          { //update height
            //calculate dbh based on height
            fAllomValue = mp_oAllom->CalcAdultDbh(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
          }
          else if (mp_iDbhCode[iSp][iType] == iCode)
          { //update dbh
            //calculate height based on dbh
            fAllomValue = mp_oAllom->CalcAdultHeight(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          }
          ChangeTreeType(p_oTree, adult);
        }
      } //end of else if (sapling == p_oTree->m_iTreeType)


      //>>>>>>>>>>> Tree is an adult or snag <<<<<<<<<<<
      else if (adult == p_oTree->m_iType || snag == p_oTree->m_iType)
      {
        if (mp_iHeightCode[iSp][iType] == iCode)
        { //update height
          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set adult height to negative value.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[iCode] = fValue;
          //calculate dbh based on height
          fValue = mp_oAllom->CalcAdultDbh(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fValue;
          bAllometryUpdated = true;
        }
        else if (mp_iDbhCode[iSp][iType] == iCode)
        { //update dbh
          if (fValue <= 0) {
            modelErr stcErr;
            stcErr.iErrorCode = ILLEGAL_OP;
            stcErr.sFunction = "clTreePopulation::UpdateTree" ;
            stcErr.sMoreInfo = "Attempt to set adult DBH to negative value.";
            throw(stcErr);
          }
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fValue;
          //calculate height based on dbh
          fValue = mp_oAllom->CalcAdultHeight(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fValue;
          bAllometryUpdated = true;
        }
        else
        { //this is not an allometric value - just assign it
          p_oTree->mp_fFloatValues[iCode] = fValue;
          return;
        }
        //Check dbh - smaller than sapling cutoff if adult?  If so, send this back to
        //sapling status
        if (adult == p_oTree->m_iType && p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] < mp_fMinAdultDbh[iSp])
        {
          //Re-do the allometry based on sapling equations, keeping constant the
          //thing that was assigned
          if (mp_iDbhCode[iSp][iType] == iCode)
          { //update dbh
            //calculate height based on dbh
            fAllomValue = mp_oAllom->CalcSaplingHeight(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] = fAllomValue;
          }
          else if (mp_iHeightCode[iSp][iType] == iCode)
          { //update height
            //calculate dbh based on height and assign
            fAllomValue = mp_oAllom->CalcSaplingDbh(fValue, iSp);
            p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
          }
          ChangeTreeType(p_oTree, sapling);
        }
      } //end of else if (adult == p_oTree->m_iTreeType)

      //>>>>>>>>>>> Tree is an another type <<<<<<<<<<<
      else
      {
        //throw an error that this tree is not an expected type - this is mostly
        //to make sure we find this if we're adding other types
        modelErr stcErr;
        stcErr.iErrorCode = TREE_WRONG_TYPE;
        stcErr.sFunction = "clTreePopulation::UpdateTree" ;
        throw(stcErr);
      }
    }
    else
    {

      //Don't automatically update allometry

      //Check to see if we're supposed to update height - if we are,
      //set the "allometry updated" flag so the hash table will be
      //resorted
      if (mp_iHeightCode[iSp][iType] == iCode)
      {
        bAllometryUpdated = true;
      }

      p_oTree->mp_fFloatValues[iCode] = fValue;

      //If this is a sapling, and we're updating either DBH or diam10, update
      //the other
      if (sapling == p_oTree->m_iType)
      {
        if (mp_iDiam10Code[iSp][iType] == iCode)
        {
          //calculate dbh based on diam10
          fAllomValue = mp_oAllom->ConvertDiam10ToDbh(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] = fAllomValue;
        }
        else if (mp_iDbhCode[iSp][iType] == iCode)
        {
          //calculate diam10 based on dbh
          fAllomValue = mp_oAllom->ConvertDbhToDiam10(fValue, iSp);
          p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iType]] = fAllomValue;
        }
      }

      if (seedling == p_oTree->m_iType && p_oTree->mp_fFloatValues[mp_iHeightCode[iSp][iType]] > mp_fMaxSeedlingHeight[iSp])
      {
        //This height makes the tree taller than the max seedling height
        ChangeTreeType(p_oTree, sapling);
      }
      else if (sapling == p_oTree->m_iType && mp_iHeightCode[iSp][iType] == iCode && fValue < mp_fMaxSeedlingHeight[iSp])
      {
        //This height makes the tree shorter than the max seedling height
        ChangeTreeType(p_oTree, seedling);
      }
      //Check to see if this sapling is big enough to be an adult
      else if (sapling == p_oTree->m_iType && p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] >= mp_fMinAdultDbh[iSp])
      {
        ChangeTreeType(p_oTree, adult);
      }
      else if (adult == p_oTree->m_iType && p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iType]] < mp_fMinAdultDbh[iSp])
      {
        ChangeTreeType(p_oTree, sapling);
      }
    }

    //If we're supposed to update now, do it
    if (bAllometryUpdated)
    {
      if (bUpdateNow)
        UpdateTreeInHashTable(p_oTree);
      else
        m_bDoUpdates = true;
    }

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// ChangeTreeType()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::ChangeTreeType(clTree * p_oTree, enum iTreeType iNewType) {
  try
  {
    //Copies of the old arrays
    float * p_fFloatCopy = NULL;
    int * p_iIntCopy = NULL;
    string * p_sStringCopy = NULL;
    bool * p_bBoolCopy = NULL;
    unsigned short int iSp, iOldType, i, j;

    iSp = p_oTree->m_iSpecies;
    iOldType = p_oTree->m_iType;
    //Declare the copy arrays and copy in the tree's values
    p_fFloatCopy = new float[mp_iNumTreeFloatVals[iSp][iOldType]];
    for (i = 0; i < mp_iNumTreeFloatVals[iSp][iOldType]; i++)
      p_fFloatCopy[i] = p_oTree->mp_fFloatValues[i];
    p_iIntCopy = new int[mp_iNumTreeIntVals[iSp][iOldType]];
    for (i = 0; i < mp_iNumTreeIntVals[iSp][iOldType]; i++)
      p_iIntCopy[i] = p_oTree->mp_iIntValues[i];
    p_sStringCopy = new string[mp_iNumTreeStringVals[iSp][iOldType]];
    for (i = 0; i < mp_iNumTreeStringVals[iSp][iOldType]; i++)
      p_sStringCopy[i] = p_oTree->mp_sStringValues[i];
    p_bBoolCopy = new bool[mp_iNumTreeBoolVals[iSp][iOldType]];
    for (i = 0; i < mp_iNumTreeBoolVals[iSp][iOldType]; i++)
      p_bBoolCopy[i] = p_oTree->mp_bBoolValues[i];

    //Delete out the tree's old variable array and redeclare
    delete[] p_oTree->mp_fFloatValues; p_oTree->mp_fFloatValues = NULL;
    delete[] p_oTree->mp_iIntValues; p_oTree->mp_iIntValues = NULL;
    delete[] p_oTree->mp_sStringValues; p_oTree->mp_sStringValues = NULL;
    delete[] p_oTree->mp_bBoolValues; p_oTree->mp_bBoolValues = NULL;
    p_oTree->mp_fFloatValues = new float[mp_iNumTreeFloatVals[iSp][iNewType]];
    for (i = 0; i < mp_iNumTreeFloatVals[iSp][iNewType]; i++)
      p_oTree->mp_fFloatValues[i] = 0.0;
    p_oTree->mp_iIntValues = new int[mp_iNumTreeIntVals[iSp][iNewType]];
    for (i = 0; i < mp_iNumTreeIntVals[iSp][iNewType]; i++)
      p_oTree->mp_iIntValues[i] = 0;
    p_oTree->mp_sStringValues = new string[mp_iNumTreeStringVals[iSp][iNewType]];
    for (i = 0; i < mp_iNumTreeStringVals[iSp][iNewType]; i++)
      p_oTree->mp_sStringValues[i] = "";

    p_oTree->mp_bBoolValues = new bool[mp_iNumTreeBoolVals[iSp][iNewType]];
    for (i = 0; i < mp_iNumTreeBoolVals[iSp][iNewType]; i++)
      p_oTree->mp_bBoolValues[i] = false;

    //Assign the new type
    p_oTree->m_iType = iNewType;

    //Now any other values that are present in both types
    for (i = 0; i < mp_iNumTreeIntVals[iSp][iOldType]; i++)
      for (j = 0; j < mp_iNumTreeIntVals[iSp][iNewType]; j++)
        if (mp_sIntLabels[iSp][iNewType][j].compare(mp_sIntLabels[iSp][iOldType][i]) == 0)
          p_oTree->mp_iIntValues[j] = p_iIntCopy[i];
    for (i = 0; i < mp_iNumTreeFloatVals[iSp][iOldType]; i++)
      for (j = 0; j < mp_iNumTreeFloatVals[iSp][iNewType]; j++)
        if (mp_sFloatLabels[iSp][iNewType][j].compare(mp_sFloatLabels[iSp][iOldType][i]) == 0)
          p_oTree->mp_fFloatValues[j] = p_fFloatCopy[i];
    for (i = 0; i < mp_iNumTreeStringVals[iSp][iOldType]; i++)
      for (j = 0; j < mp_iNumTreeStringVals[iSp][iNewType]; j++)
        if (mp_sStringLabels[iSp][iNewType][j].compare(mp_sStringLabels[iSp][iOldType][i]) == 0)
          p_oTree->mp_sStringValues[j] = p_sStringCopy[i];
    for (i = 0; i < mp_iNumTreeBoolVals[iSp][iOldType]; i++)
      for (j = 0; j < mp_iNumTreeBoolVals[iSp][iNewType]; j++)
        if (mp_sBoolLabels[iSp][iNewType][j].compare(mp_sBoolLabels[iSp][iOldType][i]) == 0)
          p_oTree->mp_bBoolValues[j] = p_bBoolCopy[i];

    //If seedling to sapling, do a dbh
    if (iOldType == seedling && iNewType == sapling)
      p_oTree->mp_fFloatValues[mp_iDbhCode[iSp][iNewType]] =
          mp_oAllom->ConvertDiam10ToDbh(p_fFloatCopy[mp_iDiam10Code[iSp][iOldType]], iSp);

    //If adult to sapling, do a diam10
    if (iOldType == adult && iNewType == sapling)
      p_oTree->mp_fFloatValues[mp_iDiam10Code[iSp][iNewType]] =
          mp_oAllom->ConvertDbhToDiam10(p_fFloatCopy[mp_iDbhCode[iSp][iOldType]], iSp);

    //Delete the copy arrays
    delete[] p_fFloatCopy;
    delete[] p_iIntCopy;
    delete[] p_sStringCopy;
    delete[] p_bBoolCopy;

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::UpdateTree" ;
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetMinAdultDBH()
//////////////////////////////////////////////////////////////////////////////
float clTreePopulation::GetMinAdultDBH(int iSpecies) {
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies)
  {
    //invalid species - throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetMinAdultDBH" ;
    std::stringstream s;
    s << "Unrecognized species " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return mp_fMinAdultDbh[iSpecies];
}


/////////////////////////////////////////////////////////////////////////////
// TranslateSpeciesNameToCode()
/////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::TranslateSpeciesNameToCode(string sSpeciesName) {
  short int iCode = -1, //preset to "not found" code
      i; //loop counter

  if (NULL == mp_speciesCodes) return iCode;

  //compare the species string passed to each of the name strings in our list
  // - if there's a match set iCode to that species's code
  for (i = 0; i < m_iNumSpecies; i++)
  {
    if (mp_speciesCodes[i].sName.compare(sSpeciesName) == 0)
      iCode = mp_speciesCodes[i].iCode;
  } //end of for (i = 0; i < m_iNumSpecies; i++)

  return iCode;
}


/////////////////////////////////////////////////////////////////////////////
// TranslateSpeciesCodeToName()
/////////////////////////////////////////////////////////////////////////////
string clTreePopulation::TranslateSpeciesCodeToName(int iSpecies) {
  short int i; //loop counter

  if (NULL == mp_speciesCodes) return "";

  //compare the species code passed to each of the codes in our list - if
  //there's a match return that species's name
  for (i = 0; i < m_iNumSpecies; i++)
  {
    if (mp_speciesCodes[i].iCode == iSpecies)
      return mp_speciesCodes[i].sName;
  } //end of for (i = 0; i < m_iNumSpecies; i++)

  return "";
}




//////////////////////////////////////////////////////////////////////////////
// RegisterDataMember()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterDataMember(string sLabel, int iSpecies,
    int iType, short int **p_iNumTreeVals, string ***p_sLabels) {
  short int iNumVals, //number of values for this species/type combo
  iReturnCode = -1, //what we'll return
  i;

  //Check to make sure that the data label is not empty
  if (0 == sLabel.length()) {
    modelErr stcErr;
    stcErr.sFunction = "clTreePopulation::RegisterDataMember" ;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sMoreInfo = "Data label is required.";
    throw(stcErr);
  }

  //Find the first open space for this type and species
  iNumVals = p_iNumTreeVals[iSpecies][iType];
  for (i = 0; i < iNumVals; i++) {
    if (0 == p_sLabels[iSpecies][iType][i].length()) {
      iReturnCode = i;
      break;
    }
    if (p_sLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      modelErr stcErr;
          stcErr.sFunction = "clTreePopulation::RegisterDataMember" ;
          stcErr.iErrorCode = ILLEGAL_OP;
          stcErr.sMoreInfo = "Data member already registered";
          throw(stcErr);
    }
  }

  //If we didn't find anything, throw an error
  if (-1 == iReturnCode)
  {
    modelErr stcErr;
    stcErr.sFunction = "clTreePopulation::RegisterDataMember" ;
    stcErr.iErrorCode = ILLEGAL_OP;
    stcErr.sMoreInfo = "Too many tree data member registrations";
    throw(stcErr);
  }

  p_sLabels[iSpecies][iType][iReturnCode] = sLabel;
  return iReturnCode;
}


//////////////////////////////////////////////////////////////////////////////
// RegisterInt()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterInt(string sLabel, int iSpecies, int iType) {
  return RegisterDataMember(sLabel, iSpecies, iType, mp_iNumTreeIntVals,
      mp_sIntLabels);
}



//////////////////////////////////////////////////////////////////////////////
// RegisterFloat()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterFloat(string sLabel, int iSpecies, int iType) {
  return RegisterDataMember(sLabel, iSpecies, iType, mp_iNumTreeFloatVals,
      mp_sFloatLabels);
}



//////////////////////////////////////////////////////////////////////////////
// RegisterChar()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterChar(string sLabel, int iSpecies, int iType) {
  return RegisterDataMember(sLabel, iSpecies, iType, mp_iNumTreeStringVals,
      mp_sStringLabels);
}



//////////////////////////////////////////////////////////////////////////////
// RegisterBool()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::RegisterBool(string sLabel, int iSpecies, int iType) {
  return RegisterDataMember(sLabel, iSpecies, iType, mp_iNumTreeBoolVals,
      mp_sBoolLabels);
}



//////////////////////////////////////////////////////////////////////////////
// GetDataCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetIntDataCode(string sLabel, int iSpecies,
    int iType) {
  short int iReturnCode = -1, i;

  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetIntDataCode" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //Make sure the label passed wasn't an empty string
  if (0 == sLabel.length())
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDataCode" ;
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < mp_iNumTreeIntVals[iSpecies][iType]; i++)
    if (mp_sIntLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}

//----------------------------------------------------------------------------
short int clTreePopulation::GetFloatDataCode(string sLabel, int iSpecies,
    int iType) {
  short int iReturnCode = -1, i;

  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetFloatDataCode" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //Make sure the label passed wasn't an empty string
  if (0 == sLabel.length())
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDataCode" ;
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < mp_iNumTreeFloatVals[iSpecies][iType]; i++)
    if (mp_sFloatLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}

//----------------------------------------------------------------------------
short int clTreePopulation::GetStringDataCode(string sLabel, int iSpecies,
    int iType) {
  short int iReturnCode = -1, i;

  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetStringDataCode" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //Make sure the label passed wasn't an empty string
  if (0 == sLabel.length())
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDataCode" ;
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < mp_iNumTreeStringVals[iSpecies][iType]; i++)
    if (mp_sStringLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}

//----------------------------------------------------------------------------
short int clTreePopulation::GetBoolDataCode(string sLabel, int iSpecies, int iType) {
  short int iReturnCode = -1, i;

  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetBoolDataCode" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //Make sure the label passed wasn't an empty string
  if (0 == sLabel.length())
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDataCode" ;
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < mp_iNumTreeBoolVals[iSpecies][iType]; i++)
    if (mp_sBoolLabels[iSpecies][iType][i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetDataLabel
//////////////////////////////////////////////////////////////////////////////
string clTreePopulation::GetIntDataLabel(short int iCode, int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetIntDataLabel" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= mp_iNumTreeIntVals[iSpecies][iType])
    return "";
  else
    return mp_sIntLabels[iSpecies][iType][iCode];
}

//----------------------------------------------------------------------------
string clTreePopulation::GetFloatDataLabel(short int iCode, int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetFloatDataLabel" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= mp_iNumTreeFloatVals[iSpecies][iType])
    return "";
  else
    return mp_sFloatLabels[iSpecies][iType][iCode];
}

//----------------------------------------------------------------------------
string clTreePopulation::GetStringDataLabel(short int iCode, int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetCharDataLabel" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= mp_iNumTreeStringVals[iSpecies][iType])
    return "";
  else
    return mp_sStringLabels[iSpecies][iType][iCode];
}

//----------------------------------------------------------------------------
string clTreePopulation::GetBoolDataLabel(short int iCode, int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType >= m_iNumTypes)
  { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetBoolDataLabel" ;
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }

  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= mp_iNumTreeBoolVals[iSpecies][iType])
    return "";
  else
    return mp_sBoolLabels[iSpecies][iType][iCode];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetXCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetXCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType
      >= m_iNumTypes) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetXCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iXCode[iSpecies][iType];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetYCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetYCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType
      >= m_iNumTypes) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetYCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iYCode[iSpecies][iType];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetHeightCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetHeightCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || iType < 0 || iType
      >= m_iNumTypes) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetHeightCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iHeightCode[iSpecies][iType];
}


//////////////////////////////////////////////////////////////////////////////
// GetDbhCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetDbhCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || (iType != sapling && iType
      != adult && iType != stump && iType != snag)) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDbhCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iDbhCode[iSpecies][iType];
}

//////////////////////////////////////////////////////////////////////////////
// GetCrownRadiusCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetCrownRadiusCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || (iType != sapling && iType
      != adult && iType != snag)) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetCrownRadiusCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iCrownRadCode[iSpecies][iType];
}

//////////////////////////////////////////////////////////////////////////////
// GetCrownDepthCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetCrownDepthCode(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || (iType != sapling && iType
      != adult && iType != snag)) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetCrownDepthCode";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iCrownDepthCode[iSpecies][iType];
}

//////////////////////////////////////////////////////////////////////////////
// GetAgeCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetAgeCode(int iSpecies) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetAgeCode";
    std::stringstream s;
    s << "Unrecognized species. Species = " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iAgeCode[iSpecies];
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetWhyDeadCode()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetWhyDeadCode(int iSpecies) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetAgeCode";
    std::stringstream s;
    s << "Unrecognized species. Species = " << iSpecies;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iWhyDeadCode[iSpecies];
}

//----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// GetDiam10Code()
//////////////////////////////////////////////////////////////////////////////
short int clTreePopulation::GetDiam10Code(int iSpecies, int iType) {
  //validate the species and type codes
  if (iSpecies < 0 || iSpecies >= m_iNumSpecies || (iType != sapling && iType
      != seedling)) { //throw an error
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clTreePopulation::GetDiam10Code";
    std::stringstream s;
    s << "Unrecognized species or type. Species = " << iSpecies << " Type = " << iType;
    stcErr.sMoreInfo = s.str();
    throw stcErr;
  }
  return mp_iDiam10Code[iSpecies][iType];
}

//----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
// DataMemberRegistrations()
//////////////////////////////////////////////////////////////////////////////
void clTreePopulation::DataMemberRegistrations() {
  try
  {

    clBehaviorBase * p_oBehavior;
    int iNumBehaviors, i;

    //Ask each behavior to register its data members
    iNumBehaviors = mp_oSimManager->GetNumberOfBehaviors();
    for (i = 0; i < iNumBehaviors; i++)
    {
      p_oBehavior = mp_oSimManager->GetBehaviorObject(i);
      p_oBehavior->RegisterTreeDataMembers();
    } //end of for (i = 0; i < iNumBehaviors; i++)

  } //end of try block
  catch (modelErr & err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::DataMemberRegistrations" ;
    throw(stcErr);
  }
}
