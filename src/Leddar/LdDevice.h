// *****************************************************************************
// Module..: Leddar
//
/// \file    LdDevice.h
///
/// \brief   Base class of all devices.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LdConnection.h"
#include "LdObject.h"

#include "LdPropertiesContainer.h"

namespace LeddarDevice
{

    class LdDevice : public LeddarCore::LdObject
    {
    public:
        ~LdDevice();

        virtual void Connect( void );
        virtual void Disconnect( void );
        virtual LeddarConnection::LdConnection *GetConnection( void ) { return mConnection; }
        LeddarCore::LdPropertiesContainer *GetProperties( void ) { return mProperties; }
    protected:
        LdDevice( LeddarConnection::LdConnection *aConnection, LeddarCore::LdPropertiesContainer *aProperties = nullptr );
        LeddarCore::LdPropertiesContainer *mProperties;
        bool mDeleteConnection;

    private:
        LdDevice( const LdDevice &aDevice ); //Disable copy constructor
        LdDevice &operator=( const LdDevice &aDevice );//Disable equal constructor

        bool mDeleteProperties;
        LeddarConnection::LdConnection *mConnection;
    };
}
