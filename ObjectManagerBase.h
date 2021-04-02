//---------------------------------------------------------------------------

#ifndef ObjectManagerBaseH
#define ObjectManagerBaseH
//---------------------------------------------------------------------------
#include "Messages.h"
#include "DataTypes.h"
#include <xercesc/dom/DOM.hpp>


class clWorkerBase;
class clSimManager;

/**
 * OBJECT MANAGER - Version 1.0
 *
 * This class is a virtual base class for the various types of object
 * managers - those which handle behaviors, populations, and grids. An object
 * manager controls all of the objects of a given type. It creates and destroys
 * them, triggers their management functions, and controls access to them. The
 * Simulation Manager directs the object managers so that they trigger the
 * correct functioning at the proper time within a run.
 *
 * This class should not be instantiated as an object but used only as a base
 * class for the other manager classes.  It serves as a common interface for
 * the Simulation Manager to work with the various object managers.
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Changes:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 * <br>November 6, 2012 - Took away mp_oSimManager's static designation; it
 * interfered with testing
*/
class clObjectManagerBase {
protected:

  /**The array of objects under the control of the object manager.*/
  clWorkerBase **mp_oObjectArray;

  /**The number of objects currently under management.*/
  int m_iNumObjects;

  /**Pointer to the Simulation Manager.*/
  clSimManager *mp_oSimManager;

  /**
   * Creates the managed objects for a run.  Which objects are created is
   * controlled by the file that is passed; each object manager must know what
   * to do with the file.  The objects are created only - they are not given the
   * opportunity to do any setup at this point other than what is in their
   * constructors.
   * @param p_oDoc Pointer to the DOM tree created from the parsed XML file.
   */
  virtual void CreateObjects(xercesc::DOMDocument *p_oDoc) {;};

public:

  /**
   * Returns the version number of the clObjectManagerBase class.
   * @return clObjectManagerBase class version number.
   */
  int GetObjectVersion() {return 1;};

  /**
   * Constructor
   * @param p_oSimManager Pointer to the Simulation Manager object
   */
  clObjectManagerBase(clSimManager *p_oSimManager); //constructor

  /**
   * Destructor
   */
  virtual ~clObjectManagerBase(); //destructor

  /**
   * Deletes the managed objects array.
   */
  void FreeMemory();

  /**
   * Returns the number of objects under management for an object manager.
   */
  int GetNumberOfObjects() {return m_iNumObjects;};

  /**
   * Gets a specified object under management.
   * @param iObjectNumber Object's zero-based index number in the object
   * manager's object array.
   * @return The requested object, or NULL if there is no such object index.
   */
  clWorkerBase* PassObjectPointer(int iObjectNumber);

  /**
   * Gets a specified object under management.
   * @param sName Object's namestring.
   * @return The requested object, or NULL if there is no such object.
   */
  clWorkerBase* PassObjectPointer(std::string sName);

  /**
   * Triggers the setup process for all objects managed by this manager.  To
   * trigger setup, each managed object's DoSetup function is called.
   * @param p_oDoc Pointer to parsed and validated DOM tree.  This function
   * assumes that the file is of a type that it knows how to read.
   * @param iFileType What type of file is being passed.
   */
  void DoObjectSetup(xercesc::DOMDocument *p_oDoc, fileType iFileType);

  /**
   * Completes timestep cleanup tasks both for this object manager and all its
   * managed objects.  Each managed object's TimestepCleanup function is called.
   */
  void TimestepCleanup();

  /**
   * Completes any end-of-run cleanup tasks both for this object manager and all
   * its managed objects.  Each managed object's EndOfRunCleanup function is
   * called.
   */
  void EndOfRunCleanup();

};
//-----------------------------------------------------------------------------
#endif
