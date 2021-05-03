// *****************************************************************************
// Module..: Leddar
//
/// \file    LdTextProperty.cpp
///
/// \brief   Definition of functions for class LdTextProperty.
///
/// \author  Patrick Boulay
///
/// \since   February 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdTextProperty.h"
#include "LtExceptions.h"
#include "LtScope.h"
#include "LtStringUtils.h"

#include <cassert>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarCore::LdTextProperty::LdTextProperty( const LdTextProperty &aProperty )
///
/// \brief	Copy constructor
///
/// \author	Alain Ferron
/// \date	December 2020
///
/// \param	aProperty	The property.
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdTextProperty::LdTextProperty( const LdTextProperty &aProperty )
    : LdProperty( aProperty )
{
    std::lock_guard<std::recursive_mutex> lock( aProperty.mPropertyMutex );
    mForceUppercase = aProperty.mForceUppercase;
    mType           = aProperty.mType;
}

// *****************************************************************************
// Function: LdTextProperty::LdTextProperty
//
/// \brief   Constructor.
///
/// \param   aCategory    See LdProperty.
/// \param   aFeatures    See LdProperty.
/// \param   aId          See LdProperty.
/// \param   aDeviceId    See LdProperty.
/// \param   aMaxLength   The maximum number of characters (not including null-term).
/// \param   aDescription See LdProperty.
/// \param   aType        Encoding (ASCII or UTF16)
///
/// \author  Louis Perreault
///
/// \since   May 2014
// *****************************************************************************

LeddarCore::LdTextProperty::LdTextProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aMaxLength, eType aType,
                                            const std::string &aDescription )
    : LdProperty( LdProperty::TYPE_TEXT, aCategory, aFeatures, aId, aDeviceId, aMaxLength, aMaxLength, aDescription )
    , mForceUppercase( false )
    , mType( aType )
{
}

// *****************************************************************************
// Function: LdTextProperty::PerformSetTextValue
//
/// \brief   Set the value from a string
///
/// \param   aIndex  Currently not used.
/// \param   aValue  string providing the new content.
///
/// \exception std::out_of_range Index not valid, verify property count.
/// \exception std::length_error String size exceed maxmim length fixed by the property constructor.
///
/// \author  Louis Perreault
///
/// \since   June 2014
// *****************************************************************************

void LeddarCore::LdTextProperty::PerformSetValue( size_t aIndex, const std::string &aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( PerformCount() == 0 && aIndex == 0 )
    {
        PerformSetCount( 1 );
    }

    if( aIndex >= PerformCount() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    if( mType == TYPE_ASCII || mType == TYPE_UTF8 )
    {
        if( aValue.length() > PerformMaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

        memset( Storage() + aIndex * PerformMaxLength(), 0, PerformMaxLength() );
        memcpy( Storage() + aIndex * PerformMaxLength(), aValue.c_str(), aValue.length() );
    }
    else
    {
        if( aValue.length() * 2 > PerformMaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

        // Convert UTF8 to UTF16 - note : not really but kept for retro compatibility
        memset( Storage() + aIndex * PerformMaxLength(), 0, PerformMaxLength() );

        for( size_t i = 0; i < aValue.length(); ++i )
        {
            reinterpret_cast<uint16_t *>( Storage() + aIndex * PerformMaxLength() )[i] = static_cast<uint16_t>( aValue[i] );
        }
    }

    EmitSignal( LdObject::VALUE_CHANGED );
    SetInitialized( true );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::PerformForceValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Force value for not editable values
///
/// \param   aIndex  Currently not used.
/// \param   aValue  string providing the new content.
///
/// \exception std::out_of_range Index not valid, verify property count.
/// \exception std::length_error String size exceed maximum length fixed by the property constructor.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdTextProperty::PerformForceValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetValue( aIndex, aValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::PerformSetValue( size_t aIndex, const std::wstring &aValue )
///
/// \brief   Set the value from a wstring
///
/// \exception std::out_of_range Index not valid, verify property count.
/// \exception std::length_error String size exceed maximum length fixed by the property constructor.
///
/// \param   aIndex  Currently not used.
/// \param   aValue  string providing the new text.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdTextProperty::PerformSetValue( size_t aIndex, const std::wstring &aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( PerformCount() == 0 && aIndex == 0 )
    {
        PerformSetCount( 1 );
    }

    if( aIndex >= PerformCount() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    if( mType == TYPE_ASCII )
    {
        std::string lASCIIString( aValue.begin(), aValue.end() );

        if( lASCIIString.length() > PerformMaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

        memset( Storage() + aIndex * PerformMaxLength(), 0, PerformMaxLength() );
        memcpy( Storage() + aIndex * PerformMaxLength(), lASCIIString.c_str(), lASCIIString.length() );
    }
    else if( mType == TYPE_UTF16 )
    {
        if( aValue.size() * 2 > PerformMaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

        memset( Storage(), 0, PerformMaxLength() );

        for( size_t i = 0; i < aValue.size(); ++i )
        {
            reinterpret_cast<uint16_t *>( Storage() + aIndex * PerformMaxLength() )[i] = static_cast<uint16_t>( aValue[i] );
        }
    }
    else
    {
#ifdef _WIN32
        std::string lResult = LeddarUtils::LtStringUtils::Utf8Encode( aValue );

        if( lResult.length() > PerformMaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );

        memset( Storage(), 0, PerformMaxLength() );
        memcpy( Storage() + aIndex * PerformMaxLength(), lResult.c_str(), lResult.length() );
#else
        throw std::logic_error( "Do not use wstring with UTF8 on non windows platform." );
#endif
    }

    EmitSignal( LdObject::VALUE_CHANGED );
    SetInitialized( true );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue )
///
/// \brief  Set the property value
///
/// \author David Lévy
/// \date   February 2021
///
/// \exception  std::invalid_argument   Thrown when an invalid argument error condition occurs.
///
/// \param  aIndex      Zero-based index of the.
/// \param  aNewValue   The new value.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdTextProperty::PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue )
{
    if( aNewValue.type() == typeid( std::string ) )
    {
        PerformSetValue( aIndex, boost::any_cast<std::string>( aNewValue ) );
    }
    else if( aNewValue.type() == typeid( const char * ) )
    {
        PerformSetValue( aIndex, boost::any_cast<const char *>( aNewValue ) );
    }
    else if( aNewValue.type() == typeid( std::wstring ) )
    {
        PerformSetValue( aIndex, boost::any_cast<std::wstring>( aNewValue ) );
    }
    else
        throw std::invalid_argument( "Invalid value type" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::PerformForceValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Force value for not editable values
///
/// \param   aIndex  Currently not used.
/// \param   aValue  string providing the new content.
///
/// \exception std::out_of_range Index not valid, verify property count.
/// \exception std::length_error String size exceed maximum length fixed by the property constructor.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdTextProperty::PerformForceValue( size_t aIndex, const std::wstring &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetValue( aIndex, aValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarCore::LdProperty *LeddarCore::LdTextProperty::PerformClone()
///
/// \brief	Performs the clone action
///
/// \returns	Pointer to a LeddarCore::LdProperty.
///
/// \author	Alain Ferron
/// \date	March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdProperty *LeddarCore::LdTextProperty::PerformClone() { return new LdTextProperty( *this ); }

// *****************************************************************************
// Function: LdTextProperty::PerformValue
//
/// \brief   Return the string value
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
std::string LeddarCore::LdTextProperty::PerformValue( size_t aIndex ) const
{
    VerifyInitialization();

    if( mType == TYPE_UTF16 )
    {
        throw LeddarException::LtException( "Can not return string on UTF16 text property." );
    }

    return std::string( CStorage() + PerformMaxLength() * aIndex, std::find( CStorage() + PerformMaxLength() * aIndex, CStorage() + PerformMaxLength() * ( aIndex + 1 ), '\0' ) );
}

// *****************************************************************************
// Function: LdTextProperty::PerformWValue
//
/// \brief   Return the wstring value
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

std::wstring LeddarCore::LdTextProperty::PerformWValue( size_t aIndex ) const
{
    VerifyInitialization();
    std::wstring lResult;

    if( mType == TYPE_ASCII )
    {
#ifdef _WIN32
        std::string lUtf8Str =
            std::string( CStorage() + PerformMaxLength() * aIndex, std::find( CStorage() + PerformMaxLength() * aIndex, CStorage() + PerformMaxLength() * ( aIndex + 1 ), '\0' ) );
        lResult = LeddarUtils::LtStringUtils::Utf8Decode( lUtf8Str );
#else
        throw LeddarException::LtException( "Can not return wstring on ASCII text property - Do not use wstring on non Windows platform." );
#endif
    }
    else if( TYPE_UTF8 == mType )
    {
#ifdef _WIN32
        std::string lUtf8Str =
            std::string( CStorage() + PerformMaxLength() * aIndex, std::find( CStorage() + PerformMaxLength() * aIndex, CStorage() + PerformMaxLength() * ( aIndex + 1 ), '\0' ) );
        lResult = LeddarUtils::LtStringUtils::Utf8Decode( lUtf8Str );
#else
        throw LeddarException::LtException( "Can not return wstring on UTF8 text property - Do not use wstring on non Windows platform." );
#endif
    }
    else
    {
        const uint16_t *lUTF16Buffer = reinterpret_cast<const uint16_t *>( CStorage() + PerformMaxLength() * aIndex );
        uint16_t lMaxLength          = PerformMaxLength() / 2;

        for( int i = 0; i < lMaxLength; ++i )
        {
            if( lUTF16Buffer[i] == 0 )
                break;

            lResult += static_cast<wchar_t>( lUTF16Buffer[i] );
        }
    }

    return lResult;
}

// *****************************************************************************
// Function: LdTextProperty::PerformGetStringValue
//
/// \brief   Return the string value. If the property is an UTF16,  it will be converted in UTF8.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
std::string LeddarCore::LdTextProperty::PerformGetStringValue( size_t aIndex ) const
{
    if( mType == TYPE_UTF16 )
    {
        std::wstring lWString = PerformWValue( aIndex );
        return std::string( lWString.begin(), lWString.end() );
    }

    return PerformValue( aIndex );
}

// *****************************************************************************
// Function: LdProperty::PerformSetRawStorage
//
/// \brief   Set storage directly in memory - This function should be only used when reading ltl recording for pseudo utf16
///
/// \param   aBuffer Buffer to copy
/// \param   aCount  Number of element in the buffer
/// \param   aSize   Size of each element in the buffer
///
/// \author  David Levy
///
/// \since   May 2018
// *****************************************************************************
void LeddarCore::LdTextProperty::PerformSetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
{
    CanEdit();

    if( ( mType != TYPE_UTF16 || aSize > 1 ) && aSize == PerformStride() ) // Size usually > 1 for live sensors, but = 1 for ltl recording utf16
    {
        LeddarCore::LdProperty::PerformSetRawStorage( aBuffer, aCount, aSize );
        return;
    }

    size_t lSize = aSize, lCount = aCount;

    if( lSize == 1 && lCount > 1 ) // Should only be the case when reading ltl recording of M16
    {
        lCount = aSize;
        lSize  = aCount;
    }

    if( mType == TYPE_ASCII || mType == TYPE_UTF8 )
    {
        if( lSize > PerformStride() )
            throw std::logic_error( "Property storage size is too small." );
    }
    else // pseudo utf16
    {
        if( lSize * 2 > PerformStride() )
            throw std::logic_error( "Property storage size is too small." );
    }

    if( PerformCount() != lCount )
    {
        PerformSetCount( lCount );
    }

    // Text properties are stored in utf8 in recording files, set value takes care of utf8 / utf16
    for( size_t i = 0; i < lCount; ++i )
    {
        uint8_t *lBuff = aBuffer + i * lSize;
        std::string lText( lBuff, lBuff + lSize );
        PerformSetValue( i, lText );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::PerformForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
///
/// \brief  Force raw storage for non-editable properties
///
/// \param   aBuffer Buffer to copy
/// \param   aCount  Number of element in the buffer
/// \param   aSize   Size of each element in the buffer
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdTextProperty::PerformForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetRawStorage( aBuffer, aCount, aSize );
}
