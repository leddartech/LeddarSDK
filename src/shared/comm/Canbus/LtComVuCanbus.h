////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   comm/Canbus/LtComVuCanbus.h
///
/// \brief  Defines data used in CANbus protocol for Vu sensors
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

namespace LtComCanBus
{
    const uint8_t VU_AMPLITUDE_SCALE = 64;
    const uint8_t VU_THREHSOLD_SCALE = 64;
    const uint8_t VU_FIRMWARE_VERSION_SIZE = 8;
    const uint8_t VU_SERIAL_NBR_SIZE = 32;
    const uint8_t VU_DEVICE_NAME_SIZE = 32;
    const uint8_t VU_SW_PART_NBR_SIZE = 32;
    const uint8_t VU_HW_PART_NBR_SIZE = 32;
    const uint8_t LEDDARVU8_MAX_CAN_DETECTIONS = 96;

    //Limits
    const uint8_t VU_MIN_ACC = 0;
    const uint8_t VU_MAX_ACC = 10;
    const uint8_t VU_MIN_OVERS = 0;
    const uint8_t VU_MAX_OVERS = 5;
    const uint8_t VU_MIN_BASE_POINT_COUNT = 2;
    const uint8_t VU_MAX_BASE_POINT_COUNT = 128;
    const int8_t VU_MIN_SMOOTHING = -16;
    const int8_t VU_MAX_SMOOTHING = 16;
    const uint8_t VU_MIN_AUTOECHO_AVG = 0;
    const uint8_t VU_MAX_AUTOECHO_AVG = 8;
    const uint8_t VU_MIN_AUTOFRAME_AVG = 1;
    const uint16_t VU_MAX_AUTOFRAME_AVG = 8192;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eRequestIdVu
    ///
    /// \brief  Values that represent request Identifiers to be sent with a request
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eRequestIdVu
    {
        VU_CMD_STOP_SEND_DETEC      = 1,    /// Stop sending detections continuously
        VU_CMD_SEND_DETECT_ONCE     = 2,    /// Send detection once. Argument available in userguide
        VU_CMD_START_SEND_DETECT    = 3,    /// Start sending detections continuously. Argument available in userguide
        VU_CMD_GET_INPUT_DATA       = 4,    /// Get constant data
        VU_CMD_GET_HOLDING_DATA     = 5,    /// Get config data
        VU_CMD_SET_HOLDING_DATA     = 6,    /// Set config data
        VU_CMD_SET_BASE_ADDRESS     = 7,    /// Set base address for the following read / write commands
        VU_CMD_READ_DATA            = 8,    /// Read data from sensor at specified address
        VU_CMD_WRITE_DATA           = 9,    /// Read data on sensor at specified address
        VU_CMD_SEND_OP_CODE         = 10    /// Send module operation code
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eDeviceIdsVu
    ///
    /// \brief  Values that represent device ids for various command
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eDeviceIdsVu
    {
        //For input data (read only)
        VU_ID_SEGMENT_NUMBER     = 0,    /// Number of segments
        VU_ID_DEVICE_ID          = 1,    /// Device type and options (internal use)
        VU_ID_FIRMWARE_VERSION   = 2,    /// Firmware version. Id 2 to 3
        VU_ID_BOOTLOADER_VERSION = 4,    /// Boot loader bersion. Id 4 to 5
        VU_ID_FPGA_VERSION       = 6,    /// FPGA version
        VU_ID_SERIAL_NUMBER      = 7,    /// Serial number (ASCII). Id 7 to 12
        VU_ID_DEVICE_NAME        = 13,   /// Serial number (UTF16). Id 13 to 18
        VU_ID_HW_PART_NBR        = 19,   /// Software part number. Id 19 to 24
        VU_ID_SW_PART_NBR        = 25,   /// Software part number. Id 25 to 30

        //For holding data
        VU_ID_ACQ_CONFIG           = 0,    /// Accumulation exponent, oversampling exponent and number of base sample (one byte each)
        VU_ID_SMOOTHING_THRESHOLD  = 1,    /// Smoothing and Detection threshold
        VU_ID_LED_POWER            = 2,    /// Laser power % - Saturation configuration - Auto laser power delay
        VU_ID_ACQ_OPTIONS          = 3,    /// Byte 2 & 3 = Distance unit - Byte 4 & 5 = Bit 0 = auto led power. Bit 2 = Object demerging (enable = 1). Bit 3 = crosstalk removal (enable = 0)
        VU_ID_CAN_PORT_CONF1       = 4,    /// Byte 2 = baudrate. Byte 3 = Frame format. Byte 4 to 7 = Tx base ID
        VU_ID_CAN_PORT_CONF2       = 5,   /// Byte 4 to 7 = Rx base ID
        VU_ID_CAN_PORT_CONF3       = 6,   /// Byte 3 = Max number of detections by frame (1 to 96). Byte 4 to 5 = Inter-message delay (0 to 65535 ms). Byte 5 to 6 = Inter-cycle delay (0 to 65535 ms)
        VU_ID_SEGMENT_ENABLE       = 8,   /// Bitfield of enabled segments
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eAcquisitionOptionsVu
    ///
    /// \brief  Values that represent acquisition options for VU8 sensors.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eAcquisitionOptionsVu
    {
        VU_ACQ_AUTO_LED_POWER          = 0x1,   /// Automatic laser power
        VU_ACQ_DEMERGE_ENABLE          = 0x2,   /// Enable demerge object
        VU_ACQ_STATIC_NOISE_REM_ENABLE = 0x4,   /// Enable static noise removal
        VU_ACQ_PRECISION_ENABLE        = 0x8,   /// Enable precision / smoothing
        VU_ACQ_SATURATION_COMP_ENABLE  = 0x10,  /// Enable saturation compensation
        VU_ACQ_OVERSHOOT_MNGMT_ENABLE  = 0x20,  /// Enable overshoot managment
        VU_ACQ_AUTO_LED_POWER_MODE     = 0x40   /// Automatic laser power mode
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \struct sVuCANEcho
    ///
    /// \brief  A VU8 echo for CAN protocol.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef struct sVuCANEcho
    {
        uint16_t mDistance;
        uint16_t mAmplitude;
        uint16_t mFlag;         /// See \ref LtComCanBus::eFlagMaskVu
        uint16_t mSegment;
    } sVuCANEcho;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eFlagMaskVu
    ///
    /// \brief  Values that represent flag mask for VU8 detections.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eFlagMaskVu
    {
        VU_FLAG_VALID     = 0x1, /// Detection is valid (always set)
        VU_FLAG_DEMERGED  = 0x2, /// Detection is the result of object demerging
        VU_FLAG_SATURATED = 0x8  /// Detection is saturated
    };
}
