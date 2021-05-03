// *****************************************************************************
// Module..: Leddar
//
/// \file    LdBitFieldProperty.h
///
/// \brief   Definition of LdBitFieldProperty class.
///          This property make easy to manipulates individual bits.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdProperty.h"
#include <bitset>

namespace LeddarCore
{

    /// \brief  Specialization of LdProperty for a property that is a series a
    ///         individual bits.
    class LdBitFieldProperty : public LdProperty
    {
      public:
        LdBitFieldProperty( const LdBitFieldProperty &aProperty );
        LdBitFieldProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint32_t aDeviceId, uint32_t aUnitSize, const std::string &aDescription = "" );

        static uint8_t MaskToBit( uint64_t aMask );

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

        bool BitState( size_t aIndex, uint8_t aBitIndex ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformBitState( aIndex, aBitIndex );
        }

        void SetBit( size_t aIndex, uint8_t aBitIndex )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetBit( aIndex, aBitIndex );
        }
        void ResetBit( size_t aIndex, uint8_t aBitIndex )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformResetBit( aIndex, aBitIndex );
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

        void SetExclusivityMask( uint64_t aMask )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetExclusivityMask( aMask );
        }
        bool ValidateExclusivity( const std::bitset<64> &aValue ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValidateExclusivity( aValue );
        }

        uint64_t GetLimit( void ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformGetLimit();
        }
        void SetLimit( uint64_t aLimit )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetLimit( aLimit );
        }

      private:
        LdProperty *PerformClone() override;
        std::string PerformGetStringValue( size_t aIndex ) const override;
        void PerformSetStringValue( size_t aIndex, const std::string &aValue ) override;
        void PerformForceStringValue( size_t aIndex, const std::string &aValue ) override;

        uint32_t PerformValue( size_t aIndex = 0 ) const;
        template <typename T> T PerformValueT( size_t aIndex ) const;

        bool PerformBitState( size_t aIndex, uint8_t aBitIndex ) const { return ( PerformValueT<uint64_t>( aIndex ) & ( uint64_t( 1 ) << aBitIndex ) ) != 0; }

        void PerformSetBit( size_t aIndex, uint8_t aBitIndex ) { PerformSetValue( aIndex, PerformValueT<uint64_t>( aIndex ) | uint64_t( 1 ) << aBitIndex ); }
        void PerformResetBit( size_t aIndex, uint8_t aBitIndex ) { PerformSetValue( aIndex, PerformValueT<uint64_t>( aIndex ) & ~( uint64_t( 1 ) << aBitIndex ) ); }
        void PerformSetValue( size_t aIndex, uint64_t aValue );
        void PerformForceValue( size_t aIndex, uint64_t aValue );

        void PerformSetExclusivityMask( uint64_t aMask ) { mExclusivityMask = aMask; }
        bool PerformValidateExclusivity( const std::bitset<64> &aValue ) const;

        uint64_t PerformGetLimit( void ) const { return mLimit; }
        void PerformSetLimit( uint64_t aLimit );

        void PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue ) override;

        LdBitFieldProperty() = delete; // Remove default constructor

        template <typename T> void SetValueT( size_t aIndex, uint64_t aValue );
        bool mDoNotEmitSignal;
        uint64_t mExclusivityMask; ///< Tells which bits are exclusives
        uint64_t mLimit;           ///< The maximum value of the bitfield
    };

} // namespace LeddarCore