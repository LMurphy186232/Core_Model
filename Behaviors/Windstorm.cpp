//---------------------------------------------------------------------------
// Windstorm.cpp
//---------------------------------------------------------------------------
#include "Windstorm.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "ModelMath.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "TreeSearch.h"
#include "Plot.h"
#include <sstream>

#define WINDSTORMMIN 0.1

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clWindstorm::clWindstorm( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "Windstorm";
    m_sXMLRoot = "Windstorm";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 2;
    m_fMinimumVersionNumber = 2;

    mp_iDeadCodes = NULL;
    mp_fStormProbabilities = NULL;
    mp_fStormSeverities = NULL;
    mp_fA = NULL;
    mp_fB = NULL;
    mp_fC = NULL;
    mp_fMinDBH = NULL;
    mp_iBa_xCodes = NULL;
    mp_iDensity_xCodes = NULL;
    m_cQuery = NULL;

    m_iTotalNumSpecies = 0;
    m_iFirstStormTimestep = 0;
    m_fSSTPeriod = 0;
    m_fTrendInterceptI = 0;
    m_iSeverityCode = 0;
    m_fSineG = 0;
    mp_oResultsGrid = NULL;
    m_fSineF = 0;
    m_fTrendSlopeM = 0;
    m_fSineD = 0;

    //Hard-coded values
    m_iNumReturnIntervals = 11;
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
    stcErr.sFunction = "clWindstorm::clWindstorm" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clWindstorm::~clWindstorm()
{
  if (mp_iDeadCodes) {
    int i;
    for (i = 0; i < m_iTotalNumSpecies; i++)
      delete[] mp_iDeadCodes[i];
  }
  delete[] mp_iDeadCodes;
  delete[] mp_fStormProbabilities;
  delete[] mp_fStormSeverities;
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fC;
  delete[] mp_fMinDBH;
  delete[] mp_iBa_xCodes;
  delete[] mp_iDensity_xCodes;
  delete[] m_cQuery;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////

void clWindstorm::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clTreePopulation *p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
    m_iTotalNumSpecies = p_oPop->GetNumberOfSpecies();

    ReadParFile( p_oDoc, p_oPop );
    DoGridSetup();
    GetDeadCodes( p_oPop );
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
    stcErr.sFunction = "clWindstorm::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// ReadParFile()
////////////////////////////////////////////////////////////////////////////

void clWindstorm::ReadParFile( xercesc::DOMDocument * p_oDoc, clTreePopulation *p_oPop )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    std::stringstream sLabel;
    double fTemp;
    int iTemp;
    int i;

    //Declare our arrays
    mp_fA = new double[m_iTotalNumSpecies];
    mp_fB = new double[m_iTotalNumSpecies];
    mp_fC = new double[m_iTotalNumSpecies];
    mp_fMinDBH = new double[m_iTotalNumSpecies];
    mp_fStormProbabilities = new double[m_iNumReturnIntervals];
    mp_fStormSeverities = new double[m_iNumReturnIntervals];

    //Set up our doubleVal array that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Storm intercept for tree mortality (a)
    FillSpeciesSpecificValue( p_oElement, "ws_stmInterceptA", "ws_siaVal", p_fTempValues,
         m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fA[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Storm DBH exponent (b)
    FillSpeciesSpecificValue( p_oElement, "ws_stmDBHExpB", "ws_sdebVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Storm intensity coefficient (c)
    FillSpeciesSpecificValue( p_oElement, "ws_stmIntensCoeffC", "ws_sicVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fC[p_fTempValues[i].code] = p_fTempValues[i].val;

    //Minimum DBH to which storm damage applies
    FillSpeciesSpecificValue( p_oElement, "ws_minDBH", "ws_mdVal", p_fTempValues,
       m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer to the appropriate array buckets and make sure all values are
    //greater than 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fMinDBH[p_fTempValues[i].code] = p_fTempValues[i].val;
      if (mp_fMinDBH[p_fTempValues[i].code] < 0) {
        modelErr stcErr;
        stcErr.sFunction = "clWindstorm::ReadParFile" ;
        stcErr.sMoreInfo = "Minimum DBH for windstorms cannot be negative.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Timestep to start storms
    FillSingleValue( p_oElement, "ws_stmTSToStartStorms", &m_iFirstStormTimestep, true );
    if (m_iFirstStormTimestep < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clWindstorm::ReadParFile" ;
      stcErr.sMoreInfo = "Timestep to start windstorms cannot be negative.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Get the parameter file values for storm severity for each return interval
    //Make sure each value is between 0 and 1
    FillSingleValue( p_oElement, "ws_severityReturnInterval1", &fTemp, true );
    if (fTemp < 0 || fTemp > 1) {
      modelErr stcErr;
      stcErr.sFunction = "clWindstorm::ReadParFile" ;
      stcErr.sMoreInfo = "Windstorm severities must be between 0 and 1.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
    mp_fStormSeverities[0] = fTemp;

    iTemp = 5;
    for ( i = 1; i < m_iNumReturnIntervals; i++ )
    {
      sLabel << "ws_severityReturnInterval" << iTemp;
      FillSingleValue( p_oElement, sLabel.str(), &fTemp, true );
      sLabel.str("");
      //Validate that all values are greater than or equal to 0
      if (fTemp < 0 || fTemp > 1) {
        modelErr stcErr;
        stcErr.sFunction = "clWindstorm::ReadParFile" ;
        stcErr.sMoreInfo = "Windstorm severities must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      mp_fStormSeverities[i] = fTemp;
      iTemp *= 2;
    }

    //Calculate the probabilities for each return interval; any interval where
    //the severity is 0 has 0 probability
    if (mp_fStormSeverities[0] < 0.00001)
      mp_fStormProbabilities[0] = 0;
    else
      mp_fStormProbabilities[0] = 1;
    iTemp = 5;
    for ( i = 1; i < m_iNumReturnIntervals; i++ ) {
      if (mp_fStormSeverities[i] < 0.00001) {
        mp_fStormProbabilities[i] = 0;
      } else {
        mp_fStormProbabilities[i] = 1.0 / (float)iTemp;
      }
      iTemp *= 2;
    }

    //SST periodicity (Sr)
    FillSingleValue( p_oElement, "ws_stmSSTPeriod", &m_fSSTPeriod, true );
    if (fabs(m_fSSTPeriod - 0) < 0.001) {
      modelErr stcErr;
      stcErr.sFunction = "clWindstorm::ReadParFile" ;
      stcErr.sMoreInfo = "SST periodicity cannot be zero.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Sine function d
    FillSingleValue( p_oElement, "ws_stmSineD", &m_fSineD, true );

    //Sine function f
    FillSingleValue( p_oElement, "ws_stmSineF", &m_fSineF, true );

    //Sine function g
    FillSingleValue( p_oElement, "ws_stmSineG", &m_fSineG, true );

    //Trend function slope (m)
    FillSingleValue( p_oElement, "ws_stmTrendSlopeM", &m_fTrendSlopeM, true );

    //Trend function intercept (i)
    FillSingleValue( p_oElement, "ws_stmTrendInterceptI", &m_fTrendInterceptI, true );

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
    stcErr.sFunction = "clWindstorm::ReadParFile" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// DoGridSetup()
/////////////////////////////////////////////////////////////////////////////

void clWindstorm::DoGridSetup()
{
  clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
  std::stringstream sLabel;
  int i;

  mp_oResultsGrid = mp_oSimManager->CreateGrid( "Windstorm Results", //grid name
           0, //number of int data members
           0, //number of float data members
           0, //number of char data members
           0, //number of bool data members
           p_oPlot->GetXPlotLength(), //X cell length
           p_oPlot->GetYPlotLength() ); //Y cell length

  //Now adjust for the package data members
  mp_oResultsGrid->ChangePackageDataStructure( 0, //number of package int data members
        ( 2 * m_iTotalNumSpecies ) + 1, //number of package float data members
        0, //number of package char data members
        0 ); //number of package bool data members

  //Declare the arrays for holding data member codes
  mp_iBa_xCodes = new short int[m_iTotalNumSpecies];
  mp_iDensity_xCodes = new short int[m_iTotalNumSpecies];

  for ( i = 0; i < m_iTotalNumSpecies; i++ )
  {
    sLabel << "ba_" << i;
    mp_iBa_xCodes[i] = mp_oResultsGrid->RegisterPackageFloat(sLabel.str());
    sLabel.str("");
    sLabel << "density_" << i;
    mp_iDensity_xCodes[i] = mp_oResultsGrid->RegisterPackageFloat(sLabel.str());
    sLabel.str("");
  }

  m_iSeverityCode = mp_oResultsGrid->RegisterPackageFloat( "severity" );
}

/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clWindstorm::Action()
{
  float *p_fDeadBA = NULL,  //basal area killed per species
        *p_fDeadDen = NULL; //density killed per species
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees = NULL;
    clTree * p_oTree;
    clPackage *p_oStormPackage, *p_oNextPackage;
    float fDbh, //tree's dbh
          fMortProb, //probability of mortality
          fX, //for calculating effects of cyclicity
          fTrendBit, //trend effect on frequency
          fSineBit, //sinusoidal effect on frequency
          fCyclicity, //total cyclicity effect
          fPlotArea = mp_oSimManager->GetPlotObject()->GetPlotArea();
    int iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(),
        iCurrentTimestep = mp_oSimManager->GetCurrentTimestep(),
        iSp, iTp, i, j, iDead;

    p_fDeadBA = new float[m_iTotalNumSpecies]; //basal area killed per species
    p_fDeadDen = new float[m_iTotalNumSpecies]; //density killed per species

    //Start by cleaning up the windstorm results grid - delete all packages
    p_oStormPackage = mp_oResultsGrid->GetFirstPackageOfCell(0, 0);
    while (p_oStormPackage) {
      p_oNextPackage = p_oStormPackage->GetNextPackage();
      mp_oResultsGrid->DeletePackage(p_oStormPackage);
      p_oStormPackage = p_oNextPackage;
    }

    p_oStormPackage = NULL;

    //If it's not time to start storms yet, don't
    if (iCurrentTimestep < m_iFirstStormTimestep) {
      delete[] p_fDeadBA;
      delete[] p_fDeadDen;
      return;
    }

    //Calculate the effects of cyclicity
    fX = 4.0 * (iCurrentTimestep - m_iFirstStormTimestep) * iNumYearsPerTimestep / m_fSSTPeriod;
    fSineBit = m_fSineD * sin( M_PI * (fX - m_fSineG)/(2 * m_fSineF));
    fTrendBit = m_fTrendSlopeM * fX + m_fTrendInterceptI;
    fCyclicity = fSineBit + fTrendBit;

    //Figure out what storms happen
    for ( i = 0; i < m_iNumReturnIntervals; i++ )
    {
      for ( j = 0; j < iNumYearsPerTimestep; j++ )
      {
        if ( clModelMath::GetRand() <= mp_fStormProbabilities[i] * fCyclicity )
        {
          //A storm is happening - kill some trees
          for (iSp = 0; iSp < m_iTotalNumSpecies; iSp++) {
            p_fDeadBA[iSp] = 0;
            p_fDeadDen[iSp] = 0;
          }

          if (p_oBehaviorTrees) {
           p_oBehaviorTrees->StartOver();
          } else {
           p_oBehaviorTrees = p_oPop->Find( m_cQuery );
          }
          p_oTree = p_oBehaviorTrees->NextTree();
          while (p_oTree) {

            iSp = p_oTree->GetSpecies();
            iTp = p_oTree->GetType();

            //Double-check this tree's appropriateness by making sure we have a
            //non -1 return code for the dead data member
            if ( -1 == mp_iDeadCodes[iSp][iTp - clTreePopulation::sapling] )
              goto nextTree;

            //Make sure it's big enough to kill
            p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDbh );
            if ( fDbh < mp_fMinDBH[iSp] )
              goto nextTree;

            //Make sure it's not already dead
            p_oTree->GetValue(mp_iDeadCodes[iSp][iTp - clTreePopulation::sapling], &iDead);
            if (iDead > notdead)
              goto nextTree;

            //OK!  This tree might die in this storm.  Get its mortality
            //probability
            if (mp_fStormSeverities[i] < WINDSTORMMIN) {
              //Storm severity is below the minimum intensity
              fMortProb = mp_fStormSeverities[i];
            } else {
              fMortProb = mp_fA[iSp] + (mp_fC[iSp] * mp_fStormSeverities[i] *
                pow(fDbh, mp_fB[iSp]));

              if (fMortProb >= 10) fMortProb = 1.0;
              else if (fMortProb <= -10)  fMortProb = 0.0;
              else {
                fMortProb = exp(fMortProb);
                fMortProb = fMortProb / (1 + fMortProb);
              }
              if (clModelMath::GetRand() <= fMortProb) {
                //This tree dies
                iDead = storm;
                p_oTree->SetValue(mp_iDeadCodes[iSp][iTp - clTreePopulation::sapling], iDead);
                p_fDeadDen[iSp]++;
                p_fDeadBA[iSp] += clModelMath::CalculateBasalArea(fDbh);
              }
            }

            nextTree:
            p_oTree = p_oBehaviorTrees->NextTree();
          }

          //Create a package in the Windstorm Results grid with this storm's
          //stats
          if (NULL == p_oStormPackage) {
            p_oStormPackage = mp_oResultsGrid->CreatePackageAtPoint(0, 0);
          } else {
            p_oStormPackage = mp_oResultsGrid->CreatePackage(p_oStormPackage);
          }
          p_oStormPackage->SetValue(m_iSeverityCode, (float)mp_fStormSeverities[i]);
          for (iSp = 0; iSp < m_iTotalNumSpecies; iSp++) {
            p_oStormPackage->SetValue(mp_iBa_xCodes[iSp], p_fDeadBA[iSp] / fPlotArea);
            p_oStormPackage->SetValue(mp_iDensity_xCodes[iSp], p_fDeadDen[iSp] / fPlotArea);
          }
        }
      }
    }

    delete[] p_fDeadBA;
    delete[] p_fDeadDen;
  }
  catch ( modelErr & err )
  {
    delete[] p_fDeadBA;
    delete[] p_fDeadDen;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] p_fDeadBA;
    delete[] p_fDeadDen;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fDeadBA;
    delete[] p_fDeadDen;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clWindstorm::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clWindstorm::FormatQueryString(clTreePopulation *p_oPop)
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
// GetDeadCodes()
////////////////////////////////////////////////////////////////////////////
void clWindstorm::GetDeadCodes(clTreePopulation *p_oPop)
{
  int i, j;

  mp_iDeadCodes = new int * [m_iTotalNumSpecies];
  for ( i = 0; i < m_iTotalNumSpecies; i++ )
  {
    mp_iDeadCodes[i] = new int[2];
    for (j = 0; j < 2; j++) {
      mp_iDeadCodes[i][j] = -1;
    }
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Throw an error if this is a seedling
    if (clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clWindstorm::GetDeadCodes" ;
      stcErr.sMoreInfo = "Windstorm cannot be applied to seedlings.";
      throw( stcErr );
    }

    //Get the code
    mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                 [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
         p_oPop->GetIntDataCode( "dead", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
    if (-1 == mp_iDeadCodes[mp_whatSpeciesTypeCombos[i].iSpecies]
                           [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling]) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clWindstorm::GetDeadCodes" ;
      stcErr.sMoreInfo = "You must use tree mortality behaviors along with Windstorm.";
      throw( stcErr );
    }
  }
}
