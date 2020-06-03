// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionFactory.cpp
///
/// \brief   Factory to create connection
///
/// \author  Patrick Boulay
///
/// \since   February 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdConnectionFactory.h"

#include "LtDefines.h"
#include "comm/LtComLeddarTechPublic.h"
#include "LdConnectionInfo.h"
#include "LdConnectionUniversalSpi.h"
#include "LdConnectionUniversalModbus.h"
#include "LdLibModbusSerial.h"
#include "LdSpiFTDI.h"
#include "LdProtocolLeddartechUSB.h"
#include "LdLibUsb.h"
#include "LdSpiBCM2835.h"
#include "LdCanKomodo.h"
#include "LdProtocolCan.h"
#include "LdProtocolLeddartechEthernet.h"
#include "LdProtocolLeddartechEthernetUDP.h"
#include "LdEthernet.h"

using namespace LeddarConnection;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdConnection * LdConnectionFactory::CreateConnection( const LdConnectionInfo *aConnectionInfo, LeddarConnection::LdConnection *aConnection, uint32_t aForcedDeviceType )
///
/// \brief  Create the associated connection with the connection info object.
///
/// \exception  std::invalid_argument   Thrown when aConnectionInfo is nullptr or invalid.
///
/// \param          aConnectionInfo     Information describing the connection.
/// \param [in,out] aConnection         Only used with Modbus/Canbus connection.
///                                     Specify the shared modbus connection when multiple sensor on the same line.
///                                     Set to nullptr on single device on the COM port.
/// \param          aForcedDeviceType   If the device type is not 0, it will force the device type to the type aForcedDeviceType.
///                                     Only used for raw connection to rescue broken firmware devices.
///                                     Or for CANbus
///
/// \return nullptr if it fails, else the new connection.
///
/// \author David Levy
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdConnection *
LdConnectionFactory::CreateConnection( const LdConnectionInfo *aConnectionInfo, LeddarConnection::LdConnection *aConnection, uint32_t aForcedDeviceType )
{
    if( aConnectionInfo == nullptr )
    {
        throw std::invalid_argument( "Connection not valid." );
    }

#ifdef BUILD_USB

    if( aConnectionInfo->GetType() == LdConnectionInfo::CT_USB )
    {
        const LdConnectionInfoUsb *lConnectionInfo = dynamic_cast<const LdConnectionInfoUsb *>( aConnectionInfo );
        LdInterfaceUsb *lUsbInterface = new LdLibUsb( lConnectionInfo );
        return new LdProtocolLeddartechUSB( lConnectionInfo, lUsbInterface );
    }

#endif
#ifdef BUILD_SPI

    if( aConnectionInfo->GetType() == LdConnectionInfo::CT_SPI_FTDI )
    {

        const LdConnectionInfoSpi *lConnectionInfo = dynamic_cast<const LdConnectionInfoSpi *>( aConnectionInfo );
        LdInterfaceSpi *lSpiInterface = new LdSpiFTDI( lConnectionInfo );
        LdConnection *lConnection = new LdConnectionUniversalSpi( lConnectionInfo, lSpiInterface );

        if( lConnection->GetDeviceType() == 0 && aForcedDeviceType != 0 )
        {
            lConnection->SetDeviceType( aForcedDeviceType );
        }

        return lConnection;
    }

#endif
#ifdef BUILD_SPI_BCM2835

    if( aConnectionInfo->GetType() == LdConnectionInfo::CT_SPI_BCM2835 )
    {
        const LdConnectionInfoSpi *lConnectionInfo = dynamic_cast<const LdConnectionInfoSpi *>( aConnectionInfo );
        LdInterfaceSpi *lSpiInterface = new LdSpiBCM2835( lConnectionInfo );
        return new LdConnectionUniversalSpi( lConnectionInfo, lSpiInterface );
    }

#endif
#ifdef BUILD_MODBUS

    if( aConnectionInfo->GetType() == LdConnectionInfo::CT_LIB_MODBUS )
    {
        const LdConnectionInfoModbus *lConnectionInfo = dynamic_cast<const LdConnectionInfoModbus *>( aConnectionInfo );
        LdInterfaceModbus *lConnectionModbus = new LdLibModbusSerial( lConnectionInfo, ( aConnection != nullptr &&
                aConnection->GetInterface() != nullptr ? aConnection->GetInterface() : nullptr ) );
        lConnectionModbus->Connect();

        if( lConnectionModbus->GetDeviceType() != LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8 && aForcedDeviceType == 0 )
            return lConnectionModbus;

        lConnectionModbus->Disconnect(); //Need to disconnect from modbus so we can connect to universal
        LeddarConnection::LdConnection *lConnectionUniversal = new LdConnectionUniversalModbus( lConnectionInfo, lConnectionModbus );

        if( lConnectionUniversal->GetDeviceType() == 0 && aForcedDeviceType != 0 )
        {
            lConnectionUniversal->SetDeviceType( aForcedDeviceType );
        }

        if( aConnection != nullptr )
        {
            lConnectionUniversal->SetDeviceType( aConnection->GetDeviceType() );
        }

        return lConnectionUniversal;
    }

#endif

#if defined(BUILD_CANBUS) && defined(BUILD_CANBUS_KOMODO)

    if( aConnectionInfo->GetType() == LdConnectionInfo::CT_CAN_KOMODO )
    {
        const LdConnectionInfoCan *lConnectionInfo = dynamic_cast<const LdConnectionInfoCan *>( aConnectionInfo );
        LdCanKomodo *lInterface = new LdCanKomodo( lConnectionInfo, aConnection );

        switch( aForcedDeviceType )
        {
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_EVALKIT:
                return new LdProtocolCan( lConnectionInfo, lInterface, true );
                break;

            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8:
                return new LdProtocolCan( lConnectionInfo, lInterface, false );
                break;

            default:
                throw std::invalid_argument( "Unsupported device type for canbus protocol" );
        }
    }

#endif

#ifdef BUILD_ETHERNET

    if( aConnectionInfo->GetType() == LdConnectionInfo::CT_ETHERNET_LEDDARTECH )
    {
        const LdConnectionInfoEthernet *lConnectionInfo = dynamic_cast<const LdConnectionInfoEthernet *>( aConnectionInfo );
        LdInterfaceEthernet *lConnectionEthernet =  new LdEthernet( lConnectionInfo );

        if( lConnectionInfo->GetProtocoleType() == LeddarConnection::LdConnectionInfoEthernet::PT_TCP )
        {
            return new LdProtocolLeddartechEthernet( lConnectionInfo, lConnectionEthernet );
        }
        else if( lConnectionInfo->GetProtocoleType() == LeddarConnection::LdConnectionInfoEthernet::PT_UDP )
        {
            return new LdProtocolLeddartechEthernetUDP( lConnectionInfo, lConnectionEthernet );
        }
        else
            return nullptr;
    }

#endif
    throw std::invalid_argument( "Invalid connection type." );
    return nullptr;
}
