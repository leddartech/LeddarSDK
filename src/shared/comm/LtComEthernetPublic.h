#pragma once

#include "comm/LtComLeddarTechPublic.h"

#define LT_ETHERNET_IDT_REQUEST_IDENTIFY        (0x0011)
#define LT_ETHERNET_IDENTIFY_PROT_VERSION       (0x0001)
#define LT_ETHERNET_ANSWER_OK                   (0x0000)

namespace LtComEthernetPublic
{
#define REGMAP_PRODUCT_ID_LENGTH            32
#define REGMAP_PRODUCT_SERIAL_LENGTH        32
#define REGMAP_FIRMWATE_VERSION_LENGTH      32
#define REGMAP_PRODUCT_NAME_LENGTH          32
#define REGMAP_MAC_ADDRESS_LENGTH           6

#pragma pack(push, 1)
    // Total size       :   8 bytes
    typedef struct _sLtAddress
    {
        uint8_t mBytes[ 4 ];
        uint8_t mReserved[ 4 ];   // For alignment
    } sLtIpAddress;

    // Total size       :   8 bytes
    typedef struct _sLtEthernetAddress
    {
        uint8_t mBytes[ 6 ];
        uint8_t mReserved0[ 2 ];
    } sLtEthernetAddress;

    typedef enum eLtCommIdtServerState
    {
        /// \brief  Bit field of identification protocol server states.
        LT_COMM_IDT_SERVER_STATE_BUSY           = 0x0001,   ///< Busy server.
        LT_COMM_IDT_SERVER_STATE_CONNECTED      = 0x0002,   ///< Server connected.
        LT_COMM_IDT_SERVER_STATE_ERROR          = 0x0004,   ///< Server in error.
        LT_COMM_IDT_SERVER_STATE_READY          = 0x0008,   ///< Server ready.
        LT_COMM_IDT_SERVER_STATE_RUNNING        = 0x0010,   ///< Server running.
        LT_COMM_IDT_SERVER_STATE_PROG           = 0x0020,   ///< Sensor device in programming mode.
        LT_COMM_IDT_SERVER_STATE_OVRTEMP        = 0x0040,   ///< Over temperature detected.
        LT_COMM_IDT_SERVER_STATE_CONFIG_ERR     = 0x0080,   ///< Configuration error.
        LT_COMM_IDT_SERVER_STATE_DEVINFO_ERR    = 0x0100    ///< Device info error.
    } eLtCommIdtServerState;

    typedef enum eLtCommIPV4RequestCodes
    {
        LT_COMM_ID_IPV4_ETHERNET_ADDRESS        = 0x0010,   ///< (0x0010) {LtIpv4EthernetAddress} Mac address
        LT_COMM_ID_IPV4_DETECTION_OUTPUTS_V2    = 0x0091,   ///< (0x0091) {LtIpv4DetOutputs}[number of lanes] - Array of output detection statuses for each zone
        LT_COMM_ID_IPV4_IP_ADDRESS              = 0x00B0,   ///< (0x00B0) {LtIpv4IpAddress}
        LT_COMM_ID_IPV4_IP_GATEWAY              = 0x00C0,   ///< (0x00C0) {LtIpv4IpAddress}
        LT_COMM_ID_IPV4_IP_NET_MASK             = 0x00D0,   ///< (0x00D0) {LtIpv4IpAddress}
        LT_COMM_ID_IPV4_SERVER_STATE            = 0x0120,   ///< (0x0120) {LtUInt32} - See IPV4_IDT_SERVER_STATE_...
        LT_COMM_ID_IPV4_BUSY_PROGRESS           = 0x0130,   ///< (0x0130) {LtUInt16} - Progress status bar information
        LT_COMM_ID_IPV4_SERVER_STATE_MSG        = 0x0150,   ///< (0x0150) {char[]} - Server state message
        LT_COMM_ID_IPV4_IP_MODE                 = 0x0190,   ///< (0x0190) {LtUInt8} - DHCP or static IP mode. See LT_IPV4_IP_MODE...
        LT_COMM_ID_IPV4_IP_PHY_MODE             = 0x0191,   ///< (0x0191) {LtUInt8} - Negotiation Mode. See LT_IPV4_IP_PHY_MODE...
        LT_COMM_ID_IPV4_TCP_BUFFER_SIZE         = 0x5001,   ///< (0x5001) {LtUInt16} TCP Buffer size
        LT_COMM_ID_IPV4_UDP_BUFFER_SIZE         = 0x5002   ///< (0x5002) {LtUInt16} UDP Buffer size
    } eLtCommIPV4RequestCodes;



#pragma pack(pop)
#pragma pack(push,4)
    // Total size       :   8 bytes
    typedef struct _LtIpv4EthernetAddress
    {
        uint8_t mBytes[6];
        uint8_t mReserved0[2];
    }
    LtIpv4EthernetAddress;

    // Total size       :   8 bytes
    typedef struct _LtIpv4IpAddress
    {
        uint8_t mBytes[4];
        uint8_t mReserved[4];   // For alignment
    }
    LtIpv4IpAddress;

    //Identification server answer for DTec platform
    typedef struct sLtIdtAnswerIdentifyDtec
    {
        LtComLeddarTechPublic::sLtCommAnswerHeader   mHeader;
        LtIpv4EthernetAddress mEthernetAddress; // 8 bytes (2 reserved)

        LtIpv4IpAddress mIpAddress; // 8 bytes
        LtIpv4IpAddress mIpGateway;
        LtIpv4IpAddress mIpNetMask;

        uint16_t mDeviceType;      // See PDTECS_IDT_DEVICE_TYPE_...
        uint16_t mFirmwareVersion;
        uint16_t mProtocolVersion; // See LT_IPV4_IDENTIFY_PROT_VERSION...
        uint16_t mSoftwareVersion; // See PDTECS_SOFTWARE_TYPE_...

        uint32_t mOptions;        // Bit field that defines various device options (meaning defined for each platform).
        uint32_t mServerState;    // See LT_IPV4_IDT_SERVER_STATE_...
        uint16_t mBusyProgress;   // Number of steps remaining before the current operation completes
        uint8_t  mReserved[2];    // For alignment and provision for adding more info
        // All reserved fields must be set to zero
        uint32_t mSoftwareCRC32;

        char mDeviceName[LT_COMM_DEVICE_NAME_LENGTH];           // (64) - Semi UTF16 encoding
        char mSerialNumber[LT_COMM_SERIAL_NUMBER_LENGTH];       // (32)
        char mStateMessage[LT_COMM_IDT_STATE_MESSAGE_LENGTH];   // (64)
        char mSoftwarePartNumber[LT_COMM_PART_NUMBER_LENGTH];   // (16)
        char mHardwarePartNumber[LT_COMM_PART_NUMBER_LENGTH];   // (16)
    }
    sLtIdtAnswerIdentifyDtec;

    //Identification server answer for LCA2 discrete platform
    typedef struct sLtIdtAnswerIdentifyLCA2Discrete
    {
        LtComLeddarTechPublic::sLtCommAnswerHeader mHeader;             ///< Structure header; 16 bytes

        // Ethernet Info
        uint8_t   mMacAddress[REGMAP_MAC_ADDRESS_LENGTH];               ///< Mac address

        // Device Info
        uint16_t  mDeviceType;                                          ///< Device type.
        char      mPartNumber[REGMAP_PRODUCT_ID_LENGTH];                ///< Hardware part number
        char      mSoftPartNumber[REGMAP_PRODUCT_ID_LENGTH];            ///< Software part number
        char      mSerialNumber[REGMAP_PRODUCT_SERIAL_LENGTH];          ///< Serial number
        char      mFirmwareVersion[REGMAP_FIRMWATE_VERSION_LENGTH];     ///< Firmware version

        // Config Data
        uint8_t   mDeviceName[REGMAP_PRODUCT_NAME_LENGTH];               ///< Device name

        // Sensor state
        uint32_t  mSensorState;                                       ///< Sensor state
        uint16_t  mDataPort;                                          ///< Port number to reach for get Leddar data
    } sLtIdtAnswerIdentifyLCA2Discrete;

    //Identification server answer for Leddar Auto platforms
    typedef struct sLtIdtAnswerIdentifyLCAuto
    {
        LtComLeddarTechPublic::sLtCommAnswerHeader mHeader;                 ///< Structure header; 16 bytes

        // Ethernet Info
        sLtEthernetAddress mMacAddress;                                     ///< Mac address; 8 bytes

        // Device Info
        uint16_t  mDeviceType;                                              ///< Device type.
        char      mPartNumber[LT_COMM_PART_NUMBER_LENGTH];                  ///< Hardware part number
        char      mSoftPartNumber[LT_COMM_PART_NUMBER_LENGTH];              ///< Software part number
        char      mSerialNumber[LT_COMM_SERIAL_NUMBER_LENGTH];              ///< Serial number
        char      mFirmwareVersion[LT_COMM_FIRMWARE_VERSION_LENGTH];        ///< Firmware version

        // Config Data
        uint8_t   mDeviceName[LT_COMM_DEVICE_NAME_LENGTH];                  ///< Device name - UTF8 encoded

        // Sensor state
        uint32_t  mSensorState;                                             ///< Sensor state
        uint16_t  mDataPort;                                                ///< Port number to reach for get Leddar data
    } sLtIdtAnswerIdentifyLCAuto;
#pragma pack(pop)
}

