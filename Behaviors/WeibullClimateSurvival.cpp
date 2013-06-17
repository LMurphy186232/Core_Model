//---------------------------------------------------------------------------
#include "WeibullClimateSurvival.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "Allometry.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clWeibullClimateSurvival::clWeibullClimateSurvival( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
  clMortalityBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "WeibullClimatemortshell";
    m_sXMLRoot = "WeibullClimateSurvival";

    //Null out our pointers
    mp_fCompC = NULL;
    mp_fCompD = NULL;
    mp_fGamma = NULL;
    mp_fMaxCrowdingRadius = NULL;
    mp_fMinimumNeighborDBH = NULL;
    mp_fSizeX0 = NULL;
    mp_fSizeXb = NULL;
    mp_fSizeMinDBH = NULL;
    mp_fPrecipA = NULL;
    mp_fPrecipB = NULL;
    mp_fPrecipC = NULL;
    mp_fTempA = NULL;
    mp_fTempB = NULL;
    mp_fTempC = NULL;
    mp_fMaxRG = NULL;
    mp_iIndexes = NULL;
    mp_fPrecipEffect = NULL;
    mp_fTempEffect = NULL;

    m_fMinSaplingHeight = 0;
    m_iNumTotalSpecies = 0;

    //Version 1
    m_fVersionNumber = 1.0;
    m_fMinimumVersionNumber = 1.0;
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
    stcErr.sFunction = "clWeibullClimateSurvival::clWeibullClimateSurvival" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clWeibullClimateSurvival::~clWeibullClimateSurvival()
{
  delete[] mp_fCompC;
  delete[] mp_fCompD;
  delete[] mp_fGamma;
  delete[] mp_fMaxCrowdingRadius;
  delete[] mp_fMinimumNeighborDBH;
  delete[] mp_fSizeX0;
  delete[] mp_fSizeXb;
  delete[] mp_fSizeMinDBH;
  delete[] mp_fPrecipA;
  delete[] mp_fPrecipB;
  delete[] mp_fPrecipC;
  delete[] mp_fTempA;
  delete[] mp_fTempB;
  delete[] mp_fTempC;
  delete[] mp_fMaxRG;
  delete[] mp_iIndexes;
  delete[] mp_fPrecipEffect;
  delete[] mp_fTempEffect;
}


////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateSurvival::ReadParameterFile( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values
    doubleVal *p_dTempValues;
    short int i; //loop counter

    m_fMinSaplingHeight = 50;

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    //Get the minimum sapling height
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
      {
        m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
      }
    }

    //If any of the types is seedling, error out
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
           && clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType )
           {
             modelErr stcErr;
             stcErr.iErrorCode = BAD_DATA;
             stcErr.sFunction = "clWeibullClimateSurvival::ReadParameterFile" ;
             stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
             throw( stcErr );
      }

    //Make the list of indexes
    mp_iIndexes = new short int[m_iNumTotalSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //This alone is sized total number of species
    mp_fMinimumNeighborDBH = new float[m_iNumTotalSpecies];

    //The rest are sized number of species to which this behavior applies
    mp_fCompC = new float[m_iNumBehaviorSpecies];
    mp_fCompD = new float[m_iNumBehaviorSpecies];
    mp_fGamma = new float[m_iNumBehaviorSpecies];
    mp_fPrecipA = new float[m_iNumBehaviorSpecies];
    mp_fPrecipB = new float[m_iNumBehaviorSpecies];
    mp_fPrecipC = new float[m_iNumBehaviorSpecies];
    mp_fTempA = new float[m_iNumBehaviorSpecies];
    mp_fTempB = new float[m_iNumBehaviorSpecies];
    mp_fTempC = new float[m_iNumBehaviorSpecies];
    mp_fMaxRG = new float[m_iNumBehaviorSpecies];
    mp_fSizeX0 = new double [m_iNumBehaviorSpecies];
    mp_fSizeXb = new float[m_iNumBehaviorSpecies];

    mp_fMaxCrowdingRadius = new float[m_iNumBehaviorSpecies];
    mp_fSizeMinDBH = new float[m_iNumBehaviorSpecies];

    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    p_dTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fTempValues[i].code = mp_iWhatSpecies[i];
      p_dTempValues[i].code = mp_iWhatSpecies[i];
    }

    //Fill the variables

    //Maximum potential survival
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimMaxSurv",
        "mo_wcmsVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxRG[i] = p_fTempValues[i].val;

    //Max crowding radius
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimMaxNeighRad", "mo_wcmnrVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxCrowdingRadius[i] = p_fTempValues[i].val;

    //Size sensitivity to crowding parameter (gamma)
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimCompEffGamma",
        "mo_wccegVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fGamma[i] = p_fTempValues[i].val;

    //Minimum neighbor DBH
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimMinNeighDBH",
        "mo_wcmndVal", mp_fMinimumNeighborDBH, p_oPop, true );

    //C
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimCompEffC",
        "mo_wccecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCompC[i] = p_fTempValues[i].val;

    //D
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimCompEffD",
        "mo_wccedVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCompD[i] = p_fTempValues[i].val;

    //Size effect mode (X0)
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimSizeEffX0",
        "mo_wcsex0Val", p_dTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSizeX0[i] = p_dTempValues[i].val;

    //Size effect variance (Xb)
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimSizeEffXb",
        "mo_wcsexbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSizeXb[i] = p_fTempValues[i].val;

    //Size effect minimum DBH
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimSizeEffMinDBH",
        "mo_wcsemdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSizeMinDBH[i] = p_fTempValues[i].val;

    //Temperature effect a
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimTempEffA",
        "mo_wcteaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempA[i] = p_fTempValues[i].val;

    //Temperature effect b
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimTempEffB",
        "mo_wctebVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempB[i] = p_fTempValues[i].val;

    //Temperature effect c
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimTempEffC",
        "mo_wctecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempC[i] = p_fTempValues[i].val;

    //Precipitation effect a
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimPrecipEffA",
        "mo_wcpeaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipA[i] = p_fTempValues[i].val;

    //Precipitation effect b
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimPrecipEffB",
        "mo_wcpebVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipB[i] = p_fTempValues[i].val;

    //Precipitation effect c
    FillSpeciesSpecificValue( p_oElement, "mo_weibClimPrecipEffC",
        "mo_wcpecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipC[i] = p_fTempValues[i].val;

    delete[] p_fTempValues;
    delete[] p_dTempValues;
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
    stcErr.sFunction = "clWeibullClimateSurvival::ReadParameterFile" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateSurvival::ValidateData()
{
  try
  {

    int i;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {

      //Make sure that the max radius of neighbor effects is > 0
      if ( mp_fMaxCrowdingRadius[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max crowding radius must be greater than 0.";
        throw( stcErr );
      }

      //Make sure that the maximum survival for each species is between 0 and 1
      if ( mp_fMaxRG[i] < 0 || mp_fMaxRG[i] > 1)
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max potential survival must be between 0 and 1.";
        throw( stcErr );
      }

      //Make sure that the size effect mode is not 0
      if ( 0 >= mp_fSizeX0[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Size effect mode values must be greater than 0.";
        throw( stcErr );
      }

      //Make sure that the size effect variance is not 0
      if ( 0 == mp_fSizeXb[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Size effect variance values cannot be 0.";
        throw( stcErr );
      }

      //Make sure that the temp a is not 0
      if ( 0 == mp_fTempA[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Temperature effect A values cannot be 0.";
        throw( stcErr );
      }

      //Make sure that the precip a is not 0
      if ( 0 == mp_fPrecipA[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Precipitation effect A values cannot be 0.";
        throw( stcErr );
      }
    }

    for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      //Make sure that the minimum neighbor DBH is not negative
      if ( 0 > mp_fMinimumNeighborDBH[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Minimum neighbor DBH for NCI cannot be less than 0.";
        throw( stcErr );
      }
    }
  }
  catch ( modelErr & err )
  {
    err.sFunction = "clWeibullClimateSurvival::ValidateData";
    err.iErrorCode = BAD_DATA;
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
    stcErr.sFunction = "clWeibullClimateSurvival::ValidateData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateSurvival::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  ReadParameterFile( p_oDoc );
  ValidateData();

  mp_fTempEffect = new float[m_iNumBehaviorSpecies];
  mp_fPrecipEffect = new float[m_iNumBehaviorSpecies];

}

////////////////////////////////////////////////////////////////////////////
// DoMort
////////////////////////////////////////////////////////////////////////////
deadCode clWeibullClimateSurvival::DoMort( clTree * p_oTree, const float & fDbh, const short int & iSpecies )
{

    float fCrowdingEffect, //tree's crowding effect
          fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(),
          fSizeEffect, //tree's size effect
          fSEDbh,   //DBH for the purposes of calculating the size effect
          fSurvivalProb; //Tree's annual survival probability
    int iNumBiggerNeighbors;
    short int iSpInd;
    bool bIsDead;

    //Get a number of neighbors with a DBH greater than the target
    iNumBiggerNeighbors = GetNumLargerNeighbors(p_oTree);

    iSpInd = mp_iIndexes[iSpecies];


    fCrowdingEffect = exp(-mp_fCompC[iSpInd] *
                 pow(fDbh, mp_fGamma[iSpInd]) *
                 pow(iNumBiggerNeighbors, mp_fCompD[iSpInd]));
    //Make sure it's between 0 and 1
    if ( fCrowdingEffect < 0 ) fCrowdingEffect = 0;
    if ( fCrowdingEffect > 1 ) fCrowdingEffect = 1;

    //Get the tree's size effect
    if (fDbh < mp_fSizeMinDBH[iSpInd])
      fSEDbh = mp_fSizeMinDBH[iSpInd];
    else
      fSEDbh = fDbh;
    fSizeEffect = exp( -0.5 * pow(
        log( fSEDbh / mp_fSizeX0[iSpInd] ) /
             mp_fSizeXb[iSpInd], 2 ) );

    //Make sure it's bounded between 0 and 1
    if ( fSizeEffect < 0 ) fSizeEffect = 0;
    if ( fSizeEffect > 1 ) fSizeEffect = 1;

    //Calculate annual survival probability
    fSurvivalProb = mp_fMaxRG[iSpInd] *
                        fSizeEffect *
                        fCrowdingEffect *
                        mp_fPrecipEffect[iSpInd] *
                        mp_fTempEffect[iSpInd];

    fSurvivalProb = pow( fSurvivalProb, fNumberYearsPerTimestep );
    bIsDead = clModelMath::GetRand() >= fSurvivalProb;

    if (bIsDead) return natural;
    else return notdead;

}



////////////////////////////////////////////////////////////////////////////
// GetNumLargerNeighbors
////////////////////////////////////////////////////////////////////////////
int clWeibullClimateSurvival::GetNumLargerNeighbors(clTree *p_oTarget)
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  char cQuery[75]; //format search strings into this
  float fNeighDbh, //neighbor's dbh
        fTargetDbh, //target's dbh
        fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iIsDead, //whether a neighbor is dead
      iNumNeighbors = 0;
  short int iNeighSpecies, iNeighType, //species and type for neighbor
       iTargetSpecies = p_oTarget->GetSpecies(), //target tree's species
       iTargetType = p_oTarget->GetType(), //target tree's type
       iDeadCode; //neighbor's dead code

    //Get the target tree's DBH
    p_oTarget->GetValue( p_oPop->GetDbhCode( iTargetSpecies, iTargetType ), & fTargetDbh );

    //Format the query to get all competing neighbors
    p_oTarget->GetValue( p_oPop->GetXCode( iTargetSpecies, iTargetType ), & fTargetX );
    p_oTarget->GetValue( p_oPop->GetYCode( iTargetSpecies, iTargetType ), & fTargetY );

    //Get all trees taller than seedlings within the max crowding radius -
    //seedlings don't compete
    sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", mp_fMaxCrowdingRadius[mp_iIndexes[iTargetSpecies]], "FROM x=", fTargetX,
         "y=", fTargetY, "::height=", m_fMinSaplingHeight );
    p_oAllNeighbors = p_oPop->Find( cQuery );

    //Loop through and find all the bigger neighbors
    p_oNeighbor = p_oAllNeighbors->NextTree();

    while ( p_oNeighbor ) {

      iNeighSpecies = p_oNeighbor->GetSpecies();
      iNeighType = p_oNeighbor->GetType();

      if ( clTreePopulation::seedling != iNeighType &&
           clTreePopulation::snag != iNeighType) {

        //Get the neighbor's dbh
        p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fNeighDbh );

        if ( fNeighDbh >= mp_fMinimumNeighborDBH[iNeighSpecies] &&
             fNeighDbh > fTargetDbh) {

          //Make sure the neighbor's not dead due to a disturbance event
          iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
          if ( -1 != iDeadCode )  p_oNeighbor->GetValue( iDeadCode, & iIsDead );
          else iIsDead = notdead;

          if ( notdead == iIsDead || natural == iIsDead)
            iNumNeighbors++;
        }
      }

      p_oNeighbor = p_oAllNeighbors->NextTree();
    }
    return iNumNeighbors;
}

////////////////////////////////////////////////////////////////////////////
// PreMortCalcs
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateSurvival::PreMortCalcs( clTreePopulation * p_oPop )
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  float fPlotTemp = p_oPlot->GetMeanAnnualTemp(), //Current plot temperature
        fPlotPrecip = p_oPlot->GetMeanAnnualPrecip(); //Current plot precipitation
  int j;

  for ( j = 0; j < m_iNumBehaviorSpecies; j++ ) {
    mp_fPrecipEffect[j] = exp(-0.5*pow(fabs(fPlotPrecip - mp_fPrecipC[j])/mp_fPrecipA[j], mp_fPrecipB[j]));
    mp_fTempEffect[j] = exp(-0.5*pow(fabs(fPlotTemp - mp_fTempC[j])/mp_fTempA[j], mp_fTempB[j]));
  }
}
