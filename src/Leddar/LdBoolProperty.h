// *****************************************************************************
// Module..: Leddar
//
/// \file    LdBoolProperty.h
///
/// \brief   Definition of class LdBoolProperty.
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
    /// \brief  Specialization of LdProperty for a property that is a boolean value.
    class LdBoolProperty : public LdProperty
    {

    public:
        LdBoolProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, const std::string &aDescription = "" );

        bool Value( size_t aIndex = 0 ) const;
        void SetValue( size_t aIndex, bool aValue );
        void ForceValue( size_t aIndex, bool aValue );

        virtual std::string GetStringValue( size_t aIndex = 0 ) const override;
        virtual void SetStringValue( size_t aIndex, const std::string &aValue ) override;
        virtual void ForceStringValue( size_t aIndex, const std::string &aValue ) override;

    private:
        LdBoolProperty(); //Remove c++99 default constructor
    };
}
