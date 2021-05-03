////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdResultEchoes.h
///
/// \brief  Declares the LdResultEchoes class.
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LdDoubleBuffer.h"
#include "LdIntegerProperty.h"
#include "LdResultProvider.h"

#include <cassert>

namespace LeddarUtils
{
    namespace LtMathUtils
    {
        struct LtPointXYZ;
    }
} // namespace LeddarUtils

namespace LeddarConnection
{
    struct LdEcho
    {
        int32_t mDistance;      ///< Scaled distance
        uint32_t mAmplitude;    ///< Scaled amplitude
        uint32_t mBase;         ///< Amplitude value that correspond the 0 amplitude
        uint16_t mChannelIndex; ///< Channel index
        uint16_t mFlag;         ///< Detection flag
        uint64_t mTimestamp;    ///< Echo timestamp
        float mX, mY, mZ;       ///< Cartesian coordinates

        bool operator==( const LdEcho &aEcho ) const
        {
            if( std::abs( static_cast<int64_t>( aEcho.mAmplitude ) - mAmplitude ) <= 1 && std::abs( aEcho.mDistance - mDistance ) <= 1 && aEcho.mBase == mBase &&
                aEcho.mChannelIndex == mChannelIndex && aEcho.mFlag == mFlag && std::abs( aEcho.mX - mX ) < 0.01 && std::abs( aEcho.mY - mY ) < 0.01 &&
                std::abs( aEcho.mZ - mZ ) < 0.01 )
                return true;
            else
                return false;
        }
    };

    typedef struct EchoBuffer
    {
        std::vector<LdEcho> mEchoes;
        uint32_t mCount           = 0;
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

        void Init( uint32_t aDistanceScale, uint32_t aAmplitudeScale, uint32_t aMaxDetections );
        bool IsInitialized( void ) const { return mIsInitialized; }
        void Swap();
        std::unique_lock<std::mutex> GetUniqueLock( eBuffer aBuffer, bool aDefer = false ) const { return mDoubleBuffer.GetUniqueLock( aBuffer, aDefer ); }

        std::vector<LdEcho> *GetEchoes( eBuffer aBuffer = B_GET );
        float GetEchoDistance( size_t aIndex ) const;
        float GetEchoAmplitude( size_t aIndex ) const;
        float GetEchoBase( size_t aIndex ) const;
        void SetEchoCount( uint32_t aValue ) { mDoubleBuffer.GetBuffer( B_SET )->Buffer()->mCount = aValue; }
        uint32_t GetEchoCount( eBuffer aBuffer = B_GET ) const;
        uint32_t GetDistanceScale( void ) const { return mDistanceScale; }
        void SetDistanceScale( uint32_t aNewScale ) { mDistanceScale = aNewScale; }
        uint32_t GetAmplitudeScale( void ) const { return mAmplitudeScale; }
        void SetAmplitudeScale( uint32_t aNewScale ) { mAmplitudeScale = aNewScale; }
        uint32_t GetTimestamp( eBuffer aBuffer = B_GET ) const;
        void SetTimestamp( uint32_t aTimestamp );
        const LeddarCore::LdPropertiesContainer *GetProperties() const { return mDoubleBuffer.GetProperties(); }
        void SetPropertyRawStorage(uint32_t aId, uint8_t *aBuffer, size_t aCount, uint32_t aSize) {mDoubleBuffer.ForceRawStorage(aId, aBuffer, aCount, aSize);}
        void SetPropertyValue( uint32_t aId, uint32_t aIndex, boost::any aValue ) {mDoubleBuffer.SetPropertyValue(aId, aIndex, aValue);}
        void AddProperty( LeddarCore::LdProperty *aProperty ) {mDoubleBuffer.AddProperty(aProperty);}
        void SetPropertyCount( uint32_t aId, size_t aCount ) { mDoubleBuffer.SetPropertyCount( aId, aCount ); }
        // Useful for cartesian coordinates
        double GetVFOV( void ) const { return mVFOV; }
        void SetVFOV( const double aVFOV ) { mVFOV = aVFOV; }
        double GetHFOV( void ) const { return mHFOV; }
        void SetHFOV( const double aHFOV ) { mHFOV = aHFOV; }
        uint16_t GetHChan( void ) const { return mHChan; }
        void SetHChan( const uint16_t aHChan ) { mHChan = aHChan; }
        uint16_t GetVChan( void ) const { return mVChan; }
        void SetVChan( const uint16_t aVChan ) { mVChan = aVChan; }

#ifdef _DEBUG
        std::string ToString( void );
#endif

      private:
        bool mIsInitialized;
        uint32_t mDistanceScale;
        uint32_t mAmplitudeScale;
        double mHFOV, mVFOV;
        uint16_t mHChan, mVChan;

        LdDoubleBuffer<EchoBuffer> mDoubleBuffer;
    };
} // namespace LeddarConnection
