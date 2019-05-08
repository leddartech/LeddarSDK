////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdResultEchoes.cpp
///
/// \brief  Implements the LdResultEchoes class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdResultEchoes.h"
#include "LtTimeUtils.h"
#include "LtMathUtils.h"
#include "LdPropertyIds.h"

#ifdef _DEBUG
#include <sstream>
#endif
using namespace LeddarConnection;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdResultEchoes::LdResultEchoes( void ) : mIsInitialized( false ), mDistanceScale( 0 ), mAmplitudeScale( 0 ), mCurrentLedPower( LeddarCore::LdProperty::CAT_INFO, LeddarCore::LdProperty::F_SAVE, LeddarCore::LdPropertyIds::ID_CURRENT_LED_INTENSITY, 0, sizeof( uint16_t ), "Current led power", false )
///
/// \brief  Constructor.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdResultEchoes::LdResultEchoes( void ) :
    mIsInitialized( false ),
    mDistanceScale( 0 ),
    mAmplitudeScale( 0 ),
    mHFOV( 0 ),
    mVFOV( 0 ),
    mHChan( 0 ),
    mVChan( 0 ),
    mCurrentLedPower( LeddarCore::LdProperty::CAT_INFO, LeddarCore::LdProperty::F_SAVE, LeddarCore::LdPropertyIds::ID_CURRENT_LED_INTENSITY, 0, sizeof( uint16_t ), "Current led power", false )
{
    mCurrentLedPower.ForceValue( 0, 0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdResultEchoes::~LdResultEchoes()
///
/// \brief  Destructor.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdResultEchoes::~LdResultEchoes()
{
}

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
void
LdResultEchoes::Init( uint32_t aDistanceScale, uint32_t aAmplitudeScale, uint32_t aMaxDetections )
{
    if( !mIsInitialized )
    {
        // Max detection need to be over 0
        if( aMaxDetections == 0 )
        {
            assert( 0 );
        }

        mEchoBuffer1.mEchoes.resize( aMaxDetections );
        mEchoBuffer2.mEchoes.resize( aMaxDetections );

        mDoubleBuffer.Init( &mEchoBuffer1, &mEchoBuffer2, LdResultProvider::mTimestamp );

        mDistanceScale = aDistanceScale;
        mAmplitudeScale = aAmplitudeScale;
        mCurrentLedPower.SetCount( 2 );

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
void
LeddarConnection::LdResultEchoes::Swap()
{
    if( mCurrentLedPower.Count() == 2 )
    {
        int64_t lOldLedPower = mCurrentLedPower.Value( 0 );
        mCurrentLedPower.ForceValue( 0, mCurrentLedPower.Value( 1 ) );
        mCurrentLedPower.ForceValue( 1, lOldLedPower );
    }

    mDoubleBuffer.Swap();
}

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
uint32_t
LdResultEchoes::GetEchoCount( eBuffer aBuffer ) const
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }

    return static_cast< const EchoBuffer * >( aBuffer == B_GET ? mDoubleBuffer.GetConstBuffer( B_GET )->mBuffer : mDoubleBuffer.GetConstBuffer( B_SET )->mBuffer )->mCount;
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
std::vector<LdEcho> *
LdResultEchoes::GetEchoes( eBuffer aBuffer )
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }

    return &static_cast< EchoBuffer * >( aBuffer == B_GET ? mDoubleBuffer.GetBuffer( B_GET )->mBuffer : mDoubleBuffer.GetBuffer( B_SET )->mBuffer )->mEchoes;
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
float
LdResultEchoes::GetEchoDistance( size_t aIndex ) const
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }

    return static_cast<float>( static_cast< const EchoBuffer * >( mDoubleBuffer.GetConstBuffer( B_GET )->mBuffer )->mEchoes[aIndex].mDistance ) / mDistanceScale;
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
float
LdResultEchoes::GetEchoAmplitude( size_t aIndex ) const
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }

    return static_cast<float>( static_cast< EchoBuffer * >( mDoubleBuffer.GetConstBuffer( B_GET )->mBuffer )->mEchoes[aIndex].mAmplitude ) / mAmplitudeScale;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarUtils::LtMathUtils::LtPointXYZ LeddarConnection::LdResultEchoes::GetEchoCoordinates( size_t aIndex ) const
///
/// \brief  Gets echo cartesian coordinates
///
/// \param  aIndex  Zero-based index of the echo.
///
/// \return The echo cartesian coordinates.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarUtils::LtMathUtils::LtPointXYZ LeddarConnection::LdResultEchoes::GetEchoCoordinates( size_t aIndex ) const
{
    return static_cast< EchoBuffer * >( mDoubleBuffer.GetConstBuffer( B_GET )->mBuffer )->mEchoes[aIndex].ToXYZ( mHFOV, mVFOV, mHChan, mVChan, mDistanceScale );
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
float
LdResultEchoes::GetEchoBase( size_t aIndex ) const
{
    if( !mIsInitialized )
    {
        assert( 0 );
    }

    return static_cast<float>( static_cast< EchoBuffer * >( mDoubleBuffer.GetConstBuffer( B_GET )->mBuffer )->mEchoes[aIndex].mBase ) / mAmplitudeScale;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarConnection::LdResultEchoes::SetCurrentLedPower( uint16_t aValue )
///
/// \brief  Set the current led power
///
/// \param [in] aValue  : Current led power.
///
/// \author David Levy
/// \date   May 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarConnection::LdResultEchoes::SetCurrentLedPower( uint16_t aValue )
{
    if( mCurrentLedPower.Count() < 2 )
    {
        mCurrentLedPower.ForceValue( 0, aValue );
    }
    else
    {
        mCurrentLedPower.ForceValue( 1, aValue );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint16_t LeddarConnection::LdResultEchoes::GetCurrentLedPower( eBuffer aBuffer ) const
///
/// \brief  Gets current LED power
///
/// \param  aBuffer The buffer (get or set).
///
/// \return The current LED power.
///
/// \author David Levy
/// \date   May 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t
LeddarConnection::LdResultEchoes::GetCurrentLedPower( eBuffer aBuffer ) const
{
    if( ( mCurrentLedPower.Count() < 2 || aBuffer == B_GET ) )
    {
        return static_cast<uint32_t>( mCurrentLedPower.Value( 0 ) );
    }
    else
    {
        return static_cast<uint32_t>( mCurrentLedPower.Value( 1 ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarUtils::LtMathUtils::LtPointXYZ LeddarConnection::LdEcho::ToXYZ( double aHFOV, double aVFOV, uint16_t aHChanNbr, uint16_t aVChanNbr, uint32_t aDistanceScale ) const
///
/// \brief  Converts an echo to cartesian coordinates (from spherical coordinates)
///
/// \exception  std::out_of_range   Thrown when the input argument are out of range (from SphericalToCartesian)
///
/// \param  aHFOV           The horizontal field of view.
/// \param  aVFOV           The vertical field of view.
/// \param  aHChanNbr       The horizontal channel number.
/// \param  aVChanNbr       The vertical channel number.
/// \param  aDistanceScale  The distance scale.
///
/// \return The given data converted to a LeddarConnection::LdPointXYZ.
///
/// \author David Levy
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarUtils::LtMathUtils::LtPointXYZ LeddarConnection::LdEcho::ToXYZ( double aHFOV, double aVFOV, uint16_t aHChanNbr, uint16_t aVChanNbr, uint32_t aDistanceScale ) const
{
    if( aHFOV < 0 || aVFOV < 0 || aHChanNbr == 0 || aVChanNbr == 0 )
    {
        throw std::invalid_argument( "Argument out of allowed values" );
    }

    //First get the angles from channel number
    uint16_t lHIndex = mChannelIndex % aHChanNbr;
    uint16_t lVIndex = mChannelIndex / aHChanNbr;

    //angle taken from this page : https://upload.wikimedia.org/wikipedia/commons/8/8c/Spherical_Coordinates_%28Latitude%2C_Longitude%29.svg but rotate axis so z is the sensor axis
    double theta = LeddarUtils::LtMathUtils::DegreeToRadian( lHIndex * aHFOV / aHChanNbr + aHFOV / ( 2.0 * aHChanNbr ) - aHFOV / 2 ); //angle from sensor axis on horizontal plane
    double delta = LeddarUtils::LtMathUtils::DegreeToRadian( lVIndex * aVFOV / aVChanNbr + aVFOV / ( 2.0 * aVChanNbr ) - aVFOV / 2 ); //angle from the point to the horizontal plane

    return LeddarUtils::LtMathUtils::SphericalToCartesian( double( mDistance ) / aDistanceScale, theta, delta );
}

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
std::string
LdResultEchoes::ToString( void )
{
    std::stringstream lResult;
    mDoubleBuffer.Lock( B_GET );

    std::vector<LdEcho> lEchoes = static_cast< EchoBuffer * >( mDoubleBuffer.GetConstBuffer( B_GET )->mBuffer )->mEchoes;

    for( uint32_t i = 0; i < static_cast< EchoBuffer * >( mDoubleBuffer.GetConstBuffer( B_GET )->mBuffer )->mCount; ++i )
    {
        lResult << "[" << lEchoes[ i ].mChannelIndex << "]:\t " << lEchoes[ i ].mAmplitude << "\t " << lEchoes[ i ].mDistance << std::endl;
    }

    mDoubleBuffer.UnLock( B_GET );
    return lResult.str();
}

#endif
