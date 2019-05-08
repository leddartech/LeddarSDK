// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionModbusStructures.h
///
/// \brief   Namespace containing structures of modbus protocole.
///
/// \author  Patrick Boulay
/// \author  Frédéric Parent
///
/// \since   September 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include <stdint.h>
#include "comm/Modbus/LtComModbus.h"

namespace LeddarConnection
{
    namespace LdConnectionModbuStructures
    {
#pragma pack(push, 1)

        // Server ID structure
        struct sModbusServerId
        {
            uint8_t  mNumberOfBytes;
            char     mSerialNumber[ 32 ];
            uint8_t  mRunIndicator;
            char     mDeviceName[ 32 ];
            char     mHardwarePartNumber[ 32 ];
            char     mSoftwarePartNumber[ 32 ];
            uint16_t mFirmwareVersion[ 4 ];
            uint16_t mBootloaderVersion[ 4 ];
            uint16_t mFPGABuildVersion;
            uint32_t mDeviceOption;
            uint16_t mDeviceType;
        };

        // Serial Port Settings
        struct sModbusSerialPortSetting
        {
            uint8_t  mLogicalSerialPortNumber;
            uint32_t mBaudRate;
            uint8_t  mDataSize;
            uint8_t  mParity;
            uint8_t  mStopBit;
            uint8_t  mFlowControl;
            uint8_t  mModbusAddr;
            uint8_t  mMaxEchos;
            uint16_t mDistanceResolution;
        };

        // CAN Port Settings
        struct sCANPortSetting
        {
            uint8_t  mLogicalCANPortNumber;
            uint32_t mBaudRate;
            uint8_t  mFrameFormat;
            uint32_t mTxBaseId;
            uint32_t mRxBaseId;
            uint8_t  mMaxNumberDetection;
            uint16_t mDistanceResolution;
            uint16_t mInterMessageDelay;
            uint16_t mInterCycleDelay;
        };

        // Carrier Settings
        struct sCarrierDeviceInfo
        {
            char     mHardwarePartNumber[32];    ///< Carrier hardware part number.
            char     mHardwareSerialNumber[32];  ///< Carrier hardware serial number.
            uint32_t mCarrierDeviceOption;       ///< Carrier device option.
        };


        /* ============================================================== */

        struct sModbusHeader
        {
            uint8_t  mModbusAddress;
            uint8_t  mFunctionCode;
        };

        struct sModbusReadDataReq
        {
            uint32_t    mBaseAddress;
            uint8_t     mNumberOfBytesToRead;
        };

        struct sModbusReadDataAnswer
        {
            uint32_t    mBaseAddress;
            uint8_t     mNumberOfReadBytes;
            uint8_t     mData[LTMODBUS_RTU_MAX_ADU_LENGTH - sizeof( sModbusHeader ) - sizeof( uint32_t ) - sizeof( uint8_t ) - MODBUS_CRC_SIZE]; // Size must be 247 bytes...
        };

        struct sModbusWriteDataReq
        {
            uint32_t    mBaseAddress;
            uint8_t     mNumberOfBytesToWrite;
            uint8_t     mData[LTMODBUS_RTU_MAX_ADU_LENGTH - sizeof( sModbusHeader ) - sizeof( uint32_t ) - sizeof( uint8_t ) - MODBUS_CRC_SIZE]; // Size must be 247 bytes...
        };

        struct sModbusWriteDataAnswer
        {
            uint32_t    mBaseAddress;
            uint8_t     mNumberOfWrittenBytes;
        };

        struct sModbusSendOpCodeReq
        {
            uint8_t     mOpCode;
            uint8_t     mOptionalArg;
        };

        struct sModbusSendOpCodeAnswer
        {
            uint8_t     mOpCode;
            uint8_t     mRetVal;
        };

        struct sModbusGetSerialPortSettingReq
        {
            uint8_t  mSubFunctionCode;
        };

        struct sModbusGetSerialPortSettingAnswer
        {
            uint8_t     mSubFunctionCode;
            uint8_t  mNumberOfSerialPort;
            uint8_t  mCurrentSerialPort;
            sModbusSerialPortSetting mSerialPortSettings[4];
        };

        struct sModbusSetSerialPortSettingReq
        {
            uint8_t  mSubFunctionCode;
            sModbusSerialPortSetting mSerialPortSettings[4];
        };

        struct sModbusSetSerialPortSettingAnswer
        {
            uint8_t     mSubFunctionCode;
        };

        struct sModbusGetCarrierFirmwareInfoReq
        {
            uint8_t     mSubFunctionCode;
        };

        struct sModbusGetCarrierFirmwareInfoAnswer
        {
            uint8_t     mSubFunctionCode;
            char        mFirmwarePartNumber[32];    ///< Carrier firmware part number.
            uint16_t    mFirmwareVersion[4];        ///< Carrier firmware build version (in format of A.B.C.D).
        };

        struct sModbusGetCarrierDeviceInfoReq
        {
            uint8_t     mSubFunctionCode;
        };

        struct sModbusGetCarrierDeviceInfoAnswer
        {
            uint8_t     mSubFunctionCode;
            sCarrierDeviceInfo  mCarrierDeviceInfo;
        };

        struct sModbusGetCANPortSettingReq
        {
            uint8_t     mSubFunctionCode;
        };

        struct sModbusGetCANPortSettingAnswer
        {
            uint8_t  mSubFunctionCode;
            uint8_t         mNumberOfCANPort;
            sCANPortSetting mCANPortSettings[2];
        };

        struct sModbusSetCANPortSettingReq
        {
            uint8_t  mSubFunctionCode;
            sCANPortSetting mCANPortSettings[2];
        };

        struct sModbusSetCANPortSettingAnswer
        {
            uint8_t     mSubFunctionCode;
        };


        /* ============================================================== */



        struct sModbusPacket
        {
            sModbusHeader    mHeader;
            union
            {
                union
                {
                    sModbusReadDataReq                  mReadData;
                    sModbusWriteDataReq                 mWriteData;
                    sModbusSendOpCodeReq                mSendOpCode;
                    sModbusGetSerialPortSettingReq      mGetSerialPortSetting;
                    sModbusSetSerialPortSettingReq      mSetSerialPortSetting;
                    sModbusGetCarrierFirmwareInfoReq    mGetCarrierFirwwareInfo;
                    sModbusGetCarrierDeviceInfoReq      mGetCarrierDeviceInfo;
                    sModbusGetCANPortSettingReq         mGetCANPortSetting;
                    sModbusSetCANPortSettingReq         mSetCANPortSetting;
                } uRequest;
                union
                {
                    sModbusServerId                     mServerId;
                    sModbusReadDataAnswer               mReadData;
                    sModbusWriteDataAnswer              mWriteData;
                    sModbusSendOpCodeAnswer             mSendOpCode;
                    sModbusGetSerialPortSettingAnswer   mGetSerialPortSetting;
                    sModbusSetSerialPortSettingAnswer   mSetSerialPortSetting;
                    sModbusGetCarrierFirmwareInfoAnswer mGetCarrierFirwwareInfo;
                    sModbusGetCarrierDeviceInfoAnswer   mGetCarrierDeviceInfo;
                    sModbusGetCANPortSettingAnswer      mGetCANPortSetting;
                    sModbusSetCANPortSettingAnswer      mSetCANPortSetting;
                } uAnswer;

                uint8_t mRawDataArray[LTMODBUS_RTU_MAX_ADU_LENGTH - sizeof( sModbusHeader )];   // at least two bytes is reserved for Modbus CRC16...
            };
        };
    }
#pragma pack(pop)
}
