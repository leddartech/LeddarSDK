////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdDetectionPacket.h
///
/// \brief	Declares the ld Detection packet class
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace LeddarConnection
{
    class LdDetectionPacket
    {
      private:
        struct DetectionHeader;

      public:
        LdDetectionPacket( const uint8_t *aPacket, size_t aLength )
            : mBuffer( const_cast<uint8_t *>( aPacket ) )
            , mSize( aLength )
            , mOwnedBuffer( false )
        {
            mHeaderSize  = sizeof( DetectionHeader );
            mPayLoadSize = mSize - mHeaderSize;
        };

        LdDetectionPacket( const LdDetectionPacket & ) = delete;

        LdDetectionPacket &operator=( const LdDetectionPacket & ) = delete;

        LdDetectionPacket( size_t aHeaderSize, size_t aPayloadSize )
            : mPayLoadSize( aPayloadSize )
            , mHeaderSize( aHeaderSize )
            , mOwnedBuffer( true )
        {
            mSize   = aHeaderSize + aPayloadSize;
            mBuffer = new uint8_t[mSize];

            memset( mBuffer, 0, mHeaderSize );
        }
        static size_t GetSizeOfFixedHeader() { return sizeof( DetectionHeader ); }

        constexpr static uint8_t GetHeaderVersion() { return 0; }

        size_t GetHeaderSize() const { return mHeaderSize; }

        const uint8_t *GetPayLoad() const { return mBuffer + GetHeaderSize(); }

        size_t GetPayLoadSize() const { return mPayLoadSize; }

        uint8_t GetDetectionQty() const { return GetHeader()->mDetectionQty; }
        uint32_t GetSequenceNumber() const { return GetHeader()->mSequence; }
        bool isExtended() const { return GetHeader()->mExtension; }
        uint8_t GetVersion() const { return GetHeader()->mVersion; }

        uint16_t GetFrameCfgIdx() const { return GetHeader()->mFrameCfg; }
        uint16_t GetConfigNumber() const { return GetHeader()->mConfig; }
        uint8_t GetOpticalTile() const { return GetHeader()->mOpticalTile; }
        uint8_t GetLayer() const { return GetHeader()->mLayer; }

        uint16_t GetSegmentOffset() const { return GetHeader()->mSegmentOffset; }
        uint16_t GetSegmentQty() const { return GetHeader()->mSegmentQty; }

        const uint8_t *GetPacket() const { return mBuffer; }
        size_t GetPacketSize() const { return mSize; }
        static size_t GetFixedHeaderSize() { return sizeof( DetectionHeader ); }

        ~LdDetectionPacket()
        {
            if( mOwnedBuffer )
            {
                delete[] mBuffer;
            }
        }

      protected:
        uint8_t *mBuffer = nullptr;
        DetectionHeader *GetHeader() const { return reinterpret_cast<DetectionHeader *>( mBuffer ); }

      private:
#pragma pack( 1 )

        struct DetectionHeader
        {
            // LITTLE_ENDIAN
            uint32_t mDetectionQty : 8;
            uint32_t mSequence : 20;
            uint32_t mPadding : 1;
            uint32_t mExtension : 1;
            uint32_t mVersion : 2;

            uint32_t mFrameCfg : 9;
            uint32_t mConfig : 9;
            uint32_t mOpticalTile : 7;
            uint32_t mPadding2 : 3;
            uint32_t mLayer : 4;

            uint32_t mSegmentOffset : 15;
            uint32_t mSegmentQty : 15;
            uint32_t mPadding3 : 2;
        };
#pragma pack()
        size_t mPayLoadSize = 0;
        size_t mHeaderSize  = 0;

        size_t mSize      = 0;
        bool mOwnedBuffer = false;
    };

} // namespace LeddarConnection
