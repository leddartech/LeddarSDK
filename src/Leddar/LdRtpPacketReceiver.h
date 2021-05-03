////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdRtpPacketReceiver.h
///
/// \brief	Declares the ld rtp packet class
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LdRtpPacket.h"
namespace LeddarConnection
{
    class LdRtpPacketReceiver : public LdRtpPacket
    {

      public:
        LdRtpPacketReceiver( const uint8_t *aPacket, size_t aLength )
            : LdRtpPacket( aPacket, aLength )
        {
            if( GetProtocolVersion() != GetSupportedProtocolVersion() )
            {
                throw std::runtime_error( "RTP header: Unexpected protocol version" );
            }
            if( GetPayLoadSize() == 0 )
            {
                throw std::runtime_error( "RTP header: Payload is empty" );
            }
            mSequence  = ntohs( GetHeader()->mSequence );
            mTimestamp = ntohl( GetHeader()->mTimestamp );
            mSSRC      = ntohl( GetHeader()->mSources[0] );
        };

        ~LdRtpPacketReceiver() = default;

      private:
    };
} // namespace LeddarConnection