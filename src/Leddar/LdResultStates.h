// *****************************************************************************
// Module..: Leddar
//
/// \file    LdResultStates.h
///
/// \brief   Echoes result.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdResultProvider.h"

#include "LdPropertiesContainer.h"

namespace LeddarConnection
{
    class LdResultStates : public LdResultProvider
    {
    public:
        LdResultStates( void );
        ~LdResultStates() = default;

        void Init( uint32_t aTemperatureScale, uint32_t aCpuLoadScale );
        bool IsInitialized( void ) const { return mIsInitialized;  }
        std::string ToString( void ) const;

        uint32_t GetTimestamp( void ) const;
        virtual void SetTimestamp( uint32_t aTimestamp );

        LeddarCore::LdPropertiesContainer *GetProperties( void ) { return &mProperties; }

    private:
        bool     mIsInitialized;
        LeddarCore::LdPropertiesContainer mProperties;
    };
}
