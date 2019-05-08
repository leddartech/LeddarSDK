/// ****************************************************************************
///
/// \file      shared/comm/Modbus/LtComLeddarM16Modbus.h
///
/// \brief     Structure definition for M16 sensor using Modbus communication
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

namespace LtComLeddarM16Modbus
{
    const uint8_t M16_MAX_SERIAL_DETECTIONS = 48;
    const uint8_t M16_DISTANCE_SCALE = 1;
    const uint8_t M16_AMPLITUDE_SCALE = 64;
    const uint16_t M16_TEMPERATURE_SCALE = 256;
    const uint16_t M16_SENSITIVITY_SCALE = 256;
    const uint8_t M16_SERIAL_NBR_SIZE = 32;
    const uint8_t M16_DEVICE_NAME_SIZE = 64;
    const uint8_t M16_SW_PART_NBR_SIZE = 16;
    const uint8_t M16_HW_PART_NBR_SIZE = 16;
    const uint16_t M16_MIN_DELAY = 1;   ///< Minimum value for ID_CHANGE_DELAY property
    const uint16_t M16_MAX_DELAY = 8192;   ///< Maximum value for ID_CHANGE_DELAY property

    const uint16_t M16_WAIT_AFTER_REQUEST = 2000;   ///< Time to wait in us after a request to be sure the next request is properly transmitted. 2ms should be enough in most cases

    enum eDeviceId
    {
        DID_ACCUMULATION_EXP = 0,
        DID_OVERSAMPLING_EXP = 1,
        DID_BASE_POINT_COUNT = 2,
        DID_REFRESH_RATE = 3,
        DID_THRESHOLD_OFFSET = 4,
        DID_LED_INTENSITY = 5,
        DID_ACQ_OPTIONS = 6,
        DID_CHANGE_DELAY = 7,
        DID_COM_SERIAL_PORT_MAX_ECHOES = 8,
        DID_PRECISION = 11,
        DID_COM_SERIAL_PORT_ECHOES_RES = 14,
        DID_SEGMENT_ENABLE_COM = 15,
        DID_SEGMENT_ENABLE_DEVICE = 18,
        DID_COM_SERIAL_PORT_STOP_BITS = 27,
        DID_COM_SERIAL_PORT_PARITY = 28,
        DID_COM_SERIAL_PORT_BAUDRATE = 29,
        DID_COM_SERIAL_PORT_ADDRESS = 30
    };

#pragma pack(push,1)
    typedef struct
    {
        uint8_t     mSize;                      /// Number of bytes of information (excluding this one). Currently 0x95 since the size of information returned is fixed.
        char        mSerialNumber[32];          /// Serial number as an ASCII string
        uint8_t     mRunStatus;                 /// Run status 0: OFF, 0xFF:ON. Should always return 0xFF, otherwise the sensor is defective
        char        mDeviceName[64];            /// The device name as a Unicode string
        char        mSoftwarePartNumber[16];    /// The software part number as an ASCII string
        char        mHardwarePartNumber[16];    /// The hardware part number as an ASCII string
        uint16_t    mFirmwareVersion[4];        /// The full firmware version as 4 16 - bit values
        uint32_t    mFirmwareCRC;               /// The firmware 32 - bit CRC
        uint16_t    mFirmwareType;              /// The firmware type(LeddarTech internal use)
        uint16_t    mFPGAVersion;               /// The FPGA version
        uint32_t    mDeviceOptions;             /// Device option flags(LeddarTech internal use)
        uint16_t    mDeviceId;                  /// Device identification code (9 for sensor module)
    } sLeddarM16ServerId;

    typedef struct
    {
        uint16_t    mDistance;
        uint16_t    mAmplitude;
        uint8_t     mFlags;
        /// Low 4 bits are flags describing the measurement:
        /// Bit 0 - Detection is valid(will always be set)
        /// Bit 1 - Detection was the result of object demerging
        /// Bit 2 - Reserved
        /// Bit 3 - Detection is saturated
        /// High 4 bits are the segment number.
    } sLeddarM16Detections0x41;

    typedef struct
    {
        uint16_t    mDistance;
        uint16_t    mAmplitude;
        uint8_t     mFlags;
        uint8_t     mSegment;
    } sLeddarM16Detections0x6A;
#pragma pack(pop)

    typedef enum eLtCommPlatformM16ModbusAcqOptions
    {
        /// \brief  Bits field acquisition options saved in configuration.
        M16_ACQ_OPTIONS_NONE = 0x0000,           ///< No acquisition options selected.

        M16_ACQ_OPTIONS_AUTO_LED_INTENSITY = 0x0001,        ///< Automatic led intensity.
        M16_ACQ_OPTIONS_DEMERGE_OBJECTS = 0x0004,           ///< Enable the two object demerge algorithm.
        M16_ACQ_OPTIONS_XTALK_REMOVAL_DISABLE = 0x0008,     ///< Disable xtalk removal algorithm (logical inverse).

        M16_ACQ_OPTIONS_MASK = 0x000D                       ///< Supported option mask.
    } eLtCommPlatformM16ModbusAcqOptions;
}
