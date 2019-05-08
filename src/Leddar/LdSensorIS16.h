////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorIS16.h
///
/// \brief  Declares the LdSensorIS16 class
///
/// Copyright (c) 2019 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined(BUILD_M16) && defined(BUILD_USB)

#include "LdSensorM16.h"

namespace LeddarDevice
{
    class LdSensorIS16 : virtual public LdSensorM16
    {
    public:
        explicit LdSensorIS16( LeddarConnection::LdConnection *aConnection );
        ~LdSensorIS16() {};

        void GetConstants( void ) override;

        void    UpdateParamsForTargetRefreshRate( float aTargetRefreshRate );

    protected:
        void UpdateConstants( void ) override;

    private:
        void InitProperties( void );

    };
}

#endif