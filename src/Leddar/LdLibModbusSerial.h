// *****************************************************************************
// Module..: Leddar
//
/// \file    LdLibModbusSerial.h
///
/// \brief   Class definition of LdLibModbusSerial
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

#include "LdInterfaceModbus.h"
#include "LdConnectionInfoModbus.h"

#include <vector>

struct _modbus;
typedef struct _modbus modbus_t;

namespace LeddarConnection
{
    class LdLibModbusSerial : public LdInterfaceModbus
    {
    public:
        LdLibModbusSerial( const LdConnectionInfoModbus *aConnectionInfo, LdConnection *aExistingConnection = nullptr );
        virtual            ~LdLibModbusSerial();
        virtual void        Connect( void ) override;
        virtual bool        IsConnected( void ) const override { return mHandle != nullptr; }
        virtual void        Disconnect( void ) override;
        virtual void        SendRawRequest( uint8_t *aBuffer, uint32_t aSize ) override;
        virtual void        ReadRegisters( uint16_t aAddr, uint8_t aNb, uint16_t *aDest ) override;
        virtual void        ReadInputRegisters( uint16_t aAddr, uint8_t aNb, uint16_t *aDest );
        virtual void        WriteRegister( uint16_t aAddr, int aValue ) override;
        virtual size_t      ReceiveRawConfirmation( uint8_t *aBuffer, uint32_t aSize ) override;
        int                 ReceiveRawConfirmationLT( uint8_t *aBuffer, int aDeviceType );
        virtual void        Flush( void );
        virtual modbus_t    *GetHandle( void ) { return mHandle; }
        virtual uint16_t    FetchDeviceType( void ) override;

        virtual bool        IsVirtualCOMPort( void ) override;

        static std::vector<LdConnectionInfo *> GetDeviceList( void );

    protected:
        modbus_t                *mHandle;
        bool                    mSharedHandle;

    };
}

#endif