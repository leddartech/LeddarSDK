////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorM16Laser.h
///
/// \brief  Declares the LdSensorM16Laser class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#include "LdSensorM16.h"

namespace LeddarDevice
{
    class LdSensorM16Laser : virtual public LdSensorM16
    {
    public:
        explicit LdSensorM16Laser( LeddarConnection::LdConnection *aConnection );
        ~LdSensorM16Laser();

        virtual void    GetConstants( void ) override;
        float           GetStartTraceDistance( int aValue = -1 );

    private:
        void InitProperties( void );

    };
}

#endif