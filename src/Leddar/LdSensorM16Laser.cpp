////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorM16Laser.cpp
///
/// \brief  Implements the LdSensorM16Laser class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdSensorM16Laser.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#include "LdPropertyIds.h"
#include "LdIntegerProperty.h"

#include "comm/Legacy/M16/LtComM16.h"

using namespace LeddarCore;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::LdSensorM16Laser::LdSensorM16Laser( LeddarConnection::LdConnection *aConnection )
///
/// \brief  Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param [in,out] aConnection Connection information
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorM16Laser::LdSensorM16Laser( LeddarConnection::LdConnection *aConnection ) :
    LdSensorM16( aConnection )
{
    InitProperties();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::LdSensorM16Laser::~LdSensorM16Laser()
///
/// \brief  Destructor
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorM16Laser::~LdSensorM16Laser()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Laser::InitProperties( void )
///
/// \brief  Initializes the properties for this specific sensor
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16Laser::InitProperties( void )
{
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_START_TRACE,
                              LtComM16::M16_ID_CFG_START_TRACE_INDEX, 4, "Number of base points before the sensor actually starting to detect" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_START_TRACE_LIMITS,
                              LtComM16::M16_ID_LIMIT_START_TRACE_INDEX, 4, "Limits of Start_trace" ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Laser::GetConstants( void )
///
/// \brief  Gets the constants
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensorM16Laser::GetConstants( void )
{
    LdSensorM16::GetConstants();

    uint16_t lIds[] =
    {
        LtComM16::M16_ID_LIMIT_START_TRACE_INDEX
    };

    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, LT_ALEN( lIds ), sizeof( lIds[ 0 ] ), lIds, sizeof( lIds[ 0 ] ) );
    mProtocolConfig->SendRequest();

    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );

    LdIntegerProperty *lStartTrace = GetProperties()->GetIntegerProperty( LdPropertyIds::ID_START_TRACE );
    LdIntegerProperty *lStartTraceLimits = GetProperties()->GetIntegerProperty( LdPropertyIds::ID_START_TRACE_LIMITS );
    lStartTrace->SetLimits( lStartTraceLimits->Value( 0 ), lStartTraceLimits->Value( 1 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn float LeddarDevice::LdSensorM16Laser::GetStartTraceDistance( int aValue )
///
/// \brief  Convert start trace property value to a distance in meter
///
/// \exception  std::logic_error    Raised when a logic error condition occurs.
///
/// \param  aValue  empty or -1 for current start trace distance
///                 anything > 0 to compute for a theoretical value
///
/// \return The start trace distance in meter.
///
/// \author David Levy
/// \date   June 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
float
LeddarDevice::LdSensorM16Laser::GetStartTraceDistance( int aValue )
{
    if( mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_TIMEBASE_DELAY )->Count() == 0 )
    {
        throw std::logic_error( "Call GetCalib before computing start trace distance." );
    }

    if( mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_REAL_DISTANCE_OFFSET )->Count() == 0 ||
            mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_BASE_SAMPLE_DISTANCE )->Count() == 0 )
    {
        throw std::logic_error( "Call GetConstants before computing start trace distance." );
    }

    float lMinTBD = 0, lPointDist = 0, lOffset = 0;

    for( size_t i = 0; i < mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_TIMEBASE_DELAY )->Count(); ++i )
    {
        if( mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_TIMEBASE_DELAY )->Value( i ) < lMinTBD )
        {
            lMinTBD = mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_TIMEBASE_DELAY )->Value( i );
        }
    }

    lPointDist = mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_BASE_SAMPLE_DISTANCE )->Value( 0 );
    lOffset =  mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_REAL_DISTANCE_OFFSET )->Value( 0 );

    int32_t lStartTraceValue = 0;

    if( aValue == -1 )
    {
        lStartTraceValue = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_START_TRACE )->ValueT<int32_t>( 0 );
    }
    else if( aValue >= 0 )
    {
        lStartTraceValue = aValue;
    }
    else
    {
        throw std::logic_error( "Wrong argument value (start trace is supposed to be >=0)." );
    }

    return lStartTraceValue * lPointDist + lMinTBD - lOffset;
}
#endif