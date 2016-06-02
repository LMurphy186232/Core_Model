//---------------------------------------------------------------------------
#include "MastingNonSpatialDisperse.h"
#include "TreePopulation.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Plot.h"
#include "ModelMath.h"
#include "Grid.h"
#include <sstream>
#include <gsl/gsl_randist.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clMastingNonSpatialDisperse::clMastingNonSpatialDisperse(
    clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clDisperseBase(p_oSimManager) {
  try
  {
    m_sNameString = "MastingNonSpatialDisperse";
    m_sXMLRoot = "MastingNonSpatialDisperse";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Null pointers
    mp_fBinomialP = NULL;
    mp_fInvGaussMu = NULL;
    mp_fInvGaussLambda = NULL;
    mp_fNormalMean = NULL;
    mp_fNormalStandardDev = NULL;
    mp_iGroup = NULL;
    mp_iFunction = NULL;
    mp_fBasalArea = NULL;
    mp_iEvent = NULL;

    m_bGroupsUsed = false;

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
    stcErr.sFunction = "clMastingNonSpatialDisperse::clMastingNonSpatialDisperse" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clMastingNonSpatialDisperse::~clMastingNonSpatialDisperse() {
  int i;

  //Delete arrays
  if (mp_fInvGaussMu)
    for (i = 0; i < numevents; i++)
      delete[] mp_fInvGaussMu[i];

  if (mp_fInvGaussLambda)
    for (i = 0; i < numevents; i++)
      delete[] mp_fInvGaussLambda[i];

  if (mp_fNormalMean)
    for (i = 0; i < numevents; i++)
      delete[] mp_fNormalMean[i];

  if (mp_fNormalStandardDev)
    for (i = 0; i < numevents; i++)
      delete[] mp_fNormalStandardDev[i];

  if (mp_iFunction)
    for (i = 0; i < numevents; i++)
      delete[] mp_iFunction[i];


  delete[] mp_fBinomialP;
  delete[] mp_fInvGaussMu;
  delete[] mp_fInvGaussLambda;
  delete[] mp_fNormalMean;
  delete[] mp_fNormalStandardDev;
  delete[] mp_iGroup;
  delete[] mp_iFunction;
  delete[] mp_fBasalArea;
  delete[] mp_iEvent;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clMastingNonSpatialDisperse::DoShellSetup(DOMDocument * p_oDoc) {
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  intVal * p_iTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );;
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int iNumFunctionSpecies, //number of species using a given function
              i, j; //loop counters

    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    p_iTempValues = new intVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_iTempValues[i].code = mp_iWhatSpecies[i];

    mp_iEvent = new mastEvent[m_iNumBehaviorSpecies];
    //Declare the parameter arrays
    mp_fInvGaussMu = new double*[numevents];
    mp_fInvGaussLambda = new double*[numevents];
    mp_fNormalMean = new double*[numevents];
    mp_fNormalStandardDev = new double*[numevents];
    mp_iFunction = new pdf*[numevents];
    for (i = 0; i < numevents; i++) {
      mp_fInvGaussMu[i] = new double[m_iNumBehaviorSpecies];
      mp_fInvGaussLambda[i] = new double[m_iNumBehaviorSpecies];
      mp_fNormalMean[i] = new double[m_iNumBehaviorSpecies];
      mp_fNormalStandardDev[i] = new double[m_iNumBehaviorSpecies];
      mp_iFunction[i] = new pdf[m_iNumBehaviorSpecies];
    }

    mp_fBinomialP = new double[m_iNumBehaviorSpecies];
    mp_iGroup = new short int[m_iNumBehaviorSpecies];

    mp_fBasalArea = new float[m_iNumBehaviorSpecies];

    //Get the values from the parameter file

    //P parameter for binomial distribution for deciding when to mast
    FillSpeciesSpecificValue( p_oElement, "di_nonSpatialMastBinomialP",
        "di_nsmbpVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBinomialP[i] = p_fTempValues[i].val;

    //Masting function
    FillSpeciesSpecificValue( p_oElement, "di_nonSpatialMastMastFunction",
        "di_nsmmfVal", p_iTempValues, m_iNumBehaviorSpecies, p_oPop, true );

    //Transfer the values over - if any are not a valid function number,
    //throw an error
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( p_iTempValues[i].val != inverse_gaussian_pdf &&
           p_iTempValues[i].val != normal_pdf ) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingNonSpatialDisperse::DoShellSetup" ;
        std::stringstream s;
        s << "Invalid disperse function code " << p_iTempValues[i].val;
        stcErr.sMoreInfo = s.str();
        throw( stcErr );
      } else {
        mp_iFunction[mast][i] = (pdf)p_iTempValues[i].val;
      }
    }

    //Non-masting function
    FillSpeciesSpecificValue( p_oElement, "di_nonSpatialNonMastFunction",
           "di_nsnmfVal", p_iTempValues, m_iNumBehaviorSpecies, p_oPop, true );

    //Transfer the values over - if any are not a valid function number,
    //throw an error
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( p_iTempValues[i].val != inverse_gaussian_pdf &&
           p_iTempValues[i].val != normal_pdf ) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMastingNonSpatialDisperse::DoShellSetup" ;
        std::stringstream s;
        s << "Invalid disperse function code " << p_iTempValues[i].val;
        stcErr.sMoreInfo = s.str();
        throw( stcErr );
      } else {
        mp_iFunction[nonmast][i] = (pdf)p_iTempValues[i].val;
      }
    }

    //Group affiliation for synchrony
    FillSpeciesSpecificValue( p_oElement, "di_nonSpatialMastGroup",
        "di_nsmgVal", p_iTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iGroup[i] = p_iTempValues[i].val;

    //Check for multi-species groups
    m_bGroupsUsed = false;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      for (j = i+1; j < m_iNumBehaviorSpecies; j++) {
        if (mp_iGroup[j] == mp_iGroup[i]) {
          m_bGroupsUsed = true; break;
        }
      }
    }

    //Parameters for masting inverse gaussian distribution
    //Start by getting a list of species that use this distribution - we'll only
    //require parameters for those species
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( inverse_gaussian_pdf == mp_iFunction[mast][i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTempValues; p_fTempValues = NULL;
    p_fTempValues = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( inverse_gaussian_pdf == mp_iFunction[mast][i] ) {
        p_fTempValues[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {
      //Mu
      FillSpeciesSpecificValue( p_oElement, "di_nonSpatialMastInvGaussMu",
          "di_nsmigmVal", p_fTempValues, iNumFunctionSpecies, p_oPop, true );

      //None are allowed to be 0 or less
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        if (p_fTempValues[i].val <= 0) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clMastingNonSpatialDisperse::DoShellSetup" ;
          stcErr.sMoreInfo = "Inverse gaussian mu values must be greater than zero.";
          throw( stcErr );
        }
      }

      //Transfer to permanent array - slightly kludgily because we don't
      //have an easy way to translate between species indexes
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        for (j = 0; i < m_iNumBehaviorSpecies; j++) {
          if (p_fTempValues[i].code == mp_iWhatSpecies[j]) {
            mp_fInvGaussMu[mast][j] = p_fTempValues[i].val;
            break;
          }
        }
      }

      //Lambda
      FillSpeciesSpecificValue( p_oElement, "di_nonSpatialMastInvGaussLambda",
          "di_nsmiglVal", p_fTempValues, iNumFunctionSpecies, p_oPop, true );

      //None are allowed to be 0 or less
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        if (p_fTempValues[i].val <= 0) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clMastingNonSpatialDisperse::DoShellSetup" ;
          stcErr.sMoreInfo = "Inverse gaussian lambda values must be greater than zero.";
          throw( stcErr );
        }
      }

      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        for (j = 0; i < m_iNumBehaviorSpecies; j++) {
          if (p_fTempValues[i].code == mp_iWhatSpecies[j]) {
            mp_fInvGaussLambda[mast][j] = p_fTempValues[i].val;
            break;
          }
        }
      }
    } //end of if (iNumFunctionSpecies > 0) {

    //Parameters for masting normal distribution
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
     if ( normal_pdf == mp_iFunction[mast][i] )
       iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTempValues; p_fTempValues = NULL;
    p_fTempValues = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( normal_pdf == mp_iFunction[mast][i] ) {
        p_fTempValues[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {
      //Mean
      FillSpeciesSpecificValue( p_oElement, "di_nonSpatialMastNormalMean",
          "di_nsmnmVal", p_fTempValues, iNumFunctionSpecies, p_oPop, true );
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        for (j = 0; i < m_iNumBehaviorSpecies; j++) {
          if (p_fTempValues[i].code == mp_iWhatSpecies[j]) {
            mp_fNormalMean[mast][j] = p_fTempValues[i].val;
            break;
          }
        }
      }

      //Standard deviation
      FillSpeciesSpecificValue( p_oElement, "di_nonSpatialMastNormalStdDev",
          "di_nsmnsdVal", p_fTempValues, iNumFunctionSpecies, p_oPop, true );
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        for (j = 0; i < m_iNumBehaviorSpecies; j++) {
          if (p_fTempValues[i].code == mp_iWhatSpecies[j]) {
            mp_fNormalStandardDev[mast][j] = p_fTempValues[i].val;
            break;
          }
        }
      }
    } //end of if (iNumFunctionSpecies > 0) {

    //Parameters for non-masting inverse gaussian distribution
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      if ( inverse_gaussian_pdf == mp_iFunction[nonmast][i] )
        iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTempValues; p_fTempValues = NULL;
    p_fTempValues = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( inverse_gaussian_pdf == mp_iFunction[nonmast][i] ) {
        p_fTempValues[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {
      //Mu
      FillSpeciesSpecificValue( p_oElement, "di_nonSpatialNonMastInvGaussMu",
          "di_nsnmigmVal", p_fTempValues, iNumFunctionSpecies, p_oPop, true );

      //None are allowed to be 0 or less
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        if (p_fTempValues[i].val <= 0) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clMastingNonSpatialDisperse::DoShellSetup" ;
          stcErr.sMoreInfo = "Inverse gaussian mu values must be greater than zero.";
          throw( stcErr );
        }
      }

      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        for (j = 0; i < m_iNumBehaviorSpecies; j++) {
          if (p_fTempValues[i].code == mp_iWhatSpecies[j]) {
            mp_fInvGaussMu[nonmast][j] = p_fTempValues[i].val;
            break;
          }
        }
      }

      //Lambda
      FillSpeciesSpecificValue( p_oElement, "di_nonSpatialNonMastInvGaussLambda",
          "di_nsnmiglVal", p_fTempValues, iNumFunctionSpecies, p_oPop, true );

      //None are allowed to be 0 or less
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        if (p_fTempValues[i].val <= 0) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          stcErr.sFunction = "clMastingNonSpatialDisperse::DoShellSetup" ;
          stcErr.sMoreInfo = "Inverse gaussian lambda values must be greater than zero.";
          throw( stcErr );
        }
      }

      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        for (j = 0; i < m_iNumBehaviorSpecies; j++) {
          if (p_fTempValues[i].code == mp_iWhatSpecies[j]) {
            mp_fInvGaussLambda[nonmast][j] = p_fTempValues[i].val;
            break;
          }
        }
      }
    } //end of if (iNumFunctionSpecies > 0) {

    //Parameters for non-masting normal distribution
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
     if ( normal_pdf == mp_iFunction[nonmast][i] )
       iNumFunctionSpecies++;

    //Now declare the float temp array to match and load with these species
    delete[] p_fTempValues; p_fTempValues = NULL;
    p_fTempValues = new doubleVal[iNumFunctionSpecies];
    iNumFunctionSpecies = 0;
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if ( normal_pdf == mp_iFunction[nonmast][i] ) {
        p_fTempValues[iNumFunctionSpecies].code = mp_iWhatSpecies[i];
        iNumFunctionSpecies++;
      }
    }

    if ( iNumFunctionSpecies > 0 )
    {
      //Mean
      FillSpeciesSpecificValue( p_oElement, "di_nonSpatialNonMastNormalMean",
          "di_nsnmnmVal", p_fTempValues, iNumFunctionSpecies, p_oPop, true );
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        for (j = 0; i < m_iNumBehaviorSpecies; j++) {
          if (p_fTempValues[i].code == mp_iWhatSpecies[j]) {
            mp_fNormalMean[nonmast][j] = p_fTempValues[i].val;
            break;
          }
        }
      }

      //Standard deviation
      FillSpeciesSpecificValue( p_oElement, "di_nonSpatialNonMastNormalStdDev",
          "di_nsnmnsdVal", p_fTempValues, iNumFunctionSpecies, p_oPop, true );
      for ( i = 0; i < iNumFunctionSpecies; i++ ) {
        for (j = 0; i < m_iNumBehaviorSpecies; j++) {
          if (p_fTempValues[i].code == mp_iWhatSpecies[j]) {
            mp_fNormalStandardDev[nonmast][j] = p_fTempValues[i].val;
            break;
          }
        }
      }
    } //end of if (iNumFunctionSpecies > 0) {

    delete[] p_fTempValues;
    delete[] p_iTempValues;

  }
  catch ( modelErr & err )
  {
    delete[] p_fTempValues;
    delete[] p_iTempValues;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTempValues;
    delete[] p_iTempValues;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempValues;
    delete[] p_iTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMastingNonSpatialDisperse::DoShellSetup" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// AddSeeds()
////////////////////////////////////////////////////////////////////////////
void clMastingNonSpatialDisperse::AddSeeds() {
  bool *p_bDone = NULL;
  float *p_fNumNewSeeds = NULL;

    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    int iNumXCells = mp_oSeedGrid->GetNumberXCells(), //number x cells in seed grid
        iNumYCells = mp_oSeedGrid->GetNumberYCells(), //number y cells in seed grid
        iDraw; //whether or not to mast
    float fNumGridSeeds, //seeds already in one grid cell
          fNumSeeds,
          fTotalBasalArea=0, //total group basal area
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
    short int i, j, iNumInGroup, iX, iY;
  try {
    p_bDone = new bool[m_iNumBehaviorSpecies]; //for group management
    p_fNumNewSeeds = new float[m_iNumBehaviorSpecies];

    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      p_bDone[i] = false;
      p_fNumNewSeeds[i] = 0;
      mp_iEvent[i] = nonmast;
      mp_fBasalArea[i] = 0;
    }

    if (m_bGroupsUsed) ComputeBasalArea();

    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      //Has this species already been done as part of another group? Only
      //continue if no
      if (false == p_bDone[i]) {

        //Is this a masting event or not?
        iDraw = gsl_ran_binomial(clModelMath::randgen, mp_fBinomialP[i], 1);
        if (iDraw != 0 && iDraw != 1) { //error condition
          modelErr stcErr;
          stcErr.iErrorCode = UNKNOWN;
          stcErr.sMoreInfo = "Invalid binomial random draw.";
          stcErr.sFunction = "clMastingNonSpatialDisperse::AddSeeds" ;
          throw( stcErr );
        }

        //Do the appropriate random draw for this species (group)
        if (mp_iFunction[iDraw][i] == normal_pdf) {
          fNumSeeds = mp_fNormalMean[iDraw][i] +
                           gsl_ran_gaussian(clModelMath::randgen,
                               mp_fNormalStandardDev[iDraw][i]);
        } else if (mp_iFunction[iDraw][i] == inverse_gaussian_pdf) {
            fNumSeeds = clModelMath::InverseGaussianRandomDraw(
                mp_fInvGaussMu[iDraw][i],
                mp_fInvGaussLambda[iDraw][i]);
        }

        if (fNumSeeds < 0) fNumSeeds = 0;

        //Scale from seeds per year to seeds per timestep
        fNumSeeds *= iNumYrs;

        //Split seeds by group basal area - first get total basal area for
        //group
        fTotalBasalArea = 0;
        iNumInGroup = 0;
        for ( j = 0; j < m_iNumBehaviorSpecies; j++ ) {
          if (mp_iGroup[j] == mp_iGroup[i]) {
            fTotalBasalArea += mp_fBasalArea[j];
            iNumInGroup++;
          }
        }

        //If there were no trees in the group - set it up such that the split
        //is equal
        if (0 == fTotalBasalArea) {
          fTotalBasalArea = iNumInGroup;
          for ( j = 0; j < m_iNumBehaviorSpecies; j++ ) {
            if (mp_iGroup[j] == mp_iGroup[i]) {
              mp_fBasalArea[j] = 1.0;
            }
          }
        }

        //Portion out the seeds by relative basal area - even if the species
        //is ungrouped, this will still come out okay
        for ( j = 0; j < m_iNumBehaviorSpecies; j++ ) {
          if (mp_iGroup[j] == mp_iGroup[i]) {
            p_fNumNewSeeds[j] = fNumSeeds * (mp_fBasalArea[j]/fTotalBasalArea);
            p_bDone[j] = true;
            if (iDraw == mast) mp_iEvent[j] = mast;
            else mp_iEvent[j] = nonmast;
          }
        }
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

        for (i = 0; i < m_iNumBehaviorSpecies; i++) {

          //Place the seeds in the seed grid
          mp_oSeedGrid->GetValueOfCell( iX, iY, mp_iNumSeedsCode[mp_iWhatSpecies[i]], & fNumGridSeeds );
          fNumGridSeeds += p_fNumNewSeeds[i] * fArea;
          mp_oSeedGrid->SetValueOfCell( iX, iY, mp_iNumSeedsCode[mp_iWhatSpecies[i]], fNumGridSeeds );
        }
      }
    }

    delete[] p_bDone;
    delete[] p_fNumNewSeeds;
  }
  catch ( modelErr & err )
  {
    delete[] p_bDone;
    delete[] p_fNumNewSeeds;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_bDone;
    delete[] p_fNumNewSeeds;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_bDone;
    delete[] p_fNumNewSeeds;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMastingNonSpatialDisperse::AddSeeds" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ComputeBasalArea()
////////////////////////////////////////////////////////////////////////////
void clMastingNonSpatialDisperse::ComputeBasalArea() {
  short int *p_iIndexes = NULL;
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oAll; //trees representing our query
    clTree * p_oTree; //individual tree
    char cQuery[30]; //query string for tree search
    float fDbh; //tree's dbh
    int iNumSpecies = p_oPop->GetNumberOfSpecies(), iSp, i;

    //Set up a translation array for species indexes for the behavior species
    p_iIndexes = new short int[iNumSpecies];
    for (i = 0; i < iNumSpecies; i++) {
      p_iIndexes[i] = -1;
    }
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      p_iIndexes[mp_iWhatSpecies[i]] = i;
    }

    //Query for all adults
    sprintf( cQuery, "%s%d", "type=", clTreePopulation::adult );

    p_oAll = p_oPop->Find( cQuery );
    p_oTree = p_oAll->NextTree();
    while ( p_oTree )
    {
      iSp = p_oTree->GetSpecies();
      if (p_iIndexes[iSp] > -1) {

        //Get the "basal area" of this tree
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, p_oTree->GetType() ), & fDbh );
        mp_fBasalArea[p_iIndexes[iSp]] += pow( fDbh * 0.5, 2 );

      }
      p_oTree = p_oAll->NextTree();
    }

    delete[] p_iIndexes;
  }
  catch ( modelErr & err )
  {
    delete[] p_iIndexes;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_iIndexes;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_iIndexes;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clMastingNonSpatialDisperse::ComputeBasalArea" ;
    throw( stcErr );
  }
}



////////////////////////////////////////////////////////////////////////////
// GetMastEvent()
////////////////////////////////////////////////////////////////////////////
mastEvent clMastingNonSpatialDisperse::GetMastEvent(int iSp) {
  mastEvent iRet = nonmast;
  int i;
  for (i = 0; i < m_iNumBehaviorSpecies; i++)
    if (mp_iWhatSpecies[i] == iSp) {
      iRet = mp_iEvent[i];
      break;
    }
  return iRet;
}
//----------------------------------------------------------------------------

