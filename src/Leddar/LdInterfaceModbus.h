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
#ifdef BUILD_MODBUS

#include "LdConnection.h"
#include "LdConnectionInfoModbus.h"

namespace LeddarConnection
{
    class LdInterfaceModbus : public LdConnection
    {
    public:
        virtual void        Connect( void ) override = 0;
        virtual void        Disconnect( void ) override = 0;
        virtual void        SendRawRequest( uint8_t *aBuffer, uint32_t aSize ) = 0;
        virtual size_t      ReceiveRawConfirmation( uint8_t *aBuffer, uint32_t aSize ) = 0;
        virtual void        ReadRegisters( uint16_t aAddr, uint8_t aNb, uint16_t *aDest ) = 0;
        virtual void        WriteRegister( uint16_t aAddr, int aValue ) = 0;
        virtual uint16_t    FetchDeviceType( void ) = 0;

        virtual bool   IsVirtualCOMPort( void ) = 0;

    protected:
        LdInterfaceModbus( const LdConnectionInfoModbus *aConnectionInfo, LdConnection *aInterface = 0 ) : LdConnection( aConnectionInfo, aInterface ),
            mConnectionInfoModbus( aConnectionInfo ) {};

        const LdConnectionInfoModbus *mConnectionInfoModbus;
    };
}

#endif