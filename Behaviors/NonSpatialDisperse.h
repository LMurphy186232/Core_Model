//---------------------------------------------------------------------------

#ifndef BathRecruitmentH
#define BathRecruitmentH
//---------------------------------------------------------------------------
#include "DisperseBase.h"



/**
 * Non-spatial disperse - Version 1.0
 *
 * This class performs non-spatial dispersal. There are two kinds. The first is
 * non-density dependent, or bath, dispersal.  This kind of dispersal
 * represents a constant seed rain which does not depend on the presence of
 * parent trees in the plot.  The other kind is density-dependent dispersal.
 * The number of seeds produced depends on the amount of basal area of
 * reproductively mature trees of that species in the plot.
 *
 * The mean number of seeds per square meter per year is calculated as
 * lambda = BA*m + b, where m is the slope of lambda, b is the intercept of
 * lambda, and BA is the amount of basal area in square meters per hectare.
 * Lambda is multiplied by number of square meters per seed grid cell and number
 * of years per timestep to arrive at the expected number of seeds per grid
 * cell.
 *
 * The slope of lambda controls density-dependent dispersal.  Setting it equal
 * to zero turns off density-dependent dispersal.  The intercept of lambda
 * controls non-density dependent dispersal, and same thing goes there.
 *
 * This behavior is not connected to any specific species-type combo, but is
 * connected to specific species.  There should be applyTo tags for this
 * behavior and a unique list of species will be compiled for those tags with
 * the type being ignored.
 *
 * This behavior's namestring and parameter file call string are both
 * "NonSpatialDisperse".
 *
 * Copyright 2011 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
 */
class clNonSpatialDispersal : virtual public clDisperseBase {

  public:

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clNonSpatialDispersal(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clNonSpatialDispersal();

  protected:

  /**Array of species with each one's dbh for reproduction*/
  double *mp_fDbhForReproduction;
  /**Slope of the lambda function - sized number of species.*/
  double *mp_fSlopeOfLambda;
  /**Intercept of the lambda function - sized number of species.*/
  double *mp_fInterceptOfLambda;
  /**Area of plot in hectares, for scaling basal area*/
  float m_fAreaOfPlotInHa;

  /**
   * Reads in parameter file values.
   * @param p_oDoc DOM tree of parsed parameter file.
   */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Computes the total basal area of all trees over reproductive
  * age of a given species.
  *
  * @param p_oPop Pointer to tree population object.
  * @param iSpecies Species for which to calculate basal area
  * @return Basal area in square meters per hectare.
  */
  float ComputeBasalArea(clTreePopulation *p_oPop, short int iSpecies);

  /**
   * Does dispersal.  For each species for which the slope of lambda is not
   * zero, ComputeBasalArea() is called.  Lambda is calculated and scaled to
   * the appropriate grid cell size and number of years. The resulting number
   * of seedlings is assigned to the grid cell.
   */
  void AddSeeds();

};
//---------------------------------------------------------------------------
#endif
