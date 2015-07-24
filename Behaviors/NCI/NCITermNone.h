#ifndef NONCITERM_H_
#define NONCITERM_H_

#include "NCITermBase.h"

/**
 * Class that provides no NCI term (value of 1).
 */
class clNCITermNone: virtual public clNCITermBase {
public:
  ncivals CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies) {
    ncivals toreturn; toreturn.fNCI1 = 1; return toreturn;};
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {;};
};

#endif /* NONCITERM_H_ */
