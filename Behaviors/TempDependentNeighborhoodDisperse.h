//---------------------------------------------------------------------------

#ifndef TempDependentNeighborhoodDisperseH
#define TempDependentNeighborhoodDisperseH
//---------------------------------------------------------------------------
#include "DisperseBase.h"



/**
 * Temperature dependent neighborhood disperse - Version 1.0
 *
 * This class disperses seeds.  The number of seeds is a function of the plot
 * annual mean temperature and the basal area of conspecific adults.
 *
 * The number of seeds per square meter per year is calculated as:
 * Seeds = a + fec*BAC, where a is a parameter, and BAC is the basal area of
 * conspecific adults within a specified radius, in square meters.
 *
 * The "fec" term is fecundity, calculated as:
 * fec = fb*exp(-0.5*(((X-fm)/fn)^2)), where X is the plot annual mean
 * temperature and fb, fm, and fn are parameters.
 *
 * The number of seeds per square meter is multiplied by number
 * of square meters per seed grid cell and number of years per timestep to
 * arrive at the number of seeds per grid cell.
 *
 * The above equation includes an intercept term, which means there is the
 * possibility for bath rain of species for which there are no adults. A species
 * for which there are adults will always disperse. A species for which there
 * are no adults must undergo a "presence test". The site favorability for a
 * species is given by:
 * p = exp(-0.5*((X-pm)/pb)^2), where X is the plot annual mean
 * temperature and pb and pm are parameters. This function is bounded between 0
 * and 1. There is a parameter which is the presence threshold; if the result of
 * the function is above this value, the species is allowed to disperse.
 *
 * This behavior is not connected to any specific species-type combo, but is
 * connected to specific species.  There should be applyTo tags for this
 * behavior and a unique list of species will be compiled for those tags with
 * the type being ignored.
 *
 * This behavior's namestring and parameter file call string are both
 * "TemperatureDependentNeighborhoodDisperse".
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */
class clTempDependentNeighborhoodDisperse : virtual public clDisperseBase {

  public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clTempDependentNeighborhoodDisperse(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clTempDependentNeighborhoodDisperse();

  protected:

  /**Fecundity M - sized number of species.*/
  double *mp_fFecM;
  /**Fecundity N - sized number of species.*/
  double *mp_fFecN;
  /**Presence M - sized number of species.*/
  double *mp_fPresM;
  /**Presence B - sized number of species.*/
  double *mp_fPresB;
  /**Presence threshold - sized number of species.*/
  double *mp_fThreshold;
  /**A parameter - sized number of species.*/
  double *mp_fA;
  /**B parameter - sized number of species.*/
  double *mp_fB;
  /**Neighborhood search radius.*/
  double m_fRadius;
  /**Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;
  /**Whether or not presence testing is required for this run.  If all values in
   * the threshold parameter are set to zero, it is not required.*/
  bool m_bDoPresenceTesting;

  /**
   * Reads in parameter file values.
   * @param p_oDoc DOM tree of parsed parameter file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Does dispersal. This starts by establishing whether there are adults of all
   * species in the plot, if presence testing is required. If there any species
   * for which there are no adults, the presence test function is calculated. If
   * a species fails the test, its intercept for this time step is set to zero.
   * Then, mean annual temperature is retrieved from the plot object and the
   * fecundity term calculated for each species. For each grid cell, the
   * neighborhood basal area of each species is calculated. The number of seeds
   * per square meter is calculated and scaled to the appropriate grid cell size
   * and number of years. The resulting number of seeds is assigned to the grid
   * cell.
   */
  void AddSeeds();

};
//---------------------------------------------------------------------------
#endif
