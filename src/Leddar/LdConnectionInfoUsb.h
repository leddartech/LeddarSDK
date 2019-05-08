// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionInfoUsb.h
///
/// \brief   Connection information on Usb devices.
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

#include "LdConnectionInfo.h"
#include "LtStringUtils.h"
#include "comm/LtComUSBPublic.h"
#include "LtDefines.h"

namespace LeddarConnection
{
    class LdConnectionInfoUsb : public LdConnectionInfo
    {
    public:
        LdConnectionInfoUsb( eConnectionType aConnectionType, const std::string &aDisplayName, uint16_t aVendorID,
                             uint16_t aProductID, uint8_t aBusNumber, uint8_t aDeviceAddress, const std::string &aSerialNumber,
                             const LtComUSBPublic::LtComUsbIdtAnswerIdentify &aInfos, bool aAlreadyopen ):
            LdConnectionInfo( aConnectionType, aDisplayName ),
            mVendorID( aVendorID ),
            mProductID( aProductID ),
            mBusNumber( aBusNumber ),
            mDeviceAddress( aDeviceAddress ),
            mSerialNumber( aSerialNumber ),
            mAlreadyOpen( aAlreadyopen ),
            mInfos( aInfos ) 
            {
                LeddarUtils::LtStringUtils::IntToString( aDeviceAddress ); 
            }

        ~LdConnectionInfoUsb() {}

        uint16_t    GetVendorID( void ) const { return mVendorID; }
        uint16_t    GetProductID( void ) const { return mProductID; }
        uint8_t     GetBusNumber( void ) const { return mBusNumber; }
        uint8_t     GetDeviceAddress( void ) const { return mDeviceAddress; }
        void        SetDeviceAddress( uint8_t aDeviceAddress ) { mDeviceAddress = aDeviceAddress; }
        std::string GetSerialNumber( void ) const { return mSerialNumber; }
        LtComUSBPublic::LtComUsbIdtAnswerIdentify GetInfos( void ) const {return mInfos;}
        bool     IsAlreadyOpen( void ) const { return mAlreadyOpen;  }

    protected:
        uint16_t mVendorID;
        uint16_t mProductID;
        uint8_t  mBusNumber;
        uint8_t  mDeviceAddress;
        std::string mSerialNumber;
        bool     mAlreadyOpen;
        LtComUSBPublic::LtComUsbIdtAnswerIdentify mInfos;

    private:
        LdConnectionInfoUsb( const LdConnectionInfoUsb &aConnectionInfo ); //Disable copy constructor
        LdConnectionInfoUsb &operator=( const LdConnectionInfoUsb &aConnectionInfo );//Disable equal constructor
    };
}
#endif