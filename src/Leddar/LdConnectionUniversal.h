////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdConnectionUniversal.h
///
/// \brief  Declares the LdConnectionUniversal class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdConnection.h"
#include "LdConnectionInfo.h"
#include "LdDefines.h"
#include "LdPropertiesContainer.h"

namespace LeddarConnection
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdConnectionUniversal.
    ///
    /// \brief  Base class of LdConnectionUniversal
    ///
    /// \author Patrick Boulay
    /// \date   February 2016
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdConnectionUniversal : public LdConnection
    {
    public:
        virtual          ~LdConnectionUniversal();
        virtual void     Connect( void ) override = 0;
        virtual void     Disconnect( void ) override = 0;
        virtual void     Init( void ) override;
        virtual void     RawConnect( void ) = 0;
        virtual void     Read( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aIsReadyTimeout = 0 ) = 0;
        virtual void     Read( uint8_t aOpCode, uint32_t aAddress, uint8_t *aData, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aIsReadyTimeout = 0 );
        virtual void     Write( uint8_t aOpCode, uint32_t aAddress, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aPostIsReadyTimeout = 10000,
                                const int16_t   &aPreIsReadyTimeout = 0,
                                const uint16_t &aWaitAfterOpCode = 0 ) = 0;
        virtual void     Write( uint8_t aOpCode, uint32_t aAddress, uint8_t *aData, const uint32_t &aDataSize, int16_t aCRCTry = 0, const int16_t &aPostIsReadyTimeout = 10000,
                                const int16_t   &aPreIsReadyTimeout = 0, const uint16_t &aWaitAfterOpCode = 0 );
        virtual void     ReadRegister( const uint32_t &aAddress, uint8_t *aBuffer, const uint16_t &aSize );
        virtual void     ReadRegister( const uint32_t &aAddress, uint8_t *aBuffer, const uint16_t &aSize, int16_t aCRCTry );
        virtual void     WriteRegister( const uint32_t &aAddress, const uint8_t *aBuffer, const uint16_t &aSize );
        virtual void     WriteRegister( const uint32_t &aAddress, const uint8_t *aBuffer, const uint16_t &aSize, int16_t aCRCTry );
        virtual void     Reset( LeddarDefines::eResetType aType, bool aEnterBootloader ) = 0;
        virtual void     SetAlwaysReadyCheck( bool aValue );
        virtual void     SetWriteEnable( bool aStatus, int16_t aCrcTry = 0 );
        virtual uint8_t  GetStatusRegister( int16_t aCRCTry = 0 );
        virtual bool     IsDeviceReady( int32_t aTimeout, int16_t aCRCTry = 0 );
        virtual bool     IsWriteEnable( int16_t aCrcTry = 0 );
        virtual uint16_t InternalBuffers( uint8_t *( &aInputBuffer ), uint8_t *( &aOutputBuffer ) ) = 0;

    protected:
        LdConnectionUniversal( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        static std::string  GetErrorInfo( uint32_t aErrorCode );
        void                SetDeviceReadyTimeout( uint16_t aDeviceReadyTimeout ) { mDeviceReadyTimeout = aDeviceReadyTimeout; }
        bool                mIsBigEndian;
        bool                mAlwaysReadyCheck;

    private:
        uint16_t mDeviceReadyTimeout;
    };
}

