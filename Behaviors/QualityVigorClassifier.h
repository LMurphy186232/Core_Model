#ifndef QUALITYVIGORCLASSIFIER_H_
#define QUALITYVIGORCLASSIFIER_H_

#include "BehaviorBase.h"

class clTree;
class clTreePopulation;

/**
 * Quality Vigor Classifier version 1.0
 * Manages tree classification based on stem vigor, species classification, and
 * quality.
 *
 * <b>Assigning a classification</b><br/>
 * The possible states of "species type" are deciduous and coniferous. The
 * possible states of "vigor" are vigorous and not vigorous. The possible states
 * of "quality" are sawlog potential and no sawlog potential.
 *
 * Only adults can be assigned a class number.
 * @htmlonly
  <table border=1>
  <tr>
  <th>Spp Type</th>  <th>Vigor</th><th>Sawlog?</th><th>DBH</th><th>Class</th>
  </tr>
  <tr>
  <td>Deciduous</td> <td>Vig</td>  <td>Yes</td>    <td>Any</td><td>1</td>
  </tr>
  <tr>
  <td>Deciduous</td> <td>Vig</td>  <td>No</td>     <td>Any</td><td>2</td>
  </tr>
  <tr>
  <td>Deciduous</td> <td>Not</td>  <td>Yes</td>    <td>&gt; 23 cm</td> <td>3</td>
  </tr>
  <tr>
  <td>Deciduous</td> <td>Not</td>  <td>Yes</td>    <td>&lt;= 23 cm</td><td>4</td>
  </tr>
  <tr>
  <td>Deciduous</td> <td>Not</td>  <td>No</td>     <td>Any</td><td>4</td>
  </tr>
  <tr>
  <td>Coniferous</td><td>Vig</td>  <td>NA</td>     <td>Any</td><td>5</td>
  </tr>
  <tr>
  <td>Coniferous</td><td>Not</td>  <td>NA</td>     <td>Any</td><td>6</td>
  </tr>
  </table>
  @endhtmlonly
 *
 * <b>Assigning a vigor and quality to new adults</b><br/>
 * For each species, there is a probability that a new adult of that species is
 * vigorous, and for deciduous species, that it is sawlog quality. A random
 * number is compared to these values to determine the vigor and quality of a
 * new adult tree. Classification can then proceed according to the table above.
 *
 * <b>Initial conditions</b><br/>
 * Trees may come in with a class already assigned in a tree map. If not, the
 * user can specify the proportion of trees vigorous and sawlog in various size
 * classes. Any trees not covered by one of the two previous assignment methods
 * will be assigned based on the new adult proportions.
 *
 * <b>Changes in vigor and quality</b><br/>
 * The probability of transition between vigor states and quality states is
 * calculated with the same functional form (but different parameters). The
 * probability of being vigorous or of being of sawlog quality is:
 *
 * @htmlonly
  <center>prob= exp(&beta;X)/(1+exp(&beta;X))</center>
   @endhtmlonly where:

   @htmlonly
   <center>&beta;X= &beta;<sub>0</sub> + &beta;<sub>1init</sub> +
   &beta;<sub>2sp</sub>*DBH + &beta;<sub>3sp</sub>*ln(DBH)</center>
   @endhtmlonly
 *
 * where &beta;<sub>1init</sub> is a beta for the initial state.
 *
 * This behavior adds three tree data members: two booleans, "vigorous" and
 * "sawlog", and one int called "treeclass" with a possible value from 1-6.
 *
 * This behavior can only be applied to adult trees.
 *
 * The namestring and parameter file call string are "QualityVigorClassifier".
 *
 * Copyright 2012 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>January 9, 2012 - Created (LEM)
 */
class clQualityVigorClassifier: virtual public clBehaviorBase {
public:

  friend class clTestQualityVigorClassifier;

  /**
   * Constructor.
   * @param p_oSimManager Sim Manager object.
   */
  clQualityVigorClassifier(clSimManager *p_oSimManager);

  /**
   * Destructor.
   */
  ~clQualityVigorClassifier();

  /**
   * Does class management. This gets all trees that this behavior acts on.
   * First, vigor and quality transitions are dealt with. If a tree does not
   * already have a class, it will get assignments based on the new adult
   * probabilities. If it does, it will get assessed for both vigor class and
   * quality class transition.
   */
  void Action();

  /**
   * Does setup by calling ReadParameterFile() and
   * HandleInitialConditionsTrees().
   * @param p_oDoc DOM tree of parsed input file.
   */
  void GetData(xercesc::DOMDocument *p_oDoc);

  /**
   * Registers the "Biomass" float data member.  The return codes are captured
   * in the mp_iBiomassCodes array.
   * @throw modelErr if this behavior is being applied to any tree type except
   * adults.
   */
  void RegisterTreeDataMembers();

  /**
   * Assigns a class to a tree based on vigor and quality. The class value is
   * placed in the tree's "treeclass" data member. If vigor or quality is
   * missing, the class value assigned is 0.
   * @param p_oPop Tree population.
   * @param p_oTree Tree to assign class to.
   */
  void AssignClass(clTreePopulation *p_oPop, clTree *p_oTree);

private:

  /**String to pass to clTreePopulation::Find() in order to get the trees to
  * apply damage to.  This will instigate a species/type search for all the
  * species and types to which this behavior applies.*/
  char *m_cQuery;

  /** Beta0 for vigor transition. Sized number of species assigned to this
   * behavior.*/
  double *mp_fVigBeta0;

  /** Beta1 for vigor transition, initial class 1. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fVigBeta11;

  /** Beta1 for vigor transition, initial class 2. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fVigBeta12;

  /** Beta1 for vigor transition, initial class 3. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fVigBeta13;

  /** Beta1 for vigor transition, initial class 4. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fVigBeta14;

  /** Beta1 for vigor transition, initial class 5. Sized number of species
   * assigned to this behavior. If species is deciduous, this is ignored.*/
  double *mp_fVigBeta15;

  /** Beta1 for vigor transition, initial class 6. Sized number of species
   * assigned to this behavior. If species is deciduous, this is ignored.*/
  double *mp_fVigBeta16;

  /** Beta2 for vigor transition. Sized number of species assigned to this
   * behavior.*/
  double *mp_fVigBeta2;

  /** Beta3 for vigor transition. Sized number of species assigned to this
   * behavior.*/
  double *mp_fVigBeta3;

  /** Beta0 for quality transition. Sized number of species assigned to this
   * behavior. If species is coniferous, this is ignored.*/
  double *mp_fQualBeta0;

  /** Beta1 for quality transition, initial class 1. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fQualBeta11;

  /** Beta1 for quality transition, initial class 2. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fQualBeta12;

  /** Beta1 for quality transition, initial class 3. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fQualBeta13;

  /** Beta1 for quality transition, initial class 4. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fQualBeta14;

  /** Beta2 for quality transition. Sized number of species assigned to this
   * behavior. If species is coniferous, this is ignored.*/
  double *mp_fQualBeta2;

  /** Beta3 for quality transition. Sized number of species assigned to this
   * behavior. If species is coniferous, this is ignored.*/
  double *mp_fQualBeta3;

  /** Probability of new adults being vigorous. Sized number of species assigned
   * to this behavior.*/
  double *mp_fNewAdultProbVigorous;

  /** Probability of new adults being sawlog quality. Sized number of species
   * assigned to this behavior. If species is coniferous, this is ignored.*/
  double *mp_fNewAdultProbSawlog;

  /**For array access*/
  short int *mp_iIndexes;

  /**Return codes for "vigorous" tree data member. Sized number of species
   * assigned to this behavior.*/
  short int *mp_iVigorousInd;

  /**Return codes for "sawlog" tree data member. Sized number of species
   * assigned to this behavior.*/
  short int *mp_iSawlogInd;

  /**Return codes for "treeclass" tree data member. Sized number of species
   * assigned to this behavior.*/
  short int *mp_iTreeclassInd;

  /** Whether or not a species is deciduous. (False means coniferous.) Sized
   * number of species assigned to this behavior.*/
  bool *mp_bDeciduous;

  /**
   * Reads the data from the parameter file.
   * @param p_oDoc DOM tree of parsed input file.
   * @param p_oPop Tree population.
   */
  void ReadParameterFile(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

  /**
   * Handles initial conditions trees. This will look for initial conditions
   * defined proportions of vigorous and sawlog info. If missing, or for any
   * trees for which it is missing, the new adult probabilities will be used
   * to assign vigorous and sawlog status, and then class.
   * @param p_oPop Tree population.
   * @param p_oDoc DOM tree of parsed input file.
   */
  void HandleInitialConditionsTrees(xercesc::DOMDocument *p_oDoc, clTreePopulation *p_oPop);

  /**
   * Assigns a vigor and quality to a tree based on the new adult probabilities.
   * @param p_oTree Tree to assign vigor and quality to.
   */
  void AssignVigorQuality(clTree *p_oTree);

  /**
   * Evaluates possible transition in vigor and quality for a tree, then assigns
   * the new values.
   * @param p_oPop Tree population.
   * @param p_oTree Tree to evaluate.
   */
  void EvolveVigorQuality(clTreePopulation *p_oPop, clTree *p_oTree);

  /**
  * Formats the string in m_cQuery.  This value will be used in Action() to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  * @param p_oPop Tree population object.
  */
  void FormatQueryString(clTreePopulation *p_oPop);
};

#endif
