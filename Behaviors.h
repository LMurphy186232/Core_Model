//---------------------------------------------------------------------------

#ifndef BehaviorsH
#define BehaviorsH
//---------------------------------------------------------------------------

#include "ObjectManagerBase.h"
#include "Constants.h"
#include "BehaviorBase.h"

/**
 * Behaviors - Version 1.0
 * Behavior Manager
 *
 * Copyright 2003 Charles D. Canham.
 * @author Lora E. Murphy
 *
 * <br>Edit history:
 * <br>-----------------
 * <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clBehaviorManager : public clObjectManagerBase {

public:

  /**
   * This constructor structure makes sure that there's no default constructor
   * while also saying that this child class doesn't need its own constructor
   * to do anything.
   */
  clBehaviorManager(clSimManager *p_oSimManager) :
                            clObjectManagerBase(p_oSimManager){;};
  //~clBehaviorManager(); Destructor not needed


  /**
   * Creates the behavior objects for a run.  The objects are created from the
   * behaviorList tags in the document.  If there are currently objects created,
   * they are deleted and the new objects created.  This function will validate
   * the species and tree types (if any) to apply to each behavior.
   * @param p_oDoc DOM tree parsed from the XML file.
   * @throws ModelErr if a species or tree type is invalid.
   */
  void CreateObjects(xercesc::DOMDocument *p_oDoc);

protected:

 /**Holds setup information for a single behavior object.*/
 struct behaviorData {
   /**The name string of the behavior.  This must be one that the behavior
   * manager knows as a class in this project.*/
   std::string sNameString;
   /**Behavior version number in the parameter file.*/
   float fVersion;
   /**The number of this behavior in the behavior list, to differentiate
     * between possible multiple copies of a behavior*/
    short int iBehaviorListNumber;
    /**The number of species/type combos to which to apply to this behavior
     * (if any).*/
    short int iNumCombos;
    /**Array of species/type combos.*/
    stcSpeciesTypeCombo *p_whatCombos;
  };

  /**
   * Creates an individual behavior object. The manager will create the
   * object and add it to the end of the behavior list array. If there is
   * already a behavior present with this name, this will overwrite that
   * behavior. It will also validate the version number from the parameter file.
   * @param p_data A pointer to a behaviorData structure containing the
   * behavior's setup information.
   * @throws ModelErr if the behavior isn't recognized.
   */
  void CreateBehavior(behaviorData *p_data);
};

//----------------------------------------------------------------------------
#endif
