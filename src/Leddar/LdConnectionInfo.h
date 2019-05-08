// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionInfo.h
///
/// \brief   Interface of the connection info retreived by the device list.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#include <string>

namespace LeddarConnection
{
    class LdConnectionInfo
    {
    public:
        enum eConnectionType
        {
#ifdef BUILD_SPI_FTDI
            CT_SPI_FTDI = 0,
#endif
#ifdef BUILD_SPI_BCM2835
            CT_SPI_BCM2835 = 1,
#endif
#ifdef BUILD_MODBUS
            CT_LIB_MODBUS = 2,
#endif
#ifdef BUILD_ETHERNET
            CT_ETHERNET_UNIVERSAL = 3,
            CT_ETHERNET_LEDDARTECH = 4,
#endif
#ifdef BUILD_USB
            CT_USB = 5,
#endif
#ifdef BUILD_CANBUS_KOMODO
            CT_CAN_KOMODO = 6
#endif
        };

        virtual ~LdConnectionInfo();

        virtual const std::string &GetDisplayName( void ) const {
            return mDisplayName;
        }

        virtual const std::string &GetAddress( void ) const {
            return mAddress;
        }

        virtual void SetAddress( const std::string &aAddress ) {
            mAddress = aAddress;
        }

        const eConnectionType &GetType( void ) const {
            return mType;
        }

    protected:
        LdConnectionInfo( eConnectionType aConnectionType, const std::string &aDisplayName );

        std::string mDisplayName;
        std::string mAddress;
        eConnectionType mType;

    };
}
