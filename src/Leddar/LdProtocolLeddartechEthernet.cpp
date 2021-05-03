// *****************************************************************************
// Module..: LeddarPrivate
//
/// \file    LdProtocolLeddartechEthernet.h
///
/// \brief   Class definition of LdProtocolLeddartechEthernet
///
/// \author  Patrick Boulay
///
/// \since   February 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdProtocolLeddartechEthernet.h"
#if defined(BUILD_ETHERNET)

#include "comm/LtComLeddarTechPublic.h"
#include "comm/LtComEthernetPublic.h"
#include "comm/LtComLeddarTechPublic.h"

#include "LtExceptions.h"
#include "LtStringUtils.h"

#include <cstring>

using namespace LeddarConnection;

// *****************************************************************************
// Function: LdProtocolLeddartechEthernet::LdProtocolLeddartechEthernet
//
/// \brief   Constructor
///
/// \param  aConnectionInfo Connection information
/// \param  aInterface      Interface to connect to
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************
LdProtocolLeddartechEthernet::LdProtocolLeddartechEthernet( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface ) :
    LdProtocolLeddarTech( aConnectionInfo, aInterface )
{
    mInterfaceEthernet = dynamic_cast< LdInterfaceEthernet * >( aInterface );
    SetDeviceType( dynamic_cast<const LdConnectionInfoEthernet *>( aConnectionInfo )->GetDeviceType() );

}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernet::~LdProtocolLeddartechEthernet
//
/// \brief   Destructor
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************
LdProtocolLeddartechEthernet::~LdProtocolLeddartechEthernet( void )
{

}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernet::Write
//
/// \brief   Send data to the ethernet interface.
///
/// \param  aSize   Size of data to send.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
void
LdProtocolLeddartechEthernet::Write( uint32_t aSize )
{
    mInterfaceEthernet->Send( mTransferInputBuffer, aSize );
}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernet::Read
//
/// \brief   Receive data from the ethernet interface.
///
/// \param  aSize   Size of data to receive.
///
/// \author  Patrick Boulay
///
/// \since   August 2017
// *****************************************************************************
uint32_t LdProtocolLeddartechEthernet::Read( uint32_t aSize )
{
    if( aSize > mTransferBufferSize )
    {
        ResizeInternalBuffers( aSize );
    }

    return mInterfaceEthernet->Receive( mTransferOutputBuffer, aSize );
}

// *****************************************************************************
// Function: LdPrvProtocolLeddarTech::Connect
//
/// \brief   Establish a connection with de device.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
void
LdProtocolLeddartechEthernet::Connect( void )
{
    // Connect interface
    mInterface->Connect();
    mIsConnected = true;

    // Query device type
    if( !mIsDataServer && ( 0 == GetDeviceType() || LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_AUTO_FAMILY == GetDeviceType() ) )
    {
        QueryDeviceType();
    }
}

// *****************************************************************************
// Function: LdPrvProtocolLeddarTech::Disconnect
//
/// \brief   Disconnect the device.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
void
LdProtocolLeddartechEthernet::Disconnect( void )
{
    LdProtocolLeddarTech::Disconnect();
}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernet::SetEchoState
//
/// \brief   Set whether a periodic PING is needed to keep the connection
///          alive.
///
/// \param   aState  true: PING is needed (default), false not PING needed
///                  (useful for debugging with breakpoints or when updating firmware).
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
void
LdProtocolLeddartechEthernet::SetEchoState( bool aState )
{
    StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_SET );
    uint8_t lState = ( aState ? 1 : 0 );
    AddElement( LEDDARTECH_ID_ECHO_STATE, 1, sizeof( uint8_t ), &lState, sizeof( uint8_t ) );
    SendRequest();
    ReadAnswer();
}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernet::ReadAnswer
//
/// \brief   Read the header of the answer of the previous request
///
/// \throw   Throw LdComException, see VerifyConnection
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************
void
LdProtocolLeddartechEthernet::ReadAnswer( void )
{
    VerifyConnection();

    LtComLeddarTechPublic::sLtCommAnswerHeader *lHeader = reinterpret_cast<LtComLeddarTechPublic::sLtCommAnswerHeader *>( mTransferOutputBuffer );

    Read( sizeof( LtComLeddarTechPublic::sLtCommAnswerHeader ) );

    if( lHeader->mRequestCode != mRequestCode )
    {
        mInterfaceEthernet->FlushBuffer();
        throw LeddarException::LtComException( "Received a different request code than the request, expected: " + LeddarUtils::LtStringUtils::IntToString(
                mRequestCode ) + " received: " + LeddarUtils::LtStringUtils::IntToString( lHeader->mRequestCode ) );
    }

    mAnswerCode = lHeader->mAnswerCode;
    mMessageSize = lHeader->mAnswerSize - sizeof( LtComLeddarTechPublic::sLtCommAnswerHeader );
    mElementOffset = 0;

    if( mMessageSize > 0 )
    {
        // Read the payload
        Read( static_cast<uint32_t>( mMessageSize ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdProtocolLeddartechEthernet::ReadRequest( void )
///
/// \brief  Reads the request
///
/// \author Patrick Boulay
/// \date   February 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdProtocolLeddartechEthernet::ReadRequest( void )
{
    LdProtocolLeddarTech::ReadRequest();

    if( mMessageSize > 0 )
    {
        // Read the payload
        mElementOffset = 0;
        Read( static_cast<uint32_t>( mMessageSize ) );
    }

}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernet::QueryDeviceType
//
/// \brief   Fetch the device type from the sensor (config server)
///
/// \throw   LtComException If the device is not connected.
///
/// \author  David Levy
///
/// \since   January 2018
// *****************************************************************************
void
LdProtocolLeddartechEthernet::QueryDeviceType( void )
{
    std::vector<uint16_t> lDeviceIds;
    lDeviceIds.push_back( LtComLeddarTechPublic::LT_COMM_ID_DEVICE_TYPE );
    StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
    AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, static_cast<uint16_t>( lDeviceIds.size() ), sizeof( uint16_t ), &lDeviceIds[0], sizeof( uint16_t ) );
    SendRequest();

    ReadAnswer();

    if( !ReadElement() || GetAnswerCode() != LtComLeddarTechPublic::LT_COMM_ANSWER_OK || mElementCount != 1 || mElementSize != sizeof( uint16_t ) )
        return;

    SetDeviceType( *( reinterpret_cast< uint16_t * >( mTransferOutputBuffer + mElementValueOffset ) ) );
}
#endif
