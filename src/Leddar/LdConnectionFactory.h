// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionFactory.h
///
/// \brief   Factory to create connection
///
/// \author  Patrick Boulay
///
/// \since   February 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdConnection.h"

namespace LeddarConnection
{
    class LdConnectionFactory
    {
    public:
        ~LdConnectionFactory(){};
        static LdConnection* CreateConnection( const LdConnectionInfo* aConnectionInfo, LeddarConnection::LdConnection * aConnection = nullptr, uint32_t aForcedDeviceType = 0 );

    private:
        LdConnectionFactory( void ){};
        
    };
}

