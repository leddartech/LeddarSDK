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

        LdTextProperty( LdProperty::eCategories aCategory, uint32_t aFeatures,
                        uint32_t aId, uint16_t aDeviceId,
                        uint32_t aMaxLength, eType aType = TYPE_ASCII, const std::string &aDescription = "" );

        uint32_t MaxLength( void ) const { return UnitSize(); }

        std::string Value( size_t aIndex = 0 ) const;
        std::wstring WValue( size_t aIndex = 0 ) const;
        void SetValue( size_t aIndex, const std::string &aValue );
        void ForceValue( size_t aIndex, const std::string &aValue );
        void SetValue( size_t aIndex, const std::wstring &aValue );
        void ForceValue( size_t aIndex, const std::wstring &aValue );

        virtual void SetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize ) override;
        virtual void ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize ) override;
        virtual std::string GetStringValue( size_t /*aIndex*/ = 0 ) const override;

        virtual void SetStringValue( size_t aIndex, const std::string &aValue ) override {
            SetValue( aIndex, aValue );
        }

        virtual void ForceStringValue( size_t aIndex, const std::string &aValue ) override {ForceValue( aIndex, aValue );}

        void ForceUppercase( void ) { mForceUppercase = true; }
        eType GetEncoding( void ) const { return mType; }

    private:
        bool        mForceUppercase;
        eType       mType;
    };
}
