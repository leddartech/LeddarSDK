// *****************************************************************************
// Module..: Leddar
//
/// \file    LdDeviceFactory.cpp
///
/// \brief   Factory to create devices.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdDeviceFactory.h"

#include "LtDefines.h"

#include "LdCarrierEnhancedModbus.h"
#include "LdSensorVu8.h"
#include "LdSensorVu8Modbus.h"
#include "LdSensorVu8Can.h"
#include "LdSensorOneModbus.h"
#include "LdSensorM16.h"
#include "LdSensorM16Laser.h"
#include "LdSensorIS16.h"
#include "LdSensorM16Modbus.h"
#include "LdSensorM16Can.h"
#include "LdSensorPixell.h"
#include "LdSensorDTec.h"

#include "comm/LtComLeddarTechPublic.h"

using namespace LeddarDevice;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdSensor * LdDeviceFactory::CreateSensor( LeddarConnection::LdConnection *aConnection )
///
/// \brief  Create device associated to a connection type.
///         The device will connect if its not already done to get the device type.
///         Take ownership of aConnection
///
/// \exception  std::invalid_argument   Thrown when aConnectionInfo is nullptr.
/// \exception  LtConnectionFailed      If the connection failed.
///
/// \param [in,out] aConnection Connection to the sensor
///
/// \return nullptr if it fails, else a pointer to the new sensor.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdSensor *
LdDeviceFactory::CreateSensor( LeddarConnection::LdConnection *aConnection )
{
    if( aConnection == nullptr )
    {
        throw std::invalid_argument( "Connection not valid." );
    }

    if( !aConnection->IsConnected() )
    {
        aConnection->Connect();
    }

    uint32_t lDeviceType = aConnection->GetDeviceType();
    return CreateSensorFromDeviceType( lDeviceType, aConnection );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdSensor *LeddarDevice::LdDeviceFactory::CreateSensorFromDeviceType( uint32_t aDeviceType, LeddarConnection::LdConnection *aConnection )
///
/// \brief  Creates sensor from device type
///
/// \param          aDeviceType Type of the device.
/// \param [in,out] aConnection The connection to the device.
///
/// \return nullptr if it fails, else the new sensor from device type.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LdSensor *LeddarDevice::LdDeviceFactory::CreateSensorFromDeviceType( uint32_t aDeviceType, LeddarConnection::LdConnection *aConnection )
{
#ifdef BUILD_VU

    if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8 )
    {
        if( aConnection &&
                ( aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_LIB_MODBUS ||
                  aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_SPI_FTDI ) )
        {
            LeddarDevice::LdSensorVu8 *lSensor = new LdSensorVu8( aConnection );

#ifdef BUILD_MODBUS

            if( aConnection && aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_LIB_MODBUS )
            {
                lSensor->SetCarrier( new LdCarrierEnhancedModbus( aConnection ) );
            }

#endif //BUILD_MODBUS
            return lSensor;
        }

#ifdef BUILD_CANBUS

        if( aConnection && aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_CAN_KOMODO )
        {
            return new LeddarDevice::LdSensorVu8Can( aConnection );
        }

#endif //BUILD_CANBUS
    }

#endif //BUILD_VU
#if defined(BUILD_ONE) && defined(BUILD_MODBUS)

    if( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_EVALKIT == aDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_LONG_RANGE == aDeviceType )
    {
        return new LdSensorOneModbus( aConnection );
    }

#endif //BUILD_ONE && BUILD_MODBUS
#if defined(BUILD_M16)

    if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_EVALKIT || aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 )
    {
#if defined(BUILD_USB)

        if( !aConnection || aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_USB ) //!aConnection for recording TODO : enlever le !aConnection
            return new LdSensorM16( aConnection );

#endif //BUILD_USB
#if defined(BUILD_MODBUS)

        if( aConnection && aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_LIB_MODBUS )
        {
            return new LdSensorM16Modbus( aConnection );
        }

#endif //BUILD_MODBUS
#ifdef BUILD_CANBUS

        if( aConnection && aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_CAN_KOMODO )
        {
            return new LeddarDevice::LdSensorM16Can( aConnection );
        }

#endif //BUILD_CANBUS
    }
    else if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER )
    {
#if defined(BUILD_USB)

        if( !aConnection || aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_USB ) //!aConnection for recording TODO : enlever le !aConnection
            return new LdSensorM16Laser( aConnection );

#endif //BUILD_USB
#if defined(BUILD_MODBUS)

        if( aConnection && aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_LIB_MODBUS )
        {
            return new LdSensorM16Modbus( aConnection );
        }

#endif //BUILD_MODBUS
#ifdef BUILD_CANBUS

        if( aConnection && aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_CAN_KOMODO )
        {
            return new LeddarDevice::LdSensorM16Can( aConnection );
        }

#endif //BUILD_CANBUS
    }
    else if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16 )
    {
#if defined(BUILD_USB)

        if( !aConnection || aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_USB ) //!aConnection for recording TODO : enlever le !aConnection
            return new LdSensorIS16( aConnection );

#endif //BUILD_USB
#if defined(BUILD_MODBUS)

        if( aConnection && aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_LIB_MODBUS )
        {
            return new LdSensorM16Modbus( aConnection );
        }

#endif //BUILD_MODBUS
#ifdef BUILD_CANBUS

        if( aConnection && aConnection->GetConnectionInfo()->GetType() == LeddarConnection::LdConnectionInfo::CT_CAN_KOMODO )
        {
            return new LeddarDevice::LdSensorM16Can( aConnection );
        }

#endif //BUILD_CANBUS
    }

#endif //BUILD_M16

#if defined(BUILD_AUTO) && defined(BUILD_ETHERNET)

    if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_PIXELL )
    {
        LeddarDevice::LdSensor *lSensor = new LdSensorPixell( aConnection );

        return lSensor;
    }

#endif //BUILD_AUTO && BUILD_ETHERNET
#if defined(BUILD_ETHERNET) && defined(BUILD_DTEC)

    if( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_DTEC == aDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SIDETEC_M == aDeviceType ||
            LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_TRACKER == aDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VTEC == aDeviceType ||
            LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_TRACKER_TRANS == aDeviceType )
    {
        return new LdSensorDTec( aConnection );
    }

#endif //defined(BUILD_ETHERNET) && defined(BUILD_DTEC)
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdSensor *LeddarDevice::LdDeviceFactory::CreateSensorForRecording( uint32_t aDeviceType, LeddarDevice::LdSensor::eProtocol aProtocol )
///
/// \brief  Instantiate a sensor class corresponding to the provided device type and protocol type. Used for recording
///
/// \param  aDeviceType Type of the device.
/// \param  aProtocol   The protocol.
///
/// \return nullptr if it cannot instantiate sensor else the sensor
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
LdSensor *LeddarDevice::LdDeviceFactory::CreateSensorForRecording( uint32_t aDeviceType, LeddarDevice::LdSensor::eProtocol aProtocol )
{
    if( aDeviceType == 0 || aProtocol == 0 )
    {
        throw std::invalid_argument( "Invalid device type or protocol" );
    }

#ifdef BUILD_VU

    if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VU8 )
    {
#if defined(BUILD_MODBUS) || defined(BUILD_SPI)

        if( aProtocol == LeddarDevice::LdSensor::P_SPI || aProtocol == LeddarDevice::LdSensor::P_MODBUS_UNIVERSAL )
        {
            LeddarDevice::LdSensorVu8 *lSensor = new LdSensorVu8( nullptr );

#if defined(BUILD_MODBUS)

            if( aProtocol == LeddarDevice::LdSensor::P_MODBUS_UNIVERSAL )
            {
                lSensor->SetCarrier( new LdCarrierEnhancedModbus( nullptr ) );
            }

#endif
            return lSensor;

        }

#endif
#ifdef BUILD_MODBUS

        if( aProtocol == LeddarDevice::LdSensor::P_MODBUS )
        {
            return new LeddarDevice::LdSensorVu8Modbus( nullptr );
        }

#endif
#ifdef BUILD_CANBUS

        if( aProtocol == LeddarDevice::LdSensor::P_CAN )
        {
            return new LeddarDevice::LdSensorVu8Can( nullptr );
        }

#endif
    }

#endif
#if defined(BUILD_ONE) && defined(BUILD_MODBUS)

    if( ( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_EVALKIT == aDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SCH_LONG_RANGE == aDeviceType ) &&
            aProtocol == LeddarDevice::LdSensor::P_MODBUS )
    {
        return new LdSensorOneModbus( nullptr );
    }

#endif
#if defined(BUILD_M16)

    if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_EVALKIT || aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16 )
    {
#if defined(BUILD_USB)

        if( aProtocol == LeddarDevice::LdSensor::P_USB )
            return new LdSensorM16( nullptr );

#endif
#if defined(BUILD_MODBUS)

        if( aProtocol == LeddarDevice::LdSensor::P_MODBUS )
            return new LdSensorM16Modbus( nullptr );

#endif
#ifdef BUILD_CANBUS

        if( aProtocol == LeddarDevice::LdSensor::P_CAN )
        {
            return new LeddarDevice::LdSensorM16Can( nullptr );
        }

#endif
    }
    else if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_M16_LASER )
    {
#if defined(BUILD_USB)

        if( aProtocol == LeddarDevice::LdSensor::P_USB )
            return new LdSensorM16Laser( nullptr );

#endif
#if defined(BUILD_MODBUS)

        if( aProtocol == LeddarDevice::LdSensor::P_MODBUS )
            return new LdSensorM16Modbus( nullptr );

#endif
#ifdef BUILD_CANBUS

        if( aProtocol == LeddarDevice::LdSensor::P_CAN )
        {
            return new LeddarDevice::LdSensorM16Can( nullptr );
        }

#endif
    }
    else if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_IS16 )
    {
#if defined(BUILD_USB)

        if( aProtocol == LeddarDevice::LdSensor::P_USB )
            return new LdSensorIS16( nullptr );

#endif
#if defined(BUILD_MODBUS)

        if( aProtocol == LeddarDevice::LdSensor::P_MODBUS )
            return new LdSensorM16Modbus( nullptr );

#endif
#ifdef BUILD_CANBUS

        if( aProtocol == LeddarDevice::LdSensor::P_CAN )
        {
            return new LeddarDevice::LdSensorM16Can( nullptr );
        }

#endif
    }

#endif

#if defined(BUILD_AUTO) && defined(BUILD_ETHERNET)

    if( aDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_PIXELL )
    {
        LeddarDevice::LdSensor *lSensor = new LdSensorPixell( nullptr );

        return lSensor;
    }

#endif //BUILD_AUTO && BUILD_ETHERNET
#if defined(BUILD_ETHERNET) && defined(BUILD_DTEC)

    if( ( LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_DTEC == aDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SIDETEC_M == aDeviceType ||
            LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_TRACKER == aDeviceType || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VTEC == aDeviceType ||
            LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_TRACKER_TRANS == aDeviceType )
            && aProtocol == LeddarDevice::LdSensor::P_ETHERNET )
    {
        return new LdSensorDTec( nullptr );
    }

#endif //defined(BUILD_ETHERNET) && defined(BUILD_DTEC)
    return nullptr;
}
