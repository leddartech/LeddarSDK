// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensorVu8.h
///
/// \brief   Definition of LeddarVu 8 sensor class.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#if defined(BUILD_VU)

#include "LdSensorVu.h"

namespace LeddarDevice
{
    class LdSensorVu8: virtual public LdSensorVu
    {
    public:
        explicit LdSensorVu8( LeddarConnection::LdConnection *aConnection );
        ~LdSensorVu8( void );
        virtual void   GetConfig( void ) override;
        virtual void   SetConfig( void ) override;
        virtual void   GetConstants( void ) override;
        virtual void   GetStates( void ) override;

    private:
        bool mPredictedTempAvailable;
    };
}

#endif