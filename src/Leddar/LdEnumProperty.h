////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdEnumProperty.cpp
///
/// \brief  Implements the ldEnumProperty class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LdProperty.h"

namespace LeddarCore
{

    /// \brief  Class for properties that have a limited set of pre-defined values (currently only support unsigned integer values)
    class LdEnumProperty : public LdProperty
    {
      public:
        LdEnumProperty( const LdEnumProperty &aProperty );
        LdEnumProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aUnitSize, bool aStoreValue = true,
                        const std::string &aDescription = "" );
        bool IsStoreValue() const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformIsStoreValue();
        }

        size_t EnumSize( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformEnumSize();
        }
        std::string EnumText( size_t aIndex ) const /// param[in] aIndex : index of the enum (not the property)
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformEnumText( aIndex );
        }
        uint64_t EnumValue( size_t aIndex ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformEnumValue( aIndex );
        } /// param[in] aIndex : index of the enum (not the property)

        size_t ValueIndex( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValueIndex( aIndex );
        }
        uint32_t Value( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValue( aIndex );
        }
        template <typename T> T ValueT( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValueT<T>( aIndex );
        }
        uint64_t DeviceValue( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformDeviceValue( aIndex );
        }
        uint64_t GetKeyFromValue( const std::string &aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetKeyFromValue( aValue );
        }
        size_t GetEnumIndexFromValue( uint64_t aEnumValue ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetEnumIndexFromValue( aEnumValue );
        }
        void SetEnumSize( size_t aSize )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetEnumSize( aSize );
        }
        void AddEnumPair( uint64_t aValue, const std::string &aText )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformAddEnumPair( aValue, aText );
        }
        void ClearEnum( void )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformClearEnum();
        }
        void SetValueIndex( size_t aArrayIndex, size_t aEnumIndex )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValueIndex( aArrayIndex, aEnumIndex );
        }
        void ForceValueIndex( size_t aArrayIndex, size_t aEnumIndex )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValueIndex( aArrayIndex, aEnumIndex );
        }
        void SetValue( size_t aIndex, uint64_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValue( aIndex, aValue );
        }
        void ForceValue( size_t aIndex, uint64_t aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValue( aIndex, aValue );
        }

      private:
        LdProperty *PerformClone() override;
        std::string PerformGetStringValue( size_t aIndex ) const override;
        void PerformSetStringValue( size_t aIndex, const std::string &aValue ) override;
        void PerformForceStringValue( size_t aIndex, const std::string &aValue ) override;

        bool PerformIsStoreValue() const { return mStoreValue; }
        size_t PerformEnumSize( void ) const { return mEnumValues.size(); }
        std::string PerformEnumText( size_t aIndex ) const { return mEnumValues[aIndex].second; } /// param[in] aIndex : index of the enum (not the property)
        uint64_t PerformEnumValue( size_t aIndex ) const { return mEnumValues[aIndex].first; }    /// param[in] aIndex : index of the enum (not the property)
        size_t PerformValueIndex( size_t aIndex ) const;
        uint32_t PerformValue( size_t aIndex ) const;
        template <typename T> T PerformValueT( size_t aIndex ) const;
        uint64_t PerformDeviceValue( size_t aIndex ) const;
        uint64_t PerformGetKeyFromValue( const std::string &aValue );
        size_t PerformGetEnumIndexFromValue( uint64_t aEnumValue ) const;
        void PerformSetEnumSize( size_t aSize )
        {
            mEnumValues.clear();
            mEnumValues.reserve( aSize );
        }
        void PerformAddEnumPair( uint64_t aValue, const std::string &aText );
        void PerformClearEnum( void );
        void PerformSetValueIndex( size_t aArrayIndex, size_t aEnumIndex );
        void PerformForceValueIndex( size_t aArrayIndex, size_t aEnumIndex );
        void PerformSetValue( size_t aIndex, uint64_t aValue );
        void PerformForceValue( size_t aIndex, uint64_t aValue );
        void PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue ) override;

        // To prevent implicit conversion of the bool argument aStoreValue by a char array
        LdEnumProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aUnitSize, char[] ) = delete;

        typedef std::pair<uint64_t, std::string> Pair;
        std::vector<Pair> mEnumValues;
        bool mStoreValue;

        template <typename T> void SetStorageValueT( size_t aIndex, uint64_t aValue );
    };
} // namespace LeddarCore