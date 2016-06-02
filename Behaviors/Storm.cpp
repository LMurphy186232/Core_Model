//---------------------------------------------------------------------------
// Storm.cpp
//---------------------------------------------------------------------------
#include "Storm.h"
#include "ParsingFunctions.h"
#include "SimManager.h"
#include "ModelMath.h"
#include "Grid.h"
#include <math.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////////
clStorm::clStorm( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ) {
  try {
    //Set the namestring
    m_sNameString = "Storm";
    m_sXMLRoot = "Storm";

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 3;
    m_fMinimumVersionNumber = 1;

    mp_fStormProbabilities = NULL;
    mp_oStormGrid = NULL;
    mp_oSusceptibilityMap = NULL;
    mp_stormsList = NULL;
    m_fStdDev = 0;
    RandomDraw = NULL;

    m_fTrendInterceptI = 0;
    m_iSusceptibility = uniform;
    m_iStormTimeCode = -1;
    m_i1DmgIndexCode = -1;
    m_fSineD = 0;
    m_fSineG = 0;
    m_fSineF = 0;
    m_iStochasticity = deterministic;
    m_fTrendSlopeM = 0;
    m_iSusceptIndexCode = -1;
    m_fSSTPeriod = 0;
    m_iDmgIndexCode = -1;
    m_iDistribution = normal;

    m_iNumScheduledStorms = 0;

    //Hard-coded values
    m_iNumSeverityClasses = 10;
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
    stcErr.sFunction = "clStorm::clStorm";
    throw( stcErr );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clStorm::~clStorm() {
  delete[] mp_fStormProbabilities;
  delete[] mp_stormsList;
}

//////////////////////////////////////////////////////////////////////////////
// GetStormProbability
//////////////////////////////////////////////////////////////////////////////
float clStorm::GetStormProbability(int iReturnInterval) {
  if (iReturnInterval < 0 || iReturnInterval >= m_iNumSeverityClasses) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_ARGUMENT;
    stcErr.sFunction = "clStorm::GetStormProbability";
    std::stringstream s;
    s << "Bad argument to GetStormProbability: " << iReturnInterval;
    stcErr.sMoreInfo = s.str();
    throw( stcErr );
  }
  return mp_fStormProbabilities[iReturnInterval];
}

//////////////////////////////////////////////////////////////////////////////
// SetStormProbability
//////////////////////////////////////////////////////////////////////////////
void clStorm::SetStormProbability(int iReturnInterval, float fValue) {
  if (iReturnInterval < 0 || iReturnInterval >= m_iNumSeverityClasses) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_ARGUMENT;
    stcErr.sFunction = "clStorm::SetStormProbability";
    std::stringstream s;
    s << "Bad return interval argument to SetStormProbability: " << iReturnInterval;
    stcErr.sMoreInfo = s.str();
    throw( stcErr );
  }
  if (fValue < 0 || fValue > 1) {
    modelErr stcErr;
    stcErr.iErrorCode = BAD_ARGUMENT;
    stcErr.sFunction = "clStorm::SetStormProbability";
    std::stringstream s;
    s << "Bad probability argument to SetStormProbability: " << fValue;
    stcErr.sMoreInfo = s.str();
    throw( stcErr );
  }
  mp_fStormProbabilities[iReturnInterval] = fValue;
}

/////////////////////////////////////////////////////////////////////////////
// GetData()
/////////////////////////////////////////////////////////////////////////////
void clStorm::GetData( xercesc::DOMDocument * p_oDoc ) {
  try {
    ReadParFile( p_oDoc );
    DoGridSetup();
    CalculateStormProbabilities();
    SetStochFuncPointer();
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
    stcErr.sFunction = "clStorm::GetData" ;
    throw( stcErr );
  }

}

/////////////////////////////////////////////////////////////////////////////
// ReadParFile()
/////////////////////////////////////////////////////////////////////////////

void clStorm::ReadParFile( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    DOMElement * p_oDocElement = GetParentParametersElement(p_oDoc),
               * p_oElement;
    DOMNodeList * p_oNodeList, //for searching for tags in the parameter file
       * p_oStormsList; //for getting the list of points
    DOMNode * p_oDocNode; //a search result
    XMLCh *sVal;
    std::stringstream sLabel;
    char *cData;
    double fTemp,
    //This has to be a float so we can do math properly
    fNumYrs = mp_oSimManager->GetNumberOfYearsPerTimestep();
    int iTemp;
    int i;

    //Declare the mp_fStormProbabilities array, where we'll put the return
    //intervals until they're transformed into probabilities
    mp_fStormProbabilities = new float[m_iNumSeverityClasses];

    //Get the parameter file values for the return intervals
    for ( i = 0; i < m_iNumSeverityClasses; i++ )
    {
      sLabel << "st_s" << i + 1 << "ReturnInterval";
      FillSingleValue( p_oDocElement, sLabel.str(), &fTemp, true );
      sLabel.str("");
      //Validate that all values are greater than or equal to 0
      if ( fTemp < 0 )
      {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::ReadParFile" ;
        stcErr.sMoreInfo = "Storm return intervals cannot be negative.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      mp_fStormProbabilities[i] = fTemp;
    }

    //Get the susceptibility
    FillSingleValue( p_oDocElement, "st_susceptibility", & iTemp, true );

    //Make sure the value is valid and assign
    if ( mapped == iTemp )
    {
      m_iSusceptibility = mapped;
    }
    else if (uniform == iTemp )
    {
      m_iSusceptibility = uniform;
    }
    else
    {
      //Unrecognized value - throw error
      modelErr stcErr;
      stcErr.sFunction = "clStorm::ReadParFile";
      std::stringstream s;
      s << "Unrecognized value for susceptibility: " << iTemp;
      stcErr.sMoreInfo = s.str();
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Get the stochasticity
    FillSingleValue( p_oDocElement, "st_stochasticity", & iTemp, true );

    if ( deterministic == iTemp ) {
      m_iStochasticity = deterministic;
    }
    else if ( stochastic == iTemp ) {
      m_iStochasticity = stochastic;
    }
    else {
      //Unrecognized value - throw error
      modelErr stcErr;
      stcErr.sFunction = "clStorm::ReadParFile";
      std::stringstream s;
      s << "Unrecognized value for stochasticity: " << iTemp;
      stcErr.sMoreInfo = s.str();
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //If the damage pattern is stochastic, get the probability distribution
    //function
    if ( stochastic == m_iStochasticity )
    {

      FillSingleValue( p_oDocElement, "st_probFunction", & iTemp, true );

      //Make sure the value is valid and assign
      if ( lognormal == iTemp )
      {
        m_iDistribution = lognormal;
      }
      else if ( normal == iTemp )
      {
        m_iDistribution = normal;
      }
      else
      {
        //Unrecognized value - throw error
        modelErr stcErr;
        stcErr.sFunction = "clStorm::ReadParFile";
        std::stringstream s;
        s << "Unrecognized value for damage pattern probability distribution function: " << iTemp;
        stcErr.sMoreInfo = s.str();
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //If the value was normal or lognormal, get the standard deviation;
      //if it is negative binomial, get the clumping parameter
      if ( normal == m_iDistribution || lognormal == m_iDistribution ) {
        FillSingleValue( p_oDocElement, "st_standardDeviation", & m_fStdDev, true );
      }
    }

    //Sine function d
    FillSingleValue( p_oDocElement, "st_stmSineD", &m_fSineD, true );

    //Sine function f
    FillSingleValue( p_oDocElement, "st_stmSineF", &m_fSineF, true );

    //Sine function g
    FillSingleValue( p_oDocElement, "st_stmSineG", &m_fSineG, true );

    //SST periodicity (Sr)
    FillSingleValue( p_oDocElement, "st_stmSSTPeriod", &m_fSSTPeriod, true );
    if (fabs(m_fSineD) > VERY_SMALL_VALUE && fabs(m_fSSTPeriod) < VERY_SMALL_VALUE) {
      modelErr stcErr;
      stcErr.sFunction = "clStorm::ReadParFile" ;
      stcErr.sMoreInfo = "SST periodicity cannot be zero.";
      stcErr.iErrorCode = BAD_DATA;
      throw( stcErr );
    }

    //Trend function slope (m)
    FillSingleValue( p_oDocElement, "st_stmTrendSlopeM", &m_fTrendSlopeM, true );

    //Trend function intercept (i)
    FillSingleValue( p_oDocElement, "st_stmTrendInterceptI", &m_fTrendInterceptI, true );

    //Scheduled storm events
    sVal = XMLString::transcode( "st_stmScheduledStorms" );
    p_oNodeList = p_oDoc->getElementsByTagName( sVal );
    XMLString::release(&sVal);

    if ( 0 == p_oNodeList->getLength() ) return;

    p_oDocNode = p_oNodeList->item( 0 );
    p_oElement = ( DOMElement * ) p_oDocNode;
    sVal = XMLString::transcode( "st_stmEvent" );
    p_oStormsList = p_oElement->getElementsByTagName( sVal );
    XMLString::release(&sVal);
    if (p_oStormsList->getLength() == 0) return;
    m_iNumScheduledStorms = p_oStormsList->getLength();

    mp_stormsList = new stcStorms[m_iNumScheduledStorms];
    for ( i = 0; i < m_iNumScheduledStorms; i++ )
    {
      p_oDocNode = p_oStormsList->item( i );
      p_oElement = ( DOMElement * ) p_oDocNode;
      sVal = XMLString::transcode( "min" );
      cData = XMLString::transcode( p_oElement->getAttributeNode( sVal )->getNodeValue() );
      mp_stormsList[i].fMin = atof( cData );
      delete[] cData; cData = NULL;
      XMLString::release(&sVal);
      sVal = XMLString::transcode( "max" );
      cData = XMLString::transcode( p_oElement->getAttributeNode( sVal )->getNodeValue() );
      mp_stormsList[i].fMax = atof( cData );
      delete[] cData; cData = NULL;
      XMLString::release(&sVal);
      sVal = XMLString::transcode( "yr" );
      cData = XMLString::transcode( p_oElement->getAttributeNode( sVal )->getNodeValue() );
      mp_stormsList[i].iTS = (int)ceil(atoi( cData ) / fNumYrs);
      delete[] cData; cData = NULL;
      XMLString::release(&sVal);
    } //end of for (j = 0; j < m_iNumScheduledStorms; j++)

    for ( i = 0; i < m_iNumScheduledStorms; i++ )
    {
      if (mp_stormsList[i].fMin > mp_stormsList[i].fMax) {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::ReadParFile" ;
        stcErr.sMoreInfo = "For scheduled storms, minimum cannot be greater than maximum.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      if (mp_stormsList[i].fMin > 1.0 || mp_stormsList[i].fMin < 0.0) {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::ReadParFile" ;
        stcErr.sMoreInfo = "For scheduled storms, minimum must be between 0 and 1.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      if (mp_stormsList[i].fMax > 1.0 || mp_stormsList[i].fMax < 0.0) {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::ReadParFile" ;
        stcErr.sMoreInfo = "For scheduled storms, maximum must be between 0 and 1.";
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
    stcErr.sFunction = "clStorm::ReadParFile" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// DoGridSetup()
/////////////////////////////////////////////////////////////////////////////

void clStorm::DoGridSetup()
{
  try
  {

    //If the susceptibility is "mapped", then we require the existence of the
    //"Storm Susceptibility" grid.  So get it and throw an error if it doesn't
    //exist.
    if ( mapped == m_iSusceptibility )
    {
      mp_oSusceptibilityMap = mp_oSimManager->GetGridObject( "Storm Susceptibility" );
      if ( NULL == mp_oSusceptibilityMap )
      {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::DoGridSetup" ;
        stcErr.sMoreInfo = "A grid map for the grid \"Storm Susceptibility\" is required for this file.";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }

      //Get the index for the "index" data member and throw an error if it
      //doesn't exist
      m_iSusceptIndexCode = mp_oSusceptibilityMap->GetFloatDataCode( "index" );
      if ( -1 == m_iSusceptIndexCode )
      {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::DoGridSetup" ;
        stcErr.sMoreInfo = "The grid map for the grid \"Storm Susceptibility\" is missing the data member\"index\".";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }

    //Check to see if the user put in grid data for "Storm Damage", in which
    //case it would already be created (ignore any actual values in this case)
    mp_oStormGrid = mp_oSimManager->GetGridObject( "Storm Damage" );
    if ( NULL != mp_oStormGrid )
    {

      //If there was also a susceptibility grid, make sure that the
      //number of grid cells are the same
      if ( NULL != mp_oSusceptibilityMap )
      {
        if ( mp_oSusceptibilityMap->GetNumberXCells() != mp_oStormGrid->GetNumberXCells()
             || mp_oSusceptibilityMap->GetNumberYCells() != mp_oStormGrid->GetNumberYCells() )
             {
               modelErr stcErr;
               stcErr.sFunction = "clStorm::DoGridSetup" ;
               stcErr.sMoreInfo = "The grid cell resolutions for \"Storm Susceptibility\" and \"Storm Damage\" must match.";
               stcErr.iErrorCode = BAD_DATA;
               throw( stcErr );
        }
      }

      m_i1DmgIndexCode = mp_oStormGrid->GetPackageFloatDataCode( "1dmg_index" );
      m_iDmgIndexCode = mp_oStormGrid->GetFloatDataCode( "dmg_index" );
      m_iStormTimeCode = mp_oStormGrid->GetFloatDataCode( "stormtime" );
      if ( -1 == m_i1DmgIndexCode)
      {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::DoGridSetup" ;
        stcErr.sMoreInfo = "The grid map for the grid \"Storm Damage\" is missing the data member\"1dmg_index\".";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      if ( -1 == m_iDmgIndexCode)
      {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::DoGridSetup" ;
        stcErr.sMoreInfo = "The grid map for the grid \"Storm Damage\" is missing the data member\"dmg_index\".";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
      if ( -1 == m_iStormTimeCode)
      {
        modelErr stcErr;
        stcErr.sFunction = "clStorm::DoGridSetup" ;
        stcErr.sMoreInfo = "The grid map for the grid \"Storm Damage\" is missing the data member\"stormtime\".";
        stcErr.iErrorCode = BAD_DATA;
        throw( stcErr );
      }
    }
    else
    {

      //The grid "Storm Damage" was not created, so create it now.  If the
      //grid "Storm Susceptibility" exists, use its grid resolution; otherwise
      //use the default
      float fXCellLength = 0, fYCellLength = 0;
      if ( NULL != mp_oSusceptibilityMap )
      {
        fXCellLength = mp_oSusceptibilityMap->GetLengthXCells();
        fYCellLength = mp_oSusceptibilityMap->GetLengthYCells();
      }
      mp_oStormGrid = mp_oSimManager->CreateGrid( "Storm Damage", //grid name
           0, //number integers
           2, //number floats
           0, //number chars
           0, //number bools
           fXCellLength, fYCellLength );
      mp_oStormGrid->ChangePackageDataStructure( 0, //number of package int data members
           1, //number of package float data members
           0, //number of package char data members
           0 ); //number of package bool data members

      m_i1DmgIndexCode = mp_oStormGrid->RegisterPackageFloat( "1dmg_index" );
      m_iDmgIndexCode = mp_oStormGrid->RegisterFloat( "dmg_index" );
      m_iStormTimeCode = mp_oStormGrid->RegisterFloat( "stormtime" );
    }

    //Initialize values in "Storm Damage"
    TimestepCleanup();

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
    stcErr.sFunction = "clStorm::DoGridSetup" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// PackageCleanup()
/////////////////////////////////////////////////////////////////////////////
void clStorm::PackageCleanup()
{
  clPackage *p_oPkg;
  int iX, iY, iNumXCells, iNumYCells;
  iNumXCells = mp_oStormGrid->GetNumberXCells();
  iNumYCells = mp_oStormGrid->GetNumberYCells();
  for ( iX = 0; iX < iNumXCells; iX++ )
  {
    for ( iY = 0; iY < iNumYCells; iY++ )
    {
      p_oPkg = mp_oStormGrid->GetFirstPackageOfCell(iX, iY);
      while (p_oPkg) {
        mp_oStormGrid->DeletePackage(p_oPkg);
        p_oPkg = mp_oStormGrid->GetFirstPackageOfCell(iX, iY);
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// CalculateStormProbabilities()
/////////////////////////////////////////////////////////////////////////////
void clStorm::CalculateStormProbabilities()
{
  int i;

  //Calculate each probability
  for ( i = 0; i < m_iNumSeverityClasses; i++ )
  {
    if (mp_fStormProbabilities[i] != 0) {
      mp_fStormProbabilities[i] = ( float )1 / mp_fStormProbabilities[i];
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// Action()
/////////////////////////////////////////////////////////////////////////////
void clStorm::Action()
{
  try
  {
    float fX, //for calculating effects of cyclicity
          fTrendBit, //trend effect on frequency
          fSineBit, //sinusoidal effect on frequency
          fCyclicity, //total cyclicity effect
          fMeanSeverity; //mean storm severity to apply
    int iNumberYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep(),
        iCurrentTimestep = mp_oSimManager->GetCurrentTimestep(),
        i, j, iTS = iCurrentTimestep;
    bool bStorms = false;  //whether or not we had a storm this timestep

    PackageCleanup();

    //Perform any scheduled storms
    if (m_iNumScheduledStorms > 0) {
      for (i = 0; i < m_iNumScheduledStorms; i++) {
        if (mp_stormsList[i].iTS == iTS) {
          //Get the storm's mean severity
          fMeanSeverity = ((mp_stormsList[i].fMax - mp_stormsList[i].fMin)
                             * clModelMath::GetRand()) + mp_stormsList[i].fMin;
          ApplyDamage( fMeanSeverity );
          bStorms = true;
        }
      }
    }

    //Calculate the effects of cyclicity
    if (fabs(m_fSineD) > VERY_SMALL_VALUE) {
      fX = 4.0 * ((iCurrentTimestep * iNumberYearsPerTimestep)) / m_fSSTPeriod;
      fSineBit = m_fSineD * sin( M_PI * (fX - m_fSineG)/(2 * m_fSineF));
      fTrendBit = m_fTrendSlopeM * fX + m_fTrendInterceptI;
      fCyclicity = fSineBit + fTrendBit;
    } else fCyclicity = 1;


    for ( i = 0; i < m_iNumSeverityClasses; i++ )
    {
      for ( j = 0; j < iNumberYearsPerTimestep; j++ )
      {
        if ( clModelMath::GetRand() <= mp_fStormProbabilities[i] * fCyclicity )
        {
          //Get the storm's mean severity
          fMeanSeverity = ( ( float )i / 10 ) + (clModelMath::GetRand()/10);
          ApplyDamage( fMeanSeverity );
          bStorms = true;
        }
      }
    }


    DoSeverityAverages();
    AdjustTimeSinceLastStormCounter(bStorms);

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
    stcErr.sFunction = "clStorm::Action" ;
    throw( stcErr );
  }
}

/////////////////////////////////////////////////////////////////////////////
// SetStochFuncPointer()
/////////////////////////////////////////////////////////////////////////////
void clStorm::SetStochFuncPointer()
{
  if ( stochastic != m_iStochasticity ) return;

  if ( lognormal == m_iDistribution )
  {
    RandomDraw = & clStorm::LognormalDraw;
  }
  else if ( normal == m_iDistribution )
  {
    RandomDraw = & clStorm::NormalDraw;
  }
}


/////////////////////////////////////////////////////////////////////////////
// ApplyDamage()
/////////////////////////////////////////////////////////////////////////////
void clStorm::ApplyDamage( float fMeanSeverity )
{
  clPackage *p_oPkg;
  float fDamage;
  int iX, iY, iNumXCells, iNumYCells;
  iNumXCells = mp_oStormGrid->GetNumberXCells();
  iNumYCells = mp_oStormGrid->GetNumberYCells();

  //Calculate the appropriate damage depending on the damage pattern
  if ( deterministic == m_iStochasticity )
  {
    if ( uniform == m_iSusceptibility )
    {

      //Deterministic uniform - each new package of each grid cell gets this
      //storm's damage index.
      if ( fMeanSeverity > 1 ) fMeanSeverity = 1;

      for ( iX = 0; iX < iNumXCells; iX++ )
      {
        for ( iY = 0; iY < iNumYCells; iY++ )
        {
          p_oPkg = mp_oStormGrid->CreatePackageOfCell(iX, iY);
          p_oPkg->SetValue(m_i1DmgIndexCode, fMeanSeverity);
        }
      }
    }
    else if ( mapped == m_iSusceptibility )
    {
      //Deterministic mapped
      float fSusceptibility;
      for ( iX = 0; iX < iNumXCells; iX++ )
      {
        for ( iY = 0; iY < iNumYCells; iY++ )
        {

          //Get the susceptibility index for this cell
          mp_oSusceptibilityMap->GetValueOfCell( iX, iY, m_iSusceptIndexCode, & fSusceptibility );

          //Get the new damage and add it
          fDamage = fSusceptibility * fMeanSeverity;
          if ( fDamage > 1 ) fDamage = 1;
          p_oPkg = mp_oStormGrid->CreatePackageOfCell(iX, iY);
          p_oPkg->SetValue(m_i1DmgIndexCode, fDamage);
        }
      }
    }
  }
  else if ( stochastic == m_iStochasticity )
  {

    if ( uniform == m_iSusceptibility )
    {
      //Stochastic uniform
      for ( iX = 0; iX < iNumXCells; iX++ )
      {
        for ( iY = 0; iY < iNumYCells; iY++ )
        {

          //Get the new storm's damage value
          fDamage = ( * this.*RandomDraw ) ( fMeanSeverity );
          if ( fDamage > 1 ) fDamage = 1;
          p_oPkg = mp_oStormGrid->CreatePackageOfCell(iX, iY);
          p_oPkg->SetValue(m_i1DmgIndexCode, fDamage);
        }
      }
    }
    else if ( mapped == m_iSusceptibility )
    {
      //Stochastic mapped
      float fSusceptibility;
      for ( iX = 0; iX < iNumXCells; iX++ )
      {
        for ( iY = 0; iY < iNumYCells; iY++ )
        {

          //Get the susceptibility index for this cell
          mp_oSusceptibilityMap->GetValueOfCell( iX, iY, m_iSusceptIndexCode, & fSusceptibility );

          //Get the new storm's damage value
          fDamage = ( * this.*RandomDraw ) ( fMeanSeverity );
          fDamage *= fSusceptibility;
          if ( fDamage > 1 ) fDamage = 1;
          p_oPkg = mp_oStormGrid->CreatePackageOfCell(iX, iY);
          p_oPkg->SetValue(m_i1DmgIndexCode, fDamage);
        }
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// AdjustTimeSinceLastStormCounter()
/////////////////////////////////////////////////////////////////////////////
void clStorm::AdjustTimeSinceLastStormCounter(bool bStormThisTimestep)
{
  float fCounter,
        fNumYearsPerTimestep = mp_oSimManager->GetNumberOfYearsPerTimestep();
  int iX, iY, iNumXCells = mp_oStormGrid->GetNumberXCells(),
      iNumYCells = mp_oStormGrid->GetNumberYCells();

  if (bStormThisTimestep) {

    fCounter = 0;
  }
  else {
  	mp_oStormGrid->GetValueOfCell( 0, 0, m_iStormTimeCode, & fCounter );
  	fCounter += fNumYearsPerTimestep;
  }

  for ( iX = 0; iX < iNumXCells; iX++ )
  {
    for ( iY = 0; iY < iNumYCells; iY++ )
    {
      mp_oStormGrid->SetValueOfCell( iX, iY, m_iStormTimeCode, fCounter );
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// DoSeverityAverages()
/////////////////////////////////////////////////////////////////////////////
void clStorm::DoSeverityAverages()
{
  clPackage *p_oPkg;
  float fAvg, fVal;
  int iX, iY, iNumXCells = mp_oStormGrid->GetNumberXCells(),
      iNumYCells = mp_oStormGrid->GetNumberYCells(),
      iPkgCount;

  for ( iX = 0; iX < iNumXCells; iX++ )
  {
    for ( iY = 0; iY < iNumYCells; iY++ )
    {
      fAvg = 0;
      iPkgCount = 0;
      p_oPkg = mp_oStormGrid->GetFirstPackageOfCell(iX, iY);
      while (p_oPkg) {
        p_oPkg->GetValue(m_i1DmgIndexCode, &fVal);
        fAvg += fVal;
        iPkgCount++;
        p_oPkg = p_oPkg->GetNextPackage();
      }
      if (fAvg > 0) fAvg /= iPkgCount;
      mp_oStormGrid->SetValueOfCell( iX, iY, m_iDmgIndexCode, fAvg );
    }
  }
}
