////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdConnectionUniversalSpi.cpp
///
/// \brief  Implements the LdConnectionUniversalSpi class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdConnectionUniversalSpi.h"
#ifdef BUILD_SPI

#include "LdConnectionUniversalDefines.h"

#include "comm/LtComLeddarTechPublic.h"

#include "LtCRCUtils.h"
#include "LtExceptions.h"
#include "LtIntUtilities.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"

#define CHIP_SELECT 3
#define BITS_PER_SAMPLE 8
#define DEFAULT_BUFFER_SIZE 2048
#define SPI_UNIVERSAL_PAYLOAD_SIZE 512

// can be used for short, unsigned short, word, unsigned word (2-byte types)
#define BYTESWAP16(n) (((n&0xFF00)>>8)|((n&0x00FF)<<8))

// can be used for int or unsigned int or float (4-byte types)
#define BYTESWAP32(n) ((BYTESWAP16((n&0xFFFF0000)>>16))|((BYTESWAP16(n&0x0000FFFF))<<16))

// can be used for unsigned long long or double (8-byte types)
#define BYTESWAP64(n) ((BYTESWAP32((n&0xFFFFFFFF00000000)>>32))|((BYTESWAP32(n&0x00000000FFFFFFFF))<<32))

#define DIR_OUT(_index)     (1<<_index)
#define DIR_IN(_index)      (0<<_index)
#define MASK_PIN(_index)    (1<<_index)
#define PIN_SET(_index)     (1<<_index)
#define PIN_CLR(_index)     (0<<_index)

#define LENGTH_SIZE   2
#define OPCODE_SIZE   1
#define ADDRESS_SIZE  3
#define CRC_SIZE      2
#define HEADER_SIZE  (OPCODE_SIZE+ADDRESS_SIZE+LENGTH_SIZE)
#define OVERHEAD_SIZE (HEADER_SIZE+CRC_SIZE)

using namespace LeddarConnection;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdConnectionUniversalSpi::LdConnectionUniversalSpi( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface )
///
/// \brief  Constructor
///
/// \param          aConnectionInfo Information describing the connection.
/// \param [in,out] aInterface      If non-null, the interface.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdConnectionUniversalSpi::LdConnectionUniversalSpi( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface ) :
    LdConnectionUniversal( aConnectionInfo, aInterface )
{
    mTransferBufferSize = DEFAULT_BUFFER_SIZE;
    mTransferInputBuffer  = new uint8_t[mTransferBufferSize + OVERHEAD_SIZE];
    mTransferOutputBuffer = new uint8_t[mTransferBufferSize + OVERHEAD_SIZE];
    mSpiInterface         = dynamic_cast<LdInterfaceSpi *>( aInterface );
    mWriteBuffer.resize( SPI_UNIVERSAL_PAYLOAD_SIZE + HEADER_SIZE + CRC_SIZE );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdConnectionUniversalSpi::~LdConnectionUniversalSpi()
///
/// \brief  Destructor
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdConnectionUniversalSpi::~LdConnectionUniversalSpi()
{
    if( IsConnected() )
    {
        LdConnectionUniversalSpi::Disconnect();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::Connect( void )
///
/// \brief  Connect to SPI device using SPI Universal Connection
///
/// \exception  std::exception  Thrown when an exception error condition occurs.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::Connect( void )
{
    try
    {
        mSpiInterface->Connect();
        mSpiInterface->SetSpiConfig( LdInterfaceSpi::CS_ACTIVEL, CHIP_SELECT,
                                     dynamic_cast< const LdConnectionInfoSpi * >( mConnectionInfo )->GetClock(),
                                     LdInterfaceSpi::CPOL_HIGH, LdInterfaceSpi::CPHA_FIRST, BITS_PER_SAMPLE );
        InitIO();
        LdConnectionUniversal::Init();
    }
    catch( std::exception &)
    {
        Disconnect();
        throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::Disconnect( void )
///
/// \brief  Disconnect the SPI device.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::Disconnect( void )
{
    if( IsConnected() )
    {
        mInterface->Disconnect();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::RawConnect( void )
///
/// \brief  Connect to SPI device using SPI Universal Connection with out any initialization.
///
/// \exception  std::exception  Thrown when an exception error condition occurs.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::RawConnect( void )
{
    try
    {
        mSpiInterface->Connect();
        mSpiInterface->SetSpiConfig( LdInterfaceSpi::CS_ACTIVEL, CHIP_SELECT,
                                     dynamic_cast< const LdConnectionInfoSpi * >( mConnectionInfo )->GetClock(),
                                     LdInterfaceSpi::CPOL_HIGH,
                                     LdInterfaceSpi::CPHA_FIRST, BITS_PER_SAMPLE );
        InitIO();
    }
    catch( std::exception &)
    {
        Disconnect();
        throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::Read( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry, const int16_t &aIsReadyTimeout )
///
/// \brief  Read data from the device.
///
/// \exception  LeddarException::LtNotConnectedException    Thrown when a Lt Not Connected error condition occurs.
/// \exception  LeddarException::LtTimeoutException         Thrown when a Lt Timeout error condition occurs.
/// \exception  LeddarException::LtComException             Thrown when a Lt Com error condition occurs.
///
/// \param  aOpCode         Hexa corresponding to the function (read, write, read status, ...) sent to the device.
/// \param  aAddress        Address of the data to read.
/// \param  aDataSize       Size of memory to read.
/// \param  aCRCTry         Number of retry if CRC check fail. (0 mean no CRC check).
/// \param  aIsReadyTimeout Timeout in milliseconds of the timer that wait the device to be ready after the write operation. The value must be to -
///                         1 if no waiting is required.
///
/// \author Patrick Boulay
/// \author Vincent Simard Bilodeau
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::Read( uint8_t         aOpCode,
                                uint32_t        aAddress,
                                const uint32_t &aDataSize,
                                int16_t         aCRCTry,
                                const int16_t  &aIsReadyTimeout )
{

    if( mInterface->IsConnected() == false )
    {
        throw LeddarException::LtNotConnectedException( "SPI device not connected." );
    }

    // Check if device is ready (only for 0xb opcode)
    int16_t lIsReadyTimeout = this->mAlwaysReadyCheck ? 5000 : 0;

    if( ( aIsReadyTimeout > 0 || lIsReadyTimeout ) && aOpCode == 0xb )
    {
        if( !IsDeviceReady( std::max( aIsReadyTimeout, lIsReadyTimeout ) ) )
            throw LeddarException::LtTimeoutException( "Timeout expired. Device not ready for other operation.", true );
    }

    // Addresses and data sizes are in big endian with the Universal protocol SPI
    // Convert size and address in big endian
    uint32_t lBigEndianAddr = aAddress, lBigEndianDataSize = aDataSize;

    if( !mIsBigEndian )
    {
        lBigEndianAddr = LeddarUtils::LtIntUtilities::SwapEndian<uint32_t>( aAddress );
        lBigEndianDataSize = LeddarUtils::LtIntUtilities::SwapEndian<uint16_t>( aDataSize );
    }

    // Retry transaction is required
    while( aCRCTry >= 0 )
    {

        // Build transaction header
        memset( mTransferInputBuffer, 0, aDataSize + OVERHEAD_SIZE );
        memset( mTransferOutputBuffer, 0, aDataSize + OVERHEAD_SIZE );
        memcpy( mTransferInputBuffer, &aOpCode, OPCODE_SIZE );
        memcpy( mTransferInputBuffer + OPCODE_SIZE, ( ( uint8_t * )&lBigEndianAddr ) + 1, ADDRESS_SIZE );
        memcpy( mTransferInputBuffer + OPCODE_SIZE + ADDRESS_SIZE, ( ( uint8_t * )&lBigEndianDataSize ), LENGTH_SIZE );
        mSpiInterface->Write( mTransferInputBuffer, HEADER_SIZE );

        // Let time to the MCU
        // to prepare the answer
        LeddarUtils::LtTimeUtils::WaitBlockingMicro( 1000 );

        // Clock to get the payload
        mSpiInterface->Read( mTransferOutputBuffer + HEADER_SIZE, aDataSize + CRC_SIZE, true );

        // Check crc
        uint16_t lCrc16 = ( ( *( mTransferOutputBuffer + HEADER_SIZE + aDataSize ) ) << 8 ) + *( mTransferOutputBuffer + HEADER_SIZE + aDataSize + 1 );

        if( aCRCTry > 0 )
        {
            try
            {
                CrcCheck( mTransferInputBuffer, mTransferOutputBuffer + HEADER_SIZE, aDataSize, lCrc16 );
                return;
            }
            catch( LeddarException::LtComException &e )
            {
                if( aCRCTry <= 1 )
                {
                    e.SetExtraInformation( "Read address: 0x"
                                           + LeddarUtils::LtStringUtils::IntToString( aAddress, 16 )
                                           + " size: " + LeddarUtils::LtStringUtils::IntToString( aDataSize ) );
                    throw;
                }
            }

            LeddarUtils::LtTimeUtils::Wait( 1 );
        }

        --aCRCTry;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::Write( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry, const int16_t &aPostIsReadyTimeout, const int16_t &aPreIsReadyTimeout, const uint16_t &aWaitAfterOpCode )
///
/// \brief  Write data from the device.
///
/// \exception  LeddarException::LtNotConnectedException    If the device is not connected.
/// \exception  LeddarException::LtTimeoutException         If the timeout is expired and the device is not ready for other instruction.
/// \exception  LeddarException::LtComException             Thrown when a Lt Com error condition occurs.
///
/// \param  aOpCode             Hexa corresponding to the function (read, write, read status, ...) sent to the device.
/// \param  aAddress            Address of the data to read.
/// \param  aDataSize           Size of memory to read.
/// \param  aCRCTry             Number of retry if CRC check fail. (0 mean no CRC check).
/// \param  aPostIsReadyTimeout Timeout in milliseconds of the timer that wait the device to be ready after the write operation. (Default 1000)
///                             The value must be to -1 if no waiting is required.
/// \param  aPreIsReadyTimeout  Timeout in milliseconds of the timer that wait the device to be ready after the write operation. (Default 1000)
///                             The value must be to -1 if no waiting is required.
/// \param  aWaitAfterOpCode    Wait time in ms after the OpCode is sent. Some command need a sleep after the command is sent.
///
/// \author Patrick Boulay
/// \author Vincent Simard Bilodeau
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::Write( uint8_t          aOpCode,
                                 uint32_t         aAddress,
                                 const uint32_t  &aDataSize,
                                 int16_t          aCRCTry,
                                 const int16_t   &aPostIsReadyTimeout,
                                 const int16_t   &aPreIsReadyTimeout,
                                 const uint16_t  &aWaitAfterOpCode )
{
    if( mInterface->IsConnected() == false )
    {
        throw LeddarException::LtNotConnectedException( "SPI device not connected." );
    }

    // Check if device is ready (only for 0xb opcode)
    int16_t lIsReadyTimeout = this->mAlwaysReadyCheck ? 5000 : 0;

    if( ( aPreIsReadyTimeout > 0 || lIsReadyTimeout ) && aOpCode == 0xb )
    {
        if( !IsDeviceReady( std::max( aPreIsReadyTimeout, lIsReadyTimeout ) ) )
            throw LeddarException::LtTimeoutException( "Timeout expired. Device not ready for other operation.", true );
    }

    uint32_t lBytes2Send = aDataSize;
    uint32_t lBytesSent = 0;

    do
    {
        uint16_t lBytesSend = ( lBytes2Send <= SPI_UNIVERSAL_PAYLOAD_SIZE ) ? lBytes2Send : SPI_UNIVERSAL_PAYLOAD_SIZE;

        uint32_t lBigEndianAddr = aAddress;
        uint16_t lBigEndianDataSize = lBytesSend;

        if( !mIsBigEndian )
        {
            lBigEndianAddr = LeddarUtils::LtIntUtilities::SwapEndian<uint32_t>( aAddress + lBytesSent );
            lBigEndianDataSize = LeddarUtils::LtIntUtilities::SwapEndian<uint16_t>( lBytesSend );
        }

        int16_t lCRCTry = aCRCTry;

        while( lCRCTry >= 0 )
        {
            uint8_t *lWriteBuffer = &mWriteBuffer[ 0 ];
            memcpy( lWriteBuffer, &aOpCode, OPCODE_SIZE );
            memcpy( lWriteBuffer + OPCODE_SIZE, ( ( char * )&lBigEndianAddr ) + 1, ADDRESS_SIZE );
            memcpy( lWriteBuffer + OPCODE_SIZE + ADDRESS_SIZE, &lBigEndianDataSize, LENGTH_SIZE );

            if( lBytesSend != 0 )
            {
                memcpy( lWriteBuffer + HEADER_SIZE, mTransferInputBuffer + HEADER_SIZE + lBytesSent, lBytesSend );
            }

            uint16_t lCrc16 = LeddarUtils::LtCRCUtils::Crc16( CRCUTILS_CRC16_INIT_VALUE, lWriteBuffer, HEADER_SIZE + lBytesSend );
            lCrc16 = LeddarUtils::LtIntUtilities::SwapEndian<uint16_t>( lCrc16 );

            memcpy( lWriteBuffer + HEADER_SIZE + lBytesSend, &lCrc16, CRC_SIZE );
            mSpiInterface->Transfert( lWriteBuffer, mTransferOutputBuffer, HEADER_SIZE + lBytesSend + CRC_SIZE, true );

            if( aWaitAfterOpCode > 0 )
            {
                LeddarUtils::LtTimeUtils::Wait( aWaitAfterOpCode );
            }

            if( aPostIsReadyTimeout > 0 )
            {
                if( !IsDeviceReady( aPostIsReadyTimeout ) )
                {
                    throw LeddarException::LtTimeoutException( "Timeout expired. Device not ready for other operation.", true );
                }
            }

            // Check error on transaction
            if( lCRCTry > 0 )
            {
                uint16_t lTransactionInfo = 0;
                ReadRegister( 0x00FFFB00 + offsetof( sTransactionCfg, mTransactionInfo ), ( uint8_t * )&lTransactionInfo, sizeof( lTransactionInfo ), 0 );

                if( lTransactionInfo != REGMAP_NO_ERR )
                {
                    if( lCRCTry <= 1 )
                    {
                        throw LeddarException::LtComException( "Write operation failed: " + GetErrorInfo( lTransactionInfo ) + ". Address: "
                                                               + LeddarUtils::LtStringUtils::IntToString( aAddress, 16 )
                                                               + " size: " + LeddarUtils::LtStringUtils::IntToString( aDataSize ) );
                    }

                    LeddarUtils::LtTimeUtils::Wait( 10 );

                }
                else
                {
                    break;
                }
            }

            --lCRCTry;
        }

        lBytesSent += lBytesSend;
        lBytes2Send -= lBytesSend;
    }
    while( lBytesSent < aDataSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::HardReset( bool aEnterBootloader )
///
/// \brief  Hard Reset device.
///
/// \param  aEnterBootloader    Enter to bootloader on reset.
///
/// \author Patrick Boulay
/// \date   April 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::HardReset( bool aEnterBootloader )
{
    uint32_t lPins;
    uint32_t lMask = PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TCK_SCK ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDI_MOSI ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDO_MISO ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TMS_CS ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_RESET ) );
    uint32_t lDirection = DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TCK_SCK ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDI_MOSI ) ) |
                          DIR_IN( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDO_MISO ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TMS_CS ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_RESET ) );

    if( aEnterBootloader )
    {
        lPins = PIN_CLR( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TCK_SCK ) ) |
                PIN_CLR( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDI_MOSI ) ) |
                PIN_CLR( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TMS_CS ) );
    }
    else
    {
        lPins = PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TCK_SCK ) ) |
                PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDI_MOSI ) ) |
                PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TMS_CS ) );
    }

    mSpiInterface->InitGPIO( lDirection );
    mSpiInterface->WriteGPIO( lMask, lPins | PIN_CLR( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_RESET ) ) );
    LeddarUtils::LtTimeUtils::Wait( 100 );
    mSpiInterface->WriteGPIO( lMask, lPins | PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_RESET ) ) );
    LeddarUtils::LtTimeUtils::Wait( 2000 );
    mSpiInterface->SetSpiConfig( LdInterfaceSpi::CS_ACTIVEL, CHIP_SELECT,
                                 dynamic_cast<const LdConnectionInfoSpi *>( mConnectionInfo )->GetClock(),
                                 LdInterfaceSpi::CPOL_HIGH, LdInterfaceSpi::CPHA_FIRST, BITS_PER_SAMPLE );

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::InitIO()
///
/// \brief  Init the IO pins
///
/// \author Patrick Boulay
/// \date   June 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::InitIO()
{
    uint32_t lPins;
    uint32_t lMask = PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TCK_SCK ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDI_MOSI ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDO_MISO ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TMS_CS ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_RESET ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_0 ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_1 ) ) |
                     PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_2 ) );

    uint32_t lDirection = DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TCK_SCK ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDI_MOSI ) ) |
                          DIR_IN( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDO_MISO ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TMS_CS ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_RESET ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_0 ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_1 ) ) |
                          DIR_OUT( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_2 ) );

    lPins = PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TCK_SCK ) ) |
            PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TDI_MOSI ) ) |
            PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_TMS_CS ) ) |
            PIN_SET( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_RESET ) ) |
            PIN_CLR( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_0 ) ) |
            PIN_CLR( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_1 ) ) |
            PIN_CLR( mSpiInterface->GetGPIOPin( LdInterfaceSpi::SPI_PIN_GPIO_2 ) );

    mSpiInterface->InitGPIO( lDirection );
    mSpiInterface->WriteGPIO( lMask, lPins );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint16_t LdConnectionUniversalSpi::InternalBuffers( uint8_t *( &aInputBuffer ), uint8_t *( &aOutputBuffer ) )
///
/// \brief  Get the pointers of the internal input and output buffers.
///
/// \param [out]    aInputBuffer    Pointer of the input buffer.
/// \param [out]    aOutputBuffer   Pointer of the output buffer.
///
/// \return Buffer size.
///
/// \author Patrick Boulay
/// \date   June 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t
LdConnectionUniversalSpi::InternalBuffers( uint8_t *( &aInputBuffer ), uint8_t *( &aOutputBuffer ) )
{
    aInputBuffer = &mTransferInputBuffer[HEADER_SIZE];
    aOutputBuffer = &mTransferOutputBuffer[HEADER_SIZE];
    return SPI_UNIVERSAL_PAYLOAD_SIZE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::CrcCheck( uint8_t *aHeader, uint8_t *aData, const uint32_t &aDataSize, uint16_t aCrc16 )
///
/// \brief  CRC check on the last transaction and a buffer (with opcode, address and garbage bytes).
///
/// \exception  LeddarException::LtCrcException Thrown when a Lt CRC error condition occurs.
/// \exception  LtCrcException                  if the CRC is not valid.
///
/// \param [in,out] aHeader     Data pointer to the header to perform the checksum on.
/// \param [in,out] aData       Data   Pointer to the message to perform the checksum on.
/// \param          aDataSize   Data buffer length of the message in bytes.
/// \param          aCrc16      CRC value to compare to.
///
/// \author Patrick Boulay
/// \date   May 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::CrcCheck( uint8_t         *aHeader,
                                    uint8_t         *aData,
                                    const uint32_t  &aDataSize,
                                    uint16_t         aCrc16 )
{
    uint16_t lCrc16 = LeddarUtils::LtCRCUtils::Crc16( CRCUTILS_CRC16_INIT_VALUE, aHeader, HEADER_SIZE );
    lCrc16 = LeddarUtils::LtCRCUtils::Crc16( lCrc16, aData, aDataSize );

    if( lCrc16 != aCrc16 )
    {
        throw LeddarException::LtCrcException( "CRC error on transaction." );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdConnectionUniversalSpi::Reset( LeddarDefines::eResetType aType, bool aEnterBootloader )
///
/// \brief  Reset device.
///
/// \exception  LeddarException::LtNotConnectedException    Thrown when a Lt Not Connected error condition occurs.
/// \exception  std::invalid_argument                       Thrown when an invalid argument error condition occurs.
///
/// \param  aType               Reset type.
/// \param  aEnterBootloader    Flag to enter the bootloader.
///
/// \author Patrick Boulay
/// \date   April 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdConnectionUniversalSpi::Reset( LeddarDefines::eResetType aType, bool aEnterBootloader )
{
    if( mInterface->IsConnected() == false )
    {
        throw LeddarException::LtNotConnectedException( "SPI device not connected." );
    }

    if( aType == LeddarDefines::RT_SOFT_RESET )
    {
        Write( 0x99, 0, 0 );
        LeddarUtils::LtTimeUtils::Wait( 100 );

        if( !IsDeviceReady( 10000 ) )
        {
            throw std::invalid_argument( "Device never rebooted after software reset." );
        }

        return;
    }
    else if( aType == LeddarDefines::RT_HARD_RESET )
    {
        HardReset( aEnterBootloader );
        return;
    }

    throw std::invalid_argument( "Reset type not implemented for this type of device." );
}

#endif
