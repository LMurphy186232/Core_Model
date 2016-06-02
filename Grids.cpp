//---------------------------------------------------------------------------
//#include <sstream.h>

#include "Grids.h"
#include "Grid.h"
#include "Plot.h"
#include "SimManager.h"
//----------------------------------------------------------------------------
using namespace std;
/////////////////////////////////////////////////////////////////////////////
// CreateGrid
/////////////////////////////////////////////////////////////////////////////
clGrid* clGridManager::CreateGrid(string sGridName,
      short int iNumIntVals, short int iNumdoubleVals, short int iNumStringVals,
      short int iNumBoolVals, float fXCellLength,  float fYCellLength) {

  clWorkerBase **p_oTempObjectArray; //for making a copy of the object array
  clGrid *p_oReturnGrid = NULL;  //for returning the new grid
  int iIndex = -1,      //place in the object array where the new grid goes
      i;                //loop counter

  //Create our grid
  p_oReturnGrid =
      new clGrid(mp_oSimManager, sGridName, iNumIntVals, iNumdoubleVals,
          iNumStringVals, iNumBoolVals, fXCellLength, fYCellLength);

  //Special case - first grid object created - just create it and exit
  if (NULL == mp_oObjectArray) {
    m_iNumObjects = 1;
    mp_oObjectArray = new clWorkerBase*[m_iNumObjects];
    mp_oObjectArray[0] = p_oReturnGrid;
    return p_oReturnGrid;
  }

  //Determine the next open pointer in the behavior array - if this is a
  //new definition for an existing object then delete the old object - the
  //index will be set to the "hole" created
  for (i = 0; i < m_iNumObjects; i++)
    if (sGridName.compare(mp_oObjectArray[i]->GetName()) == 0) {
      delete mp_oObjectArray[i];
      mp_oObjectArray[i] = NULL;
    }
  for (i = 0; i < m_iNumObjects; i++)
    if (!mp_oObjectArray[i]) {
      iIndex = i;
      break;
    }

  //Check to see if we found an open spot in the array - if we didn't increase
  //the size of the array by one
  if (-1 == iIndex) {
    //make a copy of the array
    p_oTempObjectArray = new clWorkerBase*[m_iNumObjects];
    for (i = 0; i < m_iNumObjects; i++)
      p_oTempObjectArray[i] = mp_oObjectArray[i];
    //resize the object array
    delete[] mp_oObjectArray;
    m_iNumObjects++;
    mp_oObjectArray = new clWorkerBase*[m_iNumObjects];
    //copy the old array back in
    for (i = 0; i < m_iNumObjects - 1; i++)
      mp_oObjectArray[i] = p_oTempObjectArray[i];
    delete[] p_oTempObjectArray;
    iIndex = m_iNumObjects - 1;
  } //end of if (-1 == iIndex)

  //Put our new object in the array
  mp_oObjectArray[iIndex] = p_oReturnGrid;
  return p_oReturnGrid;
}

/////////////////////////////////////////////////////////////////////////////
// CreateObjects
/////////////////////////////////////////////////////////////////////////////
void clGridManager::CreateObjects(DOMDocument *p_oDoc) {
  try {
    clWorkerBase *p_oTemp;
    clGrid *p_oGrid = NULL;   //grid object to apply map values to
    DOMNodeList *p_oNodeList,     //for searching for tags
                *p_oChildren,     //for searching nested tags
                *p_oGridObjects;  //for getting all grid definitions
    DOMNode *p_oNode;             //for capturing a particular tag
    DOMElement *p_oElement,       //for casting a tag to access its data
               *p_oGridEl;        //a single grid's data
    XMLCh *sVal;
    string sData,                  //for capturing text data
           sGridName;              //name of the grid from the map file
    float fCellXLen, fCellYLen;   //Length of grid cells in X and Y directions
    int iNumGrids,                //Number of grid objects to initialize
        i, j;                     //loop counters
    short int iNumInts, iNumFloats, iNumChars, iNumBools,
              iNumPackageInts, iNumPackageFloats, iNumPackageChars,iNumPackageBools;

    //If there are already grid objects, erase all existing objects
    if (m_iNumObjects > 0) FreeMemory();

    //Get the number of grid objects to define
    sVal = XMLString::transcode("grid");
    p_oGridObjects = p_oDoc->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumGrids = p_oGridObjects->getLength();

    //Loop through the grid objects and create each one
    for (i = 0; i < iNumGrids; i++) {
      fCellXLen = 0; fCellYLen = 0;

      p_oNode = p_oGridObjects->item(i);
      p_oGridEl = (DOMElement *) p_oNode;

      //Get the name of the grid
      sVal = XMLString::transcode("gridName");
      sGridName = XMLString::transcode(p_oGridEl->getAttributeNode(sVal)->getNodeValue());
      XMLString::release(&sVal);

      //Do we already have this grid object?
      p_oTemp = PassObjectPointer(sGridName);
      //If we do have this grid already - skip out to the next grid - it will
      //read the map values when GetData() is called
      if (p_oTemp) goto NextGrid;

      //Get grid cell length values, if present
      sVal = XMLString::transcode("ma_lengthXCells");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        fCellXLen = atof(sData.c_str());
      }
      sVal = XMLString::transcode("ma_lengthYCells");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        fCellYLen = atof(sData.c_str());
      }

      //Get numbers of the various data types
      //Ints
      sVal = XMLString::transcode("ma_intCodes");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("ma_intCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumInts = p_oChildren->getLength();
      } else iNumInts = 0;
      //Floats
      sVal = XMLString::transcode("ma_floatCodes");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("ma_floatCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumFloats = p_oChildren->getLength();
      } else iNumFloats = 0;
      //Chars
      sVal = XMLString::transcode("ma_charCodes");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("ma_charCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumChars = p_oChildren->getLength();
      } else iNumChars = 0;
      //Bools
      sVal = XMLString::transcode("ma_boolCodes");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("ma_boolCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumBools = p_oChildren->getLength();
      } else iNumBools = 0;

      //Create the grid and give it the data
      p_oGrid = CreateGrid(sGridName, iNumInts, iNumFloats, iNumChars,
                           iNumBools, fCellXLen, fCellYLen);

      //Register the data members by getting their labels
      //*********************Ints***********************
      if (iNumInts > 0) {
        //Get the list of data members again
        sVal = XMLString::transcode("ma_intCodes");
        p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        if (0 != p_oNodeList->getLength()) {
          p_oNode = p_oNodeList->item(0);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("ma_intCode");
          p_oChildren = p_oElement->getElementsByTagName(sVal);
          XMLString::release(&sVal);

          //Go through and register each data member
          for (j = 0; j < iNumInts; j++) {
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            sData = XMLString::transcode(p_oElement->getAttributeNode(
                     sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_oGrid->RegisterInt(sData);
          }  //end of for (j = 0; j < iNumInts; j++)
        } //end of if (0 != p_oNodeList->getLength())
      } //end of if (iNumInts > 0)

      //*********************Floats***********************
      if (iNumFloats > 0) {
        //Get the list of data members again
        sVal = XMLString::transcode("ma_floatCodes");
        p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        if (0 != p_oNodeList->getLength()) {
          p_oNode = p_oNodeList->item(0);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("ma_floatCode");
          p_oChildren = p_oElement->getElementsByTagName(sVal);
          XMLString::release(&sVal);

          //Go through and register each data member
          for (j = 0; j < iNumFloats; j++) {
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            sData = XMLString::transcode(p_oElement->getAttributeNode(
                     sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_oGrid->RegisterFloat(sData);
          }  //end of for (j = 0; j < iNumFloats; j++)
        } //end of if (0 != p_oNodeList->getLength())
      } //end of if (iNumFloats > 0)

      //*********************Chars***********************
      if (iNumChars > 0) {
        //Get the list of data members again
        sVal = XMLString::transcode("ma_charCodes");
        p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        if (0 != p_oNodeList->getLength()) {
          p_oNode = p_oNodeList->item(0);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("ma_charCode");
          p_oChildren = p_oElement->getElementsByTagName(sVal);
          XMLString::release(&sVal);

          //Go through and register each data member
          for (j = 0; j < iNumChars; j++) {
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            sData = XMLString::transcode(p_oElement->getAttributeNode(
                     sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_oGrid->RegisterString(sData);
          }  //end of for (j = 0; j < iNumChars; j++)
        } //end of if (0 != p_oNodeList->getLength())
      } //end of if (iNumChars > 0)

      //*********************Bools***********************
      if (iNumBools > 0) {
        //Get the list of data members again
        sVal = XMLString::transcode("ma_boolCodes");
        p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        if (0 != p_oNodeList->getLength()) {
          p_oNode = p_oNodeList->item(0);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("ma_boolCode");
          p_oChildren = p_oElement->getElementsByTagName(sVal);
          XMLString::release(&sVal);

          //Go through and register each data member
          for (j = 0; j < iNumBools; j++) {
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            sData = XMLString::transcode(p_oElement->getAttributeNode(
                     sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_oGrid->RegisterBool(sData);
          }  //end of for (j = 0; j < iNumBools; j++)
        } //end of if (0 != p_oNodeList->getLength())
      } //end of if (iNumBools > 0)




      //Package info
      //Get numbers of the various data types
      //Ints
      sVal = XMLString::transcode("ma_packageIntCodes");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("ma_intCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumPackageInts = p_oChildren->getLength();
      } else iNumPackageInts = 0;
      //Floats
      sVal = XMLString::transcode("ma_packageFloatCodes");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("ma_floatCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumPackageFloats = p_oChildren->getLength();
      } else iNumPackageFloats = 0;
      //Chars
      sVal = XMLString::transcode("ma_packageCharCodes");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("ma_charCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumPackageChars = p_oChildren->getLength();
      } else iNumPackageChars = 0;
      //Bools
      sVal = XMLString::transcode("ma_packageBoolCodes");
      p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 != p_oNodeList->getLength()) {
        p_oNode = p_oNodeList->item(0);
        p_oElement = (DOMElement *) p_oNode;
        sVal = XMLString::transcode("ma_boolCode");
        p_oChildren = p_oElement->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumPackageBools = p_oChildren->getLength();
      } else iNumPackageBools = 0;

      //If there was no package info, we're done; skip out
      if (0 == iNumPackageInts && 0 == iNumPackageFloats && 0 == iNumPackageChars
          && 0 == iNumPackageBools) goto NextGrid;

      //Otherwise, register the package data members
      p_oGrid->ChangePackageDataStructure(iNumPackageInts, iNumPackageFloats,
          iNumPackageChars, iNumPackageBools);

      //*********************Package Ints***********************
      if (iNumPackageInts > 0) {
        //Get the list of data members again
        sVal = XMLString::transcode("ma_packageIntCodes");
        p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        if (0 != p_oNodeList->getLength()) {
          p_oNode = p_oNodeList->item(0);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("ma_intCode");
          p_oChildren = p_oElement->getElementsByTagName(sVal);
          XMLString::release(&sVal);

          //Go through and register each data member
          for (j = 0; j < iNumPackageInts; j++) {
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            sData = XMLString::transcode(p_oElement->getAttributeNode(
                     sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_oGrid->RegisterPackageInt(sData);
          }  //end of for (j = 0; j < iNumPackageInts; j++)
        } //end of if (0 != p_oNodeList->getLength())
      } //end of if (iNumPackageInts > 0)

      //*********************Package Floats***********************
      if (iNumPackageFloats > 0) {
        //Get the list of data members again
        sVal = XMLString::transcode("ma_packageFloatCodes");
        p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        if (0 != p_oNodeList->getLength()) {
          p_oNode = p_oNodeList->item(0);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("ma_floatCode");
          p_oChildren = p_oElement->getElementsByTagName(sVal);
          XMLString::release(&sVal);

          //Go through and register each data member
          for (j = 0; j < iNumPackageFloats; j++) {
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            sData = XMLString::transcode(p_oElement->getAttributeNode(
                     sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_oGrid->RegisterPackageFloat(sData);
          }  //end of for (j = 0; j < iNumPackageFloats; j++)
        } //end of if (0 != p_oNodeList->getLength())
      } //end of if (iNumPackageFloats > 0)

      //*********************Package Chars***********************
      if (iNumPackageChars > 0) {
        //Get the list of data members again
        sVal = XMLString::transcode("ma_packageCharCodes");
        p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        if (0 != p_oNodeList->getLength()) {
          p_oNode = p_oNodeList->item(0);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("ma_charCode");
          p_oChildren = p_oElement->getElementsByTagName(sVal);
          XMLString::release(&sVal);

          //Go through and register each data member
          for (j = 0; j < iNumPackageChars; j++) {
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            sData = XMLString::transcode(p_oElement->getAttributeNode(
                     sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_oGrid->RegisterPackageString(sData);
          }  //end of for (j = 0; j < iNumPackageChars; j++)
        } //end of if (0 != p_oNodeList->getLength())
      } //end of if (iNumPackageChars > 0)

      //*********************Package Bools***********************
      if (iNumPackageBools > 0) {
        //Get the list of data members again
        sVal = XMLString::transcode("ma_packageBoolCodes");
        p_oNodeList = p_oGridEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        if (0 != p_oNodeList->getLength()) {
          p_oNode = p_oNodeList->item(0);
          p_oElement = (DOMElement *) p_oNode;
          sVal = XMLString::transcode("ma_boolCode");
          p_oChildren = p_oElement->getElementsByTagName(sVal);
          XMLString::release(&sVal);

          //Go through and register each data member
          for (j = 0; j < iNumPackageBools; j++) {
            p_oNode = p_oChildren->item(j);
            p_oElement = (DOMElement *) p_oNode;
            //Get the data label, which is an attribute of the tag
            sVal = XMLString::transcode("label");
            sData = XMLString::transcode(p_oElement->getAttributeNode(
                     sVal)->getNodeValue());
            XMLString::release(&sVal);
            p_oGrid->RegisterPackageBool(sData);
          }  //end of for (j = 0; j < iNumPackageBools; j++)
        } //end of if (0 != p_oNodeList->getLength())
      } //end of if (iNumPackageBools > 0)

      NextGrid: ;//label so we can skip out to the next grid
    } //end of for (i = 0; i < iNumGrids; i++)
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGridManager::CreateObjects";
    throw(stcErr);
  }
}
//----------------------------------------------------------------------------
