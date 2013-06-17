//---------------------------------------------------------------------------
// InsectInfestation.cpp
//---------------------------------------------------------------------------
#include "InsectInfestation.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "ModelMath.h"
#include "TreePopulation.h"
#include "Plot.h"

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clInsectInfestation::clInsectInfestation( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "InsectInfestation";
    m_sXMLRoot = "InsectInfestation";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_iDataCodes = NULL;
    mp_fIntercept = NULL;
    mp_fMax = NULL;
    mp_fX0 = NULL;
    mp_fXb = NULL;
    mp_fMinDBH = NULL;
    m_cQuery = NULL;
    m_iYearsOfInfestation = 0;
    m_iFirstTimestep = 0;
    m_iTotalNumSpecies = 0;

    m_iNewTreeInts = 1;
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
    stcErr.sFunction = "clInsectInfestation::clInsectInfestation" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clInsectInfestation::~clInsectInfestation()
{
  if (mp_iDataCodes) {
    int i;
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_iDataCodes[i];
  }
  delete[] mp_iDataCodes;
  delete[] mp_fIntercept;
  delete[] mp_fMax;
  delete[] mp_fX0;
  delete[] mp_fXb;
  delete[] mp_fMinDBH;
  delete[] m_cQuery;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////

void clInsectInfestation::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation *p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");

    m_iYearsOfInfestation = 0;
    ReadParFile( p_oDoc, p_oPop );
    FormatQueryString( p_oPop );
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
    stcErr.sFunction = "clInsectInfestation::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// ReadParFile()
////////////////////////////////////////////////////////////////////////////

void clInsectInfestation::ReadParFile( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  floatVal * p_fTempValues = NULL; //for getting species-specific values
  try {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    int i;

    //Declare our arrays
    mp_fIntercept = new float[m_iTotalNumSpecies];
    mp_fMax = new float[m_iTotalNumSpecies];
    mp_fX0 = new float[m_iTotalNumSpecies];
    mp_fXb = new float[m_iTotalNumSpecies];
    mp_fMinDBH = new float[m_iTotalNumSpecies];

    //Set up our floatVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new floatVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Intercept parameter - rate of infestation at time 1
    FillSpeciesSpecificValue( p_oElement, "di_insectIntercept", "di_iiVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //between 0 and 1
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fIntercept[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fIntercept[p_fTempValues[i].code] < 0 ||
          mp_fIntercept[p_fTempValues[i].code] > 1) {
        modelErr stcErr;
        stcErr.sFunction = "clInsectInfestation::ReadParFile" ;
        stcErr.sMoreInfo = "Intercept value must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Maximum infestation rate
    FillSpeciesSpecificValue( p_oElement, "di_insectMaxInfestation",
        "di_imiVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //between 0 and 1
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fMax[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fMax[p_fTempValues[i].code] < 0 ||
          mp_fMax[p_fTempValues[i].code] > 1) {
        modelErr stcErr;
        stcErr.sFunction = "clInsectInfestation::ReadParFile" ;
        stcErr.sMoreInfo = "Max infestation value must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //X0
    FillSpeciesSpecificValue( p_oElement, "di_insectX0", "di_ix0Val",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //not 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fX0[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fX0[p_fTempValues[i].code] == 0) {
        modelErr stcErr;
        stcErr.sFunction = "clInsectInfestation::ReadParFile" ;
        stcErr.sMoreInfo = "X0 cannot be 0.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Xb
    FillSpeciesSpecificValue( p_oElement, "di_insectXb", "di_ixbVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fXb[p_fTempValues[i].code] = p_fTempValues[i].val;
    }

    //Minimum DBH to infest
    FillSpeciesSpecificValue( p_oElement, "di_insectMinDBH", "di_imdVal",
        p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //greater than 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fMinDBH[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fMinDBH[p_fTempValues[i].code] < 0) {
        modelErr stcErr;
        stcErr.sFunction = "clInsectInfestation::ReadParFile" ;
        stcErr.sMoreInfo = "Minimum DBH for insect infestation cannot be negative.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Timestep to start infestation
    FillSingleValue( p_oElement, "di_insectStartTimestep", &m_iFirstTimestep, true );
    if (m_iFirstTimestep < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clInsectInfestation::ReadParFile" ;
      stcErr.sMoreInfo = "Timestep to start infestation cannot be negative.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    delete[] p_fTempValues;
  }
  catch ( modelErr & err )
  {
    delete[] p_fTempValues;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTempValues;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clInsectInfestation::ReadParFile" ;
    throw( stcErr );
  }
}


/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clInsectInfestation::Action()
{
  if (m_iFirstTimestep > mp_oSimManager->GetCurrentTimestep()) return;
  if (m_iFirstTimestep < mp_oSimManager->GetCurrentTimestep() &&
      0 == m_iYearsOfInfestation) return;

  long *p_iTotalTrees = new long[m_iTotalNumSpecies], //total eligible trees
       *p_iInfTrees = new long[m_iTotalNumSpecies]; //number infested trees
  float *p_fTargetRate = new float[m_iTotalNumSpecies],//target infestation rate
        *p_fNewProb = new float[m_iTotalNumSpecies];//prob of getting infested
                                                    //this timestep
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch *p_oBehaviorTrees = p_oPop->Find( m_cQuery );
    clTree * p_oTree = p_oBehaviorTrees->NextTree();
    float fDbh;
    long iTot = 0;
    int i, iSp, iTp, iInf,
          iNumYrsTimestep = (int)mp_oSimManager->GetNumberOfYearsPerTimestep();

    if (m_iFirstTimestep < mp_oSimManager->GetCurrentTimestep()) {


      //Get the current rate of infestation
      GetInfestationRate(p_iTotalTrees, p_iInfTrees);

      //Check to make sure there are some infested trees - if not, call a halt
      for (i = 0; i < m_iTotalNumSpecies; i++) iTot += p_iInfTrees[i];
      if (0 == iTot) {
        m_iYearsOfInfestation = 0;
        delete[] p_iInfTrees;
        delete[] p_iTotalTrees;
        delete[] p_fTargetRate;
        delete[] p_fNewProb;
        return;
      }

      //Figure out current infestation rate and probability an ininfested tree
      //will be infested this timestep
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        iSp = mp_iWhatSpecies[i];
        p_fTargetRate[iSp] = mp_fIntercept[iSp] +
                 ((mp_fMax[iSp] - mp_fIntercept[iSp])/
                     (1+pow(m_iYearsOfInfestation / mp_fX0[iSp], mp_fXb[iSp])));
        p_fNewProb[iSp] = ((p_fTargetRate[iSp] * p_iTotalTrees[iSp])
                 - p_iInfTrees[iSp])/ //Number we need to infest this time step
                 (p_iTotalTrees[iSp]- p_iInfTrees[iSp]); //Number of uninfested
        if (p_fNewProb[iSp] < 0) p_fNewProb[iSp] = 0;
      }
    } else {
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        iSp = mp_iWhatSpecies[i];
        p_fNewProb[iSp] = mp_fIntercept[iSp];
      }
    }

    //Go through trees - increment counters and assess for new infestation
    while (p_oTree) {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      if (-1 < mp_iDataCodes[iSp][iTp]) {
        p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
        if (fDbh >= mp_fMinDBH[iSp]) {
          p_oTree->GetValue(mp_iDataCodes[iSp][iTp], &iInf);
          if (iInf > 0) {
            iInf += iNumYrsTimestep;
            p_oTree->SetValue(mp_iDataCodes[iSp][iTp], iInf);
          } else {
            if (clModelMath::GetRand() <= p_fNewProb[iSp]) {
              iInf = iNumYrsTimestep;
              p_oTree->SetValue(mp_iDataCodes[iSp][iTp], iInf);
            }
          }
        }
    }
      p_oTree = p_oBehaviorTrees->NextTree();
    }

    m_iYearsOfInfestation += iNumYrsTimestep;

    delete[] p_iInfTrees;
    delete[] p_iTotalTrees;
    delete[] p_fTargetRate;
    delete[] p_fNewProb;
  }
  catch ( modelErr & err )
  {
    delete[] p_iInfTrees;
    delete[] p_iTotalTrees;
    delete[] p_fTargetRate;
    delete[] p_fNewProb;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_iInfTrees;
    delete[] p_iTotalTrees;
    delete[] p_fTargetRate;
    delete[] p_fNewProb;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_iInfTrees;
    delete[] p_iTotalTrees;
    delete[] p_fTargetRate;
    delete[] p_fNewProb;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clInsectInfestation::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clInsectInfestation::FormatQueryString(clTreePopulation *p_oPop)
{
  char * cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];
  char cQueryPiece[5]; //for assembling the search query
  int i;
  bool bSapling = false, bAdult = false;

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

  //Remove the last comma
  cQueryTemp[strlen( cQueryTemp ) - 1] = '\0';

  //Now put it in m_cQuery, sized correctly
  m_cQuery = new char[strlen( cQueryTemp ) + 1];
  strcpy( m_cQuery, cQueryTemp );
  delete[] cQueryTemp;
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clInsectInfestation::RegisterTreeDataMembers()
{

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int i, j,
    iNumTypes = p_oPop->GetNumberOfTypes();

  m_iTotalNumSpecies = p_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data member
  mp_iDataCodes = new short int * [m_iTotalNumSpecies];
  for ( i = 0; i < m_iTotalNumSpecies; i++ )
  {
    mp_iDataCodes[i] = new short int[iNumTypes];
    for (j = 0; j < iNumTypes; j++)
      mp_iDataCodes[i][j] = -1;
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Make sure that only allowed types have been applied
    if ( clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::snag != mp_whatSpeciesTypeCombos[i].iType )
         {

           modelErr stcErr;
           stcErr.sFunction = "clInsectInfestation::RegisterTreeDataMembers" ;
           stcErr.sMoreInfo = "This behavior can only be applied to saplings, adults, and snags.";
           stcErr.iErrorCode = BAD_DATA;
           throw( stcErr );
    }

    //Register the code and capture it
    mp_iDataCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                 [mp_whatSpeciesTypeCombos[i].iType] =
         p_oPop->RegisterInt("YearsInfested", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}


////////////////////////////////////////////////////////////////////////////
// GetInfestationRate()
////////////////////////////////////////////////////////////////////////////
void clInsectInfestation::GetInfestationRate(long *p_iTotalTrees, long *p_iInfTrees) {
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTreeSearch *p_oBehaviorTrees = p_oPop->Find( m_cQuery );
  clTree * p_oTree = p_oBehaviorTrees->NextTree();
  float fDbh;
  int i, iSp, iTp, iInf;

  //Zero out counting arrays
  for (i = 0; i < m_iTotalNumSpecies; i++) {
    p_iInfTrees[i] = 0;
    p_iTotalTrees[i] = 0;
  }

  while (p_oTree) {
    iSp = p_oTree->GetSpecies();
    iTp = p_oTree->GetType();

    if (-1 < mp_iDataCodes[iSp][iTp]) {
      p_oTree->GetValue(p_oPop->GetDbhCode(iSp, iTp), &fDbh);
      if (fDbh >= mp_fMinDBH[iSp]) {
        p_iTotalTrees[iSp]++;
        p_oTree->GetValue(mp_iDataCodes[iSp][iTp], &iInf);
        if (iInf > 0)
          p_iInfTrees[iSp]++;
      }
    }
    p_oTree = p_oBehaviorTrees->NextTree();
  }
}
