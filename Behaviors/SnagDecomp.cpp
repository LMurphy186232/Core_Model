//---------------------------------------------------------------------------
// SnagDecomp.cpp
//---------------------------------------------------------------------------
#include "SnagDecomp.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "ModelMath.h"
#include "Grid.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clSnagDecomp::clSnagDecomp(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try
  {
    //Set the namestring
    m_sNameString = "SnagDecayClassDynamics";
    m_sXMLRoot = "SnagDecayClassDynamics";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 2;
    m_fMinimumVersionNumber = 1;

    //Indicate that this behavior intends to add 1 int, 2 float, and 1 bool tree data members
    m_iNewTreeInts = 2; //decay class and dead (snags)
    m_iNewTreeFloats = 2; //new break height and old break height
    m_iNewTreeBools = 2; //fall (adults)


    mp_fTreefallBeta = NULL;
    mp_fSnagfallBeta = NULL;
    mp_fSnagfallGamma = NULL;

    mp_iAdultFallCode = NULL;
    mp_iNewBreakCode = NULL;
    mp_iDeadCodes = NULL;
    mp_iSnagDCCode = NULL;
    mp_iOldBreakCode = NULL;
    mp_oBasalAreaGrid = NULL;

    m_cQuery = NULL;
    mp_iIndexes = NULL;

   m_fSnagfallZeta = 0;
   m_fTreefallTheta = 0;
   m_fTreefallAlpha = 0;
   m_fSnagfallEta = 0;
   m_fSnagfallAlpha = 0;
   m_iLiveBACode = -1;
   m_fMaxBreakHeight = 0;
   m_fSnagfallKappa = 0;
   m_iNumSpecies = 0;
   m_fTreefallDelta = 0;
   m_fTreefallLamda = 0;
   m_fGridCellLength = 0;
   m_iCutBACode = -1;
   m_fTreefallIota = 0;
   m_fMinBreakHeight = 0;
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
    stcErr.sFunction = "clSnagDecomp::clSnagDecomp" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clSnagDecomp::~clSnagDecomp() {

  delete[] mp_fTreefallBeta;
  delete[] mp_fSnagfallBeta;
  delete[] mp_fSnagfallGamma;

  delete[] m_cQuery;

  for (int i=0; i<m_iNumSpecies; i++) {
    delete[] mp_iNewBreakCode[i];
    delete[] mp_iSnagDCCode[i];
    delete[] mp_iDeadCodes[i];
  }

  delete[] mp_iNewBreakCode;
  delete[] mp_iSnagDCCode;
  delete[] mp_iDeadCodes;

  delete[] mp_iOldBreakCode;
  delete[] mp_iAdultFallCode;

  delete[] mp_iIndexes;

}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clSnagDecomp::GetData(xercesc::DOMDocument * p_oDoc) {
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

    char cQueryTemp[75];
    char cQueryPiece[5]; //for assembling the search query
    int i,j,k; //loop counters
    const static char * cDCArrayCodes[] = {"Live","1", "2", "3", "4", "5"}; //correspond to tags in parameter file
    char cConcatenatedCode[50]; //strings holding names of XML tags
    floatVal *p_fTempValues; //for getting species-specific values
    float fCumulativeProb; //column totals for parameter validation
    float fYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    float * p_fProbGEk; //for holding cumulative transition probabilities during timestep rescaling

    float fPopGridCellArea; //the area of each cell in the tree population grid (used as a basis for cut basal area grid)

    m_iNumSpecies = p_oPop->GetNumberOfSpecies();
    mp_iIndexes = new short int [m_iNumSpecies];

    //Make the list of indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare the temp array and populate it with the species to which this
    //behavior applies
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
    p_fTempValues[i].code = mp_iWhatSpecies[i];

    //*************************
    // Read in parameters
    //*************************


    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    mp_fTreefallBeta = new float [m_iNumBehaviorSpecies];
    mp_fSnagfallBeta = new float [m_iNumBehaviorSpecies];

    //Declare our arrays
    mp_fSnagfallGamma = new float [6];

    //Get the parameter file values
    FillSingleValue( p_oElement, "sd_snagDecompTreefallAlpha", & m_fTreefallAlpha, true );

    FillSpeciesSpecificValue( p_oElement, "sd_snagDecompTreefallBeta", "sd_sdtbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fTreefallBeta[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    FillSingleValue( p_oElement, "sd_snagDecompTreefallDelta", & m_fTreefallDelta, true );
    FillSingleValue( p_oElement, "sd_snagDecompTreefallTheta", & m_fTreefallTheta, true );
    FillSingleValue( p_oElement, "sd_snagDecompTreefallIota", & m_fTreefallIota, true );
    FillSingleValue( p_oElement, "sd_snagDecompTreefallLambda", & m_fTreefallLamda, true );

    FillSingleValue( p_oElement, "sd_snagDecompSnagfallAlpha", & m_fSnagfallAlpha, true );

    FillSingleValue( p_oElement, "sd_minSnagBreakHeight", & m_fMinBreakHeight, true );
    FillSingleValue( p_oElement, "sd_maxSnagBreakHeight", & m_fMaxBreakHeight, true );

    //Make sure min<max and both >0
    if ( m_fMinBreakHeight < 0 || m_fMaxBreakHeight < 0 || m_fMaxBreakHeight < m_fMinBreakHeight)
    {
      modelErr stcErr;
      stcErr.sFunction = "clSnagDecomp::GetData" ;
      stcErr.sMoreInfo = "Max break height must be greater than or equal to min break height, and both values must be non-negative.";
      stcErr.iErrorCode = BAD_DATA;
      delete[] p_fTempValues;
      throw( stcErr );
    } //end if

    FillSpeciesSpecificValue( p_oElement, "sd_snagDecompSnagfallBeta", "sd_sdsbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fSnagfallBeta[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    mp_fSnagfallGamma[0] = 0;
    mp_fSnagfallGamma[1] = 0;
    FillSingleValue( p_oElement, "sd_snagDecompSnagfallGamma2", & mp_fSnagfallGamma[2], true );
    FillSingleValue( p_oElement, "sd_snagDecompSnagfallGamma3", & mp_fSnagfallGamma[3], true );
    FillSingleValue( p_oElement, "sd_snagDecompSnagfallGamma4", & mp_fSnagfallGamma[4], true );
    FillSingleValue( p_oElement, "sd_snagDecompSnagfallGamma5", & mp_fSnagfallGamma[5], true );
    FillSingleValue( p_oElement, "sd_snagDecompSnagfallZeta", & m_fSnagfallZeta, true );
    FillSingleValue( p_oElement, "sd_snagDecompSnagfallEta", & m_fSnagfallEta, true );
    FillSingleValue( p_oElement, "sd_snagDecompSnagfallKappa", & m_fSnagfallKappa, true );

    delete[] p_fTempValues;

    //for each column (initial condition)
    for ( j = 0; j <= 5; j++) {

      //initialize cumulative probability to zero
      fCumulativeProb=0;

      //for each row (final condition)
      for (k = j; k <=5; k++) {
        if (k == 0) {
          mp_fSnagTransProb[j][k]=0.0; //no live-to-live possibility
        }
        else {

          //Make strings with names of the appropriate tag and subtag
          sprintf(cConcatenatedCode, "sd_snagDecomp%sTo%sProb", cDCArrayCodes[j],cDCArrayCodes[k]);
          FillSingleValue( p_oElement, cConcatenatedCode, & mp_fSnagTransProb[j][k], true );
          fCumulativeProb += mp_fSnagTransProb[j][k];

          //Make sure values are non-negative
          if ( mp_fSnagTransProb[j][k] < 0.0 )
          {
            modelErr stcErr;
            stcErr.sFunction = "clSnagDecomp::GetData" ;
            stcErr.sMoreInfo = "Values for transition probabilites cannot be less than 0.";
            stcErr.iErrorCode = BAD_DATA;
            throw( stcErr );
          } //end if
        } //end if (k, matrix rows)
      } //end for (k, matrix rows)

      //Make sure column probabilities add up to one.
      if ( fabs(fCumulativeProb - 1.0) > 0.0001 )
      {
        modelErr stcErr;
        stcErr.sFunction = "clSnagDecomp::GetData" ;
        stcErr.sMoreInfo = "Sum of transition probabilities must add up to 1 within each column.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      } //end if


      /****************************************
       * re-scale transition matrix for timestep
       * other than 5 years
       * *************************************/
      if (fabs((fYearsPerTimestep)-5.0)>0.1) {
        p_fProbGEk = new float [7];
        //calculate cumulative transition probabilities
        p_fProbGEk[6]=0.0;
        for (k=5;k>=j;k--) {
          p_fProbGEk[k] = p_fProbGEk[k+1] + mp_fSnagTransProb[j][k];
        }
        //for (k=j;k<=5;k++) {
        for (k=5;k>=j;k--) {
          //re-scale cumulative probabilities
          p_fProbGEk[k] = 1 - pow((1 - p_fProbGEk[k]),fYearsPerTimestep/5.0);
          //calculate probabilities for each state
          mp_fSnagTransProb[j][k] = p_fProbGEk[k] - p_fProbGEk[k+1];
        }
        delete[] p_fProbGEk;
      } //end if (rescaling)

    } //end for (j, matrix columns)

    //*************************
    // Format query string
    //*************************

    //Do a species search on all the species
    strcpy( cQueryTemp, "species=" );
    for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
    {
      sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
      strcat( cQueryTemp, cQueryPiece );
    }

    sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
    strcat( cQueryTemp, cQueryPiece );

    strcat( cQueryTemp, "::type=" );
    sprintf( cQueryPiece, "%d,%d", clTreePopulation::adult, clTreePopulation::snag);
    strcat( cQueryTemp, cQueryPiece );

    //Now put it in m_cQuery, sized correctly
    m_cQuery = new char[strlen( cQueryTemp ) + 1];
    strcpy( m_cQuery, cQueryTemp );

    /************************************************
     * Set-up the basal area and cut basal area grids
     * **********************************************/

    /* Find the resolution for the basal area grids that is
     * the closest square multiple of the tree population
     * grid to 400 m2.  This is so the grid cells are compatible
     * with the harvest results grid. */
    fPopGridCellArea = pow(p_oPop->GetGridCellSize(),2);

    i=1;
    while (pow(i,2)*fPopGridCellArea < 400)
    i++;

    if ((400 - pow(i-1,2)*fPopGridCellArea) < (pow(i,2)*fPopGridCellArea - 400))
    m_fGridCellLength = (i-1)*pow(fPopGridCellArea,0.5);
    else
    m_fGridCellLength = i*pow(fPopGridCellArea,0.5);

    //create the basal area grid
    mp_oBasalAreaGrid = mp_oSimManager->CreateGrid( "Snag Decay Class Dynamics Basal Area", //grid name
        0, //number of int data members
        2, //num float data members
        0, //number of char data members
        0, //number of bool data members
        m_fGridCellLength, //X cell length
        m_fGridCellLength ); //Y cell length

    //Register the data members
    m_iLiveBACode = mp_oBasalAreaGrid->RegisterFloat( "Live BA Per Ha" );
    m_iCutBACode = mp_oBasalAreaGrid->RegisterFloat( "Cut BA Per Ha" );

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
    stcErr.sFunction = "clSnagDecomp::GetData" ;
    throw( stcErr );
  }
}



//////////////////////////////////////////////////////////////////////////////
// CalcGridValues()
//////////////////////////////////////////////////////////////////////////////
void clSnagDecomp::CalcGridValues() {
  try
  {

    /***************************************
     * Calculations for the live basal area grid
     * **************************************/

    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oTreeSearch;
    clTree * p_oTree;
    char * cQuery = new char[10];

    //retrieve list of live adult trees and snags
    sprintf( cQuery, "type=%d,%d", clTreePopulation::adult, clTreePopulation::snag);
    p_oTreeSearch = p_oPop->Find(cQuery);
    delete[] cQuery;

    //tree species, dbh, and position
    int iSp,iOldSnag;
    float fDBH, fXPosition, fYPosition,
    fTempBAHolder; //temp value used in calculating basal area

    const float fZero = 0.0; //used to represent the float value 0.0


    //size of the basal area grid
    short int iNumXCells = mp_oBasalAreaGrid->GetNumberXCells(),
    iNumYCells = mp_oBasalAreaGrid->GetNumberYCells(),
    i,j; //loop counters

    //reset all values for the basal area grid
    for ( i = 0; i < iNumXCells; i++ )
    for ( j = 0; j < iNumYCells; j++ )
    mp_oBasalAreaGrid->SetValueOfCell(i, j, m_iLiveBACode, fZero);

    p_oTree = p_oTreeSearch->NextTree();

    while ( p_oTree )
    {

      //find species and decay class
      iSp = p_oTree->GetSpecies();

      if (p_oTree->GetType()==clTreePopulation::snag)
      p_oTree->GetValue(mp_iSnagDCCode[iSp][clTreePopulation::snag], & iOldSnag);
      else
      iOldSnag=0;

      //proceed if the tree is alive, or if it is a snag that was created this timestep
      if (!iOldSnag) {

        //get the necessary tree info
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, clTreePopulation::adult), & fDBH );
        p_oTree->GetValue( p_oPop->GetXCode( iSp, clTreePopulation::adult), & fXPosition);
        p_oTree->GetValue( p_oPop->GetYCode( iSp, clTreePopulation::adult), & fYPosition);

        //get the BA so far in that tree's cell
        mp_oBasalAreaGrid->GetValueAtPoint(fXPosition, fYPosition, m_iLiveBACode, & fTempBAHolder);

        //add the current tree's BA (converted to per ha) to it
        fTempBAHolder += (clModelMath::CalculateBasalArea(fDBH)) / (pow(m_fGridCellLength/100.0,2));

        //write the new BA back to the grid
        mp_oBasalAreaGrid->SetValueAtPoint(fXPosition, fYPosition, m_iLiveBACode, fTempBAHolder);

      } //end if

      p_oTree = p_oTreeSearch->NextTree();

    } //end while (p_oTree)

    //adjust basal area for smaller area of edge cells

    for (i=0; i<iNumXCells;i++) {
      mp_oBasalAreaGrid->GetValueOfCell(i, iNumYCells-1, m_iLiveBACode, & fTempBAHolder);
      fTempBAHolder *= m_fGridCellLength; //multiply by old length
      fTempBAHolder /= mp_oBasalAreaGrid->GetEndYOfCell(iNumYCells-1) - mp_oBasalAreaGrid->GetOriginYOfCell(iNumYCells-1); //divide by new length
      mp_oBasalAreaGrid->SetValueOfCell(i, iNumYCells-1, m_iLiveBACode, fTempBAHolder);
    }
    for (j=0; j<iNumYCells;j++) {
      mp_oBasalAreaGrid->GetValueOfCell(iNumXCells-1, j, m_iLiveBACode, & fTempBAHolder);
      fTempBAHolder *= m_fGridCellLength; //multiply by old length
      fTempBAHolder /= mp_oBasalAreaGrid->GetEndXOfCell(iNumXCells-1) - mp_oBasalAreaGrid->GetOriginXOfCell(iNumXCells-1); //divide by new length
      mp_oBasalAreaGrid->SetValueOfCell(iNumXCells-1, j, m_iLiveBACode, fTempBAHolder);
    }

    /***************************************
     * Calculations for the cut basal area grid
     * **************************************/

    clGrid * p_oHarvestResultsGrid = mp_oSimManager->GetGridObject("Harvest Results");

    float fCellBACut, //temp value used in calculating cut basal area
    fTempCellBACut, //temp value used in calculating cut basal area
    //position of cells in the harvest results grid
    fHarvestResultsXPosition,
    fHarvestResultsYPosition;

    int iCellCut; // -1 if the cell was not cut this timestep

    short int
    //cell reference within basal area grid
    iXCell,
    iYCell;

    //reset all values for the cut basal area grid
    for ( i = 0; i < iNumXCells; i++ )
    for ( j = 0; j < iNumYCells; j++ )
    mp_oBasalAreaGrid->SetValueOfCell( i, j, m_iCutBACode, fZero);

    //if the harvest results grid exists
    //(created by clDisturbance and related behaviors)
    if (p_oHarvestResultsGrid) {

      short int //size of the harvest results grid
      iNumHarvestResultsXCells = p_oHarvestResultsGrid->GetNumberXCells(),
      iNumHarvestResultsYCells = p_oHarvestResultsGrid->GetNumberYCells(),
      k; //loop counter

      for ( i = 0; i < iNumHarvestResultsXCells; i++ )
      for ( j = 0; j < iNumHarvestResultsYCells; j++ ) {

        p_oHarvestResultsGrid->GetValueOfCell(i, j, p_oHarvestResultsGrid->GetIntDataCode("Harvest Type"), & iCellCut);

        //if cutting occurred in the cell this timestep
        if (iCellCut != -1) {

          //calculate the cut basal area for this cell across species
          //and size classes (indexed by k)
          fCellBACut = 0.0;
          for (k=0; k < p_oHarvestResultsGrid->GetNumberFloatDataMembers(); k++) {

            p_oHarvestResultsGrid->GetValueOfCell(i,j,k, & fTempCellBACut);

            fCellBACut += fTempCellBACut;

          } //end for (k)

          //convert to per ha
          fCellBACut /= (pow(m_fGridCellLength,2)/10000);

          //find the cell in BasalArea we're in, then add the cut BA value to it (both cut and live ba)
          p_oHarvestResultsGrid->GetPointOfCell(i,j, & fHarvestResultsXPosition, & fHarvestResultsYPosition);

          //correct basal area if we're on one of the far edges
          mp_oBasalAreaGrid->GetCellOfPoint(fHarvestResultsXPosition, fHarvestResultsYPosition, &iXCell, &iYCell);

          if (iXCell==(iNumXCells-1)) {
            fCellBACut *= m_fGridCellLength; //multiply by old length
            fCellBACut /= mp_oBasalAreaGrid->GetEndXOfCell(iNumXCells-1) - mp_oBasalAreaGrid->GetOriginXOfCell(iNumXCells-1); //divide by new length
          }
          if (iXCell==(iNumXCells-1)) {
            fCellBACut *= m_fGridCellLength; //multiply by old length
            fCellBACut /= mp_oBasalAreaGrid->GetEndYOfCell(iNumYCells-1) - mp_oBasalAreaGrid->GetOriginYOfCell(iNumYCells-1); //divide by new length
          }

          //add to cut ba
          mp_oBasalAreaGrid->GetValueAtPoint(fHarvestResultsXPosition,fHarvestResultsYPosition, m_iCutBACode, & fTempCellBACut);
          fTempCellBACut += fCellBACut;
          mp_oBasalAreaGrid->SetValueAtPoint(fHarvestResultsXPosition,fHarvestResultsYPosition, m_iCutBACode, fTempCellBACut);
          //add to live ba
          mp_oBasalAreaGrid->GetValueAtPoint(fHarvestResultsXPosition,fHarvestResultsYPosition, m_iLiveBACode, & fTempCellBACut);
          fTempCellBACut += fCellBACut;
          mp_oBasalAreaGrid->SetValueAtPoint(fHarvestResultsXPosition,fHarvestResultsYPosition, m_iLiveBACode, fTempCellBACut);

        } //end if CellCut

      } //end for loops (HarvestResults cells)

      //for testing purposes

      //for (i=0;i<iNumXCells;i++) {
      //	for (j=0;j<iNumYCells;j++) {
      //		mp_oBasalAreaGrid->GetValueOfCell(i,j, m_iLiveBACode, & fTempCellBACut);
      //		printf("%2.20f\t", fTempCellBACut);
      //		mp_oBasalAreaGrid->GetValueOfCell(i,j, m_iCutBACode, & fTempCellBACut);
      //		printf("%2.8f\n", fTempCellBACut);
      //		}
      //	printf("\n");
      //	}


    } //end if harvest results grid exists


  } //end try
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
    stcErr.sFunction = "clSnagDecomp::CalcGridValues" ;
    throw( stcErr );
  }
}



////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clSnagDecomp::Action() {
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    float fRandom, //random number
    fFallProb = 0, //modelled probability of tree or snag fall
    fBreakHeight,
    fHeight, fDBH, fXPosition, fYPosition, //snag info
    fLiveBasalArea, fCutBasalArea, //live basal area and cut basal area values for the cell a snag is in
    fNegOne = -1.0;
    short int iSp, iTp, //the tree's species, type
    iFallDataCode;
    int iInitCondition, //the snag's starting decay class
        iDead,
    iNewCondition, //the snag's new decay class for the cell the snag is in
    iCutOrNot; //indicator of cutting in the cell the snag is in

    clGrid *p_oDetailedSubstrateGrid;

    p_oDetailedSubstrateGrid = mp_oSimManager->GetGridObject( "DetailedSubstrate" );

    //calculate our basal area and cut basal area grid values
    //for this timestep
    CalcGridValues();

    //Get the trees (snags) that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      //Get the snag's species, type, height, DBH, position, and initial condition
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      p_oTree->GetValue(mp_iDeadCodes[iSp][iTp], &iDead);

      if (iTp==clTreePopulation::adult && notdead == iDead) //tree is still alive
      goto nextTree;

      p_oTree->GetValue(p_oPop->GetHeightCode(iSp,iTp), & fHeight);
      p_oTree->GetValue(p_oPop->GetDbhCode(iSp,iTp), & fDBH);
      p_oTree->GetValue(mp_iSnagDCCode[iSp][iTp], & iInitCondition);
      p_oTree->GetValue( p_oPop->GetXCode( iSp, iTp), & fXPosition);
      p_oTree->GetValue( p_oPop->GetYCode( iSp, iTp), & fYPosition);

      //Get the basal area and cut basal area values for the cell the snag is in
      mp_oBasalAreaGrid->GetValueAtPoint(fXPosition, fYPosition, m_iLiveBACode, & fLiveBasalArea);
      mp_oBasalAreaGrid->GetValueAtPoint(fXPosition, fYPosition, m_iCutBACode, & fCutBasalArea);

      //indicator of whether cutting occurred or not
      if (fCutBasalArea < 0.1)
      iCutOrNot = 0;
      else
      iCutOrNot = 1;

      //get random number to find whether the snag is standing at end of time step
      fRandom = clModelMath::GetRand();

      if (iTp==clTreePopulation::adult) { // tree died this timestep, apply tree fall model
        fFallProb = m_fTreefallAlpha + mp_fTreefallBeta[mp_iIndexes[iSp]] + m_fTreefallDelta*fDBH + m_fTreefallTheta*fLiveBasalArea + (m_fTreefallIota + m_fTreefallLamda*fDBH)*iCutOrNot;
        //values to be used later
        iInitCondition = 0;
        iFallDataCode = mp_iAdultFallCode[iSp];

      }

      else { //tree was already a snag, apply snag fall model
        fFallProb = m_fSnagfallAlpha + mp_fSnagfallBeta[mp_iIndexes[iSp]] +
                    mp_fSnagfallGamma[iInitCondition] +
                    m_fSnagfallZeta*log(fDBH) +
                    m_fSnagfallEta*pow(log(fDBH),2) +
                    m_fSnagfallKappa*fCutBasalArea;
        //values to be used later
        iFallDataCode = mp_iDeadCodes[iSp][iTp];
        p_oTree->SetValue(mp_iOldBreakCode[iSp], fNegOne);
        p_oTree->GetValue(mp_iNewBreakCode[iSp][iTp], &fBreakHeight);
        //if it broke last timestep, transfer break height to old break height member
        if (fabs(fBreakHeight - fNegOne) > 0.0001 ) {

          p_oTree->SetValue(mp_iOldBreakCode[iSp], fBreakHeight);
          p_oTree->SetValue(mp_iNewBreakCode[iSp][iTp], fNegOne);

        } //end if

      } //end else

      if (iTp==clTreePopulation::adult) {
        p_oTree->SetValue(iFallDataCode, false);
      } else {
        p_oTree->SetValue(iFallDataCode, notdead);
      }

      if (fFallProb > 20)
      fFallProb = 0.999999; //avoid overflow
      else if (fFallProb < -20)
      fFallProb = 0.000001; //avoid underflow
      else {
        fFallProb = exp(fFallProb) / (1 + exp(fFallProb));
        //adjust for timestep length
        fFallProb = 1 - pow(1-fFallProb,(mp_oSimManager->GetNumberOfYearsPerTimestep())/5.0);
      }

      if (fRandom < fFallProb) { //it fell over

        if (iTp==clTreePopulation::adult) {
          p_oTree->SetValue(iFallDataCode, true);
          //if it died and fell this timestep, and substrate isn't being used, remove the tree
          if (!p_oDetailedSubstrateGrid)
          p_oPop->KillTree( p_oTree, remove_tree );
        } else {
          p_oTree->SetValue(iFallDataCode, natural);
        }


      } else { //it didn't fall over, run DC transition

        //get random number to find the snag's new decay class
        fRandom = clModelMath::GetRand();

        iNewCondition = iInitCondition - 1;
        while (fRandom > 0.0) {
          iNewCondition++;
          fRandom -= mp_fSnagTransProb[iInitCondition][iNewCondition];
        }

        p_oTree->SetValue(mp_iNewBreakCode[iSp][iTp], fNegOne);

        //assign its new condition (if it stayed in the same DC, nothing happens)
        if (iNewCondition > iInitCondition) { // it advanced to a later snag DC
          p_oTree->SetValue( mp_iSnagDCCode[iSp][iTp], iNewCondition );

          //if new dc = 5 and init dc wasn't, assign value to new break data member
          if (iNewCondition == 5) {

            fRandom = clModelMath::GetRand();
            fRandom *= (m_fMaxBreakHeight - m_fMinBreakHeight);
            fRandom += m_fMinBreakHeight;

            if (fRandom < fHeight)
            p_oTree->SetValue(mp_iNewBreakCode[iSp][iTp], fRandom);

          } //end if (new dc 5)

        }//end if (advances dc)


      } //end else (no fall)

      nextTree:
      p_oTree = p_oBehaviorTrees->NextTree();

    } //end while (p_oTree)

  } //end try
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
    stcErr.sFunction = "clSnagDecomp::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clSnagDecomp::RegisterTreeDataMembers() {

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  char cDeadLabel[] = "dead";
  short int i;

  m_iNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data members
  mp_iSnagDCCode = new short int * [m_iNumSpecies];
  mp_iNewBreakCode = new short int * [m_iNumSpecies];
  mp_iDeadCodes = new short int * [m_iNumSpecies];

  for (i=0; i<m_iNumSpecies; i++) {
    mp_iSnagDCCode[i] = new short int [clTreePopulation::snag + 1];
    mp_iNewBreakCode[i] = new short int [clTreePopulation::snag + 1];
    mp_iDeadCodes[i] = new short int [clTreePopulation::snag + 1];

  } //end for

  mp_iAdultFallCode = new short int [m_iNumSpecies];
  mp_iOldBreakCode = new short int [m_iNumSpecies];

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {

    //Make sure that only allowed types have been applied
    if (clTreePopulation::snag != mp_whatSpeciesTypeCombos[i].iType
        && clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType) {

      modelErr stcErr;
      stcErr.sFunction = "clSnagDecomp::RegisterTreeDataMembers";
      stcErr.sMoreInfo = "This behavior can only be applied to adults and snags.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Register the decay class code for adults and snags
    //Register the new break height code for adults and snags
    mp_iSnagDCCode[mp_whatSpeciesTypeCombos[i].iSpecies][mp_whatSpeciesTypeCombos[i].iType]
        = p_oPop->RegisterInt("SnagDecayClass",
            mp_whatSpeciesTypeCombos[i].iSpecies,
            mp_whatSpeciesTypeCombos[i].iType);

    mp_iNewBreakCode[mp_whatSpeciesTypeCombos[i].iSpecies][mp_whatSpeciesTypeCombos[i].iType]
        = p_oPop->RegisterFloat("NewBreakHeight",
            mp_whatSpeciesTypeCombos[i].iSpecies,
            mp_whatSpeciesTypeCombos[i].iType);

    if (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::adult) {

      //get the adult dead data member code
      mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
          = -1;

      mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
          = p_oPop->GetIntDataCode(cDeadLabel,
              mp_whatSpeciesTypeCombos[i].iSpecies,
              mp_whatSpeciesTypeCombos[i].iType);

      if (mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
          == -1) {
        modelErr stcErr;
        stcErr.sFunction = "clSnagDecomp::RegisterTreeDataMembers";
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " must have a mortality behavior assigned to be compatible with snag dynamics.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //register the adult fall data member
      mp_iAdultFallCode[mp_whatSpeciesTypeCombos[i].iSpecies]
          = p_oPop->RegisterBool("Fall", mp_whatSpeciesTypeCombos[i].iSpecies,
              clTreePopulation::adult);

    } //end if

    else if (mp_whatSpeciesTypeCombos[i].iType == clTreePopulation::snag) { //snag

      //register the snag dead data member code (or just get it if it's already registered)
      mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
          = -1;

      mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
          = p_oPop->GetIntDataCode(cDeadLabel,
              mp_whatSpeciesTypeCombos[i].iSpecies,
              mp_whatSpeciesTypeCombos[i].iType);

      if (mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
          == -1)
        mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]
            = p_oPop->RegisterInt(cDeadLabel,
                mp_whatSpeciesTypeCombos[i].iSpecies,
                mp_whatSpeciesTypeCombos[i].iType);

      //register the snag old break height data member
      mp_iOldBreakCode[mp_whatSpeciesTypeCombos[i].iSpecies]
          = p_oPop->RegisterFloat("SnagOldBreakHeight",
              mp_whatSpeciesTypeCombos[i].iSpecies, clTreePopulation::snag);

    } //end else


  } //end for combos


}
