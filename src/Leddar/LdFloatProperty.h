// *****************************************************************************
// Module..: Leddar
//
/// \file    LdFloatProperty.cpp
///
/// \brief   Definition of functions for class LdFloatProperty.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdProperty.h"

namespace LeddarCore
{
    /// \brief  Specialization of LdProperty for a property that is floating point
    ///         value.

    class LdFloatProperty : public LdProperty
    {

      public:
        LdFloatProperty( const LdFloatProperty &aProperty );
        LdFloatProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aUnitSize, uint32_t aScale, uint32_t aDecimals,
                         const std::string &aDescription = "" );

        float MinValue( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformMinValue();
        }
        float MaxValue( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformMaxValue();
        }
        uint32_t Decimals( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformDecimals();
        }
        uint32_t Scale( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformScale();
        }
        int32_t RawDeviceValue( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformRawDeviceValue( aIndex );
        }
        float Value( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValue( aIndex );
        }
        float DeviceValue( size_t aIndex = 0 )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformDeviceValue( aIndex );
        }

        void SetDecimals( uint32_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformSetDecimals( aValue );
        }
        uint32_t GetScale( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetScale();
        }
        void SetScale( uint32_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetScale( aValue );
        }
        void SetMaxLimits( void )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetMaxLimits();
        }
        void SetLimits( float aMin, float aMax )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetLimits( aMin, aMax );
        }
        void SetRawLimits( int32_t aMin, int32_t aMax )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetRawLimits( aMin, aMax );
        }
        int32_t RawValue( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformRawValue(aIndex);
        }

        void ForceRawValue( size_t aIndex, int32_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceRawValue( aIndex, aValue );
        }
        void SetValue( size_t aIndex, float aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValue( aIndex, aValue );
        }
        void ForceValue( size_t aIndex, float aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValue( aIndex, aValue );
        }

      private:
        LdProperty *PerformClone() override;
        void PerformSetRawValue( size_t aIndex, int32_t aValue ) override;
        std::string PerformGetStringValue( size_t aIndex ) const override;
        void PerformSetStringValue( size_t aIndex, const std::string &aValue ) override;
        void PerformForceStringValue( size_t aIndex, const std::string &aValue ) override;
        bool PerformSigned( void ) const override { return mScale == 0; }

        float PerformMinValue( void ) const { return mMinValue; }
        float PerformMaxValue( void ) const { return mMaxValue; }
        uint32_t PerformDecimals( void ) const { return mDecimals; }
        uint32_t PerformScale( void ) const { return mScale; }
        int32_t PerformRawDeviceValue( size_t aIndex ) const
        {
            VerifyInitialization();
            return reinterpret_cast<const int32_t *>( BackupStorage() )[aIndex];
        }
        float PerformValue( size_t aIndex ) const;
        float PerformDeviceValue( size_t aIndex ) const;

        void PerformSetDecimals( uint32_t aValue ) { mDecimals = aValue; }
        uint32_t PerformGetScale( void ) const { return mScale; }
        void PerformSetScale( uint32_t aValue ) { mScale = aValue; }
        void PerformSetMaxLimits( void );
        void PerformSetLimits( float aMin, float aMax );
        void PerformSetRawLimits( int32_t aMin, int32_t aMax );
        int32_t PerformRawValue( size_t aIndex = 0 ) const;

        void PerformForceRawValue( size_t aIndex, int32_t aValue );
        void PerformSetValue( size_t aIndex, float aValue );
        void PerformForceValue( size_t aIndex, float aValue );
        void PerformSetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize ) override;
        void PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue ) override;

        float mMinValue, mMaxValue;
        uint32_t mScale; // Scale of 0 means its a float, else it's a fixed point (an integer that must be divided by the scale)
        uint32_t mDecimals;
    };
} // namespace LeddarCore