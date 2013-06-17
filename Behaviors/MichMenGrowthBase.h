//---------------------------------------------------------------------------

#ifndef MichMenGrowthBaseH
#define MichMenGrowthBaseH
//---------------------------------------------------------------------------
#include "GrowthBase.h"

//class DOMDocument;

/**
* Michaelis-Menton growth base - Version 1.0
*
* This is the base class for growth behavior shell classes which use the
* Michaelis-Menton function for growth.  The variables held in common across
* classes are static here.
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>October 20, 2011 - Wiped the slate clean for SORTIE 7.0 (LEM)
*/
class clMichMenBase : virtual public clGrowthBase {
//note: need the virtual keyword to avoid base class ambiguity.
  public:

  /**
  * Constructor.
  * @param p_oSimManager Sim Manager object.
  */
  clMichMenBase(clSimManager *p_oSimManager);

  /**
  * Destructor.  Deletes the static arrays if they haven't already been deleted.
  */
  virtual ~clMichMenBase();

  protected:

  //These are the variables held in common by the descendent classes.

  //I did have these as static but I un-static-ed them because it was hard
  //to tell which species required what.  I know that means multiple copies
  //but for now that's okay.

  /**Slope of diameter growth response. Array size is number of species. Old
   * parameter g2.*/
  float *mp_fSlopeDiamGrowthResponse;
  /**Slope of height growth response. Array size is number of species.*/
  float *mp_fSlopeHeightGrowthResponse;
  /**Adult constant basal area increment in cm2/timestep (parameter file value
   * is expected in cm2/yr). Array size is number of species.*/
  float *mp_fAdultConstBAInc;
  /**Adult constant radial increment in cm/timestep (parameter file value is
   * expected in mm/yr). Old parameter g4. Array size is number of species.*/
  float *mp_fAdultConstRadInc;
  /**Asymptotic diameter growth. Old parameter g1.  Array size is number of
   * species.*/
  float *mp_fAsympDiamGrowth;
  /**Asymptotic height growth. Old parameter g1.  Array size is number of
   * species.*/
  float *mp_fAsympHeightGrowth;

  /**Whether or not growth is limited to constant radial increment*/
  bool m_bConstRadialLimited;
  /**Whether or not growth is limited to constant basal area increment*/
  bool m_bConstBasalAreaLimited;

  /**
  * Extracts growth parameters.  This should be called in child DoShellSetup()
  * functions. If a child behavior wants a particular array filled, simply
  * declare it; all non-needed arrays can be left NULL.  Only those species to
  * which a child behavior apply will be filled.
  *
  * @param p_oDoc DOM tree of parsed input file.
  */
  void GetParameterFileData(xercesc::DOMDocument *p_oDoc);

  /**
  * Calculates the result of the Michaelis-Menton function using diameter growth
  * parameters.
  *
  * @param iSpecies Species number
  * @param fGli GLI value as a percentage between 0 and 100
  * @return Function result.
  */
  inline float CalculateMichaelisMentonDiam(short int &iSpecies, float &fGli)
  { return ( (mp_fAsympDiamGrowth[iSpecies] * fGli) /
            (mp_fSlopeDiamGrowthResponse[iSpecies] + fGli) ); };

  /**
  * Calculates the result of the Michaelis-Menton function using height growth
  * parameters.
  *
  * @param iSpecies Species number
  * @param fGli GLI value as a percentage between 0 and 100
  * @return Function result.
  */
  inline float CalculateMichaelisMentonHeight(short int &iSpecies, float &fGli)
  { return ( (mp_fAsympHeightGrowth[iSpecies] * fGli) /
             (mp_fSlopeHeightGrowthResponse[iSpecies] + fGli) ); };

  /**
  * Applies applicable growth limits according to the flags set.  If a flag is
  * set and the growth passed is higher than the applicable limit, the limit is
  * returned instead; otherwise, the growth is returned.
  *
  * If m_bConstRadialLimited is set to true, the limit is the value in
  * mp_fAdultConstRadInc for that species.  If m_bConstBasalAreaLimited is set
  * to true, then the limit is the value in mp_fAdultConstBAInc divided by the
  * tree's diameter (recall that mp_fAdultConstBAInc is in squared units).
  *
  * @param iSpecies Species number
  * @param fAmountDiamIncrease Amount of diameter increase in cm/timestep
  * @param fDiam Tree diameter in cm
  * @return  The amount of growth in cm
  */
  float ApplyGrowthLimits(const short int &iSpecies,
      const float &fAmountDiamIncrease, const float &fDiam);

  /**
  * Gets the proper value for the "Growth" data member for mortality
  * calculations based on growth.  In order to keep old slow growers from
  * dying, if the tree's DBH is greater than 30 cm and its GLI is greater than
  * 10, the return value is 10.  Otherwise, the tree's amount of annual
  * diameter increase is used.
  * @param p_oTree Tree to get "Growth" for.
  * @param fDiameterGrowth Amount of diameter growth to be added.
  * Value to place in "Growth", in mm radial growth/yr.
  */
  float GetGrowthMemberValue(clTree *p_oTree, float fDiameterGrowth);

};
//---------------------------------------------------------------------------
#endif
