// *****************************************************************************
// Module..: Leddar
//
/// \file    LdResultProvider.h
///
/// \brief   Base class of results.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdObject.h"
#include "LdPropertiesContainer.h"
#include "LdIntegerProperty.h"

namespace LeddarConnection
{
    class LdResultProvider : public LeddarCore::LdObject
    {
    public:
        LdResultProvider( void );
        void UpdateFinished( void ) { EmitSignal( LeddarCore::LdObject::NEW_DATA ); }

        uint32_t        GetTimestamp( void ) const { return static_cast<uint32_t>( mTimestamp->Value() ); }
        virtual void    SetTimestamp( uint32_t aTimestamp ) { mTimestamp->ForceValue( 0, aTimestamp ); }

        LeddarCore::LdPropertiesContainer *GetProperties( void ) { return &mProperties; }

    protected:
        LeddarCore::LdIntegerProperty *mTimestamp;
        LeddarCore::LdPropertiesContainer mProperties;

    private:
        LdResultProvider( const LdResultProvider &aProvider ); //Disable copy constructor
        LdResultProvider &operator=( const LdResultProvider &aProvider );//Disable equal constructor
    };
}