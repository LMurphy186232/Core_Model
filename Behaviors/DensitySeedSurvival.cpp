//---------------------------------------------------------------------------
// DensitySeedSurvival.cpp
//---------------------------------------------------------------------------
#include "DensitySeedSurvival.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Grid.h"
#include "ParsingFunctions.h"
#include "ModelMath.h"
#include "Plot.h"
#include <math.h>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clDensitySeedSurvival::clDensitySeedSurvival(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try {
    m_sNameString = "DensityDependentSeedSurvival";
    m_sXMLRoot = "DensityDependentSeedSurvival";

    mp_fDensDepSteepness = NULL;
    mp_fDensDepSlope = NULL;
    mp_iSeedGridCode = NULL;
    mp_fMinHeight = NULL;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 2;
    m_fMinimumVersionNumber = 1;

    m_bSeeds = false;
    m_fXYEdgeCellArea = 0;
    m_fXEdgeCellArea = 0;
    m_fSearchRadius = 0;
    m_fYEdgeCellArea = 0;
    mp_oSeedGrid = NULL;
    m_fNormalSearchArea = 0;

  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDensitySeedSurvival::clDensitySeedSurvival";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clDensitySeedSurvival::~clDensitySeedSurvival() {
  delete[] mp_fDensDepSteepness;
  delete[] mp_fDensDepSlope;
  delete[] mp_iSeedGridCode;
  delete[] mp_fMinHeight;
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clDensitySeedSurvival::GetData(DOMDocument * p_oDoc) {
  try {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop =
        (clTreePopulation *) mp_oSimManager->GetPopulationObject(
            "treepopulation");
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    doubleVal * p_fTemp = NULL; //for getting species-specific values
    std::stringstream sLabel;
    float fXTemp, fYTemp;
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Declare our arrays
    mp_fDensDepSteepness = new double[iNumSpecies];
    mp_fDensDepSlope = new double[iNumSpecies];
    mp_iSeedGridCode = new short int[iNumSpecies];

    //Set up our temp array - pre-load with this behavior's species
    p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      p_fTemp[i].code = mp_iWhatSpecies[i];

    //Density dependence slope
    FillSpeciesSpecificValue(p_oElement, "es_densDepSlope", "es_ddsVal",
        p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fDensDepSlope[p_fTemp[i].code] = p_fTemp[i].val;

    //Density dependence steepness
    FillSpeciesSpecificValue(p_oElement, "es_densDepSteepness", "es_ddstVal",
        p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fDensDepSteepness[p_fTemp[i].code] = p_fTemp[i].val;

    //Fetch the seed grid
    mp_oSeedGrid = mp_oSimManager->GetGridObject("Dispersed Seeds");
    if (mp_oSeedGrid == NULL) {
      modelErr stcErr;
      stcErr.sFunction = "clDensitySeedSurvival::GetData";
      stcErr.sMoreInfo = "Disperse behaviors must be used with density-dependent seed survival.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw(stcErr);
    }

    //Now get the data codes
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      sLabel << "seeds_" << mp_iWhatSpecies[i];
      mp_iSeedGridCode[mp_iWhatSpecies[i]] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iSeedGridCode[mp_iWhatSpecies[i]]) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensitySeedSurvival::GetData";
        stcErr.sMoreInfo = "Unexpected grid data return code.";
        throw(stcErr);
      }
      sLabel.str("");
    }

    if (m_bSeeds) {
      //Calculate the cell areas
      //Normal cell area - straightforward
      m_fNormalSearchArea = mp_oSeedGrid->GetLengthXCells()
          * mp_oSeedGrid->GetLengthYCells();

      //X and Y edges - get the length of the last cells in the X and Y direction
      // (might be the same as all the other cells)
      fXTemp = p_oPlot->GetXPlotLength() - (mp_oSeedGrid->GetLengthXCells()
          * floor(p_oPlot->GetXPlotLength() / mp_oSeedGrid->GetLengthXCells()));
      fXTemp = (fXTemp == 0 ? mp_oSeedGrid->GetLengthXCells() : fXTemp);
      fYTemp = p_oPlot->GetYPlotLength() - (mp_oSeedGrid->GetLengthYCells()
          * floor(p_oPlot->GetYPlotLength() / mp_oSeedGrid->GetLengthYCells()));
      fYTemp = (fYTemp == 0 ? mp_oSeedGrid->GetLengthYCells() : fYTemp);

      m_fXEdgeCellArea = fXTemp * mp_oSeedGrid->GetLengthYCells();
      m_fYEdgeCellArea = fYTemp * mp_oSeedGrid->GetLengthXCells();
      m_fXYEdgeCellArea = fXTemp * fYTemp;

    } else {

      //Using tree neighbors - get the extra parameters required

      //Minimum neighbor height
      mp_fMinHeight = new double[iNumSpecies];
      FillSpeciesSpecificValue(p_oElement, "es_densDepMinNeighHeight",
          "es_ddmnhVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
      for (i = 0; i < m_iNumBehaviorSpecies; i++)
        mp_fMinHeight[p_fTemp[i].code] = p_fTemp[i].val;

      //Neighbor search radius
      FillSingleValue(p_oElement, "es_densDepSearchRadius", &m_fSearchRadius, true);

      //Calculate the area of search circle
      m_fNormalSearchArea = m_fSearchRadius * m_fSearchRadius * 3.14159;
    }

    delete[] p_fTemp;
  } catch (modelErr& err) {
    throw(err);
  } catch (modelMsg & msg) {
    throw(msg);
  } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDensitySeedSurvival::GetData";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Action
///////////////////////////////////////////////////////////////////////////////
void clDensitySeedSurvival::Action() {
  if (m_bSeeds) ActionUseSeeds();
  else ActionUseTrees();
}

///////////////////////////////////////////////////////////////////////////////
// ActionUseSeeds
///////////////////////////////////////////////////////////////////////////////
void clDensitySeedSurvival::ActionUseSeeds() {
  float fNumSeeds, fNewSeeds, fCellArea;
  int iNumXCells = mp_oSeedGrid->GetNumberXCells(), iNumYCells =
      mp_oSeedGrid->GetNumberYCells(), iX, iY, i;

  for (iX = 0; iX < iNumXCells; iX++) {
    if (iX == iNumXCells - 1)
      fCellArea = m_fXEdgeCellArea;
    else
      fCellArea = m_fNormalSearchArea;

    for (iY = 0; iY < iNumYCells; iY++) {

      if (iY == iNumYCells - 1) {
        if (iX == iNumXCells - 1)
          fCellArea = m_fXYEdgeCellArea;
        else
          fCellArea = m_fYEdgeCellArea;
      }

      //Go through the species
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {

        mp_oSeedGrid->GetValueOfCell(iX, iY,
            mp_iSeedGridCode[mp_iWhatSpecies[i]], &fNumSeeds);

        //Reduce the number of seeds appropriately
        if (fNumSeeds > 1) {
          fNewSeeds = fNumSeeds * exp(
              -mp_fDensDepSlope[mp_iWhatSpecies[i]] * pow((fNumSeeds
                  / fCellArea), mp_fDensDepSteepness[mp_iWhatSpecies[i]]));
        } else {
          fNewSeeds = fNumSeeds;
        }

        if (fNumSeeds < 0)
          fNumSeeds = 0;
        if (fNewSeeds > fNumSeeds)
          fNewSeeds = fNumSeeds;

        mp_oSeedGrid->SetValueOfCell(iX, iY,
            mp_iSeedGridCode[mp_iWhatSpecies[i]], fNewSeeds);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// ActionUseTrees
///////////////////////////////////////////////////////////////////////////////
void clDensitySeedSurvival::ActionUseTrees() {
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  clTreeSearch *p_oAllNeighbors;
  clTree *p_oTree;
  std::stringstream sQuery; //format search strings into this
  float fNumSeeds, fNewSeeds, fX, fY, fHeight, fMinHeight = 1000;
  int *p_iNeighCount = new int[p_oPop->GetNumberOfSpecies()];
  short int *p_iIndexes = new short int[p_oPop->GetNumberOfSpecies()];
  int iNumXCells = mp_oSeedGrid->GetNumberXCells(),
      iNumYCells = mp_oSeedGrid->GetNumberYCells(),
      iNumberTotalSpecies = p_oPop->GetNumberOfSpecies(), iX, iY, i;

  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    if (fMinHeight > mp_fMinHeight[mp_iWhatSpecies[i]])
      fMinHeight = mp_fMinHeight[mp_iWhatSpecies[i]];
  }

  //Make an array to make it easier to identify and count used species
  for ( i = 0; i < iNumberTotalSpecies; i++ ) p_iIndexes[i] = -1;
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) p_iIndexes[mp_iWhatSpecies[i]] = i;

  for (iX = 0; iX < iNumXCells; iX++) {
    for (iY = 0; iY < iNumYCells; iY++) {

      for (i = 0; i < iNumberTotalSpecies; i++) p_iNeighCount[i] = 0;

      mp_oSeedGrid->GetPointOfCell(iX, iY, &fX, &fY);
      //Get all trees taller than cutoff within the search radius
      sQuery << "distance=" << m_fSearchRadius << "FROM x=" << fX << "y="
             << fY << "::height=" << fMinHeight;
      p_oAllNeighbors = p_oPop->Find(sQuery.str());
      sQuery.str("");

      //Count out the eligible trees by species
      p_oTree = p_oAllNeighbors->NextTree();
      while (p_oTree) {
        //Eligible species?
        if (p_iIndexes[p_oTree->GetSpecies()] > -1) {
          //Get the height and count it if it's above the min for that species
          p_oTree->GetValue(p_oPop->GetHeightCode(p_oTree->GetSpecies(),
                                                  p_oTree->GetType()), &fHeight);
          if (fHeight >= mp_fMinHeight[p_oTree->GetSpecies()])
            p_iNeighCount[p_oTree->GetSpecies()]++;

        }
        p_oTree = p_oAllNeighbors->NextTree();
      }

      //Go through the species and do the seeds
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {

        mp_oSeedGrid->GetValueOfCell(iX, iY,
            mp_iSeedGridCode[mp_iWhatSpecies[i]], &fNumSeeds);

        //Reduce the number of seeds appropriately
        if (p_iNeighCount[mp_iWhatSpecies[i]] > 0) {
          fNewSeeds = fNumSeeds * exp(
              -mp_fDensDepSlope[mp_iWhatSpecies[i]] * pow((p_iNeighCount[mp_iWhatSpecies[i]]
                  / m_fNormalSearchArea), mp_fDensDepSteepness[mp_iWhatSpecies[i]]));
        } else {
          fNewSeeds = fNumSeeds;
        }

        if (fNumSeeds < 0)
          fNumSeeds = 0;
        if (fNewSeeds > fNumSeeds)
          fNewSeeds = fNumSeeds;

        mp_oSeedGrid->SetValueOfCell(iX, iY,
            mp_iSeedGridCode[mp_iWhatSpecies[i]], fNewSeeds);
      }
    }
  }
  delete[] p_iNeighCount;
  delete[] p_iIndexes;
}


//////////////////////////////////////////////////////////////////////////////
// SetNameData()
//////////////////////////////////////////////////////////////////////////////
void clDensitySeedSurvival::SetNameData(std::string sNameString) {
 try {

   //Check the string passed and set the flags accordingly
   if (sNameString.compare("DensityDependentSeedSurvival") == 0) {
     m_bSeeds = true;
   } else if (sNameString.compare("ConspecificTreeDensityDependentSeedSurvival") == 0) {
     m_bSeeds = false;
   } else {
     modelErr stcErr;
     stcErr.iErrorCode = BAD_DATA;
     std::stringstream s;
     s << "Unrecognized behavior name \"" << sNameString << "\".";
     stcErr.sFunction = "clDensitySeedSurvival::SetNameData";
     stcErr.sMoreInfo = s.str();
throw(stcErr);
   }
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clDensitySeedSurvival::SetNameData";
   throw(stcErr);
 }
}
