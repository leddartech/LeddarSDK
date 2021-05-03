////////////////////////////////////////////////////////////////////////////////////////////////////
/// file:   Leddar/LdProtocolLeddartechUSB.h
///
/// summary:    Declares the LdProtocolLeddartechUSB class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#ifdef BUILD_USB

#include "LdProtocolLeddarTech.h"
#include "LdInterfaceUsb.h"

namespace LeddarConnection
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdProtocolLeddartechUSB.
    ///
    /// \brief  Class to specify the LeddarTech protocol for USB communication
    ///
    /// \author Patrick Boulay
    /// \date   February 2017
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdProtocolLeddartechUSB : public LdProtocolLeddarTech
    {
    public:
        enum eEndPoint
        {
            EP_CONFIG = 1,
            EP_DATA = 2
        };

        LdProtocolLeddartechUSB( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface );
        LdProtocolLeddartechUSB( const LdConnectionInfo *aConnectionInfo, LdConnection *aProtocol, eEndPoint aEndPoint );
        ~LdProtocolLeddartechUSB();

        virtual void QueryDeviceInfo( void ) override;
        virtual void ReadAnswer( void ) override;

    protected:
        virtual void Write( uint32_t aSize ) override;
        virtual uint32_t Read( uint32_t aSize ) override;

    private:
        LdInterfaceUsb *mInterfaceUSB;
        uint8_t     mEndPoint;
    };
}

#endif
