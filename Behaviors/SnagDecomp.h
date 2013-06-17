//---------------------------------------------------------------------------
// SnagDecomp
//---------------------------------------------------------------------------
#if !defined(SnagDecomp_H)
  #define SnagDecomp_H

#include "BehaviorBase.h"
#include "Grid.h"

class clGrid;


/**
* Snag Decay Class Dynamics Version 2.0
*
* This class simulates the dynamics of standing dead trees as they pass through
* a series of decay classes and eventually fall.
*
* Trees that die from natural causes in the timestep are run through a tree fall
* model to determine whether they become a snag, or fall and are removed from
* tree population.  Snags created in previous timesteps are run through a
* separate fall model to determine whether they remain standing or are removed.
* Snags remaining standing are run through a decay class transition matrix
* (decay class is a parameter of the snag fall model).
*
* This behavior adds one integer data member called "SnagDecayClass"
* representing snag decay class.  There are five decay classes, numbered 1-5.
* Parameter values can be set to utilize fewer classes if desired.
*
* This behavior adds two float data members called "NewBreakHeight" and
* "SnagOldBreakHeight", both of which represent the height at which a snag has
* broken.  This is held by NewBreakHeight if the breakage occurred this
* timestep, or SnagOldBreakHeight otherwise.  If the snag is unbroken or did
* not break at the appropriate time, the value of these members is -1.0.
*
* This behavior adds one bool data member called "Fall".  This represents
* whether or not a tree that has died this timestep has fallen (true), or
* remains standing as a snag (false).
*
* Parameters correspond to the tree fall and snag fall models in Vanderwel et
* al. 2006 (Can. J. For. Res. 36: 2769-2779), and to a transition matrix
* representing the probability of undergoing a transition from a given decay
* class to another (or the same one) during the timestep.  All transition
* matrix values must be non-negative and the sum of each column should add up
* to 1.
*
* All parameters are based on a five-year timestep; the behavior will
* adjust fall probabilities for other timestep lengths if necessary.
*
* This class's namestring and parameter call string are both
* "SnagDecayClassDynamics".
*
* This behavior can be applied to adults and snags.  It should run after
* mortality and before substrate and dead tree remover behaviors are applied in
* each timestep.
*
* Copyright 2011 Charles D. Canham.
* @author Mark Vanderwel
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clSnagDecomp : virtual public clBehaviorBase {
  public:

  /**
  * Constructor.
  * @param p_oSimManager clSimManager object.
  */
  clSnagDecomp(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes arrays.
  */
  ~clSnagDecomp();

  /**
  * Runs snags through the fall models and decay class transition matrix.  A
  * query is sent to the tree population to get all snags to which this
  * behavior is applied.  For each that is still standing, the new decay class
  * is calculated and the value is placed in the "SnagDecayClass" int tree data
  * member.  Snags that fall are removed by calling clTreePopulation::KillTree()
  */

  void Action();


  /**
   * Calculates live and cut basal area on a grid.  Grid cell size is a multiple
   * of the tree population grid that approximates 20 x 20 m.  Cut basal area is
   * calculated if there was harvesting in the current timestep (behavior
   * Disturbance). These values are later used in the tree and snag fall models.
	*/
 void CalcGridValues();


  /**
  * Does setup for this behavior.  Steps:
  * <ol>
  * <li>Reads values from the parameter file and validates them.</li>
  * <li>Formats the query string, which is used to do tree searches</li>
  * </ol>
  * @param p_oDoc DOM tree of parsed input file.
  * @throw modelErr if:
  * <ul>
  * <li>Any value in the transition matrix is less than 0.</li>
  * <li>The sum of values for each transition matrix column do not add up to 1.</li>
  * </ul>
  */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
  * Registers the "SnagDecayClass" int data member.  The return codes are
  * captured in the mp_iSnagDCCodes array.
  * @throw modelErr if this behavior is being applied to any tree type except
  * snags.
  */
  void RegisterTreeDataMembers();

  protected:
  /**Species-specific matrices of transition probabilities.  Array size is
  * 6 x 6.*/
  float mp_fSnagTransProb[6][6];

   /**Parameters used in treefall and snag fall models*/
	float m_fTreefallAlpha;
	float *mp_fTreefallBeta;
	float m_fTreefallDelta;
	float m_fTreefallTheta;
	float m_fTreefallIota;
	float m_fTreefallLamda;

	float m_fSnagfallAlpha;
	float * mp_fSnagfallBeta;
	float * mp_fSnagfallGamma;
	float m_fSnagfallZeta;
	float m_fSnagfallEta;
	float m_fSnagfallKappa;

	/**The minimum snag break height.  Actually break height is a height between
	 * min and max drawn from a uniform distribution.*/
	float m_fMinBreakHeight;
  /**The maximum snag break height.*/
	float m_fMaxBreakHeight;

  /**String to pass to clTreePopulation::Find() in order to get the trees for
  * which to run snag dynamics.  This will instigate a species search for all
  * the species to which this behavior applies.*/
  char *m_cQuery;

  /**Grid to hold live and cut basal area in local cells*/
  clGrid * mp_oBasalAreaGrid;


  /*data member codes for grids*/
  short int m_iLiveBACode;
  short int m_iCutBACode;

  /*size of BA grid cells*/
  float m_fGridCellLength;



  /**Holds relevant data member codes.  First array index is # species and
   * second, if present, is type (adult or snag).*/
  short int ** mp_iSnagDCCode;
  short int ** mp_iNewBreakCode;
  short int ** mp_iDeadCodes;
  short int * mp_iAdultFallCode;
  short int * mp_iOldBreakCode;

  short int * mp_iIndexes;

  /**Keep a copy of this for the destructor*/
  int m_iNumSpecies;
};
//---------------------------------------------------------------------------

#endif // SnagDecomp_H
