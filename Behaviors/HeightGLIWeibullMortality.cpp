//---------------------------------------------------------------------------
#include "HeightGLIWeibullMortality.h"
#include "MortalityOrg.h"
#include "ModelMath.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clHeightGLIWeibullMortality::clHeightGLIWeibullMortality(clSimManager *p_oSimManager) :
      clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
      clMortalityBase(p_oSimManager) {
 try {

   m_sNameString = "height gli weibull mortshell";
   m_sXMLRoot = "HeightGLIWeibullMortality";

   mp_iLightCodes = NULL;
   mp_iHeightCodes = NULL;
   mp_iBrowsedCodes = NULL;
   mp_fA = NULL;
   mp_fB = NULL;
   mp_fC = NULL;
   mp_fD = NULL;
   mp_fMaxMort = NULL;
   mp_fBrowsedA = NULL;
   mp_fBrowsedB = NULL;
   mp_fBrowsedC = NULL;
   mp_fBrowsedD = NULL;
   mp_fBrowsedMaxMort = NULL;
   mp_iIndexes = NULL;

   //Version 2
   m_fVersionNumber = 2.0;
   m_fMinimumVersionNumber = 1.0;

   m_iYearsPerTimestep = 0;
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clHeightGLIWeibullMortality::clHeightGLIWeibullMortality";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clHeightGLIWeibullMortality::~clHeightGLIWeibullMortality() {
   int i;
   if (mp_iLightCodes) {
     for (i = 0; i < m_iNumBehaviorSpecies; i++) {
       delete[] mp_iLightCodes[i];
     }
   }
   delete[] mp_iLightCodes;

   if (mp_iHeightCodes) {
     for (i = 0; i < m_iNumBehaviorSpecies; i++) {
       delete[] mp_iHeightCodes[i];
     }
   }
   delete[] mp_iHeightCodes;

   if (mp_iBrowsedCodes) {
     for (i = 0; i < m_iNumBehaviorSpecies; i++) {
       delete[] mp_iBrowsedCodes[i];
     }
   }
   delete[] mp_iBrowsedCodes;

   delete[] mp_fA;
   delete[] mp_fB;
   delete[] mp_fC;
   delete[] mp_fD;
   delete[] mp_fMaxMort;
   delete[] mp_fBrowsedA;
   delete[] mp_fBrowsedB;
   delete[] mp_fBrowsedC;
   delete[] mp_fBrowsedD;
   delete[] mp_fBrowsedMaxMort;
   delete[] mp_iIndexes;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clHeightGLIWeibullMortality::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  try {
    clTreePopulation *p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Set up the array indexes
    mp_iIndexes = new int[iNumSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Get number of years per timestep
    m_iYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    GetTreeDataMemberCodes();
    ReadParameterFileData(p_oDoc);
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clHeightGLIWeibullMortality::DoShellSetup";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clHeightGLIWeibullMortality::ReadParameterFileData(xercesc::DOMDocument *p_oDoc) {
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try {
    clTreePopulation *p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

    short int iNumTypes = p_oPop->GetNumberOfTypes(),
              i, j;
    bool bBrowsing;

    //Declare the arrays we'd like read
    mp_fA = new double[m_iNumBehaviorSpecies];
    mp_fB = new double[m_iNumBehaviorSpecies];
    mp_fC = new double[m_iNumBehaviorSpecies];
    mp_fD = new double[m_iNumBehaviorSpecies];
    mp_fMaxMort = new double[m_iNumBehaviorSpecies];
    mp_fBrowsedA = new double[m_iNumBehaviorSpecies];
    mp_fBrowsedB = new double[m_iNumBehaviorSpecies];
    mp_fBrowsedC = new double[m_iNumBehaviorSpecies];
    mp_fBrowsedD = new double[m_iNumBehaviorSpecies];
    mp_fBrowsedMaxMort = new double[m_iNumBehaviorSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //"a"
    FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibA",
      "mo_hgwaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array - flip the sign
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fA[mp_iIndexes[p_fTempValues[i].code]] = -p_fTempValues[i].val;

    //"b"
    FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibB",
      "mo_hgwbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //"c"
    FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibC",
      "mo_hgwcVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fC[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //"d"
    FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibD",
      "mo_hgwdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fD[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Max mortality
    FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibMaxMort",
      "mo_hgwmmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array, making sure all values are
    //between 0 and 1
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val < 0 || p_fTempValues[i].val > 1) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clHeightGLIWeibullMortality::ReadParameterFileData";
        stcErr.sMoreInfo = "All values in max mortality must be between 0 and 1.";
        throw(stcErr);
      }
      mp_fMaxMort[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Check to see if there are any browsed trees
    bBrowsing = false;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      for ( j = 0; j < iNumTypes; j++ ) {
        if (mp_iBrowsedCodes[i] [j] > -1) {
          bBrowsing = true;
          break;
        }
      }
    }
    if (bBrowsing) {
      //"a"
      FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibBrowsedA",
        "mo_hgwbaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array - flip the sign
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fBrowsedA[mp_iIndexes[p_fTempValues[i].code]] = -p_fTempValues[i].val;

      //"b"
      FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibBrowsedB",
        "mo_hgwbbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fBrowsedB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //"c"
      FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibBrowsedC",
        "mo_hgwbcVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fBrowsedC[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //"d"
      FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibBrowsedD",
        "mo_hgwbdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fBrowsedD[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      //Max mortality
      FillSpeciesSpecificValue( p_oElement, "mo_heightGLIWeibBrowsedMaxMort",
        "mo_hgwbmmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer values to our permanent array, making sure all values are
      //between 0 and 1
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        if (p_fTempValues[i].val < 0 || p_fTempValues[i].val > 1) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clHeightGLIWeibullMortality::ReadParameterFileData";
          stcErr.sMoreInfo = "All values in max mortality must be between 0 and 1.";
          throw(stcErr);
        }
        mp_fBrowsedMaxMort[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
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
    stcErr.sFunction = "clHeightGLIWeibullMortality::DoShellSetup";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////
// DoMort()
///////////////////////////////////////////////////////////////////////////
deadCode clHeightGLIWeibullMortality::DoMort(clTree *p_oTree, const float &fDiam,
     const short int &iSp) {

  float fGLI, fHeight, fMortProb;
  int iTp = p_oTree->GetType();
  bool bBrowsed = false;

  //Get the tree's light level
  p_oTree->GetValue(mp_iLightCodes[mp_iIndexes[iSp]][iTp], &fGLI);

  //Get the tree's height
  p_oTree->GetValue(mp_iHeightCodes[mp_iIndexes[iSp]][iTp], &fHeight);

  //Get whether this tree is browsed, if appropriate
  if (mp_iBrowsedCodes[mp_iIndexes[iSp]][iTp] > -1) {
    p_oTree->GetValue(mp_iBrowsedCodes[mp_iIndexes[iSp]][iTp], &bBrowsed);
  }

  //Calculate the mortality probability
  if (bBrowsed) {
    fMortProb = mp_fBrowsedMaxMort[mp_iIndexes[iSp]] *
        exp(mp_fBrowsedA[mp_iIndexes[iSp]] * pow(fHeight, mp_fBrowsedB[mp_iIndexes[iSp]]) -
            mp_fBrowsedC[mp_iIndexes[iSp]] * pow(fGLI, mp_fBrowsedD[mp_iIndexes[iSp]]));
  } else {
    fMortProb = mp_fMaxMort[mp_iIndexes[iSp]] *
        exp(mp_fA[mp_iIndexes[iSp]] * pow(fHeight, mp_fB[mp_iIndexes[iSp]]) -
            mp_fC[mp_iIndexes[iSp]] * pow(fGLI, mp_fD[mp_iIndexes[iSp]]));
  }

  //Compound by number of years per timestep
  if (1 != m_iYearsPerTimestep)
    fMortProb = 1 - pow(1 - fMortProb, m_iYearsPerTimestep);

  if(clModelMath::GetRand() < fMortProb)
    return natural;
  else
    return notdead;
}

//////////////////////////////////////////////////////////////////////////////
// GetTreeDataMemberCodes()
//////////////////////////////////////////////////////////////////////////////
void clHeightGLIWeibullMortality::GetTreeDataMemberCodes()
{
  try
  {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    char cLightLabel[] = "Light",
         cBrowsedLabel[] = "Browsed";
    short int i, j,  //loop counters
              iSp, iTp, //for easier code reading
              iNumTypes = p_oPop->GetNumberOfTypes();

    //Declare and initialize the return codes arrays
    mp_iLightCodes = new short int * [m_iNumBehaviorSpecies];
    mp_iHeightCodes = new short int * [m_iNumBehaviorSpecies];
    mp_iBrowsedCodes = new short int * [m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_iLightCodes[i] = new short int[iNumTypes];
      mp_iHeightCodes[i] = new short int[iNumTypes];
      mp_iBrowsedCodes[i] = new short int[iNumTypes];

      for ( j = 0; j < iNumTypes; j++ )
      {
        mp_iLightCodes[i] [j] = -1;
        mp_iHeightCodes[i] [j] = -1;
        mp_iBrowsedCodes[i] [j] = -1;
      }
    }

    //Get the codes for all species/types to which this behavior applies -
    //throw an error if any is -1
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ ) {
      iSp = mp_whatSpeciesTypeCombos[i].iSpecies;
      iTp = mp_whatSpeciesTypeCombos[i].iType;
      mp_iLightCodes[mp_iIndexes[iSp]][iTp] =
                    p_oPop->GetFloatDataCode(cLightLabel, iSp, iTp);
      if (-1 == mp_iLightCodes[mp_iIndexes[iSp]][iTp]) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clHeightGLIWeibullMortality::GetTreeDataMemberCodes";
        stcErr.sMoreInfo = "All trees that use Height GLI Mortality must also use a light behavior.";
        throw(stcErr);
      }
      mp_iHeightCodes[mp_iIndexes[iSp]][iTp] = p_oPop->GetHeightCode(iSp, iTp);
      mp_iBrowsedCodes[mp_iIndexes[iSp]][iTp] = p_oPop->GetBoolDataCode(cBrowsedLabel, iSp, iTp);
    }
  }
  catch ( modelErr & err )
  {
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clHeightGLIWeibullMortality::GetTreeDataMemberCodes" ;
    throw( stcErr );
  }
}
