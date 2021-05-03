// *****************************************************************************
// Module..: LeddarPrivate
//
/// \file    LdProtocolLeddartechEthernetUDP.h
///
/// \brief   Class definition of LdProtocolLeddartechEthernetUDP
///
/// \author  Patrick Boulay
///
/// \since   August 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdProtocolLeddartechEthernetUDP.h"
#if defined(BUILD_ETHERNET)

#include "LdConnectionInfoEthernet.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"

#include "comm/LtComEthernetPublic.h"

using namespace LeddarConnection;

// *****************************************************************************
// Function: LdProtocolLeddartechEthernetUDP::LdProtocolLeddartechEthernetUDP
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

LdProtocolLeddartechEthernetUDP::LdProtocolLeddartechEthernetUDP( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface ) :
    LdProtocolLeddarTech( aConnectionInfo, aInterface )
{
    mInterfaceEthernet = dynamic_cast< LdInterfaceEthernet * >( aInterface );
    mConnectionInfoEthernet = dynamic_cast< const LeddarConnection::LdConnectionInfoEthernet * >( aConnectionInfo );
    SetDeviceType( dynamic_cast<const LdConnectionInfoEthernet *>( aConnectionInfo )->GetDeviceType() );
}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernetUDP::~LdProtocolLeddartechEthernetUDP
//
/// \brief   Destructor
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

LdProtocolLeddartechEthernetUDP::~LdProtocolLeddartechEthernetUDP( void )
{

}


// *****************************************************************************
// Function: LdPrvProtocolLeddartechEthernet::Read
//
/// \brief   Receive data from the ethernet interface through the UDP protocol.
///
/// \param  aSize   Size of data to receive.
///
/// \author  Patrick Boulay
///
/// \since   August 2017
// *****************************************************************************

uint32_t LdProtocolLeddartechEthernetUDP::Read( uint32_t )
{
    uint16_t lPortFrom = 0;
    std::string lAddressFrom = "";
    return mInterfaceEthernet->ReceiveFrom( lAddressFrom, lPortFrom, mTransferOutputBuffer, mTransferBufferSize );
}


// *****************************************************************************
// Function: LdProtocolLeddartechEthernetUDP::Connect
//
/// \brief   Establish a connection with the data server device.
///
/// \author  Patrick Boulay
///
/// \since   August 2017
// *****************************************************************************

void
LdProtocolLeddartechEthernetUDP::Connect( void )
{
    // Connect interface
    mInterfaceEthernet->OpenUDPSocket( mConnectionInfoEthernet->GetPort() );
    mIsConnected = true;
}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernetUDP::Disconnect
//
/// \brief   Disconnect the data server device.
///
/// \author  Patrick Boulay
///
/// \since   August 2017
// *****************************************************************************

void
LdProtocolLeddartechEthernetUDP::Disconnect( void )
{
    if( mIsConnected )
        mInterfaceEthernet->CloseUDPSocket();
    mIsConnected = false;
}

// *****************************************************************************
// Function: LdProtocolLeddartechEthernetUDP::ReadAnswer
//
/// \brief   Read one packet in the UDP buffer
///
/// \author  Patrick Boulay
///
/// \since   August 2017
// *****************************************************************************
void
LdProtocolLeddartechEthernetUDP::ReadAnswer( void )
{
    VerifyConnection();

    LtComLeddarTechPublic::sLtCommAnswerHeader *lHeader = reinterpret_cast<LtComLeddarTechPublic::sLtCommAnswerHeader *>( mTransferOutputBuffer );

    Read( 0 ); //Argument is not used in UDP. UDP protocol reads the whole message (opposed to TCP stream)

    mRequestCode = lHeader->mRequestCode;
    mAnswerCode = lHeader->mAnswerCode;
    mMessageSize = lHeader->mAnswerSize - sizeof( LtComLeddarTechPublic::sLtCommAnswerHeader );
    mElementOffset = sizeof( LtComLeddarTechPublic::sLtCommAnswerHeader );
}

#endif
