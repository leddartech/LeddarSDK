// *****************************************************************************
// Module..: LeddarPrivate
//
/// \file    LdProtocolLeddartechEthernetUDP.h
///
/// \brief   Class definition of LdProtocolLeddartechEthernetUDP
///
/// \author  Patrick Boulay
///
/// \since   February 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_ETHERNET)

#include "LdInterfaceEthernet.h"
#include "LdProtocolLeddarTech.h"

namespace LeddarConnection
{
    class LdProtocolLeddartechEthernetUDP : public LdProtocolLeddarTech
    {
    public:
        LdProtocolLeddartechEthernetUDP( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        virtual ~LdProtocolLeddartechEthernetUDP( void );

        virtual void Connect( void ) override;
        virtual void Disconnect( void ) override;
        virtual void ReadAnswer( void ) override;

    protected:
        virtual uint32_t Read( uint32_t ) override;


    private:
        LdInterfaceEthernet *mInterfaceEthernet;
        const LdConnectionInfoEthernet *mConnectionInfoEthernet;

    };
}

#endif