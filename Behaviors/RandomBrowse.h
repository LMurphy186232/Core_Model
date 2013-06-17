//---------------------------------------------------------------------------
// clRandomBrowse
//---------------------------------------------------------------------------
#if !defined(RandomBrowse_H)
  #define RandomBrowse_H

#include "BehaviorBase.h"

/**
* Random Browse Version 1.0
*
* This class randomly chooses trees to be browsed according to a species
* specific probability of browse. The same browse probability can be used every
* time, or it can be the mean in a normal draw each timestep for each species.
*
* This behavior adds a bool data member called "Browsed" to trees that holds
* whether (true) or not (false) the tree has been browsed.
*
* This class's namestring and parameter call string are both "RandomBrowse".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clRandomBrowse : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clRandomBrowse(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clRandomBrowse();

  /**
  * Decides who is browsed. A query is sent to the tree population to get all
  * trees to which this behavior is applied. For each, a random number is
  * compared to that species' browse probability. The result is placed in the
  * "Browsed" bool tree data member.
  */
  void Action();

  /**
  * Does setup for this behavior. This gets parameters and formats the query
  * string for getting trees from the tree population.
  * @param p_oDoc DOM tree of parsed input file.
  * @throw modelErr if any probability is not between 0 and 1.
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "Browsed" bool data member.  The return codes are captured
  * in the mp_iBrowsedCodes array.
  */
  void RegisterTreeDataMembers();

  protected:
  /** Probability of browse (or mean prob). Array size is total number of
   * species.*/
  float *mp_fBrowseProb;

  /** Standard deviation of browse probability, if using a normal draw each
   * timestep. Array size is total number of species.*/
  float *mp_fBrowseStdDev;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to calculate volume.  This will instigate a species/type search for
  * all the species and types to which this behavior applies.*/
  char *m_cQuery;

  /**Holds data member codes for "Browsed" bool data member.  First array index
  * is # species, second is number types.*/
  short int **mp_iBrowsedCodes;

  /**For the destructor.*/
  short int m_iNumSpecies;

  /**What distribution function to use to randomize the probabilities each
   * timestep - currently only deterministic and normal supported */
  pdf m_iPDF;

};
//---------------------------------------------------------------------------
#endif // RandomBrowse_H
