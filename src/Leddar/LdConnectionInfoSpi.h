// *****************************************************************************
// Module..: Leddar
//
/// \file    LdConnectionInfoSpi.h
///
/// \brief   Connection information on SPI devices.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#pragma once

#include "LtDefines.h"
#ifdef BUILD_SPI

#include "LdConnectionInfo.h"
#include "LtStringUtils.h"
#include "LtDefines.h"

namespace LeddarConnection
{
    class LdConnectionInfoSpi : public LdConnectionInfo
    {
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \fn LdConnectionInfoSpi::LdConnectionInfoSpi( eConnectionType aConnectionType, const std::string &aDisplayName, uint32_t aAddress, uint32_t aClock = 1000 )
        ///
        /// \brief  Constructor
        ///
        /// \param  aConnectionType Type of the connection.
        /// \param  aDisplayName    Name to display for this device.
        /// \param  aAddress        Address for this SPI device.
        /// \param  aClock          (Optional) Clock speed (default: 1000 kHz).
        ///
        /// \author Patrick Boulay
        /// \date   March 2016
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        LdConnectionInfoSpi( eConnectionType aConnectionType, const std::string &aDisplayName, uint32_t aAddress, uint32_t aClock = 1000 ) :
            LdConnectionInfo( aConnectionType, aDisplayName ),
            mIntAddress( aAddress ),
            mClock( aClock ) {
				SetAddress( LeddarUtils::LtStringUtils::IntToString( aAddress ) );
        }

		virtual ~LdConnectionInfoSpi(){}

        virtual const uint32_t &GetIntAddress( void ) const {
            return mIntAddress;
        }

        virtual const uint32_t &GetClock( void ) const {
            return mClock;
        }

        virtual void SetClock( uint32_t aClock ) {
            mClock = aClock;
        }

    protected:
        uint32_t mIntAddress;
        uint32_t mClock;
    };
}

#endif