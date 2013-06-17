//---------------------------------------------------------------------------
#include "WeibullClimateGrowth_bak.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "GrowthOrg.h"
#include "Allometry.h"
#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clWeibullClimateGrowth::clWeibullClimateGrowth( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
  clGrowthBase( p_oSimManager )
{
  try
  {
    //Set namestring
    m_sNameString = "WeibullClimategrowthshell";
    m_sXMLRoot = "WeibullClimateGrowth";

    //Null out our pointers
    mp_iGrowthCodes = NULL;
    mp_fCompC = NULL;
    mp_fCompD = NULL;
    mp_fGamma = NULL;
    //mp_fMaxCrowdingRadius = NULL;
    //mp_fMinimumNeighborDBH = NULL;
    //mp_fSizeX0 = NULL;
    //mp_fSizeXb = NULL;
    //mp_fSizeMinDBH = NULL;
    //mp_fPrecipA = NULL;
    //mp_fPrecipB = NULL;
    //mp_fPrecipC = NULL;
    //mp_fTempA = NULL;
    //mp_fTempB = NULL;
    //mp_fTempC = NULL;
    mp_fMaxRG = NULL;
    mp_iIndexes = NULL;
    m_cQuery = NULL;

    //m_fMinSaplingHeight = 0;
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
    stcErr.sFunction = "clWeibullClimateGrowth::clWeibullClimateGrowth" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clWeibullClimateGrowth::~clWeibullClimateGrowth()
{
  int i;
  if ( mp_iGrowthCodes ) {
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      delete[] mp_iGrowthCodes[i];
  }
  delete[] mp_iGrowthCodes;
  delete[] mp_fCompC;
  delete[] mp_fCompD;
  delete[] mp_fGamma;
  //delete[] mp_fPrecipA;
  //delete[] mp_fPrecipB;
  //delete[] mp_fPrecipC;
  //delete[] mp_fTempA;
  //delete[] mp_fTempB;
  //delete[] mp_fTempC;
  //delete[] mp_fMaxCrowdingRadius;
  //delete[] mp_fMinimumNeighborDBH;
  //delete[] mp_fSizeX0;
  //delete[] mp_fSizeXb;
  //delete[] mp_fSizeMinDBH;
  delete[] mp_fMaxRG;
  delete[] mp_iIndexes;
  delete[] m_cQuery;
}


////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateGrowth::ReadParameterFile( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    floatVal * p_fTempValues; //for getting species-specific values
    doubleVal * p_dTempValues; //for getting species-specific values
    short int i; //loop counter

    //m_fMinSaplingHeight = 50;

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    //Get the minimum sapling height
    /*for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
      {
        m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
      }
    }*/

    //If any of the types is seedling, error out
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
           && clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType )
           {
             modelErr stcErr;
             stcErr.iErrorCode = BAD_DATA;
             stcErr.sFunction = "clWeibullClimateGrowth::ReadParameterFile" ;
             stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
             throw( stcErr );
      }

    //Make the list of indexes
    mp_iIndexes = new short int[m_iNumTotalSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //This alone is sized total number of species
    //mp_fMinimumNeighborDBH = new float[m_iNumTotalSpecies];

    //The rest are sized number of species to which this behavior applies
    mp_fCompC = new float[m_iNumBehaviorSpecies];
    mp_fCompD = new float[m_iNumBehaviorSpecies];
    mp_fGamma = new float[m_iNumBehaviorSpecies];
    //mp_fPrecipA = new float[m_iNumBehaviorSpecies];
    //mp_fPrecipB = new float[m_iNumBehaviorSpecies];
    //mp_fPrecipC = new float[m_iNumBehaviorSpecies];
    //mp_fTempA = new float[m_iNumBehaviorSpecies];
    //mp_fTempB = new float[m_iNumBehaviorSpecies];
    //mp_fTempC = new float[m_iNumBehaviorSpecies];
    mp_fMaxRG = new float[m_iNumBehaviorSpecies];
    //mp_fMaxCrowdingRadius = new float[m_iNumBehaviorSpecies];
    //mp_fSizeX0 = new double[m_iNumBehaviorSpecies];
    //mp_fSizeXb = new float[m_iNumBehaviorSpecies];
    //mp_fSizeMinDBH = new float[m_iNumBehaviorSpecies];

    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    p_dTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_fTempValues[i].code = mp_iWhatSpecies[i];
      p_dTempValues[i].code = mp_iWhatSpecies[i];
    }

    //Fill the variables

    //Maximum potential growth
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimMaxGrowth", "gr_wcmgVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxRG[i] = p_fTempValues[i].val;

    //Max crowding radius
    /*FillSpeciesSpecificValue( p_oElement, "gr_weibClimMaxNeighRad", "gr_wcmnrVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMaxCrowdingRadius[i] = p_fTempValues[i].val;*/

    //Size sensitivity to crowding parameter (gamma)
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimCompEffGamma",
        "gr_wccegVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fGamma[i] = p_fTempValues[i].val;

    //Minimum neighbor DBH
    /*FillSpeciesSpecificValue( p_oElement, "gr_weibClimMinNeighDBH",
        "gr_wcmndVal", mp_fMinimumNeighborDBH, p_oPop, true );*/

    //C
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimCompEffC",
        "gr_wccecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCompC[i] = p_fTempValues[i].val;

    //D
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimCompEffD",
        "gr_wccedVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCompD[i] = p_fTempValues[i].val;

    //Size effect mode (X0)
    /*FillSpeciesSpecificValue( p_oElement, "gr_weibClimSizeEffX0",
        "gr_wcsex0Val", p_dTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSizeX0[i] = p_dTempValues[i].val;

    //Size effect variance (Xb)
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimSizeEffXb",
        "gr_wcsexbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSizeXb[i] = p_fTempValues[i].val;

    //Size effect minimum DBH
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimSizeEffMinDBH",
        "gr_wcsemdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSizeMinDBH[i] = p_fTempValues[i].val;*/

    //Temperature effect a
    /*FillSpeciesSpecificValue( p_oElement, "gr_weibClimTempEffA",
        "gr_wcteaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempA[i] = p_fTempValues[i].val;

    //Temperature effect b
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimTempEffB",
        "gr_wctebVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempB[i] = p_fTempValues[i].val;

    //Temperature effect c
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimTempEffC",
        "gr_wctecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTempC[i] = p_fTempValues[i].val; */

    /*//Precipitation effect a
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimPrecipEffA",
        "gr_wcpeaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipA[i] = p_fTempValues[i].val;

    //Precipitation effect b
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimPrecipEffB",
        "gr_wcpebVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipB[i] = p_fTempValues[i].val;

    //Precipitation effect c
    FillSpeciesSpecificValue( p_oElement, "gr_weibClimPrecipEffC",
        "gr_wcpecVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPrecipC[i] = p_fTempValues[i].val; */

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
    stcErr.sFunction = "clWeibullClimateGrowth::ReadParameterFile" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateGrowth::ValidateData()
{
  try
  {

    int i;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {

      //Make sure that the max radius of neighbor effects is > 0
      /*if ( mp_fMaxCrowdingRadius[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max crowding radius must be greater than 0.";
        throw( stcErr );
      }*/

      //Make sure that the maximum growth for each species is > 0
      if ( mp_fMaxRG[i] <= 0 )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "All values for max potential growth must be greater than 0.";
        throw( stcErr );
      }

      /*//Make sure that the size effect mode is not 0
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
      }*/

      //Make sure that the temp a is not 0
     /* if ( 0 == mp_fTempA[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Temperature effect A values cannot be 0.";
        throw( stcErr );
      }*/

      //Make sure that the precip a is not 0
      /*if ( 0 == mp_fPrecipA[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Precipitation effect A values cannot be 0.";
        throw( stcErr );
      }*/
    }

    /*for ( i = 0; i < m_iNumTotalSpecies; i++ )
    {
      //Make sure that the minimum neighbor DBH is not negative
      if ( 0 > mp_fMinimumNeighborDBH[i] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "Minimum neighbor DBH for NCI cannot be less than 0.";
        throw( stcErr );
      }
    }*/
  }
  catch ( modelErr & err )
  {
    err.sFunction = "clWeibullClimateGrowth::ValidateData";
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
    stcErr.sFunction = "clWeibullClimateGrowth::ValidateData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetGrowthCodes()
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateGrowth::GetGrowthCodes()
{
  int i;

  //Get codes for growth
  mp_iGrowthCodes = new short int * [m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    mp_iGrowthCodes[i] = new short int[2];
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {

    //Get the code from growth org
    mp_iGrowthCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
         [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
         mp_oGrowthOrg->GetGrowthCode( mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateGrowth::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    FormatQuery();
    ReadParameterFile( p_oDoc );
    ValidateData();
    GetGrowthCodes();
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
    stcErr.sFunction = "clWeibullClimateGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clWeibullClimateGrowth::SetNameData(std::string sNameString)
{
  try
  {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("WeibullClimateGrowth") == 0 )
    {
      m_iGrowthMethod = diameter_auto;
    }
    else if (sNameString.compare("WeibullClimateGrowth diam only") == 0 )
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
      stcErr.sFunction = "clWeibullClimateGrowth::SetNameData" ;
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
    stcErr.sFunction = "clWeibullClimateGrowth::SetNameData" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// CalcGrowthValue
////////////////////////////////////////////////////////////////////////////
float clWeibullClimateGrowth::CalcDiameterGrowthValue( clTree * p_oTree, clTreePopulation * p_oPop, float fHeightGrowth )
{
  float fAmountDiamIncrease; //amount diameter increase

  //Get the tree's growth - it's already calculated
  p_oTree->GetValue( mp_iGrowthCodes[mp_iIndexes[p_oTree->GetSpecies()]] [p_oTree->GetType() - clTreePopulation::sapling],
      & fAmountDiamIncrease );

  return fAmountDiamIncrease;
}

////////////////////////////////////////////////////////////////////////////
// PreGrowthCalcs
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateGrowth::PreGrowthCalcs( clTreePopulation * p_oPop )
{
  float *p_fTempEffect,
        *p_fPrecipEffect;
  int i;
  try
  {
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    clTreeSearch * p_oNCITrees; //trees that this growth behavior applies to
    clAllometry * p_oAllom = p_oPop->GetAllometryObject();
    clTree * p_oTree; //a single tree we're working with
    float fCrowdingEffect, //tree's crowding effect
          fPlotTemp = p_oPlot->GetMeanAnnualTemp(), //Current plot temperature
          fPlotPrecip = p_oPlot->GetMeanAnnualPrecip(), //Current plot precipitation
          fSizeEffect, //tree's size effect
          fDbh, //tree's dbh
          fSEDbh, //DBH for the purposes of calculating the size effect
          fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(), fAmountDiamIncrease, //amount diameter increase
          fTempDiamIncrease; //amount diameter increase - intermediate
    int iIsDead, //whether a tree is dead
        iNumBiggerNeighbors;
    short int iSpecies, iType, //type and species of a tree
              iSpInd,
              j,
              iDeadCode; //tree's dead code

    p_fPrecipEffect = new float[m_iNumBehaviorSpecies];
    p_fTempEffect = new float[m_iNumBehaviorSpecies];

    //Calculate precipitation and temperature effect for all species
    for ( j = 0; j < m_iNumBehaviorSpecies; j++ ) {
      //p_fPrecipEffect[j] = exp(-0.5*pow(fabs(fPlotPrecip - mp_fPrecipC[j])/mp_fPrecipA[j], mp_fPrecipB[j]));
      //p_fTempEffect[j] = exp(-0.5*pow(fabs(fPlotTemp - mp_fTempC[j])/mp_fTempA[j], mp_fTempB[j]));
    }

    p_oNCITrees = p_oPop->Find( m_cQuery );

    //************************************
    // Loop through and to calculate growth for each tree
    //************************************
    p_oTree = p_oNCITrees->NextTree();
    while ( p_oTree )
    {
      iSpecies = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      if ( -1 != mp_iGrowthCodes[mp_iIndexes[iSpecies]] [iType - clTreePopulation::sapling] )
      {

        //Make sure tree's not dead
        iDeadCode = p_oPop->GetIntDataCode( "dead", iSpecies, iType );
        if ( -1 != iDeadCode )
        {
          p_oTree->GetValue( iDeadCode, & iIsDead );
        }
        else
          iIsDead = notdead;

        if ( notdead == iIsDead )
        {

          p_oTree->GetValue( p_oPop->GetDbhCode( iSpecies, iType ), & fDbh );

          //First calculate the pieces that have no DBH component and thus will
          //not change in our loop

          //Get a number of neighbors with a DBH greater than the target
          iNumBiggerNeighbors = GetNumLargerNeighbors(p_oTree, p_oPop);

          iSpInd = mp_iIndexes[iSpecies];

          //To correctly compound growth over the number of years per timestep,
          //we have to loop over the number of years, re-calculating the parts
          //with DBH and incrementing the DBH each time
          fAmountDiamIncrease = 0;
          for ( i = 0; i < fNumberYearsPerTimestep; i++ )
          {

            fCrowdingEffect = exp(-mp_fCompC[iSpInd] *
                         pow(fDbh, mp_fGamma[iSpInd]) *
                         pow(iNumBiggerNeighbors, mp_fCompD[iSpInd]));
            //Make sure it's between 0 and 1
            if ( fCrowdingEffect < 0 ) fCrowdingEffect = 0;
            if ( fCrowdingEffect > 1 ) fCrowdingEffect = 1;

            //Get the tree's size effect
            /*if (fDbh < mp_fSizeMinDBH[iSpInd])
              fSEDbh = mp_fSizeMinDBH[iSpInd];
            else
              fSEDbh = fDbh;
            fSizeEffect =
            exp( -0.5 * pow( log( fSEDbh / mp_fSizeX0[iSpInd] ) / mp_fSizeXb[iSpInd], 2 ) );

            //Make sure it's bounded between 0 and 1
            if ( fSizeEffect < 0 ) fSizeEffect = 0;
            if ( fSizeEffect > 1 ) fSizeEffect = 1;*/

            //Calculate actual growth in cm/yr
            fTempDiamIncrease = mp_fMaxRG[iSpInd] *
                                fSizeEffect *
                                fCrowdingEffect *
                                p_fPrecipEffect[iSpInd] *
                                p_fTempEffect[iSpInd];

            //Add it to the running total of diameter increase
            fAmountDiamIncrease += fTempDiamIncrease;

            //Increase the DBH for the next loop.  If this is a sapling,
            //convert to a dbh value from what would be a diam10 increase
            if ( clTreePopulation::sapling == iType )
            {
              fDbh += p_oAllom->ConvertDiam10ToDbh( fTempDiamIncrease, iSpecies );
            }
            else
            {
              fDbh += fTempDiamIncrease;
            }
          }

          //Assign the growth back to "Growth" and hold it
          p_oTree->SetValue( mp_iGrowthCodes[iSpInd] [iType - clTreePopulation::sapling], fAmountDiamIncrease );

        } //end of if (bIsNCITree)
      }

      p_oTree = p_oNCITrees->NextTree();
    }

    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;

  }
  catch ( modelErr & err )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempEffect;
    delete[] p_fPrecipEffect;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clWeibullClimateGrowth::PreCalcGrowth" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetNumLargerNeighbors
////////////////////////////////////////////////////////////////////////////
int clWeibullClimateGrowth::GetNumLargerNeighbors(clTree *p_oTarget, clTreePopulation *p_oPop)
{
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
/*    p_oTarget->GetValue( p_oPop->GetDbhCode( iTargetSpecies, iTargetType ), & fTargetDbh );

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

          //Make sure the neighbor's not dead
          iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
          if ( -1 != iDeadCode )  p_oNeighbor->GetValue( iDeadCode, & iIsDead );
          else iIsDead = notdead;

          if ( notdead == iIsDead ) iNumNeighbors++;
        }
      }

      p_oNeighbor = p_oAllNeighbors->NextTree();
    }*/
    return iNumNeighbors;
}

////////////////////////////////////////////////////////////////////////////
// FormatQuery
////////////////////////////////////////////////////////////////////////////
void clWeibullClimateGrowth::FormatQuery()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  char * cQuery = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50],
         cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSapling = false, bAdult = false;

  if (m_iNumSpeciesTypeCombos == 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clWeibullClimateGrowth::FormatQuery";
    stcErr.sMoreInfo = "Weibull Climate Growth is not applied to any trees.";
    throw( stcErr );
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    if (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::sapling)
      bSapling = true;
    else if (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::adult)
      bAdult = true;
  }

  //Do a type/species search on all the types and species
  strcpy( cQuery, "species=" );
  for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
  {
    sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
    strcat( cQuery, cQueryPiece );
  }
  sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
  strcat( cQuery, cQueryPiece );

  //Now type
  strcat( cQuery, "::type=" );
  if ( bSapling )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::sapling, "," );
    strcat( cQuery, cQueryPiece );
  }
  if ( bAdult )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::adult, "," );
    strcat( cQuery, cQueryPiece );
  }

  //Remove the last comma
  cQuery[strlen( cQuery ) - 1] = '\0';

  //Now put it in m_cQuery, sized correctly
  m_cQuery = new char[strlen( cQuery ) + 1];
  strcpy( m_cQuery, cQuery );
  delete[] cQuery;
}
