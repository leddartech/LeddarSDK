////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdResultEchoes.h
///
/// \brief  Declares the LdResultEchoes class.
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdIntegerProperty.h"
#include "LdResultProvider.h"
#include "LdDoubleBuffer.h"

#include <cassert>

namespace LeddarUtils
{
    namespace LtMathUtils
    {
        struct LtPointXYZ;
    }
}

namespace LeddarConnection
{
    struct LdEcho
    {
        int32_t  mDistance;     ///< Scaled distance
        uint32_t mAmplitude;    ///< Scaled amplitude
        uint32_t mBase;         ///< [M16 - Internal use] - Amplitude value that correspond the 0 amplitude
        uint16_t mChannelIndex; ///< Channel index
        uint16_t mFlag;         ///< Detection flag
        uint64_t mTimestamp;    ///< Echo timestamp

        LeddarUtils::LtMathUtils::LtPointXYZ ToXYZ( double aHFOV, double aVFOV, uint16_t aHChanNbr, uint16_t aVChanNbr, uint32_t aDistanceScale ) const;
        bool operator==( const LdEcho &aEcho ) const {
            if( std::abs( static_cast<int64_t>( aEcho.mAmplitude ) - mAmplitude ) <= 1 &&
                    std::abs( aEcho.mDistance - mDistance ) <= 1 && aEcho.mBase == mBase && aEcho.mChannelIndex == mChannelIndex && aEcho.mFlag == mFlag )
                return true;
            else
                return false;
        }
    };

    typedef struct EchoBuffer
    {
        EchoBuffer(): mCount( 0 ), mScanDirection( 0 ) {};
        std::vector<LdEcho> mEchoes;
        uint32_t    mCount;
        uint8_t mScanDirection;
    } EchoBuffer;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdResultEchoes.
    ///
    /// \brief  A result provider for the echoes.
    ///
    /// \author Patrick Boulay
    /// \date   March 2016
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdResultEchoes : public LdResultProvider
    {
    public:
        LdResultEchoes( void );
        ~LdResultEchoes();

        void        Init( uint32_t aDistanceScale, uint32_t aAmplitudeScale, uint32_t aMaxDetections );
        bool        IsInitialized( void ) const { return mIsInitialized;  }
        void        Swap();
        void        Lock( eBuffer aBuffer ) {mDoubleBuffer.Lock( aBuffer );}
        void        UnLock( eBuffer aBuffer ) {mDoubleBuffer.UnLock( aBuffer );}
        uint32_t    GetTimestamp( eBuffer aBuffer = B_GET ) const {return mDoubleBuffer.GetTimestamp( aBuffer );}
        void        SetTimestamp( uint32_t aTimestamp ) override { mDoubleBuffer.SetTimestamp( aTimestamp );}
        uint64_t    GetTimestamp64( eBuffer aBuffer = B_GET ) const {return mDoubleBuffer.GetTimestamp64( aBuffer );}
        void        SetTimestamp64( uint64_t aTimestamp ) { mDoubleBuffer.SetTimestamp64( aTimestamp );}
        uint64_t    GetFrameId( eBuffer aBuffer = B_GET ) const {return mDoubleBuffer.GetFrameId( aBuffer );}
        void        SetFrameId( uint64_t aFrameId ) { mDoubleBuffer.SetFrameId( aFrameId );}

        uint32_t            GetEchoCount( eBuffer aBuffer = B_GET ) const;
        std::vector<LdEcho> *GetEchoes( eBuffer aBuffer = B_GET );
        float               GetEchoDistance( size_t aIndex ) const;
        float               GetEchoAmplitude( size_t aIndex ) const;
        LeddarUtils::LtMathUtils::LtPointXYZ GetEchoCoordinates( size_t aIndex ) const;
        float               GetEchoBase( size_t aIndex ) const;
        size_t              GetEchoesSize( void ) const { return static_cast< EchoBuffer * >( mDoubleBuffer.GetConstBuffer( B_GET )->mBuffer )->mEchoes.size(); }
        void                SetEchoCount( uint32_t aValue ) { static_cast< EchoBuffer * >( mDoubleBuffer.GetBuffer( B_SET )->mBuffer )->mCount = aValue; }

        void                SetCurrentLedPower( uint16_t aValue );
        uint16_t            GetCurrentLedPower( eBuffer aBuffer = B_GET ) const;
        uint32_t            GetDistanceScale( void ) const {return mDistanceScale;}
        void                SetDistanceScale( uint32_t aNewScale ) { mDistanceScale = aNewScale; }
        uint32_t            GetAmplitudeScale( void ) const {return mAmplitudeScale;}
        void                SetAmplitudeScale( uint32_t aNewScale )  { mAmplitudeScale = aNewScale;}

        uint8_t             GetScanDirection( eBuffer aBuffer = B_GET ) const { return static_cast< EchoBuffer * >( mDoubleBuffer.GetConstBuffer( aBuffer )->mBuffer )->mScanDirection; }
        void                SetScanDirection( uint8_t aValue ) { static_cast< EchoBuffer * >( mDoubleBuffer.GetBuffer( B_SET )->mBuffer )->mScanDirection = aValue; }


        //Useful for cartesian coordinates
        double              GetVFOV( void ) const { return mVFOV; }
        void                SetVFOV( const double aVFOV ) { mVFOV = aVFOV; }
        double              GetHFOV( void ) const { return mHFOV; }
        void                SetHFOV( const double aHFOV ) { mHFOV = aHFOV; }
        uint16_t            GetHChan( void ) const { return mHChan; }
        void                SetHChan( const uint16_t aHChan ) { mHChan = aHChan; }
        uint16_t            GetVChan( void ) const { return mVChan; }
        void                SetVChan( const uint16_t aVChan ) { mVChan = aVChan; }

#ifdef _DEBUG
        std::string ToString( void );
#endif

    private:
        bool mIsInitialized;
        uint32_t mDistanceScale;
        uint32_t mAmplitudeScale;
        double mHFOV, mVFOV;
        uint16_t mHChan, mVChan;

        LeddarCore::LdIntegerProperty mCurrentLedPower;
        LdDoubleBuffer mDoubleBuffer;
        EchoBuffer mEchoBuffer1, mEchoBuffer2;
    };
}
