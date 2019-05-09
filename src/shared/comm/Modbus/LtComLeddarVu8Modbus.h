/// ****************************************************************************
///
/// \file      shared/comm/Modbus/LtComLeddarVu8Modbus.h
///
/// \brief     Structure definition for Vu8 sensor using Modbus communication
///
/// \author    David Levy
///
/// \since     September 2017
///
/// \copyright (c) 2017 LeddarTech Inc. All rights reserved.
///
/// ***************************************************************************

#pragma once

#include "comm/Modbus/LtComModbus.h"

namespace LtComLeddarVu8Modbus
{
#define LEDDARVU8_MAX_SERIAL_DETECTIONS 40
#define LEDDARVU8_MAX_CAN_DETECTIONS    96
#define LEDDARVU8_CHANNEL_COUNT         8
#define LEDDARVU8_DISTANCE_SCALE        1
#define LEDDARVU8_AMPLITUDE_SCALE       64
#define LEDDARVU8_THRESHOLD_SCALE       64
#define LEDDARVU8_MIN_SMOOTHING        -16
#define LEDDARVU8_MAX_SMOOTHING         16
#define LEDDARVU8_WAIT_AFTER_REQUEST    2000    //Time to wait in us after a request to be sure the next request is properly transmitted. 2ms should be enough in most cases
    const uint8_t LEDDARVU8_HSEGMENT = 8;
    const uint8_t LEDDARVU8_VSEGMENT = 1;
    const uint8_t LEDDARVU8_RSEGMENT = 1;

    enum eLtCommPlatformVu8ModbusAcqOptions
    {
        VU8_ACQ_OPTIONS_NONE                    = 0x0000,   ///< No acquisition options selected.
        VU8_ACQ_OPTIONS_AUTO_LED_POWER          = 0x0001,   ///< Automatic light source power
        VU8_ACQ_OPTIONS_DEMERGE_OBJECT          = 0x0002,   ///< Demerge object
        VU8_ACQ_OPTIONS_STATIC_NOISE_REMOVAL    = 0x0004,   ///< Static noise removal
        VU8_ACQ_OPTIONS_PRECISION               = 0x0008,   ///Precision/smoothing
        VU8_ACQ_OPTIONS_SATURATION_COMPENSATION = 0x0010,   ///Saturation compensation
        VU8_ACQ_OPTIONS_OVERSHOOT_MANAGEMENT    = 0x0020    ///Overshoot management
    };

    enum eDeviceId
    {
        DID_ACCUMULATION_EXP = 0,
        DID_OVERSAMPLING_EXP = 1,
        DID_BASE_POINT_COUNT = 2,
        DID_THRESHOLD_OFFSET = 4,
        DID_LED_INTENSITY = 5,
        DID_ACQ_OPTIONS = 6,
        DID_LED_AUTO_FRAME_AVG = 7,
        DID_LED_AUTO_ECHO_AVG = 9,
        DID_PRECISION = 11,
        DID_SEGMENT_ENABLE = 12
    };

#pragma pack(push, 1)
    typedef struct
    {
        uint16_t    mDistance;
        uint16_t    mAmplitude;
        uint8_t     mFlag;
        uint8_t     mSegment;
    } sLeddarVu8ModbusDetections;

    typedef struct
    {
        uint32_t    mTimestamp;
        uint8_t     mLedPower;
        uint16_t    mAcquisitionStatus;
    } sLeddarVu8ModbusDetectionsTrailing;

    /// \struct sModbusServerId
    /// \brief  Modbus server ID structure.
    typedef struct
    {
        uint8_t     mNbBytes;                   ///< Number of structure byte excluded this field.
        char        mSerialNumber[ 32 ];        ///< Receiver serial number.
        uint8_t     mRunIndicator;              ///< Run indicator.
        char        mDeviceName[ 32 ];          ///< Receiver device name.
        char        mHardwarePartNumber[ 32 ];  ///< Receiver hardware part number.
        char        mSoftwarePartNumber[ 32 ];  ///< Receiver software part number.
        uint16_t    mFirwareVersion[ 4 ];       ///< Receiver software version.
        uint16_t    mBootloaderVersion[ 4 ];    ///< Receiver bootloader version.
        uint16_t    mFpgaVersion;               ///< Receiver FPGA version.
        uint32_t    mDeviceOptions;             ///< Receiver device options.
        uint16_t    mDeviceId;                  ///< Receiver device ID.
    } sLeddarVu8ModbusServerId;

    typedef struct
    {
        uint8_t     mLogicalPortNumber;         ///< Serial logical port number.
        uint32_t    mBaudrate;                  ///< Serial port baudrate.
        uint8_t     mDataSize;                  ///< Serial port data size.
        uint8_t     mParity;                    ///< Serial port parity mode.
        uint8_t     mStopBits;                  ///< Serial port stop bits.
        uint8_t     mFlowControl;               ///< Serial port flow control.
        uint8_t     mAddress;                   ///< Serial port ModBus address.
        uint8_t     mMaxEchoes;                 ///< Max echoes to send by serial port.
        uint16_t    mEchoesResolution;          ///< Echoes distance resolution.
    } sLeddarVu8ModbusSerialPortSettings;

    typedef struct
    {
        uint8_t     mLogicalPortNumber;         ///< Can logical port number.
        uint32_t    mBaudrate;                  ///< Can port baudrate.
        uint8_t     mFrameFormat;               ///< Frame format
        uint32_t    mTxBaseId;                  ///< Tx base ID
        uint32_t    mRxBaseId;                  ///< Rx base ID
        uint8_t     mMaxEchoes;                 ///< Max echoes to send by can port.
        uint16_t    mEchoesResolution;          ///< Echoes distance resolution.
        uint16_t    mInterMsgDelay;             ///< Inter-message delay 0 trhough 65535 milliseconds
        uint16_t    mInterCycleDelay;           ///< Inter-cycle delay 0 through 65535 milliseconds
    } sLeddarVu8ModbusCanPortSettings;

    typedef struct
    {
        char        mHardwarePartNumber[ 32 ];    ///< Hardware part number
        char        mHardwareSerialNumber[ 32 ];  ///< Hardware serial number
        uint32_t    mCarrierDeviceOption;         ///< Carrier device option
    } sLeddarVu8ModbusCarrierInfo;

    typedef struct
    {
        char        mFirmwarePartNumber[ 32 ];    ///< Firmware part number
        uint16_t    mFirmwareVersion[ 4 ];        ///< Firmware version
    } sLeddarVu8ModbusCarrierFirmwareInfo;
#pragma pack(pop)
}
