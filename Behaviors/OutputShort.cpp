//---------------------------------------------------------------------------
#include <fstream>
#include "OutputShort.h"
#include "SimManager.h"
#include "Plot.h"
#include "TreePopulation.h"
#include "GhostTreePopulation.h"
#include "ParsingFunctions.h"
#include "PlatformFuncs.h"
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clShortOutput::clShortOutput( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "ShortOutput";

    //Versions
    m_fVersionNumber = 1.1;
    m_fMinimumVersionNumber = 1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Null pointers, initialize variables to empty
    mp_subplots = NULL;
    m_iNumSubplotsToSave = 0;

    mp_fLiveRBA = NULL;
    mp_fLiveABA = NULL;
    mp_fLiveRDN = NULL;
    mp_fLiveADN = NULL;
    mp_fSubRBA = NULL;
    mp_fSubABA = NULL;
    mp_fSubRDN = NULL;
    mp_fSubADN = NULL;
    mp_fDeadABA = NULL;
    mp_fDeadADN = NULL;
    mp_bSaveLiveRBA = NULL;
    mp_bSaveLiveABA = NULL;
    mp_bSaveLiveRDN = NULL;
    mp_bSaveLiveADN = NULL;
    mp_bSaveDeadABA = NULL;
    mp_bSaveDeadADN = NULL;
    mp_bSaveAnyLive = NULL;
    mp_bSaveAnyLiveBA = NULL;

    m_fXCellLength = 0;
    m_iNumXCells = 0;
    m_iNumTypes = 0;
    m_fYCellLength = 0;
    m_bUseLive = true;
    m_iNumYCells = 0;
    m_bUseDead = false;
    m_iNumSpecies = 0;
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
    stcErr.sFunction = "clShortOutput::clShortOutput" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clShortOutput::~clShortOutput()
{
  int i, j; //loop counter

  //Delete arrays

  for ( i = 0; i < m_iNumSubplotsToSave; i++ ) {
    delete[] mp_subplots[i].p_cellList; mp_subplots[i].p_cellList = NULL;
  }
  delete[] mp_subplots; mp_subplots = NULL;

  for ( i = 0; i < m_iNumTypes; i++ ) {
    delete[] mp_fLiveABA[i];
    delete[] mp_fLiveRBA[i];
    delete[] mp_fLiveADN[i];
    delete[] mp_fLiveRDN[i];
  }
  delete[] mp_fLiveABA;
  delete[] mp_fLiveRBA;
  delete[] mp_fLiveADN;
  delete[] mp_fLiveRDN;

  for ( i = 0; i < m_iNumSubplotsToSave; i++ ) {
    for ( j = 0; j < m_iNumTypes; j++ ) {
      delete[] mp_fSubRBA[i][j];
      delete[] mp_fSubABA[i][j];
      delete[] mp_fSubRDN[i][j];
      delete[] mp_fSubADN[i][j];
    }
    delete[] mp_fSubRBA[i];
    delete[] mp_fSubABA[i];
    delete[] mp_fSubRDN[i];
    delete[] mp_fSubADN[i];
  }
  delete[] mp_fSubRBA;
  delete[] mp_fSubABA;
  delete[] mp_fSubRDN;
  delete[] mp_fSubADN;

  for ( i = 0; i < m_iNumTypes; i++ ) {
    for ( j = 0; j < m_iNumSpecies; j++) {
      delete[] mp_fDeadABA[i][j];
      delete[] mp_fDeadADN[i][j];
    }
    delete[] mp_fDeadABA[i];
    delete[] mp_fDeadADN[i];
  }
  delete[] mp_fDeadABA;
  delete[] mp_fDeadADN;

  for ( i = 0; i < m_iNumTypes; i++ ) {
    delete[] mp_bSaveDeadABA[i];
    delete[] mp_bSaveDeadADN[i];
  }
  delete[] mp_bSaveDeadABA;
  delete[] mp_bSaveDeadADN;

  delete[] mp_bSaveLiveRBA;
  delete[] mp_bSaveLiveABA;
  delete[] mp_bSaveLiveRDN;
  delete[] mp_bSaveLiveADN;

  delete[] mp_bSaveAnyLive;
  delete[] mp_bSaveAnyLiveBA;
}

/////////////////////////////////////////////////////////////////////////////
// DeclareDataArrays()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::DeclareDataArrays() {
  int i, j; //loop counters

  mp_fLiveRBA = new float *[m_iNumTypes];
  mp_fLiveABA = new float *[m_iNumTypes];
  mp_fLiveRDN = new float *[m_iNumTypes];
  mp_fLiveADN = new float *[m_iNumTypes];
  for ( i = 0; i < m_iNumTypes; i++ ) {
    mp_fLiveRBA[i] = new float[m_iNumSpecies];
    mp_fLiveABA[i] = new float[m_iNumSpecies];
    mp_fLiveRDN[i] = new float[m_iNumSpecies];
    mp_fLiveADN[i] = new float[m_iNumSpecies];
  }

  mp_fDeadABA = new float **[m_iNumTypes];
  mp_fDeadADN = new float **[m_iNumTypes];
  for ( i = 0; i < m_iNumTypes; i++ ) {
    mp_fDeadABA[i] = new float *[m_iNumSpecies];
    mp_fDeadADN[i] = new float *[m_iNumSpecies];
    for ( j = 0; j < m_iNumSpecies; j++) {
      mp_fDeadABA[i][j] = new float [remove_tree+1];
      mp_fDeadADN[i][j] = new float [remove_tree+1];
    }
  }

  mp_bSaveAnyLive = new bool[m_iNumTypes];
  mp_bSaveAnyLiveBA = new bool[m_iNumTypes];

  mp_bSaveLiveABA = new bool[m_iNumTypes];
  mp_bSaveLiveADN = new bool[m_iNumTypes];
  mp_bSaveLiveRBA = new bool[m_iNumTypes];
  mp_bSaveLiveRDN = new bool[m_iNumTypes];

  mp_bSaveDeadABA = new bool*[m_iNumTypes];
  mp_bSaveDeadADN = new bool*[m_iNumTypes];
  for ( i = 0; i < m_iNumTypes; i++ ) {
    mp_bSaveDeadABA[i] = new bool[remove_tree+1];
    mp_bSaveDeadADN[i] = new bool[remove_tree+1];
  }
}

/////////////////////////////////////////////////////////////////////////////
// DeclareDataArraysForSubplots()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::DeclareDataArraysForSubplots() {
  int i, j;

  if ( m_iNumSubplotsToSave > 0 ) {
    mp_fSubRBA = new float **[m_iNumSubplotsToSave];
    mp_fSubABA = new float **[m_iNumSubplotsToSave];
    mp_fSubRDN = new float **[m_iNumSubplotsToSave];
    mp_fSubADN = new float **[m_iNumSubplotsToSave];
    for ( i = 0; i < m_iNumSubplotsToSave; i++ ) {
      mp_fSubRBA[i] = new float *[m_iNumTypes];
      mp_fSubABA[i] = new float *[m_iNumTypes];
      mp_fSubRDN[i] = new float *[m_iNumTypes];
      mp_fSubADN[i] = new float *[m_iNumTypes];
      for ( j = 0; j < m_iNumTypes; j++ ) {
        mp_fSubRBA[i][j] = new float[m_iNumSpecies];
        mp_fSubABA[i][j] = new float[m_iNumSpecies];
        mp_fSubRDN[i][j] = new float[m_iNumSpecies];
        mp_fSubADN[i][j] = new float[m_iNumSpecies];
      }
    }
  }
}


/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oTrees = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement; //document element

    m_iNumSpecies = p_oTrees->GetNumberOfSpecies();
    m_iNumTypes = p_oTrees->GetNumberOfTypes();

    //Get the doc root element
    p_oElement = p_oDoc->getDocumentElement();

    //Get the filename
    FillSingleValue( p_oElement, "so_filename", &m_sFileName, true );

    //Make sure the file extension is on - if not, append it
    if (std::string::npos == m_sFileName.find(SHORT_OUTPUT_FILE_EXT)) {
      m_sFileName += SHORT_OUTPUT_FILE_EXT;
    }

    DeclareDataArrays();

    //Extract tree info
    ExtractLiveTreeInfo( p_oDoc );
    ExtractDeadTreeInfo( p_oDoc );

    //Extract subplot info
    ExtractSubplotInfo( p_oDoc );

    DeclareDataArraysForSubplots();

    //Write file header
    WriteFileHeader();

    //Write the initial conditions
    Action();

  } //end of try block
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
    stcErr.sFunction = "clShortOutput::GetData" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// ExtractLiveTreeInfo()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::ExtractLiveTreeInfo( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    DOMNodeList * p_oTypeList; //holds the list of tree type info structures
    DOMNode * p_oDocNode;
    DOMElement * p_oTreeInfo; //one tree type's info
    short int iNumTypes, //how many types we have output data for
              iType,
              i; //loop counter
    char * cData; //extracting text data


    //****************************
    // Look for tree info
    //****************************
    //Allocate the grid memory and initialize
    for ( i = 0; i < m_iNumTypes; i++ ) {
      mp_bSaveLiveRBA[i] = false;
      mp_bSaveLiveABA[i] = false;
      mp_bSaveLiveRDN[i] = false;
      mp_bSaveLiveADN[i] = false;
    }

    //Go through each type and break out its info
    p_oTypeList = p_oDoc->getElementsByTagName( XMLString::transcode( "so_treeTypeInfo" ) );
    iNumTypes = p_oTypeList->getLength();
    for ( i = 0; i < iNumTypes; i++ ) {
      p_oDocNode = p_oTypeList->item( i );
      p_oTreeInfo = ( DOMElement * ) p_oDocNode;

      //get the type name - if something goes wrong throw an error
      cData = XMLString::transcode( p_oTreeInfo->getAttributeNode( XMLString::transcode( "type" ) )->getNodeValue() );
      if ( strcmp( "Seedling", cData ) == 0 )
        iType = clTreePopulation::seedling;
      else if ( strcmp( "Sapling", cData ) == 0 )
        iType = clTreePopulation::sapling;
      else if ( strcmp( "Adult", cData ) == 0 )
        iType = clTreePopulation::adult;
      else if ( strcmp( "Snag", cData ) == 0 )
        iType = clTreePopulation::snag;
      else
      {
        modelErr stcErr;
        stcErr.sFunction = "clShortOutput::ExtractTreeInfo" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Unrecognized or unsupported type - ";
        stcErr.sMoreInfo += cData;
        throw( stcErr );
      }

      //Check the data save tags
      mp_bSaveLiveRBA[iType] = TestForSave( p_oTreeInfo, "so_saveRBA" );
      mp_bSaveLiveABA[iType] = TestForSave( p_oTreeInfo, "so_saveABA" );
      mp_bSaveLiveRDN[iType] = TestForSave( p_oTreeInfo, "so_saveRDN" );
      mp_bSaveLiveADN[iType] = TestForSave( p_oTreeInfo, "so_saveADN" );

      //If this is a seedling, turn off options for basal area
      if ( clTreePopulation::seedling == iType ) {
        mp_bSaveLiveRBA[iType] = false;
        mp_bSaveLiveABA[iType] = false;
      }
    } //end of for ( i = 0; i < iNumTypes; i++ ) {

    //Set up master flags for density and basal area, and using any live trees
    m_bUseLive = false;
    for ( i = 0; i < m_iNumTypes; i++ ) {
      mp_bSaveAnyLiveBA[i] = mp_bSaveLiveABA[i] || mp_bSaveLiveRBA[i];
      mp_bSaveAnyLive[i] = mp_bSaveAnyLiveBA[i] || mp_bSaveLiveADN[i] ||
                            mp_bSaveLiveRDN[i];
      m_bUseLive = m_bUseLive || mp_bSaveAnyLive[i];
    }
  } //end of try block
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
    stcErr.sFunction = "clShortOutput::ExtractLiveTreeInfo" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// ExtractDeadTreeInfo()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::ExtractDeadTreeInfo( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    DOMNodeList * p_oTypeList; //holds the list of tree type info structures
    DOMNode * p_oDocNode;
    DOMElement * p_oTreeInfo; //one tree type's info
    short int iNumTypes, //how many types we have output data for
              iType, iDeadReason,
              i, j; //loop counter
    char * cData; //extracting text data


    //****************************
    // Look for tree info
    //****************************
    //Allocate the grid memory and initialize
    for ( i = 0; i < m_iNumTypes; i++ ) {
      for (j = 0; j <= remove_tree; j++) {
        mp_bSaveDeadABA[i][j] = false;
        mp_bSaveDeadADN[i][j] = false;
      }
    }

    //Go through each type and break out its info
    p_oTypeList = p_oDoc->getElementsByTagName( XMLString::transcode( "so_deadTreeTypeInfo" ) );
    iNumTypes = p_oTypeList->getLength();
    for ( i = 0; i < iNumTypes; i++ ) {
      p_oDocNode = p_oTypeList->item( i );
      p_oTreeInfo = ( DOMElement * ) p_oDocNode;

      //get the type name - if something goes wrong throw an error
      cData = XMLString::transcode( p_oTreeInfo->getAttributeNode( XMLString::transcode( "type" ) )->getNodeValue() );
      if ( strcmp( "Seedling", cData ) == 0 )
        iType = clTreePopulation::seedling;
      else if ( strcmp( "Sapling", cData ) == 0 )
        iType = clTreePopulation::sapling;
      else if ( strcmp( "Adult", cData ) == 0 )
        iType = clTreePopulation::adult;
      else if ( strcmp( "Snag", cData ) == 0 )
        iType = clTreePopulation::snag;
      else
      {
        modelErr stcErr;
        stcErr.sFunction = "clShortOutput::ExtractDeadTreeInfo" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Unrecognized or unsupported type - ";
        stcErr.sMoreInfo += cData;
        throw( stcErr );
      }

      //get the dead reason code - if something goes wrong throw an error
      cData = XMLString::transcode( p_oTreeInfo->getAttributeNode( XMLString::transcode( "reason" ) )->getNodeValue() );
      if ( strcmp( "harvest", cData ) == 0 )
        iDeadReason = harvest;
      else if ( strcmp( "natural", cData ) == 0 )
        iDeadReason = natural;
      else if ( strcmp( "disease", cData ) == 0 )
        iDeadReason = disease;
      else if ( strcmp( "fire", cData ) == 0 )
        iDeadReason = fire;
      else if ( strcmp( "insects", cData ) == 0 )
        iDeadReason = insects;
      else if ( strcmp( "storm", cData ) == 0 )
        iDeadReason = storm;
      else if ( strcmp( "remove_tree", cData ) == 0 )
        iDeadReason = remove_tree;
      else
      {
        modelErr stcErr;
        stcErr.sFunction = "clShortOutput::ExtractDeadTreeInfo" ;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = "Unrecognized or unsupported dead reason code - ";
        stcErr.sMoreInfo += cData;
        throw( stcErr );
      }

      //Check the data save tags
      mp_bSaveDeadABA[iType][iDeadReason] = TestForSave( p_oTreeInfo, "so_saveABA" );
      mp_bSaveDeadADN[iType][iDeadReason] = TestForSave( p_oTreeInfo, "so_saveADN" );

      //If this is a seedling, turn off options for basal area
      if ( clTreePopulation::seedling == iType ) {
        mp_bSaveDeadABA[iType][iDeadReason] = false;
      }
    } //end of for ( i = 0; i < iNumTypes; i++ ) {

    //Set up master flags for density and basal area, and using any live trees
    m_bUseDead = false;
    for ( i = 0; i < m_iNumTypes; i++ ) {
      for (j = 0; j <= remove_tree; j++) {
        m_bUseDead = m_bUseDead || mp_bSaveDeadABA[i][j] ||
            mp_bSaveDeadADN[i][j];
      }
    }
  } //end of try block
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
    stcErr.sFunction = "clShortOutput::ExtractDeadTreeInfo" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// ExtractSubplotInfo()
////////////////////////////////////////////////////////////////////////////
void clShortOutput::ExtractSubplotInfo( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    stcCoords * p_gridListForVal; //this is a list of grid cells that we will
    //validate before putting in the object list
    DOMNodeList * p_oGridCellList, //for getting the list of grid cells to save for
         * p_oSubplotList; //list of subplots
    DOMNode * p_oDocNode; //for retrieving data
    DOMElement * p_oSubplot, //one subplot element
         * p_oElement; //generic element object
    double fPlotLengthX = mp_oSimManager->GetPlotObject()->GetXPlotLength(),
           fPlotLengthY = mp_oSimManager->GetPlotObject()->GetYPlotLength(),
           fXLength, fYLength;
    int iX, iY; //grid cell X and Y coordinate numbers
    short int iNumSubplots, //number of subplots found
         iNumCells, //number of grid cells found for one subplot
         i, j, k, m; //loop counters
    bool bDuplicate; //for searching for duplicate values

    //Get the number of cells in each direction
    m_iNumXCells = p_oPop->GetNumXCells();
    m_iNumYCells = p_oPop->GetNumYCells();
    m_fXCellLength = p_oPop->GetGridCellSize();
    m_fYCellLength = p_oPop->GetGridCellSize();

    //Check to see if there is user-specified grid cell size information - we've
    //already collected the default info in case there isn't
    p_oElement = p_oDoc->getDocumentElement();
    fXLength = -1;
    FillSingleValue( p_oElement, "so_subplotXLength", &fXLength, false );
    if (fXLength > 0) {
      m_fXCellLength = fXLength;
      m_iNumXCells = (int)ceil( mp_oSimManager->GetPlotObject()->GetXPlotLength() / m_fXCellLength );
    }
    fYLength = -1;
    FillSingleValue( p_oElement, "so_subplotYLength", &fYLength, false );
    if (fYLength > 0) {
      m_fYCellLength = fYLength;
      m_iNumYCells = (int)ceil( mp_oSimManager->GetPlotObject()->GetYPlotLength() / m_fYCellLength );
    }

    //Get all the subplot info structures out of the file, if any
    p_oSubplotList = p_oDoc->getElementsByTagName( XMLString::transcode( "so_subplot" ) );
    iNumSubplots = p_oSubplotList->getLength();
    if ( 0 == iNumSubplots ) return; //if nothing found exit without error

    //Allocate memory
    m_iNumSubplotsToSave = iNumSubplots;
    mp_subplots = new stcSubplotInfo[m_iNumSubplotsToSave];
    for ( i = 0; i < m_iNumSubplotsToSave; i++ )
    {
      mp_subplots[i].p_cellList = NULL;
      mp_subplots[i].iNumCells = 0;
    } //end of for (i = 0; i < m_iNumSubplotsToSave; i++)

    //Go through the subplots in the file one at a time and extract the info
    for ( i = 0; i < m_iNumSubplotsToSave; i++ )
    {
      p_oDocNode = p_oSubplotList->item( i );
      p_oSubplot = ( DOMElement * ) p_oDocNode;

      //Get subplot name
      FillSingleValue( p_oSubplot, "so_subplotName", &mp_subplots[i].sSubplotName, true);

      //Get grid coords
      p_oGridCellList = p_oSubplot->getElementsByTagName( XMLString::transcode( "pointSet" ) );
      if ( 0 == p_oGridCellList->getLength() )
      {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clShortOutput::ExtractSubplotInfo" ;
        stcErr.sMoreInfo = "pointSet";
        throw( stcErr );
      }
      p_oDocNode = p_oGridCellList->item( 0 );
      p_oElement = ( DOMElement * ) p_oDocNode;
      p_oGridCellList = p_oElement->getElementsByTagName( XMLString::transcode( "po_point" ) );
      iNumCells = p_oGridCellList->getLength();
      if ( 0 == iNumCells )
      {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clShortOutput::ExtractSubplotInfo" ;
        stcErr.sMoreInfo = "po_point";
        throw( stcErr );
      }

      //Put the grid coords in a temp list so we can validate them
      p_gridListForVal = new stcCoords[iNumCells];
      k = 0;
      for ( j = 0; j < iNumCells; j++ )
      {
        p_oDocNode = p_oGridCellList->item( j );
        p_oElement = ( DOMElement * ) p_oDocNode;
        iX = atoi( XMLString::transcode( p_oElement->getAttributeNode( XMLString::transcode( "x" ) )->getNodeValue() ) );
        iY = atoi( XMLString::transcode( p_oElement->getAttributeNode( XMLString::transcode( "y" ) )->getNodeValue() ) );
        //Make sure that the values are within the plot
        if ( iX >= m_iNumXCells || iX < 0 || iY >= m_iNumYCells || iY < 0 )
        {
          modelErr stcErr;
          stcErr.sFunction = "clShortOutput::ExtractSubplotInfo" ;
          std::stringstream s;
          s << "Point value outside plot - " << iX << ", " << iY;
          stcErr.sMoreInfo = s.str();
          stcErr.iErrorCode = BAD_DATA;
        }
        //Make sure they're not duplicates - this is not cause to abort if
        //they are
        bDuplicate = false;
        for ( m = 0; m < j; m++ )
          if ( iX == p_gridListForVal[m].iX && iY == p_gridListForVal[m].iY )
            bDuplicate = true;
        if ( !bDuplicate )
        { //our values are valid and not duplicates
          p_gridListForVal[k].iX = iX;
          p_gridListForVal[k].iY = iY;
          k++;
        }
        else
          iNumCells--; //duplicate - decrease the count by one
      } //end of for (j = 0; j < iNumCells; j++)

      //transfer the no-duplicates list over to our object variables
      if ( iNumCells > 0 )
      {
        mp_subplots[i].p_cellList = new stcCoords[iNumCells];
        mp_subplots[i].iNumCells = iNumCells;
        for ( j = 0; j < iNumCells; j++ )
        {
          mp_subplots[i].p_cellList[j].iX = p_gridListForVal[j].iX;
          mp_subplots[i].p_cellList[j].iY = p_gridListForVal[j].iY;
        }
      }
      delete[] p_gridListForVal;

      //Calculate the subplot's area, correcting for odd-sized end cells
      mp_subplots[i].fArea = 0;
      for ( j = 0; j < iNumCells; j++ )
      {
        if (mp_subplots[i].p_cellList[j].iX < m_iNumXCells - 1) {
          fXLength = m_fXCellLength;
        } else {
          fXLength = fPlotLengthX - (mp_subplots[i].p_cellList[j].iX * m_fXCellLength);
        }
        if (mp_subplots[i].p_cellList[j].iY < m_iNumYCells - 1) {
          fYLength = m_fYCellLength;
        } else {
          fYLength = fPlotLengthY - (mp_subplots[i].p_cellList[j].iY * m_fYCellLength);
        }
        mp_subplots[i].fArea += fXLength * fYLength;
      }
      mp_subplots[i].fArea /= M_SQ_PER_HA;

    } //end of for (i = 0; i < m_iNumSubplotsToSave; i++)
  } //end of try block
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
    stcErr.sFunction = "clShortOutput::ExtractSubplotInfo" ;
    throw( stcErr );
  }
}



/////////////////////////////////////////////////////////////////////////////
// TestForSave()
/////////////////////////////////////////////////////////////////////////////
bool clShortOutput::TestForSave( DOMElement * p_oParentElement, const char * cNodeName )
{
  DOMNodeList * p_oNodeList; //for searching by tag name
  DOMNode * p_oDocNode; //for retrieving one node of the search
  DOMElement * p_oElement; //for casting a DOM_Node to a DOMElement
  char * cData; //for extracting the attribute text

  p_oNodeList = p_oParentElement->getElementsByTagName( XMLString::transcode( cNodeName ) );
  if ( 0 == p_oNodeList->getLength() ) return false; //nothing found

  p_oDocNode = p_oNodeList->item( 0 );
  p_oElement = ( DOMElement * ) p_oDocNode;
  cData = XMLString::transcode( p_oElement->getAttributeNode( XMLString::transcode( "save" ) )->getNodeValue() );
  if ( strcmp( "true", cData ) == 0 ) return true;

  return false;
}


/////////////////////////////////////////////////////////////////////////////
// WriteFileHeader()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::WriteFileHeader()
{
  try
  {
  	using namespace std;
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
    fstream out(m_sFileName.c_str(), ios::out | ios::trunc );
    char cABAString[80],
         cABATotalString[80],
         cRBAString[80],
         cADNString[80],
         cADNTotalString[80],
         cRDNString[80],
         cABARoot[80],
         cABATotalRoot[80],
         cADNRoot[80],
         cADNTotalRoot[80];
    short int i, j, k; //loop counter

    out << "Location: " << p_oPlot->GetPlotTitle() << "\n"
        << "From parameter file: " << mp_oSimManager->GetParFilename() << "\n"
        << "Basal area is normalized to m^2/ha; density to #/ha.\n"
        << "Subplots:\n0 = Whole plot\n";

    //Write subplot list
    for ( i = 0; i < m_iNumSubplotsToSave; i++ )
      out << i + 1 << " = " << mp_subplots[i].sSubplotName << "\n";

    //Now write column headers
    out << "Step\tSubplot";

    for ( i = 0; i < m_iNumTypes; i++ )
    {

      //Set up column header strings
      if ( clTreePopulation::seedling == i ) {
        strcpy( cADNString, "Sdl Abs Den: " );
        strcpy( cADNTotalString, "Sdl Den Total:" );
        strcpy( cRDNString, "Sdl Rel Den: " );

      } else if ( clTreePopulation::sapling == i ) {

        strcpy( cABAString, "Sapl Abs BA: " );
        strcpy( cABATotalString, "Sapl BA Total:" );
        strcpy( cRBAString, "Sapl Rel BA: " );
        strcpy( cADNString, "Sapl Abs Den: " );
        strcpy( cADNTotalString, "Sapl Den Total:" );
        strcpy( cRDNString, "Sapl Rel Den: " );

      } else if ( clTreePopulation::adult == i ) {

        strcpy( cABAString, "Adult Abs BA: " );
        strcpy( cABATotalString, "Adult BA Total:" );
        strcpy( cRBAString, "Adult Rel BA: " );
        strcpy( cADNString, "Adult Abs Den: " );
        strcpy( cADNTotalString, "Adult Den Total:" );
        strcpy( cRDNString, "Adult Rel Den: " );

      } else if ( clTreePopulation::snag == i ) {

        strcpy( cABAString, "Snag Abs BA: " );
        strcpy( cABATotalString, "Snag BA Total:" );
        strcpy( cRBAString, "Snag Rel BA: " );
        strcpy( cADNString, "Snag Abs Den: " );
        strcpy( cADNTotalString, "Snag Den Total:" );
        strcpy( cRDNString, "Snag Rel Den: " );

      }

      if ( mp_bSaveLiveABA[i] ) {
        for ( j = 0; j < m_iNumSpecies; j++ )
          out << "\t" << cABAString << p_oPop->TranslateSpeciesCodeToName( j );

        out << "\t" << cABATotalString;
      }

      if ( mp_bSaveLiveRBA[i] )
        for ( j = 0; j < m_iNumSpecies; j++ )
          out << "\t" << cRBAString << p_oPop->TranslateSpeciesCodeToName( j );

      if ( mp_bSaveLiveADN[i] ) {
        for ( j = 0; j < m_iNumSpecies; j++ )
          out << "\t" << cADNString << p_oPop->TranslateSpeciesCodeToName( j );

        out << "\t" << cADNTotalString;
      }

      if ( mp_bSaveLiveRDN[i] )
        for ( j = 0; j < m_iNumSpecies; j++ )
          out << "\t" << cRDNString << p_oPop->TranslateSpeciesCodeToName( j );
    }

    //*************************
    // Dead trees
    //*************************
    for ( i = 0; i < m_iNumTypes; i++ ) {
      //Set up column header strings
      if ( clTreePopulation::seedling == i ) {
        strcpy( cADNRoot, "Dead Sdl Abs Den: " );
        strcpy( cADNTotalRoot, "Dead Sdl Den Total:" );

      } else if ( clTreePopulation::sapling == i ) {

        strcpy( cABARoot, "Dead Sapl Abs BA: " );
        strcpy( cABATotalRoot, "Dead Sapl BA Total:" );
        strcpy( cADNRoot, "Dead Sapl Abs Den: " );
        strcpy( cADNTotalRoot, "Dead Sapl Den Total:" );

      } else if ( clTreePopulation::adult == i ) {

        strcpy( cABARoot, "Dead Adult Abs BA: " );
        strcpy( cABATotalRoot, "Dead Adult BA Total:" );
        strcpy( cADNRoot, "Dead Adult Abs Den: " );
        strcpy( cADNTotalRoot, "Dead Adult Den Total:" );

      } else if ( clTreePopulation::snag == i ) {

        strcpy( cABARoot, "Dead Snag Abs BA: " );
        strcpy( cABATotalRoot, "Dead Snag BA Total:" );
        strcpy( cADNRoot, "Dead Snag Abs Den: " );
        strcpy( cADNTotalRoot, "Dead Snag Den Total:" );

      }

      for (j = 0; j <= remove_tree; j++) {
        switch (j) {
        case harvest:
          sprintf(cABAString, "Harvest %s", cABARoot);
          sprintf(cABATotalString, "Harvest %s", cABATotalRoot);
          sprintf(cADNString, "Harvest %s", cADNRoot);
          sprintf(cADNTotalString, "Harvest %s", cADNTotalRoot);
          break;
        case natural:
          sprintf(cABAString, "Natural %s", cABARoot);
          sprintf(cABATotalString, "Natural %s", cABATotalRoot);
          sprintf(cADNString, "Natural %s", cADNRoot);
          sprintf(cADNTotalString, "Natural %s", cADNTotalRoot);
          break;
        case disease:
          sprintf(cABAString, "Disease %s", cABARoot);
          sprintf(cABATotalString, "Disease %s", cABATotalRoot);
          sprintf(cADNString, "Disease %s", cADNRoot);
          sprintf(cADNTotalString, "Disease %s", cADNTotalRoot);
          break;
        case fire:
          sprintf(cABAString, "Fire %s", cABARoot);
          sprintf(cABATotalString, "Fire %s", cABATotalRoot);
          sprintf(cADNString, "Fire %s", cADNRoot);
          sprintf(cADNTotalString, "Fire %s", cADNTotalRoot);
          break;
        case insects:
          sprintf(cABAString, "Insect %s", cABARoot);
          sprintf(cABATotalString, "Insect %s", cABATotalRoot);
          sprintf(cADNString, "Insect %s", cADNRoot);
          sprintf(cADNTotalString, "Insect %s", cADNTotalRoot);
          break;
        case storm:
          sprintf(cABAString, "Storm %s", cABARoot);
          sprintf(cABATotalString, "Storm %s", cABATotalRoot);
          sprintf(cADNString, "Storm %s", cADNRoot);
          sprintf(cADNTotalString, "Storm %s", cADNTotalRoot);
          break;
        case remove_tree:
          sprintf(cABAString, "Remove %s", cABARoot);
          sprintf(cABATotalString, "Remove %s", cABATotalRoot);
          sprintf(cADNString, "Remove %s", cADNRoot);
          sprintf(cADNTotalString, "Remove %s", cADNTotalRoot);
          break;
        }

        if ( mp_bSaveDeadABA[i][j] ) {
          for ( k = 0; k < m_iNumSpecies; k++ )
            out << "\t" << cABAString << p_oPop->TranslateSpeciesCodeToName( k );

          out << "\t" << cABATotalString;
        }

        if ( mp_bSaveDeadADN[i][j] ) {
          for ( k = 0; k < m_iNumSpecies; k++ )
            out << "\t" << cADNString << p_oPop->TranslateSpeciesCodeToName( k );

          out << "\t" << cADNTotalString;
        }
      }
    }

    out << "\n";
    out.close();

    //As a check, see if the file exists. If not, throw an error
    //because we didn't have write permission.
    if (!DoesFileExist(m_sFileName)) {
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sFunction = "clShortOutput::WriteFileHeader";
      std::stringstream s;
      s << "SORTIE could not write the file \"" << m_sFileName
        << "\". Check for invalid directories.";
      stcErr.sMoreInfo = s.str();
      throw( stcErr );
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
    stcErr.sFunction = "clShortOutput::WriteFileHeader" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetLiveTreeStats()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::GetLiveTreeStats() {
 try {
    using namespace std;
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    clTreePopulation * p_oTrees = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oTypeTrees; //pointer to search results after type search
    clTree * p_oTree; //for getting trees from the search object
    float fDbh, //dbh of tree if not a seed or seedling
         fTotal, //for relativizing values
         fBA, //tree's basal area
         fCoord, //tree's X or Y coordinate
         fPlotArea = p_oPlot->GetPlotArea(); //area of the plot, in hectares
    int iTreeX, iTreeY, //tree's X and Y grid cell numbers
        iSpecies, //tree's species
        i, j, k, iTypeIndex, iSubplotIndex; //loop counters

    for ( i = 0; i < m_iNumTypes; i++ ) {
      for ( j = 0; j < m_iNumSpecies; j++ ) {
        mp_fLiveRBA[i][j] = 0;
        mp_fLiveABA[i][j] = 0;
        mp_fLiveRDN[i][j] = 0;
        mp_fLiveADN[i][j] = 0;
      }
    }

    if ( m_iNumSubplotsToSave > 0 ) {
      for ( i = 0; i < m_iNumSubplotsToSave; i++ ) {
        for ( j = 0; j < m_iNumTypes; j++ ) {
          for ( k = 0; k < m_iNumSpecies; k++ ) {
            mp_fSubRBA[i][j][k] = 0;
            mp_fSubABA[i][j][k] = 0;
            mp_fSubRDN[i][j][k] = 0;
            mp_fSubADN[i][j][k] = 0;
          }
        }
      }
    }

    //******************************
    // Tree data saving
    //******************************
    if ( 0 == m_iNumSubplotsToSave ) {
        //Get all trees of this type
      p_oTypeTrees = p_oTrees->Find( "all" );

      //Go through the trees one at a time and add each's data to the running
      //totals
      p_oTree = p_oTypeTrees->NextTree();
      while ( p_oTree ) {
        iTypeIndex = p_oTree->GetType();
        if ( mp_bSaveAnyLive[iTypeIndex] )  {
          iSpecies = p_oTree->GetSpecies();
          if ( mp_bSaveAnyLiveBA[iTypeIndex] ) { //Basal area
            //Get the dbh
            p_oTree->GetValue( p_oTrees->GetDbhCode( iSpecies, iTypeIndex ), & fDbh );
            //Calculate basal area and add to array total
            fBA = clModelMath::CalculateBasalArea( fDbh );
            mp_fLiveABA[iTypeIndex] [iSpecies] += fBA;
          } //end of check for basal area

          //Density
          mp_fLiveADN[iTypeIndex] [iSpecies] ++;
        }
        p_oTree = p_oTypeTrees->NextTree();
      } //end of while (p_oTree)
    } else { //if ( 0 == m_iNumSubplotsToSave ) {
      //Get all trees of this type
      p_oTypeTrees = p_oTrees->Find( "all" );

      //Go through the trees one at a time and add each's data to the running
      //totals
      p_oTree = p_oTypeTrees->NextTree();
      while ( p_oTree ) {
        iTypeIndex = p_oTree->GetType();
        if ( mp_bSaveAnyLive[iTypeIndex] )  {
          iSpecies = p_oTree->GetSpecies();

          //Get the tree's X and Y grid cell numbers
          p_oTree->GetValue( p_oTrees->GetXCode( iSpecies, iTypeIndex ), & fCoord );
          iTreeX = (int)floor( fCoord / m_fXCellLength );
          p_oTree->GetValue( p_oTrees->GetYCode( iSpecies, iTypeIndex ), & fCoord );
          iTreeY = (int)floor( fCoord / m_fYCellLength );

          if ( mp_bSaveAnyLiveBA[iTypeIndex] ) { //Basal area
            //Get the dbh
            p_oTree->GetValue( p_oTrees->GetDbhCode( iSpecies, iTypeIndex ), & fDbh );
            //Calculate basal area and add to array total
            fBA = clModelMath::CalculateBasalArea( fDbh );
            mp_fLiveABA[iTypeIndex][iSpecies] += fBA;
          } //end of check for basal area

          //Density
          mp_fLiveADN[iTypeIndex][iSpecies] ++;

          //*****************************
          // Subplots
          //*****************************
          //Check to see if this tree is in any subplots
          for ( iSubplotIndex = 0; iSubplotIndex < m_iNumSubplotsToSave; iSubplotIndex++ ) {
            for ( i = 0; i < mp_subplots[iSubplotIndex].iNumCells; i++ )
              if ( iTreeX == mp_subplots[iSubplotIndex].p_cellList[i].iX
                && iTreeY == mp_subplots[iSubplotIndex].p_cellList[i].iY ) {
                if ( mp_bSaveAnyLiveBA[iTypeIndex] )
                  mp_fSubABA[iSubplotIndex][iTypeIndex][iSpecies] += fBA;
                mp_fSubADN[iSubplotIndex][iTypeIndex][iSpecies] ++;
              }
          }
        }
        p_oTree = p_oTypeTrees->NextTree();
      } //end of while (p_oTree)
    }

    for ( iTypeIndex = 0; iTypeIndex < m_iNumTypes; iTypeIndex++ ) {

      //*******************************
      // Calculate relative values, where applicable
      //*******************************
      if ( mp_bSaveLiveRBA[iTypeIndex] ) {
        fTotal = 0;
        //Add up all the basal area for all the species
        for ( i = 0; i < m_iNumSpecies; i++ )
          fTotal += mp_fLiveABA[iTypeIndex][i];

        //Divide species values by total and put in relative BA array
        for ( i = 0; i < m_iNumSpecies; i++ ) {
          if ( fTotal > 0 ) {
            mp_fLiveRBA[iTypeIndex][i] = mp_fLiveABA[iTypeIndex][i] / fTotal;
          } else {
            mp_fLiveRBA[iTypeIndex][i] = 0;
          }
        }

        //Now do subplots
        for ( iSubplotIndex = 0; iSubplotIndex < m_iNumSubplotsToSave; iSubplotIndex++ ) {
          fTotal = 0;
          for ( i = 0; i < m_iNumSpecies; i++ )
            fTotal += mp_fSubABA[iSubplotIndex][iTypeIndex][i];

          for ( i = 0; i < m_iNumSpecies; i++ ) {
            if ( fTotal > 0 ) {
              mp_fSubRBA[iSubplotIndex][iTypeIndex][i] =
                   mp_fSubABA[iSubplotIndex][iTypeIndex][i] / fTotal;
            } else {
              mp_fSubRBA[iSubplotIndex][iTypeIndex][i] = 0;
            }
          }
        }
      } //end of if ( mp_bSaveLiveRBA[iTypeIndex] )

      if ( mp_bSaveLiveRDN[iTypeIndex] ) {
        fTotal = 0;
        //Add up all the density for all the species
        for ( i = 0; i < m_iNumSpecies; i++ )
          fTotal += mp_fLiveADN[iTypeIndex][i];
        //Divide species values by total and put in relative den array
        for ( i = 0; i < m_iNumSpecies; i++ ) {
          if ( fTotal > 0 ) {
            mp_fLiveRDN[iTypeIndex][i] = mp_fLiveADN[iTypeIndex][i] / fTotal;
          } else {
            mp_fLiveRDN[iTypeIndex][i] = 0;
          }
        }

        //Now do subplots
        for ( iSubplotIndex = 0; iSubplotIndex < m_iNumSubplotsToSave; iSubplotIndex++ ) {
          fTotal = 0;
          for ( i = 0; i < m_iNumSpecies; i++ )
            fTotal += mp_fSubADN[iSubplotIndex][iTypeIndex][i];

          for ( i = 0; i < m_iNumSpecies; i++ ) {
            if ( fTotal > 0 ) {
              mp_fSubRDN[iSubplotIndex][iTypeIndex][i] =
                   mp_fSubADN[iSubplotIndex][iTypeIndex][i] / fTotal;
            } else {
              mp_fSubRDN[iSubplotIndex][iTypeIndex][i] = 0;
            }
          }
        }
      } //end of if ( mp_bSaveLiveRDN[iTypeIndex] )

      // Convert all absolute values to numbers per hectare by dividing by plot area
      if ( mp_bSaveLiveABA[iTypeIndex] ) {
        for ( i = 0; i < m_iNumSpecies; i++ )
          mp_fLiveABA[iTypeIndex][i] /= fPlotArea;

        for ( i = 0; i < m_iNumSubplotsToSave; i++ )
          for ( j = 0; j < m_iNumSpecies; j++ )
            mp_fSubABA[i][iTypeIndex][j] /= mp_subplots[i].fArea;
      }

      if ( mp_bSaveLiveADN[iTypeIndex] ) {
        for ( i = 0; i < m_iNumSpecies; i++ )
          mp_fLiveADN[iTypeIndex][i] /= fPlotArea;

        for ( i = 0; i < m_iNumSubplotsToSave; i++ )
          for ( j = 0; j < m_iNumSpecies; j++ )
            mp_fSubADN[i][iTypeIndex][j] /= mp_subplots[i].fArea;
      }
    } //end of for (iTypeIndex = 0; iTypeIndex < m_iNumTypesToSave; iTypeIndex++)
  } //end of try block
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
    stcErr.sFunction = "clShortOutput::GetLiveTreeStats" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetDeadTreeStats()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::GetDeadTreeStats() {
 try {
    using namespace std;
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    clTreePopulation * p_oTrees = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clGhostTreePopulation * p_oGhostTrees = ( clGhostTreePopulation * ) mp_oSimManager->GetPopulationObject( "GhostTreePopulation" );
    clDeadTree * p_oTree; //for getting trees from the search object
    float fDbh, //dbh of tree if not a seed or seedling
          fBA, //tree's basal area
          fPlotArea = p_oPlot->GetPlotArea(); //area of the plot, in hectares
    int iSpecies, //tree's species
        i, j, k, iTypeIndex, iDeadIndex; //loop counters

    for ( i = 0; i < m_iNumTypes; i++ ) {
      for ( j = 0; j < m_iNumSpecies; j++ ) {
        for ( k = 0; k <= remove_tree; k++) {
          mp_fDeadABA[i][j][k] = 0;
          mp_fDeadADN[i][j][k] = 0;
        }
      }
    }

    //Go through the trees one at a time and add each's data to the running
    //totals
    p_oTree = p_oGhostTrees->GetFirstTree();
    while ( p_oTree ) {
      iTypeIndex = p_oTree->GetType();
      iDeadIndex = p_oTree->GetDeadReasonCode();
      if ( mp_bSaveDeadADN[iTypeIndex][iDeadIndex] ||
           mp_bSaveDeadABA[iTypeIndex][iDeadIndex])  {
        iSpecies = p_oTree->GetSpecies();
        if ( mp_bSaveDeadABA[iTypeIndex][iDeadIndex] ) { //Basal area
          //Get the dbh
          p_oTree->GetValue( p_oTrees->GetDbhCode( iSpecies, iTypeIndex ), & fDbh );
          //Calculate basal area and add to array total
          fBA = clModelMath::CalculateBasalArea( fDbh );
          mp_fDeadABA[iTypeIndex][iSpecies][iDeadIndex] += fBA;
        } //end of check for basal area

        //Density
        mp_fDeadADN[iTypeIndex][iSpecies][iDeadIndex]++;
      }
      p_oTree = p_oTree->GetNext();
    } //end of while (p_oTree)

    for ( i = 0; i < m_iNumTypes; i++ ) {
      for ( j = 0; j < m_iNumSpecies; j++ ) {
        for ( k = 0; k <= remove_tree; k++) {
          mp_fDeadABA[i][j][k] /= fPlotArea;
          mp_fDeadADN[i][j][k] /= fPlotArea;
        }
      }
    }
  } //end of try block
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
    stcErr.sFunction = "clShortOutput::GetDeadTreeStats" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// WriteTimestepData()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::WriteTimestepData()
{
  try
  {
    using namespace std;
    fstream out( m_sFileName.c_str(), ios::app | ios::out );
    float fTotal;  //for relativizing values
    int iSpecies, iType, iSubplot, iDeadReason; //loop counters

    //Write data to file
    out << mp_oSimManager->GetCurrentTimestep() << "\t0"; //zeroth subplot = whole
    //plot
    for ( iType = 0; iType < m_iNumTypes; iType++ ) {

      if ( mp_bSaveLiveABA[iType] ) {
        fTotal = 0;
        for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ ) {
          fTotal += mp_fLiveABA[iType][iSpecies];
          out << "\t" << mp_fLiveABA[iType][iSpecies];
        }
        //Total line
        out << "\t" << fTotal;
      }

      if ( mp_bSaveLiveRBA[iType] )
        for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ )
          out << "\t" << mp_fLiveRBA[iType][iSpecies];

      if ( mp_bSaveLiveADN[iType] ) {
        fTotal = 0;
        for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ ) {
          fTotal += mp_fLiveADN[iType][iSpecies];
          out << "\t" << mp_fLiveADN[iType][iSpecies];
        }
        //Total line
        out << "\t" << fTotal;
      }

      if ( mp_bSaveLiveRDN[iType] )
        for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ )
          out << "\t" << mp_fLiveRDN[iType][iSpecies];

    } //end of for ( i = 0; i < m_iNumTypes; i++ )


    // Dead trees
    for ( iType = 0; iType < m_iNumTypes; iType++ ) {
      for (iDeadReason = 0; iDeadReason <= remove_tree; iDeadReason++) {
        if ( mp_bSaveDeadABA[iType][iDeadReason] ) {
          fTotal = 0;
          for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ ) {
            fTotal += mp_fDeadABA[iType][iSpecies][iDeadReason];
            out << "\t" << mp_fDeadABA[iType][iSpecies][iDeadReason];
          }
          //Total line
          out << "\t" << fTotal;
        }

        if ( mp_bSaveDeadADN[iType][iDeadReason] ) {
          fTotal = 0;
          for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ ) {
            fTotal += mp_fDeadADN[iType][iSpecies][iDeadReason];
            out << "\t" << mp_fDeadADN[iType][iSpecies][iDeadReason];
          }
          //Total line
          out << "\t" << fTotal;
        }
      }
    }

    out << "\n";

    //Now subplots
    for ( iSubplot = 0; iSubplot < m_iNumSubplotsToSave; iSubplot++ ) {
      out << mp_oSimManager->GetCurrentTimestep() << "\t" << iSubplot + 1;

      for ( iType = 0; iType < m_iNumTypes; iType++ ) {

        if ( mp_bSaveLiveABA[iType] ) {
          fTotal = 0;
          for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ ) {
            fTotal += mp_fSubABA[iSubplot][iType][iSpecies];
            out << "\t" << mp_fSubABA[iSubplot][iType][iSpecies];
          }
          //Total line
          out << "\t" << fTotal;
        }

        if ( mp_bSaveLiveRBA[iType] )
          for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ )
            out << "\t" << mp_fSubRBA[iSubplot][iType][iSpecies];

        if ( mp_bSaveLiveADN[iType] ) {
          fTotal = 0;
          for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ ) {
            fTotal += mp_fSubADN[iSubplot][iType][iSpecies];
            out << "\t" << mp_fSubADN[iSubplot][iType][iSpecies];
          }
          //Total line
          out << "\t" << fTotal;
        }

        if ( mp_bSaveLiveRDN[iType] )
          for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ )
            out << "\t" << mp_fSubRDN[iSubplot][iType][iSpecies];

      } //end of for ( iType = 0; iType < m_iNumTypes; iType++ )

      // Dead trees - no subplots for them
      for ( iType = 0; iType < m_iNumTypes; iType++ ) {
      for (iDeadReason = 0; iDeadReason <= remove_tree; iDeadReason++) {
        if ( mp_bSaveDeadABA[iType][iDeadReason] ) {
          for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ ) {
            out << "\tNA";
          }
          //Total line
          out << "\tNA";
        }

        if ( mp_bSaveDeadADN[iType][iDeadReason] ) {
          for ( iSpecies = 0; iSpecies < m_iNumSpecies; iSpecies++ ) {
            out << "\tNA";
          }
          //Total line
          out << "\tNA";
        }
      }
    }

      out << "\n";
    }

    out.close();
  } //end of try block
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
    stcErr.sFunction = "clShortOutput::WriteTimestepData" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clShortOutput::Action()
{
  if (m_bUseLive) GetLiveTreeStats();
  if (m_bUseDead) GetDeadTreeStats();
  WriteTimestepData();

}
