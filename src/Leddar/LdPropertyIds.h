// *****************************************************************************
// Module..: Leddar
//
/// \file    LdPropertyIds.h
///
/// \brief   ID list of properties.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************
#pragma once

namespace LeddarCore
{
    namespace LdPropertyIds
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \enum   eLdPropertyIds
        ///
        /// \brief  Internal IDs of all the properties
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        typedef enum eLdPropertyIds
        {
            // Sensor's properties
            ID_DEVICE_TYPE                  = 0x610018,
            ID_CONNECTION_TYPE              = 0x610019,
            ID_DEVICE_NAME                  = 0x000022,
            ID_PART_NUMBER                  = 0x000F03,
            ID_SOFTWARE_PART_NUMBER         = 0x0010FC,
            ID_MANUFACTURER_NAME            = 0x610021,
            ID_SERIAL_NUMBER                = 0x000F00,
            ID_BUILD_DATE                   = 0x610022,
            ID_FIRMWARE_VERSION_STR         = 0x610013, //a sensor can have str, int or both
            ID_FIRMWARE_VERSION_INT         = 0x0010fd, //should be the same as ID_FIRMWARE_VERSION_STR - but kept for retro-compatibility
            ID_BOOTLOADER_VERSION           = 0x610014,
            ID_ASIC_VERSION                 = 0x610015,
            ID_FPGA_VERSION                 = 0x610016,
            ID_GROUP_ID_NUMBER              = 0x610020,
            ID_CARRIER_FIRMWARE_VERSION     = 0x610033,
            ID_CARRIER_FIRMWARE_PART_NUMBER = 0x610037,
            ID_CARRIER_PART_NUMBER          = 0x610034,
            ID_CARRIER_SERIAL_NUMBER        = 0x610035,
            ID_CARRIER_OPTIONS              = 0x610036,
            ID_MAC_ADDRESS                  = 0x610038, //Mac address in text format
            ID_IP_ADDRESS                   = 0x000F01,
            ID_IP_MODE                      = 0x000F04, //DHCP Mode
            ID_DATA_SERVER_PORT             = 0x000098,
            ID_DATA_SERVER_PROTOCOL         = 0x001F01,
            ID_OPTIONS                      = 0x000F02,
            ID_ACQ_OPTIONS                  = 0x0000BC,
            ID_TEMPERATURE_SCALE            = 0x001017,
            ID_CPU_LOAD_SCALE               = 0x500102,
            ID_APD_TEMPERATURE_SCALE        = 0x00101A,
            ID_CRC32                        = 0x0010FE,
            ID_ANGLE_OVR                    = 0x001025,
            ID_TEST_MODE                    = 0x0000DC,
            ID_APD_VBIAS_VOLTAGE_T0         = 0x001141,
            ID_APD_VBIAS_MULTI_FACTOR       = 0x001142,
            ID_APD_VBIAS_T0                 = 0x001143,
            ID_APD_OPTIONS                  = 0x001144,
            ID_MEMS_PHASE                   = 0x001145,
            ID_BUFFER_SIZE_TCP              = 0x001F02,
            ID_BUFFER_SIZE_UDP              = 0x001F03,
            ID_SATURATION_PROC_EN           = 0x001162,
            ID_XTALK_OPTIC_SEG_ENABLE       = 0x001146,
            ID_XTALK_OPTIC_LINE_ENABLE      = 0x001147,
            ID_XTALK_OPTIC_ECH_SEG_ENABLE   = 0x00115A,
            ID_XTALK_OPTIC_ECH_LINE_ENABLE  = 0x00115B,
            ID_APD_TRACK_TEMP_COMP_ENABLE   = 0x001149,
            ID_ACC_DIST_ENABLE              = 0x001150,
            ID_ACC_DIST_POSITION            = 0x001151,
            ID_ACC_DIST_EXP                 = 0x001152,
            ID_LIMIT_ACC_DIST_POS           = 0x000118,
            ID_LIMIT_ACC_DIST_EXP           = 0x000119,
            ID_THRESHOLD_OPTIONS            = 0x00011A,
            ID_AUTOMATIC_THRESHOLD_SENSI    = 0x00011B,
            ID_AUTOMATIC_THRESHOLD_SENSI_LIMITS = 0x00011E,
            ID_THREHSOLD_POS_OFFSET         = 0x610039, //Position of the threshold, different from usual sensitivity / threshold
            ID_LIMIT_THREHSOLD_POS_OFFSET   = 0x00011D,
            ID_TEMP_COMP                    = 0x001148,
            ID_AUTO_HYSTERISIS_WIDTH        = 0x00115C,
            ID_AUTO_HOR_DISTRIBUTION_WIDTH  = 0x00115D,
            ID_AUTO_VER_DISTRIBUTION_WIDTH  = 0x00115E,
            ID_AUTO_CONTROLLER_DELAY        = 0x00115F,
            ID_AUTO_CONTROLLER_STEP         = 0x001160,
            ID_DIGITAL_FILTER_ENABLE        = 0x001161,

            ID_VSEGMENT                     = 0x001021,
            ID_HSEGMENT                     = 0x001020,
            ID_RSEGMENT                     = 0x001023,
            ID_REF_SEG_MASK                 = 0x610026,

            ID_BASE_SAMPLE_DISTANCE         = 0x0000E0,
            ID_DETECTION_LENGTH             = 0x00100F,
            ID_DISTANCE_SCALE               = 0x001003,
            ID_RAW_AMP_SCALE_BITS           = 0xAA0025,
            ID_RAW_AMP_SCALE                = 0x001002,
            ID_FILTERED_AMP_SCALE_BITS      = 0xAA0027,
            ID_FILTERED_AMP_SCALE           = 0x001004,
            ID_NB_SAMPLE_MAX                = 0x00100E, //Max trace length

            ID_ACCUMULATION_EXP             = 0x0000A0,
            ID_ACCUMULATION_LIMITS          = 0x001011,
            ID_OVERSAMPLING_EXP             = 0x0000A1,
            ID_OVERSAMPLING_LIMITS          = 0x001012,
            ID_BASE_POINT_COUNT             = 0x0000A2,
            ID_BASE_POINT_COUNT_LIMITS      = 0x000086,
            ID_SEGMENT_ENABLE               = 0x610008,
            ID_SEGMENT_ENABLE_COM           = 0x000122,
            ID_REF_PULSE_RATE               = 0x610027,
            ID_PRECISION                    = 0x610005, //precision = smoothing
            ID_PRECISION_ENABLE             = 0x610006,
            ID_PRECISION_LIMITS             = 0x000107,
            ID_XTALK_ECHO_REMOVAL_ENABLE    = 0x610011,
            ID_XTALK_REMOVAL_ENABLE         = 0x610010,
            ID_SATURATION_COMP_ENABLE       = 0x610009,
            ID_OVERSHOOT_MNG_ENABLE         = 0x61000B,
            ID_SENSIVITY                    = 0x610007, //Sensitivity = threshold offset
            ID_SENSIVITY_OLD                = 0x0000A3, //Sensitivity = threshold offset - used in M16 sensors
            ID_SENSIVITY_LIMITS             = 0x001018,
            ID_PULSE_RATE                   = 0x610024,
            ID_CHANGE_DELAY                 = 0x0000BD,
            ID_CHANGE_DELAY_LIMITS          = 0x001014,
            ID_GAIN_ENABLE                  = 0x0000A4,
            ID_REFRESH_RATE                 = 0x001008,
            ID_REFRESH_RATE_LIST            = 0x0010FB, // IS16
            ID_TRACE_POINT_STEP             = 0x001009, //Distance between two trace points
            ID_START_TRACE                  = 0x0000E1,
            ID_START_TRACE_LIMITS           = 0x0000E2,
            ID_NUMBER_TRACE_SENT            = 0x00009B,
            ID_TRACE_TYPE                   = 0x00FFFE, //Trace type (M16 only)
            ID_DISTANCE_RESOLUTION          = 0x0000D8,
            ID_ECHO_AMPLITUDE_MAX           = 0x001026,

            ID_LED_INTENSITY                = 0x00002A,
            ID_LED_INTENSITY_LIST           = 0xAA0013,
            ID_LED_PWR_ENABLE               = 0x61000C,
            ID_LED_AUTO_PWR_ENABLE          = 0x61000D,
            ID_LED_AUTO_FRAME_AVG           = 0x61000F,
            ID_LED_AUTO_ECHO_AVG            = 0x61000E,
            ID_LED_AUTO_FRAME_AVG_LIMITS    = 0x00004A,
            ID_LED_AUTO_ECHO_AVG_LIMITS     = 0x00004B,
            ID_LED_USR_PWR_COUNT            = 0xAA0061,
            ID_PWM_LASER                    = 0x0000DF,

            ID_DEMERGING_ENABLE             = 0x610012,
            ID_STATIC_NOISE_REMOVAL_ENABLE  = 0x0000CF,
            ID_STATIC_NOISE_TEMP_ENABLE     = 0x00FFCF, //Static noise temperature removal enable
            ID_STATIC_NOISE_CALIB_TEMP      = 0x00FFCD, //The temperature at which the static noise got calibrated
            ID_STATIC_NOISE_UPDATE_ENABLE   = 0x0000D2, //LeddarOne only - Pulse noise = static noise
            ID_STATIC_NOISE_UPDATE_RATE     = 0x0000D0, //LeddarOne only - Pulse noise = static noise
            ID_STATIC_NOISE_UPDATE_AVERAGE  = 0x0000D1, //LeddarOne only - Pulse noise = static noise
            ID_LEARNED_TRACE_OPTIONS        = 0x000134,
            ID_ALGO_REQUESTS                = 0x001709,
            ID_ALGO_RIPPLE_CLEANER          = 0x61003A,

            ID_THRESH_AGG_AMP               = 0x1153,
            ID_THRESH_VICTIM_DIST           = 0x1154,
            ID_THRESH_ELEC_AGG_AMP          = 0x1155,
            ID_THRESH_ECH_VICTIM_LEFT       = 0x1156,
            ID_THRESH_ECH_VICTIM_RIGHT      = 0x1157,
            ID_THRESH_GAUSS_SENSIB          = 0x1158,
            ID_THRESH_M_PEAK                = 0x1159,

            ID_TIMEBASE_DELAY               = 0x00012A,
            ID_INTENSITY_COMPENSATIONS      = 0x000132,
            ID_REAL_DISTANCE_OFFSET         = 0x001006,
            ID_MAX_ECHOES_PER_CHANNEL       = 0x001024,
            ID_CHANNEL_AREA                 = 0x001027,


            ID_ORIGIN_X                     = 0x0000A5,
            ID_ORIGIN_Y                     = 0x0000A6,
            ID_ORIGIN_Z                     = 0x0000A7,
            ID_YAW                          = 0x0000A8,
            ID_PITCH                        = 0x0000A9,
            ID_ROLL                         = 0x0000AA,
            ID_HFOV                         = 0x200003,
            ID_VFOV                         = 0x200004,

            ID_LICENSE                      = 0x000104,
            ID_LICENSE_INFO                 = 0x000105,
            ID_VOLATILE_LICENSE             = 0x000120,
            ID_VOLATILE_LICENSE_INFO        = 0x000121,

            ID_COM_SERIAL_PORT_BAUDRATE         = 0x0000AB,
            ID_COM_SERIAL_PORT_DATA_BITS        = 0x0000AC,
            ID_COM_SERIAL_PORT_PARITY           = 0x0000AD,
            ID_COM_SERIAL_PORT_STOP_BITS        = 0x0000AE,
            ID_COM_SERIAL_PORT_ADDRESS          = 0x0000AF,
            ID_COM_SERIAL_PORT_FLOW_CONTROL     = 0x00FFFF,
            ID_COM_SERIAL_PORT_BAUDRATE_OPTIONS = 0x0000DB, //Baudrate options mask
            ID_COM_SERIAL_PORT_LOGICAL_PORT     = 0xAA1007,
            ID_COM_SERIAL_PORT_MAX_ECHOES       = 0x0000C9,
            ID_COM_SERIAL_PORT_ECHOES_RES       = 0xAA1009,
            ID_COM_SERIAL_PORT_CURRENT_PORT     = 0xAA100A,
            ID_COM_CAN_PORT_BAUDRATE            = 0x0000B0,
            ID_COM_CAN_PORT_TX_MSG_BASE_ID      = 0x0000B1,
            ID_COM_CAN_PORT_RX_MSG_BASE_ID      = 0x0000B2,
            ID_COM_CAN_PORT_FRAME_FORMAT        = 0x0000B3,
            ID_COM_CAN_PORT_PORT_OPTIONS        = 0x0000D3, //can options mask
            ID_COM_CAN_PORT_MAILBOX_DELAY       = 0x0000D4,
            ID_COM_CAN_PORT_PORT_ACQCYCLE_DELAY = 0x0000D5,
            ID_COM_CAN_PORT_MAX_ECHOES          = 0x0000D6,
            ID_COM_CAN_PORT_OPTIONS_MASK        = 0x0000DA,
            ID_COM_CAN_PORT_LOGICAL_PORT        = 0xAA1015,
            ID_COM_CAN_PORT_ECHOES_RES          = 0xAA1017,

            //IS16
            ID_IS16_ZONE_RISING_DB              = 0x0000BE,
            ID_IS16_ZONE_FALLING_DB             = 0x0000BF,
            ID_IS16_OUTPUT_NPN_PNP              = 0x0000C0,
            ID_IS16_OUTPUT_INVERSION            = 0x0000C1,
            ID_IS16_ZONE_FARS                   = 0x0000C2,
            ID_IS16_ZONE_NEARS                  = 0x0000C3,
            ID_IS16_ZONE_SEGMENT_ENABLES        = 0x0000C4,
            ID_IS16_ZONE_ENABLES                = 0x0000C5,
            ID_IS16_TEACH_MARGIN                = 0x0000C6,
            ID_IS16_ZONE_EDIT_MODE              = 0x0000C7,
            ID_IS16_LOCK_PANEL                  = 0x0000C8,
            ID_IS16_ALGO_TYPE                   = 0x0000CC,
            ID_IS16_LCD_CONTRAST                = 0x0000CD,
            ID_IS16_LCD_BRIGHTNESS              = 0x0000CE,


            // Result State's properties
            ID_RS_CPU_LOAD                      = 0x500100,
            ID_CURRENT_LED_INTENSITY            = 0x500103,
            ID_RS_BACKUP                        = 0x500104, // Calibration backup: 0=invalid backup, 1=factory backup, 2=user backup
            ID_RS_TIMESTAMP                     = 0x001A01,
            ID_RS_SYSTEM_TEMP                   = 0x001A03,
            ID_RS_DISCRETE_OUTPUTS              = 0x001A0F, // Flag to know if there is something in the detection zone (IS16 & evalkit)
            ID_RS_ACQ_CURRENT_PARAMS            = 0x001A10, // Sent with every trace, contains led power and several flags (M16)
            ID_RS_PREDICT_TEMP                  = 0x001A13,
            ID_RS_APD_TEMP                      = 0x001A14,
            ID_RS_APD_GAIN                      = 0x001A15,
            ID_RS_NOISE_LEVEL                   = 0x001A16,
            ID_RS_ADC_RSSI                      = 0x001A17,
            ID_RS_SNR                           = 0x001A18,
            ID_RS_V3M_TEMP                      = 0x001A19,
            ID_RS_TIMESTAMP64                   = 0x001A1A,
            ID_RS_PMIC_TEMP                     = 0x001A20
        } eLdPropertyIds;
    }

}

