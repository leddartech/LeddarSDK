////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorM16Can.cpp
///
/// \brief  Implements the LdSensorM16Can class
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdSensorM16Can.h"
#if defined(BUILD_M16) && defined(BUILD_CANBUS)

#include "LdPropertyIds.h"
#include "LdBitFieldProperty.h"
#include "LdBoolProperty.h"
#include "LdEnumProperty.h"
#include "LdFloatProperty.h"
#include "LdIntegerProperty.h"
#include "LdTextProperty.h"

#include "LdProtocolCan.h"
#include "comm/Canbus/LtComM16Canbus.h"
#include "comm/LtComLeddarTechPublic.h"

#include "LtStringUtils.h"
#include "LtTimeUtils.h"
#include "LtExceptions.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::LdSensorM16Can::LdSensorM16Can( LeddarConnection::LdConnection *aConnection )
///
/// \brief  Constructor
///
/// \param [in,out] aConnection If non-null, the connection.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorM16Can::LdSensorM16Can( LeddarConnection::LdConnection *aConnection ) : LdSensor( aConnection ),
    mLastTimestamp( 0 )
{
    mProtocol = dynamic_cast<LeddarConnection::LdProtocolCan *>( aConnection );

    InitProperties();

    mEchoes.Init( 1, LtComCanBus::M16_AMPLITUDE_SCALE_STD, LtComCanBus::CAN_MAX_DETECTIONS );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Can::InitProperties()
///
/// \brief  Initializes the properties for this sensor
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorM16Can::InitProperties()
{
    using namespace LeddarCore;

    //Constants
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS, 0, 4, "Device option - Internal use" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_INT, 0, 2, "Firmware version" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, 0, 2, "FPGA version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, 0, LtComCanBus::M16_SERIAL_NBR_SIZE,
                              LeddarCore::LdTextProperty::TYPE_ASCII, "Serial number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_NAME, 0, LtComCanBus::M16_DEVICE_NAME_SIZE,
                              LeddarCore::LdTextProperty::TYPE_UTF16, "Device name" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO,  LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER, 0, LtComCanBus::M16_SW_PART_NBR_SIZE,
                              LeddarCore::LdTextProperty::TYPE_ASCII, "Software part number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO,  LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, 0, LtComCanBus::M16_HW_PART_NBR_SIZE,
                              LeddarCore::LdTextProperty::TYPE_ASCII, "Hardware part number" ) );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_CAN );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();

    // Set segments constants
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ForceValue( 0, 1 );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->ForceValue( 0, 16 );

    //Configuration
    // Accumulation, oversampling and base point count are not available on IS16
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_ACCUMULATION_EXP, 0, 1, "Accumulation exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_OVERSAMPLING_EXP, 0, 1, "Oversampling exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_BASE_POINT_COUNT, 0, 1, "Number of base samples" ) );

    //IS16 only
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_REFRESH_RATE, 0, 2, true,
                              "Target refresh rate. Formula is 12800/2^n" ) );
    LdEnumProperty *lRefreshRate = GetProperties()->GetEnumProperty( LdPropertyIds::ID_REFRESH_RATE );
    lRefreshRate->AddEnumPair( 8, "50 Hz" );
    lRefreshRate->AddEnumPair( 9, "25 Hz" );
    lRefreshRate->AddEnumPair( 10, "12.5 Hz" );
    lRefreshRate->AddEnumPair( 11, "6.25 Hz" );
    lRefreshRate->AddEnumPair( 12, "3.125 Hz" );
    lRefreshRate->AddEnumPair( 13, "1.5625 Hz" );


    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_SENSIVITY_OLD, 0, 4, LtComCanBus::M16_THREHSOLD_SCALE, 2,
                              "Threshold" ) );
    mProperties->GetFloatProperty( LdPropertyIds::ID_SENSIVITY_OLD )->SetLimits( -5, 100 );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_LED_INTENSITY, LtComCanBus::M16_ID_LED_POWER, 1, true,
                              "Led/laser power %" ) );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 100, "100" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 90, "90" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 80, "80" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 65, "65" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 50, "50" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 35, "35" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 20, "20" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 10, "10" );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_ACQ_OPTIONS, LtComCanBus::M16_ID_ACQ_OPTIONS, 2,
                              "Acquisition options" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_CHANGE_DELAY, LtComCanBus::M16_ID_AUTO_ACQ_DELAY, 2,
                              "Auto led delay (in frame)" ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_CHANGE_DELAY )->SetLimits( 1, 8192 );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_PRECISION, LtComCanBus::M16_ID_SMOOTHING, 1, "Smoothing",
                              true ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_PRECISION )->SetLimits( -17, 16 );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES, LtComCanBus::M16_ID_DISTANCE_UNITS,
                              2, true, "Distance units" ) );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->AddEnumPair( 1, "m" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->AddEnumPair( 10, "dm" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->AddEnumPair( 100, "cm" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->AddEnumPair( 1000, "mm" );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_SEGMENT_ENABLE_COM,
                              LtComCanBus::M16_ID_SEGMENT_ENABLE_COM, 2, "Segment enable (communication)" ) );
    GetProperties()->GetBitProperty( LdPropertyIds::ID_SEGMENT_ENABLE_COM )->SetLimit( ( 1 << GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->Value() ) - 1 );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE, 0, 2, true,
                              "Baud rate (in kbps)" ) );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->AddEnumPair( 0, "1000 kbps" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->AddEnumPair( 1, "500 kbps" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->AddEnumPair( 2, "250 kbps" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->AddEnumPair( 3, "125 kbps" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->AddEnumPair( 4, "100 kbps" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->AddEnumPair( 5, "50 kbps" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->AddEnumPair( 6, "20 kbps" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->AddEnumPair( 7, "10 kbps" );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT, 0,
                              "Frame format - false = standard" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID, 0, 4, "Tx base id" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID, 0, 4, "Rx base id" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS, 0, 1, "Operation mode" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES, 0, 1, "Maximum echoes" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY, 0, 2,
                              "Inter-message delay" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY, 0, 2,
                              "Inter-cycle delay" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_SEGMENT_ENABLE, LtComCanBus::M16_ID_SEGMENT_ENABLE, 2,
                              "Enable / disable selected channels pair on the device (enable = 0)" ) );
    GetProperties()->GetBitProperty( LdPropertyIds::ID_SEGMENT_ENABLE )->SetLimit( ( 1 << GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->Value() ) - 1 );

    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_SYSTEM_TEMP, 0, 4, 0, 2, "System Temperature" ) );
    GetResultStates()->Init( LtComCanBus::M16_TEMPERATURE_SCALE, 0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Can::GetConfig( void )
///
/// \brief  Gets the configuration from the sensor
///
/// \exception  std::runtime_error  Raised when we received an erroneous answer.
/// \exception  std::logic_error    Raised when we have an unhandled case.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorM16Can::GetConfig( void )
{
    //Cannot use a standard function with the device id (for all ids) because sometimes an id is used for several properties
    // It's written as a switch, and default throw an exception in order to be sure that new property are correctly handled
    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        switch( ( *lIter )->GetId() )
        {
            case LeddarCore::LdPropertyIds::ID_LED_INTENSITY:
            case LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS:
            case LeddarCore::LdPropertyIds::ID_CHANGE_DELAY:
            case LeddarCore::LdPropertyIds::ID_PRECISION:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES:
            case LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE_COM:
            case LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE:
            {
                LtComCanBus::sCanData lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_HOLDING_DATA, ( *lIter )->GetDeviceId() );

                ( *lIter )->SetCount( 1 );

                if( ( *lIter )->UnitSize() == 1 )
                {
                    ( *lIter )->SetRawValue( 0, lConfigData.mFrame.Cmd.mArg[0] );
                }
                else if( ( *lIter )->UnitSize() == 2 )
                {
                    ( *lIter )->SetRawValue( 0, *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] ) );
                }
                else if( ( *lIter )->UnitSize() == 4 )
                {
                    ( *lIter )->SetRawValue( 0, *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[0] ) );
                }
                else
                {
                    throw std::logic_error( "Unhandled unit size" );
                }

                break;
            }

            case LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP:
            case LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP:
            case LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT:
            case LeddarCore::LdPropertyIds::ID_REFRESH_RATE:
            case LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY:
            case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY:
                //Specific treatment below, do nothing here
                break;

            default:
                throw std::logic_error( "Unhandled property" );
        }
    }

    // Acquisition config
    if( mProtocol->GetDeviceType() == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 ||
            mProtocol->GetDeviceType() == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER )
    {
        LtComCanBus::sCanData lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_HOLDING_DATA, LtComCanBus::M16_ID_ACQ_CONFIG );
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[0] );
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[1] );
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[2] );
    }
    else if( mProtocol->GetDeviceType() == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16 )
    {
        LtComCanBus::sCanData lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_HOLDING_DATA, LtComCanBus::M16_ID_REFRESH_RATE );
        mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[0] );
    }
    else
    {
        throw std::runtime_error( "Please fetch constants before getting configuration." );
    }

    // Detection threshold
    LtComCanBus::sCanData lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_HOLDING_DATA, LtComCanBus::M16_ID_THRESHOLD );
    mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD )->SetRawValue( 0, *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //Can port configuration 1
    lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_HOLDING_DATA, LtComCanBus::M16_ID_CAN_PORT_CONF1 );
    mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[0] );
    mProperties->GetBoolProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->SetValue( 0, ( lConfigData.mFrame.Cmd.mArg[1] != 0 ) );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->SetValue( 0, *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //Can port configuration 2
    lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_HOLDING_DATA, LtComCanBus::M16_ID_CAN_PORT_CONF2 );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->SetValue( 0, *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //Can port configuration 3
    lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_HOLDING_DATA, LtComCanBus::M16_ID_CAN_PORT_CONF3 );
    mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[0] );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[1] );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->SetValue( 0, *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->SetValue( 0, *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[4] ) );

    if( mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS )->BitState( 0, 3 ) )
    {
        GetResultEchoes()->SetAmplitudeScale( LtComCanBus::M16_AMPLITUDE_SCALE_FLAG );
    }
    else
    {
        GetResultEchoes()->SetAmplitudeScale( LtComCanBus::M16_AMPLITUDE_SCALE_STD );
    }

    //And make them all not modified
    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Can::SetConfig( void )
///
/// \brief  Sets the configuration on the sensor
///
/// \exception  std::runtime_error  Raised when we received an erroneous answer. (from SetValue)
/// \exception  std::logic_error    Raised when we have an unhandled case.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorM16Can::SetConfig( void )
{
    //Cannot use a standard function with the device id (for all ids) because sometimes an id is used for several properties
    // It's written as a switch, and default throw an exception in order to be sure that new property are correctly handled
    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            switch( ( *lIter )->GetId() )
            {
                case LeddarCore::LdPropertyIds::ID_LED_INTENSITY:
                case LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS:
                case LeddarCore::LdPropertyIds::ID_CHANGE_DELAY:
                case LeddarCore::LdPropertyIds::ID_PRECISION:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES:
                case LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE_COM:
                case LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE:
                {
                    LtComCanBus::sCanData lConfigData = {};
                    lConfigData.mFrame.Cmd.mCmd = LtComCanBus::M16_CMD_SET_HOLDING_DATA;
                    lConfigData.mFrame.Cmd.mSubCmd = ( *lIter )->GetDeviceId();

                    if( ( *lIter )->UnitSize() == 1 )
                    {
                        lConfigData.mFrame.Cmd.mArg[0] = static_cast<uint8_t>( ( *lIter )->RawValue( 0 ) );
                    }
                    else if( ( *lIter )->UnitSize() == 2 )
                    {
                        *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] ) = static_cast<uint16_t>( ( *lIter )->RawValue( 0 ) );
                    }
                    else if( ( *lIter )->UnitSize() == 4 )
                    {
                        *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[0] ) = static_cast<uint32_t>( ( *lIter )->RawValue( 0 ) );
                    }
                    else
                    {
                        throw std::logic_error( "Unhandled unit size" );
                    }

                    mProtocol->SetValue( lConfigData );
                    ( *lIter )->SetClean();
                    break;
                }

                case LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP:
                case LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP:
                case LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT:
                case LeddarCore::LdPropertyIds::ID_REFRESH_RATE:
                case LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY:
                case LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY:
                    //Specific treatment below, do nothing here
                    break;

                default:
                    throw std::logic_error( "Unhandled property" );
            }
        }
    }

    // Acquisition config
    if( mProtocol->GetDeviceType() == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 ||
            mProtocol->GetDeviceType() == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER )
    {
        if( mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->Modified() ||
                mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->Modified() ||
                mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->Modified() )
        {
            LtComCanBus::sCanData lConfigData = {};
            lConfigData.mFrame.Cmd.mCmd = LtComCanBus::M16_CMD_SET_HOLDING_DATA;
            lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::M16_ID_ACQ_CONFIG;
            lConfigData.mFrame.Cmd.mArg[0] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->ValueT<uint8_t>( 0 );
            lConfigData.mFrame.Cmd.mArg[1] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->ValueT<uint8_t>( 0 );
            lConfigData.mFrame.Cmd.mArg[2] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->ValueT<uint8_t>( 0 );

            mProtocol->SetValue( lConfigData );
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetClean();
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetClean();
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetClean();
        }
    }
    else if( mProtocol->GetDeviceType() == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16 )
    {
        if( mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE )->Modified() )
        {
            LtComCanBus::sCanData lConfigData = {};
            lConfigData.mFrame.Cmd.mCmd = LtComCanBus::M16_CMD_SET_HOLDING_DATA;
            lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::M16_ID_REFRESH_RATE;
            lConfigData.mFrame.Cmd.mArg[0] = mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE )->ValueT<uint8_t>( 0 );
            mProtocol->SetValue( lConfigData );
            mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE )->SetClean();
        }
    }
    else
    {
        throw std::runtime_error( "Please fetch constants before setting configuration." );
    }

    // Detection threshold
    if( mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::M16_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::M16_ID_THRESHOLD;
        *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) = mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD )->RawValue( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD )->SetClean();
    }

    //Can port configuration 1
    if( mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->Modified() ||
            mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->Modified() ||
            mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::M16_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::M16_ID_CAN_PORT_CONF1;
        lConfigData.mFrame.Cmd.mArg[0] = mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->Value( 0 );
        lConfigData.mFrame.Cmd.mArg[1] = mProperties->GetBoolProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->Value( 0 );
        *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->SetClean();
        mProperties->GetBoolProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->SetClean();
    }

    //Can port configuration 2
    if( mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::M16_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::M16_ID_CAN_PORT_CONF2;
        *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->ValueT<uint32_t>( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->SetClean();
    }

    //Can port configuration 3
    if( mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS )->Modified() ||
            mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->Modified() ||
            mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->Modified() ||
            mProperties->GetProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::M16_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::M16_ID_CAN_PORT_CONF3;
        lConfigData.mFrame.Cmd.mArg[0] = mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS )->Value( 0 );
        lConfigData.mFrame.Cmd.mArg[1] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->ValueT<uint8_t>( 0 );
        *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->ValueT<uint16_t>( 0 );
        *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[4] ) = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->ValueT<uint16_t>( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->SetClean();
    }

    LeddarUtils::LtTimeUtils::Wait( 100 ); //To be sure the sensor has taken the change into account

    GetConfig(); //We call getconfig to re-fetch config from sensor in case there has been a problem (usb cable is still plugged in for example)
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Can::GetConstants( void )
///
/// \brief  Gets the constants data from the sensor
///
/// \exception  std::runtime_error  Raised when we received an erroneous answer
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorM16Can::GetConstants( void )
{
    //Device type
    LtComCanBus::sCanData lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_INPUT_DATA, LtComCanBus::M16_ID_DEVICE_ID );
    uint16_t lDeviceId = *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] );
    uint32_t lOptions = *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] );

    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->ForceValue( 0, lDeviceId );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_OPTIONS )->ForceValue( 0, lOptions );

    mProtocol->SetDeviceType( lDeviceId );

    //Firmware version
    lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_INPUT_DATA, LtComCanBus::M16_ID_FIRMWARE_VERSION );
    uint16_t lFirmwareVersion = *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FIRMWARE_VERSION_INT )->ForceValue( 0, lFirmwareVersion );

    //FPGA version
    lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_INPUT_DATA, LtComCanBus::M16_ID_FPGA_VERSION );
    uint16_t lFPGAVersion = *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FIRMWARE_VERSION_INT )->ForceValue( 0, lFPGAVersion );

    if( lConfigData.mFrame.Cmd.mArg[4] != 0xFF )
    {
        throw std::runtime_error( "Defective sensor." );
    }

    //Serial number
    char lSerialNbr[LtComCanBus::M16_SERIAL_NBR_SIZE + 1] = {0}; //+1 to be sure it ends with a 0
    uint8_t j = 0;

    for( uint8_t i = 0; i < LtComCanBus::M16_SERIAL_NBR_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_INPUT_DATA, LtComCanBus::M16_ID_SERIAL_NUMBER + j++ );
        uint8_t lSize = LtComCanBus::M16_SERIAL_NBR_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lSerialNbr[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SERIAL_NUMBER )->ForceValue( 0, std::string( lSerialNbr ) );

    //Device name
    uint8_t lDeviceName[LtComCanBus::M16_DEVICE_NAME_SIZE + 1] = {0};
    j = 0;

    for( uint8_t i = 0; i < LtComCanBus::M16_DEVICE_NAME_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_INPUT_DATA, LtComCanBus::M16_ID_DEVICE_NAME + j++ );
        uint8_t lSize = LtComCanBus::M16_DEVICE_NAME_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lDeviceName[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_DEVICE_NAME )->ForceRawStorage( lDeviceName, 1, LtComCanBus::M16_DEVICE_NAME_SIZE );

    //Software part number
    char lSWPartNumber[LtComCanBus::M16_SW_PART_NBR_SIZE + 1] = {0};
    j = 0;

    for( uint8_t i = 0; i < LtComCanBus::M16_SW_PART_NBR_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_INPUT_DATA, LtComCanBus::M16_ID_SW_PART_NBR + j++ );
        uint8_t lSize = LtComCanBus::M16_SW_PART_NBR_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lSWPartNumber[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SOFTWARE_PART_NUMBER )->ForceValue( 0, std::string( lSWPartNumber ) );

    //Hard part number
    char lHWPartNumber[LtComCanBus::M16_HW_PART_NBR_SIZE + 1] = { 0 };
    j = 0;

    for( uint8_t i = 0; i < LtComCanBus::M16_HW_PART_NBR_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_INPUT_DATA, LtComCanBus::M16_ID_HW_PART_NBR + j++ );
        uint8_t lSize = LtComCanBus::M16_HW_PART_NBR_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lHWPartNumber[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_PART_NUMBER )->ForceValue( 0, std::string( lHWPartNumber ) );

    //And make them all not modified
    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_INFO );

    for( std::vector<LeddarCore::LdProperty *>::iterator lIter = lProperties.begin(); lIter != lProperties.end(); ++lIter )
    {
        if( ( *lIter )->Modified() )
        {
            ( *lIter )->SetClean();
        }
    }

    GetConnection()->SetDeviceType( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->ValueT<uint16_t>() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorM16Can::GetData( void )
///
/// \brief  Gets the latest data from the sensor
///
/// \return True if new data, else false
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarDevice::LdSensorM16Can::GetData( void )
{
    bool lRet = GetEchoes();

    if( !mProtocol->IsStreaming() )
        GetStates();

    return lRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorM16Can::GetEchoes( void )
///
/// \brief  Gets the echoes
///
/// \exception  std::runtime_error  Raised when a we couldn't fetch all echoes or we received an unexpected id
///
/// \return True if it succeeds, false if it fails.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarDevice::LdSensorM16Can::GetEchoes( void )
{
    LtComCanBus::sCanData lNextData;

    if( mProtocol->IsStreaming() )
    {
        mProtocol->ReadDetectionAnswer();
        lNextData = mProtocol->GetNextDetectionData();

        if( lNextData.mId == 0 )
        {
            return false;
        }
    }
    else
    {
        uint8_t lFlag = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS )->Value( 0 ) & 0x9; //Only bit 0 and 3 should be re-sent here

        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::M16_CMD_SEND_DETECT_ONCE;
        lConfigData.mFrame.Cmd.mSubCmd = lFlag;

        if( !mProtocol->SendRequestAndWaitForAnswer( lConfigData ) )
        {
            LeddarException::LtTimeoutException( "Timeout when fetching echoes" );
        }

        lNextData = mProtocol->GetNextDetectionData();
    }

    if( lNextData.mId != GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 ) + 1 )
    {
        //Peut etre verifier si id = 0x750 || 0x752, cest un vieil echo, on le dump et on recommence?
        throw std::runtime_error( "Unexpected data, id = " + LeddarUtils::LtStringUtils::IntToString( lNextData.mId, 16 ) );
    }

    uint8_t lEchoCount = lNextData.mFrame.Cmd.mCmd;
    uint8_t lCrurrentLedPower = lNextData.mFrame.Cmd.mArg[0];
    uint32_t lTimestamp = *reinterpret_cast<uint32_t *>( &lNextData.mFrame.Cmd.mArg[2] );

    std::vector<LeddarConnection::LdEcho> *lEchoes = mEchoes.GetEchoes( LeddarConnection::B_SET );
    mEchoes.Lock( LeddarConnection::B_SET );
    mEchoes.SetEchoCount( lEchoCount );

    uint16_t lTimeout = 500;

    uint8_t i = 0;

    for( i = 0; i < lEchoCount; )
    {
        while( lTimeout > 0 )
        {
            if( !mProtocol->ReadDetectionAnswer() )
            {
                --lTimeout;
                LeddarUtils::LtTimeUtils::Wait( 1 );
                continue;
            }

            break;
        }

        if( lTimeout == 0 )
        {
            LeddarException::LtTimeoutException( "Timeout when fetching echoes" );
        }

        lNextData = mProtocol->GetNextDetectionData();

        if( lNextData.mId != GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 ) &&
                ( lNextData.mId < GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 ) + 2 ||
                  lNextData.mId > GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 ) + 2 + LtComCanBus::CAN_MAX_DETECTIONS ) )
        {
            throw std::runtime_error( "Unexpected data, id = 0x" + LeddarUtils::LtStringUtils::IntToString( lNextData.mId, 16 ) );
        }

        LtComCanBus::sM16CANEcho *lEcho = reinterpret_cast<LtComCanBus::sM16CANEcho *>( lNextData.mFrame.mRawData );

        if( GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS )->BitState( 0, 3 ) ) //Detection with flag
        {
            ( *lEchoes )[ i ].mAmplitude = lEcho->mDetectionFlag.mAmplitude;
            ( *lEchoes )[ i ].mDistance = lEcho->mDetectionFlag.mDistance;
            ( *lEchoes )[ i ].mChannelIndex = lEcho->mDetectionFlag.mSegment;
            ( *lEchoes )[ i ].mFlag = lEcho->mDetectionFlag.mFlag;
            ++i;
        }
        else
        {
            ( *lEchoes )[ i ].mAmplitude = lEcho->mDetectionStd.mAmplitude;
            ( *lEchoes )[ i ].mDistance = lEcho->mDetectionStd.mDistance;
            ( *lEchoes )[ i ].mChannelIndex = lEcho->mDetectionStd.mSegment;

            if( ++i < lEchoCount )
            {
                ( *lEchoes )[ i ].mAmplitude = lEcho->mDetectionStd.mAmplitude2;
                ( *lEchoes )[ i ].mDistance = lEcho->mDetectionStd.mDistance2;
                ( *lEchoes )[ i ].mChannelIndex = lEcho->mDetectionStd.mSegment2;
                ++i;
            }
        }
    }

    if( i != lEchoCount )
    {
        mEchoes.UnLock( LeddarConnection::B_SET );
        throw std::runtime_error( "Missing echoes" );
    }

    mEchoes.SetCurrentLedPower( lCrurrentLedPower );
    mEchoes.SetTimestamp( lTimestamp );
    mEchoes.UnLock( LeddarConnection::B_SET );

    if( lTimestamp != mLastTimestamp ) // Trigg callbacks only if its a new frame
    {
        mEchoes.Swap();
        mLastTimestamp = lTimestamp;
        mEchoes.UpdateFinished();
    }
    else
    {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Can::GetStates(void)
///
/// \brief  Gets the latest states from the sensor
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorM16Can::GetStates( void )
{
    //Only the temperature is available at the moment (not even a timestamp - we can get it from echoes)
    LtComCanBus::sCanData lConfigData = mProtocol->GetValue( LtComCanBus::M16_CMD_GET_INPUT_DATA, LtComCanBus::M16_ID_TEMP );

    uint16_t lRawTemp = *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] );
    GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )->ForceRawValue( 0, lRawTemp );

    GetResultStates()->SetTimestamp( mEchoes.GetTimestamp( LeddarConnection::B_GET ) ); //we use latest echo timestamp, better than nothing
    GetResultStates()->UpdateFinished();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Can::EnableStreamingDetections( bool aEnable )
///
/// \brief  Enables / disabled the streaming of the detections
///
/// \param  aEnable True to enable, false to disable.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorM16Can::EnableStreamingDetections( bool aEnable )
{
    uint8_t lFlag = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_OPTIONS )->Value( 0 ) & 0x9; //Only bit 0 and 3 should be re-sent here
    mProtocol->EnableStreamingDetections( aEnable, lFlag );
}

#endif
