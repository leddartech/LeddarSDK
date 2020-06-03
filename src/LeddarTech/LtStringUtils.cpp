////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   LeddarTech/LtStringUtils.cpp
///
/// \brief  Utilities for std::string
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LtStringUtils.h"
#include "LtExceptions.h"

#include <cerrno>
#include <climits>
#include <stdlib.h> //TODO use c++ lib when migrating to c++11
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cmath>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "iphlpapi.lib" )
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <fcntl.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn int64_t LeddarUtils::LtStringUtils::StringToInt( const std::string &aData, int aBase )
///
/// \brief  Convert string to int
///
/// \exception  std::overflow_error     The value exceed maximum of Int value.
/// \exception  std::underflow_error    The value is under the minimum of Int value.
/// \exception  std::invalid_argument   Invalid input string, no conversion could be performed.
///
/// \param  aData   String to convert.
/// \param  aBase   Base of the number.
///
/// \returns    The converted value.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
int64_t
LeddarUtils::LtStringUtils::StringToInt( const std::string &aData, int aBase )
{
    int64_t lResultValue = 0;

    // Use the old C way to convert string to int
    errno = 0;
    char *lEnd;
    lResultValue = strtoll( aData.c_str(), &lEnd, aBase );

    if( errno == ERANGE && lResultValue == LLONG_MAX )
    {
        throw std::overflow_error( "Number over maximum possible value." );
    }
    else if( errno == ERANGE && lResultValue == LLONG_MIN )
    {
        throw std::underflow_error( "Number under minimum possible value." );
    }

    if( errno != 0 || *lEnd != '\0' )
    {
        throw std::invalid_argument( "Invalid input string, no conversion could be performed." );
    }

    return lResultValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint64_t LeddarUtils::LtStringUtils::StringToUInt( const std::string &aData, int aBase )
///
/// \brief  Convert string to unsigned int
///
/// \exception  std::overflow_error     The value exceed maximum of Int value.
/// \exception  std::invalid_argument   Invalid input string, no conversion could be performed.
///
/// \param  aData   String to convert.
/// \param  aBase   Base of the number.
///
/// \returns    The converted value.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t
LeddarUtils::LtStringUtils::StringToUInt( const std::string &aData, int aBase )
{
    uint64_t lResultValue = 0;

    // Use the old C way to convert string to int
    errno = 0;
    char *lEnd;
    lResultValue = strtoull( aData.c_str(), &lEnd, aBase );

    if( errno == ERANGE && lResultValue == ULLONG_MAX )
    {
        throw std::overflow_error( "Number over maximum possible value." );
    }

    if( errno != 0 || *lEnd != '\0' )
    {
        throw std::invalid_argument( "Invalid input string, no conversion could be performed." );
    }

    return lResultValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn float LeddarUtils::LtStringUtils::StringToFloat( const std::string &aData )
///
/// \brief  Convert string to float
///
/// \exception  std::invalid_argument   Invalid input string, no conversion could be performed.
///
/// \param  aData   String to convert.
///
/// \return The converted value.
///
/// \author Patrick Boulay
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
float
LeddarUtils::LtStringUtils::StringToFloat( const std::string &aData )
{
    char *lEnd;
    float lResult = std::strtof( aData.c_str(), &lEnd );

    if( lEnd == aData.c_str() )
    {
        throw std::invalid_argument( std::string( "Invalid argument for function: " ) + aData );
    }

    return lResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarUtils::LtStringUtils::HexStringToByteArray( const std::string &aHexString, uint8_t *aHexByte )
///
/// \brief  Convert a string of hex (ex: FE01) to an array of byte (ex: 0xFE01 )
///
/// \param          aHexString  Hexadecimal string to convert.
/// \param [out]    aHexByte    Array of bytes.
///
/// \author Patrick Boulay
/// \date   December 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarUtils::LtStringUtils::HexStringToByteArray( const std::string &aHexString, uint8_t *aHexByte )
{
    const uint32_t lFinalIndex = static_cast<uint32_t>( aHexString.size() / 2 ) - 1;

    for( uint32_t i = 0; i < aHexString.size(); i += 2 )
    {
        aHexByte[ lFinalIndex - ( i / 2 ) ] = static_cast<uint8_t>( LeddarUtils::LtStringUtils::StringToUInt( aHexString.substr( i, 2 ), 16 ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::string LeddarUtils::LtStringUtils::ByteArrayToHexString( const uint8_t *aHexByte, uint32_t aLength )
///
/// \brief  Convert an array of bytes to a string
///
/// \param  aHexByte    Array of hexadecimal bytes.
/// \param  aLength     Size of the array of bytes
///
/// \return A string containing the hex values
///
/// \author Patrick Boulay
/// \date   December 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string
LeddarUtils::LtStringUtils::ByteArrayToHexString( const uint8_t *aHexByte, uint32_t aLength )
{
    std::stringstream lResult;

    for( int i = aLength; i > 0; --i )
    {
        lResult << std::setfill( '0' ) << std::setw( 2 ) << std::hex << ( uint32_t )aHexByte[i - 1];
    }

    return lResult.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LeddarUtils::LtStringUtils::StringToIp4Addr( const std::string &aIpAddrStr )
///
/// \brief  Convert an IPv4 string to a uint32_t (network byte order).
///
/// \exception  LeddarException::LtInfoException    Thrown when conversion fail.
///
/// \param  aIpAddrStr  IP address in format (xxx.xxx.xxx.xxx)
///
/// \return Returns an IPv4 address in long (network byte order).
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t
LeddarUtils::LtStringUtils::StringToIp4Addr( const std::string &aIpAddrStr )
{
    uint32_t lIPValue = 0;
    int lRes = inet_pton( AF_INET, aIpAddrStr.c_str(), &lIPValue );

    if( 0 == lRes )
        throw LeddarException::LtInfoException( "Invalid IP string" );
    else if( lRes < 0 )
        throw LeddarException::LtInfoException( "Error converting IP String - Error = " + errno );

    return lIPValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::string LeddarUtils::LtStringUtils::Ip4AddrToString( uint32_t aIpAddr )
///
/// \brief  Convert a uint32_t (network byte order) to an IPv4 string.
///
/// \exception  LeddarException::LtInfoException    Thrown when conversion fail.
///
/// \param  aIpAddr IP address uint32_t format (network byte order)
///
/// \return String in xxx.xxx.xxx.xxx format.
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string
LeddarUtils::LtStringUtils::Ip4AddrToString( uint32_t aIpAddr )
{
    char lIP[INET_ADDRSTRLEN];

    if( inet_ntop( AF_INET, &aIpAddr, lIP, INET_ADDRSTRLEN ) == nullptr )
        throw LeddarException::LtInfoException( "Invalid IP value - Error = " + errno );

    return lIP;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarUtils::LtStringUtils::Ip4PortStringToValues( const std::string &aIp4Port, std::string *aIp, uint16_t *aPort )
///
/// \brief  Convert an IPV4 string IP:port to an IP string and a port uint16
///
/// \exception  std::overflow_error Raised when an overflow error condition occurs.
/// \exception  std::logic_error    Raised when a logic error condition occurs.
///
/// \param          aIp4Port    The IPv4 and port string.
/// \param [out]    aIp         If non-null, the IPv4.
/// \param [out]    aPort       If non-null, the port.
///
/// \author David Levy
/// \date   February 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarUtils::LtStringUtils::Ip4PortStringToValues( const std::string &aIp4Port, std::string *aIp, uint16_t *aPort )
{
    size_t colonPos = aIp4Port.find( ':' );

    if( colonPos != std::string::npos )
    {
        *aIp = aIp4Port.substr( 0, colonPos );
        std::string portPart = aIp4Port.substr( colonPos + 1 );
        uint64_t lPort = StringToUInt( portPart, 10 );

        if( lPort > UINT16_MAX )
            throw std::overflow_error( "Port is too big." );

        *aPort = static_cast< uint16_t >( lPort );
    }
    else
    {
        throw std::logic_error( "Wrong format." );
    }
}

#ifdef _WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::string LeddarUtils::LtStringUtils::Utf8Encode( const std::wstring &lWstr )
///
/// \brief  Convert a windows widechar array to multibyte array (UTF8)
///
/// \param  lWstr   Windows (unicode) wstring
///
/// \return UTF8 encoded string
///
/// \author David Levy
/// \date   October 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string
LeddarUtils::LtStringUtils::Utf8Encode( const std::wstring &lWstr )
{
    if( lWstr.empty() )
        return std::string();

    int size_needed = WideCharToMultiByte( CP_UTF8, 0, &lWstr[0], ( int )lWstr.size(), NULL, 0, NULL, NULL );

    std::string lUtf8Str( size_needed, 0 );
    WideCharToMultiByte( CP_UTF8, 0, &lWstr[0], ( int )lWstr.size(), &lUtf8Str[0], size_needed, NULL, NULL );

    return lUtf8Str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::wstring LeddarUtils::LtStringUtils::Utf8Decode( const std::string &lStr )
///
/// \brief  Convert multibyte array (UTF8) to a windows widechar array
///
/// \param  lStr    UTF8 encoded string.
///
/// \return Windows (unicode) wstring
///
/// \author David Levy
/// \date   October 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring
LeddarUtils::LtStringUtils::Utf8Decode( const std::string &lStr )
{
    if( lStr.empty() )
        return std::wstring();

    int size_needed = MultiByteToWideChar( CP_UTF8, 0, &lStr[0], ( int )lStr.size(), NULL, 0 );

    std::wstring lwStr( size_needed, 0 );
    MultiByteToWideChar( CP_UTF8, 0, &lStr[0], ( int )lStr.size(), &lwStr[0], size_needed );
    return lwStr;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::vector<std::string> LeddarUtils::LtStringUtils::Split( const std::string &aLine, char aSeparator )
///
/// \brief  Split a string in multiple tokens using a separator.
///
/// \param  aLine       Line to split.
/// \param  aSeparator  The separator.
///
/// \return Vector of string of the splited line.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::string>
LeddarUtils::LtStringUtils::Split( const std::string &aLine, char aSeparator )
{
    std::vector<std::string> lResult;
    const char *lStr = aLine.c_str();

    do
    {
        const char *begin = lStr;

        while( *lStr != aSeparator && *lStr )
            lStr++;

        lResult.push_back( std::string( begin, lStr ) );
    }
    while( 0 != *lStr++ );

    return lResult;
}