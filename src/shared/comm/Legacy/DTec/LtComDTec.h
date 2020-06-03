// *****************************************************************************
/// \file    LtComDTec.h
///
/// \brief   Structures and defines for DTec sensor communication.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "comm/LtComEthernetPublic.h"

namespace LtComDTec
{
    //Default constants
    const uint8_t DTEC_NUMBER_OF_CHANNEL = 16;
    const uint8_t DTEC_MAX_ECHOES_BY_CHANNEL = 6;

    const uint16_t DTEC_IDT_PORT      = 48620;
    const uint16_t DTEC_CONFIG_PORT   = 48620;
    const uint16_t DTEC_DATA_PORT     = 48621;
    const uint16_t DTEC_AUX_DATA_PORT = 48622;

    const uint32_t DTEC_DISTANCE_SCALE     = 65536;
    const uint32_t DTEC_TEMPERATURE_SCALE  = 65536;
    const uint16_t DTEC_RAW_AMP_SCALE      = 256;
    const uint32_t DTEC_FILTERED_AMP_SCALE = 2097152;
    const uint16_t DTEC_NBR_SAMPLE_MAX     = 256;

    const uint16_t SIDETECM_ID_OFFSET = 0x800; ///< Offset for some ids between the side-tec Morpho and normal protocol.
    const uint8_t LT_IPV4_IDT_REQUEST_IP_CONFIG = 0x1; ///< Id to re-init ip config of a sensor

// IP configuration modes
    enum eIPMode
    {
        LT_IPV4_IP_MODE_DYNAMIC = 0, ///< DHCP
        LT_IPV4_IP_MODE_STATIC = 1   ///< Static
    };

    /// \brief IP configuration storage type
    enum eStorage
    {
        LT_IPV4_IP_STORAGE_TEMPORARY = 0, ///< Applicable until next reboot
        LT_IPV4_IP_STORAGE_PERMANENT = 1  ///< Commit to non-volatile memory
    };

    /// \brief  PHY Mode
    enum ePHYMode
    {
        LT_IPV4_PHY_MODE_AUTO_NEGOTIATION = 1 << 0,
        LT_IPV4_PHY_MODE_HALF_DUPLEX_10   = 1 << 1,
        LT_IPV4_PHY_MODE_FULL_DUPLEX_10   = 1 << 2,
        LT_IPV4_PHY_MODE_HALF_DUPLEX_100  = 1 << 3,
        LT_IPV4_PHY_MODE_FULL_DUPLEX_100  = 1 << 4,
    };


    // Configuration and calibration elements
    enum eXTECComIds
    {
        PDTECS_ID_BOOTLOADER_PART_NUMBER              = 0x100E, ///< {char[LT_COMM_PART_NUMBER_LENGTH]} or empty if unknown
        PDTECS_ID_BOOTLOADER_VERSION                  = 0x1013, ///< {LtUInt16} or empty if unknown - Bootloader buid number
        PDTECS_ID_RECEIVER_BRD_VERSION                = 0x1047, ///< {LtUInt8} - see \ref eReceiverBoardVersion : PDTECS_RECEIVER_BRD_...
        // Configuration elements
        PDTECS_ID_CFG_PAN_TILT_POSITION               = 0x1020, ///< {PDTECS_SXYCoordFP} - In degrees
        PDTECS_ID_CFG_ACTIVE_ZONE_MASK                = 0x106A, ///< {LtUInt32} - Bit mask for enabling or disabling detection zones
        PDTECS_ID_CFG_LANE_MODE                       = 0x1022, ///< {LtUInt8} - See PDTECS_LANE_MODE_...
        PDTECS_ID_CFG_ROTATE_IMAGE                    = 0x1023, ///< {LtUInt8} - 0: Non-Reversed; !=0: 180° rotation. Device must be reset for action to take place.
        PDTECS_ID_CFG_VANISHING                       = 0x1025, ///< {PDTECS_SXYCoordU16}
        PDTECS_ID_CFG_STOP_BAR_1                      = 0x1026, ///< {PDTECS_SXYCoordU16}
        PDTECS_ID_CFG_STOP_BAR_2                      = 0x1027, ///< {PDTECS_SXYCoordU16}
        PDTECS_ID_CFG_STOP_BAR_DISTANCE               = 0x1028, ///< {LtFixedPoint}
        PDTECS_ID_CFG_LANE_DIST_MIN                   = 0x1029, ///< {LtFixedPoint}[number of lanes]
        PDTECS_ID_CFG_LANE_DIST_MAX                   = 0x102A, ///< {LtFixedPoint}[number of lanes]
        PDTECS_ID_CFG_LANE_XS                         = 0x102B, ///< {LtFixedPoint[2]}[number of lanes]
        PDTECS_ID_CFG_LANE_LEFT_LIMIT                 = 0x102C, ///< {LtUInt16}[number of lanes]
        PDTECS_ID_CFG_LANE_RIGHT_LIMIT                = 0x102D, ///< {LtUInt16}[number of lanes]
        PDTECS_ID_CFG_PRESENCE_VIRTUAL_CHANNEL        = 0x102E, ///< {LtInt16}[number of lanes] - Virtual channel for presence mode for each zone of detection. (-1) to disable presence mode.
        PDTECS_ID_CFG_EXCLUSION_ZONE                  = 0x102F, ///< {PDTECS_SExclusionZone}[number of channels]
        PDTECS_ID_CFG_GT                              = 0x1030, ///< {LtFixedPoint[4][4]}
        PDTECS_ID_CFG_LOOK_AT                         = 0x1031, ///< {LtFixedPoint[3]}
        PDTECS_ID_CFG_VIDENC_RC                       = 0x103C, ///< {LtUInt8} - See PDTECS_VIDENC_RC_...
        PDTECS_ID_CFG_VIDENC_BIT_RATE                 = 0x103D, ///< {LtUInt32} - Video encoder targeted bit rate in bps
        PDTECS_ID_CFG_VIDENC_RESOLUTION               = 0x103F, ///< {LtUInt8} - Video encoder resolution, see PDTECS_VIDENC_RES...
        PDTECS_ID_CFG_VIDENC_IFRAME_INTERVAL          = 0x1049, ///< {LtUInt8} - 0 = I-Frame at beginning of sequence only; 1 = Only I-Frames; N = N-1 P-Frames between each I-Frame
        PDTECS_ID_CFG_AUTO_EXP_ZONE                   = 0x1040, ///< {LtUInt64} - Auto exposition enabled zones bit field. Image is divided in a 8x8 matrix.
        PDTECS_ID_CFG_AUTO_EXP_BIAS                   = 0x1041, ///< {DTecInt8} - Auto exposition positive or negative bias (from -5 to +5).
        PDTECS_ID_CFG_BRIGHTNESS_V2                   = 0x1045, ///< {DTecInt16} - (-1) To enable the auto-exposure. [0-1000] to force brightness with 0.1% resolution.
        PDTECS_ID_CFG_LANE_LABEL                      = 0x104C, ///< {char[LT_IPV4_LANE_LABEL_LENGTH]}[number of lanes]
        PDTECS_ID_CFG_LANE_ARROW_TYPE                 = 0x104D, ///< {LtUInt8} - Arrow type IDs managed on PC software side
        PDTECS_ID_CFG_LANE_OPTIONS                    = 0x1069, ///< {LtUInt16}[number of zones} - Configuration options bit field for each zone
        PDTECS_ID_CFG_TIME                            = 0x1072, ///< {LtUInt64} - Windows time on a 64-bits unsigned integer for Config App usage.
        PDTECS_ID_CFG_ROLL                            = 0x1074, ///< {LtFixedPoint} - 3D model roll value
        PDTECS_ID_CFG_COUNTER_VIRTUAL_CHANNEL         = 0x1077, ///< {LtInt16}[number of lanes] - Virtual channel for counting mode for each zone of detection. (-1) to disable counting mode.
        PDTECS_ID_CFG_NB_SCANS                        = 0x1078, ///< {LtUInt8} - FPGA parameter: Number of scans. See PDTECS_NB_SCANS_...
        PDTECS_ID_CFG_NB_OVERSAMPLING                 = 0x1079, ///< {LtUInt8} - FPGA parameter: Number of oversampling taps. See PDTECS_NB_OVERSAMPLING_...
        PDTECS_ID_CFG_PARASITE_REJECT                 = 0x107A, ///< {LtUInt8} - FPGA parameter: 0 = disabled; 1 = enabled
        PDTECS_ID_CFG_TRANS_IMP_GAIN                  = 0x107B, ///< {LtUInt8} - FPGA parameter: 0 = disabled; 1 = enabled
        PDTECS_ID_CFG_TRANS_IMP_ATT                   = 0x107C, ///< {LtUInt8} - FPGA parameter: 0 = disabled; 1 = enabled
        PDTECS_ID_CFG_FAILSAFE_MODE                   = 0x107E, ///< {LtUInt8} - Failsafe threshold mode. See PDTECS_FAILSAFE_MODE_...
        PDTECS_ID_CFG_FAILSAFE_THRESHOLDS             = 0x107F, ///< {PDTECS_SFailsafeThresholds} - Failsafe manual mode thresholds.
        PDTECS_ID_CFG_LED_INTENSITY                   = 0x1080, ///< {LtUInt8} - 0 to PDTECS_LED_INTENSITY_MAX mapped to proper FPGA register value.
        PDTECS_ID_CFG_WATCHDOG_TIMEOUT                = 0x1081, ///< {LtUInt16} - Watchdog timeout in seconds: 0=disable
        PDTECS_ID_CFG_XTALK_REMOVAL_STATE             = 0x1082, ///< {LtUInt8} - 0 = Disable  1 = Enable
        PDTECS_ID_CFG_STATIC_NOISE_REMOVAL_STATE      = 0x1083, ///< {LtUInt8} - 0 = Disable  1 = Enable
        PDTECS_ID_CFG_PEAK_CHECK_PULSE_WIDTH_STATE    = 0x1084, ///< {LtUInt8} - 0 = Disable  1 = Enable
        PDTECS_ID_CFG_PEAK_OVERSHOOT_MANAGEMENT_STATE = 0x1085, ///< {LtUInt8} - 0 = Disable  1 = Enable
        PDTECS_ID_CFG_PEAK_DEFAULT_SAT_COMP_STATE     = 0x1086, ///< {LtUInt8} - 0 = Disable  1 = Enable
        PDTECS_ID_CFG_PEAK_XTALK_ECHO_REMOVAL_STATE   = 0x1087, ///< {LtUInt8} - 0 = Disable  1 = Enable
        PDTECS_ID_CFG_PEAK_CALIB_REF_PULSE_STATE      = 0x1088, ///< {LtUInt8} - 0 = Disable  1 = Enable
        PDTECS_ID_CFG_PEAK_COMP_TEMP_STATE            = 0x1089, ///< {LtUInt8} - 0 = Disable  1 = Enable

        // Calibration
        PDTECS_ID_CAL_CHAN_TIMEBASE_DELAY   = 0x1014, ///< {LtFixedPoint}[number of channels]
        PDTECS_ID_CAL_CHAN_AREA             = 0x1015, ///< {PDTECS_SCalChanArea}[number of channels]
        PDTECS_ID_CAL_APD                   = 0x1016, ///< {PDTECS_SCalApd}
        PDTECS_ID_CAL_AMP                   = 0x1017, ///< {LtFixedPoint}[PDTECS_BIAS_COUNT]; In V; DistanceScale
        PDTECS_ID_CAL_IMG                   = 0x1018, ///< {PDTECS_SCalImg}

        //Other
        LT_COMM_ID_IPV4_IP_PHY_MODE         = 0x0191, ///< (0x0191) {LtUInt8} - Negotiation Mode. See LT_IPV4_PHY_MODE...
    };


    /// \brief  Values that represent receiver board versions
    enum eReceiverBoardVersion
    {
        PDTECS_RECEIVER_BRD_24A0009_1       = 0x00,
        PDTECS_RECEIVER_BRD_24A0009_2       = 0x01,
        PDTECS_RECEIVER_BRD_24A0027_1       = 0x02,
        PDTECS_RECEIVER_BRD_MAX             = 0x03,
        PDTECS_RECEIVER_BRD_VERSION_MASK    = 0x0F
    };

    ///<Struct to request ip config reset on DTEC id server
    typedef struct _LtIpv4RequestIpConfig
    {
        LtComLeddarTechPublic::sLtCommRequestHeader mHeader;

        uint8_t         mMode; // See LT_IPV4_IP_MODE_...
        uint8_t         mStorage;  // See LT_IPV4_IP_STORAGE_...
        uint8_t         mPhyMode; // See LT_IPV4_PHY_MODE_...
        uint8_t         mPadding[5];

        /* Destination device serial number, or "Everyone" to reset all devices on
        * the network to DHCP mode. */
        char            mSerialNumber[LT_COMM_SERIAL_NUMBER_LENGTH];

        /* Next fields are ignored if:
        * - LT_IPV4_IP_OPT_STATIC bit is cleared in mIpOptions
        * - mSerialNumber = "Everyone"
        */
        LtComEthernetPublic::LtIpv4IpAddress mIpAddress; // Static IP address
        LtComEthernetPublic::LtIpv4IpAddress mIpGateway; // Static Gateway
        LtComEthernetPublic::LtIpv4IpAddress mIpNetMask; // Static IP net mask

        uint8_t         mReserved[56];
    } LtIpv4RequestIpConfig;

    typedef struct PDTECS_SXYCoordFP
    {
        uint32_t mX; // {LtFixedPoint} In Deg. ; DistanceScale
        uint32_t mY; // {LtFixedPoint} In Deg. ; DistanceScale
    }
    PDTECS_SXYCoordFP;

    typedef struct PDTECS_SCalChanArea
    {
        uint16_t mBottom; // All in pixels relative to full resolution image origin
        uint16_t mLeft  ;
        uint16_t mRight ;
        uint16_t mTop   ;
    } PDTECS_SCalChanArea;

    typedef struct PDTECS_SCalApd
    {
        uint32_t mApdVBiasAt25      ; //{LtFixedPoint} In V   ; DistanceScale
        uint32_t mApdVBiasTempFactor; //{LtFixedPoint} In V/K ; DistanceScale
    } PDTECS_SCalApd;

    typedef struct PDTECS_SCalImg
    {
        uint16_t     mImageStartPixelXAxis;   // In pixels; Horizontal image offset relative to the most left position of the active field of view. Max value = 158.
        uint16_t     mImageStartPixelYAxis;   // In pixels; Vertical image offset relative to the most upper position of the active field of view. Max value = 118.
        uint8_t      mPadding[4]; // For alignement
    } PDTECS_SCalImg;
}
