//---------------------------------------------------------------------------
// GapLight.cpp
//---------------------------------------------------------------------------
#include "GapLight.h"
#include "SimManager.h"
#include "Grid.h"
#include "TreePopulation.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clGapLight::clGapLight(clSimManager * p_oSimManager) :
  clWorkerBase(p_oSimManager), clBehaviorBase(p_oSimManager),
      clLightBase(p_oSimManager) {

  //Set the namestring
  m_sNameString = "gaplightshell";
  m_sXMLRoot = "GapLight";

  m_bCalcedGaps = false;
  mp_oGapGrid = NULL;
  m_iGapCode = -1;

  m_bNeedsCommonParameters = false;
}

////////////////////////////////////////////////////////////////////////////
// DoShellSetup()
////////////////////////////////////////////////////////////////////////////
void clGapLight::DoShellSetup( xercesc::DOMDocument * p_oDoc )
{
  try
  {

    //Is there already a grid from the parameter file?
    mp_oGapGrid = mp_oSimManager->GetGridObject( "Gap Light" );

    if ( !mp_oGapGrid )
    {
      //Create the grid with one bool data member
      mp_oGapGrid = mp_oSimManager->CreateGrid( "Gap Light", 0, 0, 0, 1 );
      //Register the data member - called "Is Gap"
      m_iGapCode = mp_oGapGrid->RegisterBool( "Is Gap" );
    }
    else
    {
      //Get the data member code
      m_iGapCode = mp_oGapGrid->GetBoolDataCode( "Is Gap" );
      if ( -1 == m_iGapCode )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clGapLight::DoShellSetup" ;
        stcErr.sMoreInfo = "Gap Light grid was incorrectly set up in the parameter file.  Missing bool \"Is Gap\".";
        throw( stcErr );
      }
    }
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
    stcErr.sFunction = "clGapLight::DoShellSetup" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// CalculateGapStatus()
/////////////////////////////////////////////////////////////////////////////*/
void clGapLight::CalculateGapStatus( clTreePopulation * p_oPop )
{
  try
  {

    clTreeSearch * p_oAllTrees; //search object for getting all trees
    clTree * p_oTree; //for working with a single tree
    char cQuery[10];
    float fX, fY; //tree's X and Y coords
    int iIsDead;
    short int iSp, iType, //species and type of a given tree
    iNumXCells = mp_oGapGrid->GetNumberXCells(),
    iNumYCells = mp_oGapGrid->GetNumberYCells(),
    iDeadCode,     //code for whether or not a adult's dead
    i, j; //loop counters
    bool bNotGap = true;

    //Set all the values to true
    iNumXCells = mp_oGapGrid->GetNumberXCells();
    iNumYCells = mp_oGapGrid->GetNumberYCells();
    for ( i = 0; i < iNumXCells; i++ )
      for ( j = 0; j < iNumYCells; j++ )
        mp_oGapGrid->SetValueOfCell( i, j, m_iGapCode, bNotGap );

    bNotGap = false;

    //Ask the tree population to find all adults
    sprintf( cQuery, "%s%d", "type=", clTreePopulation::adult );
    p_oAllTrees = p_oPop->Find( cQuery );

    //Go through the trees one at a time
    p_oTree = p_oAllTrees->NextTree();
    while ( p_oTree )
    {
      iSp = p_oTree->GetSpecies();
      iType = p_oTree->GetType();

      //Make sure the neighbor's not dead
      iDeadCode = p_oPop->GetIntDataCode("dead", iSp, iType);
      if (-1 != iDeadCode) {
        p_oTree->GetValue(iDeadCode, &iIsDead);
      } else iIsDead = notdead;

      if (notdead == iIsDead) {

        //Get the tree's X and Y coordinates
        p_oTree->GetValue( p_oPop->GetXCode( iSp, iType ), & fX );
        p_oTree->GetValue( p_oPop->GetYCode( iSp, iType ), & fY );

        //Set this to not gap
        mp_oGapGrid->SetValueAtPoint( fX, fY, m_iGapCode, bNotGap );
      }

      p_oTree = p_oAllTrees->NextTree();
    } //end of while (p_oTree)
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
    stcErr.sFunction = "clGapLight::CalculateGapStatus" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// CalcLightValue()
////////////////////////////////////////////////////////////////////////////
float clGapLight::CalcLightValue( clTree * p_oTree, clTreePopulation * p_oPop )
{
  float fX, fY; //holders for the tree's X and Y location
  short int iSpecies, iType; //for holding species and type
  bool bIsGap;

  if (false == m_bCalcedGaps) {
    CalculateGapStatus(p_oPop);
    m_bCalcedGaps = true;
  }

  iSpecies = p_oTree->GetSpecies();
  iType = p_oTree->GetType();
  p_oTree->GetValue( p_oPop->GetXCode( iSpecies, iType ), & fX );
  p_oTree->GetValue( p_oPop->GetYCode( iSpecies, iType ), & fY );

  //Get the gap status in this tree's grid cell
  mp_oGapGrid->GetValueAtPoint( fX, fY, m_iGapCode, & bIsGap );
  if ( bIsGap ) return 100.0;
  else return 0.0;

}
