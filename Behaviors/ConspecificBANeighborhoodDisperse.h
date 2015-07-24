//---------------------------------------------------------------------------

#ifndef ConspecificBANeighborhoodDisperseH
#define ConspecificBANeighborhoodDisperseH
//---------------------------------------------------------------------------
#include "DisperseBase.h"



/**
 * Conspecific BA dependent neighborhood disperse - Version 1.0
 *
 * This class disperses seeds. The number of seeds is a function of the basal
 * area of conspecific adults.
 *
 * The number of seeds per square meter per year is calculated as:
 *
 * <center><i>Seeds = a + b*BAC</i></center>
 *
 *  where a and b are parameters, and BAC is the basal area of
 * conspecific adults within a specified radius, in square meters.
 *
 * The number of seeds per square meter is multiplied by number
 * of square meters per seed grid cell and number of years per timestep to
 * arrive at the number of seeds per grid cell.
 *
 * This behavior is not connected to any specific species-type combo, but is
 * connected to specific species.  There should be applyTo tags for this
 * behavior and a unique list of species will be compiled for those tags with
 * the type being ignored.
 *
 * This behavior's namestring and parameter file call string are both
 * "ConspecificBANeighborhoodDisperse".
 *
 * Copyright 2013 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>November 7, 2013 - Created (LEM)
 */
class clConspecificBANeighborhoodDisperse : virtual public clDisperseBase {

  public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clConspecificBANeighborhoodDisperse(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clConspecificBANeighborhoodDisperse();

  protected:

  /**A parameter - sized number of species.*/
  float *mp_fA;

  /**B parameter - sized number of species.*/
  float *mp_fB;

  /**Neighborhood search radius.*/
  float m_fRadius;

  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;


  /**
   * Reads in parameter file values.
   * @param p_oDoc DOM tree of parsed parameter file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Does dispersal. For each grid cell, the neighborhood basal area of each
   * species is calculated. The number of seeds per square meter is calculated
   * and scaled to the appropriate grid cell size and number of years. The
   * resulting number of seeds is assigned to the grid cell.
   */
  void AddSeeds();

};
//---------------------------------------------------------------------------
#endif
