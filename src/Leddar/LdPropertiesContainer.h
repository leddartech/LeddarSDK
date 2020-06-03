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
#include "LdEnumProperty.h"
#include "LdFloatProperty.h"
#include "LdIntegerProperty.h"
#include "LdTextProperty.h"
#include "LdBufferProperty.h"
#include "LdProperty.h"


namespace LeddarCore
{
    class LdPropertiesContainer : public LdObject
    {
    public:
        LdPropertiesContainer();
        ~LdPropertiesContainer();

        void AddProperty( LeddarCore::LdProperty *aProperty );
        void AddProperties( LeddarCore::LdPropertiesContainer *aProperties );
        void AddPropertiesFromFile( std::string aFilename );
        LeddarCore::LdProperty *GetProperty( uint32_t aId );

        LdBitFieldProperty *GetBitProperty( uint32_t aId );
        LdBoolProperty     *GetBoolProperty( uint32_t aId );
        LdEnumProperty     *GetEnumProperty( uint32_t aId );
        LdFloatProperty    *GetFloatProperty( uint32_t aId );
        LdIntegerProperty  *GetIntegerProperty( uint32_t aId );
        LdTextProperty     *GetTextProperty( uint32_t aId );
        LdBufferProperty   *GetBufferProperty( uint32_t aId );

        LdProperty         *FindProperty( uint32_t aId );
        LdProperty         *FindDeviceProperty( uint32_t aDeviceId );

        std::vector<LdProperty *> FindPropertiesByCategories( LdProperty::eCategories aCategory );
        std::vector<LdProperty *> FindPropertiesByFeature( uint32_t aFeature );
        bool                      IsModified( LdProperty::eCategories aCategory );

        const std::map< uint32_t, LeddarCore::LdProperty *> *GetContent( void ) const { return  &mProperties; }
        void SetPropertiesOwnership( bool aIsPropertiesOwner ) { mIsPropertiesOwner = aIsPropertiesOwner; }

        virtual void Callback( LdObject *aSender, const SIGNALS aSignal, void * /*aExtraData*/ ) override;
    private:
        bool mIsPropertiesOwner;
        std::map< uint32_t, LeddarCore::LdProperty *> mProperties;
    };
}
