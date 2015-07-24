#include "NCITermBARatioDBHDefault.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <sstream>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCITermBARatioDBHDefault::clNCITermBARatioDBHDefault() {
  m_fMaxSaplingRadius = 0;
  m_fMaxAdultRadius = 0;
  m_fMaxCrowdingRadius = 0;

  m_fMinSaplingHeight = 0;
  m_fDefaultBA = 0;
  iNumNCIs = 2;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateNCITerm
//////////////////////////////////////////////////////////////////////////////
clNCITermBase::ncivals clNCITermBARatioDBHDefault::CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies) {

  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  ncivals toReturn;
  std::stringstream sQuery; //format search strings into this
  float fNeighDbh, //neighbor's dbh
        fNeighBA,
        fTotBA = 0,
        fBARatio,
        fDistance, //distance between target and neighbor
        fNeighX, fNeighY; //holders for the neighbor tree's X and Y
  int iIsDead, //neighbor's damage value
      iNumNeighbors = 0;
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iDeadCode; //neighbor's dead code

  //Format the query to get all competing neighbors

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sQuery << "distance=" << m_fMaxCrowdingRadius << "FROM x="
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
    if ((m_fMaxAdultRadius < fDistance && clTreePopulation::adult == iNeighType) ||
        (m_fMaxSaplingRadius < fDistance && clTreePopulation::sapling == iNeighType))
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
  if (fTotBA > 0) fBARatio = (fTotBA / iNumNeighbors) / m_fDefaultBA;
  else fBARatio = 1;


  toReturn.fNCI1 = fBARatio;
  toReturn.fNCI2 = fTotBA;

  return toReturn;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clNCITermBARatioDBHDefault::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {

  int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
      i;

  //Get the default DBH
  FillSingleValue(p_oElement, "nciBADefaultDBH", &m_fDefaultBA, true);
  //Calculate the BA
  m_fDefaultBA = clModelMath::CalculateBasalArea(m_fDefaultBA);

  //Max crowding radius
  FillSingleValue( p_oElement, "nciMaxAdultCrowdingRadius", &m_fMaxAdultRadius, true );

  //Max crowding radius
  FillSingleValue( p_oElement, "nciMaxSaplingCrowdingRadius", &m_fMaxSaplingRadius, true );

  m_fMaxCrowdingRadius =
        m_fMaxAdultRadius > m_fMaxSaplingRadius ? m_fMaxAdultRadius : m_fMaxSaplingRadius;

  //Get the minimum sapling height
  m_fMinSaplingHeight = 50;
  for (i = 0; i < iNumTotalSpecies; i++) {
    if (p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight ) {
      m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
    }
  }
}
