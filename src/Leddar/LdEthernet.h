// *****************************************************************************
// Module..: Leddar
//
/// \file    LdEthernet.h
///
/// \brief   Base class of LdEthernet
///
/// \author  Patrick Boulay
///
/// \since   October 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_ETHERNET

#include "LdConnectionInfoEthernet.h"
#include "LdInterfaceEthernet.h"

#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#else
#define SOCKET int
#endif

namespace LeddarConnection
{
    class LdEthernet : public LdInterfaceEthernet
    {
    public:
        explicit LdEthernet( const LdConnectionInfoEthernet *aConnectionInfo, LdConnection *aInterface = nullptr );
        virtual ~LdEthernet( void );

        // TCP/IP
        virtual void   Connect( void ) override;
        virtual void   Disconnect( void ) override;
        virtual void   Send( uint8_t *aBuffer, uint32_t aSize ) override;
        virtual size_t Receive( uint8_t *aBuffer, uint32_t aSize ) override;
        virtual void   FlushBuffer( void ) override;
        virtual bool   IsConnected( void ) const override { return mIsConnected; }

        // UDP
        virtual void     SendTo( const std::string &aIpAddress, uint16_t aPort, const uint8_t *aData, uint32_t aSize ) override;
        virtual uint32_t ReceiveFrom( std::string &aIpAddress, uint16_t &aPort, uint8_t *aData, uint32_t aSize ) override;
        virtual void     OpenUDPSocket( uint32_t aPort, uint32_t aTimeout = 2000 ) override;
        virtual void     CloseUDPSocket( void ) override;
        static uint64_t CloseSocket( const SOCKET aSocket );

        static std::vector<std::pair<SOCKET, unsigned long> > OpenScanRequestSockets();
        static void GetDevicesListSendRequest( const std::vector<std::pair<SOCKET, unsigned long> > &aInterfaces, bool aWideBroadcast = false );
        static std::vector<LeddarConnection::LdConnectionInfo *> GetDevicesListReadAnswer( const std::vector<std::pair<SOCKET, unsigned long> > &aInterfaces );
        static std::vector<LeddarConnection::LdConnectionInfo *> GetDeviceList( uint32_t aTimeoutms = 500, bool aWideBroadcast = false );

    protected:
        SOCKET  mSocket;
        SOCKET  mUDPSocket;
        bool    mIsConnected;
    };
}

#endif