////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   shared/comm/Canbus/LtComM16Canbus.h
///
/// \brief  Defines data used in CANbus protocol for M16 sensors
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>

namespace LtComCanBus
{
    const uint32_t M16_TEMPERATURE_SCALE = 65536;
    const uint32_t M16_THREHSOLD_SCALE = 524288;
    const uint32_t M16_AMPLITUDE_SCALE_STD = 4;
    const uint32_t M16_AMPLITUDE_SCALE_FLAG = 64;
    const uint8_t M16_ANSWER_ID_OFFSET = 128;
    const uint8_t M16_SERIAL_NBR_SIZE = 32;
    const uint8_t M16_DEVICE_NAME_SIZE = 64;
    const uint8_t M16_SW_PART_NBR_SIZE = 16;
    const uint8_t M16_HW_PART_NBR_SIZE = 16;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eRequestIdM16
    ///
    /// \brief  Values that represent request Identifiers to be sent with a request
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eRequestIdM16
    {
        M16_CMD_SEND_DETECT_ONCE_LEG  = 1,  ///< Legacy: Send detection once
        M16_CMD_START_SEND_DETECT_LEG = 2,  ///< Legacy: Start sending detections continuously
        M16_CMD_STOP_SEND_DETEC       = 3,  ///< Stop sending detections continuously
        M16_CMD_SEND_DETECT_ONCE      = 4,  ///< Send detection once. Argument available in userguide
        M16_CMD_START_SEND_DETECT     = 5,  ///< Start sending detections continuously. Argument available in userguide
        M16_CMD_GET_INPUT_DATA        = 6,  ///< Get constant data
        M16_CMD_GET_HOLDING_DATA      = 7,  ///< Get config data
        M16_CMD_SET_HOLDING_DATA      = 8   ///< Set config data
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eDeviceIdsM16
    ///
    /// \brief  Values that represent device ids for various command
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum eDeviceIdsM16
    {
        //For input data (read only)
        M16_ID_TEMP             = 0,    ///< Unscalled temperature
        M16_ID_DEVICE_ID        = 1,    ///< Device type and options (internal use)
        M16_ID_FIRMWARE_VERSION = 2,    ///< Firmware version
        M16_ID_FPGA_VERSION     = 3,    ///< FPGA version
        M16_ID_SERIAL_NUMBER    = 4,    ///< Serial number (ASCII). Id 4 to 9
        M16_ID_DEVICE_NAME      = 10,   ///< Serial number (UTF16). Id 10 to 20
        M16_ID_SW_PART_NBR      = 21,   ///< Software part number. Id 21 to 23
        M16_ID_HW_PART_NBR      = 24,   ///< Software part number. Id 24 to 26

        //For holding data
        M16_ID_ACQ_CONFIG           = 0,    ///< Accumulation exponent, oversampling exponent and number of base sample (one byte each)
        M16_ID_REFRESH_RATE         = 1,    ///< Refresh rate (IS16 only)
        M16_ID_THRESHOLD            = 2,    ///< Detection threshold
        M16_ID_LED_POWER            = 3,    ///< Led (or laser) power %
        M16_ID_ACQ_OPTIONS          = 4,    ///< Bit 0 = auto led power. Bit 2 = Object demerging (enable = 1). Bit 3 = crosstalk removal (enable = 0)
        M16_ID_AUTO_ACQ_DELAY       = 5,    ///< Number of detection before the led power is changed if necessary
        M16_ID_SMOOTHING            = 6,    ///< Smoothing - Stabilizes measurement (values between -16 and 16 - Disabled = -17)
        M16_ID_DISTANCE_UNITS       = 7,    ///< Distance unit (m = 1, dm = 00, cm = 100, mm = 1000)
        M16_ID_SEGMENT_ENABLE_COM   = 8,    ///< Bitfield of enabled segment (communication only)
        M16_ID_CAN_PORT_CONF1       = 9,    ///< Byte 2 = baudrate. Byte 3 = Frame format. Byte 4 to 7 = Tx base ID
        M16_ID_CAN_PORT_CONF2       = 10,   ///< Byte 4 to 7 = Rx base ID
        M16_ID_CAN_PORT_CONF3       = 11,   ///< Byte 2 = CAN operation mode. Byte 3 = Max number of detections by frame (1 to 96). Byte 4 to 5 = Inter-message delay (0 to 65535 ms). Byte 5 to 6 = Inter-cycle delay (0 to 65535 ms)
        M16_ID_SEGMENT_ENABLE       = 14,   ///< Bitfield of enabled pair of segments (enable / disable sensor segment by pair)
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \union  sM16CANEcho
    ///
    /// \brief  A M16 echo for CAN protocol.
    ///         Can either be raw data, a single detection with flag or two detections.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef union sM16CANEcho
    {
        uint8_t data[8];
        struct
        {
            uint16_t mDistance;
            uint16_t mAmplitude: 12;
            uint16_t mSegment:   4;
            uint16_t mDistance2;
            uint16_t mAmplitude2: 12;
            uint16_t mSegment2:   4;

        } mDetectionStd;
        struct
        {
            uint16_t mDistance;
            uint16_t mAmplitude;
            uint8_t mFlag;
            uint8_t mSegment;
        } mDetectionFlag;
    } sM16CANEcho;
}