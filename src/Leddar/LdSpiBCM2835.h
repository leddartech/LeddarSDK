// *****************************************************************************
// Module..: LeddarConnection
//
/// \file    LdSpiBCM2835.h
///
/// \brief   Implementation of the BCM2835 SPI interface on a Raspberry PI.
///          You need to uncomment the define BUILD_SPI_BCM2835 in LtDefines.h to compile this class.
///
/// \author  Patrick Boulay
///
/// \since   April 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"

#if defined(BUILD_SPI_BCM2835) && defined(BUILD_SPI)

#include "LdInterfaceSpi.h"
#include "LdConnectionInfo.h"

#include <vector>

namespace LeddarConnection
{
    class LdSpiBCM2835 : public LdInterfaceSpi
    {
    public:
        LdSpiBCM2835( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface = nullptr );
        virtual ~LdSpiBCM2835();

        virtual void Connect( void ) override;
        virtual void Disconnect( void ) override;
        virtual bool IsConnected( void )  override;
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

        virtual void    ChipSelectEnable( void );
        virtual void    ChipSelectDisable( void );

        static std::vector<LdConnectionInfo *> GetDeviceList( void );

    private:

        static void InitLib( void );
        static bool mInitialized;
        bool mIsConnected;
    };

}

#endif