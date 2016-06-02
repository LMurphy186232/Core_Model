//---------------------------------------------------------------------------
// EpiphyticEstablishment.cpp
//---------------------------------------------------------------------------
#include "EpiphyticEstablishment.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "ParsingFunctions.h"
#include "LightOrg.h"
#include "Plot.h"

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clEpiphyticEstablishment::clEpiphyticEstablishment( clSimManager * p_oSimManager ) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ),
     clLightBase( p_oSimManager), clGLIBase( p_oSimManager )
{
  try
  {
    m_sNameString = "EpiphyticEstablishment";
    m_sXMLRoot = "EpiphyticEstablishment";

    mp_iDeadCodes = NULL;
    mp_fA = NULL;
    mp_fB = NULL;
    mp_fC = NULL;
    mp_fM = NULL;
    mp_fN = NULL;
    m_cQuery = NULL;

    m_iNumTotalSpecies = 0;
    m_iSeedlingSpecies = -1;
    m_fMaxSearchDistance = 0;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;
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
    stcErr.sFunction = "clEpiphyticEstablishment::clEpiphyticEstablishment" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clEpiphyticEstablishment::~clEpiphyticEstablishment()
{
  int i;
  if ( mp_iDeadCodes )
    for ( i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iDeadCodes[i];
  delete[] mp_iDeadCodes;
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fC;
  delete[] mp_fM;
  delete[] mp_fN;
  delete[] m_cQuery;
}

///////////////////////////////////////////////////////////////////////////////
// GetParameterFileData
/////////////////////////////////////////////////////////////////////////////
void clEpiphyticEstablishment::GetParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  try {
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

  short int i;

  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  //Set up our temp array - pre-load with this behavior's species
  p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTemp[i].code = mp_iWhatSpecies[i];

  //Declare arrays
  mp_fA = new double[m_iNumTotalSpecies];
  mp_fB = new double[m_iNumTotalSpecies];
  mp_fC = new double[m_iNumTotalSpecies];
  mp_fM = new double[m_iNumTotalSpecies];
  mp_fN = new double[m_iNumTotalSpecies];

  //A
  FillSpeciesSpecificValue( p_oElement, "ep_epiphyticA", "ep_eaVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
  //Now transfer the values to the permanent array
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fA[p_fTemp[i].code] = p_fTemp[i].val;

  //B
  FillSpeciesSpecificValue( p_oElement, "ep_epiphyticB", "ep_ebVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fB[p_fTemp[i].code] = p_fTemp[i].val;

  //C
  FillSpeciesSpecificValue( p_oElement, "ep_epiphyticC", "ep_ecVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fC[p_fTemp[i].code] = p_fTemp[i].val;

  //M
  FillSpeciesSpecificValue( p_oElement, "ep_epiphyticM", "ep_emVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fM[p_fTemp[i].code] = p_fTemp[i].val;

  //N
  FillSpeciesSpecificValue( p_oElement, "ep_epiphyticN", "ep_enVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true );
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_fN[p_fTemp[i].code] = p_fTemp[i].val;

  //Seedling species
  FillSingleValue( p_oElement, "ep_epiphyticSeedlingSpecies", & m_iSeedlingSpecies, true );

  // Parameters used for GLI calculations

  FillSingleValue( p_oElement, "li_numAltGrids", & m_iNumAltAng, true );
  //Number of azimuth angles
  FillSingleValue( p_oElement, "li_numAziGrids", & m_iNumAziAng, true );
  //Minimum sun angle
  FillSingleValue( p_oElement, "li_minSunAngle", & m_fMinSunAngle, true );

  //Validation

  //Make sure the value for m_iNumAltAng and m_iNumAziAng is greater than 0
  if ( m_iNumAltAng < 1 || m_iNumAziAng < 1 )
  {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_DATA;
    stcErr.sFunction = "clEpiphyticEstablishment::GetParameterFileData" ;
    stcErr.sMoreInfo = "Number of altitude and azimuth sky divisions must be at least 1.";
    throw( stcErr );

  }

  delete[] p_fTemp;

  }
  catch ( modelErr & err )
  {
    delete[] p_fTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clEpiphyticEstablishment::GetParameterFileData" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clEpiphyticEstablishment::GetData( xercesc::DOMDocument * p_oDoc )
{
  //Make sure the light org object got a chance to do setup, which it
  //wouldn't have if this is the only light behavior around
  if ( mp_oLightOrg->GetMaxTreeHeight() <= 0 )
    mp_oLightOrg->DoSetup( mp_oSimManager, p_oDoc );

  GetParameterFileData( p_oDoc );
  GetTreeDataMemberCodes();
  DoLightSetup();
  FormatQueryString();
}

///////////////////////////////////////////////////////////////////////////////
// DoLightSetup
/////////////////////////////////////////////////////////////////////////////
void clEpiphyticEstablishment::DoLightSetup()
{
  try
  {
    float fAngChunk;
    int i, j, iHalfAzi; //for partitioning the sky hemisphere

    //Populate the sky brightness and photo arrays

    //Declare 'em
    mp_fBrightness = new float * [m_iNumAltAng];
    mp_fPhoto = new float * [m_iNumAltAng];
    for ( i = 0; i < m_iNumAltAng; i++ )
    {
      mp_fBrightness[i] = new float[m_iNumAziAng];
      mp_fPhoto[i] = new float[m_iNumAziAng];
      for ( j = 0; j < m_iNumAziAng; j++ )
        mp_fBrightness[i] [j] = 0;
    }

    m_fSinMinSunAng = sin( m_fMinSunAngle );
    m_iMinAngRow = (int)floor( m_fSinMinSunAng * m_iNumAltAng );

    PopulateGLIBrightnessArray();

    m_fAziChunkConverter = m_iNumAziAng / 360.0; //force float conversion
    m_fRcpTanMinAng = 1 / ( tan( m_fMinSunAngle ) );

    mp_fAziSlope = new float[m_iNumAziAng];
    iHalfAzi = m_iNumAziAng / 2;
    //Get the size of each azimuth chunk in radians
    fAngChunk = ( 2.0 * M_PI ) / m_iNumAziAng;
    for ( i = 0; i < iHalfAzi; i++ )
    {
      //slope = tan of azimuth angle
      mp_fAziSlope[i] = 1 / ( tan( fAngChunk * ( i + 0.5 ) ) );
      mp_fAziSlope[i + iHalfAzi] = mp_fAziSlope[i];
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
    stcErr.sFunction = "clEpiphyticEstablishment::DoLightSetup" ;
    throw( stcErr );
  }
}

///////////////////////////////////////////////////////////////////////////////
// GetGLI()
/////////////////////////////////////////////////////////////////////////////
float clEpiphyticEstablishment::GetGLI( clTreePopulation * p_oPop,
    const float & fX, const float & fY, const float &fHeight )
{
  clTreeSearch * p_oShaders; //neighborhood trees who can shade each target
  clTree * p_oNeighbor; //shading neighbors
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  clAllometry * p_oAllom = p_oPop->GetAllometryObject();
  char cQuery[75]; //format search strings into this
  float fGli, //global light index - end result of all this math
  fMaxSearchRad;
  int i, j;

  //Initialize photo array
  for ( i = 0; i < m_iNumAltAng; i++ )
    for ( j = 0; j < m_iNumAziAng; j++ )
      mp_fPhoto[i] [j] = 1.0;

  //Calculate search radius to look for shading neighbors
  fMaxSearchRad = (mp_oLightOrg->GetMaxTreeHeight() - fHeight) *
      m_fRcpTanMinAng + p_oAllom->GetMaxCrownRadius();

  //Get a list of all trees that are within the search radius and taller than
  //the fish-eye photo height
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", fMaxSearchRad, " FROM x=", fX, "y=", fY,
      "::height=", fHeight );
  p_oShaders = p_oPop->Find( cQuery );

  p_oNeighbor = p_oShaders->NextTree();
  while ( p_oNeighbor != NULL )
  {

    //Skip seedlings, which don't shade
    if ( clTreePopulation::seedling != p_oNeighbor->GetType() )

      //Add the effect of the neighbor to the simulated fisheye photo
      AddTreeToGliFishEye( fX, fY, fHeight, p_oNeighbor, p_oPlot, p_oPop, p_oAllom );

    p_oNeighbor = p_oShaders->NextTree();
  } //end of while (neighbor != NULL)

  //Calculate GLI
  fGli = 0.0;
  for ( i = 0; i < m_iNumAltAng; i++ )
    for ( j = 0; j < m_iNumAziAng; j++ )
      fGli += ( mp_fPhoto[i] [j] * mp_fBrightness[i] [j] );
  fGli *= 100;

  return fGli;
}


///////////////////////////////////////////////////////////////////////////////
// GetTreeDataMemberCodes()
/////////////////////////////////////////////////////////////////////////////
void clEpiphyticEstablishment::GetTreeDataMemberCodes()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  int i, j, iSp, iTp, iNumTypes = p_oPop->GetNumberOfTypes();

  mp_iDeadCodes = new short int * [m_iNumTotalSpecies];
  for ( i = 0; i < m_iNumTotalSpecies; i++ )
  {
    mp_iDeadCodes[i] = new short int[iNumTypes];
    for(j = 0; j < iNumTypes; j++)
      mp_iDeadCodes[i][j] = -1;
  }

  for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
    iSp = mp_whatSpeciesTypeCombos[i].iSpecies;
    iTp = mp_whatSpeciesTypeCombos[i].iType;

    if (clTreePopulation::adult != iTp && clTreePopulation::sapling != iTp) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clEpiphyticEstablishment::GetTreeDataMemberCodes" ;
      stcErr.sMoreInfo = "This behavior can only be applied to saplings and adults.";
      throw( stcErr );
    }

    mp_iDeadCodes[iSp][iTp] = p_oPop->GetIntDataCode("dead", iSp, iTp);
    if (-1 == mp_iDeadCodes[iSp][iTp]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clEpiphyticEstablishment::GetTreeDataMemberCodes" ;
      stcErr.sMoreInfo = "All trees must have a mortality behavior assigned.";
      throw( stcErr );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clEpiphyticEstablishment::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clAllometry *p_oAllom = p_oPop->GetAllometryObject();
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree, *p_oSeedling;

    //Linked list for holding our new seedlings
    struct newSeedling {
      float fX, fY, fHeight;
      newSeedling *nextSeedling;
    } *seedlings = NULL, *currentSeedling = NULL, *temp;

    float fX, fY, //coordinates for calculating GLI
         fGLI,
         fGLIHeight, //height at which to calculate GLI
         fHeight, fSeedlingHeight, //heights of substrate tree and seedling
         fSeedlingProb, //probability of a seedling on a dead substrate tree
         fCrownDepth; //crown depth of substrate tree
    int iDead,
        iSp, iTp;

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the dead data member
      if ( -1 != mp_iDeadCodes[iSp][iTp] )
      {
        //See if this tree is dead
        p_oTree->GetValue(mp_iDeadCodes[iSp][iTp], &iDead);
        if (iDead > notdead) {

          //Get the GLI halfway from the ground to the crown
          p_oTree->GetValue(p_oPop->GetHeightCode(iSp, iTp), &fHeight);
          if (iTp == clTreePopulation::sapling)
            fCrownDepth = p_oAllom->CalcSaplingCrownDepth(p_oTree);
          else
            fCrownDepth = p_oAllom->CalcAdultCrownDepth(p_oTree);

          fGLIHeight = (fHeight - fCrownDepth) / 2.0;
          p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
          p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

          fGLI = GetGLI(p_oPop, fX, fY, fGLIHeight);

          //Calculate the chance that there is a seedling
          fSeedlingProb  = 1 -
          (1/( 1 + exp(mp_fA[iSp] + mp_fB[iSp] * fHeight + mp_fC[iSp] * fGLI)));

          if (clModelMath::GetRand() <= fSeedlingProb) {

            //Yep - put a seedling there - get the height and go from cm to m
            fSeedlingHeight = (mp_fM[iSp] + mp_fN[iSp] * fHeight) / 100.0;
            //Put the seedling's particulars in a linked list
            temp = new newSeedling;
            temp->fX = fX;
            temp->fY = fY;
            temp->fHeight = fSeedlingHeight;
            temp->nextSeedling = NULL;
            if (NULL == seedlings) {
              seedlings = temp;
              currentSeedling = seedlings;
            }
            else {
              currentSeedling->nextSeedling = temp;
              currentSeedling = temp;
            }
          }
        }
      }
      p_oTree = p_oBehaviorTrees->NextTree();
    }

    //Now create all the seedlings
    currentSeedling = seedlings;
    while (currentSeedling != NULL) {
      p_oSeedling = p_oPop->CreateTree(currentSeedling->fX, currentSeedling->fY,
                         m_iSeedlingSpecies, clTreePopulation::seedling, 0);
              p_oSeedling->SetValue(
                  p_oPop->GetHeightCode(p_oSeedling->GetSpecies(),
                                        p_oSeedling->GetType()),
                  currentSeedling->fHeight, true, true);
      temp = currentSeedling->nextSeedling;
      delete currentSeedling;
      currentSeedling = temp;
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
    stcErr.sFunction = "clEpiphyticEstablishment::Action" ;
    throw( stcErr );
  }
}



////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clEpiphyticEstablishment::FormatQueryString()
{
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
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
