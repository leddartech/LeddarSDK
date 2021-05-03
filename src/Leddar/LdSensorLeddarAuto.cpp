// *****************************************************************************
// Module..: LeddarPrototype
//
/// \file    LdSensorLeddarAuto.cpp
///
/// \brief   Class of all LeddarAuto sensors.
///
/// \author  Patrick Boulay
///
/// \since   December 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************
#include "LdSensorLeddarAuto.h"
#if defined( BUILD_ETHERNET ) && defined( BUILD_AUTO )

#include "LdPropertyIds.h"

#include "LdBitFieldProperty.h"
#include "LdBoolProperty.h"
#include "LdBufferProperty.h"
#include "LdFloatProperty.h"
#include "LdIntegerProperty.h"
#include "LdTextProperty.h"

#include "LdConnectionFactory.h"
#include "LdProtocolLeddarTech.h"

#include "LtCRCUtils.h"
#include "LtExceptions.h"
#include "LtFileUtils.h"
#include "LtMathUtils.h"
#include "LtScope.h"
#include "LtStringUtils.h"

#include "comm/LtComEthernetPublic.h"
#include "comm/LtComLeddarTechPublic.h"

#include <cstring>
#include <limits>

using namespace LeddarCore;

// *****************************************************************************
// Function: LdSensorLeddarAuto::LdSensorLeddarAuto
//
/// \brief   Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param  aConnection Connection information
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************
LeddarDevice::LdSensorLeddarAuto::LdSensorLeddarAuto( LeddarConnection::LdConnection *aConnection )
    : LdSensor( aConnection )
    , mProtocolConfig( nullptr )
    , mProtocolData( nullptr )
    , mPingEnabled( true )
    , mAllDataReceived( false )
{
    LdSensorLeddarAuto::InitProperties();
    mProtocolConfig = dynamic_cast<LeddarConnection::LdProtocolLeddartechEthernet *>( aConnection );
    auto lTimeStamp = new LeddarCore::LdIntegerProperty( LeddarCore::LdProperty::CAT_INFO, LeddarCore::LdProperty::F_SAVE | LeddarCore::LdProperty::F_NO_MODIFIED_WARNING,
                                                         LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP64, 0, 8, "Timestamp in usec since 1970/01/01" );
    lTimeStamp->ForceValue( 0, 0 );
    GetResultEchoes()->AddProperty( lTimeStamp );
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::~LdSensorLeddarAuto
//
/// \brief   Destructor
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************
LeddarDevice::LdSensorLeddarAuto::~LdSensorLeddarAuto( void )
{
    if( mProtocolData != nullptr )
    {
        delete mProtocolData;
        mProtocolData = nullptr;
    }
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::Connect
//
/// \brief   Connect to device
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::Connect( void )
{
    LdDevice::Connect();
    ConnectDataServer();
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::Disconnect
//
/// \brief   Disconnect the device
///
/// \author  Patrick Boulay
///
/// \since   August 2017
// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::Disconnect( void )
{
    if( mProtocolData != nullptr )
    {
        mProtocolData->Disconnect();
    }

    LdDevice::Disconnect();
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::ConnectDataServer
//
/// \brief   Connect to the data server
///
/// \author  Patrick Boulay
///
/// \since   August 2017
// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::ConnectDataServer( void )
{
    std::vector<uint16_t> lDeviceIds;
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_AUTO_DATA_SERVER_PORT );
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_AUTO_DATA_SERVER_PROTOCOL );
    RequestProperties( mProperties, lDeviceIds );

    // Create connection info object for the data server
    const LeddarConnection::LdConnectionInfoEthernet *lConnectionInfoEthernet =
        dynamic_cast<const LeddarConnection::LdConnectionInfoEthernet *>( mProtocolConfig->GetConnectionInfo() );

    LeddarConnection::LdConnectionInfoEthernet *lConnectionInfoEthernetDataServer = nullptr;

    mIsTCPDataServer = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DATA_SERVER_PROTOCOL )->Value() == LtComLeddarTechPublic::LT_COMM_PROTOCOL_TCP;

    // TCP connection to the data server
    if( mIsTCPDataServer )
    {
        lConnectionInfoEthernetDataServer = new LeddarConnection::LdConnectionInfoEthernet(
            lConnectionInfoEthernet->GetIP(), mProperties->GetIntegerProperty( LdPropertyIds::ID_DATA_SERVER_PORT )->ValueT<uint32_t>(), "", lConnectionInfoEthernet->GetType(),
            LeddarConnection::LdConnectionInfoEthernet::PT_TCP );
    }
    // UDP connection to the data server
    else
    {
        lConnectionInfoEthernetDataServer = new LeddarConnection::LdConnectionInfoEthernet(
            lConnectionInfoEthernet->GetAddress(), mProperties->GetIntegerProperty( LdPropertyIds::ID_DATA_SERVER_PORT )->ValueT<uint32_t>(), "",
            lConnectionInfoEthernet->GetType(), LeddarConnection::LdConnectionInfoEthernet::PT_UDP );
    }

    LeddarConnection::LdConnection *lDataServerConnection = LeddarConnection::LdConnectionFactory::CreateConnection( lConnectionInfoEthernetDataServer );
    mProtocolData                                         = dynamic_cast<LeddarConnection::LdProtocolLeddarTech *>( lDataServerConnection );
    mProtocolData->SetDataServer( true );

    mProtocolData->Connect();
}

/// *****************************************************************************
/// Function: LdSensorLeddarAuto::InitProperties
///
/// \brief   Create properties for this specific sensor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
/// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::InitProperties( void )
{
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_NAME,
                                                  LtComLeddarTechPublic::LT_COMM_ID_DEVICE_NAME, LT_COMM_DEVICE_NAME_LENGTH, LeddarCore::LdTextProperty::TYPE_UTF8,
                                                  "Device name" ) );

    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, LtComLeddarTechPublic::LT_COMM_ID_HW_PART_NUMBER,
                                                  LT_COMM_PART_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII, "Part number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER,
                                                  LtComLeddarTechPublic::LT_COMM_ID_SOFTWARE_PART_NUMBER, LT_COMM_PART_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII,
                                                  "Software part number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, LtComLeddarTechPublic::LT_COMM_ID_SERIAL_NUMBER,
                                                  LT_COMM_SERIAL_NUMBER_LENGTH, LeddarCore::LdTextProperty::TYPE_ASCII, "Serial number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, LtComLeddarTechPublic::LT_COMM_ID_FPGA_VERSION,
                                                  LT_COMM_FPGA_VERSION_LENGTH, LeddarCore::LdTextProperty::TYPE_ASCII, "FPGA version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_GROUP_ID_NUMBER, LtComLeddarTechPublic::LT_COMM_ID_GROUP_ID,
                                                  LT_COMM_GROUP_ID_LENGTH, LeddarCore::LdTextProperty::TYPE_ASCII, "Group id" ) );
    mProperties->AddProperty(
        new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_RELEASE_TYPE, LtComLeddarTechPublic::LT_COMM_ID_RELEASE_TYPE, 1, "Release type" ) );
    mProperties->AddProperty(
        new LdBitFieldProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS, LtComLeddarTechPublic::LT_COMM_ID_DEVICE_OPTIONS, 4, "Device options" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL,
                                                     LtComLeddarTechPublic::LT_COMM_ID_MAX_ECHOES_PER_CHANNEL, 2, "Maximum echoes per channel" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_DISTANCE_SCALE,
                                                     LtComLeddarTechPublic::LT_COMM_ID_DISTANCE_SCALE, 4, "Distance scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_ECHO_AMPLITUDE_MAX,
                                                     LtComLeddarTechPublic::LT_COMM_ID_ECHO_AMPLITUDE_MAX, 4, "Maximum possible amplitude value" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_RAW_AMP_SCALE,
                                                     LtComLeddarTechPublic::LT_COMM_ID_AMPLITUDE_SCALE, 4, "Raw amplitude scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FILTERED_AMP_SCALE,
                                                     LtComLeddarTechPublic::LT_COMM_ID_FILTERED_SCALE, 4, "Filtered amplitude scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_TEMPERATURE_SCALE,
                                                     LtComLeddarTechPublic::LT_COMM_ID_TEMPERATURE_SCALE, 4, "Temperature scale" ) );

    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_MAC_ADDRESS,
                                                    LtComEthernetPublic::LT_COMM_ID_IPV4_ETHERNET_ADDRESS, sizeof( LtComEthernetPublic::LtIpv4EthernetAddress ), "Mac address" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IP_ADDRESS,
                                                    LtComEthernetPublic::LT_COMM_ID_IPV4_IP_ADDRESS, sizeof( LtComEthernetPublic::LtIpv4IpAddress ), "IP Address configuration" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IP_MODE,
                                                  LtComEthernetPublic::LT_COMM_ID_IPV4_IP_MODE, "Static/DHCP IP" ) );

    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, /*LdProperty::F_EDITABLE |*/ LdProperty::F_SAVE, LdPropertyIds::ID_DATA_SERVER_PORT,
                                                     LtComLeddarTechPublic::LT_COMM_ID_AUTO_DATA_SERVER_PORT, 2, "Data port" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_DATA_SERVER_PROTOCOL,
                                                     LtComLeddarTechPublic::LT_COMM_ID_AUTO_DATA_SERVER_PROTOCOL, 1, "Data server protocol", false ) );

    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_BUFFER_SIZE_TCP,
                                                     LtComEthernetPublic::LT_COMM_ID_IPV4_TCP_BUFFER_SIZE, 2, "TCP Buffer size", false ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_BUFFER_SIZE_UDP,
                                                     LtComEthernetPublic::LT_COMM_ID_IPV4_UDP_BUFFER_SIZE, 2, "UDP Buffer size", false ) );

    // Calib
    GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_TIMEBASE_DELAY,
                                                       LtComLeddarTechPublic::LT_COMM_ID_TIMEBASE_DELAYS, 4, 0, 4, "Timebase delays" ) );
    GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_INTENSITY_COMPENSATIONS,
                                                       LtComLeddarTechPublic::LT_COMM_ID_COMPENSATIONS, 4, 0, 3, "Led power compensations" ) );

    mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->SetDeviceId( LtComLeddarTechPublic::LT_COMM_ID_AUTO_CHANNEL_NUMBER_VERTICAL );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->SetDeviceId( LtComLeddarTechPublic::LT_COMM_ID_AUTO_CHANNEL_NUMBER_HORIZONTAL );

    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_ETHERNET );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();

    // Extra result state properties
    GetResultStates()->GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP )->SetDeviceId( LtComLeddarTechPublic::LT_COMM_ID_TIMESTAMP );
    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_SYSTEM_TEMP,
                                                                          LtComLeddarTechPublic::LT_COMM_ID_SYS_TEMP, 4, 0, 2, "System Temperature" ) );
    GetResultStates()->GetProperties()->AddProperty(
        new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_CPU_LOAD, LtComLeddarTechPublic::LT_COMM_ID_CPU_LOAD_V2, 4, 0, 2, "Cpu Load" ) );
}

/// *****************************************************************************
/// Function: LdSensorLeddarAuto::GetConstants
///
/// \brief   Get device constants from device
///
/// \author  Patrick Boulay
///
/// \since   February 2017
/// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::GetConstants( void )
{
    GetCategoryPropertiesFromDevice( LeddarCore::LdProperty::CAT_CONSTANT, LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_DEVICE );

    GetResultStates()
        ->GetProperties()
        ->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )
        ->SetScale( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE )->ValueT<uint32_t>() );

    uint32_t lTotalSegments =
        mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ValueT<uint16_t>() * mProperties->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->ValueT<uint16_t>();
    uint32_t lMaxTotalEchoes = lTotalSegments * mProperties->GetIntegerProperty( LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL )->ValueT<uint8_t>();

    GetResultEchoes()->Init( mProperties->GetIntegerProperty( LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>(),
                             mProperties->GetIntegerProperty( LdPropertyIds::ID_RAW_AMP_SCALE )->ValueT<uint32_t>(), lMaxTotalEchoes );

    GetResultEchoes()->SetVChan( mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ValueT<uint16_t>() );
    GetResultEchoes()->SetHChan( mProperties->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->ValueT<uint16_t>() );
    GetResultEchoes()->SetVFOV( mProperties->GetFloatProperty( LdPropertyIds::ID_VFOV )->Value() );
    GetResultEchoes()->SetHFOV( mProperties->GetFloatProperty( LdPropertyIds::ID_HFOV )->Value() );
    GetResultEchoes()->Swap();
    GetResultStates()->Init( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE )->ValueT<uint32_t>(), 0 );

    UpdateConstants();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorLeddarAuto::UpdateConstants(void)
///
/// \brief  Updates the constants / scales
///
/// \author David Levy
/// \date   April 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorLeddarAuto::UpdateConstants( void )
{
    GetResultStates()
        ->GetProperties()
        ->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )
        ->SetScale( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE )->ValueT<uint32_t>() );
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::GetConfig
//
/// \brief   Get device configuration
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::GetConfig( void )
{
    GetCategoryPropertiesFromDevice( LeddarCore::LdProperty::CAT_CONFIGURATION, LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_CONFIG );
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::GetCalib
//
/// \brief   Get calibration properties from the device
///
/// \author  Patrick Boulay
///
/// \since   October 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::GetCalib( void )
{
    GetCategoryPropertiesFromDevice( LeddarCore::LdProperty::CAT_CALIBRATION, LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_CAL );
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::GetData
//
/// \brief   Get data on the device
///
/// \return True if a new data was received.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

bool LeddarDevice::LdSensorLeddarAuto::GetData( void )
{
    // Verify if the data mask is set
    if( mDataMask == DM_NONE )
    {
        SetDataMask( GetDataMaskAll() );
    }

    bool lReceivedData = false;

    // TCP Data server
    if( mIsTCPDataServer )
    {
        uint32_t lDataMask = mDataMask;
        std::exception lSavedException; // should use c++11 exception_ptr

        while( lDataMask > 0 )
        {
            RequestData( lDataMask );

            mAllDataReceived = false;

            // while( !mAllDataReceived && ( mProtocolData->GetRequestCode() != mProtocolData->GetReceivedRequestCode() ) )
            while( !mAllDataReceived )
            {
                // Read available data on the data channel
                try
                {
                    mProtocolData->ReadAnswer();
                }
                catch( LeddarException::LtComException &e )
                {
                    if( e.GetErrType() == LeddarException::ERROR_COM_READ && e.GetErrorMsg() == "Data reception was too slow (timed out once)." )
                    {
                        lSavedException = e;
                    }
                    else
                    {
                        throw;
                    }
                }

                uint16_t lRequestCode = mProtocolData->GetRequestCode();

                lReceivedData |= ProcessData( lRequestCode );

                if( std::string( lSavedException.what() ) == "Data reception was too slow (timed out once)." )
                {
                    throw lSavedException;
                }
            }
        }
    }
    // UDP Data server
    else
    {
        // Read available data on the data channel
        mProtocolData->ReadAnswer();
        uint16_t lRequestCode = mProtocolData->GetRequestCode();

        lReceivedData |= ProcessData( lRequestCode );
    }

    return lReceivedData;
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::ProcessData
//
/// \brief   Process data from the socket
///
/// \param   aRequestCode Code previously requested on the socket
///
/// \return  True if a data was processed.
///
/// \author  Patrick Boulay
///
/// \since   September 2017
// *****************************************************************************

bool LeddarDevice::LdSensorLeddarAuto::ProcessData( uint16_t aRequestCode )
{
    if( aRequestCode == LtComLeddarTechPublic::LT_COMM_DATASRV_REQUEST_SEND_ECHOES )
    {
        return ProcessEchoes();
    }
    else if( aRequestCode == LtComLeddarTechPublic::LT_COMM_DATASRV_REQUEST_SEND_STATES )
    {
        return ProcessStates();
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorLeddarAuto::RequestData( uint32_t &aMask )
///
/// \brief  Request data from the sensor
///
/// \param [in,out] aMask    Data mask to request. It will request only once.
///
/// \return Return true if a request was done.
///
/// \author Patrick Boulay
/// \date   September 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarDevice::LdSensorLeddarAuto::RequestData( uint32_t &aMask )
{
    // StartRequest and SendRequest are not used on UDP data server connection
    if( ( aMask & DM_ECHOES ) == DM_ECHOES )
    {
        mProtocolData->StartRequest( LtComLeddarTechPublic::LT_COMM_DATASRV_REQUEST_SEND_ECHOES );
        mProtocolData->SendRequest();
        aMask -= DM_ECHOES;
        return true;
    }
    else if( ( aMask & DM_STATES ) == DM_STATES )
    {
        mProtocolData->StartRequest( LtComLeddarTechPublic::LT_COMM_DATASRV_REQUEST_SEND_STATES );
        mProtocolData->SendRequest();
        aMask -= DM_STATES;
        return true;
    }

    return false;
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::ProcessEchoes
//
/// \brief   Process echoes read by GetData
///
/// return true if new echoes - false if no new echo
///
/// \author  Patrick Boulay
///
/// \since   September 2017
// *****************************************************************************
bool LeddarDevice::LdSensorLeddarAuto::ProcessEchoes( void )
{
    if( LtComLeddarTechPublic::LT_COMM_ANSWER_NO_NEW_DATA == mProtocolData->GetAnswerCode() )
    {
        SetDataReceived( true );
        return false;
    }
    else if( 0 != mProtocolData->GetAnswerCode() )
    {
        throw LeddarException::LtComException( "Error processing echoes - Answer code(" + LeddarUtils::LtStringUtils::IntToString( mProtocolData->GetAnswerCode() ) + ")",
                                               mProtocolData->GetAnswerCode(), false );
    }

    if( mProtocolData->GetMessageSize() == 0 ) // Should not happen, case should be covered by answer code != 0
    {

        // cppcheck-suppress unreadVariable
        auto lLock = mEchoes.GetUniqueLock( LeddarConnection::B_SET );
        mEchoes.SetEchoCount( 0 );
        SetDataReceived( true );
        return false;
    }

    auto lLock                                     = mEchoes.GetUniqueLock( LeddarConnection::B_SET );
    std::vector<LeddarConnection::LdEcho> &lEchoes = *mEchoes.GetEchoes( LeddarConnection::B_SET );

    uint8_t lDataReceivedStatus     = 0;
    uint32_t lTimestamp             = 0;
    uint32_t lStartIndexAndCount[2] = {};
    bool lFlush = false, lNewTrace = false;

    while( mProtocolData->ReadElement() )
    {
        if( !lFlush || mProtocolData->GetElementId() == LtComLeddarTechPublic::LT_COMM_ID_STATUS )
        {
            switch( mProtocolData->GetElementId() )
            {
            case LtComLeddarTechPublic::LT_COMM_ID_TIMESTAMP:

                mProtocolData->PushElementDataToBuffer( &lTimestamp, mProtocolData->GetElementCount(), sizeof( uint32_t ), sizeof( uint32_t ) );

                if( lTimestamp + std::numeric_limits<int16_t>::max() < mEchoes.GetTimestamp( LeddarConnection::B_SET ) ) // Timestamp loop
                {
                    lNewTrace = true;
                    mEchoes.SetTimestamp( lTimestamp );
                }
                else if( lTimestamp < mEchoes.GetTimestamp( LeddarConnection::B_SET ) ||
                         ( lTimestamp <= mEchoes.GetTimestamp( LeddarConnection::B_GET ) &&
                           lTimestamp + std::numeric_limits<int16_t>::max() > mEchoes.GetTimestamp( LeddarConnection::B_GET ) ) )
                {
                    // Old packet ignore it (should only be possible in UDP)
                    lFlush = true;
                }
                else if( lTimestamp > mEchoes.GetTimestamp( LeddarConnection::B_SET ) )
                {
                    // Check if we missed lDataReceivedStatus (should only be possible in UDP)
                    if( mEchoes.GetTimestamp( LeddarConnection::B_SET ) > mEchoes.GetTimestamp( LeddarConnection::B_GET ) )
                    {
                        // Emit trace changes.
                        lLock.unlock();
                        ComputeCartesianCoordinates();
                        mEchoes.Swap();
                        mEchoes.UpdateFinished();
                        lLock = mEchoes.GetUniqueLock( LeddarConnection::B_SET );
                    }

                    lNewTrace = true;
                    mEchoes.SetTimestamp( lTimestamp );
                }

                break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_TIMESTAMP64:
            {
                uint64_t lTimestamp64 = 0;
                mProtocolData->PushElementDataToBuffer( &lTimestamp64, mProtocolData->GetElementCount(), sizeof( uint64_t ), sizeof( uint64_t ) );
                mEchoes.SetPropertyValue(LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP64, 0, lTimestamp64);
            }
            break;

            case LtComLeddarTechPublic::LT_COMM_ID_FRAME_ID:
            {
                uint64_t lFrameId = 0;
                mProtocolData->PushElementDataToBuffer( &lFrameId, mProtocolData->GetElementCount(), sizeof( uint64_t ), sizeof( uint64_t ) );
                mEchoes.SetPropertyValue(LeddarCore::LdPropertyIds::ID_RS_FRAME_ID, 0, lFrameId);
            }
            break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_NUMBER_DATA_SENT:
                mProtocolData->PushElementDataToBuffer( lStartIndexAndCount, mProtocolData->GetElementCount(), sizeof( uint32_t ), sizeof( uint32_t ) );

                // If the timestamp is the same as the previous packet, the echoes was
                // sent into several packets
                if( !lNewTrace )
                {
                    mEchoes.SetEchoCount( mEchoes.GetEchoCount( LeddarConnection::B_SET ) + lStartIndexAndCount[1] );
                }
                else
                {
                    mEchoes.SetEchoCount( lStartIndexAndCount[1] );
                    LeddarConnection::LdEcho lEmptyEcho = {};
                    std::fill( mEchoes.GetEchoes( LeddarConnection::B_SET )->begin(), mEchoes.GetEchoes( LeddarConnection::B_SET )->end(), lEmptyEcho );
                }

                break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_ECHOES_AMPLITUDE:
                mProtocolData->PushElementDataToBuffer( &lEchoes[lStartIndexAndCount[0]].mAmplitude, mProtocolData->GetElementCount(),
                                                        sizeof( ( (LeddarConnection::LdEcho *)0 )->mAmplitude ), sizeof( lEchoes[0] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_ECHOES_DISTANCE:
                mProtocolData->PushElementDataToBuffer( &lEchoes[lStartIndexAndCount[0]].mDistance, mProtocolData->GetElementCount(),
                                                        sizeof( ( (LeddarConnection::LdEcho *)0 )->mDistance ), sizeof( lEchoes[0] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_ECHOES_CHANNEL_INDEX:
                mProtocolData->PushElementDataToBuffer( &lEchoes[lStartIndexAndCount[0]].mChannelIndex, mProtocolData->GetElementCount(),
                                                        sizeof( ( (LeddarConnection::LdEcho *)0 )->mChannelIndex ), sizeof( lEchoes[0] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_ECHOES_VALID:
                mProtocolData->PushElementDataToBuffer( &lEchoes[lStartIndexAndCount[0]].mFlag, mProtocolData->GetElementCount(),
                                                        sizeof( ( (LeddarConnection::LdEcho *)0 )->mFlag ), sizeof( lEchoes[0] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_ECHOES_TIMESTAMP_UTC:
                mProtocolData->PushElementDataToBuffer( &lEchoes[lStartIndexAndCount[0]].mTimestamp, mProtocolData->GetElementCount(),
                                                        sizeof( ( (LeddarConnection::LdEcho *)0 )->mTimestamp ), sizeof( lEchoes[0] ) );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_STATUS:

                mProtocolData->PushElementDataToBuffer( &lDataReceivedStatus, mProtocolData->GetElementCount(), sizeof( lDataReceivedStatus ), sizeof( lDataReceivedStatus ) );
                SetDataReceived( lDataReceivedStatus == 0 ? false : true );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_NOISE_LEVEL: 
                mEchoes.SetPropertyRawStorage( LeddarCore::LdPropertyIds::ID_RS_NOISE_LEVEL, reinterpret_cast<uint8_t *>( mProtocolData->GetElementData() ),
                                               mProtocolData->GetElementCount(), mProtocolData->GetElementSize() );
                break;

            case LtComLeddarTechPublic::LT_COMM_ID_AUTO_NOISE_LEVEL_MEAN:
            {
                uint32_t lNoiseMean = 0;
                mProtocolData->PushElementDataToBuffer( &lNoiseMean, mProtocolData->GetElementCount(), sizeof( uint32_t ), sizeof( uint32_t ) );
                mEchoes.SetPropertyValue( LeddarCore::LdPropertyIds::ID_RS_NOISE_LEVEL_AVG, 0, lNoiseMean );
            }
            break;
            }
        }
    }

    lLock.unlock();

    if( lFlush )
    {
        return false;
    }

    if( lDataReceivedStatus != 0 )
    {
        ComputeCartesianCoordinates();
        mEchoes.Swap();
        mEchoes.UpdateFinished();
    }

    return true;
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::ProcessStates
//
/// \brief   Process echoes read by GetData
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
bool LeddarDevice::LdSensorLeddarAuto::ProcessStates( void )
{
    SetDataReceived( true );

    if( LtComLeddarTechPublic::LT_COMM_ANSWER_NO_NEW_DATA == mProtocolData->GetAnswerCode() )
    {
        return false;
    }
    else if( LtComLeddarTechPublic::LT_COMM_ANSWER_OK != mProtocolData->GetAnswerCode() )
    {
        throw LeddarException::LtComException( "Error processing states - Answer code(" + LeddarUtils::LtStringUtils::IntToString( mProtocolData->GetAnswerCode() ) + ")",
                                               mProtocolData->GetAnswerCode(), false );
    }

    if( mProtocolData->GetMessageSize() == 0 ) // Should not happen, case should be covered by answer code != 0
    {
        return false;
    }

    uint32_t lTimeStamp = 0;

    // Read the first element to check if its a new timestamp
    if( mProtocolData->ReadElement() && LtComLeddarTechPublic::LT_COMM_ID_TIMESTAMP == mProtocolData->GetElementId() )
    {
        mProtocolData->PushElementDataToBuffer( &lTimeStamp, 1, sizeof( lTimeStamp ), sizeof( lTimeStamp ) );

        if( 0 != lTimeStamp && lTimeStamp != GetResultStates()->GetTimestamp() )
        {
            GetResultStates()->SetTimestamp( lTimeStamp );
            mProtocolData->ReadElementToProperties( GetResultStates()->GetProperties() );
            mStates.UpdateFinished();
        }
        else
            return false;
    }
    else
        return false;

    return true;
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::SetDataMask
//
/// \brief   Set data mask for the data end point.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::SetDataMask( uint32_t aDataMask )
{
    mDataMask          = aDataMask;
    uint32_t lDataMask = ConvertDataMaskToLTDataMask( aDataMask );
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_DATA_LEVEL_V2, 1, sizeof( lDataMask ), &lDataMask, sizeof( lDataMask ) );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException(
            "Set data mask error, request code: " + LeddarUtils::LtStringUtils::IntToString( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET ) +
                " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
            LeddarException::ERROR_COM_WRITE );
    }
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::SetConfig
//
/// \brief   Set config to the device
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::SetConfig( void )
{
    SetCategoryPropertiesOnDevice( LeddarCore::LdProperty::CAT_CONFIGURATION, LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET_CONFIG );
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::WriteConfig
//
/// \brief   Write config to the device
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::WriteConfig( void ) { SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_WRITE_CONFIG ); }

// *****************************************************************************
// Function: LdSensorLeddarAuto::RestoreConfig
//
/// \brief   Restore config to the device
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::RestoreConfig( void )
{
    SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESTORE_CONFIG );

    // TODO: To complete
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::Reset
//
/// \brief   Reset the device
///
/// \param   aType Reset type
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions, uint32_t )
{
    if( aType == LeddarDefines::RT_CONFIG_RESET )
    {
        SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESET_CONFIG );
    }
    else
    {
        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESET );

        if( aOptions == LeddarDefines::RO_NO_OPTION || ( aOptions & LeddarDefines::RO_FACTORY ) != 0 || ( aOptions & LeddarDefines::RO_MAIN ) != 0 )
        {
            uint8_t lSoftwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_MAIN;
            if( ( aOptions & LeddarDefines::RO_FACTORY ) != 0 )
                lSoftwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_FACTORY;
            mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_SOFTWARE_TYPE, 1, sizeof( lSoftwareType ), &lSoftwareType, sizeof( lSoftwareType ) );
        }

        if( ( aOptions & LeddarDefines::RO_SAFEMODE ) != 0 )
        {
            uint32_t lDummy = 0; // Value does not matter
            mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_SOFT_RESET, 1, sizeof( lDummy ), &lDummy, sizeof( lDummy ) );
        }

        mProtocolConfig->SendRequest();
        mProtocolConfig->ReadAnswer();
    }
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::RequestProperties
//
/// \brief   Request values of properties from the device.
///
/// \param   aProperties Property container.
/// \param   aDeviceIds  List of device ids to get from the device.
///
/// \author  Patrick Boulay
///
/// \since   June 2017
// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::RequestProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds )
{
    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, static_cast<uint16_t>( aDeviceIds.size() ), sizeof( uint16_t ), &aDeviceIds[0],
                                 sizeof( uint16_t ) );
    mProtocolConfig->SendRequest();

    mProtocolConfig->ReadAnswer();

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Request properties, request code: " + LeddarUtils::LtStringUtils::IntToString( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET ) +
                                                   " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                                               LeddarException::ERROR_COM_READ );
    }

    mProtocolConfig->ReadElementToProperties( aProperties );
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::SetProperties
//
/// \brief   Set values  of properties to the device.
///
/// \param   aProperties    Property container.
/// \param   aDeviceIds     List of device ids to set on the device.
/// \param   aRetryNbr      Number of retry of the Read before it actually throws the timeout throw. Usefull when firmware has to compute something before answering.
///
/// \author  Patrick Boulay
///
/// \since   June 2017
// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::SetProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds, unsigned int aRetryNbr )
{
    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );

    for( size_t i = 0; i < aDeviceIds.size(); ++i )
    {
        LeddarCore::LdProperty *lProperty = aProperties->FindDeviceProperty( aDeviceIds[i] );

        if( lProperty != nullptr )
        {
            auto lStorage = lProperty->GetStorage();
            mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET );
            mProtocolConfig->AddElement( aDeviceIds[i], static_cast<uint16_t>( lProperty->Count() ), lProperty->UnitSize(), lStorage.data(),
                                         static_cast<uint32_t>( lProperty->Stride() ) );
            mProtocolConfig->SendRequest();

            unsigned int lCount = aRetryNbr;
            bool lRetry         = false;

            do
            {
                try
                {
                    lRetry = false;
                    mProtocolConfig->ReadAnswer();

                    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
                    {
                        throw LeddarException::LtComException(
                            "Set properties error, request code: " + LeddarUtils::LtStringUtils::IntToString( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET ) +
                                " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                            LeddarException::ERROR_COM_WRITE );
                    }
                }
                catch( LeddarException::LtComException &e )
                {
                    if( e.GetDisconnect() == true )
                        throw;

                    ( lCount-- != 0 ) ? lRetry = true : throw;
                }
            } while( lRetry );
        }
    }
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::GetCategoryPropertiesFromDevice
//
/// \brief   Get properties from the device
///
/// \param   aCategory      Property category
/// \param   aRequestCode   Request code
///
/// \author  Patrick Boulay
///
/// \since   October 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::GetCategoryPropertiesFromDevice( LdProperty::eCategories aCategory, uint16_t aRequestCode )
{
    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );
    mProtocolConfig->StartRequest( aRequestCode );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Get category properties error, request code: " + LeddarUtils::LtStringUtils::IntToString( aRequestCode ) +
                                                   " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                                               LeddarException::ERROR_COM_READ );
    }

    mProtocolConfig->ReadElementToProperties( GetProperties() );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( aCategory );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::SetCategoryPropertiesOnDevice
//
/// \brief   Set properties on the device
///
/// \param   aCategory      Property category
/// \param   aRequestCode   Request code
///
/// \author  Patrick Boulay
///
/// \since   October 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::SetCategoryPropertiesOnDevice( LdProperty::eCategories aCategory, uint16_t aRequestCode )
{
    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );
    mProtocolConfig->StartRequest( aRequestCode );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( aCategory );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            auto lStorage = ( *lIter )->GetStorage();
            mProtocolConfig->AddElement( ( *lIter )->GetDeviceId(), static_cast<uint16_t>( ( *lIter )->Count() ), ( *lIter )->UnitSize(), lStorage.data(),
                                         static_cast<uint32_t>( ( *lIter )->Stride() ) );
        }
    }

    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Get category properties, request code: " + LeddarUtils::LtStringUtils::IntToString( aRequestCode ) +
                                                   " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                                               LeddarException::ERROR_COM_WRITE );
    }

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::SendCommand
//
/// \brief   Send command to sensor
///
/// \param   aRequestCode   Request code
///
/// \author  Patrick Boulay
///
/// \since   October 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::SendCommand( uint16_t aRequestCode, unsigned int aRetryNbr )
{
    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );
    mProtocolConfig->StartRequest( aRequestCode );
    mProtocolConfig->SendRequest();

    unsigned int lCount = aRetryNbr;
    bool lRetry         = false;

    do
    {
        try
        {
            lRetry = false;
            mProtocolConfig->ReadAnswer();

            if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
            {
                throw LeddarException::LtComException( "Send command, request code: " + LeddarUtils::LtStringUtils::IntToString( aRequestCode ) +
                                                           " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                                                       LeddarException::ERROR_COM_UNKNOWN );
            }
        }
        catch( LeddarException::LtComException &e )
        {
            if( e.GetDisconnect() == true )
                throw;

            ( lCount-- != 0 ) ? lRetry = true : throw;
        }
    } while( lRetry );
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::SendPing
//
/// \brief   Send a ping to the sensor to detect if its still alive.
///
/// \throw   Throw a LtNotConnectedException exception if the sensor is not connected.
///
/// \author  Patrick Boulay
///
/// \since   October 2017
// *****************************************************************************
void LeddarDevice::LdSensorLeddarAuto::SendPing( void )
{
    if( mPingEnabled )
        SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_ECHO );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDefines::sLicense LeddarDevice::LdSensorLeddarAuto::SendLicense( const uint8_t *aLicense, bool aVolatile = false )
///
/// \brief  Sends the license to sensor.
///
/// \exception  LeddarException::LtComException Thrown when a communication error occurs.
/// \exception  std::runtime_error              Raised when the license is invalid.
///
/// \param  aLicense    The license to send.
/// \param  aVolatile   (Optional) True if temporary volatile - Internal use.
///
/// \returns    A LeddarDefines::sLicense.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDefines::sLicense LeddarDevice::LdSensorLeddarAuto::SendLicense( const uint8_t *aLicense, bool aVolatile = false )
{
    // Store the license in the property, send it to the sensor, and read back from the sensor license info
    LdBufferProperty *lLicenseProp = GetProperties()->GetBufferProperty( aVolatile ? LdPropertyIds::ID_VOLATILE_LICENSE : LdPropertyIds::ID_LICENSE );

    if( lLicenseProp->Count() == 0 )
        lLicenseProp->SetCount( 1 );

    lLicenseProp->SetValue( 0, aLicense, LT_COMM_LICENSE_KEY_LENGTH );
    lLicenseProp->SetClean();

    std::vector<uint16_t> lDeviceIds;
    lDeviceIds.push_back( aVolatile ? LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE : LtComLeddarTechPublic::LT_COMM_ID_LICENSE );
    SetProperties( GetProperties(), lDeviceIds );

    LeddarDefines::sLicense lResultLicense;

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Wrong answer code : " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ) );
    }

    lDeviceIds.push_back( aVolatile ? LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE_INFO : LtComLeddarTechPublic::LT_COMM_ID_LICENSE_INFO );
    RequestProperties( GetProperties(), lDeviceIds );

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException( "Wrong answer code : " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ) );
    }

    LdIntegerProperty *lLicenseInfo = GetProperties()->GetIntegerProperty( aVolatile ? LdPropertyIds::ID_VOLATILE_LICENSE_INFO : LdPropertyIds::ID_LICENSE_INFO );
    lResultLicense.mLicense         = lLicenseProp->GetStringValue( 0 );
    lResultLicense.mType            = lLicenseInfo->Value() & 0xFFFF;
    lResultLicense.mSubType         = static_cast<uint8_t>( lLicenseInfo->ValueT<uint32_t>() >> 16 );

    for( const auto &lDeviceId : lDeviceIds )
    {
        if(auto *lProp = GetProperties()->FindDeviceProperty(lDeviceId))
            lProp->SetClean();
    }

    if( lResultLicense.mType == 0 )
    {
        throw std::runtime_error( "Invalid license." );
    }

    return lResultLicense;
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::SendLicense
//
/// \brief   Send the license to sensor.
///
/// param[in] aLicense : hexadecimal representation of a LT_COMM_LICENSE_KEY_LENGTH (16) bytes long buffer
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
LeddarDefines::sLicense LeddarDevice::LdSensorLeddarAuto::SendLicense( const std::string &aLicense, bool aVolatile )
{
    if( aLicense.length() != LT_COMM_LICENSE_KEY_LENGTH * 2 && aLicense.length() != 0 )
        throw std::runtime_error( "Invalid license length." );

    // Convert the user string to 16 bytes license
    uint8_t lBuffer[LT_COMM_LICENSE_KEY_LENGTH];

    for( size_t i = 0; i < aLicense.length(); i += 2 )
    {
        lBuffer[i / 2] = (uint8_t)strtoul( aLicense.substr( i, 2 ).c_str(), nullptr, 16 );
    }

    return SendLicense( lBuffer, aVolatile );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::vector<LeddarDevice::sLicense> LeddarDevice::LdSensorLeddarAuto::GetLicenses( void )
///
/// \brief  Gets the licenses on the sensor
///
/// \return The permanent licenses on the sensor.
///
/// \author David Levy
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<LeddarDefines::sLicense> LeddarDevice::LdSensorLeddarAuto::GetLicenses( void )
{
    std::vector<LeddarDefines::sLicense> lLicenses;

    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );
    std::vector<uint16_t> lDeviceIds;
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_LICENSE );
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_LICENSE_INFO );
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE );
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_VOLATILE_LICENSE_INFO );
    RequestProperties( GetProperties(), lDeviceIds );

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        return lLicenses;
    }

    for( const auto &lDeviceId : lDeviceIds )
    {
        if(auto *lProp = GetProperties()->FindDeviceProperty(lDeviceId))
            lProp->SetClean();
    }

    LdIntegerProperty *lLicenseInfo = GetProperties()->GetIntegerProperty( LdPropertyIds::ID_LICENSE_INFO );
    LdBufferProperty *lLicenseProp  = GetProperties()->GetBufferProperty( LdPropertyIds::ID_LICENSE );

    for( size_t i = 0; i < lLicenseProp->Count(); ++i )
    {
        LeddarDefines::sLicense lResultLicense;
        lResultLicense.mLicense = lLicenseProp->GetStringValue( i );
        lResultLicense.mType    = lLicenseInfo->Value( i ) & 0xFFFF;
        lResultLicense.mSubType = static_cast<uint8_t>( lLicenseInfo->ValueT<uint32_t>( i ) >> 16 );

        lLicenses.push_back( lResultLicense );
    }

    return lLicenses;
}

// *****************************************************************************
// Function: LdSensorLeddarAuto::RemoveLicense
//
/// \brief   Remove license on the device
///
/// \param  aLicense License to remove.
///
/// \author  Patrick Boulay
///
/// \since   November 2017
// *****************************************************************************

void LeddarDevice::LdSensorLeddarAuto::RemoveLicense( const std::string &aLicense )
{
    LdBufferProperty *lLicenseProp = GetProperties()->GetBufferProperty( LdPropertyIds::ID_LICENSE );
    std::string lCurrentLicense    = lLicenseProp->GetStringValue();
    std::transform( lCurrentLicense.begin(), lCurrentLicense.end(), lCurrentLicense.begin(), ::toupper );

    std::string lToRemove = aLicense;
    std::transform( lToRemove.begin(), lToRemove.end(), lToRemove.begin(), ::toupper );

    if( lToRemove == lCurrentLicense )
    {
        try
        {
            uint8_t lEmptyLicense[LT_COMM_LICENSE_KEY_LENGTH] = { 0 };
            SendLicense( lEmptyLicense );
        }
        catch( std::runtime_error &e )
        {
            // Invalid license sent on purpose to remove the real license
            if( strcmp( e.what(), "Invalid license." ) != 0 )
                throw;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorLeddarAuto::RemoveAllLicenses( void )
///
/// \brief  Removes all licenses
///
/// \author David Levy
/// \date   August 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorLeddarAuto::RemoveAllLicenses( void )
{
    uint8_t lEmptyLicense[LT_COMM_LICENSE_KEY_LENGTH];
    memset( lEmptyLicense, 0, LT_COMM_LICENSE_KEY_LENGTH );

    try
    {
        SendLicense( lEmptyLicense, false );
    }
    catch( std::runtime_error &e )
    {
        // Invalid license sent on purpose to remove the real license
        if( strcmp( e.what(), "Invalid license." ) != 0 )
            throw;
    }

    try
    {
        SendLicense( lEmptyLicense, true );
    }
    catch( std::runtime_error &e )
    {
        // Invalid license sent on purpose to remove the real license
        if( strcmp( e.what(), "Invalid license." ) != 0 )
            throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorLeddarAuto::UpdateFirmware( eFirmwareType aFirmwareType, const LdFirmwareData& aFirmwareData, LeddarCore::LdIntegerProperty* aProcessPercentage,
/// LeddarCore::LdBoolProperty* aCancel )
///
/// \brief  Updates the firmare of the sensor.
///
/// \exception  LeddarException::LtException    Thrown when a Lt error condition occurs.
/// \exception  LeddarException::LtComException Thrown when a Lt Com error condition occurs.
///
/// \param          aFirmwareType       Firmware type to send. (see \ref LeddarDevice::LdSensor::eFirmwareType)
/// \param          aFirmwareData       Firmware file (.bin).
/// \param [in,out] aProcessPercentage  If non-null, Pourcentage of completion set by this function.
/// \param [in,out] aCancel             If non-null, Set to true to cancel the operation.
///
/// \author Patrick Boulay
/// \date   November 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorLeddarAuto::UpdateFirmware( eFirmwareType aFirmwareType, const LdFirmwareData &aFirmwareData, LeddarCore::LdIntegerProperty *aProcessPercentage,
                                                       LeddarCore::LdBoolProperty *aCancel )
{
    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );
    uint8_t lFirmwareType = 0;

    switch( aFirmwareType )
    {
    case FT_DSP:
        lFirmwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_MAIN;
        break;

    case FT_FPGA:
        lFirmwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_FPGA;
        break;

    case FT_OS:
        lFirmwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_OS;
        break;

    default:
        throw LeddarException::LtException( "Invalid firmware type: " + LeddarUtils::LtStringUtils::IntToString( aFirmwareType ) );
    }

    uint16_t lCrc = LeddarUtils::LtCRCUtils::ComputeCRC16( &aFirmwareData.mFirmwareData[0], aFirmwareData.mFirmwareData.size() );

    uint32_t lFileSize             = static_cast<uint32_t>( aFirmwareData.mFirmwareData.size() );
    uint32_t lOpenCloseSessionFlag = 0;

    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_CRC16, 1, sizeof( lCrc ), &lCrc, sizeof( lCrc ) );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_PROCESSOR, 1, sizeof( lFirmwareType ), &lFirmwareType, sizeof( lFirmwareType ) );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_FILE_LENGTH, 1, sizeof( lFileSize ), &lFileSize, sizeof( lFileSize ) );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_OPEN_UPDATE_SESSION, 1, sizeof( lOpenCloseSessionFlag ), &lOpenCloseSessionFlag,
                                 sizeof( lOpenCloseSessionFlag ) );
    mProtocolConfig->SendRequest();

    mProtocolConfig->ReadAnswer();

    if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
    {
        throw LeddarException::LtComException(
            "Update firmware error, request code: " + LeddarUtils::LtStringUtils::IntToString( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE ) +
                " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
            LeddarException::ERROR_COM_WRITE );
    }

    // The device will tell us the block size to send the file.
    uint32_t lBlockSize = 1024;

    while( mProtocolConfig->ReadElement() )
    {
        if( mProtocolConfig->GetElementId() == LtComLeddarTechPublic::LT_COMM_ID_BLOCK_LENGTH )
        {
            lBlockSize = *( static_cast<uint32_t *>( mProtocolConfig->GetElementData() ) );
        }
    }

    if( lBlockSize == 0 )
    {
        throw LeddarException::LtException( "Transfert block length invalid(0)." );
    }

    // Send the file block by block
    uint32_t lCount = 0;

    while( ( lCount < lFileSize ) && ( aCancel == nullptr || !aCancel->Value() ) )
    {
        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE );
        mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_PROCESSOR, 1, sizeof( lFirmwareType ), &lFirmwareType, sizeof( lFirmwareType ) );

        if( lFileSize - lCount >= lBlockSize )
        {
            mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_RAW_DATA, lBlockSize, 1, &aFirmwareData.mFirmwareData[lCount], 1 );
        }
        else
        {
            mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_RAW_DATA, lFileSize - lCount, 1, &aFirmwareData.mFirmwareData[lCount], 1 );
        }

        mProtocolConfig->SendRequest();
        mProtocolConfig->ReadAnswer();

        if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
        {
            throw LeddarException::LtComException(
                "Update firmware error, request code: " + LeddarUtils::LtStringUtils::IntToString( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE ) +
                    " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                LeddarException::ERROR_COM_WRITE );
        }

        if( aProcessPercentage != nullptr )
        {
            aProcessPercentage->SetValue( 0, static_cast<uint8_t>( 100 * static_cast<double>( lCount ) / lFileSize ) );
        }

        lCount += lBlockSize;
    }

    if( aProcessPercentage != nullptr )
    {
        aProcessPercentage->SetValue( 0, 100 );
    }

    // Close the update session.
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_PROCESSOR, 1, sizeof( lFirmwareType ), &lFirmwareType, sizeof( lFirmwareType ) );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_CLOSE_UPDATE_SESSION, 1, sizeof( lOpenCloseSessionFlag ), &lOpenCloseSessionFlag,
                                 sizeof( lOpenCloseSessionFlag ) );
    mProtocolConfig->SendRequest();

    unsigned int lRetryCount = 15;
    bool lRetry              = false;

    do
    {
        try
        {
            lRetry = false;
            mProtocolConfig->ReadAnswer();

            if( mProtocolConfig->GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK )
            {
                throw LeddarException::LtComException(
                    "Update firmware error, request code: " + LeddarUtils::LtStringUtils::IntToString( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE ) +
                        " wrong answer code: " + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode() ),
                    LeddarException::ERROR_COM_WRITE, true );
            }
        }
        catch( LeddarException::LtComException &e )
        {
            if( e.GetDisconnect() == true )
                throw;

            ( lRetryCount-- != 0 ) ? lRetry = true : throw;
        }
    } while( lRetry );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn eFirmwareType LeddarDevice::LdSensorLeddarAuto::LtbTypeToFirmwareType( uint16_t aLtbType )
///
/// \brief  Ltb type to firmware type
///
/// \param  aLtbType    Type of the ltb.
///
/// \returns    An eFirmwareType.
///
/// \author David Levy
/// \date   August 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensor::eFirmwareType LeddarDevice::LdSensorLeddarAuto::LtbTypeToFirmwareType( uint32_t aLtbType )
{
    switch( aLtbType )
    {
    case LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_LEDDARAUTO_BIN:
        return FT_DSP;

    case LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_LEDDARAUTO_FGPA:
        return FT_FPGA;

    case LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_LEDDARAUTO_OS:
        return FT_OS;

    default:
        break;
    }

    return FT_INVALID;
}
#endif
