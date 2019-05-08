// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensorVu8Modbus.cpp
///
/// \brief   Definition of LeddarVu8 sensor class using modbus protocole.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdSensorVu8Modbus.h"
#if defined(BUILD_VU) && defined(BUILD_MODBUS)

#include "LdPropertyIds.h"
#include "LdBitFieldProperty.h"
#include "LdBoolProperty.h"
#include "LdEnumProperty.h"
#include "LdFloatProperty.h"
#include "LdIntegerProperty.h"
#include "LdTextProperty.h"
#include "LdConnectionUniversal.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"

#include "comm/Modbus/LtComLeddarVu8Modbus.h"

#include <limits>
#include <string.h>


using namespace LeddarCore;
using namespace LeddarDevice;


// *****************************************************************************
// Function: LdSensorVu8Modbus::LdSensorVu8Modbus
//
/// \brief   Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param   aConnection Associated connection for the LeddarVu8 Modbus.
///          The connection must be a child of LdLibModbusSerial interface.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

LdSensorVu8Modbus::LdSensorVu8Modbus( LeddarConnection::LdConnection *aConnection ) :
    LdSensor( aConnection ),
    mConnectionInfoModbus( nullptr ),
    mInterface( nullptr )
{
    if( aConnection != nullptr )
    {
        mConnectionInfoModbus = dynamic_cast< const LeddarConnection::LdConnectionInfoModbus * >( aConnection->GetConnectionInfo() );
        mInterface = dynamic_cast< LeddarConnection::LdLibModbusSerial * >( aConnection );
    }

    InitProperties();
}

// *****************************************************************************
// Function: LdSensorVu8Modbus::~LdSensorVu8Modbus
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

LdSensorVu8Modbus::~LdSensorVu8Modbus()
{
}

// *****************************************************************************
// Function: LdSensorVu8Modbus::Connect
//
/// \brief   Connect to the sensor
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSensorVu8Modbus::Connect( void )
{
    LdDevice::Connect();

    // Make sure the acqusition engine is started.
    mInterface->WriteRegister( 0x0A, 1 );
}

// *****************************************************************************
// Function: LdSensorVu8Modbus::GetStates
//
/// \brief   Get the states from the device.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSensorVu8Modbus::GetStates()
{

}

// *****************************************************************************
// Function: LdSensorVu8Modbus::GetEchoes
//
/// \brief   Get the echoes
///
/// \return  Return true if there is new echoes
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************
bool
LdSensorVu8Modbus::GetEchoes( void )
{
    uint8_t lRawRequest[ 10 ] = { mConnectionInfoModbus->GetModbusAddr(), 0x41 };
    uint8_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 2 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmationLT( lResponse, GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->ValueT<uint16_t>() );

    if( lReceivedSize <= MODBUS_DATA_OFFSET )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) );
    }

    uint8_t lEchoCount = lResponse[MODBUS_DATA_OFFSET];

    if( lReceivedSize >= MODBUS_DATA_OFFSET + 1u + lEchoCount * 6u )
    {
        mEchoes.SetEchoCount( lEchoCount );
        LtComLeddarVu8Modbus::sLeddarVu8ModbusDetections *lDetections = ( LtComLeddarVu8Modbus::sLeddarVu8ModbusDetections * )&lResponse[MODBUS_DATA_OFFSET + 1];

        for( uint8_t i = 0; ( i < lEchoCount ) && ( i < LEDDARVU8_MAX_SERIAL_DETECTIONS ); ++i )
        {
            LeddarConnection::LdEcho &lEcho = ( *mEchoes.GetEchoes( LeddarConnection::B_SET ) )[i];
            lEcho.mDistance = lDetections->mDistance;
            lEcho.mAmplitude = lDetections->mAmplitude;
            lEcho.mFlag = lDetections->mFlag;
            lEcho.mChannelIndex = lDetections->mSegment;
            lDetections++;
        }

        LtComLeddarVu8Modbus::sLeddarVu8ModbusDetectionsTrailing *lDetectionsTrail = ( LtComLeddarVu8Modbus::sLeddarVu8ModbusDetectionsTrailing * )lDetections;

        if( lReceivedSize >= MODBUS_DATA_OFFSET + 1u + lEchoCount * 6u + 7u )
        {
            if( mEchoes.GetTimestamp( LeddarConnection::B_GET ) == lDetectionsTrail->mTimestamp )
            {
                return false;
            }

            mEchoes.SetTimestamp( lDetectionsTrail->mTimestamp );
            mEchoes.SetCurrentLedPower( lDetectionsTrail->mLedPower );
        }
    }
    else
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Not enough data received, size: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) );
    }

    mEchoes.Swap();
    mEchoes.UpdateFinished();
    return true;
}

// *****************************************************************************
// Function: LdSensorVu8Modbus::GetConfig
//
/// \brief   Get config properties on the sensor
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSensorVu8Modbus::GetConfig( void )
{
    //Get sensor config values
    uint16_t lResponse2[LTMODBUS_RTU_MAX_ADU_LENGTH / 2] = { 0 };
    mInterface->ReadRegisters( 0, 3, lResponse2 );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LEDDARVU8_WAIT_AFTER_REQUEST );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetValue( 0, lResponse2[0] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetValue( 0, lResponse2[1] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetValue( 0, lResponse2[2] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetClean();

    memset( lResponse2, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
    mInterface->ReadRegisters( 4, 4, lResponse2 );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LEDDARVU8_WAIT_AFTER_REQUEST );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY )->SetRawValue( 0, static_cast<int16_t>( lResponse2[0] ) );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY )->SetClean();
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->SetValue( 0, lResponse2[1] );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->SetClean();
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS )->SetValue( 0, lResponse2[2] );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CHANGE_DELAY )->SetValue( 0, lResponse2[3] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CHANGE_DELAY )->SetClean();

    memset( lResponse2, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
    mInterface->ReadRegisters( 11, 3, lResponse2 );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LEDDARVU8_WAIT_AFTER_REQUEST );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetValue( 0, static_cast<int16_t>( lResponse2[0] ) );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetClean();
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->SetValue( 0, lResponse2[1] );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->SetClean();


    // Get serial port configuration information
    GetSerialConfig();

    // Get CAN configuration information
    GetCanConfig();

    //  Get carrier info
    GetCarrierInfoConfig();
    GetCarrierFirmwareInfoConfig();

}

// *****************************************************************************
// Function: LdSensorVu8Modbus::GetConstants
//
/// \brief   Get constants properties on the sensor
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSensorVu8Modbus::GetConstants( void )
{
    //Get server info
    uint8_t lRawRequest[10] = { mConnectionInfoModbus->GetModbusAddr(), 0x11 };
    uint8_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    mInterface->SendRawRequest(lRawRequest, 2);
    size_t lReceivedSize = mInterface->ReceiveRawConfirmation(lResponse, 0);
    LeddarUtils::LtTimeUtils::WaitBlockingMicro(LEDDARVU8_WAIT_AFTER_REQUEST);

    if (lReceivedSize < sizeof(LtComLeddarVu8Modbus::sLeddarVu8ModbusServerId))
    {
        mInterface->Flush();
        throw LeddarException::LtComException("Received size too small, received: " + LeddarUtils::LtStringUtils::IntToString(lReceivedSize) + " expected: " +
            LeddarUtils::LtStringUtils::IntToString(
                sizeof(LtComLeddarVu8Modbus::sLeddarVu8ModbusServerId)));
    }

    LtComLeddarVu8Modbus::sLeddarVu8ModbusServerId lServedId = *(reinterpret_cast<LtComLeddarVu8Modbus::sLeddarVu8ModbusServerId*>(&lResponse[MODBUS_DATA_OFFSET]));

    GetProperties()->GetTextProperty(LdPropertyIds::ID_SERIAL_NUMBER)->ForceValue(0, lServedId.mSerialNumber);
    GetProperties()->GetTextProperty(LdPropertyIds::ID_DEVICE_NAME)->ForceValue(0, lServedId.mDeviceName);
    GetProperties()->GetTextProperty(LdPropertyIds::ID_PART_NUMBER)->ForceValue(0, lServedId.mHardwarePartNumber);
    GetProperties()->GetTextProperty(LdPropertyIds::ID_SOFTWARE_PART_NUMBER)->ForceValue(0, lServedId.mSoftwarePartNumber);

    LeddarCore::LdIntegerProperty* lFirmwareVersion = GetProperties()->GetIntegerProperty(LdPropertyIds::ID_FIRMWARE_VERSION_INT);
    LeddarCore::LdIntegerProperty* lBootloaderVersion = GetProperties()->GetIntegerProperty(LdPropertyIds::ID_BOOTLOADER_VERSION);
    lFirmwareVersion->SetCount(4);
    lBootloaderVersion->SetCount(4);

    for (int i = 0; i < 4; i++)
    {
        lFirmwareVersion->ForceValue(i, lServedId.mFirwareVersion[i]);
        lBootloaderVersion->ForceValue(i, lServedId.mBootloaderVersion[i]);
    }

    GetProperties()->GetIntegerProperty(LdPropertyIds::ID_FPGA_VERSION)->ForceValue(0, lServedId.mFpgaVersion);
    GetProperties()->GetBitProperty(LdPropertyIds::ID_OPTIONS)->ForceValue(0, lServedId.mDeviceOptions);
    GetProperties()->GetIntegerProperty(LdPropertyIds::ID_DEVICE_TYPE)->ForceValue(0, lServedId.mDeviceId);

    if (GetConnection()->GetDeviceType() == 0)
    {
        GetConnection()->SetDeviceType(lServedId.mDeviceId);
    }

    UpdateConstants();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu8Modbus::UpdateConstants( void )
///
/// \brief  Updates the constants
///
/// \author David Levy
/// \date   February 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu8Modbus::UpdateConstants( void )
{
    LeddarCore::LdIntegerProperty *lDistScale = GetProperties()->GetIntegerProperty( LdPropertyIds::ID_DISTANCE_SCALE );
    LeddarCore::LdIntegerProperty *lAmpScale = GetProperties()->GetIntegerProperty( LdPropertyIds::ID_FILTERED_AMP_SCALE );

    lDistScale->ForceValue( 0, LEDDARVU8_DISTANCE_SCALE );
    lAmpScale->ForceValue( 0, LEDDARVU8_AMPLITUDE_SCALE );

    mEchoes.Init( lDistScale->ValueT<uint32_t>(), lAmpScale->ValueT<uint32_t>(), LEDDARVU8_MAX_SERIAL_DETECTIONS );

    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->ForceValue( 0, LtComLeddarVu8Modbus::LEDDARVU8_HSEGMENT );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ForceValue( 0, LtComLeddarVu8Modbus::LEDDARVU8_VSEGMENT );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_RSEGMENT )->ForceValue( 0, LtComLeddarVu8Modbus::LEDDARVU8_RSEGMENT );
    GetProperties()->GetBitProperty( LdPropertyIds::ID_SEGMENT_ENABLE )->SetLimit( ( 1 << ( GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->Value() + 1 ) ) - 1 );
}

// *****************************************************************************
// Function: LdSensorVu8Modbus::GetSerialConfig
//
/// \brief   Get serial config properties on the sensor
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSensorVu8Modbus::GetSerialConfig( void )
{
    uint8_t lRawRequest[ 10 ] = { mConnectionInfoModbus->GetModbusAddr(), 0x45, 0 };
    uint8_t lResponse[ LTMODBUS_RTU_MAX_ADU_LENGTH ] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 3 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lResponse, 0 );


    if( lReceivedSize < sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusSerialPortSettings ) )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small, received: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) + " expected: " +
                                               LeddarUtils::LtStringUtils::IntToString(
                                                       sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusSerialPortSettings ) ) );
    }

    uint8_t lNumberSerial = lResponse[MODBUS_DATA_OFFSET + 1];
    uint8_t lCurrentSerial = lResponse[MODBUS_DATA_OFFSET + 2];

    LeddarCore::LdEnumProperty *lSPBaud = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE );
    LeddarCore::LdIntegerProperty *lSPDataBits = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_DATA_BITS );
    LeddarCore::LdIntegerProperty *lSPParity = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_PARITY );
    LeddarCore::LdIntegerProperty *lSPStopBits = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS );
    LeddarCore::LdIntegerProperty *lSPPortAddress = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS );
    LeddarCore::LdIntegerProperty *lSPFlowControl = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_FLOW_CONTROL );
    LeddarCore::LdIntegerProperty *lSPLogicalPort = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_LOGICAL_PORT );
    LeddarCore::LdIntegerProperty *lSMaxEchoes = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES );
    LeddarCore::LdEnumProperty *lSPEchoesRes = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_CURRENT_PORT )->ForceValue( 0, lCurrentSerial );

    lSPBaud->SetCount( lNumberSerial );
    lSPDataBits->SetCount( lNumberSerial );
    lSPParity->SetCount( lNumberSerial );
    lSPStopBits->SetCount( lNumberSerial );
    lSPPortAddress->SetCount( lNumberSerial );
    lSPFlowControl->SetCount( lNumberSerial );
    lSPLogicalPort->SetCount( lNumberSerial );
    lSMaxEchoes->SetCount( lNumberSerial );
    lSPEchoesRes->SetCount( lNumberSerial );

    LtComLeddarVu8Modbus::sLeddarVu8ModbusSerialPortSettings *lSerialPortSetting = ( LtComLeddarVu8Modbus::sLeddarVu8ModbusSerialPortSettings * )&lResponse[MODBUS_DATA_OFFSET + 3];

    for( int i = 0; i < lNumberSerial; ++i )
    {
        lSPBaud->SetValue( i, lSerialPortSetting->mBaudrate );
        lSPDataBits->ForceValue( i, lSerialPortSetting->mDataSize );
        lSPParity->SetValue( i, lSerialPortSetting->mParity );
        lSPStopBits->ForceValue( i, lSerialPortSetting->mStopBits );
        lSPPortAddress->SetValue( i, lSerialPortSetting->mAddress );
        lSPFlowControl->ForceValue( i, lSerialPortSetting->mFlowControl );
        lSPLogicalPort->ForceValue( i, lSerialPortSetting->mLogicalPortNumber );
        lSMaxEchoes->SetValue( i, lSerialPortSetting->mMaxEchoes );
        lSPEchoesRes->SetValue( i, lSerialPortSetting->mEchoesResolution );

        lSerialPortSetting++;
    }

}

// *****************************************************************************
// Function: LdSensorVu8Modbus::GetCanConfig
//
/// \brief   Get CAN config properties on the sensor
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSensorVu8Modbus::GetCanConfig( void )
{
    uint8_t lRawRequest[ 10 ] = { mConnectionInfoModbus->GetModbusAddr(), 0x45, 4 };
    uint8_t lResponse[ LTMODBUS_RTU_MAX_ADU_LENGTH ] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 3 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lResponse, 0 );


    if( lReceivedSize < sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusCanPortSettings ) )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small, received: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) + " expected: " +
                                               LeddarUtils::LtStringUtils::IntToString(
                                                       sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusCanPortSettings ) ) );
    }

    LtComLeddarVu8Modbus::sLeddarVu8ModbusCanPortSettings *lCanPortSetting = ( LtComLeddarVu8Modbus::sLeddarVu8ModbusCanPortSettings * )&lResponse[MODBUS_DATA_OFFSET + 2];

    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_LOGICAL_PORT )->ForceValue( 0, lCanPortSetting->mLogicalPortNumber );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->SetValue( 0, lCanPortSetting->mBaudrate );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->SetValue( 0, lCanPortSetting->mFrameFormat );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->SetValue( 0, lCanPortSetting->mTxBaseId );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->SetValue( 0, lCanPortSetting->mRxBaseId );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->SetValue( 0, lCanPortSetting->mMaxEchoes );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->SetValue( 0, lCanPortSetting->mEchoesResolution );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->SetValue( 0, lCanPortSetting->mInterMsgDelay );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->SetValue( 0, lCanPortSetting->mInterCycleDelay );


}

// *****************************************************************************
// Function: LdSensorVu8Modbus::GetCarrierInfoConfig
//
/// \brief   Get carrier info properties on the sensor
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSensorVu8Modbus::GetCarrierInfoConfig( void )
{
    uint8_t lRawRequest[ 10 ] = { mConnectionInfoModbus->GetModbusAddr(), 0x45, 3 };
    uint8_t lResponse[ LTMODBUS_RTU_MAX_ADU_LENGTH ] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 3 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lResponse, 0 );


    if( lReceivedSize < sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusCarrierInfo ) )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small, received: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) + " expected: " +
                                               LeddarUtils::LtStringUtils::IntToString(
                                                       sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusCarrierInfo ) ) );
    }

    LtComLeddarVu8Modbus::sLeddarVu8ModbusCarrierInfo *lCarrierInfo = ( LtComLeddarVu8Modbus::sLeddarVu8ModbusCarrierInfo * )&lResponse[MODBUS_DATA_OFFSET + 1];


    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_CARRIER_PART_NUMBER )->ForceValue( 0, lCarrierInfo->mHardwarePartNumber );
    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_CARRIER_SERIAL_NUMBER )->ForceValue( 0, lCarrierInfo->mHardwareSerialNumber );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_CARRIER_OPTIONS )->ForceValue( 0, lCarrierInfo->mCarrierDeviceOption );

}

// *****************************************************************************
// Function: LdSensorVu8Modbus::GetCarrierFirmwareInfoConfig
//
/// \brief   Get firmware carrier info properties on the sensor
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSensorVu8Modbus::GetCarrierFirmwareInfoConfig( void )
{
    uint8_t lRawRequest[ 10 ] = { mConnectionInfoModbus->GetModbusAddr(), 0x45, 2 };
    uint8_t lResponse[ LTMODBUS_RTU_MAX_ADU_LENGTH ] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 3 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lResponse, 0 );


    if( lReceivedSize < sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusCarrierFirmwareInfo ) )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small, received: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) + " expected: " +
                                               LeddarUtils::LtStringUtils::IntToString(
                                                       sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusCarrierInfo ) ) );
    }

    LtComLeddarVu8Modbus::sLeddarVu8ModbusCarrierFirmwareInfo *lCarrierFirmwareInfo = ( LtComLeddarVu8Modbus::sLeddarVu8ModbusCarrierFirmwareInfo * )&lResponse[MODBUS_DATA_OFFSET + 1];


    LeddarCore::LdIntegerProperty *lCarrierFirmwareVersion = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CARRIER_FIRMWARE_VERSION );
    lCarrierFirmwareVersion->SetCount( 4 );

    for( int i = 0; i < 4; i++ )
    {
        lCarrierFirmwareVersion->ForceValue( i, lCarrierFirmwareInfo->mFirmwareVersion[i] );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_CARRIER_FIRMWARE_PART_NUMBER )->ForceValue( 0, lCarrierFirmwareInfo->mFirmwarePartNumber );

}


// *****************************************************************************
// Function: LdSensorVu8Modbus::SetConfig
//
/// \brief   Set configuration to device
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************
void
LdSensorVu8Modbus::SetConfig( void )
{
    //All properties with a device different from 0 (plus ID_ACCUMULATION_EXP) have to be written individually with command 0x06 into that register
    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lPropertyIter = lProperties.begin(); lPropertyIter != lProperties.end(); ++lPropertyIter )
    {
        if( ( *lPropertyIter )->Modified() && ( ( *lPropertyIter )->GetDeviceId() != 0 || ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP ) )
        {
            int lValue;

            switch( ( *lPropertyIter )->GetType() )
            {
                case LeddarCore::LdProperty::TYPE_BITFIELD:
                    lValue = dynamic_cast<LeddarCore::LdBitFieldProperty *>( ( *lPropertyIter ) )->Value();
                    break;

                case LeddarCore::LdProperty::TYPE_BOOL:
                    lValue = dynamic_cast<LeddarCore::LdBoolProperty *>( ( *lPropertyIter ) )->Value();
                    break;

                case LeddarCore::LdProperty::TYPE_ENUM:
                    if( ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_LED_INTENSITY )
                        lValue = dynamic_cast< LeddarCore::LdEnumProperty * >( ( *lPropertyIter ) )->Value();
                    else
                        assert( false ); //Check for other enum if we return the value or the index

                    break;

                case LeddarCore::LdProperty::TYPE_FLOAT:
                    if( dynamic_cast< LeddarCore::LdFloatProperty * >( *lPropertyIter )->GetScale() != 0 )
                    {
                        lValue = ( *lPropertyIter )->RawValue();
                    }
                    else
                    {
                        throw std::logic_error( "Float properties must have a scale for modbus communication." );
                    }

                    break;

                case LeddarCore::LdProperty::TYPE_INTEGER:
                    lValue = dynamic_cast<LeddarCore::LdIntegerProperty *>( ( *lPropertyIter ) )->ValueT<int32_t>();
                    break;

                case LeddarCore::LdProperty::TYPE_TEXT:
                    assert( false ); //No text property available at the moment

                default:
                    assert( false );
                    lValue = 0;
                    break;
            }

            mInterface->WriteRegister( ( *lPropertyIter )->GetDeviceId(), lValue );
            ( *lPropertyIter )->SetClean();
            LeddarUtils::LtTimeUtils::WaitBlockingMicro( LEDDARVU8_WAIT_AFTER_REQUEST );
        }
    }


    SetCanConfig();
    SetSerialConfig();
}

// *****************************************************************************
// Function: LdSensorVu8Modbus::SetSerialConfig
//
/// \brief   Set serial ports configuration to device
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************
void
LdSensorVu8Modbus::SetSerialConfig( void )
{
    uint8_t lBuffer[ LTMODBUS_RTU_MAX_ADU_LENGTH ] = { 0 };
    lBuffer[ 0 ] = mConnectionInfoModbus->GetModbusAddr();
    lBuffer[ 1 ] = 0x45;
    lBuffer[ 2 ] = 1;

    uint8_t lNumberOfSerialPorts = static_cast<uint8_t>( GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_LOGICAL_PORT )->Count() );
    LtComLeddarVu8Modbus::sLeddarVu8ModbusSerialPortSettings *lSerialPortSettings = ( LtComLeddarVu8Modbus::sLeddarVu8ModbusSerialPortSettings * )&lBuffer[ 3 ];

    for( int i = 0; i < lNumberOfSerialPorts; ++i )
    {
        lSerialPortSettings->mLogicalPortNumber = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_LOGICAL_PORT )->ValueT<uint8_t>( i );
        lSerialPortSettings->mBaudrate = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE )->Value( i );
        lSerialPortSettings->mDataSize = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_DATA_BITS )->ValueT<uint8_t>( i );
        lSerialPortSettings->mParity = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_PARITY )->ValueT<uint8_t>( i );
        lSerialPortSettings->mStopBits = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS )->ValueT<uint8_t>( i );
        lSerialPortSettings->mFlowControl = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_FLOW_CONTROL )->ValueT<uint8_t>( i );
        lSerialPortSettings->mAddress = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->ValueT<uint8_t>( i );
        lSerialPortSettings->mMaxEchoes = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES )->ValueT<uint8_t>( i );
        lSerialPortSettings->mEchoesResolution = static_cast<uint32_t>( GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES )->Value( i ) );
        lSerialPortSettings++;
    }

    // Write acquisition configuration
    mInterface->SendRawRequest( lBuffer, ( sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusSerialPortSettings ) * lNumberOfSerialPorts ) + 3 );

    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lBuffer, 0 );

    if( ( lReceivedSize != 5 ) || ( lBuffer[ 1 ] != 0x45 ) || ( lBuffer[ 2 ] != 1 ) )
    {
        mInterface->Flush();
        LeddarException::LtComException( "Error to write serial port configuration." );
        return;
    }
}

// *****************************************************************************
// Function: LdSensorVu8Modbus::SetCanConfig
//
/// \brief   Set CAN ports configuration to device
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************
void
LdSensorVu8Modbus::SetCanConfig( void )
{
    uint8_t lBuffer[ LTMODBUS_RTU_MAX_ADU_LENGTH ] = { 0 };
    lBuffer[ 0 ] = mConnectionInfoModbus->GetModbusAddr();
    lBuffer[ 1 ] = 0x45;
    lBuffer[ 2 ] = 5;

    LtComLeddarVu8Modbus::sLeddarVu8ModbusCanPortSettings *lCanPortSettings = ( LtComLeddarVu8Modbus::sLeddarVu8ModbusCanPortSettings * )&lBuffer[ 3 ];

    lCanPortSettings->mLogicalPortNumber = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_LOGICAL_PORT )->ValueT<uint8_t>();
    lCanPortSettings->mBaudrate = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE )->Value();
    lCanPortSettings->mFrameFormat = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT )->Value();
    lCanPortSettings->mTxBaseId = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID )->ValueT<uint32_t>();
    lCanPortSettings->mRxBaseId = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID )->ValueT<uint32_t>();
    lCanPortSettings->mMaxEchoes = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->ValueT<uint8_t>();
    lCanPortSettings->mEchoesResolution = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES )->Value();
    lCanPortSettings->mInterMsgDelay = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->ValueT<uint16_t>();
    lCanPortSettings->mInterCycleDelay = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->ValueT<uint16_t>();

    // Write acquisition configuration
    mInterface->SendRawRequest( lBuffer, sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusCanPortSettings ) + 3 );

    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lBuffer, 0 );

    if( ( lReceivedSize != 5 ) || ( lBuffer[ 1 ] != 0x45 ) || ( lBuffer[ 2 ] != 5 ) )
    {
        mInterface->Flush();
        LeddarException::LtComException( "Error to write can port configuration." );
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorVu8Modbus::InitProperties( void )
///
/// \brief   Create properties for this specific sensor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu8Modbus::InitProperties( void )
{
    using namespace LeddarCore;

    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_RSEGMENT, 0, 2, "Number of reference segment" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, 0, 32, LdTextProperty::TYPE_ASCII,
                              "Serial Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_NAME, 0, 32, LdTextProperty::TYPE_ASCII,
                              "Device Name" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, 0, 32, LdTextProperty::TYPE_ASCII,
                              "Part Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER, 0, 32, LdTextProperty::TYPE_ASCII,
                              "Software Part Number" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_INT, 0, 2, "Firmware Version" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_BOOTLOADER_VERSION, 0, 2, "Bootloader Version" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, 0, 2, "FPGA Version" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS, 0, 4, "Options (Internal Use)" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_TYPE, 0, 2, "Device Type" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_DISTANCE_SCALE, 0, 4, "Distance Scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FILTERED_AMP_SCALE, 0, 4, "Amplitude Scale" ) );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_MODBUS );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();

    // Serial Port properties
    // The property count of serial port properties is the number of serial port available on the sensor, see the function GetSerialConfig for more information.
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_LOGICAL_PORT, 0, 1,
                              "Serial Port Logical Port" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE, 0, 4, true,
                              "Serial Port Baudrate" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_DATA_BITS, 0, 1, "Serial Port Data Bits" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE | LdProperty::F_EDITABLE, LdPropertyIds::ID_COM_SERIAL_PORT_PARITY, 0, 1,
                              "Serial Port Parity: 0 = None - 1 = Odd - 2 = Even" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS, 0, 1, "Serial port number of stop bits" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS, 0, 1,
                              "Serial Port Serial Port Address" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_FLOW_CONTROL, 0, 1,
                              "Serial Port Flow Control" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES, 0, 1,
                              "Serial Port Maximum Echoes" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES, 0, 2, true,
                              "Serial Port Distance Resolution" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_CURRENT_PORT, 0, 1,
                              "Serial Port Current Port Number" ) );

    // CAN port properties
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_LOGICAL_PORT, 0, 1, "CAN Port Logical Port" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE, 0, 4, true,
                              "CAN Port Baud Rate" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT, 0, 1, true,
                              "CAN Port Frame Format" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_TX_MSG_BASE_ID, 0, 4,
                              "CAN Port Base Tx Id" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_RX_MSG_BASE_ID, 0, 4,
                              "CAN Port Base Rx Id" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES, 0, 1,
                              "CAN Port Maximum Echoes" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES, 0, 2, true,
                              "CAN Port Distance Resolution" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY, 0, 2,
                              "CAN Port Inter-Message Delay" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY, 0, 2,
                              "CAN Port Inter-Cycle Delay" ) );

    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_PART_NUMBER, 0, 32, LdTextProperty::TYPE_ASCII,
                              "Carrier Board Part Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_SERIAL_NUMBER, 0, 32, LdTextProperty::TYPE_ASCII,
                              "Carrier Board Serial Number" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_OPTIONS, 0, 4, "Option (Internal Use)" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_FIRMWARE_VERSION, 0, 4,
                              "Carrier Board Firmware Version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_SAVE, LdPropertyIds::ID_CARRIER_FIRMWARE_PART_NUMBER, 0, 32, LdTextProperty::TYPE_ASCII,
                              "Carrier Board Firmware Part Number" ) );

    //The properties below have a device id, corresponding to the register used with command 0x03 and 0x06
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACCUMULATION_EXP,
                              LtComLeddarVu8Modbus::DID_ACCUMULATION_EXP, 2, "Accumulation Exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_OVERSAMPLING_EXP,
                              LtComLeddarVu8Modbus::DID_OVERSAMPLING_EXP, 2, "Oversampling Exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_BASE_POINT_COUNT,
                              LtComLeddarVu8Modbus::DID_BASE_POINT_COUNT, 2, "Points" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SENSIVITY,
                              LtComLeddarVu8Modbus::DID_THRESHOLD_OFFSET, 2, LEDDARVU8_THRESHOLD_SCALE, 3, "Threshold Offset" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_INTENSITY,
                              LtComLeddarVu8Modbus::DID_LED_INTENSITY, 1, false, "Laser Intensity" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACQ_OPTIONS,
                              LtComLeddarVu8Modbus::DID_ACQ_OPTIONS, 2, "Acquisition Options" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CHANGE_DELAY,
                              LtComLeddarVu8Modbus::DID_CHANGE_DELAY, 2, "Change Delay(Frame)" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PRECISION,
                              LtComLeddarVu8Modbus::DID_PRECISION, 4, "Smoothing", true ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SEGMENT_ENABLE,
                              LtComLeddarVu8Modbus::DID_SEGMENT_ENABLE, 2, "Segments Enable" ) );

    // Set limits and enums
    mProperties->GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_PARITY )->SetLimits( 0, 2 );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS )->SetLimits( 1, 2 );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetLimits( 1, MODBUS_MAX_ADDR );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES )->SetLimits( 1, LEDDARVU8_MAX_SERIAL_DETECTIONS );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_PRECISION )->SetLimits( LEDDARVU8_MIN_SMOOTHING, LEDDARVU8_MAX_SMOOTHING );

    LeddarCore::LdEnumProperty *lSerialBaud = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE );
    lSerialBaud->AddEnumPair( 9600, "9600" );
    lSerialBaud->AddEnumPair( 19200, "19200" );
    lSerialBaud->AddEnumPair( 38400, "38400" );
    lSerialBaud->AddEnumPair( 57600, "57600" );
    lSerialBaud->AddEnumPair( 115200, "115200" );

    LeddarCore::LdEnumProperty *lSerialResolution = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES );
    lSerialResolution->AddEnumPair( 1, "m" );
    lSerialResolution->AddEnumPair( 10, "dm" );
    lSerialResolution->AddEnumPair( 100, "cm" );
    lSerialResolution->AddEnumPair( 1000, "mm" );

    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAX_ECHOES )->SetLimits( 1, LEDDARVU8_MAX_CAN_DETECTIONS );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_MAILBOX_DELAY )->SetLimits( 0, std::numeric_limits<uint16_t>::max() );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY )->SetLimits( 0, std::numeric_limits<uint16_t>::max() );

    LeddarCore::LdEnumProperty *lCanBaud = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_BAUDRATE );
    lCanBaud->AddEnumPair( 10000, "10000" );
    lCanBaud->AddEnumPair( 20000, "20000" );
    lCanBaud->AddEnumPair( 50000, "50000" );
    lCanBaud->AddEnumPair( 100000, "100000" );
    lCanBaud->AddEnumPair( 125000, "125000" );
    lCanBaud->AddEnumPair( 250000, "250000" );
    lCanBaud->AddEnumPair( 500000, "500000" );
    lCanBaud->AddEnumPair( 1000000, "1000000" );

    LeddarCore::LdEnumProperty *lCanFrameFormat = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_FRAME_FORMAT );
    lCanFrameFormat->AddEnumPair( 0, "Standard 11 bits" );
    lCanFrameFormat->AddEnumPair( 1, "Extended 29 bits" );

    LeddarCore::LdEnumProperty *lCanResolution = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_CAN_PORT_ECHOES_RES );
    lCanResolution->AddEnumPair( 1, "m" );
    lCanResolution->AddEnumPair( 10, "dm" );
    lCanResolution->AddEnumPair( 100, "cm" );
    lCanResolution->AddEnumPair( 1000, "mm" );

    LeddarCore::LdEnumProperty *lLedPower = GetProperties()->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY );
    lLedPower->AddEnumPair( 6, "6" );
    lLedPower->AddEnumPair( 28, "28" );
    lLedPower->AddEnumPair( 53, "53" );
    lLedPower->AddEnumPair( 81, "81" );
    lLedPower->AddEnumPair( 100, "100" );
}
#endif