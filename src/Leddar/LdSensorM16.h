////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorM16.h
///
/// \brief  Declares the LdSensorM16 class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#include "LdSensor.h"
#include "LdProtocolLeddartechUSB.h"

namespace LeddarDevice
{
    class LdSensorM16 : public LdSensor
    {
    public:
        explicit LdSensorM16( LeddarConnection::LdConnection *aConnection );
        ~LdSensorM16();

        virtual void Connect( void ) override;

        virtual void GetConstants( void ) override;
        virtual void UpdateConstants( void ) override;
        virtual void GetConfig( void ) override;
        virtual void SetConfig( void ) override;
        virtual void WriteConfig( void ) override;
        virtual void RestoreConfig( void ) override;
        virtual void GetCalib() override;

        virtual void SetDataMask( uint32_t aDataMask ) override;
        virtual bool GetData( void ) override;
        virtual bool GetEchoes( void ) override { return false; }; //Dont use this function, use GetData
        virtual void GetStates( void ) override {}; //Dont use this function, use GetData

        virtual void    Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions = LeddarDefines::RO_NO_OPTION, uint32_t = 0 ) override;
        void            RequestProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds );
        void            SetProperties( LeddarCore::LdPropertiesContainer *aProperties, std::vector<uint16_t> aDeviceIds, unsigned int aRetryNbr = 0 );

        virtual void                        RemoveLicense( const std::string &aLicense ) override;
        virtual void                        RemoveAllLicenses( void ) override;
        virtual LeddarDefines::sLicense     SendLicense( const std::string &aLicense, bool aVolatile = false ) override;
        LeddarDefines::sLicense             SendLicense( const uint8_t *aLicense, bool aVolatile = false );
        virtual std::vector<LeddarDefines::sLicense> GetLicenses( void ) override;

    protected:
        LeddarConnection::LdProtocolLeddartechUSB    *mProtocolConfig;
        LeddarConnection::LdProtocolLeddartechUSB    *mProtocolData;
        LeddarCore::LdPropertiesContainer            *mResultStatePropeties;

        virtual bool    ProcessStates( void );
        void            ProcessEchoes( void );

    private:
        LdSensorM16( const LdSensorM16 &aSensor ); //Disable copy constructor
        LdSensorM16 &operator=( const LdSensorM16 &aSensor );//Disable equal constructor

        void InitProperties( void );
        void GetListing( void );
        void GetIntensityMappings( void );
    };
}

#endif