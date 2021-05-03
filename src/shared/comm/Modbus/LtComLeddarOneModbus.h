/// ****************************************************************************
///
/// \file      shared/comm/Modbus/LtComLeddarOneModbus.h
///
/// \brief     Structure definition for LeddarOne sensor using Modbus communication
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

namespace LtComLeddarOneModbus
{

    constexpr uint8_t ONE_MAX_SERIAL_DETECTIONS = 3;
    constexpr uint16_t ONE_DISTANCE_SCALE       = 1000;
    constexpr uint16_t ONE_TEMPERATURE_SCALE    = 256;
    constexpr uint16_t ONE_AMPLITUDE_SCALE      = 256;
    constexpr uint8_t ONE_MAX_ACC_EXP           = 12;
    constexpr uint8_t ONE_MAX_OVERS_EXP         = 3;
    constexpr int8_t ONE_MIN_SMOOTHING          = -17; ///< Min value is -16, -17 is used to disable it
    constexpr int8_t ONE_MAX_SMOOTHING          = 16;
    constexpr uint16_t ONE_WAIT_AFTER_REQUEST =
        2000; ///< Time to wait in us after a request to be sure the next request is properly transmitted. 2ms should be enough in most cases

    constexpr uint16_t ONE_TIMEBASE_SCALE         = 5000;
    constexpr uint8_t ONE_MIN_BASE_POINT_COUNT    = 2;
    constexpr uint8_t ONE_MAX_BASE_POINT_COUNT    = 15;
    constexpr uint8_t ONE_MAX_BASE_POINT_COUNT_LR = 30;
    constexpr uint8_t ONE_MIN_PULSE_NOISE_RATE    = 0;
    constexpr uint8_t ONE_MAX_PULSE_NOISE_RATE    = 10;
    constexpr uint8_t ONE_MIN_PULSE_NOISE_AVG     = 0;
    constexpr uint8_t ONE_MAX_PULSE_NOISE_AVG     = 32;
    constexpr uint8_t ONE_PART_NUMBER_LENGTH      = 11;
    constexpr uint8_t ONE_SERIAL_NUMBER_OLD       = 8; ///< Serial number length for firmware <=3
    constexpr uint8_t ONE_SERIAL_NUMBER           = 16;
    constexpr uint16_t ONE_MAX_AMPLITUDE          = 256;

    // Values for the firmware update
    constexpr uint8_t SCU_STO   = 1;
    constexpr uint8_t SCU_STX   = 2;
    constexpr uint8_t SCU_EOT   = 4;
    constexpr uint8_t SCU_ACK   = 6;
    constexpr uint8_t SCU_CRC16 = 0x43;

    constexpr uint8_t SCU_HEADER       = 3;
    constexpr uint8_t SCU_OVERHEAD     = 5;
    constexpr uint8_t SCU_SHORT_PACKET = 128;
    constexpr uint16_t SCU_LONG_PACKET  = 1024;

    enum eDeviceId
    {
        DID_ACCUMULATION_EXP            = 0,
        DID_OVERSAMPLING_EXP            = 1,
        DID_BASE_POINT_COUNT            = 2,
        DID_LED_INTENSITY               = 4,
        DID_ACQQUISITION_OPTIONS        = 6, // See eAcquisitionOptions
        DID_CHANGE_DELAY                = 7,
        DID_STATIC_NOISE_REMOVAL_ENABLE = 9,
        DID_STATIC_NOISE_UPDATE_ENABLE  = 10,
        DID_PRECISION                   = 11,
        DID_STATIC_NOISE_UPDATE_RATE    = 12,
        DID_STATIC_NOISE_UPDATE_AVERAGE = 13,
        DID_COM_SERIAL_PORT_BAUDRATE    = 29,
        DID_COM_SERIAL_PORT_ADDRESS     = 30
    };

    enum eCommands
    {
        CMD_GET_CALIB       = 0x43, ///< [Request] None [Answer] sLeddarOneGetCalibOld or sLeddarOneGetCalib
        CMD_WRITE_CONFIG    = 0x46, ///< [Request] None [Answer] None
        CMD_SOFTWARE_RESET  = 0x47, ///< [Request] None [Answer] None
        CMD_JUMP_BOOTLOADER = 0x48, ///< Switch the device in YMODEM mode to receive update[Request] None [Answer] None
    };

    enum eAcquisitionOptions
    {
        ONE_ACQ_OPTIONS_AUTO_LED_INTENSITY = 1 ///< Automatic led intensity.
    };

#pragma pack( push, 1 )
    typedef struct sLeddarOneServerId
    {
        uint8_t mSize;                /// Number of bytes of information (excluding this one). 0x33 or 0x43 depending on firmware version
        char mSerialNumber[8];        /// Serial number as an ASCII string
        uint8_t mRunStatus;           /// Run status 0: OFF, 0xFF:ON. Should always return 0xFF, otherwise the sensor is defective
        char mSoftwarePartNumber[11]; /// The software part number as an ASCII string
        char mHardwarePartNumber[11]; /// The hardware part number as an ASCII string
        uint16_t mFirmwareVersion[4]; /// The full firmware version as 4 16 - bit values
        uint32_t mFirmwareCRC;        /// The firmware 32 - bit CRC
        uint16_t mFPGAVersion;        /// The FPGA version
        uint32_t mDeviceOptions;      /// Device option flags(LeddarTech internal use)
        uint16_t mDeviceId;           /// Device identification code
        char mSerialNumberV2[16];     /// Serial number as an ASCII string, only used after Firmware update from september 2017
    } sLeddarOneServerId;

    typedef struct sLeddarOneDetections
    {
        uint16_t mTimeStampLSB;
        uint16_t mTimeStampMSB;
        uint16_t mTemperature;
        uint16_t mNumberDetections;
        uint16_t mDistance1;
        uint16_t mAmplitude1;
        uint16_t mDistance2;
        uint16_t mAmplitude2;
        uint16_t mDistance3;
        uint16_t mAmplitude3;
    } sLeddarOneDetections;

    typedef struct sLeddarOneGetCalibOld
    {
        int16_t mTimeBaseDelay; ///< Timebase delay scaled with ONE_TIMEBASE_SCALE
        uint8_t mHardwarePartNumber[ONE_PART_NUMBER_LENGTH];
        uint8_t mSerialNumber[ONE_SERIAL_NUMBER_OLD];
        uint32_t mOptions;

    } sLeddarOneGetCalibOld;

    typedef struct sLeddarOneGetCalib
    {
        int16_t mTimeBaseDelay; ///< Timebase delay scaled with ONE_TIMEBASE_SCALE
        uint8_t mHardwarePartNumber[ONE_PART_NUMBER_LENGTH];
        uint8_t mSerialNumber[ONE_SERIAL_NUMBER];
        uint32_t mOptions;
        int16_t mCompensations[6]; ///<Compensation scaled with ONE_TIMEBASE_SCALE

    } sLeddarOneGetCalib;
#pragma pack( pop )
} // namespace LtComLeddarOneModbus
