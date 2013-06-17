//---------------------------------------------------------------------------

#ifndef GridsH
#define GridsH

#include "ObjectManagerBase.h"

class clGrid;
/**
 Grid Manager - Version 1.0
 The file names for this unit are TheGrids.x because for some reason "Grids"
 were unacceptable to Builder.

 The Grid Manager has a job unique among object managers in that the grid
 objects are relatively dumb.  They are not all unique classes, they are
 separate instantiations of the same class.  So the Grid Manager takes care
 of their construction.

 Definitions of grid objects in the parameter or other file are assumed to be
 part of the definitions for the behaviors that will work on them, so the grid
 manager does not have a data reading routine.  If another object wishes to
 initialize a grid object with data, it must assign the values itself after
 requesting that the grid manager create the grid.

 Copyright 2003 Charles D. Canham.
 @author Lora E. Murphy

 <br>Edit history:
 <br>-----------------
 <br>April 28, 2004 - Submitted as beta (LEM)
*/
class clGridManager : public clObjectManagerBase {
public:

/**
 * Constructor. This constructor structure makes sure that there's no default
 * constructor while also saying that this child class doesn't need its own
 * constructor to do anything.
 * @param p_oSimManager Sim Manager object.
*/
  clGridManager(clSimManager *p_oSimManager) :
                            clObjectManagerBase(p_oSimManager){;};
  //~clGridManager(); No destructor needed

/**
 * Creates grid objects from input file if there are maps.
 * @param p_oDoc DOM tree of parsed input file.
*/
  void CreateObjects(xercesc::DOMDocument *p_oDoc);

/**
 * This creates a grid object. If there is already a grid object with the passed
 * name, it overwrites it.
 * @param sGridName The new grid's namestring.
 * @param iNumIntVals Number of integer data members in a grid cell record. Can
 * be 0.
 * @param iNumFloatVals Number of float data members in a grid cell record. Can
 * be 0.
 * @param iNumStringVals Number of string data members in a grid cell record.
 * Can be 0.
 * @param iNumBoolVals Number of bool data members in a grid cell record. Can
 * be 0.
 * @param fXCellLength The length of a grid cell in the X direction, in meters.
 * Not required. If ommitted this will default to the plot's cell length.
 * @param fYCellLength The length of a grid cell in the Y direction, in meters.
 * Not required. If this is ommitted (i.e. = 0), it is assumed the grid cells
 * are square and the value for the X length is used.
 * @return A pointer to the new grid object.
*/
  clGrid* CreateGrid(std::string sGridName, short int iNumIntVals,
      short int iNumFloatVals, short int iNumStringVals, short int iNumBoolVals,
      float fXCellLength = 0,  float fYCellLength = 0);

protected:


};


//---------------------------------------------------------------------------
#endif
