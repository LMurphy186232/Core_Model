//---------------------------------------------------------------------------
#include "NCIMasterGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include "Allometry.h"

#include "NCI/DefaultCrowdingEffect.h"
#include "NCI/DefaultDamageEffect.h"
#include "NCI/DefaultNCITerm.h"
#include "NCI/DefaultShadingEffect.h"
#include "NCI/DefaultSizeEffect.h"
#include "NCI/NCITermWithNeighborDamage.h"
#include "NCI/NCILargerNeighbors.h"
#include "NCI/NoCrowdingEffect.h"
#include "NCI/NoDamageEffect.h"
#include "NCI/NoNCITerm.h"
#include "NCI/NoShadingEffect.h"
#include "NCI/NoSizeEffect.h"
#include "NCI/NoTemperatureEffect.h"
#include "NCI/WeibullTemperatureEffect.h"
#include "NCI/NoPrecipitationEffect.h"
#include "NCI/WeibullPrecipitationEffect.h"
#include "NCI/SizeEffectLowerBounded.h"
#include "NCI/NoNitrogenEffect.h"
#include "NCI/GaussianNitrogenEffect.h"

#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIMasterGrowth::clNCIMasterGrowth(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clGrowthBase(p_oSimManager) {
  try
  {
    //Set namestring
    m_sNameString = "ncigrowthshell";
    m_sXMLRoot = "NCIMasterGrowth";

    //Null out our pointers
    mp_iGrowthCodes = NULL;

    mp_fMaxPotentialValue = NULL;

    mp_oCrowdingEffect = NULL;
    mp_oDamageEffect = NULL;
    mp_oNCITerm = NULL;
    mp_oShadingEffect = NULL;
    mp_oSizeEffect = NULL;
    mp_oPrecipEffect = NULL;
    mp_oTempEffect = NULL;
    mp_oNEffect = NULL;

    m_iNumTotalSpecies = 0;

    //Version 3
    m_fVersionNumber = 3.0;
    m_fMinimumVersionNumber = 3.0;
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
    stcErr.sFunction = "clNCIMasterGrowth::clNCIMasterGrowth" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clNCIMasterGrowth::~clNCIMasterGrowth() {
  if (mp_iGrowthCodes) {
    for (int i = 0; i < m_iNumTotalSpecies; i++) {
      delete[] mp_iGrowthCodes[i];
    }
    delete[] mp_iGrowthCodes;
  }
  delete[] mp_fMaxPotentialValue;

  delete mp_oCrowdingEffect;
  delete mp_oDamageEffect;
  delete mp_oNCITerm;
  delete mp_oShadingEffect;
  delete mp_oSizeEffect;
  delete mp_oPrecipEffect;
  delete mp_oTempEffect;
  delete mp_oNEffect;
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::ReadParameterFile(xercesc::DOMDocument * p_oDoc) {
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  floatVal * p_fTempValues; //for getting species-specific values
  int iVal;
  short int i; //loop counters

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Maximum potential growth
  mp_fMaxPotentialValue = new float[p_oPop->GetNumberOfSpecies()];
  FillSpeciesSpecificValue( p_oElement, "gr_nciMaxPotentialGrowth", "gr_nmpgVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fMaxPotentialValue[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;

  //Which shading term?
  FillSingleValue(p_oElement, "nciWhichShadingEffect", &iVal, true);
  if (iVal == no_shading) {
    mp_oShadingEffect = new clNoShadingEffect();
  } else if (iVal == default_shading) {
    mp_oShadingEffect = new clDefaultShadingEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterGrowth::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized shading term.";
    throw(err);
  }

  //Which crowding term?
  FillSingleValue(p_oElement, "nciWhichCrowdingEffect", &iVal, true);
  if (iVal == no_crowding_effect) {
    mp_oCrowdingEffect = new clNoCrowdingEffect();
  } else if (iVal == default_crowding_effect) {
    mp_oCrowdingEffect = new clDefaultCrowdingEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterGrowth::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized crowding term.";
    throw(err);
  }

  //Which NCI term?
  FillSingleValue(p_oElement, "nciWhichNCITerm", &iVal, true);
  if (iVal == no_nci_term) {
    mp_oNCITerm = new clNoNCITerm();
  } else if (iVal == default_nci_term) {
    mp_oNCITerm = new clDefaultNCITerm();
  } else if (iVal == nci_with_neighbor_damage) {
    mp_oNCITerm = new clNCITermWithNeighborDamage();
  } else if (iVal == larger_neighbors) {
    mp_oNCITerm = new clNCILargerNeighbors();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterGrowth::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized NCI term.";
    throw(err);
  }

  //Which size effect term?
  FillSingleValue(p_oElement, "nciWhichSizeEffect", &iVal, true);
  if (iVal == no_size_effect) {
    mp_oSizeEffect = new clNoSizeEffect();
  } else if (iVal == default_size_effect) {
    mp_oSizeEffect = new clDefaultSizeEffect();
  } else if (iVal == size_effect_bounded) {
    mp_oSizeEffect = new clSizeEffectLowerBounded();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterGrowth::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized size effect term.";
    throw(err);
  }

  //Which damage effect term?
  FillSingleValue(p_oElement, "nciWhichDamageEffect", &iVal, true);
  if (iVal == no_damage_effect) {
    mp_oDamageEffect = new clNoDamageEffect();
  } else if (iVal == default_damage_effect) {
    mp_oDamageEffect = new clDefaultDamageEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterGrowth::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized damage effect term.";
    throw(err);
  }

  //Which precipitation effect term?
  FillSingleValue(p_oElement, "nciWhichPrecipitationEffect", &iVal, true);
  if (iVal == no_precip_effect) {
    mp_oPrecipEffect = new clNoPrecipitationEffect();
  } else if (iVal == weibull_precip_effect) {
    mp_oPrecipEffect = new clWeibullPrecipitationEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterGrowth::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized precipitation effect term.";
    throw(err);
  }

  //Which temperature effect term?
  FillSingleValue(p_oElement, "nciWhichTemperatureEffect", &iVal, true);
  if (iVal == no_temp_effect) {
    mp_oTempEffect = new clNoTemperatureEffect();
  } else if (iVal == weibull_temp_effect) {
    mp_oTempEffect = new clWeibullTemperatureEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterGrowth::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized temperature effect term.";
    throw(err);
  }

  //Which nitrogen effect term?
  FillSingleValue(p_oElement, "nciWhichNitrogenEffect", &iVal, true);
  if (iVal == no_nitrogen_effect) {
    mp_oNEffect = new clNoNitrogenEffect();
  } else if (iVal == gauss_nitrogen_effect) {
    mp_oNEffect = new clGaussianNitrogenEffect();
  } else {
    modelErr err;
    err.sFunction = "clNCIMasterGrowth::ReadParameterFile";
    err.iErrorCode = BAD_DATA;
    err.sMoreInfo = "Unrecognized nitrogen effect term.";
    throw(err);
  }

  mp_oCrowdingEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oDamageEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oNCITerm->DoSetup(p_oPop, this, p_oElement);
  mp_oShadingEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oSizeEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oPrecipEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oTempEffect->DoSetup(p_oPop, this, p_oElement);
  mp_oNEffect->DoSetup(p_oPop, this, p_oElement);
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::ValidateData() {
  int i;
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    //Make sure that the maximum growth for each species is > 0
    if (mp_fMaxPotentialValue[mp_iWhatSpecies[i]] <= 0) {
      modelErr err;
      err.sFunction = "clNCIMasterGrowth::ValidateData";
      err.iErrorCode = BAD_DATA;
      err.sMoreInfo = "All values for max potential growth must be greater than 0.";
      throw(err);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// GetTreeMemberCodes()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::GetTreeMemberCodes() {
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  int iNumTypes = p_oPop->GetNumberOfTypes(),
      i, j;

  //Get codes for growth
  mp_iGrowthCodes = new short int * [m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_iGrowthCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iGrowthCodes[i][j] = -1;
    }
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    //Get the code from growth org
    mp_iGrowthCodes[mp_whatSpeciesTypeCombos[i].iSpecies][mp_whatSpeciesTypeCombos[i].iType]
        = mp_oGrowthOrg->GetGrowthCode(mp_whatSpeciesTypeCombos[i].iSpecies,
            mp_whatSpeciesTypeCombos[i].iType);
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::DoShellSetup(xercesc::DOMDocument * p_oDoc) {
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  FormatQueryString();
  ReadParameterFile( p_oDoc );
  ValidateData();
  GetTreeMemberCodes();

}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::FormatQueryString()
{
  std::stringstream sQueryTemp;
  int i;
  bool bSapling = false, bAdult = false, bSnag = false;

  //Do a type/species search on all the types and species
  sQueryTemp << "species=";
  for (i = 0; i < m_iNumBehaviorSpecies - 1; i++) {
    sQueryTemp << mp_iWhatSpecies[i] << ",";
  }
  sQueryTemp << mp_iWhatSpecies[m_iNumBehaviorSpecies - 1];

  //Find all the types
  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType ) {
      bSapling = true;
    } else if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType ) {
      bAdult = true;
    } else if ( clTreePopulation::snag == mp_whatSpeciesTypeCombos[i].iType ) {
      bSnag = true;
    }
  }
  sQueryTemp << "::type=";
  if (bSapling) {
    sQueryTemp << clTreePopulation::sapling << ",";
  } if (bAdult) {
    sQueryTemp << clTreePopulation::adult << ",";
  } if (bSnag) {
    sQueryTemp << clTreePopulation::snag << ",";
  }

  //Remove the last comma and put it in m_sQuery
  m_sQuery = sQueryTemp.str().substr(0, sQueryTemp.str().length() - 1);
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::SetNameData(std::string sNameString) {
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("NCIMasterGrowth") == 0 )
    {
      m_iGrowthMethod = diameter_auto;
    }
    else if (sNameString.compare("NCIMasterGrowth diam only") == 0 )
    {
      m_iGrowthMethod = diameter_only;
    }
    else
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sMoreInfo = s.str();
      stcErr.sFunction = "clNCIMasterGrowth::SetNameData" ;
      throw( stcErr );
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
    stcErr.sFunction = "clNCIMasterGrowth::SetNameData" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// CalcGrowthValue
////////////////////////////////////////////////////////////////////////////
float clNCIMasterGrowth::CalcDiameterGrowthValue(clTree * p_oTree,
    clTreePopulation * p_oPop, float fHeightGrowth) {
  float fAmountDiamIncrease; //amount diameter increase

  //Get the tree's growth - it's already calculated
  p_oTree->GetValue( mp_iGrowthCodes[p_oTree->GetSpecies()][p_oTree->GetType()],
      & fAmountDiamIncrease );

  return fAmountDiamIncrease;
}

////////////////////////////////////////////////////////////////////////////
// PreGrowthCalcs
////////////////////////////////////////////////////////////////////////////
void clNCIMasterGrowth::PreGrowthCalcs(clTreePopulation * p_oPop) {
  float *p_fTempEffect,
        *p_fPrecipEffect,
        *p_fNEffect;
  try
  {
#ifdef NCI_WRITER
    using namespace std;
    int iTS = mp_oSimManager->GetCurrentTimestep();
    char cFilename[100];
    sprintf(cFilename, "%s%d%s", "GrowthNCI", iTS, ".txt");
    fstream out( cFilename, ios::trunc | ios::out );
    out << "Timestep\tSpecies\tDBH\tNCI\tSize Effect\tCrowding Effect\tDamage Effect\tGrowth\n";
#endif

    clTreeSearch * p_oNCITrees; //trees that this growth behavior applies to
    clAllometry * p_oAllom = p_oPop->GetAllometryObject();
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTree * p_oTree; //a single tree we're working with
    float fDamageEffect, //this tree's damage effect
    fCrowdingEffect, //tree's crowding effect
    fNCI, //the NCI
    fSizeEffect, //tree's size effect
    fShadingEffect, //tree's shading effect
    fDiam, //tree's diameter
    fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(), fAmountDiamIncrease, //amount diameter increase
    fTempDiamIncrease; //amount diameter increase - intermediate
    int iIsDead;
    short int iSpecies, iType, //type and species of a tree
    i, //loop counter
    iDeadCode; //tree's dead code

    p_fPrecipEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fTempEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fNEffect = new float[p_oPop->GetNumberOfSpecies()];
    //Calculate climate effects for all species
    for (i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fPrecipEffect[mp_iWhatSpecies[i]] = mp_oPrecipEffect->CalculatePrecipitationEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fTempEffect[mp_iWhatSpecies[i]] = mp_oTempEffect->CalculateTemperatureEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fNEffect[mp_iWhatSpecies[i]] = mp_oNEffect->CalculateNitrogenEffect(p_oPlot, mp_iWhatSpecies[i]);
    }

    p_oNCITrees = p_oPop->Find( m_sQuery );

    //************************************
    // Loop through and to calculate growth for each tree
    //************************************
    p_oTree = p_oNCITrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      if ( -1 != mp_iGrowthCodes[iSpecies][iType]) {

        //Make sure tree's not dead
        iDeadCode = p_oPop->GetIntDataCode( "dead", p_oTree->GetSpecies(), p_oTree->GetType() );
        if ( -1 != iDeadCode )
        {
          p_oTree->GetValue( iDeadCode, & iIsDead );
        }
        else
        iIsDead = notdead;

        if (notdead == iIsDead) {

          if (iType == clTreePopulation::seedling) {
            p_oTree->GetValue(p_oPop->GetDiam10Code(iSpecies, iType), &fDiam);
          } else {
            p_oTree->GetValue(p_oPop->GetDbhCode(iSpecies, iType), &fDiam);
          }

          //First calculate the pieces that have no DBH component and thus will
          //not change in our loop

          //Get NCI
          fNCI = mp_oNCITerm->CalculateNCITerm(p_oTree, p_oPop, p_oPlot);

          //Get the tree's damage effect
          fDamageEffect = mp_oDamageEffect->CalculateDamageEffect(p_oTree);

          //Get the tree's shading effect
          fShadingEffect = mp_oShadingEffect->CalculateShadingEffect(p_oTree);

          //To correctly compound growth over the number of years per timestep,
          //we have to loop over the number of years, re-calculating the parts
          //with DBH and incrementing the DBH each time
          fAmountDiamIncrease = 0;
          for ( i = 0; i < fNumberYearsPerTimestep; i++ ) {

            fCrowdingEffect = mp_oCrowdingEffect->CalculateCrowdingEffect(p_oTree, fDiam, fNCI);

            //Get the tree's size effect
            fSizeEffect = mp_oSizeEffect->CalculateSizeEffect(iSpecies, fDiam);

            //Calculate actual growth in cm/yr
            fTempDiamIncrease = mp_fMaxPotentialValue[iSpecies] * fSizeEffect *
                fCrowdingEffect * fShadingEffect * fDamageEffect *
                p_fPrecipEffect[iSpecies] * p_fTempEffect[iSpecies] * p_fNEffect[iSpecies];

            //Add it to the running total of diameter increase
            fAmountDiamIncrease += fTempDiamIncrease;

            //Increase the DBH for the next loop.  If this is a sapling,
            //convert to a dbh value from what would be a diam10 increase
            if ( clTreePopulation::sapling == iType )
            {
              fDiam += p_oAllom->ConvertDiam10ToDbh( fTempDiamIncrease, iSpecies );
            }
            else
            {
              fDiam += fTempDiamIncrease;
            }
          }

          //Assign the growth back to "Growth" and hold it
          p_oTree->SetValue( mp_iGrowthCodes[iSpecies][iType], fAmountDiamIncrease );

#ifdef NCI_WRITER
            p_oTree->GetValue( p_oPop->GetDbhCode( iSpecies, iType ), & fDbh );
            out << iTS << "\t" << p_oTree->GetSpecies() << "\t" << fDbh
                << "\t" << fNCI << "\t" << fSizeEffect << "\t" << fCrowdingEffect
                << "\t" << fDamageEffect << "\t" << fAmountDiamIncrease << "\n";
#endif

        } //end of if (bIsNCITree)
      }

      p_oTree = p_oNCITrees->NextTree();
    }

    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;

#ifdef NCI_WRITER
    out.close();
#endif

  }
  catch ( modelErr & err )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;
    throw( err );
  }
  catch ( ... )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clNCIMasterGrowth::PreCalcGrowth" ;
    throw( stcErr );
  }
}

