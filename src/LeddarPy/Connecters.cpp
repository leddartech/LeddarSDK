// *****************************************************************************
// Module..: LeddarPy
//
/// \file    Connecters.cpp
///
/// \brief   Implementations of functions to connect used in LeddarPy module
///
/// \author  David Levy
///
/// \since   December 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "Connecters.h"
#include "PythonHelper.h"
#include "LtStringUtils.h"

#include "LtComLeddarTechPublic.h"
#include "LdLibModbusSerial.h"
#include "LdSpiFTDI.h"
#include "LdLibUsb.h"
#include "LdEthernet.h"
#include "LdProtocolCan.h"
#include "LdCanKomodo.h"

#include "LdSensor.h"
#include "LdDeviceFactory.h"

#include "LdConnectionFactory.h"

LeddarConnection::LdConnection *LdCreateConnection( const LeddarConnection::LdConnectionInfo *aConnectionInfo, LeddarConnection::LdConnection *aConnection = nullptr,
        uint32_t aForcedDeviceType = 0 )
{return LeddarConnection::LdConnectionFactory::CreateConnection( aConnectionInfo, aConnection, aForcedDeviceType );}

LeddarDevice::LdSensor *LdCreateSensor( LeddarConnection::LdConnection *aConnection ) {return LeddarDevice::LdDeviceFactory::CreateSensor( aConnection );}



// *****************************************************************************
// Function: ConnectEthernet
//
/// \brief   Connect to a ethernet device using ip / port
///
/// param[in] aSensor: pointer to the pointer where to store the sensor handle if we connect to it
/// param[in] aIP: IP of the sensor to connect to
/// param[in] aPort: port of the sensor to connect to
///
/// \return  False if it connect to the requested sensor. True if connected
///
/// \author  David Levy
///
/// \since   December 2017
// *****************************************************************************
bool ConnectEthernet( LeddarDevice::LdSensor **aSensor, const std::string &aIP, const int aPort, const int aTimeout )
{
    if( ( *aSensor ) != nullptr )
    {
        DebugTrace( "Already connected to a sensor." );
        return false;
    }

    bool lConnected = false;

    try
    {
        DebugTrace( "Connecting to LeddarAuto @ " + aIP + ":" + LeddarUtils::LtStringUtils::IntToString( aPort ) );

        LeddarConnection::LdConnectionInfoEthernet *lConnectionInfo = nullptr;
        lConnectionInfo = new LeddarConnection::LdConnectionInfoEthernet( aIP, aPort, "", LeddarConnection::LdConnectionInfo::CT_ETHERNET_LEDDARTECH );

        lConnectionInfo->SetTimeout( aTimeout );
        LeddarConnection::LdConnection *lConnection = LdCreateConnection( lConnectionInfo );

        *aSensor = LdCreateSensor( lConnection );
        ( *aSensor )->Connect();

        DebugTrace( "Connected." );
        lConnected = true;

        ( *aSensor )->GetConstants();
        ( *aSensor )->GetConfig();
        ( *aSensor )->GetCalib();

        return true;
    }
    catch( std::exception &e )
    {
        DebugTrace( "Not connected." );

        if( ( *aSensor ) != nullptr )
        {
            if( lConnected )
                ( *aSensor )->Disconnect();

            delete( *aSensor );
            ( *aSensor ) = nullptr;
        }

        DebugTrace( e.what() );
        return false;
    }
}

// *****************************************************************************
// Function: ConnectSerial
//
/// \brief   Connect to the sensor on the serial interface
///
/// param[in] aSensor: pointer to the pointer where to store the sensor handle if we connect to it
/// param[in] aSensorName: Name of the sensor to connect to
/// param[in] aModbusAddress: Modbus address to connect to
///
/// \return  False if it didnt found the requested sensor. True if connected
///
/// \author  David Levy
///
/// \since   December 2017
// *****************************************************************************
bool ConnectSerial( LeddarDevice::LdSensor **aSensor, const std::string &aConnectionString, const int aModbusAddress, const int aBaudRate )
{
    if( *aSensor != nullptr )
    {
        DebugTrace( "Already connected to a sensor." );
        return false;
    }

    bool lConnected = false;

    try
    {
        DebugTrace( "Connecting to a serial device @ " + aConnectionString + " - " + LeddarUtils::LtStringUtils::IntToString( aModbusAddress ) + "/" +
                    LeddarUtils::LtStringUtils::IntToString( aBaudRate ) );
        auto *lConnectionInfo = new LeddarConnection::LdConnectionInfoModbus( aConnectionString, "Serial Sensor", aBaudRate, LeddarConnection::LdConnectionInfoModbus::MB_PARITY_NONE, 8, 1,
                aModbusAddress );
        LeddarConnection::LdConnection *lConnection = LdCreateConnection( lConnectionInfo );
        *aSensor = LdCreateSensor( lConnection );
        ( *aSensor )->Connect();

        DebugTrace( "Connected." );
        lConnected = true;

        ( *aSensor )->GetConstants();
        ( *aSensor )->GetConfig();
        return true;
    }
    catch( std::exception &e )
    {
        DebugTrace( "Not connected." );

        if( ( *aSensor ) != nullptr )
        {
            if( lConnected )
                ( *aSensor )->Disconnect();

            delete( *aSensor );
            ( *aSensor ) = nullptr;
        }

        DebugTrace( e.what() );
        return false;
    }

}

/// *****************************************************************************
/// Function: ConnectUsb
///
/// \brief   Connect to the sensor on the USB interface
///
/// param[in] aSensor: pointer to the pointer where to store the sensor handle if we connect to it
/// param[in] aSensorName: Serial number of the sensor to connect to
///
/// \return  False if it didnt found the requested sensor. True if connected
///
/// \author  David Levy
///
/// \since   December 2017
/// *****************************************************************************
bool ConnectUsb( LeddarDevice::LdSensor **aSensor, const std::string &aSerial )
{
    if( *aSensor != nullptr )
    {
        DebugTrace( "Already connected to a sensor." );
        return false;
    }

    bool lConnected = false;

    try
    {
        DebugTrace( "Connecting to a usb device @ " + aSerial );
        std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList = LeddarConnection::LdLibUsb::GetDeviceList( 0x28F1, 0x0400 );
        LeddarConnection::LdConnectionInfoUsb *lConnectionInfo = nullptr;

        for( auto &lInfo : lConnectionsList )
        {
            if( dynamic_cast<LeddarConnection::LdConnectionInfoUsb *>( lInfo )->GetSerialNumber() == aSerial )
                lConnectionInfo = dynamic_cast<LeddarConnection::LdConnectionInfoUsb *>( lInfo );
            else
            {
                delete lInfo;
                lInfo = nullptr;
            }
        }

        if( lConnectionInfo == nullptr )
        {
            DebugTrace( "No sensor found with requested serial number." );
            return false;
        }

        LeddarConnection::LdConnection *lConnection = LdCreateConnection( lConnectionInfo );
        lConnection->Connect(); //We can connect to USB to fetch device type
        *aSensor = LdCreateSensor( lConnection );
        DebugTrace( "Connected." );
        lConnected = true;

        ( *aSensor )->GetConstants();
        ( *aSensor )->GetConfig();
        return true;
    }
    catch( std::exception &e )
    {
        DebugTrace( "Not connected." );

        if( ( *aSensor ) != nullptr )
        {
            if( lConnected )
                ( *aSensor )->Disconnect();

            delete( *aSensor );
            ( *aSensor ) = nullptr;
        }

        DebugTrace( e.what() );
        return false;
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool ConnectSPIFTDI( LeddarDevice::LdSensor **aSensor, const std::string &aName )
///
/// \brief  Connects to a sensor using a SPI FTDI connection
///
/// \param [in,out] aSensor pointer to the pointer where to store the sensor handle if we connect to it
/// \param          aName   The name of the sensor to connect to.
///
/// \return True if it succeeds, false if it fails.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ConnectSPIFTDI( LeddarDevice::LdSensor **aSensor, const std::string &aName )
{
    if( *aSensor != nullptr )
    {
        DebugTrace( "Already connected to a sensor." );
        return false;
    }

    bool lConnected = false;

    try
    {
        DebugTrace( "Connecting to a SPI FTDI device @ " + aName );
        std::vector<LeddarConnection::LdConnectionInfo *> lConnectionsList = LeddarConnection::LdSpiFTDI::GetDeviceList();

        for( unsigned int i = 0; i < lConnectionsList.size(); ++i )
        {
            if( aName.compare( lConnectionsList[i]->GetDisplayName() ) == 0 )
            {
                DebugTrace( "Sensor found" );
                deleteAllButOneConnections( lConnectionsList, i );

                // Create the connection object
                LeddarConnection::LdConnection *lConnection = LdCreateConnection( lConnectionsList[i] );
                *aSensor = LdCreateSensor( lConnection );

                DebugTrace( "Connected." );
                lConnected = true;

                ( *aSensor )->GetConstants();
                ( *aSensor )->GetConfig();
                return true;
            }
        }

        deleteAllButOneConnections( lConnectionsList );
        DebugTrace( "No sensor found with requested name." );
        return false;
    }
    catch( std::exception &e )
    {
        DebugTrace( "Not connected." );

        if( ( *aSensor ) != nullptr )
        {
            if( lConnected )
                ( *aSensor )->Disconnect();

            delete( *aSensor );
            ( *aSensor ) = nullptr;
        }

        DebugTrace( e.what() );
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool ConnectCanKomodo( LeddarDevice::LdSensor **aSensor, int aDeviceType, int aRx, int aTx )
///
/// \brief  Connects a sensor using can komodo
///
/// \param [in,out] aSensor     pointer to the pointer where to store the sensor handle if we connect to it
/// \param          aDeviceType Type of the device.
/// \param          aRx         The receive id.
/// \param          aTx         The transmit id.
///
/// \returns    True if it succeeds, false if it fails.
///
/// \author David Levy
/// \date   May 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ConnectCanKomodo( LeddarDevice::LdSensor **aSensor, int aDeviceType, int aRx, int aTx, int aBaudrate )
{
    if( *aSensor != nullptr )
    {
        DebugTrace( "Already connected to a sensor." );
        return false;
    }

    bool lConnected = false;

    try
    {
        bool lM16 = false;

        switch( aDeviceType )
        {
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16:
            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_EVALKIT:
                lM16 = true;
                break;

            case LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8:
                lM16 = false;
                break;

            default:
                throw std::invalid_argument( "Unsupported device type" ); //Should never reach this point, its already checked before
        }

        DebugTrace( std::string( "lM16: " ) + std::to_string( lM16 ) );
        auto lList = LeddarConnection::LdCanKomodo::GetDeviceList();

        if( lList.size() == 0 )
        {
            DebugTrace( "No CAN Komodo found" );
            return false;
        }

        LeddarConnection::LdConnectionInfoCan *lInfo = dynamic_cast<LeddarConnection::LdConnectionInfoCan *>( lList[0] );
        lInfo->SetBaseIdRx( aRx );
        lInfo->SetBaseIdTx( aTx );
        lInfo->SetSpeed( aBaudrate );
        LeddarConnection::LdConnection *lConnection = LdCreateConnection( lInfo, nullptr, aDeviceType );
        lConnection->SetDeviceType( aDeviceType );
        *aSensor = LdCreateSensor( lConnection );
        ( *aSensor )->Connect();

        DebugTrace( "Connected." );
        lConnected = true;

        ( *aSensor )->GetConstants();
        ( *aSensor )->GetConfig();
        DebugTrace( "Fetched properties." );
        return true;
    }
    catch( std::exception &e )
    {
        DebugTrace( "Not connected." );

        if( ( *aSensor ) != nullptr )
        {
            if( lConnected )
                ( *aSensor )->Disconnect();

            delete( *aSensor );
            ( *aSensor ) = nullptr;
        }

        DebugTrace( e.what() );
        return false;
    }
}
