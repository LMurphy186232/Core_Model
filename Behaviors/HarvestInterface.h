//---------------------------------------------------------------------------

#ifndef HarvestInterfaceH
#define HarvestInterfaceH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"

class clGrid;
using namespace whyDead;

/** Maximum length of the argument string to pass to the user executable */
#define MAX_ARGUMENT_LENGTH 1000

/**
* Harvest Interface - Version 1.1
*
* This behavior serves as an interface between SORTIE and a user-written
* executable for harvesting. (Although code that did something besides
* harvesting is certainly possible for the creative programmer as well.)
*
* This behavior executes on a user-defined cycle. When it executes, it gathers
* all eligible trees and writes them to a text file. It then calls the user
* executable. It expects the executable to create a file (with a pre-arranged
* name) with the list of trees to cut.
*
* The user may also ask for new tree data members to be added on the
* executable's behalf. If this is the case, the user executable can also write
* a file of trees to to update with new values for those data members. The
* new data members (which are all floats), and their initial values, are passed
* in on the behavior's parameter file call string as follows: "HarvestInterface
* (name1) (name2)" etc. The initial values will always be 0. (We cannot do
* otherwise - a SORTIE run is constantly creating new trees, but we don't know
* which ones they are; so this behavior will never know (outside of initial
* conditions) which trees are new and which have a legitimately set value of 0.)
*
* The file SORTIE writes for the user executable is tab-delimited text and has
* the following format:
* Line 1, two columns: Current timestep, total number of timesteps
* Line 2, column names, 6+n columns:  "X", "Y", "Species", "Type", "Diam",
* "Height", [any additional variables that the user wishes].
* Subsequent lines, 6+n columns, one line per tree: X, Y, species number,
* type number, DBH/diam10, height, [any additional variables that the user
* wishes, including new variables they have defined]. The "Diam" value is
* diameter at 10 cm if the tree type is seedling, and DBH in all other cases.
*
* The file format of the user response files is identical to that of the SORTIE
* file, with the same columns in the same order. Each timestep, all these
* files are overwritten. Any other files in the working directory are ignored.
* (Incidentally, the timestep line in the response file will be ignored by
* SORTIE, so technically it doesn't have to be anything in particular.)
*
* The list of harvested trees will be used to update the "Harvest Results" grid.
* This grid is the same as that produced by the clDisturbance class; however, if
* clDisturbance is also trying to use this grid in the same run, they will not
* play nicely together and will probably overwrite each other's results (at
* best). For each harvested species, this grid contains members called
* "Cut Density_0_x" and "Cut Basal Area_0_x" where x is the species number.
* These are raw density/BA values for each grid cell (8 m x 8 m) and are not
* converted to per ha amounts. The grid also contains a data member called
* "Harvest Type" that equals -1 if there was no harvesting in a grid cell this
* timestep, or 0 (partial cut) if there were harvested trees.
*
* The user executable runs once per timestep. It does not stay running. So
* it must do any necessary initialization and setup each time. The executable
* can be written in any language, and can do anything it wishes. The only two
* requirements is that it be a standalone executable, and that it produce the
* file of trees to harvest that SORTIE expects.
*
* If the user executable wants input parameters through SORTIE, it can have this
* behavior pass them as a string argument when the executable is launched. If
* it uses a file for input parameters, and SORTIE is running in batch mode, the
* user can specify a file with parameters for each run, one set per line; this
* behavior will extract the line corresponding to the batch number and write it
* to a specified file before calling the user executable.
*
* The name string for this behavior is "HarvestInterface"; the parameter file
* call string is as described above.
*
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clHarvestInterface : virtual public clBehaviorBase {

  public:

  /**
   * Constructor.
   *
   * @param p_oSimManager Sim Manager object.
  */
  clHarvestInterface(clSimManager *p_oSimManager);

  /**
   * Destructor
  */
  ~clHarvestInterface();

  /**
   * Performs the harvest. The timestep is compared to
   * m_iNextTimestepToHarvest. If they are not equal, the behavior exits. If
   * they are equal, then:
   * <ul>
   * <li>m_iNextTimestepToHarvest is incremented with the value in
   * m_iPeriodicity</li>
   * <li>All trees to which this behavior applies are collected and written
   * to an input file for the harvesting program</li>
   * <li>The harvesting program is called, passing the parameters and the
   * input file of trees</li>
   * <li>The harvest (and possibly update) output file(s) are read into
   * memory</li>
   * <li>All trees to which this behavior applies are compared to the harvest
   * (and output) list(s) and if they're on it (by way of type, species, X, Y,
   * and diameter being equal) they are killed (or updated).</li>
   * </ul>
  */
  void Action();

  /**
  * Performs setup. This reads from the parameter file. It verifies the
  * working directory and its contents. If this is a batch run and the user
  * has asked for the batch parameter file splitting, it writes this run's
  * parameters.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if:
  * <ul>
  * <li>The user executable is missing</li>
  * <li>Tree data members requested in the harvest file do not exist (data
  * members that already exist, not those being created by this interface)</li>
  * <li>Number of timesteps between calls to the user executable is
  * negative</li>
  * <li>Set of batch parameters too short for the number of requested runs</li>
  * </ul>
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Captures the namestring passed to this behavior. This parses out any tree
  * data members that may be riding along on the string.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

  /**
  * Registers any new data members requested by the user.
  */
  void RegisterTreeDataMembers();


  protected:

  /** Enum for types of tree data members */
  enum member_type {notassigned, /**<Not assigned a type*/
                    isfloat, /**<Float data member*/
                    isint,   /**<Int data member*/
                    ischar,  /**<Char data member*/
                    isbool   /**<Bool data member*/
  } *mp_iMemberType; /**<Array for holding tree data member types. Array
   size is m_iNumFileColumns.*/

  /** Array of labels for any new tree data members that this interface is to
   * create. The array is sized m_iNewTreeFloats. */
  std::string *mp_sNewTreeDataMembers;

  /** Array of columns in the files. These are labels of tree data members
   * (with the exception of the generic "Diam"). The array is sized
   * m_iNumFileColumns. */
  std::string *mp_sFileColumns;

  /** Path to user executable */
  std::string m_sExecutable;

  /** Path and filename of the input file to the harvest executable that this
   * behavior will prepare */
  std::string m_sInputFile;

  /** Path and filename of the file of the trees to cut that is created by the
   * harvest executable */
  std::string m_sTreesToCutFile;

  /** Path and filename of the file of the trees to update that is created by
   * the harvest executable - empty string if not needed */
  std::string m_sTreesToUpdateFile;

  /** Path and filename of the batch parameters input file that this will
   * read - empty string if not needed*/
  std::string m_sBatchFileIn;

  /** Path and filename of the parameters output file that this will write in
   * the event of a batch - empty string if not needed*/
  std::string m_sBatchParamsOut;

  /** Argument string to pass to the user executable - empty string if not
   * needed*/
  std::string m_sArguments;

  /** Which trees this behavior is applied to. The first index is number of
   * species, the second is number of types.
   */
  bool **mp_bAppliesToTrees;

  /** Initial values for the new float data members (if any). Array size is
   * m_iNewTreeFloats. */
  //float *mp_fInitialValues;

  /** Translation array between file columns and user-defined data members.
   * Array size is m_iNumFileColumns. For each column, the value is the
   * matching array index in mp_cNewTreeDataMembers, or -1 if the column is
   * not a new user-defined tree data member.*/
  int *mp_iColumnTranslation;

  /** Which of the file columns are user-defined new tree data members.
   * Array size is m_iNumFileColumns.*/
  bool *mp_bUserDefinedColumn;

  /** The next timestep a harvest will be performed */
  int m_iNextTimestepToHarvest;

  /** How often, in timesteps, harvests occur */
  int m_iPeriod;

  /** Number of columns in the text files. */
  int m_iNumFileColumns;

  /** Total number of species, for the destructor */
  int m_iNumSpecies;

  /** Number of cut ranges allowed in "Harvest Results" grid - this comes from
   * the class clDisturbance.*/
  short int m_iNumAllowedCutRanges;

  /**Reason code to pass to the tree population when trees are killed.*/
  deadCode m_iReasonCode;

  /**Harvest Type data member code in "Harvest Results" grid*/
  short int m_iHarvestTypeCode;

  /**Cut Density data member codes in "Harvest Results" grid. Array size is
   * number of cut ranges by number of species*/
  short int **mp_iDenCutCodes;

  /**Cut Basal Area data member codes in "Harvest Results" grid. Array size is
   * number of cut ranges by number of species*/
  short int **mp_iBaCutCodes;

  /** Harvest results grid, with same format as in clDisturbance. */
  clGrid *mp_oResultsGrid;

  /**
   * Resets all the values in the Harvest Results grid. Harvest Type becomes
   * -1; all others are 0.
  */
  void ResetResultsGrid();

  /**
   * Sets up the "Harvest Results grid" and gets all the return codes.
  */
  void SetupResultsGrid();


};
//---------------------------------------------------------------------------
#endif

