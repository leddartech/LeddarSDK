// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensor.h
///
/// \brief   Base class of all sensors.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdConnection.h"
#include "LdDefines.h"
#include "LdDevice.h"
#include "LdResultEchoes.h"
#include "LdResultStates.h"

namespace LeddarRecord
{
    class LdLjrRecordReader;
    class LdPrvLtlRecordReader;
} // namespace LeddarRecord

namespace LeddarDevice
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \struct LdFirmwareData.
    ///
    /// \brief  Small structure that hold the data for a firmware update
    ///         Most of the time its just a simple std::vector<uint8_t> holder
    ///         But in few cases, update require several vectors
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    struct LdFirmwareData
    {
        // Firmware data is usually just a std::vector<uint8_t>. Second constructor take care of specific case
        explicit LdFirmwareData( const std::vector<uint8_t> &aFirmwareData ) : mFirmwareData( aFirmwareData ) {};

        explicit LdFirmwareData( const std::vector<uint8_t> &aFPGAData, const std::vector<uint8_t> &aAlgoData )
            : mFirmwareData( aFPGAData )
            , mAlgoData( aAlgoData ){};

        std::vector<uint8_t> mFirmwareData;
        std::vector<uint8_t> mAlgoData;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdSensor.
    ///
    /// \brief  Interface class to all sensors.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdSensor : public LdDevice
    {
        friend class LeddarRecord::LdLjrRecordReader;
        friend class LeddarRecord::LdPrvLtlRecordReader;

      public:
        /// \brief  Available data mask
        enum eDataMask
        {
            DM_NONE   = 0,
            DM_STATES = 1,
            DM_ECHOES = 2,
            DM_ALL    = DM_STATES | DM_ECHOES
        };

        /// \brief  Type of firmware data to send
        enum eFirmwareType
        {
            FT_DSP,
            FT_FPGA,
            FT_ASIC,
            FT_FACTORY,
            FT_OS,
            FT_INVALID = 0xFF
        };

        /// \brief  Possible communication protocol
        enum eProtocol
        {
            P_NONE             = 0,
            P_MODBUS           = 1, ///< Raw modbus
            P_MODBUS_UNIVERSAL = 2, ///< SPI protocol through modbus
            P_SPI              = 3, ///< SPI protocol
            P_USB              = 4, ///< USB
            P_CAN              = 5, ///< CANBus
            P_ETHERNET         = 6  ///< Ethernet
        };

        ~LdSensor() override;
        virtual void                        StartAcquisition(void);
        virtual void                        StopAcquisition(void);
        virtual void GetConfig( void ) {}
        virtual void SetConfig( void ) = 0;
        virtual void WriteConfig( void ) {}
        virtual void RestoreConfig( void ) {}
        virtual void GetConstants( void ) {}
        virtual void GetCalib( void ){}
        virtual void UpdateConstants( void ){}
        virtual bool GetData( void );
        virtual bool GetEchoes( void )                                                                                                                       = 0;
        virtual void GetStates( void )                                                                                                                       = 0;
        virtual void Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions = LeddarDefines::RO_NO_OPTION, uint32_t aSubOptions = 0 ) = 0;
        LeddarConnection::LdResultEchoes *GetResultEchoes( void ) { return &mEchoes; }
        LeddarConnection::LdResultStates *GetResultStates( void ) { return &mStates; }

        virtual void SetDataMask( uint32_t aDataMask ) { mDataMask = aDataMask; }

        virtual void RemoveLicense( const std::string & /*aLicense*/ ) {}
        virtual void RemoveAllLicenses( void ) {}
        virtual LeddarDefines::sLicense SendLicense( const std::string &, bool = false ) { return LeddarDefines::sLicense(); }
        virtual std::vector<LeddarDefines::sLicense> GetLicenses( void ) { return std::vector<LeddarDefines::sLicense>( 0 ); }
        void RemoveVolatileLicense( void );
        LeddarDefines::sLicense GetVolatileLicense();
        void SendVolatileLicense( const std::string &aLicence ) { SendLicense( aLicence, true ); }

        virtual void UpdateFirmware( const std::string &aFileName, LeddarCore::LdIntegerProperty *aProcessPercentage, LeddarCore::LdBoolProperty *aCancel );
        virtual eFirmwareType LtbTypeToFirmwareType( uint32_t ) { return FT_INVALID; }
        virtual void UpdateFirmware( eFirmwareType, const LdFirmwareData &, LeddarCore::LdIntegerProperty *, LeddarCore::LdBoolProperty * )
        {
            throw std::logic_error( "Firmware update not implemented for this sensor" );
        }

      protected:
        explicit LdSensor( LeddarConnection::LdConnection *aConnection, LeddarCore::LdPropertiesContainer *aProperties = nullptr );
        virtual void ComputeCartesianCoordinates();
        LeddarConnection::LdResultEchoes mEchoes;
        LeddarConnection::LdResultStates mStates;

        static uint32_t GetDataMaskAll( void ) { return DM_ALL; }
        virtual uint32_t ConvertDataMaskToLTDataMask( uint32_t aMask );
        uint32_t mDataMask;

      private:
        void InitProperties( void );
    };
} // namespace LeddarDevice
