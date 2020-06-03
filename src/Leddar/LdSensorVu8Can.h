////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorVu8Can.h
///
/// \brief  Declares the LdSensorVu8Can class. Class that handle basic communication to the Vu8 using CANbus protocol
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined(BUILD_VU) && defined(BUILD_CANBUS)

#include "LdSensor.h"

namespace LeddarConnection
{
    class LdProtocolCan;
}

namespace LeddarDevice
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdSensorVu8Can
    ///
    /// \brief  Class for a Vu8 sensor using CAN protocol (without the universal protocol)
    ///
    /// \author David Levy
    /// \date   October 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdSensorVu8Can : public LdSensor
    {
    public:
        explicit LdSensorVu8Can( LeddarConnection::LdConnection *aConnection );
        virtual ~LdSensorVu8Can() {}

        virtual void    GetConstants( void ) override;
        virtual void    GetConfig( void ) override;
        virtual void    SetConfig( void ) override;
        virtual bool    GetData( void ) override;
        virtual bool    GetEchoes( void ) override;
        virtual void    GetStates( void ) override {}; //Nothing to fetch
        virtual void    Reset( LeddarDefines::eResetType, LeddarDefines::eResetOptions = LeddarDefines::RO_NO_OPTION, uint32_t = 0 ) override {throw std::logic_error( "Reset not available in CANbus" );};

        void            EnableStreamingDetections( bool aEnable );

    private:
        LeddarConnection::LdProtocolCan *mProtocol;
        uint32_t mLastTimestamp;

        void InitProperties();
    };
}

#endif