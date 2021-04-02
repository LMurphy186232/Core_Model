//---------------------------------------------------------------------------
#include "TempDependentNeighborhoodDisperse.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Plot.h"
#include "ModelMath.h"
#include "Grid.h"

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clTempDependentNeighborhoodDisperse::clTempDependentNeighborhoodDisperse( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clDisperseBase( p_oSimManager )
{
  try
  {
    m_sNameString = "TemperatureDependentNeighborhoodDisperse";
    m_sXMLRoot = "TemperatureDependentNeighborhoodDisperse";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Null pointers
    mp_fFecM = NULL;
    mp_fFecN = NULL;
    mp_fA = NULL;
    mp_fB = NULL;
    mp_fPresB = NULL;
    mp_fPresM = NULL;
    mp_fThreshold = NULL;
    m_bDoPresenceTesting = false;
    m_fRadius = 0;
    m_fMinSaplingHeight = 0;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;
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
    stcErr.sFunction = "clTempDependentNeighborhoodDisperse::clTempDependentNeighborhoodDisperse" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clTempDependentNeighborhoodDisperse::~clTempDependentNeighborhoodDisperse()
{
  //Delete arrays
  delete[] mp_fFecM;
  delete[] mp_fFecN;
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fPresB;
  delete[] mp_fPresM;
  delete[] mp_fThreshold;
}



////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clTempDependentNeighborhoodDisperse::DoShellSetup( DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    int iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    short int i; //loop counter

    //Get the minimum sapling height
    m_fMinSaplingHeight = 50;
    for ( i = 0; i < iNumTotalSpecies; i++ )
      if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
        m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );

    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare the arrays for holding the variables
    mp_fA = new double[iNumTotalSpecies];
    mp_fB = new double[iNumTotalSpecies];
    mp_fFecM = new double[iNumTotalSpecies];
    mp_fFecN = new double[iNumTotalSpecies];
    mp_fPresB = new double[iNumTotalSpecies];
    mp_fPresM = new double[iNumTotalSpecies];
    mp_fThreshold = new double[iNumTotalSpecies];

    //Capture the values from the parameter file

    //Fecundity M
    FillSpeciesSpecificValue( p_oElement, "di_tempDepNeighFecM",
        "di_tdnfmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fFecM[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Fecundity N
    FillSpeciesSpecificValue( p_oElement, "di_tempDepNeighFecN",
        "di_tdnfnVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fFecN[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Presence M
    FillSpeciesSpecificValue( p_oElement, "di_tempDepNeighPresM",
        "di_tdnpmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPresM[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Presence B
    FillSpeciesSpecificValue( p_oElement, "di_tempDepNeighPresB",
        "di_tdnpbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPresB[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Presence threshold
    FillSpeciesSpecificValue( p_oElement, "di_tempDepNeighPresThreshold",
        "di_tdnptVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fThreshold[p_fTempValues[i].code] = p_fTempValues[i].val;
    //Reinforce zero and one to ensure that proper behavior occurs
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (mp_fThreshold[mp_iWhatSpecies[i]] < 1e-5)
        mp_fThreshold[mp_iWhatSpecies[i]] = -0.01;
      if (mp_fThreshold[mp_iWhatSpecies[i]] > (1 - 1e-5))
        mp_fThreshold[mp_iWhatSpecies[i]] = 1.01;
    }

    //A
    FillSpeciesSpecificValue( p_oElement, "di_tempDepNeighA",
        "di_tdnaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //B
    FillSpeciesSpecificValue( p_oElement, "di_tempDepNeighB",
        "di_tdnbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Neighborhood search radius
    FillSingleValue(p_oElement, "di_tempDepNeighRadius", &m_fRadius, true );

    //Check to see if presence testing is required for this run. If all values
    //in the threshold parameter are zero, it is not. Set a flag to save
    //processing later.
    m_bDoPresenceTesting = false;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( mp_fThreshold[mp_iWhatSpecies[i]] > 0 ) {
        m_bDoPresenceTesting = true;
        break;
      }
    }
    delete[] p_fTempValues;

  }
  catch ( modelErr & err )
  {
    delete[] p_fTempValues;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTempValues;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTempDependentNeighborhoodDisperse::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// AddSeeds()
////////////////////////////////////////////////////////////////////////////
void clTempDependentNeighborhoodDisperse::AddSeeds()
{
  float *p_fBAC = NULL,
        *p_fFecundity = NULL,
        *p_fFunctionalA = NULL; //Intercept actually used - zero this out if a
            //species fails presence testing

  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
    clTree * p_oNeighbor; //competing neighbor
    char cQuery[75]; //format search strings into this
    int iNumXCells = mp_oSeedGrid->GetNumberXCells(), //number x cells in seed grid
        iNumYCells = mp_oSeedGrid->GetNumberYCells(), //number y cells in seed grid
        iTotalNumSpecies = p_oPop->GetNumberOfSpecies();
    float fNumGridSeeds, //seeds already in one grid cell
          fTemp,
          fMeanTempC = p_oPlot->GetMeanAnnualTemp(),
          fNeighDbh, //DBH of neighbors for calculating basal area
          //grid cell lengths in the middle of the grid
          fMidX = mp_oSeedGrid->GetLengthXCells(),
          fMidY = mp_oSeedGrid->GetLengthYCells(),
          fMidArea = fMidX * fMidY, //shortcut
          //possible short grid cell lengths on the edge of the grid
          fEndX = p_oPlot->GetXPlotLength() - ((iNumXCells - 1)*fMidX),
          fEndY = p_oPlot->GetYPlotLength() - ((iNumYCells - 1)*fMidY),
          fArea, //one cell's area
          fCellX, fCellY; //one cell's X and Y dimensions
    int iNumYrs = mp_oSimManager->GetNumberOfYearsPerTimestep();
    short int i, iSpecies,
              iNeighSpecies, iNeighType,
              iX, iY;

    p_fBAC = new float[iTotalNumSpecies];
    p_fFunctionalA = new float[iTotalNumSpecies];
    p_fFecundity = new float[iTotalNumSpecies];

    //Set all functional intercept values equal to that parameter
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      iSpecies = mp_iWhatSpecies[i];
      p_fFunctionalA[iSpecies] = mp_fA[iSpecies];
    }

    if ( m_bDoPresenceTesting ) {

      //Determine if there are any species that have no adults present in the
      //plot
      bool *p_bPresent = new bool[iTotalNumSpecies];
      for ( i = 0; i < iTotalNumSpecies; i++ ) p_bPresent[i] = true;
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        p_bPresent[mp_iWhatSpecies[i]] = false; //Species this behavior is not
                                           //applied to are now set to true
      iSpecies = iTotalNumSpecies - m_iNumBehaviorSpecies;
      sprintf(cQuery, "type=%d", clTreePopulation::adult);
      p_oAllNeighbors = p_oPop->Find(cQuery);
      p_oNeighbor = p_oAllNeighbors->NextTree();
      while (p_oNeighbor && iSpecies < iTotalNumSpecies) {
        if (false == p_bPresent[p_oNeighbor->GetSpecies()]) {
          p_bPresent[p_oNeighbor->GetSpecies()] = true;
          iSpecies++;
        }
        p_oNeighbor = p_oAllNeighbors->NextTree();
      }

      //For any species not present - calculate presence test function
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        iSpecies = mp_iWhatSpecies[i];
        if (false == p_bPresent[iSpecies]) {
          fTemp = exp(-0.5*pow((fMeanTempC - mp_fPresM[iSpecies])/mp_fPresB[iSpecies],2));
          if (fTemp < mp_fThreshold[iSpecies]) //fail - set intercept to 0
            p_fFunctionalA[iSpecies] = 0;
        }
      }
      delete[] p_bPresent;
    }

    //Calculate fecundity for each species
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      iSpecies = mp_iWhatSpecies[i];
      p_fFecundity[iSpecies] = mp_fB[iSpecies] *
        exp(-0.5*pow((fMeanTempC - mp_fFecM[iSpecies])/mp_fFecN[iSpecies],2));
    }

    //Loop through the grid cells of the seed grid and put seeds in each
    for ( iX = 0; iX < iNumXCells; iX++ ) {
      for ( iY = 0; iY < iNumYCells; iY++ ) {

        //Get the area of this cell, correcting for possible short edges
        if (iX != iNumXCells - 1 && iY != iNumYCells - 1) fArea = fMidArea;
        else {
          if (iX == iNumXCells - 1) fCellX = fEndX;
          else fCellX = fMidX;
          if (iY == iNumYCells - 1) fCellY = fEndY;
          else fCellY = fMidY;
          fArea = fCellX * fCellY;
        }

        //Get the coordinates of the grid cell center - dangerously reuse
        //fCellX and fCellY
        mp_oSeedGrid->GetPointOfCell(iX, iY, &fCellX, &fCellY);

        //Get conspecific adult basal area for each species in the neighborhood
        //of this grid cell
        for (i=0; i<iTotalNumSpecies; i++)
          p_fBAC[i] = 0;
        sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fRadius,
            "FROM x=", fCellX, "y=", fCellY, "::height=", m_fMinSaplingHeight );
        p_oAllNeighbors = p_oPop->Find( cQuery );
        p_oNeighbor = p_oAllNeighbors->NextTree();
        while ( p_oNeighbor ) {
          iNeighType = p_oNeighbor->GetType();
          if ( clTreePopulation::adult == iNeighType) {
            iNeighSpecies = p_oNeighbor->GetSpecies();
            //Get the neighbor's dbh
            p_oNeighbor->GetValue(p_oPop->GetDbhCode(iNeighSpecies, iNeighType),
                & fNeighDbh );
            p_fBAC[iNeighSpecies] += clModelMath::CalculateBasalArea(fNeighDbh);
          }
          p_oNeighbor = p_oAllNeighbors->NextTree();
        }

        //Calculate the number of seeds for each species
        for (i = 0; i < m_iNumBehaviorSpecies; i++) {
          iSpecies = mp_iWhatSpecies[i];
          fNumGridSeeds = p_fFunctionalA[iSpecies] + p_fFecundity[iSpecies] *
              p_fBAC[iSpecies];

          //Scale from seeds per square meter per year to seeds per grid cell
          //per timestep
          fNumGridSeeds *= iNumYrs * fArea;

          //Place the seeds in the seed grid - dangerously reuse fLambda
          mp_oSeedGrid->GetValueOfCell( iX, iY, mp_iNumSeedsCode[mp_iWhatSpecies[i]], & fTemp );
          fTemp += fNumGridSeeds;
          mp_oSeedGrid->SetValueOfCell( iX, iY, mp_iNumSeedsCode[mp_iWhatSpecies[i]], fTemp );
        }
      }
    }

    delete[] p_fBAC;
    delete[] p_fFecundity;
    delete[] p_fFunctionalA;
  }
  catch ( modelErr & err )
  {
    delete[] p_fBAC;
    delete[] p_fFecundity;
    delete[] p_fFunctionalA;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fBAC;
    delete[] p_fFecundity;
    delete[] p_fFunctionalA;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fBAC;
    delete[] p_fFecundity;
    delete[] p_fFunctionalA;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTempDependentNeighborhoodDisperse::AddSeeds" ;
    throw( stcErr );
  }
}
