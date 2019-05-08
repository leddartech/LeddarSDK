// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionUniversalSpi.h
///
/// \brief   Base class of LdConnectionUniversalSpi
///
/// \author  Patrick Boulay
///
/// \since   February 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_SPI

#include "LdConnectionUniversal.h"
#include "LdConnectionInfo.h"
#include "LdInterfaceSpi.h"

namespace LeddarConnection
{
    class LdConnectionUniversalSpi : public LdConnectionUniversal
    {
    public:
        LdConnectionUniversalSpi( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        ~LdConnectionUniversalSpi();
        virtual void     Connect() override;
        virtual void     RawConnect( void ) override;
        virtual void     Disconnect( void ) override;
        virtual void     Read( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aIsReadyTimeout = 0 )override;
        virtual void     Write( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aPostIsReadyTimeout = 10000,
                                const int16_t   &aPreIsReadyTimeout = 0, const uint16_t &aWaitAfterOpCode = 0 ) override;

        virtual void     Reset( LeddarDefines::eResetType aType, bool aEnterBootloader ) override;
        virtual uint16_t InternalBuffers( uint8_t *( &aInputBuffer ), uint8_t *( &aOutputBuffer ) ) override;

    protected:
        virtual void     CrcCheck( uint8_t *aHeader, uint8_t *aData, const uint32_t &aDataSize, uint16_t aCrc16 );
        virtual void     HardReset( bool aEnterBootloader );
        virtual void     InitIO( void );
        LdInterfaceSpi   *mSpiInterface;

    private:
        std::vector<uint8_t>  mWriteBuffer;
    };
}

#endif