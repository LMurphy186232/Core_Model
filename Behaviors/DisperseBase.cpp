//---------------------------------------------------------------------------
#include "DisperseBase.h"
#include "Grid.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "DisperseOrg.h"
#include <stdio.h>
#include <sstream>

clGrid *clDisperseBase::mp_oSeedGrid = NULL;
short int *clDisperseBase::mp_iNumSeedsCode = NULL;
short int clDisperseBase::m_iTotalSpecies = 0;
short int clDisperseBase::m_iIsGapCode = -1;
short int clDisperseBase::m_iGapCountCode = -1;
bool clDisperseBase::m_bUpdatedGapStatus = false;
clDisperseOrg *clDisperseBase::mp_oDisperseOrg = NULL;

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clDisperseBase::clDisperseBase(clSimManager *p_oSimManager) :
     clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager)  {
  try {
    //Set the hooked flag to false
    m_bHooked = false;

    //Now - see if the disperse org object is NULL.  If it is, create it and
    //pass a self pointer so this object will be hooked
    if (mp_oDisperseOrg == NULL)
      mp_oDisperseOrg = new clDisperseOrg(this);

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    m_bUpdatedGapStatus = false;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisperseBase::clDisperseBase";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clDisperseBase::~clDisperseBase()
{
  //Null these 'cause they're static and might get deleted more than once
  delete[] mp_iNumSeedsCode; mp_iNumSeedsCode = NULL;
  //De-ref for future setup
  mp_oSeedGrid = NULL;

  if (m_bHooked) {
    delete mp_oDisperseOrg;
    mp_oDisperseOrg = NULL;
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clDisperseBase::GetData(xercesc::DOMDocument *p_oDoc) {
 try {
   //If this is hooked, call the growth org's DoSetup function
   if (m_bHooked) {
     SetUpBase();
     mp_oDisperseOrg->DoSetup(mp_oSimManager, p_oDoc);
   }

   DoShellSetup(p_oDoc);
 }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrowthBase::GetData";
    throw(stcErr);
  }
}


////////////////////////////////////////////////////////////////////////////
// TimestepCleanup()
///////////////////////////////////////////////////////////////////////////
void clDisperseBase::TimestepCleanup()
{
  try
  {
    if (m_bHooked) {
    short int iNumXCells = mp_oSeedGrid->GetNumberXCells(), iNumYCells = mp_oSeedGrid->GetNumberYCells(), iX, iY, iSp; //loop counters
    float fZero = 0;

    //Zero out the seeds grid
    for (iX = 0; iX < iNumXCells; iX++)
    {
      for (iY = 0; iY < iNumYCells; iY++)
      {
        for (iSp = 0; iSp < m_iTotalSpecies; iSp++)
        {
          mp_oSeedGrid->SetValueOfCell(iX, iY, mp_iNumSeedsCode[iSp], fZero);
        }
      }
    }

    //Reset the gap status updated flag
    m_bUpdatedGapStatus = false;
    }
  }
  catch (modelErr& err)
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
    stcErr.sFunction = "clDisperseBase::TimestepCleanup";
    throw(stcErr);
  }
}


////////////////////////////////////////////////////////////////////////////
// SetUpBase()
////////////////////////////////////////////////////////////////////////////
void clDisperseBase::SetUpBase()
{
  try
  {

    if (mp_oSeedGrid != NULL) return; //this function has already been called

    clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
    std::stringstream sLabel;
    short int i; //loop counters

    m_iTotalSpecies = p_oPop->GetNumberOfSpecies();

    //Declare the code indexes
    mp_iNumSeedsCode = new short int[m_iTotalSpecies];

    //First check to see if the grid has already been created
    mp_oSeedGrid = mp_oSimManager->GetGridObject("Dispersed Seeds");

    if (mp_oSeedGrid != NULL)
    {

      //The grid has already been created - get the indexes and throw an error
      //if any are not there
      for (i = 0; i < m_iTotalSpecies; i++)
      {
        sLabel << "seeds_" << i;
        mp_iNumSeedsCode[i] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
        sLabel.str("");
        if (-1 == mp_iNumSeedsCode[i])
        {
          modelErr stcErr;
          stcErr.iErrorCode = DATA_MISSING;
          stcErr.sFunction = "clDisperseBase::SetUpBase";
          stcErr.sMoreInfo = "The seed grid was incorrectly set up in the parameter file.  It's missing a seed count data member for each species.";
          throw(stcErr);
        }
      }

      m_iGapCountCode = mp_oSeedGrid->GetIntDataCode("count");
      m_iIsGapCode = mp_oSeedGrid->GetBoolDataCode("Is Gap");
      if (-1 == m_iGapCountCode || -1 == m_iIsGapCode)
      {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clDisperseBase::SetUpBase";
        stcErr.sMoreInfo = "The seed grid was incorrectly set up in the parameter file.  It's missing an adult count or gap status data member.";
        throw(stcErr);
      }
    }
    else
    {

      //The grid didn't already exist - so create it now
      mp_oSeedGrid = mp_oSimManager->CreateGrid("Dispersed Seeds", //grid name
           1, //number of int data members
           m_iTotalSpecies, //number of float data members
           0, //number of char data members
           1); //number of bool data members

      //Register the data members
      for (i = 0; i < m_iTotalSpecies; i++)
      {
        sLabel << "seeds_" << i;
        mp_iNumSeedsCode[i] = mp_oSeedGrid->RegisterFloat(sLabel.str());
        sLabel.str("");
      }

      m_iIsGapCode = mp_oSeedGrid->RegisterBool("Is Gap");
      m_iGapCountCode = mp_oSeedGrid->RegisterInt("count");
    }
  }
  catch (modelErr& err)
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
    stcErr.sFunction = "clDisperseBase::SetUpBase";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// Action()
//////////////////////////////////////////////////////////////////////////////
void clDisperseBase::Action() {
  try {

    //If this is a hooked object, call the disperse org object.  Otherwise, do
    //nothing.
    if (m_bHooked)
      mp_oDisperseOrg->DoDisperse();
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisperseBase::Action";
    throw(stcErr);
  }
}
