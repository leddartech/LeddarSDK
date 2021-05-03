// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensorVu.h
///
/// \brief   Base class of sensor Leddar Vu.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_VU)

#include "LdSensor.h"
#include "LdCarrierEnhancedModbus.h"
#include "LdConnectionUniversal.h"
#include "LtFileUtils.h"

namespace LeddarDevice
{

    class LdSensorVu : virtual public LdSensor
    {

    public:
        enum eTransfertMode
        {
            TM_FREE_RUN,
            TM_BLOCKING,
            TM_PARTIAL_BLOCKING
        };

        ~LdSensorVu( void );
        virtual bool                                GetEchoes( void ) override;
        virtual void                                GetConfig( void ) override;
        virtual void                                SetConfig( void ) override;
        virtual void                                GetConstants( void ) override;
        virtual void                                UpdateConstants( void ) override;
        virtual void                                GetCalib( void ) override;
        virtual void                                GetStates( void ) override;
        void                                        ResetToDefaultWithoutWriteEnable( int16_t aCRCTry = 0 );
        void                                        ResetToDefault( void );
        virtual void                                Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions = LeddarDefines::RO_NO_OPTION, uint32_t = 0 ) override;

        virtual void                                RemoveLicense( const std::string &aLicense ) override;
        virtual void                                RemoveAllLicenses( void ) override;
        virtual LeddarDefines::sLicense             SendLicense( const std::string &aLicense, bool aVolatile = false ) override;
        virtual std::vector<LeddarDefines::sLicense> GetLicenses( void ) override;

        void                                        CreateBackup( void );
        void                                        DeleteBackup( void );
        void UpdateFirmware( const std::string &aFileName, LeddarCore::LdIntegerProperty *aProcessPercentage, LeddarCore::LdBoolProperty *aCancel ) override;
        void UpdateFirmware( eFirmwareType, const LdFirmwareData &, LeddarCore::LdIntegerProperty *, LeddarCore::LdBoolProperty * ) override;

        void                                        SetTransferMode( eTransfertMode aMode );
        static uint32_t                             GetBankAddress( uint8_t aBankType );

#ifdef BUILD_MODBUS
        LdCarrierEnhancedModbus                    *GetCarrier( void ) { return mCarrier; }
        void                                        SetCarrier( LdCarrierEnhancedModbus *aCarrier );
#endif


    protected:
        explicit LdSensorVu( LeddarConnection::LdConnection *aConnection );

        LeddarConnection::LdConnectionUniversal   *mConnectionUniversal;
        uint16_t                                   mChannelCount;
        int32_t                                   *mCalibrationOffsetBackup;
        int32_t                                   *mCalibrationLedBackup;
        bool                                       mRepair;

    private:
        void InitProperties( void );
        void UpdateDSP( const uint8_t *aData, const uint32_t &aDataSize, LeddarCore::LdBoolProperty *aCancel, LeddarCore::LdIntegerProperty *aProcessPercentage,
                        LeddarCore::LdIntegerProperty *aState );
        void GetUniqueId( uint32_t *aUniqueId );
        void OpenFirmwareUpdateSession( void );
        void StartFirmwareUpdateProcess( uint32_t addr, uint32_t dataSize, uint16_t crc );
        void GetFirmwareUpdateStatus( uint8_t *aStatus );
        void CloseFirmwareUpdateSession( uint8_t *aStatus );
        uint16_t GetAppCrc16( uint32_t aSize );
        void UpdateFPGA( const uint8_t *aAlgo, const uint32_t &aAlgoSize, const uint8_t *aData, const uint32_t &aDataSize, LeddarCore::LdBoolProperty *aCancel, bool aVerify,
                         LeddarCore::LdIntegerProperty *aProcessPercentage, LeddarCore::LdIntegerProperty *aState );
        void OpenFPGAUpdateSession( uint32_t aAlgoSize, uint32_t aDataSize, uint16_t aFpgaCrc );
        void CloseFPGAUpdateSession( void );
        void StartFpgaUpdateProcess( void );
        void GetFpgaUpdateStatus( void );
        void UnlockBootloader( uint32_t aMagicNumber );
        void UpdateAsic( const IntelHEX::IntelHexMem &aIntelHex, bool aVerify, LeddarCore::LdIntegerProperty *aProcessPercentage );

#ifdef BUILD_MODBUS
        LdCarrierEnhancedModbus                   *mCarrier;
#endif
        bool                                       mErrorFlag;
        bool                                       mBackupFlagAvailable;
    };
}

#endif