// *****************************************************************************
// Module..: Leddar
//
/// \file    LdEthernet.cpp
///
/// \brief   Base class of LdEthernet using the standard Windows and Linux libraries.
///
/// \author  Patrick Boulay
///
/// \since   October 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdEthernet.h"
#ifdef BUILD_ETHERNET

#include "LdConnectionInfo.h"
#include "LtStringUtils.h"
#include "LtExceptions.h"
#include "comm/LtComLeddarTechPublic.h"
#include "comm/LtComEthernetPublic.h"
#include "comm/Legacy/DTec/LtComDTec.h"
#include "LtTimeUtils.h"
#include "LtSystemUtils.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN


#if defined(_WIN32) && defined(__MINGW32__)
#define _WIN32_WINNT 0x0501
#endif

#ifdef _WIN32

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment (lib, "iphlpapi.lib")

#define LAST_ERROR WSAGetLastError()

#else

#define LAST_ERROR errno
#define SOCKET_ERROR -1

#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#endif

//For request broadcast
#define HELLO_PORT                          48620

#ifdef _WIN32
class LdWSAManager
{
public:
    LdWSAManager() {
        WSADATA lWsaData;

        if( WSAStartup( MAKEWORD( 2, 2 ), &lWsaData ) != 0 ) {
            throw std::runtime_error( "Failed to initialize socket (WSAStartup)." );
        }
    }

    ~LdWSAManager() {
        WSACleanup();
    }
};

static LdWSAManager sWSAManager;
#endif

// *****************************************************************************
// Function: LdEthernet::LdEthernet
//
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   October 2016
// *****************************************************************************
LeddarConnection::LdEthernet::LdEthernet( const LdConnectionInfoEthernet *aConnectionInfo, LdConnection *aInterface ) :
    LdInterfaceEthernet( aConnectionInfo, aInterface ),
#ifdef _WIN32
    mSocket( INVALID_SOCKET ),
    mUDPSocket( INVALID_SOCKET ),
#else
    mSocket( 0 ),
    mUDPSocket( 0 ),
#endif
    mIsConnected( false )
{
    SetDeviceType( aConnectionInfo->GetDeviceType() );
}

// *****************************************************************************
// Function: LdEthernet::~LdEthernet
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   October 2016
// *****************************************************************************
LeddarConnection::LdEthernet::~LdEthernet( void )
{
    Disconnect();
}

// *****************************************************************************
// Function: LdEthernet::Connect
//
/// \brief   Connect to device using ethernet.
///
/// \author  Patrick Boulay
///
/// \since   October 2016
// *****************************************************************************
void
LeddarConnection::LdEthernet::Connect( void )
{
    // Resolve the server address and port
    struct addrinfo lHints;
    struct addrinfo *lResult = nullptr;
    memset( &lHints, 0, sizeof( lHints ) );

    lHints.ai_family = AF_UNSPEC;
    lHints.ai_socktype = SOCK_STREAM;
    lHints.ai_protocol = IPPROTO_TCP;

    if( getaddrinfo( mConnectionInfoEthernet->GetIP().c_str(), LeddarUtils::LtStringUtils::IntToString( mConnectionInfoEthernet->GetPort() ).c_str(),
                     &lHints, &lResult ) != 0 )
    {
        throw LeddarException::LtComException( "Failed to initialize socket (getaddrinfo): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
    }

    mSocket = socket( lResult->ai_family, lResult->ai_socktype, lResult->ai_protocol );

    try
    {

#ifdef _WIN32

        if( mSocket == INVALID_SOCKET )
#else
        if( mSocket < 0 )
#endif
        {
            throw LeddarException::LtComException( "Failed to initialize socket (socket): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
        }


        // Set receive timeout.
#ifdef _WIN32
        uint32_t lTimeout = mConnectionInfoEthernet->GetTimeout();
        uint32_t lSockOptResult = setsockopt( mSocket, SOL_SOCKET, SO_RCVTIMEO, ( char * )&lTimeout, sizeof( lTimeout ) );

        if( lSockOptResult == SOCKET_ERROR )
#else
        struct timeval lTimeout;

        lTimeout.tv_sec = mConnectionInfoEthernet->GetTimeout() / 1000;
        lTimeout.tv_usec = ( mConnectionInfoEthernet->GetTimeout() % 1000 ) * 1000;
        int lSockOptResult = setsockopt( mSocket, SOL_SOCKET, SO_RCVTIMEO, &lTimeout, sizeof( lTimeout ) );

        if( lSockOptResult < 0 )
#endif
        {
            throw LeddarException::LtComException( "Failed to set socket option SO_RCVTIMEO (setsockopt): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
        }

        // Set send timeout.
#ifdef _WIN32
        lSockOptResult = setsockopt( mSocket, SOL_SOCKET, SO_SNDTIMEO, ( char * )&lTimeout, sizeof( lTimeout ) );

        if( lSockOptResult == SOCKET_ERROR )
        {
#else
        lTimeout.tv_sec = mConnectionInfoEthernet->GetTimeout() / 1000;
        lTimeout.tv_usec = ( mConnectionInfoEthernet->GetTimeout() % 1000 ) * 1000;
        lSockOptResult = setsockopt( mSocket, SOL_SOCKET, SO_SNDTIMEO, &lTimeout, sizeof( lTimeout ) );

        if( lSockOptResult < 0 )
        {
#endif
            throw LeddarException::LtComException( "Failed to set socket option SO_SNDTIMEO (setsockopt): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
        }


        int32_t lConnectResult = connect( mSocket, lResult->ai_addr, ( int )lResult->ai_addrlen );
#ifdef _WIN32

        if( lConnectResult == SOCKET_ERROR )
#else
        if( lConnectResult < 0 )
#endif
        {
            throw LeddarException::LtComException( "Failed to initialize socket (connect): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
        }

        mIsConnected = true;

    }
    catch( ... )
    {
        if( lResult != nullptr )
        {
            freeaddrinfo( lResult );

            try
            {
                mSocket = CloseSocket( mSocket );
            }
            catch( ... ) //Only throw if it cannot close the socket, we want to throw the first exception as it will be more meaningfull
            {
            }
        }

        throw;
    }
}

// *****************************************************************************
// Function: LdEthernet::Disconnect
//
/// \brief   Disconnect
///
/// \author  Patrick Boulay
///
/// \since   October 2016
// *****************************************************************************
void
LeddarConnection::LdEthernet::Disconnect( void )
{
#ifdef _WIN32

    if( mSocket != INVALID_SOCKET )
    {
        mSocket = CloseSocket( mSocket );
    }

    if( mUDPSocket != INVALID_SOCKET )
    {
        CloseUDPSocket();
    }

#else

    if( mSocket > 0 )
    {
        mSocket = CloseSocket( mSocket );
    }

    if( mUDPSocket > 0 )
    {
        CloseUDPSocket();
    }

#endif
    mIsConnected = false;
}

// *****************************************************************************
// Function: LdEthernet::Send
//
/// \brief   Send data to device
///
/// \param   aBuffer Buffer to send.
/// \param   aSize   Size of the buffer to send.
///
/// \exception LtComException when the send fail
///
/// \author  Patrick Boulay
///
/// \since   October 2016
// *****************************************************************************
void
LeddarConnection::LdEthernet::Send( uint8_t *aBuffer, uint32_t aSize )
{

    uint32_t lBytesToSend = aSize;
    const char *lCurrentBuffer = ( const char * )aBuffer;
    int lBytesSent = 0;

    while( lBytesToSend > 0 )
    {
        lBytesSent = send( mSocket, lCurrentBuffer, lBytesToSend, 0 );

        if( lBytesSent <= 0 )
            break;

        lCurrentBuffer += lBytesSent;
        lBytesToSend -= lBytesSent;
    }

    if( lBytesSent == 0 )
    {
        throw LeddarException::LtComException( "Error in Send (connection close.).", 0, true );
    }

#ifdef _WIN32
    else if( lBytesSent == SOCKET_ERROR )
    {
        if( WSAECONNABORTED == LAST_ERROR || WSAECONNRESET == LAST_ERROR )
#else

    else if( lBytesSent < 0 )
    {
        if( ECONNABORTED == LAST_ERROR || ECONNRESET == LAST_ERROR )
#endif
        {
            throw LeddarException::LtComException( "Error in Send (connection close.).", 0, true );
        }
        else
        {
            throw LeddarException::LtComException( "Error in Send (send): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
        }
    }
}

// *****************************************************************************
// Function: LdEthernet::Receive
//
/// \brief   Receive data from device
///
/// \param   aBuffer Buffer to send.
/// \param   aSize   Size of the buffer to receive.
///
/// \return  Byte received
///
/// \author  Patrick Boulay
///
/// \since   October 2016
// *****************************************************************************
size_t
LeddarConnection::LdEthernet::Receive( uint8_t *aBuffer, uint32_t aSize )
{
    int32_t lBytesReceived = 0;

    try
    {
        lBytesReceived = recv( mSocket, ( char * )aBuffer, aSize, MSG_WAITALL );
    }
    catch( std::exception &e )
    {
        throw LeddarException::LtComException( "Error in recv() : (" + LeddarUtils::LtStringUtils::IntToString( LAST_ERROR ) + ")" + std::string(
                e.what() ) );
    }

    if( lBytesReceived == 0 )
    {
        throw LeddarException::LtComException( "Error in Receive ( connection close ).", 0, true );
    }

#ifdef _WIN32
    else if( lBytesReceived == SOCKET_ERROR )
    {
        if( WSAECONNABORTED == LAST_ERROR || WSAECONNRESET == LAST_ERROR )
#else

    else if( lBytesReceived < 0 )
    {
        if( ECONNABORTED == LAST_ERROR || ECONNRESET == LAST_ERROR )
#endif
        {
            throw LeddarException::LtComException( "Error in Receive (connection close.).", 0, true );
        }
        else
        {
            throw LeddarException::LtComException( "Error in Receive (recv): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ), LeddarException::ERROR_COM_READ );
        }
    }

    if( lBytesReceived < static_cast<int32_t>(aSize) )
    {
        //Retry once to get all expected data
        int32_t lBytesReceived2 = recv( mSocket, ( char * )( aBuffer + lBytesReceived ), aSize, MSG_WAITALL );
        lBytesReceived += lBytesReceived2;

        if( lBytesReceived < static_cast<int32_t>(aSize) )
        {
            throw LeddarException::LtComException( "Incomplete data received.", LeddarException::ERROR_COM_READ, false );
        }
        else
        {
            throw LeddarException::LtComException( "Data reception was too slow (timed out once).", LeddarException::ERROR_COM_READ, false );
        }
    }

    return ( uint32_t )lBytesReceived;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn int LeddarConnection::LdEthernet::CloseSocket( const SOCKET aSocket )
///
/// \brief  Closes a socket
///
/// \exception  LeddarException::LtComException Thrown when the socket cannot be closed.
///
/// \param  aSocket The socket.
///
/// \returns    INVALID_SOCKET for windows, else 0.
///
/// \author David Levy
/// \date   May 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t LeddarConnection::LdEthernet::CloseSocket( const SOCKET aSocket )
{
    int lRet = 0;
#ifdef _WIN32
    lRet = closesocket( aSocket );
#else
    errno = 0;
    lRet = close( aSocket );
#endif

    if( lRet != 0 )
    {
        throw LeddarException::LtComException( "Failed to close socket: " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
    }

#ifdef _WIN32
    return INVALID_SOCKET;
#else
    return lRet;
#endif
}

// *****************************************************************************
// Function: LdEthernet::OpenUDPSocket
//
/// \brief   Open an UDP socket
///
/// \param   aPort    UDP port to open
/// \param   aTimeout Receive timeout in milliseconds (default 2000).
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

void
LeddarConnection::LdEthernet::OpenUDPSocket( uint32_t aPort, uint32_t aTimeout )
{
    mUDPSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
#ifdef _WIN32

    // Throwing here makes sure the object will not exist if the socket
    // is not created.
    if( mUDPSocket == INVALID_SOCKET )
#else
    if( mUDPSocket < 0 )
#endif
    {
        throw LeddarException::LtComException( "Unable to open the socket with UDP protocol." );
    }

    // Set receive timeout and OS buffer size
    int lSocketBufferSize = 100000;
#ifdef WIN32
    int lResult = setsockopt( mUDPSocket, SOL_SOCKET, SO_RCVTIMEO, ( char * )&aTimeout, sizeof( aTimeout ) );

    if( SOCKET_ERROR == lResult )
    {
        throw LeddarException::LtComException( "Unable to set option on UDP socket " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ), LAST_ERROR );
    }

    lResult = setsockopt( mUDPSocket, SOL_SOCKET, SO_RCVBUF, ( char * )&lSocketBufferSize, sizeof( lSocketBufferSize ) );

    if( SOCKET_ERROR == lResult )
    {
        throw LeddarException::LtComException( "Unable to set option on UDP socket " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ), LAST_ERROR );
    }

#else
    struct timeval lTimeout;

    lTimeout.tv_sec = aTimeout / 1000;
    lTimeout.tv_usec = ( aTimeout % 1000 ) * 1000;
    int lResult = setsockopt( mUDPSocket, SOL_SOCKET, SO_RCVTIMEO, &lTimeout, sizeof( lTimeout ) );

    if( SOCKET_ERROR == lResult )
    {
        throw LeddarException::LtComException( "Unable to set option on UDP socket" + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ), LAST_ERROR );
    }

    lResult = setsockopt( mUDPSocket, SOL_SOCKET, SO_RCVBUF, &lSocketBufferSize, sizeof( lSocketBufferSize ) );

    if( SOCKET_ERROR == lResult )
    {
        throw LeddarException::LtComException( "Unable to set option on UDP socket" + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ), LAST_ERROR );
    }

#endif

    if( aPort != 0 )
    {
        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;

#ifdef _WIN32
        addr.sin_addr.s_addr = htonl( INADDR_ANY );
#else
        addr.sin_addr.s_addr = INADDR_ANY;
#endif
        addr.sin_port = htons( aPort );
        memset( addr.sin_zero, 0, sizeof( addr.sin_zero ) );

        if( bind( mUDPSocket, ( const sockaddr * )&addr, sizeof( addr ) ) < 0 )
        {
            throw LeddarException::LtComException( "Unable to set bind UDP socket(" + LeddarUtils::LtStringUtils::IntToString( LAST_ERROR ) + ")", LAST_ERROR );
        }
    }
}

// *****************************************************************************
// Function: LdEthernet::CloseUDPSocket
//
/// \brief   Close the UDP socket
///
/// \author  Patrick Boulay
///
/// \since   August 2017
// *****************************************************************************

void
LeddarConnection::LdEthernet::CloseUDPSocket( void )
{
    mUDPSocket = CloseSocket( mUDPSocket );
}

// *****************************************************************************
// Function: LdEthernet::OpenScanRequestSockets
//
/// \brief   Check available interfaces and open a socket for every suitable interface
///             To be called before GetDevicesListSendRequest
///
/// \return  Structures containing the associated vector ip and subnet mask
///
/// \author  David Levy
///
/// \since   November 2016
// *****************************************************************************

std::vector<std::pair<SOCKET, unsigned long> > LeddarConnection::LdEthernet::OpenScanRequestSockets()
{
    std::vector<std::pair<SOCKET, unsigned long> > lInterfaces;
    // Initialize Connection

    std::vector< std::pair<unsigned long, unsigned long> > lIPPairs; //first = ip, second = subnetmask

    //Get the ip / subnet mask for all interfaces
#ifdef _WIN32
    ULONG lSize = 0;

    if( GetIpAddrTable( nullptr, &lSize, false ) == ERROR_INSUFFICIENT_BUFFER )
    {
        MIB_IPADDRTABLE *lBufferIP = new MIB_IPADDRTABLE[lSize];

        if( GetIpAddrTable( lBufferIP, &lSize, false ) != NO_ERROR )
            throw LeddarException::LtComException( "Failed to get subnet mask (GetIpAddrTable)." );

        for( DWORD i = 0; i < lBufferIP->dwNumEntries; ++i )
        {
            lIPPairs.push_back( { lBufferIP->table[i].dwAddr, lBufferIP->table[i].dwMask } );

        }
    }
    else
        throw LeddarException::LtComException( "Failed to get ip / subnet mask (GetIpAddrTable)." );

#else
    // BSD-style implementation
    struct ifaddrs *ifap;

    if( getifaddrs( &ifap ) == 0 )
    {
        struct ifaddrs *p = ifap;

        while( p )
        {
            if( p->ifa_addr != nullptr && p->ifa_netmask != nullptr )
                lIPPairs.push_back( std::make_pair( ( reinterpret_cast<sockaddr_in *>( p->ifa_addr ) )->sin_addr.s_addr,
                                                    ( reinterpret_cast<sockaddr_in *>( p->ifa_netmask ) )->sin_addr.s_addr ) );

            p = p->ifa_next;
        }

        freeifaddrs( ifap );
    }
    else
        throw LeddarException::LtComException( "Failed to get ip / subnet mask (getifaddrs)." );

#endif

    // For each interface, create ordinary UDP socket and bind it
    for( size_t i = 0; i < lIPPairs.size(); ++i )
    {
        SOCKET lSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

#ifdef _WIN32

        if( lSocket == INVALID_SOCKET )
#else
        if( lSocket < 0 )
#endif
        {
            continue;
        }

        struct sockaddr_in addr = {};

        addr.sin_family = AF_INET;

#ifdef _WIN32
        addr.sin_addr.s_addr = lIPPairs[i].first; //Weird : need to bind to the computer IP on windows

#else
        addr.sin_addr.s_addr = INADDR_ANY; //And to the global broadcast address in linux

#endif
        addr.sin_port = htons( HELLO_PORT );

        //allow broadcast
        int broadcast = 1;

        //SO_REUSEADDR is the default behavior in windows. Usefull in linux to avoid "address already in use error in case of crash
        if( setsockopt( lSocket, SOL_SOCKET, SO_BROADCAST, ( char * )&broadcast, sizeof( broadcast ) ) == 0 &&
                setsockopt( lSocket, SOL_SOCKET, SO_REUSEADDR, ( char * )&broadcast, sizeof( broadcast ) ) == 0 &&
                bind( lSocket, ( const sockaddr * )&addr, sizeof( addr ) ) == 0 )
        {
            // The multicast scan address is the interface address filtered by the subnet mask
            unsigned long lScanIP = lIPPairs[i].first  | ~lIPPairs[i].second;
            lInterfaces.push_back( std::make_pair( lSocket, lScanIP ) );
        }
        else
        {
            lSocket = CloseSocket( lSocket );
        }
    }

    if( lInterfaces.size() == 0 )
        throw LeddarException::LtComException( "Failed to open & bind socket." );

    return lInterfaces;
}

// *****************************************************************************
// Function: LdEthernet::GetDevicesListSendRequest
//
/// \brief   Scan for ethernet devices on network
///              To be called before GetDevicesListReadAnswer and after OpenScanRequestSocket
///
/// \param[in] Structures containing the associated vector ip and subnet mask
/// \param[in] Broad to all subnet or only to the subnet we are in. Usefull in a network without router if a device is misconfigured
///
/// \author  David Levy
///
/// \since   November 2016
// *****************************************************************************
void LeddarConnection::LdEthernet::GetDevicesListSendRequest( const std::vector<std::pair<SOCKET, unsigned long> > &aInterfaces, bool aWideBroadcast )
{

    typedef struct                          // LT_COMM_ID_REQUEST_HEADER
    {
        uint16_t  mSrvProtVersion;    ///< Protocol version.
        uint16_t  mRequestCode;       ///< Protocol request code.
        uint32_t  mRequestTotalSize;  ///< Request total size in bytes. The size includes this header size.
    } sLtCommRequestHeader;

    sLtCommRequestHeader request =
    {
        LT_ETHERNET_IDENTIFY_PROT_VERSION,
        LT_ETHERNET_IDT_REQUEST_IDENTIFY,
        sizeof( sLtCommRequestHeader )
    };


    bool lAllBroadcastFail  = true;

    for( size_t i = 0; i < aInterfaces.size(); ++i )
    {
        sockaddr_in addr = {};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = aWideBroadcast ? 0xFFFFFFFF : aInterfaces[i].second;
        addr.sin_port        = htons( HELLO_PORT );

        //Broadcast request
        if( sendto( aInterfaces[i].first, ( const char * )&request, sizeof( request ), 0, ( struct sockaddr * ) &addr, sizeof( addr ) ) >= 0 )
        {
            lAllBroadcastFail = false;
        }
    }

    if( lAllBroadcastFail )
        throw LeddarException::LtComException( "Failed to broadcast request." );
}

// *****************************************************************************
// Function: LdEthernet::GetDevicesListReadAnswer
//
/// \brief   Scan for ethernet devices on network - To be called after GetDevicesListSendRequest
///             The function release the ownership of the returned objects.
///             The sockets needs to be closed after use
///
/// \return  Vector of LeddarConnection::LdConnectionInfo* with all the devices that answered
///
/// \author  David Levy
///
/// \since   November 2016
// *****************************************************************************
std::vector<LeddarConnection::LdConnectionInfo *> LeddarConnection::LdEthernet::GetDevicesListReadAnswer(
    const std::vector<std::pair<SOCKET, unsigned long> > &aInterfaces )
{
    std::vector<LeddarConnection::LdConnectionInfo *> lResultList;

    //Compute answer(s)
    sockaddr_in addrFrom;
    fd_set rdFs;
    struct timeval lTimeout = {0, 001000};
    int lSelRet;
#ifdef _WIN32
    int addrFromSize;
#else
    unsigned int addrFromSize;
#endif
    int lResult, lMaxfd = 0; //lMaxfd value is ignored in windows
    char bufferIn[1024];

    FD_ZERO( &rdFs );

    for( size_t i = 0; i < aInterfaces.size(); ++i )
    {
#ifndef _WIN32
        lMaxfd = ( lMaxfd > aInterfaces[i].first ) ? lMaxfd : aInterfaces[i].first;
#endif
        FD_SET( aInterfaces[i].first, &rdFs );
    }

    do
    {
        lSelRet = select( lMaxfd + 1, &rdFs, nullptr, nullptr, &lTimeout );

        if( lSelRet > 0 )
        {
            for( size_t i = 0; i < aInterfaces.size(); ++i )
            {
                if( FD_ISSET( aInterfaces[i].first, &rdFs ) )
                {
                    addrFromSize = sizeof( addrFrom );
                    lResult = recvfrom( aInterfaces[i].first, bufferIn, sizeof( bufferIn ), 0, ( struct sockaddr * ) &addrFrom, &addrFromSize );

                    if( lResult >= 0 && static_cast<unsigned int>( lResult ) > sizeof( LtComLeddarTechPublic::sLtCommAnswerHeader ) )
                    {
                        LtComLeddarTechPublic::sLtCommAnswerHeader const *pPtr = ( LtComLeddarTechPublic::sLtCommAnswerHeader * )bufferIn;

                        if( ( pPtr->mSrvProtVersion == LT_ETHERNET_IDENTIFY_PROT_VERSION ) &&
                                ( pPtr->mAnswerCode == LT_ETHERNET_ANSWER_OK ) &&
                                ( pPtr->mRequestCode == LT_ETHERNET_IDT_REQUEST_IDENTIFY ) )
                        {
                            LeddarConnection::LdConnectionInfoEthernet *lConnecInfo = nullptr;
                            LeddarConnection::LdConnectionInfo::eConnectionType lConnectionType;
                            char lIp[INET_ADDRSTRLEN];
                            inet_ntop( AF_INET, &addrFrom.sin_addr, lIp, INET_ADDRSTRLEN );
                            LeddarConnection::LdConnectionInfoEthernet::eStatus lUsed = LeddarConnection::LdConnectionInfoEthernet::S_UNDEF;

                            if( pPtr->mAnswerSize == sizeof( LtComEthernetPublic::sLtIdtAnswerIdentifyLCA2Discrete ) &&
                                    reinterpret_cast<LtComEthernetPublic::sLtIdtAnswerIdentifyLCA2Discrete const *>( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_LCA2_DISCRETE )
                            {
                                LtComEthernetPublic::sLtIdtAnswerIdentifyLCA2Discrete *lHeader = reinterpret_cast<LtComEthernetPublic::sLtIdtAnswerIdentifyLCA2Discrete *>( bufferIn );

                                lConnectionType = LeddarConnection::LdConnectionInfo::CT_ETHERNET_UNIVERSAL;

                                if( lHeader->mSensorState & LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_ERROR )
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_ERROR;
                                }
                                else if( ( lHeader->mSensorState & ( LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_RUNNING | LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_CONNECTED ) ) ==
                                         ( LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_RUNNING | LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_CONNECTED ) )
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_CONNECTED;
                                }
                                else if( lHeader->mSensorState & LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_RUNNING )
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_NOT_CONNECTED;
                                }

                                lConnecInfo  = new LeddarConnection::LdConnectionInfoEthernet(
                                    lIp, lHeader->mDataPort, "", lConnectionType, LeddarConnection::LdConnectionInfoEthernet::PT_TCP, lUsed, 2000, std::string( reinterpret_cast<char *>( lHeader->mDeviceName ) ) );
                                lConnecInfo->SetDeviceType( lHeader->mDeviceType );
                            }
                            else if( pPtr->mAnswerSize == sizeof( LtComEthernetPublic::sLtIdtAnswerIdentifyLCAuto ) &&
                                     ( reinterpret_cast<LtComEthernetPublic::sLtIdtAnswerIdentifyLCAuto const *>( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_LCA2_REFDESIGN ||
                                       reinterpret_cast<LtComEthernetPublic::sLtIdtAnswerIdentifyLCAuto const *>( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_PIXELL ||
                                       reinterpret_cast<LtComEthernetPublic::sLtIdtAnswerIdentifyLCAuto const *>( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_LCA3_DISCRETE ) )
                            {
                                LtComEthernetPublic::sLtIdtAnswerIdentifyLCAuto *lHeader = reinterpret_cast<LtComEthernetPublic::sLtIdtAnswerIdentifyLCAuto *>( bufferIn );
                                lConnectionType = LeddarConnection::LdConnectionInfo::CT_ETHERNET_LEDDARTECH;

                                if( lHeader->mSensorState & LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_ERROR )
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_ERROR;
                                }
                                else if( ( lHeader->mSensorState & ( LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_RUNNING | LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_CONNECTED ) ) ==
                                         ( LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_RUNNING | LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_CONNECTED ) )
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_CONNECTED;
                                }
                                else if( lHeader->mSensorState & LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_RUNNING )
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_NOT_CONNECTED;
                                }

                                lConnecInfo = new LeddarConnection::LdConnectionInfoEthernet(
                                    lIp, lHeader->mDataPort, "", lConnectionType, LeddarConnection::LdConnectionInfoEthernet::PT_TCP, lUsed, 2000, std::string( reinterpret_cast<char *>( lHeader->mDeviceName ) ) );
                                lConnecInfo->SetDeviceType( lHeader->mDeviceType );
                            }
                            else if( pPtr->mAnswerSize == sizeof( LtComEthernetPublic::sLtIdtAnswerIdentifyDtec ) &&
                                     ( reinterpret_cast< LtComEthernetPublic::sLtIdtAnswerIdentifyDtec const * >( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_DTEC ||
                                       reinterpret_cast< LtComEthernetPublic::sLtIdtAnswerIdentifyDtec const * >( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_SIDETEC_M ||
                                       reinterpret_cast< LtComEthernetPublic::sLtIdtAnswerIdentifyDtec const * >( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_TRACKER ||
                                       reinterpret_cast< LtComEthernetPublic::sLtIdtAnswerIdentifyDtec const * >( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_VTEC ||
                                       reinterpret_cast< LtComEthernetPublic::sLtIdtAnswerIdentifyDtec const * >( bufferIn )->mDeviceType == LtComLeddarTechPublic::LT_COMM_DEVICE_TYPE_TRACKER_TRANS ) )
                            {
                                LtComEthernetPublic::sLtIdtAnswerIdentifyDtec *lHeader = reinterpret_cast< LtComEthernetPublic::sLtIdtAnswerIdentifyDtec * >( bufferIn );
                                lConnectionType = LeddarConnection::LdConnectionInfo::CT_ETHERNET_LEDDARTECH;

                                if( lHeader->mServerState & LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_CONNECTED )
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_CONNECTED;
                                }
                                else if( lHeader->mServerState & LtComEthernetPublic::LT_COMM_IDT_SERVER_STATE_RUNNING )
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_NOT_CONNECTED;
                                }
                                else
                                {
                                    lUsed = LeddarConnection::LdConnectionInfoEthernet::S_ERROR;
                                }

#ifdef _WIN32
                                std::wstring lWideName = std::wstring( reinterpret_cast<wchar_t *>( lHeader->mDeviceName ) );
                                std::string lDeviceName = LeddarUtils::LtStringUtils::Utf8Encode( lWideName );
#else

                                //Poor conversion, but this encoding is poorly supported in c++99 non windows
                                for( uint8_t i = 0; i < LT_COMM_DEVICE_UNICODE_NAME_LENGTH; ++i )
                                {
                                    lHeader->mDeviceName[i] = lHeader->mDeviceName[2 * i];
                                }

                                std::string lDeviceName = std::string( lHeader->mDeviceName );

#endif
                                lConnecInfo = new LeddarConnection::LdConnectionInfoEthernet(
                                    lIp, LtComDTec::DTEC_CONFIG_PORT, "", lConnectionType, LeddarConnection::LdConnectionInfoEthernet::PT_TCP, lUsed, 2000, lDeviceName );
                                lConnecInfo->SetDeviceType( lHeader->mDeviceType );

                            }
                            else
                            {
                                continue;
                            }


                            // Check if the device is already in the result list to avoid duplicate device in the result list.
                            bool lExistingDevice = false;

                            for( std::vector<LeddarConnection::LdConnectionInfo *>::iterator lIter = lResultList.begin(); lIter != lResultList.end(); ++lIter )
                            {
                                LeddarConnection::LdConnectionInfoEthernet *lExistingConnectionInfo = dynamic_cast<LeddarConnection::LdConnectionInfoEthernet *>( *lIter );

                                if( lConnecInfo != nullptr &&
                                        lExistingConnectionInfo->GetIP() == lConnecInfo->GetIP() &&
                                        lExistingConnectionInfo->GetPort() == lConnecInfo->GetPort() )
                                {
                                    lExistingDevice = true;
                                    delete lConnecInfo;
                                    break;
                                }
                            }

                            if( !lExistingDevice )
                            {
                                lResultList.push_back( lConnecInfo );
                            }
                        }
                        else
                        {
                            // Don't throw an error: just ignore this packet and continue to scan other online devices...
                            //throw std::runtime_error("Unknown identify echo received");
                        }

                    }
                    else if( lResult >= 0 )
                    {
                        // Don't throw an error: just ignore this packet and continue to scan other online devices...
                        //throw std::runtime_error("Unknown packet received");
                    }
                }
            }
        }

    }
    while( lSelRet > 0 );

    return lResultList;
}

// *****************************************************************************
// Function: LdEthernet::GetDeviceList
//
/// \brief   Scan for ethernet devices on network
///             The function release the ownership of the returned objects.
///
/// \return  Vector of LeddarConnection::LdConnectionInfo* with all the devices that answered
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
std::vector<LeddarConnection::LdConnectionInfo *>
LeddarConnection::LdEthernet::GetDeviceList( uint32_t aTimeoutms, bool aWideBroadcast )
{
    std::vector< std::pair<SOCKET, unsigned long> > lInterfaces;
    lInterfaces = LeddarConnection::LdEthernet::OpenScanRequestSockets();

    LeddarConnection::LdEthernet::GetDevicesListSendRequest( lInterfaces, aWideBroadcast );
    LeddarUtils::LtTimeUtils::WaitBlockingMicro( aTimeoutms * 1000 );
    std::vector<LeddarConnection::LdConnectionInfo *> lConnectionList = LeddarConnection::LdEthernet::GetDevicesListReadAnswer( lInterfaces );

    for( size_t i = 0; i < lInterfaces.size(); ++i )
    {
#ifdef _WIN32

        if( lInterfaces[i].first != INVALID_SOCKET )
#else
        if( lInterfaces[i].first >= 0 )
#endif
            lInterfaces[i].first = LeddarConnection::LdEthernet::CloseSocket( lInterfaces[i].first );
    }

    lInterfaces.clear();
    return lConnectionList;
}

// *****************************************************************************
// Function: LdEthernet::SendTo
//
/// \brief   Send data to UDP socket
///
/// \param   aIpAddress Ip address to send the data.
/// \param   aPort      Port to send the data.
/// \param   aData      Data to send.
/// \param   aSize      Size of data to send.
///
/// \exception LtComException when the sendTo fail
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

void
LeddarConnection::LdEthernet::SendTo( const std::string &aIpAddress, uint16_t aPort, const uint8_t *aData, uint32_t aSize )
{
    sockaddr_in lAddress;

    lAddress.sin_family = AF_INET;
    lAddress.sin_port = htons( aPort );
#ifdef WIN32
    lAddress.sin_addr.S_un.S_addr = LeddarUtils::LtStringUtils::StringToIp4Addr( aIpAddress );
#else
    lAddress.sin_addr.s_addr = LeddarUtils::LtStringUtils::StringToIp4Addr( aIpAddress );
#endif

    const int lResult = sendto( mUDPSocket, ( const char * )aData, aSize, 0, ( const sockaddr * )&lAddress, sizeof( lAddress ) );

    if( lResult == SOCKET_ERROR )
    {
        throw LeddarException::LtComException( "Error to send UDP data on address: " + aIpAddress  + " on port: " + LeddarUtils::LtStringUtils::IntToString(
                aPort ) );
    }
}

// *****************************************************************************
// Function: LdEthernet::ReceiveFrom
//
/// \brief   Receive data from UDP socket
///
/// \param   aIpAddress The IPV4 address of the sender of the received packet.
/// \param   aPort      The port number where the data originated.
/// \param   aData      Buffer to receive the data.
/// \param   aSize      Size expected to receive.
///
/// \return  Number of bytes received
///
/// \exception LtComException when the recvfrom fail
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
uint32_t
LeddarConnection::LdEthernet::ReceiveFrom( std::string &aIpAddress, uint16_t &aPort, uint8_t *aData, uint32_t aSize )
{
    sockaddr_in lAddress;
    socklen_t lAddressSize = sizeof( lAddress );

    const int32_t lResult = recvfrom( mUDPSocket, ( char * )aData, aSize, 0, ( sockaddr * )&lAddress, &lAddressSize );


#ifdef WIN32
    aIpAddress = LeddarUtils::LtStringUtils::Ip4AddrToString( lAddress.sin_addr.S_un.S_addr );
#else
    aIpAddress = LeddarUtils::LtStringUtils::Ip4AddrToString( lAddress.sin_addr.s_addr );
#endif
    aPort = ntohs( lAddress.sin_port );

    if( lResult == SOCKET_ERROR )
    {
        throw LeddarException::LtComException( "Error to receive UDP data on address: " + aIpAddress + " on port: "
                                               + LeddarUtils::LtStringUtils::IntToString( aPort ) + " ("
                                               + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) + ")" );
    }
    else if( lResult == 0 )
    {
        throw LeddarException::LtComException( "Error in Receive ( connection close ).", true );
    }


    return static_cast<uint32_t>( lResult );
}

// *****************************************************************************
// Function: LdEthernet::FlushBuffer
//
/// \brief   Flush all data in the TCP input buffer
///
/// \exception LtComException when the setsockopt fail
///
/// \author  David Levy
///
/// \since   December 2017
// *****************************************************************************
void
LeddarConnection::LdEthernet::FlushBuffer( void )
{
    //Set a temporary low timeout to flush the buffer
#ifdef _WIN32
    uint32_t lTimeout = 1;
    uint32_t lSockOptResult = setsockopt( mSocket, SOL_SOCKET, SO_RCVTIMEO, ( char * )&lTimeout, sizeof( lTimeout ) );

    if( lSockOptResult == SOCKET_ERROR )
#else
    struct timeval lTimeout;

    lTimeout.tv_sec = 0;
    lTimeout.tv_usec = 1000;
    int lSockOptResult = setsockopt( mSocket, SOL_SOCKET, SO_RCVTIMEO, &lTimeout, sizeof( lTimeout ) );

    if( lSockOptResult < 0 )
#endif
    {
        throw LeddarException::LtComException( "Failed to set socket option SO_RCVTIMEO (setsockopt): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
    }

    uint8_t lBuffer[512];

    while( recv( mSocket, ( char * )lBuffer, 512, 0 ) > 0 ) {}

    // Set back the receive timeout.
#ifdef _WIN32
    lTimeout = mConnectionInfoEthernet->GetTimeout();
    lSockOptResult = setsockopt( mSocket, SOL_SOCKET, SO_RCVTIMEO, ( char * )&lTimeout, sizeof( lTimeout ) );

    if( lSockOptResult == SOCKET_ERROR )
#else
    lTimeout.tv_sec = mConnectionInfoEthernet->GetTimeout() / 1000;

    lTimeout.tv_usec = ( mConnectionInfoEthernet->GetTimeout() % 1000 ) * 1000;
    lSockOptResult = setsockopt( mSocket, SOL_SOCKET, SO_RCVTIMEO, &lTimeout, sizeof( lTimeout ) );

    if( lSockOptResult < 0 )
#endif
    {
        throw LeddarException::LtComException( "Failed to set socket option SO_RCVTIMEO (setsockopt): " + LeddarUtils::LtSystemUtils::ErrnoToString( LAST_ERROR ) );
    }
}

#endif