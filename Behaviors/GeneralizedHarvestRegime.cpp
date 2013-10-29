//---------------------------------------------------------------------------
#include "GeneralizedHarvestRegime.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "TreePopulation.h"
#include "Plot.h"
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clGeneralizedHarvestRegime::clGeneralizedHarvestRegime( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{

  m_sNameString = "GeneralizedHarvestRegime";
  m_sXMLRoot = "GeneralizedHarvestRegime";

  //Versions
  m_fVersionNumber = 1.1;
  m_fMinimumVersionNumber = 1;

  m_iReasonCode = harvest;

  m_iNewTreeBools = 1;

  mp_iBiomassCode = NULL;
  mp_fCutProbAlpha = NULL;
  mp_fCutProbBeta = NULL;
  mp_fCutProbGamma = NULL;
  mp_fCutProbMu = NULL;
  mp_iHarvestCode = NULL;

  m_fRemoveM = 0;
  m_fLogProbB = 0;
  m_iNumSpecies = 0;
  m_fTotalBiomass = 0;
  m_fLogProbA = 0;
  m_fCutProbC = 0;
  m_fScale = 0;
  m_fRemoveA = 0;
  mp_oPop = NULL;
  m_fRemoveB = 0;
  m_fCutProbB = 0;
  m_fLogProbM = 0;
  m_fTotalBA = 0;
  m_fAllowedRange = 0;
  m_fCutProbA = 0;
  m_bUseBiomass = true;

  //Allowed file types
  m_iNumAllowedTypes = 2;
  mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
  mp_iAllowedFileTypes[0] = parfile;
  mp_iAllowedFileTypes[1] = detailed_output;

}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clGeneralizedHarvestRegime::~clGeneralizedHarvestRegime()
{
  delete[] mp_iBiomassCode;
  delete[] mp_fCutProbAlpha;
  delete[] mp_fCutProbBeta;
  delete[] mp_fCutProbGamma;
  delete[] mp_fCutProbMu;
  delete[] mp_iHarvestCode;
}


//////////////////////////////////////////////////////////////////////////////
// ReadHarvestParameterFileData()
//////////////////////////////////////////////////////////////////////////////
void clGeneralizedHarvestRegime::ReadHarvestParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

  mp_fCutProbAlpha = new float[m_iNumSpecies];
  mp_fCutProbBeta = new float[m_iNumSpecies];
  mp_fCutProbGamma = new float[m_iNumSpecies];
  mp_fCutProbMu = new float[m_iNumSpecies];

  // Logging probability A
  FillSingleValue( p_oElement, "di_genHarvLogProbA", &m_fLogProbA, true);

  // Logging probability B
  FillSingleValue( p_oElement, "di_genHarvLogProbB", &m_fLogProbB, true);

  // Logging probability M
  FillSingleValue( p_oElement, "di_genHarvLogProbM", &m_fLogProbM, true);

  // Amount to remove alpha
  FillSingleValue( p_oElement, "di_genHarvLogRemoveAlpha", &m_fRemoveA, true);

  // Amount to remove beta
    FillSingleValue( p_oElement, "di_genHarvLogRemoveBeta", &m_fRemoveB, true);

  // Amount to remove mu
  FillSingleValue( p_oElement, "di_genHarvLogRemoveMu", &m_fRemoveM, true);

  // Gamma scale parameter
  FillSingleValue( p_oElement, "di_genHarvGammaScale", &m_fScale, true);

  // Cut probability alpha - species specific-->
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbAlpha",
      "di_ghlcpaVal", mp_fCutProbAlpha, mp_oPop, true );

  // Cut probability beta - species specific-->
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbBeta",
      "di_ghlcpbVal", mp_fCutProbBeta, mp_oPop, true );

  // Cut probability gamma - species specific-->
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbGamma",
      "di_ghlcpgVal", mp_fCutProbGamma, mp_oPop, true );

  // Cut probability mu - species specific-->
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbMu",
      "di_ghlcpmVal", mp_fCutProbMu, mp_oPop, true );

  // Cut probability A
  FillSingleValue( p_oElement, "di_genHarvLogCutProbA", &m_fCutProbA, true);

  // Cut probability B
  FillSingleValue( p_oElement, "di_genHarvLogCutProbB", &m_fCutProbB, true);

  // Cut probability C
  FillSingleValue( p_oElement, "di_genHarvLogCutProbC", &m_fCutProbC, true);

  //Allowed range
  FillSingleValue( p_oElement, "di_genHarvAllowedDeviation", &m_fAllowedRange, true);

  //Whether to use biomass (true) or basal area (false)
  FillSingleValue( p_oElement, "di_genHarvUseBiomassOrBA", & m_bUseBiomass, true );
}

//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clGeneralizedHarvestRegime::GetData( xercesc::DOMDocument * p_oDoc )
{
  ReadHarvestParameterFileData( p_oDoc );
  GetDataCodes();
}

//////////////////////////////////////////////////////////////////////////////
// Action()
//////////////////////////////////////////////////////////////////////////////
void clGeneralizedHarvestRegime::Action()
{
  char cQuery[20];
  sprintf(cQuery, "%s%d", "type=", clTreePopulation::adult);
  clTreeSearch *p_oAdults = mp_oPop->Find(cQuery);
  clTree *p_oTree = p_oAdults->NextTree(), *p_oNextTree;
  float *p_fPlotCutProb = new float[m_iNumSpecies];
  float fAmtBAToCut,  //target amount of plot BA to cut
        fPercentBAToCut, //target percentage of plot BA to cut
        fPercentBAToCutMean,
        fThisBA,
        fCutProbSigma,
        fCutProb,
        fCorrectionFactor,
        fBARemoved = 0,
        fDbh;
  int i, iSp;
  bool bIsHarvested;

  //Are we harvesting this timestep? If not, exit
  if (!CutThisTimestep()) return;

  //*****************************
  //Calculate the proportion of BA to remove (the correct value, either total
  //biomass or total BA, is held in m_fTotalBiomass)
  fPercentBAToCutMean = m_fRemoveA * exp(-m_fRemoveM * pow(m_fTotalBiomass, m_fRemoveB));
  if (fPercentBAToCutMean <= 0) return;

  //Cut it off at 100 draws; after that we'll go with 100
  i = 0;
  do {
    fPercentBAToCut = clModelMath::GammaRandomDraw(fPercentBAToCutMean, m_fScale);
    i++;
  } while (fPercentBAToCut > 100 && i < 100);
  if (i == 99 && fPercentBAToCut > 100) fPercentBAToCut = 100;

  //LEM: Specifically don't do this next step - rounding down to 100 means that
  //the entire tail of the gamma >= 100 is artificially lumped at 100. That's
  //why we throw out values greater than 100 and draw again.
  //Bound the result between zero and one hundred
  //fPercentBAToCut = fPercentBAToCut > 100 ? 100 : fPercentBAToCut;
  fPercentBAToCut = fPercentBAToCut < 0 ? 0 : fPercentBAToCut;
  //Multiply by the total amount of basal area to get the actual target
  fAmtBAToCut = fPercentBAToCut * 0.01 * m_fTotalBA; //convert from percent to proportion

  //******************************
  //Calculate the non size dependent portions of the cut probability
  for (i = 0; i < m_iNumSpecies; i++) {
    p_fPlotCutProb[i] = 1 - (mp_fCutProbGamma[i] * exp(-mp_fCutProbBeta[i] *
                          pow(fPercentBAToCut, mp_fCutProbAlpha[i])));
  }
  fCutProbSigma = m_fCutProbA + m_fCutProbB *
                              pow(fPercentBAToCut, m_fCutProbC);


  //*****************************
  //First harvesting pass
  while ( p_oTree ) {
    iSp = p_oTree->GetSpecies();
    //Calculate cut probability
    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);
    fCutProb = p_fPlotCutProb[iSp] *
    exp(-0.5*pow(((fDbh - mp_fCutProbMu[iSp])/fCutProbSigma), 2));

    if (clModelMath::GetRand() <= fCutProb) {
      p_oTree->SetValue(mp_iHarvestCode[iSp], true);
      //Calculate basal area
      fThisBA = clModelMath::CalculateBasalArea( fDbh );
      fBARemoved += fThisBA;
    } else p_oTree->SetValue(mp_iHarvestCode[iSp], false);
    p_oTree = p_oAdults->NextTree();

  } //end of while (p_oTree)

  //*****************************
  //Second harvesting pass, if needed
  if ((fabs(fBARemoved - fAmtBAToCut) / fAmtBAToCut) >= m_fAllowedRange) {
    p_oAdults->StartOver();
    p_oTree = p_oAdults->NextTree();
    fCorrectionFactor = fAmtBAToCut / fBARemoved;
    if (fCorrectionFactor < 1) { //too much cut - put some back
      while ( p_oTree ) {
        iSp = p_oTree->GetSpecies();
        p_oTree->GetValue(mp_iHarvestCode[iSp], &bIsHarvested);
        if (bIsHarvested) {

          //Calculate cut probability
          p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);
          fCutProb = p_fPlotCutProb[iSp] *
          exp(-0.5*pow(((fDbh - mp_fCutProbMu[iSp])/fCutProbSigma), 2));
          fCutProb *= fCorrectionFactor;

          if (clModelMath::GetRand() <= fCutProb) {
            p_oTree->SetValue(mp_iHarvestCode[iSp], true);
            //Calculate basal area
            fThisBA = clModelMath::CalculateBasalArea( fDbh );
            fBARemoved += fThisBA;
          } else p_oTree->SetValue(mp_iHarvestCode[iSp], false);
        }
        p_oTree = p_oAdults->NextTree();
      } //end of while (p_oTree)
    } else { //too little cut - get some more
      while ( p_oTree ) {
        iSp = p_oTree->GetSpecies();
        p_oTree->GetValue(mp_iHarvestCode[iSp], &bIsHarvested);
        if (!bIsHarvested) {
          //Calculate cut probability
          p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);
          fCutProb = p_fPlotCutProb[iSp] *
          exp(-0.5*pow(((fDbh - mp_fCutProbMu[iSp])/fCutProbSigma), 2));
          fCutProb *= fCorrectionFactor;

          if (clModelMath::GetRand() <= fCutProb) {
            p_oTree->SetValue(mp_iHarvestCode[iSp], true);
            //Calculate basal area
            fThisBA = clModelMath::CalculateBasalArea( fDbh );
            fBARemoved += fThisBA;
          } else p_oTree->SetValue(mp_iHarvestCode[iSp], false);
        }
        p_oTree = p_oAdults->NextTree();
      } //end of while (p_oTree)
    }
  }

  //Killing pass
  p_oAdults->StartOver();
  p_oTree = p_oAdults->NextTree();
  while ( p_oTree ) {
    p_oNextTree = p_oAdults->NextTree();
    p_oTree->GetValue(mp_iHarvestCode[iSp], &bIsHarvested);
    if (bIsHarvested) mp_oPop->KillTree(p_oTree, m_iReasonCode);
    p_oTree = p_oNextTree;
  } //end of while (p_oTree)
  delete[] p_fPlotCutProb;
}


//////////////////////////////////////////////////////////////////////////////
// GetDataCodes()
//////////////////////////////////////////////////////////////////////////////
void clGeneralizedHarvestRegime::GetDataCodes() {

  if (m_bUseBiomass == false) return;

  int i;
  mp_iBiomassCode = new short int [m_iNumSpecies];
  for (i = 0; i < m_iNumSpecies; i++) {
    mp_iBiomassCode[i] =
      mp_oPop->GetFloatDataCode("Biomass", i, clTreePopulation::adult);
    if (-1 == mp_iBiomassCode[i]) {
      modelErr stcErr;
      stcErr.sFunction = "clGeneralizedHarvestRegime::GetBiomassCodes";
      std::stringstream s;
      s << "Type/species combo species=" << i << " type="
        << clTreePopulation::adult << " does not have Dimension Analysis applied.";
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = s.str();
      throw(stcErr);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// CutThisTimestep()
//////////////////////////////////////////////////////////////////////////////
bool clGeneralizedHarvestRegime::CutThisTimestep() {
  char cQuery[20];
  sprintf(cQuery, "%s%d", "type=", clTreePopulation::adult);
  clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
  clTreeSearch *p_oAdults = mp_oPop->Find(cQuery);
  clTree *p_oTree = p_oAdults->NextTree();
  float fVal, fLogProb,
        fPlotArea = p_oPlot->GetPlotArea();

  m_fTotalBiomass = 0;
  m_fTotalBA = 0;

  if (m_bUseBiomass) {

    //Add up total biomass and BA
    while (p_oTree) {
      p_oTree->GetValue(mp_iBiomassCode[p_oTree->GetSpecies()], &fVal);
      m_fTotalBiomass += fVal;

      p_oTree->GetValue(mp_oPop->GetDbhCode(p_oTree->GetSpecies(),
          p_oTree->GetType()), &fVal);
      m_fTotalBA += clModelMath::CalculateBasalArea(fVal);

      p_oTree = p_oAdults->NextTree();
    }
    m_fTotalBiomass /= fPlotArea;
  } else {
    //Add up total BA only
    while (p_oTree) {

      p_oTree->GetValue(mp_oPop->GetDbhCode(p_oTree->GetSpecies(),
          p_oTree->GetType()), &fVal);
      m_fTotalBA += clModelMath::CalculateBasalArea(fVal);

      p_oTree = p_oAdults->NextTree();
    }
    m_fTotalBiomass = m_fTotalBA / fPlotArea;
  }

  //Assess the probability equation - remember that this is probability of
  //NOT logging
  if (0 == m_fTotalBiomass) return false;

  fLogProb = m_fLogProbA * exp(-m_fLogProbM * pow(m_fTotalBiomass, m_fLogProbB));

  if (clModelMath::GetRand() <= fLogProb) return false;
  else return true;
}

////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
////////////////////////////////////////////////////////////////////////////
void clGeneralizedHarvestRegime::RegisterTreeDataMembers()
{
  int i;

  mp_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  m_iNumSpecies = mp_oPop->GetNumberOfSpecies();

  //Declare the array and register our new data member
  mp_iHarvestCode = new short int [m_iNumSpecies];
  for ( i = 0; i < m_iNumSpecies; i++ ) mp_iHarvestCode[i] = -1;

  for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ ) {
    //Make sure that only allowed types have been applied
    if ( clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType ) {
      modelErr stcErr;
      stcErr.sFunction = "clGeneralizedHarvestRegime::RegisterTreeDataMembers" ;
      stcErr.sMoreInfo = "This behavior can only be applied to adults.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Register the code and capture it
    mp_iHarvestCode[mp_whatSpeciesTypeCombos[i].iSpecies] =
      mp_oPop->RegisterBool( "Gen Harvest",
                              mp_whatSpeciesTypeCombos[i].iSpecies,
                              mp_whatSpeciesTypeCombos[i].iType );
  }

  //Make sure every species was registered
  for ( i = 0; i < m_iNumSpecies; i++ )
    if (-1 == mp_iHarvestCode[i]) {
      modelErr stcErr;
      stcErr.sFunction = "clGeneralizedHarvestRegime::RegisterTreeDataMembers" ;
      stcErr.sMoreInfo = "This behavior must be applied to all species.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }
}
