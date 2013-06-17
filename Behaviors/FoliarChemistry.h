//---------------------------------------------------------------------------
// FoliarChemistry.h
//---------------------------------------------------------------------------
#if !defined(FoliarChemistry_H)
  #define FoliarChemistry_H

#include "BehaviorBase.h"

class clGrid;

/**
* Foliar Chemistry Calculator Version 1.0
*
* This behavior calculates foliar chemistry, partitioned by component. The
* components are N, P, SLA, lignins, tannins, phenolics, fiber, and cellulose.
*
* Foliar dry weight is calculated as a * DBH ^ b. For each of the components,
* the foliar dry weight is multiplied by the concentration for that component.
* Any negative values are set to zero. Values are in kilograms.
*
* The values are collected into a grid called "Foliar Chemistry".
*
* This class's namestring and parameter file call string is "FoliarChemistry."
*
* This behavior may not be applied to seedlings.
*
* <br>Edit history:
* <br>-----------------
* <br>December 2, 2008 - Created (LEM)
* <br>February 13, 2009 - Corrected component calculation (LEM)
* <br>April 6, 2009 - Changed grid units from Mg to kg (LEM)
*/
class clFoliarChemistry : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clFoliarChemistry(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clFoliarChemistry();

  /**
  * Makes value calculations.  First, the values in the "Foliar Chemistry"
  * grid are cleared.  Then a query is sent to the tree population to get all
  * trees to which this behavior is applied.  For each, the amount of each
  * component is calculated, and the species values are totaled and
  * placed in the "Foliar Chemistry" grid.
  */
  void Action();

  /**
  * Does setup for this behavior. Calls:
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

  protected:

  /** Grid holding total values for each species.  The grid name is
   * "Foliar Chemistry".  It has 5 times X float data members, where X = the
   * total number of species.  The data member names are "N_x", "P_x", "SLA_x",
   * "lignin_x", "fiber_x", "cellulose_x", "tannins_x", and "phenolics_x", where
   * "x" is the species number.*/
  clGrid* mp_oGrid;

  /** "a" for foliar dry weight. Array size is # total species.*/
  float *mp_fA;

  /** "b" for foliar dry weight. Array size is # total species.*/
  float *mp_fB;

  /** Concentration for N.  Array size is # total species.*/
  float *mp_fN;

  /** Concentration for P.  Array size is # total species.*/
  float *mp_fP;

  /** Concentration for lignin.  Array size is # total species.*/
  float *mp_fLignin;

  /** Concentration for fiber.  Array size is # total species.*/
  float *mp_fFiber;

  /** Concentration for cellulose.  Array size is # total species.*/
  float *mp_fCellulose;

  /** Concentration for tannins.  Array size is # total species.*/
  float *mp_fTannins;

  /** Concentration for phenolics.  Array size is # total species.*/
  float *mp_fPhenolics;

  /** Concentration for SLA.  Array size is # total species.*/
  float *mp_fSLA;


  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate volume.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  char *m_cQuery;

  /** Holds codes for DBH data member.  First array index is total # species,
   * second is number types (3 - sapling, adult, snag).*/
  short int **mp_iDbhCodes;

  /** Holds codes for X data member.  First array index is total # species,
   * second is number types (3 - sapling, adult, snag).*/
  short int **mp_iXCodes;

  /** Holds codes for Y data member.  First array index is total # species,
   * second is number types (3 - sapling, adult, snag).*/
  short int **mp_iYCodes;

  /** Whether this behavior applies to each kind of tree.  First array index is
   * total # species, second is number types (3 - sapling, adult, snag).*/
  bool **mp_bAppliesTo;

  /** Holds data member codes for the "N_x" data members of the "Foliar
   * Chemistry" grid.  Array size is total # species.*/
  short int *mp_iNCodes;

  /** Holds data member codes for the "P_x" data members of the "Foliar
   * Chemistry" grid.  Array size is total # species.*/
  short int *mp_iPCodes;

  /** Holds data member codes for the "SLA_x" data members of the "Foliar
   * Chemistry" grid.  Array size is total # species.*/
  short int *mp_iSLACodes;

  /** Holds data member codes for the "lignin_x" data members of the
   * "Foliar Chemistry" grid.  Array size is total # species.*/
  short int *mp_iLigninCodes;

  /** Holds data member codes for the "fiber_x" data members of the "Foliar
   * Chemistry" grid.  Array size is total # species.*/
  short int *mp_iFiberCodes;

  /** Holds data member codes for the "cellulose_x" data members of the
   * "Foliar Chemistry" grid.  Array size is total # species.*/
  short int *mp_iCelluloseCodes;

  /** Holds data member codes for the "tannins_x" data members of the
   * "Foliar Chemistry" grid.  Array size is total # species.*/
  short int *mp_iTanninsCodes;

  /** Holds data member codes for the "phenolics_x" data members of the
   * "Foliar Chemistry" grid.  Array size is total # species.*/
  short int *mp_iPhenolicsCodes;

  /** Total number of species.  For the destructor.*/
  short int m_iNumTotalSpecies;

  /**
  * Sets up the mp_bWhichAppliesTo array with the flags for each species and
  * tree type and whether this behavior applies to it.
  */
  void GetAppliesTo();

  /**
  * Sets up the mp_iDbhCodes.
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
  * Sets up the "Foliar Chemistry" grid.  This ignores any maps.
  */
  void SetupGrid();
};
//---------------------------------------------------------------------------
#endif // FoliarChemistry_H
