// *****************************************************************************
// Module..: Leddar
//
/// \file    LdBufferProperty.cpp
///
/// \brief   Definition of functions for class LdBufferProperty
///
/// \author  David Levy
///
/// \since   November 2017
//
// Copyright (c) 2017 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdBufferProperty.h"

#include "LtStringUtils.h"
#include "LtScope.h"

#include <sstream>
#include <iomanip>
#include <cstring>
#include <cerrno>


// *****************************************************************************
// Function: LdBufferProperty::LdBufferProperty
//
/// \brief   Constructor.
///
/// The limits are set to the maximum range for the unit size.
///
/// \param   aCategory    See LdProperty.
/// \param   aFeatures    See LdProperty.
/// \param   aId          See LdProperty.
/// \param   aDeviceId    See LdProperty.
/// \param   aBufferSize  Size in bytes of the buffer.
/// \param   aDescription See LdProperty.
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
LeddarCore::LdBufferProperty::LdBufferProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId,
        uint16_t aDeviceId, uint32_t aBufferSize, const std::string &aDescription ) :
    LdProperty( LdProperty::TYPE_BUFFER, aCategory, aFeatures, aId, aDeviceId, aBufferSize, aBufferSize, aDescription )
{
}

// *****************************************************************************
// Function: LdBufferProperty::Value
//
/// \brief   Current value in the property.
///
/// param[in] aIndex : Index of the property to read
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
const uint8_t *
LeddarCore::LdBufferProperty::Value( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= Count() )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

    return CStorage() + Stride() * aIndex;
}

// *****************************************************************************
// Function: LdBufferProperty::DeviceValue
//
/// \brief   Current value in the property.
///
/// param[in] aIndex : Index of the property to read
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
const uint8_t *
LeddarCore::LdBufferProperty::DeviceValue( size_t aIndex ) const
{
    if( aIndex >= Count() )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

    return BackupStorage() + Stride() * aIndex;
}

// *****************************************************************************
// Function: LdBufferProperty::GetStringValue
//
/// \brief   Current value in the property as a string (Hexadecimal display of uint8_t buffer).
///
/// param[in] aIndex : Index of the property to read
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
std::string
LeddarCore::LdBufferProperty::GetStringValue( size_t aIndex ) const
{
    if( aIndex >= Count() )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

    std::stringstream lResult;

    for( size_t i = 0; i < Size(); ++i )
    {
        lResult << std::uppercase << std::setfill( '0' ) << std::setw( 2 ) << std::setbase( 16 ) << unsigned( Value( aIndex )[i] );
    }

    return lResult.str();
}

// *****************************************************************************
// Function: LdBufferProperty::SetStringValue
//
/// \brief   Set the value of the property from a string (Hexadecimal display of uint8_t buffer).
///
/// param[in] aIndex : Index of the property to set
/// param[in] aValue : String value to set
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
void
LeddarCore::LdBufferProperty::SetStringValue( size_t aIndex, const std::string &aValue )
{
    CanEdit();

    if( aIndex >= Count() && ( Count() != 0 && aIndex != 0 ) )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    else if( ( aValue.size() / 2 ) > Size() )
        throw std::out_of_range( "String too long. Verify property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

    std::vector<uint8_t> lBuffer( Size(), 0 );

    for( size_t i = 0; i < aValue.size(); i += 2 )
    {
        errno = 0;
        lBuffer[i / 2] = ( uint8_t )strtoul( aValue.substr( i, 2 ).c_str(), nullptr, 16 );

        if( ( 0 == lBuffer[i / 2] && aValue.substr( i, 2 ) != "00" ) || errno != 0 )
        {
            throw std::invalid_argument( "Could not convert hex string to uint8_t values (error " + LeddarUtils::LtStringUtils::IntToString( errno ) + " ) Property id: " +
                                         LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }
    }

    SetValue( aIndex, &lBuffer[0], static_cast<uint32_t>( aValue.size() / 2 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::ForceStringValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Force the value of the property from a string (Hexadecimal display of uint8_t buffer).
///
/// param[in] aIndex : Index of the property to set
/// param[in] aValue : String value to set
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdBufferProperty::ForceStringValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetStringValue( aIndex, aValue );
}

// *****************************************************************************
// Function: LdBufferProperty::SetValue
//
/// \brief   Set the value of the property from a uint8_t buffer.
///
/// param[in] aIndex : Index of the property to set
/// param[in] aBuffer : Buffer to set
/// param[in] aBufferSize : Buffer size
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
void
LeddarCore::LdBufferProperty::SetValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize )
{
    CanEdit();

    if( !IsInitialized() && Size() == 0 )
    {
        Resize( aBufferSize );
    }

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( Count() == 0 && aIndex == 0 )
    {
        SetCount( 1 );
    }

    if( aIndex >= Count() )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    else if( aBufferSize > Size() )
        throw std::out_of_range( "Buffer too large. Verify property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

    memcpy( Storage() + Size()*aIndex, aBuffer, aBufferSize );
    EmitSignal( LdObject::VALUE_CHANGED );
    SetInitialized( true );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::ForceValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize )
///
/// \brief  Force the value of the property from a uint8_t buffer.
///
/// param[in] aIndex : Index of the property to set
/// param[in] aBuffer : Buffer to set
/// param[in] aBufferSize : Buffer size
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdBufferProperty::ForceValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValue( aIndex, aBuffer, aBufferSize );
}

// *****************************************************************************
// Function: LdBufferProperty::SetRawStorage
//
/// \brief   Set storage directly in memory
///
/// param[in] aBuffer : Buffer to copy
/// param[in] aCount : Number of element in the buffer
/// param[in] aSizeaBufferSize :  Size of each buffer (they all must have the same size)
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
void
LeddarCore::LdBufferProperty::SetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize )
{
    CanEdit();

    if( !IsInitialized() && Size() == 0 )
    {
        Resize( aBufferSize );
    }

    if( aBufferSize > Size() )
        throw std::out_of_range( "Buffer too large. Verify property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    else if( Size() == aBufferSize )
        LdProperty::SetRawStorage( aBuffer, aCount, aBufferSize );
    else //Input buffers are too small, we must pad them with 0 before passing them to base function
    {
        std::vector<uint8_t> lNewBuffer( aCount * Size(), 0 );

        for( uint8_t i = 0; i < aCount; ++i )
        {
            memcpy( &lNewBuffer[i * Size()], &aBuffer[i * aBufferSize], aBufferSize );
        }

        LdProperty::SetRawStorage( &lNewBuffer[0], aCount, static_cast<uint32_t>( Size() ) );
    }

    SetInitialized( true );
}

// *****************************************************************************
// Function: LdBufferProperty::SetRawStorageOffset
//
/// \brief   Set storage directly in memory with an offset
///
/// param[in] aBuffer : Buffer to copy
/// param[in] aCount : Number of element in the buffer
/// param[in] aSizeaBufferSize :  Size of each buffer (they all must have the same size)
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
void
LeddarCore::LdBufferProperty::SetRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize )
{
    CanEdit();

    if( aOffset > Count()*Size() )
    {
        throw std::out_of_range( "Offset is over the property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }
    else if( ( aOffset + aSize ) > ( Count()*Size() ) )
    {
        throw std::out_of_range( "Offset and size is over the property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    memcpy( static_cast<uint8_t *>( &Storage()[aOffset] ), aBuffer, aSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize )
///
/// \brief  Force storage directly in memory
///
/// param[in] aBuffer : Buffer to copy
/// param[in] aCount : Number of element in the buffer
/// param[in] aSizeaBufferSize :  Size of each buffer (they all must have the same size)
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdBufferProperty::ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetRawStorage( aBuffer, aCount, aBufferSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarCore::LdBufferProperty::ForceRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize )
///
/// \brief  Force storage directly in memory with an offset
///
/// param[in] aBuffer : Buffer to copy
/// param[in] aCount : Number of element in the buffer
/// param[in] aSizeaBufferSize :  Size of each buffer (they all must have the same size)
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdBufferProperty::ForceRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetRawStorageOffset( aBuffer, aOffset, aSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::Resize( uint32_t aNewSize )
///
/// \brief  Resizes the buffer
///
/// \exception  std::logic_error    Raised when the property is not empty.
///
/// \param  aNewSize    New size of the buffer.
///
/// \author David Levy
/// \date   August 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBufferProperty::Resize( uint32_t aNewSize )
{
    if( Size() != 0 && Count() != 0 )
    {
        throw std::logic_error( "Cannot resize buffer if its not empy. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    mStride = aNewSize;
    mUnitSize = aNewSize;
}
