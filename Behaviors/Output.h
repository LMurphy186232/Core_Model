//---------------------------------------------------------------------------

#ifndef OutputH
#define OutputH

#include "BehaviorBase.h"
#include "Constants.h"
#include <stdio.h>

class clGrid;

class clTreeSearch;
class clTree;
class clDeadTree;

using namespace std;

/**
* OUTPUT - Version 1.3
*
* This is a behavior which saves output from the model run.  It would normally
* be run last in a timestep.
*
* This behavior supports saving tree data and generic grid data.  Tree data is
* defined separately for each type (seed, seedling, sapling, adult, or dead).
*
* Filenames must be specified for the detailed output file.  The user can also
* define subplots. Each of these is stored in its own separate file.
*
* Note - all file writing is done using the old C library functions instead of
* the newer C++ functions (fstream and family).  The code was originally written
* with C++ functions and ran unacceptably slow.
*
* The behavior's namestring and parameter file callstring are both "Output".
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Changes:
* <br>-----------------
* <br>April 28, 2004 - Submitted as beta (LEM)
* <br>October 24, 2005 - Changed the way we search for trees; the results
* should be the same.
* <br>May 10, 2006 - Added support for subplots and updated to version
* 1.1 (LEM)
* <br>June 20, 2007 - Changed to strings along with Linux port (LEM)
* <br>January 2, 2008 - Support for user-defined subplot resolution and made
* version 1.2 (LEM)
* <br>January 24, 2008 - Made all path separators forward slashes for gzip and
* tar - this fixes a bug with escape characters (LEM)
* <br>January 25, 2011 - Added support for dead trees (LEM)
*/
class clOutput : public clBehaviorBase {

  public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clOutput(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clOutput();

  /**
  * Performs output data collection file writing.  It begins by examining all
  * of the desired output types and seeing which ones are ready for updating
  * for this timestep.
  */
  void Action();

  /**
  * Gets the detailed output filename.
  * @return Detailed output filename.
  */
  string GetDetailedOutputFilename()
   {return m_sFileRoot + DETAILED_OUTPUT_FILE_EXT;};

  /**
  * Figures out the filename for the detailed output timestep file and places
  * it in cFilename. This will produce the name for subplots as well.
  *
  * @param iTimestep Timestep.
  * @param iSubplot Index of the subplot - -1 = no subplot.
  * @return The completed filename.
  */
  string GetTimestepFilename(int iTimestep, int iSubplot);

  protected:

  /**
  * Reads in the parameter file values.
  *
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if the filename is too long.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  //This is the filename of the detailed output file - any other files by this name
  //will be overwritten
  string m_sFileRoot, /**<Root name of the detailed output file with no
                         file extension on it*/
         m_sTarball;  /**<Root detailed output file name plus extension for
                         tarball (gzip'ed and tar'ed)*/

  /**
  * Defines what tree output data to save
  */
  struct stcTreeOutputInfo {
   short int iSaveFreq;      /**<How often, in timesteps, to save data*/
   short int iSumTimestep;   /**<For keeping track of save frequency*/
   short int iNumInts;       /**<How many int data members we're saving for*/
   short int iNumFloats;     /**<How many float data members we're saving for*/
   short int iNumStrings;      /**<How many string data members we're saving for*/
   short int iNumBools;      /**<How mnay bool data members we're saving for*/
   short int *p_iIntCodes;   /**<Codes for int data members*/
   short int *p_iFloatCodes; /**<Codes for float data members*/
   short int *p_iStringCodes;/**<Codes for string data members*/
   short int *p_iBoolCodes;  /**<Codes for bool data members*/
   bool bSaveThisTimeStep;   /**<Shortcut for quickly determining whether this tree is saved this timestep*/
  }
  /**Array of stcTreeOutputInfo's, number of species by number of types*/
  **mp_treeSettings,
  /**Compiled settings across live and dead trees for tree map writing - if
   * any tree type/species, live or dead, saves a data member, it will be
   * here, so that it can be written*/
  **mp_masterTreeSettings,
  /**Array for dead trees, # species by # types by # dead reason codes */
  ***mp_deadTreeSettings;

  /**
  * Data structure for defining what grid output data to save
  */
  struct stcGridOutputInfo {
   clGrid *p_oGridPointer;     /**<Pointer to the grid object*/
   short int iSaveFreq;            /**<How often, in timesteps, to save data*/
   short int iSumTimestep;         /**<For keeping track of save frequency*/
   short int iNumInts;             /**<How many int data members we're saving*/
   short int iNumFloats;           /**<How many float data members we're saving*/
   short int iNumStrings;          /**<How many string data members we're saving*/
   short int iNumBools;            /**<How many bool data members we're saving*/
   short int *p_iIntCodes;         /**<Codes for int data members*/
   short int *p_iFloatCodes;       /**<Codes for float data members*/
   short int *p_iStringCodes;      /**<Codes for string data members*/
   short int *p_iBoolCodes;        /**<Codes for bool data members*/
   short int iNumPackageInts;      /**<How many package int data members*/
   short int iNumPackageFloats;    /**<How many package float data members*/
   short int iNumPackageStrings;   /**<How many package string data members*/
   short int iNumPackageBools;     /**<How mnay package bool data members*/
   short int *p_iPackageIntCodes;  /**<Codes for package int data members*/
   short int *p_iPackageFloatCodes;/**<Codes for package float data members*/
   short int *p_iPackageStringCodes;/**<Codes for package string data members*/
   short int *p_iPackageBoolCodes; /**<Codes for package bool data members*/
  } *mp_gridSettings; /**<array of stcGridOutputInfo's, one for each grid object
                     that we're saving data for*/
  short int m_iNumGridsToSave; /**<number of grid objects we're saving data for*/

  /**
  * Data structure for saving subplot data.  They get the same data saved as
  * the plot as a whole.
  */
  struct stcSubplotInfo {
    string sSubplotName; /**<Name of the subplot so it can be identified in
                         output files*/
    string sSubplotTarball; /**<Root subplot detailed output file name plus
                         extension for tarball (gzip'ed and tar'ed)*/
    bool **p_bUseCell; /**<Array of grids, with each having a true or false
                           designation for whether to include them in this
                           subplot*/
  } *mp_subplots; /**<An array of stcSubplotInfos, one for each subplot to save*/
  short int m_iNumSubplotsToSave; /**<Number of subplots we're saving data for*/

  /**Number of species*/
  short int m_iNumSpecies;

  /**Number of types*/
  short int m_iNumTypes;

  /**Number of plot cells in the X direction - for subplots*/
  short int m_iNumXCells;

  /**Number of plot cells in the Y direction - for subplots*/
  short int m_iNumYCells;

  /**Length of subplot cells in X direction - defaults to match tree
   * population*/
  float m_fXCellLength;

  /**Length of subplot cells in Y direction - defaults to match tree
   * population*/
  float m_fYCellLength;

  /**
  * Writes the detailed output header file.
  */
  void WriteDetailedOutputHeader();

  /**
  * Sets up the timestep file and writes the header data.
  *
  * @param sFilename to write to.
  */
  void WriteTimestepHeader(string sFilename);

  /**
  * Writes the closing tag for the timestep file.
  *
  * @param sFilename to write to.
  */
  void WriteTimestepFooter(string sFilename);

  /**
  * Appends the tree data to the file.
  *
  * @param p_sFilename Array of files to write to.  The first is the whole
  * plot, the rest are subplots in order.
  */
  void WriteTreeData(string *p_sFilename);

  /**
  * Appends the grid data to the file.
  * Why is this here when grid reading is in the grid object?  Because we don't
  * necessarily want to save the whole grid.  Output may have a subset of data
  * members to save.  Subset or no, all data members are always saved in the
  * settings list so that the map can be used as input.
  *
  * @param sFilename to write to.
  */
  void WriteGridData(string sFilename);

  /**
  * Extracts the output data relating to tree saving.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void ExtractLiveTreeInfo(xercesc::DOMDocument *p_oDoc);

  /**
  * Extracts the output data relating to dead tree saving.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void ExtractDeadTreeInfo(xercesc::DOMDocument *p_oDoc);

  /**
  * Extracts the output data relating to grid saving.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void ExtractGridInfo(xercesc::DOMDocument *p_oDoc);

  /**
  * Extracts the subplot data.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void ExtractSubplotInfo(xercesc::DOMDocument *p_oDoc);

  /**
   * Makes the master tree settings, for writing to the treemap header.
   */
  void MakeMasterTreeSettings();

  /**
   * Writes the XML for a single tree.
   * @param p_oTree The tree to write.
   * @param cBuf The buffer to write to.
   * @param cTemp Temp char string.
   * @param oOut File to write.
   */
  void WriteTree(clTree *p_oTree, char *cBuf, char *cTemp, FILE *oOut);

  /**
   * Writes the XML for a single dead tree.
   * @param p_oTree The tree to write.
   * @param cBuf The buffer to write to.
   * @param cTemp Temp char string.
   * @param oOut File to write.
   */
  void WriteGhost(clDeadTree *p_oTree, char *cBuf, char *cTemp, FILE *oOut);

  /**
  * Adds a string to a buffer string.  If the buffer is full, it is flushed to
  * file before the new string is added.
  *
  * @param cBuf Buffer string.
  * @param cToAdd String to add to buffer.
  * @param out File to flush buffer to.
  * @param iBufferSize Size of the buffer.
  */
  inline void AddToBuffer(char *cBuf, char *cToAdd, FILE *out, int iBufferSize) {
    if ((int)(strlen(cToAdd) + strlen(cBuf)) > iBufferSize) { //max out buffer?
      fwrite(cBuf, strlen(cBuf), 1, out);  //flush the buffer to file
      cBuf[0] = '\0';   //reset
    }
    strcat(cBuf, cToAdd);
  }
};
//---------------------------------------------------------------------------
#endif
