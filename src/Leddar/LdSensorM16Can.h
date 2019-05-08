////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorM16Can.h
///
/// \brief  Declares the LdSensorM16Can class
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_CANBUS)

#include "LdSensor.h"

namespace LeddarConnection
{
    class LdProtocolCan;
}

namespace LeddarDevice
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdSensorM16Can
    ///
    /// \brief  Class for a M16 sensor using CAN protocol
    ///
    /// \author David Levy
    /// \date   October 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdSensorM16Can : public LdSensor
    {
    public:
        explicit LdSensorM16Can( LeddarConnection::LdConnection *aConnection );
        virtual ~LdSensorM16Can() {};

        virtual void    GetConfig( void ) override;
        virtual void    SetConfig( void ) override;
        virtual void    GetConstants( void ) override;
        virtual bool    GetData( void ) override;
        virtual bool    GetEchoes( void ) override;
        virtual void    GetStates( void ) override;
        virtual void    Reset( LeddarDefines::eResetType, LeddarDefines::eResetOptions = LeddarDefines::RO_NO_OPTION ) override {throw std::logic_error( "Reset not available in CANbus" );};

        void            EnableStreamingDetections( bool aEnable );


    private:
        LeddarConnection::LdProtocolCan *mProtocol;
        uint32_t mLastTimestamp;

        void InitProperties();
    };
}

#endif