//---------------------------------------------------------------------------

#ifndef OutputShortH
#define OutputShortH

#include "BehaviorBase.h"
#include "Constants.h"



/**
* Creates a tab-delimited text output file.  It's a shortcut
* if a detailed output file isn't what you need.  It supports subplots - as
* many as you want.
*
* This will save absolute and relative basal area and density for snags,
* adults, and saplings, and absolute density for seedlings. This will also save
* absolute basal area and density for dead trees.
*
* The namestring of this behavior, and the name which it is called in the
* behavior list of the parameter file, is "ShortOutput".
*
* Copyright 2004 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>April 28, 2004 - Submitted as beta (LEM)
* <br>June 24, 2004 - Added snag support (LEM)
* <br>October 25, 2005 - Changed the way we search for trees (LEM)
* <br>January 2, 2007 - Support for user-defined subplot resolution and made
* version 1.1 (LEM)
* <br>January 11, 2011 - Added dead trees (LEM)
* <br>October 4, 2011 - Added support for variable subplot cell size (LEM)
*/

class clShortOutput : public clBehaviorBase {

  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clShortOutput(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clShortOutput();

  /**
  * Collects data and writes the output file.
  */
  void Action();

  protected:

  /**
  * Reads the desired options out of the parameter file.
  * @param p_oDoc DOM tree of the parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**Output file name.  If it already exists, new data is appended at the end*/
  std::string m_sFileName;

  /** Array for one timestep's relative basal area - # types by # species */
  float **mp_fLiveRBA;

  /** Array for one timestep's absolute basal area - # types by # species */
  float **mp_fLiveABA;

  /** Array for one timestep's relative density - # types by # species */
  float **mp_fLiveRDN;

  /** Array for one timestep's absolute density - # types by # species */
  float **mp_fLiveADN;

  /** Array for one timestep's relative basal area by subplot - # subplots by
   * # types by # species */
  float ***mp_fSubRBA;

  /** Array for one timestep's absolute basal area by subplot - # subplots by
   * # types by # species */
  float ***mp_fSubABA;

  /** Array for one timestep's relative density by subplot - # subplots by
   * # types by # species */
  float ***mp_fSubRDN;

  /** Array for one timestep's absolute density by subplot - # subplots by
   * # types by # species */
  float ***mp_fSubADN;

  /** Array for one timestep's dead absolute basal area - # types by # species by
   * # dead reason codes*/
  float ***mp_fDeadABA;

  /** Array for one timestep's dead absolute density - # types by # species by
   * # dead reason codes*/
  float ***mp_fDeadADN;

  /**Whether to save relative basal area. Ignored for seedlings. Array size #
   * types.*/
  bool *mp_bSaveLiveRBA;

  /**Whether to save absolute basal area. Ignored for seedlings. Array size #
   * types.*/
  bool *mp_bSaveLiveABA;

  /**Whether to save relative density. Array size # types.*/
  bool *mp_bSaveLiveRDN;

  /**Whether to save absolute density. Array size # types.*/
  bool *mp_bSaveLiveADN;

  /**Whether to save absolute basal area. Ignored for seedlings. Array size #
   * types by # dead reason codes.*/
  bool **mp_bSaveDeadABA;

  /**Whether to save absolute density. Array size # types by # dead reason
   * codes.*/
  bool **mp_bSaveDeadADN;

  /**Shortcut flag for whether to save any information, by type */
  bool *mp_bSaveAnyLive;

  /**Shortcut flag for whether to save basal area information, by type */
  bool *mp_bSaveAnyLiveBA;

  /**Shortcut flag for whether any live trees are to be saved all */
  bool m_bUseLive;

  /**Shortcut flag for whether any dead trees are to be saved all */
  bool m_bUseDead;


  /**
  * Structure for holding coordinates
  */
  struct stcCoords {int iX; /**<Grid cell X number*/
                    int iY; /**<Grid cell Y number*/
                    };

  /**
  * Data structure for saving subplot data.  They get the same data saved as
  * the plot as a whole.
  */
  struct stcSubplotInfo {
    /**Name of the subplot so it can be identified in output files*/
    std::string sSubplotName;
    stcCoords *p_cellList; /**<Array of grid cell coords*/
    float fArea; /**<Subplot's area, in ha*/
    short int iNumCells; /**<Number of grid cells in this subplot*/
  } *mp_subplots; /**<An array of stcSubplotInfos, one for each subplot to save*/
  short int m_iNumSubplotsToSave; /**<Number of subplots we're saving data for*/

  /**Number of plot cells in the X direction - for subplots*/
  short int m_iNumXCells;

  /**Number of plot cells in the Y direction - for subplots*/
  short int m_iNumYCells;

  /**Number of types*/
  short int m_iNumTypes;

  /**Number of species*/
  short int m_iNumSpecies;

  /**Length of subplot cells in X direction - defaults to match tree
   * population*/
  float m_fXCellLength;

  /**Length of subplot cells in Y direction - defaults to match tree
   * population*/
  float m_fYCellLength;

  /**
  * Writes output file header.
  */
  void WriteFileHeader();

  /**
  * Extracts the output data relating to tree saving from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void ExtractLiveTreeInfo(xercesc::DOMDocument *p_oDoc);

  /**
  * Extracts the output data relating to tree saving from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void ExtractDeadTreeInfo(xercesc::DOMDocument *p_oDoc);

  /**
  * Extracts the subplot data from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  */
  void ExtractSubplotInfo(xercesc::DOMDocument *p_oDoc);

  /**
  * Tests to see if a given node exists and has an attribute called "save"
  * equal to true.
  *
  * @param p_oParentElement Pointer to the parent element of the node in
  * question.
  * @param cNodeName Tag name of the node.
  * @return True if the save attribute is true; if the node does not exist or
  * is equal to false, false is returned.
  */
  bool TestForSave(DOMElement *p_oParentElement, const char *cNodeName);

  /**
   * For a time step, collects all basal area and density information for dead
   * trees.
   */
  void GetDeadTreeStats();

  /**
   * For a time step, collects all basal area and density information for live
   * trees.
   */
  void GetLiveTreeStats();

  /**
   * Allocates memory for all of our arrays.
   */
  void DeclareDataArrays();

  /**
   * Allocates memory for those arrays pertaining to subplots. This must occur
   * after the information has been extracted from the parameter file because we
   * need to know how many subplots there are.
   */
  void DeclareDataArraysForSubplots();

  /**
   * Write the data for a single time step.
   */
  void WriteTimestepData();

};
//---------------------------------------------------------------------------
#endif
