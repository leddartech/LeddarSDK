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

#include "LtScope.h"
#include "LtStringUtils.h"

#include <cerrno>
#include <cstring>
#include <iomanip>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarCore::LdBufferProperty::LdBufferProperty( const LdBufferProperty &aProperty )
///
/// \brief	Copy constructor
///
/// \author	Alain Ferron
/// \date	December 2020
///
/// \param	aProperty	The property.
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdBufferProperty::LdBufferProperty( const LdBufferProperty &aProperty )
    : LdProperty( aProperty )
{
}

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
LeddarCore::LdBufferProperty::LdBufferProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aBufferSize,
                                                const std::string &aDescription )
    : LdProperty( LdProperty::TYPE_BUFFER, aCategory, aFeatures, aId, aDeviceId, aBufferSize, aBufferSize, aDescription )
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
const uint8_t *LeddarCore::LdBufferProperty::Value( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= PerformCount() )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

    return CStorage() + mStride * aIndex;
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
const uint8_t *LeddarCore::LdBufferProperty::DeviceValue( size_t aIndex ) const
{
    if( aIndex >= PerformCount() )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

    return BackupStorage() + PerformStride() * aIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue )
///
/// \brief  Sets property value
///
/// \author David Lévy
/// \date   February 2021
///
/// \exception  std::invalid_argument   Thrown when an invalid argument error condition occurs.
///
/// \param  aIndex      Zero-based index of the.
/// \param  aNewValue   The new value.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBufferProperty::PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue )
{
    if( aNewValue.type() == typeid( std::vector<uint8_t> ) )
    {
        PerformSetValue( aIndex, boost::any_cast<std::vector<uint8_t>>( aNewValue ) );
    }
    else if( aNewValue.type() == typeid( std::string ) )
    {
        PerformSetStringValue( aIndex, boost::any_cast<std::string>( aNewValue ) );
    }
    else if( aNewValue.type() == typeid( const char * ) )
    {
        PerformSetStringValue( aIndex, boost::any_cast<const char *>( aNewValue ) );
    }
    else
        throw std::invalid_argument( "Invalid value type" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarCore::LdProperty *LeddarCore::LdBufferProperty::PerformClone()
///
/// \brief	Performs the clone action
///
/// \returns	Null if it fails, else a pointer to a LeddarCore::LdProperty.
///
/// \author	Alain Ferron
/// \date	March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdProperty *LeddarCore::LdBufferProperty::PerformClone() { return new LdBufferProperty( *this ); }

// *****************************************************************************
// Function: LdBufferProperty::PerformGetStringValue
//
/// \brief   Current value in the property as a string (Hexadecimal display of uint8_t buffer).
///
/// param[in] aIndex : Index of the property to read
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************
std::string LeddarCore::LdBufferProperty::PerformGetStringValue( size_t aIndex ) const
{
    if( aIndex >= PerformCount() )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

    std::stringstream lResult;

    for( size_t i = 0; i < PerformSize(); ++i )
    {
        lResult << std::uppercase << std::setfill( '0' ) << std::setw( 2 ) << std::setbase( 16 ) << unsigned( Value( aIndex )[i] );
    }

    return lResult.str();
}

// *****************************************************************************
// Function: LdBufferProperty::PerformSetStringValue
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
void LeddarCore::LdBufferProperty::PerformSetStringValue( size_t aIndex, const std::string &aValue )
{
    CanEdit();

    if( aIndex >= PerformCount() && ( PerformCount() != 0 && aIndex != 0 ) )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    else if( ( aValue.size() / 2 ) > PerformSize() )
        throw std::out_of_range( "String too long. Verify property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

    std::vector<uint8_t> lBuffer( PerformSize(), 0 );

    for( size_t i = 0; i < aValue.size(); i += 2 )
    {
        errno          = 0;
        lBuffer[i / 2] = (uint8_t)strtoul( aValue.substr( i, 2 ).c_str(), nullptr, 16 );

        if( ( 0 == lBuffer[i / 2] && aValue.substr( i, 2 ) != "00" ) || errno != 0 )
        {
            throw std::invalid_argument( "Could not convert hex string to uint8_t values (error " + LeddarUtils::LtStringUtils::IntToString( errno ) +
                                         " ) Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
        }
    }

    PerformSetValue( aIndex, &lBuffer[0], static_cast<uint32_t>( aValue.size() / 2 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::PerformForceStringValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Force the value of the property from a string (Hexadecimal display of uint8_t buffer).
///
/// param[in] aIndex : Index of the property to set
/// param[in] aValue : String value to set
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBufferProperty::PerformForceStringValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetStringValue( aIndex, aValue );
}

std::vector<uint8_t> LeddarCore::LdBufferProperty::PerformGetValue( size_t aIndex ) const
{
    auto lValue = Value( aIndex );
    std::vector<uint8_t> lVectorValues( lValue, lValue + PerformStride() );

    return lVectorValues;
}

std::vector<uint8_t> LeddarCore::LdBufferProperty::PerformGetDeviceValue( size_t aIndex ) const
{
    auto lValue = DeviceValue( aIndex );
    std::vector<uint8_t> lVectorValues( lValue, lValue + PerformStride() );

    return lVectorValues;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::PerformSetValue( const size_t aIndex, const std::vector<uint8_t> &aBuffer )
///
/// \brief  Set property value
///
/// \author David Lévy
/// \date   February 2021
///
/// \param  aIndex  Zero-based index of the.
/// \param  aBuffer The buffer.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBufferProperty::PerformSetValue( const size_t aIndex, const std::vector<uint8_t> &aBuffer ) { PerformSetValue( aIndex, aBuffer.data(), aBuffer.size() ); }

// *****************************************************************************
// Function: LdBufferProperty::PerformSetValue
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
void LeddarCore::LdBufferProperty::PerformSetValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize )
{
    CanEdit();

    if( !IsInitialized() && PerformSize() == 0 )
    {
        Resize( aBufferSize );
    }

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( PerformCount() == 0 && aIndex == 0 )
    {
        PerformSetCount( 1 );
    }

    if( aIndex >= PerformCount() )
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    else if( aBufferSize > PerformSize() )
        throw std::out_of_range( "Buffer too large. Verify property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

    memcpy( Storage() + PerformSize() * aIndex, aBuffer, aBufferSize );
    EmitSignal( LdObject::VALUE_CHANGED );
    SetInitialized( true );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::PerformForceValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize )
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
void LeddarCore::LdBufferProperty::PerformForceValue( const size_t aIndex, const uint8_t *aBuffer, const uint32_t aBufferSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetValue( aIndex, aBuffer, aBufferSize );
}

// *****************************************************************************
// Function: LdBufferProperty::PerformSetRawStorage
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
void LeddarCore::LdBufferProperty::PerformSetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize )
{
    CanEdit();

    if( !IsInitialized() && PerformSize() == 0 )
    {
        Resize( aBufferSize );
    }

    if( aBufferSize > PerformSize() )
        throw std::out_of_range( "Buffer too large. Verify property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    else if( PerformSize() == aBufferSize )
        LdProperty::PerformSetRawStorage( aBuffer, aCount, aBufferSize );
    else // Input buffers are too small, we must pad them with 0 before passing them to base function
    {
        std::vector<uint8_t> lNewBuffer( aCount * PerformSize(), 0 );

        for( uint8_t i = 0; i < aCount; ++i )
        {
            memcpy( &lNewBuffer[i * PerformSize()], &aBuffer[i * aBufferSize], aBufferSize );
        }

        LdProperty::PerformSetRawStorage( &lNewBuffer[0], aCount, static_cast<uint32_t>( PerformSize() ) );
    }

    SetInitialized( true );
}

// *****************************************************************************
// Function: LdBufferProperty::PerformSetRawStorageOffset
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
void LeddarCore::LdBufferProperty::PerformSetRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize )
{
    CanEdit();

    if( aOffset > PerformCount() * PerformSize() )
    {
        throw std::out_of_range( "Offset is over the property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }
    else if( ( aOffset + aSize ) > ( PerformCount() * PerformSize() ) )
    {
        throw std::out_of_range( "Offset and size is over the property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    memcpy( static_cast<uint8_t *>( &Storage()[aOffset] ), aBuffer, aSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBufferProperty::PerformForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize )
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
void LeddarCore::LdBufferProperty::PerformForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aBufferSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetRawStorage( aBuffer, aCount, aBufferSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarCore::LdBufferProperty::PerformForceRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize )
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
void LeddarCore::LdBufferProperty::PerformForceRawStorageOffset( uint8_t *aBuffer, uint32_t aOffset, uint32_t aSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetRawStorageOffset( aBuffer, aOffset, aSize );
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
    if( PerformSize() != 0 && PerformCount() != 0 )
    {
        throw std::logic_error( "Cannot resize buffer if its not empy. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    mStride   = aNewSize;
    mUnitSize = aNewSize;
}
