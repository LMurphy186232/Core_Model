//---------------------------------------------------------------------------
// StormKilledPartitionedBiomass.cpp
//---------------------------------------------------------------------------
#include "StormKilledPartitionedBiomass.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "Constants.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clStormKilledPartitionedBiomass::clStormKilledPartitionedBiomass( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "Storm Killed Paritioned Biomass";
    m_sXMLRoot = "StormKilledPartitionedBiomass";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_fDBHLeafA = NULL;
    mp_fDBHLeafB = NULL;
    mp_fDBHBranchA = NULL;
    mp_fDBHBranchB = NULL;
    mp_fDBHBoleA = NULL;
    mp_fDBHBoleB = NULL;
    mp_fHeightLeafA = NULL;
    mp_fHeightLeafB = NULL;
    mp_fHeightBoleA = NULL;
    mp_fHeightBoleB = NULL;
    m_cQuery = NULL;
    mp_iLeafCodes = NULL;
    mp_iBranchCodes = NULL;
    mp_iBoleCodes = NULL;
    mp_iHLeafCodes = NULL;
    mp_iHBoleCodes = NULL;
    mp_iDbhCodes = NULL;
    mp_iHeightCodes = NULL;
    mp_iXCodes = NULL;
    mp_iYCodes = NULL;
    mp_iStmDmgCodes = NULL;
    mp_oGrid = NULL;
    m_bIsDbh = true;
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
    stcErr.sFunction = "clStormKilledPartitionedBiomass::clStormKilledPartitionedBiomass" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clStormKilledPartitionedBiomass::~clStormKilledPartitionedBiomass()
{
  delete[] mp_iDbhCodes;
  delete[] mp_iHeightCodes;
  delete[] mp_iXCodes;
  delete[] mp_iYCodes;
  delete[] mp_iStmDmgCodes;
  delete[] mp_fDBHLeafA;
  delete[] mp_fDBHLeafB;
  delete[] mp_fDBHBranchA;
  delete[] mp_fDBHBranchB;
  delete[] mp_fDBHBoleA;
  delete[] mp_fDBHBoleB;
  delete[] mp_fHeightLeafA;
  delete[] mp_fHeightLeafB;
  delete[] mp_fHeightBoleA;
  delete[] mp_fHeightBoleB;
  delete[] m_cQuery;
  delete[] mp_iLeafCodes;
  delete[] mp_iBranchCodes;
  delete[] mp_iBoleCodes;
  delete[] mp_iHLeafCodes;
  delete[] mp_iHBoleCodes;
}

//////////////////////////////////////////////////////////////////////////////
// SetNameData()
/////////////////////////////////////////////////////////////////////////////*/
void clStormKilledPartitionedBiomass::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("StormKilledPartitionedDBHBiomass") == 0) {
      m_bIsDbh = true;
    } else if (sNameString.compare("StormKilledPartitionedHeightBiomass") == 0) {
      m_bIsDbh = false;
    }
    else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sFunction = "clStormKilledPartitionedBiomass::SetNameData";
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clStormKilledPartitionedBiomass::SetNameData";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
/////////////////////////////////////////////////////////////////////////////
void clStormKilledPartitionedBiomass::GetParameterFileData( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  doubleVal * p_fTempValues; //for getting species-specific values
  int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
      i;

  //Set up our doubleVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  if (m_bIsDbh) {
    //*******************
    //This is DBH biomass
    //*******************

    //Declare the appropriate arrays
    mp_fDBHLeafA = new double[iNumTotalSpecies];
    mp_fDBHLeafB = new double[iNumTotalSpecies];
    mp_fDBHBranchA = new double[iNumTotalSpecies];
    mp_fDBHBranchB = new double[iNumTotalSpecies];
    mp_fDBHBoleA = new double[iNumTotalSpecies];
    mp_fDBHBoleB = new double[iNumTotalSpecies];

    FillSpeciesSpecificValue( p_oElement, "an_partBioDbhLeafA", "an_pbdlaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fDBHLeafA[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    FillSpeciesSpecificValue( p_oElement, "an_partBioDbhLeafB", "an_pbdlbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fDBHLeafB[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    FillSpeciesSpecificValue( p_oElement, "an_partBioDbhBranchA", "an_pbdbraVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fDBHBranchA[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    FillSpeciesSpecificValue( p_oElement, "an_partBioDbhBranchB", "an_pbdbrbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fDBHBranchB[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    FillSpeciesSpecificValue( p_oElement, "an_partBioDbhBoleA", "an_pbdboaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fDBHBoleA[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    FillSpeciesSpecificValue( p_oElement, "an_partBioDbhBoleB", "an_pbdbobVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fDBHBoleB[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
  }  else {

    //*******************
    //This is Height biomass
    //*******************
    //Declare the appropriate arrays
    mp_fHeightLeafA = new double[iNumTotalSpecies];
    mp_fHeightLeafB = new double[iNumTotalSpecies];
    mp_fHeightBoleA = new double[iNumTotalSpecies];
    mp_fHeightBoleB = new double[iNumTotalSpecies];

    FillSpeciesSpecificValue( p_oElement, "an_partBioHeightLeafA", "an_pbhlaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fHeightLeafA[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    FillSpeciesSpecificValue( p_oElement, "an_partBioHeightLeafB", "an_pbhlbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fHeightLeafB[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    FillSpeciesSpecificValue( p_oElement, "an_partBioHeightBoleA", "an_pbhboaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fHeightBoleA[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
    FillSpeciesSpecificValue( p_oElement, "an_partBioHeightBoleB", "an_pbhbobVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, false );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fHeightBoleB[p_fTempValues[i].code] = p_fTempValues[i].val;
    }
  }
  delete[] p_fTempValues;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clStormKilledPartitionedBiomass::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    int i;

    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      if (mp_whatSpeciesTypeCombos[i].iType != clTreePopulation::snag) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Storm Killed Partitioned Biomass can only be applied to snags.";
        stcErr.sFunction = "clStormKilledPartitionedBiomass::GetData";
        throw(stcErr);
      }
    }

    GetTreeCodes(p_oPop);
    GetParameterFileData(p_oDoc, p_oPop);
    FormatQueryString(p_oPop);
    SetupGrid(p_oPop);
    Action();
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
    stcErr.sFunction = "clStormKilledPartitionedBiomass::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetTreeCodes()
/////////////////////////////////////////////////////////////////////////////
void clStormKilledPartitionedBiomass::GetTreeCodes(clTreePopulation *p_oPop)
{
  int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
      i;

  mp_iXCodes = new short int[iNumTotalSpecies];
  mp_iYCodes = new short int[iNumTotalSpecies];
  mp_iStmDmgCodes = new short int[iNumTotalSpecies];
  for (i = 0; i < iNumTotalSpecies; i++) {
    mp_iXCodes[i] = -1;
    mp_iYCodes[i] = -1;
    mp_iStmDmgCodes[i] = -1;
  }
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_iXCodes[mp_iWhatSpecies[i]] = p_oPop->GetXCode(mp_iWhatSpecies[i], clTreePopulation::snag);
    mp_iYCodes[mp_iWhatSpecies[i]] = p_oPop->GetYCode(mp_iWhatSpecies[i], clTreePopulation::snag);
    mp_iStmDmgCodes[mp_iWhatSpecies[i]] = p_oPop->GetIntDataCode("stm_dmg", mp_iWhatSpecies[i], clTreePopulation::snag);
    if (-1 == mp_iStmDmgCodes[mp_iWhatSpecies[i]]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "All snags to which the Storm Killed Partitioned Biomass behavior are applied must also have the Storm Damage Applier applied.";
      stcErr.sFunction = "clStormKilledPartitionedBiomass::GetTreeCodes";
      throw(stcErr);
    }
  }

  if (m_bIsDbh) {
    mp_iDbhCodes = new short int[iNumTotalSpecies];
    for (i = 0; i < iNumTotalSpecies; i++) {
      mp_iDbhCodes[i] = -1;
    }
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_iDbhCodes[mp_iWhatSpecies[i]] = p_oPop->GetDbhCode(mp_iWhatSpecies[i], clTreePopulation::snag);
    }

  } else {
    mp_iHeightCodes = new short int[iNumTotalSpecies];
    for (i = 0; i < iNumTotalSpecies; i++) {
      mp_iHeightCodes[i] = -1;
    }
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_iHeightCodes[mp_iWhatSpecies[i]] = p_oPop->GetHeightCode(mp_iWhatSpecies[i], clTreePopulation::snag);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clStormKilledPartitionedBiomass::SetupGrid(clTreePopulation *p_oPop)
{
  std::stringstream sLabel;
  short int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
      i; //loop counter

  //Declare the arrays for our grid codes
  mp_iLeafCodes = new short int[iNumTotalSpecies];
  mp_iBranchCodes = new short int[iNumTotalSpecies];
  mp_iBoleCodes = new short int[iNumTotalSpecies];
  mp_iHLeafCodes = new short int[iNumTotalSpecies];
  mp_iHBoleCodes = new short int[iNumTotalSpecies];

  //Check to see if this grid has already been created
  mp_oGrid = mp_oSimManager->GetGridObject("Storm Killed Partitioned Biomass");

  if (NULL == mp_oGrid) {

    //Create the grid with five float data members for each species
    mp_oGrid = mp_oSimManager->CreateGrid( "Storm Killed Partitioned Biomass",
        0,                      //number of ints
        5 * iNumTotalSpecies, //number of floats
        0,                      //number of chars
        0);                     //number of bools

    //Register the data member called "leaf_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "leaf_" << i;
      mp_iLeafCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
    //Register the data member called "branch_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "branch_" << i;
      mp_iBranchCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
    //Register the data member called "bole_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "bole_" << i;
      mp_iBoleCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
    //Register the data member called "hleaf_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "hleaf_" << i;
      mp_iHLeafCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
    //Register the data member called "hbole_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "hbole_" << i;
      mp_iHBoleCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
  }
  else {
    //Grid already exists - get the codes
    //Get the data member called "leaf_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "leaf_" << i;
      mp_iLeafCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iLeafCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clStormKilledPartitionedBiomass::SetupGrid";
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str()
            << "\" member of the \"Storm Killed Paritioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");
    }

    //Get the data member called "branch_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "branch_" << i;
      mp_iBranchCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iBranchCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clStormKilledPartitionedBiomass::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str()
            << "\" member of the \"Storm Killed Paritioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");
    }

    //Get the data member called "bole_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "bole_" << i;
      mp_iBoleCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iBoleCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clStormKilledPartitionedBiomass::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str()
            << "\" member of the \"Storm Killed Paritioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");
    }

    //Get the data member called "hleaf_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "hleaf_" << i;
      mp_iHLeafCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iHLeafCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clStormKilledPartitionedBiomass::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str()
            << "\" member of the \"Storm Killed Paritioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");
    }

    //Get the data member called "hbole_x"
    for (i = 0; i < iNumTotalSpecies; i++) {
      sLabel << "hbole_" << i;
      mp_iHBoleCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iHBoleCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clStormKilledPartitionedBiomass::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str()
            << "\" member of the \"Storm Killed Paritioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clStormKilledPartitionedBiomass::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    float fGridValue, //value in a grid
    fDbh, fHeight,
    fX, fY,
    fValue; //tree's value
    int iNumXCells = mp_oGrid->GetNumberXCells(),
        iNumYCells = mp_oGrid->GetNumberYCells(),
        iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
        iStmDmg,
        iX, iY, iSp;

    //Reset the values in the species total grid to 0
    fValue = 0;
    if (m_bIsDbh) {
      for (iX = 0; iX < iNumXCells; iX++) {
        for (iY = 0; iY < iNumYCells; iY++) {
          for (iSp = 0; iSp < iNumTotalSpecies; iSp++) {
            mp_oGrid->SetValueOfCell(iX, iY, mp_iLeafCodes[iSp], fValue);
            mp_oGrid->SetValueOfCell(iX, iY, mp_iBranchCodes[iSp], fValue);
            mp_oGrid->SetValueOfCell(iX, iY, mp_iBoleCodes[iSp], fValue);
          }
        }
      }
    }
    else {
      for (iX = 0; iX < iNumXCells; iX++) {
        for (iY = 0; iY < iNumYCells; iY++) {
          for (iSp = 0; iSp < iNumTotalSpecies; iSp++) {
            mp_oGrid->SetValueOfCell(iX, iY, mp_iHLeafCodes[iSp], fValue);
            mp_oGrid->SetValueOfCell(iX, iY, mp_iHBoleCodes[iSp], fValue);
          }
        }
      }
    }

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();

    if (m_bIsDbh) {
      while ( p_oTree )
      {

        iSp = p_oTree->GetSpecies();

        //Double-check this tree's appropriateness
        if ( -1 < mp_iStmDmgCodes[iSp] )
        {
          //Get this tree's storm damage value
          p_oTree->GetValue( mp_iStmDmgCodes[iSp], &iStmDmg);
          if ( 1000 == iStmDmg || 2000 == iStmDmg) {

            //Get this tree's coordinates
            p_oTree->GetValue( mp_iXCodes[iSp], &fX);
            p_oTree->GetValue( mp_iYCodes[iSp], &fY);

            //Get this tree's DBH
            p_oTree->GetValue( mp_iDbhCodes[iSp], & fDbh );

            //Calculate this tree's leaf biomass and add it to the total
            fValue = (mp_fDBHLeafA[iSp] * fDbh) + mp_fDBHLeafB[iSp];
            if (fValue > 0) {
              mp_oGrid->GetValueAtPoint(fX, fY, mp_iLeafCodes[iSp], &fGridValue);
              fGridValue += fValue;
              mp_oGrid->SetValueAtPoint(fX, fY, mp_iLeafCodes[iSp], fGridValue);
            }

            //Calculate this tree's branch biomass and add it to the total
            fValue = (mp_fDBHBranchA[iSp] * fDbh) + mp_fDBHBranchB[iSp];
            if (fValue > 0) {
              mp_oGrid->GetValueAtPoint(fX, fY, mp_iBranchCodes[iSp], &fGridValue);
              fGridValue += fValue;
              mp_oGrid->SetValueAtPoint(fX, fY, mp_iBranchCodes[iSp], fGridValue);
            }

            //Calculate this tree's bole biomass and add it to the total
            fValue = (mp_fDBHBoleA[iSp] * fDbh) + mp_fDBHBoleB[iSp];
            if (fValue > 0) {
              mp_oGrid->GetValueAtPoint(fX, fY, mp_iBoleCodes[iSp], &fGridValue);
              fGridValue += fValue;
              mp_oGrid->SetValueAtPoint(fX, fY, mp_iBoleCodes[iSp], fGridValue);
            }
          }
        }

        p_oTree = p_oBehaviorTrees->NextTree();
      }

      //Transform all values to Mg
      for (iX = 0; iX < iNumXCells; iX++) {
        for (iY = 0; iY < iNumYCells; iY++) {
          for (iSp = 0; iSp < iNumTotalSpecies; iSp++) {
            mp_oGrid->GetValueOfCell(iX, iY, mp_iLeafCodes[iSp], &fValue);
            fValue *= CONVERT_KG_TO_MG;
            mp_oGrid->SetValueOfCell(iX, iY, mp_iLeafCodes[iSp], fValue);
            mp_oGrid->GetValueOfCell(iX, iY, mp_iBranchCodes[iSp], &fValue);
            fValue *= CONVERT_KG_TO_MG;
            mp_oGrid->SetValueOfCell(iX, iY, mp_iBranchCodes[iSp], fValue);
            mp_oGrid->GetValueOfCell(iX, iY, mp_iBoleCodes[iSp], &fValue);
            fValue *= CONVERT_KG_TO_MG;
            mp_oGrid->SetValueOfCell(iX, iY, mp_iBoleCodes[iSp], fValue);
          }
        }
      }
    }
    else {
      while ( p_oTree )
      {

        iSp = p_oTree->GetSpecies();

        //Double-check this tree's appropriateness
        if ( -1 < mp_iStmDmgCodes[iSp] )
        {
          //Get this tree's storm damage value
          p_oTree->GetValue( mp_iStmDmgCodes[iSp], &iStmDmg);
          if ( 1000 == iStmDmg || 2000 == iStmDmg) {

            //Get this tree's coordinates
            p_oTree->GetValue( mp_iXCodes[iSp], &fX);
            p_oTree->GetValue( mp_iYCodes[iSp], &fY);

            //Get this tree's height
            p_oTree->GetValue( mp_iHeightCodes[iSp], & fHeight );

            //Calculate this tree's leaf biomass and add it to the total
            fValue = (mp_fHeightLeafA[iSp] * fHeight) + mp_fHeightLeafB[iSp];
            if (fValue > 0) {
              mp_oGrid->GetValueAtPoint(fX, fY, mp_iHLeafCodes[iSp], &fGridValue);
              fGridValue += fValue;
              mp_oGrid->SetValueAtPoint(fX, fY, mp_iHLeafCodes[iSp], fGridValue);
            }

            //Calculate this tree's bole biomass and add it to the total
            fValue = (mp_fHeightBoleA[iSp] * fHeight) + mp_fHeightBoleB[iSp];
            if (fValue > 0) {
              mp_oGrid->GetValueAtPoint(fX, fY, mp_iHBoleCodes[iSp], &fGridValue);
              fGridValue += fValue;
              mp_oGrid->SetValueAtPoint(fX, fY, mp_iHBoleCodes[iSp], fGridValue);
            }
          }
        }

        p_oTree = p_oBehaviorTrees->NextTree();
      }

      //Transform all values to Mg
      for (iX = 0; iX < iNumXCells; iX++) {
        for (iY = 0; iY < iNumYCells; iY++) {
          for (iSp = 0; iSp < iNumTotalSpecies; iSp++) {
            mp_oGrid->GetValueOfCell(iX, iY, mp_iHLeafCodes[iSp], &fValue);
            fValue *= CONVERT_KG_TO_MG;
            mp_oGrid->SetValueOfCell(iX, iY, mp_iHLeafCodes[iSp], fValue);
            mp_oGrid->GetValueOfCell(iX, iY, mp_iHBoleCodes[iSp], &fValue);
            fValue *= CONVERT_KG_TO_MG;
            mp_oGrid->SetValueOfCell(iX, iY, mp_iHBoleCodes[iSp], fValue);
          }
        }
      }
    }
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
    stcErr.sFunction = "clStormKilledPartitionedBiomass::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clStormKilledPartitionedBiomass::FormatQueryString(clTreePopulation *p_oPop)
{
  char * cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];
  char cQueryPiece[15]; //for assembling the search query
  int i;

  //Do a type/species search on all the species
  strcpy( cQueryTemp, "species=" );
  for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
  {
    sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
    strcat( cQueryTemp, cQueryPiece );
  }
  sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
  strcat( cQueryTemp, cQueryPiece );

  sprintf( cQueryPiece, "%s%d", "::type=", clTreePopulation::snag );
  strcat( cQueryTemp, cQueryPiece );

  //Now put it in m_cQuery, sized correctly
  m_cQuery = new char[strlen( cQueryTemp ) + 1];
  strcpy( m_cQuery, cQueryTemp );
  delete[] cQueryTemp;
}
