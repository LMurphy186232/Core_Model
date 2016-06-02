//---------------------------------------------------------------------------
#include <fstream>
#include <sstream>
#include "Grid.h"
#include "Plot.h"
#include "SimManager.h"
//---------------------------------------------------------------------------
using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clGrid::clGrid(clSimManager *p_oSimManager, const std::string sGridName,
    short int iNumIntVals, short int iNumdoubleVals, short int iNumStringVals,
    short int iNumBoolVals, float fXCellLength,  float fYCellLength) :
    clWorkerBase(p_oSimManager) {
  try {
    clPlot *p_oPlot;    //pointer to plot object, for querying plot dimensions
    int i, j, k;        //loop counters

    //Allowed file types
    m_iNumAllowedTypes = 4;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = map;
    mp_iAllowedFileTypes[2] = detailed_output_timestep;
    mp_iAllowedFileTypes[3] = detailed_output;

    //Check the data - grid name can't be empty, grid lengths and numbers of
    //variables can't be negative. Throw errors if any of these things is true.
    if (sGridName.length() == 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::clGrid";
      stcErr.sMoreInfo = "Grid name must be supplied.";
      throw(stcErr);
    }
    if (fXCellLength < 0 || fYCellLength < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::clGrid";
      std::stringstream s;
      s << "Either X or Y grid cell length is less than 0.  X = " <<
          fXCellLength << " Y = " << fYCellLength;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    if (iNumIntVals < 0 || iNumdoubleVals < 0 || iNumStringVals < 0 ||
        iNumBoolVals < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::clGrid";
      stcErr.sMoreInfo = "Data member numbers cannot be negative.";
      throw(stcErr);
    }

    //Take the data we were passed and assign it
    p_oPlot = mp_oSimManager->GetPlotObject();
    m_sNameString = sGridName;
    if (0 == fXCellLength)
      fXCellLength = p_oPlot->GetXCellLength();
    m_fXCellSize = fXCellLength;
    if (0 == fYCellLength)  m_fYCellSize = fXCellLength;
    else m_fYCellSize = fYCellLength;

    //Figure out the number of grids in X and Y directions
    m_fXPlotLength = p_oPlot->GetXPlotLength();
    m_fYPlotLength = p_oPlot->GetYPlotLength();
    m_iNumXCells = (int)ceil(m_fXPlotLength / m_fXCellSize);
    m_iNumYCells = (int)ceil(m_fYPlotLength / m_fYCellSize);

    //Assign the data member numbers
    m_iNumIntVals = iNumIntVals;
    m_iNumFloatVals = iNumdoubleVals;
    m_iNumStringVals = iNumStringVals;
    m_iNumBoolVals = iNumBoolVals;

    //Size and initialize the arrays
    mp_gridVals = new stcRecords*[m_iNumXCells];
    for (i = 0; i < m_iNumXCells; i++) {
      mp_gridVals[i] = new stcRecords[m_iNumYCells];
      for (j = 0; j < m_iNumYCells; j++) {
        //Integer data members
        if (m_iNumIntVals == 0) mp_gridVals[i][j].p_iIntVals = NULL;
        else {
          mp_gridVals[i][j].p_iIntVals = new int[m_iNumIntVals];
          for (k = 0; k < m_iNumIntVals; k++)
            mp_gridVals[i][j].p_iIntVals[k] = 0;
        }
        //Float data members
        if (m_iNumFloatVals == 0) mp_gridVals[i][j].p_fFloatVals = NULL;
        else {
          mp_gridVals[i][j].p_fFloatVals = new float[m_iNumFloatVals];
          for (k = 0; k < m_iNumFloatVals; k++)
            mp_gridVals[i][j].p_fFloatVals[k] = 0;
        }
        //String data members
        if (m_iNumStringVals == 0) mp_gridVals[i][j].p_sStringVals = NULL;
        else {
          mp_gridVals[i][j].p_sStringVals = new string[m_iNumStringVals];
          for (k = 0; k < m_iNumStringVals; k++)
            mp_gridVals[i][j].p_sStringVals[k] = "";
        }
        //Bool data members
        if (m_iNumBoolVals == 0) mp_gridVals[i][j].p_bBoolVals = NULL;
        else {
          mp_gridVals[i][j].p_bBoolVals = new bool[m_iNumBoolVals];
          for (k = 0; k < m_iNumBoolVals; k++)
            mp_gridVals[i][j].p_bBoolVals[k] = false;
        }

        mp_gridVals[i][j].p_oPackage = NULL;
      }  //end of for (j = 0; j < m_iNumYCells; j++)
    } //end of for (i = 0; i < m_iNumXCells; i++)

    //Label arrays
    if (0 == m_iNumIntVals) mp_sIntLabels = NULL;
    else {
      mp_sIntLabels = new string[m_iNumIntVals];
      for (i = 0; i < m_iNumIntVals; i++) {
        mp_sIntLabels[i] = "";
      }
    }
    if (0 == m_iNumFloatVals) mp_sFloatLabels = NULL;
    else {
      mp_sFloatLabels = new string[m_iNumFloatVals];
      for (i = 0; i < m_iNumFloatVals; i++) {
        mp_sFloatLabels[i] = "";
      }
    }
    if (0 == m_iNumStringVals) mp_sStringLabels = NULL;
    else {
      mp_sStringLabels = new string[m_iNumStringVals];
      for (i = 0; i < m_iNumStringVals; i++) {
        mp_sStringLabels[i] = "";
      }
    }
    if (0 == m_iNumBoolVals) mp_sBoolLabels = NULL;
    else {
      mp_sBoolLabels = new string[m_iNumBoolVals];
      for (i = 0; i < m_iNumBoolVals; i++) {
        mp_sBoolLabels[i] = "";
      }
    }

    //******************************************************************
    // Packages
    //******************************************************************
    //Set up the number of data members and label lists to be identical to the
    //grids
    m_bPackageChangeAllowed = true;
    m_bPackageDataChanged = false;
    m_iNumPackageIntVals = m_iNumIntVals;
    m_iNumPackageFloatVals = m_iNumFloatVals;
    m_iNumPackageStringVals = m_iNumStringVals;
    m_iNumPackageBoolVals = m_iNumBoolVals;
    if (0 == m_iNumIntVals) mp_sPackageIntLabels = NULL;
    else {
      mp_sPackageIntLabels = new string[m_iNumIntVals];
      for (i = 0; i < m_iNumIntVals; i++) {
        mp_sPackageIntLabels[i] = "";
      }
    }
    if (0 == m_iNumFloatVals) mp_sPackageFloatLabels = NULL;
    else {
      mp_sPackageFloatLabels = new string[m_iNumFloatVals];
      for (i = 0; i < m_iNumFloatVals; i++) {
        mp_sPackageFloatLabels[i] = "";
      }
    }
    if (0 == m_iNumStringVals) mp_sPackageStringLabels = NULL;
    else {
      mp_sPackageStringLabels = new string[m_iNumStringVals];
      for (i = 0; i < m_iNumStringVals; i++) {
        mp_sPackageStringLabels[i] = "";
      }
    }
    if (0 == m_iNumBoolVals) mp_sPackageBoolLabels = NULL;
    else {
      mp_sPackageBoolLabels = new string[m_iNumBoolVals];
      for (i = 0; i < m_iNumBoolVals; i++) {
        mp_sPackageBoolLabels[i] = "";
      }
    }

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::clGrid";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// ChangePackageDataStructure()
/////////////////////////////////////////////////////////////////////////////
void clGrid::ChangePackageDataStructure(short int iNumIntVals,
    short int iNumdoubleVals, short int iNumCharVals, short int iNumBoolVals) {
  try {
    short int i;

    //If packages have been created, this is not allowed - throw an error
    if (!m_bPackageChangeAllowed) {
      modelErr stcErr;
      stcErr.iErrorCode = ILLEGAL_OP;
      stcErr.sFunction = "clGrid::ChangePackageDataStructure";
      stcErr.sMoreInfo = "Package data structure may not be changed after packages have been created.";
      throw(stcErr);
    }

    //Delete all the old label arrays
    if (mp_sPackageIntLabels) delete[] mp_sPackageIntLabels;
    mp_sPackageIntLabels = NULL;

    if (mp_sPackageFloatLabels) delete[] mp_sPackageFloatLabels;
    mp_sPackageFloatLabels = NULL;

    if (mp_sPackageStringLabels) delete[] mp_sPackageStringLabels;
    mp_sPackageStringLabels = NULL;

    if (mp_sPackageBoolLabels) delete[] mp_sPackageBoolLabels;
    mp_sPackageBoolLabels = NULL;

    //Assign the new variables
    m_iNumPackageIntVals = iNumIntVals;
    m_iNumPackageFloatVals = iNumdoubleVals;
    m_iNumPackageStringVals = iNumCharVals;
    m_iNumPackageBoolVals = iNumBoolVals;

    //Declare the arrays
    if (0 == m_iNumPackageIntVals) mp_sPackageIntLabels = NULL;
    else {
      mp_sPackageIntLabels = new string[iNumIntVals];
      for (i = 0; i < iNumIntVals; i++) {
        mp_sPackageIntLabels[i] = "";
      }
    }
    if (0 == m_iNumPackageFloatVals) mp_sPackageFloatLabels = NULL;
    else {
      mp_sPackageFloatLabels = new string[iNumdoubleVals];
      for (i = 0; i < iNumdoubleVals; i++) {
        mp_sPackageFloatLabels[i] = "";
      }
    }
    if (0 == m_iNumPackageStringVals) mp_sPackageStringLabels = NULL;
    else {
      mp_sPackageStringLabels = new string[iNumCharVals];
      for (i = 0; i < iNumCharVals; i++) {
        mp_sPackageStringLabels[i] = "";
      }
    }
    if (0 == m_iNumPackageBoolVals) mp_sPackageBoolLabels = NULL;
    else {
      mp_sPackageBoolLabels = new string[iNumBoolVals];
      for (i = 0; i < iNumBoolVals; i++) {
        mp_sPackageBoolLabels[i] = "";
      }
    }
    m_bPackageDataChanged = true;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::ChangePackageDataStructure";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// CreatePackage()
/////////////////////////////////////////////////////////////////////////////
clPackage* clGrid::CreatePackage(
    clPackage *p_oPreviousPackage) {
  clPackage *p_oNewPackage, *p_oNextPackage;
  //Verify that the pointer isn't NULL - if it is, throw an error
  if (NULL == p_oPreviousPackage) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::CreatePackage(clPackage)";
    stcErr.sMoreInfo = "Pointer passed cannot be NULL.";
    throw(stcErr);
  }
  p_oNewPackage = new clPackage(this, p_oPreviousPackage->mp_parentCell);

  //Put this new variable in between the previous pointer passed and any
  //"next" it might have
  p_oNextPackage = p_oPreviousPackage->mp_oNext;
  p_oPreviousPackage->mp_oNext = p_oNewPackage;
  p_oNewPackage->mp_oNext = p_oNextPackage;

  return p_oNewPackage;
}

/////////////////////////////////////////////////////////////////////////////
// CreatePackageOfCell()
/////////////////////////////////////////////////////////////////////////////
clPackage* clGrid::CreatePackageOfCell(int iX, int iY) {
  clPackage *p_oNewPackage, *p_oNextPackage;
  //Verify that the grid numbers are valid - if they aren't, throw an error
  if (iX < 0 || iX >= m_iNumXCells || iY < 0 || iY >= m_iNumYCells) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::CreatePackageOfGrid";
    std::stringstream s;
    s << "Invalid grid numbers.  X = " << iX << ", Y = " << iY;
    stcErr.sMoreInfo = s.str();
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  p_oNewPackage = new clPackage(this, &mp_gridVals[iX][iY]);

  //Put this new variable in between the previous pointer passed and any
  //"next" it might have
  p_oNextPackage = mp_gridVals[iX][iY].p_oPackage;
  mp_gridVals[iX][iY].p_oPackage = p_oNewPackage;
  p_oNewPackage->mp_oNext = p_oNextPackage;

  return p_oNewPackage;
}

/////////////////////////////////////////////////////////////////////////////
// CreatePackageAtPoint()
/////////////////////////////////////////////////////////////////////////////
clPackage* clGrid::CreatePackageAtPoint(float fX, float fY) {
  clPackage *p_oNewPackage, *p_oNextPackage;
  int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

  //Verify that the grid numbers are valid - if they aren't, throw an error
  if (fX >= m_fXPlotLength || fX < 0
      || fY >= m_fYPlotLength || fY < 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::CreatePackageAtPoint";
    std::stringstream s;
    s << "At least one grid coordinate is outside of grid - X = " << fX
        << " & Y = " << fY;
    stcErr.sMoreInfo = s.str();
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }

  //Divide the X and Y values by the size of the grid divisions to get grid
  //cell indexes
  iXGrid = (int)floor(fX / m_fXCellSize);
  iYGrid = (int)floor(fY / m_fYCellSize);

  p_oNewPackage = new clPackage(this, &mp_gridVals[iXGrid][iYGrid]);

  //Put this new variable in between the previous pointer passed and any
  //"next" it might have
  p_oNextPackage = mp_gridVals[iXGrid][iYGrid].p_oPackage;
  mp_gridVals[iXGrid][iYGrid].p_oPackage = p_oNewPackage;
  p_oNewPackage->mp_oNext = p_oNextPackage;

  return p_oNewPackage;
}

/////////////////////////////////////////////////////////////////////////////
// GetFirstPackageOfCell()
/////////////////////////////////////////////////////////////////////////////
clPackage* clGrid::GetFirstPackageOfCell(int iX, int iY) {
  //Verify that the grid numbers are valid - if they aren't, throw an error
  if (iX < 0 || iX >= m_iNumXCells || iY < 0 || iY >= m_iNumYCells) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetFirstPackageOfCell";
    std::stringstream s;
    s << "Invalid grid numbers for grid " << m_sNameString << ".  X = "
        << iX << ", Y = " << iY;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }
  return mp_gridVals[iX][iY].p_oPackage;
}

/////////////////////////////////////////////////////////////////////////////
// GetFirstPackageAtPoint()
/////////////////////////////////////////////////////////////////////////////
clPackage* clGrid::GetFirstPackageAtPoint(float fX, float fY) {
  int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

  //Verify that the grid numbers are valid - if they aren't, throw an error
  if (fX >= m_fXPlotLength || fX < 0
      || fY >= m_fYPlotLength || fY < 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetFirstPackageAtPoint";
    std::stringstream s;
    s << "At least one grid coordinate is outside of grid " << m_sNameString
        << " - X = " << fX << " & Y = " << fY;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  }

  //Divide the X and Y values by the size of the grid divisions to get grid
  //cell indexes
  iXGrid = (int)floor(fX / m_fXCellSize);
  iYGrid = (int)floor(fY / m_fYCellSize);

  return mp_gridVals[iXGrid][iYGrid].p_oPackage;
}

//////////////////////////////////////////////////////////////////////////////
// GetPointOfCell
//////////////////////////////////////////////////////////////////////////////
void clGrid::GetPointOfCell(short int iCellX, short int iCellY,
    float *fX, float *fY) {
  try {
    float fTemp;

    //Make sure the cell points are valid
    if (iCellX < 0 || iCellX >= m_iNumXCells ||
        iCellY < 0 || iCellY >= m_iNumYCells) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetPointOfCell";
      std::stringstream s;
      s << "Invalid grid numbers for grid " << m_sNameString << ".  X = "
          << iCellX << ", Y = " << iCellY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //If this is not the last cell in a row in either direction, the calculation
    //is straightforward
    if (iCellX < (m_iNumXCells - 1) && iCellY < (m_iNumYCells - 1)) {
      *fX = (iCellX * m_fXCellSize) + (m_fXCellSize / 2);
      *fY = (iCellY * m_fYCellSize) + (m_fYCellSize / 2);
    } else {
      //We have to correct for the possibility that the last grid cells are short
      //Find half of the length of the last cell
      if (iCellX == m_iNumXCells - 1)
        fTemp = (m_fXPlotLength -
            ((m_iNumXCells - 1) * m_fXCellSize)) / 2;
      else fTemp = m_fXCellSize / 2;
      *fX = (iCellX * m_fXCellSize) + fTemp;

      if (iCellY == m_iNumYCells - 1)
        fTemp = (m_fYPlotLength - ((m_iNumYCells - 1) * m_fYCellSize)) / 2;
      else fTemp = m_fYCellSize / 2;
      *fY = (iCellY * m_fYCellSize) + fTemp;
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetPointOfCell";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetCellOfPoint()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetCellOfPoint(float fX, float fY, short int *iCellX,
    short int *iCellY) {
  try {
    //Make sure the cell points are valid
    if (fX >= m_fXPlotLength || fX < 0 || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetPointOfCell";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    *iCellX = (int)floor(fX / m_fXCellSize);
    *iCellY = (int)floor(fY / m_fYCellSize);
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetCellOfPoint";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// DeletePackage()
/////////////////////////////////////////////////////////////////////////////
void clGrid::DeletePackage(clPackage *p_oOldPackage){
  try {
    clPackage *p_oPreviousPackage;

    //If the pointer to the package is NULL, do nothing; exit
    if (NULL == p_oOldPackage) return;

    //If this was the first pointer of the cell, have the parent point to
    //the next package
    if (p_oOldPackage == p_oOldPackage->mp_parentCell->p_oPackage) {
      p_oOldPackage->mp_parentCell->p_oPackage = p_oOldPackage->mp_oNext;

      //If not the first, find the package or grid cell that points to this so we
      //can NULL out its "next" pointer
    } else {
      p_oPreviousPackage = p_oOldPackage->mp_parentCell->p_oPackage;
      while (p_oPreviousPackage != NULL &&
          p_oPreviousPackage->mp_oNext != p_oOldPackage)
        p_oPreviousPackage = p_oPreviousPackage->mp_oNext;

      if (p_oPreviousPackage)
        p_oPreviousPackage->mp_oNext = p_oOldPackage->mp_oNext;
    }

    delete p_oOldPackage;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::DeletePackage";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor()
/////////////////////////////////////////////////////////////////////////////
clGrid::~clGrid() {
  clPackage *p_oPackage, *p_oNextPackage;
  int i, j; //loop counters
  //Delete the values array
  for (i = 0; i < m_iNumXCells; i++) {
    for (j = 0; j < m_iNumYCells; j++) {
      //delete data arrays
      delete[] mp_gridVals[i][j].p_iIntVals;
      delete[] mp_gridVals[i][j].p_fFloatVals;
      delete[] mp_gridVals[i][j].p_sStringVals;
      delete[] mp_gridVals[i][j].p_bBoolVals;
      //delete any linked packages
      p_oPackage = mp_gridVals[i][j].p_oPackage;
      while (p_oPackage) {
        p_oNextPackage = p_oPackage->mp_oNext;
        delete p_oPackage;
        p_oPackage = p_oNextPackage;
      } //end of while (p_record)
    } //end of for (j = 0; j < m_iNumYCells; j++)
    delete[] mp_gridVals[i];
  } //end of for (i = 0; i < m_iNumXCells; i++)
  delete[] mp_gridVals; mp_gridVals = NULL;

  //Delete the label arrays
  delete[] mp_sIntLabels; mp_sIntLabels = NULL;
  delete[] mp_sFloatLabels; mp_sFloatLabels = NULL;
  delete[] mp_sStringLabels; mp_sStringLabels = NULL;
  delete[] mp_sBoolLabels; mp_sBoolLabels = NULL;

  //Delete the package label arrays
  delete[] mp_sPackageIntLabels; mp_sPackageIntLabels = NULL;
  delete[] mp_sPackageFloatLabels; mp_sPackageFloatLabels = NULL;
  delete[] mp_sPackageStringLabels; mp_sPackageStringLabels = NULL;
  delete[] mp_sPackageBoolLabels; mp_sPackageBoolLabels = NULL;

  //Set the various values to zero - this way if this has been called before
  //there won't be an error trying to delete the values array again - i.e.
  //erroring out followed by destructor
  m_iNumXCells = 0;
  m_iNumYCells = 0;
  m_fXCellSize = 0;
  m_fYCellSize = 0;
  m_iNumIntVals = 0;
  m_iNumFloatVals = 0;
  m_iNumStringVals = 0;
  m_iNumBoolVals = 0;
  m_iNumPackageIntVals = 0;
  m_iNumPackageFloatVals = 0;
  m_iNumPackageStringVals = 0;
  m_iNumPackageBoolVals = 0;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetData(DOMDocument *p_oDoc) {
  try {
    if (NULL == p_oDoc) { //throw error
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sFunction = "clGrid::GetData";
      stcErr.sMoreInfo = "DOM document pointer passed was NULL.";
      throw(stcErr);
    }

    //Read a map file, if present
    ReadMapFile(p_oDoc);

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetData";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetValueAtPoint()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetValueAtPoint(float fX, float fY, short int iCode,
    int *p_iValHolder) {
  try {

    int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

    //Make sure the code is valid and the coordinates are within the grid - if
    //not throw an error
    if (iCode < 0 || iCode >= m_iNumIntVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueAtPoint (int)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    if (fX >= m_fXPlotLength || fX < 0
        || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueAtPoint (int)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    iXGrid = (int)floor(fX / m_fXCellSize);
    iYGrid = (int)floor(fY / m_fYCellSize);

    //Return the value pointed to by the indexes
    *p_iValHolder = mp_gridVals[iXGrid][iYGrid].p_iIntVals[iCode];
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetValueAtPoint (int)";
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// GetValueAtPoint()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetValueAtPoint(float fX, float fY,  short int iCode,
    float *p_fValHolder) {
  try {

    int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

    //Make sure the code is valid and the coordinates are within the grid - if
    //not throw an error
    if (iCode < 0 || iCode >= m_iNumFloatVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueAtPoint (float)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    if (fX >= m_fXPlotLength || fX < 0
        || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueAtPoint (float)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    iXGrid = (int)floor(fX / m_fXCellSize);
    iYGrid = (int)floor(fY / m_fYCellSize);

    //Return the value pointed to by the indexes
    *p_fValHolder = mp_gridVals[iXGrid][iYGrid].p_fFloatVals[iCode];
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetValueAtPoint (float)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetValueAtPoint()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetValueAtPoint(float fX, float fY, short int iCode,
    string *p_sValHolder) {
  try {

    int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

    //Make sure the code is valid and the coordinates are within the grid - if
    //not throw an error
    if (iCode < 0 || iCode >= m_iNumIntVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueAtPoint (char)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    if (fX >= m_fXPlotLength || fX < 0
        || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueAtPoint (char)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    iXGrid = (int)floor(fX / m_fXCellSize);
    iYGrid = (int)floor(fY / m_fYCellSize);

    //Return the value pointed to by the indexes
    *p_sValHolder = mp_gridVals[iXGrid][iYGrid].p_sStringVals[iCode];
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetValueAtPoint (char)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetValueAtPoint()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetValueAtPoint(float fX, float fY, short int iCode,
    bool *p_bValHolder) {
  try {

    int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

    //Make sure the code is valid and the coordinates are within the grid - if
    //not throw an error
    if (iCode < 0 || iCode >= m_iNumBoolVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueAtPoint (bool)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    if (fX >= m_fXPlotLength || fX < 0
        || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueAtPoint (bool)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    iXGrid = (int)floor(fX / m_fXCellSize);
    iYGrid = (int)floor(fY / m_fYCellSize);

    //Return the value pointed to by the indexes
    *p_bValHolder = mp_gridVals[iXGrid][iYGrid].p_bBoolVals[iCode];
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetValueAtPoint (bool)";
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// GetValueOfCell()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetValueOfCell(int iX, int iY, short int iCode,
    int *p_iValHolder) {
  try {
    //Make sure that the code is valid and that the grid coordinates are OK -
    //if not throw error
    if (iCode < 0 || iCode >= m_iNumIntVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueOfCell (int)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueOfCell (int)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << iX
          << " & Y = " << iY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    *p_iValHolder = mp_gridVals[iX][iY].p_iIntVals[iCode];

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetValueOfCell (int)";
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// GetValueOfCell()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetValueOfCell(int iX, int iY, short int iCode,
    float *p_fValHolder) {
  try {
    //Make sure that the code is valid and that the grid coordinates are OK -
    //if not throw error
    if (iCode < 0 || iCode >= m_iNumFloatVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueOfCell (float)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueOfCell (float)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << iX
          << " & Y = " << iY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    *p_fValHolder = mp_gridVals[iX][iY].p_fFloatVals[iCode];

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetValueOfCell (float)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetValueOfCell()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetValueOfCell(int iX, int iY, short int iCode,
    string *p_sValHolder) {
  try {
    //Make sure that the code is valid and that the grid coordinates are OK -
    //if not throw error
    if (iCode < 0 || iCode >= m_iNumStringVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueOfCell (char)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueOfCell (char)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << iX
          << " & Y = " << iY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    *p_sValHolder = mp_gridVals[iX][iY].p_sStringVals[iCode];

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetValueOfCell (char)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetValueOfCell()
/////////////////////////////////////////////////////////////////////////////
void clGrid::GetValueOfCell(int iX, int iY, short int iCode,
    bool *p_bValHolder) {
  try {
    //Make sure that the code is valid and that the grid coordinates are OK -
    //if not throw error
    if (iCode < 0 || iCode >= m_iNumBoolVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueOfCell (bool)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::GetValueOfCell (bool)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << iX
          << " & Y = " << iY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    *p_bValHolder = mp_gridVals[iX][iY].p_bBoolVals[iCode];

  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::GetValueOfCell (bool)";
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// GetAverageIntValue()
/////////////////////////////////////////////////////////////////////////////
float clGrid::GetAverageIntValue(float fX, float fY, short int iCode,
    float fRadius) {
  return GetAverageValue(fX, fY, iCode, fRadius, false);
}

/////////////////////////////////////////////////////////////////////////////
// GetAveragedoubleValue()
/////////////////////////////////////////////////////////////////////////////
float clGrid::GetAverageFloatValue(float fX, float fY, short int iCode,
    float fRadius) {
  return GetAverageValue(fX, fY, iCode, fRadius, true);
}

/////////////////////////////////////////////////////////////////////////////
// GetAverageValue()
/////////////////////////////////////////////////////////////////////////////
float clGrid::GetAverageValue(float fX, float fY, short int iCode,
    float fRadius, bool bFloat) {
  double fTemp, fTemp2, fFrac;  //for testing a special case
  float fAverage = 0,  //the average value to be returned
      fXDist = 0;  //distance in X direction - used when finding max and min
  //grid cells along the X axis
  int iNumGridCells = 0, //the number of grid cells included in the average
      //the max and min grid cells in the X and Y directions described by the circle
      iMinXGridCell = 0, iMinYGridCell = 0, iMaxXGridCell = 0,
      iMaxYGridCell = 0,
      iGoodX, iGoodY, //grid numbers corrected for torus
      iYGrid, iXGrid; //loop counters

  //Error checking - throw an error if any of the following is true:
  //The radius is less than 0, the point is not within the plot, the code is
  //invalid
  if (0 > iCode || (bFloat && iCode >= m_iNumFloatVals)
      || (!bFloat && iCode >= m_iNumIntVals)) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetAverageValue";
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Invalid data code - " << iCode;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  } else if (fX >= m_fXPlotLength || fX < 0
      || fY >= m_fYPlotLength || fY < 0) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetAverageValue";
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "At least one grid coordinate is outside of grid - X = " << fX
        << " & Y = " << fY;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  } else if (fRadius < 0) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetAverageValue";
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sMoreInfo = "Radius cannot be less than 0.";
    throw(stcErr);
  }

  //If the radius is 0 - just return the point in the grid cell in question
  if (0 == fRadius) {
    iGoodX = (int)floor(fX / m_fXCellSize);
    iGoodY = (int)floor(fY / m_fYCellSize);
    if (bFloat)
      return mp_gridVals[iGoodX][iGoodY].p_fFloatVals[iCode];
    else
      return mp_gridVals[iGoodX][iGoodY].p_iIntVals[iCode];
  }

  //find the limits of the circle in the positive and negative directions for Y
  //and translate into grid cell numbers
  //Shortcut:
  //If the distance cutoff is greater than 1/2 the Y plot length, only use
  //that value so that we don't wrap
  if (fRadius >= (m_fYCellSize*m_iNumYCells / 2)) {
    iMaxYGridCell = (int)(floor(floor(fY/m_fYCellSize)+(float)(m_iNumYCells/2.0)));
    iMinYGridCell = (int)(floor(floor(fY/m_fYCellSize)-(float)(m_iNumYCells/2.0))+1);
  } else {
    //iMaxYGridCell = ceil((fY + fRadius) / m_fYCellSize);
    iMaxYGridCell = (int)floor((fY + fRadius) / m_fYCellSize);
    iMinYGridCell = (int)floor((fY - fRadius) / m_fYCellSize);
  }

  //as we move along the grid cells on the Y axis of our circle, calculate the
  //distance in the X direction to the edges of the circle
  for (iYGrid = iMinYGridCell; iYGrid <= iMaxYGridCell; iYGrid++) {
    //we know XY of center, and Y of point on edge of circle - solve for X
    //X2 = X1 - square root(distance^2 - (Y1 - Y2)^2)
    //When we are south of the target Y, add one to grid cell number so we'll
    //be calculating the northern edge
    if ((iYGrid * m_fYCellSize) < fY)
      fXDist = fRadius * fRadius -
      ((fY - ((iYGrid+1)*m_fYCellSize)) * (fY - ((iYGrid+1)*m_fYCellSize)));
    else
      fXDist = fRadius * fRadius -
      ((fY - (iYGrid * m_fYCellSize)) * (fY - (iYGrid * m_fYCellSize)));

    if (fXDist > 0) {
      fXDist = sqrt(fXDist);
      //If the distance is greater than 1/2 X plot length, change to be 1/2
      //X plot length so we don't wrap around the plot
      if (fXDist >= (m_fYCellSize*m_iNumYCells / 2)) {
        iMaxXGridCell =
            (int)floor(floor(fX/m_fXCellSize)+(float)(m_iNumXCells/2.0));
        iMinXGridCell =
            (int)floor(floor(fX/m_fXCellSize)-(float)(m_iNumXCells/2.0))+1;
      } else {
        //fXDist is positive (east) and negative (west) solution
        //special case - if the max X falls exactly on the intersection of
        //two grid cell lines, AND the current Y is to the south, decrement
        //max by one
        fTemp = (fX + fXDist) / m_fXCellSize;
        fFrac = modf(fTemp, &fTemp2);
        if (fFrac == 0 && (iYGrid*m_fYCellSize < fY)) {
          iMaxXGridCell = (int)floor(fTemp) - 1;
          iMinXGridCell = (int)floor((fX - fXDist) / m_fXCellSize);
        } else {//Normal case - fTemp is not an integer
          iMaxXGridCell = (int)floor(fTemp);
          iMinXGridCell = (int)floor((fX - fXDist) / m_fXCellSize);
        }
      }
    } else if (fXDist == 0) {
      //The radius falls on a grid cell boundary.  If we are south of the
      //target - grab the cells on either side of the boundary.  If we are
      //north - grab only the cell to the east
      if ((iYGrid * m_fYCellSize) < fY) {
        iMaxXGridCell = (int)floor(fX / m_fXCellSize);
        iMinXGridCell = iMaxXGridCell - 1;
      } else {
        iMaxXGridCell = (int)floor(fX / m_fXCellSize);
        iMinXGridCell = iMaxXGridCell;
      }
    } else {  //a solution < 0 means that there's no triangle - there are no
      //cells to the east or west of the current one
      iMaxXGridCell = (int)(floor(fX) / m_fXCellSize);
      iMinXGridCell = iMaxXGridCell;
    }

    //Correct the iYGrid value for the torus, if necessary
    if (iYGrid >= m_iNumYCells) iGoodY = iYGrid - m_iNumYCells;
    else if (iYGrid < 0) iGoodY = iYGrid + m_iNumYCells;
    else iGoodY = iYGrid;

    //Now that we have the grid cells, add their values to the running total
    //being kept in fAverage
    for (iXGrid = iMinXGridCell; iXGrid <= iMaxXGridCell; iXGrid++) {

      //Correct the iXGrid value for the torus, if necessary
      if (iXGrid >= m_iNumXCells) iGoodX = iXGrid - m_iNumXCells;
      else if (iXGrid < 0) iGoodX = iXGrid + m_iNumXCells;
      else iGoodX = iXGrid;

      if (bFloat)
        fAverage += mp_gridVals[iGoodX][iGoodY].p_fFloatVals[iCode];
      else
        fAverage += mp_gridVals[iGoodX][iGoodY].p_iIntVals[iCode];
      iNumGridCells++;
    }
  } //end of for (iYGrid = iMinYGridCell; iYGrid <= iMaxYGridCell; iYGrid++)

  //Now that we have a running total of all values, divide in number of grid
  //cells to get the average and return it
  fAverage /= iNumGridCells;
  return fAverage;
}

/////////////////////////////////////////////////////////////////////////////
// GetAverageIntValue()
/////////////////////////////////////////////////////////////////////////////
float clGrid::GetAverageIntValue(float fFromX, float fFromY, float fToX, float fToY,
    short int iCode) {
  return GetAverageValue(fFromX, fFromY, fToX, fToY, iCode, false);
}

/////////////////////////////////////////////////////////////////////////////
// GetAveragedoubleValue()
/////////////////////////////////////////////////////////////////////////////
float clGrid::GetAverageFloatValue(float fFromX, float fFromY, float fToX, float fToY,
    short int iCode) {
  return GetAverageValue(fFromX, fFromY, fToX, fToY, iCode, true);
}

/////////////////////////////////////////////////////////////////////////////
// GetAverageValue()
/////////////////////////////////////////////////////////////////////////////
float clGrid::GetAverageValue(float fFromX, float fFromY, float fToX, float fToY,
    short int iCode, bool bFloat) {
  float fAverage = 0;  //the average value to be returned
  int iMinXGridCell = (int)floor(fFromX/m_fXCellSize) ,  //minimum X grid
      iMinYGridCell = (int)floor(fFromY/m_fYCellSize),  //minimum Y grid
      iMaxXGridCell = (int)floor(fToX/m_fXCellSize),  //maximum X grid
      iMaxYGridCell = (int)floor(fToY/m_fYCellSize),  //maximum Y grid
      iNumCells = 0,
      iYGrid, iXGrid; //loop counters

  //Error checking - throw an error if any of the following is true:
  //The points are not within the plot, the "from" point is less than the "to"
  //point, or the code is invalid
  if (0 > iCode || (bFloat && iCode >= m_iNumFloatVals)
      || (!bFloat && iCode >= m_iNumIntVals)) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetAverageValue";
    stcErr.iErrorCode = BAD_DATA;
    std::stringstream s;
    s << "Invalid data code - " << iCode;
    stcErr.sMoreInfo = s.str();
    throw(stcErr);
  } else if (fFromX >= m_fXPlotLength || fFromX < 0 ||
      fToX >= m_fXPlotLength || fToX < 0 ||
      fFromY >= m_fYPlotLength || fFromY < 0 ||
      fToY >= m_fYPlotLength || fToY < 0) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetAverageValue";
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sMoreInfo = "At least one grid coordinate is outside of grid.";
    throw(stcErr);
  } else if (fFromX > fToX || fFromY > fToY) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetAverageValue";
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sMoreInfo = "The rectangle has negative area.";
    throw(stcErr);
  }

  for (iXGrid = iMinXGridCell; iXGrid <= iMaxXGridCell; iXGrid++) {
    for (iYGrid = iMinYGridCell; iYGrid <= iMaxYGridCell; iYGrid++) {

      iNumCells++;

      if (bFloat)
        fAverage += mp_gridVals[iXGrid][iYGrid].p_fFloatVals[iCode];
      else
        fAverage += mp_gridVals[iXGrid][iYGrid].p_iIntVals[iCode];
    }
  }
  //Now that we have a running total of all values, divide in number of grid
  //cells to get the average and return it
  fAverage /= iNumCells;
  return fAverage;
}


/////////////////////////////////////////////////////////////////////////////
// GetOriginXOfCell()
////////////////////////////////////////////////////////////////////////////
float clGrid::GetOriginXOfCell(int iX) {
  if (iX < 0 || iX >= m_iNumXCells) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetOriginXOfCell";
    stcErr.sMoreInfo = "X grid number is not inside the plot.";
    stcErr.iErrorCode = BAD_ARGUMENT;
    throw(stcErr);
  }

  return iX*m_fXCellSize;
}

/////////////////////////////////////////////////////////////////////////////
// GetOriginYOfCell()
////////////////////////////////////////////////////////////////////////////
float clGrid::GetOriginYOfCell(int iY) {
  if (iY < 0 || iY >= m_iNumYCells) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetOriginYOfCell";
    stcErr.sMoreInfo = "Y grid number is not inside the plot.";
    stcErr.iErrorCode = BAD_ARGUMENT;
    throw(stcErr);
  }

  return iY*m_fYCellSize;
}

/////////////////////////////////////////////////////////////////////////////
// GetEndXOfCell()
////////////////////////////////////////////////////////////////////////////
float clGrid::GetEndXOfCell(int iX) {
  if (iX < 0 || iX >= m_iNumXCells) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetEndXOfCell";
    stcErr.sMoreInfo = "X grid number is not inside the plot.";
    stcErr.iErrorCode = BAD_ARGUMENT;
    throw(stcErr);
  }

  float fValue = ((iX+1)*m_fXCellSize) - VERY_SMALL_VALUE;
  if (fValue < m_fXPlotLength) return fValue;
  else return (m_fXPlotLength - VERY_SMALL_VALUE);
}

/////////////////////////////////////////////////////////////////////////////
// GetEndYOfCell()
////////////////////////////////////////////////////////////////////////////
float clGrid::GetEndYOfCell(int iY) {
  if (iY < 0 || iY >= m_iNumYCells) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::GetEndYOfCell";
    stcErr.sMoreInfo = "Y grid number is not inside the plot.";
    stcErr.iErrorCode = BAD_ARGUMENT;
    throw(stcErr);
  }

  float fValue = ((iY+1)*m_fYCellSize) - VERY_SMALL_VALUE;
  if (fValue < m_fYPlotLength) return fValue;
  else return (m_fYPlotLength - VERY_SMALL_VALUE);
}

/////////////////////////////////////////////////////////////////////////////
// SetValueAtPoint()
////////////////////////////////////////////////////////////////////////////
void clGrid::SetValueAtPoint(float fX, float fY, short int iCode, float fValue) {
  try {

    int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

    //Make sure the data member code is valid and coordinates are within the
    //grid.  If not, throw error
    if (iCode < 0 || iCode >= m_iNumFloatVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueAtPoint (float)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (fX >= m_fXPlotLength || fX < 0
        || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueAtPoint (float)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    iXGrid = (int)floor(fX / m_fXCellSize);
    iYGrid = (int)floor(fY / m_fYCellSize);

    //Set the value pointed to by the indexes
    mp_gridVals[iXGrid][iYGrid].p_fFloatVals[iCode] = fValue;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::SetValueAtPoint (float)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetValueAtPoint()
/////////////////////////////////////////////////////////////////////////////
void clGrid::SetValueAtPoint(float fX, float fY, short int iCode, int iValue) {
  try {

    int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

    //Make sure the data member code is valid and coordinates are within the
    //grid.  If not, throw error
    if (iCode < 0 || iCode >= m_iNumIntVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueAtPoint (int)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (fX >= m_fXPlotLength || fX < 0
        || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueAtPoint (int)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    iXGrid = (int)floor(fX / m_fXCellSize);
    iYGrid = (int)floor(fY / m_fYCellSize);

    //Set the value pointed to by the indexes
    mp_gridVals[iXGrid][iYGrid].p_iIntVals[iCode] = iValue;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::SetValueAtPoint (int)";
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// SetValueAtPoint()
/////////////////////////////////////////////////////////////////////////////
void clGrid::SetValueAtPoint(float fX, float fY, short int iCode, string sValue) {
  try {

    int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

    //Make sure the data member code is valid and coordinates are within the
    //grid.  If not, throw error
    if (iCode < 0 || iCode >= m_iNumStringVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueAtPoint (char)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (fX >= m_fXPlotLength || fX < 0
        || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueAtPoint (char)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    iXGrid = (int)floor(fX / m_fXCellSize);
    iYGrid = (int)floor(fY / m_fYCellSize);

    //Set the value pointed to by the indexes - don't go over the max val. size
    mp_gridVals[iXGrid][iYGrid].p_sStringVals[iCode] = sValue;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::SetValueAtPoint (char)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetValueAtPoint()
/////////////////////////////////////////////////////////////////////////////
void clGrid::SetValueAtPoint(float fX, float fY, short int iCode, bool bValue) {
  try {

    int iXGrid = 0, iYGrid = 0; //the X and Y grid indexes for the values array

    //Make sure the data member code is valid and coordinates are within the
    //grid.  If not, throw error
    if (iCode < 0 || iCode >= m_iNumBoolVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueAtPoint (bool)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (fX >= m_fXPlotLength || fX < 0
        || fY >= m_fYPlotLength || fY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueAtPoint (bool)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << fX
          << " & Y = " << fY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Divide the X and Y values by the size of the grid divisions to get grid
    //cell indexes
    iXGrid = (int)floor(fX / m_fXCellSize);
    iYGrid = (int)floor(fY / m_fYCellSize);

    //Set the value pointed to by the indexes
    mp_gridVals[iXGrid][iYGrid].p_bBoolVals[iCode] = bValue;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::SetValueAtPoint (bool)";
    throw(stcErr);
  }
}


/////////////////////////////////////////////////////////////////////////////
// SetValueOfCell()
/////////////////////////////////////////////////////////////////////////////
void clGrid::SetValueOfCell(int iX, int iY, short int iCode, float fValue) {
  try {
    //Make sure the data member code is valid and coordinates are within the
    //grid.  If not, throw error
    if (iCode < 0 || iCode >= m_iNumFloatVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueOfCell (float)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueOfCell (float)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << iX
          << " & Y = " << iY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    else mp_gridVals[iX][iY].p_fFloatVals[iCode] = fValue;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::SetValueOfCell (float)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetValueOfCell()
/////////////////////////////////////////////////////////////////////////////
void clGrid::SetValueOfCell(int iX, int iY, short int iCode, int iValue) {
  try {
    //Make sure the data member code is valid and coordinates are within the
    //grid.  If not, throw error
    if (iCode < 0 || iCode >= m_iNumIntVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueOfCell (int)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueOfCell (int)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << iX
          << " & Y = " << iY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    else mp_gridVals[iX][iY].p_iIntVals[iCode] = iValue;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::SetValueOfCell (int)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetValueOfCell()
/////////////////////////////////////////////////////////////////////////////
void clGrid::SetValueOfCell(int iX, int iY, short int iCode, string sValue){
  try {
    //Make sure the data member code is valid and coordinates are within the
    //grid.  If not, throw error
    if (iCode < 0 || iCode >= m_iNumStringVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueOfCell (char)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueOfCell (char)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << iX
          << " & Y = " << iY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    mp_gridVals[iX][iY].p_sStringVals[iCode] = sValue;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::SetValueOfCell (char)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetValueOfCell()
/////////////////////////////////////////////////////////////////////////////
void clGrid::SetValueOfCell(int iX, int iY, short int iCode, bool bValue) {
  try {
    //Make sure the data member code is valid and coordinates are within the
    //grid.  If not, throw error
    if (iCode < 0 || iCode >= m_iNumBoolVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueOfCell (bool)";
      std::stringstream s;
      s << "Invalid data code - " << iCode;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    } else if (iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::SetValueOfCell (bool)";
      std::stringstream s;
      s << "At least one grid coordinate is outside of grid - X = " << iX
          << " & Y = " << iY;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    else mp_gridVals[iX][iY].p_bBoolVals[iCode] = bValue;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::SetValueOfCell (float)";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// ReadMapFile()
/////////////////////////////////////////////////////////////////////////////
void clGrid::ReadMapFile(DOMDocument *p_oDoc) {
  try {
    clPlot *p_oPlot;
    clPackage *p_oPackage,        //for assigning and removing packages
    *p_oNextPackage;
    DOMNodeList *p_oNodeList,   //for searching for tags
    *p_oPackageList, //for searching for packages
    *p_oChildren;   //for searching for children
    DOMNode *p_oNode;           //for capturing a particular tag
    DOMElement *p_oElement,     //for casting a tag to access its data
    *p_oCell,        //for working with one grid cell's data
    *p_oPackageEl,    //for working with one package's data
    *p_oGrid;        //this grid's data
    DOMAttr *p_oAttr;           //XML tag attribute
    XMLCh *sVal;
    struct stcCodeTrans {       //for translating the data member codes in map
      short int iCodeInMap,     //what the code is in the map
      iCodeForGrid;   //what the code is to the grid
    } *p_ints = NULL, *p_floats = NULL,  //array for each data member type
        *p_chars = NULL, *p_bools = NULL,
        *p_cohInts = NULL, *p_cohFloats = NULL, //packages
        *p_cohChars = NULL, *p_cohBools = NULL;
    char *cData;                //for capturing text data
    string sTemp;         //For initializing grid values
    float fMapXLen, fMapYLen,   //X & Y grid lengths from map
    fCellXLen, fCellYLen, //Length of grid cells in X and Y directions
    //          f,                    //Loop counter for potentially large values
    fTemp = 0;            //For initializing grid values
    int iNumGridVals,           //Number of grid values in the map file
    iXGridVal, iYGridVal,   //X and Y grid numbers for a map value
    iNumElements,
    iNumCellVals,           //Number of values of a particular data type in
    //one cell
    iNumPackages,           //Number of packages for a cell
    i, j, k, m,             //loop counters
    iTemp = 0;              //For initializing grid values
    short int iNumInts, iNumFloats, iNumStrings, iNumBools,
    iNumCohInts, iNumCohFloats, iNumCohStrings, iNumCohBools,
    iMapCode,         //codes for values read from the map file
    iGridCode;        //code translated from map to grid values
    bool bFound = false,        //for searching for a map in par file
        bTemp = false;         //For initializing grid values


    //This could be a parameter file - we're going to have to get the portion
    //of it that deals with this grid if it is
    sVal = XMLString::transcode("grid");
    p_oNodeList = p_oDoc->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumElements = p_oNodeList->getLength();
    if (0 == iNumElements) //nothing there - exit
      return;
    else { //hunt through until we find the map for this grid
      for (i = 0; i < iNumElements; i++) {
        p_oNode = p_oNodeList->item(i);
        p_oGrid = (DOMElement *)p_oNode;
        sVal = XMLString::transcode("gridName");
        cData = XMLString::transcode(p_oGrid->getAttributeNode(sVal)->getNodeValue());
        XMLString::release(&sVal);
        if (m_sNameString.compare(cData) == 0) {
          p_oGrid = (DOMElement *) p_oNode;
          bFound = true;
          delete[] cData; cData = NULL;
          break;
        }
        delete[] cData; cData = NULL;
      }
      if (!bFound) //we didn't find a map
        return;
    }

    //*************************************************
    // Read the map header
    //*************************************************
    //Get the plot size for this grid.  If the data is present in the file and
    //doesn't match what's loaded for the run, throw error.

    //Get the plot object
    p_oPlot = mp_oSimManager->GetPlotObject();
    //X plot length
    sVal = XMLString::transcode("ma_plotLenX");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength())
      fMapXLen = p_oPlot->GetXPlotLength(); //ensure a match
    else {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
      fMapXLen = atof(cData);
      delete[] cData; cData = NULL;
    } //end of else

    //Y plot length
    sVal = XMLString::transcode("ma_plotLenY");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength())
      fMapYLen = p_oPlot->GetYPlotLength(); //ensure a match
    else {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
      fMapYLen = atof(cData);
      delete[] cData; cData = NULL;
    } //end of else

    //Make sure the plot lengths match what's been loaded from the par file
    //If not - throw error
    if (fMapXLen != p_oPlot->GetXPlotLength()
        || fMapYLen != p_oPlot->GetYPlotLength()) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::ReadMapFile";
      std::stringstream s;
      s << "A plot dimension for this map doesn't match the plot.  Map X = "
          << fMapXLen << " Map Y = " << fMapYLen;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Get the grid cell size for this grid, if present.  If the data is present
    //in the file and doesn't match what's loaded for the run, throw error.
    //X grid length
    sVal = XMLString::transcode("ma_lengthXCells");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength())
      fCellXLen = m_fXCellSize;
    else {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
      fCellXLen = atof(cData);
      delete[] cData; cData = NULL;
    } //end of else

    //Y Grid Length
    sVal = XMLString::transcode("ma_lengthYCells");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength())
      fCellYLen = m_fYCellSize;
    else {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      cData = XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
      fCellYLen = atof(cData);
      delete[] cData; cData = NULL;
    } //end of else

    //Validate grid cell size data
    if (fCellXLen != m_fXCellSize || fCellYLen != m_fYCellSize) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGrid::ReadMapFile";
      std::stringstream s;
      s << "One of the grid cell sizes for this map doesn't match.  X = "
          << fCellXLen << " Y = " << fCellYLen;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }

    //Validate data members
    //Make sure the data codes for each member are already registered
    //***************Ints*****************
    sVal = XMLString::transcode("ma_intCodes");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength()) {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("ma_intCode");
      p_oChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumInts = p_oChildren->getLength();
      if (iNumInts > 0) {
        //Declare the translation array for the int codes
        p_ints = new stcCodeTrans[iNumInts];
        for (i = 0; i < iNumInts; i++) {
          //Get the code for this data member according to the map
          p_oNode = p_oChildren->item(i);
          p_oElement = (DOMElement *) p_oNode;
          cData = XMLString::transcode(p_oElement->getFirstChild()->
              getNodeValue());
          p_ints[(int)i].iCodeInMap = atoi(cData);
          //If it's not a number throw an error
          if (0 == p_ints[i].iCodeInMap && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Can't convert code to number: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          //Get the data label, which is an attribute of the tag
          sVal = XMLString::transcode("label");
          cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
          XMLString::release(&sVal);
          p_ints[(int)i].iCodeForGrid = GetIntDataCode(cData);
          if (-1 == p_ints[i].iCodeForGrid) { //unrecognized label - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Unrecognized data member label: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == p_ints[i].iCodeForGrid)
          delete[] cData; cData = NULL;
        }  //end of for (i = 0; i < iNumInts; i++)
      } //end of if (iNumInts > 0)
    }  //end of if (0 != p_oNodeList->getLength())-int data member processing

    //***************Floats*****************
    sVal = XMLString::transcode("ma_floatCodes");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength()) {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("ma_floatCode");
      p_oChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumFloats = p_oChildren->getLength();
      if (iNumFloats > 0) {
        //Declare the translation array for the float codes
        p_floats = new stcCodeTrans[iNumFloats];
        for (i = 0; i < iNumFloats; i++) {
          //Get the code for this data member according to the map
          p_oNode = p_oChildren->item(i);
          p_oElement = (DOMElement *) p_oNode;
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          p_floats[i].iCodeInMap = atoi(cData);
          //If it's not a number throw an error
          if (0 == p_floats[i].iCodeInMap && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Can't convert code to number: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          //Get the data label, which is an attribute of the tag
          sVal = XMLString::transcode("label");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          XMLString::release(&sVal);
          p_floats[i].iCodeForGrid = GetFloatDataCode(cData);
          if (-1 == p_floats[i].iCodeForGrid) {//unrecognized label- throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Unrecognized data member label: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == p_floats[i].iCodeForGrid)
          delete[] cData; cData = NULL;
        }  //end of for (i = 0; i < iNumFloats; i++)
      } //end of if (iNumFloats > 0)
    }  //end of if (0 != p_oNodeList->getLength())-for float data member processing

    //***************Strings*****************
    sVal = XMLString::transcode("ma_charCodes");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength()) {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("ma_charCode");
      p_oChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumStrings = p_oChildren->getLength();
      if (iNumStrings > 0) {
        //Declare the translation array for the int codes
        p_chars = new stcCodeTrans[iNumStrings];
        for (i = 0; i < iNumStrings; i++) {
          //Get the code for this data member according to the map
          p_oNode = p_oChildren->item(i);
          p_oElement = (DOMElement *) p_oNode;
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          p_chars[i].iCodeInMap = atoi(cData);
          //If it's not a number throw an error
          if (0 == p_chars[i].iCodeInMap && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Can't convert code to number: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          //Get the data label, which is an attribute of the tag
          sVal = XMLString::transcode("label");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          XMLString::release(&sVal);
          sTemp = cData;
          p_chars[i].iCodeForGrid = GetStringDataCode(sTemp);
          if (-1 == p_chars[i].iCodeForGrid) {//unrecognized label - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Unrecognized data member label: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == p_chars[i].iCodeForGrid)
          delete[] cData; cData = NULL;
        }  //end of for (i = 0; i < iNumChars; i++)
      } //end of if (iNumChars > 0)
    }  //end of if (0 != p_oNodeList->getLength())- for char data member processing

    //***************Bools*****************
    sVal = XMLString::transcode("ma_boolCodes");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength()) {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("ma_boolCode");
      p_oChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumBools = p_oChildren->getLength();
      if (iNumBools > 0) {
        //Declare the translation array for the int codes
        p_bools = new stcCodeTrans[iNumBools];
        for (i = 0; i < iNumBools; i++) {
          //Get the code for this data member according to the map
          p_oNode = p_oChildren->item(i);
          p_oElement = (DOMElement *) p_oNode;
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          p_bools[i].iCodeInMap = atoi(cData);
          //If it's not a number throw an error
          if (0 == p_bools[i].iCodeInMap && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Can't convert code to number: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          //Get the data label, which is an attribute of the tag
          sVal = XMLString::transcode("label");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          XMLString::release(&sVal);
          p_bools[i].iCodeForGrid = GetBoolDataCode(cData);
          if (-1 == p_bools[i].iCodeForGrid) {//unrecognized label - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Unrecognized data member label: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == p_bools[i].iCodeForGrid)
          delete[] cData; cData = NULL;
        }  //end of for (i = 0; i < iNumBools; i++)
      } //end of if (iNumBools > 0)
    }  //end of if (0 != p_oNodeList->getLength() - bool data member processing





    //As with regular data members - so too with package data members
    //*************** Package Ints*****************
    sVal = XMLString::transcode("ma_packageIntCodes");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength()) {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("ma_intCode");
      p_oChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCohInts = p_oChildren->getLength();
      if (iNumCohInts > 0) {
        //Declare the translation array for the int codes
        p_cohInts = new stcCodeTrans[iNumCohInts];
        for (i = 0; i < iNumCohInts; i++) {
          //Get the code for this data member according to the map
          p_oNode = p_oChildren->item(i);
          p_oElement = (DOMElement *) p_oNode;
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          p_cohInts[i].iCodeInMap = atoi(cData);
          //If it's not a number throw an error
          if (0 == p_cohInts[i].iCodeInMap && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Can't convert code to number: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          //Get the data label, which is an attribute of the tag
          sVal = XMLString::transcode("label");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          XMLString::release(&sVal);
          p_cohInts[i].iCodeForGrid = GetPackageIntDataCode(cData);
          if (-1 == p_cohInts[i].iCodeForGrid) {//unrecognized label-throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Unrecognized data member label: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == p_cohInts[i].iCodeForGrid)
          delete[] cData; cData = NULL;
        }  //end of for (i = 0; i < iNumCohInts; i++)
      } //end of if (iNumCohInts > 0)
    }  //end of if (0 != p_oNodeList->getLength()) - int data member processing

    //***************Package Floats*****************
    sVal = XMLString::transcode("ma_packageFloatCodes");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength()) {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("ma_floatCode");
      p_oChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCohFloats = p_oChildren->getLength();
      if (iNumCohFloats > 0) {
        //Declare the translation array for the float codes
        p_cohFloats = new stcCodeTrans[iNumCohFloats];
        for (i = 0; i < iNumCohFloats; i++) {
          //Get the code for this data member according to the map
          p_oNode = p_oChildren->item(i);
          p_oElement = (DOMElement *) p_oNode;
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          p_cohFloats[i].iCodeInMap = atoi(cData);
          //If it's not a number throw an error
          if (0 == p_cohFloats[i].iCodeInMap && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Can't convert code to number: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          //Get the data label, which is an attribute of the tag
          sVal = XMLString::transcode("label");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          XMLString::release(&sVal);
          p_cohFloats[i].iCodeForGrid = GetPackageFloatDataCode(cData);
          if (-1 == p_cohFloats[i].iCodeForGrid) {//unknown label-throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Unrecognized data member label: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == p_cohFloats[i].iCodeForGrid)
          delete[] cData; cData = NULL;
        }  //end of for (i = 0; i < iNumCohFloats; i++)
      } //end of if (iNumCohFloats > 0)
    }  //end of if (0 != p_oNodeList->getLength())-float data member processing

    //***************Package strings*****************
    sVal = XMLString::transcode("ma_packageCharCodes");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength()) {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("ma_charCode");
      p_oChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCohStrings = p_oChildren->getLength();
      if (iNumCohStrings > 0) {
        //Declare the translation array for the int codes
        p_cohChars = new stcCodeTrans[iNumCohStrings];
        for (i = 0; i < iNumCohStrings; i++) {
          //Get the code for this data member according to the map
          p_oNode = p_oChildren->item((int)i);
          p_oElement = (DOMElement *) p_oNode;
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          p_cohChars[i].iCodeInMap = atoi(cData);
          //If it's not a number throw an error
          if (0 == p_cohChars[i].iCodeInMap && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Can't convert code to number: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          //Get the data label, which is an attribute of the tag
          sVal = XMLString::transcode("label");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          XMLString::release(&sVal);
          sTemp = cData;
          p_cohChars[i].iCodeForGrid = GetPackageStringDataCode(sTemp);
          if (-1 == p_cohChars[i].iCodeForGrid) {//unknown label - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Unrecognized data member label: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == p_cohChars[i].iCodeForGrid)
          delete[] cData; cData = NULL;
        }  //end of for (i = 0; i < iNumCohChars; i++)
      } //end of if (iNumCohChars > 0)
    }  //end of if (0 != p_oNodeList->getLength())- char data member processing

    //***************Bools*****************
    sVal = XMLString::transcode("ma_packageBoolCodes");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 != p_oNodeList->getLength()) {
      p_oNode = p_oNodeList->item(0);
      p_oElement = (DOMElement *) p_oNode;
      sVal = XMLString::transcode("ma_boolCode");
      p_oChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCohBools = p_oChildren->getLength();
      if (iNumCohBools > 0) {
        //Declare the translation array for the int codes
        p_cohBools = new stcCodeTrans[iNumCohBools];
        for (i = 0; i < iNumCohBools; i++) {
          //Get the code for this data member according to the map
          p_oNode = p_oChildren->item(i);
          p_oElement = (DOMElement *) p_oNode;
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          p_cohBools[i].iCodeInMap = atoi(cData);
          //If it's not a number throw an error
          if (0 == p_cohBools[i].iCodeInMap && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Can't convert code to number: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          //Get the data label, which is an attribute of the tag
          sVal = XMLString::transcode("label");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          XMLString::release(&sVal);
          p_cohBools[i].iCodeForGrid = GetPackageBoolDataCode(cData);
          if (-1 == p_cohBools[i].iCodeForGrid) {//unknown label - throw error
            modelErr stcErr;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.sMoreInfo = "Unrecognized data member label: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          } //end of if (-1 == p_cohBools[i].iCodeForGrid)
          delete[] cData; cData = NULL;
        }  //end of for (i = 0; i < iNumCohBools; i++)
      } //end of if (iNumCohBools > 0)
    }  //end of if (0 != p_oNodeList->getLength() - bool data member processing




    //*************************************************
    // Read the map values
    //*************************************************
    //Initialize all the grid values to zero, empty, etc. so if there are any
    //values  missing from the file they will be zero
    for (i = 0; i < m_iNumXCells; i++)
      for (j = 0; j < m_iNumYCells; j++) {
        for (k = 0; k < m_iNumIntVals; k++) SetValueOfCell(i, j, k, iTemp);
        for (k = 0; k < m_iNumFloatVals; k++)  SetValueOfCell(i, j, k, fTemp);
        for (k = 0; k < m_iNumStringVals; k++) SetValueOfCell(i, j, k, "");
        for (k = 0; k < m_iNumBoolVals; k++) SetValueOfCell(i, j, k, bTemp);
        //Delete all packages
        p_oPackage = mp_gridVals[i][j].p_oPackage;
        while (p_oPackage) {
          p_oNextPackage = p_oPackage->GetNextPackage();
          delete p_oPackage;
          p_oPackage = p_oNextPackage;
        }
        mp_gridVals[i][j].p_oPackage = NULL;
      }

    //Set up a loop through the grid values in the map and assign them
    sVal = XMLString::transcode("ma_v");
    p_oNodeList = p_oGrid->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumGridVals = p_oNodeList->getLength();
    for (i = 0; i < iNumGridVals; i++) {
      p_oNode = p_oNodeList->item(i);
      p_oCell = (DOMElement *) p_oNode;
      if (NULL == p_oCell) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clGrid::ReadMapFile";
        std::stringstream s;
        s << "Can't find cell # " << i << " for grid " << m_sNameString;
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }

      //Get the X and Y grid values
      sVal = XMLString::transcode("x");
      p_oAttr = p_oCell->getAttributeNode(sVal);
      XMLString::release(&sVal);
      if (NULL == p_oAttr) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clGrid::ReadMapFile";
        stcErr.sMoreInfo = "Can't find X attribute for a cell for grid ";
        stcErr.sMoreInfo += m_sNameString;
        throw(stcErr);
      }
      cData = XMLString::transcode(p_oAttr->getNodeValue());
      iXGridVal = atoi(cData);
      delete[] cData; cData = NULL;
      sVal = XMLString::transcode("y");
      p_oAttr = p_oCell->getAttributeNode(sVal);
      XMLString::release(&sVal);
      if (NULL == p_oAttr) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clGrid::ReadMapFile";
        stcErr.sMoreInfo = "Can't find X attribute for a cell for grid ";
        stcErr.sMoreInfo += m_sNameString;
        throw(stcErr);
      }
      cData = XMLString::transcode(p_oAttr->getNodeValue());
      iYGridVal = atoi(cData);
      delete[] cData; cData = NULL;
      //      iXGridVal = atoi(XMLString::transcode(p_oCell->getAttributeNode(
      //           XMLString::transcode("x"))->getNodeValue()));
      //      iYGridVal = atoi(XMLString::transcode(p_oCell->getAttributeNode(
      //           XMLString::transcode("y"))->getNodeValue()));

      //*******************Int values*****************
      sVal = XMLString::transcode("int");
      p_oChildren = p_oCell->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCellVals = p_oChildren->getLength();
      for (j = 0; j < iNumCellVals; j++) {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *)p_oNode;
        //Get the value's code and translate it to the grid's code
        iGridCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(
            sVal)->getNodeValue());
        iMapCode = atoi(cData);
        delete[] cData; cData = NULL;
        XMLString::release(&sVal);
        for (k = 0; k < iNumInts; k++)
          if (iMapCode == p_ints[k].iCodeInMap) {
            iGridCode = p_ints[k].iCodeForGrid;
            break;
          }
        if (-1 == iGridCode) { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clGrid::ReadMapFile";
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized int code " << iMapCode << " in map " << m_sNameString;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        cData =
            XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        iTemp = atoi(cData);
        //Make sure it's a valid int
        if (0 == iTemp && strcmp(cData, "0") != 0) {
          modelErr stcErr;
          stcErr.sFunction = "clGrid::ReadMapFile";
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sMoreInfo = "Invalid int value in map: ";
          stcErr.sMoreInfo += cData;
          delete[] cData; cData = NULL;
          throw(stcErr);
        }
        delete[] cData; cData = NULL;
        SetValueOfCell(iXGridVal, iYGridVal, iGridCode, iTemp);
      } //end of for (j = 0; j < iNumCellVals; j++) - ints

      //*******************Float values*****************
      sVal = XMLString::transcode("fl");
      p_oChildren = p_oCell->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCellVals = p_oChildren->getLength();
      for (j = 0; j < iNumCellVals; j++) {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *)p_oNode;
        //Get the value's code and translate it to the grid's code
        iGridCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(
            sVal)->getNodeValue());
        XMLString::release(&sVal);
        iMapCode = atoi(cData);
        delete[] cData; cData = NULL;
        for (k = 0; k < iNumFloats; k++)
          if (iMapCode == p_floats[k].iCodeInMap) {
            iGridCode = p_floats[k].iCodeForGrid;
            break;
          }
        if (-1 == iGridCode) { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clGrid::ReadMapFile";
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized float code " << iMapCode << " in map " << m_sNameString;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        cData =
            XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        fTemp = atof(cData);
        //Make sure it's a valid float
        if (0 == fTemp && cData[0] != '0') {
          modelErr stcErr;
          stcErr.sFunction = "clGrid::ReadMapFile";
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sMoreInfo = "Invalid float value in map: ";
          stcErr.sMoreInfo += cData;
          delete[] cData; cData = NULL;
          throw(stcErr);
        }
        delete[] cData; cData = NULL;
        SetValueOfCell(iXGridVal, iYGridVal, iGridCode, fTemp);
      } //end of for (j = 0; j < iNumCellVals; j++) - floats

      //*******************String values*****************
      sVal = XMLString::transcode("ch");
      p_oChildren = p_oCell->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCellVals = p_oChildren->getLength();
      for (j = 0; j < iNumCellVals; j++) {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *)p_oNode;
        //Get the value's code and translate it to the grid's code
        iGridCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(sVal)->getNodeValue());
        iMapCode = atoi(cData);
        delete[] cData; cData = NULL;
        XMLString::release(&sVal);
        for (k = 0; k < iNumStrings; k++)
          if (iMapCode == p_chars[k].iCodeInMap) {
            iGridCode = p_chars[k].iCodeForGrid;
            break;
          }
        if (-1 == iGridCode) { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clGrid::ReadMapFile";
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized char code " << iMapCode << " in map " << m_sNameString;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        cData =
            XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        sTemp = cData;
        delete[] cData; cData = NULL;
        SetValueOfCell(iXGridVal, iYGridVal, iGridCode, sTemp);
      } //end of for (j = 0; j < iNumCellVals; j++) - chars

      //*******************Bool values*****************
      sVal = XMLString::transcode("bl");
      p_oChildren = p_oCell->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCellVals = p_oChildren->getLength();
      for (j = 0; j < iNumCellVals; j++) {
        p_oNode = p_oChildren->item(j);
        p_oElement = (DOMElement *)p_oNode;
        //Get the value's code and translate it to the grid's code
        iGridCode = -1;
        sVal = XMLString::transcode("c");
        cData = XMLString::transcode(p_oElement->getAttributeNode(
            sVal)->getNodeValue());
        iMapCode = atoi(cData);
        delete[] cData; cData = NULL;
        XMLString::release(&sVal);
        for (k = 0; k < iNumBools; k++)
          if (iMapCode == p_bools[k].iCodeInMap) {
            iGridCode = p_bools[k].iCodeForGrid;
            break;
          }
        if (-1 == iGridCode) { //unrecognized code - throw error
          modelErr stcErr;
          stcErr.sFunction = "clGrid::ReadMapFile";
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized bool code " << iMapCode << " in map " << m_sNameString;
          stcErr.sMoreInfo = s.str();
          throw(stcErr);
        }
        cData =
            XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
        if (strcmp(cData, "true") == 0) bTemp = true;
        else if (strcmp(cData, "false") == 0) bTemp = false;
        else { //invalid bool - throw error
          modelErr stcErr;
          stcErr.sFunction = "clGrid::ReadMapFile";
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sMoreInfo = "Invalid bool value in map: ";
          stcErr.sMoreInfo += cData;
          throw(stcErr);
          delete[] cData; cData = NULL;
        }
        delete[] cData; cData = NULL;
        SetValueOfCell(iXGridVal, iYGridVal, iGridCode, bTemp);
      } //end of for (j = 0; j < iNumCellVals; j++) - bools

      //****************Packages******************
      p_oPackage = NULL;
      sVal = XMLString::transcode("pkg");
      p_oPackageList = p_oCell->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumPackages = p_oPackageList->getLength();
      for (m = 0; m < iNumPackages; m++) {
        p_oNode = p_oPackageList->item(m);
        p_oPackageEl = (DOMElement *) p_oNode;
        //Create a new package and add to the end of the package list
        if (p_oPackage) {
          p_oNextPackage = CreatePackage(p_oPackage);
          p_oPackage = p_oNextPackage;
        }
        else p_oPackage = CreatePackageOfCell(iXGridVal, iYGridVal);

        //Fill each of the data members of the package
        //*******************Package int values*****************
        sVal = XMLString::transcode("pint");
        p_oChildren = p_oPackageEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumCellVals = p_oChildren->getLength();
        for (j = 0; j < iNumCellVals; j++) {
          p_oNode = p_oChildren->item(j);
          p_oElement = (DOMElement *)p_oNode;
          //Get the value's code and translate it to the grid's code
          iGridCode = -1;
          sVal = XMLString::transcode("c");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          iMapCode = atoi(cData);
          delete[] cData; cData = NULL;
          XMLString::release(&sVal);
          for (k = 0; k < iNumCohInts; k++)
            if (iMapCode == p_cohInts[k].iCodeInMap) {
              iGridCode = p_cohInts[k].iCodeForGrid;
              break;
            }
          if (-1 == iGridCode) { //unrecognized code - throw error
            modelErr stcErr;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.iErrorCode = BAD_DATA;
            std::stringstream s;
            s << "Unrecognized package " << "int code " << iMapCode << " in map " << m_sNameString;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          iTemp = atoi(cData);
          //Make sure it's a valid int
          if (0 == iTemp && strcmp(cData, "0") != 0) {
            modelErr stcErr;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sMoreInfo = "Invalid int value in map: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          p_oPackage->SetValue(iGridCode, iTemp);
        } //end of for (j = 0; j < iNumCellVals; j++) - ints

        //*******************Package float values*****************
        sVal = XMLString::transcode("pfl");
        p_oChildren = p_oPackageEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumCellVals = p_oChildren->getLength();
        for (j = 0; j < iNumCellVals; j++) {
          p_oNode = p_oChildren->item(j);
          p_oElement = (DOMElement *)p_oNode;
          //Get the value's code and translate it to the grid's code
          iGridCode = -1;
          sVal = XMLString::transcode("c");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          iMapCode = atoi(cData);
          delete[] cData; cData = NULL;
          XMLString::release(&sVal);
          for (k = 0; k < iNumCohFloats; k++)
            if (iMapCode == p_cohFloats[k].iCodeInMap) {
              iGridCode = p_cohFloats[k].iCodeForGrid;
              break;
            }
          if (-1 == iGridCode) { //unrecognized code - throw error
            modelErr stcErr;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.iErrorCode = BAD_DATA;
            std::stringstream s;
            s << "Unrecognized package " << "float code " << iMapCode << " in map " << m_sNameString;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          fTemp = atof(cData);
          //Make sure it's a valid float
          if (0 == fTemp && cData[0] != '0') {
            modelErr stcErr;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sMoreInfo = "Invalid float value in map: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          p_oPackage->SetValue(iGridCode, fTemp);
        } //end of for (j = 0; j < iNumCellVals; j++) - floats

        //*******************Package char values*****************
        sVal = XMLString::transcode("pch");
        p_oChildren = p_oPackageEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumCellVals = p_oChildren->getLength();
        for (j = 0; j < iNumCellVals; j++) {
          p_oNode = p_oChildren->item(j);
          p_oElement = (DOMElement *)p_oNode;
          //Get the value's code and translate it to the grid's code
          iGridCode = -1;
          sVal = XMLString::transcode("c");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          iMapCode = atoi(cData);
          XMLString::release(&sVal);
          delete[] cData; cData = NULL;
          for (k = 0; k < iNumCohStrings; k++)
            if (iMapCode == p_cohChars[k].iCodeInMap) {
              iGridCode = p_cohChars[k].iCodeForGrid;
              break;
            }
          if (-1 == iGridCode) { //unrecognized code - throw error
            modelErr stcErr;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.iErrorCode = BAD_DATA;
            std::stringstream s;
            s << "Unrecognized package char code " << iMapCode << " in map " << m_sNameString;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          p_oPackage->SetValue(iGridCode, cData);
          delete[] cData; cData = NULL;
        } //end of for (j = 0; j < iNumCellVals; j++) - chars

        //*******************Package bool values*****************
        sVal = XMLString::transcode("pbl");
        p_oChildren = p_oPackageEl->getElementsByTagName(sVal);
        XMLString::release(&sVal);
        iNumCellVals = p_oChildren->getLength();
        for (j = 0; j < iNumCellVals; j++) {
          p_oNode = p_oChildren->item(j);
          p_oElement = (DOMElement *)p_oNode;
          //Get the value's code and translate it to the grid's code
          iGridCode = -1;
          sVal = XMLString::transcode("c");
          cData = XMLString::transcode(p_oElement->getAttributeNode(
              sVal)->getNodeValue());
          iMapCode = atoi(cData);
          delete[] cData; cData = NULL;
          XMLString::release(&sVal);
          for (k = 0; k < iNumCohBools; k++)
            if (iMapCode == p_cohBools[k].iCodeInMap) {
              iGridCode = p_cohBools[k].iCodeForGrid;
              break;
            }
          if (-1 == iGridCode) { //unrecognized code - throw error
            modelErr stcErr;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.iErrorCode = BAD_DATA;
            std::stringstream s;
            s << "Unrecognized package bool code " << iMapCode << " in map " << m_sNameString;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }
          cData =
              XMLString::transcode(p_oElement->getFirstChild()->getNodeValue());
          if (strcmp(cData, "true") == 0) bTemp = true;
          else if (strcmp(cData, "false") == 0) bTemp = false;
          else { //invalid bool - throw error
            modelErr stcErr;
            stcErr.sFunction = "clGrid::ReadMapFile";
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sMoreInfo = "Invalid bool value in map: ";
            stcErr.sMoreInfo += cData;
            delete[] cData; cData = NULL;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;
          p_oPackage->SetValue(iGridCode, bTemp);
        } //end of for (j = 0; j < iNumCellVals; j++) - bools
      } //end of for (m = 0; m < iNumPackages; m++)

    } //end of for (i = 0; i < iNumGridVals; i++)

    //Delete the code translation arrays
    delete[] p_ints; p_ints = NULL;
    delete[] p_floats; p_floats = NULL;
    delete[] p_chars; p_chars = NULL;
    delete[] p_bools; p_bools = NULL;
    delete[] p_cohInts; p_cohInts = NULL;
    delete[] p_cohFloats; p_cohFloats = NULL;
    delete[] p_cohChars; p_cohChars = NULL;
    delete[] p_cohBools; p_cohBools = NULL;
  } //end of try block
  catch(modelErr &err) {throw(err);} //throw without clearing this object
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch(...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrid::ReadMapFile";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// RegisterDataMember()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterDataMember(const string sLabel, short int iNumVals,
    string *p_sLabels) {
  short int iReturnCode = -1, //what we'll return
      i; //loop counter

  //Check to make sure that the data label is not empty
  if (0 == sLabel.length()) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::RegisterDataMember";
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sMoreInfo = "Data label is required.";
    throw(stcErr);
  }

  //Find the first open space
  for (i = 0; i < iNumVals; i++)
    if (0 == p_sLabels[i].length()) {
      iReturnCode = i;
      break;
    }

  //If we didn't find anything, throw an error
  if (-1 == iReturnCode) {
    modelErr stcErr;
    stcErr.sFunction = "clGrid::RegisterDataMember";
    stcErr.iErrorCode = ILLEGAL_OP;
    stcErr.sMoreInfo = "Too many data member registrations.";
    throw(stcErr);
  }

  p_sLabels[iReturnCode] = sLabel;
  return iReturnCode;
}

//////////////////////////////////////////////////////////////////////////////
// RegisterInt()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterInt(const string sLabel) {
  short int iReturnCode;
  iReturnCode = RegisterDataMember(sLabel, m_iNumIntVals, mp_sIntLabels);
  //We may need to set up a package data member too
  if (!m_bPackageDataChanged)
    RegisterDataMember(sLabel, m_iNumPackageIntVals,
        mp_sPackageIntLabels);
  return iReturnCode;
}

//////////////////////////////////////////////////////////////////////////////
// RegisterFloat()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterFloat(const string sLabel) {
  short int iReturnCode;
  iReturnCode = RegisterDataMember(sLabel, m_iNumFloatVals, mp_sFloatLabels);
  //We may need to set up a package data member too
  if (!m_bPackageDataChanged)
    RegisterDataMember(sLabel, m_iNumPackageFloatVals,
        mp_sPackageFloatLabels);
  return iReturnCode;
}

//////////////////////////////////////////////////////////////////////////////
// RegisterString()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterString(const string sLabel) {
  short int iReturnCode;
  iReturnCode = RegisterDataMember(sLabel, m_iNumStringVals, mp_sStringLabels);
  //We may need to set up a package data member too
  if (!m_bPackageDataChanged)
    RegisterDataMember(sLabel, m_iNumPackageStringVals,
        mp_sPackageStringLabels);
  return iReturnCode;
}

//////////////////////////////////////////////////////////////////////////////
// RegisterBool()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterBool(const string sLabel) {
  short int iReturnCode;
  iReturnCode = RegisterDataMember(sLabel, m_iNumBoolVals, mp_sBoolLabels);
  //We may need to set up a package data member too
  if (!m_bPackageDataChanged)
    RegisterDataMember(sLabel, m_iNumPackageBoolVals,
        mp_sPackageBoolLabels);
  return iReturnCode;
}

//////////////////////////////////////////////////////////////////////////////
// RegisterPackageInt()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterPackageInt(const string sLabel) {
  return RegisterDataMember(sLabel, m_iNumPackageIntVals, mp_sPackageIntLabels);
}

//////////////////////////////////////////////////////////////////////////////
// RegisterPackageFloat()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterPackageFloat(const string sLabel) {
  return RegisterDataMember(sLabel,m_iNumPackageFloatVals,mp_sPackageFloatLabels);
}

//////////////////////////////////////////////////////////////////////////////
// RegisterPackageString()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterPackageString(const string sLabel) {
  return RegisterDataMember(sLabel, m_iNumPackageStringVals, mp_sPackageStringLabels);
}

//////////////////////////////////////////////////////////////////////////////
// RegisterPackageBool()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::RegisterPackageBool(const string sLabel) {
  return RegisterDataMember(sLabel, m_iNumPackageBoolVals, mp_sPackageBoolLabels);
}

//////////////////////////////////////////////////////////////////////////////
// GetDataCode()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::GetIntDataCode(const string sLabel) {
  short int iReturnCode = -1, i;

  //Make sure the label passed wasn't an empty string
  if (sLabel.length() == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetIntDataCode";
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < m_iNumIntVals; i++)
    if (mp_sIntLabels[i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------
short int clGrid::GetFloatDataCode(const string sLabel) {
  short int iReturnCode = -1, i;

  //Make sure the label passed wasn't an empty string
  if (sLabel.length() == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetFloatDataCode";
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < m_iNumFloatVals; i++)
    if (mp_sFloatLabels[i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------
short int clGrid::GetStringDataCode(const string sLabel) {
  short int iReturnCode = -1, i;

  //Make sure the label passed wasn't an empty string
  if (sLabel.length() == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetStringDataCode";
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < m_iNumStringVals; i++)
    if (mp_sStringLabels[i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------
short int clGrid::GetBoolDataCode(const string sLabel) {
  short int iReturnCode = -1, i;

  //Make sure the label passed wasn't an empty string
  if (sLabel.length() == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetBoolDataCode";
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < m_iNumBoolVals; i++)
    if (mp_sBoolLabels[i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetPackageDataCode()
//////////////////////////////////////////////////////////////////////////////
short int clGrid::GetPackageIntDataCode(const string sLabel) {
  short int iReturnCode = -1, i;

  //Make sure the label passed wasn't an empty string
  if (sLabel.length() == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetPackageIntDataCode";
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < m_iNumPackageIntVals; i++)
    if (mp_sPackageIntLabels[i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------
short int clGrid::GetPackageFloatDataCode(const string sLabel) {
  short int iReturnCode = -1, i;

  //Make sure the label passed wasn't an empty string
  if (sLabel.length() == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetPackageFloatDataCode";
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < m_iNumPackageFloatVals; i++)
    if (mp_sPackageFloatLabels[i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------
short int clGrid::GetPackageStringDataCode(const string sLabel) {
  short int iReturnCode = -1, i;

  //Make sure the label passed wasn't an empty string
  if (sLabel.length() == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetPackageStringDataCode";
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < m_iNumPackageStringVals; i++)
    if (mp_sPackageStringLabels[i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------
short int clGrid::GetPackageBoolDataCode(const string sLabel) {
  short int iReturnCode = -1, i;

  //Make sure the label passed wasn't an empty string
  if (sLabel.length() == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clGrid::GetPackageBoolDataCode";
    stcErr.sMoreInfo = "Data label is required.";
    throw stcErr;
  }

  //Search for the label to return
  for (i = 0; i < m_iNumPackageBoolVals; i++)
    if (mp_sPackageBoolLabels[i].compare(sLabel) == 0) {
      iReturnCode = i;
      break;
    }
  return iReturnCode;
}
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetDataLabel()
//////////////////////////////////////////////////////////////////////////////
const string clGrid::GetIntDataLabel(short int iCode) {
  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= m_iNumIntVals) return "";
  else return mp_sIntLabels[iCode];
}
//----------------------------------------------------------------------------
const string clGrid::GetFloatDataLabel(short int iCode) {
  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= m_iNumFloatVals) return "";
  else return mp_sFloatLabels[iCode];
}
//----------------------------------------------------------------------------
const string clGrid::GetStringDataLabel(short int iCode) {
  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= m_iNumStringVals) return "";
  else return mp_sStringLabels[iCode];
}
//----------------------------------------------------------------------------
const string clGrid::GetBoolDataLabel(short int iCode) {
  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= m_iNumBoolVals) return "";
  else return mp_sBoolLabels[iCode];
}
//----------------------------------------------------------------------------
const string clGrid::GetPackageIntDataLabel(short int iCode) {
  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= m_iNumPackageIntVals) return "";
  else return mp_sPackageIntLabels[iCode];
}
//----------------------------------------------------------------------------
const string clGrid::GetPackageFloatDataLabel(short int iCode) {
  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= m_iNumPackageFloatVals) return "";
  else return mp_sPackageFloatLabels[iCode];
}
//----------------------------------------------------------------------------
const string clGrid::GetPackageStringDataLabel(short int iCode) {
  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= m_iNumPackageStringVals) return "";
  else return mp_sPackageStringLabels[iCode];
}
//----------------------------------------------------------------------------
const string clGrid::GetPackageBoolDataLabel(short int iCode) {
  //If the code's invalid, return an empty string
  if (iCode < 0 || iCode >= m_iNumPackageBoolVals) return "";
  else return mp_sPackageBoolLabels[iCode];
}
//----------------------------------------------------------------------------



//****************************************************************************

//                    PACKAGE CLASS FUNCTIONS

//****************************************************************************

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clPackage::clPackage(clGrid *p_oParentGrid,
    clGrid::stcRecords *p_oParentCell){
  try {
    int k;        //loop counters
    //Validate the pointers passed
    if (NULL == p_oParentGrid || NULL == p_oParentCell) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::clPackage";
      stcErr.sMoreInfo = "Parent pointers cannot be NULL.";
      throw(stcErr);
    }

    mp_oParentGrid = p_oParentGrid;
    mp_parentCell = p_oParentCell;

    //Size and initialize the arrays
    if (mp_oParentGrid->m_iNumPackageIntVals == 0) mp_iIntVals = NULL;
    else {
      mp_iIntVals = new int[mp_oParentGrid->m_iNumPackageIntVals];
      for (k = 0; k < mp_oParentGrid->m_iNumPackageIntVals; k++)
        mp_iIntVals[k] = 0;
    }
    //Float data members
    if (mp_oParentGrid->m_iNumPackageFloatVals == 0) mp_fFloatVals = NULL;
    else {
      mp_fFloatVals = new float[mp_oParentGrid->m_iNumPackageFloatVals];
      for (k = 0; k < mp_oParentGrid->m_iNumPackageFloatVals; k++)
        mp_fFloatVals[k] = 0;
    }
    //Char data members
    if (mp_oParentGrid->m_iNumPackageStringVals == 0) mp_sStringVals = NULL;
    else {
      mp_sStringVals = new string[mp_oParentGrid->m_iNumPackageStringVals];
      for (k = 0; k < mp_oParentGrid->m_iNumPackageStringVals; k++) {
        mp_sStringVals[k] == "";
      }
    }
    //Bool data members
    if (mp_oParentGrid->m_iNumPackageBoolVals == 0) mp_bBoolVals = NULL;
    else {
      mp_bBoolVals = new bool[mp_oParentGrid->m_iNumPackageBoolVals];
      for (k = 0; k < mp_oParentGrid->m_iNumPackageBoolVals; k++)
        mp_bBoolVals[k] = false;
    }

    mp_oNext = NULL;
  } //end of try block
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::clPackage";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clPackage::~clPackage() {
  delete[] mp_iIntVals;
  delete[] mp_fFloatVals;
  delete[] mp_sStringVals;
  delete[] mp_bBoolVals;
}

//////////////////////////////////////////////////////////////////////////////
// SetValue
//////////////////////////////////////////////////////////////////////////////
void clPackage::SetValue(short int iCode, int iValue) {
  try {
    //Verify the code
    if (iCode < 0 || iCode >= mp_oParentGrid->m_iNumPackageIntVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::SetValue(int)";
      std::stringstream s;
      s << "Invalid data code:  " << iCode << " for grid " << mp_oParentGrid->m_sNameString;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    mp_iIntVals[iCode] = iValue;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::SetValue(int)";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
void clPackage::SetValue(short int iCode, float fValue) {
  try {
    //Verify the code
    if (iCode < 0 || iCode >= mp_oParentGrid->m_iNumPackageFloatVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::SetValue(float)";
      std::stringstream s;
      s << "Invalid data code:  " << iCode << " for grid " << mp_oParentGrid->m_sNameString;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    mp_fFloatVals[iCode] = fValue;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::SetValue(float)";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
void clPackage::SetValue(short int iCode, string sValue) {
  try {
    //Verify the code
    if (iCode < 0 || iCode >= mp_oParentGrid->m_iNumPackageStringVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::SetValue(char)";
      std::stringstream s;
      s << "Invalid data code:  " << iCode << " for grid " << mp_oParentGrid->m_sNameString;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    mp_sStringVals[iCode] = sValue;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::SetValue(char)";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
void clPackage::SetValue(short int iCode, bool bValue) {
  try {
    //Verify the code
    if (iCode < 0 || iCode >= mp_oParentGrid->m_iNumPackageBoolVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::SetValue(bool)";
      std::stringstream s;
      s << "Invalid data code:  " << iCode << " for grid " << mp_oParentGrid->m_sNameString;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    mp_bBoolVals[iCode] = bValue;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::SetValue(bool)";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// GetValue
//////////////////////////////////////////////////////////////////////////////
void clPackage::GetValue(short int iCode, int *p_iValHolder) {
  try {
    //Verify the code
    if (iCode < 0 || iCode >= mp_oParentGrid->m_iNumPackageIntVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::GetValue(int)";
      std::stringstream s;
      s << "Invalid data code:  " << iCode << " for grid " << mp_oParentGrid->m_sNameString;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    *p_iValHolder = mp_iIntVals[iCode];
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::GetValue(int)";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
void clPackage::GetValue(short int iCode, float *p_fValHolder) {
  try {
    //Verify the code
    if (iCode < 0 || iCode >= mp_oParentGrid->m_iNumPackageFloatVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::GetValue(float)";
      std::stringstream s;
      s << "Invalid data code:  " << iCode << " for grid " << mp_oParentGrid->m_sNameString;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    *p_fValHolder = mp_fFloatVals[iCode];
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::GetValue(float)";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
void clPackage::GetValue(short int iCode, string *p_sValHolder) {
  try {
    //Verify the code
    if (iCode < 0 || iCode >= mp_oParentGrid->m_iNumPackageStringVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::GetValue(char)";
      std::stringstream s;
      s << "Invalid data code:  " << iCode << " for grid " << mp_oParentGrid->m_sNameString;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    *p_sValHolder = mp_sStringVals[iCode];
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::GetValue(char)";
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
void clPackage::GetValue(short int iCode, bool *p_bValHolder) {
  try {
    //Verify the code
    if (iCode < 0 || iCode >= mp_oParentGrid->m_iNumPackageBoolVals) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clPackage::GetValue(bool)";
      std::stringstream s;
      s << "Invalid data code:  " << iCode << " for grid " << mp_oParentGrid->m_sNameString;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
    *p_bValHolder = mp_bBoolVals[iCode];
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPackage::GetValue(bool)";
    throw(stcErr);
  }
}
