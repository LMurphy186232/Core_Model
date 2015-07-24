//---------------------------------------------------------------------------
#include "StochDoubleLogTempDepNeighDisperse.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Plot.h"
#include "ModelMath.h"
#include "Grid.h"

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clStochDoubleLogTempDepNeighDisperse::clStochDoubleLogTempDepNeighDisperse( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clDisperseBase( p_oSimManager )
{
  try
  {
    m_sNameString = "StochDoubleLogTempDepNeighDisperse";
    m_sXMLRoot = "StochDoubleLogTempDepNeighDisperse";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Null pointers
    mp_fPA = NULL;
    mp_fPB = NULL;
    mp_fPM = NULL;
    mp_fAl = NULL;
    mp_fBl = NULL;
    mp_fCl = NULL;
    mp_fAh = NULL;
    mp_fBh = NULL;
    mp_fCh = NULL;
    mp_fA = NULL;
    mp_fB = NULL;

    m_fRadius = 0;
    m_fMinSaplingHeight = 0;
    m_fAnalysisPlotSize = 0;
    m_fAnnualizePeriod = 1;
    m_bDeterministic = false;
    m_bFecTempDep = false;

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
    stcErr.sFunction = "clStochDoubleLogTempDepNeighDisperse::clStochDoubleLogTempDepNeighDisperse" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clStochDoubleLogTempDepNeighDisperse::~clStochDoubleLogTempDepNeighDisperse()
{
  //Delete arrays
  delete[] mp_fPA;
  delete[] mp_fPB;
  delete[] mp_fPM;
  delete[] mp_fAl;
  delete[] mp_fBl;
  delete[] mp_fCl;
  delete[] mp_fAh;
  delete[] mp_fBh;
  delete[] mp_fCh;
  delete[] mp_fA;
  delete[] mp_fB;
}


////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clStochDoubleLogTempDepNeighDisperse::DoShellSetup( DOMDocument * p_oDoc )
{
  floatVal * p_fTempValues = NULL; //for getting species-specific values
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
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare the arrays for holding the variables
    mp_fPA = new float[iNumTotalSpecies];
    mp_fPB = new float[iNumTotalSpecies];
    mp_fPM = new float[iNumTotalSpecies];
    mp_fAl = new float[iNumTotalSpecies];
    mp_fBl = new float[iNumTotalSpecies];
    mp_fCl = new float[iNumTotalSpecies];
    mp_fAh = new float[iNumTotalSpecies];
    mp_fBh = new float[iNumTotalSpecies];
    mp_fCh = new float[iNumTotalSpecies];
    mp_fA = new float[iNumTotalSpecies];
    mp_fB = new float[iNumTotalSpecies];

    //Capture the values from the parameter file

    //PA
    FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighPA",
        "di_sdltdnpaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //PB
    FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighPB",
        "di_sdltdnpbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPB[p_fTempValues[i].code] = p_fTempValues[i].val;

    //PM
    FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighPM",
        "di_sdltdnpmVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fPM[p_fTempValues[i].code] = p_fTempValues[i].val;

    //A
    FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighA",
        "di_sdltdnaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //B
    FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighB",
        "di_sdltdnbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Neighborhood search radius
    FillSingleValue(p_oElement, "di_stochDoubLogTempDepNeighRadius", &m_fRadius, true );

    //Original analysis plot size, square meters
    FillSingleValue(p_oElement, "di_stochDoubLogTempDepNeighPlotSize", &m_fAnalysisPlotSize, true );

    //Deterministic or Poisson seed distribution
    FillSingleValue(p_oElement, "di_stochDoubLogTempDepNeighDeterministic", &m_bDeterministic, false );

    //Use temperature dependent portion of fecundity
    FillSingleValue(p_oElement, "di_stochDoubLogTempDepNeighTempFec", &m_bFecTempDep, true );

    //T, in years, to go from cumulative to annualized probability
    FillSingleValue(p_oElement, "di_stochDoubLogTempDepNeighT", &m_fAnnualizePeriod, true );

    //If we are using temperature dependent fecundity, get those values
    if (m_bFecTempDep) {
      //Al
      FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighAl",
          "di_sdltdnalVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer to the appropriate array buckets
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fAl[p_fTempValues[i].code] = p_fTempValues[i].val;

      //Bl
      FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighBl",
          "di_sdltdnblVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer to the appropriate array buckets
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fBl[p_fTempValues[i].code] = p_fTempValues[i].val;

      //Cl
      FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighCl",
          "di_sdltdnclVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer to the appropriate array buckets
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fCl[p_fTempValues[i].code] = p_fTempValues[i].val;

      //Ah
      FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighAh",
          "di_sdltdnahVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer to the appropriate array buckets
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fAh[p_fTempValues[i].code] = p_fTempValues[i].val;

      //Bh
      FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighBh",
          "di_sdltdnbhVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer to the appropriate array buckets
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fBh[p_fTempValues[i].code] = p_fTempValues[i].val;

      //Ch
      FillSpeciesSpecificValue( p_oElement, "di_stochDoubLogTempDepNeighCh",
          "di_sdltdnchVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
      //Transfer to the appropriate array buckets
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fCh[p_fTempValues[i].code] = p_fTempValues[i].val;
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
    stcErr.sFunction = "clStochDoubleLogTempDepNeighDisperse::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// AddSeeds()
////////////////////////////////////////////////////////////////////////////
void clStochDoubleLogTempDepNeighDisperse::AddSeeds()
{
  float *p_fBAC = NULL,
        *p_fFecundity = NULL;
  bool *p_bPresent = NULL;

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
          fNumYrs = mp_oSimManager->GetNumberOfYearsPerTimestep(),
          fMeanTempC = p_oPlot->GetMeanAnnualTemp(),
          fMeanTempK = fMeanTempC + 273.15,
          fNeighDbh, //DBH of neighbors for calculating basal area
          //grid cell lengths in the middle of the grid
          fMidX = mp_oSeedGrid->GetLengthXCells(),
          fMidY = mp_oSeedGrid->GetLengthYCells(),
          fMidArea = fMidX * fMidY, //shortcut
          //possible short grid cell lengths on the edge of the grid
          fEndX = p_oPlot->GetXPlotLength() - ((iNumXCells - 1)*fMidX),
          fEndY = p_oPlot->GetYPlotLength() - ((iNumYCells - 1)*fMidY),
          fArea, //one cell's area
          fPCc, fPCa,
          fCellX, fCellY; //one cell's X and Y dimensions
    short int i, iSpecies,
              iNeighSpecies, iNeighType,
              iX, iY;

    p_fBAC = new float[iTotalNumSpecies];
    p_fFecundity = new float[iTotalNumSpecies];
    p_bPresent = new bool[iTotalNumSpecies];

    //Determine if there are any species that have no adults present in the
    //plot
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

    //For any species not present, do colonization
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      iSpecies = mp_iWhatSpecies[i];
      if (false == p_bPresent[iSpecies]) {

        //Calculate cumulative probability of colonization
        fPCc = mp_fPA[iSpecies] * exp(-0.5*pow((fMeanTempC - mp_fPM[iSpecies])/mp_fPB[iSpecies],2));

        //Calculate annualized probability of colonization
        fPCa = 1-pow((1-fPCc),(1/m_fAnnualizePeriod));
        if (clModelMath::GetRand() <= fPCa) //colonize
          p_bPresent[iSpecies] = true;
      }
    }

    //Calculate fecundity for each species
    if (m_bFecTempDep) {
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        iSpecies = mp_iWhatSpecies[i];
        p_fFecundity[iSpecies] = mp_fB[iSpecies] *
            (mp_fAl[iSpecies] + ((1-mp_fAl[iSpecies])/(1+pow(mp_fBl[iSpecies]/fMeanTempK , mp_fCl[iSpecies])))) *
            (mp_fAh[iSpecies] + ((1-mp_fAh[iSpecies])/(1+pow(fMeanTempK /mp_fBh[iSpecies], mp_fCh[iSpecies]))));
      }
    } else {
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        iSpecies = mp_iWhatSpecies[i];
        p_fFecundity[iSpecies] = mp_fB[iSpecies] ;
      }
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
        //Turn into correction factor
        fArea /= m_fAnalysisPlotSize;

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
          if (p_bPresent[iSpecies]) {

            fNumGridSeeds = mp_fA[iSpecies] + p_fFecundity[iSpecies] * p_fBAC[iSpecies];

            //Poisson draw if appropriate
            if (!m_bDeterministic) {
              fNumGridSeeds = clModelMath::PoissonRandomDraw(fNumGridSeeds);
            }

            //Scale to seeds per grid cell per timestep
            fNumGridSeeds *= fNumYrs * fArea;

            //Place the seeds in the seed grid
            mp_oSeedGrid->GetValueOfCell( iX, iY, mp_iNumSeedsCode[mp_iWhatSpecies[i]], & fTemp );
            fTemp += fNumGridSeeds;
            mp_oSeedGrid->SetValueOfCell( iX, iY, mp_iNumSeedsCode[mp_iWhatSpecies[i]], fTemp );
          }
        }
      }
    }

    delete[] p_fBAC;
    delete[] p_fFecundity;
    delete[] p_bPresent;
  }
  catch ( modelErr & err )
  {
    delete[] p_fBAC;
    delete[] p_fFecundity;
    delete[] p_bPresent;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fBAC;
    delete[] p_fFecundity;
    delete[] p_bPresent;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fBAC;
    delete[] p_fFecundity;
    delete[] p_bPresent;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clStochDoubleLogTempDepNeighDisperse::AddSeeds" ;
    throw( stcErr );
  }
}
