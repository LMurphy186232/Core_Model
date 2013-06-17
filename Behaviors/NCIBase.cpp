//---------------------------------------------------------------------------
// NCIBase.cpp
//---------------------------------------------------------------------------
#include "NCIBase.h"
#include "TreePopulation.h"
#include "SimManager.h"
#include "Plot.h"
#include "Constants.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clNCIBase::clNCIBase()
{
  try
  {
    //Null out our pointers
    mp_fAlpha = NULL;
    mp_fBeta = NULL;
    mp_fMaxPotentialValue = NULL;
    mp_fLambda = NULL;
    mp_fMaxCrowdingRadius = NULL;
    mp_iIndexes = NULL;
    mp_fCrowdingSlope = NULL;
    mp_fCrowdingSteepness = NULL;
    mp_fXb = NULL;
    mp_fX0 = NULL;
    mp_fShadingCoefficient = NULL;
    mp_fShadingExponent = NULL;
    mp_fMedDamageEta = NULL;
    mp_fFullDamageEta = NULL;
    mp_fGamma = NULL;
    mp_fMinimumNeighborDBH = NULL;
    mp_fMedDamageStormEff = NULL;
    mp_fFullDamageStormEff = NULL;
    mp_iDamageCodes = NULL;
    mp_ShadingEffect = NULL;
    mp_CrowdingEffect = NULL;
    mp_NCI = NULL;
    mp_iLightCodes = NULL;

    m_bIncludeSnags = false;
    m_fDbhDivisor = 0;
    m_iNumTotalSpecies = 0;
    m_fMinSaplingHeight = 0;
    m_iNumBehSpecies = 0;
    DamageEffect = NULL;
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
    stcErr.sFunction = "clNCIBase::clNCIBase" ;
    throw( stcErr );
  }
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
clNCIBase::~clNCIBase()
{
  int i;

  delete[] mp_fAlpha;
  delete[] mp_fBeta;
  delete[] mp_fMaxPotentialValue;
  delete[] mp_fMaxCrowdingRadius;
  if ( mp_fLambda )
    for ( i = 0; i < m_iNumBehSpecies; i++ )
      delete[] mp_fLambda[i];
  delete[] mp_fLambda;
  delete[] mp_iIndexes;
  delete[] mp_fCrowdingSlope;
  delete[] mp_fCrowdingSteepness;
  delete[] mp_fXb;
  delete[] mp_fX0;
  delete[] mp_fMinimumNeighborDBH;
  delete[] mp_fShadingCoefficient;
  delete[] mp_fShadingExponent;
  delete[] mp_fMedDamageEta;
  delete[] mp_fFullDamageEta;
  delete[] mp_fGamma;
  delete[] mp_fMedDamageStormEff;
  delete[] mp_fFullDamageStormEff;
  if ( mp_iDamageCodes )
  {
    for ( int i = 0; i < m_iNumTotalSpecies; i++ )
      delete[] mp_iDamageCodes[i];
  }
  delete[] mp_iDamageCodes;
  if ( mp_iLightCodes )
  {
    for ( int i = 0; i < m_iNumBehSpecies; i++ )
      delete[] mp_iLightCodes[i];
  }
  delete[] mp_iLightCodes;
  delete[] mp_ShadingEffect;
  delete[] mp_CrowdingEffect;
  delete[] mp_NCI; 
}

//////////////////////////////////////////////////////////////////////////////
// DoNCISetup()
//////////////////////////////////////////////////////////////////////////////
void clNCIBase::DoNCISetup( clTreePopulation * p_oPop, int iNumBehaviorSpecies )
{
  int i, j;

  m_iNumBehSpecies = iNumBehaviorSpecies;
  m_iNumTotalSpecies = p_oPop->GetNumberOfSpecies();

  m_fMinSaplingHeight = 50;
  //Get the minimum sapling height
  for ( i = 0; i < m_iNumTotalSpecies; i++ )
  {
    if ( p_oPop->GetMaxSeedlingHeight( i ) < m_fMinSaplingHeight )
    {
      m_fMinSaplingHeight = p_oPop->GetMaxSeedlingHeight( i );
    }
  }

  mp_iIndexes = new short int[m_iNumTotalSpecies];
  //This one alone is sized total number of species
  mp_fMinimumNeighborDBH = new float[clNCIBase::m_iNumTotalSpecies];

  //The rest are sized number of species to which this behavior applies
  mp_fMedDamageEta = new float[m_iNumBehSpecies];
  mp_fFullDamageEta = new float[m_iNumBehSpecies];
  mp_fAlpha = new float[m_iNumBehSpecies];
  mp_fBeta = new float[m_iNumBehSpecies];
  mp_fCrowdingSlope = new float[m_iNumBehSpecies];
  mp_fCrowdingSteepness = new float[m_iNumBehSpecies];
  mp_fXb = new float[m_iNumBehSpecies];
  mp_fMaxPotentialValue = new float[m_iNumBehSpecies];
  mp_fX0 = new float[m_iNumBehSpecies];
  mp_fGamma = new float[m_iNumBehSpecies];
  mp_fMedDamageStormEff = new float[m_iNumBehSpecies];
  mp_fFullDamageStormEff = new float[m_iNumBehSpecies];
  mp_fMaxCrowdingRadius = new float[m_iNumBehSpecies];
  mp_fLambda = new float * [m_iNumBehSpecies];
  mp_fShadingCoefficient = new float[m_iNumBehSpecies];
  mp_fShadingExponent = new float[m_iNumBehSpecies];
  for ( i = 0; i < m_iNumBehSpecies; i++ )
  {
    mp_fLambda[i] = new float[m_iNumTotalSpecies];
    for ( j = 0; j < m_iNumTotalSpecies; j++ )
      mp_fLambda[i] [j] = 0;
  }
}

////////////////////////////////////////////////////////////////////////////
// CalculateDamageEffect
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateDamageEffect( clTree * p_oTree )
{
  int iDamage;
  if ( -1 == mp_iDamageCodes[p_oTree->GetSpecies()] [p_oTree->GetType()] )
  {
    return 1;
  }
  p_oTree->GetValue( mp_iDamageCodes[p_oTree->GetSpecies()] [p_oTree->GetType()], & iDamage );
  if ( 0 == iDamage ) return 1; //no damage
  else if ( 2000 <= iDamage ) //full damage
         return mp_fFullDamageStormEff[mp_iIndexes[p_oTree->GetSpecies()]];

  //medium damage
  return mp_fMedDamageStormEff[mp_iIndexes[p_oTree->GetSpecies()]];
}

////////////////////////////////////////////////////////////////////////////
// CalculateShadingEffect
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateShadingEffect( clTree * p_oTree )
{
  float fAmountShade;
  int iSpecies = p_oTree->GetSpecies();

  //Get the tree's shading
  p_oTree->GetValue( mp_iLightCodes[mp_iIndexes[iSpecies]] [p_oTree->GetType() - clTreePopulation::sapling], & fAmountShade );

  return exp( -mp_fShadingCoefficient[mp_iIndexes[iSpecies]]
       * pow( fAmountShade, mp_fShadingExponent[mp_iIndexes[iSpecies]] ) );
}

////////////////////////////////////////////////////////////////////////////
// CalculateShadingEffectNoExp
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateShadingEffectNoExp( clTree * p_oTree )
{

  float fAmountShade;
  int iSpecies = p_oTree->GetSpecies();

  //Get the tree's shading
  p_oTree->GetValue( mp_iLightCodes[mp_iIndexes[iSpecies]] [p_oTree->GetType() - clTreePopulation::sapling], & fAmountShade );

  return exp( -mp_fShadingCoefficient[mp_iIndexes[iSpecies]] * fAmountShade );

}

////////////////////////////////////////////////////////////////////////////
// CalculateCrowdingEffect
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateCrowdingEffect( const float & fDbh, const float & fNCI, const int & iSpecies )
{
  //Avoid a domain error - if NCI is 0, return 1
  if ( fNCI > 0 )
  {
    return exp( -mp_fCrowdingSlope[mp_iIndexes[iSpecies]] * pow( fDbh, mp_fGamma[mp_iIndexes[iSpecies]] )
         * pow( fNCI, mp_fCrowdingSteepness[mp_iIndexes[iSpecies]] ) );
  }
  return 1.0;
}

////////////////////////////////////////////////////////////////////////////
// CalculateCrowdingEffectNoExp
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateCrowdingEffectNoExp( const float & fDbh, const float & fNCI, const int & iSpecies )
{
  return exp( -mp_fCrowdingSlope[mp_iIndexes[iSpecies]] * pow( fDbh, mp_fGamma[mp_iIndexes[iSpecies]] ) * fNCI );
}

////////////////////////////////////////////////////////////////////////////
// CalculateCrowdingEffectNoDbh
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateCrowdingEffectNoDbh( const float & fDbh, const float & fNCI, const int & iSpecies )
{
  //Avoid a domain error - if NCI is 0, return 1
  if ( fNCI > 0 )
  {
    return exp( -mp_fCrowdingSlope[mp_iIndexes[iSpecies]] * pow( fNCI, mp_fCrowdingSteepness[mp_iIndexes[iSpecies]] ) );
  }
  return 1.0;
}

////////////////////////////////////////////////////////////////////////////
// CalculateCrowdingEffectSimple
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateCrowdingEffectSimple( const float & fDbh, const float & fNCI, const int & iSpecies )
{
  return exp( -mp_fCrowdingSlope[mp_iIndexes[iSpecies]] * fNCI );
}

////////////////////////////////////////////////////////////////////////////
// CalculateNCI
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateNCI( clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot )
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  char cQuery[75]; //format search strings into this
  float fNCI = 0, //nci - the end result of all this math
      fDistance, //distance between target and neighbor
      fDbh, //neighbor's dbh
      fDamageEffect, //neighbor's damage effect
      fNeighX, fNeighY, //holders for the neighbor tree's X and Y
      fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iDamage, iIsDead; //neighbor's damage value
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iTargetSpecies = p_oTree->GetSpecies(), //target tree's species
  iDeadCode; //neighbor's dead code

  //Format the query to get all competing neighbors
  p_oTree->GetValue( p_oPop->GetXCode( iTargetSpecies, p_oTree->GetType() ), & fTargetX );
  p_oTree->GetValue( p_oPop->GetYCode( iTargetSpecies, p_oTree->GetType() ), & fTargetY );

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", mp_fMaxCrowdingRadius[mp_iIndexes[iTargetSpecies]], "FROM x=", fTargetX,
      "y=", fTargetY, "::height=", m_fMinSaplingHeight );
  p_oAllNeighbors = p_oPop->Find( cQuery );

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor )
  {
    if ( p_oNeighbor != p_oTree )
    {

      iNeighSpecies = p_oNeighbor->GetSpecies();
      iNeighType = p_oNeighbor->GetType();

      if ( clTreePopulation::seedling != iNeighType && ((clTreePopulation::snag != iNeighType) || m_bIncludeSnags))
      {

        //Get the neighbor's dbh
        p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fDbh );

        if ( fDbh >= mp_fMinimumNeighborDBH[iNeighSpecies] )
        {

          //Make sure the neighbor's not dead
          iDeadCode = p_oPop->GetIntDataCode( "dead", iNeighSpecies, iNeighType );
          if ( -1 != iDeadCode )
          {
            p_oNeighbor->GetValue( iDeadCode, & iIsDead );
          }
          else
            iIsDead = notdead;

          if ( notdead == iIsDead || natural == iIsDead )
          {

            //Get the neighbor's X and Y values
            p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
            p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

            //Get the distance between the two trees
            fDistance = p_oPlot->GetDistance( fTargetX, fTargetY, fNeighX, fNeighY );

            //Get the neighbor's damage effect
            if ( -1 == mp_iDamageCodes[iNeighSpecies] [iNeighType] )
            {
              fDamageEffect = 1;
            }
            else
            {
              p_oNeighbor->GetValue( mp_iDamageCodes[iNeighSpecies] [iNeighType], & iDamage );
              if ( 0 == iDamage )
              {
                fDamageEffect = 1;
              }
              else
              {
                //What damage category is the neighbor in?
                if ( iDamage < 2000 )
                {
                  fDamageEffect = mp_fMedDamageEta[mp_iIndexes[iTargetSpecies]];
                }
                else
                {
                  fDamageEffect = mp_fFullDamageEta[mp_iIndexes[iTargetSpecies]];
                }
              }
            }

            //Only continue if distance is not 0 - it will be a fluke condition to
            //allow a tree that is literally standing on top of another one not to
            //affect it competitively, but there it is
            if ( 0 != fDistance )

              //Add competitive effect to NCI
              fNCI += fDamageEffect * mp_fLambda[mp_iIndexes[iTargetSpecies]] [iNeighSpecies]
                                                                               * ( pow( fDbh / m_fDbhDivisor, mp_fAlpha[mp_iIndexes[iTargetSpecies]] )
                                                                                   / pow( fDistance, mp_fBeta[mp_iIndexes[iTargetSpecies]] ) );

          }
        }
      }
    }
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  return fNCI;
}

////////////////////////////////////////////////////////////////////////////
// CalculateNCINoEta
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateNCINoEta( clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot )
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  char cQuery[75]; //format search strings into this
  float fNCI = 0, //nci - the end result of all this math
      fDistance, //distance between target and neighbor
      fDbh, //neighbor's dbh
      fNeighX, fNeighY, //holders for the neighbor tree's X and Y
      fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iIsDead; //whether a neighbor is dead
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iTargetSpecies, //target tree's species
  iDeadCode; //neighbor's dead code

  iTargetSpecies = p_oTree->GetSpecies();

  //Format the query to get all competing neighbors
  p_oTree->GetValue( p_oPop->GetXCode( iTargetSpecies, p_oTree->GetType() ), & fTargetX );
  p_oTree->GetValue( p_oPop->GetYCode( iTargetSpecies, p_oTree->GetType() ), & fTargetY );

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", mp_fMaxCrowdingRadius[mp_iIndexes[iTargetSpecies]], "FROM x=", fTargetX,
      "y=", fTargetY, "::height=", m_fMinSaplingHeight );
  p_oAllNeighbors = p_oPop->Find( cQuery );

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor )
  {
    if ( p_oNeighbor != p_oTree )
    {

      iNeighSpecies = p_oNeighbor->GetSpecies();
      iNeighType = p_oNeighbor->GetType();

      if ( clTreePopulation::seedling != iNeighType && ((clTreePopulation::snag != iNeighType) || m_bIncludeSnags))
      {

        //Get the neighbor's dbh
        p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fDbh );

        if ( fDbh >= mp_fMinimumNeighborDBH[iNeighSpecies] )
        {

          //Make sure the neighbor's not dead
          iDeadCode = p_oPop->GetIntDataCode( "dead", p_oNeighbor->GetSpecies(), p_oNeighbor->GetType() );
          if ( -1 != iDeadCode )
          {
            p_oNeighbor->GetValue( iDeadCode, & iIsDead );
          }
          else
            iIsDead = notdead;

          if ( notdead == iIsDead || natural == iIsDead )
          {

            //Get the neighbor's X and Y values
            p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
            p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

            //Get the distance between the two trees
            fDistance = p_oPlot->GetDistance( fTargetX, fTargetY, fNeighX, fNeighY );


            //Only continue if distance is not 0 - it will be a fluke condition to
            //allow a tree that is literally standing on top of another one not to
            //affect it competitively, but there it is
            if ( 0 != fDistance )

              //Add competitive effect to NCI
              fNCI += mp_fLambda[mp_iIndexes[iTargetSpecies]] [iNeighSpecies]
                                                               * ( pow( ( fDbh / m_fDbhDivisor ), mp_fAlpha[mp_iIndexes[iTargetSpecies]] )
                                                                   / pow( fDistance, mp_fBeta[mp_iIndexes[iTargetSpecies]] ) );

          }
        }
      }
    }
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  return fNCI;
}

////////////////////////////////////////////////////////////////////////////
// CalculateNCINoDivisor
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateNCINoDivisor( clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot )
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  char cQuery[75]; //format search strings into this
  float fNCI = 0, //nci - the end result of all this math
      fDistance, //distance between target and neighbor
      fDbh, //neighbor's dbh
      fDamageEffect, //neighbor's damage effect
      fNeighX, fNeighY, //holders for the neighbor tree's X and Y
      fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iDamage, //neighbor's damage value
  iIsDead; //whether a neighbor is dead
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iTargetSpecies = p_oTree->GetSpecies(), //target tree's species
  iDeadCode; //neighbor's dead code


  //Format the query to get all competing neighbors
  p_oTree->GetValue( p_oPop->GetXCode( iTargetSpecies, p_oTree->GetType() ), & fTargetX );
  p_oTree->GetValue( p_oPop->GetYCode( iTargetSpecies, p_oTree->GetType() ), & fTargetY );

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", mp_fMaxCrowdingRadius[mp_iIndexes[iTargetSpecies]], "FROM x=", fTargetX,
      "y=", fTargetY, "::height=", m_fMinSaplingHeight );
  p_oAllNeighbors = p_oPop->Find( cQuery );

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor )
  {
    if ( p_oNeighbor != p_oTree )
    {

      iNeighSpecies = p_oNeighbor->GetSpecies();
      iNeighType = p_oNeighbor->GetType();

      if ( clTreePopulation::seedling != iNeighType && ((clTreePopulation::snag != iNeighType) || m_bIncludeSnags))
      {

        //Get the neighbor's dbh
        p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fDbh );

        if ( fDbh >= mp_fMinimumNeighborDBH[iNeighSpecies] )
        {

          //Make sure the neighbor's not dead
          iDeadCode = p_oPop->GetIntDataCode( "dead", p_oNeighbor->GetSpecies(), p_oNeighbor->GetType() );
          if ( -1 != iDeadCode )
          {
            p_oNeighbor->GetValue( iDeadCode, & iIsDead );
          }
          else
            iIsDead = notdead;

          if ( notdead == iIsDead || natural == iIsDead )
          {

            //Get the neighbor's X and Y values
            p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
            p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

            //Get the distance between the two trees
            fDistance = p_oPlot->GetDistance( fTargetX, fTargetY, fNeighX, fNeighY );

            //Get the neighbor's damage effect
            if ( -1 == mp_iDamageCodes[iNeighSpecies] [iNeighType] )
            {
              fDamageEffect = 1;
            }
            else
            {
              p_oNeighbor->GetValue( mp_iDamageCodes[iNeighSpecies] [iNeighType], & iDamage );
              if ( 0 == iDamage )
              {
                fDamageEffect = 1;
              }
              else
              {
                //What damage category is the neighbor in?
                if ( iDamage < 2000 )
                {
                  fDamageEffect = mp_fMedDamageEta[mp_iIndexes[iTargetSpecies]];
                }
                else
                {
                  fDamageEffect = mp_fFullDamageEta[mp_iIndexes[iTargetSpecies]];
                }
              }
            }

            //Only continue if distance is not 0 - it will be a fluke condition to
            //allow a tree that is literally standing on top of another one not to
            //affect it competitively, but there it is
            if ( 0 != fDistance )

              //Add competitive effect to NCI
              fNCI += fDamageEffect * mp_fLambda[mp_iIndexes[iTargetSpecies]] [iNeighSpecies]
                                                                               * ( pow( fDbh, mp_fAlpha[mp_iIndexes[iTargetSpecies]] )
                                                                                   / pow( fDistance, mp_fBeta[mp_iIndexes[iTargetSpecies]] ) );

          }
        }
      }
    }
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  return fNCI;
}

////////////////////////////////////////////////////////////////////////////
// CalculateNCINoEtaNoDivisor
////////////////////////////////////////////////////////////////////////////
float clNCIBase::CalculateNCINoEtaNoDivisor( clTree * p_oTree, clTreePopulation * p_oPop, clPlot * p_oPlot )
{
  clTreeSearch * p_oAllNeighbors; //neighborhood trees within crowding radius
  clTree * p_oNeighbor; //competing neighbor
  char cQuery[75]; //format search strings into this
  float fNCI = 0, //nci - the end result of all this math
      fDistance, //distance between target and neighbor
      fDbh, //neighbor's dbh
      fNeighX, fNeighY, //holders for the neighbor tree's X and Y
      fTargetX, fTargetY; //holders for the target tree's X and Y location
  int iIsDead; //whether a neighbor is dead
  short int iNeighSpecies, iNeighType, //species and type for neighbor
  iTargetSpecies = p_oTree->GetSpecies(), //target tree's species
  iDeadCode; //neighbor's dead code

  //Format the query to get all competing neighbors
  p_oTree->GetValue( p_oPop->GetXCode( iTargetSpecies, p_oTree->GetType() ), & fTargetX );
  p_oTree->GetValue( p_oPop->GetYCode( iTargetSpecies, p_oTree->GetType() ), & fTargetY );

  //Get all trees taller than seedlings within the max crowding radius -
  //seedlings don't compete
  sprintf( cQuery, "%s%f%s%f%s%f%s%f", "distance=", mp_fMaxCrowdingRadius[mp_iIndexes[iTargetSpecies]], "FROM x=", fTargetX,
      "y=", fTargetY, "::height=", m_fMinSaplingHeight );
  p_oAllNeighbors = p_oPop->Find( cQuery );

  //Loop through and assess the competitive effects of each
  p_oNeighbor = p_oAllNeighbors->NextTree();

  while ( p_oNeighbor )
  {
    if ( p_oNeighbor != p_oTree )
    {

      iNeighSpecies = p_oNeighbor->GetSpecies();
      iNeighType = p_oNeighbor->GetType();

      if ( clTreePopulation::seedling != iNeighType && ((clTreePopulation::snag != iNeighType) || m_bIncludeSnags))
      {

        //Get the neighbor's dbh
        p_oNeighbor->GetValue( p_oPop->GetDbhCode( iNeighSpecies, iNeighType ), & fDbh );

        if ( fDbh >= mp_fMinimumNeighborDBH[iNeighSpecies] )
        {

          //Make sure the neighbor's not dead
          iDeadCode = p_oPop->GetIntDataCode( "dead", p_oNeighbor->GetSpecies(), p_oNeighbor->GetType() );
          if ( -1 != iDeadCode )
          {
            p_oNeighbor->GetValue( iDeadCode, & iIsDead );
          }
          else
            iIsDead = notdead;

          if ( notdead == iIsDead || natural == iIsDead )
          {

            //Get the neighbor's X and Y values
            p_oNeighbor->GetValue( p_oPop->GetXCode( iNeighSpecies, iNeighType ), & fNeighX );
            p_oNeighbor->GetValue( p_oPop->GetYCode( iNeighSpecies, iNeighType ), & fNeighY );

            //Get the distance between the two trees
            fDistance = p_oPlot->GetDistance( fTargetX, fTargetY, fNeighX, fNeighY );


            //Only continue if distance is not 0 - it will be a fluke condition to
            //allow a tree that is literally standing on top of another one not to
            //affect it competitively, but there it is
            if ( 0 != fDistance )

              //Add competitive effect to NCI
              fNCI += mp_fLambda[mp_iIndexes[iTargetSpecies]] [iNeighSpecies]
                                                               * ( pow( fDbh, mp_fAlpha[mp_iIndexes[iTargetSpecies]] )
                                                                   / pow( fDistance, mp_fBeta[mp_iIndexes[iTargetSpecies]] ) );

          }
        }
      }
    }
    p_oNeighbor = p_oAllNeighbors->NextTree();
  }

  return fNCI;
}

////////////////////////////////////////////////////////////////////////////
// SetFunctionPointers
////////////////////////////////////////////////////////////////////////////
void clNCIBase::SetFunctionPointers()
{

  int i;
  bool bUseDamage = false, bUseDivisor, bUseExp, bUseGamma;

  //********************************
  //Shading effect
  //********************************
  mp_ShadingEffect = new Ptr2ShadingEffect[m_iNumBehSpecies];
  //We don't need to go through the index array here, since the array
  //values should match; we don't care what the species actually is
  for ( i = 0; i < m_iNumBehSpecies; i++ )
  {
    //make sure we don't have any funny business with floating point
    //equality - we want to catch a value like -0.0 too
    if ( fabs( mp_fShadingCoefficient[i] ) < VERY_SMALL_VALUE )
    {
      //No shading
      mp_ShadingEffect[i] = & clNCIBase::CalculateNoShadingEffect;
    }
    else if ( fabs( mp_fShadingExponent[i] - 1 ) < VERY_SMALL_VALUE )
    { //is the value 1?
      //Shading with no exponent
      mp_ShadingEffect[i] = & clNCIBase::CalculateShadingEffectNoExp;
    }
    else
    {
      //Use the full function
      mp_ShadingEffect[i] = & clNCIBase::CalculateShadingEffect;
    }
  }

  //********************************
  //NCI
  //********************************
  //Determine if storm damage is used
  for ( i = 0; i < m_iNumBehSpecies; i++ )
  {
    if ( mp_fMedDamageStormEff[i] < 1 || mp_fFullDamageStormEff[i] < 1 ||
         mp_fMedDamageEta[i] < 1 || mp_fFullDamageEta[i] < 1)
    {
      bUseDamage = true;
      break;
    }
  }
 
  //Determine if there is a non-1 value for the DBH divisor
  bUseDivisor = ( fabs( m_fDbhDivisor - 1 ) > VERY_SMALL_VALUE );

  mp_NCI = new Ptr2CalculateNCI[m_iNumBehSpecies];
  for ( i = 0; i < m_iNumBehSpecies; i++ )
  {
    if ( bUseDamage && bUseDivisor )
    {
      //Use the full function
      mp_NCI[i] = & clNCIBase::CalculateNCI;
    }
    else if ( bUseDamage && !bUseDivisor )
    {
      //No divisor, but storm damage
      mp_NCI[i] = & clNCIBase::CalculateNCINoDivisor;
    }
    else if ( !bUseDamage && bUseDivisor )
    {
      //Yes damage, no divisor
      mp_NCI[i] = & clNCIBase::CalculateNCINoEta;
    }
    else
    {
      //No damage, no divisor
      mp_NCI[i] = & clNCIBase::CalculateNCINoEtaNoDivisor;
    }
  }

  //********************************
  //Damage Effect
  //********************************
  if ( bUseDamage )
  {
    DamageEffect = & clNCIBase::CalculateDamageEffect;
  }
  else
  {
    DamageEffect = & clNCIBase::CalculateNoDamageEffect;
  }

  //********************************
  //Crowding Effect
  //********************************
  mp_CrowdingEffect = new Ptr2CrowdingEffect[m_iNumBehSpecies];
  for ( i = 0; i < m_iNumBehSpecies; i++ )
  {
    bUseExp = ( fabs( mp_fCrowdingSteepness[i] - 1 ) > VERY_SMALL_VALUE );
    bUseGamma = ( fabs( mp_fGamma[i] ) > VERY_SMALL_VALUE );

    if (fabs(mp_fCrowdingSlope[i]) < VERY_SMALL_VALUE ||
        mp_fMaxCrowdingRadius[i] < VERY_SMALL_VALUE) {
      mp_CrowdingEffect[i] = & clNCIBase::CalculateNoCrowdingEffect;
    }
    else if ( !bUseExp && !bUseGamma )
    {
      mp_CrowdingEffect[i] = & clNCIBase::CalculateCrowdingEffectSimple;
    }
    else if ( !bUseExp && bUseGamma )
    {
      mp_CrowdingEffect[i] = & clNCIBase::CalculateCrowdingEffectNoExp;
    }
    else if ( bUseExp && !bUseGamma )
    {
      mp_CrowdingEffect[i] = & clNCIBase::CalculateCrowdingEffectNoDbh;
    }
    else
    {
      mp_CrowdingEffect[i] = & clNCIBase::CalculateCrowdingEffect;
    }
  }
}
