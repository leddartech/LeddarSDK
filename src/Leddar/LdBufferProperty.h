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
        LdBufferProperty( const LdBufferProperty &aProperty );
        LdBufferProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aBufferSize, const std::string &aDescription = "" );

        size_t Size( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformSize();
        }

        std::vector<uint8_t> GetValue( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetValue( aIndex );
        };
        std::vector<uint8_t> GetDeviceValue( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetDeviceValue( aIndex );
        };
        void SetValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValue( aIndex, aBuffer, aBufferSize );
        }
        void SetValue( const size_t aIndex, const std::vector<uint8_t> &aBuffer )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValue( aIndex, aBuffer );
        }
        void ForceValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValue( aIndex, aBuffer, aBufferSize );
        }

        void SetRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetRawStorageOffset( aBuffer, aOffset, aSize );
        }
        void ForceRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceRawStorageOffset( aBuffer, aOffset, aSize );
        }

      private:
        LdProperty *PerformClone() override;
        std::string PerformGetStringValue( size_t aIndex ) const override;
        void PerformSetStringValue( size_t aIndex, const std::string &aValue ) override;
        void PerformForceStringValue( size_t aIndex, const std::string &aValue ) override;
        void PerformSetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize ) override;
        void PerformForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize ) override;

        size_t PerformSize( void ) const { return PerformStride(); }
        std::vector<uint8_t> PerformGetValue( size_t aIndex ) const;
        std::vector<uint8_t> PerformGetDeviceValue( size_t aIndex ) const;
        void PerformSetValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize );
        void PerformSetValue( const size_t aIndex, const std::vector<uint8_t> &aBuffer );
        void PerformForceValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize );
        void PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue ) override;

        virtual void PerformSetRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize );
        virtual void PerformForceRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize );

        const uint8_t *Value( size_t aIndex = 0 ) const;
        const uint8_t *DeviceValue( size_t aIndex = 0 ) const;
        void Resize( uint32_t aNewSize );
    };
} // namespace LeddarCore