////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdRtpPacket.cpp
///
/// \brief	Implements the RTP packet class
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LdRtpPacket.h"
#include <cstring>
////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarConnection::LdRtpPacket::LdRtpPacket( const uint8_t *aPacket, size_t aLength )
///
/// \brief	Constructor
///
/// \author	Alain Ferron
/// \date	February 2021
///
/// \param	aPacket	The packet.
/// \param	aLength	The length.
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdRtpPacket::LdRtpPacket( const uint8_t *aPacket, size_t aLength )
    : mBuffer( const_cast<uint8_t *>( aPacket ) )
    , mSize( aLength )
{
    auto lCsrcCount = GetHeader()->mCsrcCount;
    auto lCsrcSize  = lCsrcCount << 2;
    mHeaderSize     = sizeof( RTPHeader );
    mHeaderSize += lCsrcSize;

    if( GetHeader()->mPadding )
    {
        aLength -= aPacket[aLength - 1];
    }

    mPayLoadSize = aLength - mHeaderSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarConnection::LdRtpPacket::LdRtpPacket( size_t aHeaderSize, size_t aPayloadSize )
///
/// \brief	Constructor
///
/// \author	Alain Ferron
/// \date	February 2021
///
/// \param	aHeaderSize 	Size of the header.
/// \param	aPayloadSize	Size of the payload.
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdRtpPacket::LdRtpPacket( size_t aHeaderSize, size_t aPayloadSize )
    : mPayLoadSize( aPayloadSize )
    , mHeaderSize( aHeaderSize )
    , mOwnedBuffer( true )
{
    mSize   = aHeaderSize + aPayloadSize;
    mBuffer = new uint8_t[mSize];

    GetHeader()->mPadding = 0;

    memset( mBuffer, 0, mHeaderSize );
}
