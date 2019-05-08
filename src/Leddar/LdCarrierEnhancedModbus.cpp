// *****************************************************************************
// Module..: Leddar
//
/// \file    LdCarrierEnhancedModbus.cpp
///
/// \brief   Definition of the LeddarVu8 carrier board connecting by Modbus
///
/// \author  Patrick Boulay
///
/// \since   September 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdCarrierEnhancedModbus.h"
#if defined(BUILD_MODBUS)

#include "LdConnection.h"
#include "LdConnectionInfoModbus.h"
#include "LdConnectionModbusStructures.h"
#include "LdConnectionUniversalModbus.h"
#include "LdPropertyIds.h"

#include "LdBitFieldProperty.h"
#include "LdTextProperty.h"

#include "LtTimeUtils.h"

#include "comm/Modbus/LtComLeddarVu8Modbus.h"
#define _VU8
#include "comm/PlatformM7DefinitionsShared.h"
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include "comm/registerMap.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#undef _VU8

#include <sstream>
#include <cstring>

using namespace LeddarCore;

// *****************************************************************************
// Function: LdCarrierEnhancedModbus::LdCarrierEnhancedModbus
//
/// \brief   Constructor
///
/// \param   aConnection
/// \param   aProperties Properties set  if already define, 0 to create a new set
///
/// \author  Patrick Boulay
///
/// \since   September 2016
// *****************************************************************************

LeddarDevice::LdCarrierEnhancedModbus::LdCarrierEnhancedModbus( LeddarConnection::LdConnection *aConnection )
{
    mModbusConnection = dynamic_cast< LeddarConnection::LdConnectionUniversalModbus * >( aConnection );

    InitProperties();
}

// *****************************************************************************
// Function: LdCarrierEnhancedModbus::GetConstants
//
/// \brief   Get contants of the carrier.
///
/// \author  Patrick Boulay
/// \author  Frédéric Parent
///
/// \since   September 2016
// *****************************************************************************

void
LeddarDevice::LdCarrierEnhancedModbus::GetConstants( void )
{
    LeddarConnection::LdConnectionModbuStructures::sModbusPacket lPacket = {};
    uint32_t lInSize, lOutSize;

    // Get connection
    const LeddarConnection::LdConnectionInfoModbus *lConnectionInfo = dynamic_cast< const LeddarConnection::LdConnectionInfoModbus * >( mModbusConnection->GetConnectionInfo() );
    LeddarConnection::LdInterfaceModbus *lInterfaceModbus = dynamic_cast< LeddarConnection::LdInterfaceModbus * >( mModbusConnection->GetInterface() );

    // Get carrier part number
    lOutSize = static_cast<uint32_t>( sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                                      sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusGetCarrierDeviceInfoReq ) );
    lInSize = static_cast<uint32_t>( sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                                     sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusGetCarrierDeviceInfoAnswer ) +
                                     MODBUS_CRC_SIZE );

    lPacket.mHeader.mModbusAddress                          = lConnectionInfo->GetModbusAddr();
    lPacket.mHeader.mFunctionCode                           = 0x45;
    lPacket.uRequest.mGetCarrierDeviceInfo.mSubFunctionCode = 3;

    lInterfaceModbus->SendRawRequest( ( uint8_t * )&lPacket, lOutSize );
    lInterfaceModbus->ReceiveRawConfirmation( ( uint8_t * )&lPacket, lInSize );

    mProperties.GetTextProperty( LeddarCore::LdPropertyIds::ID_CARRIER_PART_NUMBER )->ForceValue( 0, lPacket.uAnswer.mGetCarrierDeviceInfo.mCarrierDeviceInfo.mHardwarePartNumber );
    mProperties.GetTextProperty( LeddarCore::LdPropertyIds::ID_CARRIER_SERIAL_NUMBER )->ForceValue( 0, lPacket.uAnswer.mGetCarrierDeviceInfo.mCarrierDeviceInfo.mHardwareSerialNumber );
    mProperties.GetBitProperty( LeddarCore::LdPropertyIds::ID_CARRIER_OPTIONS )->ForceValue( 0, lPacket.uAnswer.mGetCarrierDeviceInfo.mCarrierDeviceInfo.mCarrierDeviceOption );

    // Get carrier firmware version
    lOutSize = static_cast<uint32_t>( sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                                      sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusGetCarrierFirmwareInfoReq ) );
    lInSize = static_cast<uint32_t>( sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                                     sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusGetCarrierFirmwareInfoAnswer ) +
                                     MODBUS_CRC_SIZE );

    memset( &lPacket, 0, sizeof( lPacket ) );
    lPacket.mHeader.mModbusAddress                          = lConnectionInfo->GetModbusAddr();
    lPacket.mHeader.mFunctionCode                           = 0x45;
    lPacket.uRequest.mGetCarrierDeviceInfo.mSubFunctionCode = 2;

    lInterfaceModbus->SendRawRequest( ( uint8_t * )&lPacket, lOutSize );
    lInterfaceModbus->ReceiveRawConfirmation( ( uint8_t * )&lPacket, lInSize );

    mProperties.GetTextProperty( LeddarCore::LdPropertyIds::ID_CARRIER_FIRMWARE_PART_NUMBER )->ForceValue( 0, lPacket.uAnswer.mGetCarrierFirwwareInfo.mFirmwarePartNumber );
    std::ostringstream lStr;
    lStr << ( int )lPacket.uAnswer.mGetCarrierFirwwareInfo.mFirmwareVersion[0] << "." <<
         ( int )lPacket.uAnswer.mGetCarrierFirwwareInfo.mFirmwareVersion[1] << "." <<
         ( int )lPacket.uAnswer.mGetCarrierFirwwareInfo.mFirmwareVersion[2] << "." <<
         ( int )lPacket.uAnswer.mGetCarrierFirwwareInfo.mFirmwareVersion[3];
    mProperties.GetTextProperty( LeddarCore::LdPropertyIds::ID_CARRIER_FIRMWARE_VERSION )->ForceValue( 0, lStr.str() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdCarrierEnhancedModbus::InitProperties(void)
///
/// \brief  Initializes the properties for the carrier
///
/// \author Patrick Boulay
/// \date   September 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdCarrierEnhancedModbus::InitProperties( void )
{
    mProperties.AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_FIRMWARE_VERSION, 0, REGMAP_FIRMWATE_VERSION_LENGTH,
                             LdTextProperty::TYPE_ASCII, "Carrier Firmware Version" ) );
    mProperties.AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_FIRMWARE_PART_NUMBER, 0,
                             REGMAP_FIRMWATE_VERSION_LENGTH, LdTextProperty::TYPE_ASCII, "Carrier Software Part Number" ) );
    mProperties.AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_PART_NUMBER, 0, REGMAP_PRODUCT_ID_LENGTH,
                             LdTextProperty::TYPE_ASCII,
                             "Carrier Part Number" ) );
    mProperties.AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_OPTIONS, 0, 4, "Carrier Options" ) );
    mProperties.AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_SERIAL_NUMBER, 0, REGMAP_PRODUCT_ID_LENGTH,
                             LdTextProperty::TYPE_ASCII,
                             "Carrier Serial Number" ) );

    // Serial ports settings
    mPropertiesSerial.AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE, 0, 4, true,
                                   "Serial Port Baudrate" ) );
    mPropertiesSerial.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS, 0, 1,
                                   "Serial Port Modbus Address" ) );
    mPropertiesSerial.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES, 0, 1,
                                   "Serial Port Maximum Echoes" ) );
    mPropertiesSerial.AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES, 0, 2, true,
                                   "Serial Port Distance Resolution" ) );
    mPropertiesSerial.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_DATA_BITS, 0, 1,
                                   "Serial Port Data Bit" ) );
    mPropertiesSerial.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_PARITY, 0, 1,
                                   "Serial Port Parity" ) );
    mPropertiesSerial.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS, 0, 1,
                                   "Serial Port Stop Bit" ) );
    mPropertiesSerial.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_FLOW_CONTROL, 0, 1,
                                   "Serial Port Flow Control" ) );
    mPropertiesSerial.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_LOGICAL_PORT, 0, 1,
                                   "Serial Port Logical Port Number" ) );

    mPropertiesSerial.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetLimits( 1, MODBUS_MAX_ADDR );
    mPropertiesSerial.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES )->SetLimits( 1, LEDDARVU8_MAX_SERIAL_DETECTIONS );

    LeddarCore::LdEnumProperty *lSerialBaud = mPropertiesSerial.GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE );
    lSerialBaud->AddEnumPair( 9600, "9600" );
    lSerialBaud->AddEnumPair( 19200, "19200" );
    lSerialBaud->AddEnumPair( 38400, "38400" );
    lSerialBaud->AddEnumPair( 57600, "57600" );
    lSerialBaud->AddEnumPair( 115200, "115200" );

    LeddarCore::LdEnumProperty *lSerialResolution = mPropertiesSerial.GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES );
    lSerialResolution->AddEnumPair( 1, "m" );
    lSerialResolution->AddEnumPair( 10, "dm" );
    lSerialResolution->AddEnumPair( 100, "cm" );
    lSerialResolution->AddEnumPair( 1000, "mm" );


    // CAN ports properties
    mPropertiesCAN.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_LOGICAL_PORT, 0, 1, "CAN Port Logical Port" ) );
    mPropertiesCAN.AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE, 0, 4, true,
                                "CAN Port Baud Rate" ) );
    mPropertiesCAN.AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT, 0, 1, true,
                                "CAN Port Frame Format" ) );
    mPropertiesCAN.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID, 0, 4,
                                "CAN Port Base Tx Id" ) );
    mPropertiesCAN.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID, 0, 4,
                                "CAN Port Base Rx Id" ) );
    mPropertiesCAN.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES, 0, 1,
                                "CAN Port Maximum Echoes" ) );
    mPropertiesCAN.AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES, 0, 2, true,
                                "CAN Port Distance Resolution" ) );
    mPropertiesCAN.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY, 0, 2,
                                "CAN Port Inter-Message Delay" ) );
    mPropertiesCAN.AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY, 0, 2,
                                "CAN Port Inter-Cycle Delay" ) );

    mPropertiesCAN.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->SetLimits( 1, LEDDARVU8_MAX_CAN_DETECTIONS );
    mPropertiesCAN.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->SetLimits( 0, std::numeric_limits<uint16_t>::max() );
    mPropertiesCAN.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->SetLimits( 0, std::numeric_limits<uint16_t>::max() );

    LeddarCore::LdEnumProperty *lCanBaud = mPropertiesCAN.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE );
    lCanBaud->AddEnumPair( 10000, "10000" );
    lCanBaud->AddEnumPair( 20000, "20000" );
    lCanBaud->AddEnumPair( 50000, "50000" );
    lCanBaud->AddEnumPair( 100000, "100000" );
    lCanBaud->AddEnumPair( 125000, "125000" );
    lCanBaud->AddEnumPair( 250000, "250000" );
    lCanBaud->AddEnumPair( 500000, "500000" );
    lCanBaud->AddEnumPair( 1000000, "1000000" );

    LeddarCore::LdEnumProperty *lCanFrameFormat = mPropertiesCAN.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT );
    lCanFrameFormat->AddEnumPair( 0, "Standard 11 bits" );
    lCanFrameFormat->AddEnumPair( 1, "Extended 29 bits" );

    LeddarCore::LdEnumProperty *lCanResolution = mPropertiesCAN.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES );
    lCanResolution->AddEnumPair( 1, "m" );
    lCanResolution->AddEnumPair( 10, "dm" );
    lCanResolution->AddEnumPair( 100, "cm" );
    lCanResolution->AddEnumPair( 1000, "mm" );

    mProperties.AddProperties( &mPropertiesCAN );
    mProperties.AddProperties( &mPropertiesSerial );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdCarrierEnhancedModbus::GetConfig(void)
///
/// \brief  Gets the configuration of the carrier board
///
/// \author Patrick Boulay
/// \date   September 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdCarrierEnhancedModbus::GetConfigSerial( void )
{
    LeddarConnection::LdConnectionModbuStructures::sModbusPacket lSendRequestBuffer;
    LeddarConnection::LdInterfaceModbus *lModbus = dynamic_cast<LeddarConnection::LdInterfaceModbus *>( mModbusConnection->GetInterface() );

    const LeddarConnection::LdConnectionInfoModbus *lConnectionModbus = dynamic_cast< const LeddarConnection::LdConnectionInfoModbus * >
            ( mModbusConnection->GetConnectionInfo() );

    lSendRequestBuffer.mHeader.mModbusAddress = lConnectionModbus->GetModbusAddr();
    lSendRequestBuffer.mHeader.mFunctionCode = 0x45;
    lSendRequestBuffer.uRequest.mGetSerialPortSetting.mSubFunctionCode = 0;

    size_t lOutSize = sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                      sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusGetSerialPortSettingReq );
    size_t lInSize = 0;  //Use timeout to determine end of modbus transaction (undefined size to read...)

    lModbus->SendRawRequest( ( uint8_t * )&lSendRequestBuffer, ( uint32_t )lOutSize );

    LeddarConnection::LdConnectionModbuStructures::sModbusPacket lReceiveConfirmationBuffer;
    lModbus->ReceiveRawConfirmation( ( uint8_t * )&lReceiveConfirmationBuffer, ( uint32_t )lInSize );

    LdEnumProperty *lSerialPortBaudrate = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE );
    LdIntegerProperty *lSerialPortModbusAddress = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS );
    LdIntegerProperty *lSerialPortMaxEchoes = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES );
    LdEnumProperty *lSerialPortEchoesResolution = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES );
    LdIntegerProperty *lSerialPortDataBit = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_DATA_BITS );
    LdIntegerProperty *lSerialPortParity = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_PARITY );
    LdIntegerProperty *lSerialPortStopBit = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS );
    LdIntegerProperty *lSerialPortFlowControl = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_FLOW_CONTROL );
    LdIntegerProperty *lSerialPortLogicalPort = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_LOGICAL_PORT );

    lSerialPortBaudrate->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );
    lSerialPortModbusAddress->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );
    lSerialPortMaxEchoes->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );
    lSerialPortEchoesResolution->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );
    lSerialPortDataBit->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );
    lSerialPortParity->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );
    lSerialPortStopBit->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );
    lSerialPortFlowControl->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );
    lSerialPortLogicalPort->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort );

    for( uint8_t i = 0; i < lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mNumberOfSerialPort; ++i )
    {
        lSerialPortBaudrate->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mBaudRate );
        lSerialPortModbusAddress->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mModbusAddr );
        lSerialPortMaxEchoes->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mMaxEchos );
        lSerialPortEchoesResolution->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mDistanceResolution );
        lSerialPortDataBit->ForceValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mDataSize );
        lSerialPortParity->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mParity );
        lSerialPortStopBit->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mStopBit );
        lSerialPortFlowControl->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mFlowControl );
        lSerialPortLogicalPort->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetSerialPortSetting.mSerialPortSettings[i].mLogicalSerialPortNumber );
    }

    lSerialPortBaudrate->SetClean();
    lSerialPortModbusAddress->SetClean();
    lSerialPortMaxEchoes->SetClean();
    lSerialPortEchoesResolution->SetClean();
    lSerialPortDataBit->SetClean();
    lSerialPortParity->SetClean();
    lSerialPortStopBit->SetClean();
    lSerialPortFlowControl->SetClean();
    lSerialPortLogicalPort->SetClean();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdCarrierEnhancedModbus::SetConfig(void)
///
/// \brief  Sets the configuration of the carrier board
///
/// \author Patrick Boulay
/// \date   September 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdCarrierEnhancedModbus::SetConfigSerial( void )
{
    if( mPropertiesSerial.IsModified( LdProperty::CAT_CONFIGURATION ) )
    {
        LdEnumProperty *lSerialPortBaudrate = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE );
        LdIntegerProperty *lSerialPortModbusAddress = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS );
        LdIntegerProperty *lSerialPortMaxEchoes = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES );
        LdEnumProperty *lSerialPortEchoesResolution = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES );
        LdIntegerProperty *lSerialPortDataBit = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_DATA_BITS );
        LdIntegerProperty *lSerialPortParity = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_PARITY );
        LdIntegerProperty *lSerialPortStopBit = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS );
        LdIntegerProperty *lSerialPortFlowControl = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_FLOW_CONTROL );
        LdIntegerProperty *lSerialPortLogicalPort = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_LOGICAL_PORT );

        LeddarConnection::LdConnectionModbuStructures::sModbusPacket lSendRequestBuffer;
        LeddarConnection::LdInterfaceModbus *lModbus = dynamic_cast<LeddarConnection::LdInterfaceModbus *>( mModbusConnection->GetInterface() );

        const LeddarConnection::LdConnectionInfoModbus *lConnectionModbus = dynamic_cast<const LeddarConnection::LdConnectionInfoModbus *>
                ( mModbusConnection->GetConnectionInfo() );
        lSendRequestBuffer.mHeader.mModbusAddress = lConnectionModbus->GetModbusAddr();
        lSendRequestBuffer.mHeader.mFunctionCode = 0x45;
        lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSubFunctionCode = 1;

        for( uint8_t i = 0; i < lSerialPortBaudrate->Count(); ++i )
        {
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mLogicalSerialPortNumber = lSerialPortLogicalPort->ValueT<uint8_t>( i );
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mBaudRate = lSerialPortBaudrate->Value( i );
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mModbusAddr = lSerialPortModbusAddress->ValueT<uint8_t>( i );
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mMaxEchos = lSerialPortMaxEchoes->ValueT<uint8_t>( i );
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mDistanceResolution = lSerialPortEchoesResolution->Value( i );
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mDataSize = lSerialPortDataBit->ValueT<uint8_t>( i );
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mParity = lSerialPortParity->ValueT<uint8_t>( i );
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mStopBit = lSerialPortStopBit->ValueT<uint8_t>( i );
            lSendRequestBuffer.uRequest.mSetSerialPortSetting.mSerialPortSettings[i].mFlowControl = lSerialPortFlowControl->ValueT<uint8_t>( i );
        }


        size_t lOutSize = sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                          offsetof( LeddarConnection::LdConnectionModbuStructures::sModbusSetSerialPortSettingReq, mSerialPortSettings ) +
                          ( sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusSerialPortSetting ) * lSerialPortBaudrate->Count() );
        size_t lInSize = sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                         sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusSetSerialPortSettingAnswer ) +
                         MODBUS_CRC_SIZE;

        lModbus->SendRawRequest( ( uint8_t * )&lSendRequestBuffer, ( uint32_t )lOutSize );


        LeddarConnection::LdConnectionModbuStructures::sModbusPacket lConfirmation;
        lModbus->ReceiveRawConfirmation( ( uint8_t * )&lConfirmation, ( uint32_t )lInSize );

        lSerialPortBaudrate->SetClean();
        lSerialPortModbusAddress->SetClean();
        lSerialPortMaxEchoes->SetClean();
        lSerialPortEchoesResolution->SetClean();
        lSerialPortDataBit->SetClean();
        lSerialPortParity->SetClean();
        lSerialPortStopBit->SetClean();
        lSerialPortFlowControl->SetClean();
        lSerialPortLogicalPort->SetClean();

        // Wait a little bit time to let carrier board to reload changes on all serial port before to continue any transaction:
        // the current serial port where we are connected will not respond now...
        LeddarUtils::LtTimeUtils::Wait( 200 );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdCarrierEnhancedModbus::GetConfigCAN(void)
///
/// \brief  Gets configuration of CAN port settings
///
/// \author Patrick Boulay
/// \date   September 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdCarrierEnhancedModbus::GetConfigCAN( void )
{
    LdIntegerProperty *lCANLogicalPort = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_LOGICAL_PORT );
    LdEnumProperty *lCANBaudRate = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE );
    LdEnumProperty *lCANFrameFormat = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT );
    LdIntegerProperty *lCANTxMsgBaseId = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID );
    LdIntegerProperty *lCANRxMsgBaseId = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID );
    LdIntegerProperty *lCANMaxEchoes = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES );
    LdEnumProperty *lCANEchoesRes = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES );
    LdIntegerProperty *lCANMailboxDelay = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY );
    LdIntegerProperty *lCANAcqCycleDelay = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY );


    LeddarConnection::LdConnectionModbuStructures::sModbusPacket lSendRequestBuffer;
    LeddarConnection::LdInterfaceModbus *lModbus = dynamic_cast<LeddarConnection::LdInterfaceModbus *>( mModbusConnection->GetInterface() );

    const LeddarConnection::LdConnectionInfoModbus *lConnectionModbus = dynamic_cast< const LeddarConnection::LdConnectionInfoModbus * >
            ( mModbusConnection->GetConnectionInfo() );
    lSendRequestBuffer.mHeader.mModbusAddress = lConnectionModbus->GetModbusAddr();
    lSendRequestBuffer.mHeader.mFunctionCode = 0x45;
    lSendRequestBuffer.uRequest.mGetCANPortSetting.mSubFunctionCode = 4;

    size_t lOutSize = sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                      sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusGetCANPortSettingReq );
    size_t lInSize = 0;  //Use timeout to determine end of modbus transaction (undefined size to read...)

    lModbus->SendRawRequest( ( uint8_t * )&lSendRequestBuffer, ( uint32_t )lOutSize );

    LeddarConnection::LdConnectionModbuStructures::sModbusPacket lReceiveConfirmationBuffer;
    lModbus->ReceiveRawConfirmation( ( uint8_t * )&lReceiveConfirmationBuffer, ( uint32_t )lInSize );

    lCANLogicalPort->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );
    lCANBaudRate->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );
    lCANFrameFormat->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );
    lCANTxMsgBaseId->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );
    lCANRxMsgBaseId->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );
    lCANMaxEchoes->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );
    lCANEchoesRes->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );
    lCANMailboxDelay->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );
    lCANAcqCycleDelay->SetCount( lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort );

    for( uint8_t i = 0; i < lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mNumberOfCANPort; ++i )
    {
        lCANLogicalPort->ForceValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mLogicalCANPortNumber );
        lCANBaudRate->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mBaudRate );
        lCANFrameFormat->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mFrameFormat );
        lCANTxMsgBaseId->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mTxBaseId );
        lCANRxMsgBaseId->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mRxBaseId );
        lCANMaxEchoes->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mMaxNumberDetection );
        lCANEchoesRes->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mDistanceResolution );
        lCANMailboxDelay->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mInterMessageDelay );
        lCANAcqCycleDelay->SetValue( i, lReceiveConfirmationBuffer.uAnswer.mGetCANPortSetting.mCANPortSettings[i].mInterCycleDelay );
    }

    lCANLogicalPort->SetClean();
    lCANBaudRate->SetClean();
    lCANFrameFormat->SetClean();
    lCANTxMsgBaseId->SetClean();
    lCANRxMsgBaseId->SetClean();
    lCANMaxEchoes->SetClean();
    lCANEchoesRes->SetClean();
    lCANMailboxDelay->SetClean();
    lCANAcqCycleDelay->SetClean();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdCarrierEnhancedModbus::SetConfigCAN( void )
///
/// \brief  Sets configuration of CAN port settings
///
/// \author Patrick Boulay
/// \date   September 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdCarrierEnhancedModbus::SetConfigCAN( void )
{
    if( mPropertiesCAN.IsModified( LdProperty::CAT_CONFIGURATION ) )
    {

        LdIntegerProperty *lCANLogicalPort = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_LOGICAL_PORT );
        LdEnumProperty *lCANBaudRate = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE );
        LdEnumProperty *lCANFrameFormat = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT );
        LdIntegerProperty *lCANTxMsgBaseId = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID );
        LdIntegerProperty *lCANRxMsgBaseId = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID );
        LdIntegerProperty *lCANMaxEchoes = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES );
        LdEnumProperty *lCANEchoesRes = mProperties.GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES );
        LdIntegerProperty *lCANMailboxDelay = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY );
        LdIntegerProperty *lCANAcqCycleDelay = mProperties.GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY );

        LeddarConnection::LdConnectionModbuStructures::sModbusPacket lSendRequestBuffer;
        LeddarConnection::LdInterfaceModbus *lModbus = dynamic_cast<LeddarConnection::LdInterfaceModbus *>( mModbusConnection->GetInterface() );

        const LeddarConnection::LdConnectionInfoModbus *lConnectionModbus = dynamic_cast<const LeddarConnection::LdConnectionInfoModbus *>
                ( mModbusConnection->GetConnectionInfo() );
        lSendRequestBuffer.mHeader.mModbusAddress = lConnectionModbus->GetModbusAddr();
        lSendRequestBuffer.mHeader.mFunctionCode = 0x45;
        lSendRequestBuffer.uRequest.mSetCANPortSetting.mSubFunctionCode = 5;

        for( uint8_t i = 0; i < lCANLogicalPort->Count(); ++i )
        {
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mLogicalCANPortNumber = lCANLogicalPort->ValueT<uint8_t>( i );
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mBaudRate = lCANBaudRate->Value( i );
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mFrameFormat = lCANFrameFormat->Value( i );
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mRxBaseId = lCANTxMsgBaseId->ValueT<uint32_t>( i );
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mTxBaseId = lCANRxMsgBaseId->ValueT<uint32_t>( i );
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mMaxNumberDetection = lCANMaxEchoes->ValueT<uint8_t>( i );
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mDistanceResolution = lCANEchoesRes->Value( i );
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mInterMessageDelay = lCANMailboxDelay->ValueT<uint16_t>( i );
            lSendRequestBuffer.uRequest.mSetCANPortSetting.mCANPortSettings[i].mInterCycleDelay = lCANAcqCycleDelay->ValueT<uint16_t>( i );
        }

        size_t lOutSize = sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                          offsetof( LeddarConnection::LdConnectionModbuStructures::sModbusSetCANPortSettingReq, mCANPortSettings ) +
                          ( sizeof( LeddarConnection::LdConnectionModbuStructures::sCANPortSetting ) * lCANLogicalPort->Count() );
        size_t lInSize = sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                         sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusSetCANPortSettingAnswer ) +
                         MODBUS_CRC_SIZE;

        lModbus->SendRawRequest( ( uint8_t * )&lSendRequestBuffer, ( uint32_t )lOutSize );

        LeddarConnection::LdConnectionModbuStructures::sModbusPacket lConfirmation;
        lModbus->ReceiveRawConfirmation( ( uint8_t * )&lConfirmation, ( uint32_t )lInSize );

        // Wait a little bit time to let carrier board to reload changes on all CAN port before to continue any transaction.
        LeddarUtils::LtTimeUtils::Wait( 100 );

        lCANLogicalPort->SetClean();
        lCANBaudRate->SetClean();
        lCANFrameFormat->SetClean();
        lCANTxMsgBaseId->SetClean();
        lCANRxMsgBaseId->SetClean();
        lCANMaxEchoes->SetClean();
        lCANEchoesRes->SetClean();
        lCANMailboxDelay->SetClean();
        lCANAcqCycleDelay->SetClean();
    }
}

#endif