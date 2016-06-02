
//---------------------------------------------------------------------------
#include "DensitySelfThinning.h"
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
clDensitySelfThinning::clDensitySelfThinning(clSimManager *p_oSimManager) :
      clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
      clMortalityBase(p_oSimManager) {
 try {

   m_sNameString = "densityselfthinningmortshell";
   m_sXMLRoot = "DensitySelfThinning";

   mp_fSelfThinRadius = NULL;
   mp_fMinDensityForMort = NULL;
   mp_fSelfThinAsymptote = NULL;
   mp_fSelfThinDiamEffect = NULL;
   mp_fSelfThinDensityEffect = NULL;
   mp_iIndexes = NULL;

   m_iNumberYearsPerTimestep = 0;

 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clDensitySelfThinning::clDensitySelfThinning";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clDensitySelfThinning::~clDensitySelfThinning() {

 delete[] mp_fSelfThinRadius;
 delete[] mp_fMinDensityForMort;
 delete[] mp_fSelfThinAsymptote;
 delete[] mp_fSelfThinDiamEffect;
 delete[] mp_fSelfThinDensityEffect;
 delete[] mp_iIndexes;

}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clDensitySelfThinning::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  doubleVal *p_fTempValues = NULL;
  try {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
    short int i; //loop counter

    m_iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    //This behavior can only be applied with a one year timestep, therefore throw error if timestep is not 1.
    if ( m_iNumberYearsPerTimestep != 1 )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDensitySelfThinning::DoShellSetup" ;
      stcErr.sMoreInfo = "This behavior can only be used with a one year timestep";
      throw( stcErr );
     }

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    mp_iIndexes = new short int[m_iNumTotalSpecies];

    //Make the list of indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //If any of the types is Adult, error out
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
      if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensitySelfThinning::DoShellSetup" ;
        stcErr.sMoreInfo = "This behavior cannot be applied to Adults";
        throw( stcErr );
      }

    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
           p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare the arrays for holding the variables and initilize to null
    mp_fSelfThinRadius = new double[m_iNumBehaviorSpecies];
    mp_fMinDensityForMort = new double[m_iNumBehaviorSpecies];
    mp_fSelfThinAsymptote = new double[m_iNumBehaviorSpecies];
    mp_fSelfThinDiamEffect = new double[m_iNumBehaviorSpecies];
    mp_fSelfThinDensityEffect = new double[m_iNumBehaviorSpecies];

    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fSelfThinRadius [i] = 0;
      mp_fMinDensityForMort [i] = 0;
      mp_fSelfThinAsymptote [i] = 0;
      mp_fSelfThinDiamEffect [i] = 0;
      mp_fSelfThinDensityEffect [i] = 0;
    }

    //Capture the values from the parameter file

    //Self Thinning Radius
    FillSpeciesSpecificValue(p_oElement, "mo_selfThinRadius",
             "mo_strVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets and check that they are positive
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fSelfThinRadius[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      if ( mp_fSelfThinRadius[mp_iIndexes[p_fTempValues[i].code]] <= 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensitySelfThinning::DoShellSetup" ;
        stcErr.sMoreInfo = "The Density Self Thinning parameter SelfThinRadius must be a positive number.";
        throw( stcErr );
      }
    }
    //Minimum Density for Mortality
    FillSpeciesSpecificValue(p_oElement, "mo_minDensityForMort",
               "mo_mdfmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets and check that they are positive
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fMinDensityForMort[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      if ( mp_fMinDensityForMort[mp_iIndexes[p_fTempValues[i].code]] < 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensitySelfThinning::DoShellSetup" ;
        stcErr.sMoreInfo = "The Density Self Thinning parameter MinDensityForMort must be greater than or equal to 0.";
        throw( stcErr );
      }
    }

    //Self Thinning Asymptote
    FillSpeciesSpecificValue(p_oElement, "mo_selfThinAsymptote",
               "mo_staVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets and check that they are positive
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fSelfThinAsymptote[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      if ( mp_fSelfThinAsymptote[mp_iIndexes[p_fTempValues[i].code]] <= 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensitySelfThinning::DoShellSetup" ;
        stcErr.sMoreInfo = "The Density Self Thinning parameter SelfThinAsymptote must be a positive number.";
        throw( stcErr );
      }
    }

    //Self Thinning Diameter Effect
    FillSpeciesSpecificValue(p_oElement, "mo_selfThinDiamEffect",
               "mo_stdieVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fSelfThinDiamEffect[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Self Thinning Density Effect
    FillSpeciesSpecificValue(p_oElement, "mo_selfThinDensityEffect",
               "mo_stdeeVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets and check that they are positive
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fSelfThinDensityEffect[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      if ( mp_fSelfThinDensityEffect[mp_iIndexes[p_fTempValues[i].code]] <= 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensitySelfThinning::DoShellSetup" ;
        stcErr.sMoreInfo = "The Density Self Thinning parameter SelfThinDensityEffect must be a positive number.";
        throw( stcErr );
      }
    }

   delete[] p_fTempValues;

  }
  catch (modelErr&err) {
    delete[] p_fTempValues;
    throw(err);
  }
  catch (modelMsg &msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    delete[] p_fTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDensitySelfThinning::DoShellSetup";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// CalculateNeigborhoodTreeCountAndMeanDiam10
////////////////////////////////////////////////////////////////////////////

void clDensitySelfThinning::CalculateNeighborhoodTreeCountAndMeanDiam10( clTree * p_oTree, float *p_fTreeCount, float *p_fMeanDiam10)
{
  try
  {
    clTreePopulation *p_oPop = (clTreePopulation*)mp_oSimManager->GetPopulationObject("treepopulation");

    clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
    clTree * p_oNeighbor; //competing neighbor
    std::stringstream sQuery; //format search strings into this

    float fSumNeighDiam10 = 0, //Sum of the neighbour's Diam10 - eventually to be divided by iNeighborCount
         fDiam10, //neighbor's Diam10
         fTargetX, fTargetY; //holders for the target tree's X and Y location
    int iNeighborCount =0; //Number of neighboring trees

    short int iNeighSpecies, iNeighType, //species and type for neighbor
         iTargetSpecies = p_oTree->GetSpecies(); //target tree's species

    //Get target tree's coordinates
    p_oTree->GetValue( p_oPop->GetXCode( iTargetSpecies, p_oTree->GetType() ), & fTargetX );
    p_oTree->GetValue( p_oPop->GetYCode( iTargetSpecies, p_oTree->GetType() ), & fTargetY );

    //Get all trees within the max crowding radius -
    sQuery << "distance=" << mp_fSelfThinRadius[mp_iIndexes[iTargetSpecies]]
           << "FROM x=" << fTargetX << "y=" << fTargetY << "::height=" << 0.0;
    p_oAllNeighbors = p_oPop->Find(sQuery.str());

    //Loop through and calculate neighborhood values
    p_oNeighbor = p_oAllNeighbors->NextTree();
    while ( p_oNeighbor )
    {
      if ( p_oNeighbor != p_oTree )
      {
        iNeighType = p_oNeighbor->GetType();
        if ( iNeighType == clTreePopulation::seedling || iNeighType == clTreePopulation::sapling )
        {
          iNeighborCount++;
          iNeighSpecies = p_oNeighbor->GetSpecies();

           //Get the neighbor's Diam10
           p_oNeighbor->GetValue( p_oPop->GetDiam10Code( iNeighSpecies, iNeighType ), & fDiam10 );
           fSumNeighDiam10 += fDiam10;
        }

      }
      p_oNeighbor = p_oAllNeighbors->NextTree();
    }

    *p_fTreeCount = static_cast<float>(iNeighborCount);
    *p_fMeanDiam10 = fSumNeighDiam10/static_cast<float>(iNeighborCount);
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
    stcErr.sFunction = "clDensitySelfThinning::CalculateNeighborhoodTreeCountAndMeanDiam10" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clDensitySelfThinning::DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies) {
  float fDeathProb, //Probability of mortality
  fRandom = clModelMath::GetRand(); //random number
  float fDensity; //Tree density in neighborhood of target tree
  float fMeanDiam10; //Mean Diam10 of trees in target tree's neighborhood
  float fTreeCount; //Number of tree in target tree's neighborhood

  //Get number of trees and mean Diam10 in target tree's neighborhood
  CalculateNeighborhoodTreeCountAndMeanDiam10(p_oTree, &fTreeCount, &fMeanDiam10);

  //Calculate density of trees in the neighborhood (trees/ha)
  fDensity = (fTreeCount*10000.0)/(3.1415926*mp_fSelfThinRadius[mp_iIndexes[iSpecies]]*mp_fSelfThinRadius[mp_iIndexes[iSpecies]]);

  if (fDensity <= mp_fMinDensityForMort[mp_iIndexes[iSpecies]])
    fDeathProb = 0; //If density is less than the min density for mortality, the tree lives.
  else
    //Calculate probability of mortality
    fDeathProb = ((mp_fSelfThinAsymptote[mp_iIndexes[iSpecies]]+ (mp_fSelfThinDiamEffect[mp_iIndexes[iSpecies]] * fMeanDiam10)) * fDensity)/
    (((mp_fSelfThinAsymptote[mp_iIndexes[iSpecies]] + (mp_fSelfThinDiamEffect[mp_iIndexes[iSpecies]]* fMeanDiam10))/mp_fSelfThinDensityEffect[mp_iIndexes[iSpecies]]) + fDensity);

  //If the probability of mortality is greater than the random number, kill the tree
  if (fRandom <  fDeathProb)
    return natural;
  else
    return notdead;
}
