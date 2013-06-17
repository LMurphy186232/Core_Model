//---------------------------------------------------------------------------
// PartitionedBiomass.cpp
//---------------------------------------------------------------------------
#include "PartitionedBiomass.h"
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
clPartitionedBiomass::clPartitionedBiomass( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "PartitionedBiomass";
    m_sXMLRoot = "PartitionedBiomass";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 2;
    m_fMinimumVersionNumber = 2;

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
    mp_bAppliesTo = NULL;
    mp_iLeafCodes = NULL;
    mp_iBranchCodes = NULL;
    mp_iBoleCodes = NULL;
    mp_iHLeafCodes = NULL;
    mp_iHBoleCodes = NULL;
    mp_iDbhCodes = NULL;
    mp_iHeightCodes = NULL;
    mp_iXCodes = NULL;
    mp_iYCodes = NULL;
    mp_oGrid = NULL;

    m_iNumTotalSpecies = 0;
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
    stcErr.sFunction = "clPartitionedBiomass::clPartitionedBiomass" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clPartitionedBiomass::~clPartitionedBiomass()
{
  int i;

  if ( mp_bAppliesTo )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_bAppliesTo[i];
  delete[] mp_bAppliesTo;

  if ( mp_iDbhCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iDbhCodes[i];
  delete[] mp_iDbhCodes;

  if ( mp_iHeightCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iHeightCodes[i];
  delete[] mp_iHeightCodes;

  if ( mp_iXCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iXCodes[i];
  delete[] mp_iXCodes;

  if ( mp_iYCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iYCodes[i];
  delete[] mp_iYCodes;

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
void clPartitionedBiomass::SetNameData(std::string sNameString) {
  try {

    //Check the string passed and set the flags accordingly
    if (sNameString.compare("PartitionedDBHBiomass") == 0) {
      m_bIsDbh = true;
    } else if (sNameString.compare("PartitionedHeightBiomass") == 0) {
      m_bIsDbh = false;
    }
    else {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Unrecognized behavior name \"" << sNameString << "\".";
      stcErr.sFunction = "clPartitionedBiomass::SetNameData";
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPartitionedBiomass::SetNameData";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
/////////////////////////////////////////////////////////////////////////////
void clPartitionedBiomass::GetParameterFileData( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  floatVal * p_fTempValues; //for getting species-specific values
  int i;

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  if (m_bIsDbh) {
    //*******************
    //This is DBH biomass
    //*******************

    //Declare the appropriate arrays
    mp_fDBHLeafA = new float[m_iNumTotalSpecies];
    mp_fDBHLeafB = new float[m_iNumTotalSpecies];
    mp_fDBHBranchA = new float[m_iNumTotalSpecies];
    mp_fDBHBranchB = new float[m_iNumTotalSpecies];
    mp_fDBHBoleA = new float[m_iNumTotalSpecies];
    mp_fDBHBoleB = new float[m_iNumTotalSpecies];

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
    mp_fHeightLeafA = new float[m_iNumTotalSpecies];
    mp_fHeightLeafB = new float[m_iNumTotalSpecies];
    mp_fHeightBoleA = new float[m_iNumTotalSpecies];
    mp_fHeightBoleB = new float[m_iNumTotalSpecies];

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
void clPartitionedBiomass::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

    GetAppliesTo();
    GetTreeCodes(p_oPop);
    GetParameterFileData(p_oDoc, p_oPop);
    FormatQueryString(p_oPop);
    SetupGrid();
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
    stcErr.sFunction = "clPartitionedBiomass::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetAppliesTo()
/////////////////////////////////////////////////////////////////////////////
void clPartitionedBiomass::GetAppliesTo()
{
  int iNumTypes = 3,
      i, j;

  mp_bAppliesTo = new bool*[m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_bAppliesTo[i] = new bool[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_bAppliesTo[i][j] = false;
    }
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    if (clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType) {
      modelErr stcErr;
      stcErr.sFunction = "clPartitionedBiomass::GetAppliesTo" ;
      stcErr.sMoreInfo = "Partitioned Biomass behavior cannot be applied to seedlings.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
    mp_bAppliesTo[mp_whatSpeciesTypeCombos[i].iSpecies]
                  [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] = true;
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetTreeCodes()
/////////////////////////////////////////////////////////////////////////////
void clPartitionedBiomass::GetTreeCodes(clTreePopulation *p_oPop)
{
  int iNumTypes = 3,
      i, j;

  mp_iXCodes = new short int*[m_iNumTotalSpecies];
  mp_iYCodes = new short int*[m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_iXCodes[i] = new short int[iNumTypes];
    mp_iYCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iXCodes[i][j] = p_oPop->GetXCode(i, j + clTreePopulation::sapling);
      mp_iYCodes[i][j] = p_oPop->GetYCode(i, j + clTreePopulation::sapling);
    }
  }

  if (m_bIsDbh) {
    mp_iDbhCodes = new short int*[m_iNumTotalSpecies];
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      mp_iDbhCodes[i] = new short int[iNumTypes];
      for (j = 0; j < iNumTypes; j++) {
        mp_iDbhCodes[i][j] = p_oPop->GetDbhCode(i, j + clTreePopulation::sapling);
      }
    }
  } else {
    mp_iHeightCodes = new short int*[m_iNumTotalSpecies];
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      mp_iHeightCodes[i] = new short int[iNumTypes];
      for (j = 0; j < iNumTypes; j++) {
        mp_iHeightCodes[i][j] = p_oPop->GetHeightCode(i, j + clTreePopulation::sapling);
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clPartitionedBiomass::SetupGrid()
{
  std::stringstream sLabel;
  short int i; //loop counter

  //Declare the arrays for our grid codes
  mp_iLeafCodes = new short int[m_iNumTotalSpecies];
  mp_iBranchCodes = new short int[m_iNumTotalSpecies];
  mp_iBoleCodes = new short int[m_iNumTotalSpecies];
  mp_iHLeafCodes = new short int[m_iNumTotalSpecies];
  mp_iHBoleCodes = new short int[m_iNumTotalSpecies];

  //Check to see if this grid has already been created
  mp_oGrid = mp_oSimManager->GetGridObject("Partitioned Biomass");

  if (NULL == mp_oGrid) {

    //Create the grid with five float data members for each species
    mp_oGrid = mp_oSimManager->CreateGrid( "Partitioned Biomass",
        0,                       //number of ints
        5 * m_iNumTotalSpecies, //number of floats
        0,                       //number of chars
        0);                      //number of bools

    //Register the data member called "leaf_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "leaf_" << i;
      mp_iLeafCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
    //Register the data member called "branch_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "branch_" << i;
      mp_iBranchCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
    //Register the data member called "bole_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "bole_" << i;
      mp_iBoleCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
    //Register the data member called "hleaf_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "hleaf_" << i;
      mp_iHLeafCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
    //Register the data member called "hbole_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "hbole_" << i;
      mp_iHBoleCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
      sLabel.str("");
    }
  }
  else {
    //Grid already exists - get the codes
    //Get the data member called "leaf_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "leaf_" << i;
      mp_iLeafCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iLeafCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clPartitionedBiomass::SetupGrid";
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel
            << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");

      //Get the data member called "branch_x"
      sLabel << "branch_" << i;
      mp_iBranchCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iBranchCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clPartitionedBiomass::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel
            << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");

      //Get the data member called "bole_x"
      sLabel << "bole_" << i;
      mp_iBoleCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iBoleCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clPartitionedBiomass::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel
            << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");

      //Get the data member called "hleaf_x"
      sLabel << "hleaf_" << i;
      mp_iHLeafCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iHLeafCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clPartitionedBiomass::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel
            << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      sLabel.str("");

      //Get the data member called "hbole_x"
      sLabel << "hbole_" << i;
      mp_iHBoleCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iHBoleCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clPartitionedBiomass::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel
            << "\" member of the \"Partitioned Biomass\" grid.";
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
void clPartitionedBiomass::Action()
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
        iX, iY, iSp, iTp;

    //Reset the values in the species total grid to 0
    fValue = 0;
    if (m_bIsDbh) {
      for (iX = 0; iX < iNumXCells; iX++) {
        for (iY = 0; iY < iNumYCells; iY++) {
          for (iSp = 0; iSp < m_iNumTotalSpecies; iSp++) {
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
          for (iSp = 0; iSp < m_iNumTotalSpecies; iSp++) {
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
        iTp = p_oTree->GetType() - clTreePopulation::sapling;

        //Double-check this tree's appropriateness
        if ( mp_bAppliesTo[iSp][iTp] )
        {
          //Get this tree's coordinates
          p_oTree->GetValue( mp_iXCodes[iSp][iTp], &fX);
          p_oTree->GetValue( mp_iYCodes[iSp][iTp], &fY);

          //Get this tree's DBH
          p_oTree->GetValue( mp_iDbhCodes[iSp] [iTp], & fDbh );

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

        p_oTree = p_oBehaviorTrees->NextTree();
      }

      //Transform all values to Mg
      for (iX = 0; iX < iNumXCells; iX++) {
        for (iY = 0; iY < iNumYCells; iY++) {
          for (iSp = 0; iSp < m_iNumTotalSpecies; iSp++) {
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
        iTp = p_oTree->GetType() - clTreePopulation::sapling;

        //Double-check this tree's appropriateness
        if ( mp_bAppliesTo[iSp][iTp] )
        {
          //Get this tree's coordinates
          p_oTree->GetValue( mp_iXCodes[iSp][iTp], &fX);
          p_oTree->GetValue( mp_iYCodes[iSp][iTp], &fY);

          //Get this tree's height
          p_oTree->GetValue( mp_iHeightCodes[iSp] [iTp], & fHeight );

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

        p_oTree = p_oBehaviorTrees->NextTree();
      }

      //Transform all values to Mg
      for (iX = 0; iX < iNumXCells; iX++) {
        for (iY = 0; iY < iNumYCells; iY++) {
          for (iSp = 0; iSp < m_iNumTotalSpecies; iSp++) {
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
    stcErr.sFunction = "clPartitionedBiomass::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clPartitionedBiomass::FormatQueryString(clTreePopulation *p_oPop)
{
  char * cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];
  char cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSapling = false, bAdult = false, bSnag = false;

  //Do a type/species search on all the types and species
  strcpy( cQueryTemp, "species=" );
  for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
  {
    sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
    strcat( cQueryTemp, cQueryPiece );
  }
  sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
  strcat( cQueryTemp, cQueryPiece );

  //Find all the types
  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSapling = true;
    }
    else if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType )
    {
      bAdult = true;
    }
    else if ( clTreePopulation::snag == mp_whatSpeciesTypeCombos[i].iType )
    {
      bSnag = true;
    }
  }
  strcat( cQueryTemp, "::type=" );
  if ( bSapling )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::sapling, "," );
    strcat( cQueryTemp, cQueryPiece );
  }
  if ( bAdult )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::adult, "," );
    strcat( cQueryTemp, cQueryPiece );
  }
  if ( bSnag )
  {
    sprintf( cQueryPiece, "%d%s", clTreePopulation::snag, "," );
    strcat( cQueryTemp, cQueryPiece );
  }

  //Remove the last comma
  cQueryTemp[strlen( cQueryTemp ) - 1] = '\0';

  //Now put it in m_cQuery, sized correctly
  m_cQuery = new char[strlen( cQueryTemp ) + 1];
  strcpy( m_cQuery, cQueryTemp );
  delete[] cQueryTemp;
}
