//---------------------------------------------------------------------------
// VolumeCalculator.cpp
//---------------------------------------------------------------------------
#include "VolumeCalculator.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clVolumeCalculator::clVolumeCalculator( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "TreeVolumeCalculator";
    m_sXMLRoot = "TreeVolumeCalculator";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Indicate that this behavior intends to add one tree float data member
    m_iNewTreeFloats = 1;

    mp_fTaperA = NULL;
    mp_fTaperB = NULL;
    mp_fTaperC = NULL;
    mp_fTaperD = NULL;
    mp_fTaperF = NULL;
    mp_fTaperG = NULL;
    mp_fTaperI = NULL;
    mp_fTaperJ = NULL;
    mp_fTaperK = NULL;
    mp_fBarkA = NULL;
    mp_fBarkB = NULL;
    mp_fBarkC = NULL;
    mp_iIndexes = NULL;
    mp_iVolumeCodes = NULL;
    m_cQuery = NULL;

    m_fSegmentLength = 0;
    m_fStumpHeight = 0;
    m_fMinUsableDiam = 0;

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
    stcErr.sFunction = "clVolumeCalculator::clVolumeCalculator" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clVolumeCalculator::~clVolumeCalculator()
{
  delete[] m_cQuery;
  delete[] mp_fTaperA;
  delete[] mp_fTaperB;
  delete[] mp_fTaperC;
  delete[] mp_fTaperD;
  delete[] mp_fTaperF;
  delete[] mp_fTaperG;
  delete[] mp_fTaperI;
  delete[] mp_fTaperJ;
  delete[] mp_fTaperK;
  delete[] mp_fBarkA;
  delete[] mp_fBarkB;
  delete[] mp_fBarkC;
  delete[] mp_iIndexes;
  if ( mp_iVolumeCodes )
  {
    for ( int i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iVolumeCodes[i];
    }
  }
  delete[] mp_iVolumeCodes;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clVolumeCalculator::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    doubleVal * p_fTempValues; //for getting species-specific values
    int i;

    //Set up our doubleVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Declare our arrays
    mp_fTaperA = new double[m_iNumBehaviorSpecies];
    mp_fTaperB = new double[m_iNumBehaviorSpecies];
    mp_fTaperC = new double[m_iNumBehaviorSpecies];
    mp_fTaperD = new double[m_iNumBehaviorSpecies];
    mp_fTaperF = new double[m_iNumBehaviorSpecies];
    mp_fTaperG = new double[m_iNumBehaviorSpecies];
    mp_fTaperI = new double[m_iNumBehaviorSpecies];
    mp_fTaperJ = new double[m_iNumBehaviorSpecies];
    mp_fTaperK = new double[m_iNumBehaviorSpecies];
    mp_fBarkA = new double[m_iNumBehaviorSpecies];
    mp_fBarkB = new double[m_iNumBehaviorSpecies];
    mp_fBarkC = new double[m_iNumBehaviorSpecies];

    //Get the parameter file values

    //a in the taper equation (also known as a0)
    FillSpeciesSpecificValue( p_oElement, "vo_taperA", "vo_taVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperA[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //b in the taper equation (also known as a1)
    FillSpeciesSpecificValue( p_oElement, "vo_taperB", "vo_tbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //c in the taper equation (also known as a2)
    FillSpeciesSpecificValue( p_oElement, "vo_taperC", "vo_tcVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperC[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //d in the taper equation (also known as b1)
    FillSpeciesSpecificValue( p_oElement, "vo_taperD", "vo_tdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperD[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //f in the taper equation (also known as b2)
    FillSpeciesSpecificValue( p_oElement, "vo_taperF", "vo_tfVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperF[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //g in the taper equation (also known as b3)
    FillSpeciesSpecificValue( p_oElement, "vo_taperG", "vo_tgVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperG[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //i in the taper equation (also known as b4)
    FillSpeciesSpecificValue( p_oElement, "vo_taperI", "vo_tiVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperI[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //j in the taper equation (also known as b5)
    FillSpeciesSpecificValue( p_oElement, "vo_taperJ", "vo_tjVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperJ[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //k in the taper equation (also known as b6)
    FillSpeciesSpecificValue( p_oElement, "vo_taperK", "vo_tkVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fTaperK[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //a in the math library's AddBarkToDBH() function (also known as a1)
    FillSpeciesSpecificValue( p_oElement, "vo_barkA", "vo_baVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBarkA[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //b in the math library's AddBarkToDBH() function (also known as a2)
    FillSpeciesSpecificValue( p_oElement, "vo_barkB", "vo_bbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBarkB[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //c in the math library's AddBarkToDBH() function (also known as a3)
    FillSpeciesSpecificValue( p_oElement, "vo_barkC", "vo_bcVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fBarkC[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;

    //Stump height, in cm
    FillSingleValue( p_oElement, "vo_stumpHeight", & m_fStumpHeight, true );
    //Make sure it's greater than 0
    if ( m_fStumpHeight < 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clVolumeCalculator::GetData" ;
      stcErr.sMoreInfo = "Stump height cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      delete[] p_fTempValues;
      throw( stcErr );
    }
    m_fStumpHeight *= 0.01; //convert from cm to m

    //Minimum usable diameter, in cm
    FillSingleValue( p_oElement, "vo_minUsableDiam", & m_fMinUsableDiam, true );
    //Make sure it's greater than 0
    if ( m_fMinUsableDiam < 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clVolumeCalculator::GetData" ;
      stcErr.sMoreInfo = "Minimum usable diameter cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      delete[] p_fTempValues;
      throw( stcErr );
    }

    //Length of tree trunk volume segments, in m
    FillSingleValue( p_oElement, "vo_segmentLength", & m_fSegmentLength, true );
    //Make sure it's greater than 0
    if ( m_fSegmentLength <= 0 )
    {
      modelErr stcErr;
      stcErr.sFunction = "clVolumeCalculator::GetData" ;
      stcErr.sMoreInfo = "Segment length cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      delete[] p_fTempValues;
      throw( stcErr );
    }

    FormatQueryString();
    Action();

    delete[] p_fTempValues;
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
    stcErr.sFunction = "clVolumeCalculator::GetData" ;
    throw( stcErr );
  }
}


////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clVolumeCalculator::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    float fVolume, //tree's volume
          fDBH, //tree's dbh
          fTreeHeight; //tree's height
    int iSp, iTp;

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find( m_cQuery );

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the volume data member
      if ( -1 != mp_iVolumeCodes[mp_iIndexes[iSp]][iTp - clTreePopulation::sapling] ) {

        //Get this tree's volume and set it into the tree
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDBH );
        p_oTree->GetValue(p_oPop->GetHeightCode( iSp, iTp ), &fTreeHeight);
        fVolume = GetTreeVolume(fTreeHeight, fDBH, iSp);
        p_oTree->SetValue( mp_iVolumeCodes[mp_iIndexes[iSp]][iTp - clTreePopulation::sapling], fVolume);
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
    stcErr.sFunction = "clVolumeCalculator::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clVolumeCalculator::FormatQueryString()
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

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clVolumeCalculator::RegisterTreeDataMembers()
{

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int i;

  mp_iIndexes = new short int[p_oPop->GetNumberOfSpecies()];

  //Make the list of indexes
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_iIndexes[mp_iWhatSpecies[i]] = i;

  //Declare the array and register our new data member
  mp_iVolumeCodes = new short int * [m_iNumBehaviorSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
  {
    mp_iVolumeCodes[i] = new short int[3];
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Make sure that only allowed types have been applied
    if ( clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::snag != mp_whatSpeciesTypeCombos[i].iType )
         {

           modelErr stcErr;
           stcErr.sFunction = "clVolumeCalculator::RegisterTreeDataMembers" ;
           stcErr.sMoreInfo = "This behavior can only be applied to saplings, adults, and snags.";
           stcErr.iErrorCode = BAD_DATA;
           throw( stcErr );
    }

    //Register the code and capture it
    mp_iVolumeCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
         [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
         p_oPop->RegisterFloat( "Volume", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}

////////////////////////////////////////////////////////////////////////////
// GetTreeVolume()
////////////////////////////////////////////////////////////////////////////
float clVolumeCalculator::GetTreeVolume( const float & fTreeHeight, const float & fDBH, const int & iSpecies )
{

  float fX, fP, fQ, fZ, fTaper1, //taper equation first term
       fBeginHeight, //height at bottom of segment
       fEndHeight, //height at end of segment
       fBeginDiam, //diameter inside bark at beginning of segment
       fEndDiam, //diameter inside bark at end of segment
       fOneThird = 0.333333,
       fXExp1, //Xi exponent pieces that we can calculate ahead of time
       fXExp2, //Xi exponent pieces that must be calculated each segment
       fSegmentLength = m_fSegmentLength, //actual segment length of piece to calculate
       fOBDBH, //dbh outside bark
       fVolume = 0; //total volume
  int iNumSegments = (int)ceil( fTreeHeight / m_fSegmentLength ), //# of trunk segments
       iSpInd = mp_iIndexes[iSpecies], //index of this species in arrays
       i;

  if (fTreeHeight <= m_fStumpHeight) return fVolume;

  //Calculate values that only need to be calculated once
  fP = 1.3 / fTreeHeight;
  fOBDBH = clModelMath::AddBarkToDBH( fDBH, mp_fBarkA[iSpInd], mp_fBarkB[iSpInd], mp_fBarkC[iSpInd] );
  fXExp1 = mp_fTaperF[iSpInd] * ( 1 / exp( fOBDBH / fTreeHeight ) ) + mp_fTaperI[iSpInd] * ( 1 / fOBDBH );
  fTaper1 = mp_fTaperA[iSpInd] * pow( fOBDBH, mp_fTaperB[iSpInd] ) * pow( fTreeHeight, mp_fTaperC[iSpInd] );

  //Set up the segment starting point for the first segment
  fBeginHeight = m_fStumpHeight;
  fZ = fBeginHeight / fTreeHeight;
  fQ = 1 - pow( fZ, fOneThird );
  fX = fQ / ( 1 - pow( fP, fOneThird ));
  fXExp2 = mp_fTaperD[iSpInd] * pow( fZ, 4 ) +
           mp_fTaperG[iSpInd] * pow( fX, 0.1 ) +
           mp_fTaperJ[iSpInd] * pow( fTreeHeight, fQ ) +
           mp_fTaperK[iSpInd] * fX;
  fBeginDiam = fTaper1 * pow( fX, fXExp1 + fXExp2 );

  //Loop over the segments
  for ( i = 0; i < iNumSegments; i++ )
  {

    //Get the ending height
    fEndHeight = fBeginHeight + m_fSegmentLength;

    if ( fEndHeight >= fTreeHeight ) {

      //The end of the segment will be exactly the end of the tree height -
      //adjust accordingly.  The diameter at the very end of the tree is 0.
      fEndHeight = fTreeHeight;
      fSegmentLength = fEndHeight - fBeginHeight;
      fEndDiam = 0;
    }
    else {

      //Get the ending diameter inside the bark
      fZ = fEndHeight / fTreeHeight;
      fQ = 1 - pow( fZ, fOneThird );
      fX = fQ / ( 1 - pow( fP, fOneThird ));
      fXExp2 = mp_fTaperD[iSpInd] * pow( fZ, 4 ) +
               mp_fTaperG[iSpInd] * pow( fX, 0.1 ) +
               mp_fTaperJ[iSpInd] * pow( fTreeHeight, fQ ) +
               mp_fTaperK[iSpInd] * fX;
      fEndDiam = fTaper1 * pow( fX, fXExp1 + fXExp2 );
    }

    //Make sure that the ending diameter is bigger than the minimum
    if (fEndDiam < m_fMinUsableDiam) break;

    //Calculate the volume and add it in - convert from cm to m on the diams
    //Multiplying diams by 0.005 is like dividing by 2 (to make a radius) and
    //then dividing by 100 (to get to m from cm)
    fVolume += (M_PI * (pow(fBeginDiam * 0.005, 2) + pow(fEndDiam * 0.005, 2))
                * 0.5 * fSegmentLength);

    //Set up for the next segment
    fBeginHeight = fEndHeight;
    fBeginDiam = fEndDiam;
  }

  return fVolume;
}
