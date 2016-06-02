//---------------------------------------------------------------------------
// DensitySeedSurvival
//---------------------------------------------------------------------------
#if !defined(DensitySeedSurvival_H)
  #define DensitySeedSurvival_H

#include "BehaviorBase.h"

class clGrid;


/**
* Density-Dependent Seed Survival - version 2.0
*
* This behavior reduces the number of seeds due to conspecific density-dependent
* effects. The "density" in question can be either other seeds, or trees in
* the neighborhood.
*
* The number of seeds this behavior leaves of a given species in a particular
* "Dispersed Seeds" grid cell is:
* @htmlonly
  <br><center>R<sub>sp</sub> = S<sub>sp</sub>*exp(-c*Den<sub>sp</sub><sup>&delta;</sup>)</center><br>
  @endhtmlonly
  where:</p>
* <ul>
* <li>R<sub>sp</sub> is the number of seedlings of a given species in the seed
* grid cell (competition is conspecific only)</li>
* <li>S<sub>sp</sub> is the potential number of seedlings of the species, i.e.
* the number of seeds dispersed to that grid cell.</li>
* <li>Den<sub>sp</sub> is either:
* <ul>
*   <li>The density of seed of the species per square meter in that grid cell;
*       or</li>
*   <li>The density of conspecific trees above a given height within a given
*       radius of the cell center, stems/m2.</li>
  </ul>
* <li>c is an input parameter - density-dependent slope</li>
* <li>@htmlonly &delta; @endhtmlonly is an input parameter - density-dependent
* steepness</li>
* </ul>
*
* Single seeds always survive, if using seeds-only density dependence.
*
* A fatal error is thrown if a disperse behavior is not also defined for the
* run.
*
* This class's namestring is "DensityDependentSeedSurvival". In the case of
* conspecific seed density, the parameter file call string is also
* "DensityDependentSeedSurvival"; in the case of conspecific tree neighbors,
* it's "ConspecificTreeDensityDependentSeedSurvival".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clDensitySeedSurvival : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

 public:

  /**
  * Constructor.
  *
  * @param p_oSimManager Sim Manager object.
  */
  clDensitySeedSurvival(clSimManager *p_oSimManager);

  /**
  * Destructor.
  */
  ~clDensitySeedSurvival();

  /**
  * Reads in values from the parameter file.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Captures the namestring passed to this behavior.  This is overridden from
  * clBehaviorBase so we can capture the namestring passed.  Since this class
  * can create multiple kinds of behaviors that function differently, this will
  * capture what kind of behavior this is supposed to be.
  *
  * @param sNameString Behavior's namestring.
  */
  void SetNameData(std::string sNameString);

  /**
  * Performs density-dependent seed survival by calling either ActionUseSeeds
  * or ActionUseTrees.
  */
  void Action();

  protected:

  /**Pointer to the "Dispersed Seeds" grid created by disperse behaviors.*/
  clGrid *mp_oSeedGrid;

  /**Density-dependence steepness parameter.  Array size is total number of
  * species.*/
  double *mp_fDensDepSteepness;

  /**Density-dependence slope parameter.  Array size is total number of
  * species.*/
  double *mp_fDensDepSlope;

  /**Minimum height for neighbor trees. Only used if m_bSeeds = FALSE. Array
   * size is total number of species.*/
  double *mp_fMinHeight;

  /**Data member codes for seed grid for number of seeds.  Array size is #
  * total species.*/
  short int *mp_iSeedGridCode;

  /**Area normally searched. If m_bSeeds = TRUE, this is the area of interior
   * cells of the "Dispersed Seeds" grid, in square meters. If m_bSeeds = FALSE,
   * this is the area of the circle with radius m_fSearchRadius. */
  float m_fNormalSearchArea;

  /**Area of the last row of "Dispersed Seeds" grid cells in the X direction,
  * in square meters.  This allows for grid cells that don't divide evenly
  * into plot lengths. Only used if m_bSeeds = TRUE.*/
  float m_fXEdgeCellArea;

  /**Area of the last row of "Dispersed Seeds" grid cells in the Y direction,
  * in square meters.  This allows for grid cells that don't divide evenly
  * into plot lengths. Only used if m_bSeeds = TRUE.*/
  float m_fYEdgeCellArea;

  /**Area of the last cell of "Dispersed Seeds" grid cell in both the X and Y
  * direction, in square meters.  This allows for grid cells that don't divide
  * evenly into plot lengths. Only used if m_bSeeds = TRUE.*/
  float m_fXYEdgeCellArea;

  /**Radius, in meters, for which to search for conspecific trees, starting at
   * the center of a grid cell. Only used if m_bSeeds = FALSE. */
  double m_fSearchRadius;

  /**If true, we are using the density of conspecific seeds; if false, we are
   * using conspecific trees.*/
  bool m_bSeeds;

  /**
  * Performs density-dependent seed survival when m_bSeeds = TRUE. For each grid
  * cell in the seed grid, for each species to which this behavior applies, the
  * number of seeds left is calculated by assessing the equation above. A random
  * round is used to take care of fractional parts of seeds.
  */
  void ActionUseSeeds();

  /**
  * Performs density-dependent seed survival when m_bSeeds = FALSE.
  */
  void ActionUseTrees();
};
//---------------------------------------------------------------------------
#endif // DensitySeedSurvival_H

