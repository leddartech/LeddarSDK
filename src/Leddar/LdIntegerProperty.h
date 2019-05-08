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
        LdIntegerProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId,
                           uint16_t aDeviceId, uint32_t aUnitSize, const std::string &aDescription = "", const bool aSigned = false );

        int64_t MinValue( void ) const;
        template <typename T> T MinValueT( void ) const;
        int64_t MaxValue( void ) const;
        template <typename T> T MaxValueT( void ) const;
        int64_t Value( size_t aIndex = 0 ) const;
        template <typename T> T ValueT( size_t aIndex = 0 ) const;

        void SetLimits( int64_t aMin, int64_t aMax );
        void SetLimitsUnsigned( uint64_t aMin, uint64_t aMax );
        void SetValue( size_t aIndex, int64_t aValue );
        void ForceValue( size_t aIndex, int64_t aValue );
        void SetValueUnsigned( size_t aIndex, uint64_t aValue );
        void ForceValueUnsigned( size_t aIndex, uint64_t aValue );
        virtual bool Signed( void ) const override { return mSigned; }

        virtual std::string GetStringValue( size_t aIndex = 0 ) const override;
        virtual void SetStringValue( size_t aIndex, const std::string &aValue ) override;
        virtual void ForceStringValue( size_t aIndex, const std::string &aValue ) override {ForceStringValue( aIndex, aValue, 10 );};
        void SetStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase );
        void ForceStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase );

    private:
        LdIntegerProperty();//Remove c++99 default constructor

        int64_t mMinValueS, mMaxValueS;
        uint64_t mMinValueU, mMaxValueU;
        const bool mSigned; //Should not be changed after initialisation in the constructor

        template <typename T> void SetValueT( size_t aIndex, T aValue );
    };
}