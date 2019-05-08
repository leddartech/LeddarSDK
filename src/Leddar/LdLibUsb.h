// *****************************************************************************
// Module..: LeddarConnection
//
/// \file    LdLibUsb.h
///
/// \brief   Class definition of ldLibUsb
///
/// \author  David Levy
///
/// \since   January 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_USB

#include "LdInterfaceUsb.h"
#include "LdConnectionInfo.h"

#include <vector>

class libusb_context;
class libusb_device_handle;


namespace LeddarConnection
{

    class LdLibUsb : public LdInterfaceUsb
    {
    public:
        LdLibUsb( const LdConnectionInfoUsb *aConnectionInfo, LdConnection *aInterface = nullptr );
        ~LdLibUsb();

        virtual void Connect( void ) override;
        virtual void Disconnect( void ) override;

        virtual void Read( uint8_t aEndPoint, uint8_t *aData, uint32_t aSize ) override;
        virtual void Write( uint8_t aEndPoint, uint8_t *aData, uint32_t aSize ) override;
        virtual void ControlTransfert( uint8_t aRequestType, uint8_t aRequest, uint8_t *aData, uint32_t aSize, uint16_t aTimeout = 1000 ) override;

        static std::vector<LdConnectionInfo *> GetDeviceList( uint32_t aVendorId = 0, uint32_t aProductId = 0, const std::string &aSerialNumber = "" );

    protected:
        libusb_context *Context( void );
        libusb_device_handle *mHandle;

    private:
        static void VerifyError( int aCode );
        static libusb_context *mContext;

        uint32_t mReadTimeout;
        uint32_t mWriteTimeout;
    };

}
#endif