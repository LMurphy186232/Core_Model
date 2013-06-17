#include "BehaviorBase.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include <stdio.h>
#include <math.h>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clBehaviorBase::clBehaviorBase(clSimManager *p_oSimManager)
: clWorkerBase(p_oSimManager) {
  try {
    m_fVersionNumber = 0;
    m_fMinimumVersionNumber = 0;
    m_iNumSpeciesTypeCombos = 0;
    m_iNumBehaviorSpecies = 0;
    mp_whatSpeciesTypeCombos = NULL;
    mp_iWhatSpecies = NULL;
    m_iNewTreeInts = 0;
    m_iNewTreeFloats = 0;
    m_iNewTreeChars = 0;
    m_iNewTreeBools = 0;
    m_iBehaviorListNumber = 0;
    m_sXMLRoot = "";
  }
  catch(modelErr &err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBehaviorBase::clBehaviorBase";
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clBehaviorBase::~clBehaviorBase() {
  delete[] mp_whatSpeciesTypeCombos;
  delete[] mp_iWhatSpecies;
}


/////////////////////////////////////////////////////////////////////////////
// SetSpeciesTypeCombos
/////////////////////////////////////////////////////////////////////////////
void clBehaviorBase::SetSpeciesTypeCombos(
    short int iNumCombos, stcSpeciesTypeCombo *p_whatCombos) {
  try {
    clPopulationBase *p_oTemp;  //for getting the tree population object
    clTreePopulation *p_oTrees; //tree population - for getting species
    short int iNumSpecies, iNumTypes, //for validating species and type numbers
    *p_iSpeciesList,  //for compiling a list of unique species
    i, j;             //loop counters
    bool bFound;                //for checking species for uniqueness

    if (mp_whatSpeciesTypeCombos) {
      //we've already got it - can't do this again!
      modelErr stcErr;
      stcErr.sFunction = "clBehaviorBase::SetSpeciesTypeCombos";
      stcErr.iErrorCode = DATA_READ_ONLY;
      stcErr.sMoreInfo = "behavior species/type data";
      throw(stcErr);
    }
    m_iNumSpeciesTypeCombos = iNumCombos;

    //ERROR CHECKING
    //Check to see if we have any combos
    if (0 >= m_iNumSpeciesTypeCombos) {
      //We have no combos  - if the pointer is NULL, exit without error
      if (NULL == p_whatCombos) return;
      //Otherwise - problem - throw error
      else {
        modelErr stcErr;
        stcErr.sFunction = "clBehaviorBase::SetSpecies";
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Number of combos doesn't match species/type combo list.";
        throw(stcErr);
      }
      //Number of combos in the list isn't zero - make sure the pointer isn't NULL
    } else {
      if (NULL == p_whatCombos) { //throw error
        modelErr stcErr;
        stcErr.sFunction = "clBehaviorBase::SetSpecies";
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Number of combos doesn't match species/type combo list.";
        throw(stcErr);
      }
    }

    //Create a new array and copy in the values
    mp_whatSpeciesTypeCombos = new stcSpeciesTypeCombo[m_iNumSpeciesTypeCombos];
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      mp_whatSpeciesTypeCombos[i].iSpecies = p_whatCombos->iSpecies;
      mp_whatSpeciesTypeCombos[i].iType = p_whatCombos->iType;
      p_whatCombos++;
    }

    //Validate that all the species and types are good
    p_oTemp = mp_oSimManager->GetPopulationObject("treepopulation");
    p_oTrees = (clTreePopulation*)p_oTemp;
    iNumSpecies = p_oTrees->GetNumberOfSpecies();
    iNumTypes = p_oTrees->GetNumberOfTypes();
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      if (mp_whatSpeciesTypeCombos[i].iSpecies < 0 ||
          mp_whatSpeciesTypeCombos[i].iSpecies >= iNumSpecies) {
        //Bad species - throw error
        modelErr stcErr;
        stcErr.sFunction = "clBehaviorBase::SetSpecies";
        stcErr.iErrorCode = BAD_DATA;
        std::stringstream s;
        s << "Unrecognized species number: " << mp_whatSpeciesTypeCombos[i].iSpecies;
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
      if (mp_whatSpeciesTypeCombos[i].iType < 0 ||
          mp_whatSpeciesTypeCombos[i].iType >= iNumTypes) {
        //Bad types - throw error
        modelErr stcErr;
        stcErr.sFunction = "clBehaviorBase::SetTypes";
        stcErr.iErrorCode = BAD_DATA;
        std::stringstream s;
        s << "Unrecognized type: " << mp_whatSpeciesTypeCombos[i].iType;
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //Assemble the list of unique species present and put that in the species
    //array

    //Declare the temp. species array to be as big as the combo list to make
    //sure we have space for everything, and initialize values to -1
    p_iSpeciesList = new short int[m_iNumSpeciesTypeCombos];
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++)
      p_iSpeciesList[i] = -1;

    //Go through each combo, and for the species for that combo, if it's not
    //already on the temp list, add it
    //Re-use iNumSpecies to keep track of how many species were found
    iNumSpecies = 0;
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      bFound = false;
      //Test to see if this species is already on the list
      for (j = 0; j < iNumSpecies; j++) {
        if (mp_whatSpeciesTypeCombos[i].iSpecies == p_iSpeciesList[j])
        {bFound = true; break;}
      }
      if (!bFound) {
        //Add the species to the list and increment the number of found species
        //by one
        p_iSpeciesList[iNumSpecies] = mp_whatSpeciesTypeCombos[i].iSpecies;
        iNumSpecies++;
      }
    } //end of for (i = 0; i < m_iNumSpeciesTypeCombos; i++)

    //At this point p_iSpeciesList should have the unique species list - declare
    //mp_iWhatSpecies and assign values
    m_iNumBehaviorSpecies = iNumSpecies;
    mp_iWhatSpecies = new short int[m_iNumBehaviorSpecies];
    for (i = 0; i < iNumSpecies; i++)
      mp_iWhatSpecies[i] = p_iSpeciesList[i];

    delete[] p_iSpeciesList;

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBehaviorBase::SetSpeciesTypeCombos";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetSpeciesTypeCombo()
//////////////////////////////////////////////////////////////////////////////
struct stcSpeciesTypeCombo clBehaviorBase::GetSpeciesTypeCombo(short int iIndex) {
  //If the index isn't valid - throw error
  if (iIndex < 0 || iIndex >= m_iNumSpeciesTypeCombos) {
    modelErr stcErr;
    stcErr.sFunction = "clBehaviorBase::GetSpeciesTypeCombo";
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Index \"" << iIndex << "\" is not a valid species/type combo index.";
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  else return (mp_whatSpeciesTypeCombos[iIndex]);
}


/////////////////////////////////////////////////////////////////////////////
// GetBehaviorSpecies
/////////////////////////////////////////////////////////////////////////////
short int clBehaviorBase::GetBehaviorSpecies(short int iIndex) {
  //If the index isn't valid - throw error
  if (iIndex < 0 || iIndex >= m_iNumBehaviorSpecies) {
    modelErr stcErr;
    stcErr.sFunction = "clBehaviorBase::GetBehaviorSpecies";
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Index \"" << iIndex << "\" is not a valid species index.";
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  else return (mp_iWhatSpecies[iIndex]);
}


/////////////////////////////////////////////////////////////////////////////
// ValidateVersionNumber
/////////////////////////////////////////////////////////////////////////////
short int clBehaviorBase::ValidateVersionNumber(float fTestVersion) {
  //Make all comparisons to the first two digits after the decimal
  //Does this match the current version?
  if (fabs(fTestVersion - m_fVersionNumber) < 0.01)
    return 1;
  //Is it in the range?
  else if (fTestVersion >= m_fMinimumVersionNumber
      && fTestVersion <= m_fVersionNumber)
    return 0;
  else return -1;
}


/////////////////////////////////////////////////////////////////////////////
// GetParentParametersElement
/////////////////////////////////////////////////////////////////////////////
DOMElement* clBehaviorBase::GetParentParametersElement(xercesc::DOMDocument *p_oDoc) {
  DOMNodeList *p_oNodeList;
  DOMNode *p_oDocNode;
  XMLCh *sVal;
  std::stringstream sTemp;
  sTemp << m_sXMLRoot << m_iBehaviorListNumber;

  sVal = XMLString::transcode(sTemp.str().c_str());
  p_oNodeList = p_oDoc->getElementsByTagName(sVal);
  XMLString::release(&sVal);
  if (0 == p_oNodeList->getLength()) {
    modelErr stcErr;
    stcErr.iErrorCode = DATA_MISSING;
    stcErr.sFunction = "clBehaviorBase::GetParentParametersElement";
    std::stringstream s;
    s << m_sXMLRoot << m_iBehaviorListNumber;
    stcErr.sMoreInfo = s.str();
    throw (&stcErr);
  }

  p_oDocNode = p_oNodeList->item(0);
  return (DOMElement *) p_oDocNode;
}

////////////////////////////////////////////////////////////////////////////
// FormatSpeciesTypeQueryString()
////////////////////////////////////////////////////////////////////////////
/*string clBehaviorBase::FormatSpeciesTypeQueryString()
{
  string sQuery;
  char cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSeedling = false, bSapling = false, bAdult = false, bSnag = false;

  //Do a type/species search on all the types and species
  sQuery = "species=";
  for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
  {
    sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
    sQuery.append( cQueryPiece );
  }
  sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
  sQuery.append( cQueryPiece );

  //Find all the types
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    if ( clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSeedling = true;
    }
    if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSapling = true;
    }
    else if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType )
    {
      bAdult = true;
    }
    else if ( clTreePopulation::snag == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSnag = true;
    }
  }
  sQuery.append("::type=");
  if ( bSeedling )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::seedling, "," );
    sQuery.append( cQueryPiece );
  }
  if ( bSapling )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::sapling, "," );
    sQuery.append( cQueryPiece );
  }
  if ( bAdult )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::adult, "," );
    sQuery.append( cQueryPiece );
  }
  if ( bSnag )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::snag, "," );
    sQuery.append( cQueryPiece );
  }

  //Remove the last comma
  return sQuery.substr(0, sQuery.length() - 1);
}*/


/////////////////////////////////////////////////////////////////////////////
// Virtual functions - we must have their definitions here.  If we don't we get
// a linker error
/////////////////////////////////////////////////////////////////////////////
void clBehaviorBase::Action() {;}
void clBehaviorBase::RegisterTreeDataMembers() {;}

