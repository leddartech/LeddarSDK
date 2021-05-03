////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdProtocolLeddarTechHelper.h
///
/// \brief	Helper class related to Leddartech protocol
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "LdProperty.h"

#include <cstdint>
#include <memory>
#include <mutex>

namespace LeddarDevice
{
    class LdSensor;
}

namespace LeddarCore
{
    class LdPropertiesContainer;
}

namespace LeddarConnection
{
    class LdProtocolLeddarTech;
    class LdConnectionInfo;
    class LdConnection;
    class LdProtocolLeddarTechHelper
    {
      public:
        explicit LdProtocolLeddarTechHelper( LeddarDevice::LdSensor *aSensor, LdProtocolLeddarTech *aProtocolConfig );

        virtual ~LdProtocolLeddarTechHelper();
        const LdConnectionInfo *GetConnectionInfo( void ) const;
        LdConnection *GetInterface( void ) const;
        
        void SetDataMask( uint32_t aDataMask );

        void SendCommand( uint16_t aRequestCode, unsigned int aRetryNbr = 0 );

        void GetCategoryPropertiesFromDevice( LeddarCore::LdProperty::eCategories aCategory, uint16_t aRequestCode );

        void SetCategoryPropertiesOnDevice( LeddarCore::LdProperty::eCategories aCategory, uint16_t aRequestCode );

      private:
        mutable std::mutex mMutex;
        LeddarDevice::LdSensor *mSensor = nullptr;
        LdProtocolLeddarTech *mProtocolConfig = nullptr;
        LeddarCore::LdPropertiesContainer *mProperties = nullptr;
    };

} // namespace LeddarConnection