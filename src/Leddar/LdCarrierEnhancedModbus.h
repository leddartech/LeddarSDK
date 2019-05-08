// *****************************************************************************
// Module..: Leddar
//
/// \file    LdCarrierEnhancedModbus.h
///
/// \brief   Definition of the LeddarVu8 carrier board connecting by Modbus.
///
/// \author  Patrick Boulay
///
/// \since   September 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_MODBUS)

#include "LdSensor.h"

namespace LeddarDevice
{

    class LdCarrierEnhancedModbus
    {
    public:
        explicit LdCarrierEnhancedModbus( LeddarConnection::LdConnection *aConnection );
        virtual ~LdCarrierEnhancedModbus() {};
        void GetConstants( void );
        void GetConfigCAN( void );
        void GetConfigSerial( void );
        void SetConfigCAN( void );
        void SetConfigSerial( void );

        LeddarCore::LdPropertiesContainer *GetProperties( void ) { return &mProperties;  }

    protected:
        struct sCarrierDeviceInformation
        {
            uint8_t  mModbusAddress;
            uint8_t  mFunctionCode;
            uint8_t  mSubFunctionCode;
            char     mHardwarePartNumber[ 32 ];
            char     mHardwareSerialNumber[ 32 ];
            uint32_t mOptions;
            uint16_t mCrc;
        };
        struct sCarrierFirmwareInformation
        {
            uint8_t  mModbusAddress;
            uint8_t  mFunctionCode;
            uint8_t  mSubFunctionCode;
            char     mFirmwarePartNumber[ 32 ];
            char     mFirmwareVersion[ 4 ];
        };


        LeddarConnection::LdConnection    *mModbusConnection;
        LeddarCore::LdPropertiesContainer  mProperties;

    private:
        void InitProperties( void );

        LeddarCore::LdPropertiesContainer  mPropertiesCAN;
        LeddarCore::LdPropertiesContainer  mPropertiesSerial;

    };
}

#endif