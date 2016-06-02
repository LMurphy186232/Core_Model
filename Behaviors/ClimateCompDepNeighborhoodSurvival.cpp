//---------------------------------------------------------------------------
#include "ClimateCompDepNeighborhoodSurvival.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Plot.h"
#include "Allometry.h"
#include "Grid.h"
#include <stdio.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clClimateCompDepNeighborhoodSurvival::clClimateCompDepNeighborhoodSurvival( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
  clMortalityBase( p_oSimManager )
{
  //Set namestring
  m_sNameString = "ClimateCompDepNeighborhoodmortshell";
  m_sXMLRoot = "ClimateCompDepNeighborhoodSurvival";

  //Null out our pointers
  mp_fM = NULL;
  mp_fN = NULL;
  mp_fA = NULL;
  mp_fB = NULL;
  mp_fC = NULL;
  mp_fD = NULL;
  mp_fTempFunction = NULL;
  mp_fWDFunction = NULL;
  mp_iGridSurvivalCodes = NULL;

  mp_oGrid = NULL;
  m_fRadius = 0;
  m_iBATCode = -1;
  m_fMinSaplingHeight = 0;

  //Version 1
  m_fVersionNumber = 1.0;
  m_fMinimumVersionNumber = 1.0;
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clClimateCompDepNeighborhoodSurvival::~clClimateCompDepNeighborhoodSurvival()
{
  delete[] mp_fM;
  delete[] mp_fN;
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fC;
  delete[] mp_fD;
  delete[] mp_fTempFunction;
  delete[] mp_fWDFunction;
  delete[] mp_iGridSurvivalCodes;
}


////////////////////////////////////////////////////////////////////////////
// ReadParameterFile()
////////////////////////////////////////////////////////////////////////////
void clClimateCompDepNeighborhoodSurvival::ReadParameterFile( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    doubleVal * p_fTempValues; //for getting species-specific values
    short int i, //loop counter
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

    m_fMinSaplingHeight = 50;
    //Get the minimum sapling height
    for ( i = 0; i < iNumTotalSpecies; i++ )
      if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
        m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );

    //The rest are sized number of species to which this behavior applies
    mp_fM = new double[iNumTotalSpecies];
    mp_fN = new double[iNumTotalSpecies];
    mp_fA = new double[iNumTotalSpecies];
    mp_fB = new double[iNumTotalSpecies];
    mp_fC = new double[iNumTotalSpecies];
    mp_fD = new double[iNumTotalSpecies];

    //Set up our doubleVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Fill the variables

    //A
    FillSpeciesSpecificValue( p_oElement, "mo_climCompDepNeighA", "mo_ccdnaVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //B
    FillSpeciesSpecificValue( p_oElement, "mo_climCompDepNeighB", "mo_ccdnbVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

    //M
    FillSpeciesSpecificValue( p_oElement, "mo_climCompDepNeighM", "mo_ccdnmVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fM[p_fTempValues[i].code] = p_fTempValues[i].val;

    //N
    FillSpeciesSpecificValue( p_oElement, "mo_climCompDepNeighN", "mo_ccdnnVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fN[p_fTempValues[i].code] = p_fTempValues[i].val;

    //C
    FillSpeciesSpecificValue( p_oElement, "mo_climCompDepNeighC", "mo_ccdncVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fC[p_fTempValues[i].code] = p_fTempValues[i].val;

    //D
    FillSpeciesSpecificValue( p_oElement, "mo_climCompDepNeighD", "mo_ccdndVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fD[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Max crowding radius
    FillSingleValue( p_oElement, "mo_climCompDepNeighRadius", &m_fRadius, true );

    delete[] p_fTempValues;
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
    stcErr.sFunction = "clClimateCompDepNeighborhoodSurvival::ReadParameterFile" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ValidateData
////////////////////////////////////////////////////////////////////////////
void clClimateCompDepNeighborhoodSurvival::ValidateData()
{
  try
  {

    int i;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {

      //Make sure that N is not 0
      if ( 0 == mp_fN[mp_iWhatSpecies[i]] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "N values cannot be 0.";
        throw( stcErr );
      }

      //Make sure that D is not 0
      if ( 0 == mp_fD[mp_iWhatSpecies[i]] )
      {
        modelErr stcErr;
        stcErr.sMoreInfo = "D values cannot be 0.";
        throw( stcErr );
      }
    }

    //Make sure that the neighbor search radius is not less than zero
    if ( 0 > m_fRadius )
    {
      modelErr stcErr;
      stcErr.sMoreInfo = "Neighbor search radius cannot be less than 0.";
      throw( stcErr );
    }
  }
  catch ( modelErr & err )
  {
    err.sFunction = "clClimateCompDepNeighborhoodSurvival::ValidateData";
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
    stcErr.sFunction = "clClimateCompDepNeighborhoodSurvival::ValidateData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clClimateCompDepNeighborhoodSurvival::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  ReadParameterFile( p_oDoc, p_oPop );
  ValidateData();
  SetupGrid(p_oPop);
  mp_fTempFunction = new double[p_oPop->GetNumberOfSpecies()];
  mp_fWDFunction = new double[p_oPop->GetNumberOfSpecies()];
}

////////////////////////////////////////////////////////////////////////////
// DoMort
////////////////////////////////////////////////////////////////////////////
deadCode clClimateCompDepNeighborhoodSurvival::DoMort( clTree * p_oTree, const float & fDbh, const short int & iSpecies )
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  float fX, fY,
        fSurvivalProb, //Tree's annual survival probability
        fBAT; //Neighborhood basal area of adult trees
  int iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
  short int iType = p_oTree->GetType(),
            iX, iY;
  bool bIsDead;

  //Has survival been calculated for this tree's cell?
  p_oTree->GetValue( p_oPop->GetXCode( iSpecies, iType ), & fX );
  p_oTree->GetValue( p_oPop->GetYCode( iSpecies, iType ), & fY );
  mp_oGrid->GetValueAtPoint( fX, fY, mp_iGridSurvivalCodes[iSpecies], &fSurvivalProb);
  if ( -1 == fSurvivalProb ) {

    //Get the basal area of neighbors, if it has not already been gotten
    mp_oGrid->GetCellOfPoint(fX, fY, &iX, &iY);
    mp_oGrid->GetPointOfCell( iX, iY, & fX, & fY );
    mp_oGrid->GetValueOfCell(iX, iY, m_iBATCode, &fBAT);
    if (fBAT < 0) {
      fBAT = GetBAT(fX, fY, p_oPop);
      mp_oGrid->SetValueOfCell(iX, iY, m_iBATCode, fBAT);
    }

    //Calculate survival probability for this grid cell
    fSurvivalProb = exp(-1.0 * mp_fA[iSpecies] * pow(fBAT, mp_fB[iSpecies])) *
              mp_fTempFunction[iSpecies] * mp_fWDFunction[iSpecies];
    fSurvivalProb = pow( fSurvivalProb, iNumberYearsPerTimestep );
    mp_oGrid->SetValueOfCell(iX, iY, mp_iGridSurvivalCodes[iSpecies], fSurvivalProb);
  }

  bIsDead = clModelMath::GetRand() >= fSurvivalProb;

  if (bIsDead) return natural;
  else return notdead;
}



////////////////////////////////////////////////////////////////////////////
// GetBAT
////////////////////////////////////////////////////////////////////////////
float clClimateCompDepNeighborhoodSurvival::GetBAT(float &fX, float &fY, clTreePopulation *p_oPop)
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  char cQuery[75]; //format search strings into this
  float fDbh, //neighbor's dbh
        fBAT = 0;
  int iIsDead; //whether a neighbor is dead
  short int iSpecies, iType, //species and type for neighbor
            iDeadCode; //neighbor's dead code

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fRadius, "FROM x=", fX,
       "y=", fY, "::height=", m_fMinSaplingHeight );
  p_oAllNeighbors = p_oPop->Find( cQuery );

  //Loop through and find all the adults
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor ) {

    iType = p_oNeighbor->GetType();
    if ( clTreePopulation::adult == iType) {
      iSpecies = p_oNeighbor->GetSpecies();
      //Get the neighbor's dbh
      p_oNeighbor->GetValue( p_oPop->GetDbhCode( iSpecies, iType ), & fDbh );

      //Make sure the neighbor's not dead due to a disturbance event
      iDeadCode = p_oPop->GetIntDataCode( "dead", iSpecies, iType );
      if ( -1 != iDeadCode )  p_oNeighbor->GetValue( iDeadCode, & iIsDead );
      else iIsDead = notdead;

      if ( notdead == iIsDead || natural == iIsDead)
        fBAT += clModelMath::CalculateBasalArea(fDbh);

    }

    p_oNeighbor = p_oAllNeighbors->NextTree();
  }
  return fBAT;
}

////////////////////////////////////////////////////////////////////////////
// PreMortCalcs
////////////////////////////////////////////////////////////////////////////
void clClimateCompDepNeighborhoodSurvival::PreMortCalcs( clTreePopulation * p_oPop )
{
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  float fPlotTemp = p_oPlot->GetMeanAnnualTemp(), //Current plot temperature
        fWD = p_oPlot->GetWaterDeficit(),
        fValue;
  short int i, ind, iX, iY,
      iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
      iNumXCells = mp_oGrid->GetNumberXCells(),
      iNumYCells = mp_oGrid->GetNumberYCells();


  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    ind = mp_iWhatSpecies[i];
    mp_fTempFunction[ind] = exp(-0.5*pow((fPlotTemp - mp_fM[ind])/mp_fN[ind], 2));
    mp_fWDFunction[ind] = exp(-0.5*pow((fWD - mp_fC[ind])/mp_fD[ind], 2));
  }

  //Reset the values in the survival grid to -1
  fValue = -1;
  for (iX = 0; iX < iNumXCells; iX++)
    for (iY = 0; iY < iNumYCells; iY++) {
      mp_oGrid->SetValueOfCell(iX, iY, m_iBATCode, fValue);
      for (i = 0; i < iNumTotalSpecies; i++)
          mp_oGrid->SetValueOfCell(iX, iY, mp_iGridSurvivalCodes[i], fValue);
    }

}

/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clClimateCompDepNeighborhoodSurvival::SetupGrid(clTreePopulation *p_oPop)
{
  std::stringstream sLabel;
  short int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
            i; //loop counter

  //Declare the arrays for our grid codes
  mp_iGridSurvivalCodes = new short int[iNumTotalSpecies];

  //Check to see if this grid has already been created
  mp_oGrid = mp_oSimManager->GetGridObject("Climate Comp Dep Neighborhood Survival");

  if (NULL == mp_oGrid) {

    //Create the grid with five float data members for each species
    mp_oGrid = mp_oSimManager->CreateGrid( "Climate Comp Dep Neighborhood Survival",
                          0,                    //number of ints
                          iNumTotalSpecies + 1, //number of floats
                          0,                    //number of chars
                          0);                   //number of bools

    //Register the data member called "survival_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "survival_" << i;
      mp_iGridSurvivalCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }

    //Register the data member called "BAT"
    m_iBATCode = mp_oGrid->RegisterFloat("BAT");
  }
  else {
    //Grid already exists - get the codes
    //Get the data member called "survival_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "survival_" << i;
      mp_iGridSurvivalCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iGridSurvivalCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clClimateCompDepNeighborhoodSurvival::SetupGrid";
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str()
          << "\" member of the \"Climate Comp Dep Neighborhood Survival\" grid.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      m_iBATCode = mp_oGrid->GetFloatDataCode( "BAT" );
      if (-1 == m_iBATCode) {
        modelErr stcErr;
        stcErr.sFunction = "clClimateCompDepNeighborhoodSurvival::SetupGrid" ;
        stcErr.sMoreInfo = "Couldn't find the \"BAT\" member of the \"Climate Comp Dep Neighborhood Survival\" grid.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }
  }

}
