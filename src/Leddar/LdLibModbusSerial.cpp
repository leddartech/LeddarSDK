// *****************************************************************************
// Module..: Leddar
//
/// \file    LdLibModbusSerial.cpp
///
/// \brief   Class definition of LdLibModbusSerial
///
/// \author  Patrick Boulay
///
/// \since   September 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdLibModbusSerial.h"
#ifdef BUILD_MODBUS

#include "LdConnectionInfoModbus.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"
#include "LtSystemUtils.h"

#include "comm/LtComLeddarTechPublic.h"
#include "comm/Modbus/LtComLeddarM16Modbus.h"
#include "comm/Modbus/LtComLeddarOneModbus.h"
#include "comm/Modbus/LtComLeddarVu8Modbus.h"

extern "C"
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include "modbus.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#include "modbus-LT.h"
#include "modbus-private.h"
}
#include "LtTimeUtils.h"
#include <cerrno>

// *****************************************************************************
// Function: LdLibModbusSerial::LdLibModbusSerial
//
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   September 2016
// *****************************************************************************
LeddarConnection::LdLibModbusSerial::LdLibModbusSerial( const LdConnectionInfoModbus *aConnectionInfo, LdConnection *aExistingConnection )
    : LeddarConnection::LdInterfaceModbus( aConnectionInfo )
    , mHandle( nullptr )
    , mSharedHandle( false )
{
    LeddarConnection::LdLibModbusSerial *lExistingModbusConnection = dynamic_cast<LeddarConnection::LdLibModbusSerial *>( aExistingConnection );

    if( lExistingModbusConnection != nullptr && lExistingModbusConnection->GetHandle() != nullptr )
    {
        mHandle       = lExistingModbusConnection->GetHandle();
        mSharedHandle = true;
    }
}

// *****************************************************************************
// Function: LdLibModbusSerial::~LdLibModbusSerial
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   September 2016
// *****************************************************************************
LeddarConnection::LdLibModbusSerial::~LdLibModbusSerial() { LdLibModbusSerial::Disconnect(); }

// *****************************************************************************
// Function: LdLibModbusSerial::Connect
//
/// \brief   Connect to the device using the information in LdConnectionInfo provided in the constructor.
///
/// \exception LtConnectionFailed If the connection failed.
///
/// \author  Patrick Boulay
///
/// \since   September 2016
// *****************************************************************************
void LeddarConnection::LdLibModbusSerial::Connect( void )
{
    Disconnect();

    try
    {
        ConnectRaw();

        modbus_set_response_timeout( mHandle, 10, 0 );
        modbus_set_byte_timeout( mHandle, 0, 100000 );

        SetDeviceType( FetchDeviceType() );
    }
    catch( std::exception & /*e*/ )
    {
        Disconnect();
        throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdLibModbusSerial::ConnectRaw()
///
/// \brief  Connects and do nothing else
///
/// \author David Lévy
/// \date   November 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdLibModbusSerial::ConnectRaw()
{
    char lParityChar = ' ';

    switch( mConnectionInfoModbus->GetParity() )
    {
    case LdConnectionInfoModbus::MB_PARITY_NONE:
        lParityChar = 'N';
        break;

    case LdConnectionInfoModbus::MB_PARITY_EVEN:
        lParityChar = 'E';
        break;

    case LdConnectionInfoModbus::MB_PARITY_ODD:
        lParityChar = 'O';
        break;
    }

    mHandle = modbus_new_rtu( mConnectionInfoModbus->GetSerialPort().c_str(), mConnectionInfoModbus->GetBaud(), lParityChar, mConnectionInfoModbus->GetDataBits(),
                              mConnectionInfoModbus->GetStopBits() );

    if( mHandle == nullptr )
    {
        throw LeddarException::LtConnectionFailed( "Wrong argument on modbus device creation, Serial Port:" + mConnectionInfoModbus->GetSerialPort() +
                                                       " Baud: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoModbus->GetBaud() ) + " Parity: " + lParityChar +
                                                       " Data Bits: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoModbus->GetDataBits() ) +
                                                       " Stop Bits: " + LeddarUtils::LtStringUtils::IntToString( mConnectionInfoModbus->GetStopBits() ),
                                                   true );
    }

    // Set slave address
    if( modbus_set_slave( mHandle, mConnectionInfoModbus->GetModbusAddr() ) != 0 )
    {
        modbus_free( mHandle );
        mHandle = nullptr;

        throw LeddarException::LtConnectionFailed( "Connection failed, libmodbus errno: (" + LeddarUtils::LtStringUtils::IntToString( errno ) + std::string( "),  msg: " ) +
                                                       std::string( modbus_strerror( errno ) ),
                                                   true );
    }

    // Connect to device
    if( modbus_connect( mHandle ) != 0 )
    {
        modbus_free( mHandle );
        mHandle = nullptr;

        throw LeddarException::LtConnectionFailed( "Connection failed, libmodbus errno: (" + LeddarUtils::LtStringUtils::IntToString( errno ) + std::string( "),  msg: " ) +
                                                       std::string( modbus_strerror( errno ) ),
                                                   true );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn int LeddarConnection::LdLibModbusSerial::ReadRawData( uint8_t *aBuffer )
///
/// \brief  Reads data (maximum RTU_MAX_ADU_LENGTH) from device until it times out
///
/// \param [in,out] aBuffer The buffer to read data to
///
/// \returns    Number of bytes read.
///
/// \author David Lévy
/// \date   November 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
int LeddarConnection::LdLibModbusSerial::ReadRawData( uint8_t *aBuffer ) { return modbus_receive_raw_data_timeoutEnd( mHandle, aBuffer ); }

int LeddarConnection::LdLibModbusSerial::WriteRawData( uint8_t *aBuffer, size_t aSize, bool aCRC ) { return modbus_send_raw_data(mHandle, aBuffer, aSize, aCRC ? 1 : 0); }

// *****************************************************************************
// Function: LdLibModbusSerial::Disconnect
//
/// \brief   Disconnect the device.
///
/// \author  Patrick Boulay
///
/// \since   July 2016
// *****************************************************************************
void LeddarConnection::LdLibModbusSerial::Disconnect( void )
{
    // mIsConnected = false;

    if( mHandle != nullptr && !mSharedHandle )
    {
        modbus_close( mHandle );
        modbus_free( mHandle );
        mHandle = nullptr;
    }
}

// *****************************************************************************
// Function: LdLibModbusSerial::SendRawRequest
//
/// \brief   Send raw request on modbus interface
///
/// \param   aBuffer Buffer to send
/// \param   aSize Data length to send: this not include the length of Modbus CRC field.
///
/// \exception LtComException on error in sending request
///
/// \author  Patrick Boulay
///
/// \since   September 2016
// *****************************************************************************
void LeddarConnection::LdLibModbusSerial::SendRawRequest( uint8_t *aBuffer, uint32_t aSize )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtNotConnectedException( "Modbus device not connected.", true );
    }

    modbus_flush( mHandle );

    // Set slave address
    if( modbus_set_slave( mHandle, mConnectionInfoModbus->GetModbusAddr() ) != 0 )
    {
        throw LeddarException::LtConnectionFailed( "Connection failed, libmodbus errno: (" + LeddarUtils::LtStringUtils::IntToString( errno ) + std::string( "),  msg: " ) +
                                                       std::string( modbus_strerror( errno ) ),
                                                   true );
    }

    int lResult = modbus_send_raw_request( mHandle, aBuffer, aSize );

    if( lResult < 0 )
    {
        throw LeddarException::LtComException( "Error on modbus_send_raw_request in SendRawRequest." );
    }
}

// *****************************************************************************
// Function: LdLibModbusSerial::ReadRegisters
//
/// \brief   Read registers on modbus interface (use function 0x03)
///             You DO NOT need to convert data to/from big endian
///
/// \param   aAddr Address to read from
/// \param   aNb Number of register to read
/// \param   aDest Array containing the values of the register
///
/// \exception LtComException on error in reading registers
///
// *****************************************************************************
void LeddarConnection::LdLibModbusSerial::ReadRegisters( uint16_t aAddr, uint8_t aNb, uint16_t *aDest )
{
    // Set slave address
    if( modbus_set_slave( mHandle, mConnectionInfoModbus->GetModbusAddr() ) != 0 )
    {
        throw LeddarException::LtConnectionFailed( "Connection failed, libmodbus errno: (" + LeddarUtils::LtStringUtils::IntToString( errno ) + std::string( "),  msg: " ) +
                                                       std::string( modbus_strerror( errno ) ),
                                                   true );
    }

    int lStatus = modbus_read_registers( mHandle, aAddr, aNb, aDest );

    if( lStatus < 0 )
    {
        throw LeddarException::LtComException( "Error on modbus_read_registers in ReadRegisters." );
    }
}

// *****************************************************************************
// Function: LdLibModbusSerial::ReadInputRegisters
//
/// \brief   Read registers on modbus interface (use function 0x04)
///             You DO NOT need to convert data to/from big endian
///
/// \param   aAddr Address to read from
/// \param   aNb Number of register to read
/// \param   aDest Array containing the values of the register
///
/// \exception LtComException on error in reading input registers
///
// *****************************************************************************
void LeddarConnection::LdLibModbusSerial::ReadInputRegisters( uint16_t aAddr, uint8_t aNb, uint16_t *aDest )
{
    // Set slave address
    if( modbus_set_slave( mHandle, mConnectionInfoModbus->GetModbusAddr() ) != 0 )
    {
        throw LeddarException::LtConnectionFailed( "Connection failed, libmodbus errno: (" + LeddarUtils::LtStringUtils::IntToString( errno ) + std::string( "),  msg: " ) +
                                                       std::string( modbus_strerror( errno ) ),
                                                   true );
    }

    int lStatus = modbus_read_input_registers( mHandle, aAddr, aNb, aDest );

    if( lStatus < 0 )
    {
        throw LeddarException::LtComException( "Error on modbus_read_input_registers in ReadRegistersReadInputRegisters." );
    }
}

// *****************************************************************************
// Function: LdLibModbusSerial::WriteRegister
//
/// \brief   Write registers on modbus interface (use function 0x06)
///             You DO NOT need to convert data to/from big endian
///
/// \param   aAddr Address to write to
/// \param   aValue Value to write to the register
///
/// \exception LtComException on error in reading registers
///
// *****************************************************************************
void LeddarConnection::LdLibModbusSerial::WriteRegister( uint16_t aAddr, int aValue )
{
    // Set slave address
    if( modbus_set_slave( mHandle, mConnectionInfoModbus->GetModbusAddr() ) != 0 )
    {
        throw LeddarException::LtConnectionFailed( "Connection failed, libmodbus errno: (" + LeddarUtils::LtStringUtils::IntToString( errno ) + std::string( "),  msg: " ) +
                                                       std::string( modbus_strerror( errno ) ),
                                                   true );
    }

    int lStatus = modbus_write_register( mHandle, aAddr, aValue );

    if( lStatus < 0 )
    {
        throw LeddarException::LtComException( "Error on modbus_write_register in WriteRegisters." );
    }
}

// *****************************************************************************
// Function: LdLibModbusSerial::ReceiveRawConfirmation
//
/// \brief   Receive raw confirmation
///
/// \param   aBuffer    Buffer to receive to
/// \param   aSize      Data length to received. This include the full length of
///                     answer packet ie :
///                     - Modbus address
///                     - Function code
///                     - Data
///                     - CRC16
///                     Set to 0 if data length is undefined: this function will use
///                     a timeout event to determine the end of Modbus frame transaction.
///
/// \exception LtComException on error on reception of if an error code in the returned function code.
///
/// \author  Patrick Boulay
/// \author  Frédéric Parent
///
/// \since   September 2016
// *****************************************************************************
size_t LeddarConnection::LdLibModbusSerial::ReceiveRawConfirmation( uint8_t *aBuffer, uint32_t aSize )
{
    int lResult;

    // Set slave address
    if( modbus_set_slave( mHandle, mConnectionInfoModbus->GetModbusAddr() ) != 0 )
    {
        throw LeddarException::LtConnectionFailed( "Connection failed, libmodbus errno: (" + LeddarUtils::LtStringUtils::IntToString( errno ) + std::string( "),  msg: " ) +
                                                       std::string( modbus_strerror( errno ) ),
                                                   true );
    }

    // Receive raw data.
    if( aSize )
    {
        lResult = modbus_receive_raw_confirmation_sizeEnd( mHandle, aBuffer, aSize );

        if( lResult < 0 )
        {
            modbus_flush( mHandle );
            throw LeddarException::LtComException( "Error on modbus modbus_receive_raw_confirmation_sizeEnd in ReceiveRawConfirmation (" +
                                                   LeddarUtils::LtSystemUtils::ErrnoToString( errno ) + ")." );
        }
    }
    else
    {
        lResult = modbus_receive_raw_confirmation_timeoutEnd( mHandle, aBuffer );

        if( lResult < 0 )
        {
            modbus_flush( mHandle );
            throw LeddarException::LtComException( "Error on modbus modbus_receive_raw_confirmation_timeoutEnd in ReceiveRawConfirmation (" +
                                                   LeddarUtils::LtSystemUtils::ErrnoToString( errno ) + ")." );
        }
    }

    // Check if the received message has an error
    if( ( aBuffer[1] >> 7 ) == 1 )
    {
        throw LeddarException::LtComException( "Received message has an error." );
    }

    return lResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn int LeddarConnection::LdLibModbusSerial::ReceiveRawConfirmationLT( uint8_t *aBuffer, int aDeviceType )
///
/// \brief  Receive raw confirmation from command 0x41
///
/// \param [in,out] aBuffer     The buffer to store data into
/// \param          aDeviceType Type of the device for sensor specific 0x41 command
///
/// \return Number of bytes received, or -1 in error.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
int LeddarConnection::LdLibModbusSerial::ReceiveRawConfirmationLT( uint8_t *aBuffer, int aDeviceType )
{
    int lResult;

    // Set slave address
    if( modbus_set_slave( mHandle, mConnectionInfoModbus->GetModbusAddr() ) != 0 )
    {
        throw LeddarException::LtConnectionFailed( "Connection failed, libmodbus errno: (" + LeddarUtils::LtStringUtils::IntToString( errno ) + std::string( "),  msg: " ) +
                                                       std::string( modbus_strerror( errno ) ),
                                                   true );
    }

    // Receive raw data
    if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16 || aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_EVALKIT ||
        aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 || aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER )
    {
        lResult = modbus_receive_raw_confirmation_0x41_0x6A_M16( mHandle, aBuffer );
    }
    else if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8 )
    {
        lResult = modbus_receive_raw_confirmation_0x41_LeddarVu( mHandle, aBuffer );
    }
    else
    {
        throw std::runtime_error( "LT custom command not supported for this sensor." );
    }

    if( lResult < 0 )
    {
        modbus_flush( mHandle );
        throw LeddarException::LtComException( "Error on modbus modbus_receive_raw_confirmation_timeoutEnd in ReceiveRawConfirmation (" +
                                               LeddarUtils::LtStringUtils::IntToString( lResult ) + ")." );
    }

    // Check if the received message has an error
    if( ( aBuffer[1] >> 7 ) == 1 )
    {
        throw LeddarException::LtComException( "Received message has an error." );
    }

    return lResult;
}

// *****************************************************************************
// Function: LdLibModbusSerial::IsVirtualCOMPort
//
/// \brief   Return true if the device is connected thought a Virtual COM Port
///
/// \author  Patrick Boulay
///
/// \since   January 2017
// *****************************************************************************
bool LeddarConnection::LdLibModbusSerial::IsVirtualCOMPort( void )
{
    return ( this->mConnectionInfoModbus->GetDescription().compare( std::string( "LeddarTech Virtual COM Port" ) ) == 0 );
}

// *****************************************************************************
// Function: LdLibModbusSerial::GetDeviceList
//
/// \brief   Return list of connected device.
///          The function release the ownership of the returned objects.
///
/// \return  Vector of LdConnectionInfo
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

std::vector<LeddarConnection::LdConnectionInfo *> LeddarConnection::LdLibModbusSerial::GetDeviceList( void )
{
    std::vector<LeddarConnection::LdConnectionInfo *> lResultList;

    std::vector<std::string> lSerialPorts = LeddarUtils::LtSystemUtils::GetSerialPorts();

    for( std::vector<std::string>::iterator lIter = lSerialPorts.begin(); lIter != lSerialPorts.end(); ++lIter )
    {
        lResultList.push_back( new LdConnectionInfoModbus( *lIter, *lIter, 115200, LdConnectionInfoModbus::MB_PARITY_NONE, 8, 1, 1 ) );
    }

    return lResultList;
}

// *****************************************************************************
// Function: LdLibModbusSerial::Flush
//
/// \brief   Call the flush function on Libmodbus
///
/// \author  Patrick Boulay
///
/// \since   January 2017
// *****************************************************************************
void LeddarConnection::LdLibModbusSerial::Flush( void ) { modbus_flush( mHandle ); }

// *****************************************************************************
// Function: LdLibModbusSerial::FetchDeviceType
//
/// \brief   Retrieve device type from sensor
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
uint16_t LeddarConnection::LdLibModbusSerial::FetchDeviceType( void )
{
    uint8_t lRawRequest[2]                       = { mConnectionInfoModbus->GetModbusAddr(), 0x11 };
    uint8_t lResponse[MODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    uint32_t lOldTimeout_sec, lOldTimeout_usec;
    modbus_get_response_timeout( mHandle, &lOldTimeout_sec, &lOldTimeout_usec );

    modbus_set_response_timeout( mHandle, 0, 100000 ); // 100 ms

    SendRawRequest( lRawRequest, 2 );
    size_t lReceivedSize = ReceiveRawConfirmation( lResponse, 0 );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( 20000 );

    uint16_t lReturn = 0;

    if( lReceivedSize <= MODBUS_DATA_OFFSET )
    {
        Flush();
        throw LeddarException::LtComException( "No data received." );
    }
    else if( lReceivedSize < MODBUS_DATA_OFFSET + lResponse[MODBUS_DATA_OFFSET] )
    {
        Flush();
    }
    else if( lReceivedSize == MODBUS_DATA_OFFSET + sizeof( LtComLeddarOneModbus::sLeddarOneServerId ) + MODBUS_CRC_SIZE )
    {
        LtComLeddarOneModbus::sLeddarOneServerId *lDeviceInfo = reinterpret_cast<LtComLeddarOneModbus::sLeddarOneServerId *>( &lResponse[MODBUS_DATA_OFFSET] );
        lReturn                                               = lDeviceInfo->mDeviceId;
    }
    else if( lReceivedSize == MODBUS_DATA_OFFSET + sizeof( LtComLeddarM16Modbus::sLeddarM16ServerId ) + MODBUS_CRC_SIZE )
    {
        LtComLeddarM16Modbus::sLeddarM16ServerId *lDeviceInfo = reinterpret_cast<LtComLeddarM16Modbus::sLeddarM16ServerId *>( &lResponse[MODBUS_DATA_OFFSET] );
        lReturn                                               = lDeviceInfo->mDeviceId;
    }
    else if( lReceivedSize == MODBUS_DATA_OFFSET + sizeof( LtComLeddarVu8Modbus::sLeddarVu8ModbusServerId ) + MODBUS_CRC_SIZE )
    {
        LtComLeddarVu8Modbus::sLeddarVu8ModbusServerId *lDeviceInfo = reinterpret_cast<LtComLeddarVu8Modbus::sLeddarVu8ModbusServerId *>( &lResponse[MODBUS_DATA_OFFSET] );
        lReturn                                                     = lDeviceInfo->mDeviceId;
    }

    modbus_set_response_timeout( mHandle, lOldTimeout_sec, lOldTimeout_usec );

    return lReturn;
}

#endif