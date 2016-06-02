#include "NCITermNCIBARatio.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "SimManager.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <sstream>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCITermNCIBARatio::clNCITermNCIBARatio(bool bUseDefaultBA) {
  mp_fAlpha = NULL;
  mp_fBeta = NULL;
  mp_fLambda = NULL;
  mp_fMinimumNeighborDBH = NULL;
  mp_iWhatSpecies = NULL;

  mp_oPlot = NULL;

  m_fMaxCrowdingRadius = 0;
  m_fMaxAdultRadius = 0;
  m_fMaxSaplingRadius = 0;
  m_iNumTotalSpecies = 0;
  m_iNumBehaviorSpecies = 0;
  m_fMinSaplingHeight = 0;
  m_fDbhAdjustor = 1;

  m_bUseDefaultBA = bUseDefaultBA;
  m_fDefaultBA = 0;
  iNumNCIs = 2;
  bRequiresTargetDiam = !m_bUseDefaultBA;

}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clNCITermNCIBARatio::~clNCITermNCIBARatio() {
  int i;

  delete[] mp_fAlpha;
  delete[] mp_fBeta;
  delete[] mp_iWhatSpecies;

  if (mp_fLambda)
    for (i = 0; i < m_iNumTotalSpecies; i++)
      delete[] mp_fLambda[i];
  delete[] mp_fLambda;

  delete[] mp_fMinimumNeighborDBH;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateNCITerm
//////////////////////////////////////////////////////////////////////////////
clNCITermBase::ncivals clNCITermNCIBARatio::CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies) {

  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  ncivals toReturn;
  std::stringstream sQuery; //format search strings into this
  float fNCI = 0, //nci - the end result of all this math
      fDistance, //distance between target and neighbor
      fDbh,
      fTotBA = 0,
      fBARatio,
      fNeighBA,
      fNeighDbh,
      fNeighX, fNeighY; //holders for the neighbor tree's X and Y
  int iIsDead,
      iNumNeighbors = 0;
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iDeadCode; //neighbor's dead code

  if (!m_bUseDefaultBA) p_oTree->GetValue( p_oPop->GetDbhCode( p_oTree->GetSpecies(), p_oTree->GetType() ), & fDbh );

  //Format the query to get all competing neighbors

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sQuery << "distance=" << m_fMaxCrowdingRadius << "FROM x="
         << fX << "y=" << fY << "::height=" << m_fMinSaplingHeight;
  p_oAllNeighbors = p_oPop->Find(sQuery.str());

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor ) {
    if ( p_oNeighbor == p_oTree ) goto nextTree;

    iNeighSpecies = p_oNeighbor->GetSpecies();
    iNeighType = p_oNeighbor->GetType();

    if ( clTreePopulation::seedling == iNeighType ||
        clTreePopulation::snag == iNeighType) goto nextTree;

    //Make sure the neighbor's not dead
    iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
    if ( -1 != iDeadCode )  p_oNeighbor->GetValue( iDeadCode, & iIsDead );
    else iIsDead = notdead;

    if (iIsDead > natural) goto nextTree;

    //Get the neighbor's dbh
    p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fNeighDbh );

    if ( fNeighDbh < mp_fMinimumNeighborDBH[iNeighSpecies] ) goto nextTree;

    //Get the neighbor's X and Y values
    p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
    p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

    //Get the distance between the two trees
    fDistance = p_oPlot->GetDistance( fX, fY, fNeighX, fNeighY );

    //Make sure it's within the appropriate radius
    if ((m_fMaxAdultRadius < fDistance && clTreePopulation::adult == iNeighType) ||
        (m_fMaxSaplingRadius < fDistance && clTreePopulation::sapling == iNeighType) ||
        0 == fDistance)
      goto nextTree;

    fNeighBA = clModelMath::CalculateBasalArea(fNeighDbh);
    fTotBA += fNeighBA;
    iNumNeighbors++;

    //Add competitive effect to NCI
    fNCI += mp_fLambda[iSpecies][iNeighSpecies] *
        (pow( (fNeighDbh * m_fDbhAdjustor), mp_fAlpha[iSpecies])
            / pow( fDistance, mp_fBeta[iSpecies] ) );


    nextTree:
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  //Calculate BA ratio
  if (fTotBA > 0) {
    if (m_bUseDefaultBA) {
      fBARatio = (fTotBA / iNumNeighbors) / m_fDefaultBA;
    } else {
      fBARatio = (fTotBA / iNumNeighbors) / clModelMath::CalculateBasalArea(fDbh);
    }
  }
  else fBARatio = 1;

  toReturn.fNCI1 = fBARatio;
  toReturn.fNCI2 = fNCI;
  return toReturn;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clNCITermNCIBARatio::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values
  std::stringstream sLabel;
  int i, j;

  m_iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies();
  mp_iWhatSpecies = new short int[m_iNumBehaviorSpecies];
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_iWhatSpecies[i] = p_oNCI->GetBehaviorSpecies(i);
  }

  //Get a pointer to the plot object
  mp_oPlot = p_oNCI->GetSimManager()->GetPlotObject();

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
  m_fMinSaplingHeight = 50;
  //Get the minimum sapling height
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight) {
      m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
    }
  }

  mp_fAlpha = new double[m_iNumTotalSpecies];
  mp_fBeta = new double[m_iNumTotalSpecies];
  mp_fMinimumNeighborDBH = new double[m_iNumTotalSpecies];
  mp_fLambda = new double*[m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_fLambda[i] = new double[m_iNumTotalSpecies];
    for (j = 0; j < m_iNumTotalSpecies; j++) {
      mp_fLambda[i][j] = -1;
    }
  }

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Max adult crowding radius
  FillSingleValue( p_oElement, "nciMaxAdultCrowdingRadius", &m_fMaxAdultRadius, true );

  //Max sapling crowding radius
  FillSingleValue( p_oElement, "nciMaxSaplingCrowdingRadius", &m_fMaxSaplingRadius, true);

  m_fMaxCrowdingRadius = m_fMaxAdultRadius > m_fMaxSaplingRadius ? m_fMaxAdultRadius : m_fMaxSaplingRadius;

  //NCI DBH adjustor
  FillSingleValue(p_oElement, "nciDbhAdjustor", & m_fDbhAdjustor, true);

  if (m_bUseDefaultBA) {
    //Get the default DBH
    FillSingleValue(p_oElement, "nciBADefaultDBH", &m_fDefaultBA, true);
    //Calculate the BA
    m_fDefaultBA = clModelMath::CalculateBasalArea(m_fDefaultBA);
  }

  //Neighbor DBH effect (alpha)
  FillSpeciesSpecificValue(p_oElement, "nciAlpha", "naVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fAlpha[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Neighbor distance effect (beta)
  FillSpeciesSpecificValue(p_oElement, "nciBeta", "nbVal", p_fTempValues,
      m_iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for (i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fBeta[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Lambda
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    sLabel << "nci" << p_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambda";
    FillSpeciesSpecificValue(p_oElement, sLabel.str(), "nlVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true);
    sLabel.str("");
    for ( j = 0; j < m_iNumBehaviorSpecies; j++ )      {
      mp_fLambda[p_fTempValues[j].code][i] = p_fTempValues[j].val;
    }
  }

  //Minimum neighbor DBH
  FillSpeciesSpecificValue( p_oElement, "nciMinNeighborDBH", "nmndVal",
      mp_fMinimumNeighborDBH, p_oPop, true);

  delete[] p_fTempValues;

  //Make sure that the max radius of neighbor effects is > 0
  if (m_fMaxCrowdingRadius < 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clNCITermNCIBARatio::DoSetup";
    stcErr.sMoreInfo = "All values for NCI max crowding radius must be greater than 0.";
    throw( stcErr );
  }

  for (i = 0; i < m_iNumTotalSpecies; i++) {
    //Make sure that the minimum neighbor DBH is not negative
    if (0 > mp_fMinimumNeighborDBH[i]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCITermNCIBARatio::DoSetup";
      stcErr.sMoreInfo = "Minimum neighbor DBH for NCI cannot be less than 0.";
      throw( stcErr );
    }
  }
}
