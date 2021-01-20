/// *****************************************************************************
/// Module..: Leddar
///
/// \file    LdSensorM16Modbus.cpp
///
/// \brief   Definition of M16 sensor using modbus protocol
///
/// \author  David Levy
///
/// \since   September 2017
///
/// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
/// *****************************************************************************

#include "LdSensorM16Modbus.h"
#if defined(BUILD_M16) && defined(BUILD_MODBUS)

#include "comm/Modbus/LtComLeddarM16Modbus.h"
#include "comm/LtComLeddarTechPublic.h"

#include "LdPropertyIds.h"

#include "LtExceptions.h"
#include "LtIntUtilities.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"

#include <string.h>

using namespace LeddarDevice;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdSensorM16Modbus::LdSensorM16Modbus( LeddarConnection::LdConnection *aConnection )
///
/// \brief  Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param [in,out] aConnection If non-null, the connection.
///
/// \author Patrick Boulay
/// \date   April 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
LdSensorM16Modbus::LdSensorM16Modbus( LeddarConnection::LdConnection *aConnection ) :
    LdSensor( aConnection ),
    mConnectionInfoModbus( nullptr ),
    mInterface( nullptr ),
    mUse0x6A( true )
{
    using namespace LeddarCore;

    if( aConnection != nullptr )
    {
        mConnectionInfoModbus = dynamic_cast< const LeddarConnection::LdConnectionInfoModbus * >( aConnection->GetConnectionInfo() );
        mInterface = dynamic_cast< LeddarConnection::LdLibModbusSerial * >( aConnection );
    }

    InitProperties();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdSensorM16Modbus::~LdSensorM16Modbus()
///
/// \brief  Destructor.
///
/// \author David Levy
/// \date   September 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
LdSensorM16Modbus::~LdSensorM16Modbus()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorM16Modbus::Connect( void )
///
/// \brief  Connect to the sensor
///
/// \author Patrick Boulay
/// \date   April 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorM16Modbus::Connect( void )
{
    LdDevice::Connect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LdSensorM16Modbus::GetEchoes0x41( void )
///
/// \brief  Get the echoes using 0x41 function
///
/// \exception  LeddarException::LtComException Thrown when we don't receive the expected amount of data.
///
/// \return Return true if there is new echoes.
///
/// \author David Levy
/// \date   September 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
LdSensorM16Modbus::GetEchoes0x41( void )
{
    uint8_t lRawRequest[ 2 ] = { mConnectionInfoModbus->GetModbusAddr(), 0x41 };
    uint8_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 2 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmationLT( lResponse, GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->ValueT<uint16_t>() );

    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );

    if( lReceivedSize <= MODBUS_DATA_OFFSET )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) );
    }

    uint8_t lEchoCount = lResponse[MODBUS_DATA_OFFSET];

    if( lReceivedSize > MODBUS_DATA_OFFSET + lEchoCount * sizeof( LtComLeddarM16Modbus::sLeddarM16Detections0x41 ) + 6 )
    {
        uint32_t lLastTimeStamp = mEchoes.GetTimestamp();
        uint32_t lTimeStamp = *reinterpret_cast<uint32_t *>( &lResponse[MODBUS_DATA_OFFSET + lEchoCount * 5u] );

        if( lLastTimeStamp != lTimeStamp )
        {
            mEchoes.SetEchoCount( lEchoCount );
            LtComLeddarM16Modbus::sLeddarM16Detections0x41 *lDetections = reinterpret_cast< LtComLeddarM16Modbus::sLeddarM16Detections0x41 * >( &lResponse[MODBUS_DATA_OFFSET + 1] );

            for( uint8_t i = 0; ( i < lEchoCount ) && ( i < LtComLeddarM16Modbus::M16_MAX_SERIAL_DETECTIONS ); ++i )
            {
                LeddarConnection::LdEcho &lEcho = ( *mEchoes.GetEchoes( LeddarConnection::B_SET ) )[i];
                lEcho.mDistance = lDetections->mDistance;
                lEcho.mAmplitude = lDetections->mAmplitude;
                lEcho.mFlag = lDetections->mFlags & 0x0F;
                lEcho.mChannelIndex = ( lDetections->mFlags & 0xF0 ) >> 4;
                lDetections++;
            }

            uint16_t lLedPower = *reinterpret_cast<uint16_t *>( &lResponse[MODBUS_DATA_OFFSET + lEchoCount * sizeof( LtComLeddarM16Modbus::sLeddarM16Detections0x41 ) + 4] );
            mEchoes.SetTimestamp( lTimeStamp );
            mEchoes.SetCurrentLedPower( lLedPower );
            ComputeCartesianCoordinates();
            mEchoes.Swap();
            mEchoes.UpdateFinished();
        }
        else
        {
            return false;
        }
    }
    else
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Not enough data received, size: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) );
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LdSensorM16Modbus::GetEchoes0x6A( void )
///
/// \brief  Get the echoes using 0x6A function
///
/// \exception  LeddarException::LtComException Thrown when we don't receive the expected amount of data.
///
/// \return Return true if there is new echoes.
///
/// \author David Levy
/// \date   December 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
LdSensorM16Modbus::GetEchoes0x6A( void )
{
    uint8_t lRawRequest[ 2 ] = { mConnectionInfoModbus->GetModbusAddr(), 0x6A };
    uint8_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 2 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmationLT( lResponse, GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->ValueT<uint16_t>() );

    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );

    if( lReceivedSize <= MODBUS_DATA_OFFSET )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) );
    }

    uint8_t lEchoCount = lResponse[MODBUS_DATA_OFFSET];

    if( lReceivedSize > MODBUS_DATA_OFFSET + lEchoCount * sizeof( LtComLeddarM16Modbus::sLeddarM16Detections0x6A ) + 6 )
    {
        uint32_t lLastTimeStamp = mEchoes.GetTimestamp();
        uint32_t lTimeStamp = *reinterpret_cast<uint32_t *>( &lResponse[MODBUS_DATA_OFFSET + lEchoCount * 5u] );

        if( lLastTimeStamp != lTimeStamp )
        {
            mEchoes.SetEchoCount( lEchoCount );
            LtComLeddarM16Modbus::sLeddarM16Detections0x6A *lDetections = reinterpret_cast< LtComLeddarM16Modbus::sLeddarM16Detections0x6A * >( &lResponse[MODBUS_DATA_OFFSET + 1] );

            for( uint8_t i = 0; ( i < lEchoCount ) && ( i < LtComLeddarM16Modbus::M16_MAX_SERIAL_DETECTIONS ); ++i )
            {
                LeddarConnection::LdEcho &lEcho = ( *mEchoes.GetEchoes( LeddarConnection::B_SET ) )[i];
                lEcho.mDistance = lDetections->mDistance;
                lEcho.mAmplitude = lDetections->mAmplitude;
                lEcho.mFlag = lDetections->mFlags;
                lEcho.mChannelIndex = lDetections->mSegment;
                lDetections++;
            }

            uint16_t lLedPower = *reinterpret_cast<uint16_t *>( &lResponse[MODBUS_DATA_OFFSET + lEchoCount * sizeof( LtComLeddarM16Modbus::sLeddarM16Detections0x6A ) + 4] );
            mEchoes.SetTimestamp( lTimeStamp );
            mEchoes.SetCurrentLedPower( lLedPower );
            ComputeCartesianCoordinates();
            mEchoes.Swap();
            mEchoes.UpdateFinished();
        }
        else
        {
            return false;
        }
    }
    else
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Not enough data received, size: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) );
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorM16Modbus::GetStates( void )
///
/// \brief  Get the states
///
/// \author David Levy
/// \date   September 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorM16Modbus::GetStates( void )
{
    uint16_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH / 2] = { 0 };
    mInterface->ReadInputRegisters( 0, 1, lResponse );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );

    GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )->ForceRawValue( 0, lResponse[0] );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorM16Modbus::GetConfig( void )
///
/// \brief  Get config properties from the sensor
///
/// \author David Levy
/// \date   September 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorM16Modbus::GetConfig( void )
{
    uint16_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH / 2] = { 0 };

    if( mInterface->GetDeviceType() == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16 )
    {
        mInterface->ReadRegisters( LtComLeddarM16Modbus::DID_REFRESH_RATE, 1, lResponse );
        LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );
        GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE )->SetValue( 0, lResponse[0] );
        GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE )->SetClean();
    }
    else
    {
        mInterface->ReadRegisters( LtComLeddarM16Modbus::DID_ACCUMULATION_EXP, 3, lResponse );
        LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetValue( 0, lResponse[0] );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->SetClean();
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetValue( 0, lResponse[1] );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->SetClean();
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetValue( 0, lResponse[2] );
        GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->SetClean();
    }



    memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
    mInterface->ReadRegisters( LtComLeddarM16Modbus::DID_THRESHOLD_OFFSET, 5, lResponse );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD )->SetRawValue( 0, lResponse[0] );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY_OLD )->SetClean();
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->SetValue( 0, lResponse[1] );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY )->SetClean();
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS )->SetValue( 0, lResponse[2] );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_ACQ_OPTIONS )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CHANGE_DELAY )->SetValue( 0, lResponse[3] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CHANGE_DELAY )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES )->SetValue( 0, lResponse[4] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES )->SetClean();

    memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
    mInterface->ReadRegisters( LtComLeddarM16Modbus::DID_PRECISION, 1, lResponse );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetValue( 0, lResponse[0] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION )->SetClean();

    memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
    mInterface->ReadRegisters( LtComLeddarM16Modbus::DID_COM_SERIAL_PORT_ECHOES_RES, 2, lResponse );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES )->SetValue( 0, lResponse[0] );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES )->SetClean();
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE_COM )->SetValue( 0, lResponse[1] );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE_COM )->SetClean();

    memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
    mInterface->ReadRegisters( LtComLeddarM16Modbus::DID_SEGMENT_ENABLE_DEVICE, 1, lResponse );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->SetValue( 0, lResponse[0] );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE )->SetClean();

    memset( lResponse, 0, LTMODBUS_RTU_MAX_ADU_LENGTH );
    mInterface->ReadRegisters( LtComLeddarM16Modbus::DID_COM_SERIAL_PORT_STOP_BITS, 4, lResponse );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS )->SetValue( 0, lResponse[0] );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS )->SetClean();
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_PARITY )->SetValue( 0, lResponse[1] );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_PARITY )->SetClean();
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE )->SetValueIndex( 0, lResponse[2] );
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE )->SetClean();
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetValue( 0, lResponse[3] );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetClean();

    UpdateConstants();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorM16Modbus::SetConfig( void )
///
/// \brief  Set config properties on the sensor
///
/// \exception  std::logic_error    Raised when an unhandled case for float properties happens.
///
/// \author David Levy
/// \date   September 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorM16Modbus::SetConfig( void )
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
                    if( ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_LED_INTENSITY ||
                            ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES ||
                            ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_PARITY ||
                            ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS ||
                            ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_REFRESH_RATE )
                        lValue = dynamic_cast< LeddarCore::LdEnumProperty * >( ( *lPropertyIter ) )->Value();
                    else if( ( *lPropertyIter )->GetId() == LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE )
                        lValue = static_cast<int>( dynamic_cast< LeddarCore::LdEnumProperty * >( ( *lPropertyIter ) )->ValueIndex() );
                    else
                        assert( false ); //Check for other enum if we return the value or the index

                    break;

                case LeddarCore::LdProperty::TYPE_FLOAT:
                    if( dynamic_cast< LeddarCore::LdFloatProperty * >( *lPropertyIter )->GetScale() != 0 )
                    {
                        lValue = dynamic_cast< LeddarCore::LdFloatProperty * >( *lPropertyIter )->RawValue();
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
                    lValue = 0 ;
                    assert( false ); //No text property available at the moment
                    break;

                default:
                    lValue = 0 ;
                    assert( false ); //No text property available at the moment
                    break;
            }

            mInterface->WriteRegister( ( *lPropertyIter )->GetDeviceId(), lValue );
            ( *lPropertyIter )->SetClean();
            LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorM16Modbus::GetConstants( void )
///
/// \brief  Get constants properties on the sensor
///
/// \exception  LeddarException::LtComException     Thrown when we don't receive the expected amount of data.
/// \exception  LeddarException::LtInfoException    Thrown when the device has an unexpected run status.
///
/// \author Patrick Boulay
/// \date   April 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorM16Modbus::GetConstants( void )
{
    LeddarCore::LdIntegerProperty *lDistScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE );
    LeddarCore::LdIntegerProperty *lAmpScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE );

    lDistScale->ForceValue( 0, LtComLeddarM16Modbus::M16_DISTANCE_SCALE );
    lAmpScale->ForceValue( 0, LtComLeddarM16Modbus::M16_AMPLITUDE_SCALE );

    mEchoes.Init( lDistScale->ValueT<uint32_t>(), lAmpScale->ValueT<uint32_t>(), LtComLeddarM16Modbus::M16_MAX_SERIAL_DETECTIONS );
    mStates.Init( LtComLeddarM16Modbus::M16_TEMPERATURE_SCALE, 1 );

    //0x11 get the device info
    uint8_t lRawRequest[2] = { mConnectionInfoModbus->GetModbusAddr(), 0x11 };
    uint8_t lResponse[LTMODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    mInterface->SendRawRequest( lRawRequest, 2 );
    size_t lReceivedSize = mInterface->ReceiveRawConfirmation( lResponse, 0 );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( LtComLeddarM16Modbus::M16_WAIT_AFTER_REQUEST );

    if( lReceivedSize <= MODBUS_DATA_OFFSET )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "No data received." );
    }
    else if( lReceivedSize < static_cast<uint8_t>( lResponse[MODBUS_DATA_OFFSET] ) )
    {
        mInterface->Flush();
        throw LeddarException::LtComException( "Received size too small, received: " + LeddarUtils::LtStringUtils::IntToString( lReceivedSize ) + " expected: " +
                                               LeddarUtils::LtStringUtils::IntToString( static_cast<uint8_t>( lResponse[MODBUS_DATA_OFFSET] ) ) );
    }

    LtComLeddarM16Modbus::sLeddarM16ServerId *lDeviceInfo = reinterpret_cast<LtComLeddarM16Modbus::sLeddarM16ServerId *>( &lResponse[MODBUS_DATA_OFFSET] );

    if( lDeviceInfo->mRunStatus != 0xFF )
    {
        throw LeddarException::LtInfoException( "Wrong run status. Received " + LeddarUtils::LtStringUtils::IntToString( lDeviceInfo->mRunStatus, 16 ) + " expected: 0xFF" );
    }

    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SERIAL_NUMBER )->ForceValue( 0, lDeviceInfo->mSerialNumber );
    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_DEVICE_NAME )->ForceRawStorage( reinterpret_cast<uint8_t *>( &( lDeviceInfo->mDeviceName ) ), 1,
            LtComLeddarM16Modbus::M16_DEVICE_NAME_SIZE );
    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SOFTWARE_PART_NUMBER )->ForceValue( 0, lDeviceInfo->mSoftwarePartNumber );
    GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_PART_NUMBER )->ForceValue( 0, lDeviceInfo->mHardwarePartNumber );

    LeddarCore::LdIntegerProperty *lFirmwareVersion = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FIRMWARE_VERSION_INT );
    lFirmwareVersion->SetCount( 4 );
    lFirmwareVersion->ForceValue( 0, lDeviceInfo->mFirmwareVersion[0] );
    lFirmwareVersion->ForceValue( 1, lDeviceInfo->mFirmwareVersion[1] );
    lFirmwareVersion->ForceValue( 2, lDeviceInfo->mFirmwareVersion[2] );
    lFirmwareVersion->ForceValue( 3, lDeviceInfo->mFirmwareVersion[3] );

    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FPGA_VERSION )->ForceValue( 0, lDeviceInfo->mFPGAVersion );
    GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_OPTIONS )->ForceValue( 0, lDeviceInfo->mDeviceOptions );
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->ForceValue( 0, lDeviceInfo->mDeviceId );

    if( GetConnection()->GetDeviceType() == 0 )
        GetConnection()->SetDeviceType( lDeviceInfo->mDeviceId );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16Modbus::UpdateConstants( void )
///
/// \brief  Updates the constants
///
/// \author David Levy
/// \date   May 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorM16Modbus::UpdateConstants( void )
{
    GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ForceValue( 0,
            GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES )->Value() );
    mEchoes.SetDistanceScale( GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES )->Value() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorM16Modbus::InitProperties( void )
///
/// \brief  Create properties for this specific sensor.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorM16Modbus::InitProperties( void )
{
    using namespace LeddarCore;

    //constants
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, 0, LtComLeddarM16Modbus::M16_SERIAL_NBR_SIZE,
                              LdTextProperty::TYPE_ASCII,
                              "Serial Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_NAME, 0, LtComLeddarM16Modbus::M16_DEVICE_NAME_SIZE,
                              LdTextProperty::TYPE_UTF16,
                              "Device name" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, 0, LtComLeddarM16Modbus::M16_HW_PART_NBR_SIZE,
                              LdTextProperty::TYPE_ASCII,
                              "Hardware part number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER, 0, LtComLeddarM16Modbus::M16_SW_PART_NBR_SIZE,
                              LdTextProperty::TYPE_ASCII, "Software part number" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_INT, 0, 2, "Firmware version" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, 0, 2, "FPGA version" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS, 0, 4, "Device options" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_DISTANCE_SCALE, 0, 4, "Distance scaling" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FILTERED_AMP_SCALE, 0, 4, "Amplitude scaling" ) );


    // Set hard coded constants
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_MODBUS );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ForceValue( 0, 1 );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->ForceValue( 0, 16 );

    //config
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SENSIVITY_OLD,
                              LtComLeddarM16Modbus::DID_THRESHOLD_OFFSET, 2, LtComLeddarM16Modbus::M16_SENSITIVITY_SCALE, 3, "Threshold offset" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_INTENSITY,
                              LtComLeddarM16Modbus::DID_LED_INTENSITY, 1, false, "Led power %" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACQ_OPTIONS,
                              LtComLeddarM16Modbus::DID_ACQ_OPTIONS, 2, "Bit field of acquisition options. See eLtCommPlatformM16ModbusAcqOptions" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_CHANGE_DELAY,
                              LtComLeddarM16Modbus::DID_CHANGE_DELAY, 2, "Change delay (in frame) for automatic led power" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES,
                              LtComLeddarM16Modbus::DID_COM_SERIAL_PORT_MAX_ECHOES, 1, "Modbus maximum detections returned by command 0x41" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PRECISION,
                              LtComLeddarM16Modbus::DID_PRECISION, 2, "Smoothing" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES,
                              LtComLeddarM16Modbus::DID_COM_SERIAL_PORT_ECHOES_RES, 2, true, "Distance resolution" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SEGMENT_ENABLE_COM,
                              LtComLeddarM16Modbus::DID_SEGMENT_ENABLE_COM, 2, "Enable / disable communication about selected channels" ) );
    GetProperties()->GetBitProperty( LdPropertyIds::ID_SEGMENT_ENABLE_COM )->SetLimit( ( 1 << GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->Value() ) - 1 );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE, LdPropertyIds::ID_SEGMENT_ENABLE,
                              LtComLeddarM16Modbus::DID_SEGMENT_ENABLE_DEVICE, 2, "Enable / disable selected channels pair on the device (enable = 0)" ) );
    GetProperties()->GetBitProperty( LdPropertyIds::ID_SEGMENT_ENABLE )->SetLimit( ( 1 << GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->Value() ) - 1 );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS,
                              LtComLeddarM16Modbus::DID_COM_SERIAL_PORT_STOP_BITS, 1, true, "Modbus stop bit" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_PARITY,
                              LtComLeddarM16Modbus::DID_COM_SERIAL_PORT_PARITY, 1, true, "Modbus parity" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE,
                              LtComLeddarM16Modbus::DID_COM_SERIAL_PORT_BAUDRATE, 4, true, "Modbus baudrate" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS,
                              LtComLeddarM16Modbus::DID_COM_SERIAL_PORT_ADDRESS, 1, "Modbus address" ) );


    //IS16 only
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_REFRESH_RATE,
                              LtComLeddarM16Modbus::DID_REFRESH_RATE, 2, true, "Target refresh rate. Formula is 12800/2^n" ) );
    LdEnumProperty *lRefreshRate = GetProperties()->GetEnumProperty( LdPropertyIds::ID_REFRESH_RATE );
    lRefreshRate->AddEnumPair( 8, "50 Hz" );
    lRefreshRate->AddEnumPair( 9, "25 Hz" );
    lRefreshRate->AddEnumPair( 10, "12.5 Hz" );
    lRefreshRate->AddEnumPair( 11, "6.25 Hz" );
    lRefreshRate->AddEnumPair( 12, "3.125 Hz" );
    lRefreshRate->AddEnumPair( 13, "1.5625 Hz" );

    // for M16, not available on IS16
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACCUMULATION_EXP,
                              LtComLeddarM16Modbus::DID_ACCUMULATION_EXP, 2, "Accumulation exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_OVERSAMPLING_EXP,
                              LtComLeddarM16Modbus::DID_OVERSAMPLING_EXP, 2, "Oversampling exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_BASE_POINT_COUNT,
                              LtComLeddarM16Modbus::DID_BASE_POINT_COUNT, 2, "Base point count, impact max detection distance" ) );

    // Set limits and enums
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ADDRESS )->SetLimits( 1, MODBUS_MAX_ADDR );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_COM_SERIAL_PORT_MAX_ECHOES )->SetLimits( 1, LtComLeddarM16Modbus::M16_MAX_SERIAL_DETECTIONS );
    GetProperties()->GetIntegerProperty( LdPropertyIds::ID_CHANGE_DELAY )->SetLimits( LtComLeddarM16Modbus::M16_MIN_DELAY, LtComLeddarM16Modbus::M16_MAX_DELAY );

    LeddarCore::LdEnumProperty *lSerialBaud = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_BAUDRATE );
    lSerialBaud->AddEnumPair( 9600, "9600" );
    lSerialBaud->AddEnumPair( 19200, "19200" );
    lSerialBaud->AddEnumPair( 38400, "38400" );
    lSerialBaud->AddEnumPair( 57600, "57600" );
    lSerialBaud->AddEnumPair( 115200, "115200" );
    lSerialBaud->AddEnumPair( 230400, "230400" );

    LeddarCore::LdEnumProperty *lSerialResolution = GetProperties()->GetEnumProperty( LdPropertyIds::ID_COM_SERIAL_PORT_ECHOES_RES );
    lSerialResolution->AddEnumPair( 1, "m" );
    lSerialResolution->AddEnumPair( 10, "dm" );
    lSerialResolution->AddEnumPair( 100, "cm" );
    lSerialResolution->AddEnumPair( 1000, "mm" );

    LeddarCore::LdEnumProperty *lLedPower = GetProperties()->GetEnumProperty( LdPropertyIds::ID_LED_INTENSITY );
    lLedPower->AddEnumPair( 10, "10" );
    lLedPower->AddEnumPair( 20, "20" );
    lLedPower->AddEnumPair( 35, "35" );
    lLedPower->AddEnumPair( 50, "50" );
    lLedPower->AddEnumPair( 65, "65" );
    lLedPower->AddEnumPair( 80, "80" );
    lLedPower->AddEnumPair( 90, "90" );
    lLedPower->AddEnumPair( 100, "100" );

    LdEnumProperty *lSerialDataBits = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_PARITY );
    lSerialDataBits->AddEnumPair( 0, "None" );
    lSerialDataBits->AddEnumPair( 1, "Odd" );
    lSerialDataBits->AddEnumPair( 2, "Even" );

    LdEnumProperty *lSerialStopBits = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_COM_SERIAL_PORT_STOP_BITS );
    lSerialStopBits->AddEnumPair( 1, "1 bit" );
    lSerialStopBits->AddEnumPair( 2, "2 bits" );

    //States
    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_RS_SYSTEM_TEMP, 0, 4, 0, 1,
            "System Temperature" ) );
}

#endif