//---------------------------------------------------------------------------
// RandomBrowse.cpp
//---------------------------------------------------------------------------
#include "RandomBrowse.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clRandomBrowse::clRandomBrowse( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "RandomBrowse";
    m_sXMLRoot = "RandomBrowse";

    m_iPDF = deterministic_pdf;
    m_iNumSpecies = 0;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Indicate that this behavior intends to add one tree bool data
    //member
    m_iNewTreeBools = 1;

    mp_fBrowseProb = NULL;
    mp_fBrowseStdDev = NULL;
    mp_iBrowsedCodes = NULL;
    m_cQuery = NULL;

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
    stcErr.sFunction = "clRandomBrowse::clRandomBrowse" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clRandomBrowse::~clRandomBrowse()
{
  delete[] m_cQuery;
  delete[] mp_fBrowseProb;
  delete[] mp_fBrowseStdDev;
  if ( mp_iBrowsedCodes )
  {
    for ( int i = 0; i < m_iNumSpecies; i++ )
    {
      delete[] mp_iBrowsedCodes[i];
    }
  }
  delete[] mp_iBrowsedCodes;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clRandomBrowse::GetData( xercesc::DOMDocument * p_oDoc )
{
  floatVal * p_fTempValues = NULL; //for getting species-specific values
  char *cQueryTemp = NULL;
  bool *p_bTypes = NULL;
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    char cQueryPiece[5]; //for assembling the search query
    float fYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
    int iNumTypes = p_oPop->GetNumberOfTypes(),
        i, iTemp;

    cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];

    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Get the PDF for browse adjustment
    FillSingleValue(p_oElement, "di_randBrowsePDF", & iTemp, true);
    //Make sure the value is valid
    if (deterministic_pdf == iTemp)
    {
      m_iPDF = deterministic_pdf;
    }
    else if (normal_pdf == iTemp)
    {
      m_iPDF = normal_pdf;
    }
    else
    {
      modelErr stcErr;
      stcErr.sFunction = "clRandomBrowse::GetData";
      std::stringstream s;
      s << "Unrecognized value for di_randBrowsePDF: " << iTemp;
      stcErr.sMoreInfo = s.str();
      stcErr.iErrorCode = BAD_DATA;
      throw(stcErr);
    }

    //Get the parameter file value for browsed probability
    mp_fBrowseProb = new float[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++) {
      mp_fBrowseProb[i] = 0;
    }
    FillSpeciesSpecificValue( p_oElement,
                              "di_randBrowseProb",
                              "di_rbpVal",
                              p_fTempValues,
                              m_iNumBehaviorSpecies,
                              p_oPop,
                              true );

    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBrowseProb[p_fTempValues[i].code] = p_fTempValues[i].val;

    //If normal draw probability, get the standard deviation
    mp_fBrowseStdDev = new float[m_iNumSpecies];
    for (i = 0; i < m_iNumSpecies; i++) {
      mp_fBrowseStdDev[i] = 0;
    }
    if (normal_pdf == m_iPDF) {
      FillSpeciesSpecificValue( p_oElement,
                                "di_randBrowseStdDev",
                                "di_rbsdVal",
                                p_fTempValues,
                                m_iNumBehaviorSpecies,
                                p_oPop,
                                true );

      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        mp_fBrowseStdDev[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Verify that the values are probabilities between 0 and 1, and compound
    //to per-timestep probability
    for ( i = 0; i < m_iNumSpecies; i++ ) {
      if (mp_fBrowseProb[i] < 0 || mp_fBrowseProb[i] > 1) {
        modelErr stcErr;
        stcErr.sFunction = "clRandomBrowse::GetData";
        std::stringstream s;
        s << "Invalid browse probability value \"" << mp_fBrowseProb[i]
          << "\" for species " << i << ".  Values must be between 0 and 1.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      mp_fBrowseProb[i] = 1 - pow(1 - mp_fBrowseProb[i], fYearsPerTimestep);
    }

    //Format the query string
    //Do a type/species search on all the types and species
    p_bTypes = new bool[iNumTypes];
    for (i = 0; i < iNumTypes; i++) {
      p_bTypes[i] = false;
    }

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
      p_bTypes[mp_whatSpeciesTypeCombos[i].iType] = true;
    }
    strcat( cQueryTemp, "::type=" );
    for ( i = 0; i < iNumTypes; i++ )
    {
      if (p_bTypes[i]) {
        sprintf( cQueryPiece, "%d%s", i, "," );
        strcat( cQueryTemp, cQueryPiece );
      }
    }

    //Remove the last comma
    cQueryTemp[strlen( cQueryTemp ) - 1] = '\0';

    //Now put it in m_cQuery, sized correctly
    m_cQuery = new char[strlen( cQueryTemp ) + 1];
    strcpy( m_cQuery, cQueryTemp );
    delete[] cQueryTemp;
    delete[] p_bTypes;
    delete[] p_fTempValues;
  }
  catch ( modelErr & err )
  {
    delete[] cQueryTemp;
    delete[] p_bTypes;
    delete[] p_fTempValues;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] cQueryTemp;
    delete[] p_bTypes;
    delete[] p_fTempValues;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] cQueryTemp;
    delete[] p_bTypes;
    delete[] p_fTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clRandomBrowse::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clRandomBrowse::Action()
{
  float *p_fProbs = new float[m_iNumSpecies];
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    int iSp, iTp;
    bool bBrowsed;

    //If we need to do the probability random draw, do it now
    if (normal_pdf == m_iPDF) {
      for (iSp = 0; iSp < m_iNumSpecies; iSp++) {
        p_fProbs[iSp] = mp_fBrowseProb[iSp] +
          clModelMath::NormalRandomDraw(mp_fBrowseStdDev[iSp]);
        while (p_fProbs[iSp] < 0 || p_fProbs[iSp] > 1) {
          p_fProbs[iSp] = mp_fBrowseProb[iSp] +
            clModelMath::NormalRandomDraw(mp_fBrowseStdDev[iSp]);
        }
      }
    } else {
      //Use the same values every time
      for (iSp = 0; iSp < m_iNumSpecies; iSp++)
        p_fProbs[iSp] = mp_fBrowseProb[iSp];
    }

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the browsed data member
      if ( -1 != mp_iBrowsedCodes[iSp][iTp] )
      {
        if (clModelMath::GetRand() <= p_fProbs[iSp]) {
          bBrowsed = true;
        } else {
          bBrowsed = false;
        }

        p_oTree->SetValue(mp_iBrowsedCodes[iSp][iTp], bBrowsed);
      }

      p_oTree = p_oBehaviorTrees->NextTree();
    }

    delete[] p_fProbs;
  }
  catch ( modelErr & err )
  {
    delete[] p_fProbs;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fProbs;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fProbs;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clRandomBrowse::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clRandomBrowse::RegisterTreeDataMembers()
{

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int iNumTypes = p_oPop->GetNumberOfTypes(),
            i, j;

  m_iNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data member
  mp_iBrowsedCodes = new short int * [m_iNumSpecies];
  for ( i = 0; i < m_iNumSpecies; i++ )
  {
    mp_iBrowsedCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++)
      mp_iBrowsedCodes[i][j] = -1;
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Register the code and capture it
    mp_iBrowsedCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                    [mp_whatSpeciesTypeCombos[i].iType] =
      p_oPop->RegisterBool( "Browsed",
                            mp_whatSpeciesTypeCombos[i].iSpecies,
                            mp_whatSpeciesTypeCombos[i].iType );
  }
}
