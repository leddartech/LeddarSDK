// *****************************************************************************
// Module..: LeddarConnection
//
/// \file    LdSpiFTDI.h
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

#pragma once

#include "LtDefines.h"
#if defined(BUILD_SPI_FTDI) && defined(BUILD_SPI)

#include "LdInterfaceSpi.h"
#include "LdConnectionInfo.h"

#include <vector>


namespace LeddarConnection
{
    class MSSELib
    {
    public:
        MSSELib();
        ~MSSELib();
        void *mLibMPSSE;
        void *mLibFTDI;
    };

    class LdSpiFTDI : public LdInterfaceSpi
    {
    public:
        explicit LdSpiFTDI( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface = nullptr );
        virtual ~LdSpiFTDI();

        virtual void Connect( void ) override;
        virtual void Disconnect( void ) override;
        virtual bool IsConnected( void ) const override { return mHandle != nullptr; }
        virtual void SetSpiConfig( eCSMode aCSMode,
                                   uint32_t aChipSelect,
                                   uint32_t aClockRate,
                                   eClockPolarity aClockPolarity,
                                   eClockPhase aClockPhase,
                                   uint32_t aBitsPerSample ) override;

        virtual void    Transfert( uint8_t *aInputData, uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert = false ) override;
        virtual void    EndTransfert( void ) override;

        virtual void    Read( uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert = false ) override;
        virtual void    Write( uint8_t *aOutputData, uint32_t aDataSize, bool aEndTransfert = false ) override;

        virtual void    InitGPIO( const uint32_t &aDirection ) override;
        virtual uint32_t ReadGPIO( const uint32_t &aPinsMask ) override;
        virtual void    WriteGPIO( const uint32_t &aPinsMask, const uint32_t &aPinsValues ) override;
        virtual uint8_t GetGPIOPin( eSpiPin aPin ) override;

        static std::vector<LdConnectionInfo *> GetDeviceList( void );

    private:
        void *mHandle;
        uint32_t mGPIOACBusDirection;
        uint32_t mGPIOADBusDirection;
        uint32_t mGPIOACBusDirectionMask;
        uint32_t mGPIOADBusDirectionMask;

        static MSSELib *mMSSELibLoader;
        static void InitLib( void );


    };

}

#endif