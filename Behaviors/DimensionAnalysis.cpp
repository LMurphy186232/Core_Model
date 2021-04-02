//---------------------------------------------------------------------------
// DimensionAnalysis.cpp
//---------------------------------------------------------------------------
#include "DimensionAnalysis.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Constants.h"
#include <math.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor()
////////////////////////////////////////////////////////////////////////////
clDimensionAnalysis::clDimensionAnalysis( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "DimensionAnalysis";
    m_sXMLRoot = "DimensionAnalysis";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 2.1;
    m_fMinimumVersionNumber = 1;

    //Indicate that this behavior intends to add one tree float data member
    m_iNewTreeFloats = 1;

    mp_fA = NULL;
    mp_fB = NULL;
    mp_fC = NULL;
    mp_fD = NULL;
    mp_fE = NULL;
    mp_fDbhConverter = NULL;
    mp_fBiomassConverter = NULL;
    mp_iEquationID = NULL;
    mp_iBiomassCodes = NULL;
    mp_iIndexes = NULL;
    mp_fCorrectionFactor = NULL;
    mp_bUseCorrectionFactor = NULL;
    mp_bConvertDBH = NULL;
    mp_iWhatDia = NULL;
    m_sQuery = "";

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
    stcErr.sFunction = "clDimensionAnalysis::clDimensionAnalysis" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor()
////////////////////////////////////////////////////////////////////////////
clDimensionAnalysis::~clDimensionAnalysis()
{
  delete[] mp_fA;
  delete[] mp_fB;
  delete[] mp_fC;
  delete[] mp_fD;
  delete[] mp_fE;
  delete[] mp_fDbhConverter;
  delete[] mp_fBiomassConverter;
  delete[] mp_iIndexes;
  delete[] mp_fCorrectionFactor;
  delete[] mp_bUseCorrectionFactor;
  delete[] mp_bConvertDBH;
  delete[] mp_iEquationID;
  delete[] mp_iWhatDia;

  if ( mp_iBiomassCodes )
  {
    for ( int i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      delete[] mp_iBiomassCodes[i];
    }
  }
  delete[] mp_iBiomassCodes;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clDimensionAnalysis::GetData( xercesc::DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  intVal * p_iTempValues = NULL; //for getting species-specific values
  boolVal * p_bTempValues = NULL; //for getting species-specific values
  std::stringstream sQueryTemp;
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    float fConvertGramsToMg = CONVERT_G_TO_KG * CONVERT_KG_TO_MG,
          fConvertLbsToMg = CONVERT_LBS_TO_KG * CONVERT_KG_TO_MG;
    int i;
    bool bSapling = false, bAdult = false, bSnag = false;

    //*************************
    // Read in parameters
    //*************************

    //Set up our arrays that will extract values only for the species
    //assigned to this behavior
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];
    p_iTempValues = new intVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_iTempValues[i].code = mp_iWhatSpecies[i];
    p_bTempValues = new boolVal[m_iNumBehaviorSpecies];
      for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
        p_bTempValues[i].code = mp_iWhatSpecies[i];

    //Declare our arrays
    mp_fA = new double[m_iNumBehaviorSpecies];
    mp_fB = new double[m_iNumBehaviorSpecies];
    mp_fC = new double[m_iNumBehaviorSpecies];
    mp_fD = new double[m_iNumBehaviorSpecies];
    mp_fE = new double[m_iNumBehaviorSpecies];
    mp_fCorrectionFactor = new double[m_iNumBehaviorSpecies];
    mp_fDbhConverter = new double[m_iNumBehaviorSpecies];
    mp_fBiomassConverter = new double[m_iNumBehaviorSpecies];
    mp_iEquationID = new int[m_iNumBehaviorSpecies];
    mp_iWhatDia = new int[m_iNumBehaviorSpecies];
    mp_bUseCorrectionFactor = new bool[m_iNumBehaviorSpecies];
    mp_bConvertDBH = new bool[m_iNumBehaviorSpecies];

    //Get the parameter file values

    //a in the biomass equation
    FillSpeciesSpecificValue( p_oElement, "bi_a", "bi_aVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fA[i] = p_fTempValues[i].val;

    //b in the biomass equation
    FillSpeciesSpecificValue( p_oElement, "bi_b", "bi_bVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fB[i] = p_fTempValues[i].val;

    //c in the biomass equation
    FillSpeciesSpecificValue( p_oElement, "bi_c", "bi_cVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fC[i] = p_fTempValues[i].val;

    //d in the biomass equation
    FillSpeciesSpecificValue( p_oElement, "bi_d", "bi_dVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fD[i] = p_fTempValues[i].val;

    //e in the biomass equation
    FillSpeciesSpecificValue( p_oElement, "bi_e", "bi_eVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fE[i] = p_fTempValues[i].val;

    //Use correction factor
    FillSpeciesSpecificValue( p_oElement, "bi_useCorrectionFactor", "bi_ucfVal", p_bTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_bUseCorrectionFactor[i] = p_bTempValues[i].val;

    //Correction factor
    FillSpeciesSpecificValue( p_oElement, "bi_correctionFactorValue", "bi_cfvVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fCorrectionFactor[i] = p_fTempValues[i].val;

    //Equation ID
    FillSpeciesSpecificValue( p_oElement, "bi_eqID", "bi_eiVal", p_iTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iEquationID[i] = p_iTempValues[i].val;

    //Make sure the equation ID is between 1 and 9
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (mp_iEquationID[i] < 1 || mp_iEquationID[i] > 9 ) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDimensionAnalysis::GetData";
        std::stringstream s;
        s << "Unidentified equation ID \"" << mp_iEquationID[i]
          << "\" for species \"" << mp_iWhatSpecies[i] << "\".";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //Meaning of "dia"
    FillSpeciesSpecificValue( p_oElement, "bi_whatDia", "bi_wdVal", p_iTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iWhatDia[i] = p_iTempValues[i].val;

    //Make sure the value is valid
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (mp_iWhatDia[i] < DBH || mp_iWhatDia[i] > DBH2 ) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDimensionAnalysis::GetData";
        std::stringstream s;
        s << "Unidentified dia meaning \"" << mp_iWhatDia[i]
          << "\" for species \"" << mp_iWhatSpecies[i] << "\".";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //DBH units
    FillSpeciesSpecificValue( p_oElement, "bi_dbhUnits", "bi_duVal", p_iTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Go through and assign the appropriate correction factor for each
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (mm == p_iTempValues[i].val) {
        mp_fDbhConverter[i] = CONVERT_CM_TO_MM;
        mp_bConvertDBH[i] = true;
      }
      else if (cm == p_iTempValues[i].val) {
        mp_fDbhConverter[i] = 1;
        mp_bConvertDBH[i] = false;
      }
      else if (in == p_iTempValues[i].val) {
        mp_fDbhConverter[i] = CONVERT_CM_TO_IN;
        mp_bConvertDBH[i] = true;
      }
      else {
        //Unrecognized value - throw an error
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDimensionAnalysis::GetData";
        std::stringstream s;
        s << "Unidentified DBH units code \"" << p_iTempValues[i].val
          << "\" for species \"" << mp_iWhatSpecies[i] << "\".";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }

    //Biomass units
    FillSpeciesSpecificValue( p_oElement, "bi_biomassUnits", "bi_buVal", p_iTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Go through and assign the appropriate correction factor for each
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (g == p_iTempValues[i].val) {
        mp_fBiomassConverter[i] = fConvertGramsToMg;
      }
      else if (kg == p_iTempValues[i].val) {
        mp_fBiomassConverter[i] = CONVERT_KG_TO_MG;
      }
      else if (lb == p_iTempValues[i].val) {
        mp_fBiomassConverter[i] = fConvertLbsToMg;
      }
      else {
        //Unrecognized value - throw an error
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDimensionAnalysis::GetData";
        std::stringstream s;
        s << "Unidentified biomass units code \"" << p_iTempValues[i].val
          << "\" for species \"" << mp_iWhatSpecies[i] << "\".";
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
    }


    //*************************
    // Format query string
    //*************************
    //Do a type/species search on all the types and species
    sQueryTemp << "species=";
    for (i = 0; i < m_iNumBehaviorSpecies - 1; i++) {
      sQueryTemp << mp_iWhatSpecies[i] << ",";
    }
    sQueryTemp << mp_iWhatSpecies[m_iNumBehaviorSpecies - 1];

    //Find all the types
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType ) {
        bSapling = true;
      } else if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType ) {
        bAdult = true;
      } else if ( clTreePopulation::snag == mp_whatSpeciesTypeCombos[i].iType ) {
        bSnag = true;
      }
    }
    sQueryTemp << "::type=";
    if (bSapling) {
      sQueryTemp << clTreePopulation::sapling << ",";
    } if (bAdult) {
      sQueryTemp << clTreePopulation::adult << ",";
    } if (bSnag) {
      sQueryTemp << clTreePopulation::snag << ",";
    }

    //Remove the last comma and put it in m_sQuery
    m_sQuery = sQueryTemp.str().substr(0, sQueryTemp.str().length() - 1);

    delete[] p_fTempValues;
    delete[] p_iTempValues;
    delete[] p_bTempValues;

    Action();
  }
  catch ( modelErr & err )
  {
    delete[] p_fTempValues;
    delete[] p_iTempValues;
    delete[] p_bTempValues;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] p_fTempValues;
    delete[] p_iTempValues;
    delete[] p_bTempValues;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDimensionAnalysis::GetData" ;
    throw( stcErr );
  }
}



////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clDimensionAnalysis::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oBehaviorTrees;
    clTree * p_oTree;
    float fBiomass, //tree's biomass
         fDBH; //tree's dbh
    int iSp, iTp;

    //Get the trees that apply to this behavior
    p_oBehaviorTrees = p_oPop->Find(m_sQuery);

    p_oTree = p_oBehaviorTrees->NextTree();
    while ( p_oTree )
    {

      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      //Double-check this tree's appropriateness by making sure we have a
      //non -1 return code for the volume data member
      if ( -1 != mp_iBiomassCodes[mp_iIndexes[iSp]] [iTp - clTreePopulation::sapling] )
      {

        //Get this tree's DBH
        p_oTree->GetValue( p_oPop->GetDbhCode( iSp, iTp ), & fDBH );
        //Convert the DBH to the appropriate units
        if (mp_bConvertDBH[mp_iIndexes[iSp]]) {
          fDBH *= mp_fDbhConverter[mp_iIndexes[iSp]];
        }
        //Check the meaning of "dia"
        if (mp_iWhatDia[mp_iIndexes[iSp]] == DBH2)
          fDBH *= fDBH;

        //Calculate the appropriate biomass
        if (1 == mp_iEquationID[mp_iIndexes[iSp]]) {
          fBiomass = pow(10, mp_fA[mp_iIndexes[iSp]] + (mp_fB[mp_iIndexes[iSp]] * (log10(pow(fDBH, mp_fC[mp_iIndexes[iSp]])))));
        }
        else if (2 == mp_iEquationID[mp_iIndexes[iSp]]) {
          fBiomass = exp(mp_fA[mp_iIndexes[iSp]] +
                         (mp_fB[mp_iIndexes[iSp]] * fDBH) +
                         (mp_fC[mp_iIndexes[iSp]] * log(pow(fDBH, mp_fD[mp_iIndexes[iSp]]))));
        }
        else if (3 == mp_iEquationID[mp_iIndexes[iSp]]) {
          fBiomass = exp(mp_fA[mp_iIndexes[iSp]] +
                         (mp_fB[mp_iIndexes[iSp]] * log(fDBH)) +
                         (mp_fC[mp_iIndexes[iSp]] * (mp_fD[mp_iIndexes[iSp]] + (mp_fE[mp_iIndexes[iSp]] * log(fDBH)))));
        }
        else if (4 == mp_iEquationID[mp_iIndexes[iSp]]) {
          fBiomass = mp_fA[mp_iIndexes[iSp]] +
                    (mp_fB[mp_iIndexes[iSp]] * fDBH) +
                    (mp_fC[mp_iIndexes[iSp]] * pow(fDBH, mp_fD[mp_iIndexes[iSp]]));
        }
        else if (5 == mp_iEquationID[mp_iIndexes[iSp]]) {
          fBiomass = mp_fA[mp_iIndexes[iSp]] +
                   (mp_fB[mp_iIndexes[iSp]] * fDBH) +
                   (mp_fC[mp_iIndexes[iSp]] * pow(fDBH, 2)) +
                   (mp_fD[mp_iIndexes[iSp]] * pow(fDBH, 3));
        }
        else if (6 == mp_iEquationID[mp_iIndexes[iSp]]) {
          fBiomass = mp_fA[mp_iIndexes[iSp]] * exp(mp_fB[mp_iIndexes[iSp]] + (mp_fC[mp_iIndexes[iSp]] * log(fDBH)) + (mp_fD[mp_iIndexes[iSp]] * fDBH));
        }
        else if (7 == mp_iEquationID[mp_iIndexes[iSp]]) {
          fBiomass = mp_fA[mp_iIndexes[iSp]] + ((mp_fB[mp_iIndexes[iSp]] * pow(fDBH, mp_fC[mp_iIndexes[iSp]]))/(pow(fDBH, mp_fC[mp_iIndexes[iSp]]) + mp_fD[mp_iIndexes[iSp]]));
        }
        else if (8 == mp_iEquationID[mp_iIndexes[iSp]]) {
          fBiomass = pow(100, mp_fA[mp_iIndexes[iSp]] + (mp_fB[mp_iIndexes[iSp]] * log10(fDBH)));
        }
        else {
          fBiomass = exp(log(mp_fA[mp_iIndexes[iSp]]) +
                           (mp_fB[mp_iIndexes[iSp]] * log(fDBH)));
        }

        //Apply a correction factor, if appropriate
        if (mp_bUseCorrectionFactor[mp_iIndexes[iSp]]) {
          fBiomass *= mp_fCorrectionFactor[mp_iIndexes[iSp]];
        }

        //Convert to Mg
        fBiomass *= mp_fBiomassConverter[mp_iIndexes[iSp]];

        p_oTree->SetValue( mp_iBiomassCodes[mp_iIndexes[iSp]] [iTp - clTreePopulation::sapling], fBiomass );
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
    stcErr.sFunction = "clDimensionAnalysis::Action" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clDimensionAnalysis::RegisterTreeDataMembers()
{

  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  short int iNumSpecies = p_oPop->GetNumberOfSpecies(),
            i;

  //Make the list of indexes
  mp_iIndexes = new int[iNumSpecies];
  for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    mp_iIndexes[mp_iWhatSpecies[i]] = i;


  //Declare the array and register our new data member
  mp_iBiomassCodes = new short int * [m_iNumBehaviorSpecies];
  for (i = 0; i < m_iNumBehaviorSpecies; i++) {
    mp_iBiomassCodes[i] = new short int[3];
  }

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
  {
    //Make sure that only allowed types have been applied
    if ( clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::sapling != mp_whatSpeciesTypeCombos[i].iType
         && clTreePopulation::snag != mp_whatSpeciesTypeCombos[i].iType )
         {

           modelErr stcErr;
           stcErr.sFunction = "clDimensionAnalysis::RegisterTreeDataMembers" ;
           stcErr.sMoreInfo = "This behavior can only be applied to saplings, adults, and snags.";
           stcErr.iErrorCode = BAD_DATA;
           throw( stcErr );
    }

    //Register the code and capture it
    mp_iBiomassCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
         [mp_whatSpeciesTypeCombos[i].iType - clTreePopulation::sapling] =
         p_oPop->RegisterFloat( "Biomass", mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType );
  }
}
