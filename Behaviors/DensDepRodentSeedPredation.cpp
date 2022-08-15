//---------------------------------------------------------------------------
// DensDepRodentSeedPredation.cpp
//---------------------------------------------------------------------------
#include "DensDepRodentSeedPredation.h"
#include "Grid.h"
#include "ModelMath.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Plot.h"
#include "TreePopulation.h"
#include "MastingDisperseAutocorrelation.h"
#include <math.h>
#include <fstream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clDensDepRodentSeedPredation::clDensDepRodentSeedPredation(
    clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try
  {
    m_sNameString = "DensDepRodentSeedPredation";
    m_sXMLRoot = "DensDepRodentSeedPredation";

    mp_oRodentGrid = NULL;
    mp_oSeedGrid = NULL;
    mp_fFuncResponseSlope = NULL;
    mp_iSeedGridCode = NULL;
    mp_fMastTimeSeries = NULL;

    mp_oSeedGrid = NULL;

    m_iNumTimesteps = 1;
    m_iRodentLambdaCode = -1;
    m_fFuncResponseA = 0;
    m_fDensDepCoeff = 0;
    m_fLastTimestepLambda = 1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

  }
  catch (modelErr& err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDensDepRodentSeedPredation::clDensDepRodentSeedPredation";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clDensDepRodentSeedPredation::~clDensDepRodentSeedPredation() {
  short int i; //loop counters
  delete[] mp_fFuncResponseSlope;
  delete[] mp_iSeedGridCode;

  if (mp_fMastTimeSeries) {
    for (i = 0; i < m_iNumTimesteps; i++) {
      delete[] mp_fMastTimeSeries[i];
    }
    delete[] mp_fMastTimeSeries;
  }

}

/////////////////////////////////////////////////////////////////////////////
// Action
/////////////////////////////////////////////////////////////////////////////
void clDensDepRodentSeedPredation::Action() {

  double fDensDep, fFuncResp, fFuncNumerator = 0, fFuncDenominator = 0, fLambda;
  float fSeeds;
  int iTimestep = mp_oSimManager->GetCurrentTimestep(),
      iNumXCells = mp_oSeedGrid->GetNumberXCells(),
      iNumYCells = mp_oSeedGrid->GetNumberYCells(), iX, iY, iSp;

  //-------------------------------------------------------------------------//
  // Calculate rodent lambda
  //-------------------------------------------------------------------------//
  //Calculate density dependence; in timestep 1, this is just 1
  if (iTimestep == 1) {
    fDensDep = 1;
  } else {
    fDensDep =  1 + (m_fDensDepCoeff / log(m_fLastTimestepLambda / (1 + m_fDensDepCoeff)));
  }

  // Calculate the functional response
  iTimestep -= 1; //turn into array index
  for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
    fFuncNumerator += mp_fFuncResponseSlope[mp_iWhatSpecies[iSp]] *
                      mp_fMastTimeSeries[iTimestep][mp_iWhatSpecies[iSp]];
    fFuncDenominator += mp_fFuncResponseSlope[mp_iWhatSpecies[iSp]];
  }
  fFuncResp = pow((exp(fFuncNumerator)/exp(fFuncDenominator)), m_fFuncResponseA);

  // Calculate lambda and bound it sensibly
  fLambda = fDensDep * fFuncResp;
  if (fLambda < 0) fLambda = 0.0001;
  if (fLambda > 1) fLambda = 1;

  // Stash this lambda for next timestep
  m_fLastTimestepLambda = fLambda;

  // Save it to the output grid
  mp_oRodentGrid->SetValueOfCell(0, 0, m_iRodentLambdaCode, (float)fLambda);




  //-------------------------------------------------------------------------//
  // Reduce the seeds
  //-------------------------------------------------------------------------//
  // Flip lambda around so we can use it as a multiplier
  fLambda = 1-fLambda;

  for ( iX = 0; iX < iNumXCells; iX++ ) {
    for ( iY = 0; iY < iNumYCells; iY++ ) {
      for (iSp = 0; iSp < m_iNumBehaviorSpecies; iSp++) {
        mp_oSeedGrid->GetValueOfCell(iX, iY, mp_iSeedGridCode[mp_iWhatSpecies[iSp]], &fSeeds);
        fSeeds *= fLambda;
        mp_oSeedGrid->SetValueOfCell(iX, iY, mp_iSeedGridCode[mp_iWhatSpecies[iSp]], fSeeds);
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clDensDepRodentSeedPredation::GetData(DOMDocument * p_oDoc) {

  ReadParameterFileData(p_oDoc);
  SetupGrids();
  GetMastingIndexes();

  m_fLastTimestepLambda = 1;

}


///////////////////////////////////////////////////////////////////////////////
// GetMastingIndexes
///////////////////////////////////////////////////////////////////////////////
void clDensDepRodentSeedPredation::GetMastingIndexes() {
  bool *p_bFoundSpecies = NULL;
  try {
    clMastingDisperseAutocorrelation *p_oDisperse;
    clBehaviorBase *p_oBeh;
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );

    std::string sNameString;

    int iNumBehaviors = mp_oSimManager->GetNumberOfBehaviors();
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(),
        iNumDispSpecies, iSpecies,
        i, j, k;

    m_iNumTimesteps = mp_oSimManager->GetNumberOfTimesteps();

    // A little flag to see if we caught all of our species
    p_bFoundSpecies = new bool[iNumSpecies];
    for (i = 0; i < iNumSpecies; i++) p_bFoundSpecies[i] = false;

    // Declare our masting index holder
    mp_fMastTimeSeries = new double*[m_iNumTimesteps];
    for (i = 0; i < m_iNumTimesteps; i++) {
      mp_fMastTimeSeries[i] = new double[iNumSpecies];
      for (j = 0; j < iNumSpecies; j++) mp_fMastTimeSeries[i][j] = -1;
    }

    // We might have more than one copy of the masting disperse behavior, so
    // make sure we check for that
    for (i = 0; i < iNumBehaviors; i++) {
      p_oBeh = mp_oSimManager->GetBehaviorObject(i);
      sNameString = p_oBeh->GetName();
      if (sNameString.compare("MastingDisperseAutocorrelation") == 0) {
        //We have a copy of the disperse behavior!
        p_oDisperse =  dynamic_cast<clMastingDisperseAutocorrelation*>(p_oBeh);

        // What species does it apply to?
        iNumDispSpecies = p_oDisperse->GetNumBehaviorSpecies();
        for (j = 0; j < iNumDispSpecies; j++) {
          iSpecies = p_oDisperse->GetBehaviorSpecies(j);

          //Quick sanity check
          if (iSpecies >= 0  && iSpecies < iNumSpecies) {
            p_bFoundSpecies[iSpecies] = true;

            // Grab the masting level
            for (k = 0; k < m_iNumTimesteps; k++) {
              mp_fMastTimeSeries[k][iSpecies] = p_oDisperse->GetMastLevel((k+1));
            }
          }
        }
      }
    }

    // Did we find all of our species?
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      if (!p_bFoundSpecies[mp_iWhatSpecies[i]]) {
        modelErr stcErr;
        stcErr.sFunction = "clDensDepRodentSeedPredation::GetMastingIndexes";
        stcErr.sMoreInfo = "Density dependent rodent seed predation must be used with Masting Disperse with Autocorrelation for all species.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    // Are all of our masting indexes values that we expect?
    for (j = 0; j < iNumSpecies; j++) {
      if (p_bFoundSpecies[j]) {
        for (i = 0; i < m_iNumTimesteps; i++) {
          if (mp_fMastTimeSeries[i][j] < 0 || mp_fMastTimeSeries[i][j] > 1) {
            modelErr stcErr;
            stcErr.sFunction = "clDensDepRodentSeedPredation::GetMastingIndexes";
            stcErr.sMoreInfo = "Found a masting index not between 0 and 1.";
            stcErr.iErrorCode = BAD_DATA;
            throw( stcErr );
          }
        }
      }
    }
    delete[] p_bFoundSpecies;
  }
  catch ( ... )
  {
    delete[] p_bFoundSpecies;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDensDepRodentSeedPredation::GetMastingIndexes" ;
    throw( stcErr );
  }

}



////////////////////////////////////////////////////////////////////////////
// SetupGrids()
////////////////////////////////////////////////////////////////////////////
void clDensDepRodentSeedPredation::SetupGrids()
{
  try
  {
    clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
    clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
    std::stringstream sLabel;
    short int i, iTotalSpecies = p_oPop->GetNumberOfSpecies();

    mp_iSeedGridCode = new short int[iTotalSpecies];

    //Fetch the seed grid
    mp_oSeedGrid = mp_oSimManager->GetGridObject( "Dispersed Seeds" );
    if ( mp_oSeedGrid == NULL )
    {
      modelErr stcErr;
      stcErr.sFunction = "clDensDepRodentSeedPredation::SetupGrids" ;
      stcErr.sMoreInfo = "Disperse behaviors must be used with neighborhood seed predation.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw( stcErr );
    }

    //Now get the data codes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    {
      sLabel << "seeds_" << mp_iWhatSpecies[i];
      mp_iSeedGridCode[mp_iWhatSpecies[i]] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
      if ( -1 == mp_iSeedGridCode[i] )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clDensDepRodentSeedPredation::SetupGrids" ;
        stcErr.sMoreInfo = "Unexpected grid data return code.";
        throw( stcErr );
      }
      sLabel.str("");
    }

    //Set up the Rodent lambda grid
     mp_oRodentGrid = mp_oSimManager->CreateGrid("Rodent Lambda", //grid name
              0, //number of int data members
              1, //number of float data members
              0, //number of char data members
              0, //number of bool data members
              p_oPlot->GetXPlotLength(), //length of X cells
              p_oPlot->GetYPlotLength()); //length of Y cells

    m_iRodentLambdaCode = mp_oRodentGrid->RegisterFloat("rodent_lambda");


  }
  catch (modelErr& err)
  {
    throw(err);
  }
  catch (modelMsg & msg)
  {
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDisperseBase::SetUpBase";
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// ReadParameterFileData()
////////////////////////////////////////////////////////////////////////////
void clDensDepRodentSeedPredation::ReadParameterFileData(xercesc::DOMDocument *p_oDoc) {
  doubleVal * p_fTemp= NULL; //for getting species-specific values
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    std::stringstream sLabel;
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    mp_fFuncResponseSlope = new double[iNumSpecies];

    //Set up our temp array - pre-load with this behavior's species
    p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
    p_fTemp[i].code = mp_iWhatSpecies[i];



    //Parameter for slope of the functional response
    FillSpeciesSpecificValue( p_oElement,
        "pr_densDepFuncRespSlope",
        "pr_ddfrsVal",
        p_fTemp,
        m_iNumBehaviorSpecies,
        p_oPop,
        true );
    //Now transfer the values to the permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_fFuncResponseSlope[p_fTemp[i].code] = p_fTemp[i].val;

    //Exponent "a" in the functional response
    FillSingleValue(p_oElement,
        "pr_densDepFuncRespA",
        &m_fFuncResponseA,
        true );

    //Density dependent coefficient
    FillSingleValue(p_oElement,
        "pr_densDepDensCoeff",
        &m_fDensDepCoeff,
        true );

    delete[] p_fTemp;
  }
  catch ( modelErr& stcErr )
  {
    delete[] p_fTemp;
    throw( stcErr );
  }
  catch ( ... )
  {
    delete[] p_fTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clDensDepRodentSeedPredation::ReadParameterFileData" ;
    throw( stcErr );
  }
}
