/// *****************************************************************************
/// Module..: Leddar
///
/// \file    LdSensorM16Modbus.h
///
/// \brief   Class of M16 Sensor communucating in modbus protocol.
///
/// \author  David Levy
///
/// \since   September 2017
///
/// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
/// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_MODBUS)

#include "LdSensor.h"
#include "LdConnectionInfoModbus.h"

#include "LdLibModbusSerial.h"

namespace LeddarDevice
{
    class LdSensorM16Modbus : public LdSensor
    {
    public:
        explicit LdSensorM16Modbus( LeddarConnection::LdConnection *aConnection );
        ~LdSensorM16Modbus( void );

        virtual void        Connect( void ) override;
        virtual void        GetConfig( void ) override;
        virtual void        SetConfig( void ) override;
        virtual void        GetConstants( void ) override;
        virtual void        UpdateConstants( void ) override;
        virtual bool        GetEchoes( void ) override {return mUse0x6A ? GetEchoes0x6A() : GetEchoes0x41(); }
        virtual void        GetStates( void ) override;
        virtual void        Reset( LeddarDefines::eResetType /*aType*/, LeddarDefines::eResetOptions = LeddarDefines::RO_NO_OPTION, uint32_t = 0 ) override {};
        bool                GetUse0x6A( void ) const { return mUse0x6A; }
        void                SetUse0x6A( bool use0x6A ) { mUse0x6A = use0x6A; }

    protected:
        const LeddarConnection::LdConnectionInfoModbus  *mConnectionInfoModbus;
        LeddarConnection::LdLibModbusSerial *mInterface;

    private:
        void    InitProperties( void );
        bool    GetEchoes0x41( void );
        bool    GetEchoes0x6A( void );

        bool    mUse0x6A; ///< Use modbus function 0x6A to get echoes. Allows entire flag, but less echoes
    };
}

#endif