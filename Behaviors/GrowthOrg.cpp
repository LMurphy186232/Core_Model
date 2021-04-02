//---------------------------------------------------------------------------
#include "GrowthOrg.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "GrowthBase.h"
#include "Allometry.h"
#include <stdio.h>
#include <fstream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clGrowthOrg::clGrowthOrg( clGrowthBase * p_oHookedShell )
{
  try
  {

    //Initialize variables to 0, NULL, etc.
    mp_oDiameterGrowthTable = NULL;
    mp_oHeightGrowthTable = NULL;
    mp_iLightCodes = NULL;
    mp_iGrowthCodes = NULL;
    mp_iDiamCodes = NULL;
    mp_iHeightCodes = NULL;
    mp_fMaxTreeHeight = NULL;
    mp_oShellList = NULL;

    m_iTotalSpecies = 0;
    m_iTotalTypes = 0;
    m_iNumShells = 0;
    mp_oPop = NULL;

    //Make sure the hooked shell object pointer isn't NULL - if it is throw
    //error
    if ( NULL == p_oHookedShell )
    {
      modelErr stcErr;
      stcErr.sFunction = "clGrowthOrg::clGrowthOrg" ;
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw( stcErr );
    }

    //Otherwise - set the flag on the shell object that it's hooked
    p_oHookedShell->m_bHooked = true;
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
    stcErr.sFunction = "clGrowthOrg::clGrowthOrg" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clGrowthOrg::~clGrowthOrg()
{
  int i; //loop counter

  if ( mp_oDiameterGrowthTable )
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_oDiameterGrowthTable[i];

  if ( mp_oHeightGrowthTable )
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_oHeightGrowthTable[i];

  if ( mp_iLightCodes )
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_iLightCodes[i];

  if ( mp_iGrowthCodes )
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_iGrowthCodes[i];

  if ( mp_iDiamCodes )
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_iDiamCodes[i];

  if ( mp_iHeightCodes )
    for ( i = 0; i < m_iTotalSpecies; i++ )
      delete[] mp_iHeightCodes[i];

  delete[] mp_fMaxTreeHeight; mp_fMaxTreeHeight = NULL;

  delete[] mp_oDiameterGrowthTable; mp_oDiameterGrowthTable = NULL;
  delete[] mp_oHeightGrowthTable; mp_oHeightGrowthTable = NULL;
  delete[] mp_iGrowthCodes; mp_iGrowthCodes = NULL;
  delete[] mp_iLightCodes; mp_iLightCodes = NULL;
  delete[] mp_iDiamCodes; mp_iDiamCodes = NULL;
  delete[] mp_iHeightCodes; mp_iHeightCodes = NULL;
  delete[] mp_oShellList; mp_oShellList = NULL;
}


//////////////////////////////////////////////////////////////////////////////
// GetMaxTreeHeights
/////////////////////////////////////////////////////////////////////////////*/
void clGrowthOrg::GetMaxTreeHeights()
{
  float fAsymptoticInterval = 0.001; //the amount to shave off the maximum
  //canopy heights
  int iNumSpecies = mp_oPop->GetNumberOfSpecies(), i;

  //Declare and populate the maximum canopy heights, shaving off that little
  //bit for an asymptotic maximum
  clAllometry *p_oAllom = mp_oPop->GetAllometryObject();
  mp_fMaxTreeHeight = new float[iNumSpecies];
  for ( i = 0; i < iNumSpecies; i++ )
  {
    mp_fMaxTreeHeight[i] = p_oAllom->GetMaxTreeHeight( i );
    mp_fMaxTreeHeight[i] -= fAsymptoticInterval;
  }
}


//////////////////////////////////////////////////////////////////////////////
// DoSetup
//////////////////////////////////////////////////////////////////////////////
void clGrowthOrg::DoSetup( clSimManager * p_oSimManager, xercesc::DOMDocument * p_oDoc )
{
  try
  {
    clPopulationBase * p_oTempPop; //for retrieving the tree population

    //Double-check - if either pointer passed is NULL, throw an error
    if ( NULL == p_oSimManager || NULL == p_oDoc )
    {
      modelErr stcErr;
      stcErr.sFunction = "clGrowthOrg::DoSetup" ;
      stcErr.sMoreInfo = "Null pointer was passed to the function.";
      stcErr.iErrorCode = CANT_FIND_OBJECT;
      throw( stcErr );
    }

    //Get the tree population object, and from it the numbers of species and types
    p_oTempPop = p_oSimManager->GetPopulationObject( "treepopulation" );
    mp_oPop = ( clTreePopulation * ) p_oTempPop;

    //Get the return codes for the "Light" data member variable for each
    //species/type combo that has a shell pointer in the functions table
    GetLightVariableCodes();

    //Get the return codes for the diameters at which to apply growth for each
    //species/type combo that has a shell pointer in the functions table
    GetDiamVariableCodes();

    //Get the return codes for the "height" data member variable for each
    //species/type combo that has a shell pointer in the functions table
    GetHeightVariableCodes();

    //Get the asymptotic tree height limits
    GetMaxTreeHeights();

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
    stcErr.sFunction = "clGrowthOrg::DoSetup" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// PopulateGrowthFunctionsTable()
//////////////////////////////////////////////////////////////////////////////
void clGrowthOrg::PopulateGrowthTables( clSimManager * p_oSimManager )
{
  try
  {
    clBehaviorBase * p_oTempBehavior; //for going through behaviors looking for
    //growth shells
    clGrowthBase * p_oGrowthShell; //growth shell object
    std::string sBehaviorName, //behavior namestrings
                sShellMarker = "growthshell";
    stcSpeciesTypeCombo combo; //species/type combo from a behavior
    short int iNumBehaviors, //total number of behaviors
         iNumCombos, //number of species/type combos for a behavior
         i, j; //loop counters
    clGrowthBase::growthType iGrowthType; //type of growth behavior

    //Declare the tables - they're # species by # types
    mp_oDiameterGrowthTable = new clGrowthBase**[m_iTotalSpecies];
    for ( i = 0; i < m_iTotalSpecies; i++ )
    {
      mp_oDiameterGrowthTable[i] = new clGrowthBase*[m_iTotalTypes];
      for ( j = 0; j < m_iTotalTypes; j++ )
        mp_oDiameterGrowthTable[i] [j] = NULL;
    }

    mp_oHeightGrowthTable = new clGrowthBase**[m_iTotalSpecies];
    for ( i = 0; i < m_iTotalSpecies; i++ )
    {
      mp_oHeightGrowthTable[i] = new clGrowthBase*[m_iTotalTypes];
      for ( j = 0; j < m_iTotalTypes; j++ )
        mp_oHeightGrowthTable[i] [j] = NULL;
    }

    //Now go through all the behaviors to pick out the growth shells - should
    //have the string "growthshell" in their namestrings
    //First time through - count the number of shells
    iNumBehaviors = p_oSimManager->GetNumberOfBehaviors();
    m_iNumShells = 0;
    for ( i = 0; i < iNumBehaviors; i++ )
    {
      p_oTempBehavior = p_oSimManager->GetBehaviorObject( i );
      sBehaviorName = p_oTempBehavior->GetName();
      if (std::string::npos != sBehaviorName.find(sShellMarker)) {
        m_iNumShells++;
      }
    }
    mp_oShellList = new clGrowthBase*[m_iNumShells];

    //Second time through - populate all the tables
    m_iNumShells = 0;
    for ( i = 0; i < iNumBehaviors; i++ )
    {
      p_oTempBehavior = p_oSimManager->GetBehaviorObject( i );
      sBehaviorName = p_oTempBehavior->GetName();
      if (std::string::npos != sBehaviorName.find(sShellMarker)) {
        //This is a growth shell object - cast its pointer as such
        p_oGrowthShell = dynamic_cast < clGrowthBase * > ( p_oTempBehavior );

        mp_oShellList[m_iNumShells] = p_oGrowthShell;
        m_iNumShells++;

        //See if it identifies as diameter-only or auto-diameter updater, in
        //which case it goes in the diameter table, or as height only, in which
        //case it goes in the height table
        iGrowthType = p_oGrowthShell->GetGrowthMethod();

        if ( clGrowthBase::diameter_auto == iGrowthType || clGrowthBase::diameter_only == iGrowthType )
        {

          //Find out which species/type combos this shell wants to act on,
          //and set its pointer in that place in the table
          iNumCombos = p_oGrowthShell->GetNumSpeciesTypeCombos();
          for ( j = 0; j < iNumCombos; j++ )
          {
            combo = p_oGrowthShell->GetSpeciesTypeCombo( j );

            //Make sure another growth shell hasn't already claimed this combo -
            //if it has, throw an error.  If not, assign this pointer
            if ( mp_oDiameterGrowthTable[combo.iSpecies] [combo.iType] != NULL )
            {
              modelErr stcErr;
              stcErr.sFunction = "clGrowthOrg::PopulateGrowthTables";
              std::stringstream s;
              s << "Type/species combo species=" << combo.iSpecies
                << " type=" << combo.iType << " has more than one growth object assigned.";
              stcErr.sMoreInfo = s.str();
              stcErr.iErrorCode = DATA_READ_ONLY;
              throw( stcErr );
            }
            else
              mp_oDiameterGrowthTable[combo.iSpecies] [combo.iType] = p_oGrowthShell;
          }
        }
        else if ( clGrowthBase::height_only == iGrowthType )
        {
          //Find out which species/type combos this shell wants to act on,
          //and set its pointer in that place in the table
          iNumCombos = p_oGrowthShell->GetNumSpeciesTypeCombos();
          for ( j = 0; j < iNumCombos; j++ )
          {
            combo = p_oGrowthShell->GetSpeciesTypeCombo( j );

            //Make sure another growth shell hasn't already claimed this combo -
            //if it has, throw an error.  If not, assign this pointer
            if ( mp_oHeightGrowthTable[combo.iSpecies] [combo.iType] != NULL )
            {
              modelErr stcErr;
              stcErr.sFunction = "clGrowthOrg::PopulateGrowthTables" ;
              std::stringstream s;
              s << "Type/species combo species=" << combo.iSpecies
                << " type=" << combo.iType << " has more than one growth object assigned.";
              stcErr.sMoreInfo = s.str();
              stcErr.iErrorCode = DATA_READ_ONLY;
              throw( stcErr );
            }
            else
              mp_oHeightGrowthTable[combo.iSpecies] [combo.iType] = p_oGrowthShell;

          }
        }
      } //end of if (NULL != strstr(cBehaviorName, cShellMarker))
    } //end of for (i = 0; i < iNumBehaviors; i++)

    //Validate the tables - make sure that all species/type combos using
    //either a diameter-only or height-only growth behavior have the other
    //kind too
    for ( i = 0; i < m_iTotalSpecies; i++ )
    {
      for ( j = 0; j < m_iTotalTypes; j++ )
      {

        if ( ( NULL == mp_oDiameterGrowthTable[i] [j] && NULL != mp_oHeightGrowthTable[i] [j] )
             || ( NULL != mp_oDiameterGrowthTable[i] [j]
             && clGrowthBase::diameter_only == mp_oDiameterGrowthTable[i] [j]->GetGrowthMethod()
             && NULL == mp_oHeightGrowthTable[i] [j] ) )
             {

               modelErr stcErr;
               stcErr.sFunction = "clGrowthOrg::PopulateGrowthTables" ;
               std::stringstream s;
               s << "Type/species combo species=" << i << " type=" << j
                 << " needs both a diameter and height growth incrementer.";
               stcErr.sMoreInfo = s.str();
               stcErr.iErrorCode = DATA_READ_ONLY;
               throw( stcErr );
        }
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
    stcErr.sFunction = "clGrowthOrg::PopulateGrowthTables" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetLightVariableCodes()
//////////////////////////////////////////////////////////////////////////////
void clGrowthOrg::GetLightVariableCodes()
{
  try
  {
    char cLabel[] = "Light";
    short int i, j; //loop counters

    //Declare and initialize the return codes array
    mp_iLightCodes = new short int * [m_iTotalSpecies];
    for ( i = 0; i < m_iTotalSpecies; i++ )
    {
      mp_iLightCodes[i] = new short int[m_iTotalTypes];
      for ( j = 0; j < m_iTotalTypes; j++ )
        mp_iLightCodes[i] [j] = -1;
    }

    //Now go through the growth functions table and get the code for
    //each species/type combo with a valid pointer
    for ( i = 0; i < m_iTotalSpecies; i++ )
      for ( j = 0; j < m_iTotalTypes; j++ )
        if ( NULL != mp_oDiameterGrowthTable[i] [j] ) {
          mp_iLightCodes[i] [j] = mp_oPop->GetFloatDataCode( cLabel, i, j );
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
    stcErr.sFunction = "clGrowthOrg::GetLightVariableCodes" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetDiamVariableCodes()
//////////////////////////////////////////////////////////////////////////////
void clGrowthOrg::GetDiamVariableCodes()
{
  try
  {
    short int i, j; //loop counters

    //Declare and initialize the return codes array
    mp_iDiamCodes = new short int * [m_iTotalSpecies];
    for ( i = 0; i < m_iTotalSpecies; i++ )
    {
      mp_iDiamCodes[i] = new short int[m_iTotalTypes];
      for ( j = 0; j < m_iTotalTypes; j++ )
        mp_iDiamCodes[i] [j] = -1;
    }

    //Now go through the growth functions table and get the code for
    //each species/type combo with a valid pointer
    for ( i = 0; i < m_iTotalSpecies; i++ )
      for ( j = 0; j < m_iTotalTypes; j++ )
        if ( NULL != mp_oDiameterGrowthTable[i] [j] )
        {
          if ( clTreePopulation::seedling == j || clTreePopulation::sapling == j )
            mp_iDiamCodes[i] [j] = mp_oPop->GetDiam10Code( i, j );
          else
            mp_iDiamCodes[i] [j] = mp_oPop->GetDbhCode( i, j );

          //If the return code is -1, throw an error
          if ( -1 == mp_iDiamCodes[i] [j] )
          {
            modelErr stcErr;
            stcErr.sFunction = "clGrowthOrg::GetDiamVariableCodes";
            std::stringstream s;
            s << "Type/species combo species=" << i << " type=" << j
              << " does not have diameter as expected.";
            stcErr.sMoreInfo = s.str();
            stcErr.iErrorCode = BAD_DATA;
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
    stcErr.sFunction = "clGrowthOrg::GetDiamVariableCodes" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetHeightVariableCodes()
//////////////////////////////////////////////////////////////////////////////
void clGrowthOrg::GetHeightVariableCodes()
{
  try
  {
    short int i, j; //loop counters

    //Declare and initialize the return codes array
    mp_iHeightCodes = new short int * [m_iTotalSpecies];
    for ( i = 0; i < m_iTotalSpecies; i++ )
    {
      mp_iHeightCodes[i] = new short int[m_iTotalTypes];
      for ( j = 0; j < m_iTotalTypes; j++ )
        mp_iHeightCodes[i] [j] = -1;
    }

    //Now go through the height growth functions table and get the code for
    //each species/type combo with a valid pointer
    for ( i = 0; i < m_iTotalSpecies; i++ )
      for ( j = 0; j < m_iTotalTypes; j++ )
        if ( NULL != mp_oHeightGrowthTable[i] [j] )
        {
          mp_iHeightCodes[i] [j] = mp_oPop->GetHeightCode( i, j );

          //If the return code is -1, throw an error
          if ( -1 == mp_iHeightCodes[i] [j] )
          {
            modelErr stcErr;
            stcErr.sFunction = "clGrowthOrg::GetHeightVariableCodes";
            std::stringstream s;
            s << "Type/species combo species=" << i << " type=" << j
              << " does not have height as expected.";
            stcErr.sMoreInfo = s.str();
            stcErr.iErrorCode = BAD_DATA;
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
    stcErr.sFunction = "clGrowthOrg::GetDiamVariableCodes" ;
    throw( stcErr );
  }
}


//////////////////////////////////////////////////////////////////////////////
// DoGrowthAssignments()
//////////////////////////////////////////////////////////////////////////////
void clGrowthOrg::DoGrowthAssignments()
{
  try
  {
    clTreeSearch * p_oAllTrees; //search object for getting all trees
    clTree * p_oTree; //for working with a single tree
    float fDiamGrowthVal, //diameter growth value to be assigned to the tree
         fHeightGrowthVal, //height growth value to be assigned to the tree
         fOldDiam, //old diameter value to be changed
         fOldHeight, //old height value to be changed
         fGrowthMemberVal; //new value going into "Growth" data member
    int iDead;
    short int iSp, //species of a given tree
               iDeadCode; //dead code for a tree
    clGrowthBase::growthType iGrowthType; //type of growth behavior

    //Call the PreGrowthCalcs() function for all shells
    for (int i = 0; i < m_iNumShells; i++) {
      mp_oShellList[i]->PreGrowthCalcs( mp_oPop );
    }

    //Ask the tree population to find all trees
    p_oAllTrees = mp_oPop->Find( "all" );

    //Go through the trees one at a time
    p_oTree = p_oAllTrees->NextTree();
    while ( p_oTree )
    {
      //Cache tree species but not tree type - it can change
      iSp = p_oTree->GetSpecies();

      //Check to see if this species/type combo has a diameter growth
      //shell to calculate its value - if it does, get it
      if ( NULL != mp_oDiameterGrowthTable[iSp] [p_oTree->GetType()] )
      {

        //Make sure this tree is not dead from a previous disturbance
        iDeadCode = mp_oPop->GetIntDataCode("dead", iSp, p_oTree->GetType());
        if (-1 != iDeadCode) {
          p_oTree->GetValue(iDeadCode, &iDead);
          if (iDead > notdead) goto nextTree;
        }

        iGrowthType = mp_oDiameterGrowthTable[iSp] [p_oTree->GetType()]->GetGrowthMethod();

        //If we're using both diam and height growth, and diam growth wants to
        //go last, and height doesn't object, call height first
        if (clGrowthBase::diameter_only == iGrowthType &&
            mp_oDiameterGrowthTable[iSp][p_oTree->GetType()]->m_bGoLast &&
            !mp_oHeightGrowthTable[iSp][p_oTree->GetType()]->m_bGoLast) {

          fHeightGrowthVal = mp_oHeightGrowthTable[iSp] [p_oTree->GetType()]->CalcHeightGrowthValue( p_oTree, mp_oPop, 0 );
          fDiamGrowthVal = mp_oDiameterGrowthTable[iSp] [p_oTree->GetType()]->CalcDiameterGrowthValue( p_oTree, mp_oPop, fHeightGrowthVal );
        } else {

          //Either growth only, or growth goes first
          fDiamGrowthVal = mp_oDiameterGrowthTable[iSp] [p_oTree->GetType()]->CalcDiameterGrowthValue( p_oTree, mp_oPop, 0 );

          //If appropriate, get the height increment as well
          if ( clGrowthBase::diameter_only == iGrowthType )
          {
            fHeightGrowthVal = mp_oHeightGrowthTable[iSp] [p_oTree->GetType()]->CalcHeightGrowthValue( p_oTree, mp_oPop, fDiamGrowthVal );
          }
        }

        //Get the new "Growth" value, if appropriate
        fGrowthMemberVal = mp_oDiameterGrowthTable[iSp] [p_oTree->GetType()]->GetGrowthMemberValue(p_oTree, fDiamGrowthVal);

        //Add the values to the tree.  Delay tree population updates until all trees are done
        p_oTree->GetValue( mp_iDiamCodes[iSp] [p_oTree->GetType()], & fOldDiam );
        fDiamGrowthVal += fOldDiam;

        if ( clGrowthBase::diameter_auto == iGrowthType )
        {
          //Allow the height to update automatically
          p_oTree->SetValue( mp_iDiamCodes[iSp] [p_oTree->GetType()], fDiamGrowthVal, false, true );
        }
        else if ( clGrowthBase::diameter_only == iGrowthType )
        {

          //Do not allow the height to update automatically
          p_oTree->SetValue( mp_iDiamCodes[iSp] [p_oTree->GetType()], fDiamGrowthVal, false, false );

          //Increment the height as well
          p_oTree->GetValue( mp_iHeightCodes[iSp] [p_oTree->GetType()], & fOldHeight );
          fHeightGrowthVal += fOldHeight;
          if (fHeightGrowthVal > mp_fMaxTreeHeight[iSp]) fHeightGrowthVal =  mp_fMaxTreeHeight[iSp];
          p_oTree->SetValue( mp_iHeightCodes[iSp] [p_oTree->GetType()], fHeightGrowthVal, false, false );
        }

        if (mp_iGrowthCodes[iSp][p_oTree->GetType()] > -1) {
          p_oTree->SetValue(mp_iGrowthCodes[iSp][p_oTree->GetType()], fGrowthMemberVal);
        }
      }

      nextTree:
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
    stcErr.sFunction = "clGrowthOrg::DoGrowthAssignments" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// DoTreeDataMemberRegistrations()
//////////////////////////////////////////////////////////////////////////////
void clGrowthOrg::DoTreeDataMemberRegistrations( clSimManager * p_oSimManager, clTreePopulation * p_oPop )
{
  try
  {
    char cLabel[] = "Growth";
    short int i, j, //loop counters
         iTotalSpecies = p_oPop->GetNumberOfSpecies(), iTotalTypes = p_oPop->GetNumberOfTypes();

    m_iTotalSpecies = p_oPop->GetNumberOfSpecies();
    m_iTotalTypes = p_oPop->GetNumberOfTypes();

    //Declare and initialize the return codes array
    mp_iGrowthCodes = new short int * [iTotalSpecies];
    for ( i = 0; i < iTotalSpecies; i++ )
    {
      mp_iGrowthCodes[i] = new short int[iTotalTypes];
      for ( j = 0; j < iTotalTypes; j++ )
        mp_iGrowthCodes[i] [j] = -1;
    }

    //Assemble the light functions table
    PopulateGrowthTables( p_oSimManager );

    //Now go through the light functions table and register the variable for
    //each species/type combo with a valid pointer
    for ( i = 0; i < iTotalSpecies; i++ )
      for ( j = 0; j < iTotalTypes; j++ )
        if ( NULL != mp_oDiameterGrowthTable[i] [j] )
          mp_iGrowthCodes[i] [j] = p_oPop->RegisterFloat( cLabel, i, j );

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
    stcErr.sFunction = "clGrowthOrg::DoTreeDataMemberRegistrations" ;
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// GetGrowthCode()
//////////////////////////////////////////////////////////////////////////////
short int clGrowthOrg::GetGrowthCode(short int iSp, short int iTp)
   {if (iSp < 0 || iSp >= m_iTotalSpecies) {
     modelErr stcErr;
     stcErr.iErrorCode = BAD_DATA;
     stcErr.sFunction = "clGrowthOrg::GetGrowthCode";
     std::stringstream s;
     s << "Invalid species " << iSp << ".";
     stcErr.sMoreInfo = s.str();
     throw( stcErr );
   }
   if (iTp < 0 || iTp >= m_iTotalTypes) {
     modelErr stcErr;
     stcErr.iErrorCode = BAD_DATA;
     stcErr.sFunction = "clGrowthOrg::GetGrowthCode";
     std::stringstream s;
     s << "Invalid type " << iTp << ".";
     throw( stcErr );
   }
   return mp_iGrowthCodes[iSp][iTp];
 }
