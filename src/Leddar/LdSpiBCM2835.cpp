// *****************************************************************************
// Module..: LeddarConnection
//
/// \file    LdSpiBCM2835.cpp
///
/// \brief   Implementation of the BCM2835 SPI interface on a Raspberry PI.
///          You need to uncomment the define SPI_BCM2835 in LdDefines.h to compile this class.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdSpiBCM2835.h"
#if defined(BUILD_SPI_BCM2835) && defined(BUILD_SPI)

#include "LdConnectionInfoSpi.h"
#include "LtExceptions.h"
#include "LtTimeUtils.h"

#include <bcm2835.h>

#include <stdexcept>

using namespace LeddarConnection;
bool LdSpiBCM2835::mInitialized = false;

// *****************************************************************************
// Function: LdSpiBCM2835::LdSpiBCM2835
//
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************
LdSpiBCM2835::LdSpiBCM2835( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface ) :
    LdInterfaceSpi( aConnectionInfo, aInterface ),
    mIsConnected( false )
{
}

// *****************************************************************************
// Function: LdSpiBCM2835::~LdSpiBCM2835
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************
LdSpiBCM2835::~LdSpiBCM2835()
{

}

// *****************************************************************************
// Function: LdSpiBCM2835::Connect
//
/// \brief   Connect to SPI device.
///
/// \exception LtComException If the device failed to connect.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::Connect()
{
    if( !mInitialized )
    {
        InitLib();
        mInitialized = true;
    }

    if( !bcm2835_spi_begin() )
    {
        throw LeddarException::LtComException( "Failed connecting to SPI device, bcm2835_spi_begin(). Are you running as root??" );
    }

    mIsConnected = true;
}

// *****************************************************************************
// Function: LdSpiBCM2835::Disconnect
//
/// \brief   Disconnect to SPI device.
///
/// \exception LtComException If the device failed to disconnect.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::Disconnect( void )
{
    bcm2835_spi_end();
    bcm2835_close();
}

// *****************************************************************************
// Function: LdSpiBCM2835::Transfert
//
/// \brief   Transfert data without disabling chip select at the end.
///
/// \param  aInputData    Array of data to send.
/// \param  aOutputData   Array of data to receive.
/// \param  aDataSize     Length of the data array.
/// \param  aEndTransfert Disable chip select to end the transfert
///
/// \exception LtComException Error on device not connected.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::Transfert( uint8_t *aInputData, uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( aDataSize == 0 )
    {
        throw std::invalid_argument( "Invalid data size." );
    }

    ChipSelectEnable();

    bcm2835_spi_transfernb( ( char * )aInputData, ( char * )aOutputData, aDataSize );

    if( aEndTransfert )
    {
        ChipSelectDisable();
    }

}

// *****************************************************************************
// Function: LdSpiBCM2835::Read
//
/// \brief   Read data with disabling chip select at the end.
///
/// \param  aOutputData   Array of data to receive.
/// \param  aDataSize     Length of the data array.
/// \param  aEndTransfert Disable chip select to end the transfert
///
/// \exception LtComException Error on device not connected.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::Read( uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( aDataSize == 0 )
    {
        throw std::invalid_argument( "Invalid data size." );
    }

    ChipSelectEnable();

    bcm2835_spi_transfern( ( char * )aOutputData, aDataSize );

    if( aEndTransfert )
    {
        ChipSelectDisable();
    }

    // To avoid 2 read transactions too close
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( 5 );
}

// *****************************************************************************
// Function: LdSpiBCM2835::Write
//
/// \brief   Write data without disabling chip select at the end.
///
/// \param  aInputData    Array of data to send.
/// \param  aDataSize     Length of the data array.
/// \param  aEndTransfert Disable chip select to end the transfert
///
/// \exception LtComException Error on device not connected.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::Write( uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    if( aDataSize == 0 )
    {
        throw std::invalid_argument( "Invalid data size." );
    }

    ChipSelectEnable();

    bcm2835_spi_writenb( ( char * )aOutputData, aDataSize );

    if( aEndTransfert )
    {
        ChipSelectDisable();
    }
}


// *****************************************************************************
// Function: LdSpiBCM2835::EndTransfert
//
/// \brief   Terminate transfert by setting the chip select to HIGH.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::EndTransfert( void )
{
    ChipSelectDisable();
}

// *****************************************************************************
// Function: LdSpiBCM2835::isConnected
//
/// \brief   Return if the SPI device is connected.
///
/// \return  true if the device is connected.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

bool
LdSpiBCM2835::IsConnected( void )
{
    return mIsConnected;
}

// *****************************************************************************
// Function: LdSpiBCM2835::GetDeviceList
//
/// \brief   Return list of connected device. (not used)
///          The function release the ownership of the returned objects.
///
/// \return  Vector of LdConnectionInfo
///
/// \exception LtComException Error to get number of channels and get channel info
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

std::vector<LdConnectionInfo *>
LdSpiBCM2835::GetDeviceList( void )
{
    InitLib();
    std::vector<LdConnectionInfo *> lResultList;

    return lResultList;
}


// *****************************************************************************
// Function: LdSpiBCM2835::SetSpiConfig
//
/// \brief   Set SPI configuration
///
/// \param  aCSMode Chip select mode (CS_ACTIVEL or CS_ACTIVEH)
/// \param  aChipSelect Chip select line to use to talk with slave, zero-based
/// \param  aClockRate Clock rate in kHz (not used)
/// \param  aClockPolarity Clock polarity, CPOL_HIGH or CPOL_LOW
/// \param  aClockPhase Clock phase, CPHA_FIRST or CPHA_SECOND
/// \param  aBitsPerSample Number of bits per sample
///
/// \exception LtComException Error on device not connected.
/// \exception invalid_argument Error on invalid arguments.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************
void
LdSpiBCM2835::SetSpiConfig( eCSMode aCSMode, uint32_t aChipSelect, uint32_t aClockRate, eClockPolarity aClockPolarity, eClockPhase aClockPhase, uint32_t aBitsPerSample )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

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

    if( aClockPolarity == CPOL_HIGH && aClockPhase == CPHA_FIRST )
    {
        bcm2835_spi_setDataMode( BCM2835_SPI_MODE0 );
    }
    else if( aClockPolarity == CPOL_HIGH && aClockPhase == CPHA_SECOND )
    {
        bcm2835_spi_setDataMode( BCM2835_SPI_MODE1 );
    }
    else if( aClockPolarity == CPOL_LOW && aClockPhase == CPHA_FIRST )
    {
        bcm2835_spi_setDataMode( BCM2835_SPI_MODE2 );
    }
    else
    {
        bcm2835_spi_setDataMode( BCM2835_SPI_MODE3 );
    }

    bcm2835_spi_setBitOrder( BCM2835_SPI_BIT_ORDER_MSBFIRST );      // The default
    bcm2835_spi_setClockDivider( BCM2835_SPI_CLOCK_DIVIDER_256 ); // The default

    // Choose the chip select line
    bcm2835_spi_chipSelect( BCM2835_SPI_CS_NONE );                       // The default

    // Get the control of chipselect by setting the CE0 to an output function
    bcm2835_gpio_fsel( RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_OUTP );
}

// *****************************************************************************
// Function: LdSpiBCM2835::ReadGPIO
//
/// \brief   Read GPIO pins
///
/// \return  Pins values in a bifield.
///
/// \exception LtComException Error on device not connected.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

uint32_t
LdSpiBCM2835::ReadGPIO( const uint32_t &aPinsMask )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    uint32_t lOutputPins = 0x0;

    for( int i = 0; i < sizeof( aPinsMask ) * 8; i++ )
    {
        // Check if the pin is in the mask
        if( ( aPinsMask & ( 1 << i ) ) != 0 )
        {
            if( bcm2835_gpio_lev( i ) == HIGH )
            {
                lOutputPins |= ( 1 << i );
            }


        }
    }

    return lOutputPins;
}

// *****************************************************************************
// Function: LdSpiBCM2835::WriteGPIO
//
/// \brief   Write GPIO pins
///
/// \param  aPinsMask   Mask to define if we write to GPIO.
/// \param  aPinsValues Pins values in a bitfield.
///
/// \exception LtComException Error on device not connected.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::WriteGPIO( const uint32_t &aPinsMask, const uint32_t &aPinsValues )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }

    for( int i = 0; i < sizeof( aPinsMask ) * 8; i++ )
    {
        // Check if the pin is in the mask
        if( ( aPinsMask & ( 1 << i ) ) != 0 )
        {
            bcm2835_gpio_write( i, ( aPinsValues & ( 1 << i ) ) != 0 ? HIGH : LOW );
        }
    }

}

// *****************************************************************************
// Function: LdSpiBCM2835::InitGPIO
//
/// \brief   Init direction of the pins (not used).
///
/// \param  aDirection Direction of the pins: 0=in, 1=out.
///
/// \exception LtComException Error on device not connected.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::InitGPIO( const uint32_t &aDirection )
{
    if( !IsConnected() )
    {
        throw LeddarException::LtComException( "SPI device not connected." );
    }
}

// *****************************************************************************
// Function: LdSpiBCM2835::GetGPIOPin
//
/// \brief   Get the respective pin index.
///
/// \param  aPin Pin number.
///
/// \return Return the pin number associated Raspberry PI pin.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

uint8_t
LdSpiBCM2835::GetGPIOPin( eSpiPin aPin )
{
    switch( aPin )
    {
        case SPI_PIN_TCK_SCK:
            return RPI_GPIO_P1_23;
            break;

        case SPI_PIN_TDI_MOSI:
            return RPI_GPIO_P1_19;
            break;

        case SPI_PIN_TDO_MISO:
            return RPI_GPIO_P1_21;
            break;

        case SPI_PIN_TMS_CS:
            return RPI_GPIO_P1_24;
            break;

        case SPI_PIN_RESET:
            return RPI_GPIO_P1_22;
            break;

        default:
            return 0;
            break;
    }
}

// *****************************************************************************
// Function: LdSpiBCM2835::InitLib
//
/// \brief   Init the external library object container
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::InitLib( void )
{
    if( !bcm2835_init() )
    {
        throw LeddarException::LtComException( "bcm2835_init failed. Are you running as root??" );
    }
}

// *****************************************************************************
// Function: LdSpiBCM2835::ChipSelectEnable
//
/// \brief   Enable the chip select
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::ChipSelectEnable( void )
{
    bcm2835_gpio_clr( GetGPIOPin( SPI_PIN_TMS_CS ) );
}

// *****************************************************************************
// Function: LdSpiBCM2835::ChipSelectDisable
//
/// \brief   Disable the chip select
///
/// \author  Patrick Boulay
///
/// \since   April 2017
// *****************************************************************************

void
LdSpiBCM2835::ChipSelectDisable( void )
{
    bcm2835_gpio_set( GetGPIOPin( SPI_PIN_TMS_CS ) );
}

#endif