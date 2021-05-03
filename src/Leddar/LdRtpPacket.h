////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdRtpPacket.h
///
/// \brief	Declares the ld rtp packet class
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace LeddarConnection
{
    class LdRtpPacket
    {
      private:
        struct RTPHeader;

      public:
        LdRtpPacket( const uint8_t *aPacket, size_t aLength );
        LdRtpPacket( const LdRtpPacket & ) = delete;
        LdRtpPacket &operator=( const LdRtpPacket & ) = delete;
        LdRtpPacket( size_t aHeaderSize, size_t aPayloadSize );

        static size_t GetSizeOfFixedHeader() { return sizeof( RTPHeader ); }
        size_t GetHeaderSize() const { return mHeaderSize; }
        const uint8_t *GetPayLoad() const { return mBuffer + GetHeaderSize(); }
        size_t GetPayLoadSize() const { return mPayLoadSize; }
        uint8_t GetPayloadType() const { return GetHeader()->mPayloadType; }
        uint16_t GetSequenceNumber() const { return mSequence; }
        uint32_t GetTimeStamp() const { return mTimestamp; }
        uint8_t GetProtocolVersion() const { return GetHeader()->mVersion; }
        bool IsPadded() const { return GetHeader()->mPadding; }
        uint8_t GetPaddingSize() const { return mBuffer[mSize - 1]; }
        bool IsMarked() const { return GetHeader()->mMarker; }
        bool isExtended() const { return GetHeader()->mExtension; }
        uint16_t GetCsrcCount() const { return GetHeader()->mCsrcCount; }
        const uint32_t *getCsrc() const { return static_cast<const uint32_t *>( &( GetHeader()->mSources[1] ) ); }
        uint32_t getSSRC() const { return mSSRC; }
        const uint8_t *GetPacket() const { return mBuffer; }
        size_t GetPacketSize() const { return mSize; }
        static size_t GetFixedHeaderSize() { return sizeof( RTPHeader ); }
        constexpr static uint8_t RTP_VERSION = 2;
        constexpr static uint8_t GetSupportedProtocolVersion() { return RTP_VERSION; }
        ~LdRtpPacket()
        {
            if( mOwnedBuffer )
            {
                delete[] mBuffer;
            }
        }

      protected:
        uint8_t *mBuffer    = nullptr;
        uint16_t mSequence  = 0;
        uint32_t mTimestamp = 0;
        size_t mPayLoadSize = 0;
        size_t mSize        = 0;
        uint32_t mSSRC      = 0;

        RTPHeader *GetHeader() const { return reinterpret_cast<RTPHeader *>( mBuffer ); }

      private:
#pragma pack( 1 )

        struct RTPHeader
        {
            // BIG_ENDIAN
            /*uint8_t mVersion : 2;
            uint8_t mPadding : 1;
            uint8_t mExtension : 1;
            uint8_t mCsrcCount : 4;

            uint8_t mMarker : 1;
            uint8_t mPayloadType : 7;*/
            // LITTLE_ENDIAN
            uint8_t mCsrcCount : 4;
            uint8_t mExtension : 1;
            uint8_t mPadding : 1;
            uint8_t mVersion : 2;

            uint8_t mPayloadType : 7;
            uint8_t mMarker : 1;

            uint16_t mSequence;
            uint32_t mTimestamp;
            uint32_t mSources[1];
        };
#pragma pack()

        size_t mHeaderSize = 0;
        bool mOwnedBuffer  = false;
    };

} // namespace LeddarConnection
