//---------------------------------------------------------------------------
#include "NCIMasterMortality.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include "Allometry.h"

#include "NCI/CrowdingEffectBase.h"
#include "NCI/DamageEffectBase.h"
#include "NCI/NCITermBase.h"
#include "NCI/ShadingEffectBase.h"
#include "NCI/SizeEffectBase.h"
#include "NCI/TemperatureEffectBase.h"
#include "NCI/PrecipitationEffectBase.h"
#include "NCI/InfectionEffectBase.h"
#include "NCI/NitrogenEffectBase.h"

#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIMasterMortality::clNCIMasterMortality( clSimManager * p_oSimManager ) :
clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clMortalityBase( p_oSimManager ), clNCIBehaviorBase()
{
  try
  {
    //Set namestring
    m_sNameString = "ncimortshell";
    m_sXMLRoot = "NCIMasterMortality";

    //Version 3
    m_fVersionNumber = 3.0;
    m_fMinimumVersionNumber = 3.0;
    m_fMaxSurvivalPeriod = 1;

    //Null out our pointers
    mp_iDeadCodes = NULL;
    mp_fMaxPotentialValue = NULL;

    m_iNumTotalSpecies = 0;

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
    stcErr.sFunction = "clNCIMasterMortality::clNCIMasterMortality" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clNCIMasterMortality::~clNCIMasterMortality() {

  int i;
  if (mp_iDeadCodes) {
    for ( i = 0; i < m_iNumTotalSpecies; i++ ) {
      delete[] mp_iDeadCodes[i];
    }
    delete[] mp_iDeadCodes;
  }
  delete[] mp_fMaxPotentialValue;
}


////////////////////////////////////////////////////////////////////////////
// GetTreeMemberCodes()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterMortality::GetTreeMemberCodes() {
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  int iNumTypes = p_oPop->GetNumberOfTypes(),
      i, j;

  //Declare the dead codes array
  mp_iDeadCodes = new short int * [m_iNumTotalSpecies];
  for ( i = 0; i < m_iNumTotalSpecies; i++ ) {
    mp_iDeadCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iDeadCodes[i][j] = -1;
    }
  }
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ ) {
    mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] =
        p_oPop->GetIntDataCode( "dead", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );

    if ( -1 == mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                             [mp_whatSpeciesTypeCombos[i].iType] )
    {
      modelErr stcErr;
      stcErr.sFunction = "clNCIMasterMortality::GetTreeMemberCodes" ;
      stcErr.sMoreInfo = "Can't find the \"dead\" data member.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clNCIMasterMortality::DoShellSetup(xercesc::DOMDocument * p_oDoc) {
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  doubleVal * p_fTempValues; //for getting species-specific values
  short int i; //loop counters


  //If any of the types is seedling, error out
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
        && clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCIMasterMortality::DoShellSetup" ;
      stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
      throw( stcErr );
    }

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Maximum survival probability
  mp_fMaxPotentialValue = new double[p_oPop->GetNumberOfSpecies()];
  FillSpeciesSpecificValue( p_oElement, "mo_nciMaxPotentialSurvival",
      "mo_nmpsVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fMaxPotentialValue[p_fTempValues[i].code] = p_fTempValues[i].val;

  delete[] p_fTempValues;

  //Make sure that the maximum survival for each species is between 0 and 1
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
  if ( mp_fMaxPotentialValue[mp_iWhatSpecies[i]] < 0 ||
      mp_fMaxPotentialValue[mp_iWhatSpecies[i]] > 1 ) {
      modelErr stcErr;
      stcErr.sFunction = "clNCIMasterMortality::DoShellSetup";
      stcErr.sMoreInfo = "All values for max survival probability must be between 0 and 1.";
      throw( stcErr );
    }
  }

  //Get survival period length
  FillSingleValue(p_oElement, "mo_nciMortSurvPeriod", &m_fMaxSurvivalPeriod, true);

  //Make sure the value is positive
  if (m_fMaxSurvivalPeriod <= 0) {
    modelErr stcErr;
    stcErr.sFunction = "clNCIMasterMortality::DoShellSetup";
    stcErr.sMoreInfo = "Survival period must be greater than 0.";
    throw( stcErr );
  }

  m_sQuery = FormatSpeciesTypeQueryString();
  ReadParameterFile(p_oElement, p_oPop, this, true);
  GetTreeMemberCodes();

}

////////////////////////////////////////////////////////////////////////////
// DoMort
////////////////////////////////////////////////////////////////////////////
deadCode clNCIMasterMortality::DoMort( clTree * p_oTree, const float & fDbh, const short int & iSpecies )
{
  int iDead;
  p_oTree->GetValue(mp_iDeadCodes[iSpecies][p_oTree->GetType()], &iDead);
  return (deadCode)iDead;
}

////////////////////////////////////////////////////////////////////////////
// PreMortCalcs
////////////////////////////////////////////////////////////////////////////
void clNCIMasterMortality::PreMortCalcs( clTreePopulation * p_oPop )
{
  float *p_fTempEffect,
        *p_fPrecipEffect,
        *p_fNEffect;
  try
  {

#ifdef NCI_WRITER
    using namespace std;
    float fX, fY;
    int iTS = mp_oSimManager->GetCurrentTimestep();
    char cFilename[100];
    sprintf(cFilename, "%s%d%s", "MortNCI", iTS, ".txt");
    fstream out( cFilename, ios::trunc | ios::out );
    out << "Timestep\tSpecies\tDBH\tNCI\tSize Effect\tCrowding Effect\tDamage Effect\tSurv Prob\tDead?\n";
#endif

    clTreeSearch * p_oNCITrees; //trees that this growth behavior applies to
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTree * p_oTree; //a single tree we're working with
    clNCITermBase::ncivals nci;
    float fDamageEffect, //this tree's damage effect
    fCrowdingEffect, //tree's crowding effect
    fSizeEffect, //tree's size effect
    fShadingEffect, //tree's shading effect
    fInfectionEffect, //tree's infection effect
    fDbh, //tree's dbh
    fX, fY,
    fSurvivalProb; //amount diameter increase - intermediate
    int iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(), iDead;
    short int iSpecies, iType, //type and species of a tree
    i; //loop counter
    bool bIsDead;

    p_fPrecipEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fTempEffect = new float[p_oPop->GetNumberOfSpecies()];
    p_fNEffect = new float[p_oPop->GetNumberOfSpecies()];
    //Calculate climate effects for all species
    for (i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fPrecipEffect[mp_iWhatSpecies[i]] = mp_oPrecipEffect->CalculatePrecipitationEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fTempEffect[mp_iWhatSpecies[i]] = mp_oTempEffect->CalculateTemperatureEffect(p_oPlot, mp_iWhatSpecies[i]);
      p_fNEffect[mp_iWhatSpecies[i]] = mp_oNEffect->CalculateNitrogenEffect(p_oPlot, mp_iWhatSpecies[i]);
    }

    //Pre-calcs for other effects
    mp_oCrowdingEffect->PreCalcs(p_oPop);
    mp_oDamageEffect->PreCalcs(p_oPop);
    mp_oNCITerm->PreCalcs(p_oPop);
    mp_oShadingEffect->PreCalcs(p_oPop);
    mp_oSizeEffect->PreCalcs(p_oPop);
    mp_oInfectionEffect->PreCalcs(p_oPop);

    p_oNCITrees = p_oPop->Find( m_sQuery );
    p_oTree = p_oNCITrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      if ( mp_bUsesThisMortality[iSpecies][iType] )
      {

        //Make sure tree's not dead
        p_oTree->GetValue( mp_iDeadCodes[iSpecies][iType], & iDead );

        if ( iDead <= natural )
        {

          p_oTree->GetValue( p_oPop->GetDbhCode( iSpecies, iType ), & fDbh );

          //Get NCI
          p_oTree->GetValue( p_oPop->GetXCode( iSpecies, iType ), & fX );
          p_oTree->GetValue( p_oPop->GetYCode( iSpecies, iType ), & fY );
          nci = mp_oNCITerm->CalculateNCITerm(p_oTree, p_oPop, p_oPlot, fX, fY, iSpecies);

          //Get crowding effect
          fCrowdingEffect = mp_oCrowdingEffect->CalculateCrowdingEffect(p_oTree, fDbh, nci, iSpecies);

          //Get the tree's damage effect
          fDamageEffect = mp_oDamageEffect->CalculateDamageEffect(p_oTree);

          //Get the tree's shading effect
          fShadingEffect = mp_oShadingEffect->CalculateShadingEffect(p_oTree);

          //Get the tree's size effect
          fSizeEffect = mp_oSizeEffect->CalculateSizeEffect(p_oTree, fDbh);

          //Get the tree's infection effect
          fInfectionEffect = mp_oInfectionEffect->CalculateInfectionEffect(p_oTree);

          //Determine whether tree survives
          fSurvivalProb = mp_fMaxPotentialValue[iSpecies] * fSizeEffect *
                fCrowdingEffect * fShadingEffect * fDamageEffect *
                p_fPrecipEffect[iSpecies] * p_fTempEffect[iSpecies] *
                p_fNEffect[iSpecies] * fInfectionEffect;
          //Get annual survival
          fSurvivalProb = pow( fSurvivalProb, 1/m_fMaxSurvivalPeriod);
          //Get timestep survival
          fSurvivalProb = pow( fSurvivalProb, iNumberYearsPerTimestep );

          bIsDead = clModelMath::GetRand() >= fSurvivalProb;

          //Assign the value back to the dead data member
          if (bIsDead) iDead = natural;
          else iDead = notdead;
          p_oTree->SetValue( mp_iDeadCodes[iSpecies][iType], iDead );

#ifdef NCI_WRITER
          //if (8 == p_oTree->GetSpecies()) {
          out << iTS << "\t"  << p_oTree->GetSpecies() << "\t" << fDbh
              << "\t" << fNCI  << "\t" << fSizeEffect << "\t"
              << fCrowdingEffect << "\t" << fDamageEffect << "\t"
              << fSurvivalProb << "\t" << bIsDead << "\n";
          //}
#endif

        } //end of if ( notdead == iDead )
      }

      p_oTree = p_oNCITrees->NextTree();
    }

#ifdef NCI_WRITER
    out.close();
#endif

    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;
  }
  catch ( modelErr & err )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    delete[] p_fNEffect;
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
    stcErr.sFunction = "clNCIMasterMortality::PreMortGrowth" ;
    throw( stcErr );
  }
}
