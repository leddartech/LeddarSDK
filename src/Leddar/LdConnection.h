// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnection.h
///
/// \brief   Base class of LdConnection
///
/// \author  Patrick Boulay
///
/// \since   February 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdConnectionInfo.h"
#include "LdObject.h"

#include <stdint.h>

namespace LeddarConnection
{

    class LdConnection : public LeddarCore::LdObject
    {
    public:
        virtual                     ~LdConnection();
        virtual void                 Connect( void ) = 0;
        virtual void                 Disconnect( void ) = 0;
        virtual void                 Init( void ) {}
        virtual bool                 IsConnected( void ) const { return ( mInterface != nullptr ? mInterface->IsConnected() : false ); };
        virtual uint16_t             GetDeviceType( void ) { return mDeviceType; }
        virtual void                 SetDeviceType( uint16_t aDeviceType ) { mDeviceType = aDeviceType; }
        const LdConnectionInfo      *GetConnectionInfo( void ) const { return dynamic_cast< const LdConnectionInfo * >( mConnectionInfo ); }
        LdConnection                *GetInterface( void ) const { return mInterface; }
        void                         TakeOwnerShip( bool aOwner ) { mOwner = aOwner; } //Take ownership of mConnectionInfo and mInterface

        virtual void                 ResizeInternalBuffers( const uint32_t &aSize );
        uint16_t                     GetInternalBuffersSize( void ) const { return mTransferBufferSize; }

    protected:
        explicit LdConnection( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface = nullptr );
        const LdConnectionInfo  *mConnectionInfo;
        uint16_t                 mDeviceType;
        LdConnection            *mInterface;

        uint8_t                 *mTransferInputBuffer; ///Sensor input: buffer of data we send to the sensor
        uint8_t                 *mTransferOutputBuffer; ///Sensor output: buffer of data we receive from the sensor
        uint32_t                 mTransferBufferSize;

    private:
        bool mOwner;
    };
}
