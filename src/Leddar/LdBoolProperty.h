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
        LdBoolProperty( const LdBoolProperty &aProperty );
        LdBoolProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, const std::string &aDescription = "" );

        bool Value( size_t aIndex = 0 ) const
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            return PerformValue( aIndex );
        }
        void SetValue( size_t aIndex, bool aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformSetValue( aIndex, aValue );
        }
        void ForceValue( size_t aIndex, bool aValue )
        {
            std::lock_guard<std::recursive_mutex> lock( mPropertyMutex );
            PerformForceValue( aIndex, aValue );
        }

      private:
        LdProperty *PerformClone() override;
        std::string PerformGetStringValue( size_t aIndex ) const override;
        void PerformSetStringValue( size_t aIndex, const std::string &aValue ) override;
        void PerformForceStringValue( size_t aIndex, const std::string &aValue ) override;

        bool PerformValue( size_t aIndex ) const;
        void PerformSetValue( size_t aIndex, bool aValue );
        void PerformForceValue( size_t aIndex, bool aValue );
        void PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue ) override;

        LdBoolProperty() = delete; // Remove default constructor
    };
} // namespace LeddarCore
