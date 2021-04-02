//---------------------------------------------------------------------------
// GapLight
//---------------------------------------------------------------------------
#if !defined(GapLight_H)
  #define GapLight_H

#include "LightBase.h"


class clGrid;
class clTreePopulation;

/**
* Gap Light - Version 1.0
*
* This behavior simplifies GLI calculations by treating GLI as binary:  either
* an area is in gap, and GLI = 100% (full sun), or an area is not in gap, and
* GLI = 0% (no sun).
*
* This behavior uses a grid object, called "Gap Light", to help it assign GLI
* values to the trees to which it is assigned.  The presence of an adult tree
* in a cell of "Gap Light" causes that cell's status to be non-gap.  The
* absence of any adult trees in a cell causes it to be gap.  For all trees to
* which this behavior is assigned, if they are in a gap cell, their GLI is set
* to 100%; otherwise it is 0%.
*
* The namestring is "gaplightshell".  The parameter file call string is
* "GapLight".
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clGapLight : public clLightBase {

 public:

  /**
  * Constructor.  Sets the namestring.
  * @param p_oSimManager Sim Manager object.
  */
  clGapLight(clSimManager *p_oSimManager);

  //~clGapLight(); //use default destructor

  /**
  * Sets up the "Gap light" grid.
  * @param p_oDoc DOM tree of parsed input file (not used).
  * @throws modelErr if the grid is incorrectly set up in the parameter file.
  */
  void DoShellSetup(xercesc::DOMDocument *p_oDoc);

  /**
  * Provides a GLI value for a tree.  This starts by determining if the gap
  * status has been updated for the timestep (is m_bCalcedGaps = false); if not,
  * then it calls CalculateGapStatus.  If the tree is in a gap cell with no
  * adult trees present, the value is 100; otherwise it is 0.
  * @param p_oTree Tree for which to get GLI.
  * @param p_oPop Tree population object.
  * @return GLI as a percentage of full sun, either 0 and 100.
  */
  float CalcLightValue(clTree *p_oTree, clTreePopulation *p_oPop);

  /**
  * Sets m_bCalcedGaps = false.
  */
  void TimestepCleanup() {m_bCalcedGaps = false;};

  protected:

  /**
  * Updates the gap status of the "Gap Light" grid.  It asks the tree
  * population for all parent trees; for each tree, the grid cell in which it
  * is is set to gap.
  * @param p_oPop Tree population object.
  */
  void CalculateGapStatus(clTreePopulation *p_oPop);

  /**Grid object which holds the gap values, called "Gap Light".  It has
  * one bool data member called "Is Gap" for holding the gap status, either
  * TRUE (if there are adults in that cell) or FALSE (if there are not).
  *
  * A map in the parameter file can only set cell resolution; any values in the
  * map will be ignored.
  */
  clGrid *mp_oGapGrid;

  short int m_iGapCode;/**<Return code for the "Is Gap" member of the "Gap
  Light" grid to get and set GLI in the cells.*/

  bool m_bCalcedGaps; /**<Whether or not the gap status has been calculated
  for this timestep.*/
};
//---------------------------------------------------------------------------
#endif // GapLight_H
