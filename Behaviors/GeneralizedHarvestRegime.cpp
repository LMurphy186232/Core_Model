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

//#ifdef DEBUG_GEN_HARV
//  out = std::fstream();
//#endif

  m_sNameString = "GeneralizedHarvestRegime";
  m_sXMLRoot = "GeneralizedHarvestRegime";

  //Versions
  m_fVersionNumber = 2.0;
  m_fMinimumVersionNumber = 1;

  m_iReasonCode = harvest;

  m_iNewTreeBools = 1;
  m_iNumUserDefDistSizeClasses = 10;

  mp_iBiomassCode = NULL;
  mp_fCutProbAlpha = NULL;
  mp_fCutProbBeta = NULL;
  mp_fCutProbGamma = NULL;
  mp_fCutProbMu = NULL;
  mp_iHarvestCode = NULL;
  mp_fCutAmtIntensityClasses = NULL;
  mp_fCutAmtIntensityClassProb = NULL;

  mp_fCutProbA = NULL;
  mp_fCutProbB = NULL;
  mp_fCutProbC = NULL;

  m_fGammaMeanRemoveM = 0;
  m_fLogProbB = 0;
  m_iNumSpecies = 0;
  m_fTotalBiomass = 0;
  m_fLogProbA = 0;

  m_fScale = 0;
  m_fGammaMeanRemoveA = 0;
  mp_oPop = NULL;
  m_fGammaMeanRemoveB = 0;
  m_fLogProbM = 0;
  m_fTotalBA = 0;
  m_fAllowedRange = 0;
  m_bUseBiomass = true;
  m_bSaplingMortality = false;
  m_bUseGammaDist = true;
  m_fSapP = 0;
  m_fSapM = 0;
  m_fSapN = 0;

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

#ifdef DEBUG_GEN_HARV
  out.close();
#endif
  delete[] mp_iBiomassCode;
  delete[] mp_fCutProbAlpha;
  delete[] mp_fCutProbBeta;
  delete[] mp_fCutProbGamma;
  delete[] mp_fCutProbMu;
  delete[] mp_iHarvestCode;
  delete[] mp_fCutAmtIntensityClasses;
  delete[] mp_fCutAmtIntensityClassProb;
  delete[] mp_fCutProbA;
  delete[] mp_fCutProbB;
  delete[] mp_fCutProbC;
}


//////////////////////////////////////////////////////////////////////////////
// ReadHarvestParameterFileData()
//////////////////////////////////////////////////////////////////////////////
void clGeneralizedHarvestRegime::ReadHarvestParameterFileData( xercesc::DOMDocument * p_oDoc )
{
  DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

  mp_fCutProbAlpha = new double[m_iNumSpecies];
  mp_fCutProbBeta = new double[m_iNumSpecies];
  mp_fCutProbGamma = new double[m_iNumSpecies];
  mp_fCutProbMu = new double[m_iNumSpecies];
  mp_fCutProbA = new double[m_iNumSpecies];
  mp_fCutProbB = new double[m_iNumSpecies];
  mp_fCutProbC = new double[m_iNumSpecies];

  //------------------------------------
  // Logging probability parameters

  // Logging probability A
  FillSingleValue( p_oElement, "di_genHarvLogProbA", &m_fLogProbA, true);

  // Logging probability B
  FillSingleValue( p_oElement, "di_genHarvLogProbB", &m_fLogProbB, true);

  // Logging probability M
  FillSingleValue( p_oElement, "di_genHarvLogProbM", &m_fLogProbM, true);

  //------------------------------------
  // Remove amount probability distribution

  //Which distribution are we using?
  FillSingleValue( p_oElement, "di_genHarvRemoveDist", & m_bUseGammaDist, false );

  if (m_bUseGammaDist) {
    // *** Using gamma ***

    // Amount to remove alpha
    FillSingleValue( p_oElement, "di_genHarvLogRemoveAlpha", &m_fGammaMeanRemoveA, true);

    // Amount to remove beta
    FillSingleValue( p_oElement, "di_genHarvLogRemoveBeta", &m_fGammaMeanRemoveB, true);

    // Amount to remove mu
    FillSingleValue( p_oElement, "di_genHarvLogRemoveMu", &m_fGammaMeanRemoveM, true);

    // Gamma scale parameter
    FillSingleValue( p_oElement, "di_genHarvGammaScale", &m_fScale, true);

  } else {
    // *** Using user-defined ***

    mp_fCutAmtIntensityClasses = new double[m_iNumUserDefDistSizeClasses];
    mp_fCutAmtIntensityClassProb = new double[m_iNumUserDefDistSizeClasses];
    double *p1 = mp_fCutAmtIntensityClasses, *p2 = mp_fCutAmtIntensityClassProb;
    double fTemp = 0;
    std::stringstream sLabel;
    int i;
    //Get first 9 values
    for (i = 0; i < (m_iNumUserDefDistSizeClasses-1); i++) {
      sLabel << "di_genHarvIntensClass" << (i+1);
      FillSingleValue( p_oElement, sLabel.str(), p1, true);
      sLabel.str("");
      sLabel << "di_genHarvIntensClassProb" << (i+1);
      FillSingleValue( p_oElement, sLabel.str(), p2, true);
      sLabel.str("");
      p1++;
      p2++;
    }
    //The last intensity class must be 1, so don't even bother to read it.
    //Get the last probability though
    mp_fCutAmtIntensityClasses[(m_iNumUserDefDistSizeClasses-1)] = 1.0;
    sLabel << "di_genHarvIntensClassProb" << m_iNumUserDefDistSizeClasses;
    FillSingleValue( p_oElement, sLabel.str(), p2, true);
    sLabel.str("");

    //Make sure all the intensity classes and harvest probabilities are between
    //0 and 1, and that all intensity classes are larger than the previous
    //class
    for (i = 0; i < m_iNumUserDefDistSizeClasses; i++) {
      if (mp_fCutAmtIntensityClasses[i] < 0 ||
          mp_fCutAmtIntensityClasses[i] > 1) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clGeneralizedHarvestRegime::ReadHarvestParameterFileData";
        stcErr.sMoreInfo = "All values for harvest intensity classes must be between 0 and 1.";
        throw( stcErr );
      }

      if (mp_fCutAmtIntensityClassProb[i] < 0 ||
          mp_fCutAmtIntensityClassProb[i] > 1) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clGeneralizedHarvestRegime::ReadHarvestParameterFileData";
        stcErr.sMoreInfo = "All values for harvest intensity class probabilities must be between 0 and 1.";
        throw( stcErr );
      }
      fTemp += mp_fCutAmtIntensityClassProb[i];
    }
    if (abs(fTemp - 1) > 0.001) {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGeneralizedHarvestRegime::ReadHarvestParameterFileData";
      stcErr.sMoreInfo = "Harvest intensity class probabilities must add up to 1.";
      throw( stcErr );
    }

    for (i = 1; i < m_iNumUserDefDistSizeClasses; i++) {
      if (mp_fCutAmtIntensityClasses[i] <= mp_fCutAmtIntensityClasses[i-1]) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clGeneralizedHarvestRegime::ReadHarvestParameterFileData";
        stcErr.sMoreInfo = "Harvest intensity classes must be ordered from lowest to highest.";
        throw( stcErr );
      }
    }

    //Make the user distribution cumulative
    for (i = 1; i < m_iNumUserDefDistSizeClasses; i++) {
      mp_fCutAmtIntensityClassProb[i] += mp_fCutAmtIntensityClassProb[i-1];
    }
  }

  //------------------------------------
  // Cut preference parameters

  // Cut probability alpha - species specific
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbAlpha",
      "di_ghlcpaVal", mp_fCutProbAlpha, mp_oPop, true );

  // Cut probability beta - species specific
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbBeta",
      "di_ghlcpbVal", mp_fCutProbBeta, mp_oPop, true );

  // Cut probability gamma - species specific
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbGamma",
      "di_ghlcpgVal", mp_fCutProbGamma, mp_oPop, true );

  // Cut probability mu - species specific
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbMu",
      "di_ghlcpmVal", mp_fCutProbMu, mp_oPop, true );

  // Cut probability A - species specific
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbA",
        "di_ghlcpaaVal", mp_fCutProbA, mp_oPop, true );

  // Cut probability B
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbB",
          "di_ghlcpbVal", mp_fCutProbB, mp_oPop, true );

  // Cut probability C
  FillSpeciesSpecificValue( p_oElement, "di_genHarvLogCutProbC",
          "di_ghlcpcVal", mp_fCutProbC, mp_oPop, true );

  //Allowed range
  FillSingleValue( p_oElement, "di_genHarvAllowedDeviation", &m_fAllowedRange, true);

  //Whether to use biomass (true) or basal area (false)
  FillSingleValue( p_oElement, "di_genHarvUseBiomassOrBA", & m_bUseBiomass, true );

  //Whether to implement sapling mortality
  FillSingleValue( p_oElement, "di_genHarvDoSaplingMort", & m_bSaplingMortality, false );

  //If we're using sapling mortality, get the parameters
  if (m_bSaplingMortality) {
    FillSingleValue( p_oElement, "di_genHarvSapMortP", &m_fSapP, true);
    FillSingleValue( p_oElement, "di_genHarvSapMortM", &m_fSapM, true);
    FillSingleValue( p_oElement, "di_genHarvSapMortN", &m_fSapN, true);
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clGeneralizedHarvestRegime::GetData( xercesc::DOMDocument * p_oDoc )
{

#ifdef DEBUG_GEN_HARV
    using namespace std;
    char cFilename[100];
    std::string sfile = mp_oSimManager->GetParFilename();
    sprintf(cFilename, "%s%s", sfile.c_str(),"GenHarv.txt");
    out.open( cFilename, ios::trunc | ios::out );
    out << "Timestep\tDesired PC\tDesired BA\tFirst Pass BA\tFinal BA\n";
#endif

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
  double *p_fPlotCutProb = new double[m_iNumSpecies],
         *p_fCutProbSigma = new double[m_iNumSpecies];
  double fAmtBAToCut,  //target amount of plot BA to cut
        fPercentBAToCut, //target percentage of plot BA to cut
        fThisBA,
        fCutProb,
        fCorrectionFactor,
        fBARemoved = 0;
  float fDbh;
  int i, iSp = 0;
  bool bIsHarvested;

  //Are we harvesting this timestep? If not, exit
  if (!CutThisTimestep()) {
    delete[] p_fPlotCutProb;
    #ifdef DEBUG_GEN_HARV
      out << mp_oSimManager->GetCurrentTimestep() << "\t0\t0\t0\t0\n";
    #endif
    return;
  }

  fPercentBAToCut = GetPercentToCut();
  fAmtBAToCut = fPercentBAToCut * 0.01 * m_fTotalBA; //convert from % to proportion
  #ifdef DEBUG_GEN_HARV
      out << mp_oSimManager->GetCurrentTimestep() << "\t" << fPercentBAToCut << "\t" << fAmtBAToCut << "\t";
  #endif

  //******************************
  //Calculate the non size dependent portions of the cut probability
  for (i = 0; i < m_iNumSpecies; i++) {
    p_fPlotCutProb[i] = 1 - (mp_fCutProbGamma[i] * exp(-mp_fCutProbBeta[i] *
                          pow(fPercentBAToCut, mp_fCutProbAlpha[i])));
    p_fCutProbSigma[i] = mp_fCutProbA[i] + mp_fCutProbB[i] *
                                  pow(fPercentBAToCut, mp_fCutProbC[i]);
  }



  //*****************************
  //First harvesting pass
  while ( p_oTree ) {
    iSp = p_oTree->GetSpecies();
    //Calculate cut probability
    p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);
    fCutProb = p_fPlotCutProb[iSp] *
    exp(-0.5*pow(((fDbh - mp_fCutProbMu[iSp])/p_fCutProbSigma[iSp]), 2));

    if (clModelMath::GetRand() <= fCutProb) {
      p_oTree->SetValue(mp_iHarvestCode[iSp], true);
      //Calculate basal area
      fThisBA = clModelMath::CalculateBasalArea( fDbh );
      fBARemoved += fThisBA;
    } else p_oTree->SetValue(mp_iHarvestCode[iSp], false);
    p_oTree = p_oAdults->NextTree();

  } //end of while (p_oTree)

   #ifdef DEBUG_GEN_HARV
      out << fBARemoved << "\t";
  #endif

  //*****************************
  //Second harvesting pass, if needed
  if ((fabs(fBARemoved - fAmtBAToCut) / fAmtBAToCut) >= m_fAllowedRange) {
    p_oAdults->StartOver();
    p_oTree = p_oAdults->NextTree();
    fCorrectionFactor = fAmtBAToCut / fBARemoved;
    fBARemoved = 0;
    if (fCorrectionFactor < 1) { //too much cut - put some back
      while ( p_oTree ) {
        iSp = p_oTree->GetSpecies();
//        p_oTree->GetValue(mp_iHarvestCode[iSp], &bIsHarvested);
        //NOTE: DON'T WORK ONLY ON HARVESTED TREES. The correction factor
        //is an all-plot correction factor. If you take only the harvested
        //trees you will way underestimate the amount to remove.
//        if (bIsHarvested) {

          //Calculate cut probability
          p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);
          fCutProb = p_fPlotCutProb[iSp] *
          exp(-0.5*pow(((fDbh - mp_fCutProbMu[iSp])/p_fCutProbSigma[iSp]), 2));
          fCutProb *= fCorrectionFactor;

          if (clModelMath::GetRand() <= fCutProb) {
            p_oTree->SetValue(mp_iHarvestCode[iSp], true);
            //Calculate basal area
            fThisBA = clModelMath::CalculateBasalArea( fDbh );
            fBARemoved += fThisBA;
          } else p_oTree->SetValue(mp_iHarvestCode[iSp], false);
//        }
        p_oTree = p_oAdults->NextTree();
      } //end of while (p_oTree)
    } else { //too little cut - get some more
      while ( p_oTree ) {
        iSp = p_oTree->GetSpecies();
//        p_oTree->GetValue(mp_iHarvestCode[iSp], &bIsHarvested);
        //if (!bIsHarvested) {
          //Calculate cut probability
          p_oTree->GetValue(mp_oPop->GetDbhCode(iSp, p_oTree->GetType()), &fDbh);
          fCutProb = p_fPlotCutProb[iSp] *
          exp(-0.5*pow(((fDbh - mp_fCutProbMu[iSp])/p_fCutProbSigma[iSp]), 2));
          fCutProb *= fCorrectionFactor;

          if (clModelMath::GetRand() <= fCutProb) {
            p_oTree->SetValue(mp_iHarvestCode[iSp], true);
            //Calculate basal area
            fThisBA = clModelMath::CalculateBasalArea( fDbh );
            fBARemoved += fThisBA;
          } else p_oTree->SetValue(mp_iHarvestCode[iSp], false);
       // }
        p_oTree = p_oAdults->NextTree();
      } //end of while (p_oTree)
    }
  }

#ifdef DEBUG_GEN_HARV
  out << fBARemoved << "\n";
#endif

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
  delete[] p_fCutProbSigma;

  //Sapling mortality, if applicable; I'm re-using some variables here
  if (m_bSaplingMortality) {

    //Get the percent of basal area removed
    fPercentBAToCut = 100.0 * fBARemoved / m_fTotalBA;

    //Get the probability of sapling mortality
    fCutProb = m_fSapP + ((1-m_fSapP)/(1 + pow(fPercentBAToCut/m_fSapM, m_fSapN)));

    //Get all the saplings
    sprintf(cQuery, "%s%d", "type=", clTreePopulation::sapling);
    p_oAdults = mp_oPop->Find(cQuery);
    p_oTree = p_oAdults->NextTree();
    while ( p_oTree ) {

      if (clModelMath::GetRand() <= fCutProb) {
        mp_oPop->KillTree(p_oTree, m_iReasonCode);
      }
      p_oTree = p_oAdults->NextTree();
    }
  }
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

////////////////////////////////////////////////////////////////////////////
// GetPercentToCut()
////////////////////////////////////////////////////////////////////////////
double clGeneralizedHarvestRegime::GetPercentToCut()
{
  double fPercentBAToCutMean,
         fPercentBAToCut;
  int i;

  //Which distribution function are we using?
  if (m_bUseGammaDist) {
    //*****************************
    // Gamma distribution
    //*****************************
    //Calculate the proportion of BA to remove (the correct value, either total
    //biomass or total BA, is held in m_fTotalBiomass)
    fPercentBAToCutMean = m_fGammaMeanRemoveA * exp(-m_fGammaMeanRemoveM * pow(m_fTotalBiomass, m_fGammaMeanRemoveB));
    if (fPercentBAToCutMean <= 0) return 0;

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

  } else {

    //*****************************
    // User-defined distribution
    //*****************************
    //Get a random number on a uniform distribution between 0 and 1 to find
    //the intensity class we're in
    float fRand = clModelMath::GetRand();
    double fUpperBound, fLowerBound;
    i = 0;
    for (i = 0; i < m_iNumUserDefDistSizeClasses; i++) {
      if (mp_fCutAmtIntensityClassProb[i] > 0 &&
         fRand <= mp_fCutAmtIntensityClassProb[i]) {
        break;
      }
    }

    if (i >= m_iNumUserDefDistSizeClasses) i = m_iNumUserDefDistSizeClasses-1;

    //Get the upper and lower bounds of the intensity class
    fUpperBound = mp_fCutAmtIntensityClasses[i];
    if (i == 0) fLowerBound = 0;
    else fLowerBound = mp_fCutAmtIntensityClasses[i-1];

    //Get a random value on a uniform distribution within those bounds
    fRand = clModelMath::GetRand();
    fPercentBAToCut = fLowerBound + fRand * (fUpperBound - fLowerBound);
    fPercentBAToCut *= 100.0; //convert from proportion to percent
  }
  return fPercentBAToCut;
}
