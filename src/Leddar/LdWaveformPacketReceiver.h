////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdWaveformPacketReceiver.h
///
/// \brief	Declares the ld rtp packet class
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LdWaveformPacket.h"
namespace LeddarConnection
{
    class LdWaveformPacketReceiver : public LdWaveformPacket
    {

      public:
        LdWaveformPacketReceiver( const uint8_t *aPacket, size_t aLength )
            : LdWaveformPacket( aPacket, aLength )
        {
            mROI = ntohl( GetHeader()->mROI );
        }
            

        ~LdWaveformPacketReceiver() = default;

      private:
    };
} // namespace LeddarConnection