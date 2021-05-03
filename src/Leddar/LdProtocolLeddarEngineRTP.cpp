////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file	Leddar/LdProtocolLeddarEngineRTP.cpp
///
/// \brief	Implements the LeddarEngine UDP data receiver  and send RTP packet to a callback
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LdProtocolLeddarEngineRTP.h"
#include "LdConnectionInfoEthernet.h"
#include "LdInterfaceEthernet.h"
#include "LdRtpPacketReceiver.h"
#include "LdWaveformPacketReceiver.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarConnection::LdProtocolLeddarEngineRTP::LdProtocolLeddarEngineRTP( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface )
///
/// \brief	Constructor
///
/// \author	Alain Ferron
/// \date	January 2021
///
/// \param 		   	aConnectionInfo	Information describing the connection.
/// \param [in,out]	aInterface	   	If non-null, the interface.
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdProtocolLeddarEngineRTP::LdProtocolLeddarEngineRTP( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface )
    : LdConnection( aConnectionInfo, aInterface )
{
    mInterfaceEthernet      = dynamic_cast<LdInterfaceEthernet *>( aInterface );
    mConnectionInfoEthernet = dynamic_cast<const LeddarConnection::LdConnectionInfoEthernet *>( aConnectionInfo );
    SetDeviceType( dynamic_cast<const LdConnectionInfoEthernet *>( aConnectionInfo )->GetDeviceType() );
    TakeOwnerShip( true );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarConnection::LdProtocolLeddarEngineRTP::~LdProtocolLeddarEngineRTP()
///
/// \brief	Destructor
///
/// \author	Alain Ferron
/// \date	January 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdProtocolLeddarEngineRTP::~LdProtocolLeddarEngineRTP() { LdProtocolLeddarEngineRTP::Disconnect(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::Connect( void )
///
/// \brief	UDP socket connection
///
/// \author	Alain Ferron
/// \date	January 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::Connect( void )
{
    if( !mIsConnected )
    {
        mInterfaceEthernet->OpenUDPSocket( mConnectionInfoEthernet->GetPort(), mConnectionInfoEthernet->GetTimeout() );
        mIsConnected = true;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::Disconnect( void )
///
/// \brief	UDP socket disconnection
///
/// \author	Alain Ferron
/// \date	January 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::Disconnect( void )
{
    if( mIsConnected )
    {
        StopAcquisition();
        mInterfaceEthernet->CloseUDPSocket();
        mIsConnected = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::SetRtpPacketCallback( std::function<void( const LdRtpPacketReceiver & )> aProcessRtpPacket )
///
/// \brief	Initialize the callback function pointer
///
/// \author	Alain Ferron
/// \date	January 2021
///
/// \param	aProcessRtpPacket	Callback
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::SetRtpPacketCallback( std::function<void( const LdRtpPacketReceiver & )> aProcessRtpPacket )
{
    mProcessRtpPacket = aProcessRtpPacket;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::SetExceptionCallback( std::function<void( const std::exception_ptr )> aHandleException )
///
/// \brief	Callback, called when the set exception
///
/// \author	Alain Ferron
/// \date	February 2021
///
/// \param	aHandleException	The handle exception.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::SetExceptionCallback( std::function<void( const std::exception_ptr )> aHandleException ) { mHandleException = aHandleException; }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	uint32_t LeddarConnection::LdProtocolLeddarEngineRTP::GetPort()
///
/// \brief	Gets UDP connection port
///
/// \author	Alain Ferron
/// \date	January 2021
///
/// \returns	The port.
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t LeddarConnection::LdProtocolLeddarEngineRTP::GetPort()
{
    auto lPort = mInterfaceEthernet->GetUDPPort();
    return lPort;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::GetDataLoop()
///
/// \brief	Gets data received from UDP connection send the RTP packet to the object callback
///
/// \author	Alain Ferron
/// \date	January 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::GetDataLoop()
{
    while( mAcquisitionning.load() )
    {
        try
        {
            auto nbByteReceived = mInterfaceEthernet->ReceiveFrom( mAddressFrom, mPortFrom, mTransferBuffer, sizeof( mTransferBuffer ) );
            LdRtpPacketReceiver lRtpPacket( mTransferBuffer, nbByteReceived );
            mProcessRtpPacket( lRtpPacket );
        }
        catch( std::exception & )
        {
            mHandleException( std::current_exception() );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::ResetSequence( uint16_t aSequence )
///
/// \brief	Resets the sequence described by aSequence
///
/// \author	Alain Ferron
/// \date	February 2021
///
/// \param	aSequence	The sequence.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::ResetSequence( uint16_t aSequence )
{
    mBaseSeq  = aSequence;
    mMaxSeq   = aSequence;
    mBadSeq   = RTP_SEQ_MOD + 1;
    mCycles   = 0;
    mReceived = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::StartAcquisition()
///
/// \brief	Starts an acquisition
///
/// \author	Alain Ferron
/// \date	January 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::StartAcquisition()
{
    if( !mAcquisitionning.exchange( true ) )
    {
        mDataThread = std::thread( &LeddarConnection::LdProtocolLeddarEngineRTP::GetDataLoop, this );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::StopAcquisition()
///
/// \brief	Stops an acquisition
///
/// \author	Alain Ferron
/// \date	January 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::StopAcquisition()
{
    if( mAcquisitionning.exchange( false ) )
    {
        mDataThread.join();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::ResetStats()
///
/// \brief	Resets the statistics
///
/// \author	Alain Ferron
/// \date	March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::ResetStats() { mResetStatsRequest.store( true ); }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	uint32_t LeddarConnection::LdProtocolLeddarEngineRTP::GetLostPacketCount() const
///
/// \brief	Gets lost packet count
///
/// \returns	The lost packet count.
///
/// \author	Alain Ferron
/// \date	February 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t LeddarConnection::LdProtocolLeddarEngineRTP::GetLostPacketCount() const
{
    auto lExtendedMax    = mCycles + mMaxSeq;
    auto lExpected       = lExtendedMax - mBaseSeq + 1;
    uint32_t lPacketLost = 0;
    if( mReceived < lExpected )
    {
        lPacketLost = lExpected - mReceived;
    }
    return lPacketLost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	void LeddarConnection::LdProtocolLeddarEngineRTP::InitSequence( uint16_t aSequence )
///
/// \brief	Initializes the sequence
///
/// \author	Alain Ferron
/// \date	February 2021
///
/// \param	aSequence	The sequence.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolLeddarEngineRTP::InitSequence( uint16_t aSequence )
{
    mProbation = mMinSequential;
    mMaxSeq    = aSequence - 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	bool LeddarConnection::LdProtocolLeddarEngineRTP::UpdateSequence( uint16_t aSequence )
///
/// \brief	Validate a source which is declared valid only after a given min sequential. A sequence state struct is updated to retrieve some
/// 	statistics
///
/// \author	Alain Ferron
/// \date	February 2021
///
/// \param	aSequence	The sequence.
///
/// \returns	True if valid sequence, false if invalid sequence.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarConnection::LdProtocolLeddarEngineRTP::UpdateSequence( uint16_t aSequence )
{
    if( mResetStatsRequest.exchange( false ) )
    {
        ResetSequence( mMaxSeq );
    }

    uint16_t lDelta = aSequence - mMaxSeq;

    // Source is not valid until mMinSequential packets with
    // sequential sequence numbers have been received.
    if( mProbation )
    {
        // Packet is in sequence
        if( aSequence == static_cast<uint16_t>( mMaxSeq + 1 ) )
        {
            mProbation--;
            mMaxSeq = aSequence;
            if( mProbation == 0 )
            {
                ResetSequence( aSequence );
                mReceived++;
                return true;
            }
        }
        else
        {
            mProbation = mMinSequential - 1;
            mMaxSeq    = aSequence;
        }
        return false;
    }
    // Sequence increase lower than maximum
    else if( lDelta < mMaxDropOut )
    {
        // in order, with permissible gap
        if( aSequence < mMaxSeq )
        {
            // Sequence number wrapped - count another 64K cycle.
            mCycles += RTP_SEQ_MOD;
        }
        mMaxSeq = aSequence;
    }
    // Sequence is lower than minimum accepted
    else if( lDelta <= RTP_SEQ_MOD - mMaxDisorder )
    {
        // The sequence number made a very large jump
        if( aSequence == mBadSeq )
        {
            // Two sequential packets, assume the sequence has changed without being notified
            ResetSequence( aSequence );
        }
        else
        {
            mBadSeq = ( aSequence + 1 ) & ( RTP_SEQ_MOD - 1 );
            return false;
        }
    }
    else
    {
        // Duplicate or reordered packet
    }
    mReceived++;

    return true;
}
