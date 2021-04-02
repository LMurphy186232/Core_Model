//---------------------------------------------------------------------------
// MichMenNegGrowth.cpp
//---------------------------------------------------------------------------
#include "MichMenNegGrowth.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "GrowthOrg.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clMichMenNegGrowth::clMichMenNegGrowth( clSimManager * p_oSimManager ) :
  clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ), clGrowthBase( p_oSimManager )
{
  try
  {
    m_sNameString = "michmenneggrowthshell";
    m_sXMLRoot = "MichaelisMentenNegativeGrowth";

    m_iGrowthMethod = height_only;
    mp_fAlpha = NULL;
    mp_fBeta = NULL;
    mp_fPhi = NULL;
    mp_fGamma = NULL;
    mp_iIndexes = NULL;
    mp_iAutoCorrCodes = NULL;
    mp_fStdDev = NULL;
    mp_fProbAutoCorr = NULL;

    m_iNewTreeFloats += 1; //make sure we leave room for the growth data member

    m_iYearsPerTimestep = 0;
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
    stcErr.sFunction = "clMichMenNegGrowth::clMichMenNegGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////*/
clMichMenNegGrowth::~clMichMenNegGrowth()
{
  delete[] mp_fAlpha;
  delete[] mp_fBeta;
  delete[] mp_fPhi;
  delete[] mp_fGamma;
  delete[] mp_iIndexes;
  delete[] mp_fStdDev;
  delete[] mp_fProbAutoCorr;
  for (int i = 0; i < m_iNumBehaviorSpecies; i++)
    delete[] mp_iAutoCorrCodes[i];
  delete[] mp_iAutoCorrCodes;
}

//////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
/////////////////////////////////////////////////////////////////////////////*/
void clMichMenNegGrowth::RegisterTreeDataMembers() {
 try {
   clTreePopulation *p_oPop = (clTreePopulation *)mp_oSimManager->GetPopulationObject("treepopulation");
   short int iNumTypes = p_oPop->GetNumberOfTypes(), //number of unique types
             iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
             i, j;           //loop counters

   //Call the base class version as well
   clGrowthBase::RegisterTreeDataMembers();

   mp_iIndexes = new int[iNumTotalSpecies];
   for (i = 0; i < iNumTotalSpecies; i++) mp_iIndexes[i] = -1;

    //Set up the array indexes
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      mp_iIndexes[mp_iWhatSpecies[i]] = i;

   //Declare the temp. types array to be as big as the combo list to make
   //sure we have space for everything, and initialize values to -1
   mp_iAutoCorrCodes = new short int*[m_iNumBehaviorSpecies];
   for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_iAutoCorrCodes[i] = new short int[iNumTypes];
   for (i = 0; i < m_iNumBehaviorSpecies; i++)
     for (j = 0; j < iNumTypes; j++)
       mp_iAutoCorrCodes[i][j] = -1;


   //Register the variables for what's actually in our type/species
   //combos
   for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
     //Register the code and capture it
     mp_iAutoCorrCodes[mp_iIndexes[mp_whatSpeciesTypeCombos[i].iSpecies]]
                      [mp_whatSpeciesTypeCombos[i].iType] =
       p_oPop->RegisterFloat("autocorr", mp_whatSpeciesTypeCombos[i].iSpecies,
                            mp_whatSpeciesTypeCombos[i].iType);
   }
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clAbsoluteGrowth::RegisterTreeDataMembers";
   throw(stcErr);
 }
}

//////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
/////////////////////////////////////////////////////////////////////////////*/
void clMichMenNegGrowth::DoShellSetup( DOMDocument * p_oDoc )
{
  doubleVal * p_fTempValues = NULL; //for getting species-specific values
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    short int i;


    //Get number of years per timestep
    m_iYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();

    //Declare the arrays we'd like read
    mp_fAlpha = new double[m_iNumBehaviorSpecies];
    mp_fBeta = new double[m_iNumBehaviorSpecies];
    mp_fPhi = new double[m_iNumBehaviorSpecies];
    mp_fGamma = new double[m_iNumBehaviorSpecies];
    mp_fStdDev = new double[m_iNumBehaviorSpecies];
    mp_fProbAutoCorr = new double[m_iNumBehaviorSpecies];

    //Declare the species-specific temp array and pre-load with the species that
    //this behavior affects
    p_fTempValues = new doubleVal[m_iNumBehaviorSpecies];
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
      p_fTempValues[i].code = mp_iWhatSpecies[i];

    //Alpha
    FillSpeciesSpecificValue( p_oElement, "gr_mmNegGrowthAlpha",
        "gr_mmngaVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fAlpha[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Beta
    FillSpeciesSpecificValue( p_oElement, "gr_mmNegGrowthBeta",
        "gr_mmngbVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array while making sure none of them
    //are 0
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val == 0) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMichMenNegGrowth::DoShellSetup";
        stcErr.sMoreInfo = "Beta values cannot equal 0.";
        throw(stcErr);
      }
      mp_fBeta[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }


    //Gamma
    FillSpeciesSpecificValue( p_oElement, "gr_mmNegGrowthGamma",
        "gr_mmnggVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fGamma[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //Phi
    FillSpeciesSpecificValue( p_oElement, "gr_mmNegGrowthPhi",
        "gr_mmngpVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fPhi[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //standard deviation of growth stochasticity in cm/year
    FillSpeciesSpecificValue( p_oElement, "gr_mmNegGrowthStdDev",
        "gr_mmngsdVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      mp_fStdDev[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    //one year probability of autocorrelation
    FillSpeciesSpecificValue( p_oElement, "gr_mmNegGrowthAutoCorrProb",
        "gr_mmngacpVal", p_fTempValues, m_iNumBehaviorSpecies, p_oPop, true );
    //Transfer values to our permanent array
    for ( i = 0; i < m_iNumBehaviorSpecies; i++ ) {
      if (p_fTempValues[i].val < 0 || p_fTempValues[i].val > 1) {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clMichMenNegGrowth::DoShellSetup";
        stcErr.sMoreInfo = "Autocorrelation probability must be between 0 and 1.";
        throw(stcErr);
      }
      mp_fProbAutoCorr[mp_iIndexes[p_fTempValues[i].code]] = p_fTempValues[i].val;
    }

    delete[] p_fTempValues;

    //Make sure all species/type combos have "Light" registered
    for ( int i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      if ( -1 == mp_oGrowthOrg->GetLightCode(mp_whatSpeciesTypeCombos[i].iSpecies,
                                           mp_whatSpeciesTypeCombos[i].iType ) )
      {
        modelErr stcErr;
        stcErr.sFunction = "clRelativeGrowth::DoShellSetup";
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " does not have a required light behavior.";
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }
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
    stcErr.sFunction = "clMichMenNegGrowth::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalcHeightGrowthValue()
/////////////////////////////////////////////////////////////////////////////*/
float clMichMenNegGrowth::CalcHeightGrowthValue(clTree *p_oTree, clTreePopulation *p_oPop, float fDiameterGrowth) {
  float fAnnualRelGrowth, //amount of annual relative growth
  fGli, //tree's gli value
  fHeight, //tree's height value
  fNewHeight, //tree's new height value
  fStochasticFactor = 0, //amount by which to vary height each year
  fAmountHeightIncrease; //amount by which the tree's height will
  //increase
  short int iSpecies = p_oTree->GetSpecies(), iType = p_oTree->GetType(),
      iLightCode = mp_oGrowthOrg->GetLightCode( iSpecies, iType ),
      ind = mp_iIndexes[iSpecies],
      iYr;

  //Get the gli value for this tree
  p_oTree->GetValue( iLightCode, & fGli );

  //Get the height for this tree
  p_oTree->GetValue( p_oPop->GetHeightCode( iSpecies, iType ), & fHeight );
  fHeight *= 100.0; //transform to cm
  fNewHeight = fHeight;

  //Get the value of the Michaelis-Menton function
  fAnnualRelGrowth = (mp_fAlpha[ind] * fGli) /
      ((mp_fAlpha[ind]/mp_fBeta[ind]) + fGli);

  //If this is a species that autocorrelates, get the previous stochastic
  //growth factor
  if (mp_fStdDev[ind] > 0 ) {
    p_oTree->GetValue(mp_iAutoCorrCodes[ind][iType], &fStochasticFactor);
    //If it's zero, assume this is a new tree and draw a stochastic factor
    //right away to act as a previous one
    if (0 == fStochasticFactor) {
      fStochasticFactor = clModelMath::NormalRandomDraw(mp_fStdDev[ind]);
    }
  }

  //Compound the relative growth over the number of years/time step
  for (iYr = 0; iYr < m_iYearsPerTimestep; iYr++) {
    if (mp_fStdDev[ind] > 0) {
      if (clModelMath::GetRand() > mp_fProbAutoCorr[ind])
        //We are not autocorrelating - get a new stochastic value
        fStochasticFactor = clModelMath::NormalRandomDraw(mp_fStdDev[ind]);
    }
    fNewHeight += ((fAnnualRelGrowth * pow(fNewHeight, mp_fPhi[ind])) -
        mp_fGamma[ind]) + fStochasticFactor;
  }
  p_oTree->SetValue(mp_iAutoCorrCodes[ind][iType], fStochasticFactor);

  //Don't allow a negative new height - set to 10 cm
  if (fNewHeight <= 10.0) fNewHeight = 10.0;

  fAmountHeightIncrease = fNewHeight - fHeight;
  //Transform to m
  fAmountHeightIncrease /= 100.0;

  return fAmountHeightIncrease;
}
