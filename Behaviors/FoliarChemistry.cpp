//---------------------------------------------------------------------------
// FoliarChemistry.cpp
//---------------------------------------------------------------------------
#include "FoliarChemistry.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clFoliarChemistry::clFoliarChemistry( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "FoliarChemistry";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_fA = NULL;
    mp_fB = NULL;
    mp_fN = NULL;
    mp_fP = NULL;
    mp_fLignin = NULL;
    mp_fFiber = NULL;
    mp_fCellulose = NULL;
    mp_fTannins = NULL;
    mp_fPhenolics = NULL;
    mp_fSLA = NULL;
    m_cQuery = NULL;
    mp_bAppliesTo = NULL;
    mp_iNCodes = NULL;
    mp_iPCodes = NULL;
    mp_iSLACodes = NULL;
    mp_iLigninCodes = NULL;
    mp_iFiberCodes = NULL;
    mp_iCelluloseCodes = NULL;
    mp_iTanninsCodes = NULL;
    mp_iPhenolicsCodes = NULL;
    mp_iDbhCodes = NULL;
    mp_iXCodes = NULL;
    mp_iYCodes = NULL;
    m_iNumTotalSpecies = 0;
    mp_oGrid = NULL;
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
    stcErr.sFunction = "clFoliarChemistry::clFoliarChemistry" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clFoliarChemistry::~clFoliarChemistry()
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

  if ( mp_iXCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iXCodes[i];
  delete[] mp_iXCodes;

  if ( mp_iYCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iYCodes[i];
  delete[] mp_iYCodes;

  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fN;
  delete[] mp_fP;
  delete[] mp_fLignin;
  delete[] mp_fFiber;
  delete[] mp_fCellulose;
  delete[] mp_fTannins;
  delete[] mp_fPhenolics;
  delete[] mp_fSLA;
  delete[] m_cQuery;
  delete[] mp_iNCodes;
  delete[] mp_iPCodes;
  delete[] mp_iSLACodes;
  delete[] mp_iLigninCodes;
  delete[] mp_iFiberCodes;
  delete[] mp_iCelluloseCodes;
  delete[] mp_iTanninsCodes;
  delete[] mp_iPhenolicsCodes;
}

/////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
/////////////////////////////////////////////////////////////////////////////
void clFoliarChemistry::GetParameterFileData( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  DOMElement * p_oElement = p_oDoc->getDocumentElement();
  floatVal * p_fTempValues; //for getting species-specific values
  int i;

  //Set up our floatVal array that will extract values only for the species
  //assigned to this behavior
  p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTempValues[i].code = mp_iWhatSpecies[i];

  //Declare the appropriate arrays
  mp_fA = new float[m_iNumTotalSpecies];
  mp_fB = new float[m_iNumTotalSpecies];
  mp_fN = new float[m_iNumTotalSpecies];
  mp_fP = new float[m_iNumTotalSpecies];
  mp_fLignin = new float[m_iNumTotalSpecies];
  mp_fFiber = new float[m_iNumTotalSpecies];
  mp_fCellulose = new float[m_iNumTotalSpecies];
  mp_fTannins = new float[m_iNumTotalSpecies];
  mp_fPhenolics = new float[m_iNumTotalSpecies];
  mp_fSLA = new float[m_iNumTotalSpecies];

  FillSpeciesSpecificValue( p_oElement, "an_foliarChemWeightA", "an_fcwaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemWeightB", "an_fcwbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemN", "an_fcnVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fN[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemP", "an_fcpVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fP[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemLignin", "an_fclVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fLignin[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemFiber", "an_fcfVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fFiber[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemCellulose", "an_fccVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fCellulose[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemTannins", "an_fctVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fTannins[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemPhenolics", "an_fcphVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fPhenolics[p_fTempValues[i].code] = p_fTempValues[i].val;
  }
  FillSpeciesSpecificValue( p_oElement, "an_foliarChemSLA", "an_fcsVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
    mp_fSLA[p_fTempValues[i].code] = p_fTempValues[i].val;
  }

  delete[] p_fTempValues;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clFoliarChemistry::GetData( xercesc::DOMDocument * p_oDoc )
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
    stcErr.sFunction = "clFoliarChemistry::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetAppliesTo()
/////////////////////////////////////////////////////////////////////////////
void clFoliarChemistry::GetAppliesTo()
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
      stcErr.sFunction = "clFoliarChemistry::GetAppliesTo" ;
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
void clFoliarChemistry::GetTreeCodes(clTreePopulation *p_oPop)
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

  mp_iDbhCodes = new short int*[m_iNumTotalSpecies];
  for (i = 0; i < m_iNumTotalSpecies; i++) {
    mp_iDbhCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++) {
      mp_iDbhCodes[i][j] = p_oPop->GetDbhCode(i, j + clTreePopulation::sapling);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetupGrid()
/////////////////////////////////////////////////////////////////////////////
void clFoliarChemistry::SetupGrid()
{
  std::stringstream sLabel;
  short int i; //loop counter

  //Declare the arrays for our grid codes
  mp_iNCodes = new short int[m_iNumTotalSpecies];
  mp_iPCodes = new short int[m_iNumTotalSpecies];
  mp_iSLACodes = new short int[m_iNumTotalSpecies];
  mp_iLigninCodes = new short int[m_iNumTotalSpecies];
  mp_iFiberCodes = new short int[m_iNumTotalSpecies];
  mp_iCelluloseCodes = new short int[m_iNumTotalSpecies];
  mp_iTanninsCodes = new short int[m_iNumTotalSpecies];
  mp_iPhenolicsCodes = new short int[m_iNumTotalSpecies];

  //Check to see if this grid has already been created
  mp_oGrid = mp_oSimManager->GetGridObject("Foliar Chemistry");

  if (NULL == mp_oGrid) {

    //Create the grid with five float data members for each species
    mp_oGrid = mp_oSimManager->CreateGrid( "Foliar Chemistry",
        0,                       //number of ints
        8 * m_iNumTotalSpecies,  //number of floats
        0,                       //number of chars
        0);                      //number of bools

    //Register the data member called "N_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "N_" << i;
      mp_iNCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
    //Register the data member called "P_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "P_" << i;
      mp_iPCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
    //Register the data member called "SLA_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "SLA_" << i;
      mp_iSLACodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
    //Register the data member called "lignin_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "lignin_" << i;
      mp_iLigninCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
    //Register the data member called "fiber_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "fiber_" << i;
      mp_iFiberCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
    //Register the data member called "cellulose_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "cellulose_" << i;
      mp_iCelluloseCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
    //Register the data member called "tannins_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "tannins_" << i;
      mp_iTanninsCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
    //Register the data member called "phenolics_x"
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      sLabel << "phenolics_" << i;
      mp_iPhenolicsCodes[i] = mp_oGrid->RegisterFloat(sLabel.str());
sLabel.str("");
    }
  }
  else {
    //Grid already exists - get the codes
    for (i = 0; i < m_iNumTotalSpecies; i++) {
      //Get the data member called "N_x"
      sLabel << "N_" << i;
      mp_iNCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iNCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clFoliarChemistry::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //Get the data member called "P_x"
      sLabel << "P_" << i;
      mp_iPCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iPCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clFoliarChemistry::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //Get the data member called "SLA_x"
      sLabel << "SLA_" << i;
      mp_iSLACodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iSLACodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clFoliarChemistry::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //Get the data member called "lignin_x"
      sLabel << "lignin_" << i;
      mp_iLigninCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iLigninCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clFoliarChemistry::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //Get the data member called "fiber_x"
      sLabel << "fiber_" << i;
      mp_iFiberCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iFiberCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clFoliarChemistry::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //Get the data member called "cellulose_x"
      sLabel << "cellulose_" << i;
      mp_iCelluloseCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iCelluloseCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clFoliarChemistry::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //Get the data member called "tannins_x"
      sLabel << "tannins_" << i;
      mp_iTanninsCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iTanninsCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clFoliarChemistry::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //Get the data member called "phenolics_x"
      sLabel << "phenolics_" << i;
      mp_iPhenolicsCodes[i] = mp_oGrid->GetFloatDataCode(sLabel.str());
      sLabel.str("");
      if (-1 == mp_iPhenolicsCodes[i]) {
        modelErr stcErr;
        stcErr.sFunction = "clFoliarChemistry::SetupGrid" ;
        std::stringstream s;
        s << "Couldn't find the \"" << sLabel.str() << "\" member of the \"Partitioned Biomass\" grid.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clFoliarChemistry::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    float fGridValue, //value in a grid
    fDbh,
    fX, fY,
    fFoliarWeight,
    fValue; //tree's value
    int iNumXCells = mp_oGrid->GetNumberXCells(),
        iNumYCells = mp_oGrid->GetNumberYCells(),
        iX, iY, iSp, iTp;

    //Reset the values in the species total grid to 0
    fValue = 0;
    for (iX = 0; iX < iNumXCells; iX++) {
      for (iY = 0; iY < iNumYCells; iY++) {
        for (iSp = 0; iSp < m_iNumTotalSpecies; iSp++) {
          mp_oGrid->SetValueOfCell(iX, iY, mp_iNCodes[iSp], fValue);
          mp_oGrid->SetValueOfCell(iX, iY, mp_iPCodes[iSp], fValue);
          mp_oGrid->SetValueOfCell(iX, iY, mp_iSLACodes[iSp], fValue);
          mp_oGrid->SetValueOfCell(iX, iY, mp_iLigninCodes[iSp], fValue);
          mp_oGrid->SetValueOfCell(iX, iY, mp_iFiberCodes[iSp], fValue);
          mp_oGrid->SetValueOfCell(iX, iY, mp_iCelluloseCodes[iSp], fValue);
          mp_oGrid->SetValueOfCell(iX, iY, mp_iTanninsCodes[iSp], fValue);
          mp_oGrid->SetValueOfCell(iX, iY, mp_iPhenolicsCodes[iSp], fValue);
        }
      }
    }

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();

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
        fFoliarWeight = mp_fA[iSp] * pow(fDbh, mp_fB[iSp]);

        //Calculate this tree's N and add it to the total
        fValue = fFoliarWeight * mp_fN[iSp];
        if (fValue > 0) {
          mp_oGrid->GetValueAtPoint(fX, fY, mp_iNCodes[iSp], &fGridValue);
          fGridValue += fValue;
          mp_oGrid->SetValueAtPoint(fX, fY, mp_iNCodes[iSp], fGridValue);
        }

        //Calculate this tree's P and add it to the total
        fValue = fFoliarWeight * mp_fP[iSp];
        if (fValue > 0) {
          mp_oGrid->GetValueAtPoint(fX, fY, mp_iPCodes[iSp], &fGridValue);
          fGridValue += fValue;
          mp_oGrid->SetValueAtPoint(fX, fY, mp_iPCodes[iSp], fGridValue);
        }

        //Calculate this tree's lignin and add it to the total
        fValue = fFoliarWeight * mp_fLignin[iSp];
        if (fValue > 0) {
          mp_oGrid->GetValueAtPoint(fX, fY, mp_iLigninCodes[iSp], &fGridValue);
          fGridValue += fValue;
          mp_oGrid->SetValueAtPoint(fX, fY, mp_iLigninCodes[iSp], fGridValue);
        }

        //Calculate this tree's fiber and add it to the total
        fValue = fFoliarWeight * mp_fFiber[iSp];
        if (fValue > 0) {
          mp_oGrid->GetValueAtPoint(fX, fY, mp_iFiberCodes[iSp], &fGridValue);
          fGridValue += fValue;
          mp_oGrid->SetValueAtPoint(fX, fY, mp_iFiberCodes[iSp], fGridValue);
        }

        //Calculate this tree's cellulose and add it to the total
        fValue = fFoliarWeight * mp_fCellulose[iSp];
        if (fValue > 0) {
          mp_oGrid->GetValueAtPoint(fX, fY, mp_iCelluloseCodes[iSp], &fGridValue);
          fGridValue += fValue;
          mp_oGrid->SetValueAtPoint(fX, fY, mp_iCelluloseCodes[iSp], fGridValue);
        }

        //Calculate this tree's tannins and add it to the total
        fValue = fFoliarWeight * mp_fTannins[iSp];
        if (fValue > 0) {
          mp_oGrid->GetValueAtPoint(fX, fY, mp_iTanninsCodes[iSp], &fGridValue);
          fGridValue += fValue;
          mp_oGrid->SetValueAtPoint(fX, fY, mp_iTanninsCodes[iSp], fGridValue);
        }

        //Calculate this tree's phenolics and add it to the total
        fValue = fFoliarWeight * mp_fPhenolics[iSp];
        if (fValue > 0) {
          mp_oGrid->GetValueAtPoint(fX, fY, mp_iPhenolicsCodes[iSp], &fGridValue);
          fGridValue += fValue;
          mp_oGrid->SetValueAtPoint(fX, fY, mp_iPhenolicsCodes[iSp], fGridValue);
        }

        //Calculate this tree's SLA and add it to the total
        fValue = fFoliarWeight * mp_fSLA[iSp];
        if (fValue > 0) {
          mp_oGrid->GetValueAtPoint(fX, fY, mp_iSLACodes[iSp], &fGridValue);
          fGridValue += fValue;
          mp_oGrid->SetValueAtPoint(fX, fY, mp_iSLACodes[iSp], fGridValue);
        }
      }

      p_oTree = p_oBehaviorTrees->NextTree();
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
    stcErr.sFunction = "clFoliarChemistry::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clFoliarChemistry::FormatQueryString(clTreePopulation *p_oPop)
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
