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
        LdBitFieldProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId,
                            uint32_t aDeviceId, uint32_t aUnitSize, const std::string &aDescription = "" );

        static uint8_t MaskToBit( uint64_t aMask );

        uint32_t Value( size_t aIndex = 0 ) const;
        template <typename T> T ValueT( size_t aIndex = 0 ) const;

        bool BitState( size_t aIndex, uint8_t aBitIndex ) const {
            return ( ValueT<uint64_t>( aIndex ) & ( uint64_t( 1 ) << aBitIndex ) ) != 0;
        }

        void SetBit( size_t aIndex, uint8_t aBitIndex ) {
            SetValue( aIndex, ValueT<uint64_t>( aIndex ) | uint64_t( 1 ) << aBitIndex );
        }
        void ResetBit( size_t aIndex, uint8_t aBitIndex ) {
            SetValue( aIndex, ValueT<uint64_t>( aIndex ) & ~( uint64_t( 1 ) << aBitIndex ) );
        }
        void SetValue( size_t aIndex, uint64_t aValue );
        void ForceValue( size_t aIndex, uint64_t aValue );

        virtual std::string GetStringValue( size_t aIndex = 0 ) const override;
        virtual void SetStringValue( size_t aIndex, const std::string &aValue ) override;
        virtual void ForceStringValue( size_t aIndex, const std::string &aValue ) override;

        void SetExclusivityMask( uint64_t aMask ) { mExclusivityMask = aMask; }
        bool ValidateExclusivity( std::bitset<64> aValue );

        uint64_t GetLimit( void ) const {return mLimit;}
        void SetLimit( uint64_t aLimit );

    private:
        LdBitFieldProperty(); //Remove c++99 default constructor

        template <typename T> void SetValueT( size_t aIndex, uint64_t aValue );
        bool mDoNotEmitSignal;
        uint64_t mExclusivityMask; ///< Tells which bits are exclusives
        uint64_t mLimit;    ///< The maximum value of the bitfield

    };

}