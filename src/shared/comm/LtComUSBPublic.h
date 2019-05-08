#pragma once

namespace LtComUSBPublic
{
    // Some field lengths
#define LT_USB_DEVICE_UNICODE_NAME_LENGTH  32                                      ///< Device name length in number of wchar.
#define LT_USB_DEVICE_NAME_LENGTH          (LT_USB_DEVICE_UNICODE_NAME_LENGTH*2)   ///< Device name length in number of char (bytes).
#define LT_USB_SERIAL_NUMBER_LENGTH        32                                      ///< Serial number length in bytes.
#define LT_USB_PART_NUMBER_LENGTH          16                                      ///< Part number length in bytes.
#define LT_USB_IDT_STATE_MESSAGE_LENGTH    64                                      ///< Identify server message length in bytes.
#pragma pack(push, 1)

    typedef enum eLtComUsbSetupRequestCmd
    {
        /// \brief  USB setup request commands.
        LT_COM_USB_SETUP_REQ_CMD_IDENTIFY = 32    ///< "Identify server" request.
    } eLtComUsbSetupRequestCmd;

    /// \struct sLtCommAnswerHeader
    /// \brief  LeddarTech protocol answer header.
    // Total size       :  16 bytes
    typedef struct                          // LT_COMM_ID_ANSWER_HEADER
    {
        uint16_t  mSrvProtVersion;    ///< Protocol version.
        uint16_t  mAnswerCode;        ///< Returned answer code.
        uint32_t  mAnswerSize;        ///< Answer total size in bytes. The size includes this header size.
        uint16_t  mRequestCode;       ///< Protocol request code associated with the answer.
        uint8_t   mReserved0[ 6 ];    ///< Reserved field for padding.
    } sLtCommAnswerHeader;

    /// \struct LtComUsbIdtAnswerIdentify
    /// \brief  Identify server answer structure.
    // Total size       :  232 bytes
    typedef struct LtComUsbIdtAnswerIdentify
    {
        sLtCommAnswerHeader mHeader;    ///< Protocol answer header.

        uint16_t mDeviceType;           ///< See PDTECS_IDT_DEVICE_TYPE_...
        uint16_t mFpgaFirmwareVersion;  ///< FPGA firmware version.
        uint16_t mProtocolVersion;      ///< Identify server protocol version.
        uint16_t mSoftwareVersion;      ///< Software version. The two (2) MSB are used for the firmware type currently
        ///< running. See \ref LtComLeddarTechPublic::eLtCommSoftwareType.

        uint32_t mOptions;              ///< Bit field that defines various device options (meaning defined for each platform).
        uint32_t mServerState;          ///< Identify server state. See \ref LtComEthernetPublic::eLtCommIdtServerState.
        uint16_t mBusyProgress;         ///< Number of steps remaining before the current operation completes.
        uint8_t  mReserved[ 2 ];          ///< Reserved field for padding: must be set to zero.
        uint32_t mSoftwareCRC32;        ///< CRC32 of the application firmware.

        char mDeviceName[ LT_USB_DEVICE_NAME_LENGTH ];   ///< Sensor device name.
        char mSerialNumber[ LT_USB_SERIAL_NUMBER_LENGTH ];   ///< Sensor serial number.
        char mStateMessage[ LT_USB_IDT_STATE_MESSAGE_LENGTH ];   ///< State message.
        char mSoftwarePartNumber[ LT_USB_PART_NUMBER_LENGTH ];   ///< Firmware part number.
        char mHardwarePartNumber[ LT_USB_PART_NUMBER_LENGTH ];   ///< Hardware part number.
    } LtComUsbIdtAnswerIdentify;

#pragma pack(pop)
}

