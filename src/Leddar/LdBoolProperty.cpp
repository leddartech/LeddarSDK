// *****************************************************************************
// Module..: Leddar
//
/// \file    LdBoolProperty.cpp
///
/// \brief   Definition of LdBoolProperty class.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdBoolProperty.h"

#include "LtScope.h"
#include "LtStringUtils.h"

#include <string>

// *****************************************************************************
// Function: LdBoolProperty::LdBoolProperty
//
/// \brief   Constructor.
///
/// \param   aCategory    See LdProperty.
/// \param   aFeatures    See LdProperty.
/// \param   aId          See LdProperty.
/// \param   aDeviceId    See LdProperty.
/// \param   aDescription See LdProperty.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
LeddarCore::LdBoolProperty::LdBoolProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, const std::string &aDescription )
    : LdProperty( LdProperty::TYPE_BOOL, aCategory, aFeatures, aId, aDeviceId, sizeof( bool ), sizeof( bool ), aDescription )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarCore::LdBoolProperty::LdBoolProperty( const LdBoolProperty &aProperty )
///
/// \brief	Copy constructor
///
/// \author	Alain Ferron
/// \date	December 2020
///
/// \param	aProperty	The property.
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdBoolProperty::LdBoolProperty( const LdBoolProperty &aProperty )
    : LdProperty( aProperty )
{
}

// *****************************************************************************
// Function: LdBoolProperty::PerformSetValue
//
/// \brief   Change the value at the given index.
///
/// \param   aIndex  Index in array of value to change.
/// \param   aValue  New value to write.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

void LeddarCore::LdBoolProperty::PerformSetValue( size_t aIndex, bool aValue )
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

    bool *lValues = reinterpret_cast<bool *>( Storage() );

    if( !IsInitialized() || lValues[aIndex] != aValue )
    {
        SetInitialized( true );
        lValues[aIndex] = aValue;
        EmitSignal( LdObject::VALUE_CHANGED );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBoolProperty::PerformForceValue( size_t aIndex, bool aValue )
///
/// \brief  Force value
///
/// \param   aIndex  Index in array of value to change.
/// \param   aValue  New value to write.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBoolProperty::PerformForceValue( size_t aIndex, bool aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetValue( aIndex, aValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBoolProperty::PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue )
///
/// \brief  Sets the property value
///
/// \author David Lévy
/// \date   February 2021
///
/// \exception  std::invalid_argument   Thrown when an invalid argument error condition occurs.
///
/// \param  aIndex      Zero-based index of the.
/// \param  aNewValue   The new value.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBoolProperty::PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue )
{
    if( aNewValue.type() == typeid( bool ) )
    {
        PerformSetValue( aIndex, boost::any_cast<bool>( aNewValue ) );
    }
    else
        throw std::invalid_argument( "Invalid value type" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarCore::LdProperty *LeddarCore::LdBoolProperty::PerformClone()
///
/// \brief	Performs the clone action
///
/// \returns	Null if it fails, else a pointer to a LeddarCore::LdProperty.
///
/// \author	Alain Ferron
/// \date	March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdProperty *LeddarCore::LdBoolProperty::PerformClone() { return new LdBoolProperty( *this ); }

// *****************************************************************************
// Function: LdBoolProperty::PerformGetStringValue
//
/// \brief   Display the value in string format
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
std::string LeddarCore::LdBoolProperty::PerformGetStringValue( size_t aIndex ) const { return ( PerformValue( aIndex ) == true ? "true" : "false" ); }

// *****************************************************************************
// Function: LdBoolProperty::PerformSetStringValue
//
/// \brief   Property writer for the value as text. Possible value: true and false (lower case)
///
/// \param   aIndex  Index of value to write.
/// \param   aValue  The new value.
///
/// \exception std::invalid_argument If the string is not valid.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

void LeddarCore::LdBoolProperty::PerformSetStringValue( size_t aIndex, const std::string &aValue )
{
    bool lNewValue = false;

    if( LeddarUtils::LtStringUtils::ToLower( aValue ) == "true" )
    {
        lNewValue = true;
    }
    else if( LeddarUtils::LtStringUtils::ToLower( aValue ) == "false" )
    {
        lNewValue = false;
    }
    else
    {
        throw( std::invalid_argument( "Invalid string value (use \"true\" or \"false\"." ) );
    }

    PerformSetValue( aIndex, lNewValue );
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBoolProperty::PerformForceStringValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Force the value
///
/// \param   aIndex  Index of value to write.
/// \param   aValue  The new value.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBoolProperty::PerformForceStringValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetStringValue( aIndex, aValue );
}

// *****************************************************************************
// Function: LdBoolProperty::PerformValue
//
/// \brief   Return the property value
///
/// \param   aIndex  Index of value.
///
/// \exception std::out_of_range Value out of range ( from std::stoi )
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

bool LeddarCore::LdBoolProperty::PerformValue( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= PerformCount() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    return reinterpret_cast<const bool *>( CStorage() )[aIndex];
}