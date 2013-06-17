#ifndef NONCITERM_H_
#define NONCITERM_H_

#include "NCITermBase.h"

/**
 * Class that provides no NCI term (value of 1).
 */
class clNoNCITerm: virtual public clNCITermBase {
public:
  float CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot) {return 1;};
  void DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {;};
};

#endif /* NONCITERM_H_ */
