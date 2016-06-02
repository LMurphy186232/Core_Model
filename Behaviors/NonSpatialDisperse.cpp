//---------------------------------------------------------------------------
#include "NonSpatialDisperse.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Plot.h"
#include "ModelMath.h"
#include "Grid.h"

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clNonSpatialDispersal::clNonSpatialDispersal( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clDisperseBase( p_oSimManager )
{
  try
  {
    m_sNameString = "NonSpatialDisperse";
    m_sXMLRoot = "NonSpatialDisperse";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Null pointers
    mp_fDbhForReproduction = NULL;
    mp_fSlopeOfLambda = NULL;
    mp_fInterceptOfLambda = NULL;
    m_fAreaOfPlotInHa = 0;

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
    stcErr.sFunction = "clNonSpatialDispersal::clNonSpatialDispersal" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clNonSpatialDispersal::~clNonSpatialDispersal()
{
  //Delete arrays
  delete[] mp_fDbhForReproduction;
  delete[] mp_fSlopeOfLambda;
  delete[] mp_fInterceptOfLambda;
}



////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clNonSpatialDispersal::DoShellSetup( DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    int iNumTotalSpecies = p_oPop->GetNumberOfSpecies();
    short int i; //loop counter

    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    m_fAreaOfPlotInHa = mp_oSimManager->GetPlotObject()->GetPlotArea();

    //Declare the arrays for holding the variables
    mp_fSlopeOfLambda = new double[iNumTotalSpecies];
    mp_fInterceptOfLambda = new double[iNumTotalSpecies];
    mp_fDbhForReproduction = new double[iNumTotalSpecies];

    //Capture the values from the parameter file

    //Slope of lambda
    FillSpeciesSpecificValue( p_oElement, "di_nonSpatialSlopeOfLambda", "di_nssolVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fSlopeOfLambda[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Intercept of lambda
    FillSpeciesSpecificValue( p_oElement, "di_nonSpatialInterceptOfLambda", "di_nsiolVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fInterceptOfLambda[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Minimum DBH for reproduction
    FillSpeciesSpecificValue( p_oElement, "di_minDbhForReproduction", "di_mdfrVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fDbhForReproduction[p_fTempValues[i].code] = p_fTempValues[i].val;


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
    stcErr.sFunction = "clNonSpatialDispersal::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// AddSeeds()
////////////////////////////////////////////////////////////////////////////
void clNonSpatialDispersal::AddSeeds()
{
  float *p_fNumNewSeeds = NULL;
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    int iNumXCells = mp_oSeedGrid->GetNumberXCells(), //number x cells in seed grid
        iNumYCells = mp_oSeedGrid->GetNumberYCells(); //number y cells in seed grid
    float fBasalArea, //basal area in sq. m./ha for a species
          fNumGridSeeds, //seeds already in one grid cell
          fLambda, //lambda - mean number of seeds per grid cell
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
    short int i, iSpecies, iX, iY;

    p_fNumNewSeeds = new float[m_iNumBehaviorSpecies];

    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      iSpecies = mp_iWhatSpecies[i];

      //If the slope is not zero, calculate the basal area
      if ( mp_fSlopeOfLambda[iSpecies] != 0 ) {
        fBasalArea = ComputeBasalArea( p_oPop, iSpecies );
      } else {
        fBasalArea = 0;
      }

      //Compute lambda
      fLambda = clModelMath::CalcPointValue( fBasalArea, mp_fSlopeOfLambda[iSpecies], mp_fInterceptOfLambda[iSpecies] );

      //Scale from seeds per year to seeds per timestep
      p_fNumNewSeeds[i] = fLambda * iNumYrs;
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

        for (i = 0; i < m_iNumBehaviorSpecies; i++) {

          //Place the seeds in the seed grid
          mp_oSeedGrid->GetValueOfCell( iX, iY, mp_iNumSeedsCode[mp_iWhatSpecies[i]], & fNumGridSeeds );
          fNumGridSeeds += p_fNumNewSeeds[i] * fArea;
          mp_oSeedGrid->SetValueOfCell( iX, iY, mp_iNumSeedsCode[mp_iWhatSpecies[i]], fNumGridSeeds );
        }
      }
    }

    delete[] p_fNumNewSeeds;
  }
  catch ( modelErr & err )
  {
    delete[] p_fNumNewSeeds;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fNumNewSeeds;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fNumNewSeeds;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clNonSpatialDispersal::AddSeeds" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// ComputeBasalArea()
////////////////////////////////////////////////////////////////////////////
float clNonSpatialDispersal::ComputeBasalArea( clTreePopulation * p_oPop, short int iSpecies )
{
  clTreeSearch * p_oAll; //trees representing our query
  clTree * p_oTree; //individual tree
  char cQuery[30]; //query string for tree search
  float fDbh, //tree's dbh
  fBasalArea = 0; //basal area of species

  //Query for all trees that could possibly be of reproductive age for this
  //species
  if ( p_oPop->GetMinAdultDBH( iSpecies ) <= mp_fDbhForReproduction[iSpecies] )
  {
    sprintf( cQuery, "%s%d%s%d", "type=", clTreePopulation::adult, "::species=", iSpecies );
  }
  else
  {
    sprintf( cQuery, "%s%d%s%d%s%d", "type=", clTreePopulation::adult, ",", clTreePopulation::sapling,
        "::species=", iSpecies );
  }

  p_oAll = p_oPop->Find( cQuery );
  p_oTree = p_oAll->NextTree();
  while ( p_oTree )
  {

    //Get the dbh of this tree
    p_oTree->GetValue( p_oPop->GetDbhCode( iSpecies, p_oTree->GetType() ), & fDbh );

    if ( fDbh >= mp_fDbhForReproduction[iSpecies] )
    {
      fBasalArea += clModelMath::CalculateBasalArea( fDbh );
    }
    p_oTree = p_oAll->NextTree();
  }

  return fBasalArea / m_fAreaOfPlotInHa;
}
