#include "NCIWithSeedlings.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include "Allometry.h"
#include <sstream>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIWithSeedlings::clNCIWithSeedlings() {
  mp_fAlpha = NULL;
  mp_fBeta = NULL;
  mp_fLambda = NULL;
  mp_fMaxCrowdingRadius = NULL;
  mp_fMinimumNeighborDiam10 = NULL;

  m_bIncludeSnags = false;
  m_fDiam10Divisor = 0;
  m_iNumTotalSpecies = 0;
  iNumNCIs = 1;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clNCIWithSeedlings::~clNCIWithSeedlings() {
  int i;

  delete[] mp_fAlpha;
  delete[] mp_fBeta;
  delete[] mp_fMaxCrowdingRadius;
  if (mp_fLambda)
    for (i = 0; i < m_iNumTotalSpecies; i++)
      delete[] mp_fLambda[i];
  delete[] mp_fLambda;

  delete[] mp_fMinimumNeighborDiam10;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateNCITerm
//////////////////////////////////////////////////////////////////////////////
clNCITermBase::ncivals clNCIWithSeedlings::CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies) {

  clAllometry *p_oAllom = p_oPop->GetAllometryObject();
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  ncivals toReturn;
  std::stringstream sQuery; //format search strings into this
  float fNCI = 0, //nci - the end result of all this math
      fDistance, //distance between target and neighbor
      fDiam10, //neighbor's dbh
      fTemp,
      fNeighX, fNeighY; //holders for the neighbor tree's X and Y
  int iIsDead; //neighbor's damage value
  short int iNeighSpecies, iNeighType, //species and type for neighbor
            iDeadCode; //neighbor's dead code

  //Format the query to get all competing neighbors

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sQuery << "distance=" << mp_fMaxCrowdingRadius[iSpecies] << "FROM x="
         << fX << "y=" << fY << "::height=0";
  p_oAllNeighbors = p_oPop->Find(sQuery.str());

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor ) {
    if (p_oTree != NULL && p_oNeighbor == p_oTree ) goto nextTree;

    iNeighSpecies = p_oNeighbor->GetSpecies();
    iNeighType = p_oNeighbor->GetType();

    if (clTreePopulation::snag == iNeighType && !m_bIncludeSnags) goto nextTree;

    //Make sure the neighbor's not dead
    iDeadCode = p_oPop->GetIntDataCode( "dead", p_oNeighbor->GetSpecies(), p_oNeighbor->GetType() );
    if ( -1 != iDeadCode ) {
      p_oNeighbor->GetValue( iDeadCode, & iIsDead );
      if (iIsDead != notdead && iIsDead != natural) goto nextTree;
    }

    //Get diam10 - if it's an adult, use the sapling allometry from DBH
    if (clTreePopulation::seedling == iNeighType ||
        clTreePopulation::sapling == iNeighType) {
      //Get the neighbor's diam10
      p_oNeighbor->GetValue( p_oPop->GetDiam10Code( iNeighSpecies, iNeighType ), & fDiam10 );
    } else {
      p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fTemp );
      fDiam10 = p_oAllom->ConvertDbhToDiam10(fTemp, iNeighSpecies);
    }

    if ( fDiam10 < mp_fMinimumNeighborDiam10[iNeighSpecies] ) goto nextTree;

    //Get the neighbor's X and Y values
    p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
    p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

    //Get the distance between the two trees
    fDistance = p_oPlot->GetDistance( fX, fY, fNeighX, fNeighY );


    //Only goto nextTree if distance is not 0 - it will be a fluke condition to
    //allow a tree that is literally standing on top of another one not to
    //affect it competitively, but there it is
    if ( fDistance < VERY_SMALL_VALUE) goto nextTree;

    //Add competitive effect to NCI
    fNCI += mp_fLambda[iSpecies][iNeighSpecies]
         * (pow( ( fDiam10 / m_fDiam10Divisor ), mp_fAlpha[iSpecies] )
         / pow( fDistance, mp_fBeta[iSpecies] ) );

    nextTree:
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }
  toReturn.fNCI1 = fNCI;
  return toReturn;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clNCIWithSeedlings::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values
  std::stringstream sLabel;
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      i, j;

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  mp_fAlpha = new double[m_iNumTotalSpecies];
  mp_fBeta = new double[m_iNumTotalSpecies];
  mp_fMaxCrowdingRadius = new double[m_iNumTotalSpecies];
  mp_fMinimumNeighborDiam10 = new double[m_iNumTotalSpecies];
  mp_fLambda = new double*[m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) mp_fLambda[i] = new double[m_iNumTotalSpecies];

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[iNumBehaviorSpecies];
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = p_oNCI->GetBehaviorSpecies(i);

  //Max crowding radius
  FillSpeciesSpecificValue( p_oElement, "nciMaxCrowdingRadius", "nmcrVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fMaxCrowdingRadius[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Neighbor DBH effect (alpha)
  FillSpeciesSpecificValue(p_oElement, "nciAlpha", "naVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true);
  //Transfer to the appropriate array buckets
  for ( i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fAlpha[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Neighbor distance effect (beta)
  FillSpeciesSpecificValue(p_oElement, "nciBeta", "nbVal", p_fTempValues,
      iNumBehaviorSpecies, p_oPop, true );
  //Transfer to the appropriate array buckets
  for (i = 0; i < iNumBehaviorSpecies; i++ )
    mp_fBeta[p_fTempValues[i].code] = p_fTempValues[i].val;

  //Lambda
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    sLabel << "nci" << p_oPop->TranslateSpeciesCodeToName(i) << "NeighborLambda";
    FillSpeciesSpecificValue(p_oElement, sLabel.str(), "nlVal",
        p_fTempValues, iNumBehaviorSpecies, p_oPop, true);
    sLabel.str("");
    for ( j = 0; j < iNumBehaviorSpecies; j++ )      {
      mp_fLambda[p_fTempValues[j].code][i] = p_fTempValues[j].val;
    }
  }

  //Minimum neighbor DBH
  FillSpeciesSpecificValue( p_oElement, "nciMinNeighborDBH", "nmndVal",
      mp_fMinimumNeighborDiam10, p_oPop, true);

  //NCI DBH divisor
  FillSingleValue(p_oElement, "nciDbhDivisor", & m_fDiam10Divisor, true);

  //Whether to include snags
  FillSingleValue( p_oElement, "nciIncludeSnagsInNCI", & m_bIncludeSnags, true );

  delete[] p_fTempValues;

  //Make sure that the max radius of neighbor effects is > 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (mp_fMaxCrowdingRadius[p_oNCI->GetBehaviorSpecies(i)] < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCIWithSeedlings::DoSetup";
      stcErr.sMoreInfo = "All values for NCI max crowding radius must be greater than 0.";
      throw( stcErr );
    }
  }
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    //Make sure that the minimum neighbor DBH is not negative
    if (0 > mp_fMinimumNeighborDiam10[i]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCIWithSeedlings::DoSetup";
      stcErr.sMoreInfo = "Minimum neighbor DBH for NCI cannot be less than 0.";
      throw( stcErr );
    }
  }

  //Make sure the DBH divisor is greater than 0
  if (m_fDiam10Divisor <= 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clNCIWithSeedlings::DoSetup";
    stcErr.sMoreInfo = "The NCI DBH divisor must be greater than 0.";
    throw( stcErr );
  }
}
