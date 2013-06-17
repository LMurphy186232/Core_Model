//---------------------------------------------------------------------------
// NeighborhoodSeedPredation.cpp
//---------------------------------------------------------------------------
#include "NeighborhoodSeedPredation.h"
#include "FuncResponseSeedPredation.h"
#include "Grid.h"
#include "ModelMath.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Plot.h"
#include "TreePopulation.h"
#include "MastingSpatialDisperse.h"
#include "MastingNonSpatialDisperse.h"
#include <math.h>
#include <fstream>
#include <sstream>

clGrid *clNeighborhoodSeedPredation::mp_oOutputGrid = NULL;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clNeighborhoodSeedPredation::clNeighborhoodSeedPredation(
    clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try
  {
    m_sNameString = "NeighborhoodSeedPredation";
    m_sXMLRoot = "NeighborhoodSeedPredation";

    mp_fNonMastingP0 = NULL;
    mp_fNonMastingPn = NULL;
    mp_fMastingP0 = NULL;
    mp_fMastingPn = NULL;
    mp_fMinNeighDBH = NULL;
    mp_bCountInMast = NULL;
    mp_iSeedGridCode = NULL;
    mp_iPropEatenCode = NULL;
    mp_iStartSeedsCode = NULL;
    mp_iIndexes = NULL;

    mp_oSeedGrid = NULL;
    m_fRadius = 0;
    m_fMastingThreshold = 0;
    m_bIsLinked = false;
    m_fPlotArea = 0;
    m_bUseThresholdToDecideMast = false;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

  }
  catch (modelErr& err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clNeighborhoodSeedPredation::clNeighborhoodSeedPredation";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clNeighborhoodSeedPredation::~clNeighborhoodSeedPredation() {
  int i;
  if (mp_fNonMastingPn)
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      delete[] mp_fNonMastingPn[i];

  if (mp_fMastingPn)
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      delete[] mp_fMastingPn[i];

  delete[] mp_fNonMastingP0;
  delete[] mp_fNonMastingPn;
  delete[] mp_fMastingP0;
  delete[] mp_fMastingPn;
  delete[] mp_iSeedGridCode;
  delete[] mp_iStartSeedsCode;
  delete[] mp_iPropEatenCode;
  delete[] mp_iIndexes;
  delete[] mp_fMinNeighDBH;
  delete[] mp_bCountInMast;

  //De-ref for future setup
  mp_oOutputGrid = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Action
/////////////////////////////////////////////////////////////////////////////
void clNeighborhoodSeedPredation::Action() {

  //Get the starting seed rain
  float fSeeds, fProp;
  int iNumXCells = mp_oSeedGrid->GetNumberXCells(),
      iNumYCells = mp_oSeedGrid->GetNumberYCells(), iX, iY, iSp;

  for ( iX = 0; iX < iNumXCells; iX++ ) {
    for ( iY = 0; iY < iNumYCells; iY++ ) {
      for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
        mp_oSeedGrid->GetValueOfCell(iX, iY, mp_iSeedGridCode[iSp], &fSeeds);
        mp_oOutputGrid->SetValueOfCell(iX, iY, mp_iStartSeedsCode[mp_iWhatSpecies[iSp]], fSeeds);
      }
    }
  }

  if (m_bIsLinked)
    DoLinkedPredation();
  else
    DoStandalonePredation();

  //Get the proportion eaten
  for ( iX = 0; iX < iNumXCells; iX++ ) {
    for ( iY = 0; iY < iNumYCells; iY++ ) {
      for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
        mp_oSeedGrid->GetValueOfCell(iX, iY, mp_iSeedGridCode[iSp], &fProp);
        mp_oOutputGrid->GetValueOfCell(iX, iY, mp_iStartSeedsCode[mp_iWhatSpecies[iSp]], &fSeeds);
        if (fSeeds == 0) fProp = 0; else fProp = 1 - (fProp/fSeeds);
        mp_oOutputGrid->SetValueOfCell(iX, iY, mp_iPropEatenCode[mp_iWhatSpecies[iSp]], fProp);
      }
    }
  }

}

/////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clNeighborhoodSeedPredation::GetData(DOMDocument * p_oDoc) {

  ReadParameterFileData(p_oDoc);
  SetupGrids();

  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();

  //Calculate the plot areas
  m_fPlotArea = p_oPlot->GetXPlotLength() * p_oPlot->GetYPlotLength();

  //If linked, make sure the functional response seed predation behavior is
  //present
  if (m_bIsLinked) {
    if (NULL == mp_oSimManager->GetBehaviorObject("LinkedFunctionalResponseSeedPredation")) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clNeighborhoodSeedPredation::GetData" ;
      stcErr.sMoreInfo = "You must use the functional response seed predation behavior.";
      throw( stcErr );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clNeighborhoodSeedPredation::SetNameData(std::string sNameString) {

  //Check the string passed and set the flags accordingly
  if (sNameString.compare("LinkedNeighborhoodSeedPredation") == 0) {
    m_bIsLinked = true;
    m_sNameString = "NeighborhoodSeedPredation";
  } else {
    m_bIsLinked = false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// DoStandalonePredation
///////////////////////////////////////////////////////////////////////////////
void clNeighborhoodSeedPredation::DoStandalonePredation() {
  float *p_fOfftakes = new float[m_iNumBehaviorSpecies];
  try {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch *p_oAllTrees = p_oPop->Find("type=3");
    clTree *p_oTree = p_oAllTrees->NextTree();
    float *p_fP0, **p_fPn, //stand-in for parameter arrays
    fNumSeeds = 0, fNewSeeds;
    int iNumXCells = mp_oSeedGrid->GetNumberXCells(),
    iNumYCells = mp_oSeedGrid->GetNumberYCells(),
    iX, iY, iSp;
    bool bMasting;

    //Check to make sure plot adult tree density is not zero - if it is, it is
    //automatically not masting
    if (NULL == p_oTree) bMasting = false;
    else {
      if (m_bUseThresholdToDecideMast) {
        //Are we masting? Find the seed density
        for ( iX = 0; iX < iNumXCells; iX++ ) {
          for ( iY = 0; iY < iNumYCells; iY++ ) {
            for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
              if (mp_bCountInMast[iSp]) {
                mp_oSeedGrid->GetValueOfCell(iX, iY, mp_iSeedGridCode[iSp], &fNewSeeds);
                fNumSeeds += fNewSeeds;
              }
            }
          }
        }
        //Set the parameters appropriate to the situation
        if (fNumSeeds / m_fPlotArea <= m_fMastingThreshold) bMasting = false;
        else bMasting = true;
      } else {
        //Ask the disperse behaviors if masting occurred
        bMasting = false;
        clBehaviorBase *p_oTemp = mp_oSimManager->GetBehaviorObject("MastingSpatialDisperse");
        if (p_oTemp != NULL){
          clMastingSpatialDisperse *p_oDisperse1 = dynamic_cast<clMastingSpatialDisperse*>(p_oTemp);
          for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
            if (p_oDisperse1->GetMastEvent(mp_iWhatSpecies[iSp]) == mast) {
              bMasting = true; break;
            }
          }
        }
        if (bMasting == false) {
          p_oTemp = mp_oSimManager->GetBehaviorObject("MastingNonSpatialDisperse");
          if (p_oTemp != NULL){
            clMastingNonSpatialDisperse *p_oDisperse2= dynamic_cast<clMastingNonSpatialDisperse*>(p_oTemp);
            for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
              if (p_oDisperse2->GetMastEvent(mp_iWhatSpecies[iSp]) == mast) {
                bMasting = true; break;
              }
            }
          }
        }
      }
    }
    if (bMasting == false){
      //Non-masting
      p_fP0 = mp_fNonMastingP0;
      p_fPn = mp_fNonMastingPn;
    } else {
      //Masting
      p_fP0 = mp_fMastingP0;
      p_fPn = mp_fMastingPn;
    }

    for ( iX = 0; iX < iNumXCells; iX++ ) {
      for ( iY = 0; iY < iNumYCells; iY++ ) {

        GetOfftakes(p_oPop, p_fP0, p_fPn, p_fOfftakes, iX, iY);

        //Go through the species and reduce seeds appropriately
        for ( iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++ ) {
          mp_oSeedGrid->GetValueOfCell( iX, iY, mp_iSeedGridCode[iSp], & fNumSeeds );

          if (fNumSeeds > 0) {

            //Reduce the number of seeds appropriately
            fNewSeeds = fNumSeeds * (1-p_fOfftakes[iSp]);

            if ( fNumSeeds < 0 ) fNumSeeds = 0;
            if ( fNewSeeds > fNumSeeds) fNewSeeds = fNumSeeds;

            mp_oSeedGrid->SetValueOfCell( iX, iY, mp_iSeedGridCode[iSp], fNewSeeds );
          }
        }
      }
    }

    delete[] p_fOfftakes;
  }
  catch ( ... )
  {
    delete[] p_fOfftakes;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clNeighborhoodSeedPredation::DoStandalonePredation" ;
    throw( stcErr );
  }

}

//////////////////////////////////////////////////////////////////////////////
// DoLinkedPredation()
//////////////////////////////////////////////////////////////////////////////
void clNeighborhoodSeedPredation::DoLinkedPredation() {
  float ***p_fOfftakes= NULL;
  int iNumXCells = mp_oSeedGrid->GetNumberXCells(), iNumYCells =
      mp_oSeedGrid->GetNumberYCells(), iX, iY;
  try {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clBehaviorBase *p_oTemp = mp_oSimManager->GetBehaviorObject(
        "LinkedFunctionalResponseSeedPredation");
    clFuncResponseSeedPredation *p_oPred = dynamic_cast<clFuncResponseSeedPredation*>(p_oTemp);
    float fZ = p_oPred->GetOfftakeRate(),
    fMinY = 100000,
    fAvgY = 0,
    fCorrectionFactor,
    fNumSeeds = 0, fNewSeeds;
    int iSp;

    //Declare a place to put all the offtakes
    p_fOfftakes = new float**[iNumXCells];
    for ( iX = 0; iX < iNumXCells; iX++ ) {
      p_fOfftakes[iX] = new float*[iNumYCells];
      for ( iY = 0; iY < iNumYCells; iY++ ) {
        p_fOfftakes[iX][iY] = new float[m_iNumBehaviorSpecies];
      }
    }

    //Logitize Z
    fZ = log(fZ/(1-fZ));

    //Calculate all the offtakes for the first time, logitize, and get the
    //minimum
    for ( iX = 0; iX < iNumXCells; iX++ ) {
      for ( iY = 0; iY < iNumYCells; iY++ ) {
        GetOfftakes(p_oPop, mp_fNonMastingP0, mp_fNonMastingPn,
            p_fOfftakes[iX][iY], iX, iY);
        for ( iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++ ) {
          p_fOfftakes[iX][iY][iSp] = log(p_fOfftakes[iX][iY][iSp]/(1-p_fOfftakes[iX][iY][iSp]));
          if (fMinY > p_fOfftakes[iX][iY][iSp]) fMinY = p_fOfftakes[iX][iY][iSp];
        }
      }
    }

    //Subtract the minimum from all offtakes and do the average
    for ( iX = 0; iX < iNumXCells; iX++ ) {
      for ( iY = 0; iY < iNumYCells; iY++ ) {
        for ( iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++ ) {
          p_fOfftakes[iX][iY][iSp] -= fMinY;
          fAvgY += p_fOfftakes[iX][iY][iSp];
        }
      }
    }
    if (fAvgY != 0)
    fAvgY /= (iNumXCells * iNumYCells * m_iNumBehaviorSpecies);

    //Calculate the correction factor
    fCorrectionFactor = (fZ - fMinY) / fAvgY;

    //Correct and back-transform all offtakes - and use them to reduce seeds
    for ( iX = 0; iX < iNumXCells; iX++ ) {
      for ( iY = 0; iY < iNumYCells; iY++ ) {
        for ( iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++ ) {

          //Correct and back-transform
          p_fOfftakes[iX][iY][iSp] *= fCorrectionFactor;
          p_fOfftakes[iX][iY][iSp] += fMinY;
          p_fOfftakes[iX][iY][iSp] = exp(p_fOfftakes[iX][iY][iSp]) /
          (1 + exp(p_fOfftakes[iX][iY][iSp]));

          //Reduce seeds
          mp_oSeedGrid->GetValueOfCell( iX, iY, mp_iSeedGridCode[iSp], & fNumSeeds );

          if (fNumSeeds > 0) {

            //Reduce the number of seeds appropriately
            fNewSeeds = fNumSeeds * (1-p_fOfftakes[iX][iY][iSp]);

            if ( fNumSeeds < 0 ) fNumSeeds = 0;
            if ( fNewSeeds > fNumSeeds) fNewSeeds = fNumSeeds;

            mp_oSeedGrid->SetValueOfCell( iX, iY, mp_iSeedGridCode[iSp], fNewSeeds );
          }
        }
      }
    }

    for ( iX = 0; iX < iNumXCells; iX++ ) {
      for ( iY = 0; iY < iNumYCells; iY++ ) {
        delete[] p_fOfftakes[iX][iY];
      }
      delete[] p_fOfftakes[iX];
    }
    delete[] p_fOfftakes;
  }
  catch ( ... )
  {
    if (p_fOfftakes) {
      for ( iX = 0; iX < iNumXCells; iX++ ) {
        for ( iY = 0; iY < iNumYCells; iY++ ) {
          delete[] p_fOfftakes[iX][iY];
        }
        delete[] p_fOfftakes[iX];
      }
      delete[] p_fOfftakes;
    }
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clNeighborhoodSeedPredation::DoLinkedPredation" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetOfftakes()
//////////////////////////////////////////////////////////////////////////////
void clNeighborhoodSeedPredation::GetOfftakes(clTreePopulation * p_oPop,
    const float* p_fP0, float **p_fPn, float *p_fOfftake, const int &iX,
    const int &iY) {

  clTreeSearch * p_oTrees;             //saplings and adults
  clTree * p_oTree;                    //single tree to count
  char cQuery[75];                     //format search string into this
  float fTotalBA, fDbh, fX, fY;
  int iNumSpecies = p_oPop->GetNumberOfSpecies(),
      iSp, iTp, i;
  float *p_fBasalArea = new float[iNumSpecies]; //basal area per species

  //Get all neighbors within the specified radius and calculate
  //basal area
  for (iSp = 0; iSp < iNumSpecies; iSp++)
    p_fBasalArea[iSp] = 0;
  mp_oSeedGrid->GetPointOfCell(iX, iY, &fX, &fY);
  sprintf(cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fRadius, "FROM x=", fX,
      "y=", fY, "::height=", 0.0);
  p_oTrees = p_oPop->Find(cQuery);
  p_oTree = p_oTrees->NextTree();

  while (NULL != p_oTree) {

    //Get the tree's DBH and make sure it's eligible
    iSp = p_oTree->GetSpecies();
    iTp = p_oTree->GetType();
    if (iTp == clTreePopulation::sapling || iTp == clTreePopulation::adult) {

      p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
      if (fDbh >= mp_fMinNeighDBH[iSp]) {

        //Get tree's "basal area" - since it's relative BA, we really
        //only need to square the DBH
        p_fBasalArea[iSp] += pow(fDbh, 2);
      }
    }
    p_oTree = p_oTrees->NextTree();
  }

  //Get relative BA for each species
  fTotalBA = 0;
  for (iSp = 0; iSp < iNumSpecies; iSp++)
    fTotalBA += p_fBasalArea[iSp];
  if (fTotalBA > 0) {
    for (iSp = 0; iSp < iNumSpecies; iSp++)
      p_fBasalArea[iSp] /= fTotalBA;
  }

  for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
    //Calculate the offtake rate (Y) for this species
    p_fOfftake[iSp] = p_fP0[iSp];
    for (i = 0; i < iNumSpecies; i++)
      p_fOfftake[iSp] += p_fBasalArea[i] * p_fPn[iSp][i];

    p_fOfftake[iSp] = exp(p_fOfftake[iSp])/(1 + exp(p_fOfftake[iSp]));
  }

  delete[] p_fBasalArea;
}

////////////////////////////////////////////////////////////////////////////
// SetupGrids()
////////////////////////////////////////////////////////////////////////////
void clNeighborhoodSeedPredation::SetupGrids()
{
  try
  {
    clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
    std::stringstream sLabel;
    short int i, iTotalSpecies = p_oPop->GetNumberOfSpecies();

    mp_iSeedGridCode = new short int[m_iNumBehaviorSpecies];

    //Fetch the seed grid
    mp_oSeedGrid = mp_oSimManager->GetGridObject( "Dispersed Seeds" );
    if ( mp_oSeedGrid == NULL )
    {
      modelErr stcErr;
      stcErr.sFunction = "clNeighborhoodSeedPredation::SetupGrids" ;
      stcErr.sMoreInfo = "Disperse behaviors must be used with neighborhood seed predation.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw( stcErr );
    }

    //Now get the data codes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      sLabel << "seeds_" << mp_iWhatSpecies[i];
      mp_iSeedGridCode[i] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
      if ( -1 == mp_iSeedGridCode[i] )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clNeighborhoodSeedPredation::SetupGrids" ;
        stcErr.sMoreInfo = "Unexpected grid data return code.";
        throw( stcErr );
      }
      sLabel.str("");
    }

    //Declare the code indexes
    mp_iStartSeedsCode = new short int[iTotalSpecies];
    mp_iPropEatenCode = new short int[iTotalSpecies];

    //Set up the output grid
    //Der tricky part. If we have both regular and linked seed predation,
    //the grid may be already set up. BUT if the grid exists because there was
    //a map in the parameter file (which we wish to ignore), we want to create
    //it anew. So: if it exists already, we will keep it as long as the cell
    //size is right.
    if (mp_oOutputGrid == NULL) {

      mp_oOutputGrid = mp_oSimManager->CreateGrid(
             "Neighborhood Seed Predation", //grid name
             0, //number of int data members
             (2*iTotalSpecies), //number of float data members
             0, //number of char data members
             0, //number of bool data members
             mp_oSeedGrid->GetLengthXCells(),
             mp_oSeedGrid->GetLengthYCells());

      //Register the data members
      for (i = 0; i < iTotalSpecies; i++)
      {
        sLabel << "startseeds_" << i;
        mp_iStartSeedsCode[i] = mp_oOutputGrid->RegisterFloat(sLabel.str());
        sLabel.str("");
      }
      for (i = 0; i < iTotalSpecies; i++)
      {
        sLabel << "propeaten_" << i;
        mp_iPropEatenCode[i] = mp_oOutputGrid->RegisterFloat(sLabel.str());
        sLabel.str("");
      }

    } else {

      if (fabs(mp_oOutputGrid->GetLengthXCells() - mp_oSeedGrid->GetLengthXCells()) > 0.001 ||
          fabs(mp_oOutputGrid->GetLengthYCells() - mp_oSeedGrid->GetLengthYCells()) > 0.001) {
        mp_oOutputGrid = mp_oSimManager->CreateGrid(
            "Neighborhood Seed Predation", //grid name
            0, //number of int data members
            (2*iTotalSpecies), //number of float data members
            0, //number of char data members
            0, //number of bool data members
            mp_oSeedGrid->GetLengthXCells(),
            mp_oSeedGrid->GetLengthYCells());

        //Register the data members
        for (i = 0; i < iTotalSpecies; i++)
        {
          sLabel << "startseeds_" << i;
          mp_iStartSeedsCode[i] = mp_oOutputGrid->RegisterFloat(sLabel.str());
          sLabel.str("");
        }
        for (i = 0; i < iTotalSpecies; i++)
        {
          sLabel << "propeaten_" << i;
          mp_iPropEatenCode[i] = mp_oOutputGrid->RegisterFloat(sLabel.str());
          sLabel.str("");
        }
      } else {

        for (i = 0; i < iTotalSpecies; i++)
        {
          sLabel << "startseeds_" << i;
          mp_iStartSeedsCode[i] = mp_oOutputGrid->GetFloatDataCode(sLabel.str());
          sLabel.str("");
          sLabel << "propeaten_" << i;
          mp_iPropEatenCode[i] = mp_oOutputGrid->GetFloatDataCode(sLabel.str());
          sLabel.str("");
        }
      }
    }

  }
  catch (modelErr& err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisperseBase::SetUpBase";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clNeighborhoodSeedPredation::ReadParameterFileData(xercesc::DOMDocument *p_oDoc) {
  floatVal * p_fTemp= NULL; //for getting species-specific values
  boolVal * p_bTemp= NULL; //for getting species-specific values
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    std::stringstream sLabel;
    int iTemp;
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i, j;

    //Declare the indexes array
    mp_iIndexes = new short int[iNumSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_iIndexes[mp_iWhatSpecies[i]] = i;

    //Declare our arrays
    mp_fMinNeighDBH = new float[iNumSpecies];
    mp_fNonMastingP0 = new float[m_iNumBehaviorSpecies];
    mp_fNonMastingPn = new float * [m_iNumBehaviorSpecies];

    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fNonMastingPn[i] = new float[iNumSpecies];
      for ( j = 0; j < iNumSpecies; j++ ) {
        mp_fNonMastingPn[i] [j] = 0;
      }
    }

    //Standalone version needs masting too
    if (false == m_bIsLinked) {
      mp_fMastingP0 = new float[m_iNumBehaviorSpecies];
      mp_fMastingPn = new float * [m_iNumBehaviorSpecies];
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
        mp_fMastingPn[i] = new float[iNumSpecies];
        for ( j = 0; j < iNumSpecies; j++ ) {
          mp_fMastingPn[i] [j] = 0;
        }
      }
    }

    //Set up our temp array - pre-load with this behavior's species
    p_fTemp = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTemp[i].code = mp_iWhatSpecies[i];

    if (m_bIsLinked) {
      //"p0"
      FillSpeciesSpecificValue( p_oElement,
          "pr_neighPredP0",
          "pr_npmp0Val",
          p_fTemp,
          m_iNumBehaviorSpecies,
          p_oPop,
          true );
      //Now transfer the values to the permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fNonMastingP0[i] = p_fTemp[i].val;

      //"pns"
      for ( i = 0; i < iNumSpecies; i++ )
      {
        sLabel << "pr_neighPredPn" << p_oPop->TranslateSpeciesCodeToName(i);
        FillSpeciesSpecificValue( p_oElement, sLabel.str(), "pr_nppnVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
        sLabel.str("");
        for ( j = 0; j < m_iNumBehaviorSpecies; j++ )
        {
          mp_fNonMastingPn[j][i] = p_fTemp[j].val;
        }
      }
    } else {

      //Masting "p0"
      FillSpeciesSpecificValue( p_oElement,
          "pr_neighPredMastingP0",
          "pr_npmp0Val",
          p_fTemp,
          m_iNumBehaviorSpecies,
          p_oPop,
          true );
      //Now transfer the values to the permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fMastingP0[i] = p_fTemp[i].val;

      //Masting "pns"
      for ( i = 0; i < iNumSpecies; i++ )
      {
        sLabel << "pr_neighPredMastingPn" << p_oPop->TranslateSpeciesCodeToName(i);
        FillSpeciesSpecificValue( p_oElement, sLabel.str(), "pr_npmpnVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
        sLabel.str("");
        for ( j = 0; j < m_iNumBehaviorSpecies; j++ )
        {
          mp_fMastingPn[j] [i] = p_fTemp[j].val;
        }
      }

      //Non-masting "p0"
      FillSpeciesSpecificValue( p_oElement,
          "pr_neighPredNonMastingP0",
          "pr_npnmp0Val",
          p_fTemp,
          m_iNumBehaviorSpecies,
          p_oPop,
          true );
      //Now transfer the values to the permanent array
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fNonMastingP0[i] = p_fTemp[i].val;

      //Non-masting "pns"
      for ( i = 0; i < iNumSpecies; i++ )
      {
        sLabel << "pr_neighPredNonMastingPn" << p_oPop->TranslateSpeciesCodeToName(i);
        FillSpeciesSpecificValue(p_oElement, sLabel.str(), "pr_npnmpnVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
        sLabel.str("");
        for ( j = 0; j < m_iNumBehaviorSpecies; j++ )
        {
          mp_fNonMastingPn[j][i] = p_fTemp[j].val;
        }
      }

      //Method to use for masting decisions
      FillSingleValue (p_oElement, "pr_neighPredMastDecisionMethod", &iTemp,true);
      if (iTemp == 0) m_bUseThresholdToDecideMast = true;
      else m_bUseThresholdToDecideMast = false;

      if (m_bUseThresholdToDecideMast) {
        //Density of seeds for masting
        FillSingleValue (p_oElement,
            "pr_neighPredMastingDensity",
            &m_fMastingThreshold,
            true);
        if ( m_fMastingThreshold < 0 )
        {
          modelErr stcErr;
          stcErr.sFunction = "clNeighborhoodSeedPredation::ReadParameterFileData" ;
          stcErr.sMoreInfo = "Masting seed density threshold cannot be negative.";
          stcErr.iErrorCode = BAD_DATA;
          throw( stcErr );
        }
        //Transform from annual to timestep value
        m_fMastingThreshold *= mp_oSimManager->GetNumberOfYearsPerTimestep();

        //Which species participate in mast
        mp_bCountInMast = new bool[m_iNumBehaviorSpecies];

        //Set up our temp array - pre-load with this behavior's species
        p_bTemp = new boolVal[m_iNumBehaviorSpecies];
        for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
          p_bTemp[i].code = mp_iWhatSpecies[i];

        FillSpeciesSpecificValue( p_oElement,
                  "pr_neighPredCounts4Mast",
                  "pr_npc4mVal",
                  p_bTemp,
                  m_iNumBehaviorSpecies,
                  p_oPop,
                  true );
        //Now transfer the values to the permanent array
        for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
          mp_bCountInMast[i] = p_bTemp[i].val;
      }
    }

    //Min neighbor DBH
    FillSpeciesSpecificValue( p_oElement, "pr_neighPredMinNeighDBH",
        "pr_npmndVal", mp_fMinNeighDBH, p_oPop, true );

    for ( i = 0; i < iNumSpecies; i++ ) {
      if ( mp_fMinNeighDBH[i] < 0 )
      {
        modelErr stcErr;
        stcErr.sFunction = "clNeighborhoodSeedPredation::ReadParameterFileData" ;
        stcErr.sMoreInfo = "Minimum neighbor DBH cannot be negative.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Neighborhood search radius
    FillSingleValue (p_oElement,
        "pr_neighPredRadius",
        &m_fRadius,
        true);
    if ( m_fRadius < 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clNeighborhoodSeedPredation::ReadParameterFileData" ;
      stcErr.sMoreInfo = "Neighborhood radius cannot be negative.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    delete[] p_fTemp;
    delete[] p_bTemp;
  }
  catch ( modelErr& stcErr )
  {
    delete[] p_fTemp;
    delete[] p_bTemp;
    throw( stcErr );
  }
  catch ( ... )
  {
    delete[] p_fTemp;
    delete[] p_bTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clNeighborhoodSeedPredation::ReadParameterFileData" ;
    throw( stcErr );
  }
}
