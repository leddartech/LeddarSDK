// *****************************************************************************
// Module..: LeddarConnection
//
/// \file    LdInterfaceSpi.h
///
/// \brief   Interface for SPI connection.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_SPI

#include "LdConnection.h"
#include "LdConnectionInfoSpi.h"

namespace LeddarConnection
{
    class LdInterfaceSpi : public LdConnection
    {
    public:
        enum eSpiPin
        {
            SPI_PIN_TCK_SCK     = 0,
            SPI_PIN_TDI_MOSI    = 1,
            SPI_PIN_TDO_MISO    = 2,
            SPI_PIN_TMS_CS      = 3,
            SPI_PIN_RESET       = 4,
            SPI_PIN_GPIO_0      = 5,
            SPI_PIN_GPIO_1      = 6,
            SPI_PIN_GPIO_2      = 7
        };

        enum eCSMode
        {
            CS_ACTIVEL = 0, // Chip select line active low
            CS_ACTIVEH      // Chip select line active high
        };

        enum eClockPolarity
        {
            CPOL_HIGH = 0,  // Clock polarity, clock considered active when high
            CPOL_LOW        // Clock polarity, clock considered active when low
        };

        enum eClockPhase
        {
            CPHA_FIRST,     // Clock phase, sampling done on first edge
            CPHA_SECOND     // Clock phase, sampling done on second edge
        };

        virtual ~LdInterfaceSpi() {}
        virtual void Connect( void ) override = 0;
        virtual void Disconnect( void ) override = 0;
        virtual bool IsConnected( void ) const override = 0;
        virtual void SetSpiConfig( eCSMode aCSMode,
                                   uint32_t aChipSelect,
                                   uint32_t aClockRate,
                                   eClockPolarity aClockPolarity,
                                   eClockPhase aClockPhase,
                                   uint32_t aBitsPerSample ) = 0;
        virtual void Transfert( uint8_t *aInputData, uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert = false ) = 0;
        virtual void EndTransfert( void ) = 0;

        virtual void Read( uint8_t *, uint32_t, bool aEndTransfert = false ) = 0;
        virtual void Write( uint8_t *, uint32_t, bool aEndTransfert = false ) = 0;

        virtual void InitGPIO( const uint32_t &aDirection ) = 0;
        virtual uint32_t ReadGPIO( const uint32_t &aPinsMask ) = 0;
        virtual void WriteGPIO( const uint32_t &aPinsMask, const uint32_t &aPinsValues ) = 0;
        virtual uint8_t GetGPIOPin( eSpiPin  aPin ) = 0;

    protected:
        LdInterfaceSpi( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface = nullptr ): LdConnection( aConnectionInfo, aInterface ) {}
    };
}

#endif