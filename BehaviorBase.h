//---------------------------------------------------------------------------

#ifndef BehaviorBaseH
#define BehaviorBaseH

#include "WorkerBase.h"
#include "DataTypes.h"

class clTreePopulation;


/**
* BehaviorBase - Version 2.0
* This class acts as a virtual parent for all behavior classes.  This allows
* the Simulation Manager to work with behavior objects without knowing anything
* about them.
*
* There should not be any objects instantiated from this class.
*
* Because I am continually surprised at the basic functions I want to mess with
* in my derived classes, every function here is virtual.  Most administrative
* dealings with behaviors have them cast to the clBehaviorBase class, so
* function-hiding won't work.  Override to your heart's content.
*
* Copyright 2011 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0; created
* version 2.0 (LEM)
* <br>May 18, 2015 - Changed GetParentParametersElement so it wasn't an
* error if element was not found (LEM)
*/
class clBehaviorBase : virtual public clWorkerBase{

public:

  /**
  * Gets the behavior version number.
  * @return Behavior version number.
  */
  virtual float GetBehaviorVersion() {return m_fVersionNumber;};

  /**
  * Constructor.  Initializes variables.
  * @param p_oSimManager Sim Manager object.
  */
  clBehaviorBase(clSimManager *p_oSimManager);

  /**
   * Destructor.  Destroys variables initialized in the constructor.
   */
  virtual ~clBehaviorBase();

  /**
   * This is the function which is called each timestep and performs the actual
   * work of the behavior.  This function is overridden in the child behavior
   * classes.
   */
  virtual void Action();

  /**
   * Makes sure that the version number of a file passed is between the minimum
   * and current version numbers.
   * @param fTestVersion Test version number to validate.
   * @return Whether the file number is valid.  Possible values:
   * <ul>
   * <li>-1 - this file has an invalid version number for this behavior</li>
   * <li>0 - this file has a valid version number but it does not match the
   * current number</li>
   * <li>1 - this file's version number matches the current version number of
   * the behavior</li>
   * </ul>
   */
  virtual short int ValidateVersionNumber(float fTestVersion);

  /**
   * Registers tree data members.  If a behavior has any tree data members to
   * add, it should register them by overriding this function.  It cannot be
   * assumed at this point that any data will have been read from the parameter
   * file for the behavior itself, although it can be assumed that the tree
   * population has read its data.
   */
  virtual void RegisterTreeDataMembers();

  /**
   * Sets the species/type combos for a behavior.  This function is only allowed
   * to be called once, because messing with it would be very awful.  These
   * don't check for duplicate values in the list passed. The unique species
   * list is assembled and placed in mp_iWhatSpecies.
   * @param iNumCombos Number of species/type combos being passed.
   * @param p_whatCombos Array of species/type combo objects.
   * @throws ModelErr If this function was called previously.
   */
  virtual void SetSpeciesTypeCombos(short int iNumCombos,
      stcSpeciesTypeCombo *p_whatCombos);

  /**
   * Sets the string for the parameter file behavior.  The string passed in the
   * parameter file is, by default, ignored; but since it could be used by a
   * behavior to pass information, a behavior class can override this function
   * to capture this.
   * @param sNameString The behavior name string from the parameter file.
   */
  virtual void SetNameData(std::string sNameString) {;};

  /**
   * Gets the number of new tree integer data members this behavior wants to
   * register.
   * @return Number of new tree integers, or zero if no new members are to be
   * registered.
   */
  virtual short int GetNewTreeInts() {return m_iNewTreeInts;};

  /**
   * Gets the number of new tree float data members this behavior wants to
   * register.
   * @return Number of new tree floats, or zero if no new members are to be
   * registered.
   */
  virtual short int GetNewTreeFloats() {return m_iNewTreeFloats;};

  /**
   * Gets the number of new tree character data members this behavior wants to
   * register.
   * @return Number of new tree characters, or zero if no new members are to be
   * registered.
   */
  virtual short int GetNewTreeChars() {return m_iNewTreeChars;};

  /**
   * Gets the number of new tree bool data members this behavior wants to
   * register.
   * @return Number of new tree bools, or zero if no new members are to be
   * registered.
   */
  virtual short int GetNewTreeBools() {return m_iNewTreeBools;};

  /**
   * Gets the number of species/type combos to which this behavior applies.
   * @return Number of species/type combos, or zero if this behavior doesn't
   * apply to any species/type combos.
   */
  virtual short int GetNumSpeciesTypeCombos() {return m_iNumSpeciesTypeCombos;};

  /**
   * Gets the number of unique tree species to which this behavior applies.
   * @return Number of behavior species, or zero if this behavior doesn't apply
   * to any species.
   */
  virtual short int GetNumBehaviorSpecies() {return m_iNumBehaviorSpecies;};

  /**
   * Gets one of this behavior's type/species combos.  To get all species or
   * types, loop through based on GetNumCombos and hit this function numerous
   * times.  I did it this way as the most secure way I could think of to
   * access the members of an array.
   * @param iIndex of the species/type combo.
   * @return The desired species/type combo.
   * @throw BAD_DATA error if the argument is not a valid array index.
   */
  struct stcSpeciesTypeCombo GetSpeciesTypeCombo(short int iIndex);

  /**
   * Gets one of the behavior's species.  To get all species, loop through based
   * on GetNumBehaviorSpecies and hit this function numerous times.  I did it
   * this way as the most secure way I could think of to access the members of
   * an array.
   * @param iIndex of the species.
   * @return The desired species number.
   * @throw BAD_DATA error if the argument is not a valid array index.
   */
  virtual short int GetBehaviorSpecies(short int iIndex);

  /**
   * Gets the behavior list number for this behavior, which differentiates
   * between multiple copies of the behavior in the behavior list.
   * @return Behavior list number.
   */
  short int GetBehaviorListNumber() {return m_iBehaviorListNumber;};

  /**
   * Sets the behavior list number for this behavior, which differentiates
   * between multiple copies of the behavior in the behavior list.
   * @param iNumber Behavior list number.
   */
  void SetBehaviorListNumber(short int iNumber)
                    {m_iBehaviorListNumber = iNumber;};

  /**
  * Formats the string for species/types query. This value can be used to
  * pass to clTreePopulation::Find() in order to get the trees to act on.
  * @return Query string.
  */
 std::string FormatSpeciesTypeQueryString();

  /**
   * This will get the correct set of parameters for this behavior based on
   * the behavior list position number.
   * @param p_oDoc Parsed XML doc.
   * @return DOMElement corresponding to this behavior; NULL if not found.
   */
  virtual DOMElement* GetParentParametersElement(xercesc::DOMDocument *p_oDoc);

protected:
  /**How many type/species combos a behavior will act on*/
  short int m_iNumSpeciesTypeCombos;

  /**How many distinct species are in the combo list - important for filling
   * species-specific values from parameter file*/
  short int m_iNumBehaviorSpecies;

  /**List of distinct species - for filling species-specific values from
   * parameter file*/
  short int *mp_iWhatSpecies;

  /**Array of species/type combos that the behavior will act on*/
  stcSpeciesTypeCombo *mp_whatSpeciesTypeCombos;

  /**The number of new tree integer data members this behavior wants to add.*/
  short int m_iNewTreeInts;

  /**The number of new tree float data members this behavior wants to add.*/
  short int m_iNewTreeFloats;

  /**The number of new tree character data members this behavior wants to add.*/
  short int m_iNewTreeChars;

  /**The number of new tree boolean data members this behavior wants to add.*/
  short int m_iNewTreeBools;

  /**The number of this behavior in the behavior list, to differentiate between
   * possible multiple copies of this behavior*/
  short int m_iBehaviorListNumber;

  /**Version number - this will be rounded to 2 digits after the decimal
   * place.*/
  float m_fVersionNumber;

  /**XML root that encloses the parameters for this behavior */
  std::string m_sXMLRoot;

  /**Minimum version number - this behavior will run parameter data for a file
   * marked between this number and the current version number, inclusive.*/
  float m_fMinimumVersionNumber;

}; //end of class clBehaviorBase

#endif
