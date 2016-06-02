//---------------------------------------------------------------------------
#include "DisperseOrg.h"
#include "TreePopulation.h"
#include "DisperseBase.h"
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include <stdio.h>
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clDisperseOrg::clDisperseOrg(clDisperseBase * p_oHookedShell)
{
  try
  {
    //Initialize variables to 0, NULL, etc.
    mp_oSeedsGrid = NULL;
    mp_oDispObjects = NULL;
    mp_fRandParameter = NULL;
    mp_iCodes = NULL;
    m_iNumDispObjects = 0;

   Adjust = NULL;
   m_iSeedDistributionMethod = deterministic_pdf;
   m_iTotalSpecies = 0;

    //Make sure the hooked shell object pointer isn't NULL - if it is throw
    //error
    if (NULL == p_oHookedShell)
    {
      modelErr stcErr;
      stcErr.sFunction = "clDisperseOrg::clDisperseOrg";
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw(stcErr);
    }

    //Otherwise - set the flag on the shell object that it's hooked
    p_oHookedShell->m_bHooked = true;
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
    stcErr.sFunction = "clDisperseOrg::clDisperseOrg";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clDisperseOrg::~clDisperseOrg()
{
  delete[] mp_oDispObjects;
  delete[] mp_fRandParameter;
  delete[] mp_iCodes;
}


//////////////////////////////////////////////////////////////////////////////
// DoSetup
/////////////////////////////////////////////////////////////////////////////
void clDisperseOrg::DoSetup(clSimManager * p_oSimManager, xercesc::DOMDocument * p_oDoc)
{
  try
  {

    //Double-check - if either pointer passed is NULL, throw an error
    if (NULL == p_oSimManager || NULL == p_oDoc)
    {
      modelErr stcErr;
      stcErr.sFunction = "clDisperseOrg::DoSetup";
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw(stcErr);
    }

    clTreePopulation * p_oPop = (clTreePopulation *) p_oSimManager->GetPopulationObject("treepopulation");
    m_iTotalSpecies = p_oPop->GetNumberOfSpecies();

    GetDisperseObjects(p_oSimManager);
    GetParameterFileData(p_oSimManager, p_oDoc);
    SetFunctionPointer();
    GetSeedGridObject(p_oSimManager);
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
    stcErr.sFunction = "clDisperseOrg::DoSetup";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetSeedGridObject
/////////////////////////////////////////////////////////////////////////////
void clDisperseOrg::GetSeedGridObject(clSimManager * p_oSimManager)
{
  std::stringstream sLabel;
  short int i;

  mp_oSeedsGrid = p_oSimManager->GetGridObject("Dispersed Seeds");

  if (NULL == mp_oSeedsGrid)
  {
    modelErr stcErr;
    stcErr.sFunction = "clDisperseOrg::GetSeedGridObject";
    stcErr.sMoreInfo = "Can't find grid \"Dispersed Seeds\".";
    stcErr.iErrorCode = CANT_FIND_OBJECT;
    throw(stcErr);
  }

  //Get the codes for each species
  mp_iCodes = new short int[m_iTotalSpecies];
  for (i = 0; i < m_iTotalSpecies; i++)
  {
    sLabel << "seeds_" << i;
    mp_iCodes[i] = mp_oSeedsGrid->GetFloatDataCode(sLabel.str());
    if (-1 == mp_iCodes[i])
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clDisperseOrg::GetSeedGridObject";
      stcErr.sMoreInfo = "Unexpected grid data return code.";
      throw(stcErr);
    }
    sLabel.str("");
  }
}


//////////////////////////////////////////////////////////////////////////////
// SetFunctionPointer()
/////////////////////////////////////////////////////////////////////////////
void clDisperseOrg::SetFunctionPointer()
{
  if (deterministic_pdf == m_iSeedDistributionMethod)
  {
    Adjust = & clDisperseOrg::DeterministicAdjust;
  }
  else if (poisson_pdf == m_iSeedDistributionMethod)
  {
    Adjust = & clDisperseOrg::PoissonAdjust;
  }
  else if (lognormal_pdf == m_iSeedDistributionMethod)
  {
    Adjust = & clDisperseOrg::LognormalAdjust;
  }
  else if (normal_pdf == m_iSeedDistributionMethod)
  {
    Adjust = & clDisperseOrg::NormalAdjust;
  }
  else if (negative_binomial_pdf == m_iSeedDistributionMethod)
  {
    Adjust = & clDisperseOrg::NegativeBinomialAdjust;
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetParameterFileData()
/////////////////////////////////////////////////////////////////////////////
void clDisperseOrg::GetParameterFileData(clSimManager * p_oSimManager, xercesc::DOMDocument * p_oDoc)
{
  try
  {
    DOMElement * p_oElement = p_oDoc->getDocumentElement();
    clPopulationBase * p_oTempPop = p_oSimManager->GetPopulationObject("treepopulation");
    clTreePopulation * p_oPop = (clTreePopulation *) p_oTempPop;
    doubleVal * p_fTemp; //for getting values from parameter file
    bool * p_bSpeciesUsed; //for getting values from the parameter file
    int iTemp = deterministic_pdf;
    short int i, j; //loop counters

    //Get the type of seedling adjustment desired
    FillSingleValue(p_oElement, "di_seedDistributionMethod", & iTemp, false);

    //Make sure the value is valid
    if (deterministic_pdf == iTemp)
    {
      m_iSeedDistributionMethod = deterministic_pdf;
    }
    else if (poisson_pdf == iTemp)
    {
      m_iSeedDistributionMethod = poisson_pdf;
    }
    else if (lognormal_pdf == iTemp)
    {
      m_iSeedDistributionMethod = lognormal_pdf;
    }
    else if (normal_pdf == iTemp)
    {
      m_iSeedDistributionMethod = normal_pdf;
    }
    else if (negative_binomial_pdf == iTemp)
    {
      m_iSeedDistributionMethod = negative_binomial_pdf;
    }
    else
    {
      modelErr stcErr;
      stcErr.sFunction = "clDisperseOrg::GetParameterFileData";
      std::stringstream s;
      s << "Unrecognized value for di_seedDistributionMethod: " << iTemp;
      stcErr.sMoreInfo = s.str();
      stcErr.iErrorCode = BAD_DATA;
      throw(stcErr);
    }

    //If lognormal, normal, or negative binomial, populate the parameters array
    if (lognormal_pdf == m_iSeedDistributionMethod ||
        normal_pdf == m_iSeedDistributionMethod ||
        negative_binomial_pdf == m_iSeedDistributionMethod)
    {
      mp_fRandParameter = new double[m_iTotalSpecies];

      //Only populate those species that use disperse - compile the list
      p_bSpeciesUsed = new bool[m_iTotalSpecies];
      for (i = 0; i < m_iTotalSpecies; i++)
      {
        p_bSpeciesUsed[i] = false;
      }

      for (i = 0; i < m_iNumDispObjects; i++)
      {
        for (j = 0; j < mp_oDispObjects[i]->GetNumBehaviorSpecies(); j++)
        {
          p_bSpeciesUsed[mp_oDispObjects[i]->GetBehaviorSpecies(j)] = true;
        }
      }

      //How many was that?
      iTemp = 0;
      for (i = 0; i < m_iTotalSpecies; i++)
      {
        if (p_bSpeciesUsed[i]) iTemp++;
      }

      p_fTemp = new doubleVal[iTemp];
      iTemp = 0;
      for (i = 0; i < m_iTotalSpecies; i++)
      {
        if (p_bSpeciesUsed[i])
        {
          p_fTemp[iTemp].code = i;
          iTemp++;
        }
      }

      if (negative_binomial_pdf == m_iSeedDistributionMethod)
      {
        FillSpeciesSpecificValue(p_oElement, "di_clumpingParameter", "di_cpVal", p_fTemp, iTemp, p_oPop, true);
      }
      else
      {
        FillSpeciesSpecificValue(p_oElement, "di_standardDeviation", "di_sdVal", p_fTemp, iTemp, p_oPop, true);
      }

      for (i = 0; i < iTemp; i++)
      {
        mp_fRandParameter[p_fTemp[i].code] = p_fTemp[i].val;
      }

      delete[] p_bSpeciesUsed;
      delete[] p_fTemp;
    }
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
    stcErr.sFunction = "clDisperseOrg::DoSetup";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetDisperseObjects()
/////////////////////////////////////////////////////////////////////////////
void clDisperseOrg::GetDisperseObjects(clSimManager * p_oSimManager)
{
  try
  {
    clBehaviorBase * p_oTempBehavior; //for going through behaviors looking for
    //growth shells
    std::string sBehaviorName, //behavior namestrings
           sShellMarker1 = "disperse",
           sShellMarker2 = "Disperse";
    short int iNumBehaviors = p_oSimManager->GetNumberOfBehaviors(), iTemp, i; //loop counters

    //Go through and count the number of disperse behaviors
    m_iNumDispObjects = 0;
    for (i = 0; i < iNumBehaviors; i++)
    {
      p_oTempBehavior = p_oSimManager->GetBehaviorObject(i);
      sBehaviorName = p_oTempBehavior->GetName();
      if (std::string::npos != sBehaviorName.find(sShellMarker1) ||
          std::string::npos != sBehaviorName.find(sShellMarker2)) {
        m_iNumDispObjects++;
      }
    }

    //Declare the list
    mp_oDispObjects = new clDisperseBase * [m_iNumDispObjects];
    for (i = 0; i < m_iNumDispObjects; i++)
    {
      mp_oDispObjects[i] = NULL;
    }

    //Now go through and get all the disperse behaviors
    iTemp = 0;
    for (i = 0; i < iNumBehaviors; i++)
    {
      p_oTempBehavior = p_oSimManager->GetBehaviorObject(i);
      sBehaviorName = p_oTempBehavior->GetName();
      if (std::string::npos != sBehaviorName.find(sShellMarker1) ||
          std::string::npos != sBehaviorName.find(sShellMarker2)) {
        mp_oDispObjects[iTemp] = dynamic_cast < clDisperseBase * > (p_oTempBehavior);
        iTemp++;
      }
    }
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
    stcErr.sFunction = "clDisperseOrg::PopulateDisperseFunctionsTable";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// AdjustSeedNumbers()
/////////////////////////////////////////////////////////////////////////////
void clDisperseOrg::AdjustSeedNumbers()
{
  try
  {
    float fNumSeeds=0;
    int iNumXCells = mp_oSeedsGrid->GetNumberXCells(),
    iNumYCells = mp_oSeedsGrid->GetNumberYCells(), iX, iY, iSp;

    //Loop through all cells and all species and adjust each
    for (iX = 0; iX < iNumXCells; iX++)
    {
      for (iY = 0; iY < iNumYCells; iY++)
      {
        for (iSp = 0; iSp < m_iTotalSpecies; iSp++)
        {
          mp_oSeedsGrid->GetValueOfCell(iX, iY, mp_iCodes[iSp], & fNumSeeds);
          if (fNumSeeds > 0)
          {
            mp_oSeedsGrid->SetValueOfCell(iX, iY, mp_iCodes[iSp], (* this.*Adjust) (fNumSeeds, iSp));
          }
        }
      }
    }
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
    stcErr.sFunction = "clDisperseOrg::AdjustSeedNumbers";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// DeterministicAdjust()
/////////////////////////////////////////////////////////////////////////////
float clDisperseOrg::DeterministicAdjust(float fNumber, int iSpecies)
{
  //return clModelMath::RandomRound(fNumber);
  return fNumber;
}


//////////////////////////////////////////////////////////////////////////////
// NormalAdjust()
/////////////////////////////////////////////////////////////////////////////
float clDisperseOrg::NormalAdjust(float fNumber, int iSpecies)
{
  //return floor(fNumber + clModelMath::NormalRandomDraw(mp_fRandParameter[iSpecies]));
  return fNumber + clModelMath::NormalRandomDraw(mp_fRandParameter[iSpecies]);
}


//////////////////////////////////////////////////////////////////////////////
// LognormalAdjust()
/////////////////////////////////////////////////////////////////////////////
float clDisperseOrg::LognormalAdjust(float fNumber, int iSpecies)
{
  //return floor(clModelMath::LognormalRandomDraw(fNumber, mp_fRandParameter[iSpecies]));
  return clModelMath::LognormalRandomDraw(fNumber, mp_fRandParameter[iSpecies]);
}


//////////////////////////////////////////////////////////////////////////////
// PoissonAdjust()
/////////////////////////////////////////////////////////////////////////////
float clDisperseOrg::PoissonAdjust(float fNumber, int iSpecies)
{
  return clModelMath::PoissonRandomDraw(fNumber);
}


//////////////////////////////////////////////////////////////////////////////
// NegativeBinomialAdjust()
/////////////////////////////////////////////////////////////////////////////
float clDisperseOrg::NegativeBinomialAdjust(float fNumber, int iSpecies)
{
  return clModelMath::NegBinomialRandomDraw(fNumber, mp_fRandParameter[iSpecies]);
}


//////////////////////////////////////////////////////////////////////////////
// DoDisperse()
/////////////////////////////////////////////////////////////////////////////
void clDisperseOrg::DoDisperse()
{
  try
  {
    int i;

    //Go through and call AddSeeds for each disperse object
    for (i = 0; i < m_iNumDispObjects; i++) {
      mp_oDispObjects[i]->AddSeeds();
    }

    //Adjust them
    AdjustSeedNumbers();
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
    stcErr.sFunction = "clDisperseOrg::DoDisperse";
    throw(stcErr);
  }
}
