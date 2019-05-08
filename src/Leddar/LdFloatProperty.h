// *****************************************************************************
// Module..: Leddar
//
/// \file    LdFloatProperty.cpp
///
/// \brief   Definition of functions for class LdFloatProperty.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdProperty.h"

namespace LeddarCore
{
    /// \brief  Specialization of LdProperty for a property that is floating point
    ///         value.

    class LdFloatProperty : public LdProperty
    {

    public:
        LdFloatProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId,
                         uint16_t aDeviceId, uint32_t aUnitSize,
                         uint32_t aScale, uint32_t aDecimals, const std::string &aDescription = "" );

        float MinValue( void ) const { return mMinValue; }
        float MaxValue( void ) const { return mMaxValue; }
        uint32_t Decimals( void ) const { return mDecimals; }
        uint32_t Scale( void ) const { return mScale; }
        int32_t  RawDeviceValue( size_t aIndex = 0 ) const { VerifyInitialization();  return reinterpret_cast<const int32_t *>( BackupStorage() )[aIndex]; }
        float Value( size_t aIndex = 0 ) const;
        float DeviceValue( size_t aIndex = 0 ) const;

        void SetDecimals( uint32_t aValue ) { mDecimals = aValue; }
        uint32_t GetScale( void ) const { return mScale; }
        void SetScale( uint32_t aValue ) { mScale = aValue; }
        void SetMaxLimits( void );
        void SetLimits( float aMin, float aMax );
        void SetRawLimits( int32_t aMin, int32_t aMax );
        int32_t RawValue( size_t aIndex = 0 ) const { return reinterpret_cast<const int32_t *>( CStorage() )[aIndex]; }
        virtual void SetRawValue( size_t aIndex, int32_t aValue ) override;
        void ForceRawValue( size_t aIndex, int32_t aValue );
        void SetValue( size_t aIndex, float aValue );
        void ForceValue( size_t aIndex, float aValue );

        virtual bool Signed( void ) const override { return mScale == 0; }

        virtual std::string GetStringValue( size_t aIndex = 0 ) const override;
        virtual void SetStringValue( size_t aIndex, const std::string &aValue ) override;
        virtual void ForceStringValue( size_t aIndex, const std::string &aValue ) override;

    private:
        float mMinValue, mMaxValue;
        uint32_t mScale; //Scale of 0 means its a float, else it's a fixed point (an integer that must be divided by the scale)
        uint32_t mDecimals;
    };
}