// *****************************************************************************
// Module..: Leddar
//
/// \file    LdInterfaceModbus.h
///
/// \brief   Base class of LdInterfaceModbus
///
/// \author  Patrick Boulay
///
/// \since   September 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_ETHERNET

#include "LdConnection.h"
#include "LdConnectionInfoEthernet.h"

namespace LeddarConnection
{
    class LdInterfaceEthernet : public LdConnection
    {
    public:
        virtual void   Connect( void ) override = 0;
        virtual void   Disconnect( void ) override = 0;
        virtual void   Send( uint8_t *lBuffer, uint32_t lSize ) = 0;
        virtual size_t Receive( uint8_t *lBuffer, uint32_t lSize ) = 0;
        virtual void   FlushBuffer( void ) = 0;

        // UDP
        virtual void     SendTo( const std::string &aIpAddress, uint16_t aPort, const uint8_t *aData, uint32_t aSize ) = 0;
        virtual uint32_t ReceiveFrom( std::string &aIpAddress, uint16_t &aPort, uint8_t *aData, uint32_t aSize ) = 0;
        virtual void     OpenUDPSocket( uint32_t aPort, uint32_t aTimeout = 2000 ) = 0;
        virtual void     CloseUDPSocket( void ) = 0;

    protected:
        explicit LdInterfaceEthernet( const LdConnectionInfoEthernet *aConnectionInfo, LdConnection *aInterface = 0 ) :
            LdConnection( aConnectionInfo, aInterface ),
            mConnectionInfoEthernet( aConnectionInfo )
        {};

        const LdConnectionInfoEthernet *mConnectionInfoEthernet;
    };
}

#endif