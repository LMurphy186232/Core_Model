#include "NCITermDefault.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "BehaviorBase.h"
#include "ParsingFunctions.h"
#include <sstream>
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCITermDefault::clNCITermDefault() {
  mp_fAlpha = NULL;
  mp_fBeta = NULL;
  mp_fLambda = NULL;
  mp_fMaxCrowdingRadius = NULL;
  mp_fMinimumNeighborDBH = NULL;

  m_bIncludeSnags = false;
  m_fDbhDivisor = 0;
  m_iNumTotalSpecies = 0;
  m_fMinSaplingHeight = 0;
  iNumNCIs = 1;
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clNCITermDefault::~clNCITermDefault() {
  int i;

  delete[] mp_fAlpha;
  delete[] mp_fBeta;
  delete[] mp_fMaxCrowdingRadius;
  if (mp_fLambda)
    for (i = 0; i < m_iNumTotalSpecies; i++)
      delete[] mp_fLambda[i];
  delete[] mp_fLambda;

  delete[] mp_fMinimumNeighborDBH;
}

//////////////////////////////////////////////////////////////////////////////
// CalculateNCITerm
//////////////////////////////////////////////////////////////////////////////
clNCITermBase::ncivals clNCITermDefault::CalculateNCITerm(clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot, const float &fX, const float &fY, const int &iSpecies) {

  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  ncivals toReturn;
  std::stringstream sQuery; //format search strings into this
  float fNCI = 0, //nci - the end result of all this math
      fDistance, //distance between target and neighbor
      fDbh, //neighbor's dbh
      fNeighX, fNeighY; //holders for the neighbor tree's X and Y
  int iIsDead; //neighbor's damage value
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iDeadCode; //neighbor's dead code

  //Format the query to get all competing neighbors

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sQuery << "distance=" << mp_fMaxCrowdingRadius[iSpecies] << "FROM x="
         << fX << "y=" << fY << "::height=" << m_fMinSaplingHeight;
  p_oAllNeighbors = p_oPop->Find(sQuery.str());

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor ) {
    if ( p_oNeighbor != p_oTree ) {

      iNeighSpecies = p_oNeighbor->GetSpecies();
      iNeighType = p_oNeighbor->GetType();

      if ( clTreePopulation::seedling != iNeighType && ((clTreePopulation::snag != iNeighType) || m_bIncludeSnags)) {

        //Get the neighbor's dbh
        p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fDbh );

        if ( fDbh >= mp_fMinimumNeighborDBH[iNeighSpecies] ) {

          //Make sure the neighbor's not dead
          iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
          if ( -1 != iDeadCode ) {
            p_oNeighbor->GetValue( iDeadCode, & iIsDead );
          } else
            iIsDead = notdead;

          if (notdead == iIsDead || natural == iIsDead) {

            //Get the neighbor's X and Y values
            p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
            p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

            //Get the distance between the two trees
            fDistance = p_oPlot->GetDistance( fX, fY, fNeighX, fNeighY );

            //Only continue if distance is not 0 - it will be a fluke condition to
            //allow a tree that is literally standing on top of another one not to
            //affect it competitively, but there it is
            if ( 0 != fDistance )

              //Add competitive effect to NCI
              fNCI += mp_fLambda[iSpecies][iNeighSpecies]
                    * ( pow( fDbh / m_fDbhDivisor, mp_fAlpha[iSpecies])
                    / pow( fDistance, mp_fBeta[iSpecies] ) );

          }
        }
      }
    }
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  toReturn.fNCI1 = fNCI;
  return toReturn;
}

//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clNCITermDefault::DoSetup(clTreePopulation *p_oPop, clBehaviorBase *p_oNCI, xercesc::DOMElement *p_oElement) {
  doubleVal * p_fTempValues; //for getting species-specific values
  std::stringstream sLabel;
  int iNumBehaviorSpecies = p_oNCI->GetNumBehaviorSpecies(),
      i, j;

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
  mp_fMaxCrowdingRadius = new double[m_iNumTotalSpecies];
  mp_fMinimumNeighborDBH = new double[m_iNumTotalSpecies];
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
      mp_fMinimumNeighborDBH, p_oPop, true);

  //NCI DBH divisor
  FillSingleValue(p_oElement, "nciDbhDivisor", & m_fDbhDivisor, true);

  //Whether to include snags
  FillSingleValue( p_oElement, "nciIncludeSnagsInNCI", & m_bIncludeSnags, true );

  delete[] p_fTempValues;

  //Make sure that the max radius of neighbor effects is > 0
  for ( i = 0; i < iNumBehaviorSpecies; i++) {
    if (mp_fMaxCrowdingRadius[p_oNCI->GetBehaviorSpecies(i)] < 0) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCITermDefault::DoSetup";
      stcErr.sMoreInfo = "All values for NCI max crowding radius must be greater than 0.";
      throw( stcErr );
    }
  }
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    //Make sure that the minimum neighbor DBH is not negative
    if (0 > mp_fMinimumNeighborDBH[i]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNCITermDefault::DoSetup";
      stcErr.sMoreInfo = "Minimum neighbor DBH for NCI cannot be less than 0.";
      throw( stcErr );
    }
  }

  //Make sure the DBH divisor is greater than 0
  if (m_fDbhDivisor <= 0) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clNCITermDefault::DoSetup";
    stcErr.sMoreInfo = "The NCI DBH divisor must be greater than 0.";
    throw( stcErr );
  }
}
