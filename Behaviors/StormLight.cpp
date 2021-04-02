//---------------------------------------------------------------------------
#include "StormLight.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "Grid.h"
#include "TreePopulation.h"
#include <stdio.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
clStormLight::clStormLight( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{

  //Set the namestring
  m_sNameString = "StormLight";
  m_sXMLRoot = "StormLight";

  mp_iStmDmgCodes = NULL;
  mp_oLightGrid = NULL;

  m_iMaxSnagDmgTime = 0;
  m_iStochasticity = deterministic_pdf;
  m_iGridLightCode = -1;
  m_fMinCanopyTrees = 0;
  m_iMaxDmgTime = 0;
  m_fMaxRadius = 0;
  m_iNumTypes = 0;
  m_fRandParameter = 0;
  m_fSlope = 0;
  m_fIntercept = 0;

  //Allowed file types
  m_iNumAllowedTypes = 2;
  mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
  mp_iAllowedFileTypes[0] = parfile;
  mp_iAllowedFileTypes[1] = detailed_output;

  //Versions
  m_fVersionNumber = 1.1;
  m_fMinimumVersionNumber = 1;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clStormLight::~clStormLight()
{
  if ( mp_iStmDmgCodes )
  {
    for ( int i = 0; i < m_iNumTypes; i++ )
      delete[] mp_iStmDmgCodes[i];
  }
  delete[] mp_iStmDmgCodes;
}

////////////////////////////////////////////////////////////////////////////
// GetData()
////////////////////////////////////////////////////////////////////////////
void clStormLight::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    DOMElement * p_oElement = GetParentParametersElement(p_oDoc);

    //**********************************************
    //Read values from the parameter file
    //**********************************************
    //Get the type of stochasticity
    int iTemp;
    FillSingleValue( p_oElement, "li_stormLightStoch", & iTemp, true );

    //Make sure the value is valid
    if ( deterministic_pdf == iTemp )
    {
      m_iStochasticity = deterministic_pdf;
    }
    else if ( lognormal_pdf == iTemp )
    {
      m_iStochasticity = lognormal_pdf;
    }
    else if ( normal_pdf == iTemp )
    {
      m_iStochasticity = normal_pdf;
    }
    else
    {
      modelErr stcErr;
      stcErr.sFunction = "clStormLight::GetData" ;
      std::stringstream s;
      s << "Unrecognized value for stochasticity: " << iTemp;
      stcErr.sMoreInfo = s.str();
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Max radius
    FillSingleValue( p_oElement, "li_stormLightRadius", & m_fMaxRadius, true );

    //The slope of the light function
    FillSingleValue( p_oElement, "li_stormLightSlope", & m_fSlope, true );

    //The intercept of the light function
    FillSingleValue( p_oElement, "li_stormLightIntercept", & m_fIntercept, true );

    //The max time since damage that a tree can count toward light calculations
    FillSingleValue( p_oElement, "li_stormLightMaxDmgTime", & m_iMaxDmgTime, true );

    //The max time since damage that a snag can count toward light calculations
    FillSingleValue( p_oElement, "li_stormLightSnagMaxDmgTime", & m_iMaxSnagDmgTime, true );

    //Minimum number of trees for full canopy
    FillSingleValue( p_oElement, "li_stormLightMinFullCanopy", & m_fMinCanopyTrees, true );
    //Error if this is negative
    if (m_fMinCanopyTrees < 0) {
      modelErr stcErr;
      stcErr.sFunction = "clStormLight::GetData" ;
      stcErr.sMoreInfo = "Minimum number of trees for full canopy cannot be less than 0.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //If a stochastic method is used, the standard deviation
    if ( deterministic_pdf != m_iStochasticity )
    {
      FillSingleValue( p_oElement, "li_stormLightRandPar", & m_fRandParameter, true );
    }

    //**********************************************
    //Set up the light grid
    //**********************************************
    //Is there already a grid from the parameter file?
    mp_oLightGrid = mp_oSimManager->GetGridObject( "Storm Light" );

    if ( !mp_oLightGrid )
    {
      //Create the grid with one float data member
      mp_oLightGrid = mp_oSimManager->CreateGrid( "Storm Light", 0, 1, 0, 0 );
      //Register the data member - called "Light"
      m_iGridLightCode = mp_oLightGrid->RegisterFloat( "Light" );
    }
    else
    {
      //Get the data member code
      m_iGridLightCode = mp_oLightGrid->GetFloatDataCode( "Light" );
      if ( -1 == m_iGridLightCode )
      {
        modelErr stcErr;
        stcErr.iErrorCode = BAD_DATA;
        stcErr.sFunction = "clStormLight::GetData" ;
        stcErr.sMoreInfo = "\"Storm Light\" grid was incorrectly set up in the parameter file.  Missing float \"Light\".";
        throw( stcErr );
      }
    }

    //**********************************************
    //Get storm damage codes
    //**********************************************
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    int i, j, iNumSpecies = p_oPop->GetNumberOfSpecies();
    m_iNumTypes = 2;

    mp_iStmDmgCodes = new int * [m_iNumTypes];
    for ( i = 0; i < m_iNumTypes; i++ )
    {
      mp_iStmDmgCodes[i] = new int[iNumSpecies];
      for ( j = 0; j < iNumSpecies; j++ )
      {
        mp_iStmDmgCodes[i] [j] = -1;
      }
    }

    //Get the values for adults and snags
    for ( i = 0; i < iNumSpecies; i++ )
    {
      mp_iStmDmgCodes[adult] [i] = p_oPop->GetIntDataCode( "stm_dmg", i, clTreePopulation::adult );
      mp_iStmDmgCodes[snag] [i] = p_oPop->GetIntDataCode( "stm_dmg", i, clTreePopulation::snag );
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
    stcErr.sFunction = "clStormLight::GetData" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Action()
////////////////////////////////////////////////////////////////////////////
void clStormLight::Action()
{
  try
  {
    clTreePopulation * p_oPop = ( clTreePopulation * ) mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreeSearch * p_oNeighbors; //neighborhood trees to check for storm damage
    clTree * p_oNeighbor; //neighbor to check for storm damage
    char cQuery[75]; //format search strings into this
    float fLight, //light level
          fMinAdultHeight = p_oPop->GetMinimumAdultHeight(),
         fX, fY; //holders for the tree's X and Y location
    int iDamage, //tree damage level
        iNumDamagedTrees, //number of damaged trees
        iNumCanopyTrees, //number of trees that count towards full canopy
        iNumXCells = mp_oLightGrid->GetNumberXCells(),
        iNumYCells = mp_oLightGrid->GetNumberYCells(),
        iSp, iTp,
        iX, iY;

    for ( iX = 0; iX < iNumXCells; iX++ )
    {
      for ( iY = 0; iY < iNumYCells; iY++ )
      {
        //First find the point at the center of the grid cell - place in fX
        //and fY
        mp_oLightGrid->GetPointOfCell( iX, iY, & fX, & fY );

        iNumDamagedTrees = 0;
        iNumCanopyTrees = 0;

        //Get a list of all trees that are within the search radius and taller
        //than the minimum adult height
        sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", m_fMaxRadius, " FROM x=", fX, "y=", fY,
             "::height=", fMinAdultHeight );
        p_oNeighbors = p_oPop->Find( cQuery );

        p_oNeighbor = p_oNeighbors->NextTree();
        while ( p_oNeighbor != NULL )
        {

          iSp = p_oNeighbor->GetSpecies();
          iTp = p_oNeighbor->GetType();

          //Skip seedlings and saplings
          if ( clTreePopulation::seedling != iTp && clTreePopulation::sapling != iTp ) {

            //Count towards the full canopy
            iNumCanopyTrees++;

            //Get damage
            if (clTreePopulation::adult == iTp)
              iDamage = mp_iStmDmgCodes[adult][iSp];
            else iDamage = mp_iStmDmgCodes[snag][iSp];

            //Skip those trees with no storm damage tag
            if (-1 != iDamage) {

              //Get the damage tag - dangerously double-using iDamage
              p_oNeighbor->GetValue(iDamage, &iDamage);

              //Natural mortality - get the age if this is a snag with no
              //damage; make it a fake damage
              if (0 == iDamage && clTreePopulation::snag == iTp) {
                p_oNeighbor->GetValue(p_oPop->GetIntDataCode("Age", iSp, iTp), &iDamage);
                iDamage += 2000;
              }

              //If it's a snag, or an adult with heavy damage, get the time
              if ((clTreePopulation::snag == iTp && iDamage > 0) ||
                  (clTreePopulation::adult == iTp && iDamage >= 2000)) {
                iDamage = iDamage > 2000 ? iDamage % 2000 : iDamage % 1000;
                if ((clTreePopulation::adult == iTp && iDamage < m_iMaxDmgTime) ||
                    (clTreePopulation::snag == iTp && iDamage < m_iMaxSnagDmgTime))
                  //Finally!  increment
                  iNumDamagedTrees++;
              }
            } else if (clTreePopulation::snag == iTp) {
              //This is the case of trees to which damage is not applied,
              //but are naturally dead.
              p_oNeighbor->GetValue(p_oPop->GetIntDataCode("Age", iSp, iTp), &iDamage);
              if (iDamage < m_iMaxSnagDmgTime)
                iNumDamagedTrees++;
            }
          }

          p_oNeighbor = p_oNeighbors->NextTree();
        } //end of while (neighbor != NULL)

        //Calculate GLI
        if (0 == iNumCanopyTrees) {
          //Allow for possible negative intercept
          fLight = 100 + m_fIntercept;
        } else if (iNumCanopyTrees < m_fMinCanopyTrees) {
          fLight = ((1 - (iNumCanopyTrees/m_fMinCanopyTrees)) * 100) +
                   clModelMath::CalcPointValue(
                        (float)iNumDamagedTrees/(float)iNumCanopyTrees,
                        m_fSlope, m_fIntercept);
        } else {
          fLight = clModelMath::CalcPointValue(
                           (float)iNumDamagedTrees/(float)iNumCanopyTrees,
                           m_fSlope, m_fIntercept);
        }

        //Do any stochastic adjustments
        if (normal_pdf == m_iStochasticity) {
          fLight += clModelMath::NormalRandomDraw(m_fRandParameter);
        } else if (lognormal_pdf == m_iStochasticity) {
          fLight = clModelMath::LognormalRandomDraw(fLight, m_fRandParameter);
        }

        if (fLight < 0) fLight = 0;
        if (fLight > 100) fLight = 100;

        //Assign GLI to the grid
        mp_oLightGrid->SetValueAtPoint( fX, fY, m_iGridLightCode, fLight );
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
    stcErr.sFunction = "clStormLight::Action" ;
    throw( stcErr );
  }
}
