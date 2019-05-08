// *****************************************************************************
// Module..: LeddarConnection
//
/// \file    LdInterfaceUsb.h
///
/// \brief   Implementation of the USB interface.
///
/// \author  David Levy
///
/// \since   January 2017
//
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_USB

#include "LdConnection.h"
#include "LdConnectionInfoUsb.h"

#include <string>

namespace LeddarConnection
{
    class LdInterfaceUsb : public LdConnection
    {
    public:
        ~LdInterfaceUsb() {}

        virtual void Connect( void ) override = 0;
        virtual void Disconnect( void ) override = 0;

        virtual void Read( uint8_t aEndPoint, uint8_t *aData, uint32_t aSize ) = 0;
        virtual void Write( uint8_t aEndPoint, uint8_t *aData, uint32_t aSize ) = 0;
        virtual void ControlTransfert( uint8_t aRequestType, uint8_t aRequest, uint8_t *aData, uint32_t aSize, uint16_t aTimeout = 1000 ) = 0;

    protected:
        // *****************************************************************************
        // Function: LdInterfaceUsb::LdInterfaceUsb
        //
        /// \brief   Constructor.
        ///
        /// \param   aConnectionInfo Connection information.
        /// \param   aInterface      Interface for this interface (optional).
        ///
        /// \author  David Levy
        ///
        /// \since   January 2017
        // *****************************************************************************
        LdInterfaceUsb( const LdConnectionInfoUsb *aConnectionInfo, LdConnection *aInterface = nullptr ) :
            LdConnection( aConnectionInfo, aInterface ),
            mConnectionInfoUsb( aConnectionInfo ) {}

        const LdConnectionInfoUsb  *mConnectionInfoUsb;
    };
}

#endif