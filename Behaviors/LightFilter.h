//---------------------------------------------------------------------------
#ifndef LightFilterH
#define LightFilterH
//---------------------------------------------------------------------------
#include "BehaviorBase.h"



/**
* Implements a light filter according to Beer's Law.  Imagine a fog that hangs
* out on the forest floor and ends abruptly at a certain height.  All trees
* shorter than the top of the fog layer will have their light attenuated but
* not blocked completely.  The closer they get to the top of the fog the more
* light is let in.  The amount of light which actually gets through is
* calculated according to Beer's Law, where %transmission = e<sup>-az</sup>,
* where a = light extinction coefficient of the filter (in 1/meter units,
* somehow) and z = thickness of the filter, in meters (which is the distance
* from the light point to the top of the filter).  This filter behavior can be
* used to, for instance, replicate the effects of an herbaceous layer in
* reducing light to young seedlings.
*
* The height of the filter is randomized slightly each time the
* thickness of the filter over the light point is calculated to introduce a
* stochastic element.
*
* Trees can be given a respite from the effects of the filter.  This behavior
* will create an integer tree data member for a counter for years of respite
* (called "lf_count").  This behavior doesn't set the value in the counter;
* that is left to another behavior, such as recruitment.  Once a counter is set,
* this behavior will decrement it until it is zero, leaving the tree alone for
* the duration.
*
* Trees are given a rooting height (integer tree data member, in mm, called
* "z", added by this behavior).  This value is added to their existing height to
* get their effective height, which is what will be applied when determining
* the thickness of the filter overhead.  Again, this behavior does not set this
* height but will use it if another behavior sets it.
*
* This behavior DOES NOT ACTUALLY CALCULATE LIGHT LEVELS.  It will check all
* tree types to which it is applied to ensure that a float data member called
* "Light" has been registerd.  This data member is assumed to contain a GLI-like
* value, which is reduced by the percent transmission calculated.  A fatal
* error will be thrown if this data member has not been registered during
* setup.
*
* This behavior only affects tree types and species to which it is applied in
* the behavior list of the parameter file.  It will ignore all other trees,
* even if they are short enough to be beneath the filter level.
*
* Units are kept in integer number of millimeters, where possible, to increase
* the amount of integer math (fast) and decrease the amount of float math
* (slow).
*
* The namestring for this behavior is "LightFilter".  Call this behavior in
* the parameter file with the string "LightFilter".
*
* Copyright 2004 Charles D. Canham
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clLightFilter : virtual public clBehaviorBase {

public:

  /**
   * Constructor.
   * @param p_oSimManager clSimManager object.
   */
  clLightFilter(clSimManager *p_oSimManager);

  /**
   * Destructor.  Deletes arrays.
   */
  ~clLightFilter();

  /**
   * Reads values from parameter file and registers our data members.
   * @param p_oDoc DOM tree of parsed input file.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Applies the light filtering to all tree type/species combos to which this
   * behavior is applied.  If the value of lf_count in a tree is greater than
   * zero, it is decremented by the number of years per timestep and no further
   * action is taken.  Otherwise, a tree's z and height values are added
   * together and Beer's Law is calculated as described above.  The value in
   * "Light" is multiplied by the resulting transmission percentage.
   *
   * This does an "all" tree search.  I figured it was simplest, since we still
   * have to validate species and type even if we do a species/type search.  The
   * presence of a return code for "z" that is greater than -1 will be used as
   * a simple proxy for assignment to species/type combo.
   */
  void Action();

protected:

  /**The light extinction coefficient of the filter.  In the parameter file
  * the units are 1/m but it will be stored here as 1/mm by dividing by 1000*/
  double m_fLightExtinctionCoefficient;

  /**The height of the filter, in mm; will be converted from the value in the
  parameter file in meters*/
  int m_iFilterHeight;

  /**Holds data member codes for "Light" - first array index is
  * species number, second is type number*/
  short int **mp_iLightCodes;

  /**Holds data member codes for "lf_count" - first array index is
  species number, second is type number*/
  short int **mp_iCounterCodes;

  /**Holds data member codes for "z" - first array index is species
  number, second is type number*/
  short int **mp_iZCodes;

  /**Holds data member codes for tree height - first array index is
  * species number, second is type number*/
  short int **mp_iHeightCodes;

  /**For the sake of the destructor - the number of species*/
  short int m_iNumSpecies;
};
//---------------------------------------------------------------------------
#endif
