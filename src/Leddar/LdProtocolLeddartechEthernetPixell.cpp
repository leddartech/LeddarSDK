////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdProtocolLeddartechEthernetPixell.cpp
///
/// \brief	Implements the LdProtocolLeddartechEthernetPixell class
///
/// Copyright (c) 2021 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdProtocolLeddartechEthernetPixell.h"

#include "LdRtpPacketReceiver.h"
#include "comm/LtComEthernetPublic.h"

#include <cstring>

#if defined( BUILD_ETHERNET )

constexpr uint8_t RTP_PAYLOAD_PIXELL = 0x40;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdProtocolLeddartechEthernetPixell::LdProtocolLeddartechEthernetPixell( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface )
///
/// \brief  Constructor
///
/// \param          aConnectionInfo Information describing the connection.
/// \param [in,out] aInterface      If non-null, the interface.
///
/// \author David Lévy
/// \date   March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdProtocolLeddartechEthernetPixell::LdProtocolLeddartechEthernetPixell( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface )
    : LdProtocolLeddarTech( aConnectionInfo, aInterface )
{
    mInterfaceEthernet = dynamic_cast<LdInterfaceEthernet *>( aInterface );
    SetDeviceType( dynamic_cast<const LdConnectionInfoEthernet *>( aConnectionInfo )->GetDeviceType() );
    mPayLoad.reserve( 200000 ); // Somehow big value to limit the number of resize in normal usage
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdProtocolLeddartechEthernetPixell::~LdProtocolLeddartechEthernetPixell( void )
///
/// \brief  Destructor
///
/// \author David Lévy
/// \date   March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdProtocolLeddartechEthernetPixell::~LdProtocolLeddartechEthernetPixell( void )
{
    if( mIsConnected )
        LdProtocolLeddartechEthernetPixell::Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolLeddartechEthernetPixell::Connect( void )
///
/// \brief  Connects this UDP connection
///
/// \author David Lévy
/// \date   March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddartechEthernetPixell::Connect( void )
{
    // Connect interface
    mInterfaceEthernet->OpenUDPSocket( dynamic_cast<const LeddarConnection::LdConnectionInfoEthernet *>( mConnectionInfo )->GetPort() );
    mIsConnected       = true;
    mRTPFrameIsValid   = false;
    mRTPSequenceNumber = 0;
    mRTPTimestamp      = 0;
    mFirstFrame        = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolLeddartechEthernetPixell::Disconnect( void )
///
/// \brief  Disconnects this UDP connection
///
/// \author David Lévy
/// \date   March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddartechEthernetPixell::Disconnect( void )
{
    if( mIsConnected )
        mInterfaceEthernet->CloseUDPSocket();
    mIsConnected = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolLeddartechEthernetPixell::ReadAnswer( void )
///
/// \brief  Reads next data from sensor
///
/// \author David Lévy
/// \date   March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddartechEthernetPixell::ReadAnswer( void )
{
    VerifyConnection();
    bool lFrameReceived = false;
    mRequestCode        = 0;
    mAnswerCode         = 0;
    mMessageSize        = 0;
    mElementOffset      = 0;
    bool lThrowError    = false;
    constexpr uint16_t lUint16LoopDelta =
        std::numeric_limits<uint16_t>::max() / 100; // Value to handle cases when the sequence number loop on the uint16 and the new number is inferior than the old one

    while( !lFrameReceived && mInterfaceEthernet->SelectUDP( 0 ) )
    {
        auto lSizeRead = Read( 0 ); // Argument is not used in UDP. UDP protocol reads the whole message (opposed to TCP stream)

        LeddarConnection::LdRtpPacketReceiver lRTPPaquet( mTransferOutputBuffer, lSizeRead );

        if( lRTPPaquet.isExtended() )
        {
            throw std::runtime_error( "Extended RTP paquet not supported." );
        }
        if( lRTPPaquet.GetPayloadType() != RTP_PAYLOAD_PIXELL )
        {
            throw std::runtime_error( "Wrong payload type." );
        }

        uint16_t lNewSequenceNumber = lRTPPaquet.GetSequenceNumber();
        uint32_t lNewTimestamp      = lRTPPaquet.GetTimeStamp();
        bool lSetNextFrameValid     = false;

        if( !mRTPFrameIsValid ) // Current frame is invalid, ignore data until we start a new frame
        {
            if( lRTPPaquet.IsMarked() ) // Next paquet is the start of a new frame
            {
                lSetNextFrameValid = true;
            }

            if( mRTPTimestamp != lNewTimestamp && ( lNewSequenceNumber == static_cast<uint16_t>( mRTPSequenceNumber + 2 ) ) )
            {
                // We missed IsMarked, but we missed only one frame and the timestamp changed, so its the first paquet of a new frame
                mRTPFrameIsValid = true;
                mPayLoad.clear();
                mRTPSequenceNumber =
                    lNewSequenceNumber - 1; // Small hack so we dont skip current data because we missed previous frame, because we know its the beginning of a new frame
            }
        }

        if( lNewSequenceNumber < mRTPSequenceNumber && lNewSequenceNumber > lUint16LoopDelta )
        {
            // Old data, just ignore it
        }
        else if( !mRTPFrameIsValid || lNewSequenceNumber != static_cast<uint16_t>( mRTPSequenceNumber + 1 ) )
        {
            // We missed a paquet
            mRTPFrameIsValid = false;
            mPayLoad.clear();

            static uint32_t sLastErrorTimestamp = 0;
            if( lNewTimestamp != sLastErrorTimestamp )
            {
                lThrowError = !mFirstFrame; // only throw the error if not the first frame
                sLastErrorTimestamp = lNewTimestamp;
            }
        }
        else
        {
            mFirstFrame = false;
            // append payload
            auto lPayload = lRTPPaquet.GetPayLoad();
            mPayLoad.insert( mPayLoad.end(), lPayload, lPayload + lRTPPaquet.GetPayLoadSize() );

            if( lRTPPaquet.IsMarked() )
            {
                // End of the frame - Copy the payload to mTransferOutputBuffer so all LeddarTech function work as usual
                if( mPayLoad.size() > mTransferBufferSize )
                {
                    ResizeInternalBuffers( mPayLoad.size() );
                }

                memcpy( mTransferOutputBuffer, mPayLoad.data(), mPayLoad.size() );
                mPayLoad.clear();
                LtComLeddarTechPublic::sLtCommAnswerHeader *lHeader = reinterpret_cast<LtComLeddarTechPublic::sLtCommAnswerHeader *>( mTransferOutputBuffer );
                mRequestCode                                        = lHeader->mRequestCode;
                mAnswerCode                                         = lHeader->mAnswerCode;
                mMessageSize                                        = lHeader->mAnswerSize - sizeof( LtComLeddarTechPublic::sLtCommAnswerHeader );
                mElementOffset                                      = sizeof( LtComLeddarTechPublic::sLtCommAnswerHeader );
                lFrameReceived                                      = true;
            }
        }

        if( lNewSequenceNumber >= static_cast<uint16_t>( mRTPSequenceNumber + 1 ) ||
            ( lNewSequenceNumber < lUint16LoopDelta && mRTPSequenceNumber > std::numeric_limits<uint16_t>::max() - lUint16LoopDelta ) )
        {
            mRTPSequenceNumber = lNewSequenceNumber;
            mRTPTimestamp      = lNewTimestamp;
        }

        if( lSetNextFrameValid )
            mRTPFrameIsValid = true;
    }

    if( lThrowError )
        throw std::runtime_error( "Missed a frame " );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolLeddartechEthernetPixell::Read( uint32_t )
///
/// \brief  Reads next UDP paquet
///
/// \param  parameter1  Argument is not used in UDP. UDP protocol reads the whole message (opposed to TCP stream)
///
/// \author David Lévy
/// \date   March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t LeddarConnection::LdProtocolLeddartechEthernetPixell::Read( uint32_t )
{
    uint16_t lPortFrom       = 0;
    std::string lAddressFrom = "";
    return mInterfaceEthernet->ReceiveFrom( lAddressFrom, lPortFrom, mTransferOutputBuffer, mTransferBufferSize );
}

#endif