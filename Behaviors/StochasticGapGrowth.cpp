//---------------------------------------------------------------------------
// StochasticGapGrowth.cpp
//---------------------------------------------------------------------------
#include "StochasticGapGrowth.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include "Grid.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clStochasticGapGrowth::clStochasticGapGrowth(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager) {
  try
  {
    //Set the namestring
    m_sNameString = "StochasticGapGrowth";
    m_sXMLRoot = "StochasticGapGrowth";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    mp_bAppliedTo = NULL;
    m_cQuery = NULL;
    mp_oGapLight = NULL;
    mp_oGrowthCounter = NULL;
    m_iCurrentCode = -1;
    m_iNumSpecies = 0;
    m_iTargetCode = -1;
    m_iIsGapCode = -1;
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
    stcErr.sFunction = "clStochasticGapGrowth::clStochasticGapGrowth" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clStochasticGapGrowth::~clStochasticGapGrowth() {
  if (mp_bAppliedTo) {
    for (int i = 0; i < m_iNumSpecies; i++)
      delete[] mp_bAppliedTo[i];
  }
  delete[] mp_bAppliedTo;
  delete[] m_cQuery;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clStochasticGapGrowth::GetData(xercesc::DOMDocument * p_oDoc) {
  char * cQueryTemp= NULL;
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    char cQueryPiece[5]; //for assembling the search query
    int iNumTypes = p_oPop->GetNumberOfTypes(), i, j;
    bool bSeedling = false, bSapling = false, bAdult = false, bSnag = false;

    cQueryTemp = new char[(p_oPop->GetNumberOfSpecies() * 4) + 50];

    //********************************************
    // Set up the grid pointers
    //********************************************

    //Get the gap light grid
    mp_oGapLight = mp_oSimManager->GetGridObject( "Gap Light" );

    if ( !mp_oGapLight )
    {
      modelErr stcErr;
      stcErr.iErrorCode = DATA_MISSING;
      stcErr.sFunction = "clStochasticGapGrowth::GetData" ;
      stcErr.sMoreInfo = "You must use the \"Gap Light\" behavior with \"Stochastic Gap Growth\".";
      throw( stcErr );
    }

    //Get the data member code
    m_iIsGapCode = mp_oGapLight->GetBoolDataCode( "Is Gap" );
    if ( -1 == m_iIsGapCode )
    {
      modelErr stcErr;
      stcErr.iErrorCode = BAD_DATA;
      stcErr.sFunction = "clGapLight::DoShellSetup" ;
      stcErr.sMoreInfo = "Gap Light grid was incorrectly set up.  Missing bool \"Is Gap\".";
      throw( stcErr );
    }

    //Set up the growth counter grid
    mp_oGrowthCounter = mp_oSimManager->CreateGrid( "Growth Counter", //grid name
        2, //number of int data members
        0, //number of float data members
        0, //number of char data members
        0, //number of bool data members
        mp_oGapLight->GetLengthXCells(), //X cell length
        mp_oGapLight->GetLengthYCells() ); //Y cell length

    //Register the data members
    m_iCurrentCode = mp_oGrowthCounter->RegisterInt( "Current" );
    m_iTargetCode = mp_oGrowthCounter->RegisterInt( "Target" );

    //********************************************
    //Set up the array of which species and types this applies to
    //********************************************
    m_iNumSpecies = p_oPop->GetNumberOfSpecies();

    mp_bAppliedTo = new bool*[m_iNumSpecies];
    for ( i = 0; i < m_iNumSpecies; i++ )
    {
      mp_bAppliedTo[i] = new bool[iNumTypes];
      for ( j = 0; j < iNumTypes; j++ )
      {
        mp_bAppliedTo[i] [j] = false;
      }
    }

    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      mp_bAppliedTo[mp_whatSpeciesTypeCombos[i].iSpecies] [mp_whatSpeciesTypeCombos[i].iType] = true;
    }

    //********************************************
    //Do a type/species search on all the types and species
    //********************************************
    strcpy( cQueryTemp, "species=" );
    for ( i = 0; i < m_iNumBehaviorSpecies - 1; i++ )
    {
      sprintf( cQueryPiece, "%d%s", mp_iWhatSpecies[i], "," );
      strcat( cQueryTemp, cQueryPiece );
    }
    sprintf( cQueryPiece, "%d", mp_iWhatSpecies[m_iNumBehaviorSpecies - 1] );
    strcat( cQueryTemp, cQueryPiece );

    //Find all the types
    for ( i = 0; i < m_iNumSpeciesTypeCombos; i++ )
    {
      if ( clTreePopulation::seedling == mp_whatSpeciesTypeCombos[i].iType )
      {
        bSeedling = true;
      }
      if ( clTreePopulation::sapling == mp_whatSpeciesTypeCombos[i].iType )
      {
        bSapling = true;
      }
      else if ( clTreePopulation::adult == mp_whatSpeciesTypeCombos[i].iType )
      {
        bAdult = true;
      }
      else if ( clTreePopulation::snag == mp_whatSpeciesTypeCombos[i].iType )
      {
        bSnag = true;
      }
    }
    strcat( cQueryTemp, "::type=" );
    if ( bSeedling )
    {
      sprintf( cQueryPiece, "%d%s", clTreePopulation::seedling, "," );
      strcat( cQueryTemp, cQueryPiece );
    }
    if ( bSapling )
    {
      sprintf( cQueryPiece, "%d%s", clTreePopulation::sapling, "," );
      strcat( cQueryTemp, cQueryPiece );
    }
    if ( bAdult )
    {
      sprintf( cQueryPiece, "%d%s", clTreePopulation::adult, "," );
      strcat( cQueryTemp, cQueryPiece );
    }
    if ( bSnag )
    {
      sprintf( cQueryPiece, "%d%s", clTreePopulation::snag, "," );
      strcat( cQueryTemp, cQueryPiece );
    }

    //Remove the last comma
    cQueryTemp[strlen( cQueryTemp ) - 1] = '\0';

    //Now put it in m_cQuery, sized correctly
    m_cQuery = new char[strlen( cQueryTemp ) + 1];
    strcpy( m_cQuery, cQueryTemp );
    delete[] cQueryTemp;
  }
  catch ( modelErr & err )
  {
    delete[] cQueryTemp;
    throw( err );
  }
  catch ( modelMsg & msg )
  {
    delete[] cQueryTemp;
    throw( msg );
  } //non-fatal error
  catch ( ... )
  {
    delete[] cQueryTemp;
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clStochasticGapGrowth::GetData" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clStochasticGapGrowth::Action() {
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oTrees;
    clTree * p_oTree;
    clAllometry *p_oAllom = p_oPop->GetAllometryObject();
    float *p_fAdultDiam10 = new float[m_iNumSpecies];
    float fX, fY;
    int iNumXCells = mp_oGrowthCounter->GetNumberXCells(), iNumYCells = mp_oGrowthCounter->GetNumberYCells(),
    iSp, iTp, iX, iY, iVal, iZero = 0, iTarget,
    iDeadCode, //dead code for a tree
    iIsDead;
    bool bIsGap;

    //Clear the grid values
    for ( iX = 0; iX < iNumXCells; iX++ )
    {
      for ( iY = 0; iY < iNumYCells; iY++ )
      {
        mp_oGrowthCounter->SetValueOfCell( iX, iY, m_iCurrentCode, iZero );
        mp_oGrowthCounter->SetValueOfCell( iX, iY, m_iTargetCode, iZero );
      }
    }

    //Get the diam10 value that corresponds to the minimum adult DBH, in case
    //we are promoting seedlings
    for (iSp = 0; iSp < m_iNumSpecies; iSp++) {
      p_fAdultDiam10[iSp] = p_oAllom->ConvertDbhToDiam10(p_oPop->GetMinAdultDBH(iSp), iSp);
    }

    //Get the trees to which this behavior applies.  For all non-gap cells,
    //count the eligible trees
    p_oTrees = p_oPop->Find( m_cQuery );
    p_oTree = p_oTrees->NextTree();
    while ( p_oTree )
    {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();
      if ( mp_bAppliedTo[iSp] [iTp] )
      {

        //Get the tree's location
        p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
        p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

        //Is this tree in a gap cell?
        mp_oGapLight->GetValueAtPoint(fX, fY, m_iIsGapCode, &bIsGap);

        //Make sure this tree is not dead from a previous disturbance
        iDeadCode = p_oPop->GetIntDataCode("dead", iSp, p_oTree->GetType());
        if (-1 != iDeadCode) {
          p_oTree->GetValue(iDeadCode, &iIsDead);
        } else iIsDead = notdead;

        if (bIsGap && notdead == iIsDead) {

          //Add this to the count in "Current"
          mp_oGrowthCounter->GetValueAtPoint(fX, fY, m_iCurrentCode, &iVal);
          iVal++;
          mp_oGrowthCounter->SetValueAtPoint(fX, fY, m_iCurrentCode, iVal);
        }
      }
      p_oTree = p_oTrees->NextTree();
    }

    //For all cells that were gap, pick a tree at random to elevate and put
    //it in "Target"; then put "Current" to 0
    for ( iX = 0; iX < iNumXCells; iX++ )
    {
      for ( iY = 0; iY < iNumYCells; iY++ )
      {
        mp_oGrowthCounter->GetValueOfCell(iX, iY, m_iCurrentCode, &iVal);
        if (iVal > 0) {
          iVal = clModelMath::RandomRound(clModelMath::GetRand() * iVal);
          if (iVal == 0) iVal = 1;
          mp_oGrowthCounter->SetValueOfCell( iX, iY, m_iCurrentCode, iZero );
          mp_oGrowthCounter->SetValueOfCell( iX, iY, m_iTargetCode, iVal );
        }
      }
    }

    //Now go back through the trees again and elevate the tree whenever
    //the counter in "Current" matches "Target"
    p_oTrees->StartOver();
    p_oTree = p_oTrees->NextTree();
    while ( p_oTree )
    {
      iSp = p_oTree->GetSpecies();
      iTp = p_oTree->GetType();
      if ( mp_bAppliedTo[iSp] [iTp] )
      {

        //Get the tree's location
        p_oTree->GetValue(p_oPop->GetXCode(iSp, iTp), &fX);
        p_oTree->GetValue(p_oPop->GetYCode(iSp, iTp), &fY);

        //Get the target value
        mp_oGrowthCounter->GetValueAtPoint( fX, fY, m_iTargetCode, &iTarget );

        if (iTarget > 0) {

          //Get the count in "Current" and increment
          mp_oGrowthCounter->GetValueAtPoint(fX, fY, m_iCurrentCode, &iVal);
          iVal++;

          //Check to see if we match "Target" yet
          if (iTarget != iVal) {
            //Not there - assign the value back and we're done
            mp_oGrowthCounter->SetValueAtPoint(fX, fY, m_iCurrentCode, iVal);
          }
          else {

            //This is the tree to elevate

            //If it's a seedling, we have to set diam10
            if (clTreePopulation::seedling == iTp) {
              p_oTree->SetValue(p_oPop->GetDiam10Code(iSp, iTp), p_fAdultDiam10[iSp], false, true);
            }
            //Otherwise set DBH
            else {
              p_oTree->SetValue(p_oPop->GetDbhCode(iSp, iTp), p_oPop->GetMinAdultDBH(iSp), false, true);
            }

            //Set the target to 0 to say we're done with this cell
            mp_oGrowthCounter->SetValueAtPoint( fX, fY, m_iTargetCode, iZero );
          }
        }
      }
      p_oTree = p_oTrees->NextTree();
    }

    delete[] p_fAdultDiam10;

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
    stcErr.sFunction = "clStochasticGapGrowth::Action" ;
    throw( stcErr );
  }
}
