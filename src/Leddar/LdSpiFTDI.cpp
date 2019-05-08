// *****************************************************************************
// Module..: LeddarConnection
//
/// \file    LdSpiFTDI.cpp
///
/// \brief   Implementation of the FTDI SPI interface.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdSpiFTDI.h"
#if defined(BUILD_SPI_FTDI) && defined(BUILD_SPI)

#include "LdConnectionInfoSpi.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"

#include <cassert>
#include <stdexcept>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#else
#define __stdcall
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <WinTypes.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif
#include<dlfcn.h>
#endif

#ifdef _WIN32
#ifdef _MSC_VER
#define gLib HMODULE
#else
#define gLib HANDLE
#endif
#else
#define gLib void*
#endif

#ifdef _WIN32
#define GET_FUN_POINTER GetProcAddress
#else
#define GET_FUN_POINTER dlsym
#endif

#define CHECK_NULL(exp){if(exp==nullptr){ throw LeddarException::LtException( std::string( "Function " ) +  __FUNCTION__ + " not found in the dynamic library." );}else{;}};

using namespace LeddarConnection;


typedef enum
{
    ADBUS0_TCK_SCK = 0,  // [OUT]
    ADBUS1_TDI_MOSI = 1, // [OUT]
    ADBUS2_TDO_MISO = 2, // [IN]
    ADBUS3_TMS_CS = 3,   // [OUT]
    ADBUS4_GPIOL0 = 4,   // [IN/OUT]
    ADBUS5_GPIOL1 = 5,   // [IN/OUT]
    ADBUS6_GPIOL2 = 6,   // [IN/OUT]
    ADBUS7_GPIOL3 = 7,   // [IN/OUT]

    ADBUS_RST = ADBUS7_GPIOL3
} eADBusPins;


// The FTDI library includes windows.h, so it must be first for typdefs.
// We create a namespace for it because it redefines some typedefs (uint32, ...)
namespace FTDI
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
#endif
#include "libMPSSE_spi.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif

    typedef FT_STATUS( *pfunc_SPI_GetNumChannels )( uint32 *numChannels );
    pfunc_SPI_GetNumChannels p_SPI_GetNumChannels;
    typedef FT_STATUS( *pfunc_SPI_GetChannelInfo )( uint32 index, FT_DEVICE_LIST_INFO_NODE *chanInfo );
    pfunc_SPI_GetChannelInfo p_SPI_GetChannelInfo;
    typedef FT_STATUS( *pfunc_SPI_OpenChannel )( uint32 index, FT_HANDLE *handle );
    pfunc_SPI_OpenChannel p_SPI_OpenChannel;
    typedef FT_STATUS( *pfunc_SPI_InitChannel )( FT_HANDLE handle, ChannelConfig *config );
    pfunc_SPI_InitChannel p_SPI_InitChannel;
    typedef FT_STATUS( *pfunc_SPI_CloseChannel )( FT_HANDLE handle );
    pfunc_SPI_CloseChannel p_SPI_CloseChannel;
    typedef FT_STATUS( *pfunc_SPI_Read )( FT_HANDLE handle, uint8 *buffer, uint32 sizeToTransfer, uint32 *sizeTransfered, uint32 options );
    pfunc_SPI_Read p_SPI_Read;
    typedef FT_STATUS( *pfunc_SPI_Write )( FT_HANDLE handle, uint8 *buffer, uint32 sizeToTransfer, uint32 *sizeTransfered, uint32 options );
    pfunc_SPI_Write p_SPI_Write;
    typedef FT_STATUS( *pfunc_SPI_ReadWrite )( FT_HANDLE handle, uint8 *inBuffer, uint8 *outBuffer, uint32 sizeToTransfer, uint32 *sizeTransferred, uint32 transferOptions );
    pfunc_SPI_ReadWrite p_SPI_ReadWrite;
    typedef FT_STATUS( *pfunc_SPI_ToggleCS )( FT_HANDLE handle, bool state );
    pfunc_SPI_ToggleCS p_SPI_ToggleCS;

    typedef FT_STATUS( *pfunc_FT_ReadGPIO )( FT_HANDLE handle, uint8 *value );
    pfunc_FT_ReadGPIO p_FT_ReadGPIO;
    typedef FT_STATUS( *pfunc_FT_WriteGPIO )( FT_HANDLE handle, uint8 dir, uint8 value );
    pfunc_FT_WriteGPIO p_FT_WriteGPIO;
    typedef FT_STATUS( *pfunc_FT_Read )( FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesReturned );
    pfunc_FT_Read p_FT_Read;
    typedef FT_STATUS( __stdcall *pfunc_FT_Write )( FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten );
    pfunc_FT_Write p_FT_Write;
    typedef FT_STATUS( *pfunc_FT_GetQueueStatus )( FT_HANDLE ftHandle, DWORD *dwRxBytes );
    pfunc_FT_GetQueueStatus p_FT_GetQueueStatus;
}

// This static class initialize the MSSE lib at the begining and clean it up at the end.
MSSELib *LdSpiFTDI::mMSSELibLoader = nullptr;

MSSELib::MSSELib()
{
    // Load the dynamic libraries
#ifdef _WIN32
    mLibMPSSE = ( void * )LoadLibrary( "libMPSSE.dll" );
    mLibFTDI = ( void * )LoadLibrary( "ftd2xx.dll" );

    if( nullptr == mLibMPSSE )
    {
        throw LeddarException::LtException( "Failed loading libMPSSE.dll. Please check if the file exists in the working directory." );
    }

    if( nullptr == mLibFTDI )
    {
        throw LeddarException::LtException( "Failed loading ftd2xx.dll. Please check if the file exists in the working directory." );
    }

#elif __linux
    mLibMPSSE = dlopen( "libMPSSE.so", RTLD_LAZY );

    if( !mLibMPSSE )
    {
        throw LeddarException::LtException( "Failed loading libMPSSE.so. Please check if the file exists in the shared library folder(/usr/lib or /usr/lib64)." );
    }

    mLibFTDI = dlopen( "libftd2xx.so", RTLD_LAZY );

    if( !mLibFTDI )
    {
        throw LeddarException::LtException( "Failed loading libftd2xx.so. Please check if the file exists in the shared library folder(/usr/lib or /usr/lib64)." );
    }

#endif

    FTDI::p_SPI_GetNumChannels = ( FTDI::pfunc_SPI_GetNumChannels )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_GetNumChannels" );
    CHECK_NULL( FTDI::p_SPI_GetNumChannels );
    FTDI::p_SPI_GetChannelInfo = ( FTDI::pfunc_SPI_GetChannelInfo )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_GetChannelInfo" );
    CHECK_NULL( FTDI::p_SPI_GetChannelInfo );
    FTDI::p_SPI_OpenChannel = ( FTDI::pfunc_SPI_OpenChannel )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_OpenChannel" );
    CHECK_NULL( FTDI::p_SPI_OpenChannel );
    FTDI::p_SPI_InitChannel = ( FTDI::pfunc_SPI_InitChannel )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_InitChannel" );
    CHECK_NULL( FTDI::p_SPI_InitChannel );
    FTDI::p_SPI_Read = ( FTDI::pfunc_SPI_Read )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_Read" );
    CHECK_NULL( FTDI::p_SPI_Read );
    FTDI::p_SPI_Write = ( FTDI::pfunc_SPI_Write )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_Write" );
    CHECK_NULL( FTDI::p_SPI_Write );
    FTDI::p_SPI_CloseChannel = ( FTDI::pfunc_SPI_CloseChannel )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_CloseChannel" );
    CHECK_NULL( FTDI::p_SPI_CloseChannel );
    FTDI::p_SPI_ReadWrite = ( FTDI::pfunc_SPI_ReadWrite )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_ReadWrite" );
    CHECK_NULL( FTDI::p_SPI_ReadWrite );
    FTDI::p_SPI_ToggleCS = ( FTDI::pfunc_SPI_ToggleCS )GET_FUN_POINTER( ( gLib )mLibMPSSE, "SPI_ToggleCS" );
    CHECK_NULL( FTDI::p_SPI_ToggleCS );

    FTDI::p_FT_ReadGPIO = ( FTDI::pfunc_FT_ReadGPIO )GET_FUN_POINTER( ( gLib )mLibMPSSE, "FT_ReadGPIO" );
    CHECK_NULL( FTDI::p_FT_ReadGPIO );
    FTDI::p_FT_WriteGPIO = ( FTDI::pfunc_FT_WriteGPIO )GET_FUN_POINTER( ( gLib )mLibMPSSE, "FT_WriteGPIO" );
    CHECK_NULL( FTDI::p_FT_WriteGPIO );
    FTDI::p_FT_Read = ( FTDI::pfunc_FT_Read )GET_FUN_POINTER( ( gLib )mLibFTDI, "FT_Read" );
    CHECK_NULL( FTDI::p_FT_Read );
    FTDI::p_FT_Write = ( FTDI::pfunc_FT_Write )GET_FUN_POINTER( ( gLib )mLibFTDI, "FT_Write" );
    CHECK_NULL( FTDI::p_FT_Write );
    FTDI::p_FT_GetQueueStatus = ( FTDI::pfunc_FT_GetQueueStatus )GET_FUN_POINTER( ( gLib )mLibFTDI, "FT_GetQueueStatus" );
    CHECK_NULL( FTDI::p_FT_GetQueueStatus );


}

MSSELib::~MSSELib()
{
#ifdef _WIN32
    FreeLibrary( ( HMODULE )mLibMPSSE );
    mLibMPSSE = nullptr;
    FreeLibrary( ( HMODULE )mLibFTDI );
    mLibFTDI = nullptr;
#elif __linux
    dlclose( mLibMPSSE );
    mLibMPSSE = nullptr;
    dlclose( mLibFTDI );
    mLibFTDI = nullptr;
#endif

}



// *****************************************************************************
// Function: LdSpiFTDI::LdSpiFTDI
//
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************
LdSpiFTDI::LdSpiFTDI( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface ) :
    LdInterfaceSpi( aConnectionInfo, aInterface ),
    mHandle( nullptr ),
    mGPIOACBusDirection( 0 ),
    mGPIOADBusDirection( 0 ),
    mGPIOACBusDirectionMask( 0xFF00 ),
    mGPIOADBusDirectionMask( 0xFF )
{
    InitLib();
}

// *****************************************************************************
// Function: LdSpiFTDI::~LdSpiFTDI
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************
LdSpiFTDI::~LdSpiFTDI()
{
}

// *****************************************************************************
// Function: LdSpiFTDI::Connect
//
/// \brief   Connect to SPI device.
///
/// \exception LtComException If the device failed to connect.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdSpiFTDI::Connect()
{
    const LdConnectionInfoSpi *lConnectionInfo = dynamic_cast< const LdConnectionInfoSpi *>( mConnectionInfo );

    FTDI::FT_STATUS lResult = FTDI::p_SPI_OpenChannel( lConnectionInfo->GetIntAddress(), &mHandle );

    if( lResult != FTDI::FT_OK )
    {
        mHandle = nullptr;
        throw LeddarException::LtComException( "Failed connecting to SPI device, SPI_OpenChannel(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }
}

// *****************************************************************************
// Function: LdSpiFTDI::Disconnect
//
/// \brief   Disconnect to SPI device.
///
/// \exception LtComException If the device failed to disconnect.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdSpiFTDI::Disconnect( void )
{
    // Exit if the device is already disconnected.
    if( mHandle == nullptr )
    {
        return;
    }

    FTDI::FT_STATUS lResult = FTDI::p_SPI_CloseChannel( mHandle );

    if( lResult != FTDI::FT_OK )
    {
        mHandle = nullptr;
        throw LeddarException::LtComException( "Failed disconnecting to SPI device, SPI_CloseChannel(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }

    mHandle = nullptr;
}

// *****************************************************************************
// Function: LdSpiFTDI::Transfert
//
/// \brief   Transfert data without disabling chip select at the end.
///
/// \param  aInputData    Array of data to send.
/// \param  aOutputData   Array of data to receive.
/// \param  aDataSize     Length of the data array.
/// \param  aEndTransfert Disable chip select to end the transfert
///
/// \exception LtComException Error on FTDI function call.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdSpiFTDI::Transfert( uint8_t *aInputData, uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( aDataSize == 0 )
    {
        throw std::invalid_argument( "Invalid data size." );
    }

    if( mHandle == nullptr )
    {
        throw LeddarException::LtComException( "SPI handle not valid." );
    }

    FTDI::uint32 lTransferedSize = 0;
    uint32_t lOptions = SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | ( aEndTransfert ? SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE : 0 );
    FTDI::FT_STATUS lResult = FTDI::p_SPI_ReadWrite( mHandle, aOutputData, aInputData, aDataSize, &lTransferedSize, lOptions );

    if( lResult != FTDI::FT_OK )
    {
        throw LeddarException::LtComException( "Error on FTDI SPI read/write, SPI_ReadWrite(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }
}

// *****************************************************************************
// Function: LdSpiFTDI::Read
//
/// \brief   Read data with disabling chip select at the end.
///
/// \param  aOutputData   Array of data to receive.
/// \param  aDataSize     Length of the data array.
/// \param  aEndTransfert Disable chip select to end the transfert
///
/// \exception LtComException Error on FTDI function call.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdSpiFTDI::Read( uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert )
{
    FTDI::uint32 lTransferedSize = 0;
    uint32_t lOptions = SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | ( aEndTransfert ? SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE : 0 );
    FTDI::FT_STATUS lResult = FTDI::p_SPI_Read( mHandle, aOutputData, aDataSize, &lTransferedSize, lOptions );

    if( lResult != FTDI::FT_OK )
    {
        throw LeddarException::LtComException( "Error on FTDI SPI read/write, SPI_Read(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }
}

// *****************************************************************************
// Function: LdSpiFTDI::Write
//
/// \brief   Write data without disabling chip select at the end.
///
/// \param  aOutputData    Array of data to send.
/// \param  aDataSize     Length of the data array.
/// \param  aEndTransfert Disable chip select to end the transfer
///
/// \exception LtComException Error on FTDI function call.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdSpiFTDI::Write( uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert )
{
    FTDI::uint32 lTransferedSize = 0;
    uint32_t lOptions = SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | ( aEndTransfert ? SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE : 0 );
    FTDI::FT_STATUS lResult = FTDI::p_SPI_Write( mHandle, aOutputData, aDataSize, &lTransferedSize, lOptions );

    if( lResult != FTDI::FT_OK )
    {
        throw LeddarException::LtComException( "Error on FTDI SPI write, SPI_Write(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }
}


// *****************************************************************************
// Function: LdSpiFTDI::EndTransfert
//
/// \brief   Terminate transfert by setting the chip select to 1.
///
/// \exception LtComException on device not connected, handle not valid or FTDI function call.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdSpiFTDI::EndTransfert( void )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( mHandle == nullptr )
    {
        throw LeddarException::LtComException( "SPI handle not valid." );
    }

    uint32_t lOptions = SPI_TRANSFER_OPTIONS_SIZE_IN_BITS | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE;
    FTDI::uint32 lTransferedSize = 0;
    uint8_t lBuffer[ 1 ] = { 0 };
    FTDI::FT_STATUS lResult = FTDI::p_SPI_Write( mHandle, lBuffer, 0, &lTransferedSize, lOptions );

    if( lResult != FTDI::FT_OK )
    {
        throw LeddarException::LtComException( "Error on FTDI SPI write, SPI_Write(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }
}

// *****************************************************************************
// Function: LdSpiFTDI::GetDeviceList
//
/// \brief   Return list of connected device.
///          The function release the ownership of the returned objects.
///
/// \return  Vector of LdConnectionInfo
///
/// \exception LtComException Error to get number of channels and get channel info
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************
std::vector<LdConnectionInfo *>
LdSpiFTDI::GetDeviceList( void )
{
    InitLib();
    std::vector<LdConnectionInfo *> lResultList;

    FTDI::uint32 lDeviceNumber = 0;
    FTDI::FT_STATUS lResult = FTDI::p_SPI_GetNumChannels( &lDeviceNumber );

    if( lResult != FTDI::FT_OK )
    {
        throw LeddarException::LtComException( "Error on FTDI get number of channels, SPI_GetNumChannels(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }

    for( uint32_t i = 0; i < lDeviceNumber; ++i )
    {
        FTDI::FT_DEVICE_LIST_INFO_NODE lDeviceInfo;
        lResult = FTDI::p_SPI_GetChannelInfo( i, &lDeviceInfo );

        if( lResult != FTDI::FT_OK )
        {
            throw LeddarException::LtComException( "Error on FTDI get channel info, SPI_GetChannelInfo(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
        }

        std::string lDescription = std::string( "FTDI : " ) + std::string( lDeviceInfo.Description ) + std::string( " : " ) + std::string( lDeviceInfo.SerialNumber );


        LdConnectionInfoSpi *lSpiInfo = new LdConnectionInfoSpi( LdConnectionInfoSpi::CT_SPI_FTDI, lDescription, i );
        lResultList.push_back( lSpiInfo );

    }

    return lResultList;
}


// *****************************************************************************
// Function: LdSpiFTDI::SetSpiConfig
//
/// \brief   Set SPI configuration
///
/// \param  aCSMode Chip select mode (CS_ACTIVEL or CS_ACTIVEH)
/// \param  aChipSelect Chip select line to use to talk with slave, zero-based
/// \param  aClockRate Clock rate in kHz
/// \param  aClockPolarity Clock polarity, CPOL_HIGH or CPOL_LOW
/// \param  aClockPhase Clock phase, CPHA_FIRST or CPHA_SECOND
/// \param  aBitsPerSample Number of bits per sample
///
/// \exception LtComException Error on device not connected, handle not valid or FTDI function call.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************
void
LdSpiFTDI::SetSpiConfig( eCSMode aCSMode, uint32_t aChipSelect, uint32_t aClockRate, eClockPolarity aClockPolarity, eClockPhase aClockPhase, uint32_t aBitsPerSample )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( mHandle == nullptr )
    {
        throw LeddarException::LtComException( "SPI handle not valid." );
    }

    FTDI::ChannelConfig lConfig;

    // Validate input arguments
    if( ( aChipSelect   > 15 )
            || ( aClockRate  > 30000 )
            || ( aBitsPerSample  > 64 )
            || ( aCSMode != CS_ACTIVEH && aCSMode != CS_ACTIVEL )
            || ( aClockPolarity != CPOL_HIGH  && aClockPolarity != CPOL_LOW )
            || ( aClockPhase != CPHA_FIRST && aClockPhase != CPHA_SECOND ) )
    {
        throw std::invalid_argument( "Invalid argument." );
    }

    lConfig.ClockRate = aClockRate * 1000; // to MHz
    lConfig.LatencyTimer = 2;
    lConfig.configOptions = 0;
    lConfig.Pin = 0;

    if( aClockPolarity == CPOL_HIGH && aClockPhase == CPHA_FIRST )
    {
        lConfig.configOptions |= SPI_CONFIG_OPTION_MODE0;
    }
    else if( aClockPolarity == CPOL_HIGH && aClockPhase == CPHA_SECOND )
    {
        lConfig.configOptions |= SPI_CONFIG_OPTION_MODE1;
    }
    else if( aClockPolarity == CPOL_LOW && aClockPhase == CPHA_FIRST )
    {
        lConfig.configOptions |= SPI_CONFIG_OPTION_MODE2;
    }
    else
    {
        lConfig.configOptions |= SPI_CONFIG_OPTION_MODE3;
    }


    if( aCSMode == CS_ACTIVEL )
    {
        lConfig.configOptions |= SPI_CONFIG_OPTION_CS_ACTIVELOW;
    }

    // Choose the chip select line
    lConfig.configOptions |= SPI_CONFIG_OPTION_CS_DBUS3;

    FTDI::FT_STATUS lResult = FTDI::p_SPI_InitChannel( mHandle, &lConfig );

    if( lResult != FTDI::FT_OK )
    {
        throw LeddarException::LtComException( "Error to init SPI device: " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }

    lResult = FTDI::p_SPI_ToggleCS( mHandle, false );

    if( lResult != FTDI::FT_OK )
    {
        throw LeddarException::LtComException( "Error on FTDI toggle chip select, SPI_ToggleCS(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }

}

// *****************************************************************************
// Function: LdSpiFTDI::ReadGPIO
//
/// \brief   Read GPIO pins
///
/// \return  Pins values in a bifield.
///
/// \exception LtComException Error on device not connected, handle not valid or FTDI function call.
///
/// \exception LtComException Error on FTDI function call.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

uint32_t
LdSpiFTDI::ReadGPIO( const uint32_t &aPinsMask )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( mHandle == nullptr )
    {
        throw LeddarException::LtComException( "SPI handle not valid." );
    }

    uint32_t lOutputPins = 0xFFFF;

    // ACBus part
    if( ( aPinsMask | mGPIOACBusDirectionMask ) == mGPIOACBusDirectionMask )
    {
        uint8_t lOuputACBusPins;

        FTDI::FT_STATUS lResult = FTDI::p_FT_ReadGPIO( mHandle, &lOuputACBusPins );

        if( lResult != FTDI::FT_OK )
        {
            throw LeddarException::LtComException( "Error on FTDI SPI to read GPIO (ACBus), FT_ReadGPIO(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
        }

        lOutputPins &= 0xFF;
        lOutputPins |= lOuputACBusPins << 8;
    }

    // ADBus part
    if( ( aPinsMask | mGPIOADBusDirectionMask ) == mGPIOADBusDirectionMask )
    {
        uint8_t lInputBuffer[ 32 ];
        uint8_t lOutputBuffer[ 32 ];
        uint16_t lNumBytesToSend = 0;
        uint16_t lNumBytesToRead = 0;
        uint16_t lNumBytesSent = 0;
        uint16_t lNumBytesRead = 0;

        lOutputBuffer[ lNumBytesToSend++ ] = 0x81;
        FTDI::FT_STATUS lResult = FTDI::p_FT_Write( mHandle, lOutputBuffer, lNumBytesToSend, ( LPDWORD )&lNumBytesSent );

        if( lResult != FTDI::FT_OK )
        {
            throw LeddarException::LtComException( "Error on FTDI SPI to read GPIO (ADBus), FT_Write(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
        }

        // Read the low GPIO byte
        lNumBytesToSend = 0; // Reset output buffer pointer
        LeddarUtils::LtTimeUtils::Wait( 2 ); // Wait for data to be transmitted and status

        // to be returned by the device driver
        // - see latency timer above
        // Check the receive buffer - there should be one byte
        lResult = FTDI::p_FT_GetQueueStatus( mHandle, ( LPDWORD )&lNumBytesToRead );

        if( lResult != FTDI::FT_OK )
        {
            throw LeddarException::LtComException( "Error on FTDI SPI to read GPIO (ADBus), FT_GetQueueStatus(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
        }

        // Get the number of bytes in the FT2232H receive buffer
        lResult = FTDI::p_FT_Read( mHandle, &lInputBuffer, lNumBytesToRead, ( LPDWORD )&lNumBytesRead );

        if( lResult != FTDI::FT_OK )
        {
            throw LeddarException::LtComException( "Error on FTDI SPI to read GPIO (ADBus), FT_Read(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
        }

        lOutputPins &= 0xFF00;
        lOutputPins |= lInputBuffer[0];
    }

    return lOutputPins;

}

// *****************************************************************************
// Function: LdSpiFTDI::WriteGPIO
//
/// \brief   Write GPIO pins
///
/// \param  aPinsMask   Mask to define if we write to ACBus and/or ADBus
/// \param  aPinsValues Pins values in a bitfield.
///
/// \exception LtComException Error on device not connected, handle not valid or FTDI function call.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

void
LdSpiFTDI::WriteGPIO( const uint32_t &aPinsMask, const uint32_t &aPinsValues )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( mHandle == nullptr )
    {
        throw LeddarException::LtComException( "SPI handle not valid." );
    }


    // ACBus part
    if( ( aPinsMask | mGPIOACBusDirectionMask ) == mGPIOACBusDirectionMask )
    {
        uint8_t lPinsValues = aPinsValues >> 8;

        FTDI::FT_STATUS lResult = FTDI::p_FT_WriteGPIO( mHandle, mGPIOACBusDirection, lPinsValues );

        if( lResult != FTDI::FT_OK )
        {
            throw LeddarException::LtComException( "Error on FTDI SPI to write GPIO (ACBus), FT_WriteGPIO(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
        }
    }

    // ADBus part
    if( ( aPinsMask | mGPIOADBusDirectionMask ) == mGPIOADBusDirectionMask )
    {
        uint8_t lPinsValues = ( uint8_t )aPinsValues;

        // Set initial states of the MPSSE interface
        uint32_t lNumBytesToSend = 0;
        uint32_t lNumBytesSent = 0;
        uint8_t lBuffer[ 32 ];
        lBuffer[ lNumBytesToSend++ ] = 0x80;                    // Configure data bits low-byte of MPSSE port
        lBuffer[ lNumBytesToSend++ ] = lPinsValues;             // Initial state config above
        lBuffer[ lNumBytesToSend++ ] = mGPIOADBusDirection;     // Direction config above

        FTDI::FT_STATUS lResult = FTDI::p_FT_Write( mHandle, lBuffer, lNumBytesToSend, ( LPDWORD )&lNumBytesSent );    // Send off the low GPIO config

        if( lResult != FTDI::FT_OK )
        {
            throw LeddarException::LtComException( "Error on FTDI SPI to write GPIO (ADBus), FT_Write(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
        }
    }
}

// *****************************************************************************
// Function: LdSpiFTDI::InitGPIO
//
/// \brief   Init direction of the pins
///
/// \param  aDirection Direction of the pins: 0=in, 1=out.
///
/// \exception LtComException Error on device not connected, handle not valid or FTDI function call.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

void
LdSpiFTDI::InitGPIO( const uint32_t &aDirection )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( mHandle == nullptr )
    {
        throw LeddarException::LtComException( "SPI handle not valid." );
    }


    mGPIOACBusDirection = aDirection >> 8;
    mGPIOADBusDirection = aDirection & 0xFF;

    // ACBus part
    FTDI::FT_STATUS lResult = FTDI::p_FT_WriteGPIO( mHandle, mGPIOACBusDirection, 0 );

    if( lResult != FTDI::FT_OK )
    {
        throw LeddarException::LtComException( "Error on FTDI SPI to write GPIO, FT_WriteGPIO(): " + LeddarUtils::LtStringUtils::IntToString( lResult ) );
    }
}

// *****************************************************************************
// Function: LdSpiFTDI::GetGPIOPin
//
/// \brief   Get the respective pin index.
///
/// \param  aPin Pin number.
///
/// \return Return the pin number associated FTDI pin.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

uint8_t
LdSpiFTDI::GetGPIOPin( eSpiPin aPin )
{
    switch( aPin )
    {
        case SPI_PIN_TCK_SCK:
            return ADBUS0_TCK_SCK;
            break;

        case SPI_PIN_TDI_MOSI:
            return ADBUS1_TDI_MOSI;
            break;

        case SPI_PIN_TDO_MISO:
            return ADBUS2_TDO_MISO;
            break;

        case SPI_PIN_TMS_CS:
            return ADBUS3_TMS_CS;
            break;

        case SPI_PIN_RESET:
            return ADBUS_RST;
            break;

        case SPI_PIN_GPIO_0:
            return ADBUS4_GPIOL0;
            break;

        case SPI_PIN_GPIO_1:
            return ADBUS5_GPIOL1;
            break;

        case SPI_PIN_GPIO_2:
            return ADBUS6_GPIOL2;
            break;

        default:
            return 0;
            break;
    }
}

// *****************************************************************************
// Function: LdSpiFTDI::InitLib
//
/// \brief   Init the external library object container
///
/// \author  Patrick Boulay
///
/// \since   January 2017
// *****************************************************************************

void
LdSpiFTDI::InitLib( void )
{
    if( mMSSELibLoader == nullptr )
    {
        mMSSELibLoader = new MSSELib();
    }
}

#endif
