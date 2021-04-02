//---------------------------------------------------------------------------
#include "HarvestInterface.h"
#include "SimManager.h"
#include "Grid.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "Disturbance.h"
#include "PlatformFuncs.h"
#include <stdio.h>
#include <fstream>
#include <math.h>
#include <sstream>

#define PARAM_FILE_LINE_LENGTH 1000

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clHarvestInterface::clHarvestInterface( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "HarvestInterface";
    m_sXMLRoot = "HarvestInterface";

    //Versions
    m_fVersionNumber = 1.1;
    m_fMinimumVersionNumber = 1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    m_iReasonCode = harvest;

    mp_sNewTreeDataMembers = NULL;
    mp_sFileColumns = NULL;
    mp_bAppliesToTrees = NULL;
    //mp_fInitialValues = NULL;
    mp_iMemberType = NULL;
    mp_bUserDefinedColumn = NULL;
    mp_iColumnTranslation = NULL;

    mp_oResultsGrid = NULL;
    mp_iDenCutCodes = NULL;
    mp_iBaCutCodes = NULL;

    m_iNewTreeFloats = 0;
    m_iNumFileColumns = 0;

    m_iPeriod = 0;
    m_iNumSpecies = 0;
    m_iNumAllowedCutRanges = 0;
    m_iNextTimestepToHarvest = 0;
    m_iHarvestTypeCode = -1;
  }
  catch ( modelErr & err )
  {
    throw( err );
  } //non-fatal error
  catch ( ... )
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clHarvestInterface::clHarvestInterface" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clHarvestInterface::~clHarvestInterface()
{
  int i;

  delete[] mp_sNewTreeDataMembers;

  delete[] mp_sFileColumns;

  if (mp_bAppliesToTrees) {
    for (i = 0; i < m_iNumSpecies; i++)
      delete[] mp_bAppliesToTrees[i];
  }
  delete[] mp_bAppliesToTrees;
  delete[] mp_iMemberType;
  //delete[] mp_fInitialValues;
  delete[] mp_bUserDefinedColumn;
  delete[] mp_iColumnTranslation;;

  if ( mp_iDenCutCodes ) {
    for ( i = 0; i < m_iNumAllowedCutRanges; i++ )
    {
      delete[] mp_iDenCutCodes[i];
      delete[] mp_iBaCutCodes[i];
    }
    delete[] mp_iDenCutCodes;
    delete[] mp_iBaCutCodes;
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetNameData
/////////////////////////////////////////////////////////////////////////////
void clHarvestInterface::SetNameData(std::string sNameString)
{
  //See if any data members have been passed in on the namestring
  std::string::size_type iStart, iEnd;
  int iValNum;

  iStart = sNameString.find('(');
  if (std::string::npos == iStart) {
    //No data members passed
    m_iNewTreeFloats = 0;
    return;
  }

  //How many new data members are there?
  while (std::string::npos != iStart) {
    m_iNewTreeFloats++;
    iStart++;
    iStart = sNameString.find('(', iStart);
  }

  //Declare a place for these new labels
  mp_sNewTreeDataMembers = new string[m_iNewTreeFloats];
  //mp_fInitialValues = new float[m_iNewTreeFloats];

  //Parse out the data members
  iValNum = 0;
  iStart = sNameString.find('(');
  while (std::string::npos != iStart) {

    //Get the name
    iEnd = sNameString.find(')', iStart);
    if (std::string::npos == iEnd) {
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Couldn't parse out the new data members.";
      stcErr.sFunction = "clHarvestInterface::SetNameData" ;
      throw( stcErr );
    }

    iStart++;
    mp_sNewTreeDataMembers[iValNum] = sNameString.substr(iStart, (iEnd-iStart));

    //Get the initial value
    /*p_cStart = strchr(p_cStart, '@');
    p_cEnd = strchr(p_cStart, ')');
    if (NULL == p_cEnd) {
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "Couldn't parse out the new data members.";
      stcErr.sFunction = "clHarvestInterface::SetNameData" ;
      throw( stcErr );
    }
    strcpy(p_cTemp, "");
    p_cStart++;
    p_cEnd--;
    iCharCount = 0;
    for (p_cLoop = p_cStart; p_cLoop != p_cEnd; p_cLoop++) {
      p_cTemp[iCharCount] = *p_cLoop;
      iCharCount++;
    }
    p_cTemp[iCharCount] = '\0';

    //Transform to a float
    mp_fInitialValues[iValNum] = atof(p_cTemp);
    if (0 == mp_fInitialValues[iValNum] &&
        '0' != p_cTemp[0]) {
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      sprintf( stcErr.cMoreInfo, "%s%s%s%s%s",
           "Initial value for new tree data member \"",
           mp_cNewTreeDataMembers[iValNum], "\" is not a number: \"",
           p_cTemp, "\"." );
      stcErr.sFunction = "clHarvestInterface::SetNameData" ;
      throw( stcErr );
    } */

    iValNum++;
    iStart = sNameString.find('(', iStart);
  }
}

/////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers
/////////////////////////////////////////////////////////////////////////////
void clHarvestInterface::RegisterTreeDataMembers()
{
  clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
  int iNumTypes, iSp, iTp, i;

  //Set up the species/type applies-to array
  m_iNumSpecies = p_oPop->GetNumberOfSpecies();
  iNumTypes = p_oPop->GetNumberOfTypes();

  mp_bAppliesToTrees = new bool*[m_iNumSpecies];
  for (iSp = 0; iSp < m_iNumSpecies; iSp++) {
    mp_bAppliesToTrees[iSp] = new bool[iNumTypes];
    for (iTp = 0; iTp < iNumTypes; iTp++) {
      mp_bAppliesToTrees[iSp][iTp] = false;
    }
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    mp_bAppliesToTrees[mp_whatSpeciesTypeCombos[i].iSpecies]
                      [mp_whatSpeciesTypeCombos[i].iType] = true;
  }

  if (0 == m_iNewTreeFloats) return;

  for (iSp = 0; iSp < m_iNumSpecies; iSp++) {
    for (iTp = 0; iTp < iNumTypes; iTp++) {
      if (mp_bAppliesToTrees[iSp][iTp]) {
        for (i = 0; i < m_iNewTreeFloats; i++) {
          p_oPop->RegisterFloat(mp_sNewTreeDataMembers[i], iSp, iTp);
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clHarvestInterface::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    using namespace std;
    DOMNodeList *p_oColumns,
                *p_oAllColumns;
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    ofstream parametersOut;
    ifstream parametersIn;
    char *cData, cTemp[PARAM_FILE_LINE_LENGTH];
    int iNumYearsPerTS = mp_oSimManager->GetNumberOfYearsPerTimestep(),
        iNumColumns, //number of extra columns added by user to input file
        iNumBaseColumns = 6, //number of default required columns
        iSp, iTp, i, j, //loop counters
        iNumLines, //line counter
        iCode,
        iParamLine, //line on the batch parameters file, if applicable
        iNumTypes = p_oPop->GetNumberOfTypes();

    //***********************************
    //Setup the "Harvest Results" grid
    //***********************************

    SetupResultsGrid();


    //***********************************
    //Read parameters
    //***********************************

    //Executable
    FillSingleValue( p_oElement, "hi_executable", &m_sExecutable, true );

    //Make sure that the harvest executable exists
    if (!DoesFileExist(m_sExecutable)) {
      modelErr stcErr;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      std::stringstream s;
      s << "Missing file: " << m_sExecutable;
      stcErr.sMoreInfo = s.str();
      stcErr.sFunction = "clHarvestInterface::GetData" ;
      throw( stcErr );
    }

    // Path and filename of the input file to the harvest executable
    FillSingleValue( p_oElement, "hi_harvestableTreesFile", &m_sInputFile, true );

    // Path and filename of the file of the trees to cut that is created by the
    // harvest executable
    FillSingleValue( p_oElement, "hi_treesToHarvestFile", &m_sTreesToCutFile, true );

    // Harvest period
    FillSingleValue( p_oElement, "hi_harvestPeriod", &m_iPeriod, true );
    if (0 > m_iPeriod) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      std::stringstream s;
      s << "Harvest period must be a non-negative number. Value: \""
        << m_iPeriod << "\".";
      stcErr.sMoreInfo = s.str();
      stcErr.sFunction = "clHarvestInterface::GetData" ;
      throw( stcErr );
    }
    if (m_iPeriod > 0) {
      m_iPeriod /= iNumYearsPerTS;
      if (m_iPeriod < 1)
        m_iPeriod = 1;
    }
    m_iNextTimestepToHarvest = m_iPeriod;

    // Path and filename of the file of the trees to update that is created by
    // the harvest executable - optional
    if (m_iNewTreeFloats > 0) {
      FillSingleValue( p_oElement, "hi_treesToUpdateFile", &m_sTreesToUpdateFile, false );
    }

    // Argument string to pass to the user executable - optional
    FillSingleValue( p_oElement, "hi_executableArguments", &m_sArguments, false );

    //***********************************
    //Read parameters for and set up the file columns
    //***********************************
    // List of columns beyond the defaults - optional
    m_iNumFileColumns = iNumBaseColumns;
    p_oAllColumns = p_oElement->getElementsByTagName( XMLString::transcode( "hi_dataMembers" ) );
    if (0 < p_oAllColumns->getLength()) {
      p_oElement = ( DOMElement * ) p_oAllColumns->item( 0 );

      p_oColumns = p_oElement->getElementsByTagName( XMLString::transcode( "hi_dataMember" ) );
      iNumColumns = p_oColumns->getLength();

      if (iNumColumns > 0) {
        m_iNumFileColumns += iNumColumns;

        //Declare the columns array so we can put the data there
        mp_sFileColumns = new string[m_iNumFileColumns];
        for ( i = 0; i < iNumColumns; i++ )
        {
          cData = xercesc::XMLString::transcode(p_oColumns->item(i)->getChildNodes()->item(0)->getNodeValue());
          mp_sFileColumns[iNumBaseColumns + i] = cData;
        }
      }
    }
    //Set up the default columns
    if (NULL == mp_sFileColumns) {
      mp_sFileColumns = new string[m_iNumFileColumns];
    }

    mp_iMemberType = new member_type[m_iNumFileColumns];
    for (i = 0; i < m_iNumFileColumns; i++)
      mp_iMemberType[i] = notassigned;

    mp_sFileColumns[0] = "X";
    mp_iMemberType[0] = isfloat;
    mp_sFileColumns[1] = "Y";
    mp_iMemberType[1] = isfloat;
    mp_sFileColumns[2] = "Species";
    mp_iMemberType[2] = isint;
    mp_sFileColumns[3] = "Type";
    mp_iMemberType[3] = isint;
    mp_sFileColumns[4] = "Diam";
    mp_iMemberType[4] = isfloat;
    mp_sFileColumns[5] = "Height";
    mp_iMemberType[5] = isfloat;

    //Verify that any additional columns exist, and what kind they are, and
    //that the kinds agree for all tree types and species
    if (m_iNumFileColumns > iNumBaseColumns) {
      for (i = iNumBaseColumns; i < m_iNumFileColumns; i++) {
        for (iSp = 0; iSp < m_iNumSpecies; iSp++) {
          for (iTp = 0; iTp < iNumTypes; iTp++) {
            if (mp_bAppliesToTrees[iSp][iTp]) {
              iCode = -1;

              //Is it a float?
              if (p_oPop->GetFloatDataCode(mp_sFileColumns[i],iSp, iTp) > -1) {
                iCode = 0;
                if (notassigned == mp_iMemberType[i]) {
                  mp_iMemberType[i] = isfloat;
                } else if (isfloat != mp_iMemberType[i]) {
                  modelErr stcErr;
                  stcErr.iErrorCode = BAD_DATA;
                  std::stringstream s;
                  s << "Tree data member \"" << mp_sFileColumns[i] << "\" of ambiguous type.";
                  stcErr.sMoreInfo = s.str();
                  stcErr.sFunction = "clHarvestInterface::GetData" ;
                  throw( stcErr );
                }
              }

              //Is it an int?
              if (p_oPop->GetIntDataCode(mp_sFileColumns[i],iSp, iTp) > -1) {
                iCode = 0;
                if (notassigned == mp_iMemberType[i]) {
                  mp_iMemberType[i] = isint;
                } else if (isint != mp_iMemberType[i]) {
                  modelErr stcErr;
                  stcErr.iErrorCode = BAD_DATA;
                  std::stringstream s;
                  s << "Tree data member \"" << mp_sFileColumns[i] << "\" of ambiguous type.";
                  stcErr.sMoreInfo = s.str();
                  stcErr.sFunction = "clHarvestInterface::GetData" ;
                  throw( stcErr );
                }
              }

              //Is it a bool?
              if (p_oPop->GetBoolDataCode(mp_sFileColumns[i],iSp, iTp) > -1) {
                iCode = 0;
                if (notassigned == mp_iMemberType[i]) {
                  mp_iMemberType[i] = isbool;
                } else if (isbool != mp_iMemberType[i]) {
                  modelErr stcErr;
                  stcErr.iErrorCode = BAD_DATA;
                  std::stringstream s;
                  s << "Tree data member \"" << mp_sFileColumns[i] << "\" of ambiguous type.";
                  stcErr.sMoreInfo = s.str();
                  stcErr.sFunction = "clHarvestInterface::GetData" ;
                  throw( stcErr );
                }
              }

              //Is it a char?
              if (p_oPop->GetStringDataCode(mp_sFileColumns[i],iSp, iTp) > -1) {
                iCode = 0;
                if (notassigned == mp_iMemberType[i]) {
                  mp_iMemberType[i] = ischar;
                } else if (ischar != mp_iMemberType[i]) {
                  modelErr stcErr;
                  stcErr.iErrorCode = BAD_DATA;
                  std::stringstream s;
                  s << "Tree data member \"" << mp_sFileColumns[i] << "\" of ambiguous type.";
                  stcErr.sMoreInfo = s.str();
                  stcErr.sFunction = "clHarvestInterface::GetData" ;
                  throw( stcErr );
                }
              }

              //Make sure we found this data member for this tree type and
              //species
              if (-1 == iCode) {
                modelErr stcErr;
                stcErr.iErrorCode = BAD_DATA;
                std::stringstream s;
                s << "Tree data member \"" << mp_sFileColumns[i]
                  << "\" missing for trees of species " << iSp
                  << " and type " << iTp << ".";
                stcErr.sMoreInfo = s.str();
                stcErr.sFunction = "clHarvestInterface::GetData" ;
                throw( stcErr );
              }
            }
          }
        }
      }
    }

    mp_bUserDefinedColumn = new bool[m_iNumFileColumns];
    mp_iColumnTranslation = new int[m_iNumFileColumns];
    for (i = 0; i < m_iNumFileColumns; i++) {
      mp_bUserDefinedColumn[i] = false;
      mp_iColumnTranslation[i] = -1;
      for (j = 0; j < m_iNewTreeFloats; j++) {
        if (0 == mp_sFileColumns[i].compare(mp_sNewTreeDataMembers[j])) {
          mp_bUserDefinedColumn[i] = true;
          mp_iColumnTranslation[i] = j;
          break;
        }
      }
    }

    //***********************************
    //Process batch parameters, if warranted
    //***********************************
    if (0 < mp_oSimManager->GetBatchNumber()) {

      // Path and filename of the batch parameters input file
      FillSingleValue( p_oElement, "hi_batchParamsFile", &m_sBatchFileIn, false );

      //Only continue if we've got a file
      if (0 < m_sBatchFileIn.length()) {

        //Make sure that the file exists
        if (!DoesFileExist(m_sBatchFileIn)) {
          modelErr stcErr;
          stcErr.iErrorCode = CANT_FIND_OBJECT;
          std::stringstream s;
          s << "Missing file: " << m_sBatchFileIn;
          stcErr.sMoreInfo = s.str();
          stcErr.sFunction = "clHarvestInterface::GetData" ;
          throw( stcErr );
        }

        // Path and filename of the parameters output file that this will write
        FillSingleValue( p_oElement, "hi_batchSingleRunParamsFile", &m_sBatchParamsOut, true );

        //What line are we on in the parameter file?
        iParamLine = mp_oSimManager->GetBatchNumber();

        //Read that line of the parameters
        parametersIn.open( m_sBatchFileIn.c_str() );
        iNumLines = 1;
        while (!parametersIn.eof() && iNumLines < iParamLine) {
          parametersIn.getline(cTemp, PARAM_FILE_LINE_LENGTH);
          if (strlen(cTemp) > 1)
            iNumLines++;
        }
        if (iNumLines != iParamLine) {
          modelErr stcErr;
          stcErr.sFunction = "clHarvestInterface::GetData";
          std::stringstream s;
          s << "Couldn't find parameters for batch run #" << iParamLine;
          stcErr.sMoreInfo = s.str();
          stcErr.iErrorCode = BAD_FILE;
          throw(stcErr);
        }

        parametersIn.getline(cTemp, PARAM_FILE_LINE_LENGTH);
        if (strlen(cTemp) < 2) {
          modelErr stcErr;
          stcErr.sFunction = "clHarvestInterface::GetData";
          std::stringstream s;
          s << "Couldn't find parameters for batch run #" << iParamLine;
          stcErr.sMoreInfo = s.str();
          stcErr.iErrorCode = BAD_FILE;
          throw(stcErr);
        }
        parametersIn.close();

        //Write parameter file for the executable
        parametersOut.open( m_sBatchParamsOut.c_str(), ios::trunc | ios::out);

        parametersOut << cTemp;
        parametersOut << endl;
        parametersOut.close();
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
    stcErr.sFunction = "clHarvestInterface::GetData" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// Action()
//////////////////////////////////////////////////////////////////////////////
void clHarvestInterface::Action()
{
  float *p_fX = NULL, *p_fY = NULL, *p_fDiam = NULL, **p_fNewVals = NULL;
  int *p_iSp = NULL, *p_iTp = NULL;
  bool *p_bFound = NULL;
  try
  {
    using namespace std;

    ResetResultsGrid();

    if (mp_oSimManager->GetCurrentTimestep() != m_iNextTimestepToHarvest) {
      return;
    } else {
      m_iNextTimestepToHarvest += m_iPeriod;
    }
    char cTemp[1000];
    fstream harvest, update, harvest2, update2; //harvested tree file
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch *p_oHarvestTrees = p_oPop->Find("all");
    clTree *p_oTree = p_oHarvestTrees->NextTree();
    string sSpecies, sType, sTemp;
    float fVal, fTemp, fDiam, fX, fY, fThisBA;
    long int i, j;
    int iSp, iTp, iTemp, iCount,
        iVal, iNumLines,
        iNumBaseColumns = 6;
    bool bVal;

    //Write all applicable trees out to file
    harvest.open( m_sInputFile.c_str(), ios::out | ios::trunc );

    //Write the first line - current timestep and number of timesteps
    harvest << mp_oSimManager->GetCurrentTimestep()
            << "\t"
            << mp_oSimManager->GetNumberOfTimesteps()
            << "\n";

    //Write the second line - the column names
    harvest << mp_sFileColumns[0];
    for (i = 1; i < m_iNumFileColumns; i++) {
      harvest << "\t" << mp_sFileColumns[i];
    }
    harvest << "\n";

    //Write the trees
    while (p_oTree) {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      if (mp_bAppliesToTrees[iSp][iTp]) {

        //Write out the required columns
        //X
        p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fVal);
        harvest << fVal << "\t";

        //Y
        p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fVal);
        harvest << fVal << "\t";

        //Species and type
        harvest << iSp << "\t" << iTp << "\t";

        //Diameter
        if (clTreePopulation::seedling == iTp) {
          p_oTree->GetValue(p_oPop->GetDiam10Code(iSp, iTp), &fVal);
        } else {
          p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fVal);
        }
        harvest << fVal << "\t";

        //Height
        p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fVal);
        harvest << fVal;

        for (i = iNumBaseColumns; i < m_iNumFileColumns; i++) {
          harvest << "\t";
          if (isfloat == mp_iMemberType[i]) {
            p_oTree->GetValue(p_oPop->GetFloatDataCode(mp_sFileColumns[i], iSp, iTp), &fVal);
            harvest << fVal;
          }
          else if (isint == mp_iMemberType[i]) {
            p_oTree->GetValue(p_oPop->GetIntDataCode(mp_sFileColumns[i], iSp, iTp), &iVal);
            harvest << iVal;
          }
          else if (isbool == mp_iMemberType[i]) {
            p_oTree->GetValue(p_oPop->GetBoolDataCode(mp_sFileColumns[i], iSp, iTp), &bVal);
            harvest << bVal;
          }
          else {
            p_oTree->GetValue(p_oPop->GetFloatDataCode(mp_sFileColumns[i], iSp, iTp), &sTemp);
            harvest << sTemp;
          }
        }
        harvest << "\n";
      }
      p_oTree = p_oHarvestTrees->NextTree();
    }
    harvest.close();

    //Call the harvest executable
    LaunchProcess(m_sExecutable, m_sArguments, "");

    //**********************************
    //Harvest trees
    //**********************************
    //Find the output file of harvested trees
    if (!DoesFileExist(m_sTreesToCutFile)) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sMoreInfo = "I can't find the harvested trees file.";
      stcErr.sFunction = "clHarvestInterface::Action" ;
      throw( stcErr );
    }

    //Count the number of lines
    harvest.open(m_sTreesToCutFile.c_str(), ios::in );
    iNumLines = -2; //skip the first two lines
    while (!harvest.eof()) {
       harvest.getline(cTemp, 1000);
       if (strlen(cTemp) > 1)
         iNumLines++;
    }

    if (0 < iNumLines) {
      //Declare arrays to hold all the data
      p_fDiam = new float[iNumLines];
      p_fX = new float[iNumLines];
      p_fY = new float[iNumLines];
      p_iSp = new int[iNumLines];
      p_iTp = new int[iNumLines];
      p_bFound = new bool[iNumLines];

      //Go back to the beginning of the file and read it into the arrays
      iCount = 0;
      harvest.close();
      //I tried to seek the file back to the beginning but couldn't get it to
      //go - so I just started over with a new filestream
      harvest2.open(m_sTreesToCutFile.c_str(), ios::in );
      //Throw out the first two lines
      harvest2.getline(cTemp, PARAM_FILE_LINE_LENGTH);
      harvest2.getline(cTemp, PARAM_FILE_LINE_LENGTH);

      while (!harvest2.eof() && iCount < iNumLines) {
        //Throw away all columns except the first five (X, Y, type, species,
        //diameter)
        harvest2 >> p_fX[iCount] >> p_fY[iCount] >> p_iSp[iCount]
                 >> p_iTp[iCount] >> p_fDiam[iCount];
        harvest2.getline(cTemp, PARAM_FILE_LINE_LENGTH);

        p_bFound[iCount] = false;
        iCount++;
      }
      harvest2.close();

      //Find each tree in the list and kill it
      p_oHarvestTrees->StartOver();
      p_oTree = p_oHarvestTrees->NextTree();
      while (p_oTree) {
        iSp = p_oTree->GetSpecies();
        iTp = p_oTree->GetType();
        if (clTreePopulation::seedling == iTp) {
          p_oTree->GetValue(p_oPop->GetDiam10Code(iSp, iTp), &fDiam);
        } else {
          p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), & fDiam);
        }
        p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), & fX);
        p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), & fY);

        //Try to find it in the list
        for (i = 0; i < iNumLines; i++) {
          if (iSp == p_iSp[i] &&
              iTp == p_iTp[i] &&
              fabs(fDiam - p_fDiam[i]) < 0.001 &&
              fabs(fX - p_fX[i]) < 0.001 &&
              fabs(fY - p_fY[i]) < 0.001 &&
              p_bFound[i] == false) {

            //Kill the tree
            p_oPop->KillTree( p_oTree, m_iReasonCode );

            //Add to harvest results grid
            mp_oResultsGrid->SetValueAtPoint( fX, fY, m_iHarvestTypeCode, clDisturbance::partial );
            mp_oResultsGrid->GetValueAtPoint( fX, fY, mp_iDenCutCodes[0][iSp], & iTemp );
            iTemp += 1;
            mp_oResultsGrid->SetValueAtPoint( fX, fY, mp_iDenCutCodes[0][iSp], iTemp );

            if (iTp != clTreePopulation::seedling)
              fThisBA = clModelMath::CalculateBasalArea( fDiam );
            else //tree is a seedling and has no BA
              fThisBA = 0.0;

            mp_oResultsGrid->GetValueAtPoint( fX, fY, mp_iBaCutCodes[0][iSp], & fTemp );
            fTemp += fThisBA;
            mp_oResultsGrid->SetValueAtPoint( fX, fY, mp_iBaCutCodes[0][iSp], fTemp );

            //Mark it as found
            p_bFound[i] = true;
            break;
          }
        }

        p_oTree = p_oHarvestTrees->NextTree();
      }

      //Make sure all the trees in the list got found
      for (i = 0; i < iNumLines; i++) {
        if (false == p_bFound[i]) {
          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized tree in harvest file. X = " <<  p_fX[i]
            << ", Y = " << p_fY[i] << ", species = " << p_iSp[i]
            << ", type = " << p_iTp[i] << ", diam = " << p_fDiam[i] << ".";
          stcErr.sMoreInfo = s.str();
          stcErr.sFunction = "clHarvestInterface::Action" ;


          throw( stcErr );
        }
      }
    } //end of if (0 < iNumLines) - harvest file reading

    delete[] p_fDiam; p_fDiam = NULL;
    delete[] p_fX; p_fX = NULL;
    delete[] p_fY; p_fY = NULL;
    delete[] p_iSp; p_iSp = NULL;
    delete[] p_iTp; p_iTp = NULL;
    delete[] p_bFound; p_bFound = NULL;

    //**********************************
    //Update trees
    //**********************************
    if (m_sTreesToUpdateFile.length() == 0) return;

    //Test to make sure the update file exists
    if (!DoesFileExist(m_sTreesToUpdateFile)) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_FILE;
      stcErr.sMoreInfo = "I can't find the updated trees file.";
      stcErr.sFunction = "clHarvestInterface::Action" ;
      throw( stcErr );
    }

    //Count the number of lines
    update.open(m_sTreesToUpdateFile.c_str(), ios::in );
    iNumLines = -2; //skip the first two lines
    while (!update.eof()) {
       update.getline(cTemp,100);
       if (strlen(cTemp) > 1)
         iNumLines++;
    }

    if (0 < iNumLines) {
      //Declare arrays to hold all the data
      p_fDiam = new float[iNumLines];
      p_fX = new float[iNumLines];
      p_fY = new float[iNumLines];
      p_iSp = new int[iNumLines];
      p_iTp = new int[iNumLines];
      p_bFound = new bool[iNumLines];
      //Array to hold values of user variables
      p_fNewVals = new float*[m_iNewTreeFloats];
      for (i = 0; i < m_iNewTreeFloats; i++)
        p_fNewVals[i] = new float[iNumLines];

      //Go back to the beginning of the file and read it into the arrays
      iCount = 0;
      update.close();
      //I tried to seek the file back to the beginning but couldn't get it to
      //go - so I just started over with a new filestream
      update2.open(m_sTreesToUpdateFile.c_str(), ios::in );
      //Throw out the first two lines
      update2.getline(cTemp, PARAM_FILE_LINE_LENGTH);
      update2.getline(cTemp, PARAM_FILE_LINE_LENGTH);
      while (!update2.eof() && iCount < iNumLines) {
        //Read in the first five columns (X, Y, type, species, diameter)
        update2 >> p_fX[iCount] >> p_fY[iCount] >> p_iSp[iCount]
                >> p_iTp[iCount] >> p_fDiam[iCount];

        //Throw away height
        update2 >> cTemp;

        //Now read in any columns for new user variables
        for (i = iNumBaseColumns; i < m_iNumFileColumns; i++) {
          if (mp_bUserDefinedColumn[i]) {
            update2 >> p_fNewVals[mp_iColumnTranslation[i]][iCount];
          }
          else {
            //Throw away the value
            update2 >> cTemp;
          }
        }

        p_bFound[iCount] = false;
        iCount++;
      }
      update2.close();

      //Find each tree in the list and update it
      p_oHarvestTrees->StartOver();
      p_oTree = p_oHarvestTrees->NextTree();
      while (p_oTree) {
        iSp = p_oTree->GetSpecies();
        iTp = p_oTree->GetType();
        if (clTreePopulation::seedling == iTp) {
          p_oTree->GetValue(p_oPop->GetDiam10Code(iSp, iTp), &fDiam);
        } else {
          p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), & fDiam);
        }
        p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), & fX);
        p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), & fY);

        //Try to find it in the list
        for (i = 0; i < iNumLines; i++) {
          if (iSp == p_iSp[i] &&
              iTp == p_iTp[i] &&
              fabs(fDiam - p_fDiam[i]) < 0.001 &&
              fabs(fX - p_fX[i]) < 0.001 &&
              fabs(fY - p_fY[i]) < 0.001 &&
              p_bFound[i] == false) {

            //Update the tree
            for (j = 0; j < m_iNewTreeFloats; j++) {
              p_oTree->SetValue(p_oPop->GetFloatDataCode(mp_sNewTreeDataMembers[j], iSp, iTp), p_fNewVals[j][i]);
            }

            //Mark it as found
            p_bFound[i] = true;
            break;
          }
        }

        p_oTree = p_oHarvestTrees->NextTree();
      }

      //Make sure all the trees in the list got found
      for (i = 0; i < iNumLines; i++) {
        if (false == p_bFound[i]) {

          modelErr stcErr;
          stcErr.iErrorCode = BAD_DATA;
          std::stringstream s;
          s << "Unrecognized tree in update file. X = " << p_fX[i]
            << ", Y = " << p_fY[i] << ", species = " << p_iSp[i]
                                                                                                                  << ", type = " << p_iTp[i] << ", diam = " << p_fDiam[i] << ".";
          stcErr.sMoreInfo = s.str();
          stcErr.sFunction = "clHarvestInterface::Action" ;
          throw( stcErr );
        }
      }
    } //end of if (0 < iNumLines) - harvest file reading

    delete[] p_fDiam;
    delete[] p_fX;
    delete[] p_fY;
    delete[] p_iSp;
    delete[] p_iTp;
    delete[] p_bFound;
    if (p_fNewVals) {
      for (i = 0; i < m_iNewTreeFloats; i++) {
        delete[] p_fNewVals[i];
      }
    }
    delete[] p_fNewVals;


  } //end of try block
  catch ( modelErr & err )
  {
    delete[] p_fDiam; p_fDiam = NULL;
    delete[] p_fX; p_fX = NULL;
    delete[] p_fY; p_fY = NULL;
    delete[] p_iSp; p_iSp = NULL;
    delete[] p_iTp; p_iTp = NULL;
    delete[] p_bFound; p_bFound = NULL;
    if (p_fNewVals) {
      for (int i = 0; i < m_iNewTreeFloats; i++)
        delete[] p_fNewVals[i];
    }
    delete[] p_fNewVals;
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
    stcErr.sFunction = "clHarvestInterface::Action" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// SetupResultsGrid()
//////////////////////////////////////////////////////////////////////////////
void clHarvestInterface::SetupResultsGrid()
{
  try
  {
    clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    clBehaviorBase *p_oTemp = mp_oSimManager->GetBehaviorObject("Harvest");
    clDisturbance *p_oHarvest = dynamic_cast<clDisturbance*>(p_oTemp);
    stringstream sLabel;
    float fCellLength = p_oPop->GetGridCellSize();
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i, j;

    m_iNumAllowedCutRanges = p_oHarvest->GetNumberOfCutRanges();


    //*************************************
    //Harvest results grid
    //*************************************

    //Declare the arrays for density and ba codes
    mp_iDenCutCodes = new short int * [m_iNumAllowedCutRanges];
    mp_iBaCutCodes = new short int * [m_iNumAllowedCutRanges];
    for ( i = 0; i < m_iNumAllowedCutRanges; i++ )
    {
      mp_iDenCutCodes[i] = new short int[iNumSpecies];
      mp_iBaCutCodes[i] = new short int[iNumSpecies];
    }

    mp_oResultsGrid = mp_oSimManager->CreateGrid( "Harvest Results", //grid name
         1 + ( m_iNumAllowedCutRanges * iNumSpecies ), //number of int data members
         ( m_iNumAllowedCutRanges * iNumSpecies ), //num float data members
         0, //number of char data members
         0, //number of bool data members
         fCellLength, //X cell length
         fCellLength ); //Y cell length

    //Register the data members
    m_iHarvestTypeCode = mp_oResultsGrid->RegisterInt( "Harvest Type" );

    for ( i = 0; i < m_iNumAllowedCutRanges; i++ )
      for ( j = 0; j < iNumSpecies; j++ )
      {
        sLabel << "Cut Density_" << i << "_" << j;
        mp_iDenCutCodes[i] [j] = mp_oResultsGrid->RegisterInt(sLabel.str());
        sLabel.str("");
        sLabel << "Cut Basal Area_" << i << "_" << j;
        mp_iBaCutCodes[i] [j] = mp_oResultsGrid->RegisterFloat(sLabel.str());
        sLabel.str("");
      }

    //Use the ResetResultsGrid function to initialize the values
    ResetResultsGrid();

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
    stcErr.sFunction = "clHarvestInterface::SetupResultsGrid" ;
    throw( stcErr );
  }
}



//////////////////////////////////////////////////////////////////////////////
// ResetResultsGrid()
//////////////////////////////////////////////////////////////////////////////
void clHarvestInterface::ResetResultsGrid()
{
  try
  {
  	clTreePopulation *p_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");
    float fValue = 0;
    int iValue;
    short int iNumXCells = mp_oResultsGrid->GetNumberXCells(),
              iNumYCells = mp_oResultsGrid->GetNumberYCells(),
              iNumSpecies = p_oPop->GetNumberOfSpecies(),
              i, j, iSp, iRange; //loop counters

    //For the harvest results grid - reset all values
    for ( i = 0; i < iNumXCells; i++ )
      for ( j = 0; j < iNumYCells; j++ )
      {

        iValue = -1;
        mp_oResultsGrid->SetValueOfCell( i, j, m_iHarvestTypeCode, iValue );

        iValue = 0;
        //Cut Density and Cut Basal Area - set values to 0
        for ( iSp = 0; iSp < iNumSpecies; iSp++ )
          for ( iRange = 0; iRange < m_iNumAllowedCutRanges; iRange++ )
          {
            mp_oResultsGrid->SetValueOfCell( i, j, mp_iDenCutCodes[iRange] [iSp], iValue );
            mp_oResultsGrid->SetValueOfCell( i, j, mp_iBaCutCodes[iRange] [iSp], fValue );
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
    stcErr.sFunction = "clHarvestInterface::ResetResultsGrid" ;
    throw( stcErr );
  }
}

