//---------------------------------------------------------------------------
// StormKilledPartitionedBiomass.h
//---------------------------------------------------------------------------
#if !defined(StormKilledPartitionedBiomass_H)
  #define StormKilledPartitionedBiomass_H

#include "BehaviorBase.h"

class clGrid;

/**
* Storm Killed Partitioned Biomass Calculator Version 1.0
*
* This behavior calculates biomass killed in storms, partitioned by tree part.
* This has two possible equations for calculating biomass:  one as a linear
* function of DBH, and one as a linear function of height.  If the DBH equation
* is used, then biomass is partitioned into branches, leaf, and bole. If the
* height equation is used, it is partitioned into leaf and bole only.
*
* The equations are:
* <center>Bio<sub>i</sub> = a<sub>i</sub> * DBH + b<sub>i</sub></center>
* <center>Bio<sub>i</sub> = a<sub>i</sub> * DBH + b<sub>i</sub></center>
*
* for the ith type, leaf, branch (if applicable), and bole.  Biomass values are
* not allowed to be 0.  Any negative values are set to zero.
*
* Trees eligible are snags with stm_dmg int data members.
*
* The values are collected into a grid called "Storm Killed Partitioned Biomass".
*
* This class's namestring is "StormKilledPartitionedBiomass."  If using with
* DBH, the parameter file call string is "StormKilledPartitionedDBHBiomass".
* If using with Height, the parameter file call string is
* "StormKilledPartitionedHeightBiomass". This behavior may only be applied to
* snags.
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clStormKilledPartitionedBiomass : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clStormKilledPartitionedBiomass(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clStormKilledPartitionedBiomass();

  /**
  * Makes value calculations.  First, the values in the "Storm Killed
  * Partitioned Biomass" grid are cleared.  Then a query is sent to the tree
  * population to get all trees to which this behavior is applied.  For each,
  * the amount of biomass (in metric tons) is calculated, and the species
  * values are totaled and placed in the "Storm Killed Partitioned Biomass"
  * grid.
  */
  void Action();

  /**
  * Does setup for this behavior.  Calls:
  * <ol>
  * <li>GetAppliesTo()</li>
  * <li>GetParameterFileData()</li>
  * <li>FormatQueryString()</li>
  * <li>SetupGrid()</li>
  * <li>Action() so that the initial conditions value will be added</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  * @throws modelErr if this behavior has been applied to seedlings.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Captures the namestring passed to this behavior. If the namestring is
  * "Partitioned DBH Biomass", this sets m_bIsDbh to true.  If the namestring
  * is "Partitioned Height Biomass", this sets m_bIsDbh to false.
  *
  * @param sNameString Behavior's namestring.
  * @throws modelErr if the namestring is not recognized.
  */
  void SetNameData(std::string sNameString);

  protected:

  /** Grid holding total values for each species.  The grid name is
   * "Storm Killed Partitioned Biomass".  It has 5 times X float data members,
   * where X = the total number of species.  The data member names are
   * "leaf_x", "branch_x", "bole_x" for the DBH biomass, where "x" is the
   * species number, and "hleaf_x" and "hbole_x" for the height biomass, where
   * "x" is the species number.*/
  clGrid* mp_oGrid;

  /** "a" for the DBH-based leaf biomass.  Array size is # total species.*/
  float *mp_fDBHLeafA;

  /** "b" for the DBH-based leaf biomass.  Array size is # total species.*/
  float *mp_fDBHLeafB;

  /** "a" for the DBH-based branch biomass.  Array size is # total species.*/
  float *mp_fDBHBranchA;

  /** "b" for the DBH-based branch biomass.  Array size is # total species.*/
  float *mp_fDBHBranchB;

  /** "a" for the DBH-based bole biomass.  Array size is # total species.*/
  float *mp_fDBHBoleA;

  /** "b" for the DBH-based bole biomass.  Array size is # total species.*/
  float *mp_fDBHBoleB;

  /** "a" for the height-based leaf biomass.  Array size is # total species.*/
  float *mp_fHeightLeafA;

  /** "b" for the height-based leaf biomass.  Array size is # total species.*/
  float *mp_fHeightLeafB;

  /** "a" for the height-based bole biomass.  Array size is # total species.*/
  float *mp_fHeightBoleA;

  /** "b" for the height-based bole biomass.  Array size is # total species.*/
  float *mp_fHeightBoleB;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate volume.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  char *m_cQuery;

  /** Holds codes for DBH data member. Array size is total # species.*/
  short int *mp_iDbhCodes;

  /** Holds codes for Height data member. Array size is total # species.*/
  short int *mp_iHeightCodes;

  /** Holds codes for X data member. Array size is total # species.*/
  short int *mp_iXCodes;

  /** Holds codes for Y data member. Array size is total # species.*/
  short int *mp_iYCodes;

  /** Holds codes for stm_dmg data member. Array size is total # species.*/
  short int *mp_iStmDmgCodes;

  /** Holds data member codes for the "leaf_x" data members of the "Partitioned
  * Biomass" grid.  Array size is total # species.*/
  short int *mp_iLeafCodes;

  /** Holds data member codes for the "branch_x" data members of the
   * "Storm Killed Partitioned Biomass" grid.  Array size is total # species.*/
  short int *mp_iBranchCodes;

  /** Holds data member codes for the "bole_x" data members of the "Storm
   * Killed Partitioned Biomass" grid.  Array size is total # species.*/
  short int *mp_iBoleCodes;

  /** Holds data member codes for the "hleaf_x" data members of the "Storm
   * Killed Partitioned Biomass" grid.  Array size is total # species.*/
  short int *mp_iHLeafCodes;

  /** Holds data member codes for the "hbole_x" data members of the "Storm
   * Killed Partitioned Biomass" grid.  Array size is total # species.*/
  short int *mp_iHBoleCodes;

  /** If true, this is DBH-based biomass.  If false, height-based.*/
  bool m_bIsDbh;

  /**
  * Sets up the mp_iDbhCodes or mp_iHeightCodes, depending on the type of
  * biomass this is.
  * @param p_oPop Tree population object
  */
  void GetTreeCodes(clTreePopulation *p_oPop);

  /**
  * Reads values from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  * @param p_oPop Tree population object.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  * @param p_oPop Tree population object.
  */
  void FormatQueryString(clTreePopulation *p_oPop);

  /**
  * Sets up the "Storm Killed Paritioned Biomass" grid.  This ignores any maps.
  * @param p_oPop Tree population object.
  */
  void SetupGrid(clTreePopulation *p_oPop);
};
//---------------------------------------------------------------------------
#endif // StormKilledPartitionedBiomass_H
