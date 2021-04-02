//---------------------------------------------------------------------------
#include "LightBase.h"
#include "LightOrg.h"
#include "Plot.h"
#include "SimManager.h"
#include "TreePopulation.h"
#include "Allometry.h"
#include <fstream>

clLightOrg *clLightBase::mp_oLightOrg = NULL;
//---------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
clLightBase::clLightBase( clSimManager * p_oSimManager ) : clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager )
{
  try
  {
    //Set the hooked flag to false
    m_bHooked = false;

    //Default the "needs common parameters" flag to true
    m_bNeedsCommonParameters = true;

    //Now - see if the light org object is NULL.  If it is, create it and pass
    //a self pointer so this object will be hooked
    if ( mp_oLightOrg == NULL )
      mp_oLightOrg = new clLightOrg( this );

    //We know that we'll need a new float data member for all light objects -
    //so set the new tree floats variable
    m_iNewTreeFloats = 1;

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    m_iNumAltAng = 0;
    m_iNumAziAng = 0;
    m_fMinSunAngle = 0;
    m_iMinAngRow = 0;
    m_fAzimuthOfNorth = 0;

    mp_fBrightness = NULL;
    mp_fPhoto = NULL;
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
    stcErr.sFunction = "clLightBase::clLightBase" ;
    throw( stcErr );
  }
}



///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
clLightBase::~clLightBase()
{
  if ( m_bHooked )
  {
    delete mp_oLightOrg;
    mp_oLightOrg = NULL;
  }

  //Delete the brightness and photo arrays
  if ( mp_fBrightness )
  {
    for ( int i = 0; i < m_iNumAltAng; i++ )
    {
      delete[] mp_fBrightness[i];
    }
  }
  if ( mp_fPhoto )
  {
    for ( int i = 0; i < m_iNumAltAng; i++ )
    {
      delete[] mp_fPhoto[i];
    }
  }
  delete[] mp_fBrightness;
  delete[] mp_fPhoto;
}



///////////////////////////////////////////////////////////////////////////////
// GetData()
///////////////////////////////////////////////////////////////////////////////
void clLightBase::GetData( xercesc::DOMDocument * p_oDoc )
{
  try
  {
    //If this is hooked, call the light org's DoSetup function
    if ( m_bHooked )
      mp_oLightOrg->DoSetup( mp_oSimManager, p_oDoc );

    DoShellSetup( p_oDoc );
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
    stcErr.sFunction = "clLightBase::GetData" ;
    throw( stcErr );
  }
}



///////////////////////////////////////////////////////////////////////////////
// Action()
///////////////////////////////////////////////////////////////////////////////
void clLightBase::Action()
{
  try
  {

    //If this is a hooked object, call the light org object.  Otherwise, do
    //nothing.
    if ( m_bHooked )
      mp_oLightOrg->DoLightAssignments();
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
    stcErr.sFunction = "clLightBase::Action" ;
    throw( stcErr );
  }
}



///////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
///////////////////////////////////////////////////////////////////////////////
void clLightBase::RegisterTreeDataMembers()
{
  try
  {

    //If this is a hooked object, call the light org object.  Otherwise, do
    //nothing.
    if ( m_bHooked )
    {
      clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
      clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;
      mp_oLightOrg->DoTreeDataMemberRegistrations( mp_oSimManager, p_oPop );
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
    stcErr.sFunction = "clLightBase::RegisterTreeDataMembers" ;
    throw( stcErr );
  }
}



///////////////////////////////////////////////////////////////////////////////
// PopulateGLIBrightnessArray()
///////////////////////////////////////////////////////////////////////////////
void clLightBase::PopulateGLIBrightnessArray()
{
  try
  {
    clPlot * p_oPlot; //use to query plot dimensions
    int iEWDivideRow, //matrix row dividing the sky into eastern and western halves
         iJulDay, iDayCount, //julian day - loop counter for going through the growing season
         iAltCoord, //place in alt row in brightness array of sun's position in the sky
         iAziCoord, //place in azi row in brightness array of sun's position in the sky
         iWestCol, //column in western half of the sky  - used when copying the
         //eastern half as mirror image
         iFirstJulDay, iLastJulDay, //first and last days of growing season
         i, j; //loop counters
    float fDayAngle, //day angle - used in calculations for sun's position
         fDeclination, //solar declination - used in calcs for sun's position
         fEccentricity, //earth orbital eccentricity - used in sun position calcs
         fSunrise, //sunrise, in solar time (time is in angular degrees where
         //24hrs = 2PI)
         f5Min, //5 min of solar time (expressed as an angle)
         fTimeNow, //counter used to loop through morning in solar time
         fLatInRadians, //plot latitude in radians
         fBeam, //strength of beam radiation coming from a particular direction
         fAltInDeg, //solar altitude angle in degrees
         fCosZenAng, //cosine of solar zenith angle
         fAltInRad, //sun's altitude angle (angle from the horizon)
         fAzimuth, //sun's azimuth angle, in radians
         fAirmass, //the effects of the airmass on attenuating solar radiation
         fClearSkyTransCoef, //clear sky transmission coefficient
         fBeamFracGlobalRad, //beam fraction of global radiation
         fTotalBeam; //the sum of all beam radiation

    //Get the plot object to query
    p_oPlot = mp_oSimManager->GetPlotObject();

    fTotalBeam = 0.0;
    i = 0;
    fLatInRadians = p_oPlot->GetLatitude() * CONVERT_TO_RADIANS;
    fClearSkyTransCoef = mp_oLightOrg->GetClearSkyTransmissionCoefficient();
    iFirstJulDay = mp_oLightOrg->GetFirstDayOfGrowingSeason();
    iLastJulDay = mp_oLightOrg->GetLastDayOfGrowingSeason();
    fBeamFracGlobalRad = mp_oLightOrg->GetBeamFractionGlobalRadiation();

    f5Min = 0.021816615; //5 min in radians

    // initialize brightness array to all dark
    for ( i = 0; i < m_iNumAltAng; i++ )
      for ( j = 0; j < m_iNumAziAng; j++ )
        mp_fBrightness[i] [j] = 0.0;

    // In the southern hemisphere, the last day of the growing season may
    // be before the first day. Adjust accordingly
    if (iLastJulDay < iFirstJulDay) iLastJulDay += 365;

    //*********************************************
    // BEAM RADIATION CALCULATION For the eastern hemisphere, calculate the
    // sun's position every 5 minutes for every day in the growing season and
    // add its brightness to the appropriate hemisphere grid in the brightness
    // array
    //*********************************************
    for ( iDayCount = iFirstJulDay; iDayCount < iLastJulDay; iDayCount++ )
    {
      // In case we had to wrap for the southern hemisphere, correct
      if (iDayCount > 365) iJulDay = iDayCount - 365;
      else iJulDay = iDayCount;

      //Solar geometery parameters
      //compute solar declination in radians
      fDayAngle = GetDayAngle( iJulDay );
      fDeclination = GetDeclination( fDayAngle );
      //compute earth orbital eccentricity in radians
      fEccentricity = GetEccentricity( fDayAngle );

      //Compute sunrise so we can loop through a day
      fSunrise = GetSunrise( fLatInRadians, fDeclination );
      fTimeNow = fSunrise;

      //*****************************************
      //Complete a single day's calculation - morning only - we'll copy the
      //eastern half of the sky into the western half because they're
      //symmetrical
      //*****************************************
      while ( fTimeNow >= 0.0 ) //while still morning and not yet noon (noon is 0 in solar time)
      {

        //Find the position of the sun at the current time as the angle from
        //the zenith
        fCosZenAng = GetCosineOfZenithAngle( fDeclination, fLatInRadians, fTimeNow );
        fAltInRad = GetAltitudeAngle( fCosZenAng );

        fAltInDeg = fAltInRad * CONVERT_TO_DEGREES; //convert to degrees

        //Translate the altitude angle to a grid row in the sky brightness array
        // - remember that the rows are equal divisions of sin(alt)
        iAltCoord = ( int )( sin( fAltInRad ) * m_iNumAltAng );
        if ( iAltCoord >= m_iNumAltAng ) iAltCoord = m_iNumAltAng - 1;

        //Compute azimuth angle of the sun in radians
        fAzimuth = GetAzimuthAngle( fDeclination, fLatInRadians, fAltInRad, fTimeNow );

        //Translate azimuth angle into row number in brightness array
        //Note that this calculation is equivalent to dividing the azimuth angle
        //by the size of the azimuth divisions - but this way saves one division
        //step
        iAziCoord = ( int )( fAzimuth * m_iNumAziAng / ( 2.0 * M_PI ) );
        if ( iAziCoord >= m_iNumAziAng ) iAziCoord = m_iNumAziAng - 1;

        //Now that we have the sun's position, calculate the airmass effect
        fAirmass = GetAirmassEffect( fAltInDeg, fCosZenAng );

        //If the sun's position is above the minimum solar angle cutoff, add the
        //radiation from it to the brightness array at that position
        if ( iAltCoord >= m_iMinAngRow )
        { //Compute beam transmission as function of path length and eccentricity
          fBeam = GetBeamRadiation( fClearSkyTransCoef, fAirmass, fEccentricity, fCosZenAng );
          fTotalBeam += fBeam;
          mp_fBrightness[iAltCoord] [iAziCoord] += fBeam;
        }

        //subtract 5 min - in solar time noon is zero and morning is positive
        fTimeNow -= f5Min;
      } //end of while (fTimeNow >= 0.0)
    } //end of the growing season's radiation calculation

    //Copy eastern hemisphere calculations into western hemisphere
    //as mirror image
    iEWDivideRow = ( int )( m_iNumAziAng / 2 );
    for ( iAltCoord = 0; iAltCoord < m_iNumAltAng; iAltCoord++ )
      for ( iAziCoord = 0; iAziCoord < iEWDivideRow; iAziCoord++ )
      {
        iWestCol = ( m_iNumAziAng - 1 ) - iAziCoord;
        mp_fBrightness[iAltCoord] [iWestCol] = mp_fBrightness[iAltCoord] [iAziCoord];
      }

    //Relativize grid cell values to percent of full sky - we only have beam
    //radiation so far so multiply by the amount of total radiation that's beam
    fTotalBeam = 1.0 / (2.0 * fTotalBeam); //remember fTotalBeam is only 0.5 of sky
    fBeam = fBeamFracGlobalRad * fTotalBeam;
    for ( i = 0; i < m_iNumAltAng; i++ )
      for ( j = 0; j < m_iNumAziAng; j++ )
        mp_fBrightness[i] [j] *= fBeam;

    //compute the diffuse radiation and add to direct beam component
    //then fill the final brightness array, will = 0 below minimum angle
    for ( i = 0; i < m_iNumAltAng; i++ )
      for ( j = 0; j < m_iNumAziAng; j++ )
      {
        if ( i >= m_iMinAngRow )
        {
          mp_fBrightness[i] [j] += ( 1.0 - fBeamFracGlobalRad ) / ( m_iNumAziAng * ( m_iNumAltAng - m_iMinAngRow ) );
        }
        else
          mp_fBrightness[i] [j] = 0.0;
      }

    //*****************************************
    //Rotate brightness array if azimuth > 0
    //*****************************************
    if (m_fAzimuthOfNorth > 0) {
      //Copy the brightness array
      float **p_fBrightCopy = new float * [m_iNumAltAng];
      for ( i = 0; i < m_iNumAltAng; i++) {
        p_fBrightCopy[i] = new float[m_iNumAziAng];
        for ( j = 0; j < m_iNumAziAng; j++ ) {
          p_fBrightCopy[i] [j] = mp_fBrightness[i] [j];
        }
      }

      //Calculate size of azimuth chunks, in radians
      float fChunkSize = ( 2.0 * M_PI ) / m_iNumAziAng;

      //Calculate how many columns of offset are required for the azimuth
      int iOffset = round(m_fAzimuthOfNorth / fChunkSize);
      int iNewCol;

      //Offset by that many columns
      for ( j = 0; j < m_iNumAziAng; j++) {
        iNewCol = j + iOffset;
        if (iNewCol >= m_iNumAziAng) iNewCol -= m_iNumAziAng;
        for ( i = 0; i < m_iNumAltAng; i++) {
          mp_fBrightness[i] [j] = p_fBrightCopy[i] [iNewCol];
        }
      }

      // Delete the copy of the brightness arrayj
      for ( int i = 0; i < m_iNumAltAng; i++ ) {
        delete[] p_fBrightCopy[i];
      }

      delete[] p_fBrightCopy;
    }

    //Write brightness array
   /* using namespace std;
        fstream brightness("GLI Brightness array.txt", ios::trunc | ios::out); brightness << "Segment";
    for (i = 0; i < m_iNumAziAng; i++) brightness << "\t" << i; for (i = 0; i < m_iNumAltAng; i++) {
    brightness << "\n" << i; for (j = 0; j < m_iNumAziAng; j++) brightness << "\t" << mp_fBrightness[i][j];
    } brightness.close();*/
  } //end of try block
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
    stcErr.sFunction = "clLightBase::PopulateBrightnessArray" ;
    stcErr.iErrorCode = UNKNOWN;
    throw( stcErr );
  }
}



///////////////////////////////////////////////////////////////////////////////
// PopulateSailLightBrightnessArray()
///////////////////////////////////////////////////////////////////////////////
void clLightBase::PopulateSailLightBrightnessArray()
{
  try
  {
    clPlot * p_oPlot; //use to query plot dimensions
    int iEWDivideRow, //row dividing the sky into eastern and western halves
         iJulDay, iDayCount,//julian day - loop counter for going through the growing season
         iAltCoord, //place in alt row in brightness array of sun's position in the sky
         iAziCoord, //place in azi row in brightness array of sun's position in the sky
         iWestCol, //column in western half of the sky  - used when copying the
         //eastern half as mirror image
         iFirstJulDay, iLastJulDay, //first and last days of growing season
         i, j; //loop counters
    float * p_fDiffuseRad, //for computing diffuse radiation
         fDayAngle, //day angle - used in calculations for sun's position
         fDeclination, //solar declination - used in calcs for sun's position
         fEccentricity, //earth orbital eccentricity - used in sun position calcs
         fSunrise, //sunrise, in solar time (time is in angular degrees where
         //24hrs = 2PI)
         f1Min, //1 minute of solar time (expressed as an angle)
         fTimeNow, //counter used to loop through morning in solar time
         fLatInRadians, //plot latitude in radians
         fBeam, //strength of beam radiation coming from a particular direction
         fCosZenAng, //cosine of solar zenith angle
         fAltInRad, //sun's altitude angle (angle from the horizon)
         fAltInDeg, //solar altitude angle in degrees
         fAzimuth, //sun's azimuth angle, in radians
         fAirmass, //the effects of the airmass on attenuating solar radiation
         fClearSkyTransCoef, //clear sky transmission coefficient
         fBeamFracGlobalRad, //beam fraction of global radiation
         fTotalBeam; //the sum of all beam radiation

    //Get the plot object to query
    p_oPlot = mp_oSimManager->GetPlotObject();

    p_fDiffuseRad = new float[m_iNumAltAng];
    fTotalBeam = 0.0;
    i = 0;
    fLatInRadians = p_oPlot->GetLatitude() * CONVERT_TO_RADIANS;

    fClearSkyTransCoef = mp_oLightOrg->GetClearSkyTransmissionCoefficient();
    iFirstJulDay = mp_oLightOrg->GetFirstDayOfGrowingSeason();
    iLastJulDay = mp_oLightOrg->GetLastDayOfGrowingSeason();
    fBeamFracGlobalRad = mp_oLightOrg->GetBeamFractionGlobalRadiation();

    f1Min = 0.004363323; //1 min in radians

    // initialize brightness array to all dark
    for ( i = 0; i < m_iNumAltAng; i++ )
      for ( j = 0; j < m_iNumAziAng; j++ )
        mp_fBrightness[i] [j] = 0.0;

    //*******************************************
    // BEAM RADIATION CALCULATION
    // For the eastern hemisphere, calculate the sun's position every minute
    // for every day in the growing season and add its brightness to the
    // appropriate hemisphere grid in the brightness array
    //*******************************************
    for ( iDayCount = iFirstJulDay; iDayCount < iLastJulDay; iDayCount++ )
    {
      // In case we had to wrap for the southern hemisphere, correct
      if (iDayCount > 365) iJulDay = iDayCount - 365;
      else iJulDay = iDayCount;
      //Solar geometery parameters
      //compute solar declination in radians
      fDayAngle = GetDayAngle( iJulDay );
      fDeclination = GetDeclination( fDayAngle );
      //compute earth orbital eccentricity in radians
      fEccentricity = GetEccentricity( fDayAngle );

      //Compute sunrise so we can loop through a day
      fSunrise = GetSunrise( fLatInRadians, fDeclination );
      fTimeNow = fSunrise;

      //*****************************************
      //Complete a single day's calculation - morning only - we'll copy the
      //eastern half of the sky into the western half because they're
      //symmetrical
      //*****************************************
      while ( fTimeNow >= 0.0 ) //while still morning and not yet noon (noon is 0
      { //in solar time

        //Find the position of the sun at the current time as the angle from
        //the zenith
        fCosZenAng = GetCosineOfZenithAngle( fDeclination, fLatInRadians, fTimeNow );
        fAltInRad = GetAltitudeAngle( fCosZenAng );

        fAltInDeg = fAltInRad * CONVERT_TO_DEGREES;
        iAltCoord = (int)floor( fAltInDeg ) - 1; //convert to degrees
        if ( iAltCoord > m_iNumAltAng ) iAltCoord = m_iNumAltAng - 1;
        if ( iAltCoord < 0 ) iAltCoord = 0;

        //Compute azimuth angle of the sun in radians
        fAzimuth = GetAzimuthAngle( fDeclination, fLatInRadians, fAltInRad, fTimeNow );

        //Translate azimuth angle into row number in brightness array
        iAziCoord = (int)floor( fAzimuth * CONVERT_TO_DEGREES ) - 1;

        //Now that we have the sun's position, calculate the airmass effect
        fAirmass = GetAirmassEffect( fAltInDeg, fCosZenAng );

        //Add the radiation to the brightness array at that position
        //Compute beam transmission as function of path length and eccentricity
        fBeam = GetBeamRadiation( fClearSkyTransCoef, fAirmass, fEccentricity, fCosZenAng );
        fTotalBeam += fBeam;
        mp_fBrightness[iAltCoord] [iAziCoord] += fBeam;

        //subtract 1 min - in solar time noon is zero and morning is positive
        fTimeNow -= f1Min;
      } //end of while (fTimeNow >= 0.0)
    } //end of the growing season's radiation calculation

    //Copy eastern hemisphere calculations into western hemisphere
    //as mirror image
    iEWDivideRow = ( int )( m_iNumAziAng / 2 ) + 1;
    for ( iAltCoord = 0; iAltCoord < m_iNumAltAng; iAltCoord++ )
      for ( iAziCoord = 0; iAziCoord < iEWDivideRow; iAziCoord++ )
      {
        iWestCol = ( m_iNumAziAng - 1 ) - iAziCoord;
        mp_fBrightness[iAltCoord] [iWestCol] = mp_fBrightness[iAltCoord] [iAziCoord];
      }

    //Relativize grid cell values to percent of total beam radiation
    fTotalBeam = 1 / ( fTotalBeam * 2.0 ); //remember fTotalBeam is only 0.5 of sky
    for ( i = 0; i < m_iNumAltAng; i++ )
      for ( j = 0; j < m_iNumAziAng; j++ )
        mp_fBrightness[i] [j] *= fTotalBeam;


    //*******************************************
    // DIFFUSE RADIATION CALCULATION
    // Diffuse radiation is isotropic - so make it equal to the area of each of
    // the sky segments.  Do one row of altitude and then use for all the other
    // rows
    //*******************************************
    fTotalBeam = 0;
    for ( i = 1; i <= m_iNumAltAng; i++ )
    {

      p_fDiffuseRad[i - 1] = sin( ( i - 0.5 ) * CONVERT_TO_RADIANS )
           * ( ( sin( CONVERT_TO_RADIANS * i ) - sin( CONVERT_TO_RADIANS * ( i - 1 ) ) ) / 360 );
      fTotalBeam += p_fDiffuseRad[i - 1];
    }

    //Multiply the amount of beam radiation by the number of azimuth angles so
    //it is the amount for the whole sky
    fTotalBeam *= ( m_iNumAziAng - 1 ); //remove the -1?
    fTotalBeam = 1 / fTotalBeam; //do division step now to save it later

    //Relativize our diffuse rad. array to percent of total sky diffuse radiation
    for ( i = 0; i < m_iNumAltAng; i++ )
    {
      p_fDiffuseRad[i] = p_fDiffuseRad[i] * fTotalBeam;
    }

    //Add together beam and diffuse radiation.  Weight them according to the
    //amount of total radiation that is beam.  The sky is dark below the
    //minimum sail mask altitude angle.
    for ( i = 0; i < m_iNumAltAng; i++ )
    {

      if ( i >= m_iMinAngRow )
      {
        //Above the sail mask altitude angle
        for ( j = 0; j < m_iNumAziAng; j++ )
          mp_fBrightness[i] [j] = fBeamFracGlobalRad * mp_fBrightness[i] [j] + ( 1.0 - fBeamFracGlobalRad ) * p_fDiffuseRad[i];

      }
      else
      {

        for ( j = 0; j < m_iNumAziAng; j++ )
          mp_fBrightness[i] [j] = 0.0;

      }
    } //end of for (i = 0; i < m_iNumAltAng; i++)

    delete[] p_fDiffuseRad;

    //Write brightness array
    /*   fstream brightness("Sail Brightness array.xls", ios::trunc | ios::out); brightness << "Segment";
    for (i = 0; i < m_iNumAltAng; i++) brightness << "\t" << i; for (i = 0; i < m_iNumAziAng; i++) {
    brightness << "\n" << i; for (j = 0; j < m_iNumAltAng; j++) brightness << "\t" << mp_fBrightness[j][i];
    } brightness.close(); */
  } //end of try block
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
    stcErr.sFunction = "clLightBase::PopulateBrightnessArray" ;
    stcErr.iErrorCode = UNKNOWN;
    throw( stcErr );
  }
}


///////////////////////////////////////////////////////////////////////////////
// PopulateSailLightBrightnessArray()
///////////////////////////////////////////////////////////////////////////////
float clLightBase::GetLightExtinctionCoefficient(clTree *p_oTree)   {
   return mp_oLightOrg->GetLightExtCoeff(p_oTree);
}
