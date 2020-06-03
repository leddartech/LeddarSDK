////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorDTec.h
///
/// \brief  Declares the LdSensorDTec class
///
/// Copyright (c) 2019 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined(BUILD_DTEC) && defined(BUILD_ETHERNET)

#include "LdSensor.h"

#include "LdProtocolLeddartechEthernet.h"
#include "LdProtocolLeddartechEthernetUDP.h"

namespace LeddarDevice
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdSensorDTec.
    ///
    /// \brief  Class used to connect, configure and communicate with DTec sensors
    ///
    /// \author David Levy
    /// \date   October 2019
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdSensorDTec : public LdSensor
    {
    public:
        explicit LdSensorDTec( LeddarConnection::LdConnection *aConnection, bool aConnectToAuxiliaryDataServer = false );
        virtual ~LdSensorDTec( void );

        void Connect( void ) override;
        void Disconnect( void ) override;
        void GetConstants( void ) override;
        void GetConfig( void ) override;
        void SetConfig( void ) override;
        void WriteConfig( void ) override;
        void GetCalib( void ) override;
        void UpdateConstants( void ) override;
        void QueryDeviceInfo( void );

        void SetDataMask( uint32_t aDataMask ) override;
        bool GetData( void ) override;
        bool GetEchoes( void ) override {throw std::logic_error( "Use GetData to fetch data from UDP stream." );}
        void GetStates( void ) override {throw std::logic_error( "Use GetData to fetch data from UDP stream." );}
        void GetStatus( void );

        void Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions = LeddarDefines::RO_NO_OPTION, uint32_t aSubOptions = 0 ) override;
        void UpdateFirmware( eFirmwareType aFirmwareType, const LdFirmwareData &aFirmwareData, LeddarCore::LdIntegerProperty *aProcessPercentage,
                             LeddarCore::LdBoolProperty * ) override;


        static void ReinitIpConfig( const std::string &aSerialNumber, uint8_t aMode, uint8_t aStorage, uint8_t aPhyMode,
                                    const std::string &aIp = "", const std::string &aSubnet = "", const std::string &aGateway = "" );

    protected:
        LeddarConnection::LdProtocolLeddartechEthernet *mProtocolConfig;
        LeddarConnection::LdProtocolLeddarTech          *mProtocolData;
        bool mPingEnabled;
        bool mAuxiliaryDataServer;

        virtual bool ProcessData( uint16_t aRequestCode );
        void SendCommand( uint16_t aRequestCode, uint32_t aTimeout = 0 );
        void ReadAnswer( uint32_t aTimeout );

    private:
        void InitProperties( void );
        void ConnectDataServer( void );

        bool ProcessEchoes( void );

        eFirmwareType LtbTypeToFirmwareType( uint32_t aLtbType ) override;

    };

}

#endif
