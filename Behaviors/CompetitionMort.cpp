//---------------------------------------------------------------------------
#include "CompetitionMort.h"
#include "MortalityOrg.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ModelMath.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clCompetitionMort::clCompetitionMort(clSimManager *p_oSimManager) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
clMortalityBase(p_oSimManager) {
  try {

    m_sNameString = "competitionmortshell";
    m_sXMLRoot = "CompetitionMortality";

    mp_fCompMort = NULL;
    mp_fCompMortMax = NULL;
    mp_iGrowthCodes = NULL;
    mp_fXb = NULL;
    mp_fMaxPotentialGrowth = NULL;
    mp_fX0 = NULL;
    mp_iIndexes = NULL;

    m_fNumberYearsPerTimestep = 0;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clCompetitionMort::clCompetitionMort";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clCompetitionMort::~clCompetitionMort() {
  int i; //loop counter

  if (mp_iGrowthCodes) {
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      delete[] mp_iGrowthCodes[i];
    }
  }

  delete[] mp_fCompMort;
  delete[] mp_fCompMortMax;
  delete[] mp_iGrowthCodes;
  delete[] mp_fXb;
  delete[] mp_fMaxPotentialGrowth;
  delete[] mp_fX0;
  delete[] mp_iIndexes;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clCompetitionMort::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  try {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
    floatVal *p_fTempValues;  //for getting species-specific values
    short int i; //loop counter

    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    mp_iIndexes = new short int[m_iNumTotalSpecies];

    //Make the list of indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //If any of the types is seedling, error out
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clCompetitionMort::DoShellSetup" ;
        stcErr.sMoreInfo = "This behavior cannot be applied to seedlings";
        throw( stcErr );
      }

    //This behavior can only be applied with a one year timestep, therefore throw error if timestep is not 1.
    if ( m_fNumberYearsPerTimestep != 1 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clCompetitionMort::DoShellSetup" ;
      stcErr.sMoreInfo = "This behavior can only be used with a one year timestep";
      throw( stcErr );
    }


    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare the arrays for holding the variables and initilize to null
    mp_fCompMort = new float[m_iNumBehaviorSpecies];
    mp_fCompMortMax = new float[m_iNumBehaviorSpecies];
    mp_fXb = new float[m_iNumBehaviorSpecies];
    mp_fMaxPotentialGrowth = new float[m_iNumBehaviorSpecies];
    mp_fX0 = new float[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fCompMort[i] = 0;
      mp_fCompMortMax[i] = 0;
      mp_fXb[i] = 0;
      mp_fMaxPotentialGrowth[i] = 0;
      mp_fX0[i] = 0;
    }

    //Capture the values from the parameter file

    //Competition-dependent mortality probability function parameters
    FillSpeciesSpecificValue(p_oElement, "mo_CompMort",
        "mo_cmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets and check that they are positive
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fCompMort[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

      if ( mp_fCompMort[mp_iIndexes[p_fTempValues[i].code]] <= 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clCompetitionMort::DoShellSetup" ;
        stcErr.sMoreInfo = "The Competition mortality parameter mp_fCompMort must be a positive number.";
        throw( stcErr );
      }
    }

    //Competition-dependent mortality probability function parameters
    FillSpeciesSpecificValue(p_oElement, "mo_CompMortMax",
        "mo_cmmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets and check that they are positive
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fCompMortMax[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      if ( mp_fCompMortMax[mp_iIndexes[p_fTempValues[i].code]] <= 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clCompetitionMort::DoShellSetup" ;
        stcErr.sMoreInfo = "The Competition mortality parameter mp_fCompMortMax must be a positive number.";
        throw( stcErr );
      }
    }


    p_oElement = p_oDoc->getDocumentElement();
    //Xb - Size effect variance
    FillSpeciesSpecificValue( p_oElement, "gr_nciSizeEffectVariance", "gr_nsevVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fXb[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Maximum potential growth
    FillSpeciesSpecificValue( p_oElement, "gr_nciMaxPotentialGrowth", "gr_nmpgVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxPotentialGrowth[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //X0 - Size effect mode
    FillSpeciesSpecificValue( p_oElement, "gr_nciSizeEffectMode", "gr_nsemVal", p_fTempValues,
        m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fX0[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Get array codes for each "Growth" value
    GetGrowthVariableCodes();

    delete[] p_fTempValues; p_fTempValues = NULL;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clCompetitionMort::DoShellSetup";
    throw(stcErr);
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetGrowthVariableCodes()
//////////////////////////////////////////////////////////////////////////////
void clCompetitionMort::GetGrowthVariableCodes() {
  try {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    std::string sLabel = "Growth";
    short int i, j,//loop counters
    iTotalTypes = p_oPop->GetNumberOfTypes(); //number of tree types

    //Declare and initialize the return codes array
    mp_iGrowthCodes = new short int*[m_iNumTotalSpecies];
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      mp_iGrowthCodes[i] = new short int[iTotalTypes];
      for (j = 0; j < iTotalTypes; j++)
        mp_iGrowthCodes[i][j] = -1;
    }

    //Now go through the growth functions table and get the code for
    //each species/type combo with a valid pointer
    for (i = 0; i < m_iNumTotalSpecies; i++)
      for (j = 0; j < iTotalTypes; j++)
        if (mp_bUsesThisMortality[i][j]) {
          mp_iGrowthCodes[i][j] = p_oPop->GetFloatDataCode(sLabel, i, j);

          //If the return code is -1, throw an error
          if (-1 == mp_iGrowthCodes[i][j]) {
            modelErr stcErr;
            stcErr.sFunction = "clCompetitionMort::GetGrowthVariableCodes";
            std::stringstream s;
            s << "Type/species combo species=" << i << " type=" <<  j
                << " does not have a light behavior compatible with its mortality behavior.";
            stcErr.iErrorCode = BAD_DATA;
            throw(stcErr);
          }
        }

  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clCompetitionMort::GetGrowthVariableCodes";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clCompetitionMort::DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies) {
  float fRandom = clModelMath::GetRand(), //random number
      fGrowth,                //growth value, in mm/year
      fPotentialGrowth,	  //potential growth
      fRelativeIncrement,  //relative increment = growth value (ie Growth)/potential growth
      fDeathProb,          //probability of death for this tree
      fDbhFromBeginningOfTimestep; //in cm.  fDbh value is updated in NCIGrowth().  To calculate fPotentialGrowth, DBH before growth is needed.



  p_oTree->GetValue(mp_iGrowthCodes[p_oTree->GetSpecies()][p_oTree->GetType()], &fGrowth);
  fDbhFromBeginningOfTimestep = fDbh - (fGrowth/10.0); //in cm

  //Calculate the potential growth, in cm
  fPotentialGrowth = mp_fMaxPotentialGrowth[mp_iIndexes[iSpecies]]
                                            * exp( -0.5 * pow( ( log( mp_fX0[mp_iIndexes[iSpecies]] / fDbhFromBeginningOfTimestep ) / mp_fXb[mp_iIndexes[iSpecies]] ), 2 ) );

  //Calculate the relative increment, no units
  //fRelativeIncrement = fGrowth/fPotentialGrowth;

  //Calculate the relative increment, no units - must convert Growth to cm
  fRelativeIncrement = (fGrowth*0.1)/fPotentialGrowth;

  //Shouldn't be a problem but it is reassuring to check!
  //Commented out by LEM per request from RA - 3/25/05
  /*   if ( fRelativeIncrement > 1 )
  {
               modelErr stcErr;
       stcErr.iErrorCode = BAD_DATA;
       stcErr.sFunction = "clCompetitionMort::DoMort" ;
       stcErr.sMoreInfo = "Relative Increment is greater than one";
       throw( stcErr );
  } */


  //Calculate probability of mortality
  if (fRelativeIncrement >= mp_fCompMortMax[mp_iIndexes[iSpecies]])
    fDeathProb = 0;
  else
    fDeathProb = pow(mp_fCompMort[mp_iIndexes[iSpecies]], (fRelativeIncrement/mp_fCompMortMax[mp_iIndexes[iSpecies]]));

  //If Ralative increment is less than mp_fCompMortMax and the probability of death is greater than the random number, kill the
  //tree
  if (fRandom <  fDeathProb)
    return natural;
  else
    return notdead;
}

