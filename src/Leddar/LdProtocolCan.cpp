////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdProtocolCan.cpp
///
/// \brief  Implements the LdProtocolCan class
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdProtocolCan.h"
#ifdef BUILD_CANBUS

#include "comm/Canbus/LtComM16Canbus.h"
#include "comm/Canbus/LtComVuCanbus.h"

#include "LtTimeUtils.h"
#include "LtStringUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdProtocolCan::LdProtocolCan( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface )
///
/// \brief  Constructor
///
/// \param          aConnectionInfo Information describing the connection.
/// \param [in,out] aInterface      If non-null, the interface.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdProtocolCan::LdProtocolCan( const LdConnectionInfo *aConnectionInfo, LdConnection *aInterface, bool aIsM16 ) : LdConnection( aConnectionInfo, aInterface ),
    mIsM16( aIsM16 ),
    mIsStreaming( false )
{
    mInterfaceCAN = dynamic_cast<LeddarConnection::LdInterfaceCan *>( aInterface );
    mInterfaceCAN->ConnectSignal( this, LeddarCore::LdObject::NEW_DATA );

    if( mInterfaceCAN->IsConnected() )
    {
        EnableStreamingDetections( false ); //If the connection crashed when we were streaming, we need to stop it when reconnecting, else GetConstant / config wont work
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdProtocolCan::~LdProtocolCan()
///
/// \brief  Destructor
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdProtocolCan::~LdProtocolCan()
{
    if( mInterfaceCAN->IsMaster() )
        LdProtocolCan::Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolCan::SendRequest( const std::vector<uint8_t> &aData )
///
/// \brief  Sends a request to the sensor
///
/// \param  aData   The data to send.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolCan::SendRequest( const std::vector<uint8_t> &aData )
{
    mInterfaceCAN->Write( dynamic_cast<const LeddarConnection::LdConnectionInfoCan *>( mConnectionInfo )->GetBaseIdRx(), aData );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolCan::SendRequest( const LtComCanBus::sCanData &aData )
///
/// \brief  Sends a request to the sensor
///
/// \param  aData   The data to send.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolCan::SendRequest( const LtComCanBus::sCanData &aData )
{
    std::vector<uint8_t> lData( aData.mFrame.mRawData, aData.mFrame.mRawData + sizeof( aData.mFrame.mRawData ) );
    SendRequest( lData );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarConnection::LdProtocolCan::SendRequestAndWaitForAnswer( const std::vector<uint8_t> &aData )
///
/// \brief  Sends a request and wait for answer
///
/// \param  aData       The data.
///
/// \return True if new data are received, else false
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarConnection::LdProtocolCan::SendRequestAndWaitForAnswer( const std::vector<uint8_t> &aData )
{
    return mInterfaceCAN->WriteAndWaitForAnswer( dynamic_cast<const LeddarConnection::LdConnectionInfoCan *>( mConnectionInfo )->GetBaseIdRx(), aData );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarConnection::LdProtocolCan::SendRequestAndWaitForAnswer( const LtComCanBus::sCanData &aData )
///
/// \brief  Sends a request and wait for an answer. (the answer is not necessarily the expected one if a sensor is streaming, or there is multiple sensors)
///
/// \param  aData       The data.
///
/// \return True if new data are received, else false
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarConnection::LdProtocolCan::SendRequestAndWaitForAnswer( const LtComCanBus::sCanData &aData )
{
    std::vector<uint8_t> lData( aData.mFrame.mRawData, aData.mFrame.mRawData + sizeof( aData.mFrame.mRawData ) );
    return SendRequestAndWaitForAnswer( lData );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LtComCanBus::sCanData LeddarConnection::LdInterfaceCan::GetNextConfigData( void )
///
/// \brief  Gets the next configuration data frame
///
/// \return The next configuration data frame.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LtComCanBus::sCanData LeddarConnection::LdProtocolCan::GetNextConfigData()
{
    if( !mBufferConfig.empty() )
    {
        LtComCanBus::sCanData lNextData = mBufferConfig.front();
        mBufferConfig.pop();

        if( lNextData.mFrame.Cmd.mArg[0] == 0xFF && lNextData.mFrame.Cmd.mArg[1] == 0xFF && lNextData.mFrame.Cmd.mArg[2] == 0xFF &&
                lNextData.mFrame.Cmd.mArg[3] == 0xFF && lNextData.mFrame.Cmd.mArg[4] == 0xFF && lNextData.mFrame.Cmd.mArg[5] == 0xFF )
        {
            throw std::runtime_error( "Sensor failed to process command:" + LeddarUtils::LtStringUtils::IntToString( lNextData.mFrame.Cmd.mCmd, 16 ) );
        }

        return lNextData;
    }
    else
    {
        return LtComCanBus::sCanData();
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LtComCanBus::sCanData LeddarConnection::LdInterfaceCan::GetNextDetectionData( void )
///
/// \brief  Gets the next detection data frame
///
/// \return The next detection data frame.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LtComCanBus::sCanData LeddarConnection::LdProtocolCan::GetNextDetectionData()
{
    if( !mBufferDetections.empty() )
    {
        LtComCanBus::sCanData lNextData = mBufferDetections.front();
        mBufferDetections.pop();
        return lNextData;
    }
    else
    {
        return LtComCanBus::sCanData();
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LtComCanBus::sCanData LeddarConnection::LdProtocolCan::GetValue( uint16_t aCommandId, uint8_t aCommandArg )
///
/// \brief  Gets a value from the sensor using provided command / arg
///
/// \exception  std::runtime_error  Raised when the sensors does not answer, and we receive and unexpected answer
///
/// \param  aCommandId  Identifier for the command.
/// \param  aCommandArg The command argument.
///
/// \return The value.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LtComCanBus::sCanData LeddarConnection::LdProtocolCan::GetValue( uint8_t aCommandId, uint8_t aCommandArg )
{
    std::vector<uint8_t> lData( 8 );
    lData[0] = aCommandId;
    lData[1] = aCommandArg;

    if( !SendRequestAndWaitForAnswer( lData ) )
    {
        throw std::runtime_error( "Couldnt get any answer from sensor" );
    }

    LtComCanBus::sCanData lConfigData = GetNextConfigData();

    if( ( mIsM16 && lConfigData.mFrame.Cmd.mCmd != aCommandId + LtComCanBus::M16_ANSWER_ID_OFFSET ) ||
            ( !mIsM16 && lConfigData.mFrame.Cmd.mCmd != aCommandId ) ||
            lConfigData.mFrame.Cmd.mSubCmd != aCommandArg )
    {
        throw std::runtime_error( "Got erroneous data. Received " + LeddarUtils::LtStringUtils::IntToString( lConfigData.mFrame.Cmd.mCmd ) + "/" +
                                  LeddarUtils::LtStringUtils::IntToString( lConfigData.mFrame.Cmd.mSubCmd ) );
    }

    return lConfigData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolCan::SetValue( const LtComCanBus::sCanData &aCommand )
///
/// \brief  Sets a value using provided CAN frame
///
/// \exception  std::runtime_error  Raised when the sensors does not answer, and we receive and unexpected answer
///
/// \param  aCommand    The command.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolCan::SetValue( const LtComCanBus::sCanData &aCommand )
{
    if( !SendRequestAndWaitForAnswer( aCommand ) )
    {
        throw std::runtime_error( "Couldnt get any answer from sensor" );
    }

    LtComCanBus::sCanData lConfigData = GetNextConfigData();

    if( ( mIsM16 && lConfigData.mFrame.Cmd.mCmd != aCommand.mFrame.Cmd.mCmd + LtComCanBus::M16_ANSWER_ID_OFFSET ) ||
            ( !mIsM16 && lConfigData.mFrame.Cmd.mCmd != aCommand.mFrame.Cmd.mCmd ) ||
            lConfigData.mFrame.Cmd.mSubCmd != aCommand.mFrame.Cmd.mSubCmd )
    {
        throw std::runtime_error( "Got erroneous data. Received " + LeddarUtils::LtStringUtils::IntToString( lConfigData.mFrame.Cmd.mCmd ) + "/" +
                                  LeddarUtils::LtStringUtils::IntToString( lConfigData.mFrame.Cmd.mSubCmd ) );
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarConnection::LdProtocolCan::ReadConfigAnswer( void )
///
/// \brief  Check if we have received a configuration data.
///
/// \return True if we received new data, else false.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarConnection::LdProtocolCan::ReadConfigAnswer( void )
{
    if( mBufferConfig.empty() )
    {
        mInterfaceCAN->Read();

        if( mBufferConfig.empty() )
        {
            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarConnection::LdProtocolCan::ReadDetectionAnswer( void )
///
/// \brief  Check if we have received a detection data.
///
/// \return True if we received new data, else false.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarConnection::LdProtocolCan::ReadDetectionAnswer( void )
{
    if( mBufferDetections.empty() )
    {
        mInterfaceCAN->Read();

        if( mBufferDetections.empty() )
        {
            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolCan::Callback( LdObject *aSender, const SIGNALS aSignal, void *aCanData )
///
/// \brief  Callback. Supports NEW_DATA signal to store new data in the correct buffer
///
/// \param [in,out] aSender     The sender.
/// \param          aSignal     The signal.
/// \param [in,out] aCanData    If non-null, information describing the can.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolCan::Callback( LdObject *aSender, const SIGNALS aSignal, void *aCanData )
{
    if( aSender == mInterfaceCAN && aSignal == LeddarCore::LdObject::NEW_DATA )
    {
        if( mIsM16 )
        {
            LtComCanBus::sCanData &lCanData = *reinterpret_cast<LtComCanBus::sCanData *>( aCanData );

            if( lCanData.mId == dynamic_cast<const LeddarConnection::LdConnectionInfoCan *>( mInterface->GetConnectionInfo() )->GetBaseIdTx() + 1 &&
                    lCanData.mFrame.Cmd.mCmd >= LtComCanBus::M16_ANSWER_ID_OFFSET )
            {
                mBufferConfig.push( lCanData );
            }
            else
            {
                mBufferDetections.push( lCanData );
            }
        }
        else //Vu8
        {
            LtComCanBus::sCanData &lCanData = *reinterpret_cast<LtComCanBus::sCanData *>( aCanData );

            if( lCanData.mId == dynamic_cast<const LeddarConnection::LdConnectionInfoCan *>( mInterface->GetConnectionInfo() )->GetBaseIdTx() )
            {
                mBufferConfig.push( lCanData );
            }
            else
            {
                mBufferDetections.push( lCanData );
            }
        }

    }
    else
    {
        throw std::logic_error( "Unhandled signal" );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdProtocolCan::EnableStreamingDetections( bool aEnable, uint8_t aFlag )
///
/// \brief  Enables / disabled the streaming of the detections
///
/// \param  aEnable True to enable, false to disable.
/// \param  aFlag   Optional flag, depending on sensor configuration
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdProtocolCan::EnableStreamingDetections( bool aEnable, uint8_t aFlag )
{
    LtComCanBus::sCanData lConfigData = {};
    lConfigData.mFrame.Cmd.mSubCmd = aFlag;

    if( mIsM16 ) //M16 doesnt answer to start / stop streaming
    {
        lConfigData.mFrame.Cmd.mCmd = aEnable ? LtComCanBus::M16_CMD_START_SEND_DETECT : LtComCanBus::M16_CMD_STOP_SEND_DETEC;
        SendRequest( lConfigData );
    }
    else // But Vu does answer
    {
        lConfigData.mFrame.Cmd.mCmd = aEnable ? LtComCanBus::VU_CMD_START_SEND_DETECT : LtComCanBus::VU_CMD_STOP_SEND_DETEC;

        uint8_t lCount = 5;

        while( true )
        {
            SendRequestAndWaitForAnswer( lConfigData );

            if( GetNextConfigData().mId != 0 )
                break;

            if( --lCount == 0 )
            {
                throw std::runtime_error( "Couldnt change streaming detection status - No answer from sensor" );
            }
        }
    }

    mIsStreaming = aEnable;
    LeddarUtils::LtTimeUtils::Wait( 10 );
}

#endif
