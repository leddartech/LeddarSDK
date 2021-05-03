////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdDetectionPacketReceiver.h
///
/// \brief	Declares the ld rtp packet class
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LdDetectionPacket.h"
namespace LeddarConnection
{
    class LdDetectionPacketReceiver : public LdDetectionPacket
    {
      public:
        LdDetectionPacketReceiver( const uint8_t *aPacket, size_t aLength )
            : LdDetectionPacket( aPacket, aLength )
        {
           
        }

        ~LdDetectionPacketReceiver() = default;
    };
} // namespace LeddarConnection