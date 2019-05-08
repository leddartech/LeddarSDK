////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdProtocolCan.h
///
/// \brief  Declares the LdProtocolCan class
///             To communicate with the sensor using the CAN protocol
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#ifdef BUILD_CANBUS

#include "LdConnection.h"

#include "LdInterfaceCan.h"

namespace LeddarConnection
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdProtocolCan
    ///
    /// \brief  Class that implement the protocol to  communicate with the sensor using the CAN protocol
    ///
    /// \author David Levy
    /// \date   October 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdProtocolCan : public LdConnection
    {
    public:
        explicit LdProtocolCan( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface, bool aIsM16 );
        virtual ~LdProtocolCan();

        void        SendRequest( const std::vector<uint8_t> &aData );
        void        SendRequest( const LtComCanBus::sCanData &aData );
        bool        SendRequestAndWaitForAnswer( const std::vector<uint8_t> &aData );
        bool        SendRequestAndWaitForAnswer( const LtComCanBus::sCanData &aData );
        bool        ReadConfigAnswer( void );
        bool        ReadDetectionAnswer( void );
        LtComCanBus::sCanData GetNextConfigData();
        LtComCanBus::sCanData GetNextDetectionData();

        LtComCanBus::sCanData   GetValue( uint8_t aCommandId, uint8_t aCommandArg );
        void                    SetValue( const LtComCanBus::sCanData &aCommand );

        void        EnableStreamingDetections( bool aEnable, uint8_t aFlag = 0 );
        bool        IsStreaming( void ) const {return mIsStreaming;}

        void Connect( void ) override {mInterfaceCAN->Connect(); EnableStreamingDetections( false );}
        void Disconnect( void ) override {mInterfaceCAN->Disconnect();}

    private:
        LdInterfaceCan *mInterfaceCAN;
        std::queue<LtComCanBus::sCanData> mBufferConfig;        ///Store data related to configuration
        std::queue<LtComCanBus::sCanData> mBufferDetections;    ///Store data related to detections
        bool mIsM16;                                            ///Currently the only difference between M16 et Vu8 CAN protocol is how the buffer are handled.
        ///     Might need to create a child class to properly handle the differences if there is more
        bool mIsStreaming;

        virtual void    Callback( LdObject *aSender, const SIGNALS aSignal, void *aCanData ) override;
    };
}

#endif