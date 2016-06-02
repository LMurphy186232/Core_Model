//---------------------------------------------------------------------------
#include "LightFilter.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "ModelMath.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clLightFilter::clLightFilter(clSimManager * p_oSimManager) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the namestring
    m_sNameString = "LightFilter";
    m_sXMLRoot = "LightFilter";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Indicate that this behavior intends to add two tree int data members
    m_iNewTreeInts = 2;

    mp_iLightCodes = NULL;
    mp_iZCodes = NULL;
    mp_iCounterCodes = NULL;
    mp_iHeightCodes = NULL;

    m_iFilterHeight = 0;
    m_iNumSpecies = 0;
    m_fLightExtinctionCoefficient = 0;
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
    stcErr.sFunction = "clLightFilter::clLightFilter";
    throw(stcErr);
  }
}



//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clLightFilter::~clLightFilter()
{

  int i;

  if (mp_iLightCodes)
  {
    for (i = 0; i < m_iNumSpecies; i++)
    {
      delete[] mp_iLightCodes[i];
    }
    delete[] mp_iLightCodes;
    mp_iLightCodes = NULL;
  }

  if (mp_iZCodes)
  {
    for (i = 0; i < m_iNumSpecies; i++)
    {
      delete[] mp_iZCodes[i];
    }
    delete[] mp_iZCodes;
    mp_iZCodes = NULL;
  }

  if (mp_iCounterCodes)
  {
    for (i = 0; i < m_iNumSpecies; i++)
    {
      delete[] mp_iCounterCodes[i];
    }
    delete[] mp_iCounterCodes;
    mp_iCounterCodes = NULL;
  }

  if (mp_iHeightCodes)
  {
    for (i = 0; i < m_iNumSpecies; i++)
    {
      delete[] mp_iHeightCodes[i];
    }
    delete[] mp_iHeightCodes;
    mp_iCounterCodes = NULL;
  }

}



/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////

void clLightFilter::GetData(xercesc::DOMDocument * p_oDoc)
{
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);
    clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
    const char * cCounterLabel = "lf_count",
         * cZLabel = "z",
         * cLightLabel = "Light";
    double fTemp;
    short int iNumTypes = p_oPop->GetNumberOfTypes(), i, j;

    m_iNumSpecies = p_oPop->GetNumberOfSpecies();

    //Declare the data member code arrays
    mp_iCounterCodes = new short int * [m_iNumSpecies];
    mp_iZCodes = new short int * [m_iNumSpecies];
    mp_iLightCodes = new short int * [m_iNumSpecies];
    mp_iHeightCodes = new short int * [m_iNumSpecies];

    for (i = 0; i < m_iNumSpecies; i++)
    {

      mp_iCounterCodes[i] = new short int[iNumTypes];
      mp_iZCodes[i] = new short int[iNumTypes];
      mp_iLightCodes[i] = new short int[iNumTypes];
      mp_iHeightCodes[i] = new short int[iNumTypes];

      //Initialize all values to -1
      for (j = 0; j < iNumTypes; j++)
      {

        mp_iCounterCodes[i] [j] = -1;
        mp_iZCodes[i] [j] = -1;
        mp_iLightCodes[i] [j] = -1;
        mp_iHeightCodes[i] [j] = -1;

      }
    }

    //Get the parameter file values

    //Light extinction coefficient
    FillSingleValue(p_oElement, "lf_lightExtinctionCoefficient", & m_fLightExtinctionCoefficient, true);

    //Convert to 1/mm units from 1/m units
    m_fLightExtinctionCoefficient /= 1000;

    //Height of fern layer
    FillSingleValue(p_oElement, "lf_heightOfFilter", & fTemp, true);

    //Convert the height of fern layer to mm - I'm willfully int casting and
    //throwing away sub-mm precision
    m_iFilterHeight = (int)(fTemp * 1000);

    //Register the data members
    for (i = 0; i < m_iNumSpeciesTypeCombos; i++) {
      mp_iCounterCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] =
           p_oPop->RegisterInt(cCounterLabel, mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType);

      mp_iZCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] =
           p_oPop->RegisterInt(cZLabel, mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType);

      //Get the codes for the "Light" and "Height" data member
      mp_iLightCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] =
           p_oPop->GetFloatDataCode(cLightLabel, mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType);


      mp_iHeightCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] =
           p_oPop->GetHeightCode(mp_whatSpeciesTypeCombos[i].iSpecies, mp_whatSpeciesTypeCombos[i].iType);

      //If the return code for Light is -1, throw an error
      if (-1 == mp_iLightCodes[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType]) {
        modelErr stcErr;
        stcErr.sFunction = "clLightFilter::GetData";
        std::stringstream s;
        s << "Type/species combo species="
          << mp_whatSpeciesTypeCombos[i].iSpecies << " type="
          << mp_whatSpeciesTypeCombos[i].iType
          << " does not have a light behavior compatible with light filtering.";
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
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
    stcErr.sFunction = "clLightFilter::GetData";
    throw(stcErr);
  }
}



/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clLightFilter::Action()
{
  try
  {
    clTreePopulation * p_oPop = (clTreePopulation *) mp_oSimManager->GetPopulationObject("treepopulation");
    clTreeSearch * p_oAllTrees; //search object for accessing all trees
    clTree * p_oTree; //one individual tree to work with
    float fLightVal, //the value in the Light data member
         fHeight, //tree's height
         fPercentTransmission; //percentage of light transmitted
    int iCounter, //tree's respite counter value
        iNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(),
         iEffectiveHeight, //tree height plus z, in mm
         iPathLength, //path length of light transmission = thickness
         //of filter, in mm
         iZ, //tree's z value (in mm)
         iFilterHeight; //randomized filter height
    short int iSp, iTp; //species and type of a tree

    //Access all trees one at a time and see to whom this applies.
    //We could also do a species/type search, but I decided to do it this way
    //because I think it will be marginally faster.
    p_oAllTrees = p_oPop->Find("all");

    p_oTree = p_oAllTrees->NextTree();
    while (p_oTree)
    {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();

      if (mp_iZCodes[iSp] [iTp] == -1) goto nextTree;

      //Get the counter value
      p_oTree->GetValue(mp_iCounterCodes[iSp] [iTp], & iCounter);
      if (iCounter > 0)
      {

        //Tree is still in respite - decrement the counter by the number of
        //years per timestep
        iCounter -= iNumYearsPerTimestep;
        if (iCounter < 0) iCounter = 0;
        p_oTree->SetValue(mp_iCounterCodes[iSp] [iTp], iCounter);

        goto nextTree;
      }

      //Get the current Z value
      p_oTree->GetValue(mp_iZCodes[iSp] [iTp], & iZ);

      //Get the current height value
      p_oTree->GetValue(mp_iHeightCodes[iSp] [iTp], & fHeight);

      //Calculate the tree's effective height
      iEffectiveHeight = (int)(fHeight * 1000) + iZ;

      //Slightly randomize the filter height
      iFilterHeight = (int)(m_iFilterHeight * (0.9995 + (0.001 * clModelMath::GetRand())));

      //If above the filter height, exit out
      if (iEffectiveHeight >= iFilterHeight) goto nextTree;

      //Get the current light value
      p_oTree->GetValue(mp_iLightCodes[iSp] [iTp], & fLightVal);

      //Calculate the thickness of the filter which is the path length of
      //light transmission
      iPathLength = iFilterHeight - iEffectiveHeight;

      //Calculate the percentage of light transmitted
      fPercentTransmission = exp(-iPathLength * m_fLightExtinctionCoefficient);

      //Multiply this by GLI and put back in the tree
      fLightVal *= fPercentTransmission;

      p_oTree->SetValue(mp_iLightCodes[iSp] [iTp], fLightVal);

    nextTree:
      p_oTree = p_oAllTrees->NextTree();
    } //end of while (p_oTree)

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
    stcErr.sFunction = "clLightFilter::Action";
    throw(stcErr);
  }
}
