//---------------------------------------------------------------------------
// StochasticGapGrowth
//---------------------------------------------------------------------------
#if !defined(StochasticGapGrowth_H)
  #define StochasticGapGrowth_H

#include "BehaviorBase.h"

class clGrid;

/**
* Stochastic Gap Growth, Version 1.0
*
* This behavior takes a shortcut for simulating rapid growth in high-light,
* very competitive conditions.
*
* The behavior creates growth under gap conditions.  To recognize gap
* conditions, it relies on the "Gap Light" grid produced by the clGapLight
* class.  For any cell which has gap status, from the group of trees in that
* cell to which this behavior is applied, a tree is randomly chosen to be given
* whatever growth is necessary for it to reach the minimum DBH for adults.  No
* other trees in the cell grow at all.  In cells that have non-gap status, no
* trees at all grow.
*
* To randomly select a tree to grow, this behavior uses a grid called "Growth
* Counter".  The behavior counts up the number of eligible trees in each cell
* of "Growth Counter".  It then multiplies a random number by the number of
* trees, arriving at a target tree (i.e. number 34 out of 56 found).  Then this
* tree is given the amount of growth necessary for it to reach the minimum
* adult DBH.
*
* The namestring and parameter file call string for this behavior is
* "StochasticGapGrowth".  Note that this behavior does not descend from
* clGrowthBase.  There is nothing to be gained from that association, and it
* is easier for this behavior to perform all of its work at once rather than
* on one tree at a time.
*
* It is a fatal error if this behavior cannot find the "Gap Light" grid during
* setup.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clStochasticGapGrowth : virtual public clBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.  Sets the namestring.
  */
  clStochasticGapGrowth(clSimManager *p_oSimManager);

  /**
  * Destructor.  Frees memory.
  */
  ~clStochasticGapGrowth();

  /**
  * Performs growth calculations.  This function cycles through all of the
  * trees twice.  On the first pass, each tree is assigned to the appropriate
  * cell in "Growth Counter".  If that cell has gap status, the number in the
  * "Current" data member is incremented.  If the cell does not have gap
  * status, nothing happens.  Then, for each cell in "Growth Counter" that has
  * a positive count in "Current", a random number is multiplied by it and
  * that number is placed in "Target".  Then the trees are cycled through again
  * in exactly the same way.  For each cell, when the tree number in "Target"
  * has been reached, that tree is allowed to grow to adult status.
  */
  void Action();

  /**
  * Does the setup for this behavior.  This gets a pointer to the "Gap Light"
  * grid and sets up the "Growth Counter" grid.  Then it populates the
  * mp_bAppliedTo array.  Then it sets up the query for doing tree searches.
  * @param p_oDoc Parsed parameter file.
  * @throws modelErr if the "Gap Light" grid cannot be found.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  protected:

  /**Pointer to the "Gap Light" grid (this behavior does not create this
  * grid).  This grid is expected to have one bool data member called "Is
  * Gap".*/
  clGrid *mp_oGapLight;

  /**Grid called "Growth Counter", created by this behavior.  It has a grid
  * cell resolution matching "Gap Light".  It has two int data members,
  * "Current" (which holds the number of trees found in gap cells) and
  * "Target" (which holds the target number of the tree, out of the total in
  * "Current", which has been chosen to grow).  Any map of this grid entered
  * in the parameter file will be ignored, including just setting the grid
  * cell resolution.*/
  clGrid *mp_oGrowthCounter;

  /**Array for determining to which trees this behavior is applied.  The
  * array size is # total species by # total types.*/
  bool **mp_bAppliedTo;

  /**Query for finding trees to which this behavior applies. This will
  * instigate a species/type search for all the species and types to which this
  * behavior applies.*/
  char *m_cQuery;

  /**Keep a copy of this for the destructor for freeing memory*/
  int m_iNumSpecies;

  /**Data member code for the "Is Gap" bool data member of the "Gap Light"
  * grid.*/
  short int m_iIsGapCode;

  /**Data member code for the "Current" int data member of the "Growth Counter"
  * grid.*/
  short int m_iCurrentCode;

  /**Data member code for the "Target" int data member of the "Growth Counter"
  * grid.*/
  short int m_iTargetCode;
};
//---------------------------------------------------------------------------
#endif // StochasticGapGrowth_H
