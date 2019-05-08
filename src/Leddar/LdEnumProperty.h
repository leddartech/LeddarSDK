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
        LdEnumProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aUnitSize, bool aStoreValue = true,
                        const std::string &aDescription = "" );

        size_t EnumSize( void ) const { return mEnumValues.size(); }
        std::string EnumText( size_t aIndex ) const { return mEnumValues[ aIndex ].second; } /// param[in] aIndex : index of the enum (not the property)
        uint64_t EnumValue( size_t aIndex ) const { return mEnumValues[ aIndex ].first; } /// param[in] aIndex : index of the enum (not the property)
        size_t ValueIndex( size_t aIndex = 0 ) const;
        uint32_t Value( size_t aIndex = 0 ) const;
        template <typename T> T ValueT( size_t aIndex = 0 ) const;
        uint64_t DeviceValue( size_t aIndex = 0 ) const;
        uint64_t GetKeyFromValue( const std::string &aValue );

        void SetEnumSize( size_t aSize ) { mEnumValues.clear(); mEnumValues.reserve( aSize ); }
        void AddEnumPair( uint64_t aValue, const std::string &aText );
        void ClearEnum( void );
        void SetValueIndex( size_t aArrayIndex, size_t aEnumIndex );
        void ForceValueIndex( size_t aArrayIndex, size_t aEnumIndex );
        void SetValue( size_t aIndex, uint64_t aValue );
        void ForceValue( size_t aIndex, uint64_t aValue );

        virtual std::string GetStringValue( size_t aIndex = 0 ) const override;
        virtual void SetStringValue( size_t aIndex, const std::string &aValue ) override;
        virtual void ForceStringValue( size_t aIndex, const std::string &aValue ) override;

    private:
        // To prevent implicit conversion of the bool argument aStoreValue by a char array
        LdEnumProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aUnitSize, char[] );

        typedef std::pair<uint64_t, std::string> Pair;
        std::vector<Pair> mEnumValues;
        bool mStoreValue;

        template <typename T> void SetStorageValueT( size_t aIndex, uint64_t aValue );
    };
}