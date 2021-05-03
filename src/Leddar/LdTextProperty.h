// *****************************************************************************
// Module..: Leddar
//
/// \file    LdTextProperty.h
///
/// \brief   Definition of class LdTextProperty.
///
/// \author  Louis Perreault
///
/// \since   May 2014
//
// Copyright (c) 2014 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdProperty.h"
#include <algorithm>

namespace LeddarCore
{

    /// \brief  Property that contains a text value.
    class LdTextProperty : public LdProperty
    {
      public:
        enum eType
        {
            TYPE_ASCII = 1,
            TYPE_UTF16,
            TYPE_UTF8
        };
        LdTextProperty( const LdTextProperty &aProperty );
        LdTextProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aMaxLength, eType aType = TYPE_ASCII,
                        const std::string &aDescription = "" );

        uint32_t MaxLength( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformMaxLength();
        }

        std::string Value( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValue( aIndex );
        }
        std::wstring WValue( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformWValue( aIndex );
        }
        void SetValue( size_t aIndex, const std::string &aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValue( aIndex, aValue );
        }
        void ForceValue( size_t aIndex, const std::string &aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValue( aIndex, aValue );
        }
        void SetValue( size_t aIndex, const std::wstring &aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValue( aIndex, aValue );
        }
        void ForceValue( size_t aIndex, const std::wstring &aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValue( aIndex, aValue );
        }

        void ForceUppercase( void )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceUppercase();
        }
        eType GetEncoding( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetEncoding();
        }

      private:
        LdProperty *PerformClone() override;
        uint32_t PerformMaxLength( void ) const { return PerformUnitSize(); }

        std::string PerformValue( size_t aIndex ) const;
        std::wstring PerformWValue( size_t aIndex ) const;
        void PerformSetValue( size_t aIndex, const std::string &aValue );
        void PerformForceValue( size_t aIndex, const std::string &aValue );
        void PerformSetValue( size_t aIndex, const std::wstring &aValue );
        void PerformForceValue( size_t aIndex, const std::wstring &aValue );

        void PerformForceUppercase( void ) { mForceUppercase = true; }
        eType PerformGetEncoding( void ) const { return mType; }

        void PerformSetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize ) override;
        void PerformForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize ) override;
        std::string PerformGetStringValue( size_t /*aIndex*/ = 0 ) const override;

        void PerformSetStringValue( size_t aIndex, const std::string &aValue ) override { PerformSetValue( aIndex, aValue ); }
        void PerformForceStringValue( size_t aIndex, const std::string &aValue ) override { PerformForceValue( aIndex, aValue ); }
        void PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue ) override;

        bool mForceUppercase;
        eType mType;
    };
} // namespace LeddarCore
