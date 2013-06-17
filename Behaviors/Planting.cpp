//---------------------------------------------------------------------------
#include "Planting.h"
//---------------------------------------------------------------------------
#include "SimManager.h"
#include "ParsingFunctions.h"
#include "Grid.h"
#include "TreePopulation.h"
#include "Plot.h"
#include "ModelMath.h"
#include <stdio.h>
#include <sstream>

#define TINY 0.01

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
clPlant::clPlant(clSimManager *p_oSimManager)
    : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ) {
  try {
    m_sNameString = "Plant";
    m_sXMLRoot = "Plant";

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    //Null pointers, initialize variables to empty
    mp_oPlantMasterGrid = NULL;
    mp_oPlantEventsGrid = NULL;
    mp_oPlantResultsGrid = NULL;
    mp_iAmtPlantCodes = NULL;
    mp_iPlantedCodes = NULL;
    mp_fInitialDiam10 = NULL;
    mp_oPop = NULL;

    m_iPlantIDCode = -1;
    m_iPlantTimestepCode = -1;
    m_iMasterTimestepCode = -1;
    m_iMasterIDCode = -1;
    m_iSpaceTypeCode = -1;
    m_iSpacingOrDensityCode = -1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;
  }
  catch(modelErr &err) {throw(err);} //non-fatal error
  catch(...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clPlant::clPlant";
    throw(stcErr);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
clPlant::~clPlant() {
  delete[] mp_iAmtPlantCodes;
  delete[] mp_iPlantedCodes;
  delete[] mp_fInitialDiam10;
}


//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clPlant::GetData(DOMDocument *p_oDoc) {
 try {
   clPackage *p_oOldPackage = NULL, //for finding where to place a new package
            *p_oNewPackage = NULL; //a newly created package
   DOMNodeList *p_oPlantEventsList, //list of plant events
               *p_oPlantSpecificsList; //sub-plant-event - species, cell
   DOMNode *p_oNode;
   DOMElement *p_oElement,
              *p_oChildElement;
   XMLCh *sVal;
   std::string sTemp;
   floatVal *p_fVals = NULL;//for extracting float data from parameter file
   float fTemp;             //for extracting float data from parameter file
   int iNumPlantEvents,     //total number of plant events to extract
       iX, iY,              //X and Y grid values for package
       iNumberTimesteps = mp_oSimManager->GetNumberOfTimesteps(),
       iTimestep,           //timestep of plant event
       iNumChildren,        //number of sub-plant-events
       iNumSpecies,         //total number of tree species from the population
       iTemp;               //for extracting integer data from parameter file
   short int i, j;          //loop counters
   char *cData;
   bool *p_bSpeciesList,    //which species are represented in a plant event
        *p_bMasterSpeciesList;//which species are represented in any plant event


   //See if there's any planting data in the parameter file
   /*sVal = XMLString::transcode("Plant");
   p_oPlantEventsList = p_oDoc->getElementsByTagName(sVal);
   XMLString::release(&sVal);
   if (0 == p_oPlantEventsList->getLength()) return;*/

   //Now get the list of all plant events
   //p_oNode = p_oPlantEventsList->item(0);
   //p_oElement = (DOMElement *) p_oNode;
   p_oElement = GetParentParametersElement(p_oDoc);
   sVal = XMLString::transcode("pl_plantEvent");
   p_oPlantEventsList = p_oElement->getElementsByTagName(sVal);
   XMLString::release(&sVal);

   //If there's no plant events, don't do anything else
   iNumPlantEvents = p_oPlantEventsList->getLength();
   if (0 == iNumPlantEvents) return;

   mp_oPop = (clTreePopulation*) mp_oSimManager->GetPopulationObject("treepopulation");

   //Declare the species list array
   iNumSpecies = mp_oPop->GetNumberOfSpecies();
   p_bSpeciesList = new bool[iNumSpecies];
   p_bMasterSpeciesList = new bool[iNumSpecies];
   for (i = 0; i < iNumSpecies; i++)
     p_bMasterSpeciesList[i] = false;

   //Declare the plant diam10 array and initialize with the default - the
   //standard seedling size.  We'll check for changes to this list later.
   mp_fInitialDiam10 = new float[iNumSpecies];
   fTemp = mp_oPop->GetNewSeedlingDiam10();
   for (i = 0; i < iNumSpecies; i++) mp_fInitialDiam10[i] = fTemp;

   //Set up the plant info grids
   SetupPlantGrids();

   //*******************************************
   //Go through each plant event and add it as a package to the grids
   //*******************************************
   for (i = 0; i < iNumPlantEvents; i++) {
     p_oNode = p_oPlantEventsList->item(i);
     p_oElement = (DOMElement *) p_oNode;

     for (j = 0; j < iNumSpecies; j++)
       p_bSpeciesList[j] = false;

     //Timestep first
     FillSingleValue(p_oElement, "pl_timestep", &iTimestep, true);

     //Validate it - needs to be between 0 and number of timesteps
     if (iTimestep <= 0 || iTimestep > iNumberTimesteps) {
       modelErr stcErr;
       stcErr.iErrorCode = BAD_DATA;
       stcErr.sFunction = "clPlant::GetData";
       std::stringstream s;
       s << "Invalid plant timestep " << iTimestep;
       stcErr.sMoreInfo = s.str();
       throw(stcErr);
     }

     //Add a package to the master plant events grid - they're in timestep
     //order, so keep it that way
     p_oOldPackage = NULL;
     p_oNewPackage = NULL;
     p_oOldPackage = mp_oPlantMasterGrid->GetFirstPackageOfCell(0, 0);
     while (p_oOldPackage) {
       //Get this package's timestep
       p_oOldPackage->GetValue(m_iMasterTimestepCode, &iTemp);

       if (iTemp >= iTimestep) //here's where we'll drop it
         break;

       p_oNewPackage = p_oOldPackage; //use temporarily as a holder
       p_oOldPackage = p_oOldPackage->GetNextPackage();
     }

     //Create the new package
     if (!p_oNewPackage) //either there's no packages in grid, or new is first
       p_oNewPackage = mp_oPlantMasterGrid->CreatePackageOfCell(0, 0);
     else   //make the new one after the one in p_oNewPackage
       p_oNewPackage = mp_oPlantMasterGrid->CreatePackage(p_oNewPackage);

     //Load up the new package with the timestep value
     p_oNewPackage->SetValue(m_iMasterTimestepCode, iTimestep);



     //Get species values and assign them
     sVal = XMLString::transcode("pl_applyToSpecies");
     p_oPlantSpecificsList = p_oElement->getElementsByTagName(sVal);
     XMLString::release(&sVal);
     iNumChildren = p_oPlantSpecificsList->getLength();
     for (j = 0; j < iNumChildren; j++) {
       p_oNode = p_oPlantSpecificsList->item(j);
       p_oChildElement = (DOMElement *) p_oNode;
       iTemp = GetNodeSpeciesCode(p_oChildElement, mp_oPop);
       if (-1 == iTemp) {  //throw an error
         modelErr stcErr;
         stcErr.iErrorCode = BAD_DATA;
         stcErr.sFunction = "clPlant::GetData";
         stcErr.sMoreInfo = "Unrecognized species in plant.";
         throw(stcErr);
       }
       p_bSpeciesList[iTemp] = true;
       p_bMasterSpeciesList[iTemp] = true;
     }



     //Get the spacing type
     FillSingleValue(p_oElement, "pl_spaceType", &sTemp, true);
     //See if it's a value we recognize
     if (sTemp.compare("gridded") == 0) iTemp = gridded;
     else if (sTemp.compare("random") == 0) iTemp = random;
     else {
       modelErr stcErr;
       stcErr.iErrorCode = BAD_DATA;
       stcErr.sFunction = "clPlant::GetData";
       stcErr.sMoreInfo = "Unrecognized value in pl_spaceType.";
       throw(stcErr);
     }
     p_oNewPackage->SetValue(m_iSpaceTypeCode, iTemp);


     //Get the spacing distance or density
     FillSingleValue(p_oElement, "pl_distanceOrDensity", &fTemp, true);
     p_oNewPackage->SetValue(m_iSpacingOrDensityCode, fTemp);

     //Create the floatVal array and load it with the species we've been given
     iTemp = 0;
     for (j = 0; j < iNumSpecies; j++)
       if (p_bSpeciesList[j]) iTemp++;

     if (p_fVals) delete[] p_fVals;
     p_fVals = new floatVal[iTemp];
     iTemp = 0;
     for (j = 0; j < iNumSpecies; j++)
       if (p_bSpeciesList[j]) {
         p_fVals[iTemp].code = j;
         iTemp++;
       }

     //Get the amount to plant for each species
     FillSpeciesSpecificValue(p_oElement, "pl_amountToPlant",
      "pl_atpVal", p_fVals, iTemp, mp_oPop, true);

     //Transfer this to our package
     for (j = 0; j < iTemp; j++)
       p_oNewPackage->SetValue(mp_iAmtPlantCodes[p_fVals[j].code],
                             p_fVals[j].val);



     //Create an ID number - just use the loop counter
     p_oNewPackage->SetValue(m_iMasterIDCode, (int)i);

     //*******************************************
     //Now:  Get the list of cells to which to apply this plant and make a
     //package for each
     //*******************************************
     sVal = XMLString::transcode("pl_applyToCell");
     p_oPlantSpecificsList = p_oElement->getElementsByTagName(sVal);
     XMLString::release(&sVal);
     iNumChildren = p_oPlantSpecificsList->getLength();
     for (j = 0; j < iNumChildren; j++) {
       p_oNode = p_oPlantSpecificsList->item(j);
       p_oChildElement = (DOMElement *) p_oNode;
       sVal = XMLString::transcode("x");
       cData = XMLString::transcode(p_oChildElement->getAttributeNode(
                       sVal)->getNodeValue());
       iX = atoi(cData);
       delete[] cData; cData = NULL;
       XMLString::release(&sVal);
       sVal = XMLString::transcode("y");
       cData = XMLString::transcode(p_oChildElement->getAttributeNode(
                       sVal)->getNodeValue());
       iY = atoi(cData);
       delete[] cData; cData = NULL;
       XMLString::release(&sVal);

       p_oOldPackage = NULL;
       p_oNewPackage = NULL;
       p_oOldPackage = mp_oPlantEventsGrid->GetFirstPackageOfCell(iX, iY);
       while (p_oOldPackage) {
         //Get this package's timestep
         p_oOldPackage->GetValue(m_iPlantTimestepCode, &iTemp);

         if (iTemp >= iTimestep) //here's where we'll drop it
           break;

         p_oNewPackage = p_oOldPackage; //use temporarily as a holder
         p_oOldPackage = p_oOldPackage->GetNextPackage();
       }

       //Create the new package
       if (!p_oNewPackage) //either there's no packages in grid, or new is first
         p_oNewPackage = mp_oPlantEventsGrid->CreatePackageOfCell(iX, iY);
       else   //make the new one after the one in p_oNewPackage
         p_oNewPackage = mp_oPlantEventsGrid->CreatePackage(p_oNewPackage);

       //Load up the new package with the timestep and id values
       p_oNewPackage->SetValue(m_iPlantTimestepCode, iTimestep);
       p_oNewPackage->SetValue(m_iPlantIDCode, (int)i);
     } //end of for (j = 0; j < iNumChildren; j++)
   } //end of for (i = 0; i < iNumPlantEvents; i++)

   //Now get the new diam10 values, if they exist - load the floatVal array with
   //the master species list representing any species that was in a plant event
   iTemp = 0;
   for (i = 0; i < iNumSpecies; i++)
     if (p_bMasterSpeciesList[i]) iTemp++;

   if (p_fVals) delete[] p_fVals;
   p_fVals = new floatVal[iTemp];
   iTemp = 0;
   for (i = 0; i < iNumSpecies; i++)
     if (p_bMasterSpeciesList[i]) {
       p_fVals[iTemp].code = i;
       p_fVals[iTemp].val = 0;
       iTemp++;
     }
   //See if there are new values for plant diam10
   FillSpeciesSpecificValue(p_oDoc->getDocumentElement(), "pl_initialDiam10",
      "pl_idVal", p_fVals, iTemp, mp_oPop, false);

   for (i = 0; i < iTemp; i++)
     if (p_fVals[i].val > 0)
       mp_fInitialDiam10[p_fVals[i].code] = p_fVals[i].val;


   //Validate the package data
   ValidatePackages();

   if (p_fVals) delete[] p_fVals;
   delete[] p_bSpeciesList;
   delete[] p_bMasterSpeciesList;

 } //end of try block
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clPlant::GetData";
   throw(stcErr);
 }
}


//////////////////////////////////////////////////////////////////////////////
// ValidatePackages()
//////////////////////////////////////////////////////////////////////////////
void clPlant::ValidatePackages() {
 try {
   clPackage *p_oPackage,     //a package to validate
            *p_oNextPackage; //for comparing other packages to this one
   float fTemp,             //for extracting a value from a package
         fPercentagesTotal; //total of all percentages, if gridded planting
   int iSpaceType,          //spacing type value
       iID,                 //package ID value
       iNextID;             //for comparing other packages' ID values
   short int iNumXCells = mp_oPlantEventsGrid->GetNumberXCells(),
             iNumYCells = mp_oPlantEventsGrid->GetNumberYCells(),
             iNumSpecies = mp_oPop->GetNumberOfSpecies(),
             i, j;          //loop counters

   //****************************************************
   // Validate packages in master plant grid
   //****************************************************
   p_oPackage = mp_oPlantMasterGrid->GetFirstPackageOfCell(0, 0);
   while (p_oPackage) {

     //Get the spacing type
     p_oPackage->GetValue(m_iSpaceTypeCode, &iSpaceType);

     //Spacing type tells us what to expect.  If the space type is gridded, make
     //sure that the space distance isn't less than or equal to 0 or greater
     //than the width of a grid cell minus an offset
     if (gridded == iSpaceType) {
       p_oPackage->GetValue(m_iSpacingOrDensityCode, &fTemp);
       if (fTemp <= 0 || fTemp > mp_oPlantEventsGrid->GetLengthXCells() - 1) {
         modelErr stcErr;
         stcErr.iErrorCode = BAD_DATA;
         stcErr.sFunction = "clPlant::ValidatePackages";
         std::stringstream s;
         s << "Plant grid spacing must be greater than 0 and less than "
           << (mp_oPlantEventsGrid->GetLengthXCells() - 1);
         stcErr.sMoreInfo = s.str();
         throw(stcErr);
       }
     }

     //Make sure that the plant amount values for each species are not negative.
     //They must be percentages that add up to 100 across the species.
     fPercentagesTotal = 0;
     for (i = 0; i < iNumSpecies; i++) {

       p_oPackage->GetValue(mp_iAmtPlantCodes[i], &fTemp);
       if (fTemp < 0) {
         modelErr stcErr;
         stcErr.iErrorCode = BAD_DATA;
         stcErr.sFunction = "clPlant::ValidatePackages";
         stcErr.sMoreInfo = "Plant amounts must be greater than 0.";
         throw(stcErr);
       }

       fPercentagesTotal += fTemp;
     }

     //We'll make a slight allowance for rounding errors - the proportions must
     //add up to 100 +- 0.01
     if (fabs(fPercentagesTotal - 100) > 0.01) {
       modelErr stcErr;
       stcErr.iErrorCode = BAD_DATA;
       stcErr.sFunction = "clPlant::ValidatePackages";
       stcErr.sMoreInfo = "Plant amounts for gridded spacings must add up to 100.";
       throw(stcErr);
     }

     p_oPackage = p_oPackage->GetNextPackage();
   } //end of while (p_oPackage)


   //****************************************************
   // Validate packages in planting events grid
   //****************************************************
   for (i = 0; i < iNumXCells; i++)
     for (j = 0; j < iNumYCells; j++) {
       p_oPackage = mp_oPlantEventsGrid->GetFirstPackageOfCell(i, j);
       while (p_oPackage) {

         //Go through all the other packages in this grid cell and make sure that
         //there is no other with the same ID number
         p_oPackage->GetValue(m_iPlantIDCode, &iID);

         p_oNextPackage = p_oPackage->GetNextPackage();
         while (p_oNextPackage) {

             p_oNextPackage->GetValue(m_iPlantIDCode, &iNextID);
             if (iID == iNextID) {
               //throw an error
               modelErr stcErr;
               stcErr.iErrorCode = BAD_DATA;
               stcErr.sFunction = "clPlant::ValidatePackages";
               stcErr.sMoreInfo = "A planting may not be applied twice to the same grid cell.";
               throw(stcErr);
             }
           p_oNextPackage = p_oNextPackage->GetNextPackage();
         } //end of while (p_oNextPackage)
         p_oPackage = p_oPackage->GetNextPackage();
       } //end of while (p_oPackage)
     } //end of for (j = 0; j < iNumYCells; j++)
 } //end of try block
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clPlant::ValidatePackages";
   throw(stcErr);
 }
}


//////////////////////////////////////////////////////////////////////////////
// TimestepCleanup()
//////////////////////////////////////////////////////////////////////////////
void clPlant::TimestepCleanup() {
 try {
   int iValue = 0;
   short int iNumXCells = mp_oPlantResultsGrid->GetNumberXCells(),
             iNumYCells = mp_oPlantResultsGrid->GetNumberYCells(),
             iNumSpecies = mp_oPop->GetNumberOfSpecies(),
             i, j, iSp; //loop counters

   //For the plant results grid - reset all values to 0
   for (i = 0; i < iNumXCells; i++)
     for (j = 0; j < iNumYCells; j++)
       for (iSp = 0; iSp < iNumSpecies; iSp++)

           mp_oPlantResultsGrid->
                   SetValueOfCell(i, j, mp_iPlantedCodes[iSp], iValue);



 } //end of try block
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clPlant::TimestepCleanup";
   throw(stcErr);
 }
}


//////////////////////////////////////////////////////////////////////////////
// SetupPlantGrids()
//////////////////////////////////////////////////////////////////////////////
void clPlant::SetupPlantGrids() {
 try {
   clPlot *p_oPlot = mp_oSimManager->GetPlotObject();
   std::stringstream sLabel;
   float fCellLength = mp_oPop->GetGridCellSize();
   short int iNumSpecies = mp_oPop->GetNumberOfSpecies(),
             i;    //loop counter

   //*************************************
   //plantmaster grid
   //*************************************
   mp_oPlantMasterGrid = mp_oSimManager->CreateGrid(
       "plantmaster",              //grid name
       0,                          //number of int data members
       0,                          //number of float data members
       0,                          //number of char data members
       0,                          //number of bool data members
       p_oPlot->GetXPlotLength(),  //X cell length
       p_oPlot->GetYPlotLength()); //Y cell length

   //Now adjust for the package data members
   mp_oPlantMasterGrid->ChangePackageDataStructure(
       3,               //number of package int data members
       1 + iNumSpecies, //number of package float data members
       0,               //number of package char data members
       0);              //number of package bool data members

   //Declare the array for holding data member codes
   mp_iAmtPlantCodes = new short int[iNumSpecies];

   //Register the data members
   m_iMasterTimestepCode = mp_oPlantMasterGrid->RegisterPackageInt("timestep");
   m_iMasterIDCode = mp_oPlantMasterGrid->RegisterPackageInt("id");
   m_iSpaceTypeCode = mp_oPlantMasterGrid->RegisterPackageInt("spacetype");
   m_iSpacingOrDensityCode = mp_oPlantMasterGrid->RegisterPackageFloat("spcorden");

   for (i = 0; i < iNumSpecies; i++) {
     sLabel << "amtPlant" << i;
     mp_iAmtPlantCodes[i] = mp_oPlantMasterGrid->RegisterPackageFloat(sLabel.str());
     sLabel.str("");
   }


   //*************************************
   //plantevents grid
   //*************************************
   mp_oPlantEventsGrid = mp_oSimManager->CreateGrid(
       "plantevents",        //grid name
       0,                    //number of int data members
       0,                    //number of float data members
       0,                    //number of char data members
       0,                    //number of bool data members
       fCellLength,          //X cell length
       fCellLength);         //Y cell length

   //Now adjust for the package data members
   mp_oPlantEventsGrid->ChangePackageDataStructure(
       2,                    //number of package int data members
       0,                    //number of package float data members
       0,                    //number of package char data members
       0);                   //number of package bool data members

   //Register the data members
   m_iPlantTimestepCode = mp_oPlantEventsGrid->RegisterPackageInt("timestep");
   m_iPlantIDCode = mp_oPlantEventsGrid->RegisterPackageInt("id");



   //*************************************
   //Planting results grid
   //*************************************
   mp_oPlantResultsGrid = mp_oSimManager->CreateGrid(
       "Planting Results",       //grid name
       iNumSpecies,          //number of int data members
       0,                    //num float data members
       0,                    //number of char data members
       0,                    //number of bool data members
       fCellLength,          //X cell length
       fCellLength);         //Y cell length

   //Declare the array for the return codes
   mp_iPlantedCodes = new short int[iNumSpecies];

   //Register the data members
   for (i = 0; i < iNumSpecies; i++) {
     sLabel << "planted" << i;
     mp_iPlantedCodes[i] = mp_oPlantResultsGrid->RegisterInt(sLabel.str());
     sLabel.str("");
   }
 } //end of try block
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clPlant::SetupPlantGrids";
   throw(stcErr);
 }
}


//////////////////////////////////////////////////////////////////////////////
// Action()
//////////////////////////////////////////////////////////////////////////////
void clPlant::Action() {
 try {
   clPackage *p_oMasterPackage, *p_oPackageHolder;
   int iTimestep = mp_oSimManager->GetCurrentTimestep(),
       iPackageTimestep;

   //Get all the master planting packages for this timestep and execute them
   p_oMasterPackage = mp_oPlantMasterGrid->GetFirstPackageOfCell(0, 0);

   while (p_oMasterPackage) {
     //Get the next package - if this is a planting package, it will be deleted
     //during the course of the planting process
     p_oPackageHolder = p_oMasterPackage->GetNextPackage();

     //Get this package's timestep
     p_oMasterPackage->GetValue(m_iMasterTimestepCode, &iPackageTimestep);
     if (iTimestep == iPackageTimestep)
       PlantTrees(p_oMasterPackage);

     //I commented this out as a quick-and-dirty way to make this work if we
     //start on a timestep other than 0 and there are defined events that
     //were supposed to happen before the start timestep.
//     else
//       break;  //we've done them all this timestep

     p_oMasterPackage = p_oPackageHolder;
   }
 } //end of try block
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clPlant::Action";
   throw(stcErr);
 }
}


///////////////////////////////////////////////////////////////////////////////
// PlantTrees()
//////////////////////////////////////////////////////////////////////////////
void clPlant::PlantTrees(clPackage *p_oMasterPackage) {
 try {
   clPlot * p_oPlot = mp_oSimManager->GetPlotObject();
   stcGridList *p_plantArea = NULL, //grid cell list for the area being planted
               *p_cellToPlant = NULL;
   float *p_fAmountToPlant, //amount of each species to plant
         fRand, //random number
         fXOrig, fYOrig, //coordinates of origin of a grid cell
         fXEdgeLength, //length of the cells in the last row of the plot, in the X direction
         fYEdgeLength, //length of the cells in the last row of the plot, in the Y direction
         fX, fY, //coordinates of new seedling
         fDiam10, //diameter at 10 cm of new seedling
         fTreeSpacing, //tree spacing if gridded planting
         fCellLength = mp_oPop->GetGridCellSize(),
         fXCellEnd, fYCellEnd, //ending coordinates of cell
         fXThisCellLength, //length of current cell in X direction
         fYThisCellLength, //length of current cell in X direction
         fNumTotalTrees, //for random planting - how many trees to plant
         fTree, //loop counter - float in case there's a bunch of trees
         fTemp, fI, fJ; //loop counters
   int iSpaceType, //type of spacing - random or gridded
       iNumXCells = mp_oPlantEventsGrid->GetNumberXCells(),
       iNumYCells = mp_oPlantEventsGrid->GetNumberYCells(),
       iNumCellsInPlantArea, //number of grid cells in the area to plant
       iCellNumber, //for picking a random cell
       iTemp, //for doing grid results
       iOffset = 1, //for gridded planting
       iNumSpecies = mp_oPop->GetNumberOfSpecies();
   short int iSp, k;  //loop counters
   bool bSpeciesPicked;  //for helping us randomly select a species

   fXEdgeLength = p_oPlot->GetXPlotLength() - (fCellLength * floor(p_oPlot->GetXPlotLength() / fCellLength));
   fXEdgeLength = (fXEdgeLength == 0 ? fCellLength : fXEdgeLength);
   fYEdgeLength = p_oPlot->GetYPlotLength() - (fCellLength * floor(p_oPlot->GetYPlotLength() / fCellLength));
   fYEdgeLength = (fYEdgeLength == 0 ? fCellLength : fYEdgeLength);

   //Get the spacing type from the package
   p_oMasterPackage->GetValue(m_iSpaceTypeCode, &iSpaceType);

   //Get the amount of each species to plant
   p_fAmountToPlant = new float[iNumSpecies];
   for (iSp = 0; iSp < iNumSpecies; iSp++) {
     p_oMasterPackage->GetValue(mp_iAmtPlantCodes[iSp], &fTemp);
     p_fAmountToPlant[iSp] = fTemp;
     //Transform the amount to plant from percentages to proportions
     p_fAmountToPlant[iSp] *= 0.01;
   }

   //Assemble the plant area
   iNumCellsInPlantArea = AssemblePlantArea(p_oMasterPackage,
       mp_oPlantEventsGrid->GetNumberXCells(),
       mp_oPlantEventsGrid->GetNumberYCells(), p_plantArea);

   if (random == iSpaceType) {
     //*********************************************
     // Random planting
     //*********************************************

     //The amount to plant is in value per hectare.  Convert to a straight-up
     //number by multiplying by area of planting area in hectares
     p_cellToPlant = p_plantArea;
     fTemp = 0;
     for (k = 0; k < iNumCellsInPlantArea; k++) {
       //Make sure we correctly calculate the area of edge cells
       if (p_cellToPlant->iX == iNumXCells - 1) fXThisCellLength = fXEdgeLength;
       else fXThisCellLength = fCellLength;
       if (p_cellToPlant->iY == iNumYCells - 1) fYThisCellLength = fYEdgeLength;
       else fYThisCellLength = fCellLength;

       fTemp += fXThisCellLength * fYThisCellLength;

       p_cellToPlant = p_cellToPlant->next;
     }
     fTemp /= 10000; //convert to hectares
     p_oMasterPackage->GetValue(m_iSpacingOrDensityCode, &fTreeSpacing);
     fTemp *= fTreeSpacing;
     //If the number of trees to plant is greater than or equal to 1, round
     //to the nearest whole number and that's how many trees we'll plant
     if (fTemp >= 1)
       fNumTotalTrees = clModelMath::Round(fTemp, 0);
     //If the number of trees to plant is between 0 and 1, don't round.  We'll
     //let a random number decide whether to plant 0 or 1.
     else {
       fRand = clModelMath::GetRand();
       if (fRand <= fTemp)
         fNumTotalTrees = 1;
       else
         fNumTotalTrees = 0;
     }

     //Plant each tree
     for (fTree = 0; fTree < fNumTotalTrees; fTree++) {

       //Randomly select a cell in the area
       fRand = clModelMath::GetRand();
       iCellNumber = (int)floor(iNumCellsInPlantArea * fRand);
       if (iCellNumber > iNumCellsInPlantArea)
         iCellNumber = iNumCellsInPlantArea;
       else if (iCellNumber <= 0)
         iCellNumber = 0;
       p_cellToPlant = p_plantArea;

       for (k = 0; k < iCellNumber; k++) {
         p_cellToPlant = p_cellToPlant->next;
       }

       //Randomly get an X and Y location within the selected cell.
       if (p_cellToPlant->iX == iNumXCells - 1) fXThisCellLength = fXEdgeLength;
       else fXThisCellLength = fCellLength;
       if (p_cellToPlant->iY == iNumYCells - 1) fYThisCellLength = fYEdgeLength;
       else fYThisCellLength = fCellLength;

       fXOrig = fCellLength * p_cellToPlant->iX;
       fYOrig = fCellLength * p_cellToPlant->iY;
       fXCellEnd = (fXOrig + fXThisCellLength) - TINY;
       fYCellEnd = (fYOrig + fYThisCellLength) - TINY;

       fX = fXOrig + ( clModelMath::GetRand() * fXThisCellLength );
       fY = fYOrig + ( clModelMath::GetRand() * fYThisCellLength );

       while (fX < fXOrig || fY < fYOrig ||
              fX >= fXCellEnd || fY >= fYCellEnd) {
         fX = fXOrig + ( clModelMath::GetRand() * fXThisCellLength );
         fY = fYOrig + ( clModelMath::GetRand() * fYThisCellLength );
       }

         //Get a randomized species based on relative abundances
         bSpeciesPicked = false;
         while (!bSpeciesPicked) {
           fRand = clModelMath::GetRand();
           iSp = (int)floor(fRand * iNumSpecies);
           if (iSp < 0) iSp = 0;
           if (iSp >= iNumSpecies) iSp = iNumSpecies - 1;
           fRand = clModelMath::GetRand();
           if (fRand < p_fAmountToPlant[iSp])
             bSpeciesPicked = true;
         }

       //Get a randomized diam10 value
       fDiam10 = mp_oPop->GetRandomDiam10Value(mp_fInitialDiam10[iSp]);

       //Plant our new seedling
       mp_oPop->CreateTree(fX, fY, iSp, clTreePopulation::seedling, fDiam10);

       //Add it to the results grid
       mp_oPlantResultsGrid->GetValueOfCell(p_cellToPlant->iX,
             p_cellToPlant->iY, mp_iPlantedCodes[iSp], &iTemp);
       iTemp++;
       mp_oPlantResultsGrid->SetValueOfCell(p_cellToPlant->iX,
             p_cellToPlant->iY, mp_iPlantedCodes[iSp], iTemp);

     } //end of for (fTree = 0; fTree < fNumTotalTrees; fTree++)

   } else {

     //*********************************************
     // Gridded planting
     // Gridded planting is done with this little built-in offset.  I'm not
     // sure why - I'm just following the old code.
     //*********************************************

     //Get the tree spacing
     p_oMasterPackage->GetValue(m_iSpacingOrDensityCode, &fTreeSpacing);

     //Plant each cell separately
     p_cellToPlant = p_plantArea;
     while (p_cellToPlant) {

       if (p_cellToPlant->iX == iNumXCells - 1) fXThisCellLength = fXEdgeLength;
       else fXThisCellLength = fCellLength;
       if (p_cellToPlant->iY == iNumYCells - 1) fYThisCellLength = fYEdgeLength;
       else fYThisCellLength = fCellLength;

       //04-04-05 LEM:  Changed loop counters
       for (fI = iOffset; fI < fXThisCellLength; fI += fTreeSpacing) {
         for(fJ = iOffset; fJ < fYThisCellLength; fJ += fTreeSpacing) {

           //Calculate the coordinates of our new seedling
           fX = (p_cellToPlant->iX * fCellLength) + fI;
           fY = (p_cellToPlant->iY * fCellLength) + fJ;

           //Randomly pick a species according to the relative abundances
           //desired
           bSpeciesPicked = false;
           while (!bSpeciesPicked) {
             fRand = clModelMath::GetRand();
             iSp = (int)floor(fRand * iNumSpecies);
             if (iSp < 0) iSp = 0;
             if (iSp >= iNumSpecies) iSp = iNumSpecies - 1;
             fRand = clModelMath::GetRand();
             if (fRand < p_fAmountToPlant[iSp])
               bSpeciesPicked = true;
           }

           //Get a randomized diam10 value
           fDiam10 = mp_oPop->GetRandomDiam10Value(mp_fInitialDiam10[iSp]);

           //Plant our new seedling
           mp_oPop->CreateTree(fX, fY, iSp, clTreePopulation::seedling, fDiam10);

           //Add it to the results grid
           mp_oPlantResultsGrid->GetValueOfCell(p_cellToPlant->iX,
                 p_cellToPlant->iY, mp_iPlantedCodes[iSp], &iTemp);
           iTemp++;
           mp_oPlantResultsGrid->SetValueOfCell(p_cellToPlant->iX,
                 p_cellToPlant->iY,  mp_iPlantedCodes[iSp], iTemp);

         } //end of for(j = iOffset; j < fCellLength; j += fTreeSpacing)
       } //end of for (i = iOffset; i < fCellLength; i += fTreeSpacing)
       p_cellToPlant = p_cellToPlant->next;
     } //end of while (p_cellToPlant
   }

   mp_oPlantMasterGrid->DeletePackage(p_oMasterPackage);
   delete[] p_fAmountToPlant;

   while (p_plantArea) {
     p_cellToPlant = p_plantArea->next;
     delete p_plantArea;
     p_plantArea = p_cellToPlant;
   }
 } //end of try block
 catch (modelErr&err) {throw(err);}
 catch (modelMsg &msg) {throw(msg);} //non-fatal error
 catch (...) {
   modelErr stcErr;
   stcErr.iErrorCode = UNKNOWN;
   stcErr.sFunction = "clPlant::PlantTrees";
   throw(stcErr);
 }
}


//////////////////////////////////////////////////////////////////////////////
// AssemblePlantArea()
//////////////////////////////////////////////////////////////////////////////
int clPlant::AssemblePlantArea(clPackage *p_oMasterPackage, const int
   &iNumXCells, const int &iNumYCells, stcGridList *&p_plantArea) {

  struct stcGridList *p_cellRecord = NULL; //for creating planting area
  int iMasterID,  //ID of master cell
  iTemp,      //for getting package integer values
  iTimestep = mp_oSimManager->GetCurrentTimestep(),
  iNumCellsInPlantArea = 0; //number of cells in the planting area
  short int i, j; //loop counters

  //********************************************
  // Extract the data from the master package
  //********************************************

  //Get the master package's ID number
  p_oMasterPackage->GetValue(m_iMasterIDCode, &iMasterID);

  //********************************************
  // Find the matching planting packages and assemble them into
  // our linked list
  //********************************************

  for (i = 0; i < iNumXCells; i++)
    for (j = 0; j < iNumYCells; j++) {

      //Reusing the p_oMasterPackage pointer
      p_oMasterPackage = mp_oPlantEventsGrid->GetFirstPackageOfCell(i, j);
      while (p_oMasterPackage) {

        //Get this package's timestep to see if it's for this timestep
        p_oMasterPackage->GetValue(m_iPlantTimestepCode, &iTemp);
        if (iTimestep != iTemp) {
          //Null out the pointer - this will break the loop
          p_oMasterPackage = NULL;
          goto nextPackage;
        }

        //Check the ID number
        p_oMasterPackage->GetValue(m_iPlantIDCode, &iTemp);
        if (iTemp != iMasterID) goto nextPackage;

        //Make this a record in our plant list - stick it at the beginning
        if (p_plantArea != NULL) {
          p_cellRecord = new stcGridList;
          p_cellRecord->iX = i;
          p_cellRecord->iY = j;
          p_cellRecord->next = p_plantArea;
          p_plantArea = p_cellRecord;
        } else {
          //Provision for first planting in linked list
          p_plantArea = new stcGridList;
          p_plantArea->iX = i;
          p_plantArea->iY = j;
          p_plantArea->next = NULL;
        }

        iNumCellsInPlantArea++;

        //We're done with this package - delete it
        mp_oPlantEventsGrid->DeletePackage(p_oMasterPackage);
        p_oMasterPackage = NULL;

        nextPackage:
        if (p_oMasterPackage)
          p_oMasterPackage = p_oMasterPackage->GetNextPackage();
      } //end of while (p_oMasterPackage)
    } //end of for (j = 0; j < iNumYCells; j++)

  return iNumCellsInPlantArea;
}
