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
#define ONE_MAX_SERIAL_DETECTIONS   3
#define ONE_DISTANCE_SCALE          1000
#define ONE_TEMPERATURE_SCALE       256
#define ONE_AMPLITUDE_SCALE         256
#define ONE_MAX_ACC_EXP             12
#define ONE_MAX_OVERS_EXP           3
#define ONE_MIN_SMOOTHING           -17 //Min value is -16, -17 is used to disable it
#define ONE_MAX_SMOOTHING           16
#define ONE_WAIT_AFTER_REQUEST      2000   //Time to wait in us after a request to be sure the next request is properly transmitted. 2ms should be enough in most cases
    const uint8_t ONE_MIN_BASE_POINT_COUNT = 2;
    const uint8_t ONE_MAX_BASE_POINT_COUNT = 15;
    const uint8_t ONE_MIN_PULSE_NOISE_RATE = 0;
    const uint8_t ONE_MAX_PULSE_NOISE_RATE = 10;
    const uint8_t ONE_MIN_PULSE_NOISE_AVG = 0;
    const uint8_t ONE_MAX_PULSE_NOISE_AVG = 32;

    enum eDeviceId
    {
        DID_ACCUMULATION_EXP = 0,
        DID_OVERSAMPLING_EXP = 1,
        DID_BASE_POINT_COUNT = 2,
        DID_LED_INTENSITY = 4,
        DID_LED_AUTO_PWR_ENABLE = 6,
        DID_CHANGE_DELAY = 7,
        DID_STATIC_NOISE_REMOVAL_ENABLE = 9,
        DID_STATIC_NOISE_UPDATE_ENABLE = 10,
        DID_PRECISION = 11,
        DID_STATIC_NOISE_UPDATE_RATE = 12,
        DID_STATIC_NOISE_UPDATE_AVERAGE = 13,
        DID_COM_SERIAL_PORT_BAUDRATE = 29,
        DID_COM_SERIAL_PORT_ADDRESS = 30
    };

#pragma pack(push,1)
    typedef struct
    {
        uint8_t     mSize;                      /// Number of bytes of information (excluding this one). 0x33 or 0x43 depending on firmware version
        char        mSerialNumber[8];           /// Serial number as an ASCII string
        uint8_t     mRunStatus;                 /// Run status 0: OFF, 0xFF:ON. Should always return 0xFF, otherwise the sensor is defective
        char        mSoftwarePartNumber[11];    /// The software part number as an ASCII string
        char        mHardwarePartNumber[11];    /// The hardware part number as an ASCII string
        uint16_t    mFirmwareVersion[4];        /// The full firmware version as 4 16 - bit values
        uint32_t    mFirmwareCRC;               /// The firmware 32 - bit CRC
        uint16_t    mFPGAVersion;               /// The FPGA version
        uint32_t    mDeviceOptions;             /// Device option flags(LeddarTech internal use)
        uint16_t    mDeviceId;                  /// Device identification code
        char        mSerialNumberV2[16];        /// Serial number as an ASCII string, only used after Firmware update from september 2017
    } sLeddarOneServerId;

    typedef struct
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
#pragma pack(pop)
}
