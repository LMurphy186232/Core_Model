#ifndef NCITERMBASE_H_
#define NCITERMBASE_H_

#include <xercesc/dom/DOM.hpp>

class clTreePopulation;
class clBehaviorBase;
class clTree;
class clPlot;

/**
 * Provides a base for objects that calculate the NCI term(s). The default is
 * that objects descended from this class can handle any tree type. If this is
 * not the case, child objects must throw the appropriate error during setup.
 */
class clNCITermBase {


public:

  /** Holds multiple values for NCI. */
  struct ncivals {
    double fNCI1;
    double fNCI2;
  };

  /**
   * Constructor. Sets defaults.
   */
  clNCITermBase() {bRequiresTargetDiam = false; iNumNCIs = 1;};

  int GetNumberNCIs() {return iNumNCIs;};

  /**
  * Performs calculations like either clGrowthBase::PreGrowthCalcs or
  * clMortalityBase::PreMortCalcs.
  */
  virtual void PreCalcs( clTreePopulation *p_oPop ){;};

  /**
   * Calculates NCI term.
   * @param p_oTree Tree for which to calculate NCI. If bRequiresTargetDiam is
   * false, be prepared for this to be NULL.
   * @param p_oPop Tree population.
   * @param p_oPlot Plot object.
   * @param fX X coordinate for which to calculate NCI. Needed if p_oTree is
   * NULL.
   * @param fY Y coordinate for which to calculate NCI. Needed if p_oTree is
   * NULL.
   * @param iSpecies Species for which to calculate NCI. Needed if p_oTree is
   * NULL.
   */
  virtual ncivals CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies) = 0;

  /**
   * Does any desired setup.
   * @param p_oPop Tree population.
   * @param p_oNCI NCI behavior object.
   * @param p_oElement Root element of the behavior.
   */
  virtual void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) = 0;

  /**
   * Destructor
   */
  virtual ~clNCITermBase(){};

  bool DoesRequireTargetDiam() {return bRequiresTargetDiam;};

protected:

  /** Whether or not this effect depends on a target diameter being available.*/
  bool bRequiresTargetDiam;

  /** Number of NCI values calculated and populated into ncivals. */
  int iNumNCIs;
};

#endif /* NCITERMBASE_H_ */
