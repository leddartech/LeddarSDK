////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdProtocolLeddartechUSB.cpp
///
/// \brief  Implements the ldProtocolLeddartechUSB class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdProtocolLeddartechUSB.h"
#ifdef BUILD_USB

#include "comm/LtComUSBPublic.h"
#include "comm/LtComLeddarTechPublic.h"

#include "LdInterfaceUsb.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"

#include <cstring>

using namespace LeddarConnection;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdProtocolLeddartechUSB::LdProtocolLeddartechUSB( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface )
///
/// \brief  Constructor
///
/// \param          aConnectionInfo Information describing the connection.
/// \param [in,out] aInterface      If non-null, the interface to connect to.
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
LdProtocolLeddartechUSB::LdProtocolLeddartechUSB( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface ) :
    LdProtocolLeddarTech( aConnectionInfo, aInterface ),
    mEndPoint( EP_CONFIG )
{
    mInterfaceUSB = dynamic_cast< LdInterfaceUsb * >( aInterface );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdProtocolLeddartechUSB::LdProtocolLeddartechUSB( const LdConnectionInfo *aConnectionInfo, LdConnection *aProtocol, eEndPoint aEndPoint )
///
/// \brief  Constructor. Use this constructor for other endpoints to share the same connection objects.
///
/// \param          aConnectionInfo Information describing the connection.
/// \param [in,out] aProtocol       If non-null, interface to connect to (with a different protocol)
/// \param          aEndPoint       End point of the protocol.
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
LdProtocolLeddartechUSB::LdProtocolLeddartechUSB( const LdConnectionInfo *aConnectionInfo, LdConnection *aProtocol, eEndPoint aEndPoint ) :
    LdProtocolLeddarTech( aConnectionInfo, aProtocol ),
    mEndPoint( aEndPoint )
{
    mDeviceType = aProtocol->GetDeviceType();
    mIsConnected = aProtocol->IsConnected();
    mInterfaceUSB = dynamic_cast<LdInterfaceUsb *>( aProtocol->GetInterface() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdProtocolLeddartechUSB::~LdProtocolLeddartechUSB( void )
///
/// \brief  Destructor
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
LdProtocolLeddartechUSB::~LdProtocolLeddartechUSB( void )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdProtocolLeddartechUSB::Write( uint32_t aSize )
///
/// \brief  Writes the given size to the device
///
/// \param  aSize   The Size of the data to send.
///
/// \author David Levy
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdProtocolLeddartechUSB::Write( uint32_t aSize )
{
    mInterfaceUSB->Write( mEndPoint, mTransferInputBuffer, aSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdProtocolLeddartechUSB::Read( uint32_t )
///
/// \brief  Reads from device
///
/// \param  (not used)
///
/// \author David Levy
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdProtocolLeddartechUSB::Read( uint32_t /*aSize*/ )
{
    mInterfaceUSB->Read( mEndPoint, mTransferOutputBuffer, mTransferBufferSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdProtocolLeddartechUSB::QueryDeviceInfo( void )
///
/// \brief  Queries device information
///
/// \exception  LeddarException::LtComException Thrown when a there is no interface or the device is not connected, see VerifyConnection.
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdProtocolLeddartechUSB::QueryDeviceInfo( void )
{
    VerifyConnection();

    LtComUSBPublic::LtComUsbIdtAnswerIdentify lInfo;
    LeddarConnection::LdInterfaceUsb *lUsbInterface = dynamic_cast< LdInterfaceUsb * >( mInterface );
    lUsbInterface->ControlTransfert( 0xC0, LtComUSBPublic::LT_COM_USB_SETUP_REQ_CMD_IDENTIFY, reinterpret_cast<uint8_t *>( &lInfo ),
                                     static_cast<uint32_t>( sizeof( lInfo ) ), 1000 );

    mIdentityInfo.mDeviceName = lInfo.mDeviceName;
    mIdentityInfo.mDeviceSerialNumber = lInfo.mSerialNumber;
    mIdentityInfo.mDeviceType = lInfo.mDeviceType;
    mDeviceType = lInfo.mDeviceType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdProtocolLeddartechUSB::ReadAnswer( void )
///
/// \brief  Reads the header of the answer of the previous request
///
/// \exception  LeddarException::LtComException Thrown when a there is no interface or the device is not connected, see VerifyConnection.
/// \exception  LeddarException::LtComException Thrown when we receive an unexpected request code.
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdProtocolLeddartechUSB::ReadAnswer( void )
{
    VerifyConnection();

    LtComLeddarTechPublic::sLtCommAnswerHeader *lHeader = reinterpret_cast<LtComLeddarTechPublic::sLtCommAnswerHeader *>( mTransferOutputBuffer );

    Read( sizeof( LtComLeddarTechPublic::sLtCommAnswerHeader ) );

    if( lHeader->mRequestCode != mRequestCode )
    {
        throw LeddarException::LtComException( "Received a different request code than the request, expected: " + LeddarUtils::LtStringUtils::IntToString(
                mRequestCode ) + " received: " + LeddarUtils::LtStringUtils::IntToString( lHeader->mRequestCode ) );
    }

    mAnswerCode = lHeader->mAnswerCode;
    mMessageSize = lHeader->mAnswerSize - sizeof( LtComUSBPublic::sLtCommAnswerHeader );
    mElementOffset = sizeof( LtComUSBPublic::sLtCommAnswerHeader );
}

#endif