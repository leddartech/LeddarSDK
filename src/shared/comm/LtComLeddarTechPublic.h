////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   shared/comm/LtComLeddarTechPublic.h
///
/// \brief  Structures and defines for LeddarTech protocol.
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define IDT_PORT                           (48620)  // Identification server port

// Protocols version
#define LT_COMM_CFG_PROT_VERSION            0x0002  ///< Current configuration server protocol version.
#define LT_COMM_DATA_PROT_VERSION           0x0003  ///< Current data server protocol version.

//#define LT_COMM_IDT_REQUEST_IP_CONFIG      (0x0001)
//#define LT_COMM_IDT_REQUEST_IDENTIFY       (0x0011)

// Current and obsolete protocols version
#ifndef LT_COMM_DATA_UDP_PROT_VERSION
#define LT_COMM_DATA_UDP_PROT_VERSION      (0x0002)
#endif

// Some field lengths
#define LT_COMM_DEVICE_UNICODE_NAME_LENGTH  32                                      ///< Device name length in number of wchar.
#define LT_COMM_DEVICE_NAME_LENGTH          (LT_COMM_DEVICE_UNICODE_NAME_LENGTH*2)  ///< Device name length in number of char (bytes).
#define LT_COMM_SERIAL_NUMBER_LENGTH        32                                      ///< Serial number length in bytes.
#define LT_COMM_FPGA_VERSION_LENGTH         32                                      ///< FPGA version length in bytes.
#define LT_COMM_GROUP_ID_LENGTH             32                                      ///< Group ID length in byte
#define LT_COMM_PART_NUMBER_LENGTH          16                                      ///< Part number length in bytes.
#define LT_COMM_IDT_STATE_MESSAGE_LENGTH    64                                      ///< Identify server message length in bytes.
#define LT_COMM_FIRMWARE_VERSION_LENGTH     32                                      ///< Firmware version length (byte)
#define LT_COMM_LICENSE_KEY_LENGTH          16                                      ///< License key length (byte)
#define LT_COMM_ALERT_MSG_LENGTH            32                                      ///< Alert message length (byte)

#define LT_COMM_IDT_REQUEST_IP_CONFIG      (0x0001)
#define LT_COMM_IDT_REQUEST_IDENTIFY       (0x0011)

/// Maximum offset from default port when going through NAT
#define MAX_PORT_OFFSET 24


namespace LtComLeddarTechPublic
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommSoftwareType
    ///
    /// \brief  Software type.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommSoftwareType
    {
        LT_COMM_SOFTWARE_TYPE_INVALID    = 0x00,    ///< Invalid software type.
        LT_COMM_SOFTWARE_TYPE_MAIN       = 0x01,    ///< Main application also called the update application.
        LT_COMM_SOFTWARE_TYPE_FPGA       = 0x02,    ///< FPGA firmware.
        LT_COMM_SOFTWARE_TYPE_FACTORY    = 0x03,    ///< Factory application.
        LT_COMM_SOFTWARE_TYPE_BL         = 0x04,    ///< Bootloader application.
        LT_COMM_SOFTWARE_TYPE_ASIC_PATCH = 0x05,    ///< Asic patch.
        LT_COMM_SOFTWARE_TYPE_CARRIER    = 0x06,    ///< Carrier board commercial firmware
        LT_COMM_SOFTWARE_TYPE_OS         = 0x07     ///< Various data used by the OS, script, driver, etc...
    } eLtCommSoftwareType;

    typedef enum eLtCommDataLevel
    {
        /// \brief  Data level bit fields list.
        LT_DATA_LEVEL_NONE = 0x00000000,   ///< Nothing is sent.
        LT_DATA_LEVEL_STATE = 0x00000002,   ///< States data can be sent.
        LT_DATA_LEVEL_ECHOES = 0x00000010,   ///< Echoes data can be sent.
        LT_DATA_LEVEL_ALL = 0x00000012    ///< All data can be sent.
    } eLtCommDataLevel;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommGenericElementsIdCodes
    ///
    /// \brief  Communication IDs
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommGenericElementsIdCodes
    {
        //*********************************************************************************************************************************************************************************************
        LT_COMM_ID_COMMON_BASE = 0x0000,                             ///< (0x0000) Element id common base.
        //*********************************************************************************************************************************************************************************************

        LT_COMM_ID_INVALID = LT_COMM_ID_COMMON_BASE,                 ///< (0x0000) Reserved to indicate presence of an invalid element.
        LT_COMM_ID_RESERVED = 0x0001,                                ///< (0x0001) For reserved elements (usually alignment padding).
        LT_COMM_ID_CRC16 = 0x0002,                                   ///< (0x0002) {uint16_t} - For the crc-16 value, use the polynomial: \f$ 2^{15} + 2^{13} + 2^0 = 0xA001\f$
        LT_COMM_ID_RAW_DATA = 0x0003,                                ///< (0x0003) For raw data like code binary, logs, etc.
        LT_COMM_ID_STATUS = 0x0004,                                  ///< (0x0004) {uint8_t} Status of the ongoing command
        /* Codes between 0x0010 to 0x001F are reserved */
        LT_COMM_ID_PROCESSOR = 0x0020,                               ///< (0x0020) {uint8_t} See \ref eLtCommSoftwareType
        /* Codes between 0x0021 to 0x002F are reserved */
        LT_COMM_ID_HW_PART_NUMBER = 0x0030,                          ///< (0x0030) {char[LT_COMM_PART_NUMBER_LENGTH]} - Hardware part number.
        LT_COMM_ID_SERIAL_NUMBER = 0x0040,                           ///< (0x0040) {char[LT_COMM_SERIAL_NUMBER_LENGTH]} - Device serial number.
        LT_COMM_ID_DEVICE_OPTIONS = 0x0050,                          ///< (0x0050) Bit field for device options. See \ref LtComM16::eLtCommDeviceOptions Width determined by LT_COMM_DEVICE_OPTION_COUNT.
        LT_COMM_ID_ELEMENT_LIST = 0x0070,                            ///< (0x0070) {uint16_t}[] - Array of element IDs (max nb of element: PROTOCOL_ELEMENT_LIST_MAX_SIZE,
        LT_COMM_ID_REQUEST_ELEMENT_LIST = 0x0080,                    ///< (0x0080) {LtPROTOCOLRequest}[] - Array of LtPROTOCOLRequest
        /* Codes between 0x0090 to 0x009F are reserved */
        LT_COMM_ID_REQUEST_HEADER = 0x00A0,                          ///< (0x00A0) {LtIpv4RequestHeader}
        /* Codes between 0x00B0 to 0x00DF are reserved */
        LT_COMM_ID_DEVICE_TYPE = 0x00E0,                             ///< (0x00E0) {uint16_t} - See PROTOCOL_IDT_DEVICE_TYPE_...
        LT_COMM_ID_RELEASE_TYPE = 0x00E1,                            ///< (0x00E1) {uint8_t} - Flag defining release type, see eLtReleaseType */
        LT_COMM_ID_FIRMWARE_VERSION = 0x00F0,                        ///< (0x00F0) {uint16_t} - Firmware version build number
        LT_COMM_ID_FIRMWARE_VERSION_V2 = 0x00F1,                     ///< (0x00F1) {uint16_t}[4] - Full firmware version: major.minor.release.build
        LT_COMM_ID_FPGA_VERSION = 0x00F2,                            ///< (0x00F2) {char[LT_COMM_FPGA_VERSION_LENGTH]} - FPGA Version
        LT_COMM_ID_FIRMWARE_VERSION_V3 = 0x00F3,                     ///< (0x00F1) {sFirmwareVersion} - Full firmware version: major.minor.release.build. See \ref LtComLeddarTechPublic::sFirmwareVersion
        LT_COMM_ID_PROTOCOL_VERSION = 0x0100,                        ///< (0x0100) {uint16_t} - See PROTOCOL_PROTOCOL_VERSION_...
        LT_COMM_ID_SOFTWARE_VERSION = 0x0110,                        ///< (0x0110) {uint16_t} - Software version
        /* Codes between 0x0120 to 0x013F are reserved */
        LT_COMM_ID_DEVICE_NAME = 0x0140,                             ///< (0x0140) char[LT_COMM_DEVICE_NAME_LENGTH] - The device name is defined as byte
        ///<                                             array but unicode char may be stored on it.
        ///<                                             All unused byte must be set to 0. This make
        ///<                                             possible to detect the presence of an unicode string.
        /* Codes between 0x0150 to 0x015F are reserved */
        LT_COMM_ID_SOFTWARE_PART_NUMBER = 0x0160,                    ///< (0x0160) {char[]} - Software part number
        LT_COMM_ID_ANSWER_HEADER = 0x0170,                           ///< (0x0170) {LtPROTOCOLAnswerHeader}
        LT_COMM_ID_SENSOR_ID = 0x0180,                               ///< (0x0180) {uint32_t} - d-tec sensor ID sending detection statuses which is actually part of its Ethernet address

        /* Codes between 0x0190 to 0x019F are reserved */
        LT_COMM_ID_CFG_BASE_SAMPLE_COUNT = 0x01A0,                   ///< (0x01A0) {uint32_t} Number of base points.
        LT_COMM_ID_CFG_ACCUMULATION_EXPONENT = 0x01A1,               ///< (0x01A1) {uint32_t} Number of accumulation in log2 exponent.
        LT_COMM_ID_CFG_OVERSAMPLING_EXPONENT = 0x01A2,               ///< (0x01A2) {uint32_t} Number of oversampling in log2 exponent.
        LT_COMM_ID_TRACE_LENGTH = 0x01A3,                            ///< (0x01A3) {uint32_t} Current trace length.
        LT_COMM_ID_ACCUMULATION = 0x01A4,                            ///< (0x01A4) {uint32_t} Number of accumulation.
        LT_COMM_ID_OVERSAMPLING = 0x01A5,                            ///< (0x01A5) {uint32_t} Number of oversampling.
        LT_COMM_ID_TRACE_LENGTH_MAX = 0x01A6,                        ///< (0x01A6) {uint32_t} Maximum trace length.
        LT_COMM_ID_OPEN_UPDATE_SESSION = 0x01A7,                     ///< (0x01A7) {uint32_t} Open an update session. Some elements are required.
        LT_COMM_ID_CLOSE_UPDATE_SESSION = 0x01A8,                    ///< (0x01A8) {uint32_t} Close an update session. Some elements are required.
        LT_COMM_ID_FILE_LENGTH = 0x01A9,                             ///< (0x01A9) {uint32_t} File length in bytes.
        LT_COMM_ID_BLOCK_LENGTH = 0x01AA,                            ///< (0x01AA) {uint32_t} Supported block length in bytes.
        LT_COMM_ID_UPDATE_LAST_ERROR = 0x01AB,                       ///< (0x01AB) {uint16_t} Last error reach during update session.
        LT_COMM_ID_LIMIT_CFG_BASE_SAMPLE_COUNT = 0x01AC,             ///< (0x01AC) {uint32_t}[2] Minimum and maximum number of base points permitted to LT_COMM_ID_CFG_BASE_SAMPLE_COUNT.
        LT_COMM_ID_LIMIT_CFG_ACCUMULATION_EXPONENT = 0x01AD,         ///< (0x01AD) {uint32_t}[2] Minimum and maximum number of accumulation permitted to LT_COMM_ID_CFG_ACCUMULATION_EXPONENT.
        LT_COMM_ID_LIMIT_CFG_OVERSAMPLING_EXPONENT = 0x01AE,         ///< (0x01AE) {uint32_t}[2] Minimum and maximum number of oversampling permitted to LT_COMM_ID_CFG_OVERSAMPLING_EXPONENT.
        LT_COMM_ID_BASE_SAMPLE_DISTANCE = 0x01AF,                    ///< (0x01AF) {float} Distance between two base count.
        LT_COMM_ID_NUMBER_OF_SEGMENTS = 0x01B0,                      ///< (0x01B0) {uint8_t} Number of detection segments (channels).
        LT_COMM_ID_TEMPERATURE_SCALE = 0x01B1,                       ///< (0x01B1) {uint32_t} Temperature scale for fixed-point value.
        LT_COMM_ID_LIGHT_SRC_FREQUENCY = 0x01B2,                     ///< (0x01B2) {uint32_t} Light source frequency in Hz.

        LT_COMM_ID_SENSOR_POSITION_X = 0x01C0,                       ///< (0x01C0) {float} Sensor X position.
        LT_COMM_ID_SENSOR_POSITION_Y = 0x01C1,                       ///< (0x01C1) {float} Sensor Y position.
        LT_COMM_ID_SENSOR_POSITION_Z = 0x01C2,                       ///< (0x01C2) {float} Sensor Z position.
        LT_COMM_ID_SENSOR_ORIENTATION_YAW = 0x01C3,                  ///< (0x01C3) {float} Sensor yaw orientation.
        LT_COMM_ID_SENSOR_ORIENTATION_PITCH = 0x01C4,                ///< (0x01C4) {float} Sensor pitch orientation.
        LT_COMM_ID_SENSOR_ORIENTATION_ROLL = 0x01C5,                 ///< (0x01C5) {float} Sensor roll orientation.
        LT_COMM_ID_LICENSE = 0x01D0,                                 ///< (0x01D0) {uint8_t[LT_COMM_LICENSE_KEY_LENGTH]} License key to add to sensor
        LT_COMM_ID_VOLATILE_LICENSE = 0x01D1,                        ///< (0x01D1) {uint8_t[LT_COMM_LICENSE_KEY_LENGTH]} Volatile license key to add to sensor
        LT_COMM_ID_GROUP_ID = 0x01D2,                                ///< (0x01D2) {char[REGMAP_GROUP_ID_LENGTH]} Group identification of the sensor
        LT_COMM_ID_PRECISION = 0x01D3,                               ///< (0x01D3) {int16_t} Value of the precision / smoothing
        LT_COMM_ID_PRECISION_LIMITS = 0x01D4,                        ///< (0x01D4) {int16_t[2]} Min and max value of the precision / smoothing
        LT_COMM_ID_PRECISION_ENABLE = 0x01D5,                        ///< (0x01D5) {bool} Enable precision / smoothing
        LT_COMM_ID_MAX_ECHOES_PER_CHANNEL = 0x01D6,                  ///< (0x01D6) {uint16_t} Max number of echoes on the same channel
        LT_COMM_ID_SATURATION_MNG_ENABLE = 0x01D7,                   ///< (0x01D7) {bool} Enable saturation management
        LT_COMM_ID_OVERSHOOT_MNG_ENABLE = 0x01D8,                    ///< (0x01D8) {bool} Enable overshoot management
        LT_COMM_ID_DEMERGING_ENABLE = 0x01D9,                        ///< (0x01D9) {bool} Enable demerging
        LT_COMM_ID_STATIC_NOISE_REMOVAL_ENABLE = 0x01DA,             ///< (0x01DA) {bool} Enable static noise removal
        LT_COMM_ID_STATIC_NOISE_TEMP_COMP_ENABLE = 0x01DB,           ///< (0x01DB) {BOOL} Enable static noise temperature compensation removal
        LT_COMM_ID_ECHOES_TEMPERATURE_CRRCTN_SLOPE = 0x01DD,         ///< (0x01DD) {float}SET/GET ECHOES TEMPERATURE CORRECTION SLOPE
        LT_COMM_ID_TIMEBASE_CALIB_TEMPERATURE = 0x01DE,              ///< (0x01DE) {float}SET/GET The temperature at which the Time base delay got calibrated
        LT_COMM_ID_STATIC_NOISE_CALIB_TEMPERATURE = 0x01DF,          ///< (0x01DF) {LtFloat32[LT_COMM_ID_LED_POWER_COUNT]}SET/GET The temperature at which the static noise got calibrated
        LT_COMM_ID_LED_POWER = 0x01E0,                               ///< (0x01E0) {uint8_t} Led power in percent
        LT_COMM_ID_LED_POWER_COUNT = 0x01E1,                         ///< (0x01E1) {uint8_t} Number of different led power available
        LT_COMM_ID_LED_POWER_AVAILABLE = 0x01E2,                     ///< (0x01E2) {uint8_t[LT_COMM_ID_LED_POWER_COUNT]} List of the led power % available in the sensor
        LT_COMM_ID_LED_POWER_ENABLE = 0x01E3,                        ///< (0x01E2) {uint_8} Led power enable (automatic/manual)
        LT_COMM_ID_HFOV = 0x01F0,                                    ///< (0x01F0) {float} Horizontal field of view of the sensor
        LT_COMM_ID_VFOV = 0x01F1,                                    ///< (0x01F1) {float} Vertical field of view of the sensor
        LT_COMM_ID_TIMEBASE_DELAYS = 0x01FA,                         ///< (0x01FA) {float[channelCount]} Timebase delay for each channel
        LT_COMM_ID_COMPENSATIONS = 0x01FB,                           ///< (0x01FB) {float[LT_COMM_ID_LED_POWER_COUNT]} Led power compensation for each channel
        LT_COMM_ID_STATIC_NOISE = 0x01FC,                            ///< (0x01FC) Raw data sent through write memory - Or array of trace received through GetCalib
        LT_COMM_ID_LICENSE_INFO = 0x01FD,                            ///< (0x01FD) {uint32_t} Info about license on sensor (bit 0-15: Access rights of the license, bit 16-17: License type).
        LT_COMM_ID_VOLATILE_LICENSE_INFO = 0x01FE,                   ///< (0x01FE) {uint32_t} Info about license on sensor (bit 0-15: Access rights of the license, bit 16-17: License type).
        LT_COMM_ID_THREHSOLD_OFFSET = 0x0200,                        ///< (0x0200) {LtFixedPoint} Threshold table offset in \ref LT_COMM_ID_AMPLITUDE_SCALE
        LT_COMM_ID_THREHSOLD_OFFSET_LIMITS = 0x0201,                 ///< (0x0201) {LtFixedPoint[2]} Min and max value of \ref LT_COMM_ID_THREHSOLD_OFFSET
        LT_COMM_ID_PULSE_RATE = 0x0202,                              ///< (0x0202) {uint32_t} Number of pulses per second, to compute theoretical frame rate versus ACC/OVR
        LT_COMM_ID_ELEC_XTALK_RMV_ENABLE = 0x0204,                   ///< (0x0204) {bool} Enable electronic xtalk removal
        LT_COMM_ID_FILTER_ENABLE = 0x0205,                           ///< (0x0205) {bool} Enable the digital filter
        LT_COMM_ID_STATIC_THRESHOLD_ENABLE = 0x0206,                 ///<          {BOOL} Enable static threshold dependent on position in trace
        //*********************************************************************************************************************************************************************************************
        LT_COMM_ID_PLATFORM_SPECIFIC_BASE = 0x1000,                  ///< (0x1000) Platform specific element ids starting base. Ids defined in a separate header file.
        LT_COMM_ID_DISTANCE_SCALE = 0x1004,                          ///<          {uint32_t} Distance scale for fixed-point value
        LT_COMM_ID_AMPLITUDE_SCALE = 0x1005,                         ///<          {uint32_t} Amplitude scale scale for fixed-point value
        LT_COMM_ID_FILTERED_SCALE = 0x1006,                          ///<          {uint32_t} Filtered scale for fixed-point value
        LT_COMM_ID_REAL_DIST_OFFSET = 0x1009,                        ///<          {LtFixedPoint} Real distance offset
        LT_COMM_ID_REFRESH_RATE = 0x100A,                            ///<          {float} Current refresh rate
        LT_COMM_ID_TRACE_POINT_STEP = 0x100B,                        ///<          {float} Trace point step
        LT_COMM_ID_CURRENT_TIME_MS = 0x100F,                         ///<          {uint32_t} - System time in ms since last reset
        LT_COMM_ID_CURRENT_TIME_S = 0x1010,                          ///<          {uint32_t} - System time in seconds since last power cycle
        LT_COMM_ID_SYS_TEMP = 0x1011,                                ///<          {LtFixedPoint} - System temperature in temperature scale (get \ref LT_COMM_ID_TEMPERATURE_SCALE).
        LT_COMM_ID_CPU_TEMP = 0x1012,                                ///< (0x1012) {LtFixedPoint} - CPU temperature in temperature scale (get \ref LT_COMM_ID_TEMPERATURE_SCALE).

        LT_COMM_ID_MEMORY_ACCESS = 0x1035,                           ///<          {sLtCommElementMemoryAccess}
        LT_COMM_ID_TRACE_LENGTH_ELEMENT = 0x1036,                    ///<          {uint16_t} - Trace buffer length sent in data server
        LT_COMM_ID_MAX_TRACE_LENGTH = 0x1037,                        ///<          {LtUInt16} - Maximum trace buffer length sent in data server
        LT_COMM_ID_SOFTWARE_TYPE = 0x1048,                           ///<          {uint8_t} - LT_COMM_SOFTWARE_TYPE_MAIN or LT_COMM_SOFTWARE_TYPE_FACTORY
        LT_COMM_ID_CPU_LOAD_V2 = 0x104A,                             ///<          {float} - CPU load in %
        LT_COMM_ID_TIMESTAMP = 0x1050,                               ///<          {uint32_t} - Timestamp of related data in ms.
        LT_COMM_ID_ECHOES_COUNT = 0x1053,                            ///<          {uint32_t} - Total number of echoes detected in PDTECS_ID_ECHOES for the current cycle.
        LT_COMM_ID_DATA_LEVEL_V2 = 0x106E,                           ///<          {uint32_t}: See LT_COMM_DATA_LEVEL_...
        LT_COMM_ID_CURRENT_TIME_SINCE_RESET_S = 0x1071,              ///<          {uint32_t} - System time in seconds since last reset
        LT_COMM_ID_ECHOES_AMPLITUDE = 0x1083,                        ///<          {uint32_t}[Number of echoes] - Amplitude scale fixed-point. Echo amplitude in ADC counts.
        LT_COMM_ID_ECHOES_DISTANCE = 0x1084,                         ///<          {LtFixedPoint}[Number of echoes] - Distance scale fixed-point. Echo distance in meters.
        LT_COMM_ID_ECHOES_BASE = 0x1085,                             ///<          {uint32_t}[Number of echoes] - Amplitude scale fixed-point. Echo base amplitude in counts (beginning of the echo rising signal position).
        LT_COMM_ID_ECHOES_MAX_INDEX = 0x1086,                        ///<          {uint16_t}[Number of echoes] - Index of the echo peak position in the signal acquisition traces.
        LT_COMM_ID_ECHOES_CHANNEL_INDEX = 0x1087,                    ///<          {uint8_t}[Number of echoes] - Index of the echo channel.
        LT_COMM_ID_ECHOES_VALID = 0x1088,                            ///<          {uint8_t}[Number of echoes] - Echo validity. 0 = marked as invalid by the algo; !=0 = marked as valid by the algo
        LT_COMM_ID_ECHOES_AMPLITUDE_LOW_SCALE = 0x1089,              ///<          {uint32_t}[Number of echoes] - Low amplitude scale fixed-point. Echo amplitude in ADC counts, including estimated amplitudes for saturated signals.
        LT_COMM_ID_ECHOES_SATURATION_WIDTH = 0x108A,                 ///<          {uint32_t}[Number of echoes] - Width in number of samples of the signal saturation for an estimated echo where the signal is saturated.
        LT_COMM_ID_SEC_SYS_TEMP = 0x108B,                            ///<          {LtFixedPoint} - Secondary system temperature in temperature scale (get \ref LT_COMM_ID_TEMPERATURE_SCALE).
        LT_COMM_ID_ECHO_AMPLITUDE_MAX = 0x108C,                      ///<          {uint32_t} - Max possible echo amplitude value
        LT_COMM_ID_VERTICAL_SEGMENT_SELECT = 0x108D,                 ///<          {uint32_t} - Enable or disable vertical line index
        LT_COMM_ID_DISABLED_CHANNELS = 0x108E,                       ///<          {uint32_t} Bitfield representing the list of disabled channels
        LT_COMM_ID_FRAME_ID = 0x1092,                                ///< (0x1092) {uint64_t} Id of the frame

        LT_COMM_ID_INTERF_ENABLE = 0x10A0,                           ///<          {bool} Enable interference algorithm
        LT_COMM_ID_INTERF_SAWTOOTH_CORRECTION_ENABLE = 0x10A1,       ///<          {bool} Trigger interference sawtooth correction algorithm
        LT_COMM_ID_INTERF_DELTA_THRESHOLD = 0x10A2,                  ///<          {int32_t} Set interference delta threshold value
        LT_COMM_ID_INTERF_DELTA_THRESHOLD_LIMITS = 0x10A3,           ///<          {int32_t[2]} Interference delta threshold limits
        LT_COMM_ID_INTERF_UNDERSHOOT_THRESHOLD = 0x10A4,             ///<          {int32_t} Set interference delta undershoot value
        LT_COMM_ID_INTERF_UNDERSHOOT_THRESHOLD_LIMITS = 0x10A5,      ///<          {int32_t[2]} Interference delta undershoot limits

        LT_COMM_ID_STATUS_ALERT = 0x10A6,                            ///< (0x10A6) {sLtCommElementAlert} Status flags.

        LT_COMM_ID_ENABLE_RANDOM_LINES_SEQ = 0x10AF,                 ///<          {uint8_t} Enable / Disable Random Lines sequence
        LT_COMM_ID_PEAK_THRESHOLD_TYPE = 0x10B0,                     ///<          {uint8_t}  Select the peak threshold type. 0 = Single Thr, 1 = Table Thr
        LT_COMM_ID_TRIGGER_MODE = 0x10B1,                            ///<          {uint8_t} Acquisition mode, 0 = Running, 1 = Trigger, 2 = Stop
        LT_COMM_ID_CROSSTALK_FILTER_SCALE = 0x10C1,                  ///< (0x10C1) {LtUint32} Optical Crosstalk Filter scale for fixed-point value
        LT_COMM_ID_INTER_TILE_CROSSTALK_FILTER_SCALE = 0x10C2,       ///< (0x10C2) {LtUint32} Inter-tile Crosstalk Filter scale for fixed-point value

        // CONSTANTS
        LT_COMM_ID_AUTO_CHANNEL_NUMBER_HORIZONTAL = 0x2001,          ///< (0x2001) {uint16_t} Number of horizontal channels
        LT_COMM_ID_AUTO_CHANNEL_NUMBER_VERTICAL = 0x2002,            ///< (0x2002) {uint16_t} Number of vertical channels
        LT_COMM_ID_AUTO_CHANNEL_SUB_NUMBER_HORIZONTAL = 0x2003,      ///< (0x2003) {uint16_t}[Number of sub zone] Number of horizontal channels
        LT_COMM_ID_AUTO_SUB_HFOV = 0x2004,                           ///< (0x2004) {LtFloat32}[Number of zones] Horizontal field of views of the sensor
        LT_COMM_ID_AUTO_SUB_POSITION = 0x2006,                       ///< (0x2006) {LtFloat32}[Number of zones] Horizontal position of each zone, relative to the center of the sensor

        LT_COMM_ID_AUTO_LED_AUTO_FRAME_AVG_LIMITS = 0x2020,          ///< (0x2020) {uint16_t[2]} Minimum and maximum number of auto acq avg frame permitted
        LT_COMM_ID_AUTO_LED_AUTO_ECHO_AVG_LIMITS = 0x2021,           ///< (0x2021) {uint16_t[2]} Minimum and maximum number of auto acq avg echo permitted

        // CONFIG
        LT_COMM_ID_AUTO_NUMBER_DATA_SENT = 0x2501,                   ///< (0x2501) {uint32_t[2]} Index of the first trace sent + number of traces sent by the data server
        LT_COMM_ID_AUTO_DATA_SERVER_PORT = 0x2502,                   ///< (0x2502) {uint16_t} LCA3 Data server port
        LT_COMM_ID_AUTO_DATA_SERVER_PROTOCOL = 0x2503,               ///< (0x2503) {uint8_t} TCP or UDP see \ref eLtCommProtocol

        LT_COMM_ID_AUTO_LED_AUTO_FRAME_AVG = 0x2520,                 ///< (0x2520) {uint16_t} auto acq avg frame
        LT_COMM_ID_AUTO_LED_AUTO_ECHO_AVG = 0x2521,                  ///< (0x2521) {uint16_t} auto acq avg echo

        LT_COMM_ID_AUTO_PROCESS_SATURATION_ENABLE = 0x253F,          ///< (0x253F) {BOOL} - saturation template replacement enable
        LT_COMM_ID_AUTO_XTALK_OPTIC_SEG_ENABLE = 0x2540,             ///< (0x2540) {bool} - xtalk optic segment enable
        LT_COMM_ID_AUTO_XTALK_OPTIC_LINE_ENABLE = 0x2541,            ///< (0x2541) {bool} - xtalk optic line enable
        LT_COMM_ID_AUTO_XTALK_OPTIC_ECH_SEG_ENABLE = 0x2542,         ///< (0x2542) {bool} - xtalk optical echoes segment enable
        LT_COMM_ID_AUTO_XTALK_OPTIC_ECH_LINE_ENABLE = 0x2543,        ///< (0x2543) {bool} - xtalk optical echoes line enable

        LT_COMM_ID_AUTO_THRESHOLD_OPTIONS = 0x2545,                  ///< (0x2545) {uint8_t} - Threshold config, see \ref eLtCommThresholdOptions
        LT_COMM_ID_AUTO_AUTOMATIC_THRESH_SENSITIVITY = 0x2546,       ///< (0x2546) {int32_t} - Automatic threshold sensitivity (in dB)
        LT_COMM_ID_AUTO_AUTOMATIC_THRESH_SENSITIVITY_LIMITS = 0x2547, ///< (0x2547) {int32_t[2]} - Min and max value of automatic threshold sensitivity

        LT_COMM_ID_AUTO_ACC_DIST_ENABLE = 0x2550,                    ///< (0x2550) {bool}     - accumulation according to distance enable
        LT_COMM_ID_AUTO_ACC_DIST_POSITION = 0x2551,                  ///< (0x2551) {uint32_t} - start position of accumulation according to distance
        LT_COMM_ID_AUTO_ACC_DIST_EXP = 0x2552,                       ///< (0x2552) {uint32_t} - number of accumulation according to distance
        LT_COMM_ID_LIMIT_AUTO_ACC_DIST_POSITION = 0x2553,            ///< (0x2553) {uint32_t} - minimum and maximum for position of accumulation according to distance
        LT_COMM_ID_LIMIT_AUTO_ACC_DIST_EXP = 0x2554,                 ///< (0x2554) {uint32_t} - minimum and maximum for number of accumulation according to distance
        LT_COMM_ID_AUTO_THREHSOLD_POS_OFFSET = 0x2555,               ///< (0x2555) {uint32_t}   Start position of threshold table offset
        LT_COMM_ID_LIMIT_AUTO_THR_POS_OFFSET = 0x2556,               ///< (0x2556) {uint32_t}   minimum and maximum for Start position of threshold table offset

        LT_COMM_ID_AUTO_THRESH_AGG_AMP = 0x2560,                     ///< (0x2560) {LtFixedPoint} - optical threshold that considered a pulse like an aggressor.
        LT_COMM_ID_AUTO_THRESH_VICTIM_DIST = 0x2561,                 ///< (0x2561) {LtFixedPoint} - optical threshold that considered a pulse like a victim
        LT_COMM_ID_AUTO_THRESH_ELEC_AGG_AMP = 0x2562,                ///< (0x2562) {LtFixedPoint} - electronic threshold that considered a pulse like an aggressor
        LT_COMM_ID_AUTO_THRESH_ECH_VICTIM_LEFT = 0x2563,             ///< (0x2563) {LtFixedPoint} - electronic threshold that considered a pulse like a victim
        LT_COMM_ID_AUTO_THRESH_ECH_VICTIM_RIGHT = 0x2564,            ///< (0x2564) {LtFixedPoint} - electronic threshold that considered a pulse like a victim
        LT_COMM_ID_AUTO_THRESH_GAUSS_SENSIB = 0x2565,                ///< (0x2565) {LtFixedPoint} - sensibility of the gaussian correction used by the echoes xtalk algorithm
        LT_COMM_ID_AUTO_THRESH_M_PEAK = 0x2566,                      ///< (0x2566) {LtFixedPoint} - detection threshold used by the Mpeaklight function

        LT_COMM_ID_AUTO_PDLT_TRIANGLE_THREHSOLD         = 0x2570,    ///< (0x2570) {LtFixedPoint} Threshold table offset in \ref LT_COMM_ID_AMPLITUDE_SCALE
        LT_COMM_ID_AUTO_PDLT_TRIANGLE_THREHSOLD_LIMITS  = 0x2571,    ///< (0x2571) {LtFixedPoint[2]} Min and max value of \ref LT_COMM_ID_AUTO_PDLT_TRIANGLE_THREHSOLD
        LT_COMM_ID_AUTO_PDLT_TRIANGLE_THR_DIST_END      = 0x2572,    ///< (0x2572) {LtFixedPoint} Meter distance value in \ref LT_COMM_ID_DISTANCE_SCALE
        LT_COMM_ID_AUTO_PDLT_TRIANGLE_THR_DIST_END_LIMITS = 0x2573,  ///< (0x2573) {LtFixedPoint[2]} Min and max value of \ref LT_COMM_ID_AUTO_PDLT_TRIANGLE_THR_DIST_END

        LT_COMM_ID_AUTO_CHANNEL_ANGLES_AZIMUT           = 0x2580,    ///< (0x2580) {LtFloat32[NumberOfChannels]} Azimut angles in degrees from the center of the FoV to the center of each channel
        LT_COMM_ID_AUTO_CHANNEL_ANGLES_ELEVATION        = 0x2581,    ///< (0x2581) {LtFloat32[NumberOfChannels]} Elevation angles in degrees from the center of the FoV to the center of each channel

        LT_COMM_ID_AUTO_SYSTEM_TIME                     = 0x259B,    ///< (0x259B) {uint64_t} Timestamp in microseconds since 1970/01/01
        LT_COMM_ID_AUTO_TIME_SYNC_METHOD                = 0x259C,    ///< (0x259C) {uint8_t} Time synchronization method to be used: 0 = none, 1 = PTP, 2 = PPS

        LT_COMM_ID_AUTO_XTALK_INTER_TILE_ENABLE         = 0x25B0,    ///< (0x25B0) {BOOL} Inter tile xtalk algorithm enable
        LT_COMM_ID_AUTO_SPACIAL_FILTER_ENABLE           = 0x25B1,    ///< (0x25B1) {BOOL} Spacial filtering algorithm enable


        //LT_COMM_ID_AUTO_ACQUISITION_MODE = 0x9876,
        //LT_COMM_ID_AUTO_ACQUISITION_MODE_LIMITS = 0x9877,
        //DATA
        LT_COMM_ID_AUTO_ECHOES_CHANNEL_INDEX = 0x2700,               ///< (0x2700) {uint16_t} - Index of the echo channel for 3D sensors.
        LT_COMM_ID_AUTO_ECHOES_VALID = 0x2701,                       ///< (0x2701) {uint16_t} - Bitfield with verious information
        LT_COMM_ID_AUTO_ECHOES_AMPLITUDE = 0x2702,                   ///< (0x2702) {int16_t} - Echo amplitude
        LT_COMM_ID_AUTO_ECHOES_DISTANCE = 0x2703,                    ///< (0x2703) {uint16_t} - Echo distance
        LT_COMM_ID_AUTO_ECHOES_TIMESTAMP_UTC = 0x2704,               ///< (0x2704) {uint64_t} - Echo UTC timestamp

        LT_COMM_ID_AUTO_APD_TEMP        = 0x2710,                    ///< (0x2710) {LtFixedPoint} - Temperature of the APD (photodiode)
        LT_COMM_ID_AUTO_APD_TEMP_SCALE  = 0x2711,                    ///< (0x2711) {LtUInt32} - Temperature scale of the APD (photodiode)

        LT_COMM_ID_AUTO_V3M_TEMP = 0x2719,                          ///< (0x2719) {LtFixedPoint} - Temperature of the V3M (photodiode) without scaling
        LT_COMM_ID_AUTO_PMIC_TEMP = 0x2720,                         ///< (0x2720) {LtFixedPoint} - Temperature of the PMIC without scaling
        LT_COMM_ID_AUTO_TIMESTAMP64 = 0x2721,                       ///< (0x2721) {LtUInt64} - TimeStamp with 64 bits resolution for PTP

        // Command parameters
        LT_COM_ID_PARAM_GROUP_CATEGORY = 0x3000,                    ///< (0x3000) {uint32_t} - Parameter for the command LT_COMM_CFGSRV_REQUEST_RESET_CONFIG see eLtResetCategoryGroup
        //*********************************************************************************************************************************************************************************************

        //*********************************************************************************************************************************************************************************************
        LT_COMM_ID_PRODUCT_SPECIFIC_BASE = 0x8000                    ///< (0x8000) Product specific element ids starting base. Ids defined in a separate header file.
                                           //*********************************************************************************************************************************************************************************************
    } eLtCommGenericElementsIdCodes;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommCfgSrvGenericRequestCodes
    ///
    /// \brief  Configuration server generic request codes.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommCfgSrvGenericRequestCodes
    {
        //*********************************************************************************************************************************************************************************************
        LT_COMM_CFGSRV_REQUEST_COMMON_SPECIFIC_BASE = 0x0000,                                           ///< (0x0000) Configuration server start base of common request.
        //*********************************************************************************************************************************************************************************************

        LT_COMM_CFGSRV_REQUEST_INVALID = LT_COMM_CFGSRV_REQUEST_COMMON_SPECIFIC_BASE,                   ///< (0x0000) Invalid request code. Never to be used by platforms or device specific protocols.
        LT_COMM_CFGSRV_REQUEST_LISTING,                                                                 ///< (0x0001) Request listing.
        LT_COMM_CFGSRV_REQUEST_GET,                                                                     ///< (0x0002) Generic get request.
        LT_COMM_CFGSRV_REQUEST_SET,                                                                     ///< (0x0003) Generic set request.
        LT_COMM_CFGSRV_REQUEST_RESET = 0x0005,                                                          ///< (0x0005) System reset request (soft reset).
        LT_COMM_CFGSRV_REQUEST_ECHO,                                                                    ///< (0x0006) Return echo request.
        LT_COMM_CFGSRV_REQUEST_UPDATE,                                                                  ///< (0x0007) Firmware update request.
        LT_COMM_CFGSRV_REQUEST_STATUS,                                                                  ///< (0x0008) Request status

        //*********************************************************************************************************************************************************************************************
        LT_COMM_CFGSRV_REQUEST_PLATFORM_SPECIFIC_BASE = 0x1000,                                         ///< (0x1000) Configuration server start base of platform specific request. Used for platform specific usage (tests, R&D advanced commands, etc.)
        //*********************************************************************************************************************************************************************************************

        LT_COMM_CFGSRV_REQUEST_GET_DEVICE = 0x7000,                                                     ///< (0x7000) Get the device information.
        LT_COMM_CFGSRV_REQUEST_GET_CAL = 0x7002,                                                        ///< (0x7002) Get shadow calibration data.
        LT_COMM_CFGSRV_REQUEST_GET_CONFIG = 0x7006,                                                     ///< (0x7006) Get shadow configuration data.
        LT_COMM_CFGSRV_REQUEST_SET_CONFIG,                                                              ///< (0x7007) Set shadow configuration data.
        LT_COMM_CFGSRV_REQUEST_WRITE_CONFIG,                                                            ///< (0x7008) Write configuration data from shadow structure to non-volatile memory.
        LT_COMM_CFGSRV_REQUEST_RESTORE_CONFIG = 0x7009,                                                 ///< (0x7009) Restore configuration data from non-volatile memory to shadow structure.
        LT_COMM_CFGSRV_REQUEST_RESET_CONFIG = 0x7011                                                    ///< (0x7011) Reset the shadow configuration to default data.

    } eLtCommCfgSrvGenericRequestCodes;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommGenericAnswerCodes
    ///
    /// \brief  Protocol answer codes list.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommGenericAnswerCodes
    {
        //*********************************************************************************************************************************************************************************************
        LT_COMM_ANSWER_COMMON_SPECIFIC_BASE = 0x0000,                               ///< (0x0000) Answer id start base of common request.
        //*********************************************************************************************************************************************************************************************

        LT_COMM_ANSWER_OK = LT_COMM_ANSWER_COMMON_SPECIFIC_BASE,                    ///< (0x0000) Request properly handled.
        LT_COMM_ANSWER_ERROR,                                                       ///< (0x0001) General error.
        LT_COMM_ANSWER_FLASH_ERROR,                                                 ///< (0x0002) Error relate to flash memory read or write.
        LT_COMM_ANSWER_HARDWARE_FAILURE,                                            ///< (0x0003) Error caused by abnormal hardware operation.
        LT_COMM_ANSWER_INVALID_DATA,                                                ///< (0x0004) The request contained invalid data.
        LT_COMM_ANSWER_INVALID_REQUEST,                                             ///< (0x0005) The request is invalid (not defined/supported).
        LT_COMM_ANSWER_PROTOCOL_ERROR,                                              ///< (0x0006) Error in the protocol (invalid error or packet structure). Session end - always set
        LT_COMM_ANSWER_UNSUPPORTED_VERSION,                                         ///< (0x0007) Unsupported protocol version (as specified in the request header).
        LT_COMM_ANSWER_LIMITED_MODE,                                                ///< (0x0008) Device is in a limited mode of operation and cannot service this request.
        LT_COMM_ANSWER_OUTPUT_SIZE_TOO_LONG,                                        ///< (0x0009) Constructed response is larger than the maximum supported packet size.
        LT_COMM_ANSWER_MISSING_ELEMENT,                                             ///< (0x000A) The request is missing a required element.
        LT_COMM_ANSWER_FPGA_ALIGN,                                                  ///< (0x000B) The FPGA is not aligned.
        LT_COMM_ANSWER_FPGA_COMM,                                                   ///< (0x000C) FPGA communication failure.
        LT_COMM_ANSWER_FPGA_PROG_DONE,                                              ///< (0x000D) The FPGA "program done" pin has not asserted.
        LT_COMM_ANSWER_NO_NEW_DATA,                                                 ///< (0x000E) There is no new data to send


        //*********************************************************************************************************************************************************************************************
        LT_COMM_ANSWER_PLATFORM_SPECIFIC = 0x0100,                                  ///< (0x0100) Range (0x0100 <= x < 0xFFFE) reserved for platform specific answer codes.
        //*********************************************************************************************************************************************************************************************

        LT_COMM_ANSWER_MULTIPLE_ERRORS = 0xFFFE,                                    ///< (0xfffe) Multiple errors.
        LT_COMM_ANSWER_INVALID = 0xFFFF                                             ///< (0xffff) Invalid answer.
    } eLtCommGenericAnswerCodes;

    typedef enum eLtCommDataSrvGenericRequestCodes
    {
        /// \brief  Generic data server request codes.
        //*********************************************************************************************************************************************************************************************
        LT_COMM_DATASRV_REQUEST_COMMON_SPECIFIC_BASE = 0x0000,                                           ///< (0x0000) Data server start base of common request.
        //*********************************************************************************************************************************************************************************************

        LT_COMM_DATASRV_REQUEST_INVALID = LT_COMM_DATASRV_REQUEST_COMMON_SPECIFIC_BASE,                  ///< (0x0000) Invalid request code. Never to be used by platforms or device specific protocols.
        LT_COMM_DATASRV_REQUEST_SEND_STATES = 0x0002,                                                    ///< (0x0002) Send sensor states.
        LT_COMM_DATASRV_REQUEST_SEND_ECHOES = 0x0020                                                     ///< (0x0020) Send echoes.
    } eLtCommDataSrvGenericRequestCodes;

    typedef enum eLtCommProtocol
    {
        /// \brief  Protocol used for communication in
        LT_COMM_PROTOCOL_INVALID = 0x00,    ///< Invalid protocol.
        LT_COMM_PROTOCOL_TCP     = 0x01,    ///< TCP.
        LT_COMM_PROTOCOL_UDP     = 0x02     ///< UDP.
    } eLtCommProtocol;

    typedef enum eLtCommThresholdOptions
    {
        /// \brief  Threshold options - Exclusives, only one should be enabled
        LT_COMM_AUTO_THRESHOPT_MANUAL    = 0x01,    ///< Manual threshold offset
        LT_COMM_AUTO_THRESHOPT_AUTOMATIC = 0x02     ///< Automatic sensibility threshold

    } eLtCommThresholdOptions;

#pragma pack(push, 1)
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \struct sLtCommAnswerHeader.
    ///
    /// \brief  Answer header received after each request sent to the sensor.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef struct                          // LT_COMM_ID_ANSWER_HEADER
    {
        uint16_t  mSrvProtVersion;    ///< Protocol version.
        uint16_t  mAnswerCode;        ///< Returned answer code.
        uint32_t  mAnswerSize;        ///< Answer total size in bytes. The size includes this header size.
        uint16_t  mRequestCode;       ///< Protocol request code associated with the answer.
        uint8_t   mReserved0[ 6 ];    ///< Reserved field for padding.
    } sLtCommAnswerHeader;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \struct sLtCommRequestHeader.
    ///
    /// \brief  Request header sent to the sensor for each request.
    ///         Total size: 8 bytes
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef struct                          // LT_COMM_ID_REQUEST_HEADER
    {
        uint16_t  mSrvProtVersion;    ///< Protocol version.
        uint16_t  mRequestCode;       ///< Protocol request code.
        uint32_t  mRequestTotalSize;  ///< Request total size in bytes. The size includes this header size.
    } sLtCommRequestHeader;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \struct sLtCommElementHeader.
    ///
    /// \brief  Header for an element
    ///         Total size: 8 bytes
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef struct
    {
        uint16_t  mElementId;         ///< Element identifier.
        uint16_t  mElementCount;      ///< Number of times the element is sent for building an array. Header is not repeated.
        uint32_t  mElementSize;       ///< Single element size in bytes excluding this header.
    } sLtCommElementHeader;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \struct sLtCommElementRequestInfo.
    ///
    /// \brief  Element structure of a request used for listing command.
    ///         Total size: 8 bytes
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef struct
    {
        uint16_t  mRequestCode;       ///< Request code.
        uint16_t  mElementId;         ///< Element identifier.
        uint16_t  mElementCount;      ///< Number of element is sent for building an array.
        uint8_t   mDirection;         ///< Direction of request.
        uint8_t   mReserved0[ 1 ];    ///< Reserved field for padding.
    } sLtCommElementRequestInfo;

    /** @struct sLtCommElementAlert
    *  @brief  Element structure used for alert.
    */
    typedef struct sLtCommElementAlert
    {
        uint64_t mCode;                                /** Alert code */
        uint64_t mStamp;                               /** Alert time stamp */
        uint8_t  mMsg[LT_COMM_ALERT_MSG_LENGTH];       /** Alert default message */
        uint8_t  mCustomMsg[LT_COMM_ALERT_MSG_LENGTH]; /** Alert custom message */
        uint8_t  mUid;                                 /** Alert unique ID */
        uint8_t  mReserved0[15];                       /** Reserved field for padding. */
    } sLtCommElementAlert;


#pragma pack(pop)

#define LEDDARTECH_ID_COMPUTE(_offset)                   (LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_PLATFORM_SPECIFIC_BASE + (_offset))
#define LEDDARTECH_ID_ECHO_STATE                         LEDDARTECH_ID_COMPUTE(0x0002) // {uint8_t} - 0 = disable; !=0 = enable

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommDeviceType
    ///
    /// \brief  Device types definitions.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommDeviceType
    {
        LT_COMM_DEVICE_TYPE_INVALID             = 0x0000,   ///< Invalid device type.
        LT_COMM_DEVICE_TYPE_DTEC                = 0x0002,   ///< Dtec product based on dtec platform.
        LT_COMM_DEVICE_TYPE_SIDETEC_M           = 0x0004,   ///< Sidetec product based on dtec platform.
        LT_COMM_DEVICE_TYPE_TRACKER             = 0x0005,   ///< Tracker product based on dtec platform.
        LT_COMM_DEVICE_TYPE_VTEC                = 0x0006,   ///< Vtec product based on dtec platform.
        LT_COMM_DEVICE_TYPE_M16_EVALKIT         = 0x0007,   ///< M16 Evalkit platform
        LT_COMM_DEVICE_TYPE_IS16                = 0x0008,   ///< IS16 platform
        LT_COMM_DEVICE_TYPE_M16                 = 0x0009,   ///< M16 platform
        LT_COMM_DEVICE_TYPE_SCH_EVALKIT         = 0x000A,   ///< Single Channel platform
        LT_COMM_DEVICE_TYPE_SCH_LONG_RANGE      = 0x000B,   ///< Single Channel Long Range platform
        LT_COMM_DEVICE_TYPE_VU8                 = 0x000D,   ///< Vu8 platform
        LT_COMM_DEVICE_TYPE_M16_LASER           = 0x000E,   ///< M16 laser
        LT_COMM_DEVICE_TYPE_LCA3_DISCRETE       = 0x000F,   ///< LCA3 Discrete
        LT_COMM_DEVICE_TYPE_TRACKER_TRANS       = 0x0010,   ///< Tracker product with updated algorithm and specific hardcoded tuning
        LT_COMM_DEVICE_TYPE_LCA2_REFDESIGN      = 0x0011,   ///< LCA2 RefDesign
        LT_COMM_DEVICE_TYPE_PIXELL              = 0x0012,   ///< Pixell

        LT_COMM_DEVICE_TYPE_AUTO_FAMILY         = 0x0100,   ///< LeddarAuto product family

        LT_COMM_DEVICE_TYPE_LCA2_DISCRETE       = 0xFFFA   ///< LCA2 Discrete platform
    } eLtCommDeviceType;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \enum   eLtCommDeviceTypeRegAddress
    ///
    /// \brief  Device types register address.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    typedef enum eLtCommDeviceTypeRegAddress
    {
        LT_COMM_DEVICE_TYPE_ADDRESS_OLD = 0x00400140,
        LT_COMM_DEVICE_TYPE_ADDRESS_NEW = 0x00046146
    } eLtCommDeviceTypeRegAddress;

    typedef enum eLtReleaseType
    {
        LT_COMM_RT_INVALID = 0, /** Invalid */
        LT_COMM_RT_DAILY = 0x01, /** Daily / nightly build */
        LT_COMM_RT_INTERNAL = 0x02, /** LeddarTech internal version */
        LT_COMM_RT_BETA = 0x03, /** Beta version */
        LT_COMM_RT_RELEASE = 0x04, /** Release (candidate) */
        //Maximum value         0xFF
    } eLtReleaseType;
#ifndef STRUCT_FWVERSION
#define STRUCT_FWVERSION
#define VERSION_BUILD_NUMBER_LENGTH 14
    /// \struct sFirmwareVersion
    /// \brief Structure which indicated the library software version.
#pragma pack(push,1)
    typedef struct sFirmwareVersion
    {
        uint16_t mMajor;
        uint16_t mMinor;
        uint16_t mRelease;              //** eLtReleaseType */
        uint8_t  mBuild[VERSION_BUILD_NUMBER_LENGTH];
    } sFirmwareVersion;
#pragma pack(pop)
#endif /* STRUCT_FWVERSION */

    // Parameters value for command LT_COMM_CFGSRV_REQUEST_RESET_CONFIG
    typedef enum eLtResetCategoryGroup
    {
        LT_COMM_RESET_FLAG_NONE         = 0x0,
        LT_COMM_RESET_FLAG_DEFAULT      = 0x1,
        LT_COMM_RESET_FLAG_CALIB        = 0x2,
        LT_COMM_RESET_FLAG_PRODUCT_INFO = 0x3,
        LT_COMM_RESET_FLAG_ALL          = 0xFFFFFFFF
    } eLtResetCategoryGroup;
}
