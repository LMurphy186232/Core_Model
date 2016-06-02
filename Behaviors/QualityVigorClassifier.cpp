#include "QualityVigorClassifier.h"
#include "TreePopulation.h"
#include "ModelMath.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clQualityVigorClassifier::clQualityVigorClassifier(clSimManager *p_oSimManager) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ){

  m_sNameString = "QualityVigorClassifier";
  m_sXMLRoot = "QualityVigorClassifier";

  m_iNewTreeInts = 1;
  m_iNewTreeBools = 2;

  //Versions
  m_fVersionNumber = 1;
  m_fMinimumVersionNumber = 1;

  //Allowed file types
  m_iNumAllowedTypes = 2;
  mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
  mp_iAllowedFileTypes[0] = parfile;
  mp_iAllowedFileTypes[1] = detailed_output;

  mp_fVigBeta0 = NULL;
  mp_fVigBeta11 = NULL;
  mp_fVigBeta12 = NULL;
  mp_fVigBeta13 = NULL;
  mp_fVigBeta14 = NULL;
  mp_fVigBeta15 = NULL;
  mp_fVigBeta16 = NULL;
  mp_fVigBeta2 = NULL;
  mp_fVigBeta3 = NULL;
  mp_fQualBeta0 = NULL;
  mp_fQualBeta11 = NULL;
  mp_fQualBeta12 = NULL;
  mp_fQualBeta13 = NULL;
  mp_fQualBeta14 = NULL;
  mp_fQualBeta2 = NULL;
  mp_fQualBeta3 = NULL;
  mp_fNewAdultProbVigorous = NULL;
  mp_fNewAdultProbSawlog = NULL;
  mp_iIndexes = NULL;
  mp_iVigorousInd = NULL;
  mp_iSawlogInd = NULL;
  mp_iTreeclassInd = NULL;
  mp_bDeciduous = NULL;
  m_cQuery = NULL;

}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clQualityVigorClassifier::~clQualityVigorClassifier() {
  delete[] mp_fVigBeta0;
  delete[] mp_fVigBeta11;
  delete[] mp_fVigBeta12;
  delete[] mp_fVigBeta13;
  delete[] mp_fVigBeta14;
  delete[] mp_fVigBeta15;
  delete[] mp_fVigBeta16;
  delete[] mp_fVigBeta2;
  delete[] mp_fVigBeta3;
  delete[] mp_fQualBeta0;
  delete[] mp_fQualBeta11;
  delete[] mp_fQualBeta12;
  delete[] mp_fQualBeta13;
  delete[] mp_fQualBeta14;
  delete[] mp_fQualBeta2;
  delete[] mp_fQualBeta3;
  delete[] mp_fNewAdultProbVigorous;
  delete[] mp_fNewAdultProbSawlog;
  delete[] mp_iVigorousInd;
  delete[] mp_iSawlogInd;
  delete[] mp_iTreeclassInd;
  delete[] mp_iIndexes;
  delete[] mp_bDeciduous;
  delete[] m_cQuery;
}

//////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
/////////////////////////////////////////////////////////////////////////////*/
void clQualityVigorClassifier::RegisterTreeDataMembers() {
 try {
   clTreePopulation *p_oPop = (clTreePopulation *)mp_oSimManager->GetPopulationObject("treepopulation");
   short int iNumTotalSpecies = p_oPop->GetNumberOfSpecies(),
             i;              //loop counters

   //If any of the types is not adult, error out
   for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
     if ( clTreePopulation::adult != mp_whatSpeciesTypeCombos[i].iType )
     {
       modelErr stcErr;
       stcErr.iErrorCode = BAD_DATA;
       stcErr.sFunction = "clQualityVigorClassifier::RegisterTreeDataMembers" ;
       stcErr.sMoreInfo = "This behavior can only be applied to adults.";
       throw( stcErr );
     }

   //Make the list of indexes
   mp_iIndexes = new short int[iNumTotalSpecies];
   for ( i = 0; i < iNumTotalSpecies; i++ ) mp_iIndexes[i] = -1;
   for ( i = 0; i < m_iNumBehaviorSpecies; i++ )
     mp_iIndexes[mp_iWhatSpecies[i]] = i;

   mp_iVigorousInd = new short int[m_iNumBehaviorSpecies];
   mp_iSawlogInd = new short int[m_iNumBehaviorSpecies];
   mp_iTreeclassInd = new short int[m_iNumBehaviorSpecies];

   for (i = 0; i < m_iNumBehaviorSpecies; i++) {
     mp_iVigorousInd[i] = -1;
     mp_iSawlogInd[i] = -1;
     mp_iTreeclassInd[i] = -1;
   }

   //Register the variables
   for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {

     mp_iVigorousInd[i] =
            p_oPop->RegisterBool("vigorous",
                                 mp_whatSpeciesTypeCombos[i].iSpecies,
                                 mp_whatSpeciesTypeCombos[i].iType);

     mp_iSawlogInd[i] =
            p_oPop->RegisterBool("sawlog",
                                 mp_whatSpeciesTypeCombos[i].iSpecies,
                                 mp_whatSpeciesTypeCombos[i].iType);

     mp_iTreeclassInd[i] =
            p_oPop->RegisterInt("treeclass",
                                 mp_whatSpeciesTypeCombos[i].iSpecies,
                                 mp_whatSpeciesTypeCombos[i].iType);
   }
 }
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clQualityVigorClassifier::RegisterTreeDataMembers";
   throw(stcErr);
 }
}

///////////////////////////////////////////////////////////////////////////////
// GetData
///////////////////////////////////////////////////////////////////////////////
void clQualityVigorClassifier::GetData(DOMDocument * p_oDoc)
{
  clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
  FormatQueryString(p_oPop);
  ReadParameterFile(p_oDoc, p_oPop);
  HandleInitialConditionsTrees(p_oDoc, p_oPop);
}
///////////////////////////////////////////////////////////////////////////////
// ReadParameterFile
///////////////////////////////////////////////////////////////////////////////
void clQualityVigorClassifier::ReadParameterFile(DOMDocument * p_oDoc, clTreePopulation *p_oPop)
{
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  boolVal *p_bTemp = NULL;
  try
  {
    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    short int i;

    //Declare our arrays
    mp_fVigBeta0 = new double[m_iNumBehaviorSpecies];
    mp_fVigBeta11 = new double[m_iNumBehaviorSpecies];
    mp_fVigBeta12 = new double[m_iNumBehaviorSpecies];
    mp_fVigBeta13 = new double[m_iNumBehaviorSpecies];
    mp_fVigBeta14 = new double[m_iNumBehaviorSpecies];
    mp_fVigBeta15 = new double[m_iNumBehaviorSpecies];
    mp_fVigBeta16 = new double[m_iNumBehaviorSpecies];
    mp_fVigBeta2 = new double[m_iNumBehaviorSpecies];
    mp_fVigBeta3 = new double[m_iNumBehaviorSpecies];
    mp_fQualBeta0 = new double[m_iNumBehaviorSpecies];
    mp_fQualBeta11 = new double[m_iNumBehaviorSpecies];
    mp_fQualBeta12 = new double[m_iNumBehaviorSpecies];
    mp_fQualBeta13 = new double[m_iNumBehaviorSpecies];
    mp_fQualBeta14 = new double[m_iNumBehaviorSpecies];
    mp_fQualBeta2 = new double[m_iNumBehaviorSpecies];
    mp_fQualBeta3 = new double[m_iNumBehaviorSpecies];
    mp_fNewAdultProbVigorous = new double[m_iNumBehaviorSpecies];
    mp_fNewAdultProbSawlog = new double[m_iNumBehaviorSpecies];
    mp_bDeciduous = new bool[m_iNumBehaviorSpecies];

    //Set up our temp array - pre-load with this behavior's species
    p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
    p_bTemp = new boolVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++) {
      p_fTemp[i].code = mp_iWhatSpecies[i];
      p_bTemp[i].code = mp_iWhatSpecies[i];
    }

    //Beta0 for vigor transition
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta0", "ma_cvb0Val",
        p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta0[i] = p_fTemp[i].val;

    //Beta1 for vigor transition, initial class 1
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta11",
        "ma_cvb11Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta11[i] = p_fTemp[i].val;

    //Beta1 for vigor transition, initial class 2
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta12",
        "ma_cvb12Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta12[i] = p_fTemp[i].val;

    //Beta1 for vigor transition, initial class 3
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta13",
        "ma_cvb13Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta13[i] = p_fTemp[i].val;

    //Beta1 for vigor transition, initial class 4
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta14",
        "ma_cvb14Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta14[i] = p_fTemp[i].val;

    //Beta1 for vigor transition, initial class 5
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta15",
        "ma_cvb15Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta15[i] = p_fTemp[i].val;

    //Beta1 for vigor transition, initial class 6
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta16",
        "ma_cvb16Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta16[i] = p_fTemp[i].val;

    //Beta2 for vigor transition
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta2",
        "ma_cvb2Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta2[i] = p_fTemp[i].val;

    //Beta3 for vigor transition
    FillSpeciesSpecificValue(p_oElement, "ma_classifierVigBeta3",
        "ma_cvb3Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fVigBeta3[i] = p_fTemp[i].val;

    //Beta0 for quality transition
    FillSpeciesSpecificValue(p_oElement, "ma_classifierQualBeta0",
        "ma_cqb0Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fQualBeta0[i] = p_fTemp[i].val;

    //Beta1 for quality transition, initial class 1
    FillSpeciesSpecificValue(p_oElement, "ma_classifierQualBeta11",
        "ma_cqb11Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fQualBeta11[i] = p_fTemp[i].val;

    //Beta1 for quality transition, initial class 2
    FillSpeciesSpecificValue(p_oElement, "ma_classifierQualBeta12",
        "ma_cqb12Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fQualBeta12[i] = p_fTemp[i].val;

    //Beta1 for quality transition, initial class 3
    FillSpeciesSpecificValue(p_oElement, "ma_classifierQualBeta13",
        "ma_cqb13Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fQualBeta13[i] = p_fTemp[i].val;

    //Beta1 for quality transition, initial class 4
    FillSpeciesSpecificValue(p_oElement, "ma_classifierQualBeta14",
        "ma_cqb14Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fQualBeta14[i] = p_fTemp[i].val;

    //Beta2 for quality transition
    FillSpeciesSpecificValue(p_oElement, "ma_classifierQualBeta2",
        "ma_cqb2Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fQualBeta2[i] = p_fTemp[i].val;

    //Beta3 for quality transition
    FillSpeciesSpecificValue(p_oElement, "ma_classifierQualBeta3",
        "ma_cqb3Val", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fQualBeta3[i] = p_fTemp[i].val;

    //Probability of new adults being vigorous
    FillSpeciesSpecificValue(p_oElement, "ma_classifierNewAdultProbVigorous",
        "ma_cnapvVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fNewAdultProbVigorous[i] = p_fTemp[i].val;

    //Probability of new adults being sawlog quality
    FillSpeciesSpecificValue(p_oElement, "ma_classifierNewAdultProbSawlog",
        "ma_cnapsVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fNewAdultProbSawlog[i] = p_fTemp[i].val;

    //Whether or not a species is deciduous. (False means coniferous.)
    FillSpeciesSpecificValue(p_oElement, "ma_classifierDeciduous", "ma_cdVal",
        p_bTemp, m_iNumBehaviorSpecies, p_oPop, true);
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_bDeciduous[i] = p_bTemp[i].val;

    delete[] p_fTemp;
    delete[] p_bTemp;
  }
  catch (modelErr& err)
  {
    delete[] p_fTemp;
    delete[] p_bTemp;
    throw(err);
  }
  catch (modelMsg & msg)
  {
    delete[] p_fTemp;
    delete[] p_bTemp;
    throw(msg);
  } //non-fatal error
  catch (...)
  {
    delete[] p_fTemp;
    delete[] p_bTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGermination::GetData";
    throw(stcErr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// AssignClass
///////////////////////////////////////////////////////////////////////////////
void clQualityVigorClassifier::AssignClass(clTreePopulation *p_oPop, clTree *p_oTree) {
  float fDBH;
  int iSpInd = mp_iIndexes[p_oTree->GetSpecies()];
  int iClass = 0;
  bool bVigorous, bSawlog;

  //Get vigor and quality
  p_oTree->GetValue(mp_iVigorousInd[iSpInd], &bVigorous);

  if (mp_bDeciduous[iSpInd]) {
    p_oTree->GetValue(mp_iSawlogInd[iSpInd], &bSawlog);
  } else bSawlog = false; //no quality for conifers

  if (mp_bDeciduous[iSpInd]) {
    if (bVigorous) {
      if (bSawlog) iClass = 1;
      else iClass = 2;
    } else {
      if (bSawlog) {
        p_oTree->GetValue(p_oPop->GetDbhCode(p_oTree->GetSpecies(), p_oTree->GetType()), &fDBH);
        if (fDBH > 23) iClass = 3;
        else iClass = 4;
      } else iClass = 4;
    }
  } else {
    if (bVigorous) iClass = 5;
    else iClass = 6;
  }

  p_oTree->SetValue(mp_iTreeclassInd[iSpInd], iClass);
}

///////////////////////////////////////////////////////////////////////////////
// AssignVigorQuality
///////////////////////////////////////////////////////////////////////////////
void clQualityVigorClassifier::AssignVigorQuality(clTree *p_oTree) {
  int iSpInd = mp_iIndexes[p_oTree->GetSpecies()];
  bool bVal;

  bVal = clModelMath::GetRand() <= mp_fNewAdultProbVigorous[iSpInd];
  p_oTree->SetValue(mp_iVigorousInd[iSpInd], bVal);

  if (mp_bDeciduous[iSpInd]) {
    bVal = clModelMath::GetRand() <= mp_fNewAdultProbSawlog[iSpInd];
    p_oTree->SetValue(mp_iSawlogInd[iSpInd], bVal);
  }
}

///////////////////////////////////////////////////////////////////////////////
// EvolveVigorQuality
///////////////////////////////////////////////////////////////////////////////
void clQualityVigorClassifier::EvolveVigorQuality(clTreePopulation *p_oPop, clTree *p_oTree) {
  float fDBH, fBeta1, fBetaX, fProb;
  int iSpInd = mp_iIndexes[p_oTree->GetSpecies()];
  int iClass = 0;
  bool bVal;

  p_oTree->GetValue(p_oPop->GetDbhCode(p_oTree->GetSpecies(), p_oTree->GetType()), &fDBH);

  //Get initial class
  p_oTree->GetValue(mp_iTreeclassInd[iSpInd], &iClass);

  // *** Vigor ***
  //Figure out which beta 1 to use
  switch (iClass) {
    case 1: fBeta1 = mp_fVigBeta11[iSpInd]; break;
    case 2: fBeta1 = mp_fVigBeta12[iSpInd]; break;
    case 3: fBeta1 = mp_fVigBeta13[iSpInd]; break;
    case 4: fBeta1 = mp_fVigBeta14[iSpInd]; break;
    case 5: fBeta1 = mp_fVigBeta15[iSpInd]; break;
    case 6: fBeta1 = mp_fVigBeta16[iSpInd]; break;
    default:
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clQualityVigorClassifier::EvolveVigorQuality" ;
      stcErr.sMoreInfo = "Unexpected class number.";
      throw( stcErr );
  }
  fBetaX = mp_fVigBeta0[iSpInd] + fBeta1 + mp_fVigBeta2[iSpInd]*fDBH +
  mp_fVigBeta3[iSpInd]*log(fDBH);

  fProb = exp(fBetaX) / (1 + exp(fBetaX));
  bVal = clModelMath::GetRand() <= fProb;
  p_oTree->SetValue(mp_iVigorousInd[iSpInd], bVal);

  // *** Quality ***
  if (mp_bDeciduous[iSpInd]) {
    //Figure out which beta 1 to use
    switch (iClass) {
    case 1: fBeta1 = mp_fQualBeta11[iSpInd]; break;
    case 2: fBeta1 = mp_fQualBeta12[iSpInd]; break;
    case 3: fBeta1 = mp_fQualBeta13[iSpInd]; break;
    case 4: fBeta1 = mp_fQualBeta14[iSpInd]; break;
    default:
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clQualityVigorClassifier::EvolveVigorQuality" ;
      stcErr.sMoreInfo = "Unexpected class number.";
      throw( stcErr );
    }
    fBetaX = mp_fQualBeta0[iSpInd] + fBeta1 + mp_fQualBeta2[iSpInd]*fDBH +
    mp_fQualBeta3[iSpInd]*log(fDBH);

    fProb = exp(fBetaX) / (1 + exp(fBetaX));
    bVal = clModelMath::GetRand() <= fProb;
    p_oTree->SetValue(mp_iSawlogInd[iSpInd], bVal);
  }
}

////////////////////////////////////////////////////////////////////////////
// FormatQueryString()
////////////////////////////////////////////////////////////////////////////
void clQualityVigorClassifier::FormatQueryString(clTreePopulation *p_oPop)
{
  char * cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];
  char cQueryPiece[20]; //for assembling the search query
  int i;

  //Do a type/species search on all the species plus adults
  strcpy( cQueryTemp, "species=" );
  for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
  {
    sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
    strcat( cQueryTemp, cQueryPiece );
  }
  sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
  strcat( cQueryTemp, cQueryPiece );

  sprintf( cQueryPiece, "::type=%d", clTreePopulation::adult);
  strcat( cQueryTemp, cQueryPiece );

  //Now put it in m_cQuery, sized correctly
  m_cQuery = new char[strlen( cQueryTemp ) + 1];
  strcpy( m_cQuery, cQueryTemp );
  delete[] cQueryTemp;
}


///////////////////////////////////////////////////////////////////////////////
// Action
///////////////////////////////////////////////////////////////////////////////
void clQualityVigorClassifier::Action() {
  clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
  clTreeSearch *p_oBehaviorTrees = p_oPop->Find( m_cQuery );
  clTree * p_oTree = p_oBehaviorTrees->NextTree();
  int iClass, iSpInd;

  while (p_oTree) {
    iSpInd = mp_iIndexes[p_oTree->GetSpecies()];

    //Get the current class number
    p_oTree->GetValue(mp_iTreeclassInd[iSpInd], &iClass);

    if (iClass == 0) {
      //New adult - assign vigor and quality
      AssignVigorQuality(p_oTree);
    } else {
      //Evolve vigor and quality
      EvolveVigorQuality(p_oPop, p_oTree);
    }

    //Assign new class
    AssignClass(p_oPop, p_oTree);

    p_oTree = p_oBehaviorTrees->NextTree();
  }
}

///////////////////////////////////////////////////////////////////////////////
// HandleInitialConditionsTrees
///////////////////////////////////////////////////////////////////////////////
void clQualityVigorClassifier::HandleInitialConditionsTrees(DOMDocument * p_oDoc, clTreePopulation *p_oPop)
{
  doubleVal * p_fTemp = NULL; //for getting species-specific values
  float **p_fPropVig = NULL, **p_fPropSaw = NULL;
  float *p_fMinDBH = NULL, *p_fMaxDBH = NULL;
  try
  {
    using namespace std;
    DOMNodeList * p_oNodeList;
    DOMNode * p_oDocNode;
    DOMElement * p_oElement;
    XMLCh *sVal;
    float fDBH;
    double fTemp; //workhorse variable
    unsigned short int iNumClasses, //for counting one species's size classes
    i, j; //loop counters
    bool bVal, bFound;

    //Look for initial density data
    sVal = XMLString::transcode( "ma_classifierSizeClass" );
    p_oNodeList = p_oDoc->getElementsByTagName( sVal );
    XMLString::release(&sVal);
    if ( 0 == p_oNodeList->getLength() ) {
      //None - set things up to use new adult values
      iNumClasses = 1;
      p_fMinDBH = new float[iNumClasses];
      p_fMaxDBH = new float[iNumClasses];
      p_fMinDBH[0] = 0;
      p_fMaxDBH[0] = 100000;
      p_fPropVig = new float*[iNumClasses];
      p_fPropSaw = new float*[iNumClasses];
      p_fPropVig[0] = new float[m_iNumBehaviorSpecies];
      p_fPropSaw[0] = new float[m_iNumBehaviorSpecies];
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        p_fPropVig[0][i] = mp_fNewAdultProbVigorous[i];
        p_fPropSaw[0][i] = mp_fNewAdultProbSawlog[i];
      }


    } else {
      iNumClasses = p_oNodeList->getLength();
      p_fMinDBH = new float[iNumClasses];
      p_fMaxDBH = new float[iNumClasses];
      p_fPropVig = new float*[iNumClasses];
      p_fPropSaw = new float*[iNumClasses];
      for (i = 0; i < iNumClasses; i++) {
        p_fPropVig[i] = new float[m_iNumBehaviorSpecies];
        p_fPropSaw[i] = new float[m_iNumBehaviorSpecies];
      }

      //Set up our temp array - pre-load with this behavior's species
      p_fTemp = new doubleVal[m_iNumBehaviorSpecies];
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {
        p_fTemp[i].code = mp_iWhatSpecies[i];
      }

      //Start parsin'
      for (i = 0; i < iNumClasses; i++) {
        p_oDocNode = p_oNodeList->item( i );
        p_oElement = ( DOMElement * ) p_oDocNode;

        FillSingleValue(p_oElement, "ma_classifierBeginDBH", &fTemp, true);
        p_fMinDBH[i] = fTemp;

        FillSingleValue(p_oElement, "ma_classifierEndDBH", &fTemp, true);
        p_fMaxDBH[i] = fTemp;

        FillSpeciesSpecificValue(p_oElement, "ma_classifierProbVigorous",
            "ma_cpvVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
        for (j = 0; j < m_iNumBehaviorSpecies; j++)
          p_fPropVig[i][j] = p_fTemp[j].val;

        FillSpeciesSpecificValue(p_oElement, "ma_classifierProbSawlog",
            "ma_cpsVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
        for (j = 0; j < m_iNumBehaviorSpecies; j++)
          p_fPropSaw[i][j] = p_fTemp[j].val;
      }
    }

    //Do the initial conditions trees
    clTreeSearch *p_oBehaviorTrees = p_oPop->Find( m_cQuery );
    clTree * p_oTree = p_oBehaviorTrees->NextTree();
    int iClass, iSpInd;

    while (p_oTree) {
      iSpInd = mp_iIndexes[p_oTree->GetSpecies()];

      //Get the current class number
      p_oTree->GetValue(mp_iTreeclassInd[iSpInd], &iClass);

      if (iClass == 0) {
        //Unassigned - assign vigor and quality
        //Get DBH and find size class
        bFound = false;
        p_oTree->GetValue(
            p_oPop->GetDbhCode(p_oTree->GetSpecies(), p_oTree->GetType()),
            &fDBH);
        for (i = 0; i < iNumClasses; i++) {
          if (fDBH > p_fMinDBH[i] && fDBH <= p_fMaxDBH[i]) {
            //Vigor:
            bVal = clModelMath::GetRand() <= p_fPropVig[i][iSpInd];
            p_oTree->SetValue(mp_iVigorousInd[iSpInd], bVal);

            //Sawlog:
            if (mp_bDeciduous[iSpInd]) {
              bVal = clModelMath::GetRand() <= p_fPropSaw[i][iSpInd];
              p_oTree->SetValue(mp_iSawlogInd[iSpInd], bVal);
            }
            bFound = true;
            break;
          }
        }
        if (!bFound) {
          //Vigor:
          bVal = clModelMath::GetRand() <= mp_fNewAdultProbVigorous[iSpInd];
          p_oTree->SetValue(mp_iVigorousInd[iSpInd], bVal);

          //Sawlog:
          if (mp_bDeciduous[iSpInd]) {
            bVal = clModelMath::GetRand() <= mp_fNewAdultProbSawlog[iSpInd];
            p_oTree->SetValue(mp_iSawlogInd[iSpInd], bVal);
          }
        }

        //Assign new class
        AssignClass(p_oPop, p_oTree);
      }

      p_oTree = p_oBehaviorTrees->NextTree();
    }

    delete[] p_fTemp;
    if (p_fPropVig) {
      for (i = 0; i < iNumClasses; i++) delete[] p_fPropVig[i];
    }
    if (p_fPropSaw) {
      for (i = 0; i < iNumClasses; i++) delete[] p_fPropSaw[i];
    }
    delete[] p_fPropVig;
    delete[] p_fPropSaw;
    delete[] p_fMinDBH;
    delete[] p_fMaxDBH;

  } //end of try block
  catch ( modelErr & err )
  {
    int i;
    delete[] p_fTemp;
    if (p_fPropVig) {
      for (i = 0; i < m_iNumBehaviorSpecies; i++) delete[] p_fPropVig[i];
    }
    if (p_fPropSaw) {
      for (i = 0; i < m_iNumBehaviorSpecies; i++) delete[] p_fPropSaw[i];
    }
    delete[] p_fPropVig;
    delete[] p_fPropSaw;
    delete[] p_fMinDBH;
    delete[] p_fMaxDBH;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    int i;
    delete[] p_fTemp;
    if (p_fPropVig) {
      for (i = 0; i < m_iNumBehaviorSpecies; i++) delete[] p_fPropVig[i];
    }
    if (p_fPropSaw) {
      for (i = 0; i < m_iNumBehaviorSpecies; i++) delete[] p_fPropSaw[i];
    }
    delete[] p_fPropVig;
    delete[] p_fPropSaw;
    delete[] p_fMinDBH;
    delete[] p_fMaxDBH;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clTreePopulation::CreateTreesFromInitialDensities" ;
    throw( stcErr );
  }
}
