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

#include "LtStringUtils.h"
#include "LtScope.h"

#include <sstream>
#include <string>
#include <cassert>
#include <limits>

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

LeddarCore::LdBitFieldProperty::LdBitFieldProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId,
        uint32_t aDeviceId, uint32_t aUnitSize, const std::string &aDescription ) :
    LdProperty( LdProperty::TYPE_BITFIELD, aCategory, aFeatures, aId, aDeviceId, aUnitSize, aUnitSize, aDescription ),
    mDoNotEmitSignal( false ),
    mExclusivityMask( 0 ),
    mLimit( 0 )
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

uint8_t
LeddarCore::LdBitFieldProperty::MaskToBit( uint64_t aMask )
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
// Function: LdBitFieldProperty::SetValue
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

void
LeddarCore::LdBitFieldProperty::SetValue( size_t aIndex, uint64_t aValue )
{
    if( !ValidateExclusivity( std::bitset<64>( aValue ) ) )
    {
        throw std::logic_error( "Several exclusive bits are set." );
    }

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( Count() == 0 && aIndex == 0 )
    {
        SetCount( 1 );
    }

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( Stride() == 1 )
    {
        SetValueT<uint8_t>( aIndex, aValue );
    }
    else if( Stride() == 2 )
    {
        SetValueT<uint16_t>( aIndex, aValue );
    }
    else if( Stride() == 4 )
    {
        SetValueT<uint32_t>( aIndex, aValue );
    }
    else if( Stride() == 8 )
    {
        SetValueT<uint64_t>( aIndex, aValue );
    }
    else
    {
        throw std::out_of_range( "Invalid stride" );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBitFieldProperty::ForceValue( size_t aIndex, uint64_t aValue )
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
void
LeddarCore::LdBitFieldProperty::ForceValue( size_t aIndex, uint64_t aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValue( aIndex, aValue );
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
template<typename T>
void LeddarCore::LdBitFieldProperty::SetValueT( size_t aIndex, uint64_t aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( Count() == 0 && aIndex == 0 )
    {
        SetCount( 1 );
    }

    if( aValue > mLimit )
    {
        throw std::out_of_range( "Value is bigger than the limit. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( sizeof( T ) != Stride() )
    {
        throw std::logic_error( "Template size does not correspond to stride. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    T *lValues = reinterpret_cast<T *>( Storage() );

    if( lValues[aIndex] != aValue )
    {
        lValues[aIndex] = static_cast<T>( aValue );

        if( !mDoNotEmitSignal )
        {
            EmitSignal( LdObject::VALUE_CHANGED );
        }
    }
}

//Template specialisation so it can be defined in the cpp file
template void LeddarCore::LdBitFieldProperty::SetValueT<uint8_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdBitFieldProperty::SetValueT<uint16_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdBitFieldProperty::SetValueT<uint32_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdBitFieldProperty::SetValueT<uint64_t>( size_t aIndex, uint64_t aValue );


// *****************************************************************************
// Function: LdBitFieldProperty::GetStringValue
//
/// \brief   Display the value in string format
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

std::string
LeddarCore::LdBitFieldProperty::GetStringValue( size_t aIndex ) const
{
    return LeddarUtils::LtStringUtils::IntToString( ValueT<uint64_t>( aIndex ), 2 );
}

// *****************************************************************************
// Function: LdBitFieldProperty::SetStringValue
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

void
LeddarCore::LdBitFieldProperty::SetStringValue( size_t aIndex, const std::string &aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( Count() == 0 && aIndex == 0 )
    {
        SetCount( 1 );
    }

    uint64_t lValue = ValueT<uint64_t>( aIndex );

    if( aValue.length() > UnitSize() * 8 )
    {
        throw std::out_of_range( "String too long. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    // SetBit must not emit a ValueChanged signal
    mDoNotEmitSignal = true;

    uint8_t bitIndex = UnitSize() * 8 - 1;

    for( size_t i = 0; i < UnitSize() * 8 - aValue.length(); ++i )
    {
        ResetBit( aIndex, bitIndex );
        --bitIndex;
    }

    for( size_t i = 0; i < aValue.length(); ++i )
    {
        char lChar = aValue[i];

        if( lChar != '0' && lChar != '1' && lChar != 'x' )
        {
            mDoNotEmitSignal = false;
            throw std::invalid_argument( "Invalid argument. The string can only contains 0, 1 and x characters. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        if( lChar == '1' )
        {
            SetBit( aIndex, bitIndex );
        }

        if( lChar == '0' )
        {
            ResetBit( aIndex, bitIndex );
        }

        --bitIndex;
    }

    mDoNotEmitSignal = false;

    uint64_t lNewValue = ValueT<uint64_t>( aIndex );

    if( lNewValue != lValue )
    {
        EmitSignal( LdObject::VALUE_CHANGED );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBitFieldProperty::ForceStringValue( size_t aIndex, const std::string &aValue )
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
void
LeddarCore::LdBitFieldProperty::ForceStringValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetStringValue( aIndex, aValue );
}

// *****************************************************************************
// Function: LdBitFieldProperty::Value
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
uint32_t
LeddarCore::LdBitFieldProperty::Value( size_t aIndex ) const
{
    return ValueT<uint32_t>( aIndex );
}

/// *****************************************************************************
/// Function: LdBitFieldProperty::ValidateExclusivity
///
/// \brief   True if only one bit is set in the exclusivity mask
///
/// \param   aValue  Value to test.
///
/// \author  David Levy
///
/// \since   June 2018
/// *****************************************************************************
bool
LeddarCore::LdBitFieldProperty::ValidateExclusivity( std::bitset<64> aValue )
{
    if( ( aValue &= mExclusivityMask ).count() > 1 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdBitFieldProperty::SetLimit( uint64_t aLimit )
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
void LeddarCore::LdBitFieldProperty::SetLimit( uint64_t aLimit )
{
    if( Count() > 0 && Value() > aLimit )
    {
        throw std::out_of_range( "Current value is bigger than the new limit. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    uint64_t lMaxValue = 0;

    if( UnitSize() == 1 )
    {
        lMaxValue = std::numeric_limits<uint8_t>::max();
    }
    else if( UnitSize() == 2 )
    {
        lMaxValue = std::numeric_limits<uint16_t>::max();
    }
    else if( UnitSize() == 4 )
    {
        lMaxValue = std::numeric_limits<uint32_t>::max();
    }
    else if( UnitSize() == 8 )
    {
        lMaxValue = std::numeric_limits<uint64_t>::max();
    }

    if( aLimit <= lMaxValue )
    {
        mLimit = aLimit;
    }
    else
    {
        throw std::out_of_range( "Limit is bigger than maximum possible value. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }
}

/// *****************************************************************************
/// Function: LdBitFieldProperty::ValueT
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
template<typename T>
T LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const
{
    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    uint64_t lValue = 0;

    if( Stride() == 1 )
    {
        lValue = reinterpret_cast<const uint8_t *>( CStorage() )[aIndex];
    }
    else if( Stride() == 2 )
    {
        lValue = reinterpret_cast<const uint16_t *>( CStorage() )[aIndex];
    }
    else if( Stride() == 4 )
    {
        lValue = reinterpret_cast<const uint32_t *>( CStorage() )[aIndex];
    }
    else if( Stride() == 8 )
    {
        lValue = reinterpret_cast<const uint64_t *>( CStorage() )[aIndex];
    }
    else
    {
        throw std::out_of_range( "Invalid stride. Bitfield property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( lValue > std::numeric_limits<T>::max() )
    {
        throw std::out_of_range( "Value is bigger than what the return type can hold. Use ValueT<TYPE> with a TYPE big enough. Bitfield property id: " +
                                 LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    return static_cast<T>( lValue );
}

//Template specialisation so it can be defined in the cpp file
template uint8_t LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const;
template uint16_t LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const;
template uint32_t LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const;
template uint64_t LeddarCore::LdBitFieldProperty::ValueT( size_t aIndex ) const;