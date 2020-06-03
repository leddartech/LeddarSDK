/// *****************************************************************************
/// Module..: Leddar
///
/// \file    LdSensorOneModbus.cpp
///
/// \brief   Definition of LeddarOne sensor using modbus protocol
///
/// \author  David Levy
///
/// \since   September 2017
///
/// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
/// *****************************************************************************

#include "LdSensorOneModbus.h"
#if defined(BUILD_ONE) && defined(BUILD_MODBUS)

#include "comm/Modbus/LtComLeddarOneModbus.h"
#include "LdConnectionModbusStructures.h"
#include "LdPropertyIds.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"
#include "LtIntUtilities.h"
#include "LtTimeUtils.h"
#include <string.h>

using namespace LeddarDevice;

/// *****************************************************************************
/// Function: LdSensorOneModbus::LdSensorOneModbus
///
/// \brief   Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \author  David Levy
///
/// \since   April 2017
/// *****************************************************************************
LdSensorOneModbus::LdSensorOneModbus( LeddarConnection::LdConnection *aConnection ) :
    LdSensor( aConnection ),
    mParameterVersion( 1 )
{
    using namespace LeddarCore;

    mConnectionInfoModbus =  aConnection == nullptr ? nullptr : dynamic_cast< const LeddarConnection::LdConnectionInfoModbus * >( aConnection->GetConnectionInfo() );

    mInterface = dynamic_cast< LeddarConnection::LdLibModbusSerial * >( aConnection );

    InitProperties();
}

/// *****************************************************************************
/// Function: LdSensorOneModbus::~LdSensorOneModbus
///
/// \brief   Destructor.
///
/// \author  David Levy
///
/// \since   September 2017
/// *****************************************************************************
LdSensorOneModbus::~LdSensorOneModbus()
{
}

/// *****************************************************************************
/// Function: LdSensorOneModbus::Connect
///
/// \brief   Connect to the sensor
///
/// \author  David Levy
///
/// \since   September 2017
/// *****************************************************************************
void
LdSensorOneModbus::Connect( void )
{
    LdDevice::Connect();
}

// *****************************************************************************
// Function: LdSensorOneModbus::GetData
//
/// \brief   Get data on the device.
///
/// \return True if a new data was received.
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
bool
LdSensorOneModbus::GetData( void )
{
    if( mDataMask == DM_NONE )
    {
        SetDataMask( DM_ALL );
    }

    return RequestData( mDataMask );
}

// *****************************************************************************
// Function: LdSensorOneModbus::RequestData
//
/// \brief   Request data from the device
///
/// param[in] Requested data mask
///
/// \return True if a new data was received.
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
bool
LdSensorOneModbus::RequestData( uint32_t &aDataMask )
{
    //GetEchoes also get states
    if( ( aDataMask &  LdSensor::DM_ECHOES ) == LdSensor::DM_ECHOES ||
            ( aDataMask &  LdSensor::DM_STATES ) == LdSensor::DM_STATES )
    {
        return GetEchoes();
    }

    return false;
}

/// *****************************************************************************
/// Function: LdSensorOneModbus::GetEchoes
///
/// \brief   Get the echoes
///
/// \return  Return true if there is new echoes
///
/// \author  David Levy
///
/// \since   September 2017
/// *****************************************************************************
bool
LdSensorOneModbus::GetEchoes( void )
{
    using namespace LeddarUtils;
    uint16_t lStartAddress = 20;
    uint16_t lStartAddressBigEndian = LtIntUtilities::SwapEndian( lStartAddress );
    uint16_t lRegisterNumber = 10;
    uint16_t lRegisterNumberBigEndian = LtIntUtilities::SwapEndian( lRegisterNumber );
    uint8_t lRawRequest[6] = { mConnectionInfoModbus->GetModbusAddr(), 0x4};
    *( uint16_t * )( &lRawRequest[2] ) = lStartAddressBigEndian; //First two values are starting register address
    *( uint16_t * )( &lRawRequest[4] ) = lRegisterNumberBigEndian; //The other two are : Quantity of Input Registers

    uint8_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 6 );
    uint32_t lSizeToReceive = static_cast<uint32_t>( sizeof( LeddarConnection::LdConnectionModbuStructures::sModbusHeader ) +
                              offsetof( LeddarConnection::LdConnectionModbuStructures::sModbusReadDataAnswer, mData ) +
                              MODBUS_CRC_SIZE +
                              sizeof( LtComLeddarOneModbus::sLeddarOneDetections ) );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lResponse, lSizeToReceive );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( ONE_WAIT_AFTER_REQUEST );

    if( lReceivedSize <= MODBUS_DATA_OFFSET )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small: " + LtStringUtils::IntToString( lReceivedSize ) );
    }
    else if( lReceivedSize < MODBUS_DATA_OFFSET + sizeof( LtComLeddarOneModbus::sLeddarOneDetections ) + 1 ) //+1 because 3rd byte is the byte count
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Not enough data received, size: " + LtStringUtils::IntToString( lReceivedSize ) );
    }
    else
    {
        LtComLeddarOneModbus::sLeddarOneDetections *lDetections = reinterpret_cast<LtComLeddarOneModbus::sLeddarOneDetections *>( &lResponse[MODBUS_DATA_OFFSET + 1] );

        uint32_t lTimestamp = ( LtIntUtilities::SwapEndian( lDetections->mTimeStampMSB ) << 16 ) + LtIntUtilities::SwapEndian( lDetections->mTimeStampLSB );
        LeddarConnection::LdResultEchoes *lEchoes = GetResultEchoes();

        if( lEchoes->GetTimestamp( LeddarConnection::B_GET ) == lTimestamp )
            return false;

        lEchoes->SetTimestamp( lTimestamp );

        GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )->ForceRawValue( 0, LtIntUtilities::SwapEndian( lDetections->mTemperature ) );
        GetResultStates()->SetTimestamp( lTimestamp );

        uint16_t lEchoCount = LtIntUtilities::SwapEndian( lDetections->mNumberDetections );
        lEchoes->SetEchoCount( lEchoCount );

        ( *lEchoes->GetEchoes( LeddarConnection::B_SET ) )[0].mDistance = LtIntUtilities::SwapEndian( lDetections->mDistance1 );
        ( *lEchoes->GetEchoes( LeddarConnection::B_SET ) )[0].mAmplitude = LtIntUtilities::SwapEndian( lDetections->mAmplitude1 );
        ( *lEchoes->GetEchoes( LeddarConnection::B_SET ) )[1].mDistance = LtIntUtilities::SwapEndian( lDetections->mDistance2 );
        ( *lEchoes->GetEchoes( LeddarConnection::B_SET ) )[1].mAmplitude = LtIntUtilities::SwapEndian( lDetections->mAmplitude2 );
        ( *lEchoes->GetEchoes( LeddarConnection::B_SET ) )[2].mDistance = LtIntUtilities::SwapEndian( lDetections->mDistance3 );
        ( *lEchoes->GetEchoes( LeddarConnection::B_SET ) )[2].mAmplitude = LtIntUtilities::SwapEndian( lDetections->mAmplitude3 );
    }

    GetResultEchoes()->Swap();
    GetResultEchoes()->UpdateFinished();
    return true;
}

/// *****************************************************************************
/// Function: LdSensorOneModbus::GetConfig
///
/// \brief   Get config properties from the sensor
///
/// \author  David Levy
///
/// \since   September 2017
/// *****************************************************************************
void
LdSensorOneModbus::GetConfig( void )
{
    //Register 3, is readable/writable but currently unused
    //All other registers are either used or not readable
    uint16_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH / 2] = { 0 };
    mInterface->ReadRegisters( 0, 5, lResponse );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( ONE_WAIT_AFTER_REQUEST );

    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetValue( 0, lResponse[0] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetValue( 0, lResponse[1] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetValue( 0, lResponse[2] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetClean();
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->SetValue( 0, lResponse[4] );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->SetClean();

    memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
    mInterface->ReadRegisters( 29, 2, lResponse );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( ONE_WAIT_AFTER_REQUEST );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE )->SetValueIndex( 0, lResponse[0] );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetValue( 0, lResponse[1] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetClean();

    if( mParameterVersion > 1 )
    {
        memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
        mInterface->ReadRegisters( 6, 2, lResponse );
        LeddarUtils::LtTimeUtils::WaitBlockingMicro( ONE_WAIT_AFTER_REQUEST );
        GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_PWR_ENABLE )->SetValue( 0, lResponse[0] != 0 );
        GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_PWR_ENABLE )->SetClean();
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CHANGE_DELAY )->SetValue( 0, lResponse[1] );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CHANGE_DELAY )->SetClean();

        memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
        mInterface->ReadRegisters( 11, 1, lResponse );
        LeddarUtils::LtTimeUtils::WaitBlockingMicro( ONE_WAIT_AFTER_REQUEST );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetValue( 0, static_cast<int16_t>( lResponse[0] ) );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetClean();
    }

    if( mParameterVersion > 2 )
    {
        memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
        mInterface->ReadRegisters( 9, 5, lResponse );
        LeddarUtils::LtTimeUtils::WaitBlockingMicro( ONE_WAIT_AFTER_REQUEST );
        GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_REMOVAL_ENABLE )->SetValue( 0, lResponse[0] != 0 );
        GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_REMOVAL_ENABLE )->SetClean();
        GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_UPDATE_ENABLE )->SetValue( 0, lResponse[1] != 0 );
        GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_UPDATE_ENABLE )->SetClean();
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_UPDATE_RATE )->SetValue( 0, lResponse[3] );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_UPDATE_RATE )->SetClean();
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_UPDATE_AVERAGE )->SetValue( 0, lResponse[4] );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_UPDATE_AVERAGE )->SetClean();
    }
}

/// *****************************************************************************
/// Function: LdSensorOneModbus::SetConfig
///
/// \brief   Set config properties on the sensor
///
/// \author  David Levy
///
/// \since   September 2017
/// *****************************************************************************
void
LdSensorOneModbus::SetConfig( void )
{
    std::vector<LeddarCore::LdProperty *> lProperties = mProperties->FindPropertiesByCategories( LeddarCore::LdProperty::CAT_CONFIGURATION );

    for( std::vector<LeddarCore::LdProperty *>::iterator lPropertyIter = lProperties.begin(); lPropertyIter != lProperties.end(); ++lPropertyIter )
    {
        if( ( *lPropertyIter )->Modified() )
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
                    else if( ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE )
                        lValue = static_cast<int>( dynamic_cast< LeddarCore::LdEnumProperty * >( ( *lPropertyIter ) )->ValueIndex() );
                    else
                        assert( false ); //Check for other enum if we return the value or the index

                    break;

                case LeddarCore::LdProperty::TYPE_FLOAT:
                    assert( false ); //No text property available at the moment
                    break;

                case LeddarCore::LdProperty::TYPE_INTEGER:
                    lValue = dynamic_cast<LeddarCore::LdIntegerProperty *>( ( *lPropertyIter ) )->ValueT<int32_t>();
                    break;

                case LeddarCore::LdProperty::TYPE_TEXT:
                    lValue = 0;
                    assert( false ); //No text property available at the moment
                    break;

                default:
                    lValue = 0;
                    assert( false );
                    break;
            }

            mInterface->WriteRegister( ( *lPropertyIter )->GetDeviceId(), lValue );
            ( *lPropertyIter )->SetClean();
            LeddarUtils::LtTimeUtils::WaitBlockingMicro( ONE_WAIT_AFTER_REQUEST );
        }
    }
}

/// *****************************************************************************
/// Function: LdSensorOneModbus::WriteConfig
///
/// \brief   Write config properties to permanent memory
///
/// \author  Patrick Boulay
///
/// \since   November 2017
/// *****************************************************************************

void
LdSensorOneModbus::WriteConfig( void )
{
    uint8_t lRawRequest[ 2 ] = { mConnectionInfoModbus->GetModbusAddr(), 0x46 };
    uint8_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 2 );
    mInterface->ReceiveRawConfirmation( lResponse, 0 );
}

/// *****************************************************************************
/// Function: LdSensorOneModbus::GetConstants
///
/// \brief   Get constants properties on the sensor
///
/// \author  David Levy
///
/// \since   September 2017
/// *****************************************************************************
void
LdSensorOneModbus::GetConstants( void )
{
    UpdateConstants();

    //Then ask the sensor for other constants
    uint8_t lRawRequest[2] = { mConnectionInfoModbus->GetModbusAddr(), 0x11 };
    uint8_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 2 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lResponse, 0 );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( ONE_WAIT_AFTER_REQUEST );

    if( lReceivedSize <= MODBUS_DATA_OFFSET )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "No data received." );
    }
    else if( lReceivedSize < MODBUS_DATA_OFFSET + lResponse[MODBUS_DATA_OFFSET] )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small, received: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) + " expected: " +
                                               LeddarUtils::LtStringUtils::IntToString( lResponse[MODBUS_DATA_OFFSET] ) );
    }

    LtComLeddarOneModbus::sLeddarOneServerId *lDeviceInfo = reinterpret_cast<LtComLeddarOneModbus::sLeddarOneServerId *>( &lResponse[MODBUS_DATA_OFFSET] );

    if( lDeviceInfo->mRunStatus != 0xFF )
    {
        throw LeddarException::LtInfoException( "Wrong run status. Received " + LeddarUtils::LtStringUtils::IntToString( lDeviceInfo->mRunStatus, 16 ) + " expected: 0xFF" );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SOFTWARE_PART_NUMBER )->ForceValue( 0, lDeviceInfo->mSoftwarePartNumber );
    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_PART_NUMBER )->ForceValue( 0, lDeviceInfo->mHardwarePartNumber );

    LeddarCore::LdIntegerProperty *lFirmwareVersion = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FIRMWARE_VERSION_INT );
    lFirmwareVersion->SetCount( 4 );
    lFirmwareVersion->ForceValue( 0, lDeviceInfo->mFirmwareVersion[0] );
    lFirmwareVersion->ForceValue( 1, lDeviceInfo->mFirmwareVersion[1] );
    lFirmwareVersion->ForceValue( 2, lDeviceInfo->mFirmwareVersion[2] );
    lFirmwareVersion->ForceValue( 3, lDeviceInfo->mFirmwareVersion[3] );

    // Guess the parameter version from what we know of software versions.
    if( ( lDeviceInfo->mFirmwareVersion[ 3 ] > 6 && lDeviceInfo->mFirmwareVersion[ 3 ] < 2560 )
            || lDeviceInfo->mFirmwareVersion[ 3 ] >= 3079 )
    {
        mParameterVersion = 4;
    }
    else if( ( lDeviceInfo->mFirmwareVersion[3] > 3 && lDeviceInfo->mFirmwareVersion[3] < 2560 )
             || lDeviceInfo->mFirmwareVersion[3] >= 3041 )
    {
        mParameterVersion = 3;
    }
    else if( lDeviceInfo->mFirmwareVersion[3] > 3021 && lDeviceInfo->mFirmwareVersion[3] <= 3034 )
    {
        mParameterVersion = 2;
    }

    if( mParameterVersion >= 4 )
        GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SERIAL_NUMBER )->ForceValue( 0, lDeviceInfo->mSerialNumberV2 );
    else
        GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SERIAL_NUMBER )->ForceValue( 0, lDeviceInfo->mSerialNumber );

    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FPGA_VERSION )->ForceValue( 0, lDeviceInfo->mFPGAVersion );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_OPTIONS )->ForceValue( 0, lDeviceInfo->mDeviceOptions );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->ForceValue( 0, lDeviceInfo->mDeviceId );

    if( GetConnection()->GetDeviceType() == 0 )
        GetConnection()->SetDeviceType( lDeviceInfo->mDeviceId );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorOneModbus::UpdateConstants()
///
/// \brief  Updates the constants with hard coded values
///
/// \author David Levy
/// \date   January 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorOneModbus::UpdateConstants()
{
    //Hard coded value
    LeddarCore::LdIntegerProperty *lHSeg = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT );
    lHSeg->ForceValue( 0, 1 );
    LeddarCore::LdIntegerProperty *lMaxEchoesByChan = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL );
    lMaxEchoesByChan->ForceValue( 0, ONE_MAX_SERIAL_DETECTIONS );
    LeddarCore::LdIntegerProperty *lDistScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE );
    lDistScale->ForceValue( 0, ONE_DISTANCE_SCALE );
    LeddarCore::LdIntegerProperty *lTempScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE );
    lTempScale->ForceValue( 0, ONE_TEMPERATURE_SCALE );
    LeddarCore::LdIntegerProperty *lAmpScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE );
    lAmpScale->ForceValue( 0, ONE_AMPLITUDE_SCALE );

    mEchoes.Init( ONE_DISTANCE_SCALE, ONE_AMPLITUDE_SCALE, ONE_MAX_SERIAL_DETECTIONS );
    mStates.Init( ONE_TEMPERATURE_SCALE, 1 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorOneModbus::InitProperties( void )
///
/// \brief   Create properties for this specific sensor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorOneModbus::InitProperties( void )
{
    using namespace LeddarCore;

    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, 0, 16, LeddarCore::LdTextProperty::TYPE_ASCII, "Serial number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, 0, 11, LeddarCore::LdTextProperty::TYPE_ASCII, "Part  number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER, 0, 11, LeddarCore::LdTextProperty::TYPE_ASCII,
                              "Software part number" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_INT, 0, 2, "Firmware version" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, 0, 2, "FPGA version" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS, 0, 4, "Device options" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_DISTANCE_SCALE, 0, 2, "Distance scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FILTERED_AMP_SCALE, 0, 2, "Amplitude scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_TEMPERATURE_SCALE, 0, 2, "Temperature scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_NONE, LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL, 0, 1, "Maximum echoes per channel" ) );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_MODBUS );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();

    mProperties->GetFloatProperty( LdPropertyIds::ID_HFOV )->ForceValue( 0, 3 );
    mProperties->GetFloatProperty( LdPropertyIds::ID_HFOV )->SetClean();

    //For the CAT_CONFIGURATION, the device Id is the register address of that property for modbus command 0x03 and 0x06
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACCUMULATION_EXP,
                              LtComLeddarOneModbus::DID_ACCUMULATION_EXP, 2, "Accumulation exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_OVERSAMPLING_EXP,
                              LtComLeddarOneModbus::DID_OVERSAMPLING_EXP, 2, "Oversampling exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_BASE_POINT_COUNT,
                              LtComLeddarOneModbus::DID_BASE_POINT_COUNT, 2, "Base point count" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_INTENSITY,
                              LtComLeddarOneModbus::DID_LED_INTENSITY, 1, false, "Led intensity" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_AUTO_PWR_ENABLE,
                              LtComLeddarOneModbus::DID_LED_AUTO_PWR_ENABLE, "Auto led power enable" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CHANGE_DELAY,
                              LtComLeddarOneModbus::DID_CHANGE_DELAY, 2, "Change delay (in frame)" ) );
    //To disable smoothing, set value to min value -1 (i.e. -17 see LtComLeddarOneModbus.h)
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PRECISION,
                              LtComLeddarOneModbus::DID_PRECISION, 2, "Smoothing", true ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_NOISE_REMOVAL_ENABLE,
                              LtComLeddarOneModbus::DID_STATIC_NOISE_REMOVAL_ENABLE, "Static noise removal enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_NOISE_UPDATE_ENABLE,
                              LtComLeddarOneModbus::DID_STATIC_NOISE_UPDATE_ENABLE, "Static noise removal adaptive learning" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_NOISE_UPDATE_RATE,
                              LtComLeddarOneModbus::DID_STATIC_NOISE_UPDATE_RATE, 2, "Static noise removal update rate" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_NOISE_UPDATE_AVERAGE,
                              LtComLeddarOneModbus::DID_STATIC_NOISE_UPDATE_AVERAGE, 2, "Static noise removal averaging" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE,
                              LtComLeddarOneModbus::DID_COM_SERIAL_PORT_BAUDRATE, 1, true, "Modbus baudrate" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS,
                              LtComLeddarOneModbus::DID_COM_SERIAL_PORT_ADDRESS, 1, "Modbus address" ) );


    // Set limits and enums
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetLimits( 1, MODBUS_MAX_ADDR );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_ACCUMULATION_EXP )->SetLimits( 0, ONE_MAX_ACC_EXP );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_OVERSAMPLING_EXP )->SetLimits( 0, ONE_MAX_OVERS_EXP );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_PRECISION )->SetLimits( ONE_MIN_SMOOTHING, ONE_MAX_SMOOTHING );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_BASE_POINT_COUNT )->SetLimits( LtComLeddarOneModbus::ONE_MIN_BASE_POINT_COUNT, LtComLeddarOneModbus::ONE_MAX_BASE_POINT_COUNT );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_STATIC_NOISE_UPDATE_RATE )->SetLimits( LtComLeddarOneModbus::ONE_MIN_PULSE_NOISE_RATE, LtComLeddarOneModbus::ONE_MAX_PULSE_NOISE_RATE );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_STATIC_NOISE_UPDATE_AVERAGE )->SetLimits( LtComLeddarOneModbus::ONE_MIN_PULSE_NOISE_AVG, LtComLeddarOneModbus::ONE_MAX_PULSE_NOISE_AVG );

    LeddarCore::LdEnumProperty *lSerialBaud = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE );
    lSerialBaud->AddEnumPair( 0, "115200" );
    lSerialBaud->AddEnumPair( 1, "9600" );
    lSerialBaud->AddEnumPair( 2, "19200" );
    lSerialBaud->AddEnumPair( 3, "38400" );
    lSerialBaud->AddEnumPair( 4, "57600" );
    lSerialBaud->AddEnumPair( 5, "115200" );
    LeddarCore::LdEnumProperty *lLedPower = GetProperties()->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY );
    lLedPower->AddEnumPair( 10, "10" );
    lLedPower->AddEnumPair( 20, "20" );
    lLedPower->AddEnumPair( 35, "35" );
    lLedPower->AddEnumPair( 55, "55" );
    lLedPower->AddEnumPair( 75, "75" );
    lLedPower->AddEnumPair( 100, "100" );

    //States
    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_RS_SYSTEM_TEMP, 0, 2,
            ONE_TEMPERATURE_SCALE, 1, "System Temperature" ) );
}

#endif