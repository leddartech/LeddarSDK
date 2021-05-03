// *****************************************************************************
// Module..: LeddarPrivate
//
/// \file    LdProtocolLeddartechEthernet.h
///
/// \brief   Class definition of LdProtocolLeddartechEthernet
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
    //TODO: Renommer en LdProtocolLeddartechEthernetTCP et faire une classe mere commune avec UDP
    class LdProtocolLeddartechEthernet : public LdProtocolLeddarTech
    {
    public:
        LdProtocolLeddartechEthernet( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        virtual ~LdProtocolLeddartechEthernet( void );

        virtual void    Connect( void ) override;
        virtual void    Disconnect( void ) override;
        virtual void    SetEchoState( bool aState );
        virtual void    ReadAnswer( void ) override;
        virtual void    ReadRequest( void ) override;
        void            QueryDeviceType( void );

    protected:
        virtual void Write( uint32_t aSize ) override;
        virtual uint32_t Read( uint32_t aSize ) override;


    private:
        LdInterfaceEthernet *mInterfaceEthernet;


    };
}
#endif