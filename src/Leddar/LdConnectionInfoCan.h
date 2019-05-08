////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdConnectionInfoCan.h
///
/// \brief  Declares the LdConnectionInfoCan class
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LtDefines.h"
#ifdef BUILD_CANBUS

#include "LdConnectionInfo.h"
#include "LtStringUtils.h"
#include <vector>
#include <stdint.h>

namespace LeddarConnection
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \class  LdConnectionInfoCan
    ///
    /// \brief  Contains all information about a CAN connection
    ///
    /// \author David Levy
    /// \date   October 2018
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class LdConnectionInfoCan : public LdConnectionInfo
    {
    public:
        explicit LdConnectionInfoCan( eConnectionType aConnectionType, const std::string &aDescription, uint16_t aPort, uint8_t aChannel = 0, uint16_t aSpeed = 1000, uint16_t aBaseIdTx = 0x750,
                                      uint16_t aBaseIdRx = 0x740,
                                      bool aStandardFrameFormat = true ) : LdConnectionInfo( aConnectionType, aDescription ),
            mDescription( aDescription ),
            mPortNumber( aPort ),
            mChannel( aChannel ),
            mSpeed( aSpeed ),
            mBaseIdTx( aBaseIdTx ),
            mBaseIdRx( aBaseIdRx ),
            mStandardFrameFormat( aStandardFrameFormat )
        {
            SetAddress(::LeddarUtils::LtStringUtils::IntToString( aPort )); 
        }

        ~LdConnectionInfoCan() {}

        // Getter / setter
        const std::string   &GetDescription( void ) const { return( mDescription ); }
        void                SetDescription( const std::string &aDescription ) { mDescription = aDescription; }
        const uint16_t      &GetPortNumber( void ) const { return( mPortNumber ); }
        void                SetPortNumber( const uint8_t &aPortNumber ) { mPortNumber = aPortNumber; }
        const uint8_t       &GetChannel( void ) const { return( mChannel ); }
        void                SetChannel( const uint8_t &aChannel ) { mChannel = aChannel; }
        const uint16_t      &GetSpeed( void ) const { return( mSpeed ); }
        void                SetSpeed( const uint16_t &aSpeed ) { mSpeed = aSpeed; }
        const uint16_t      &GetBaseIdTx( void ) const { return( mBaseIdTx ); }
        void                SetBaseIdTx( const uint16_t &aBaseIdTx ) { mBaseIdTx = aBaseIdTx;}
        const uint16_t      &GetBaseIdRx( void ) const { return( mBaseIdRx ); }
        void                SetBaseIdRx( const uint16_t &aBaseIdRx ) { mBaseIdRx = aBaseIdRx; }
        bool                GetStandardFrameFormat( void ) const { return( mStandardFrameFormat ); }
        void                SetStandardFrameFormat( bool aStandardFrameFormat ) { mStandardFrameFormat = aStandardFrameFormat; }

    private:
        std::string mDescription;
        uint16_t    mPortNumber;            /// Komodo port number - First byte is the actual port number - with a 0x8000 mask if the port is busy
        uint8_t     mChannel;               /// Channel A (0) or B (1) - Used for Komodo hardware
        uint16_t    mSpeed;                 /// Baud rate in kbps
        uint16_t    mBaseIdTx;              /// Base id for transmission (sensor to host)
        uint16_t    mBaseIdRx;              /// Base id for reception (host to sensor)
        bool        mStandardFrameFormat;   /// Standard frame format (11 bits) or Extended (29 bits)
    };
}

#endif