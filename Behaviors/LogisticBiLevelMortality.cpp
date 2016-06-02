//---------------------------------------------------------------------------
#include "LogisticBiLevelMortality.h"
#include "MortalityOrg.h"
#include "Grid.h"
#include "ModelMath.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clLogisticBiLevelMortality::clLogisticBiLevelMortality(clSimManager *p_oSimManager) :
      clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
      clMortalityBase(p_oSimManager) {
 try {

   m_sNameString = "logistic bilevel mortshell";
   m_sXMLRoot = "LogisticBiLevelMortality";

   mp_oStormLight = NULL;
   mp_fLoLightB = NULL;
   mp_fLoLightA = NULL;
   mp_fHiLightB = NULL;
   mp_fHiLightA = NULL;
   mp_fHiLightThreshold = NULL;
   mp_iIndexes = NULL;

   mp_oPop = NULL;
   m_iLightCode = -1;
   m_iYearsPerTimestep = 0;
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clLogisticBiLevelMortality::clLogisticBiLevelMortality";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clLogisticBiLevelMortality::~clLogisticBiLevelMortality() {
  delete[] mp_fLoLightB;
  delete[] mp_fLoLightA;
  delete[] mp_fHiLightB;
  delete[] mp_fHiLightA;
  delete[] mp_fHiLightThreshold;
  delete[] mp_iIndexes;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clLogisticBiLevelMortality::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try {
    mp_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int iNumSpecies = mp_oPop->GetNumberOfSpecies(), i;

    //Get number of years per timestep
    m_iYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    mp_fLoLightA = new double[m_iNumBehaviorSpecies];
    mp_fLoLightB = new double[m_iNumBehaviorSpecies];
    mp_fHiLightA = new double[m_iNumBehaviorSpecies];
    mp_fHiLightB = new double[m_iNumBehaviorSpecies];
    mp_fHiLightThreshold = new double[m_iNumBehaviorSpecies];
    mp_iIndexes = new int[iNumSpecies];

    //Set up the array indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //See if we can find the storm light grid
    mp_oStormLight = mp_oSimManager->GetGridObject("Storm Light");
    if (NULL != mp_oStormLight) {
      m_iLightCode = mp_oStormLight->GetFloatDataCode("Light");
    }

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Low-light growth "a"
    FillSpeciesSpecificValue( p_oElement, "mo_logBilevLoLiteA",
        "mo_lbllaVal", p_fTempValues, m_iNumBehaviorSpecies, mp_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fLoLightA[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Low-light growth "b"
    FillSpeciesSpecificValue( p_oElement, "mo_logBilevLoLiteB",
        "mo_lbllbVal", p_fTempValues, m_iNumBehaviorSpecies, mp_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fLoLightB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Get high-light parameters if we have a storm light grid
    if (NULL != mp_oStormLight) {

      //High-light threshold
      FillSpeciesSpecificValue( p_oElement, "mo_logBilevHiLiteThreshold",
        "mo_lbhltVal", p_fTempValues, m_iNumBehaviorSpecies, mp_oPop, true );
      //Transfer values to our permanent array, making sure all values are
      //between 0 and 100
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        if (p_fTempValues[i].val < 0 || p_fTempValues[i].val > 100) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clLogisticBiLevelMortality::DoShellSetup";
          stcErr.sMoreInfo = "All values in high-light growth threshold must be between 0 and 100.";
          throw(stcErr);
        }
        mp_fHiLightThreshold[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      }

      //High-light growth "a"
      FillSpeciesSpecificValue( p_oElement, "mo_logBilevHiLiteA",
        "mo_lbhlaVal", p_fTempValues, m_iNumBehaviorSpecies, mp_oPop, true );
      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fHiLightA[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //High-light growth "b"
      FillSpeciesSpecificValue( p_oElement, "mo_logBilevHiLiteB",
        "mo_lbhlbVal", p_fTempValues, m_iNumBehaviorSpecies, mp_oPop, true );
      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fHiLightB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    } else {
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        mp_fHiLightThreshold[i] = 100;
      }
    }

    delete[] p_fTempValues;
  }
  catch (modelErr&err) {
    delete[] p_fTempValues;
    throw(err);
  }
  catch (modelMsg &msg) {
    delete[] p_fTempValues;
    throw(msg);
  } //non-fatal error
  catch (...) {
    delete[] p_fTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clLogisticBiLevelMortality::DoShellSetup";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////
// DoMort()
///////////////////////////////////////////////////////////////////////////
deadCode clLogisticBiLevelMortality::DoMort(clTree *p_oTree, const float &fDiam,
     const short int &iSpecies) {

  float fLightLevel = 0, fSurvivalProb, fMortDiam;
  int iTp = p_oTree->GetType();

  //If applicable, get the storm light level at the tree's location
  if (NULL != mp_oStormLight) {
    float fX, fY;
    p_oTree->GetValue(mp_oPop->GetXCode(iSpecies, iTp), &fX);
    p_oTree->GetValue(mp_oPop->GetYCode(iSpecies, iTp), &fY);

    mp_oStormLight->GetValueAtPoint(fX, fY, m_iLightCode, &fLightLevel);
  }

  //If the tree is a seedling, get the tree's diam10
  if (iTp == clTreePopulation::seedling && 0 == fDiam)
    p_oTree->GetValue(mp_oPop->GetDiam10Code(iSpecies, iTp), &fMortDiam);
  else fMortDiam = fDiam;

  //Calculate the function value according to the light level
  if (fLightLevel < mp_fHiLightThreshold[mp_iIndexes[iSpecies]])
    fSurvivalProb = exp(mp_fLoLightA[mp_iIndexes[iSpecies]] + mp_fLoLightB[mp_iIndexes[iSpecies]] * fMortDiam);
  else
    fSurvivalProb = exp(mp_fHiLightA[mp_iIndexes[iSpecies]] + mp_fHiLightB[mp_iIndexes[iSpecies]] * fMortDiam);

  fSurvivalProb = fSurvivalProb / (1 + fSurvivalProb);
  //Compound by number of years per timestep
  if (1 != m_iYearsPerTimestep)
    fSurvivalProb = pow(fSurvivalProb, m_iYearsPerTimestep);

  if(clModelMath::GetRand() < fSurvivalProb)
    return notdead;
  else
    return natural;
}
