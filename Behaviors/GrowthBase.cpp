//---------------------------------------------------------------------------
#include "GrowthBase.h"
#include "GrowthOrg.h"
#include "SimManager.h"
#include "TreePopulation.h"

clGrowthOrg *clGrowthBase::mp_oGrowthOrg = NULL;
//////////////////////////////////////////////////////////////////////////////
// Constructor
// Edit history:
// -------------
// April 28, 2004 - Submitted in beta version (LEM)
//////////////////////////////////////////////////////////////////////////////
clGrowthBase::clGrowthBase(clSimManager *p_oSimManager) :
     clWorkerBase( p_oSimManager ), clBehaviorBase( p_oSimManager ) {
  try {
    //Set the hooked flag to false
    m_bHooked = false;

    //Default the "go last" flag to false
    m_bGoLast = false;

    //Now - see if the growth org object is NULL.  If it is, create it and pass
    //a self pointer so this object will be hooked
    if (mp_oGrowthOrg == NULL)
      mp_oGrowthOrg = new clGrowthOrg(this);

    //Allowed file types
    m_iNumAllowedTypes = 2;
    mp_iAllowedFileTypes = new int[m_iNumAllowedTypes];
    mp_iAllowedFileTypes[0] = parfile;
    mp_iAllowedFileTypes[1] = detailed_output;

    //We know that we'll need a new float data member for all growth objects -
    //so set the new tree floats variable
    m_iNewTreeFloats = 1;

    //Versions
    m_fVersionNumber = 1;
    m_fMinimumVersionNumber = 1;

    m_fConvertCmPerTSToMmPerYr = 0;
    m_fConvertMmPerYearToCmPerTS = 0;
    m_iGrowthMethod = diameter_auto;
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrowthBase::clGrowthBase";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////////////
clGrowthBase::~clGrowthBase() {
  if (m_bHooked) {
    delete mp_oGrowthOrg;
    mp_oGrowthOrg = NULL;
  }
}


//////////////////////////////////////////////////////////////////////////////
// GetData()
//////////////////////////////////////////////////////////////////////////////
void clGrowthBase::GetData(DOMDocument *p_oDoc) {
 try {
   //If this is hooked, call the growth org's DoSetup function
   if (m_bHooked)
     mp_oGrowthOrg->DoSetup(mp_oSimManager, p_oDoc);

   //Set up the conversion factor that will take us back from cm of diameter
   //growth per timestep to mm of radial growth per year; we need this to set
   //the lgm value for growth-based mortality's use
   //multiply by 10 to get from cm to mm, divide by 2 to get from diameter to
   //radius, divide by number of years per timestep
   m_fConvertCmPerTSToMmPerYr = 5.0 / mp_oSimManager->GetNumberOfYearsPerTimestep();
   m_fConvertMmPerYearToCmPerTS = 1.0/m_fConvertCmPerTSToMmPerYr;

   DoShellSetup(p_oDoc);
 }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrowthBase::GetData";
    throw(stcErr);
  }
}


//////////////////////////////////////////////////////////////////////////////
// Action()
//////////////////////////////////////////////////////////////////////////////
void clGrowthBase::Action() {
  try {

    //If this is a hooked object, call the growth org object.  Otherwise, do
    //nothing.
    if (m_bHooked)
      mp_oGrowthOrg->DoGrowthAssignments();
  }
  catch (modelErr&err) {throw(err);}
  catch (modelMsg &msg) {throw(msg);} //non-fatal error
  catch (...) {
    modelErr stcErr;
    stcErr.iErrorCode = UNKNOWN;
    stcErr.sFunction = "clGrowthBase::Action";
    throw(stcErr);
  }
}


///////////////////////////////////////////////////////////////////////////////
// RegisterTreeDataMembers()
///////////////////////////////////////////////////////////////////////////////
void clGrowthBase::RegisterTreeDataMembers()
{
  //If this is a hooked object, call the growth org object.  Otherwise, do
  //nothing.
  if ( m_bHooked )
  {
    clPopulationBase * p_oTemp = mp_oSimManager->GetPopulationObject( "treepopulation" );
    clTreePopulation * p_oPop = ( clTreePopulation * ) p_oTemp;
    mp_oGrowthOrg->DoTreeDataMemberRegistrations( mp_oSimManager, p_oPop );
  }
}
