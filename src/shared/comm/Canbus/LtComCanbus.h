////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   shared/comm/Canbus/LtComCanbus.h
///
/// \brief  Defines data common to CANbus protocol
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

namespace LeddarConnection
{
    class LdInterfaceCan;
}

namespace LtComCanBus
{
    const uint8_t CAN_MAX_DETECTIONS = 96;
    const uint8_t CAN_DATA_SIZE = 8;

    typedef struct sCanData
    {
        uint16_t mId;
        union
        {
            uint8_t mRawData[CAN_DATA_SIZE];    /// Full frame data
            struct
            {
                uint8_t mCmd;                   /// First byte: command
                uint8_t mSubCmd;                /// Second byte: subcommand
                uint8_t mArg[6];                /// 6 bytes left: command data
            } Cmd;
        } mFrame;
    } sCanData;

    typedef struct sCanIds
    {
        LeddarConnection::LdInterfaceCan *mInterface;
        uint16_t mConfigId;
        uint16_t mFirstDataId;
    } sCanIds;
}