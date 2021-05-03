////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdProtocolLeddartechEthernetPixell.h
///
/// \brief	Declares the LdProtocolLeddartechEthernetPixell class
///
/// Copyright (c) 2021 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#if defined( BUILD_ETHERNET )

#include "LdInterfaceEthernet.h"
#include "LdProtocolLeddarTech.h"

namespace LeddarConnection
{

    /// \brief  LeddarTech protocol adapted for Pixell: UDP based protocol, using LeddarTech protocol for headers and element, and RTP protocol for paquet division
    class LdProtocolLeddartechEthernetPixell : public LdProtocolLeddarTech
    {
      public:
        LdProtocolLeddartechEthernetPixell( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        virtual ~LdProtocolLeddartechEthernetPixell( void );

        virtual void Connect( void ) override;
        virtual void Disconnect( void ) override;
        virtual void ReadAnswer( void ) override;

      private:
        virtual uint32_t Read( uint32_t ) override;

        LdInterfaceEthernet *mInterfaceEthernet;
        uint16_t mRTPSequenceNumber = 0;
        uint32_t mRTPTimestamp = 0;
        bool mRTPFrameIsValid = false, mFirstFrame = true;
        std::vector<uint8_t> mPayLoad;
    };
} // namespace LeddarConnection

#endif