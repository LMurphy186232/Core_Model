//---------------------------------------------------------------------------
#ifndef SimManagerH
#define SimManagerH

//The interface include - should be whatever file contains the function
//definition for SendMessage().  Within projects, manage directories in order to
//include the right one.
#include "Interface.h"
#include "Constants.h"
#include "DataTypes.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

class clBehaviorBase;
class XercesDOMParser;
class clXercesErrorHandler;
class clModelEntityResolver;
class clBehaviorManager;
class clPopulationManager;
class clGridManager;
class clPlot;
class clGrid;
class clPopulationBase;
class clWorkerBase;
class clModelMath;

/**
* SIMULATION MANAGER - Version 1.0
*
* The Simulation Manager is responsible for managing the run process.  It
* accepts input files and causes them to be entered.  It also triggers a run.
*
* Objects within the run access each other through the Simulation Manager.  An
* object may have direct access to no other object, but they always have access
* to the Simulation Manager and from there can get to (almost) any other object.
*
* The Sim Manager has four states:  No_Data, Initialized, Paused, and
* Run_Complete.  These are described in more detail below.  The state in which
* the Sim Manager is in controls what actions are allowed and how actions are
* performed (for instance, starting a run is not allowed in the No_Data state).
*
* The Simulation Manager takes care of all XML input file parsing.  DTDs do exist
* but parsed files are not validated - they are merely checked for well-
* formedness.  This is because I can't get Xerces' validation to work with a
* DTD I specify - something that should be possible but has a bug in the current
* version of the libraries.  This should be fixed at some point.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clSimManager {
  friend class clTestSimManager; /**<For automated testing*/

  protected:

  /**Path to this application. Not a const because we have to set this at runtime*/
  std::string m_sAppPath;

  /**The actual random seed that was used. This is only needed in case random
   * seed in the parameter file is zero - this allows capture for re-creation
   * if needed.*/
  long m_iActualSeed;

  /**Number of years per timestep - could be fractional or less than 1*/
  float m_fNumYearsPerTimestep;

  //Versions
  /**Model's major version number. For version control.*/
  int m_iMajorVersion;

  /**Model's minor version number. For version control.*/
  int m_iMinorVersion;

  /**Seed for the random number generator. This is read from the parameter file.
   * It must be negative; if entered as a positive, it's negativized. If it's 0,
   * a negative integer is chosen.*/
  int m_iRandomSeed;

  /**Number of timesteps for the run*/
  int m_iNumTimesteps;

  /**Timestep that the model is currently on. If the model is paused this is the
   * timestep just finished.*/
  int m_iCurrentTimestep;

  /**If the model was paused this holds the original target timestep so it
   * can resume.*/
  int m_iTargetTimestep;

  /**If this is a batch run, this is the ith parameter file group of the
   * current batch file. If not in batch mode, this is 0.*/
  int m_iBatchGroup;

  /**If this is a batch run, this is the ith run of the current parameter file.
   * If not in batch mode, this is 0.*/
  int m_iBatchNumber;

  //Object managers
  clBehaviorManager *mp_oBehaviorManager; /**<The behavior object manager.*/
  clPopulationManager *mp_oPopulationManager; /**<The population object manager.*/
  clGridManager *mp_oGridManager; /**<The grid object manager.*/

  clPlot *mp_oPlot; /**<The plot object.  This does not have its own object
        manager - it remains in the purview of the simulation manager.*/

  /**File path and name of the parameter file.*/
  std::string m_sParFilename;
  /**File path and name of the parameter file, if using.*/
  std::string m_sBatchFilename;

  bool m_bUserQuit; /**<Flag for if a user is quitting the app*/
  bool m_bUserPaused; /**<Flag for if a user has paused the app*/

  //XML tools
  xercesc::XercesDOMParser *mp_oXMLParser; /**<Xerces file parser.*/

  /**
  * Triggers the timestep cleanup functions of each of the object managers.
  */
  void TimestepCleanup();

  /**
  * Performs any cleanup necessary at the end of a run and triggers the
  * end-of-run cleanup functions of each of the object managers.
  */
  void EndOfRunCleanup();

  /**
  * Reads a parsed parameter file.  After the simulation manager is done, the
  * file is passed off to the object managers to give to the objects under
  * their management.
  *
  * This does not error-trap robustly; it assumes that in the parsing process
  * the document is established to be well-formed and valid.  This, of course,
  * is a fallacy; the documents are not currently validated against a DTD but
  * should be.  I'm counting on the fact that is unlikely that an invalid file
  * will get past the objects without triggering some sort of recoverable
  * error.
  *
  * If there is an error during the parameter file reading process, all objects
  * are destroyed.
  *
  * @param p_oDoc Pointer to the DOM tree of the parsed file.
  * @param iFileType The type of file that was parsed.
  */
  void ReadParameterFile(xercesc::DOMDocument *p_oDoc, fileType iFileType);

  /**
  * Passes a parsed tree file off to the tree population object to read it as
  * it will.
  *
  * Any expected errors caught will not cause the model state to reset.
  * Unexpected errors will cause reset.
  *
  * @param p_oDoc Pointer to the DOM tree of the parsed file.
  * @param iFileType The type of file that was parsed.
  */
  void ReadTreeFile(xercesc::DOMDocument *p_oDoc, fileType iFileType);

  /**
  * Inputs a parsed map file.  It passes off the document object to the
  * grid manager to find a grid to read it.
  *
  * Any expected errors caught will not cause the model state to reset.
  * Unexpected errors will cause reset.
  *
  * @param p_oDoc Pointer to the DOM tree of the parsed file.
  * @param iFileType The type of file that was parsed.
  */
  void ReadMapFile(xercesc::DOMDocument *p_oDoc, fileType iFileType);

  /**
  * Reads in the simulation manager's needed values from the parameter file.
  *
  * @param p_oDoc Pointer to the DOM tree of the parsed file.
  */
  void DoSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Performs the transition to the No_Data state from any other state.
  */
  void GoToNoDataState();

  public:

  /**
  * Constructor.
  * @param iMajorVersion Major model version number.
  * @param iMinorVersion Minor model version number.
  * @param sAppPath Path of the application
  */
  clSimManager(int iMajorVersion, int iMinorVersion, const std::string sAppPath);

  /**
  * Destructor.
  */
  ~clSimManager();

  /**
  * Gets the path of the application.
  * @return Path to the application.
  */
  std::string GetAppPath() {return m_sAppPath;};

  /**
  * Returns the batch number of the current run.
  * @return Batch number for the current parameter file, or 0 if not in a batch
  * run.
  */
  int GetBatchNumber() {return m_iBatchNumber;};

  /**
  * Enum for simulation states.
  */
  enum simState{No_Data, /**<Object managers are created but have no objects -
                          system is ready to receive data*/
                 Initialized, /**<Sufficient input data has been received - a
                 run is possible*/
                 Paused, /**<There is an unfinished run in process*/
                 Run_Complete /**<A run is complete*/
                };

  /**
  * Returns the parameter file path and name.
  * @return Parameter file path and name.
  */
  std::string GetParFilename() {return m_sParFilename;};

  /**
  * Returns the model major version number.
  * @return Model major version number.
  */
  int GetMajorVersion() {return m_iMajorVersion;};

  /**
  * Returns the model minor version number.
  * @return Model minor version number.
  */
  int GetMinorVersion() {return m_iMinorVersion;};

  /**
  * Returns the number of years per timestep.
  * @return Number of years per timestep.  Could be fractional.
  */
  float GetNumberOfYearsPerTimestep() {return m_fNumYearsPerTimestep;};

  /**
  * Returns the random seed as read in from the parameter file.  DON'T USE THIS
  * AS INPUT TO THE RANDOM NUMBER GENERATOR!
  * @return Parameter file random seed.
  */
  int GetParameterFileRandomSeed() {return m_iRandomSeed;};

  /**
  * Returns the actual random seed, which is what should be used as the seed
  * for the random number generator.
  * @return Actual random seed.
  */
  long* GetRandomSeed() {return &m_iActualSeed;};

  /**
  * Returns the number of timesteps specified in the parameter file.
  * @return Number of timesteps for the run.
  */
  int GetNumberOfTimesteps() {return m_iNumTimesteps;}

  /**
  * Returns the current timestep being executed.
  * @return Current timestep, or 0 if a run is not underway.
  */
  int GetCurrentTimestep() {return m_iCurrentTimestep;}

  /**
  * Sets the current timestep.  This is mostly for my convenience, for testing
  *  purposes.  Mess with this at your peril.
  * @param iTimestep Current timestep to set.
  */
  void SetCurrentTimestep(int iTimestep) {
    if (iTimestep > 0 && iTimestep < m_iNumTimesteps)
      m_iCurrentTimestep = iTimestep;
  };

  /**
  * Returns the current state of the Simulation Manager.
  * @return State of the Simulation Manager, as a simState enum value.
  */
  int GetSimState() {return m_eSimState;}

  /**
  * Returns the number of behavior objects for this run.
  * @return Number of behavior objects for the currently defined run, or 0 if a
  * run is not currently defined.
  */
  int GetNumberOfBehaviors();

  /**
  * Returns the number of grid objects for this run.
  * @return Number of grid objects for the currently defined run, or 0 if a
  * run is not currently defined.
  */
  int GetNumberOfGrids();

  /**
  * Returns the number of population objects for this run.
  * @return Number of population objects for the currently defined run, or 0
  * if a run is not currently defined.
  */
  int GetNumberOfPopulations();

  /**
  * Attempts to open and read a file's contents into SORTIE.  The file type
  * code that is expected in all SORTIE XML files is checked to seee what type
  * of file this is, and to decide what to do with it.  If the file is a
  * non-SORTIE file, the state of the sim manager is not changed.  If the file
  * is a SORTIE file and there is a problem reading the file, any damage caused
  * by attempting to read is cleaned up and the state of the sim manager is set
  * to "Intialized."  If damage could not be cleaned up the state is set to
  * "Not_Initialized" and the only thing to do will be to start over with a
  * new file.
  *
  * On catching an error, this function does not automatically reset the model
  * state.  Some files which are read may produce non-fatal errors from which
  * the system can recover.  If functions called by this function can trap and
  * handle these errors, fine.  An error which is not handled by the time it
  * reaches this function gets passed further up the chain and will probably
  * call a reset of the model state to No_Data.
  *
  * @param sFileName Path and name of file to read.
  */
  void ReadFile(std::string sFileName);

  /**
  * Attempts to determine what kind of file a file is.  This doesn't dig too
  * deeply; if the file extension and file code indicate a particular file
  * type, this function believes them.  This function can recognize old SORTIE
  * file extensions and XML files.  If a file is XML, this function looks for a
  * filecode, which all SORTIE XML files should have.
  *
  * The XML file code is an 8-character string divided into four sets of two
  * characters.  The first set of two characters is the model major version;
  * the second set of two characters is the minor versions; the third set of
  * two characters is the file type; and the fourth set is the file version.
  *
  * @param sFileName Path and name of file to read.
  * @return A fileType enum value.
  */
  fileType GetFileType(std::string sFileName);

  /**
  * Attempts to determine what kind of file produced a particular parsed DOM
  * tree. This doesn't dig too deeply; if the file code indicates a recognized
  * file type, this function believes them.  If the file code is absent, the
  * filetype returned is notrecognized.
  *
  * The XML file code is an 8-character string divided into four sets of two
  * characters.  The first set of two characters is the model major version;
  * the second set of two characters is the minor versions; the third set of
  * two characters is the file type; and the fourth set is the file version.
  *
  * @param p_oDoc Pointer to the DOM tree of the parsed file.
  * @return A fileType enum value.
  */
  fileType GetFileType(xercesc::DOMDocument *p_oDoc);

  /**
  * Runs the simulation.  If the Simulation Manager state is Initialized or
  * Paused, the run goes forward from the current point.  If the state is
  * Run_Complete, the run is reset and run from the beginning.  If the state is
  * No_Data, an error is thrown.
  *
  * @param iNumStepsToRun Number of timesteps to run (optional).  A value of 0
  * runs the model to the end.  A value greater than the number of timesteps
  * left just runs the model to the end.
  *
  * @return The length of the run, in seconds.
  */
  unsigned long RunSim(int iNumStepsToRun = 0);

  /**
  * Returns a population object.
  *
  * @param sPopName Population's namestring.
  * @return The population object, or NULL if there is no such object.  For
  * most purposes, it will be necessary to cast the pointer to the particular
  * population class desired.
  */
  clPopulationBase* GetPopulationObject(std::string sPopName);

  /**
  * Returns a population object.
  *
  * @param iIndex Population's zero-based index number in the population
  * manager's object array.
  * @return The population object, or NULL if there is no such index.  For most
  * purposes, it will be necessary to cast the pointer to the particular
  * population class desired.
  */
  clPopulationBase* GetPopulationObject(int iIndex);

  /**
  * Returns a grid object.
  *
  * @param cGridName Grid's namestring.
  * @return The grid object, or NULL if there is no such object.
  */
  clGrid* GetGridObject(const char *cGridName);

  /**
  * Returns a grid object.
  *
  * @param iIndex Grid's zero-based index number in the grid manager's
  * object array.
  * @return The grid object, or NULL if there is no such object.
  */
  clGrid* GetGridObject(int iIndex);

  /**
  * Returns a behavior object.
  *
  * @param iIndex Behavior's zero-based index number in the behavior manager's
  * object array.
  * @return The behavior object, or NULL if there is no such index.  For most
  * purposes, it will be necessary to cast the pointer to the particular
  * behavior class desired.
  */
  clBehaviorBase* GetBehaviorObject(int iIndex);

  /**
  * Returns a behavior object.
  *
  * @param sBehaviorName Behavior's namestring.
  * @return The behavior object, or NULL if there is no such index.  For most
  * purposes, it will be necessary to cast the pointer to the particular
  * behavior class desired.
  */
  clBehaviorBase* GetBehaviorObject(std::string sBehaviorName);

  /**
  * Returns the plot object.
  *
  * @return The plot object, or NULL if it has not been created.
  */
  clPlot* GetPlotObject() {return mp_oPlot;};

  /**
  * Executes a batch run as specified in a batch file.
  *
  * @param sBatchFile File path and name to the batch file to execute.
  */
  void RunBatch(std::string sBatchFile);

  /**
  * Creates a new grid object.  This function merely passes along the request
  * to the grid manager.
  *
  * Note that a grid may have already been created in a grid map in a parameter
  * file by the time a behavior is ready to set up.  This function would
  * overwrite any existing grids, so if a behavior is able to take advantage of
  * grid map values, it should check for the existence of the grid before
  * creating a new one.
  *
  * @param sGridName The new grid's namestring.
  * @param iNumIntVals Number of integer data members in a grid cell record.
  * Can be 0.
  * @param iNumFloatVals Number of float data members in a grid cell record.
  * Can be 0.
  * @param iNumCharVals Number of char data members in a grid cell record.
  * Can be 0.
  * @param iNumBoolVals Number of bool data members in a grid cell record.
  * Can be 0.
  * @param fXCellLength The length of a grid cell in the X direction, in
  * meters.  Not required.  If ommitted this will default to the plot's
  * cell length.
  * @param fYCellLength The length of a grid cell in the Y direction, in
  * meters.  Not required.  If this is ommitted (i.e. = 0), it is assumed the
  * grid cells are square and the value for the X length is used.
  *
  * @return A pointer to the new grid object.
  */
clGrid* CreateGrid(std::string sGridName, short int iNumIntVals,
      short int iNumFloatVals, short int iNumCharVals, short int iNumBoolVals,
      float fXCellLength = 0,  float fYCellLength = 0);

protected:

  enum simState m_eSimState; /**<Current state of the Simulation Manager.*/
};
//////////////////////END OF CLASS clSimManager///////////////////////////////

extern modelMsg CheckForMessage();
//---------------------------------------------------------------------------
#endif
