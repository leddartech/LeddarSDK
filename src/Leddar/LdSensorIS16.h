////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorIS16.h
///
/// \brief  Declares the LdSensorIS16 class
///
/// Copyright (c) 2019 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined( BUILD_M16 ) && defined( BUILD_USB )

#include "LdSensorM16.h"

namespace LeddarDevice
{
    class LdSensorIS16 : virtual public LdSensorM16
    {
      public:
        explicit LdSensorIS16( LeddarConnection::LdConnection *aConnection );
        ~LdSensorIS16(){};

        void GetConstants( void ) override;
        void GetConfig() override;

        void UpdateParamsForTargetRefreshRate( uint32_t aTargetRefreshRate );
        void UpdateRefreshRateForTargetAccOvers();
        void SetDefaultQuickLimits( uint8_t aZone );
        void Teach(uint8_t aZone, uint16_t aDuration);

      protected:
        void UpdateConstants( void ) override;

      private:
        void InitProperties( void );
    };
} // namespace LeddarDevice

#endif