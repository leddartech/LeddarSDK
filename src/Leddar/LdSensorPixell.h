////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorPixell.h
///
/// \brief  Declares the LdSensorPixell class for the Pixell sensor
///
/// Copyright (c) 2019 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined( BUILD_ETHERNET ) && defined( BUILD_AUTO )

#include "LdSensorLeddarAuto.h"

namespace LeddarDevice
{
    class LdSensorPixell : virtual public LdSensorLeddarAuto
    {
      public:
        explicit LdSensorPixell( LeddarConnection::LdConnection *aConnection );
        ~LdSensorPixell( void );

        void ConnectDataServer( void ) override;
        void GetConstants( void ) override;
        void UpdateConstants( void ) override;
        void GetStatus( void );
        void Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions aOptions = LeddarDefines::RO_NO_OPTION, uint32_t aSubOptions = 0 ) override;
        void GetCalib( void ) override;

        uint32_t SensorChannelIndexToEchoChannelIndex( uint32_t aSensorChannelIndex );
        uint32_t EchoChannelIndexToSensorChannelIndex( uint32_t aEchoChannelIndex );

      protected:
        void ComputeCartesianCoordinates() override;

      private:
        void InitProperties( void );
    };
} // namespace LeddarDevice

#endif