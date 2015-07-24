//---------------------------------------------------------------------------

#ifndef StochDoubleLogTempDepNeighDisperseH
#define StochDoubleLogTempDepNeighDisperseH
//---------------------------------------------------------------------------
#include "DisperseBase.h"

/**
 * Stochastic double logistic temperature dependent neighborhood disperse - Version 1.0
 *
 * This class disperses seeds.  The number of seeds is a function of the plot
 * annual mean temperature and the basal area of conspecific adults.
 *
 * If seeds are dispersed for a cell, the number of seeds per GRID CELL per year
 * is calculated as:
 *
 * Seeds = (a + fec * BAC) * corr
 *
 * where
 * <ul>
 * <li>a is a parameter</li>
 * <li>BAC is the basal area of conspecific adults within a specified radius,
 * in square meters</li>
 * <li>corr is the area of a cell divided by the analysis plot area, entered
 * as a parameter</li>
 * </ul>
 *
 * The "fec" term is fecundity, calculated as:
 *
 * fec = fb* (al + ((1-al)/(1+(bl/X)^cl))) *
 *           (ah + ((1-ah)/(1+(X/bh)^ch)))
 *
 * where X is the plot annual mean temperature IN K and fb, al, bl, cl, ah, bh,
 * and ch are parameters. The user has the option to turn off the temperature-
 * dependent portion of fecundity, in which case fec = fb.
 *
 * The number of seeds per cell can be used as-is or used as the mean in a
 * Poisson draw. This value is multiplied by number of years per timestep to
 * arrive at the number of seeds per grid cell.
 *
 * Colonization:
 * The above equation for seeds includes an intercept term, which means there
 * is the possibility for bath rain of species for which there are no adults. A
 * species for which there are adults will always disperse. A species for which
 * there are no adults must undergo a "colonization test". The cumulative
 * probability of colonization (PCa) of seedlings is:
 *
 * PCc = pa*(exp(-0.5*(((temp.C - pm)/pb)^2))))
 *
 * where temp.C is the plot annual mean temperature in C and pac, pbc, and pmc
 * are parameters.
 *
 * This is annualized with a time period parameter t:
 *
 * PCa = 1 – [(1 – PCc)^(1/t)]
 *
 * If a random draw is less than or equal to PCa, then seed input is done as if
 * there were adults present in the plot, on a quadrat by quadrat basis.
 *
 * This behavior is not connected to any specific species-type combo, but is
 * connected to specific species.  There should be applyTo tags for this
 * behavior and a unique list of species will be compiled for those tags with
 * the type being ignored.
 *
 * This behavior's namestring and parameter file call string are both
 * "StochDoubleLogTempDepNeighDisperse".
 *
 * Copyright 2014 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>February 20, 2014 - Created (LEM)
 */
class clStochDoubleLogTempDepNeighDisperse : virtual public clDisperseBase {

  public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clStochDoubleLogTempDepNeighDisperse(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clStochDoubleLogTempDepNeighDisperse();

  protected:

  /**Colonization a - sized number of species.*/
  float *mp_fPA;

  /**Colonization b - sized number of species.*/
  float *mp_fPB;

  /**Colonization m - sized number of species.*/
  float *mp_fPM;

  /** Temperature dependence of fecundity al - sized number of species.*/
  float *mp_fAl;

  /** Temperature dependence of fecundity bl - sized number of species.*/
  float *mp_fBl;

  /** Temperature dependence of fecundity cl - sized number of species.*/
  float *mp_fCl;

  /** Temperature dependence of fecundity ah - sized number of species.*/
  float *mp_fAh;

  /** Temperature dependence of fecundity bh - sized number of species.*/
  float *mp_fBh;

  /** Temperature dependence of fecundity ch - sized number of species.*/
  float *mp_fCh;

  /** A parameter - sized number of species.*/
  float *mp_fA;

  /** B parameter - sized number of species.*/
  float *mp_fB;

  /** Neighborhood search radius.*/
  float m_fRadius;

  /** Minimum sapling height. For doing neighbor searches.*/
  float m_fMinSaplingHeight;

  /** Original analysis plot size, square meters.*/
  float m_fAnalysisPlotSize;

  /** T, in years, to go from cumulative to annualized probability*/
  float m_fAnnualizePeriod;

  /** If true, seed number is deterministic. If false, Poisson-distributed. */
  bool m_bDeterministic;

  /** If true, use temperature-dependent portion of fecundity. */
  bool m_bFecTempDep;

  /**
   * Reads in parameter file values.
   * @param p_oDoc DOM tree of parsed parameter file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
   * Does dispersal.
   */
  void AddSeeds();

};
//---------------------------------------------------------------------------
#endif
