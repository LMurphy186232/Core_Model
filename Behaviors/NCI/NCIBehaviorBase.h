//---------------------------------------------------------------------------
#ifndef NCIBehaviorBaseH
#define NCIBehaviorBaseH
//---------------------------------------------------------------------------
#include "NCIEffectsList.h"
#include "BehaviorBase.h"

#include <xercesc/dom/DOM.hpp>
#include <string>

class clTreePopulation;
class clBehaviorBase;
class clShadingEffectBase;
class clDamageEffectBase;
class clSizeEffectBase;
class clNCITermBase;
class clCrowdingEffectBase;
class clPrecipitationEffectBase;
class clTemperatureEffectBase;
class clNitrogenEffectBase;
class clInfectionEffectBase;

/**
* NCI behavior base
*
* This is a base class for classes that are going to implement NCI functions.
*
* Copyright 2012 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>December 18, 2013: Created (LEM)
*/
class clNCIBehaviorBase {
//note: need the virtual keyword to avoid base class ambiguity.

  public:

  /**
  * Constructor.
  */
   clNCIBehaviorBase(); //clSimManager * p_oSimManager);

  /**
  * Destructor. Frees memory.
  */
  ~clNCIBehaviorBase();

  /** Get the shading effect object. */
  clShadingEffectBase* GetShadingEffect() {return mp_oShadingEffect;};

  /** Get the damage effect object. */
  clDamageEffectBase* GetDamageEffect() { return mp_oDamageEffect;};

  /** Get the size effect object. */
  clSizeEffectBase* GetSizeEffect() { return mp_oSizeEffect;};

  /** Get the crowding effect object. */
  clCrowdingEffectBase* GetCrowdingEffect() { return mp_oCrowdingEffect;};

  /** Get the NCI term object. */
  clNCITermBase* GetNCITerm() { return mp_oNCITerm;};

  /** Get the precipitation effect object. */
  clPrecipitationEffectBase* GetPrecipEffect() { return mp_oPrecipEffect;};

  /** Get the temperature effect object. */
  clTemperatureEffectBase* GetTempEffect() { return mp_oTempEffect;};

  /** Get the nitrogen effect object. */
  clNitrogenEffectBase* GetNEffect() { return mp_oNEffect;};

  /** Get the infection effect object. */
  clInfectionEffectBase* GetInfectionEffect() { return mp_oInfectionEffect;};

  protected:

  /** The shading effect object. */
  clShadingEffectBase *mp_oShadingEffect;

  /** The damage effect object. */
  clDamageEffectBase *mp_oDamageEffect;

  /** The size effect object. */
  clSizeEffectBase *mp_oSizeEffect;

  /** The crowding effect object. */
  clCrowdingEffectBase *mp_oCrowdingEffect;

  /** The NCI term object. */
  clNCITermBase *mp_oNCITerm;

  /** The precipitation effect object. */
  clPrecipitationEffectBase *mp_oPrecipEffect;

  /** The temperature effect object. */
  clTemperatureEffectBase *mp_oTempEffect;

  /** The nitrogen effect object. */
  clNitrogenEffectBase *mp_oNEffect;

  /** The infection effect object. */
  clInfectionEffectBase *mp_oInfectionEffect;

  /**
  * Reads data from the parameter file for the effects objects. This allows
  * for a child behavior that does not use individual targets with diameters.
  * If that is the case, this will not set a size effect and will throw
  * errors for all effects that depend on a target diameter.
  * @param p_oElement Parent element for this behavior.
  * @param p_oPop Tree population object.
  * @param p_oNCI Parent behavior.
  * @param bUsingDiam If false, a behavior has no target diameter.
  * @throws modelErr if bUsingDiam is false and a user requests an effect that
  * depends on a target diameter.
  */
  void ReadParameterFile(xercesc::DOMElement * p_oElement,
                         clTreePopulation *p_oPop,
                         clBehaviorBase *p_oNCI,
                         bool bUsingDiam);

};
//---------------------------------------------------------------------------
#endif
