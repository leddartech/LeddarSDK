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

namespace LeddarConnection
{
    class LdResultProvider : public LeddarCore::LdObject
    {
      public:
        LdResultProvider( void ) = default;
        void UpdateFinished( void ) { EmitSignal( LeddarCore::LdObject::NEW_DATA ); }
        void HandleException( std::exception_ptr aEptr ) { EmitSignal( LeddarCore::LdObject::EXCEPTION, (void *)&aEptr ); }

      private:
        LdResultProvider( const LdResultProvider &aProvider ) = delete;            // Disable copy constructor
        LdResultProvider &operator=( const LdResultProvider &aProvider ) = delete; // Disable equal constructor
    };
} // namespace LeddarConnection