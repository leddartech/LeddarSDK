////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorDTec.cpp
///
/// \brief  Implements the LdSensorDTec class
///
/// Copyright (c) 2019 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdSensorDTec.h"

#if defined( BUILD_DTEC ) && defined( BUILD_ETHERNET )

#include "LdConnectionFactory.h"
#include "LdEthernet.h"

#include "LtCRCUtils.h"
#include "LtExceptions.h"
#include "LtFileUtils.h"
#include "LtScope.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"

#include "LdPropertyIds.h"
#include "comm/Legacy/DTec/LtComDTec.h"
#include "comm/Legacy/M16/LtComM16.h"
#include "comm/LtComEthernetPublic.h"
#include "comm/LtComLeddarTechPublic.h"

#ifndef _WIN32
#include <netinet/in.h>
#endif

using namespace LeddarCore;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::LdSensorDTec::LdSensorDTec( LeddarConnection::LdConnection *aConnection, bool aConnectToAuxiliaryDataServer )
///
/// \brief  Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param [in,out] aConnection                     Connection information.
/// \param          aConnectToAuxiliaryDataServer   True to connect to auxiliary data server.
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorDTec::LdSensorDTec( LeddarConnection::LdConnection *aConnection, bool aConnectToAuxiliaryDataServer )
    : LdSensor( aConnection )
    , mPingEnabled( !aConnectToAuxiliaryDataServer )
    , mAuxiliaryDataServer( aConnectToAuxiliaryDataServer )
{
    InitProperties();

    if( aConnectToAuxiliaryDataServer && aConnection->IsConnected() )
    {
        throw std::runtime_error( "Do not connect to config server when running in auxiliary data server" );
    }

    mProtocolConfig = dynamic_cast<LeddarConnection::LdProtocolLeddartechEthernet *>( aConnection );
}

LeddarDevice::LdSensorDTec::~LdSensorDTec( void ) {}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::InitProperties(void)
///
/// \brief  Initializes the properties of the device
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::InitProperties( void )
{
    // Constants
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, LtComLeddarTechPublic::LT_COMM_ID_SERIAL_NUMBER,
                                                  LT_COMM_SERIAL_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII, "Serial Number" ) );
    mProperties->AddProperty(
        new LdBitFieldProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS, LtComLeddarTechPublic::LT_COMM_ID_DEVICE_OPTIONS, 4, "Device options" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_MAC_ADDRESS,
                                                    LtComEthernetPublic::LT_COMM_ID_IPV4_ETHERNET_ADDRESS, sizeof( LtComEthernetPublic::LtIpv4EthernetAddress ), "Mac address" ) );

    // Config
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_NAME,
                                                  LtComLeddarTechPublic::LT_COMM_ID_DEVICE_NAME, LT_COMM_DEVICE_NAME_LENGTH, LeddarCore::LdTextProperty::TYPE_UTF16,
                                                  "Device name" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IP_MODE,
                                                  LtComEthernetPublic::LT_COMM_ID_IPV4_IP_MODE, "Static/DHCP IP" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IP_ADDRESS,
                                                    LtComEthernetPublic::LT_COMM_ID_IPV4_IP_ADDRESS, sizeof( LtComEthernetPublic::LtIpv4IpAddress ), "IP Address configuration" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_INTERFACE_GATEWAY_ADDRESS,
                                                    LtComEthernetPublic::LT_COMM_ID_IPV4_IP_GATEWAY, sizeof( LtComEthernetPublic::LtIpv4IpAddress ), "IP gateway configuration" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_INTERFACE_SUBNET_MASK,
                                                    LtComEthernetPublic::LT_COMM_ID_IPV4_IP_NET_MASK, sizeof( LtComEthernetPublic::LtIpv4IpAddress ),
                                                    "IP netmask configuration" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PHYSICAL_NEGOTIATION_MODE,
                                                  LtComDTec::LT_COMM_ID_IPV4_IP_PHY_MODE, 1, true, "Ethernet negotiation mode" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SENSIVITY,
                                                   LtComLeddarTechPublic::LT_COMM_ID_THREHSOLD_OFFSET, 4, 65536, 3, "Threshold offset / sensitivity" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_SENSITIVITY_ENABLE,
                                                  LtComLeddarTechPublic::LT_COMM_ID_STATIC_THRESHOLD_ENABLE, "Static threshold/sensitivity enable" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PAN_TILT,
                                                    LtComDTec::PDTECS_ID_CFG_PAN_TILT_POSITION, sizeof( LtComDTec::PDTECS_SXYCoordFP ), "Pan tilt position" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACTIVE_ZONES,
                                                      LtComDTec::PDTECS_ID_CFG_ACTIVE_ZONE_MASK, 1, "Bit mask for enabling or disabling detection zones" ) );

    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_XTALK_REMOVAL_ENABLE,
                                                  LtComDTec::PDTECS_ID_CFG_XTALK_REMOVAL_STATE, "Crosstalk removal enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_NOISE_REMOVAL_ENABLE,
                                                  LtComDTec::PDTECS_ID_CFG_STATIC_NOISE_REMOVAL_STATE, "Static noise removal enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PULSE_WIDTH_COMPENSATION,
                                                  LtComDTec::PDTECS_ID_CFG_PEAK_CHECK_PULSE_WIDTH_STATE, "Pulse width compensation enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_OVERSHOOT_MNG_ENABLE,
                                                  LtComDTec::PDTECS_ID_CFG_PEAK_OVERSHOOT_MANAGEMENT_STATE, "Overshoot managment enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SATURATION_COMP_ENABLE,
                                                  LtComDTec::PDTECS_ID_CFG_PEAK_DEFAULT_SAT_COMP_STATE, "Saturation compensation enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_XTALK_ECHO_REMOVAL_ENABLE,
                                                  LtComDTec::PDTECS_ID_CFG_PEAK_XTALK_ECHO_REMOVAL_STATE, "Echo crosstalk removal enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_TEMP_COMP,
                                                  LtComDTec::PDTECS_ID_CFG_PEAK_COMP_TEMP_STATE, "Temperature compensation enable" ) );

    /*
    Get Config - Other possible values
    4130    0x1022 PDTECS_ID_CFG_LANE_MODE
    4133    0x1025 PDTECS_ID_CFG_VANISHING
    4134    0x1026 PDTECS_ID_CFG_STOP_BAR_1
    4135    0x1027 PDTECS_ID_CFG_STOP_BAR_2
    4136    0x1028 PDTECS_ID_CFG_STOP_BAR_DISTANCE
    4137    0x1029 PDTECS_ID_CFG_LANE_DIST_MIN
    4138    0x102A PDTECS_ID_CFG_LANE_DIST_MAX
    4139    0x102B PDTECS_ID_CFG_LANE_XS
    4140    0x102C PDTECS_ID_CFG_LANE_LEFT_LIMIT
    4141    0x102D PDTECS_ID_CFG_LANE_RIGHT_LIMIT
    4142    0x102E PDTECS_ID_CFG_PRESENCE_VIRTUAL_CHANNEL
    4143    0x102F PDTECS_ID_CFG_EXCLUSION_ZONE
    4144    0x1030 PDTECS_ID_CFG_GT
    4145    0x1031 PDTECS_ID_CFG_LOOK_AT
    4212    0x1074 PDTECS_ID_CFG_ROLL
    4215    0x1077 PDTECS_ID_CFG_COUNTER_VIRTUAL_CHANNEL
    4156    0x103C PDTECS_ID_CFG_VIDENC_RC
    4157    0x103D PDTECS_ID_CFG_VIDENC_BIT_RATE
    4159    0x103F PDTECS_ID_CFG_VIDENC_RESOLUTION
    4169    0x1049 PDTECS_ID_CFG_VIDENC_IFRAME_INTERVAL
    4201    0x1069 PDTECS_ID_CFG_LANE_OPTIONS
    4222    0x107E PDTECS_ID_CFG_FAILSAFE_MODE
    4223    0x107F PDTECS_ID_CFG_FAILSAFE_THRESHOLDS
    4224    0x1080 PDTECS_ID_CFG_LED_INTENSITY
    PDTECS_ID_CFG_PEAK_CALIB_REF_PULSE_STATE
    32780   0x800C LTRCK_ID_CFG_INSTALL_TYPE
    4160    0x1040 PDTECS_ID_CFG_AUTO_EXP_ZONE
    4161    0x1041 PDTECS_ID_CFG_AUTO_EXP_BIAS
    4165    0x1045 PDTECS_ID_CFG_BRIGHTNESS_V2
    4131    0x1023 PDTECS_ID_CFG_ROTATE_IMAGE

    4172    0x104C PDTECS_ID_CFG_LANE_LABEL
    4173    0x104D PDTECS_ID_CFG_LANE_ARROW_TYPE
    4210    0x1072 PDTECS_ID_CFG_TIME

    4225    0x1081 PDTECS_ID_CFG_WATCHDOG_TIMEOUT
    */

    // Calib
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_TIMEBASE_DELAY,
                                                   LtComDTec::PDTECS_ID_CAL_CHAN_TIMEBASE_DELAY, 4, 65536, 3, "Timebase delays" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CHANNEL_AREA,
                                                    LtComDTec::PDTECS_ID_CAL_CHAN_AREA, sizeof( LtComDTec::PDTECS_SCalChanArea ), "Channel Area" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CAL_APD,
                                                    LtComDTec::PDTECS_ID_CAL_APD, sizeof( LtComDTec::PDTECS_SCalApd ), "APD calibration" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CAL_AMP,
                                                   LtComDTec::PDTECS_ID_CAL_AMP, 4, 0, 2, "Ampli calibration" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CAL_IMG,
                                                    LtComDTec::PDTECS_ID_CAL_IMG, sizeof( LtComDTec::PDTECS_SCalImg ), "Image calibration" ) );

    // Info
    mProperties->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->SetDeviceId( LtComLeddarTechPublic::LT_COMM_ID_NUMBER_OF_SEGMENTS );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL, 0, 1, "Max Detection per Segment" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DISTANCE_SCALE, LtComLeddarTechPublic::LT_COMM_ID_DISTANCE_SCALE,
                                                     4, "Distance scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RAW_AMP_SCALE, LtComLeddarTechPublic::LT_COMM_ID_AMPLITUDE_SCALE,
                                                     2, "Raw amplitude scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FILTERED_AMP_SCALE,
                                                     LtComLeddarTechPublic::LT_COMM_ID_FILTERED_SCALE, 4, "Amplitude scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_TEMPERATURE_SCALE,
                                                     LtComLeddarTechPublic::LT_COMM_ID_TEMPERATURE_SCALE, 4, "Temperature scale" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_REFRESH_RATE, LtComLeddarTechPublic::LT_COMM_ID_REFRESH_RATE, 4, 0,
                                                   2, "Theoretical refresh rate" ) );
    mProperties->AddProperty(
        new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_BOOTLOADER_VERSION, LtComDTec::PDTECS_ID_BOOTLOADER_VERSION, 2, "Bootloader version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_BOOTLOADER_PART_NUMBER, LtComDTec::PDTECS_ID_BOOTLOADER_PART_NUMBER,
                                                  LT_COMM_PART_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII, "Bootloader part number" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RECEIVER_BOARD_VERSION, LtComDTec::PDTECS_ID_RECEIVER_BRD_VERSION,
                                                     1, "Receiver board version" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_SENSIVITY_LIMITS,
                                                   LtComLeddarTechPublic::LT_COMM_ID_THREHSOLD_OFFSET_LIMITS, 4, 0, 1, "Threshold/sensitivity offset limits" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, LtComLeddarTechPublic::LT_COMM_ID_HW_PART_NUMBER,
                                                  LT_COMM_PART_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII, "Part Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER,
                                                  LtComLeddarTechPublic::LT_COMM_ID_SOFTWARE_PART_NUMBER, LT_COMM_PART_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII,
                                                  "Software part number" ) );
    mProperties->AddProperty(
        new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, LtComLeddarTechPublic::LT_COMM_ID_FPGA_VERSION, 1, "FPGA version" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_INT,
                                                     LtComLeddarTechPublic::LT_COMM_ID_FIRMWARE_VERSION, 2, "Firmware version" ) );

    // Status
    GetResultStates()->GetProperties()->AddProperty(
        new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_CPU_LOAD, LtComLeddarTechPublic::LT_COMM_ID_CPU_LOAD_V2, 4, 0, 2, "Cpu Load" ) );
    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_SYSTEM_TEMP,
                                                                          LtComLeddarTechPublic::LT_COMM_ID_SYS_TEMP, 4, 0, 2, "APD / source board temperature" ) );
    GetResultStates()->GetProperties()->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_CURRENT_TIMES_MS,
                                                                            LtComLeddarTechPublic::LT_COMM_ID_CURRENT_TIME_MS, 4, "System time in ms since last reset" ) );

    mProperties->GetIntegerProperty( LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_ETHERNET );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();

    mProperties->GetEnumProperty( LdPropertyIds::ID_PHYSICAL_NEGOTIATION_MODE )->AddEnumPair( LtComDTec::LT_IPV4_PHY_MODE_AUTO_NEGOTIATION, "Auto negotiation" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_PHYSICAL_NEGOTIATION_MODE )->AddEnumPair( LtComDTec::LT_IPV4_PHY_MODE_HALF_DUPLEX_10, "Half duplex 10" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_PHYSICAL_NEGOTIATION_MODE )->AddEnumPair( LtComDTec::LT_IPV4_PHY_MODE_FULL_DUPLEX_10, "Full duplex 10" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_PHYSICAL_NEGOTIATION_MODE )->AddEnumPair( LtComDTec::LT_IPV4_PHY_MODE_HALF_DUPLEX_100, "Half duplex 100" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_PHYSICAL_NEGOTIATION_MODE )->AddEnumPair( LtComDTec::LT_IPV4_PHY_MODE_FULL_DUPLEX_100, "Full duplex 100" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::Connect( void )
///
/// \brief  Connects to the device
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::Connect( void )
{
    if( !mAuxiliaryDataServer )
        LdDevice::Connect();

    ConnectDataServer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::ConnectDataServer( void )
///
/// \brief  Connects to the data server
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::ConnectDataServer( void )
{
    // Create connection info object for the data server
    const LeddarConnection::LdConnectionInfoEthernet *lConnectionInfoConfigServer =
        dynamic_cast<const LeddarConnection::LdConnectionInfoEthernet *>( mProtocolConfig->GetConnectionInfo() );

    uint16_t lDataPort = mAuxiliaryDataServer ? LtComDTec::DTEC_AUX_DATA_PORT : LtComDTec::DTEC_DATA_PORT;
    LeddarConnection::LdConnectionInfoEthernet *lConnectionInfoDataServer =
        new LeddarConnection::LdConnectionInfoEthernet( lConnectionInfoConfigServer->GetAddress(), lDataPort, "Data server connection", lConnectionInfoConfigServer->GetType(),
                                                        LeddarConnection::LdConnectionInfoEthernet::PT_TCP );

    LeddarConnection::LdConnection *lDataServerConnection = LeddarConnection::LdConnectionFactory::CreateConnection( lConnectionInfoDataServer );
    mProtocolData                                         = dynamic_cast<LeddarConnection::LdProtocolLeddarTech *>( lDataServerConnection );
    mProtocolData->SetDataServer( true );

    try
    {
        mProtocolData->Connect();
    }
    catch( LeddarException::LtComException & )
    {
        // Workaround a bug in older firmware if data mask is 0
        if( !mAuxiliaryDataServer )
        {
            SetDataMask( DM_ECHOES );
            LeddarUtils::LtTimeUtils::Wait( 500 );
            mProtocolData->Connect();
        }
        else
        {
            throw;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::Disconnect( void )
///
/// \brief  Disconnects from the device
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::Disconnect( void )
{
    if( mProtocolData != nullptr )
    {
        mProtocolData->Disconnect();
    }

    if( !mAuxiliaryDataServer )
        LdDevice::Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::SendCommand( uint16_t aRequestCode, uint32_t aTimeoutSec )
///
/// \brief  Sends a command to the sensor and waits for the answer
///
/// \exception  LeddarException::LtComException Thrown when a Lt Com error condition occurs.
///
/// \param  aRequestCode    The request code.
/// \param  aTimeoutSec     The number of allowed timeout.
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::SendCommand( uint16_t aRequestCode, uint32_t aTimeout )
{
    if( mAuxiliaryDataServer )
        return;

    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );

    mProtocolConfig->StartRequest( aRequestCode );
    mProtocolConfig->SendRequest();
    ReadAnswer( aTimeout );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::ReadAnswer( uint32_t aTimeout )
///
/// \brief  Wrapper around mProtocolConfig->ReadAnswer to allow timeouts
///
/// \param  aTimeout    The timeout.
///
/// \author David Levy
/// \date   November 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::ReadAnswer( uint32_t aTimeout )
{
    if( mAuxiliaryDataServer )
        return;

    // For some devices (e.g. the v-tec because of reference traces) it can
    // be quite long to write the config to permanent memory and the data
    // server does not send data during that period, so we must set the
    // timeout longer during the write.
    bool lBusy       = true;
    uint8_t lTimeout = aTimeout;

    while( lBusy )
    {
        try
        {
            // Wait for return code
            mProtocolConfig->ReadAnswer();
        }
        catch( LeddarException::LtComException )
        {
            if( lTimeout-- == 0 )
            {
                throw LeddarException::LtComException( "Read answer timeout" );
            }

            continue;
        }

        lBusy = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::GetConstant( void )
///
/// \brief  Get device constants
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::GetConstants( void )
{
    if( !mAuxiliaryDataServer )
    {
        QueryDeviceInfo();
        SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_DEVICE );

        mProtocolConfig->ReadElementToProperties( GetProperties() );

        std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONSTANT );

        for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
        {
            if( ( *lIter )->Modified() )
            {
                ( *lIter )->SetClean();
            }
        }

        uint16_t lIds[] = {
            LtComLeddarTechPublic::LT_COMM_ID_DEVICE_TYPE,     LtComLeddarTechPublic::LT_COMM_ID_NUMBER_OF_SEGMENTS, LtComLeddarTechPublic::LT_COMM_ID_DISTANCE_SCALE,
            LtComLeddarTechPublic::LT_COMM_ID_AMPLITUDE_SCALE, LtComLeddarTechPublic::LT_COMM_ID_FILTERED_SCALE,     LtComLeddarTechPublic::LT_COMM_ID_REFRESH_RATE,
            LtComDTec::PDTECS_ID_BOOTLOADER_VERSION,           LtComDTec::PDTECS_ID_BOOTLOADER_PART_NUMBER,          LtComDTec::PDTECS_ID_RECEIVER_BRD_VERSION };

        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
        mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, LT_ALEN( lIds ), sizeof( lIds[0] ), lIds, sizeof( lIds[0] ) );
        mProtocolConfig->SendRequest();

        mProtocolConfig->ReadAnswer();
        mProtocolConfig->ReadElementToProperties( GetProperties() );
    }

    UpdateConstants();

    uint32_t lTotalSegments =
        mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ValueT<uint16_t>() * mProperties->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->ValueT<uint16_t>();
    uint32_t lMaxTotalEchoes = lTotalSegments * mProperties->GetIntegerProperty( LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL )->ValueT<uint8_t>();

    GetResultEchoes()->Init( mProperties->GetIntegerProperty( LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>(),
                             mProperties->GetIntegerProperty( LdPropertyIds::ID_FILTERED_AMP_SCALE )->ValueT<uint32_t>(), lMaxTotalEchoes );
    GetResultEchoes()->SetVChan( mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ValueT<uint16_t>() );
    GetResultEchoes()->SetHChan( mProperties->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->ValueT<uint16_t>() );
    GetResultEchoes()->Swap();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::UpdateConstants( void )
///
/// \brief  Updates the constants
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::UpdateConstants( void )
{
    if( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT )->Count() == 0 )
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT )->ForceValue( 0, LtComDTec::DTEC_NUMBER_OF_CHANNEL );

    if( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL )->Count() == 0 )
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL )->ForceValue( 0, LtComDTec::DTEC_MAX_ECHOES_BY_CHANNEL );

    if( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->Count() == 0 )
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ForceValue( 0, LtComDTec::DTEC_DISTANCE_SCALE );

    if( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE )->Count() == 0 )
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE )->ForceValue( 0, LtComDTec::DTEC_FILTERED_AMP_SCALE );

    if( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE )->Count() == 0 )
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE )->ForceValue( 0, LtComDTec::DTEC_TEMPERATURE_SCALE );

    if( mProperties->GetIntegerProperty( LdPropertyIds::ID_RAW_AMP_SCALE )->Count() == 0 )
        mProperties->GetIntegerProperty( LdPropertyIds::ID_RAW_AMP_SCALE )->ForceValue( 0, LtComDTec::DTEC_RAW_AMP_SCALE );

    GetResultStates()
        ->GetProperties()
        ->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )
        ->SetScale( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE )->ValueT<uint32_t>() );

    GetProperties()
        ->GetFloatProperty( LeddarCore::LdPropertyIds::ID_CAL_AMP )
        ->SetScale( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>() );

    LdFloatProperty *lSensitivity       = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY );
    LdFloatProperty *lSensitivityLimits = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_LIMITS );
    lSensitivity->SetScale( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RAW_AMP_SCALE )->ValueT<uint32_t>() );
    lSensitivityLimits->SetScale( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RAW_AMP_SCALE )->ValueT<uint32_t>() );

    if( lSensitivityLimits->Count() == 2 )
    {
        lSensitivity->SetRawLimits( lSensitivityLimits->RawValue( 0 ), lSensitivityLimits->RawValue( 1 ) );
    }

    // Horizontal field of view
    if( GetProperties()->FindProperty( LeddarCore::LdPropertyIds::ID_OPTIONS ) && GetProperties()->GetProperty( LeddarCore::LdPropertyIds::ID_OPTIONS )->Count() > 0 ) //No value if aux data server
    {
        LdFloatProperty *lHFOV = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_HFOV );
        float lValue           = 0;

        switch( GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_OPTIONS )->Value( 0 ) & LtComM16::LT_COMM_DEVICE_OPTION_LFOV_MASK )
        {
        case LtComM16::LT_COMM_DEVICE_OPTION_18_DEG_LFOV:
            lValue = 19.4f;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_34_DEG_LFOV:
            lValue = 36.4f;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_26_DEG_LFOV:
            lValue = 26;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_60_DEG_LFOV:
            lValue = 60;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_45_DEG_LFOV:
            lValue = 48;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_10_DEG_LFOV:
            lValue = 10;
            break;

        case LtComM16::LT_COMM_DEVICE_OPTION_100_DEG_LFOV:
            lValue = 100;
            break;
        }

        lHFOV->ForceValue( 0, lValue );
        lHFOV->SetClean();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::GetConfig( void )
///
/// \brief  Get device configuration
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::GetConfig( void )
{
    if( mAuxiliaryDataServer )
        return;

    SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_CONFIG );

    mProtocolConfig->ReadElementToProperties( GetProperties() );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::SetConfig( void )
///
/// \brief  Set config to the device
///
/// \exception  LeddarException::LtComException Thrown when a Lt Com error condition occurs.
///
/// \author David Levy
/// \date   October 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::SetConfig( void )
{
    if( mAuxiliaryDataServer )
        return;

    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET_CONFIG );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

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
        throw LeddarException::LtComException( "Wrong answer code to SetConfig: 0x" + LeddarUtils::LtStringUtils::IntToString( mProtocolConfig->GetAnswerCode(), 16 ),
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

void LeddarDevice::LdSensorDTec::WriteConfig( void )
{
    if( mAuxiliaryDataServer )
        return;

    SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_WRITE_CONFIG, 10 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::GetCalib( void )
///
/// \brief  Get device calibration properties
///
/// \author Patrick Boulay
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::GetCalib( void )
{
    if( mAuxiliaryDataServer )
        return;

    SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET_CAL );

    mProtocolConfig->ReadElementToProperties( GetProperties() );

    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CALIBRATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::SetDataMask( uint32_t aDataMask )
///
/// \brief  Sends the data mask to sensor for the UDP stream
///
/// \param  aDataMask   The data mask.
///
/// \author David Levy
/// \date   November 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::SetDataMask( uint32_t aDataMask )
{
    LeddarConnection::LdProtocolLeddarTech *lProtocol = mAuxiliaryDataServer ? mProtocolData : mProtocolConfig;

    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );

    mDataMask          = aDataMask;
    uint32_t lDataMask = ConvertDataMaskToLTDataMask( aDataMask );

    lProtocol->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET );
    lProtocol->AddElement( LtComLeddarTechPublic::LT_COMM_ID_DATA_LEVEL_V2, 1, sizeof( lDataMask ), &lDataMask, sizeof( lDataMask ) );
    lProtocol->SendRequest();

    if( !mAuxiliaryDataServer )
        lProtocol->ReadAnswer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorDTec::GetData( void )
///
/// \brief  Gets the data from the device
///
/// \returns    True if it succeeds, false if it fails.
///
/// \author David Levy
/// \date   November 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarDevice::LdSensorDTec::GetData( void )
{
    try
    {
        mProtocolData->ReadRequest();
    }
    catch( LeddarException::LtTimeoutException & )
    {
        return false;
    }

    uint16_t lRequestCode = mProtocolData->GetRequestCode();

    return ProcessData( lRequestCode );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorDTec::ProcessData( uint16_t aRequestCode )
///
/// \brief  Process the received data described by aRequestCode
///
/// \param  aRequestCode    The request code.
///
/// \returns    True if it any data is processed, else false.
///
/// \author David Levy
/// \date   November 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarDevice::LdSensorDTec::ProcessData( uint16_t aRequestCode )
{
    if( aRequestCode == LtComLeddarTechPublic::LT_COMM_DATASRV_REQUEST_SEND_ECHOES )
    {
        return ProcessEchoes();
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorDTec::ProcessEchoes( void )
///
/// \brief  Process the echoes
///
/// \exception  std::runtime_error  Raised when we receive unexpected data.
///
/// \returns    True if it we received any echoes, else false.
///
/// \author David Levy
/// \date   November 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarDevice::LdSensorDTec::ProcessEchoes( void )
{
    auto lLock = mEchoes.GetUniqueLock(LeddarConnection::B_SET);
    std::vector<LeddarConnection::LdEcho> &lEchoes = *mEchoes.GetEchoes( LeddarConnection::B_SET );
    uint32_t lTimestamp                            = 0;

    while( mProtocolData->ReadElement() )
    {
        switch( mProtocolData->GetElementId() )
        {
        case LtComLeddarTechPublic::LT_COMM_ID_TIMESTAMP:
            mProtocolData->PushElementDataToBuffer( &lTimestamp, mProtocolData->GetElementCount(), sizeof( uint32_t ), sizeof( uint32_t ) );
            mEchoes.SetTimestamp( lTimestamp );
            break;

        case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_AMPLITUDE:
            mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
            mProtocolData->PushElementDataToBuffer( &lEchoes[0].mAmplitude, mProtocolData->GetElementCount(), sizeof( ( (LeddarConnection::LdEcho *)0 )->mAmplitude ),
                                                    sizeof( lEchoes[0] ) );
            break;

        case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_DISTANCE:
            mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
            mProtocolData->PushElementDataToBuffer( &lEchoes[0].mDistance, mProtocolData->GetElementCount(), sizeof( ( (LeddarConnection::LdEcho *)0 )->mDistance ),
                                                    sizeof( lEchoes[0] ) );
            break;

        case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_CHANNEL_INDEX:
            mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
            mProtocolData->PushElementDataToBuffer( &lEchoes[0].mChannelIndex, mProtocolData->GetElementCount(), sizeof( ( (LeddarConnection::LdEcho *)0 )->mChannelIndex ),
                                                    sizeof( lEchoes[0] ) );
            break;

        case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_VALID:
            mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
            mProtocolData->PushElementDataToBuffer( &lEchoes[0].mFlag, mProtocolData->GetElementCount(), sizeof( ( (LeddarConnection::LdEcho *)0 )->mFlag ), sizeof( lEchoes[0] ) );
            break;

        case LtComLeddarTechPublic::LT_COMM_ID_ECHOES_BASE:
            mEchoes.SetEchoCount( mProtocolData->GetElementCount() );
            mProtocolData->PushElementDataToBuffer( &lEchoes[0].mBase, mProtocolData->GetElementCount(), sizeof( ( (LeddarConnection::LdEcho *)0 )->mBase ), sizeof( lEchoes[0] ) );

        default:
            // The sensor sends 3 other id that are not relevant: LT_COMM_ID_ECHOES_MAX_INDEX, LT_COMM_ID_ECHOES_AMPLITUDE_LOW_SCALE and LT_COMM_ID_ECHOES_SATURATION_WIDTH
            break;
        }
    }

    lLock.unlock();
    ComputeCartesianCoordinates();
    mEchoes.Swap();
    mEchoes.UpdateFinished();

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::GetStatus( void )
///
/// \brief  Get various status from sensor and keeps mProtocolConfig connection alive
///
/// \author David Levy
/// \date   November 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::GetStatus( void )
{
    if( mAuxiliaryDataServer )
        return;

    if( mPingEnabled )
    {
        // If you just want to keep the connection alive and not fetch status, use :
        // SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_ECHO );
        // instead of the commands below

        uint16_t lIds[] = {
            LtComLeddarTechPublic::LT_COMM_ID_CPU_LOAD_V2,
            LtComLeddarTechPublic::LT_COMM_ID_SYS_TEMP,
            LtComLeddarTechPublic::LT_COMM_ID_CURRENT_TIME_MS,
        };

        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
        mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, LT_ALEN( lIds ), sizeof( lIds[0] ), lIds, sizeof( lIds[0] ) );
        mProtocolConfig->SendRequest();

        mProtocolConfig->ReadAnswer();
        mProtocolConfig->ReadElementToProperties( GetResultStates()->GetProperties() );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions, uint32_t )
///
/// \brief  Resets the sensor
///
/// \param  aType       The type of reset.
/// \param  aOptions    Reset options. See \ref LeddarDefines::eResetOptions
/// \param  parameter3  Not used.
///
/// \author David Levy
/// \date   November 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions, uint32_t )
{
    if( mAuxiliaryDataServer )
        return;

    mPingEnabled = false;
    LeddarUtils::LtScope<bool> lPingEnabler( &mPingEnabled, true );

    if( aType == LeddarDefines::RT_SOFT_RESET )
    {
        uint8_t lSoftwareType = 0;

        if( aOptions == LeddarDefines::RO_MAIN )
        {
            lSoftwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_MAIN;
        }
        else if( aOptions == LeddarDefines::RO_FACTORY )
        {
            lSoftwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_FACTORY;
        }
        else
        {
            throw LeddarException::LtComException( "Reset option not valid: " + LeddarUtils::LtStringUtils::IntToString( aOptions ) + "." );
        }

        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESET );
        mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_SOFTWARE_TYPE, 1, 1, &lSoftwareType, 1 );
        mProtocolConfig->SendRequest();
        mProtocolConfig->ReadAnswer();
        mProtocolConfig->Disconnect();
    }
    else if( aType == LeddarDefines::RT_CONFIG_RESET )
    {
        SendCommand( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_RESET_CONFIG );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::UpdateFirmware( eFirmwareType aFirmwareType, const LdFirmwareData &aFirmwareData, LeddarCore::LdIntegerProperty *aProcessPercentage,
/// LeddarCore::LdBoolProperty * )
///
/// \brief  Update the Firmware of the device.
///
/// \exception  std::logic_error                Invalid firmware type.
/// \exception  LeddarException::LtComException Thrown when a Lt Com error condition occurs.
/// \exception  std::runtime_error              If the verify fail or CRC check failed.
///
/// \param          aFirmwareType       Firmware type to update.
/// \param          aFirmwareData       Vector with firmware data.
/// \param [in,out] aProcessPercentage  Integer property to show advance.
/// \param [in,out] parameter4          Not used
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::UpdateFirmware( eFirmwareType aFirmwareType, const LdFirmwareData &aFirmwareData, LeddarCore::LdIntegerProperty *aProcessPercentage,
                                                 LeddarCore::LdBoolProperty * )
{
    if( mAuxiliaryDataServer )
        return;

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

    case FT_FACTORY:
        lFirmwareType = LtComLeddarTechPublic::LT_COMM_SOFTWARE_TYPE_FACTORY;
        break; // Send this firmware to the factory memory

    default:
        throw std::logic_error( "Invalid firmware type: " + LeddarUtils::LtStringUtils::IntToString( aFirmwareType ) );
    }

    const uint16_t lCrc = LeddarUtils::LtCRCUtils::ComputeCRC16( &aFirmwareData.mFirmwareData[0], aFirmwareData.mFirmwareData.size() );

    mProtocolConfig->SetEchoState( false );

    // Hack to support side-tec Morpho that has different ids for some elements.
    uint16_t lOffset    = 0;
    uint8_t lDeviceType = (uint8_t)mProtocolConfig->GetDeviceType();

    if( lDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SIDETEC_M )
    {
        lOffset = LtComDTec::SIDETECM_ID_OFFSET;
    }

    // Send UPDATE request to the DTEC with the code appended.
    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_UPDATE );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_CRC16 + lOffset, 1, sizeof( uint16_t ), &lCrc, sizeof( uint16_t ) );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_PROCESSOR + lOffset, 1, sizeof( uint8_t ), &lFirmwareType, sizeof( uint8_t ) );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_RAW_DATA, 1, static_cast<uint32_t>( aFirmwareData.mFirmwareData.size() ), &aFirmwareData.mFirmwareData[0], 1 );

    mProtocolConfig->SendRequest();

    ReadAnswer( 100 );
    // And request something else to be sure its ok
    mProtocolConfig->QueryDeviceType();
    LeddarUtils::LtTimeUtils::Wait( 1000 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::LdSensor::eFirmwareType LeddarDevice::LdSensorDTec::LtbTypeToFirmwareType( uint32_t aLtbType )
///
/// \brief  Convert the LTB file signature to the type of update
///
/// \param  aLtbType    Ltb file signature.
///
/// \returns    A LeddarDevice::LdSensor::eFirmwareType.
///
/// \author David Levy
/// \date   November 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensor::eFirmwareType LeddarDevice::LdSensorDTec::LtbTypeToFirmwareType( uint32_t aLtbType )
{
    switch( aLtbType )
    {
    case LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_DTEC_BIN:
        return FT_DSP;

    case LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_DTEC_FPGA:
        return FT_FPGA;

    default:
        break;
    }

    return FT_INVALID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::QueryDeviceInfo( void )
///
/// \brief  Queries device information
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::QueryDeviceInfo( void )
{
    const LeddarConnection::LdConnectionInfoEthernet *lConnectionInfoEthernet =
        dynamic_cast<const LeddarConnection::LdConnectionInfoEthernet *>( mProtocolConfig->GetConnectionInfo() );
    LeddarConnection::LdEthernet lInterface( lConnectionInfoEthernet );
    lInterface.OpenUDPSocket( IDT_PORT );

    // Prepare a request to be sent (it is always the same).
    LtComLeddarTechPublic::sLtCommRequestHeader lRequest;
    lRequest.mSrvProtVersion   = LT_ETHERNET_IDENTIFY_PROT_VERSION;
    lRequest.mRequestCode      = LT_COMM_IDT_REQUEST_IDENTIFY;
    lRequest.mRequestTotalSize = sizeof( lRequest );

    lInterface.SendTo( lConnectionInfoEthernet->GetIP(), IDT_PORT, (uint8_t *)&lRequest, sizeof( lRequest ) );

    std::string lAddress;
    uint16_t lPort;

    LtComEthernetPublic::sLtIdtAnswerIdentifyDtec lAnswer;
    const uint32_t lSize = lInterface.ReceiveFrom( lAddress, lPort, (uint8_t *)&lAnswer, sizeof( lAnswer ) );

    // Check the if the received package is valid and fill properties with data
    if( ( lSize == sizeof( lAnswer ) ) && ( lAnswer.mProtocolVersion == LT_ETHERNET_IDENTIFY_PROT_VERSION ) &&
        ( lAnswer.mHeader.mSrvProtVersion == LT_ETHERNET_IDENTIFY_PROT_VERSION ) && ( lAnswer.mHeader.mAnswerCode == LT_ETHERNET_ANSWER_OK ) &&
        ( lAnswer.mHeader.mRequestCode == LT_COMM_IDT_REQUEST_IDENTIFY ) && ( lAnswer.mHeader.mAnswerSize == lSize ) && ( lPort >= IDT_PORT ) &&
        ( lPort < IDT_PORT + MAX_PORT_OFFSET ) && ( lAnswer.mSerialNumber[LT_COMM_SERIAL_NUMBER_LENGTH - 1] == 0 ) &&
        ( lAnswer.mStateMessage[LT_COMM_IDT_STATE_MESSAGE_LENGTH - 1] == 0 ) )
    {
        GetProperties()->GetIntegerProperty( LdPropertyIds::ID_FPGA_VERSION )->SetCount( 2 );
        GetProperties()->GetIntegerProperty( LdPropertyIds::ID_FPGA_VERSION )->ForceValue( 0, lAnswer.mFirmwareVersion & 0xFF );
        GetProperties()->GetIntegerProperty( LdPropertyIds::ID_FPGA_VERSION )->ForceValue( 1, ( lAnswer.mFirmwareVersion >> 8 ) & 0xFF );
        GetProperties()->GetIntegerProperty( LdPropertyIds::ID_FIRMWARE_VERSION_INT )->ForceValue( 0, lAnswer.mSoftwareVersion );
        GetProperties()->GetTextProperty( LdPropertyIds::ID_SOFTWARE_PART_NUMBER )->ForceValue( 0, lAnswer.mSoftwarePartNumber );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorDTec::ReinitIpConfig( const std::string &aSerialNumber, uint8_t aMode, uint8_t aStorage, uint8_t aPhyMode, const std::string &aIp, const
/// std::string &aSubnet, const std::string &aGateway )
///
/// \brief  Reinitialize ip configuration of the sensor. Usefull when a sensor is "lost" i.e. misconfigured on a network without router
///
/// \exception  std::logic_error                Raised when a logic error condition occurs.
/// \exception  std::overflow_error             Raised when an overflow error condition occurs.
/// \exception  LeddarException::LtComException Thrown when a Lt Com error condition occurs.
///
/// \param  aSerialNumber   Serial number of the device to reconfigure - if "Everyone" it sends the request to all sensors available on network.
/// \param  aMode           DHCP or static ( see LT_IPV4_IP_MODE_... in LtComDTec.h)
/// \param  aStorage        Permanent or temporary ( see \ref LtComDTec::eStorage)
/// \param  aPhyMode        Communication mode ( see \ref LtComDTec::ePHYMode)
/// \param  aIp             New ip address to set (network byte order)
/// \param  aSubnet         New subnet mask (network byte order)
/// \param  aGateway        New gateway (network byte order)
///
/// \author David Levy
///
/// \date   March 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorDTec::ReinitIpConfig( const std::string &aSerialNumber, uint8_t aMode, uint8_t aStorage, uint8_t aPhyMode, const std::string &aIp,
                                                 const std::string &aSubnet, const std::string &aGateway )
{
    if( aMode != LtComDTec::LT_IPV4_IP_MODE_DYNAMIC && aMode != LtComDTec::LT_IPV4_IP_MODE_STATIC )
    {
        throw std::logic_error( "Incorrect IP mode." );
    }

    if( aStorage != LtComDTec::LT_IPV4_IP_STORAGE_TEMPORARY && aStorage != LtComDTec::LT_IPV4_IP_STORAGE_PERMANENT )
    {
        throw std::logic_error( "Incorrect storage configuration." );
    }

    if( aPhyMode != LtComDTec::LT_IPV4_PHY_MODE_AUTO_NEGOTIATION && aPhyMode != LtComDTec::LT_IPV4_PHY_MODE_HALF_DUPLEX_10 &&
        aPhyMode != LtComDTec::LT_IPV4_PHY_MODE_FULL_DUPLEX_10 && aPhyMode != LtComDTec::LT_IPV4_PHY_MODE_HALF_DUPLEX_100 &&
        aPhyMode != LtComDTec::LT_IPV4_PHY_MODE_FULL_DUPLEX_100 )
    {
        throw std::logic_error( "Incorrect storage configuration." );
    }

    if( LtComDTec::LT_IPV4_IP_MODE_STATIC == aMode && ( "" == aIp || "" == aSubnet || "" == aGateway ) )
    {
        throw std::logic_error( "Need to set ip configuration when using static mode." );
    }

    std::vector<std::pair<SOCKET, unsigned long>> lInterfaces;

    try
    {
        // Request to send
        LtComDTec::LtIpv4RequestIpConfig lRequest;
        lRequest.mHeader.mSrvProtVersion   = LT_ETHERNET_IDENTIFY_PROT_VERSION;
        lRequest.mHeader.mRequestCode      = LtComDTec::LT_IPV4_IDT_REQUEST_IP_CONFIG;
        lRequest.mHeader.mRequestTotalSize = sizeof( LtComDTec::LtIpv4RequestIpConfig );

        lRequest.mMode    = aMode;
        lRequest.mStorage = aStorage;
        lRequest.mPhyMode = aPhyMode;

        if( aSerialNumber.length() > LT_COMM_SERIAL_NUMBER_LENGTH )
        {
            throw std::overflow_error( "Serial number is too long" );
        }
        else if( aSerialNumber.length() == 0 )
        {
            throw std::invalid_argument( "Please set a serial number or input \"Everyone\" wihtout the quotes if you want to reset all devices" );
        }

        aSerialNumber.copy( lRequest.mSerialNumber, aSerialNumber.size() );
        lRequest.mSerialNumber[aSerialNumber.length()] = 0; // To be sure its null terminated

        if( aMode == LtComDTec::LT_IPV4_IP_MODE_STATIC )
        {
            *(uint32_t *)lRequest.mIpAddress.mBytes = LeddarUtils::LtStringUtils::StringToIp4Addr( aIp );
            *(uint32_t *)lRequest.mIpGateway.mBytes = LeddarUtils::LtStringUtils::StringToIp4Addr( aGateway );
            *(uint32_t *)lRequest.mIpNetMask.mBytes = LeddarUtils::LtStringUtils::StringToIp4Addr( aSubnet );
        }

        // And send the request
        lInterfaces = LeddarConnection::LdEthernet::OpenScanRequestSockets();

        bool lAllBroadcastFail = true;

        for( size_t i = 0; i < lInterfaces.size(); ++i )
        {
            sockaddr_in addr     = {};
            addr.sin_family      = AF_INET;
            addr.sin_addr.s_addr = 0xFFFFFFFF;
            addr.sin_port        = htons( LtComDTec::DTEC_IDT_PORT );

            // Broadcast request
            if( sendto( lInterfaces[i].first, (const char *)&lRequest, sizeof( lRequest ), 0, (struct sockaddr *)&addr, sizeof( addr ) ) >= 0 )
            {
                lAllBroadcastFail = false;
            }
        }

        if( lAllBroadcastFail )
            throw LeddarException::LtComException( "Failed to broadcast request." );
    }
    catch( ... )
    {
        for( size_t i = 0; i < lInterfaces.size(); ++i )
        {
#ifdef _WIN32

            if( lInterfaces[i].first != INVALID_SOCKET )
#else
            if( lInterfaces[i].first >= 0 )
#endif
                LeddarConnection::LdEthernet::CloseSocket( lInterfaces[i].first );
        }

        throw;
    }

    for( size_t i = 0; i < lInterfaces.size(); ++i )
    {
#ifdef _WIN32

        if( lInterfaces[i].first != INVALID_SOCKET )
#else
        if( lInterfaces[i].first >= 0 )
#endif
            LeddarConnection::LdEthernet::CloseSocket( lInterfaces[i].first );
    }
}
#endif // defined(BUILD_DTEC) && defined(BUILD_ETHERNET)