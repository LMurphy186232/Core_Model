//---------------------------------------------------------------------------
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sstream>

using namespace xercesc;
using namespace std;
//////////////////////////////////////////////////////////////////////////////
// FillSpeciesSpecificValue() - with floatVal array
/////////////////////////////////////////////////////////////////////////////*/
void FillSpeciesSpecificValue( xercesc::DOMElement *p_oParent, string sTagName,
    string sSubTagName, floatVal *p_array, int iNumSpecies,
    clTreePopulation *p_oPop, bool bRequired) {
  try {
    DOMNodeList *p_oNodeList;  //node list object - for catching all nodes with
    //a particular tag name
    DOMNode *p_oDocNode;       //for accessing a particular node
    DOMElement *p_oElement;    //used to cast nodes to elements to access
    //functions unique to the DOM_Element class
    XMLCh *sTag;
    char *cData;               //for extracting text values
    float fVal;                //for testing data validity
    short int iNumNodes,       //used to count the number of nodes in a nodelist
    iCode,           //used to get a species code
    *p_iSpeciesRead,   //used to keep a list of those species that we
    //already read
    i, j, k = 0;     //loop counters
    bool bDuplOrBad = false;   //for screening out duplicate and bad values

    if (p_oParent == NULL) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Parent tag";
      throw(stcErr);
    }

    //Get the parent node with tag name "strElementName" - there should only be
    //one so only take the first returned
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

    //Now get the list of all subelements with the actual data
    p_oElement = (DOMElement *) p_oDocNode; //cast to DOM_Element
    sTag = XMLString::transcode(sSubTagName.c_str());
    p_oNodeList = p_oElement->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sSubTagName;
        throw(stcErr);
      }
    }

    //Initialize our list of species we've read with values of -1
    p_iSpeciesRead = new short int [iNumSpecies];
    for (i = 0; i < iNumSpecies; i++)
      p_iSpeciesRead[i] = -1;

    //Go through the nodes and grab all the values for the species this behavior
    //will act on
    for (i = 0; i < iNumNodes; i++) {
      //Get the species code for this node
      iCode = GetNodeSpeciesCode(p_oNodeList->item(i), p_oPop);
      if (-1 != iCode) { //Only continue if it's a valid data piece

        //First check to see if it's a species we've already read - if so skip
        //We don't know for sure that there aren't duplicates
        bDuplOrBad = false;
        for (j = 0; j < iNumSpecies; j++)
          if (p_iSpeciesRead[j] == iCode) bDuplOrBad = true;

        //Check to see if the data is a valid float - convert it.  If we get 0,
        //which is either really 0 or an error, distinguish by the length of the
        //target string
        cData = XMLString::transcode(p_oNodeList->item(i)->getChildNodes()
            ->item(0)->getNodeValue());
        fVal = atof(cData);
        if (0 == fVal && cData[0] != '0') //bad
          bDuplOrBad = true;
        delete[] cData;

        //Check it against our list to see if it's a species we want-if so grab
        if (!bDuplOrBad) {
          for (j = 0; j < iNumSpecies; j++) {
            if (iCode == p_array[j].code) {
              p_array[j].val = fVal;
              p_iSpeciesRead[k] = iCode;
              k++;
            } //end of if (iCode == p_array[j].code)
          } //end of for (j = 0; j < m_iNumSpecies; j++)
        } //end of if (!bDuplicate)
      } //end of if (-1 != iCode)
    } //end of for (i = 0; i < iNumNodes; i++)

    //Make sure we got a piece of data for each species we needed - if not
    //throw an error
    for (i = 0; i < iNumSpecies; i++)
      if (-1 == p_iSpeciesRead[i]) {
        delete[] p_iSpeciesRead;
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " for all species indicated";
        throw(stcErr);
      }
    delete[] p_iSpeciesRead;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSpeciesSpecificValue";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// FillSpeciesSpecificValue() - with doubleVal array
/////////////////////////////////////////////////////////////////////////////*/
void FillSpeciesSpecificValue( xercesc::DOMElement *p_oParent, string sTagName,
    string sSubTagName, doubleVal *p_array, int iNumSpecies,
    clTreePopulation *p_oPop, bool bRequired) {
  try {
    DOMNodeList *p_oNodeList;  //node list object - for catching all nodes with
    //a particular tag name
    DOMNode *p_oDocNode;       //for accessing a particular node
    DOMElement *p_oElement;    //used to cast nodes to elements to access
    //functions unique to the DOM_Element class
    XMLCh *sTag;
    char *cData;               //for extracting text values
    double fVal;                //for testing data validity
    short int iNumNodes,       //used to count the number of nodes in a nodelist
    iCode,           //used to get a species code
    *p_iSpeciesRead,   //used to keep a list of those species that we
    //already read
    i, j, k = 0;     //loop counters
    bool bDuplOrBad = false;   //for screening out duplicate and bad values

    if (p_oParent == NULL) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Parent tag";
      throw(stcErr);
    }

    //Get the parent node with tag name "strElementName" - there should only be
    //one so only take the first returned
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

    //Now get the list of all subelements with the actual data
    p_oElement = (DOMElement *) p_oDocNode; //cast to DOM_Element
    sTag = XMLString::transcode(sSubTagName.c_str());
    p_oNodeList = p_oElement->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sSubTagName;
        throw(stcErr);
      }
    }

    //Initialize our list of species we've read with values of -1
    p_iSpeciesRead = new short int [iNumSpecies];
    for (i = 0; i < iNumSpecies; i++)
      p_iSpeciesRead[i] = -1;

    //Go through the nodes and grab all the values for the species this behavior
    //will act on
    for (i = 0; i < iNumNodes; i++) {
      //Get the species code for this node
      iCode = GetNodeSpeciesCode(p_oNodeList->item(i), p_oPop);
      if (-1 != iCode) { //Only continue if it's a valid data piece

        //First check to see if it's a species we've already read - if so skip
        //We don't know for sure that there aren't duplicates
        bDuplOrBad = false;
        for (j = 0; j < iNumSpecies; j++)
          if (p_iSpeciesRead[j] == iCode) bDuplOrBad = true;

        //Check to see if the data is a valid float - convert it.  If we get 0,
        //which is either really 0 or an error, distinguish by the length of the
        //target string
        cData = XMLString::transcode(p_oNodeList->item(i)->getChildNodes()
            ->item(0)->getNodeValue());
        fVal = strtod(cData, NULL);
        if (0 == fVal && cData[0] != '0') //bad
          bDuplOrBad = true;
        delete[] cData;

        //Check it against our list to see if it's a species we want-if so grab
        if (!bDuplOrBad) {
          for (j = 0; j < iNumSpecies; j++) {
            if (iCode == p_array[j].code) {
              p_array[j].val = fVal;
              p_iSpeciesRead[k] = iCode;
              k++;
            } //end of if (iCode == p_array[j].code)
          } //end of for (j = 0; j < m_iNumSpecies; j++)
        } //end of if (!bDuplicate)
      } //end of if (-1 != iCode)
    } //end of for (i = 0; i < iNumNodes; i++)

    //Make sure we got a piece of data for each species we needed - if not
    //throw an error
    for (i = 0; i < iNumSpecies; i++)
      if (-1 == p_iSpeciesRead[i]) {
        delete[] p_iSpeciesRead;
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " for all species indicated";
        throw(stcErr);
      }
    delete[] p_iSpeciesRead;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSpeciesSpecificValue";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// FillSpeciesSpecificValue() - with intVal array
//////////////////////////////////////////////////////////////////////////////
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, string sTagName,
    string sSubTagName, intVal *p_array, int iNumSpecies,
    clTreePopulation *p_oPop, bool bRequired) {
  try {
    DOMNodeList *p_oNodeList;  //node list object - for catching all nodes with
    //a particular tag name
    DOMNode *p_oDocNode;       //for accessing a particular node
    DOMElement *p_oElement;    //used to cast nodes to elements to access
    //functions unique to the DOM_Element class
    XMLCh *sTag;
    char *cData;               //for extracting text values
    int iVal;                  //for testing data validity
    short int iNumNodes,       //used to count the number of nodes in a nodelist
    iCode,           //used to get a species code
    *p_iSpeciesRead,   //used to keep a list of those species that we
    //already read
    i, j, k = 0;     //loop counters
    bool bDuplOrBad = false;   //for screening out duplicate and bad values

    if (p_oParent == NULL) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Parent tag";
      throw(stcErr);
    }

    //Get the parent node with tag name "strElementName" - there should only be
    //one so only take the first returned
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

    //Now get the list of all subelements with the actual data
    p_oElement = (xercesc::DOMElement *) p_oDocNode; //cast to DOM_Element
    sTag = XMLString::transcode(sSubTagName.c_str());
    p_oNodeList = p_oElement->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sSubTagName;
        throw(stcErr);
      }
    }

    //Initialize our list of species we've read with values of -1
    p_iSpeciesRead = new short int [iNumSpecies];
    for (i = 0; i < iNumSpecies; i++)
      p_iSpeciesRead[i] = -1;

    //Go through the nodes and grab all the values for the species this behavior
    //will act on
    for (i = 0; i < iNumNodes; i++) {
      //Get the species code for this node
      iCode = GetNodeSpeciesCode(p_oNodeList->item(i), p_oPop);
      if (-1 != iCode) { //Only continue if it's a valid data piece

        //First check to see if it's a species we've already read - if so skip
        //We don't know for sure that there aren't duplicates
        bDuplOrBad = false;
        for (j = 0; j < iNumSpecies; j++)
          if (p_iSpeciesRead[j] == iCode) bDuplOrBad = true;

        //Check to see if the data is a valid int - convert it.  If we get 0,
        //which is either really 0 or an error, distinguish by the first char of
        //the target string
        cData = XMLString::transcode(p_oNodeList->item(i)->getChildNodes()
            ->item(0)->getNodeValue());
        iVal = atoi(cData);
        if (0 == iVal && cData[0] != '0') //bad
          bDuplOrBad = true;
        delete[] cData;

        //Check it against our list to see if it's a species we want-if so grab
        if (!bDuplOrBad) {
          for (j = 0; j < iNumSpecies; j++) {
            if (iCode == p_array[j].code) {
              p_array[j].val = iVal;
              p_iSpeciesRead[k] = iCode;
              k++;
            } //end of if (iCode == p_array[j].code)
          } //end of for (j = 0; j < m_iNumSpecies; j++)
        } //end of if (!bDuplicate)
      } //end of if (-1 != iCode)
    } //end of for (i = 0; i < iNumNodes; i++)

    //Make sure we got a piece of data for each species we needed - if not
    //throw an error
    for (i = 0; i < iNumSpecies; i++)
      if (-1 == p_iSpeciesRead[i]) {
        delete[] p_iSpeciesRead;
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " for all species indicated";
        throw(stcErr);
      }

    delete[] p_iSpeciesRead;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSpeciesSpecificValue";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// FillSpeciesSpecificValue() - with boolVal array
/////////////////////////////////////////////////////////////////////////////*/
void FillSpeciesSpecificValue( xercesc::DOMElement *p_oParent, string sTagName,
    string sSubTagName, boolVal *p_array, int iNumSpecies,
    clTreePopulation *p_oPop, bool bRequired) {
  try {
    DOMNodeList *p_oNodeList;  //node list object - for catching all nodes with
    //a particular tag name
    DOMNode *p_oDocNode;       //for accessing a particular node
    DOMElement *p_oElement;    //used to cast nodes to elements to access
    //functions unique to the DOM_Element class
    XMLCh *sTag;
    char *cData;               //for extracting text values
    short int iNumNodes,       //used to count the number of nodes in a nodelist
    iCode,           //used to get a species code
    *p_iSpeciesRead,   //used to keep a list of those species that we
    //already read
    i, j, k = 0;     //loop counters
    bool bDuplOrBad = false;   //for screening out duplicate and bad values

    if (p_oParent == NULL) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Parent tag";
      throw(stcErr);
    }

    //Get the parent node with tag name "strElementName" - there should only be
    //one so only take the first returned
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

    //Now get the list of all subelements with the actual data
    p_oElement = (xercesc::DOMElement *) p_oDocNode; //cast to DOM_Element
    sTag = XMLString::transcode(sSubTagName.c_str());
    p_oNodeList = p_oElement->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sSubTagName;
        throw(stcErr);
      }
    }

    //Initialize our list of species we've read with values of -1
    p_iSpeciesRead = new short int [iNumSpecies];
    for (i = 0; i < iNumSpecies; i++)
      p_iSpeciesRead[i] = -1;

    //Go through the nodes and grab all the values for the species this behavior
    //will act on
    for (i = 0; i < iNumNodes; i++) {
      //Get the species code for this node
      iCode = GetNodeSpeciesCode(p_oNodeList->item(i), p_oPop);
      if (-1 != iCode) { //Only continue if it's a valid data piece

        //First check to see if it's a species we've already read - if so skip
        //We don't know for sure that there aren't duplicates
        bDuplOrBad = false;
        for (j = 0; j < iNumSpecies; j++)
          if (p_iSpeciesRead[j] == iCode) bDuplOrBad = true;

        //Check to see if the data exists
        cData = xercesc::XMLString::transcode(p_oNodeList->item(i)->getChildNodes()
            ->item(0)->getNodeValue());
        if (0 == strlen(cData)) //bad
          bDuplOrBad = true;

        //Check it against our list to see if it's a species we want-if so grab
        if (!bDuplOrBad) {
          for (j = 0; j < iNumSpecies; j++) {
            if (iCode == p_array[j].code) {
              if (strcmp(cData, "1") == 0)
                p_array[j].val = true;
              else if (strcmp(cData, "0") == 0)
                p_array[j].val = false;
              else {
                delete[] cData;
                delete[] p_iSpeciesRead;
                modelErr stcErr;
                stcErr.sFunction = "FillSpeciesSpecificValue(bool)";
                stcErr.iErrorCode = BAD_DATA;
                stcErr.sMoreInfo = "Unexpected value in ";
                stcErr.sMoreInfo += sTagName;
                throw(stcErr);
              }
              p_iSpeciesRead[k] = iCode;
              k++;
            } //end of if (iCode == p_array[j].code)
          } //end of for (j = 0; j < m_iNumSpecies; j++)
        } //end of if (!bDuplicate)
        delete[] cData; cData = NULL;
      } //end of if (-1 != iCode)
    } //end of for (i = 0; i < iNumNodes; i++)

    //Make sure we got a piece of data for each species we needed - if not
    //throw an error
    for (i = 0; i < iNumSpecies; i++)
      if (-1 == p_iSpeciesRead[i]) {
        delete[] p_iSpeciesRead;
        delete[] cData;
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue(bool)";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " for all species indicated";
        throw(stcErr);
      }

    delete[] p_iSpeciesRead;
    delete[] cData;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSpeciesSpecificValue";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// FillSpeciesSpecificValue() - with float array (get all species)
/////////////////////////////////////////////////////////////////////////////*/
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, string sTagName,
    string sSubTagName, float *p_array, clTreePopulation *p_oPop,
    bool bRequired) {
  try {
    DOMNodeList *p_oNodeList;  //node list object - for catching all nodes with
    //a particular tag name
    DOMNode *p_oDocNode;       //for accessing a particular node
    DOMElement *p_oElement;    //used to cast nodes to elements to access
    //functions unique to the DOM_Element class
    XMLCh *sTag;
    char *cData;               //for extracting text values
    float fVal;                //for testing data validity
    short int iNumNodes,       //used to count the number of nodes in a nodelist
    iCode,           //used to get a species code
    iNumSpecies,     //how many total species there are
    *p_iSpeciesRead, //used to keep a list of those species that we
    //already read
    i;               //loop counter
    bool bDuplOrBad = false;   //for screening out duplicate and bad values

    if (p_oParent == NULL) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Parent tag";
      throw(stcErr);
    }

    iNumSpecies = p_oPop->GetNumberOfSpecies();

    //Get the parent node with tag name "strElementName" - there should only be
    //one so only take the first returned
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

    //Now get the list of all subelements with the actual data
    p_oElement = (xercesc::DOMElement *) p_oDocNode; //cast to DOM_Element
    sTag = XMLString::transcode(sSubTagName.c_str());
    p_oNodeList = p_oElement->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = sSubTagName;
      throw(stcErr);
    }

    //Initialize our list of species we've read with values of -1
    p_iSpeciesRead = new short int [iNumSpecies];
    for (i = 0; i < iNumSpecies; i++)
      p_iSpeciesRead[i] = -1;

    //Go through the nodes and grab all the values for the species this behavior
    //will act on
    for (i = 0; i < iNumNodes; i++) {
      //Get the species code for this node
      iCode = GetNodeSpeciesCode(p_oNodeList->item(i), p_oPop);
      if (-1 != iCode) { //Only continue if it's a valid data piece

        //First check to see if it's a species we've already read - if so skip
        //We don't know for sure that there aren't duplicates
        bDuplOrBad = false;
        if (p_iSpeciesRead[iCode] != -1) bDuplOrBad = true;

        //Check to see if the data is a valid float - convert it.  If we get 0,
        //which is either really 0 or an error, distinguish by the first
        //character of the string
        cData = XMLString::transcode(p_oNodeList->item(i)->getChildNodes()
            ->item(0)->getNodeValue());
        fVal = atof(cData);
        if (0 == fVal && cData[0] != '0') //bad
          bDuplOrBad = true;
        delete[] cData; cData = NULL;

        //If it's good, put it in the array
        if (!bDuplOrBad) {
          p_array[iCode] = fVal;
          p_iSpeciesRead[iCode] = 1;
        } //end of if (!bDuplOrBad)
      } //end of if (-1 != iCode)
    } //end of for (i = 0; i < iNumNodes; i++)

    //Make sure we got a piece of data for each species we needed - if not
    //throw an error
    for (i = 0; i < iNumSpecies; i++)
      if (-1 == p_iSpeciesRead[i]) {
        delete[] p_iSpeciesRead;
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " for all species indicated.";
        throw(stcErr);
      }

    delete[] p_iSpeciesRead;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSpeciesSpecificValue";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// FillSpeciesSpecificValue() - with double array (get all species)
/////////////////////////////////////////////////////////////////////////////*/
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, string sTagName,
    string sSubTagName, double *p_array, clTreePopulation *p_oPop,
    bool bRequired) {
  try {
    DOMNodeList *p_oNodeList;  //node list object - for catching all nodes with
    //a particular tag name
    DOMNode *p_oDocNode;       //for accessing a particular node
    DOMElement *p_oElement;    //used to cast nodes to elements to access
    //functions unique to the DOM_Element class
    XMLCh *sTag;
    char *cData;               //for extracting text values
    float fVal;                //for testing data validity
    short int iNumNodes,       //used to count the number of nodes in a nodelist
    iCode,           //used to get a species code
    iNumSpecies,     //how many total species there are
    *p_iSpeciesRead, //used to keep a list of those species that we
    //already read
    i;               //loop counter
    bool bDuplOrBad = false;   //for screening out duplicate and bad values

    if (p_oParent == NULL) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Parent tag";
      throw(stcErr);
    }

    iNumSpecies = p_oPop->GetNumberOfSpecies();

    //Get the parent node with tag name "strElementName" - there should only be
    //one so only take the first returned
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

    //Now get the list of all subelements with the actual data
    p_oElement = (xercesc::DOMElement *) p_oDocNode; //cast to DOM_Element
    sTag = XMLString::transcode(sSubTagName.c_str());
    p_oNodeList = p_oElement->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = sSubTagName;
      throw(stcErr);
    }

    //Initialize our list of species we've read with values of -1
    p_iSpeciesRead = new short int [iNumSpecies];
    for (i = 0; i < iNumSpecies; i++)
      p_iSpeciesRead[i] = -1;

    //Go through the nodes and grab all the values for the species this behavior
    //will act on
    for (i = 0; i < iNumNodes; i++) {
      //Get the species code for this node
      iCode = GetNodeSpeciesCode(p_oNodeList->item(i), p_oPop);
      if (-1 != iCode) { //Only continue if it's a valid data piece

        //First check to see if it's a species we've already read - if so skip
        //We don't know for sure that there aren't duplicates
        bDuplOrBad = false;
        if (p_iSpeciesRead[iCode] != -1) bDuplOrBad = true;

        //Check to see if the data is a valid float - convert it.  If we get 0,
        //which is either really 0 or an error, distinguish by the first
        //character of the string
        cData = XMLString::transcode(p_oNodeList->item(i)->getChildNodes()
            ->item(0)->getNodeValue());
        fVal = strtod(cData, NULL);
        if (0 == fVal && cData[0] != '0') //bad
          bDuplOrBad = true;
        delete[] cData; cData = NULL;

        //If it's good, put it in the array
        if (!bDuplOrBad) {
          p_array[iCode] = fVal;
          p_iSpeciesRead[iCode] = 1;
        } //end of if (!bDuplOrBad)
      } //end of if (-1 != iCode)
    } //end of for (i = 0; i < iNumNodes; i++)

    //Make sure we got a piece of data for each species we needed - if not
    //throw an error
    for (i = 0; i < iNumSpecies; i++)
      if (-1 == p_iSpeciesRead[i]) {
        delete[] p_iSpeciesRead;
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " for all species indicated.";
        throw(stcErr);
      }

    delete[] p_iSpeciesRead;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSpeciesSpecificValue";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// FillSpeciesSpecificValue() - with int array (get all species)
/////////////////////////////////////////////////////////////////////////////*/
void FillSpeciesSpecificValue(xercesc::DOMElement *p_oParent, string sTagName,
    string sSubTagName, int *p_array, clTreePopulation *p_oPop,
    bool bRequired) {
  try {
    DOMNodeList *p_oNodeList;  //node list object - for catching all nodes with
    //a particular tag name
    DOMNode *p_oDocNode;       //for accessing a particular node
    DOMElement *p_oElement;    //used to cast nodes to elements to access
    //functions unique to the DOM_Element class
    XMLCh *sTag;
    char *cData;               //for extracting text values
    int iVal;                //for testing data validity
    short int iNumNodes,       //used to count the number of nodes in a nodelist
    iCode,           //used to get a species code
    iNumSpecies,     //how many total species there are
    *p_iSpeciesRead, //used to keep a list of those species that we
    //already read
    i;               //loop counter
    bool bDuplOrBad = false;   //for screening out duplicate and bad values

    if (p_oParent == NULL) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Parent tag";
      throw(stcErr);
    }

    iNumSpecies = p_oPop->GetNumberOfSpecies();

    //Get the parent node with tag name "strElementName" - there should only be
    //one so only take the first returned
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    p_oDocNode = p_oNodeList->item(0); //get first element returned by the list

    //Now get the list of all subelements with the actual data
    p_oElement = (xercesc::DOMElement *) p_oDocNode; //cast to DOM_Element
    sTag = XMLString::transcode(sSubTagName.c_str());
    p_oNodeList = p_oElement->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    iNumNodes = p_oNodeList->getLength();
    if (0 == iNumNodes) {
      modelErr stcErr;
      stcErr.sFunction = "FillSpeciesSpecificValue";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = sSubTagName;
      throw(stcErr);
    }

    //Initialize our list of species we've read with values of -1
    p_iSpeciesRead = new short int [iNumSpecies];
    for (i = 0; i < iNumSpecies; i++)
      p_iSpeciesRead[i] = -1;

    //Go through the nodes and grab all the values for the species this behavior
    //will act on
    for (i = 0; i < iNumNodes; i++) {
      //Get the species code for this node
      iCode = GetNodeSpeciesCode(p_oNodeList->item(i), p_oPop);
      if (-1 != iCode) { //Only continue if it's a valid data piece

        //First check to see if it's a species we've already read - if so skip
        //We don't know for sure that there aren't duplicates
        bDuplOrBad = false;
        if (p_iSpeciesRead[iCode] != -1) bDuplOrBad = true;

        //Check to see if the data is a valid int - convert it.  If we get 0,
        //which is either really 0 or an error, distinguish by the first
        //character of the string
        cData = XMLString::transcode(p_oNodeList->item(i)->getChildNodes()
            ->item(0)->getNodeValue());
        iVal = atoi(cData);
        if (0 == iVal && cData[0] != '0') //bad
          bDuplOrBad = true;
        delete[] cData;

        //If it's good, put it in the array
        if (!bDuplOrBad) {
          p_array[iCode] = iVal;
          p_iSpeciesRead[iCode] = 1;
        } //end of if (!bDuplOrBad)
      } //end of if (-1 != iCode)
    } //end of for (i = 0; i < iNumNodes; i++)

    //Make sure we got a piece of data for each species we needed - if not
    //throw an error
    for (i = 0; i < iNumSpecies; i++)
      if (-1 == p_iSpeciesRead[i]) {
        delete[] p_iSpeciesRead;
        modelErr stcErr;
        stcErr.sFunction = "FillSpeciesSpecificValue";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " for all species indicated.";
        throw(stcErr);
      }

    delete[] p_iSpeciesRead;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSpeciesSpecificValue";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetNodeSpeciesCode()
//////////////////////////////////////////////////////////////////////////////
short int GetNodeSpeciesCode(xercesc::DOMNode *p_oDocNode, clTreePopulation *p_oPop) {
  try {
    DOMElement *p_oElement;  //for transforming the node into an element
    XMLCh *sVal;
    char *cName; //string for the name of the species
    short int iCode; //for the species code

    p_oElement = (DOMElement *) p_oDocNode;
    sVal = XMLString::transcode("species");
    cName = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
    XMLString::release(&sVal);
    iCode = p_oPop->TranslateSpeciesNameToCode(cName);
    delete[] cName;
    return iCode;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "GetNodeSpeciesCode";
    throw (stcErr);
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////////
// FillSingleValue() - Integer version
//////////////////////////////////////////////////////////////////////////////
void FillSingleValue(xercesc::DOMElement *p_oParent,
    string sTagName, int *p_iValToFill, bool bRequired) {
  try {
    xercesc::DOMNodeList *p_oNodeList; //for searching the document
    XMLCh *sTag;
    char *cData;    //for returning the text data
    int iVal;               //the data value to extract and fill with

    //Error trap - make sure the pointers aren't NULL and that the tag name
    //isn't empty
    if (NULL == p_oParent || NULL == p_iValToFill) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(int)";
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw(stcErr);
    } else if (sTagName.length() == 0) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(int)";
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Tag name required.";
      throw(stcErr);
    }

    //Query the document for elements with our given tag name
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    if (0 == p_oNodeList->getLength()) { //if we didn't find our target tag...
      if (!bRequired) return; //if it wasn't required exit without error
      else { //if it was required throw an error
        modelErr stcErr;
        stcErr.sFunction = "FillSingleValue(int)";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    cData = xercesc::XMLString::transcode(p_oNodeList->item(0)
        ->getFirstChild()->getNodeValue());

    //Convert our found value to an int
    iVal = atoi(cData);
    if (iVal != 0) {
      *p_iValToFill = iVal;
      delete[] cData;
      return;
    }

    //If we got zero, it either means that the text value was not a valid int,
    //or the text was really zero.  Distinguish by looking at the length of the
    //string - if it's longer than 1, the text != 0 and it's not a valid value
    else {
      if (strlen(cData) > 1) {
        delete[] cData;
        modelErr stcErr;
        stcErr.sFunction = "FillSingleValue(int)";
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " is not an integer";
        throw(stcErr);
      }
      else {
        *p_iValToFill = iVal;
        delete[] cData;
        return;
      }
    }
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSingleValue(int)";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// FillSingleValue() - Float version
//////////////////////////////////////////////////////////////////////////////
void FillSingleValue(xercesc::DOMElement *p_oParent,
    string sTagName, float *p_fValToFill, bool bRequired) {
  try {
    DOMNodeList *p_oNodeList; //for searching the document
    XMLCh *sTag;
    char *cData;  //for returning the text data
    float fVal; //the data value to extract and fill with

    //Error trap - make sure the pointers aren't NULL and that the tag name
    //isn't empty
    if (NULL == p_oParent || NULL == p_fValToFill) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(float)";
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw(stcErr);
    } else if (sTagName.length() == 0) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(float)";
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Tag name required.";
      throw(stcErr);
    }

    //Query the document for elements with our given tag name
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    if (0 == p_oNodeList->getLength()) { //if we didn't find our target tag...
      if (!bRequired) return; //if it wasn't required exit without error
      else { //if it was required throw an error
        modelErr stcErr;
        stcErr.sFunction = "FillSingleValue(float)";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    cData = xercesc::XMLString::transcode(p_oNodeList->item(0)->getFirstChild()
        ->getNodeValue());

    //Convert our found value to a float
    fVal = atof(cData);
    if (fVal != 0) {  //if fVal isn't 0 we're successful
      *p_fValToFill = fVal;
      delete[] cData;
      return;
    }

    //If we got zero, it either means that the text value was not a valid float,
    //or the text was really zero.  Distinguish by looking at the length of the
    //string - if it's longer than 1, the text != 0 and it's not a valid value
    else {
      if (cData[0] != '0') {
        delete[] cData;
        modelErr stcErr;
        stcErr.sFunction = "FillSingleValue(float)";
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " is not a float";
        throw(stcErr);
      }
      else {
        *p_fValToFill = fVal;
        delete[] cData;
        return;
      }
    }
  }//end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSingleValue(float)";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// FillSingleValue() - double version
//////////////////////////////////////////////////////////////////////////////
void FillSingleValue(xercesc::DOMElement *p_oParent,
    string sTagName, double *p_fValToFill, bool bRequired) {
  try {
    DOMNodeList *p_oNodeList; //for searching the document
    XMLCh *sTag;
    char *cData;  //for returning the text data
    float fVal; //the data value to extract and fill with

    //Error trap - make sure the pointers aren't NULL and that the tag name
    //isn't empty
    if (NULL == p_oParent || NULL == p_fValToFill) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(double)";
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw(stcErr);
    } else if (sTagName.length() == 0) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(double)";
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Tag name required.";
      throw(stcErr);
    }

    //Query the document for elements with our given tag name
    sTag = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sTag);
    XMLString::release(&sTag);
    if (0 == p_oNodeList->getLength()) { //if we didn't find our target tag...
      if (!bRequired) return; //if it wasn't required exit without error
      else { //if it was required throw an error
        modelErr stcErr;
        stcErr.sFunction = "FillSingleValue(float)";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    cData = xercesc::XMLString::transcode(p_oNodeList->item(0)->getFirstChild()
        ->getNodeValue());

    //Convert our found value to a float
    fVal = strtod(cData, NULL);
    if (fVal != 0) {  //if fVal isn't 0 we're successful
      *p_fValToFill = fVal;
      delete[] cData;
      return;
    }

    //If we got zero, it either means that the text value was not a valid float,
    //or the text was really zero.  Distinguish by looking at the length of the
    //string - if it's longer than 1, the text != 0 and it's not a valid value
    else {
      if (cData[0] != '0') {
        delete[] cData;
        modelErr stcErr;
        stcErr.sFunction = "FillSingleValue(float)";
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = sTagName;
        stcErr.sMoreInfo += " is not a float";
        throw(stcErr);
      }
      else {
        *p_fValToFill = fVal;
        delete[] cData;
        return;
      }
    }
  }//end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSingleValue(float)";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// FillSingleValue() - string version
/////////////////////////////////////////////////////////////////////////////*/
void FillSingleValue(xercesc::DOMElement *p_oParent,
    string sTagName, string *p_sValToFill, bool bRequired) {
  try {
    xercesc::DOMNodeList *p_oNodeList; //for searching the document
    XMLCh *sVal;
    char *cData;  //for returning the text data

    //Error trap - make sure the pointers aren't NULL and that the tag name
    //isn't empty
    if (NULL == p_oParent || NULL == p_sValToFill) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(string)";
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw(stcErr);
    } else if (sTagName.length() == 0) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(string)";
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Tag name required.";
      throw(stcErr);
    }

    //Query the document for elements with our given tag name
    sVal = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength()) { //if we didn't find our target tag...
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSingleValue(string)";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    DOMNode *p_oNode = p_oNodeList->item(0);
    p_oNode = p_oNode->getFirstChild();
    if (NULL == p_oNode) {
      *p_sValToFill = "";
      return;
    }
    cData = XMLString::transcode(p_oNode->getNodeValue());

    //Check to make sure the string isn't empty
    if (0 == strlen(cData)) {
      delete[] cData;
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(string)";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = sTagName;
      throw(stcErr);
    }
    else *p_sValToFill = cData;
    delete[] cData;
    return;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSingleValue(string)";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// FillSingleValue() - bool version
//////////////////////////////////////////////////////////////////////////////
void FillSingleValue(xercesc::DOMElement *p_oParent,
    string sTagName, bool *p_bValToFill, bool bRequired) {
  try {
    DOMNodeList *p_oNodeList; //for searching the document
    XMLCh *sVal;
    char *cData;  //for returning the text data

    //Error trap - make sure the pointers aren't NULL and that the tag name
    //isn't empty
    if (NULL == p_oParent || NULL == p_bValToFill) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(bool)";
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw(stcErr);
    } else if (sTagName.length() == 0) {
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(bool)";
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Tag name required.";
      throw(stcErr);
    }

    //Query the document for elements with our given tag name
    sVal = XMLString::transcode(sTagName.c_str());
    p_oNodeList = p_oParent->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength()) { //if we didn't find our target tag...
      if (!bRequired) return; //if it wasn't required exit without error
      else {
        modelErr stcErr;
        stcErr.sFunction = "FillSingleValue(string)";
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sMoreInfo = sTagName;
        throw(stcErr);
      }
    }
    cData = xercesc::XMLString::transcode(p_oNodeList->item(0)
        ->getFirstChild()->getNodeValue());

    //Check to make sure the string isn't empty
    if (0 == strlen(cData)) {
      delete[] cData;
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(bool)";
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = sTagName;
      throw(stcErr);
    }

    if (strcmp(cData, "1") == 0)
      *p_bValToFill = true;
    else if (strcmp(cData, "0") == 0)
      *p_bValToFill = false;
    else {
      delete[] cData;
      modelErr stcErr;
      stcErr.sFunction = "FillSingleValue(bool)";
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Unexpected value in ";
      stcErr.sMoreInfo += sTagName;
      throw(stcErr);
    }
    delete[] cData;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "FillSingleValue(string)";
    throw(stcErr);
  }
}
