// *****************************************************************************
// Module..: LeddarPrototype
//
/// \file    LdSensorLeddarAuto.h
///
/// \brief   Class of the Leddar Auto sensor.
///
/// \author  Patrick Boulay
///
/// \since   December 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_ETHERNET) && defined(BUILD_AUTO)

#include "LdSensor.h"
#include "LdProtocolLeddartechEthernet.h"
#include "LdProtocolLeddartechEthernetUDP.h"

namespace LeddarDevice
{
    class LdSensorLeddarAuto : public LdSensor
    {
    public:
        explicit LdSensorLeddarAuto( LeddarConnection::LdConnection *aConnection );
        virtual ~LdSensorLeddarAuto( void );
        virtual void        Connect( void ) override;
        virtual void        Disconnect( void ) override;
        virtual void        ConnectDataServer( void );
        virtual bool        GetData( void ) override;
        virtual void        GetConfig( void ) override;
        virtual void        SetConfig( void ) override;
        virtual void        RestoreConfig( void ) override;
        virtual void        WriteConfig( void ) override;
        virtual void        GetConstants( void ) override;
        virtual void        UpdateConstants( void ) override;
        virtual void        GetCalib( void ) override;
        virtual bool        GetEchoes( void ) override { return false; };
        virtual void        GetStates( void ) override {};
        virtual void        Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions = LeddarDefines::RO_NO_OPTION, uint32_t = 0 ) override;
        void                SendCommand( uint16_t aRequestCode, unsigned int aRetryNbr = 0 );
        void                SendPing( void );
        void                RequestProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds );
        void                SetProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds, unsigned int aRetryNbr = 0 );

        LeddarDefines::sLicense                         SendLicense( const uint8_t *aLicense, bool aVolatile );
        virtual void                                    RemoveLicense( const std::string &aLicense ) override;
        virtual void                                    RemoveAllLicenses( void ) override;
        virtual LeddarDefines::sLicense                 SendLicense( const std::string &aLicense, bool aVolatile = false ) override;
        virtual std::vector<LeddarDefines::sLicense>    GetLicenses( void ) override;

        virtual void   SetDataMask( uint32_t aDataMask ) override;

    protected:
        virtual bool    ProcessData( uint16_t aRequestCode );
        virtual bool    RequestData( uint32_t &aMask );
        bool            ProcessEchoes( void );
        bool            ProcessStates( void );

        void            GetCategoryPropertiesFromDevice( LeddarCore::LdProperty::eCategories aCategory, uint16_t aRequestCode );
        void            SetCategoryPropertiesOnDevice( LeddarCore::LdProperty::eCategories aCategory, uint16_t aRequestCode );

        void            SetDataReceived( bool aAllDataReceived ) { mAllDataReceived = aAllDataReceived; }
        void UpdateFirmware( eFirmwareType aFirmwareType, const LdFirmwareData &aFirmwareData, LeddarCore::LdIntegerProperty *aProcessPercentage,
                             LeddarCore::LdBoolProperty *aCancel ) override;
        eFirmwareType LtbTypeToFirmwareType( uint32_t aLtbType ) override;

        LeddarConnection::LdProtocolLeddartechEthernet *mProtocolConfig;
        LeddarConnection::LdProtocolLeddarTech         *mProtocolData;

        bool mPingEnabled;
        bool mIsTCPDataServer;

    private:
        void   InitProperties( void );

        bool   mAllDataReceived;

    };
}

#endif