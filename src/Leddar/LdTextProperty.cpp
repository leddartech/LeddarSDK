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
#include "LtStringUtils.h"
#include "LtScope.h"

#include <cassert>
#include <cstring>

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

LeddarCore::LdTextProperty::LdTextProperty( LdProperty::eCategories aCategory, uint32_t aFeatures,
        uint32_t aId, uint16_t aDeviceId, uint32_t aMaxLength, eType aType, const std::string &aDescription ) :
    LdProperty( LdProperty::TYPE_TEXT, aCategory, aFeatures, aId, aDeviceId, aMaxLength, aMaxLength, aDescription ),
    mForceUppercase( false ),
    mType( aType )
{
}

// *****************************************************************************
// Function: LdTextProperty::SetTextValue
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

void
LeddarCore::LdTextProperty::SetValue( size_t aIndex, const std::string &aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( Count() == 0 && aIndex == 0 )
    {
        SetCount( 1 );
    }

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( mType == TYPE_ASCII || mType == TYPE_UTF8 )
    {
        if( aValue.length() > MaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

        memset( Storage() + aIndex * MaxLength(), 0, MaxLength() );
        memcpy( Storage() + aIndex * MaxLength(), aValue.c_str(), aValue.length() );
    }
    else
    {
        if( aValue.length() * 2 > MaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

        // Convert UTF8 to UTF16 - note : not really but kept for retro compatibility
        memset( Storage() + aIndex * MaxLength(), 0, MaxLength() );

        for( size_t i = 0; i < aValue.length(); ++i )
        {
            reinterpret_cast<uint16_t *>( Storage() + aIndex * MaxLength() )[i] = static_cast<uint16_t>( aValue[i] );
        }
    }

    EmitSignal( LdObject::VALUE_CHANGED );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::ForceValue( size_t aIndex, const std::string &aValue )
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
void
LeddarCore::LdTextProperty::ForceValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValue( aIndex, aValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::SetValue( size_t aIndex, const std::wstring &aValue )
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
void
LeddarCore::LdTextProperty::SetValue( size_t aIndex, const std::wstring &aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( Count() == 0 && aIndex == 0 )
    {
        SetCount( 1 );
    }

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( mType == TYPE_ASCII )
    {
        std::string lASCIIString( aValue.begin(), aValue.end() );

        if( lASCIIString.length() > MaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

        memset( Storage() + aIndex * MaxLength(), 0, MaxLength() );
        memcpy( Storage() + aIndex * MaxLength(), lASCIIString.c_str(), lASCIIString.length() );
    }
    else if( mType == TYPE_UTF16 )
    {
        if( aValue.size() * 2 > MaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

        memset( Storage(), 0, MaxLength() );

        for( size_t i = 0; i < aValue.size(); ++i )
        {
            reinterpret_cast< uint16_t * >( Storage() + aIndex * MaxLength() )[i] = static_cast< uint16_t >( aValue[i] );
        }

    }
    else
    {
#ifdef _WIN32
        std::string lResult = LeddarUtils::LtStringUtils::Utf8Encode( aValue );

        if( lResult.length() > MaxLength() )
            throw std::out_of_range( "Input string is too long. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );

        memset( Storage(), 0, MaxLength() );
        memcpy( Storage() + aIndex * MaxLength(), lResult.c_str(), lResult.length() );
#else
        throw std::logic_error( "Do not use wstring with UTF8 on non windows platform." );
#endif
    }

    EmitSignal( LdObject::VALUE_CHANGED );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::ForceValue( size_t aIndex, const std::string &aValue )
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
void
LeddarCore::LdTextProperty::ForceValue( size_t aIndex, const std::wstring &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValue( aIndex, aValue );
}

// *****************************************************************************
// Function: LdTextProperty::Value
//
/// \brief   Return the string value
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

std::string
LeddarCore::LdTextProperty::Value( size_t aIndex ) const
{
    if( mType == TYPE_UTF16 )
    {
        throw LeddarException::LtException( "Can not return string on UTF16 text property." );
    }

    return std::string( CStorage() + MaxLength() * aIndex, std::find( CStorage() + MaxLength() * aIndex, CStorage() + MaxLength() * ( aIndex + 1 ), '\0' ) );
}

// *****************************************************************************
// Function: LdTextProperty::WValue
//
/// \brief   Return the wstring value
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

std::wstring
LeddarCore::LdTextProperty::WValue( size_t aIndex ) const
{
    std::wstring lResult;

    if( mType == TYPE_ASCII )
    {
#ifdef _WIN32
        std::string lUtf8Str = std::string( CStorage() + MaxLength() * aIndex, std::find( CStorage() + MaxLength() * aIndex, CStorage() + MaxLength() * ( aIndex + 1 ), '\0' ) );
        lResult = LeddarUtils::LtStringUtils::Utf8Decode( lUtf8Str );
#else
        throw LeddarException::LtException( "Can not return wstring on ASCII text property - Do not use wstring on non Windows platform." );
#endif
    }
    else if( TYPE_UTF8 == mType )
    {
#ifdef _WIN32
        std::string lUtf8Str = std::string( CStorage() + MaxLength() * aIndex, std::find( CStorage() + MaxLength() * aIndex, CStorage() + MaxLength() * ( aIndex + 1 ), '\0' ) );
        lResult = LeddarUtils::LtStringUtils::Utf8Decode( lUtf8Str );
#else
        throw LeddarException::LtException( "Can not return wstring on UTF8 text property - Do not use wstring on non Windows platform." );
#endif
    }
    else
    {
        const uint16_t *lUTF16Buffer = reinterpret_cast<const uint16_t *>( CStorage() + MaxLength() * aIndex );
        uint16_t lMaxLength = MaxLength() / 2;

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
// Function: LdTextProperty::GetStringValue
//
/// \brief   Return the string value. If the property is an UTF16,  it will be converted in UTF8.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
std::string
LeddarCore::LdTextProperty::GetStringValue( size_t aIndex ) const
{
    if( mType == TYPE_UTF16 )
    {
        std::wstring lWString = WValue( aIndex );
        return std::string( lWString.begin(), lWString.end() );
    }

    return Value( aIndex );
}

// *****************************************************************************
// Function: LdProperty::SetRawStorage
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
void
LeddarCore::LdTextProperty::SetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
{
    CanEdit();

    if( ( mType != TYPE_UTF16 || aSize > 1 ) && aSize == Stride() ) //Size usually > 1 for live sensors, but = 1 for ltl recording utf16
    {
        LeddarCore::LdProperty::SetRawStorage( aBuffer, aCount, aSize );
        return;
    }

    size_t lSize = aSize, lCount = aCount;

    if( lSize == 1 && lCount > 1 ) // Should only be the case when reading ltl recording of M16
    {
        lCount = aSize;
        lSize = aCount;
    }

    if( mType == TYPE_ASCII || mType == TYPE_UTF8 )
    {
        if( lSize > Stride() )
            throw std::logic_error( "Property storage size is too small." );
    }
    else //pseudo utf16
    {
        if( lSize * 2 > Stride() )
            throw std::logic_error( "Property storage size is too small." );
    }

    if( Count() != lCount )
    {
        SetCount( lCount );
    }

    //Text properties are stored in utf8 in recording files, set value takes care of utf8 / utf16
    for( size_t i = 0; i < lCount; ++i )
    {
        uint8_t *lBuff = aBuffer + i * lSize;
        std::string lText( lBuff, lBuff + lSize );
        SetValue( i, lText );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdTextProperty::ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
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
void
LeddarCore::LdTextProperty::ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetRawStorage( aBuffer, aCount, aSize );
}
