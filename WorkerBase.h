//---------------------------------------------------------------------------

#ifndef WorkerBaseH
#define WorkerBaseH

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include "Messages.h"
#include "Constants.h"
#include "DataTypes.h"


using namespace xercesc;

class clSimManager;

/**
 * WORKER BASE - Version 1.0
 *
 * The worker base class serves as a virtual base class for all data and
 * behavior objects. These object types each have their own managers. The
 * managers themselves have a base class and this object allows that manager
 * base class to access the objects.
 *
 * This class only serves as a parent class for other classes; do not
 * instantiate objects of this class.
 *
 * In order for the dynamic casting to work, this must be a polymorphic base
 * class, which means it must have a virtual function. So make sure there's
 * always one.
 *
 * There are is a virtual function that all child classes must overload.
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 * <br>November 6, 2012 - Made mp_oSimManager not static. It interfered with
 * testing. (LEM)
 * <br>November 12, 2012 - Chars became strings (LEM)
*/
class clWorkerBase {

public:

  /**
   * Constructor. The constructor initializes the common variables.
   * @param p_oSimManager Sim Manager object.
   */
  clWorkerBase(clSimManager *p_oSimManager);

  /**
   * Destructor. Deletes the common variables.
   */
  virtual ~clWorkerBase();

  /**
   * Gets the object's namestring. This name can be used to search for the
   * object.
   * @return Namestring.
   */
  std::string GetName() {return m_sNameString;};

  /**
   * Triggers the setup process. This function should not be overridden.
   * @param p_oDoc DOM tree of parsed input file.
   * @param iFileType Input file's type.
   */
  void DoObjectSetup(xercesc::DOMDocument *p_oDoc, fileType iFileType);

  /**
   * Performs any necessary cleanup operations at the end of a timestep. Child
   * classes can override this function as necessary.
   */
  virtual void TimestepCleanup(){;};

  /**
   * Performs any necessary cleanup operations at the end of a run. Child
   * classes can override this function as necessary.
   */
  virtual void EndOfRunCleanup(){;};

  /**
   * If a behavior has registered a command line command with the sim manager,
   * this allows it to be called.
   * @param cCommand Command string.
   * @param cArguments Any arguments passed with the command.
   */
  virtual void DoCommand(char *cCommand, char *cArguments) {;};

//
// For commands that have been registered by objects, this asks for online
// help to be written. The single argument is the command name.
// virtual void WriteCommandHelp(char *cCommand){;};

  protected:
  std::string m_sNameString; /**<Object's identifying namestring.*/
  clSimManager *mp_oSimManager; /**<Pointer to the simulation manager
                object. Allows communication between objects.*/
  int *mp_iAllowedFileTypes; /**<List of the input file types this object can handle.*/
  int m_iNumAllowedTypes; /**<Number of input file types this object can handle.*/

  /**
   * Performs an object's setup. This must be overridden by child classes.
   * @param p_oDoc DOM tree of parsed input file.
   */
  virtual void GetData(xercesc::DOMDocument *p_oDoc) = 0;

  /**
   * Creates the proper identifying filecode for an XML file. When objects must
   * read or write XML files, they often need to use a filecode. This is an
   * 8-character string divided into four sets of two characters. The first set
   * of two characters is the model major version; the second set of two
   * characters is the minor versions; the third set of two characters is the
   * file type; and the fourth set is the file version. Given the file type and
   * file version, this will assemble the string and place it in cCode. cCode
   * should be declared to have room for the 8-digit code. AssembleFileCode()
   * will not check for appropriate length.
   * @param iFileType Type of file for which to assemble the code.
   * @param iFileVersion File's version number.
   * @param cCode A string into which the finished file code will be placed.
   */
  void AssembleFileCode(int iFileType, int iFileVersion, char *cCode);
};
//---------------------------------------------------------------------------
#endif
