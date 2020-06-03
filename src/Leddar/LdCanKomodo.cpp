////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdCanKomodo.cpp
///
/// \brief  Implements the LdCanKomodo class. An implementation of the CAN protocol using Komodo adapter.
///             For multi-sensors setup, one connection is the "master" and behaves as a router
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdCanKomodo.h"
#if defined(BUILD_CANBUS_KOMODO) && defined(BUILD_CANBUS)

#include "komodo.h"
#include "LtStringUtils.h"
#include "LtTimeUtils.h"

const std::string lEventString = "Event error";

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdCanKomodo::LdCanKomodo( const LdConnectionInfoCan *aConnectionInfo, LdConnection *aExistingConnection )
///
/// \brief  Constructor
///
/// \param          aConnectionInfo     Information describing the connection.
/// \param [in,out] aExistingConnection If non-null, the existing connection (for multiple sensor on the same communication port).
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdCanKomodo::LdCanKomodo( const LdConnectionInfoCan *aConnectionInfo, LdConnection *aExistingConnection ) : LdInterfaceCan( aConnectionInfo,
            aExistingConnection ),
    mHandle( 0 )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdCanKomodo::~LdCanKomodo()
///
/// \brief  Destructor
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdCanKomodo::~LdCanKomodo()
{
    if( mMaster == nullptr && mHandle != 0 )
    {
        LdCanKomodo::Disconnect(); //We dont want virtual function in destructor
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdCanKomodo::Connect( void )
///
/// \brief  Connects this object
///
/// \exception  std::logic_error    Raised when a called from a sensor that does not own the
///                                 connection.
/// \exception  std::runtime_error  Raised when already connected.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdCanKomodo::Connect( void )
{
    if( mMaster != nullptr )
    {
        throw std::logic_error( "Only the \"master\" sensor can connect" );
    }

    if( mHandle != 0 )
    {
        throw std::runtime_error( "Already connected" );
    }

    const LdConnectionInfoCan *lInfo = dynamic_cast< const LdConnectionInfoCan *>( GetConnectionInfo() );
    mHandle = km_open( lInfo->GetPortNumber() );

    if( mHandle < 0 )
    {
        int lResult = mHandle;
        mHandle = 0;
        throw std::runtime_error( "Unable to connect: " +  std::string( km_status_string( lResult ) ) );
    }

    int lRes = 0;
    uint32_t lFeatures;

    if( static_cast<km_can_ch_t>( lInfo->GetChannel() ) == KM_CAN_CH_A )
    {
        lFeatures = KM_FEATURE_CAN_A_CONFIG | KM_FEATURE_CAN_A_CONTROL | KM_FEATURE_CAN_A_LISTEN;
    }
    else
    {
        lFeatures = KM_FEATURE_CAN_B_CONFIG | KM_FEATURE_CAN_B_CONTROL | KM_FEATURE_CAN_B_LISTEN;
    }

    lRes = km_acquire( mHandle, lFeatures );

    if( static_cast<uint32_t>( lRes ) != lFeatures )
    {
        Disconnect();
        throw std::runtime_error( "Komodo configuration failed" );
    }

    lRes = km_can_bitrate( mHandle, static_cast<km_can_ch_t>( lInfo->GetChannel() ), lInfo->GetSpeed() * 1000 );

    if( lInfo->GetSpeed() * 1000 != lRes )
    {
        Disconnect();
        throw std::runtime_error( "Cant set baudrate. Requested: " + LeddarUtils::LtStringUtils::IntToString( lInfo->GetSpeed() * 1000 ) + "actual: " +
                                  LeddarUtils::LtStringUtils::IntToString( lRes ) );
    }

    if( km_timeout( mHandle, KM_TIMEOUT_IMMEDIATE ) != KM_OK ) //Non blocking read
    {
        Disconnect();
        throw std::runtime_error( "Cant timeout" );
    }

    km_can_target_power( mHandle, static_cast<km_can_ch_t>( lInfo->GetChannel() ), KM_TARGET_POWER_ON );

    if( km_enable( mHandle ) != KM_OK )
    {
        Disconnect();
        throw std::runtime_error( "Cant enable komodo" );
    }

    LeddarUtils::LtTimeUtils::Wait( 750 ); // Let time to stabilize power up...
    mIsConnected = true;
    WasteEvent();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdCanKomodo::Disconnect( void )
///
/// \brief  Disconnects from the Komodo. Should only be called from the sensor that owns the
///     connection
///
/// \exception  std::logic_error    Raised when a called from a sensor that does not own the connection.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdCanKomodo::Disconnect( void )
{
    if( mMaster != nullptr )
    {
        throw std::logic_error( "Only the \"master\" sensor can disconnect" );
    }

    if( mHandle != 0 )
    {
        km_disable( mHandle );
        const LdConnectionInfoCan *lInfo = dynamic_cast< const LdConnectionInfoCan *>( GetConnectionInfo() );
        km_can_target_power( mHandle, static_cast<km_can_ch_t>( lInfo->GetChannel() ), KM_TARGET_POWER_OFF );
        km_close( mHandle );
        mHandle = 0;
        mIsConnected = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarConnection::LdCanKomodo::Read( const LdInterfaceCan *aRequestingInterface )
///
/// \brief  Reads data from the CANbus
///
/// \exception  std::runtime_error  Raised there is an error reading data.
///
/// \param  aRequestingInterface    The interface that requested the read
///
/// \return True if data is received, else false.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarConnection::LdCanKomodo::Read( const LdInterfaceCan *aRequestingInterface )
{
    if( mMaster != nullptr )
    {
        return mMaster->Read( aRequestingInterface );
    }
    else
    {
        std::vector<uint8_t> lData( 8 );
        km_can_info_t lInfo;
        km_can_packet_t lPacket;

        int lResult = km_can_read( mHandle, &lInfo, &lPacket, static_cast<int>( lData.size() ), &lData[0] );

        if( lResult == KM_CAN_READ_EMPTY )
        {
            return false;
        }

        if( lResult < KM_OK )
        {
            throw std::runtime_error( "Couldnt read answer: " + std::string( km_status_string( lResult ) ) );
        }

        if( lInfo.events != 0 )
        {
            throw std::runtime_error( lEventString );
        }

        if( lPacket.id == 0 ) //Unexpected packet received (probably from vu8)
            return false;

        if( ForwardDataMaster( lPacket.id, lData ) == aRequestingInterface )
            return true;
        else
            return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdCanKomodo::Write( uint16_t aId, std::vector<uint8_t> *aData )
///
/// \brief  Writes provided data to the CANbus
///
/// \exception  std::runtime_error  Raised when there is an error writing data.
///
/// \param          aId     The identifier.
/// \param [in,out] aData   If non-null, the data.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdCanKomodo::Write( uint16_t aId, const std::vector<uint8_t> &aData )
{
    if( mMaster != nullptr )
    {
        mMaster->Write( aId, aData );
    }
    else
    {
        const LdConnectionInfoCan *lInfo = dynamic_cast< const LdConnectionInfoCan *>( GetConnectionInfo() );

        km_can_packet_t lPacket = {};
        lPacket.remote_req = 0;
        lPacket.extend_addr = !lInfo->GetStandardFrameFormat();
        lPacket.dlc = static_cast<uint8_t>( aData.size() );
        lPacket.id = aId;

        uint32_t lArbCount[8] = {0};
        int lResult = km_can_write( mHandle, static_cast<km_can_ch_t>( lInfo->GetChannel() ), 0, &lPacket, static_cast<int>( aData.size() ), &aData[0], lArbCount );

        if( lResult != KM_OK )
        {
            throw std::runtime_error( "Cant write to sensor: " + std::string( km_status_string( lResult ) ) );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarConnection::LdCanKomodo::WriteAndWaitForAnswer( uint16_t aId, const std::vector<uint8_t> &aData )
///
/// \brief  Writes an and wait for an answer.
///
/// \param  aId     The identifier.
/// \param  aData   The data.
///
/// \return True if it succeeds, false if it fails.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool LeddarConnection::LdCanKomodo::WriteAndWaitForAnswer( uint16_t aId, const std::vector<uint8_t> &aData )
{
    //These 3 retry are useful because right after we connect, the first two reads are invalid
    uint8_t lRetry = 3;

    while( lRetry > 0 )
    {
        Write( aId, aData );

        try
        {
            uint16_t lCount = 0;

            while( true )
            {
                if( !Read( this ) )
                {
                    LeddarUtils::LtTimeUtils::Wait( 1 );

                    if( ++lCount > 1000 )
                    {
                        return false;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    return true;
                }
            }
        }
        catch( std::runtime_error &e )
        {
            if( std::string( e.what() ) == lEventString )
            {
                --lRetry;
                continue;
            }
            else
            {
                throw;
            }
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::vector<LeddarConnection::LdConnectionInfo *> LeddarConnection::LdCanKomodo::GetDeviceList( void )
///
/// \brief  Gets the list of devices using on the komodo interface
///
/// \return Vector of connection info for each device
///             Only the port number is valid. Other info are default values
///             If MSB of the port number is set (i.e the value is > 0x80YY) it means the port number is YY but its busy
///         Release ownership of all the pointers
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<LeddarConnection::LdConnectionInfo *> LeddarConnection::LdCanKomodo::GetDeviceList( void )
{
    std::vector<LdConnectionInfo *> lConnecInfo;
    int lNbr = km_find_devices( 0, nullptr );

    if( lNbr == KM_UNABLE_TO_LOAD_LIBRARY )
    {
        throw std::runtime_error( "Couldnt load CAN-komodo library" );
    }
    else if( lNbr < 0 )
    {
        throw std::runtime_error( "Couldnt get CAN-komodo devices: " + std::string( km_status_string( lNbr ) ) );
    }
    else if( lNbr == 0 )
    {
        return lConnecInfo;
    }

    std::vector<uint16_t> lDevices( lNbr + 1 );
    lNbr = ( std::min )( lNbr, km_find_devices( lNbr, &lDevices[0] ) );


    for( int i = 0; i < lNbr; ++i )
    {
        lConnecInfo.push_back( new LeddarConnection::LdConnectionInfoCan( LeddarConnection::LdConnectionInfo::CT_CAN_KOMODO,
                               "CAN " + LeddarUtils::LtStringUtils::IntToString( lDevices[i] ), lDevices[i] ) );
    }

    return lConnecInfo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdCanKomodo::WasteEvent( void )
///
/// \brief  Waste event - Used after connection
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdCanKomodo::WasteEvent( void )
{
    uint8_t lRetry = 3;

    while( lRetry > 0 )
    {
        try
        {
            uint8_t lCount = 0;

            while( true )
            {
                if( !Read( this ) )
                {
                    LeddarUtils::LtTimeUtils::Wait( 50 );

                    if( ++lCount > 10 )
                    {
                        return;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    return;
                }
            }
        }
        catch( std::runtime_error &e )
        {
            if( std::string( e.what() ) == lEventString )
            {
                --lRetry;
                continue;
            }
            else
            {
                throw;
            }
        }
    }
}
#endif