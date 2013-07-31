//---------------------------------------------------------------------------
// DimensionAnalysis
//---------------------------------------------------------------------------
#if !defined(DimensionAnalysis_H)
  #define DimensionAnalysis_H

#include "BehaviorBase.h"

/**
* Tree Dimension Analysis Version 2.0
*
* This class calculates above-ground tree biomass.  There are 9 possible
* equations.  The equations, and all terminology, ID numbers, and parameter
* names, are taken directly from:
*
* Jenkins, J.C., D.C. Chojnacky, L.S Heath, and R.A. Birdsey.  2004.
* Comprehensive Database of Diameter-based Biomass Regressions for North
* American Tree Species.  United States Department of Agriculture, Forest
* Service General Technical Report NE-319.  http://www.nrs.fs.fed.us/
*
* The list of equations is in Table 6.  The equations are as follows:
* <table border=1 cellpadding=5>
* <tr><th>ID</th><th>Equation</th></tr>
* <tr><td>1</td><td>log10 biomass = a + b * (log10(dia^c))</td></tr>
* <tr><td>2</td><td>ln biomass = a + b * dia + c * (ln(dia^d))</td></tr>
* <tr><td>3</td><td>ln biomass = a + b * ln(dia) + c * (d + (e * ln(dia)))</td></tr>
* <tr><td>4</td><td>biomass = a + b * dia + c * (dia ^ d)</td></tr>
* <tr><td>5</td><td>biomass = a + (b * dia) + c * (dia ^ 2) + d * (dia ^ 3)</td></tr>
* <tr><td>6</td><td>biomass = a * (exp(b + (c * ln(dia)) + (d * dia)))</td></tr>
* <tr><td>7</td><td>biomass = a + ((b * (dia ^ c))/((dia ^ c) + d))</td></tr>
* <tr><td>8</td><td>log100 biomass = a + (b * log10(dia))</td></tr>
* <tr><td>9</td><td>ln biomass = ln(a) + (b * ln(dia))</td></tr>
* </table>
*
* There are lots of different parameters published in the literature, and the
* units are all over the place.  We want users to be able to enter literature
* values as published.  Therefore, the user specifies the units of DBH that
* work with the parameters they're entering:  either cm, mm, or inches.  Then
* they specify the units of biomass that the parameters they entered should
* produce:  g, k, or lbs.  This behavior does all necessary conversion so that
* in the end it reports biomass values in metric tons (Mg = 10<sup> 3</sup>
* kg).
*
* Users have the option of specifying a correction factor, which is sometimes
* used by the ln biomass or log10 biomass equations.  This value is multiplied
* by the result before converting to metric tons.
*
* The value for "dia" can vary as well. It can mean either DBH or
* DBH<sup>2</sup> (other possibilities are given in the paper, but I am making
* the decision to not support them at this time.)
*
* The biomass value is stored in a float tree data member called "Biomass" that
* this behavior adds.
*
* This class's namestring and parameter call string are both
* "DimensionAnalysis".
*
* This behavior may not be applied to seedlings.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clDimensionAnalysis : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clDimensionAnalysis(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clDimensionAnalysis();

  /**
  * Makes biomass calculations.  A query is sent to the tree population to get
  * all trees to which this behavior is applied.  For each, the biomass is
  * calculated and the value is placed in the "Biomass" float tree data member.
  *
  * This puts all the equations in an "if" statement.  I think this is as
  * fast a way as any to do this.
  */
  void Action();

  /**
  * Does setup for this behavior.  Steps:
  * <ol>
  * <li>Reads values from the parameter file and validates them.</li>
  * <li>Populates the units converter arrays.</li>
  * <li>Formats the query string, which is used to do tree searches.</li>
  * <li>Calls Action() so that the initial conditions volume will be added</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  * @throw modelErr if:
  * <ul>
  * <li>Any value for a is less than 0.</li>
  * <li>Any value for c is greater than 34 (arbitrary cutoff to avoid float
  * overflows)</li>
  * </ul>
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Biomass" float data member.  The return codes are captured
  * in the mp_iBiomassCodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * saplings, adults, and snags.
  */
  void RegisterTreeDataMembers();

  /**Units of DBH that correspond to the parameters entered by the user.*/
  enum DbhUnits {mm, /**<Millimeters*/
                 cm, /**<Centimeters*/
                 in  /**<Inches*/
                 };

  /**Units of biomass that correspond to the parameters entered by the user.*/
  enum BiomassUnits {g, /**<Gram*/
                     kg, /**<Kilogram*/
                     lb  /**<Pound*/
                     };

  /**Meaning of "dia".*/
  enum WhatDia {DBH, /**<DBH*/
                DBH2 /**<DBH squared*/
  };

  protected:
  /** <i>a</i> in a biomass equation.  Array size is # species to which this
  * behavior applies.*/
  float *mp_fA;

  /** <i>b</i> in a biomass equation.  Array size is # species to which this
  * behavior applies.*/
  float *mp_fB;

  /** <i>c</i> in a biomass equation.  Array size is # species to which this
  * behavior applies.*/
  float *mp_fC;

  /** <i>d</i> in a biomass equation.  Array size is # species to which this
  * behavior applies.*/
  float *mp_fD;

  /** <i>e</i> in a biomass equation.  Array size is # species to which this
  * behavior applies.*/
  float *mp_fE;

  /** Correction factor for a biomass equation.  This value, if desired by the
  * user, is multiplied by the finished biomass.  Array size is # species to
  * which this behavior applies.*/
  float *mp_fCorrectionFactor;

  /** Converts DBH values to the appropriate units.  Array size is # species to
  * which this behavior applies.*/
  float *mp_fDbhConverter;

  /** Converts biomass values to metric tons.  Array size is # species to
  * which this behavior applies.*/
  float *mp_fBiomassConverter;

  /** Equation ID, as a value from 1 to 9. Array size is # species to which
  * this behavior applies.*/
  int *mp_iEquationID;

  /** Meaning of "dia". Array size is # species to which this
   * behavior applies.*/
  int *mp_iWhatDia;

  /** Helps with array access.  Array size is number of total species.*/
  int *mp_iIndexes;

  /** Whether or not to use a correction factor.  Array size is # species to
  * which this behavior applies.*/
  bool *mp_bUseCorrectionFactor;

  /** Whether or not to convert DBH (i.e. whether or not DBH is supposed to be
  * in cm).  Array size is # species to which this behavior applies.*/
  bool *mp_bConvertDBH;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate biomass.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  std::string m_sQuery;

  /**Holds data member codes for "Biomass" float data member.  First array
  * index is # species to which this behavior applies, second is number types
  * (3 - sapling, adult, snag)*/
  short int **mp_iBiomassCodes;
};
//---------------------------------------------------------------------------

#endif // DimensionAnalysis_H
