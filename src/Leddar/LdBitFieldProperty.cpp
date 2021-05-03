// *****************************************************************************
// Module..: Leddar
//
/// \file    LdBitFieldProperty.cpp
///
/// \brief   Definition of LdBitFieldProperty class.
///          This property make easy to manipulates individual bits.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdBitFieldProperty.h"

#include "LtScope.h"
#include "LtStringUtils.h"

#include <cassert>
#include <limits>
#include <sstream>
#include <string>

// *****************************************************************************
// Function: LdBitFieldProperty::LdBitFieldProperty
//
/// \brief   Constructor.
///
/// \param   aCategory    See LdProperty.
/// \param   aFeatures    See LdProperty.
/// \param   aId          See LdProperty.
/// \param   aDeviceId    See LdProperty.
/// \param   aUnitSize    See LdProperty.
/// \param   aDescription See LdProperty.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
LeddarCore::LdBitFieldProperty::LdBitFieldProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint32_t aDeviceId, uint32_t aUnitSize,
                                                    const std::string &aDescription )
    : LdProperty( LdProperty::TYPE_BITFIELD, aCategory, aFeatures, aId, aDeviceId, aUnitSize, aUnitSize, aDescription )
    , mDoNotEmitSignal( false )
    , mExclusivityMask( 0 )
    , mLimit( 0 )
{
    if( aUnitSize == 1 )
    {
        mLimit = std::numeric_limits<uint8_t>::max();
    }
    else if( aUnitSize == 2 )
    {
        mLimit = std::numeric_limits<uint16_t>::max();
    }
    else if( aUnitSize == 4 )
    {
        mLimit = std::numeric_limits<uint32_t>::max();
    }
    else if( aUnitSize == 8 )
    {
        mLimit = std::numeric_limits<uint64_t>::max();
    }
    else
    {
        throw std::invalid_argument( "Invalid unit size" );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LeddarCore::LdBitFieldProperty::LdBitFieldProperty( const LdBitFieldProperty &aProperty )
///
/// \brief	Copy constructor
///
/// \author	Alain Ferron
/// \date	December 2020
///
/// \param	aProperty	The property.
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdBitFieldProperty::LdBitFieldProperty( const LdBitFieldProperty &aProperty )
    : LdProperty( aProperty )
{
    std::lock_guard<std::recursive_mutex> lock( aProperty.mPropertyMutex );
    mDoNotEmitSignal = aProperty.mDoNotEmitSignal;
    mExclusivityMask = aProperty.mExclusivityMask;
    mLimit           = aProperty.mLimit;
}

// *****************************************************************************
// Function: LdBitFieldProperty::MaskToBit
//
/// \brief   Static function to convert a single bit mask to its bit index.
///
/// \param   aMask  The mask value. Must have only one bit at 1 or an exception
///                 will be thrown.
///
/// \return  The bit index from 0 to 31 corresponding to aMask.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

uint8_t LeddarCore::LdBitFieldProperty::MaskToBit( uint64_t aMask )
{
    if( std::bitset<64>( aMask ).count() > 1 )
    {
        throw std::logic_error( "More than one bit are set." );
    }

    for( uint8_t i = 0; i < 64; ++i )
    {
        if( aMask == ( static_cast<uint64_t>( 1 ) << i ) )
        {
            return i;
        }
    }

    return 0;
}

// *****************************************************************************
// Function: LdBitFieldProperty::PerformSetValue
//
/// \brief   Change the value at the given index.
///
/// This is a raw acces to set the whole value at once. SetBit and ResetBit
/// can be used to change a single bit.
///
/// \param   aIndex  Index in array of value to change.
/// \param   aValue  New value to write.
///
/// \exception std::out_of_range Index not valid, verify property count.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

void LeddarCore::LdBitFieldProperty::PerformSetValue( size_t aIndex, uint64_t aValue )
{
    if( !PerformValidateExclusivity( std::bitset<64>( aValue ) ) )
    {
        throw std::logic_error( "Several exclusive bits are set." );
    }

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( PerformCount() == 0 && aIndex == 0 )
    {
        PerformSetCount( 1 );
    }

    if( aIndex >= PerformCount() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    if( PerformStride() == 1 )
    {
        SetValueT<uint8_t>( aIndex, aValue );
    }
    else if( PerformStride() == 2 )
    {
        SetValueT<uint16_t>( aIndex, aValue );
    }
    else if( PerformStride() == 4 )
    {
        SetValueT<uint32_t>( aIndex, aValue );
    }
    else if( PerformStride() == 8 )
    {
        SetValueT<uint64_t>( aIndex, aValue );
    }
    else
    {
        throw std::out_of_range( "Invalid stride" );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBitFieldProperty::PerformForceValue( size_t aIndex, uint64_t aValue )
///
/// \brief  Force value
///
///      This is a raw acces to set the whole value at once. SetBit and ResetBit
///     can be used to change a single bit.
///
/// \param   aIndex  Index in array of value to change.
/// \param   aValue  New value to write.
///
/// \exception std::out_of_range Index not valid, verify property count.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBitFieldProperty::PerformForceValue( size_t aIndex, uint64_t aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetValue( aIndex, aValue );
}

/// *****************************************************************************
/// Function: LdBitFieldProperty::SetValueT
///
/// \brief   Template to set the value depending on the stride of the property
///
/// This is a raw acces to set the whole value at once. SetBit and ResetBit
/// can be used to change a single bit.
///
/// \param   aIndex  Index in array of value to change.
/// \param   aValue  New value to write.
///
/// \exception std::out_of_range Value is bigger than what stride can hold.
///
/// \author  David Levy
///
/// \since   August 2018
/// *****************************************************************************
template <typename T> void LeddarCore::LdBitFieldProperty::SetValueT( size_t aIndex, uint64_t aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( PerformCount() == 0 && aIndex == 0 )
    {
        PerformSetCount( 1 );
    }

    if( aValue > mLimit )
    {
        throw std::out_of_range( "Value is bigger than the limit. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    if( sizeof( T ) != PerformStride() )
    {
        throw std::logic_error( "Template size does not correspond to stride. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    T *lValues = reinterpret_cast<T *>( Storage() );

    if( !IsInitialized() || lValues[aIndex] != aValue )
    {
        lValues[aIndex] = static_cast<T>( aValue );
        SetInitialized( true );

        if( !mDoNotEmitSignal )
        {
            EmitSignal( LdObject::VALUE_CHANGED );
        }
    }
}

// Template specialisation so it can be defined in the cpp file
template void LeddarCore::LdBitFieldProperty::SetValueT<uint8_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdBitFieldProperty::SetValueT<uint16_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdBitFieldProperty::SetValueT<uint32_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdBitFieldProperty::SetValueT<uint64_t>( size_t aIndex, uint64_t aValue );

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	LdProperty *LeddarCore::LdBitFieldProperty::PerformClone()
///
/// \brief	Performs the clone action
///
/// \returns	Null if it fails, else a pointer to a LdProperty.
///
/// \author	Alain Ferron
/// \date	March 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdProperty *LeddarCore::LdBitFieldProperty::PerformClone() { return new LdBitFieldProperty( *this ); }

// *****************************************************************************
// Function: LdBitFieldProperty::PerformGetStringValue
//
/// \brief   Display the value in string format
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
std::string LeddarCore::LdBitFieldProperty::PerformGetStringValue( size_t aIndex ) const { return LeddarUtils::LtStringUtils::IntToString( PerformValueT<uint64_t>( aIndex ), 2 ); }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBitFieldProperty::PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue )
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
void LeddarCore::LdBitFieldProperty::PerformSetAnyValue( size_t aIndex, const boost::any &aNewValue )
{
    if( aNewValue.type() == typeid( int ) )
    {
        PerformSetValue( aIndex, boost::any_cast<int>( aNewValue ) );
    }
    else if( aNewValue.type() == typeid( uint32_t ) )
    {
        PerformSetValue( aIndex, boost::any_cast<uint32_t>( aNewValue ) );
    }
    else if( aNewValue.type() == typeid( uint64_t ) )
    {
        PerformSetValue( aIndex, boost::any_cast<uint64_t>( aNewValue ) );
    }
    else
        throw std::invalid_argument( "Invalid value type" );
}

// *****************************************************************************
// Function: LdBitFieldProperty::PerformSetStringValue
//
/// \brief   Property writer for the value as text.
///
///
/// \param   aIndex  Index of value to write (binary representation).
/// \param   aValue  The new value.
///
/// \exception out_of_range : Longer string that what the property can hold
/// \exception invalid_argument : wrong string formatting
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

void LeddarCore::LdBitFieldProperty::PerformSetStringValue( size_t aIndex, const std::string &aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( PerformCount() == 0 && aIndex == 0 )
    {
        PerformSetCount( 1 );
        PerformSetValue( 0, 0 );
    }

    if( aValue.length() > PerformUnitSize() * 8 )
    {
        throw std::out_of_range( "String too long. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    // SetBit must not emit a ValueChanged signal
    mDoNotEmitSignal = true;

    if( !IsInitialized() )
        PerformSetValue( 0, 0 );

    uint8_t bitIndex = PerformUnitSize() * 8 - 1;

    for( size_t i = 0; i < PerformUnitSize() * 8 - aValue.length(); ++i )
    {
        PerformResetBit( aIndex, bitIndex );
        --bitIndex;
    }

    for( size_t i = 0; i < aValue.length(); ++i )
    {
        char lChar = aValue[i];

        if( lChar != '0' && lChar != '1' && lChar != 'x' )
        {
            mDoNotEmitSignal = false;
            throw std::invalid_argument( "Invalid argument. The string can only contains 0, 1 and x characters. Bitfield property id: " +
                                         LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
        }

        if( lChar == '1' )
        {
            PerformSetBit( aIndex, bitIndex );
        }

        if( lChar == '0' )
        {
            PerformResetBit( aIndex, bitIndex );
        }

        --bitIndex;
    }

    mDoNotEmitSignal = false;

    uint64_t lNewValue = PerformValueT<uint64_t>( aIndex );

    if( !IsInitialized() || lNewValue != PerformValueT<uint64_t>( aIndex ) )
    {
        SetInitialized( true );
        EmitSignal( LdObject::VALUE_CHANGED );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBitFieldProperty::PerformForceStringValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Force string value
///
/// \param   aIndex  Index of value to write (binary representation).
/// \param   aValue  The new value.
///
/// \exception out_of_range : Longer string that what the property can hold
/// \exception invalid_argument : wrong string formatting
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBitFieldProperty::PerformForceStringValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    PerformSetStringValue( aIndex, aValue );
}

// *****************************************************************************
// Function: LdBitFieldProperty::PerformValue
//
/// \brief   Return the property value
///
/// \param   aIndex  Index of value.
///
/// \exception std::out_of_range Index not valid (from ValueT).
/// \exception std::out_of_range Value larger than what the return type can hold (from ValueT).
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
uint32_t LeddarCore::LdBitFieldProperty::PerformValue( size_t aIndex ) const { return PerformValueT<uint32_t>( aIndex ); }

/// *****************************************************************************
/// Function: LdBitFieldProperty::PerformValidateExclusivity
///
/// \brief   True if only one bit is set in the exclusivity mask
///
/// \param   aValue  Value to test.
///
/// \author  David Levy
///
/// \since   June 2018
/// *****************************************************************************
bool LeddarCore::LdBitFieldProperty::PerformValidateExclusivity( const std::bitset<64> &aValue ) const
{
    auto lValue = aValue;
    if( ( lValue &= mExclusivityMask ).count() > 1 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBitFieldProperty::PerformSetLimit( uint64_t aLimit )
///
/// \brief  Sets the maximum value of the property
///
/// \exception  std::out_of_range   Thrown when aLimit argument is larger than the value the property can hold, or current value is bigger than new limit.
///
/// \param  aLimit  The limit.
///
/// \author David Levy
/// \date   April 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdBitFieldProperty::PerformSetLimit( uint64_t aLimit )
{
    if( PerformCount() > 0 && PerformValue() > aLimit )
    {
        throw std::out_of_range( "Current value is bigger than the new limit. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    uint64_t lMaxValue = 0;

    if( PerformUnitSize() == 1 )
    {
        lMaxValue = std::numeric_limits<uint8_t>::max();
    }
    else if( PerformUnitSize() == 2 )
    {
        lMaxValue = std::numeric_limits<uint16_t>::max();
    }
    else if( PerformUnitSize() == 4 )
    {
        lMaxValue = std::numeric_limits<uint32_t>::max();
    }
    else if( PerformUnitSize() == 8 )
    {
        lMaxValue = std::numeric_limits<uint64_t>::max();
    }

    if( aLimit <= lMaxValue )
    {
        mLimit = aLimit;
    }
    else
    {
        throw std::out_of_range( "Limit is bigger than maximum possible value. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }
}

/// *****************************************************************************
/// Function: LdBitFieldProperty::PerformValueT
///
/// \brief   Return the property value - Templated on the return type
///
/// \param   aIndex  Index of value.
///
/// \exception std::out_of_range Index not valid.
/// \exception std::out_of_range Value larger than what the return type can hold.
///
/// \author  David
///
/// \since   August 2018
/// *****************************************************************************
template <typename T> T LeddarCore::LdBitFieldProperty::PerformValueT( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= PerformCount() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    uint64_t lValue = 0;

    if( PerformStride() == 1 )
    {
        lValue = reinterpret_cast<const uint8_t *>( CStorage() )[aIndex];
    }
    else if( PerformStride() == 2 )
    {
        lValue = reinterpret_cast<const uint16_t *>( CStorage() )[aIndex];
    }
    else if( PerformStride() == 4 )
    {
        lValue = reinterpret_cast<const uint32_t *>( CStorage() )[aIndex];
    }
    else if( PerformStride() == 8 )
    {
        lValue = reinterpret_cast<const uint64_t *>( CStorage() )[aIndex];
    }
    else
    {
        throw std::out_of_range( "Invalid stride. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    if( lValue > std::numeric_limits<T>::max() )
    {
        throw std::out_of_range( "Value is bigger than what the return type can hold. Use ValueT<TYPE> with a TYPE big enough. Bitfield property id: " +
                                 LeddarUtils::LtStringUtils::IntToString( PerformGetId(), 16 ) );
    }

    return static_cast<T>( lValue );
}

// Template specialisation so it can be defined in the cpp file
template uint8_t LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const;
template uint16_t LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const;
template uint32_t LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const;
template uint64_t LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const;