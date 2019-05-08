// *****************************************************************************
// Module..: Leddar
//
/// \file    LdDevice.cpp
///
/// \brief   Base class of all devices.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdDevice.h"

#include <typeinfo>

using namespace LeddarDevice;


// *****************************************************************************
// Function: LdDevice::LdDevice
//
/// \brief   Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param   aConnection Connection object.
/// \param   aProperties Properties container for this device.
///                      If null, the device will declare a new instance of LdPropertiesContainer.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LdDevice::LdDevice( LeddarConnection::LdConnection *aConnection, LeddarCore::LdPropertiesContainer *aProperties ) :
    mProperties( aProperties ),
    mDeleteConnection( true ),
    mDeleteProperties( false ),
    mConnection( aConnection )
{
    if( mConnection ) //should be null for recording only
        mConnection->TakeOwnerShip( true );

    if( mProperties == nullptr )
    {
        mProperties = new LeddarCore::LdPropertiesContainer();
        mDeleteProperties = true;
    }
}

// *****************************************************************************
// Function: LdDevice::~LdDevice
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LdDevice::~LdDevice()
{
    if( mDeleteProperties )
    {
        delete mProperties;
        mProperties = nullptr;
    }

    if( mDeleteConnection && mConnection )
    {
        delete mConnection;
        mConnection = nullptr;
    }

}

// *****************************************************************************
// Function: LdDevice::Connect
//
/// \brief   Connect to the sensor
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdDevice::Connect( void )
{
    if( mConnection == nullptr )
    {
        throw std::runtime_error( "No connection assiciated to the device." );
    }

    // Already connected?
    if( mConnection->IsConnected() )
    {
        return;
    }

    mConnection->Connect();
    EmitSignal( LdObject::CONNECTED );
}

// *****************************************************************************
// Function: LdDevice::Disconnect
//
/// \brief   Disconnect the sensor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdDevice::Disconnect( void )
{
    if( mConnection == nullptr )
    {
        throw std::runtime_error( "No connection assiciated to the device." );
    }

    // Already disconnected
    if( !mConnection->IsConnected() )
    {
        return;
    }

    mConnection->Disconnect();
    EmitSignal( LdObject::DISCONNECTED );
}
