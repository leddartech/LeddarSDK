// *****************************************************************************
// Module..: Leddar
//
/// \file    LdPropertiesContainer.h
///
/// \brief   Containter of properties
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdBitFieldProperty.h"
#include "LdBoolProperty.h"
#include "LdBufferProperty.h"
#include "LdEnumProperty.h"
#include "LdFloatProperty.h"
#include "LdIntegerProperty.h"
#include "LdProperty.h"
#include "LdTextProperty.h"

namespace LeddarCore
{
    class LdPropertiesContainer : public LdObject
    {
      public:
        LdPropertiesContainer();
        ~LdPropertiesContainer();

        void AddProperty( LeddarCore::LdProperty *aProperty, bool aForce = false );
        void AddProperties( LeddarCore::LdPropertiesContainer *aProperties );
        void AddPropertiesFromFile( std::string aFilename, bool aUseOfBothIds = false);

        const LeddarCore::LdProperty *GetProperty( uint32_t aId ) const;
        LeddarCore::LdProperty *GetProperty( uint32_t aId ) { return const_cast<LdProperty *>( const_cast<const LdPropertiesContainer *>( this )->GetProperty( aId ) ); }
        LdBitFieldProperty *GetBitProperty( uint32_t aId );
        const LdBitFieldProperty *GetBitProperty( uint32_t aId ) const;
        LdBoolProperty *GetBoolProperty( uint32_t aId );
        const LdBoolProperty *GetBoolProperty( uint32_t aId ) const;
        LdEnumProperty *GetEnumProperty( uint32_t aId );
        const LdEnumProperty *GetEnumProperty( uint32_t aId ) const;
        LdFloatProperty *GetFloatProperty( uint32_t aId );
        const LdFloatProperty *GetFloatProperty( uint32_t aId ) const;
        LdIntegerProperty *GetIntegerProperty( uint32_t aId );
        const LdIntegerProperty *GetIntegerProperty( uint32_t aId ) const;
        LdTextProperty *GetTextProperty( uint32_t aId );
        const LdTextProperty *GetTextProperty( uint32_t aId ) const;
        LdBufferProperty *GetBufferProperty( uint32_t aId );
        const LdBufferProperty *GetBufferProperty( uint32_t aId ) const;

        const LdProperty *FindProperty( uint32_t aId ) const;
        LdProperty *FindProperty( uint32_t aId ){ return const_cast<LdProperty *>( const_cast<const LdPropertiesContainer *>( this )->FindProperty( aId ) ); }
        LdProperty *FindDeviceProperty( uint32_t aDeviceId );

        std::vector<LdProperty *> FindPropertiesByCategories( LdProperty::eCategories aCategory );
        std::vector<LdProperty *> FindPropertiesByFeature( uint32_t aFeature );
        std::vector<const LdProperty *> FindPropertiesByFeature( uint32_t aFeature ) const;
        bool IsModified( LdProperty::eCategories aCategory ) const;

        const std::map<uint32_t, LeddarCore::LdProperty *> *GetContent( void ) const { return &mProperties; }
        void SetPropertiesOwnership( bool aIsPropertiesOwner ) { mIsPropertiesOwner = aIsPropertiesOwner; }

        virtual void Callback( LdObject *aSender, const SIGNALS aSignal, void * /*aExtraData*/ ) override;

      private:
        bool mIsPropertiesOwner;
        std::map<uint32_t, LeddarCore::LdProperty *> mProperties;
    };
} // namespace LeddarCore
