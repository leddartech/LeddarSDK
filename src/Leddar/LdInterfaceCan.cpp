////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdInterfaceCan.cpp
///
/// \brief  Implements the LdInterfaceCan class - Functions independent from the hardware used to communicate
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdInterfaceCan.h"
#ifdef BUILD_CANBUS

#include "LtStringUtils.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdInterfaceCan::LdInterfaceCan( const LdConnectionInfoCan *aConnectionInfo, LdConnection *aInterface )
///
/// \brief  Constructor - If sharing a connection, register itself to the "master" of the connection
///
/// \param          aConnectionInfo Information describing the connection.
/// \param [in,out] aInterface      If non-null, the interface.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdInterfaceCan::LdInterfaceCan( const LdConnectionInfoCan *aConnectionInfo, LdConnection *aExistingInterface ) : LdConnection( aConnectionInfo, aExistingInterface ),
    mMaster( nullptr ),
    mIsConnected( false )
{
    mMaster = dynamic_cast<LdInterfaceCan *>( aExistingInterface );

    if( mMaster != nullptr )
    {
        mMaster->RegisterConnection( this );
    }
    else
    {
        RegisterConnection( this );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdInterfaceCan::~LdInterfaceCan()
///
/// \brief  Destructor - Unregister connection to the master
///             If it is the master and there are other connection alive, transfer ownership of the "master" state
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdInterfaceCan::~LdInterfaceCan()
{
    if( mMaster != nullptr )
    {
        mMaster->UnRegisterConnection( this );
    }
    else //We are the master, remove itself for the vector and change the master (to the first in the array)
    {
        UnRegisterConnection( this );

        for( size_t i = 0; i < mRegisteredIds.size(); ++i )
        {
            mRegisteredIds[i].mInterface->ChangeMaster( mRegisteredIds );
        }

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdInterfaceCan::RegisterConnection( LeddarConnection::LdInterfaceCan *aNewInterface )
///
/// \brief  Registers the connection described by aConnectionInfo
///
/// \exception  std::logic_error    Raised when a non master connection tries to register another connection.
///                                 Raised when the CANbus id of different connection overlap.
///
/// \param [in,out] aNewInterface   If non-null, information describing the connection.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdInterfaceCan::RegisterConnection( LeddarConnection::LdInterfaceCan *aNewInterface )
{
    if( mMaster != nullptr )
    {
        throw std::logic_error( "Only the master can register connection" );
    }

    //First check if the new interface ids are correct
    const LdConnectionInfoCan *lNewConnectionInfo = dynamic_cast<const LdConnectionInfoCan *>( aNewInterface->GetConnectionInfo() );

    if( lNewConnectionInfo->GetBaseIdRx() >= lNewConnectionInfo->GetBaseIdTx() && lNewConnectionInfo->GetBaseIdRx() <= lNewConnectionInfo->GetBaseIdTx() + LtComCanBus::CAN_MAX_DETECTIONS + 1 )
    {
        throw std::logic_error( "Connection ids rx and tx (may) overlap" );
    }

    //And check if it overlaps with already registered ids
    for( size_t i = 0; i < mRegisteredIds.size(); ++i )
    {
        if( lNewConnectionInfo->GetBaseIdRx() == mRegisteredIds[i].mConfigId )
        {
            throw std::logic_error( "Connection Rx id overlap: " + LeddarUtils::LtStringUtils::IntToString( lNewConnectionInfo->GetBaseIdRx(), 16 ) );
        }

        if( lNewConnectionInfo->GetBaseIdRx() >= mRegisteredIds[i].mFirstDataId && lNewConnectionInfo->GetBaseIdRx() <= mRegisteredIds[i].mFirstDataId + LtComCanBus::CAN_MAX_DETECTIONS + 1 )
        {
            throw std::logic_error( "Connection id Rx/Tx overlap: " + LeddarUtils::LtStringUtils::IntToString( lNewConnectionInfo->GetBaseIdRx(), 16 ) + "overlap " +
                                    LeddarUtils::LtStringUtils::IntToString( mRegisteredIds[i].mFirstDataId, 16 ) + " to " +
                                    LeddarUtils::LtStringUtils::IntToString( mRegisteredIds[i].mFirstDataId + LtComCanBus::CAN_MAX_DETECTIONS + 1, 16 ) );
        }

        if( mRegisteredIds[i].mConfigId >= lNewConnectionInfo->GetBaseIdTx() && mRegisteredIds[i].mConfigId <= lNewConnectionInfo->GetBaseIdTx() + LtComCanBus::CAN_MAX_DETECTIONS + 1 )
        {
            throw std::logic_error( "Connection id Tx/Rx overlap: " + LeddarUtils::LtStringUtils::IntToString( mRegisteredIds[i].mConfigId, 16 ) + "overlap " +
                                    LeddarUtils::LtStringUtils::IntToString( lNewConnectionInfo->GetBaseIdTx(), 16 ) + " to " +
                                    LeddarUtils::LtStringUtils::IntToString( lNewConnectionInfo->GetBaseIdTx() + LtComCanBus::CAN_MAX_DETECTIONS + 1, 16 ) );
        }

        if( lNewConnectionInfo->GetBaseIdTx() <= mRegisteredIds[i].mFirstDataId + LtComCanBus::CAN_MAX_DETECTIONS + 1 &&
                mRegisteredIds[i].mFirstDataId <= lNewConnectionInfo->GetBaseIdTx() + LtComCanBus::CAN_MAX_DETECTIONS + 1 )
        {
            throw std::logic_error( "Connection Tx id overlap: [" + LeddarUtils::LtStringUtils::IntToString( lNewConnectionInfo->GetBaseIdTx(), 16 ) +
                                    ";" + LeddarUtils::LtStringUtils::IntToString( lNewConnectionInfo->GetBaseIdTx() + LtComCanBus::CAN_MAX_DETECTIONS + 1, 16 ) +
                                    "] overlap [" + LeddarUtils::LtStringUtils::IntToString( mRegisteredIds[i].mFirstDataId, 16 ) +
                                    ";" + LeddarUtils::LtStringUtils::IntToString( mRegisteredIds[i].mFirstDataId + LtComCanBus::CAN_MAX_DETECTIONS + 1, 16 ) + "]" );
        }
    }

    mRegisteredIds.push_back( {aNewInterface,  lNewConnectionInfo->GetBaseIdRx(), lNewConnectionInfo->GetBaseIdTx()} );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdInterfaceCan::UnRegisterConnection( const LeddarConnection::LdInterfaceCan *aInterface )
///
/// \brief  Unregister connection
///
/// \param  aInterface  The interface to unregister
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdInterfaceCan::UnRegisterConnection( const LeddarConnection::LdInterfaceCan *aInterface )
{
    if( mMaster != nullptr )
    {
        throw std::logic_error( "Only the master can unregister connection" );
    }

    for( size_t i = 0; i < mRegisteredIds.size(); ++i )
    {
        if( mRegisteredIds[i].mInterface == aInterface )
        {
            mRegisteredIds.erase( mRegisteredIds.begin() + i );
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdInterfaceCan::ChangeMaster( LeddarConnection::LdInterfaceCan *aInterface )
///
/// \brief  Change the connection master (to the first of the RegisteredId
///
/// \param [in,out] aInterface  The new "master" interface
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdInterfaceCan::ChangeMaster(const std::vector<LtComCanBus::sCanIds> &aRegisteredIds)
{
    if( this == aRegisteredIds[0].mInterface)
    {
        mMaster = nullptr;
        mRegisteredIds = aRegisteredIds;
    }
    else
    {
        mMaster = aRegisteredIds[0].mInterface;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdInterfaceCan::ForwardDataSlave(uint16_t aId, const std::vector<uint8_t>& aData)
///
/// \brief  Forward data from the master to the interface with the corresponding id
///
/// \param  aId     The identifier.
/// \param  aData   The data.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdInterfaceCan::ForwardDataSlave( LtComCanBus::sCanData aData )
{
    EmitSignal( LeddarCore::LdObject::NEW_DATA, &aData );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarConnection::LdInterfaceCan *LeddarConnection::LdInterfaceCan::ForwardDataMaster( uint16_t aId, const std::vector<uint8_t> &aData )
///
/// \brief  Forward received data to the correct interface buffer
///
/// \exception  std::logic_error    Raised when calling this function for a "slave".
/// \exception  std::runtime_error  Raised when an unexpected id is received.
///
/// \param  aId     The identifier.
/// \param  aData   The data.
///
/// \return A pointer to the interface that received data.
///
/// \author David Levy
/// \date   October 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarConnection::LdInterfaceCan *LeddarConnection::LdInterfaceCan::ForwardDataMaster( uint16_t aId, const std::vector<uint8_t> &aData )
{
    if( mMaster != nullptr )
    {
        throw std::logic_error( "Only the master can forward data" );
    }

    for( size_t i = 0; i < mRegisteredIds.size(); ++i )
    {
        if( aId >= mRegisteredIds[i].mFirstDataId && aId <=  mRegisteredIds[i].mFirstDataId + LtComCanBus::CAN_MAX_DETECTIONS + 1 )
        {
            LtComCanBus::sCanData lData = {};
            lData.mId = aId;
            std::copy( aData.begin(), aData.end(), lData.mFrame.mRawData );
            mRegisteredIds[i].mInterface->ForwardDataSlave( lData );
            return mRegisteredIds[i].mInterface;
        }
    }

    throw std::runtime_error( "Unexpected id received: " + LeddarUtils::LtStringUtils::IntToString( aId, 16 ) );
}

#endif
