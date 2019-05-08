// *****************************************************************************
// Module..: LeddarPy
//
/// \file    Connecters.h
///
/// \brief   Definitions of functions to connect used in LeddarPy module
///
/// \author  David Levy
///
/// \since   December 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once
#include <string>


namespace LeddarDevice
{
    class LdSensor;
}
/// \enum  eConnectionType
/// \brief Connection types definitions.
/// \       Value must be superior to 0xFFFF (this enum is made to be | with eLtCommDeviceType so we can specify device type and connection type)
typedef enum eConnectionType
{
    CONNECTION_TYPE_INVALID     = 0x00000000,  ///< Unknown.
    CONNECTION_TYPE_ETHERNET    = 0x00010000,  ///< Ethernet connection.
    CONNECTION_TYPE_SERIAL      = 0x00020000,  ///< Serial connection.
    CONNECTION_TYPE_USB         = 0x00030000,  ///< Usb connection.
    CONNECTION_TYPE_SPI_FTDI    = 0x00040000,  ///< SPI-FTDI connection
    CONNECTION_TYPE_CAN_KOMODO  = 0x00050000   ///< CANBus komodo
} eConnectionType;

bool ConnectEthernet( LeddarDevice::LdSensor **aSensor, const std::string &aIP, const int aPort, const int aTimeout = 2000 );
bool ConnectSerial( LeddarDevice::LdSensor **aSensor, const std::string &aConnectionString, const int aModbusAddress, const int aBaudRate );
bool ConnectUsb( LeddarDevice::LdSensor **aSensor, const std::string &aSerial );
bool ConnectSPIFTDI( LeddarDevice::LdSensor **aSensor, const std::string &aName );
bool ConnectCanKomodo( LeddarDevice::LdSensor **aSensor, int aDeviceType, int aRx, int aTx, int aBaudrate );