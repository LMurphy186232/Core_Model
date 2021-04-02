//---------------------------------------------------------------------------
// CarbonValueCalculator
//---------------------------------------------------------------------------
#if !defined(CarbonValueCalculator_H)
  #define CarbonValueCalculator_H

#include "BehaviorBase.h"

class clGrid;

/**
* Carbon Value Calculator Version 1.0
*
* This behavior calculates carbon value per species.  The amount of carbon is
* given by the user as a percent of biomass for each species.  This behavior
* expects the Dimension Analysis behavior (clDimensionAnalysis) to calculate
* tree biomass.
*
* The user also supplies a price per metric ton of carbon.  This is multipled
* by the amount of carbon and added up into a total value of carbon per
* species.  The species carbon totals and carbon value totals are stored in
* a grid called "Carbon Value".
*
* Each tree gets a data member called "Carbon".  This contains the tree's
* own carbon value.
*
* This class's namestring and parameter call string are both
* "CarbonValueCalculator".
*
* This behavior may not be applied to seedlings.
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clCarbonValueCalculator : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clCarbonValueCalculator(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clCarbonValueCalculator();

  /**
  * Makes value calculations.  First, the values in the "Carbon Value" grid are
  * cleared.  Then a query is sent to the tree population to get all trees to
  * which this behavior is applied.  For each, the amount of biomass (in metric
  * tons) is retrieved, and the carbon amount and value calculated.  Then the
  * species values are totaled and placed in the "Carbon Value" grid.
  */
  void Action();

  /**
  * Does setup for this behavior.  Calls:
  * <ol>
  * <li>GetParameterFileData()</li>
  * <li>FormatQueryString()</li>
  * <li>SetupGrid()</li>
  * <li>GetBiomassCodes()</li>
  * <li>Action() so that the initial conditions value will be added</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Carbon" tree float data member.  The return codes are
  * captured in the mp_iTreeCarbonCodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * saplings, adults, and snags.
  */
  void RegisterTreeDataMembers();

  protected:
  /**
   * Grid holding total values for each species.  The grid name is
   * "Carbon Value".  The grid contains only 1 grid cell.  It has 2 times X
   * float data members, where X = the total number of species.  The data
   * member names are "carbon_x" (where "x" is the species number) for amount
   * of carbon, and "value_x" for value of carbon.
   */
  clGrid* mp_oValueGrid;

  /** Percent of biomass that is carbon.  In the parameter file, this is as a
   * percentage between 0 and 100.  We convert this upon reading to a
   * proportion between 0 and 1.  Array size is # total species.*/
  double *mp_fCPercentBiomass;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate volume.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  std::string m_sQuery;

  /**Holds data member codes for "Biomass" float data member (registered by
   * the clDimensionAnalysis class).  First array index is total # species,
   * second is number types (3 - sapling, adult, snag)*/
  short int **mp_iBiomassCodes;

  /**Holds data member codes for "Carbon" float data member.  First array
  * index is total # species, second is number types (3 - sapling, adult,
  * snag)*/
  short int **mp_iTreeCarbonCodes;

  /**
  * Holds data member codes for the "value_x" float data members of the
  * "Carbon Value" grid.  Array size is total # species.
  */
  short int *mp_iValueCodes;

  /**
  * Holds data member codes for the "carbon_x" float data members of the
  * "Carbon Value" grid.  Array size is total # species.
  */
  short int *mp_iCarbonCodes;

  /** Price per metric ton of carbon - currency unimportant*/
  double m_fPricePerTonCarbon;

  /**Total number of species.  For the destructor.*/
  short int m_iNumTotalSpecies;

  /**
  * Retrieves the "Biomass" float data member.  The return codes are captured
  * in the mp_iBiomassCodes array.
  * @throw modelErr if any code comes back -1 for a tree to which this behavior
  * is applied.
  */
  void GetBiomassCodes(clTreePopulation *p_oPop);

  /**
  * Reads values from the parameter file.
  * @param p_oDoc DOM tree of parsed input file.
  * @param p_oPop Tree population object.
  * @throw modelErr if the percentage of biomass that is carbon is less than 0
  * or greater than 100.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  * @param p_oPop Tree population object.
  */
  void FormatQueryString(clTreePopulation *p_oPop);

  /**
  * Sets up the "Carbon Value" grid.  This ignores any maps.
  */
  void SetupGrid();
};
//---------------------------------------------------------------------------
#endif // VolumeCalculator_H
