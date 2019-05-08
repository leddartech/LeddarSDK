// *****************************************************************************
// Module..: Leddar
//
/// \file    LdBufferProperty.h
///
/// \brief   Definition of class LdBufferProperty - A property type that store raw data in a buffer
///
/// \author  David Levy
///
/// \since   November 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdProperty.h"

namespace LeddarCore
{
    class LdBufferProperty : public LdProperty
    {
    public:
        LdBufferProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId,
                          uint16_t aDeviceId, uint32_t aBufferSize, const std::string &aDescription = "" );

        const uint8_t *Value( size_t aIndex = 0 ) const;
        const uint8_t *DeviceValue( size_t aIndex = 0 ) const;
        size_t Size( void ) const { return Stride(); }

        virtual std::string GetStringValue( size_t aIndex = 0 ) const override;
        virtual void SetStringValue( size_t aIndex, const std::string &aValue ) override;
        virtual void ForceStringValue( size_t aIndex, const std::string &aValue ) override;

        void SetValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize );
        void ForceValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize );
        virtual void SetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize ) override;
        virtual void ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize ) override;

    };
}