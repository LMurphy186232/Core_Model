//---------------------------------------------------------------------------

#ifndef GridH
#define GridH
//--------------------------------------------------------------------------

#include "WorkerBase.h"

class clPackage;

/**
 * Grid Base - Version 1.0
 * Objects of the grid data type will be instantiated from this class. The grid
 * data type handles values which vary in space. It has its own grid coordinate
 * system which may have a different resolution from the plot as a whole.
 *
 * A grid is a 2D array of stcRecord structures. The size of the array is
 * number of grid cells in the X direction by number in the Y direction (thus,
 * one stcRecord per grid cell). The stcRecord structure has four arrays - one
 * for ints, floats, strings, and bools. All stcRecords for a grid have
 * the same array size, which is set in the grid's constructor. You don't have
 * to have variables of each type when you create a grid - the grid is happy to
 * leave an array as NULL.
 *
 * In addition, each stcRecord structure (and thus, each grid cell) has an
 * optional linked list of clPackage objects. Each package object also has four
 * arrays (int, float, string, bool). All package objects for a grid are
 * also uniform, but they can be different from the main grid. So for instance,
 * your grid might contain space for 3 floats and 1 int in its cells within the
 * stcRecord structures, and then 2 bools in its packages.
 *
 * clBehavior objects can access and set grid values either through the grid's
 * own coordinates (using X and Y grid numbers) or at specific point coordinates
 * within the plot.
 *
 * This class knows about the torus shape of the clPlot.
 *
 * You cannot create objects of this correctly. Use the method
 * clSimManager::CreateGrid() to create one. This ensures that created grids
 * fall under the ownership of the clGridManager class. By the same token, do
 * not delete grids you create. clGridManager will take care of freeing the
 * memory.
 *
 * Copyright 2003 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 * <br>November 12, 2012 - Chars became strings (LEM)
*/
class clGrid : public clWorkerBase{
  friend class clGridManager;
  friend class clPackage;

public:

  struct stcRecords {
    /**Array holding integer values. Size is m_iNumIntVals*/
    int *p_iIntVals;
    /**Array holding float values. Size is m_iNumFloatVals*/
    float *p_fFloatVals;
    /**Array holding string values. Size is m_iNumStringVals*/
    std::string *p_sStringVals;
    /**Array holding bool values. Size is m_iNumBoolVals*/
    bool *p_bBoolVals;
    /**For packages, if necessary. This will link to the others.*/
    clPackage *p_oPackage;
  };

protected:

  /**
   * Constructor. This will set up the value arrays for the grid cells. Values
   * will be initialized to 0, false, or empty string, as appropriate. The grid
   * will not be ready to use after this; the variables must be registered with
   * labels first using the register functions below.
   *
   * This will also default a package data structure which is the same as the
   * grid's.
   * @param p_oSimManager A pointer to the sim manager object.
   * @param sGridName An identifying name string for the grid object.
   * @param iNumIntVals Number of integer data members in a grid cell record.
   * Can be 0.
   * @param iNumFloatVals Number of float data members in a grid cell record.
   * Can be 0.
   * @param iNumStringVals Number of string data members in a grid cell record.
   * Can be 0.
   * @param iNumBoolVals Number of bool data members in a grid cell record.
   * Can be 0.
   * @param fXCellLength The length of a grid cell in the X direction, in
   * meters. Not required. If ommitted this will default to the plot's grid
   * cell length.
   * @param fYCellLength The length of a grid cell in the Y direction, in
   * meters. Not required. If this is ommitted (i.e. = 0), it is assumed the
   * grid cells are square and the value for the X length is used.
   */
  clGrid(clSimManager *p_oSimManager, const std::string sGridName,
      short int iNumIntVals, short int iNumFloatVals, short int iNumStringVals,
      short int iNumBoolVals, float fXCellLength = 0,  float fYCellLength = 0);


  /**
   * Destructor.
   */
  ~clGrid();

/**
   * Core function for registering new data members. If successful, it will
   * return a code for that member for the species and type combo.
   * @param sLabel A string with the name of this variable.
   * @param iNumVals Number total allowed variables of this type
   * @param p_sLabelList Pointer to the correct labels array
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw If registration is unsuccessful, a BAD_DATA or ILLEGAL_OP error
   * will be thrown.
   */
  inline short int RegisterDataMember(std::string sLabel, short int iNumVals,
    std::string *p_sLabelList);

  /**Grid cell array. Size is # X grids by # Y grids.*/
  stcRecords **mp_gridVals;

  /**How many integer data members per grid cell.*/
  short int m_iNumIntVals;
  /**How many float data members per grid cell.*/
  short int m_iNumFloatVals;
  /**How many string data members per grid cell.*/
  short int m_iNumStringVals;
  /**How many bool data members per grid cell.*/
  short int m_iNumBoolVals;

  /**Labels for each of the integer data members.*/
  std::string *mp_sIntLabels;
  /**Labels for each of the float data members.*/
  std::string *mp_sFloatLabels;
  /**Labels for each of the string data members.*/
  std::string *mp_sStringLabels;
  /**Labels for each of the bool data members.*/
  std::string *mp_sBoolLabels;

  /**Length of grid cells in the X direction, in meters.*/
  float m_fXCellSize;
  /**Length of grid cells in the Y direction, in meters.*/
  float m_fYCellSize;
  /**Length of plot in Y direction*/
  float m_fYPlotLength;
  /**Length of plot in X direction*/
  float m_fXPlotLength;

  /**Number of grid divisions along the X axis.*/
  int m_iNumXCells;
  /**Number of grid divisions along the Y axis.*/
  int m_iNumYCells;

  /**Whether or not the package data structure may be changed.*/
  bool m_bPackageChangeAllowed;
  /**Whether or not the packages have been set up with a different data
   * structure from the grid.*/
  bool m_bPackageDataChanged;

  /**Number of package integer data members.*/
  short int m_iNumPackageIntVals;
  /**Number of package float data members.*/
  short int m_iNumPackageFloatVals;
  /**Number of package string data members.*/
  short int m_iNumPackageStringVals;
  /**Number of package bool data members.*/
  short int m_iNumPackageBoolVals;

  /**Labels for each of the integer data members.*/
  std::string *mp_sPackageIntLabels;
  /**Labels for each of the float data members.*/
  std::string *mp_sPackageFloatLabels;
  /**Labels for each of the string data members.*/
  std::string *mp_sPackageStringLabels;
  /**Labels for each of the bool data members.*/
  std::string *mp_sPackageBoolLabels;

  /**
   * This is the function that actually calculates a circle's average value.
   * @param fX X coordinate of circle center.
   * @param fY Y coordinate of circle center.
   * @param fRadius Radius of circle over which to average values.
   * @param bFloat If true, calculate float values; if false, calculate
   * integer vals.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @return The average specified.
   */
  float GetAverageValue(float fX, float fY, short int iCode, float fRadius,
                        bool bFloat);

 /**
  * This is the function that actually calculates a rectangle's average value.
  * @param fFromX X coordinate of the point closest to the origin.
  * @param fFromY Y coordinate of the point closest to the origin.
  * @param fToX X coordinate of the point farthest from the origin.
  * @param fToY Y coordinate of the point farthest from the origin.
  * @param iCode Code for the value desired. This is what is returned from
  * registering a variable or from using GetDataCode.
  * @param bFloat If true, calculate float values; if false, calculate
  * integer vals.
  * @return The average specified.
  * @throws modelErr if the "to" point coordinates are less than the "from"
  * point coordinates.
  */
  float GetAverageValue(float fFromX, float fFromY, float fToX, float fToY,
      short int iCode, bool bFloat);

public:

  /**
   * Reads in a grid's map, if present in a file.
   * @param p_oDoc DOM tree of parsed document.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Reads a map file. Why is this here when map file writing is in output?
   * Because a map might need to be read as part of parameter file input, and
   * we can't depend on output's having been loaded as a behavior.
   *
   * This will read a map file into the grid object. It will search for a grid
   * map with a grid name matching this grid's name. It will stop and load the
   * first one it finds; any subsequent grids with this name will be ignored.
   * If the plot info doesn't match this grid's, it will throw a BAD_DATA error.
   * The map doesn't have to have all the same data members as the grid, but all
   * data members in the map must be in the grid, including all package data.
   *
   * The absence of a map for this grid in the document causes no changes to be
   * made.
   * @param p_oDoc DOM tree of parsed document.
   */
  void ReadMapFile(xercesc::DOMDocument *p_oDoc);

  /**
   * Changes the data structure for the packages, as for changing from defaults.
   * This will wipe out all labels. This is only allowed until packages
   * actually exist - then this will throw an ILLEGAL_OP error.
   *
   * If this function is used, all variables for the package must be registered,
   * even if they have already been registered with the grid.
   * @param iNumIntVals Number of package integer data members.
   * @param iNumFloatVals Number of package float data members.
   * @param iNumStringVals Number of package string data members.
   * @param iNumBoolVals Number of package bool data members.
   */
  void ChangePackageDataStructure(short int iNumIntVals, short int iNumFloatVals,
      short int iNumStringVals, short int iNumBoolVals);

  /**
   * Retrieves the value of an integer grid data member at an (X, Y) location.
   * This function determines which internal grid cell this location is and
   * places the grid value into the variable passed.
   *
   * @param fX X coordinate of point for which the value is desired.
   * @param fY Y coordinate of point for which the value is desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param p_iValHolder Pointer to where the return value should be placed.
   * @throw BAD_DATA error if the point is not within the grid or the code is
   * not valid.
   */
  void GetValueAtPoint(float fX, float fY, short int iCode, int *p_iValHolder);

  /**
   * Retrieves the value of a float grid data member at an (X, Y) location. This
   * function determines which internal grid cell this location is and places
   * the grid value into the variable passed.
   * @param fX X coordinate of point for which the value is desired.
   * @param fY Y coordinate of point for which the value is desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param p_fValHolder Pointer to where the return value should be placed.
   * @throw BAD_DATA error if the point is not within the grid or the code is
   * not valid.
   */
  void GetValueAtPoint(float fX, float fY, short int iCode, float *p_fValHolder);

  /**
   * Retrieves the value of a string grid data member at an (X, Y) location. This
   * function determines which internal grid cell this location is and places
   * the grid value into the variable passed.
   * @param fX X coordinate of point for which the value is desired.
   * @param fY Y coordinate of point for which the value is desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param p_sValHolder Pointer to where the return value should be placed.
   * @throw BAD_DATA error if the point is not within the grid or the code is
   * not valid.
   */
  void GetValueAtPoint(float fX, float fY, short int iCode, std::string *p_sValHolder);

  /**
   * Retrieves the value of a bool grid data member at an (X, Y) location. This
   * function determines which internal grid cell this location is and places
   * the grid value into the variable passed.
   * @param fX X coordinate of point for which the value is desired.
   * @param fY Y coordinate of point for which the value is desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param p_bValHolder Pointer to where the return value should be placed.
   * @throw BAD_DATA error if the point is not within the grid or the code is
   * not valid.
   */
  void GetValueAtPoint(float fX, float fY, short int iCode, bool *p_bValHolder);

  /**
   * Retrieves the value of an integer data member for a particular grid cell.
   * The grid cell numbering in each direction starts at 0 and goes to number of
   * grid cells minus one.
   * @param iX Grid number in the X direction for cell for which the value is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the value is
   * desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param p_iValHolder Pointer to where the return value should be placed.
   * @throw BAD_DATA error if the grid cell coordinates are not within the
   * grid or the code is not valid.
   */
  void GetValueOfCell(int iX, int iY, short int iCode, int *p_iValHolder);

  /**
   * Retrieves the value of a float data member for a particular grid cell. The
   * grid cell numbering in each direction starts at 0 and goes to number of
   * grid cells minus one.
   * @param iX Grid number in the X direction for cell for which the value is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the value is
   * desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param p_fValHolder Pointer to where the return value should be placed.
   * @throw BAD_DATA error if the grid cell coordinates are not within the grid
   * or the code is not valid.
   */
  void GetValueOfCell(int iX, int iY, short int iCode, float *p_fValHolder);

  /**
   * Retrieves the value of a string data member for a particular grid cell. The
   * grid cell numbering in each direction starts at 0 and goes to number of
   * grid cells minus one.
   * @param iX Grid number in the X direction for cell for which the value is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the value is
   * desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param p_sValHolder Pointer to where the return value should be placed.
   * @throw BAD_DATA error if the grid cell coordinates are not within the grid
   * or the code is not valid.
   */
  void GetValueOfCell(int iX, int iY, short int iCode, std::string *p_sValHolder);

  /**
   * Retrieves the value of a bool data member for a particular grid cell. The
   * grid cell numbering in each direction starts at 0 and goes to number of
   * grid cells minus one.
   * @param iX Grid number in the X direction for cell for which the value is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the value is
   * desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param p_bValHolder Pointer to where the return value should be placed.
   * @throw BAD_DATA error if the grid cell coordinates are not within the grid
   * or the code is not valid.
   */
  void GetValueOfCell(int iX, int iY, short int iCode, bool *p_bValHolder);

  /**
   * Returns the average value of the grid cells in a circle for a float data
   * member. The circle is defined by a point and a radius (in meters). All grid
   * cells which are included in this circle, in whole or in part, are included
   * in the average; there is no weighting factor if it's only a partial grid.
   * This ignores packages.
   * @param fX X coordinate of circle center.
   * @param fY Y coordinate of circle center.
   * @param fRadius Radius of circle over which to average values.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @return The average specified.
   */
  float GetAverageFloatValue(float fX, float fY, short int iCode,float fRadius);

  /**
   * Returns the average value of the grid cells in a circle for a integer data
   * member. The circle is defined by a point and a radius (in meters). All grid
   * cells which are included in this circle, in whole or in part, are included
   * in the average; there is no weighting factor if it's only a partial grid.
   * This ignores packages.
   * @param fX X coordinate of circle center.
   * @param fY Y coordinate of circle center.
   * @param fRadius Radius of circle over which to average values.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @return The average specified.
   */
  float GetAverageIntValue(float fX, float fY, short int iCode, float fRadius);

  /**
  * Returns the average value of the grid cells in a rectangle for a float data
  * member. The first point is the point of the rectangle closest to the
  * origin. All cells which are included in this rectangle, in whole or in
  * part, are included in the average; there is no weighting factor if it's only
  * a partial grid. This ignores packages.
  * @param fFromX X coordinate of the point closest to the origin.
  * @param fFromY Y coordinate of the point closest to the origin.
  * @param fToX X coordinate of the point farthest from the origin.
  * @param fToY Y coordinate of the point farthest from the origin.
  * @param iCode Code for the value desired. This is what is returned from
  * registering a variable or from using GetDataCode.
  * @return The average specified.
  * @throws modelErr if the "to" point coordinates are less than the "from"
  * point coordinates.
  */
  float GetAverageFloatValue(float fFromX, float fFromY, float fToX, float fToY, short int iCode);

  /**
  * Returns the average value of the grid cells in a rectangle for an int data
  * member. The first point is the point of the rectangle closest to the
  * origin. All cells which are included in this rectangle, in whole or in
  * part, are included in the average; there is no weighting factor if it's only
  * a partial grid. This ignores packages.
  * @param fFromX X coordinate of the point closest to the origin.
  * @param fFromY Y coordinate of the point closest to the origin.
  * @param fToX X coordinate of the point farthest from the origin.
  * @param fToY Y coordinate of the point farthest from the origin.
  * @param iCode Code for the value desired. This is what is returned from
  * registering a variable or from using GetDataCode.
  * @return The average specified.
  * @throws modelErr if the "to" point coordinates are less than the "from"
  * point coordinates.
  */
  float GetAverageIntValue(float fFromX, float fFromY, float fToX, float fToY, short int iCode);

  /**
  * Gets the real X coordinate of the origin of a grid cell at a given X grid
  * number. The origin of the cell is the point closest to the origin of the
  * plot.
  * @param iX X grid number of the cell.
  * @return X origin coordinate.
  * @throws modelErr if the coordinate is not inside the plot.
  */
  float GetOriginXOfCell(int iX);

  /**
  * Gets the real Y coordinate of the origin of a grid cell at a given Y grid
  * number. The origin of the cell is the point closest to the origin of the
  * plot.
  * @param iY Y grid number of the cell.
  * @return Y origin coordinate.
  * @throws modelErr if the coordinate is not inside the plot.
  */
  float GetOriginYOfCell(int iY);

  /**
  * Gets the real X coordinate of the end of a grid cell at a given X grid
  * number. The end of the cell is the point farthest from the origin of the
  * plot. Since the grid line itself would be part of the next cell over, this
  * is the grid line minus an infinitesimal amount.
  * @param iX X grid number of the cell.
  * @return X end coordinate.
  * @throws modelErr if the coordinate is not inside the plot.
  */
  float GetEndXOfCell(int iX);

  /**
  * Gets the real Y coordinate of the end of a grid cell at a given Y grid
  * number. The end of the cell is the point farthest from the origin of the
  * plot. Since the grid line itself would be part of the next cell over, this
  * is the grid line minus an infinitesimal amount.
  * @param iY Y grid number of the cell.
  * @return Y end coordinate.
  * @throws modelErr if the coordinate is not inside the plot.
  */
  float GetEndYOfCell(int iY);

  /**
   * Retrieves first package in linked list for a grid cell. If there are no
   * packages for this cell, NULL is returned.
   * @param iX Grid number in the X direction for cell for which the package is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the package is
   * desired.
   * @return First package of grid cell, or NULL if there are no packages.
   */
  clPackage* GetFirstPackageOfCell(int iX, int iY);

  /**
   * Retrieves first package in linked list for a point. If there are no
   * packages for this point, NULL is returned.
   * @param fX X coordinate of point.
   * @param fY Y coordinate of point.
   * @return First package at that point, or NULL if there are no packages.
   */
  clPackage* GetFirstPackageAtPoint(float fX, float fY);

  /**
   * Gets the coordinates of a point at the center of a grid cell.
   * @param iCellX X number of grid cell
   * @param iCellY Y number of grid cell
   * @param fX Address of variable in which to put X coordinate
   * @param fY Address of variable in which to put Y coordinate
   */
  void GetPointOfCell(short int iCellX, short int iCellY, float *fX, float *fY);

  /**
   * Retrieves the cell numbers for a point.
   * @param fX X coordinate of point.
   * @param fY Y coordinate of point.
   * @param iCellX Address of variable in which to place X grid cell number
   * @param iCellY Address of variable in which to place X grid cell number
   */
  void GetCellOfPoint(float fX, float fY, short int *iCellX, short int *iCellY);

  /**
   * Gets number of cells in the X direction.
   * @return Number of cells in the X direction.
   */
  int GetNumberXCells() {return m_iNumXCells;};

  /**
   * Gets number of cells in the Y direction.
   * @return Number of cells in the Y direction.
   */
  int GetNumberYCells() {return m_iNumYCells;};

  /**
   * Gets the length of a cell in the X direction.
   * @return Length of the cell in the X direction, in meters.
   */
  float GetLengthXCells() {return m_fXCellSize;};

  /**
   * Gets the length of a cell in the Y direction.
   * @return Length of the cell in the Y direction, in meters.
   */
  float GetLengthYCells() {return m_fYCellSize;};

  /**
  * Gets the number of int data members.
  * @return Number of int data members.
  */
  int GetNumberIntDataMembers() {return m_iNumIntVals;};

  /**
  * Gets the number of float data members.
  * @return Number of float data members.
  */
  int GetNumberFloatDataMembers() {return m_iNumFloatVals;};

  /**
  * Gets the number of string data members.
  * @return Number of string data members.
  */
  int GetNumberStringDataMembers() {return m_iNumStringVals;};

  /**
  * Gets the number of bool data members.
  * @return Number of bool data members.
  */
  int GetNumberBoolDataMembers() {return m_iNumBoolVals;};

  /**
  * Gets the number of package int data members.
  * @return Number of package int data members.
  */
  int GetNumberIntPackageDataMembers() {return m_iNumPackageIntVals;};

  /**
  * Gets the number of package float data members.
  * @return Number of package float data members.
  */
  int GetNumberFloatPackageDataMembers() {return m_iNumPackageFloatVals;};

  /**
  * Gets the number of package string data members.
  * @return Number of package string data members.
  */
  int GetNumberStringPackageDataMembers() {return m_iNumPackageStringVals;};

  /**
  * Gets the number of package bool data members.
  * @return Number of package bool data members.
  */
  int GetNumberBoolPackageDataMembers() {return m_iNumPackageBoolVals;};

  /**
   * Gets whether or not the package data structure has changed.
   * @return True if package data structure is different from core data member
   * structure; false otherwise.
   */
  bool GetPackageDataChanged() {return m_bPackageDataChanged;};

  /**
   * Sets the value of a float data member for the grid cell containing an
   * (X, Y) location. This function determines which internal grid cell this
   * location is and assigns the grid cell for that grid.
   * @param fX X coordinate of point for which the value is desired.
   * @param fY Y coordinate of point for which the value is desired.
   * @param fValue Value to set.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @throw BAD_DATA error if the point is not within the grid or the code is
   * not valid.
   */
  void SetValueAtPoint(float fX, float fY, short int iCode, float fValue);

  /**
   * Sets the value of an integer data member for the grid cell containing an
   * (X, Y) location. This function determines which internal grid cell this
   * location is and assigns the grid cell for that grid.
   * @param fX X coordinate of point for which the value is desired.
   * @param fY Y coordinate of point for which the value is desired.
   * @param iValue Value to set.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @throw BAD_DATA error if the point is not within the grid or the code is
   * not valid.
   */
  void SetValueAtPoint(float fX, float fY, short int iCode, int iValue);

  /**
   * Sets the value of a string data member for the grid cell containing an (X, Y)
   * location. This function determines which internal grid cell this location
   * is and assigns the grid cell for that grid.
   * @param fX X coordinate of point for which the value is desired.
   * @param fY Y coordinate of point for which the value is desired.
   * @param sValue Value to set.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @throw BAD_DATA error if the point is not within the grid or the code is
   * not valid.
   */
  void SetValueAtPoint(float fX, float fY, short int iCode, std::string sValue);

  /**
   * Sets the value of a bool data member for the grid cell containing an (X, Y)
   * location. This function determines which internal grid cell this location
   * is and assigns the grid cell for that grid.
   * @param fX X coordinate of point for which the value is desired.
   * @param fY Y coordinate of point for which the value is desired.
   * @param bValue Value to set.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @throw BAD_DATA error if the point is not within the grid or the code is
   * not valid.
   */
  void SetValueAtPoint(float fX, float fY, short int iCode, bool bValue);

  /**
   * Sets the value of a float data member for a particular grid cell. The grid
   * cell numbering in each direction starts at 0 and goes to number of grid
   * cells minus one.
   * @param iX Grid number in the X direction for cell for which the value is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the value is
   * desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param fValue Value to set.
   * @throw BAD_DATA error if the grid cell coordinates are not within the
   * grid or the code is not valid.
   */
  void SetValueOfCell(int iX, int iY, short int iCode, float fValue);

  /**
   * Sets the value of an integer data member for a particular grid cell. The
   * grid cell numbering in each direction starts at 0 and goes to number of
   * grid cells minus one.
   * @param iX Grid number in the X direction for cell for which the value is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the value is
   * desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param iValue Value to set.
   * @throw BAD_DATA error if the grid cell coordinates are not within the grid
   * or the code is not valid.
   */
  void SetValueOfCell(int iX, int iY, short int iCode, int iValue);

  /**
   * Sets the value of a string data member for a particular grid cell. The grid
   * cell numbering in each direction starts at 0 and goes to number of grid
   * cells minus one.
   * @param iX Grid number in the X direction for cell for which the value is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the value is
   * desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param sValue Value to set.
   * @throw BAD_DATA error if the grid cell coordinates are not within the grid
   * or the code is not valid.
   */
  void SetValueOfCell(int iX, int iY, short int iCode, std::string sValue);

  /**
   * Sets the value of a bool data member for a particular grid cell. The grid
   * cell numbering in each direction starts at 0 and goes to number of grid
   * cells minus one.
   * @param iX Grid number in the X direction for cell for which the value is
   * desired.
   * @param iY Grid number in the Y direction for cell for which the value is
   * desired.
   * @param iCode Code for the value desired. This is what is returned from
   * registering a variable or from using GetDataCode.
   * @param bValue Value to set.
   * @throw BAD_DATA error if the grid cell coordinates are not within the
   * grid or the code is not valid.
   */
  void SetValueOfCell(int iX, int iY, short int iCode, bool bValue);

  /**
   * Registers an integer data member. This will not check to make sure that
   * there is not already a variable registered with that name.
   * @param sLabel String with the name of this variable. Keep it short.
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw ILLEGAL_OP error if there is no more space for new data members.
   */
  short int RegisterInt(const std::string sLabel);

  /**
   * Registers a float data member. This will not check to make sure
   * that there is not already a variable registered with that name.
   * @param sLabel String with the name of this variable. Keep it short.
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw ILLEGAL_OP error if there is no more space for new data members.
   */
  short int RegisterFloat(const std::string sLabel);

  /**
   * Registers a string data member. This will not check to make sure
   * that there is not already a variable registered with that name.
   * @param sLabel String with the name of this variable. Keep it short.
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw ILLEGAL_OP error if there is no more space for new data members.
   */
  short int RegisterString(const std::string sLabel);

  /**
   * Registers a bool data member. This will not check to make sure
   * that there is not already a variable registered with that name.
   * @param sLabel String with the name of this variable. Keep it short.
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw ILLEGAL_OP error if there is no more space for new data members.
   */
  short int RegisterBool(const std::string sLabel);

  /**
   * Registers a package integer data member. This function must only be used
   * if you need to set up packages with a different data structure than the
   * main grid. If the data structures are the same, then packages
   * automatically get the same data member registrations as the main grid.
   * @param sLabel String with the name of this variable. Keep it short.
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw ILLEGAL_OP error if there is no more space for new data members.
   */
  short int RegisterPackageInt(const std::string sLabel);

  /**
   * Registers a package float data member. This function must only be used if
   * you need to set up packages with a different data structure than the main
   * grid. If the data structures are the same, then packages automatically get
   * the same data member registrations as the main grid.
   * @param sLabel String with the name of this variable. Keep it short.
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw ILLEGAL_OP error if there is no more space for new data members.
   */
  short int RegisterPackageFloat(const std::string sLabel);

  /**
   * Registers a package string data member. This function must only be used if
   * you need to set up packages with a different data structure than the main
   * grid. If the data structures are the same, then packages automatically get
   * the same data member registrations as the main grid.
   * @param sLabel String with the name of this variable. Keep it short.
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw ILLEGAL_OP error if there is no more space for new data members.
   */
  short int RegisterPackageString(const std::string sLabel);

  /**
   * Registers a package bool data member. This function must only be used if
   * you need to set up packages with a different data structure than the main
   * grid. If the data structures are the same, then packages automatically get
   * the same data member registrations as the main grid.
   * @param sLabel String with the name of this variable. Keep it short.
   * @return A code that will be used to access values of that type in the
   * future. This is faster than trying to do lookups by character string.
   * @throw ILLEGAL_OP error if there is no more space for new data members.
   */
  short int RegisterPackageBool(const std::string sLabel);

//It's true that the next three could be one overloaded function. However,
//with the possibility of inadvertant casting between floats and ints, I
//wanted the choice to be explicit.

  /**
   * Creates a package. The new package will be placed after a specified other
   * package, between it and the next package on the linked list.
   * @param p_oPreviousPackage The package after which to place the new package.
   * @return Pointer to the new package.
   * @throw Error if p_oPreviousPackage is NULL.
   */
  clPackage* CreatePackage(clPackage *p_oPreviousPackage);

  /**
   * Creates a package. The new package will be placed as the first package for
   * a specific grid cell.
   * @param iX Grid cell number in the X direction.
   * @param iY Grid cell number in the Y direction.
   * @return Pointer to the new package.
   * @throw Error if the grid numbers are invalid.
   */
  clPackage* CreatePackageOfCell(int iX, int iY);

  /**
   * Creates a package. The new package will be placed as the first package for
   * the grid cell of a specific grid cell.
   * @param fX X coordinate of point.
   * @param fY Y coordinate of point.
   * @return Pointer to the new package.
   * @throw Error if the point is not within the grid boundaries.
   */
  clPackage* CreatePackageAtPoint(float fX, float fY);

  /**
   * Deletes a package. If the package is in the middle of the package list,
   * the gap will be closed.
   * @param p_oOldPackage Package to delete.
   */
  void DeletePackage(clPackage *p_oOldPackage);

  /**
   * Gets the code for an integer data member. This function will not be
   * responsible for duplicate labels.
   * @param sLabel The data member's label.
   * @return The code for the value, -1 if the label is unrecognized.
   */
  short int GetIntDataCode(const std::string sLabel);

  /**
   * Gets the code for a float data member. This function will not be
   * responsible for duplicate labels.
   * @param sLabel The data member's label.
   * @return The code for the value, -1 if the label is unrecognized.
   */
  short int GetFloatDataCode(const std::string sLabel);

  /**
   * Gets the code for a string data member. This function will not be
   * responsible for duplicate labels.
   * @param sLabel The data member's label.
   * @return The code for the value, -1 if the label is unrecognized.
   */
  short int GetStringDataCode(const std::string sLabel);

  /**
   * Gets the code for a bool data member. This function will not be
   * responsible for duplicate labels.
   * @param sLabel The data member's label.
   * @return The code for the value, -1 if the label is unrecognized.
   */
  short int GetBoolDataCode(const std::string sLabel);

  /**
   * Gets the code for a package integer data member. This function will not be
   * responsible for duplicate labels.
   * @param sLabel The data member's label.
   * @return The code for the value, -1 if the label is unrecognized.
   */
  short int GetPackageIntDataCode(const std::string sLabel);

  /**
   * Gets the code for a package float data member. This function will not be
   * responsible for duplicate labels.
   * @param sLabel The data member's label.
   * @return The code for the value, -1 if the label is unrecognized.
   */
  short int GetPackageFloatDataCode(const std::string sLabel);

  /**
   * Gets the code for a package string data member. This function will not be
   * responsible for duplicate labels.
   * @param sLabel The data member's label.
   * @return The code for the value, -1 if the label is unrecognized.
   */
  short int GetPackageStringDataCode(const std::string sLabel);

  /**
   * Gets the code for a package bool data member. This function will not be
   * responsible for duplicate labels.
   * @param sLabel The data member's label.
   * @return The code for the value, -1 if the label is unrecognized.
   */
  short int GetPackageBoolDataCode(const std::string sLabel);

  /**
   * Gets the label for an integer data member. This function will not be
   * responsible for duplicate labels.
   * @param iCode The data member's code.
   * @return The code's data label, or an empty string if the code is
   * unrecognized.
   */
  const std::string GetIntDataLabel(short int iCode);

  /**
   * Gets the label for a float data member. This function will not be
   * responsible for duplicate labels.
   * @param iCode The data member's code.
   * @return The code's data label, or an empty string if the code is
   * unrecognized.
   */
  const std::string GetFloatDataLabel(short int iCode);

  /**
   * Gets the label for a string data member. This function will not be
   * responsible for duplicate labels.
   * @param iCode The data member's code.
   * @return The code's data label, or an empty string if the code is
   * unrecognized.
   */
  const std::string GetStringDataLabel(short int iCode);

  /**
   * Gets the label for a bool data member. This function will not be
   * responsible for duplicate labels.
   * @param iCode The data member's code.
   * @return The code's data label, or an empty string if the code is
   * unrecognized.
   */
  const std::string GetBoolDataLabel(short int iCode);

  /**
   * Gets the label for a package integer data member. This function will not
   * be responsible for duplicate labels.
   * @param iCode The data member's code.
   * @return The code's data label, or an empty string if the code is
   * unrecognized.
   */
  const std::string GetPackageIntDataLabel(short int iCode);

  /**
   * Gets the label for a package float data member. This function will not be
   * responsible for duplicate labels.
   * @param iCode The data member's code.
   * @return The code's data label, or an empty string if the code is
   * unrecognized.
   */
  const std::string GetPackageFloatDataLabel(short int iCode);

  /**
   * Gets the label for a package string data member. This function will not be
   * responsible for duplicate labels.
   * @param iCode The data member's code.
   * @return The code's data label, or an empty string if the code is
   * unrecognized.
   */
  const std::string GetPackageStringDataLabel(short int iCode);

  /**
   * Gets the label for a package bool data member. This function will not be
   * responsible for duplicate labels.
   * @param iCode The data member's code.
   * @return The code's data label, or an empty string if the code is
   * unrecognized.
   */
  const std::string GetPackageBoolDataLabel(short int iCode);

}; //end of clGrid class

//struct clGrid::stcRecords;

/**
 * Package Class
 * This class is for packages, which are used by grids. All packages have the
 * same record structure. It depends on its parent grid to set the static member
 * variables.
 */
class clPackage {

  friend class clGrid;
  friend class clGridTest; /**<So we can do automated testing*/
  public:

  /**
   * Returns the next package in the linked list. Can be used for traversing
   * the linked list.
   * @return The next package, or NULL if there is no next package.
   */
  clPackage* GetNextPackage() {return mp_oNext;};

  /**
   * Sets the value of an integer data member.
   * @param iCode Data member code.
   * @param iValue Value to set.
   */
  void SetValue(short int iCode, int iValue);

  /**
   * Sets the value of a float data member.
   * @param iCode Data member code.
   * @param fValue Value to set.
   */
  void SetValue(short int iCode, float fValue);

  /**
   * Sets the value of a bool data member.
   * @param iCode Data member code.
   * @param bValue Value to set.
   */
  void SetValue(short int iCode, bool bValue);

  /**
   * Sets the value of a string data member.
   * @param iCode Data member code.
   * @param sValue Value to set.
   */
  void SetValue(short int iCode, std::string sValue);

//These next are structured the way they are so they can be overloaded.

  /**
   * Gets the value of an integer data member.
   * @param iCode Data member code.
   * @param p_iValHolder Address of variable into which to place the requested
   * data member value.
   */
  void GetValue(short int iCode, int *p_iValHolder);

  /**
   * Gets the value of a float data member.
   * @param iCode Data member code.
   * @param p_fValHolder Address of variable into which to place the requested
   * data member value.
   */
  void GetValue(short int iCode, float *p_fValHolder);

  /**
   * Gets the value of a boolean data member.
   * @param iCode Data member code.
   * @param p_bValHolder Address of variable into which to place the requested
   * data member value.
  */
  void GetValue(short int iCode, bool *p_bValHolder);

  /**
   * Gets the value of a string data member.
   * @param iCode Data member code.
   * @param p_sValHolder Address of variable into which to place the requested
   * data member value.
   */
  void GetValue(short int iCode, std::string *p_sValHolder);

protected:

  /**
   * Constructor. This will set up the value arrays for the grid cells. Values
   * will be initialized to 0, false, or empty string, as appropriate.
   * @param p_oParentGrid The grid for which this is a package.
   * @param p_oParentCell The cell which owns this package.
   */
  clPackage(clGrid *p_oParentGrid, struct clGrid::stcRecords *p_oParentCell);

  /**
   * Destructor.
   */
  ~clPackage();

  /**Package's parent grid.*/
  clGrid *mp_oParentGrid;
  /**Package's parent grid cell.*/
  clGrid::stcRecords *mp_parentCell;

  /**Array holding integer values. Size is m_iNumIntVals.*/
  int *mp_iIntVals;
  /**Array holding float values. Size is m_iNumFloatVals.*/
  float *mp_fFloatVals;
  /**Array holding string values. Size is m_iNumStringVals.*/
  std::string *mp_sStringVals;
  /**Array holding bool values. Size is m_iNumBoolVals.*/
  bool *mp_bBoolVals;

  /**Pointer to next package in linked list.*/
  clPackage *mp_oNext;
}; //end of class clPackage
//---------------------------------------------------------------------------
#endif

