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

#include "LdIntegerProperty.h"
#include "LdResultProvider.h"

#include <cassert>


namespace LeddarConnection
{
    class LdResultStates : public LdResultProvider
    {
    public:
        LdResultStates( void );
        ~LdResultStates();

        void Init( uint32_t aTemperatureScale, uint32_t aCpuLoadScale );
        bool IsInitialized( void ) const { return mIsInitialized;  }
        std::string ToString( void ) const;

    private:
        bool     mIsInitialized;
    };
}
