// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionUniversalModbus.h
///
/// \brief   Base class of LdConnectionModbus
///
/// \author  Patrick Boulay
///
/// \since   July 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_MODBUS

#include "LdConnection.h"
#include "LdConnectionInfoModbus.h"
#include "LdConnectionUniversal.h"
#include "LdInterfaceModbus.h"

namespace LeddarConnection
{
    class LdConnectionUniversalModbus : public LdConnectionUniversal
    {
    public:
        LdConnectionUniversalModbus( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        virtual ~LdConnectionUniversalModbus();
        virtual void RawConnect( void ) override;
        virtual void Connect( void ) override;
        virtual void Disconnect( void ) override;
        virtual void Init( void ) override;
        virtual bool IsConnected( void ) const override;
        virtual void Read( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aIsReadyTimeout = 0 ) override;
        virtual void Write( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t/* aCRCTry */ = 0, const int16_t &aPostIsReadyTimeout = 10000,
                            const int16_t &   /*aPreIsReadyTimeout*/ = 0, const uint16_t &aWaitAfterOpCode = 0 ) override;
        virtual void Reset( LeddarDefines::eResetType /*aType*/, bool aEnterBootloader ) override;
        virtual uint16_t InternalBuffers( uint8_t *( &aInputBuffer ), uint8_t *( &aOutputBuffer ) ) override;

    protected:
        uint16_t                      ReadDeviceType( void );
        const LdConnectionInfoModbus *mConnectionInfoModbus;
        LdInterfaceModbus            *mInterfaceModbus;
        bool                          IsEngineStop( int32_t aTimeout );
        void                          StopEngine( void );
        void                          StartEngine( void );
    };
}

#endif