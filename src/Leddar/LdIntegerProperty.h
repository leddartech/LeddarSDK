// *****************************************************************************
// Module..: Leddar
//
/// \file    LdIntegerProperty.cpp
///
/// \brief   Definition of functions for class LdIntegerProperty.
///
/// \author  Patrick Boulay
///
/// \since   February 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdProperty.h"

namespace LeddarCore
{
    class LdIntegerProperty : public LdProperty
    {
      public:
        using LdProperty::ForceStringValue;
        using LdProperty::SetStringValue;
        LdIntegerProperty( const LdIntegerProperty &aIntProperty );

        LdIntegerProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aUnitSize, const std::string &aDescription = "",
                           const bool aSigned = false );

        int64_t MinValue( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformMinValue();
        }
        template <typename T> T MinValueT( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformMinValueT<T>();
        }
        int64_t MaxValue( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformMaxValue();
        }
        template <typename T> T MaxValueT( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformMaxValueT<T>();
        }
        int64_t Value( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValue( aIndex );
        }
        template <typename T> T ValueT( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValueT<T>( aIndex );
        }

        void SetLimits( int64_t aMin, int64_t aMax )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetLimits( aMin, aMax );
        }
        void SetLimitsUnsigned( uint64_t aMin, uint64_t aMax )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetLimitsUnsigned( aMin, aMax );
        }
        void SetValue( size_t aIndex, int64_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValue( aIndex, aValue );
        }
        void ForceValue( size_t aIndex, int64_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValue( aIndex, aValue );
        }
        void SetValueUnsigned( size_t aIndex, uint64_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValueUnsigned( aIndex, aValue );
        }
        void ForceValueUnsigned( size_t aIndex, uint64_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValueUnsigned( aIndex, aValue );
        }

        void SetStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetStringValue( aIndex, aValue, aBase );
        }
        void ForceStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceStringValue( aIndex, aValue, aBase );
        }

      private:
        LdProperty *PerformClone() override;
        std::string PerformGetStringValue( size_t aIndex = 0 ) const override;
        void PerformSetStringValue( size_t aIndex, const std::string &aValue ) override;
        void PerformForceStringValue( size_t aIndex, const std::string &aValue ) override { PerformForceStringValue( aIndex, aValue, 10 ); };
        bool PerformSigned( void ) const override { return mSigned; }

        int64_t PerformMinValue( void ) const;
        template <typename T> T PerformMinValueT( void ) const;
        int64_t PerformMaxValue( void ) const;
        template <typename T> T PerformMaxValueT( void ) const;
        int64_t PerformValue( size_t aIndex = 0 ) const;
        template <typename T> T PerformValueT( size_t aIndex = 0 ) const;

        void PerformSetLimits( int64_t aMin, int64_t aMax );
        void PerformSetLimitsUnsigned( uint64_t aMin, uint64_t aMax );
        void PerformSetValue( size_t aIndex, int64_t aValue );
        void PerformForceValue( size_t aIndex, int64_t aValue );
        void PerformSetValueUnsigned( size_t aIndex, uint64_t aValue );
        void PerformForceValueUnsigned( size_t aIndex, uint64_t aValue );
        void PerformSetStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase );
        void PerformForceStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase );
        void PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue ) override;

        LdIntegerProperty() = delete; // Remove default constructor

        int64_t mMinValueS, mMaxValueS;
        uint64_t mMinValueU, mMaxValueU;
        bool mSigned; // Should not be changed after initialisation in the constructor

        template <typename T> void SetValueT( size_t aIndex, T aValue );
    };
} // namespace LeddarCore