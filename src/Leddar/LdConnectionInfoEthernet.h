// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionInfoEthernet.h
///
/// \brief   Connection information on ethernet devices.
///
/// \author  Patrick Boulay
///
/// \since   July 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_ETHERNET

#include "LdConnectionInfo.h"
#include <stdint.h>

namespace LeddarConnection
{
    class LdConnectionInfoEthernet : public LdConnectionInfo
    {
    public:
        enum eProtocolType
        {
            PT_TCP,
            PT_UDP
        };

        enum eStatus
        {
            S_UNDEF         = 0,
            S_NOT_CONNECTED = 1,
            S_CONNECTED     = 2,
            S_ERROR         = 3
        };

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \fn LeddarConnection::LdConnectionInfoEthernet::LdConnectionInfoEthernet( const std::string &aIP, const uint32_t aPort, const std::string &aDescription, const eConnectionType aType,
        ///     const eProtocolType aProtocolType = PT_TCP, eStatus aStatus = S_UNDEF, uint32_t aTimeout = 1000, const std::string &aDisplayName = "" )
        ///
        /// \brief  Constructor
        ///
        /// \param  aIP             IP Address to connect.
        /// \param  aPort           Port of the Ethernet connection.
        /// \param  aDescription    Description of the connection.
        /// \param  aType           The type of connection (Leddartech protocol, or universal).
        /// \param  aProtocolType   (Optional) Type of the protocol (TCP / UDP)
        /// \param  aStatus         (Optional) The status.
        /// \param  aTimeout        (Optional) The timeout.
        /// \param  aDisplayName    (Optional) Name of the display.
        ///
        /// \author David Levy
        /// \date   November 2018
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        LdConnectionInfoEthernet( const std::string &aIP, const uint32_t aPort, const std::string &aDescription, const eConnectionType aType, const eProtocolType aProtocolType = PT_TCP,
                                  eStatus aStatus = S_UNDEF, uint32_t aTimeout = 1000, const std::string &aDisplayName = "" ) :
            LdConnectionInfo( aType, aDisplayName ),
            mIP( aIP ),
            mPort( aPort ),
            mDescription( aDescription ),
            mTimeout( aTimeout ),
            mUsed( aStatus ),
            mProtocolType( aProtocolType ),
            mDeviceType( 0 ) 
            {
                SetAddress(aIP);
            }

        ~LdConnectionInfoEthernet() {}

        std::string     GetIP( void ) const { return mIP; }
        uint32_t        GetPort( void ) const { return mPort;}
        std::string     GetDescription( void ) const { return mDescription; }
        uint32_t        GetTimeout( void ) const { return mTimeout; }
        void            SetTimeout( uint32_t aTimeout ) { mTimeout = aTimeout; } //To be used before connect
        eStatus         GetUsed( void ) const { return mUsed; }
        eProtocolType   GetProtocoleType( void ) const { return mProtocolType; }
        uint32_t        GetDeviceType( void ) const { return mDeviceType; }
        void            SetDeviceType( uint32_t aDeviceType ) { mDeviceType = aDeviceType; }

    protected:
        std::string     mIP;
        uint32_t        mPort;
        std::string     mDescription;
        uint32_t        mTimeout;
        eStatus         mUsed;
        eProtocolType   mProtocolType;
        uint32_t        mDeviceType;
    };
}

#endif