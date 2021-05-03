////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdWaveformPacket.h
///
/// \brief	Declares the ld waveform packet class
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
    class LdWaveformPacket
    {
      private:
        struct WaveformHeader;

      public:
        LdWaveformPacket( const uint8_t *aPacket, size_t aLength )
            : mBuffer( const_cast<uint8_t *>( aPacket ) )
            , mSize( aLength )
            , mOwnedBuffer( false )
        {
            mHeaderSize  = sizeof( WaveformHeader );
            mPayLoadSize = mSize - mHeaderSize;
        };

        LdWaveformPacket( const LdWaveformPacket & ) = delete;

        LdWaveformPacket &operator=( const LdWaveformPacket & ) = delete;

        LdWaveformPacket( size_t aHeaderSize, size_t aPayloadSize )
            : mPayLoadSize( aPayloadSize )
            , mHeaderSize( aHeaderSize )
            , mOwnedBuffer( true )
        {
            mSize   = aHeaderSize + aPayloadSize;
            mBuffer = new uint8_t[mSize];

            memset( mBuffer, 0, mHeaderSize );
        }
        static size_t GetSizeOfFixedHeader() { return sizeof( WaveformHeader ); }

        constexpr static uint8_t GetHeaderVersion() { return 0; }
        size_t GetHeaderSize() const { return mHeaderSize; }

        const uint8_t *GetPayLoad() const { return mBuffer + GetHeaderSize(); }

        size_t GetPayLoadSize() const { return mPayLoadSize; }

        uint8_t GetVersion() const { return GetHeader()->mVersion; }

        bool isExtended() const { return GetHeader()->mExtension; }

        uint32_t GetSequenceNumber() const { return GetHeader()->mSequence; }

        uint8_t GetWaveformQty() const { return GetHeader()->mWaveFormQty; }

        uint16_t GetSampleQty() const { return GetHeader()->mSampleQty; }

        uint16_t GetConfigNumber() const { return GetHeader()->mCfgNum; }

        uint16_t GetFrameCfgIdx() const { return GetHeader()->mFrameCfgIdx; }

        uint16_t GetSegmentQty() const { return GetHeader()->mSegmentQty; }

        uint16_t GetROIRelativeOffset() const { return GetHeader()->mROIRelativeOffset; }

        const uint8_t *GetPacket() const { return mBuffer; }
        size_t GetPacketSize() const { return mSize; }
        static size_t GetFixedHeaderSize() { return sizeof( WaveformHeader ); }

        uint32_t GetROI() const { return mROI; }
        ~LdWaveformPacket()
        {
            if( mOwnedBuffer )
            {
                delete[] mBuffer;
            }
        }

      protected:
        uint32_t mROI    = 0;
        uint8_t *mBuffer = nullptr;
        WaveformHeader *GetHeader() const { return reinterpret_cast<WaveformHeader *>( mBuffer ); }

      private:
#pragma pack( 1 )

        struct WaveformHeader
        {
            // LITTLE_ENDIAN

            uint32_t mWaveFormQty : 8;
            uint32_t mSequence : 20;
            uint32_t mPadding : 1;
            uint32_t mExtension : 1;
            uint32_t mVersion : 2;

            uint32_t mFrameCfgIdx : 9;
            uint32_t mCfgNum : 9;
            uint32_t mPadding2 : 4;
            uint32_t mSampleQty : 10;

            uint32_t mROIRelativeOffset : 15;
            uint32_t mSegmentQty : 15;
            uint32_t mPadding3 : 2;

            uint32_t mROI;
        };
#pragma pack()
        size_t mPayLoadSize = 0;
        size_t mHeaderSize  = 0;

        size_t mSize      = 0;
        bool mOwnedBuffer = false;
    };

} // namespace LeddarConnection
