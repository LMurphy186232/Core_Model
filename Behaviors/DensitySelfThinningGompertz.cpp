//---------------------------------------------------------------------------
#include "DensitySelfThinningGompertz.h"
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
clDensitySelfThinningGompertz::clDensitySelfThinningGompertz(clSimManager *p_oSimManager) :
      clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
      clMortalityBase(p_oSimManager) {
 try {

   m_sNameString = "densitygompertzmortshell";
   m_sXMLRoot = "GompertzDensitySelfThinning";

   mp_fG = NULL;
   mp_fH = NULL;
   mp_fI = NULL;
   mp_fMinHeight = NULL;
   mp_iIndexes = NULL;

   m_fNumberYearsPerTimestep = 0;
   m_fRadius = 0;

 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clDensitySelfThinningGompertz::clDensitySelfThinningGompertz";
   throw(stcErr);
 }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clDensitySelfThinningGompertz::~clDensitySelfThinningGompertz() {

 delete[] mp_fG;
 delete[] mp_fH;
 delete[] mp_fI;
 delete[] mp_fMinHeight;
 delete[] mp_iIndexes;

}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clDensitySelfThinningGompertz::DoShellSetup(xercesc::DOMDocument *p_oDoc) {
  floatVal *p_fTempValues = NULL;
  try {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    DOMElement *p_oElement = GetParentParametersElement(p_oDoc);
    short int i; //loop counter

    m_fNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    mp_iIndexes = new short int[m_iNumTotalSpecies];

    //Make the list of indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
           p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare the arrays for holding the variables and initilize to null
    mp_fG = new float[m_iNumBehaviorSpecies];
    mp_fH = new float[m_iNumBehaviorSpecies];
    mp_fI = new float[m_iNumBehaviorSpecies];
    mp_fMinHeight = new float[m_iNumBehaviorSpecies];

    //Capture the values from the parameter file

    //Minimum height
    FillSpeciesSpecificValue(p_oElement, "mo_gompertzSelfThinningMinHeight",
             "mo_gstmhVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    //Transfer to the appropriate array buckets and check that they are positive
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      mp_fMinHeight[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
      if ( mp_fMinHeight[mp_iIndexes[p_fTempValues[i].code]] < 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensitySelfThinningGompertz::DoShellSetup" ;
        stcErr.sMoreInfo = "The Gompertz Density Self Thinning parameter minimum height must be a positive number.";
        throw( stcErr );
      }
    }
    //G
    FillSpeciesSpecificValue(p_oElement, "mo_gompertzSelfThinningG",
               "mo_gstgVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fG[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //H
    FillSpeciesSpecificValue(p_oElement, "mo_gompertzSelfThinningH",
               "mo_gsthVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fH[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //I
    FillSpeciesSpecificValue(p_oElement, "mo_gompertzSelfThinningI",
               "mo_gstiVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fI[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    FillSingleValue(p_oElement, "mo_gompertzSelfThinningRadius", &m_fRadius, true);
    if ( m_fRadius <= 0 )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensitySelfThinningGompertz::DoShellSetup" ;
        stcErr.sMoreInfo = "The Gompertz Density Self Thinning parameter search radius must be a positive number.";
        throw( stcErr );
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
    stcErr.sFunction = "clDensitySelfThinningGompertz::DoShellSetup";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// CalculateNeigborhoodTreeCount
////////////////////////////////////////////////////////////////////////////

float clDensitySelfThinningGompertz::CalculateNeighborhoodTreeCount( clTree * p_oTree)
{
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  std::stringstream sQuery; //format search strings into this

  float fNeighCount = 0,
      fTargetX, fTargetY; //holders for the target tree's X and Y location

  short int iTargetSpecies = p_oTree->GetSpecies(); //target tree's species

  //Get target tree's coordinates
  p_oTree->GetValue( p_oPop->GetXCode( iTargetSpecies, p_oTree->GetType() ), & fTargetX );
  p_oTree->GetValue( p_oPop->GetYCode( iTargetSpecies, p_oTree->GetType() ), & fTargetY );

  //Get all trees within the max crowding radius -
  sQuery << "distance=" << m_fRadius << "FROM x=" << fTargetX <<
        "y=" << fTargetY << "::height="
        << mp_fMinHeight[mp_iIndexes[iTargetSpecies]];
  p_oAllNeighbors = p_oPop->Find(sQuery.str());
  sQuery.str("");

  //Loop through and calculate neighborhood values
  p_oNeighbor = p_oAllNeighbors->NextTree();
  while ( p_oNeighbor )
  {
    // Deliberately count the target
    if (p_oNeighbor->GetSpecies() == iTargetSpecies)
      fNeighCount++;


    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  return fNeighCount;
}

////////////////////////////////////////////////////////////////////////////
// DoMort()
////////////////////////////////////////////////////////////////////////////
deadCode clDensitySelfThinningGompertz::DoMort(clTree *p_oTree, const float &fDbh, const short int &iSpecies) {
  float fDeathProb, //Probability of mortality
  fRandom = clModelMath::GetRand(); //random number
  float fDensity; //Tree density in neighborhood of target tree
  float fTreeCount; //Number of tree in target tree's neighborhood
  int ind;

  //Get number of conspecific trees in target tree's neighborhood
  fTreeCount = CalculateNeighborhoodTreeCount(p_oTree);

  //Calculate density of trees in the neighborhood (trees/m2)
  fDensity = fTreeCount/(3.1415926 * m_fRadius * m_fRadius);

  //Calculate probability of mortality
  ind = mp_iIndexes[iSpecies];
  fDeathProb = mp_fG[ind] * exp(-exp(mp_fH[ind] - (mp_fI[ind] * fDensity)));

  fDeathProb = 1 - pow(1 - fDeathProb, m_fNumberYearsPerTimestep);

  //If the probability of mortality is greater than the random number, kill the tree
  if (fRandom <  fDeathProb)
    return natural;
  else
    return notdead;
}
