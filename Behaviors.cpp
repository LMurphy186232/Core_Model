//---------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include "Behaviors.h"
#include "BehaviorBase.h"
#include "SimManager.h"
#include "TreePopulation.h"
//HEADER FILES FOR REGISTERED BEHAVIOR OBJECTS
#include "Output.h"
#include "SailLight.h"
#include "GLILight.h"
#include "QuadratGLILight.h"
#include "LightFilter.h"
#include "AbsoluteGrowth.h"
#include "RelativeGrowth.h"
#include "ConstantRadialGrowth.h"
#include "ConstantBAGrowth.h"
#include "NCIMasterGrowth.h"
#include "LogisticGrowth.h"
#include "BCMort.h"
#include "GMFMort.h"
#include "StochasticMort.h"
#include "SenescenceMort.h"
#include "SelfThinMort.h"
#include "TreeRemover.h"
#include "Substrate.h"
#include "SpatialDisperse.h"
#include "NonSpatialDisperse.h"
#include "Disturbance.h"
#include "Planting.h"
#include "Establishment.h"
#include "Germination.h"
#include "OutputShort.h"
#include "FuncResponseSeedPredation.h"
#include "MicroEstablishment.h"
#include "AllometricGrowthIncrementer.h"
#include "SimpleLinearGrowth.h"
#include "ShadedLinearGrowth.h"
#include "SizeDepLogisticGrowth.h"
#include "LognormalGrowth.h"
#include "WeibullSnagMort.h"
#include "SelectionHarvest.h" //(JM)
#include "NCIMasterMortality.h"
#include "StormDamageApplier.h"
#include "Storm.h"
#include "VolumeCalculator.h"
#include "GLIMap.h"
#include "DimensionAnalysis.h"
#include "GapLight.h"
#include "DoubleMMRelGrowth.h"
#include "ResourceMortality.h"
#include "StochasticGapGrowth.h"
#include "CompetitionMort.h"
#include "DensitySelfThinning.h"
#include "BoleVolumeCalculator.h"
#include "DensitySeedSurvival.h"
#include "LightDepSeedSurvival.h"
#include "SubstrateDepSeedSurvival.h"
#include "StormLight.h"
#include "LinearBiLevelGrowth.h"
#include "LogisticBiLevelMortality.h"
#include "LogBiLevelGrowth.h"
#include "PRSemiStochGrowth.h"
#include "StochasticBiLevelMortality.h"
#include "GLIPoints.h"
#include "TreeAgeCalculator.h"
#include "BasalAreaLight.h"
#include "StormKiller.h"
#include "HeightGLIWeibullMortality.h"
#include "PRStormBiLevelGrowth.h"
#include "MerchValueCalculator.h"
#include "CarbonValueCalculator.h"
#include "Windstorm.h"
#include "ExpResourceMortality.h"
#include "AggregatedMortality.h"
#include "ConstantGLI.h"
#include "AverageLight.h"
#include "HarvestInterface.h"
#include "PartitionedBiomass.h"
#include "SnagDecomp.h"
#include "RandomBrowse.h"
#include "BrowsedRelativeGrowth.h"
#include "BrowsedStochasticMortality.h"
#include "MastingSpatialDisperse.h"
#include "StormKilledPartitionedBiomass.h"
#include "NeighborhoodSeedPredation.h"
#include "CompetitionHarvest.h"
#include "RipleysKCalculator.h"
#include "ConditOmegaCalculator.h"
#include "DetailedSubstrate.h"
#include "StormDirectKiller.h"
#include "MastingNonSpatialDisperse.h"
#include "MichMenNegGrowth.h"
#include "MichMenPhotoinhibition.h"
#include "FoliarChemistry.h"
#include "PowerHeightGrowth.h"
#include "PostHarvestSkiddingMort.h"
#include "LaggedPostHarvestGrowth.h"
#include "EpiphyticEstablishment.h"
#include "DensitySelfThinningGompertz.h"
#include "TempDependentNeighborhoodDisperse.h"
#include "TempDependentNeighborhoodSurvival.h"
#include "ClimateChange.h"
#include "InsectInfestation.h"
#include "InsectInfestationMortality.h"
#include "StateReporter.h"
#include "GeneralizedHarvestRegime.h"
#include "SuppressionDurationMort.h"
#include "QualityVigorClassifier.h"
#include "DensDepInfestation.h"
#include "ConspecificBANeighborhoodDisperse.h"
#include "NCIMasterQuadratGrowth.h"
#include "SeasonalWaterDeficit.h"
#include "ClimateCompDepNeighborhoodSurvival.h"
#include "StochDoubleLogTempDepNeighDisperse.h"
#include "SizeDependentLogisticMortality.h"
//Test objects
#include "RandomSeedLogger.h"

using namespace xercesc;

///////////////////////////////////////////////////////////////////////////
// CreateObjects()
///////////////////////////////////////////////////////////////////////////
void clBehaviorManager::CreateObjects(xercesc::DOMDocument * p_oDoc)
{
  try
  {
    clPopulationBase * p_oTempObject = NULL;
    clTreePopulation * p_oTreePop = NULL; //for querying species
    struct behaviorData data; //data structure to pass to the
    //behavior manager to create the behavior objects
    DOMElement * p_oElement, //Element-type nodes in the document tree
    * p_oNameElement;
    DOMNode * p_oBehavior, * p_oBehaviorName, * p_oDocNode;
    DOMNodeList * p_oNodeList, //Node lists - used to grab all children of a node
    * p_oNodeList2, * p_oBehaviorChildren;
    XMLCh *sVal;
    char * cData; //for getting text data
    int iNumNodes, //used to count nodes
    iNumCombos, //used to loop through the species list
    i, j, k; //loop counters
    short int iSpeciesCode, //returned species code
    iType; //type code translated from text
    bool bDuplicate = false; //used for sniffing out duplicate species

    //Initialize the data structure
    data.iNumCombos = 0;
    data.p_whatCombos = NULL;

    //Have we already been set up?  If so, clear memory
    if (m_iNumObjects > 0) FreeMemory();

    // Get the list of behaviors so we can process them one at a time
    sVal = XMLString::transcode("behaviorList");
    p_oNodeList = p_oDoc->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    if (0 == p_oNodeList->getLength())
    {
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
      stcErr.sMoreInfo = "behaviorList";
      throw(stcErr);
    }
    p_oDocNode = p_oNodeList->item(0); //now we have the behavior list element
    p_oElement = (DOMElement *) p_oDocNode; //cast to DOM_Element object
    sVal = XMLString::transcode("behavior");
    //p_oNodeList now has the list of "behavior" elements
    p_oNodeList = p_oElement->getElementsByTagName(sVal);
    XMLString::release(&sVal);
    iNumNodes = p_oNodeList->getLength();

    if (0 == iNumNodes)
    { //no behaviors were found
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
      stcErr.sMoreInfo = "behavior";
      throw(stcErr);
    }

    //Get the tree population object
    p_oTempObject = mp_oSimManager->GetPopulationObject("treepopulation");
    if (NULL == p_oTempObject)
    {
      modelErr stcErr;
      stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      stcErr.sMoreInfo = "clTreePopulation";
      throw(stcErr);
    } //error condition
    p_oTreePop = (clTreePopulation *) p_oTempObject;

    // For each behavior, create it
    for (i = 0; i < iNumNodes; i++)
    {

      //Get the behavior name string
      p_oBehavior = p_oNodeList->item(i); //has ith "behavior" element
      p_oElement = (DOMElement *) p_oBehavior; //cast to DOM_Element object
      sVal = XMLString::transcode("behaviorName");
      p_oNodeList2 = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 == p_oNodeList2->getLength())
      {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
        std::stringstream s;
        s << "behaviorName - behavior #" << i;
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
      p_oBehaviorName = p_oNodeList2->item(0); //oBehaviorName holds the
      //"behaviorName" child of the ith "behavior" element
      //The name string is in the first child of the "behaviorName" node
      cData = XMLString::transcode(p_oBehaviorName->getChildNodes()->item(0)->getNodeValue());
      data.sNameString = cData;
      delete[] cData; cData = NULL;

      //Get the version number
      sVal = XMLString::transcode("version");
      p_oNodeList2 = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 == p_oNodeList2->getLength())
      {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
        std::stringstream s;
        s << "version - behavior #" << i;
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
      p_oBehaviorName = p_oNodeList2->item(0); //reusing this variable
      cData = XMLString::transcode(p_oBehaviorName->getChildNodes()->item(0)->getNodeValue());
      data.fVersion = atof(cData);
      if (0 == data.fVersion && strcmp(cData, "0") != 0)
      {
        //the version wasn't a number - throw an error
        delete[] cData;
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
        stcErr.sMoreInfo = "Version is not a number.";
        throw(stcErr);
      }
      delete[] cData; cData = NULL;

      //Get the behavior list number
      sVal = XMLString::transcode("listPosition");
      p_oNodeList2 = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      if (0 == p_oNodeList2->getLength()) {
        modelErr stcErr;
        stcErr.iErrorCode = DATA_MISSING;
        stcErr.sFunction = "clBehaviorManager::CreateObjects";
        std::stringstream s;
        s << "list position - behavior #" << i;
        stcErr.sMoreInfo = s.str();
        throw(stcErr);
      }
      p_oBehaviorName = p_oNodeList2->item(0); //reusing this variable
      cData = XMLString::transcode(p_oBehaviorName->getChildNodes()->item(0)->getNodeValue());
      data.iBehaviorListNumber = atoi(cData);
      if (0 == data.iBehaviorListNumber && strcmp(cData, "0") != 0) {
        //the version wasn't a number - throw an error
        delete[] cData;
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clBehaviorManager::CreateObjects";
        stcErr.sMoreInfo = "Behavior list is not a number.";
        throw(stcErr);
      }
      delete[] cData; cData = NULL;

      //Count the number of species/type combos - there may be none
      sVal = XMLString::transcode("applyTo");
      p_oBehaviorChildren = p_oElement->getElementsByTagName(sVal);
      XMLString::release(&sVal);
      iNumCombos = p_oBehaviorChildren->getLength();
      data.iNumCombos = iNumCombos;

      //Get the list of species/type combos if there are any
      //Allocate the array of species/type codes and pre-set the values to -1 -
      //that way if there are bad values or duplicates we will know at the end
      if (0 != iNumCombos)
      {
        data.p_whatCombos = new struct stcSpeciesTypeCombo[iNumCombos];
        for (j = 0; j < iNumCombos; j++)
        {
          data.p_whatCombos[j].iSpecies = -1;
          data.p_whatCombos[j].iType = -1;
        }
        for (j = 0; j < iNumCombos; j++)
        {
          p_oBehaviorName = p_oBehaviorChildren->item(j);
          p_oNameElement = (DOMElement *) p_oBehaviorName;

          //Get the species code from the attribute "species"
          sVal = XMLString::transcode("species");
          cData = XMLString::transcode(p_oNameElement->getAttributeNode(sVal)->getValue());
          XMLString::release(&sVal);
          iSpeciesCode = p_oTreePop->TranslateSpeciesNameToCode(cData);
          delete[] cData; cData = NULL;

          if (-1 == iSpeciesCode)
          { //bad species - throw error
            modelErr stcErr;
            stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
            stcErr.iErrorCode = BAD_DATA;
            std::stringstream s;
            s << "Invalid species code for behavior # " << i;
            stcErr.sMoreInfo = s.str();
            throw(stcErr);
          }

          //Get the type code from the attribute "type"
          sVal = XMLString::transcode("type");
          cData = XMLString::transcode(p_oNameElement->getAttributeNode(sVal)->getValue());
          XMLString::release(&sVal);

          //translate the type to a number
          if (strcmp(cData, "Seed") == 0)
            iType = clTreePopulation::seed;
          else if (strcmp(cData, "Seedling") == 0)
            iType = clTreePopulation::seedling;
          else if (strcmp(cData, "Sapling") == 0)
            iType = clTreePopulation::sapling;
          else if (strcmp(cData, "Adult") == 0)
            iType = clTreePopulation::adult;
          else if (strcmp(cData, "Stump") == 0)
            iType = clTreePopulation::stump;
          else if (strcmp(cData, "Snag") == 0)
            iType = clTreePopulation::snag;
          else if (strcmp(cData, "Woody_debris") == 0)
            iType = clTreePopulation::woody_debris;
          else
          { //unrecognized type - throw error
            modelErr stcErr;
            stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
            stcErr.iErrorCode = BAD_DATA;
            stcErr.sMoreInfo = "Unrecognized type: ";
            stcErr.sMoreInfo += cData;
            delete[] cData;
            throw(stcErr);
          }
          delete[] cData; cData = NULL;

          //We have valid species and type codes - check for a duplicate combo
          bDuplicate = false;
          for (k = 0; k < j; k++)
            if (iSpeciesCode == data.p_whatCombos[k].iSpecies && iType == data.p_whatCombos[k].iType)
              bDuplicate = true;
          if (!bDuplicate)
          { //our value is valid and not a duplicate
            data.p_whatCombos[j].iSpecies = iSpeciesCode;
            data.p_whatCombos[j].iType = iType;
          }
          else
            data.iNumCombos--; //duplicate - decrease the count by one

        } //end of for (j = 0; j < iNumCombos; j++)
      } //end of if (0 != iNumCombos)


      //Create behavior and pass type/species data
      CreateBehavior(& data);

      //Re-initialize the data structure
      data.iNumCombos = 0;
      if (data.p_whatCombos)
      {
        delete[] data.p_whatCombos; data.p_whatCombos = NULL;
      }
      data.sNameString = "";

    } //end of for (i = 0; i < iNumNodes; i++)

    delete[] cData;
  } //end of try block
  catch (modelErr & err)
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
    stcErr.sFunction = "clBehaviorManager::CreateObjects" ;
    throw(stcErr);
  }
}

////////////////////////////////////////////////////////////////////////////
// CreateBehavior()
/////////////////////////////////////////////////////////////////////////////
void clBehaviorManager::CreateBehavior(behaviorData * p_data)
{
  try
  {
    clWorkerBase * * p_oTempObjectArray = NULL; //in case we need to increase the
    //size of the array
    clBehaviorBase * p_oBehavior; //for casting our object to clBehaviorBase
    short int iIndex = -1, //place in the array to create the new behavior
        iReturnCode, //for testing the version number
        i; //loop counter

    //Check to make sure we have a name - if not throw an error
    if (p_data->sNameString.length() == 0)
    {
      modelErr stcErr;
      stcErr.sFunction = "clBehaviorManager::CreateBehavior" ;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sMoreInfo = "namestring";
      throw(stcErr);
    }

    //************
    // This next section was originally a good idea; but I forgot about it and
    //went on to divorce the object namestring from the parameter file call
    //string. Which was stupid. Which caused a bug that caused me to need
    //to comment this out.
    //************

    //Determine the next open pointer in the behavior array - if this is a
    //new definition for an existing object then delete the old object - the
    //index will be set to the "hole" created
    /*    for (i = 0; i < m_iNumObjects; i++)
      if (strcmp(p_data->sNameString, mp_oObjectArray[i]->GetName()) == 0)
      {
        delete mp_oObjectArray[i];
      }
    for (i = 0; i < m_iNumObjects; i++)
      if (!mp_oObjectArray[i])
      {
        iIndex = i;
        break;
      }*/

    //Check to see if we found an open spot in the array - if we didn't increase
    //the size of the array by one
    if (-1 == iIndex)
    {
      //make a copy of the array
      p_oTempObjectArray = new clWorkerBase * [m_iNumObjects];
      for (i = 0; i < m_iNumObjects; i++)
        p_oTempObjectArray[i] = mp_oObjectArray[i];
      //resize the object array
      delete[] mp_oObjectArray;
      m_iNumObjects++;
      mp_oObjectArray = new clWorkerBase * [m_iNumObjects];
      //copy the old array back in
      for (i = 0; i < m_iNumObjects - 1; i++)
        mp_oObjectArray[i] = p_oTempObjectArray[i];
      delete[] p_oTempObjectArray;
      iIndex = m_iNumObjects - 1;
      mp_oObjectArray[iIndex] = NULL;
    } //end of if (-1 == iIndex)

    //Figure out what type of object we are supposed to create, and create the
    //appropriate one

    //*************************************************
    // State Change Behaviors
    //*************************************************
    if (p_data->sNameString.compare("TemperatureClimateChange") == 0 ||
        p_data->sNameString.compare("PrecipitationClimateChange") == 0)
    {
      clClimateChange * p_oObj = new clClimateChange(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;

    }
    else if (p_data->sNameString.compare("SeasonalWaterDeficit") == 0)
    {
      clSeasonalWaterDeficit * p_oObj = new clSeasonalWaterDeficit(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;

    }

    //*************************************************
    // Light Behaviors
    //*************************************************
    else if (p_data->sNameString.compare("SailLight") == 0)
    {
      clSailLight * p_oLight = new clSailLight(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;

    }
    else if (p_data->sNameString.compare("GLILight") == 0)
    {
      clGliLight * p_oLight = new clGliLight(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;

    }
    else if (p_data->sNameString.compare("QuadratLight") == 0)
    {
      clQuadratGLILight * p_oLight = new clQuadratGLILight(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;

    }
    else if (p_data->sNameString.compare("LightFilter") == 0)
    {
      clLightFilter * p_oLight = new clLightFilter(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;
    }
    else if (p_data->sNameString.compare("GLIMapCreator") == 0)
    {
      clGLIMap * p_oLight = new clGLIMap(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;
    }
    else if (p_data->sNameString.compare("GLIPointCreator") == 0)
    {
      clGLIPoints * p_oLight = new clGLIPoints(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;
    }
    else if (p_data->sNameString.compare("GapLight") == 0)
    {
      clGapLight * p_oLight = new clGapLight(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;
    }
    else if (p_data->sNameString.compare("StormLight") == 0)
    {
      clStormLight * p_oLight = new clStormLight(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;
    }
    else if (p_data->sNameString.compare("BasalAreaLight") == 0)
    {
      clBasalAreaLight * p_oLight = new clBasalAreaLight(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;
    }
    else if (p_data->sNameString.compare("ConstantGLI") == 0)
    {
      clConstantGLI * p_oLight = new clConstantGLI(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;
    }
    else if (p_data->sNameString.compare("AverageLight") == 0)
    {
      clAverageLight * p_oLight = new clAverageLight(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oLight;
    }

    //*************************************************
    // Growth Behaviors
    //*************************************************

    else if (p_data->sNameString.compare("AbsRadialGrowth") == 0 ||
        p_data->sNameString.compare("AbsBAGrowth") == 0 ||
        p_data->sNameString.compare("AbsUnlimGrowth") == 0 ||
        p_data->sNameString.compare("AbsRadialGrowth diam only") == 0 ||
        p_data->sNameString.compare("AbsBAGrowth diam only") == 0 ||
        p_data->sNameString.compare("AbsUnlimGrowth diam only") == 0)
    {
      clAbsoluteGrowth * p_oGrowth = new clAbsoluteGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("RelRadialGrowth") == 0 ||
        p_data->sNameString.compare("RelBAGrowth") == 0 ||
        p_data->sNameString.compare("RelUnlimGrowth") == 0 ||
        p_data->sNameString.compare("RelRadialGrowth diam only") == 0 ||
        p_data->sNameString.compare("RelBAGrowth diam only") == 0 ||
        p_data->sNameString.compare("RelUnlimGrowth diam only") == 0 ||
        p_data->sNameString.compare("RelativeHeight") == 0)
    {
      clRelativeGrowth * p_oGrowth = new clRelativeGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("LogisticGrowth diam only") == 0 ||
        p_data->sNameString.compare("LogisticGrowth") == 0 ||
        p_data->sNameString.compare("LogisticGrowth height only") == 0)
    {
      clLogisticGrowth * p_oGrowth = new clLogisticGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("LognormalGrowth diam only") == 0 ||
        p_data->sNameString.compare("LognormalGrowth") == 0 ||
        p_data->sNameString.compare("LognormalGrowth height only") == 0)
    {
      clLognormalGrowth * p_oGrowth = new clLognormalGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("SizeDependentLogisticGrowth diam only") == 0
        || p_data->sNameString.compare("SizeDependentLogisticGrowth") == 0
        || p_data->sNameString.compare("SizeDependentLogisticGrowth height only") == 0)
    {
      clSizeDepLogisticGrowth * p_oGrowth = new clSizeDepLogisticGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("SimpleLinearGrowth diam only") == 0
        || p_data->sNameString.compare("SimpleLinearGrowth") == 0
        || p_data->sNameString.compare("SimpleLinearGrowth height only") == 0)
    {
      clSimpleLinearGrowth * p_oGrowth = new clSimpleLinearGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("ShadedLinearGrowth diam only") == 0
        || p_data->sNameString.compare("ShadedLinearGrowth") == 0
        || p_data->sNameString.compare("ShadedLinearGrowth height only") == 0)
    {
      clShadedLinearGrowth * p_oGrowth = new clShadedLinearGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("LinearBilevelGrowth diam only") == 0
        || p_data->sNameString.compare("LinearBilevelGrowth") == 0)
    {
      clLinearBiLevelGrowth * p_oGrowth = new clLinearBiLevelGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("NCIMasterGrowth") == 0 ||
        p_data->sNameString.compare("NCIMasterGrowth diam only") == 0)
    {
      clNCIMasterGrowth * p_oGrowth = new clNCIMasterGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;

    }
    else if (p_data->sNameString.compare("NCIMasterQuadratGrowth") == 0 ||
        p_data->sNameString.compare("NCIMasterQuadratGrowth diam only") == 0)
    {
      clNCIMasterQuadratGrowth * p_oGrowth = new clNCIMasterQuadratGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;

    }
    else if (p_data->sNameString.compare("ConstRadialGrowth") == 0 ||
        p_data->sNameString.compare("ConstRadialGrowth diam only") == 0)
    {
      clConstantRadialGrowth * p_oGrowth = new clConstantRadialGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("ConstBAGrowth") == 0 ||
        p_data->sNameString.compare("ConstBAGrowth diam only") == 0)
    {
      clConstantBAGrowth * p_oGrowth = new clConstantBAGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("HeightIncrementer") == 0 ||
        p_data->sNameString.compare("DiameterIncrementer") == 0)
    {
      clAllometricGrowthIncrementer * p_oGrowth = new clAllometricGrowthIncrementer(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("DoubleResourceRelative") == 0
        || p_data->sNameString.compare("DoubleResourceRelative diam only") == 0)
    {
      clDoubleMMRelGrowth * p_oGrowth = new clDoubleMMRelGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("StochasticGapGrowth") == 0)
    {
      clStochasticGapGrowth * p_oGrowth = new clStochasticGapGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("LogBilevelGrowth height only") == 0)
    {
      clLogBiLevelGrowth * p_oGrowth = new clLogBiLevelGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("PRSemiStochastic diam only") == 0)
    {
      clPRSemiStochGrowth * p_oGrowth = new clPRSemiStochGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("PRStormBilevelGrowth") == 0)
    {
      clPRStormBiLevelGrowth * p_oGrowth = new clPRStormBiLevelGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("BrowsedRelativeGrowth") == 0 ||
        p_data->sNameString.compare("BrowsedRelativeGrowth diam only") == 0)
    {
      clBrowsedRelativeGrowth * p_oGrowth = new clBrowsedRelativeGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("MichaelisMentenNegativeGrowth height only") == 0)
    {
      clMichMenNegGrowth * p_oGrowth = new clMichMenNegGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("MichaelisMentenPhotoinhibitionGrowth height only") == 0)
    {
      clMichMenPhotoinhibition * p_oGrowth = new clMichMenPhotoinhibition(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("PowerGrowth height only") == 0)
    {
      clPowerHeightGrowth * p_oGrowth = new clPowerHeightGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }
    else if (p_data->sNameString.compare("LaggedPostHarvestGrowth") == 0 ||
        p_data->sNameString.compare("LaggedPostHarvestGrowth diam only") == 0)
    {
      clLaggedPostHarvestGrowth * p_oGrowth = new clLaggedPostHarvestGrowth(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGrowth;
    }

    //*************************************************
    // Mortality Behaviors
    //*************************************************

    else if (p_data->sNameString.compare("BCMortality") == 0)
    {
      clBCMort * p_oMort = new clBCMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("GMFMortality") == 0)
    {
      clGMFMort * p_oMort = new clGMFMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("SelfThinning") == 0)
    {
      clSelfThinMort * p_oMort = new clSelfThinMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("Senescence") == 0)
    {
      clSenescenceMort * p_oMort = new clSenescenceMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("StochasticMortality") == 0)
    {
      clStochasticMort * p_oMort = new clStochasticMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("WeibullSnagMortality") == 0)
    {
      clWeibullSnagMort * p_oMort = new clWeibullSnagMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("NCIMasterMortality") == 0)
    {
      clNCIMasterMortality * p_oMort = new clNCIMasterMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("GrowthResourceMortality") == 0)
    {
      clResourceMortality * p_oMort = new clResourceMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("CompetitionMortality") == 0)
    {
      clCompetitionMort * p_oMort = new clCompetitionMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("DensitySelfThinning") == 0)
    {
      clDensitySelfThinning * p_oMort = new clDensitySelfThinning(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("LogisticBiLevelMortality") == 0)
    {
      clLogisticBiLevelMortality * p_oMort = new clLogisticBiLevelMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("StochasticBiLevelMortality") == 0 ||
        p_data->sNameString.compare("StochasticBiLevelMortality - GLI") == 0)
    {
      clStochasticBiLevelMortality * p_oMort = new clStochasticBiLevelMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("HeightGLIWeibullMortality") == 0)
    {
      clHeightGLIWeibullMortality * p_oMort = new clHeightGLIWeibullMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("ExponentialGrowthResourceMortality") == 0)
    {
      clExpResourceMortality * p_oMort = new clExpResourceMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("AggregatedMortality") == 0)
    {
      clAggregatedMortality * p_oMort = new clAggregatedMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("SnagDecayClassDynamics") == 0)
    {
      clSnagDecomp * p_oMort = new clSnagDecomp(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("BrowsedStochasticMortality") == 0)
    {
      clBrowsedStochasticMortality * p_oMort = new clBrowsedStochasticMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("PostHarvestSkiddingMortality") == 0)
    {
      clPostHarvestSkiddingMort * p_oMort = new clPostHarvestSkiddingMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("GompertzDensitySelfThinning") == 0)
    {
      clDensitySelfThinningGompertz * p_oMort = new clDensitySelfThinningGompertz(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("TempDependentNeighborhoodSurvival") == 0)
    {
      clTempDependentNeighborhoodSurvival * p_oMort = new clTempDependentNeighborhoodSurvival(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("ClimateCompDepNeighborhoodSurvival") == 0)
    {
      clClimateCompDepNeighborhoodSurvival * p_oMort = new clClimateCompDepNeighborhoodSurvival(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("InsectInfestationMortality") == 0)
    {
      clInsectInfestationMortality * p_oMort = new clInsectInfestationMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("SuppressionDurationMortality") == 0)
    {
      clSuppressionDurationMort * p_oMort = new clSuppressionDurationMort(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("SizeDependentLogisticMortality") == 0)
    {
      clSizeDependentLogisticMortality * p_oMort = new clSizeDependentLogisticMortality(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;

    }
    else if (p_data->sNameString.compare("RemoveDead") == 0)
    {
      clTreeRemover * p_oMort = new clTreeRemover(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oMort;
    }

    //*************************************************
    // Substrate Behavior
    //*************************************************
    else if (p_data->sNameString.compare("Substrate") == 0)
    {
      clSubstrate * p_oSubstrate = new clSubstrate(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oSubstrate;

    }
    else if (p_data->sNameString.compare("DetailedSubstrate") == 0)
    {
      clDetailedSubstrate * p_oSubstrate = new clDetailedSubstrate(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oSubstrate;

    }


    else if (p_data->sNameString.compare("EpiphyticEstablishment") == 0)
    {
      clEpiphyticEstablishment * p_oEstablish = new clEpiphyticEstablishment(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oEstablish;

    }

    //*************************************************
    // Disperse Behaviors
    //*************************************************
    else if ((p_data->sNameString.compare("GapDisperse") == 0)
        || (p_data->sNameString.compare("NonGapDisperse") == 0))
    {
      clSpatialDispersal * p_oDisperse = new clSpatialDispersal(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;
    }
    else if (p_data->sNameString.compare("NonSpatialDisperse") == 0)
    {
      clNonSpatialDispersal * p_oDisperse = new clNonSpatialDispersal(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;

    }
    else if (p_data->sNameString.compare("MastingSpatialDisperse") == 0)
    {
      clMastingSpatialDisperse * p_oDisperse = new clMastingSpatialDisperse(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;
    }
    else if (p_data->sNameString.compare("MastingNonSpatialDisperse") == 0)
    {
      clMastingNonSpatialDisperse * p_oDisperse = new clMastingNonSpatialDisperse(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;
    }
    else if (p_data->sNameString.compare("TemperatureDependentNeighborhoodDisperse") == 0)
    {
      clTempDependentNeighborhoodDisperse * p_oDisperse = new clTempDependentNeighborhoodDisperse(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;
    }
    else if (p_data->sNameString.compare("StochDoubleLogTempDepNeighDisperse") == 0)
    {
      clStochDoubleLogTempDepNeighDisperse * p_oDisperse = new clStochDoubleLogTempDepNeighDisperse(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;
    }
    else if (p_data->sNameString.compare("ConspecificBANeighborhoodDisperse") == 0)
    {
      clConspecificBANeighborhoodDisperse * p_oDisperse = new clConspecificBANeighborhoodDisperse(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;
    }

    //*************************************************
    // Seed Survival Behaviors
    //*************************************************
    else if (p_data->sNameString.compare("FunctionalResponseSeedPredation") == 0 ||
        p_data->sNameString.compare("LinkedFunctionalResponseSeedPredation") == 0)
    {
      clFuncResponseSeedPredation * p_oPredation = new clFuncResponseSeedPredation(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oPredation;
    }
    else if (p_data->sNameString.compare("NeighborhoodSeedPredation") == 0 ||
        p_data->sNameString.compare("LinkedNeighborhoodSeedPredation") == 0)
    {
      clNeighborhoodSeedPredation * p_oPredation = new clNeighborhoodSeedPredation(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oPredation;
    }
    else if (p_data->sNameString.compare("Germination") == 0)
    {
      clGermination * p_oGermination = new clGermination(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oGermination;
    }
    else if (p_data->sNameString.compare("DensityDependentSeedSurvival") == 0  ||
        p_data->sNameString.compare("ConspecificTreeDensityDependentSeedSurvival") == 0)
    {
      clDensitySeedSurvival * p_oSeedSurvival = new clDensitySeedSurvival(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oSeedSurvival;
    }
    else if (p_data->sNameString.compare("LightDependentSeedSurvival") == 0 ||
        p_data->sNameString.compare("StormLightDependentSeedSurvival") == 0)
    {
      clLightDepSeedSurvival * p_oSeedSurvival = new clLightDepSeedSurvival(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oSeedSurvival;
    }
    else if (p_data->sNameString.compare("NoGapSubstrateSeedSurvival") == 0 ||
        p_data->sNameString.compare("MicrotopographicSubstrateSeedSurvival") == 0 ||
        p_data->sNameString.compare("GapSubstrateSeedSurvival") == 0)
    {
      clSubstrateDepSeedSurvival * p_oSeedSurvival = new clSubstrateDepSeedSurvival(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oSeedSurvival;
    }

    //*************************************************
    // Establishment Behaviors
    //*************************************************

    else if (p_data->sNameString.compare("Establishment") == 0)
    {
      clEstablishment * p_oDisperse = new clEstablishment(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;
    }
    else if (p_data->sNameString.compare("MicroEstablishment") == 0)
    {
      clMicroEstablishment * p_oDisperse = new clMicroEstablishment(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisperse;
    }
    //*************************************************
    // Disturbance Behaviors
    //*************************************************
    else if (p_data->sNameString.compare("SelectionHarvest") == 0)
    {
      clSelectionHarvest * p_oSelectionHarvest;
      p_oSelectionHarvest = new clSelectionHarvest(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oSelectionHarvest;
    }
    else if (p_data->sNameString.compare("Harvest") == 0 || p_data->sNameString.compare("EpisodicMortality") == 0)
    {
      clDisturbance * p_oDisturbance = new clDisturbance(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oDisturbance;

    }
    else if (p_data->sNameString.compare("Plant") == 0)
    {
      clPlant * p_oPlant = new clPlant(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oPlant;
    }

    else if (p_data->sNameString.compare("StormDamageApplier") == 0)
    {
      clStormDamageApplier * p_oObj = new clStormDamageApplier(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("StormKiller") == 0)
    {
      clStormKiller * p_oObj = new clStormKiller(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("StormDirectKiller") == 0)
    {
      clStormDirectKiller * p_oObj = new clStormDirectKiller(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("Storm") == 0)
    {
      clStorm * p_oObj = new clStorm(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("Windstorm") == 0)
    {
      clWindstorm * p_oObj = new clWindstorm(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    //Note the name comparison isn't the same for this one
    else if (p_data->sNameString.find("HarvestInterface") != string::npos)
    {
      clHarvestInterface * p_oObj = new clHarvestInterface(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("RandomBrowse") == 0)
    {
      clRandomBrowse * p_oObj = new clRandomBrowse(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("CompetitionHarvest") == 0)
    {
      clCompetitionHarvest * p_oObj = new clCompetitionHarvest(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("InsectInfestation") == 0)
    {
      clInsectInfestation * p_oObj = new clInsectInfestation(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("GeneralizedHarvestRegime") == 0)
    {
      clGeneralizedHarvestRegime * p_oObj = new clGeneralizedHarvestRegime(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("DensDepInfestation") == 0)
    {
      clDensDepInfestation * p_oObj = new clDensDepInfestation(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }
    else if (p_data->sNameString.compare("QualityVigorClassifier") == 0)
    {
      clQualityVigorClassifier * p_oObj = new clQualityVigorClassifier(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oObj;
    }

    //*************************************************
    // Output Behaviors
    //*************************************************

    else if (p_data->sNameString.compare("Output") == 0)
    {
      clOutput * p_oOutput = new clOutput(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;

    }
    else if (p_data->sNameString.compare("ShortOutput") == 0)
    {
      clShortOutput * p_oOutput = new clShortOutput(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }

    //*************************************************
    // Analysis Behaviors
    //*************************************************
    else if (p_data->sNameString.compare("TreeVolumeCalculator") == 0)
    {
      clVolumeCalculator * p_oOutput = new clVolumeCalculator(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("TreeBoleVolumeCalculator") == 0)
    {
      clBoleVolumeCalculator * p_oOutput = new clBoleVolumeCalculator(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("DimensionAnalysis") == 0)
    {
      clDimensionAnalysis * p_oOutput = new clDimensionAnalysis(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("TreeAgeCalculator") == 0)
    {
      clTreeAgeCalculator * p_oOutput = new clTreeAgeCalculator(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("MerchValueCalculator") == 0)
    {
      clMerchValueCalculator * p_oOutput = new clMerchValueCalculator(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("CarbonValueCalculator") == 0)
    {
      clCarbonValueCalculator * p_oOutput = new clCarbonValueCalculator(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if ((p_data->sNameString.compare("PartitionedDBHBiomass") == 0)
        || (p_data->sNameString.compare("PartitionedHeightBiomass") == 0))
    {
      clPartitionedBiomass * p_oOutput = new clPartitionedBiomass(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if ((p_data->sNameString.compare("StormKilledPartitionedDBHBiomass") == 0)
        || (p_data->sNameString.compare("StormKilledPartitionedHeightBiomass") == 0))
    {
      clStormKilledPartitionedBiomass * p_oOutput = new clStormKilledPartitionedBiomass(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("RipleysK") == 0)
    {
      clRipleysKCalculator * p_oOutput = new clRipleysKCalculator(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("ConditsOmega") == 0)
    {
      clConditOmegaCalculator * p_oOutput = new clConditOmegaCalculator(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("Crown Radius Reporter") == 0)
    {
      // clCrownRadiusReporter * p_oOutput = new clCrownRadiusReporter(mp_oSimManager);
      // mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("FoliarChemistry") == 0)
    {
      clFoliarChemistry * p_oOutput = new clFoliarChemistry(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    else if (p_data->sNameString.compare("StateReporter") == 0)
    {
      clStateReporter * p_oOutput = new clStateReporter(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oOutput;
    }
    //*************************************************
    // Test objects
    //*************************************************

    else if (p_data->sNameString.compare("random seed logger") == 0)
    {
      clRandomSeedLogger * p_oTest = new clRandomSeedLogger(mp_oSimManager);
      mp_oObjectArray[iIndex] = p_oTest;
    }
    else
    { //we do not have a recognized type
      modelErr stcErr;
      stcErr.sFunction = "clBehaviorManager::CreateBehavior" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Unrecognized behavior ";
      stcErr.sMoreInfo += p_data->sNameString;
      throw(stcErr);
    }

    //Pass the species, type, and name data to the behavior
    p_oBehavior = dynamic_cast < clBehaviorBase * > (mp_oObjectArray[iIndex]);
    p_oBehavior->SetSpeciesTypeCombos(p_data->iNumCombos, p_data->p_whatCombos);
    p_oBehavior->SetNameData(p_data->sNameString);
    p_oBehavior->SetBehaviorListNumber(p_data->iBehaviorListNumber);
    iReturnCode = p_oBehavior->ValidateVersionNumber(p_data->fVersion);

    //If the return code was error, throw error
    if (-1 == iReturnCode)
    {
      modelErr stcErr;
      stcErr.sFunction = "clBehaviorManager::CreateBehavior" ;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sMoreInfo = "Behavior version number wrong for ";
      stcErr.sMoreInfo += p_data->sNameString;
      throw(stcErr);
    }
  } //end of try block
  catch (modelErr & err) { throw(err); }
  catch (modelMsg & msg) { throw(msg); } //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clBehaviorManager::CreateBehavior" ;
    throw(stcErr);
  }
}
//---------------------------------------------------------------------------
