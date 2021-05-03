////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdObject.cpp
///
/// \brief  Implements the LdObject class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdObject.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarCore::LdObject::LdObject( void )
///
/// \brief  Constructor.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdObject::LdObject( void )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarCore::LdObject::~LdObject( void )
///
/// \brief  Destructor. This object need to be disconnected to from all connected object.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdObject::~LdObject( void )
{
    std::lock_guard<std::mutex> lock( mObjectMutex );
    DisconnectAll();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdObject::ConnectSignal( LdObject *aSender, const SIGNALS aSignal )
///
/// \brief  Connect to the sender's signal.
///
/// \param [in,out] aSender Pointer to object to connect.
/// \param          aSignal Signal to connect.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdObject::ConnectSignal( LdObject *aSender, const SIGNALS aSignal ) const
{
    std::lock_guard<std::mutex> lock( mObjectMutex );
    std::pair<std::multimap< LdObject *, SIGNALS>::iterator, std::multimap< LdObject *, SIGNALS>::iterator> lRange = mReceiverMap.equal_range( aSender );

    for( std::multimap< LdObject *, SIGNALS>::iterator lIter = lRange.first; lIter != lRange.second; ++lIter )
    {
        if( lIter->second == aSignal && lIter->first == aSender )
        {
            throw std::logic_error( "This object is already connected to this signal" );
        }
    }

    mReceiverMap.insert( std::make_pair( aSender, aSignal ) );

    aSender->mConnectedObject.insert( const_cast<LeddarCore::LdObject *>(this) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdObject::DisconnectSignal( LdObject *aSender, const SIGNALS aSignal )
///
/// \brief  Disconnect to the sender's signal.
///
/// \param [in,out] aSender Pointer to the object to disconnect.
/// \param          aSignal Signal to disconnect.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdObject::DisconnectSignal( LdObject *aSender, const SIGNALS aSignal ) const
{
    std::lock_guard<std::mutex> lock( mObjectMutex );
    std::pair<std::multimap< LdObject *, SIGNALS>::iterator, std::multimap< LdObject *, SIGNALS>::iterator> lRange = mReceiverMap.equal_range( aSender );

    if( lRange.first == mReceiverMap.end() )
    {
        return;
    }

    for( std::multimap< LdObject *, SIGNALS>::iterator lIter = lRange.first; lIter != lRange.second; ++lIter )
    {
        if( lIter->second == aSignal && lIter->first == aSender )
        {
            mReceiverMap.erase( lIter );
            break;
        }
    }

    // If there is no aSender object, we need to remove this object in the mConnectedObject
    if( mReceiverMap.find( aSender ) == mReceiverMap.end() )
    {
        aSender->mConnectedObject.erase( const_cast<LeddarCore::LdObject *>(this) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdObject::DisconnectAll( void )
///
/// \brief  Disconnect this object to all senders connected.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdObject::DisconnectAll( void ) const
{
    
    // Delete links between the receiver and this object
    for( std::multimap< LdObject *, SIGNALS>::iterator lIter = mReceiverMap.begin(); lIter != mReceiverMap.end(); ++lIter )
    {
        ( *lIter ).first->mConnectedObject.erase( const_cast<LeddarCore::LdObject *>(this) );
    }

    // Delete links between this object and receiver
    for( std::set< LdObject * >::iterator lIter = mConnectedObject.begin(); lIter != mConnectedObject.end(); ++lIter )
    {
        ( *lIter )->mReceiverMap.erase( const_cast<LeddarCore::LdObject *>(this) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdObject::EmitSignal( const SIGNALS aSignal, void *aExtraData )
///
/// \brief  Notify all connected object.
///
/// \param          aSignal     Notification signal.
/// \param [in,out] aExtraData  If non-null, information describing the extra.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdObject::EmitSignal( const SIGNALS aSignal, void *aExtraData )
{
    std::lock_guard<std::mutex> lock( mObjectMutex );
    for( std::multimap< LdObject *, SIGNALS>::iterator lIter = mReceiverMap.begin(); lIter != mReceiverMap.end(); ++lIter )
    {
        if( lIter->second == aSignal )
        {
            lIter->first->Callback( this, lIter->second, aExtraData );
        }
    }
}
