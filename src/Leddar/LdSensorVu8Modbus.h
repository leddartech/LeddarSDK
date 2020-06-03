// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensorVu8Modbus.h
///
/// \brief   Definition of LeddarVu8 sensor class using modbus protocole.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_VU) && defined(BUILD_MODBUS)

#include "LdSensor.h"
#include "LdConnectionInfoModbus.h"
#include "LdLibModbusSerial.h"

namespace LeddarDevice
{
    class LdSensorVu8Modbus : public LdSensor
    {
    public:
        explicit LdSensorVu8Modbus( LeddarConnection::LdConnection *aConnection );
        ~LdSensorVu8Modbus( void );

        virtual void    Connect( void ) override;
        virtual void    GetConfig( void ) override;
        virtual void    SetConfig( void ) override;
        virtual void    GetConstants( void ) override;
        virtual void    UpdateConstants( void ) override;
        virtual bool    GetEchoes( void ) override;
        virtual void    GetStates( void ) override;
        virtual void    Reset( LeddarDefines::eResetType /*aType*/, LeddarDefines::eResetOptions = LeddarDefines::RO_NO_OPTION, uint32_t = 0 ) override {};

    protected:
        void            GetCanConfig( void );
        void            GetSerialConfig( void );
        void            GetCarrierInfoConfig( void );
        void            GetCarrierFirmwareInfoConfig( void );

        void            SetSerialConfig( void );
        void            SetCanConfig( void );

        const LeddarConnection::LdConnectionInfoModbus  *mConnectionInfoModbus;
        LeddarConnection::LdLibModbusSerial *mInterface;

    private:
        void            InitProperties( void );
    };
}

#endif