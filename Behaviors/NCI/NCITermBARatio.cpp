#include "NCITermBARatio.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <sstream>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCITermBARatio::clNCITermBARatio() {
  mp_fMaxSaplingRadius = NULL;
  mp_fMaxAdultRadius = NULL;
  mp_fMaxCrowdingRadius = NULL;
  bRequiresTargetDiam = true;

  m_fMinSaplingHeight = 0;
  iNumNCIs = 2;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clNCITermBARatio::~clNCITermBARatio() {
  delete[] mp_fMaxSaplingRadius;
  delete[] mp_fMaxAdultRadius;
  delete[] mp_fMaxCrowdingRadius;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateNCITerm
//////////////////////////////////////////////////////////////////////////////
clNCITermBase::ncivals clNCITermBARatio::CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies) {

  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  ncivals toReturn;
  std::stringstream sQuery; //format search strings into this
  float fNeighDbh, //neighbor's dbh
        fNeighBA,
        fTotBA = 0,
        fBARatio,
        fDistance, //distance between target and neighbor
        fDbh, //target's dbh
        fNeighX, fNeighY; //holders for the neighbor tree's X and Y
  int iIsDead, //neighbor's damage value
      iNumNeighbors = 0;
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iDeadCode; //neighbor's dead code

  p_oTree->GetValue( p_oPop->GetDbhCode( p_oTree->GetSpecies(), p_oTree->GetType() ), & fDbh );

  //Format the query to get all competing neighbors

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sQuery << "distance=" << mp_fMaxCrowdingRadius[iSpecies] << "FROM x="
         << fX << "y=" << fY << "::height=" << m_fMinSaplingHeight;
  p_oAllNeighbors = p_oPop->Find(sQuery.str());

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor ) {

    if (p_oNeighbor == p_oTree) goto nextTree;

    iNeighSpecies = p_oNeighbor->GetSpecies();
    iNeighType = p_oNeighbor->GetType();

    if ( clTreePopulation::seedling == iNeighType ||
        clTreePopulation::snag == iNeighType) goto nextTree;

    //Get the neighbor's X and Y values
    p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
    p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

    //Get the distance between the two trees
    fDistance = p_oPlot->GetDistance( fX, fY, fNeighX, fNeighY );

    //Make sure it's within the appropriate radius
    if ((mp_fMaxAdultRadius[iSpecies] < fDistance && clTreePopulation::adult == iNeighType) ||
        (mp_fMaxSaplingRadius[iSpecies] < fDistance && clTreePopulation::sapling == iNeighType))
      goto nextTree;

    //Make sure the neighbor's not dead
    iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
    if ( -1 != iDeadCode )  p_oNeighbor->GetValue( iDeadCode, & iIsDead );
    else iIsDead = notdead;

    if (iIsDead > natural) goto nextTree;

    //Get the neighbor's dbh
    p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fNeighDbh );

    fNeighBA = clModelMath::CalculateBasalArea(fNeighDbh);
    fTotBA += fNeighBA;
    iNumNeighbors++;

    nextTree:
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  //Calculate BA ratio
  if (fTotBA > 0) fBARatio = (fTotBA / iNumNeighbors) / clModelMath::CalculateBasalArea(fDbh);
  else fBARatio = 1;


  toReturn.fNCI1 = fBARatio;
  toReturn.fNCI2 = fTotBA;

  return toReturn;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clNCITermBARatio::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
      i;

  //Make sure this is only applied to saplings and bigger
  for (i = 0; i < p_oNCI->GetNumSpeciesTypeCombos(); i++) {
    if (p_oNCI->GetSpeciesTypeCombo(i).iType == clTreePopulation::seedling) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCINeighborBA::DoSetup";
      stcErr.sMoreInfo = "BA ratio crowding effect cannot be applied to seedlings.";
      throw( stcErr );
    }
  }

  mp_fMaxAdultRadius = new double[iNumTotalSpecies];
  mp_fMaxSaplingRadius = new double[iNumTotalSpecies];
  mp_fMaxCrowdingRadius = new double[iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Max crowding radius
  FillSpeciesSpecificValue( p_oElement, "nciMaxAdultCrowdingRadius", "nmacrVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fMaxAdultRadius[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Max crowding radius
  FillSpeciesSpecificValue( p_oElement, "nciMaxSaplingCrowdingRadius", "nmscrVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fMaxSaplingRadius[p_fTempValues[i].code] = p_fTempValues[i].val;

  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fMaxCrowdingRadius[p_oNCI->GetBehaviorSpecies(i)] =
        mp_fMaxAdultRadius[p_oNCI->GetBehaviorSpecies(i)] > mp_fMaxSaplingRadius[p_oNCI->GetBehaviorSpecies(i)] ?
            mp_fMaxAdultRadius[p_oNCI->GetBehaviorSpecies(i)] : mp_fMaxSaplingRadius[p_oNCI->GetBehaviorSpecies(i)];

  delete[] p_fTempValues;

  //Get the minimum sapling height
  m_fMinSaplingHeight = 50;
  for (i = 0; i < iNumTotalSpecies; i++) {
    if (p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight ) {
      m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
    }
  }
}
