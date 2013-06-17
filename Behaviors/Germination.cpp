//---------------------------------------------------------------------------
// Germination.cpp
//---------------------------------------------------------------------------
#include "Germination.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Grid.h"
#include "ParsingFunctions.h"
#include "ModelMath.h"
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clGermination::clGermination(clSimManager * p_oSimManager) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    m_sNameString = "Germination";
    m_sXMLRoot = "Germination";

    mp_fProportionGerminating = NULL;
    mp_iSeedGridCode = NULL;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_oSeedGrid = NULL;
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
    stcErr.sFunction = "clGermination::clGermination";
    throw(stcErr);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clGermination::~clGermination()
{
  delete[] mp_fProportionGerminating;
  delete[] mp_iSeedGridCode;
}


///////////////////////////////////////////////////////////////////////////////
// GetData
/////////////////////////////////////////////////////////////////////////////
void clGermination::GetData(DOMDocument * p_oDoc)
{
  try
  {

    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
    floatVal * p_fTemp = NULL; //for getting species-specific values
    std::stringstream sLabel;
    short int iNumSpecies = p_oPop->GetNumberOfSpecies(), i;

    //Declare our arrays
    mp_fProportionGerminating = new float[iNumSpecies];
    mp_iSeedGridCode = new short int[iNumSpecies];

    //Set up our temp array - pre-load with this behavior's species
    p_fTemp = new floatVal[m_iNumBehaviorSpecies];
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      p_fTemp[i].code = mp_iWhatSpecies[i];

    //Proportion germinating
    FillSpeciesSpecificValue(p_oElement, "ge_proportionGerminating", "ge_pgVal", p_fTemp, m_iNumBehaviorSpecies, p_oPop, true);
    //Now transfer the values to the permanent array
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
      mp_fProportionGerminating[p_fTemp[i].code] = p_fTemp[i].val;

    //Make sure that all values are between 0 and 1
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
    {
      if (mp_fProportionGerminating[mp_iWhatSpecies[i]] < 0 || mp_fProportionGerminating[mp_iWhatSpecies[i]] > 1)
      {
        modelErr stcErr;
        stcErr.sFunction = "clGermination::GetData";
        stcErr.sMoreInfo = "All values in proportion germinating must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        delete[] p_fTemp;
        throw(stcErr);
      }
    }

    //Fetch the seed grid
    mp_oSeedGrid = mp_oSimManager->GetGridObject("Dispersed Seeds");
    if (mp_oSeedGrid == NULL)
    {
      modelErr stcErr;
      stcErr.sFunction = "clGermination::GetData";
      stcErr.sMoreInfo = "Disperse behaviors must be used with germination.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      delete[] p_fTemp;
      throw(stcErr);
    }

    //Now get the data codes
    for (i = 0; i < m_iNumBehaviorSpecies; i++)
    {
      sLabel << "seeds_" << mp_iWhatSpecies[i];
      mp_iSeedGridCode[mp_iWhatSpecies[i]] = mp_oSeedGrid->GetFloatDataCode(sLabel.str());
      if (-1 == mp_iSeedGridCode[mp_iWhatSpecies[i]])
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clGermination::GetData";
        stcErr.sMoreInfo = "Unexpected grid data return code.";
        delete[] p_fTemp;
        throw(stcErr);
      }
      sLabel.str("");
    }

    delete[] p_fTemp;
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
    stcErr.sFunction = "clGermination::GetData";
    throw(stcErr);
  }
}



///////////////////////////////////////////////////////////////////////////////
// Action
///////////////////////////////////////////////////////////////////////////////
void clGermination::Action()
{
  clModelMath *p_oMath;
  float fNumSeeds;
  int iNumXCells = mp_oSeedGrid->GetNumberXCells(),
      iNumYCells = mp_oSeedGrid->GetNumberYCells(),
      iX, iY, i;

  for (iX = 0; iX < iNumXCells; iX++) {
    for (iY = 0; iY < iNumYCells; iY++) {

      //Go through the species
      for (i = 0; i < m_iNumBehaviorSpecies; i++) {

        mp_oSeedGrid->GetValueOfCell(iX, iY, mp_iSeedGridCode[mp_iWhatSpecies[i]], &fNumSeeds);

        fNumSeeds = p_oMath->RandomRound(fNumSeeds * mp_fProportionGerminating[mp_iWhatSpecies[i]]);

        mp_oSeedGrid->SetValueOfCell(iX, iY, mp_iSeedGridCode[mp_iWhatSpecies[i]], fNumSeeds);
      }
    }
  }
}
