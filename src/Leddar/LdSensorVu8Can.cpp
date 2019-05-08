////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorVu8Can.cpp
///
/// \brief  Implements the LdSensorVu8Can class
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdSensorVu8Can.h"
#if defined(BUILD_VU) && defined(BUILD_CANBUS)

#include "LdProtocolCan.h"
#include "LdPropertyIds.h"
#include "LtStringUtils.h"
#include "comm/Canbus/LtComVuCanbus.h"
#include "LtExceptions.h"
#include "LtTimeUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDevice::LdSensorVu8Can::LdSensorVu8Can( LeddarConnection::LdConnection *aConnection )
///
/// \brief  Constructor
///
/// \param [in,out] aConnection If non-null, the connection.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDevice::LdSensorVu8Can::LdSensorVu8Can( LeddarConnection::LdConnection *aConnection ) : LdSensor( aConnection ),
    mLastTimestamp( 0 )
{
    mProtocol = dynamic_cast<LeddarConnection::LdProtocolCan *>( aConnection );
    InitProperties();
    mEchoes.Init( 1, LtComCanBus::VU_AMPLITUDE_SCALE, LtComCanBus::CAN_MAX_DETECTIONS );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorVu8Can::InitProperties()
///
/// \brief  Initializes the properties for this sensor
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorVu8Can::InitProperties()
{
    using namespace LeddarCore;

    //Constants
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_TYPE, 0, 2, "Device type" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_RSEGMENT, 0, 2, "Number of reference segment" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS, 0, 4, "Device option - Internal use" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_STR, 0, LtComCanBus::VU_FIRMWARE_VERSION_SIZE, LdTextProperty::TYPE_ASCII,
                              "Firmware version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_BOOTLOADER_VERSION, 0, LtComCanBus::VU_FIRMWARE_VERSION_SIZE, LdTextProperty::TYPE_ASCII,
                              "Boot loader version" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, 0, 2, "FPGA version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, 0, LtComCanBus::VU_SERIAL_NBR_SIZE,
                              LeddarCore::LdTextProperty::TYPE_ASCII, "Serial number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_NAME, 0, LtComCanBus::VU_DEVICE_NAME_SIZE,
                              LeddarCore::LdTextProperty::TYPE_ASCII, "Device name" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO,  LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER, 0, LtComCanBus::VU_SW_PART_NBR_SIZE,
                              LeddarCore::LdTextProperty::TYPE_ASCII, "Software part number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO,  LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, 0, LtComCanBus::VU_HW_PART_NBR_SIZE,
                              LeddarCore::LdTextProperty::TYPE_ASCII, "Hardware part number" ) );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_CAN );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();

    //Configuration
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_ACCUMULATION_EXP, 0, 1, "Accumulation exponent" ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_ACCUMULATION_EXP )->SetLimitsUnsigned( LtComCanBus::VU_MIN_ACC, LtComCanBus::VU_MAX_ACC );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_OVERSAMPLING_EXP, 0, 1, "Oversampling exponent" ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_OVERSAMPLING_EXP )->SetLimitsUnsigned( LtComCanBus::VU_MIN_OVERS, LtComCanBus::VU_MAX_OVERS );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_BASE_POINT_COUNT, 0, 1, "Number of base samples" ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_BASE_POINT_COUNT )->SetLimitsUnsigned( LtComCanBus::VU_MIN_BASE_POINT_COUNT, LtComCanBus::VU_MAX_BASE_POINT_COUNT );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_PRECISION, 0, 1, "Smoothing",  true ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_PRECISION )->SetLimits( LtComCanBus::VU_MIN_SMOOTHING, LtComCanBus::VU_MAX_SMOOTHING );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_SENSIVITY, 0, 4, LtComCanBus::VU_THREHSOLD_SCALE, 2,
                              "Threshold" ) );
    mProperties->GetFloatProperty( LdPropertyIds::ID_SENSIVITY )->SetLimits( -5, 100 );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_LED_INTENSITY, 0, 1, true, "Led/laser power %" ) );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 6, "6" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 28, "28" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 53, "53" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 81, "81" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY )->AddEnumPair( 100, "100" );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_AUTO_ECHO_AVG, 0, 1,
                              "Change Delay (echoes)" ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_LED_AUTO_ECHO_AVG )->SetLimitsUnsigned( LtComCanBus::VU_MIN_AUTOECHO_AVG, LtComCanBus::VU_MAX_AUTOECHO_AVG );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_AUTO_FRAME_AVG, 0, 2,
                              "Change Delay (Frame)" ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_LED_AUTO_FRAME_AVG )->SetLimitsUnsigned( LtComCanBus::VU_MIN_AUTOFRAME_AVG, LtComCanBus::VU_MAX_AUTOFRAME_AVG );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES, 0,
                              2, true, "Distance units" ) );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->AddEnumPair( 1, "m" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->AddEnumPair( 10, "dm" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->AddEnumPair( 100, "cm" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->AddEnumPair( 1000, "mm" );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_ACQ_OPTIONS, LtComCanBus::VU_ID_ACQ_OPTIONS, 2,
                              "Acquisition options" ) ); // see eAcquisitionOptionsVu


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
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT, 0, 1, true,
                              "CAN Port Frame Format" ) );
    LeddarCore::LdEnumProperty *lCanFrameFormat = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT );
    lCanFrameFormat->AddEnumPair( 0, "Standard 11 bits" );
    lCanFrameFormat->AddEnumPair( 1, "Extended 29 bits" );

    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID, 0, 4, "Tx base id" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID, 0, 4, "Rx base id" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES, 0, 1, "Maximum echoes" ) );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->SetLimits( 1, LtComCanBus::LEDDARVU8_MAX_CAN_DETECTIONS );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY, 0, 2,
                              "Inter-message delay" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY, 0, 2,
                              "Inter-cycle delay" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_SEGMENT_ENABLE, LtComCanBus::VU_ID_SEGMENT_ENABLE, 4,
                              "Segment enable (sensor) - Enable on 0" ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorVu8Can::GetConstants( void )
///
/// \brief  Gets the constants from the sensor
///
/// \exception  std::runtime_error  Raised when the sensors does not answer, and we receive and unexpected answer (from GetValue)
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorVu8Can::GetConstants( void )
{
    //Number of segment
    LtComCanBus::sCanData lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_SEGMENT_NUMBER );
    uint16_t lNbr = *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT )->ForceValue( 0, lNbr );

    //Device type
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_DEVICE_ID );
    uint16_t lDeviceId = *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] );
    uint32_t lOptions = *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] );

    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->ForceValue( 0, lDeviceId );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_OPTIONS )->ForceValue( 0, lOptions );

    //Firmware version
    uint8_t lFirmwareVersion[LtComCanBus::VU_FIRMWARE_VERSION_SIZE] = {0};
    uint8_t j = 0;

    for( uint8_t i = 0; i < LtComCanBus::VU_FIRMWARE_VERSION_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_FIRMWARE_VERSION + j++ );
        uint8_t lSize = LtComCanBus::VU_SERIAL_NBR_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lFirmwareVersion[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_FIRMWARE_VERSION_STR )->ForceValue( 0,
            LeddarUtils::LtStringUtils::IntToString( *reinterpret_cast<uint16_t *>( &lFirmwareVersion[0] ), 10, true ) +
            LeddarUtils::LtStringUtils::IntToString( *reinterpret_cast<uint16_t *>( &lFirmwareVersion[2] ), 10, true ) +
            LeddarUtils::LtStringUtils::IntToString( *reinterpret_cast<uint16_t *>( &lFirmwareVersion[4] ), 10, true ) +
            LeddarUtils::LtStringUtils::IntToString( *reinterpret_cast<uint16_t *>( &lFirmwareVersion[6] ), 10, true ) );

    //Boot loader version
    uint8_t lBootLoader[LtComCanBus::VU_FIRMWARE_VERSION_SIZE] = {0};
    j = 0;

    for( uint8_t i = 0; i < LtComCanBus::VU_FIRMWARE_VERSION_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_BOOTLOADER_VERSION + j++ );
        uint8_t lSize = LtComCanBus::VU_SERIAL_NBR_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lBootLoader[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_BOOTLOADER_VERSION )->ForceValue( 0,
            LeddarUtils::LtStringUtils::IntToString( *reinterpret_cast<uint16_t *>( &lBootLoader[0] ), 10, true ) +
            LeddarUtils::LtStringUtils::IntToString( *reinterpret_cast<uint16_t *>( &lBootLoader[2] ), 10, true ) +
            LeddarUtils::LtStringUtils::IntToString( *reinterpret_cast<uint16_t *>( &lBootLoader[4] ), 10, true ) +
            LeddarUtils::LtStringUtils::IntToString( *reinterpret_cast<uint16_t *>( &lBootLoader[6] ), 10, true ) );

    //FPGA version
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_FPGA_VERSION );
    uint16_t lFPGAVersion = *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FPGA_VERSION )->ForceValue( 0, lFPGAVersion );

    if( lConfigData.mFrame.Cmd.mArg[4] != 0xFF )
    {
        throw std::runtime_error( "Defective sensor." );
    }

    //Serial number
    char lSerialNbr[LtComCanBus::VU_SERIAL_NBR_SIZE + 1] = {0}; //+1 to be sure it ends with a 0
    j = 0;

    for( uint8_t i = 0; i < LtComCanBus::VU_SERIAL_NBR_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_SERIAL_NUMBER + j++ );
        uint8_t lSize = LtComCanBus::VU_SERIAL_NBR_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lSerialNbr[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SERIAL_NUMBER )->ForceValue( 0, std::string( lSerialNbr ) );

    //Device name
    char lDeviceName[LtComCanBus::VU_DEVICE_NAME_SIZE + 1] = {0};
    j = 0;

    for( uint8_t i = 0; i < LtComCanBus::VU_DEVICE_NAME_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_DEVICE_NAME + j++ );
        uint8_t lSize = LtComCanBus::VU_DEVICE_NAME_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lDeviceName[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_DEVICE_NAME )->ForceValue( 0, std::string( lDeviceName ) );

    //Hard part number
    char lHWPartNumber[LtComCanBus::VU_HW_PART_NBR_SIZE + 1] = { 0 };
    j = 0;

    for( uint8_t i = 0; i < LtComCanBus::VU_HW_PART_NBR_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_HW_PART_NBR + j++ );
        uint8_t lSize = LtComCanBus::VU_HW_PART_NBR_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lHWPartNumber[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_PART_NUMBER )->ForceValue( 0, std::string( lHWPartNumber ) );

    //Software part number
    char lSWPartNumber[LtComCanBus::VU_SW_PART_NBR_SIZE + 1] = {0};
    j = 0;

    for( uint8_t i = 0; i < LtComCanBus::VU_SW_PART_NBR_SIZE; i += sizeof( lConfigData.mFrame.Cmd.mArg ) )
    {
        lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_INPUT_DATA, LtComCanBus::VU_ID_SW_PART_NBR + j++ );
        uint8_t lSize = LtComCanBus::VU_SW_PART_NBR_SIZE - i;

        if( lSize > sizeof( lConfigData.mFrame.Cmd.mArg ) )
            lSize = sizeof( lConfigData.mFrame.Cmd.mArg );

        std::copy( &lConfigData.mFrame.Cmd.mArg[0], &lConfigData.mFrame.Cmd.mArg[lSize], &lSWPartNumber[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SOFTWARE_PART_NUMBER )->ForceValue( 0, std::string( lSWPartNumber ) );

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
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->SetLimit( ( 1 << ( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT )->Value() + 1 ) ) -
            1 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorVu8Can::GetConfig( void )
///
/// \brief  Gets the configuration
///
/// \exception  std::runtime_error  Raised when the sensors does not answer, and we receive and unexpected answer (from GetValue)
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorVu8Can::GetConfig( void )
{
    //Cannot use a standard function with the device id because an id is used for several properties

    //Acquisition config
    LtComCanBus::sCanData lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_HOLDING_DATA, LtComCanBus::VU_ID_ACQ_CONFIG );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[0] );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[1] );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[2] );

    //Smoothing and detection threshold
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_HOLDING_DATA, LtComCanBus::VU_ID_SMOOTHING_THRESHOLD );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetValue( 0, reinterpret_cast<int8_t *>( lConfigData.mFrame.Cmd.mArg )[0] );
    mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY )->SetRawValue( 0, *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //Led management
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_HOLDING_DATA, LtComCanBus::VU_ID_LED_POWER );
    mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[0] );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_ECHO_AVG )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[1] );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG )->SetValue( 0, *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //Acquisition options
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_HOLDING_DATA, LtComCanBus::VU_ID_ACQ_OPTIONS );
    mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->SetValue( 0, *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] ) );
    mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS )->SetValue( 0, *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //CAN config 1
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_HOLDING_DATA, LtComCanBus::VU_ID_CAN_PORT_CONF1 );
    mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[0] );
    mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->SetValue( 0, ( lConfigData.mFrame.Cmd.mArg[1] != 0 ) );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->SetValue( 0, *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //CAN config 2
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_HOLDING_DATA, LtComCanBus::VU_ID_CAN_PORT_CONF2 );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->SetValue( 0, *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //CAN config 3
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_HOLDING_DATA, LtComCanBus::VU_ID_CAN_PORT_CONF3 );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->SetValue( 0, lConfigData.mFrame.Cmd.mArg[1] );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->SetValue( 0, *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->SetValue( 0, *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[4] ) );

    //Segment enable
    lConfigData = mProtocol->GetValue( LtComCanBus::VU_CMD_GET_HOLDING_DATA, LtComCanBus::VU_ID_SEGMENT_ENABLE );
    mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->SetValue( 0, *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) );

    //And make them all not modified
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
/// \fn void LeddarDevice::LdSensorVu8Can::SetConfig( void )
///
/// \brief  Sets the configuration on the sensor
///
/// \exception  std::runtime_error  Raised when we received an erroneous answer. (from SetValue)
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorVu8Can::SetConfig( void )
{
    //Acquisition config
    if( mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->Modified() ||
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->Modified() ||
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::VU_ID_ACQ_CONFIG;

        lConfigData.mFrame.Cmd.mArg[0] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->ValueT<uint8_t>( 0 );
        lConfigData.mFrame.Cmd.mArg[1] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->ValueT<uint8_t>( 0 );
        lConfigData.mFrame.Cmd.mArg[2] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->ValueT<uint8_t>( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetClean();

    }

    //Smoothing and detection threshold
    if( mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->Modified() ||
            mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::VU_ID_SMOOTHING_THRESHOLD;

        lConfigData.mFrame.Cmd.mArg[0] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->ValueT<int8_t>( 0 );
        *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) = mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY )->RawValue( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetClean();
        mProperties->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY )->SetClean();
    }

    //Led management
    if( mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->Modified() ||
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_ECHO_AVG )->Modified() ||
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::VU_ID_LED_POWER;

        lConfigData.mFrame.Cmd.mArg[0] = mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->Value( 0 );
        lConfigData.mFrame.Cmd.mArg[1] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_ECHO_AVG )->ValueT<uint8_t>( 0 );
        *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG )->ValueT<uint16_t>( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_ECHO_AVG )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG )->SetClean();
    }

    //Acquisition options
    if( mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->Modified() ||
            mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::VU_ID_ACQ_OPTIONS;

        *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[0] ) = mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->Value( 0 );
        *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) =  mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS )->Value( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->SetClean();
        mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS )->SetClean();
    }

    //CAN config 1
    if( mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->Modified() ||
            mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->Modified() ||
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::VU_ID_CAN_PORT_CONF1;

        lConfigData.mFrame.Cmd.mArg[0] = mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->Value( 0 );
        lConfigData.mFrame.Cmd.mArg[1] = mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->Value( 0 );
        *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) =  mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->SetClean();
        mProperties->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->SetClean();
    }

    //CAN config 2
    if( mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::VU_ID_CAN_PORT_CONF2;

        *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) =  mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->ValueT<uint32_t>( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->SetClean();
    }

    //CAN config 3
    if( mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->Modified() ||
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->Modified() ||
            mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::VU_ID_CAN_PORT_CONF3;

        lConfigData.mFrame.Cmd.mArg[1] = mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->ValueT<uint8_t>( 0 );
        *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) =  mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->ValueT<uint16_t>( 0 );
        *reinterpret_cast<uint16_t *>( &lConfigData.mFrame.Cmd.mArg[4] ) =  mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->ValueT<uint16_t>( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->SetClean();
        mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->SetClean();
    }

    //Segment enable
    if( mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->Modified() )
    {
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_HOLDING_DATA;
        lConfigData.mFrame.Cmd.mSubCmd = LtComCanBus::VU_ID_SEGMENT_ENABLE;

        *reinterpret_cast<uint32_t *>( &lConfigData.mFrame.Cmd.mArg[2] ) =  mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->Value( 0 );

        mProtocol->SetValue( lConfigData );
        mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->SetClean();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorVu8Can::GetData( void )
///
/// \brief  Gets the latest data from the sensor
///
/// \return True if new data, else false
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarDevice::LdSensorVu8Can::GetData( void )
{
    return GetEchoes();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarDevice::LdSensorVu8Can::GetEchoes( void )
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
bool LeddarDevice::LdSensorVu8Can::GetEchoes( void )
{
    LtComCanBus::sCanData lNextData = {};
    uint16_t lTimeout = 500;

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
        LtComCanBus::sCanData lConfigData = {};
        lConfigData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SEND_DETECT_ONCE;
        lConfigData.mFrame.Cmd.mSubCmd = 0; //0 = single message, 1 = multi message mode

        if( !mProtocol->SendRequestAndWaitForAnswer( lConfigData ) )
        {
            LeddarException::LtTimeoutException( "Timeout when fetching echoes" );
        }

        lNextData = mProtocol->GetNextConfigData();

        if( lNextData.mFrame.Cmd.mCmd != LtComCanBus::VU_CMD_SEND_DETECT_ONCE )
        {
            throw std::runtime_error( "Unexpected data, id = " + LeddarUtils::LtStringUtils::IntToString( lNextData.mId, 16 ) );
        }

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

        lNextData = mProtocol->GetNextDetectionData();
    }

    if( lNextData.mId != GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 ) + 1 )
    {
        throw std::runtime_error( "Unexpected data, id = " + LeddarUtils::LtStringUtils::IntToString( lNextData.mId, 16 ) );
    }

    uint8_t lEchoCount = lNextData.mFrame.Cmd.mCmd;
    uint8_t lCrurrentLedPower = lNextData.mFrame.Cmd.mSubCmd;
    uint32_t lTimestamp = *reinterpret_cast<uint32_t *>( &lNextData.mFrame.Cmd.mArg[2] );

    std::vector<LeddarConnection::LdEcho> *lEchoes = mEchoes.GetEchoes( LeddarConnection::B_SET );
    mEchoes.Lock( LeddarConnection::B_SET );
    mEchoes.SetEchoCount( lEchoCount );

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

        if( lNextData.mId < GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 ) + 2 ||
                lNextData.mId > GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>( 0 ) + 2 + LtComCanBus::CAN_MAX_DETECTIONS )
        {
            throw std::runtime_error( "Unexpected data, id = 0x" + LeddarUtils::LtStringUtils::IntToString( lNextData.mId, 16 ) );
        }

        LtComCanBus::sVuCANEcho *lEcho = reinterpret_cast<LtComCanBus::sVuCANEcho *>( lNextData.mFrame.mRawData );

        ( *lEchoes )[ i ].mAmplitude = lEcho->mAmplitude;
        ( *lEchoes )[ i ].mDistance = lEcho->mDistance;
        ( *lEchoes )[ i ].mChannelIndex = lEcho->mSegment;
        ( *lEchoes )[ i ].mFlag = lEcho->mFlag;
        ++i;
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
/// \fn void LeddarDevice::LdSensorVu8Can::EnableStreamingDetections( bool aEnable )
///
/// \brief  Enables / disabled the streaming of the detections
///
/// \param  aEnable True to enable, false to disable.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorVu8Can::EnableStreamingDetections( bool aEnable )
{
    mProtocol->EnableStreamingDetections( aEnable );
}

#endif
