////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdResultEchoes.cpp
///
/// \brief  Implements the LdResultEchoes class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdResultEchoes.h"
#include "LdPropertyIds.h"
#include "LtMathUtils.h"
#include "LtTimeUtils.h"

#ifdef _DEBUG
#include <sstream>
#endif
using namespace LeddarConnection;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdResultEchoes::LdResultEchoes( void )
///
/// \brief  Constructor.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdResultEchoes::LdResultEchoes( void )
    : mIsInitialized( false )
    , mDistanceScale( 0 )
    , mAmplitudeScale( 0 )
    , mHFOV( 0 )
    , mVFOV( 0 )
    , mHChan( 0 )
    , mVChan( 0 )

{
    auto *lTS =
        new LeddarCore::LdIntegerProperty( LeddarCore::LdProperty::CAT_INFO, LeddarCore::LdProperty::F_SAVE  | LeddarCore::LdProperty::F_NO_MODIFIED_WARNING, LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP, 0, 4, "Timestamp" );
    lTS->ForceValue( 0, 0 );
    mDoubleBuffer.AddProperty( lTS );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdResultEchoes::~LdResultEchoes()
///
/// \brief  Destructor.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdResultEchoes::~LdResultEchoes() {}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdResultEchoes::Init( uint32_t aDistanceScale, uint32_t aAmplitudeScale, uint32_t aMaxDetections )
///
/// \brief  Initialize the result object. This function need to be called before use.
///
/// \param  aDistanceScale  The distance scale.
/// \param  aAmplitudeScale The amplitude scale.
/// \param  aMaxDetections  The maximum detections.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void LdResultEchoes::Init( uint32_t aDistanceScale, uint32_t aAmplitudeScale, uint32_t aMaxDetections )
{
    if( !mIsInitialized )
    {
        // Max detection need to be over 0
        if( aMaxDetections == 0 )
        {
            assert( 0 );
        }

        mDoubleBuffer.GetBuffer( B_GET )->Buffer()->mEchoes.resize( aMaxDetections );
        mDoubleBuffer.GetBuffer( B_SET )->Buffer()->mEchoes.resize( aMaxDetections );

        mDistanceScale  = aDistanceScale;
        mAmplitudeScale = aAmplitudeScale;

        mIsInitialized = true;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdResultEchoes::Swap()
///
/// \brief  Swap the two data buffer
///
/// \author David Levy
/// \date   May 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdResultEchoes::Swap() { mDoubleBuffer.Swap(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LdResultEchoes::GetEchoCount( eBuffer aBuffer ) const
///
/// \brief  Get the number of echoes of the buffer
///
/// \param  aBuffer The buffer.
///
/// \return The echo count.
///
/// \author David Levy
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t LdResultEchoes::GetEchoCount( eBuffer aBuffer ) const
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }

    return mDoubleBuffer.GetConstBuffer( aBuffer )->Buffer()->mCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::vector<LdEcho> * LdResultEchoes::GetEchoes( eBuffer aBuffer )
///
/// \brief  Get echoes vector
///
/// \param  aBuffer The buffer.
///
/// \return Null if it fails, else a pointer to the echoes vector.
///
/// \author David Levy
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<LdEcho> *LdResultEchoes::GetEchoes( eBuffer aBuffer )
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }

    return &( mDoubleBuffer.GetBuffer( aBuffer )->Buffer()->mEchoes );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn float LdResultEchoes::GetEchoDistance( size_t aIndex ) const
///
/// \brief  Get echoes distance at index
///
/// \param  aIndex  Echo index.
///
/// \return The echo distance.
///
/// \author David Levy
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
float LdResultEchoes::GetEchoDistance( size_t aIndex ) const
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }

    return static_cast<float>( mDoubleBuffer.GetConstBuffer( B_GET )->Buffer()->mEchoes[aIndex].mDistance ) / mDistanceScale;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn float LdResultEchoes::GetEchoAmplitude( size_t aIndex ) const
///
/// \brief  Get echoes amplitude at index
///
/// \param  aIndex  Echo index.
///
/// \return The echo amplitude.
///
/// \author David Levy
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
float LdResultEchoes::GetEchoAmplitude( size_t aIndex ) const
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }
    
    return static_cast<float>( mDoubleBuffer.GetConstBuffer( B_GET )->Buffer()->mEchoes[aIndex].mAmplitude ) / mAmplitudeScale;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn float LdResultEchoes::GetEchoBase( size_t aIndex ) const
///
/// \brief  Get echoes base at index
///
/// \param  aIndex  Echo index.
///
/// \return The echo base.
///
/// \author David Levy
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
float LdResultEchoes::GetEchoBase( size_t aIndex ) const
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }
    
    return static_cast<float>( mDoubleBuffer.GetConstBuffer( B_GET )->Buffer()->mEchoes[aIndex].mBase ) / mAmplitudeScale;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LeddarConnection::LdResultEchoes::GetTimestamp( eBuffer aBuffer ) const
///
/// \brief  Gets a timestamp
///
/// \author David Lévy
/// \date   March 2021
///
/// \param  aBuffer The buffer.
///
/// \returns    The timestamp.
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t LeddarConnection::LdResultEchoes::GetTimestamp( eBuffer aBuffer ) const
{
    return mDoubleBuffer.GetProperties( aBuffer )->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP )->ValueT<uint32_t>( 0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdResultEchoes::SetTimestamp( uint32_t aTimestamp )
///
/// \brief  Sets a timestamp
///
/// \author David Lévy
/// \date   March 2021
///
/// \param  aTimestamp  The timestamp.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarConnection::LdResultEchoes::SetTimestamp( uint32_t aTimestamp ) { mDoubleBuffer.SetPropertyValue( LeddarCore::LdPropertyIds::ID_RS_TIMESTAMP, 0, aTimestamp ); }

#ifdef _DEBUG
// *****************************************************************************
// Function: LdResultEchoes::ToString

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::string LdResultEchoes::ToString( void )
///
/// \brief  Format the result into string (for debugging).
///
/// \return Formatted result.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string LdResultEchoes::ToString( void )
{
    std::stringstream lResult;
    // cppcheck-suppress unreadVariable
    auto lLock                  = mDoubleBuffer.GetUniqueLock( B_GET );
    const std::vector<LdEcho> &lEchoes = mDoubleBuffer.GetConstBuffer( B_GET )->Buffer()->mEchoes;

    for( uint32_t i = 0; i < GetEchoCount(B_GET); ++i )
    {
        lResult << "[" << lEchoes[i].mChannelIndex << "]:\t " << lEchoes[i].mAmplitude << "\t " << lEchoes[i].mDistance << std::endl;
    }

    return lResult.str();
}

#endif
