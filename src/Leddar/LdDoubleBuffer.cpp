/// *****************************************************************************
/// Module..: Leddar
///
/// \file    LdDoubleBuffer.cpp
///
/// \brief   Implementation of LdDoubleBuffer class
///
/// \author  David Levy
///
/// \since   January 2018
///
/// Copyright (c) 2018 LeddarTech Inc. All rights reserved.
/// *****************************************************************************

#include "LdDoubleBuffer.h"

#include "LtTimeUtils.h"
#include "LdPropertyIds.h"
#include "comm/LtComLeddarTechPublic.h"

#include <assert.h>

using namespace LeddarConnection;

/// *****************************************************************************
/// Function: LdDoubleBuffer::LdDoubleBuffer
///
/// \brief   Constructor
///
/// \author  David Levy
///
/// \since   January 2018
/// *****************************************************************************
LdDoubleBuffer::LdDoubleBuffer() : mFrameId( LeddarCore::LdProperty::CAT_INFO, LeddarCore::LdProperty::F_SAVE, LeddarCore::LdPropertyIds::ID_RS_FRAME_ID, LtComLeddarTechPublic::LT_COMM_ID_FRAME_ID,
              sizeof( uint64_t ), "Frame id" )
{
    mTimestamp = nullptr;
    mGetBuffer = new DataBuffer();
    mSetBuffer = new DataBuffer();
    mFrameId.SetCount( 2 );
    mFrameId.ForceValue( 0, 0 );
    mFrameId.ForceValue( 1, 0 );
}

/// *****************************************************************************
/// Function: LdDoubleBuffer::~LdDoubleBuffer
///
/// \brief   Destructor - Delete the pointer allocated in the constructor
///
/// \author  David Levy
///
/// \since   January 2018
/// *****************************************************************************
LdDoubleBuffer::~LdDoubleBuffer()
{
    delete mGetBuffer;
    delete mSetBuffer;
    mGetBuffer = nullptr;
    mSetBuffer = nullptr;
}

/// *****************************************************************************
/// Function: LdDoubleBuffer::Init
///
/// \brief   Initialize the buffer. Must be called before using the class.
///
/// \param aGetBuffer : Pointer to the first buffer
/// \param aSetBuffer : Pointer to the other buffer
/// \param aTimestamp : (optional) pointer to the timestamp integer property that will be associated to the buffers
///
/// \author  David Levy
///
/// \since   January 2018
/// *****************************************************************************
void
LdDoubleBuffer::Init( void *aGetBuffer, void *aSetBuffer, LeddarCore::LdIntegerProperty *aTimestamp, LeddarCore::LdIntegerProperty *aTimestamp64 )
{
    mGetBuffer->mBuffer = aGetBuffer;
    mSetBuffer->mBuffer = aSetBuffer;
    mTimestamp          = aTimestamp;
    mTimestamp64        = aTimestamp64;

    //Timestamp index 0 is Get buffer, index 1 is Set buffer
    if( mTimestamp )
    {
        mTimestamp->SetCount( 2 );
        mTimestamp->ForceValue( 0, 0 );
        mTimestamp->ForceValue( 1, 0 );
    }

    //Timestamp index 0 is Get buffer, index 1 is Set buffer
    if( mTimestamp64 )
    {
        mTimestamp64->SetCount( 2 );
        mTimestamp64->ForceValue( 0, 0 );
        mTimestamp64->ForceValue( 1, 0 );
    }
}

/// *****************************************************************************
/// Function: LdDoubleBuffer::Swap
///
/// \brief   Swap the two data buffer and their corresponding timestamp
///
/// \author  David Levy
///
/// \since   January 2018
/// *****************************************************************************
void LdDoubleBuffer::Swap()
{
    if( !mGetBuffer->mBuffer || !mSetBuffer->mBuffer )
        throw std::logic_error( "Buffers not initialized" );

    while( mSetBuffer->mBusy )
    {
        LeddarUtils::LtTimeUtils::WaitBlockingMicro( 1 );
    }

    mSetBuffer->mBusy = true;

    while( mGetBuffer->mBusy )
    {
        LeddarUtils::LtTimeUtils::WaitBlockingMicro( 1 );
    }

    mGetBuffer->mBusy = true;

    std::swap( mSetBuffer, mGetBuffer );

    mGetBuffer->mBusy = false;
    mSetBuffer->mBusy = false;

    if( mTimestamp && mTimestamp->Count() == 2 )
    {
        int64_t lOldTimeStamp0 = mTimestamp->Value( 0 );
        mTimestamp->ForceValue( 0, mTimestamp->Value( 1 ) );
        mTimestamp->ForceValue( 1, lOldTimeStamp0 );
    }

    if( mTimestamp64 && mTimestamp64->Count() == 2 )
    {
        int64_t lOldTimeStamp0 = mTimestamp64->Value( 0 );
        mTimestamp64->ForceValue( 0, mTimestamp64->Value( 1 ) );
        mTimestamp64->ForceValue( 1, lOldTimeStamp0 );
    }

    {
        uint64_t lOldFrameId0 = mFrameId.ValueT<uint64_t>( 0 );
        mFrameId.ForceValue( 0, mFrameId.ValueT<uint64_t>( 1 ) );
        mFrameId.ForceValue( 1, lOldFrameId0 );
    }
}

/// *****************************************************************************
/// Function: LdDoubleBuffer::GetTimeStamp
///
/// \brief   Get the timestamp of the buffer
///
/// \param aBuffer : Get or Set buffer
///
/// \author  David Levy
///
/// \since   January 2018
/// *****************************************************************************
uint32_t
LdDoubleBuffer::GetTimestamp( eBuffer aBuffer ) const
{
    if( mTimestamp && ( mTimestamp->Count() < 2 || aBuffer == B_GET ) )
    {
        return static_cast<uint32_t>( mTimestamp->Value( 0 ) );
    }
    else
    {
        return static_cast<uint32_t>( mTimestamp->Value( 1 ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdDoubleBuffer::SetTimestamp( uint32_t aTimestamp )
///
/// \brief  Sets a timestamp
///
/// \param  aTimestamp  The new timestamp.
///
/// \author David Levy
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdDoubleBuffer::SetTimestamp( uint32_t aTimestamp )
{
    if( mTimestamp && mTimestamp->Count() < 2 )
    {
        mTimestamp->ForceValue( 0, aTimestamp );
    }
    else if( mTimestamp )
    {
        mTimestamp->ForceValue( 1, aTimestamp );
    }
}

/// *****************************************************************************
/// Function: LdDoubleBuffer::GetTimestamp64
///
/// \brief   Get the timestamp of the buffer
///
/// \param aBuffer : Get or Set buffer
///
/// \author  David Levy
///
/// \since   January 2018
/// *****************************************************************************
uint64_t
LdDoubleBuffer::GetTimestamp64( eBuffer aBuffer ) const
{
    if( mTimestamp64 && ( mTimestamp64->Count() < 2 || aBuffer == B_GET ) )
    {
        return mTimestamp64->ValueT<uint64_t>( 0 );
    }
    else
    {
        return mTimestamp64->ValueT<uint64_t>( 1 );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdDoubleBuffer::SetTimestamp64( uint64_t aTimestamp )
///
/// \brief  Sets a timestamp
///
/// \param  aTimestamp  The new timestamp.
///
/// \author David Levy
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdDoubleBuffer::SetTimestamp64( uint64_t aTimestamp )
{
    if( mTimestamp64 && mTimestamp64->Count() < 2 )
    {
        mTimestamp64->ForceValueUnsigned( 0, aTimestamp );
    }
    else if( mTimestamp64 )
    {
        mTimestamp64->ForceValueUnsigned( 1, aTimestamp );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint64_t LeddarConnection::LdDoubleBuffer::GetFrameId( eBuffer aBuffer ) const
///
/// \brief  Gets frame identifier
///
/// \param  aBuffer The buffer.
///
/// \returns    The frame identifier.
///
/// \author David L�vy
/// \date   January 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t LeddarConnection::LdDoubleBuffer::GetFrameId( eBuffer aBuffer ) const
{
    if( mFrameId.Count() < 2 || aBuffer == B_GET )
    {
        return mFrameId.ValueT<uint64_t>( 0 );
    }
    else
    {
        return mFrameId.ValueT<uint64_t>( 1 );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdDoubleBuffer::SetFrameId( uint64_t aFrameId )
///
/// \brief  Sets frame identifier
///
/// \param  aFrameId    Identifier for the frame.
///
/// \author David L�vy
/// \date   January 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdDoubleBuffer::SetFrameId( uint64_t aFrameId )
{
    if( mFrameId.Count() < 2 )
    {
        mFrameId.ForceValueUnsigned( 0, aFrameId );
    }
    else
    {
        mFrameId.ForceValueUnsigned( 1, aFrameId );
    }
}
