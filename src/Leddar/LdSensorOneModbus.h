/// *****************************************************************************
/// Module..: Leddar
///
/// \file    LdSensorOneModbus.h
///
/// \brief   Class of LeddarOne Sensor communicating in modbus protocol.
///
/// \author  David Levy
///
/// \since   September 2017
///
/// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
/// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_ONE) && defined(BUILD_MODBUS)

#include "LdSensor.h"
#include "LdConnectionInfoModbus.h"

#include "LdLibModbusSerial.h"

namespace LeddarDevice
{
    class LdSensorOneModbus : public LdSensor
    {
    public:
        explicit LdSensorOneModbus( LeddarConnection::LdConnection *aConnection );
        ~LdSensorOneModbus( void );

        virtual void    Connect( void ) override;
        virtual bool    GetData( void ) override;
        virtual void    GetConfig( void ) override;
        virtual void    SetConfig( void ) override;
        virtual void    WriteConfig( void ) override;
        virtual void    GetConstants( void ) override;
        virtual void    UpdateConstants( void ) override;
        virtual bool    GetEchoes( void ) override;
        virtual void    GetStates( void ) override {}
        virtual void    Reset( LeddarDefines::eResetType /*aType*/, LeddarDefines::eResetOptions = LeddarDefines::RO_NO_OPTION, uint32_t = 0 ) override {};

    protected:
        virtual bool    RequestData( uint32_t &aDataMask );

        const LeddarConnection::LdConnectionInfoModbus  *mConnectionInfoModbus;
        LeddarConnection::LdLibModbusSerial *mInterface;

    private:
        void            InitProperties( void );
        uint8_t         mParameterVersion;
    };
}

#endif