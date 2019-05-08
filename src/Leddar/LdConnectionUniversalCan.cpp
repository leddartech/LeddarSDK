////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdConnectionUniversalCan.cpp
///
/// \brief  Implements the LdConnectionUniversalCan class
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdConnectionUniversalCan.h"
#ifdef BUILD_CANBUS

#include "LdInterfaceCan.h"
#include "LtExceptions.h"
#include "LtTimeUtils.h"
#include "LtStringUtils.h"
#include "LtIntUtilities.h"

#include "comm/Canbus/LtComVuCanbus.h"
#include "LdConnectionUniversalDefines.h"

#include <string.h>

const uint8_t CAN_FUNCTION_CODE_SIZE = 1;
const uint8_t DATA_SIZE_SIZE = 1;
const uint8_t ADDRESS_SIZE = 2;
const uint16_t DEFAULT_BUFFER_SIZE = 2048;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdConnectionUniversalCan::LdConnectionUniversalCan( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface )
///
/// \brief  Constructor
///
/// \param          aConnectionInfo Information describing the connection.
/// \param [in,out] aInterface      If non-null, the interface.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdConnectionUniversalCan::LdConnectionUniversalCan( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface ) : LdConnectionUniversal( aConnectionInfo, aInterface ),
    mCurrentBaseAddress( -1 )
{
    mInterfaceCan = dynamic_cast<LeddarConnection::LdInterfaceCan *>( aInterface );

    mTransferBufferSize = DEFAULT_BUFFER_SIZE;
    mTransferInputBuffer  = new uint8_t[mTransferBufferSize];
    mTransferOutputBuffer = new uint8_t[mTransferBufferSize];
    mInterface->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdConnectionUniversalCan::~LdConnectionUniversalCan()
///
/// \brief  Destructor
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdConnectionUniversalCan::~LdConnectionUniversalCan()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdConnectionUniversalCan::RawConnect( void )
///
/// \brief  Basic connection
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdConnectionUniversalCan::RawConnect( void )
{
    mInterfaceCan->Connect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdConnectionUniversalCan::Connect( void )
///
/// \brief  Connects this object and initialize it
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdConnectionUniversalCan::Connect( void )
{
    RawConnect();
    LdConnectionUniversal::Init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdConnectionUniversalCan::Disconnect( void )
///
/// \brief  Disconnects this object
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdConnectionUniversalCan::Disconnect( void )
{
    mInterfaceCan->Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdConnectionUniversalCan::Read( uint8_t, uint32_t aAddress, const uint32_t &aDataSize, int16_t , const int16_t &aIsReadyTimeout )
///
/// \brief  Reads data from the device.
///
/// \exception  LeddarException::LtNotConnectedException    Thrown when not connected
/// \exception  LeddarException::LtTimeoutException         Thrown when a Lt Timeout error
///                                                         condition occurs.
/// \exception  std::logic_error                            Raised when a logic error condition
///                                                         occurs.
/// \exception  std::runtime_error                          Raised when a runtime error condition
///                                                         occurs.
///
/// \param  aOpCode         Hex corresponding to the function (read, write, read status, ...) sent to the device.
/// \param  aAddress        Address of the data to read.
/// \param  aDataSize       Size of memory to read.
/// \param  parameter4      (not used in CAN) Number of retry if CRC check fail. (0 mean no CRC check).
/// \param  aIsReadyTimeout Timeout in milliseconds of the timer that wait the device to be ready
///                         after the write operation. The value must be to - 1 if no waiting is
///                         required.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdConnectionUniversalCan::Read( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t, const int16_t &aIsReadyTimeout )
{
    if( mInterface->IsConnected() == false )
    {
        throw LeddarException::LtNotConnectedException( "CAN-universal device not connected." );
    }

    if( aOpCode == REGMAP_READ || aOpCode == REGMAP_RDSR )
    {
        // Check if device is ready (only for 0xb opcode)
        int16_t lIsReadyTimeout = this->mAlwaysReadyCheck ? 5000 : 0;

        if( ( aIsReadyTimeout > 0 || lIsReadyTimeout ) && aOpCode == 0xb )
        {
            if( !IsDeviceReady( std::max( aIsReadyTimeout, lIsReadyTimeout ) ) )
                throw LeddarException::LtTimeoutException( "Timeout expired. Device not ready for other operation.", true );
        }

        uint32_t lBytesToReceive = aDataSize;
        std::vector<uint8_t> lTempBuffer( aDataSize, 0 );

        while( lBytesToReceive > 0 )
        {
            ResetBuffers();
            LtComCanBus::sCanData lCanData = {};

            // Build transaction header
            if( aOpCode == REGMAP_READ )
            {
                lCanData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_READ_DATA;

                if( lBytesToReceive >= 4 )
                    lCanData.mFrame.Cmd.mSubCmd = 4;
                else if( lBytesToReceive == 3 )
                    lCanData.mFrame.Cmd.mSubCmd = 2; //we cant receive 3 bytes, and it should never happen anyway
                else //Should be 2 or 1
                    lCanData.mFrame.Cmd.mSubCmd = lBytesToReceive;

                *reinterpret_cast<uint16_t *>( &lCanData.mFrame.Cmd.mArg[0] ) = SetBaseAddress( aAddress + aDataSize - lBytesToReceive );
            }
            else
            {
                lCanData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SEND_OP_CODE;
                lCanData.mFrame.Cmd.mArg[0] = aOpCode;
            }

            if( !mInterfaceCan->WriteAndWaitForAnswer( dynamic_cast<const LeddarConnection::LdConnectionInfoCan *>( mConnectionInfo )->GetBaseIdRx(),
                    std::vector<uint8_t>( lCanData.mFrame.mRawData, lCanData.mFrame.mRawData + LtComCanBus::CAN_DATA_SIZE ) ) )
            {
                throw LeddarException::LtComException( "Couldnt read register " + LeddarUtils::LtStringUtils::IntToString( aAddress, 16 ) );
            }

            uint8_t lCount = 100;

            while( mTransferOutputBuffer[0] == 0 ) //we havent read anything for us yet
            {
                mInterfaceCan->Read(); //Read something, not necessarily for us
                LeddarUtils::LtTimeUtils::Wait( 1 );

                if( lCount-- == 0 )
                {
                    throw LeddarException::LtTimeoutException( "Timeout waiting for sensor answer reading register " + LeddarUtils::LtStringUtils::IntToString( aAddress, 16 ) );
                }
            }

            uint8_t lReadSize = 0;

            if( aOpCode == REGMAP_READ )
            {
                lReadSize = lCanData.mFrame.Cmd.mSubCmd;
            }
            else // REGMAP_RDSR
            {
                lReadSize = aDataSize;
            }

            memcpy( &lTempBuffer[aDataSize - lBytesToReceive], &mTransferOutputBuffer[4], lReadSize );
            lBytesToReceive -= lReadSize;
        }

        //copy back the full data buffer to OutBuffer to the sensor can use it
        memcpy( mTransferOutputBuffer, &lTempBuffer[0], aDataSize );
    }
    else
    {
        throw std::invalid_argument( "Unhandled op code" );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdConnectionUniversalCan::Write( uint8_t, uint32_t aAddress, const uint32_t &aDataSize, int16_t, const int16_t &aPostIsReadyTimeout, const int16_t &, const uint16_t &aWaitAfterOpCode )
///
/// \brief  Writes data on the sensor
///
/// \exception  LeddarException::LtNotConnectedException    Thrown when Not Connected.
///
/// \param  aOpCode             Hex corresponding to the function (read, write, read status, ...) sent to the device.
/// \param  aAddress            Address of the data to write to.
/// \param  aDataSize           Size of memory to write.
/// \param  parameter4          (not used in CAN) Number of retry if CRC check fail.
/// \param  aPostIsReadyTimeout Timeout in milliseconds of the timer that wait the device to be
///                             ready after the write operation. (Default 1000)
///                             The value must be to -1 if no waiting is required.
/// \param  parameter6          (not used in CAN) Timeout in milliseconds of the timer that wait the device to be
///                             ready after the write operation. (Default 1000)
///                             The value must be to -1 if no waiting is required.
/// \param  aWaitAfterOpCode    Wait time in ms after the OpCode is sent. Some command need a
///                             sleep after the command is sent.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdConnectionUniversalCan::Write( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t, const int16_t &aPostIsReadyTimeout,
        const int16_t &, const uint16_t &aWaitAfterOpCode )
{
    if( mInterface->IsConnected() == false )
    {
        throw LeddarException::LtNotConnectedException( "SPI device not connected." );
    }

    if( aOpCode == REGMAP_WRITE || aOpCode == REGMAP_WREN || aOpCode == REGMAP_WRDIS || aOpCode == REGMAP_SWRST || aOpCode ==  REGMAP_CE )
    {
        std::vector<uint8_t> lData( aDataSize + 1 ); //+1 because sometimes we send 0 data (just the op code) and we cant memcpy on size 0 vector
        memcpy( &lData[0], mTransferInputBuffer, aDataSize );

        uint32_t lBytesToSend = aDataSize;

        while( lBytesToSend > 0 || aOpCode == REGMAP_WREN || aOpCode == REGMAP_WRDIS || aOpCode == REGMAP_SWRST || aOpCode ==  REGMAP_CE ) //We either write data or send the provided op code
        {
            LtComCanBus::sCanData lCanData = {};
            // Build transaction header
            lCanData.mFrame.Cmd.mCmd =  aOpCode == REGMAP_WRITE  ? LtComCanBus::VU_CMD_WRITE_DATA : LtComCanBus::VU_CMD_SEND_OP_CODE;

            if( lBytesToSend >= 4 )
                lCanData.mFrame.Cmd.mSubCmd = 4;
            else if( lBytesToSend == 3 ) //we cant receive 3 bytes, and it should never happen anyway
                lCanData.mFrame.Cmd.mSubCmd = 2;
            else //Should be 2 or 1 (or 0 for op code only)
                lCanData.mFrame.Cmd.mSubCmd = lBytesToSend;

            if( aOpCode == REGMAP_WRITE )
            {
                *reinterpret_cast<uint16_t *>( &lCanData.mFrame.Cmd.mArg[0] ) = SetBaseAddress( aAddress + aDataSize - lBytesToSend );
                memcpy( &lCanData.mFrame.Cmd.mArg[2], &lData[0] + aDataSize - lBytesToSend, lCanData.mFrame.Cmd.mSubCmd );
            }
            else
            {
                lCanData.mFrame.Cmd.mArg[0] = aOpCode;
            }

            if( aWaitAfterOpCode == 0 )
            {
                if( !mInterfaceCan->WriteAndWaitForAnswer( dynamic_cast<const LeddarConnection::LdConnectionInfoCan *>( mConnectionInfo )->GetBaseIdRx(),
                        std::vector<uint8_t>( lCanData.mFrame.mRawData, lCanData.mFrame.mRawData + LtComCanBus::CAN_DATA_SIZE ) ) )
                {
                    throw LeddarException::LtComException( "Couldnt write register " + LeddarUtils::LtStringUtils::IntToString( aAddress, 16 ) );
                }
            }
            else
            {
                mInterfaceCan->Write( dynamic_cast<const LeddarConnection::LdConnectionInfoCan *>( mConnectionInfo )->GetBaseIdRx(),
                                      std::vector<uint8_t>( lCanData.mFrame.mRawData, lCanData.mFrame.mRawData + LtComCanBus::CAN_DATA_SIZE ) );
                LeddarUtils::LtTimeUtils::Wait( aWaitAfterOpCode );

                if( !mInterfaceCan->Read() )
                {
                    throw LeddarException::LtComException( "Couldnt send op code " + LeddarUtils::LtStringUtils::IntToString( aOpCode, 10 ) );
                }
            }

            if( aPostIsReadyTimeout > 0 )
            {
                if( !IsDeviceReady( aPostIsReadyTimeout ) )
                {
                    throw LeddarException::LtTimeoutException( "Timeout expired. Device not ready for other operation.", true );
                }
            }

            lBytesToSend -= lCanData.mFrame.Cmd.mSubCmd;

            if( aOpCode == REGMAP_WREN || aOpCode == REGMAP_WRDIS || aOpCode == REGMAP_SWRST || aOpCode ==  REGMAP_CE )
                break;
        }
    }
    else
    {
        throw std::invalid_argument( "Unhandled op code" );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdConnectionUniversalCan::Reset( LeddarDefines::eResetType aType, bool )
///
/// \brief  Resets this object
///
/// \exception  LeddarException::LtNotConnectedException    Thrown when not connected.
/// \exception  std::runtime_error                          Raised when the device didn't reboot after the reset.
/// \exception  std::invalid_argument                       Thrown when the reset type is not supported.
///
/// \param  aType       The type of reset.
/// \param  parameter2  (not supported in CAN) True to enter boot loader.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdConnectionUniversalCan::Reset( LeddarDefines::eResetType aType, bool )
{
    if( mInterface->IsConnected() == false )
    {
        throw LeddarException::LtNotConnectedException( "CAN device not connected." );
    }

    if( aType == LeddarDefines::RT_SOFT_RESET )
    {
        Write( REGMAP_SWRST, 0, 0, 0, 0, 0, 5000 );
        LeddarUtils::LtTimeUtils::Wait( 100 );

        if( !IsDeviceReady( 10000 ) )
        {
            throw std::runtime_error( "Device never rebooted after software reset." );
        }

        return;
    }

    throw std::invalid_argument( "Reset type not implemented for this type of device." );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint16_t LeddarConnection::LdConnectionUniversalCan::InternalBuffers( uint8_t *( &aInputBuffer ), uint8_t *( &aOutputBuffer ) )
///
/// \brief  Get pointer to the communication buffers
///
/// \param [in,out] aInputBuffer    [in,out] Buffer for input data. (from sensor point of view)
/// \param [in,out] aOutputBuffer   [in,out] Buffer for output data. (from sensor point of view)
///
/// \return Buffer size
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t LeddarConnection::LdConnectionUniversalCan::InternalBuffers( uint8_t *( &aInputBuffer ), uint8_t *( &aOutputBuffer ) )
{
    aInputBuffer = mTransferInputBuffer;
    aOutputBuffer = mTransferOutputBuffer;
    return mTransferBufferSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdConnectionUniversalCan::Callback( LdObject *aSender, const SIGNALS aSignal, void *aCanData )
///
/// \brief  Callback. Supports NEW_DATA signal to store new data in the correct buffer
///
/// \param [in,out] aSender     The sender.
/// \param          aSignal     The signal.
/// \param [in,out] aCanData    If non-null, information describing the can.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdConnectionUniversalCan::Callback( LdObject *aSender, const SIGNALS aSignal, void *aCanData )
{
    if( aSender == mInterfaceCan && aSignal == LeddarCore::LdObject::NEW_DATA )
    {
        LtComCanBus::sCanData &lCanData = *reinterpret_cast<LtComCanBus::sCanData *>( aCanData );

        if( lCanData.mFrame.Cmd.mArg[0] == 0xFF && lCanData.mFrame.Cmd.mArg[1] == 0xFF && lCanData.mFrame.Cmd.mArg[2] == 0xFF &&
                lCanData.mFrame.Cmd.mArg[3] == 0xFF && lCanData.mFrame.Cmd.mArg[4] == 0xFF && lCanData.mFrame.Cmd.mArg[5] == 0xFF )
        {
            throw std::runtime_error( "Sensor failed to process command:" + LeddarUtils::LtStringUtils::IntToString( lCanData.mFrame.Cmd.mCmd, 16 ) );
        }

        memcpy( mTransferOutputBuffer, lCanData.mFrame.mRawData, LtComCanBus::CAN_DATA_SIZE );
    }
    else
    {
        throw std::logic_error( "Unhandled signal" );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdConnectionUniversalCan::ResetBuffers( void )
///
/// \brief  Resets the buffers
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdConnectionUniversalCan::ResetBuffers( void )
{
    memset( mTransferInputBuffer, 0, mTransferBufferSize );
    memset( mTransferOutputBuffer, 0, mTransferBufferSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint16_t LeddarConnection::LdConnectionUniversalCan::SetBaseAddress(uint32_t aFullAddress)
///
/// \brief  Sets base address for future commands and return the 16 LSB of the address to be sent with the next read or write command
///
/// \param  aFullAddress    The full address.
///
/// \return the 16 LSB of the address to be sent with the next read or write command
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t LeddarConnection::LdConnectionUniversalCan::SetBaseAddress( uint32_t aFullAddress )
{
    uint32_t lBaseAddress = aFullAddress & 0xFFFF0000;

    if( lBaseAddress != mCurrentBaseAddress )
    {
        LtComCanBus::sCanData lCanData = {};
        lCanData.mFrame.Cmd.mCmd = LtComCanBus::VU_CMD_SET_BASE_ADDRESS;
        *reinterpret_cast<uint32_t *>( &lCanData.mFrame.Cmd.mArg[2] ) = lBaseAddress;

        if( !mInterfaceCan->WriteAndWaitForAnswer( dynamic_cast<const LeddarConnection::LdConnectionInfoCan *>( mConnectionInfo )->GetBaseIdRx(),
                std::vector<uint8_t>( lCanData.mFrame.mRawData, lCanData.mFrame.mRawData + LtComCanBus::CAN_DATA_SIZE ) ) )
        {
            throw LeddarException::LtComException( "Couldnt set base address." );
        }

        mCurrentBaseAddress = lBaseAddress;
    }

    return ( aFullAddress & 0xFFFF );
}

#endif
