#include "NCILargerNeighbors.h"
#include "ParsingFunctions.h"
#include "BehaviorBase.h"
#include "DataTypes.h"
#include "TreePopulation.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCILargerNeighbors::clNCILargerNeighbors() {
  mp_fMaxCrowdingRadius = NULL;
  mp_fMinimumNeighborDBH = NULL;
  m_fMinSaplingHeight = 0;
  iNumNCIs = 1;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clNCILargerNeighbors::~clNCILargerNeighbors() {
  delete[] mp_fMaxCrowdingRadius;
  delete[] mp_fMinimumNeighborDBH;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateNCITerm
//////////////////////////////////////////////////////////////////////////////
clNCITermBase::ncivals clNCILargerNeighbors::CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies) {
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  ncivals toReturn;
  char cQuery[75]; //format search strings into this
  float fNeighDbh, //neighbor's dbh
  fTargetDbh = 0; //target's dbh
  int iIsDead, //whether a neighbor is dead
  iNumNeighbors = 0;
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iDeadCode; //neighbor's dead code

  //Get the target DBH
  if (p_oTree != NULL) {
    p_oTree->GetValue(p_oPop->GetDbhCode(p_oTree->GetSpecies(), p_oTree->GetType()), &fTargetDbh);
  }

  //Format the query to get all competing neighbors

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", mp_fMaxCrowdingRadius[iSpecies], "FROM x=", fX,
      "y=", fY, "::height=", m_fMinSaplingHeight );
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

        //if ( notdead == iIsDead ) iNumNeighbors++;
        if ( notdead == iIsDead || natural == iIsDead) iNumNeighbors++;
      }
    }

    p_oNeighbor = p_oAllNeighbors->NextTree();
  }
  toReturn.fNCI1 = iNumNeighbors;
  return toReturn;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clNCILargerNeighbors::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  floatVal * p_fTempValues; //for getting species-specific values
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(), i;

  mp_fMinimumNeighborDBH = new float[iNumTotalSpecies];
  mp_fMaxCrowdingRadius = new float[iNumTotalSpecies];

  m_fMinSaplingHeight = 50;

  p_fTempValues = new floatVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ ) {
    p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);
  }

  //Get the minimum sapling height
  for (i = 0; i < iNumTotalSpecies; i++) {
    if (p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight ) {
      m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
    }
  }

  //Max crowding radius
  FillSpeciesSpecificValue( p_oElement, "nciMaxCrowdingRadius", "nmcrVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fMaxCrowdingRadius[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Minimum neighbor DBH
  FillSpeciesSpecificValue( p_oElement, "nciMinNeighborDBH", "nmndVal",
      mp_fMinimumNeighborDBH, p_oPop, true);

  delete[] p_fTempValues;

  //Make sure that the max radius of neighbor effects is > 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (mp_fMaxCrowdingRadius[p_oNCI->GetBehaviorSpecies(i)] < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCILargerNeighbors::DoSetup";
      stcErr.sMoreInfo = "All values for NCI max crowding radius must be greater than 0.";
      throw( stcErr );
    }
  }
  for (i = 0; i < iNumTotalSpecies; i++) {
    //Make sure that the minimum neighbor DBH is not negative
    if (0 > mp_fMinimumNeighborDBH[i]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCILargerNeighbors::DoSetup";
      stcErr.sMoreInfo = "Minimum neighbor DBH for NCI cannot be less than 0.";
      throw( stcErr );
    }
  }
}
