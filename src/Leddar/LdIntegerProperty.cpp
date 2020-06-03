// *****************************************************************************
// Module..: Leddar
//
/// \file    LdIntegerProperty.cpp
///
/// \brief   Definition of functions for class LdIntegerProperty.
///
/// \author  Patrick Boulay
///
/// \since   February 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdIntegerProperty.h"
#include "LtStringUtils.h"
#include "LtScope.h"

#include <cassert>
#include <limits>


// *****************************************************************************
// Function: LdIntegerProperty::LdIntegerProperty
//
/// \brief   Constructor.
///
/// The limits are set to the maximum range for the unit size.
///
/// \param   aFeatures    See LdProperty.
/// \param   aId          See LdProperty.
/// \param   aDeviceId    See LdProperty.
/// \param   aUnitSize    See LdProperty.
/// \param   aCategory    See LdProperty.
/// \param   aDescription See LdProperty.
/// \param   aSigned      Is the integer signed?
///
/// \exception out_of_range : invalid unit size
///
/// \author  Louis Perreault
///
/// \since   August 2014
// *****************************************************************************

LeddarCore::LdIntegerProperty::LdIntegerProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId,
        uint16_t aDeviceId, uint32_t aUnitSize, const std::string &aDescription, const bool aSigned ) :
    LdProperty( LdProperty::TYPE_INTEGER, aCategory, aFeatures, aId, aDeviceId, aUnitSize, aUnitSize, aDescription ),
    mMinValueS( 0 ),
    mMaxValueS( 0 ),
    mMinValueU( 0 ),
    mMaxValueU( 0 ),
    mSigned( aSigned )
{
    assert( ( aUnitSize == 1 ) || ( aUnitSize == 2 ) || ( aUnitSize == 4 ) || ( aUnitSize == 8 ) );

    switch( UnitSize() )
    {
        case 1:
            if( mSigned )
            {
                mMaxValueS = std::numeric_limits<int8_t>::max();
                mMinValueS = std::numeric_limits<int8_t>::min();
            }
            else
                mMaxValueU = std::numeric_limits<uint8_t>::max();

            break;

        case 2:
            if( mSigned )
            {
                mMaxValueS = std::numeric_limits<int16_t>::max();
                mMinValueS = std::numeric_limits<int16_t>::min();
            }
            else
                mMaxValueU = std::numeric_limits<uint16_t>::max();

            break;

        case 4:
            if( mSigned )
            {
                mMaxValueS = std::numeric_limits<int32_t>::max();
                mMinValueS = std::numeric_limits<int32_t>::min();
            }
            else
                mMaxValueU = std::numeric_limits<uint32_t>::max();

            break;

        case 8:
            if( mSigned )
            {
                mMaxValueS = std::numeric_limits<int64_t>::max();
                mMinValueS = std::numeric_limits<int64_t>::min();
            }
            else
                mMaxValueU = std::numeric_limits<uint64_t>::max();

            break;

        default:
            throw std::out_of_range( "Invalid unit size." );
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn int64_t LeddarCore::LdIntegerProperty::MinValue( void ) const
///
/// \brief  Minimum value
///
/// \author David Levy
/// \date   August 2018
///
/// \exception  std::out_of_range   Thrown when the return type is not big enough for the return
///                                 value.
///
/// \return Minimum value.
////////////////////////////////////////////////////////////////////////////////////////////////////
int64_t
LeddarCore::LdIntegerProperty::MinValue( void ) const
{
    return MinValueT<int64_t>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn template<typename T> T LeddarCore::LdIntegerProperty::MinValueT( void ) const
///
/// \brief  Minimum value templated on return type.
///
/// \author David Levy
/// \date   August 2018
///
/// \exception  std::out_of_range   Thrown when the return type is not big enough for the return value
/// \exception  std::logic_error    Signed and unsigned mix
///
/// \tparam T   Generic type parameter. (uint8 to uint64 and int8 to int64).
///
/// \return Minimum value of the property typed as T.
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T LeddarCore::LdIntegerProperty::MinValueT( void ) const
{
    if( mSigned )
    {
        if( mMinValueS < 0 && T( -1 ) > T( 0 ) ) // unsigned T
        {
            throw std::out_of_range( "Value is negative with an unsigned return type. Use MinValueT<TYPE> with a signed TYPE. Property id: "
                                     + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }
        else if( mMinValueS > 0 && T( -1 ) > T( 0 ) ) // unsigned T
        {
            if( static_cast<uint64_t>( mMinValueS ) > static_cast<uint64_t>( std::numeric_limits<T>::max() ) )
            {
                throw std::out_of_range( "Return type is not big enough for the value. Use MinValueT<type> with a type big enough. Property id: " +
                                         LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }
        }
        else //signed T
        {
            if( mMinValueS > static_cast<int64_t>( std::numeric_limits<T>::max() ) || mMinValueS < static_cast<int64_t>( std::numeric_limits<T>::min() ) )
            {
                throw std::out_of_range( "Return type is not big enough for the value. Use MinValueT<type> with a type big enough. Property id: " +
                                         LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }
        }

        return static_cast<T>( mMinValueS );
    }
    else
    {
        if( mMinValueU > static_cast<uint64_t>( std::numeric_limits<T>::max() ) )
        {
            throw std::out_of_range( "Return type is not big enough for the value. Use MinValueT<type> with a type big enough. Property id: " +
                                     LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        return static_cast<T>( mMinValueU );
    }
}

//Template specialisation so it can be defined in the cpp file
template uint8_t LeddarCore::LdIntegerProperty::MinValueT( void ) const;
template int8_t LeddarCore::LdIntegerProperty::MinValueT( void )const;
template uint16_t LeddarCore::LdIntegerProperty::MinValueT( void ) const;
template int16_t LeddarCore::LdIntegerProperty::MinValueT( void ) const;
template uint32_t LeddarCore::LdIntegerProperty::MinValueT( void ) const;
template int32_t LeddarCore::LdIntegerProperty::MinValueT( void ) const;
template uint64_t LeddarCore::LdIntegerProperty::MinValueT( void ) const;
template int64_t LeddarCore::LdIntegerProperty::MinValueT( void ) const;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn int64_t LeddarCore::LdIntegerProperty::MaxValue( void ) const
///
/// \brief  Maximum value
///
/// \author David Levy
/// \date   August 2018
///
/// \exception  std::out_of_range   Thrown when the return type is not big enough for the return value
///
/// \return An int64_t.
////////////////////////////////////////////////////////////////////////////////////////////////////
int64_t
LeddarCore::LdIntegerProperty::MaxValue( void ) const
{
    return MaxValueT<int64_t>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn template<typename T> T LeddarCore::LdIntegerProperty::MaxValueT( void ) const
///
/// \brief  Maximum value templated on return type.
///
/// \author David Levy
/// \date   August 2018
///
/// \exception  std::out_of_range   Thrown when the return type is not big enough for the return value
///
/// \tparam T   Generic type parameter. (uint8 to uint64 and int8 to int64).
///
/// \return Minimum Maximum of the property typed as T.
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T LeddarCore::LdIntegerProperty::MaxValueT( void ) const
{
    if( mSigned )
    {
        if( mMaxValueS < 0 && T( -1 ) > T( 0 ) ) // unsigned T
        {
            throw std::out_of_range( "Value is negative with an unsigned return type. Use MaxValueT<TYPE> with a signed TYPE. Property id: "
                                     + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }
        else if( mMaxValueS > 0 && T( -1 ) > T( 0 ) ) // unsigned T
        {
            if( static_cast<uint64_t>( mMaxValueS ) > static_cast<uint64_t>( std::numeric_limits<T>::max() ) )
            {
                throw std::out_of_range( "Return type is not big enough for the value. Use MaxValueT<type> with a type big enough. Property id: " +
                                         LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }
        }
        else //signed T
        {
            if( mMaxValueS > static_cast<int64_t>( std::numeric_limits<T>::max() ) || mMaxValueS < static_cast<int64_t>( std::numeric_limits<T>::min() ) )
            {
                throw std::out_of_range( "Return type is not big enough for the value. Use MaxValueT<type> with a type big enough. Property id: " +
                                         LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }
        }

        return static_cast<T>( mMaxValueS );
    }
    else
    {
        if( mMaxValueU > static_cast<uint64_t>( std::numeric_limits<T>::max() ) )
        {
            throw std::out_of_range( "Return type is not big enough for the value. Use MaxValueT<type> with a type big enough. Property id: " +
                                     LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        return static_cast<T>( mMaxValueU );
    }
}

//Template specialisation so it can be defined in the cpp file
template uint8_t LeddarCore::LdIntegerProperty::MaxValueT( void ) const;
template int8_t LeddarCore::LdIntegerProperty::MaxValueT( void )const;
template uint16_t LeddarCore::LdIntegerProperty::MaxValueT( void ) const;
template int16_t LeddarCore::LdIntegerProperty::MaxValueT( void ) const;
template uint32_t LeddarCore::LdIntegerProperty::MaxValueT( void ) const;
template int32_t LeddarCore::LdIntegerProperty::MaxValueT( void ) const;
template uint64_t LeddarCore::LdIntegerProperty::MaxValueT( void ) const;
template int64_t LeddarCore::LdIntegerProperty::MaxValueT( void ) const;

// *****************************************************************************
// Function: LdIntegerProperty::SetLimits
//
/// \brief   Change the minimum and maximum allowed values.
///
/// The current value(s) will be clipped to the new limits.
///
/// \param   aMin  The minimum allowed value.
/// \param   aMax  The maximum allowed value.
///
/// \exception invalid_argument : if min > max
/// \exception out_of_range : if property hold a uint64_t
///
/// \author  Louis Perreault
///
/// \since   August 2014
// *****************************************************************************
void
LeddarCore::LdIntegerProperty::SetLimits( int64_t aMin, int64_t aMax )
{
    if( aMin > aMax )
    {
        throw std::invalid_argument( "SetLimits(): Invalid min value is higher than the max value. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 )
                                     + "(" + GetDescription() + ")"
                                     + " min: " + LeddarUtils::LtStringUtils::IntToString( aMin, 10 )
                                     + +" max: " + LeddarUtils::LtStringUtils::IntToString( aMax, 10 ) );
    }

    if( !mSigned && UnitSize() == 8 )
    {
        throw std::out_of_range( "Limit can be too big, use SetLimitsUnsigned() function instead. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( mSigned )
    {
        if( ( aMin != mMinValueS ) || ( aMax != mMaxValueS ) )
        {
            mMinValueS = aMin;
            mMaxValueS = aMax;

            // Clip current values
            const size_t lCount = Count();
            bool lValueChanged = false;

            if( lCount > 0 && IsInitialized() )
            {
                for( size_t i = 0; i < lCount; ++i )
                {
                    int64_t lValue = Value( i );

                    if( lValue < mMinValueS )
                    {
                        lValueChanged = true;
                        SetValue( i, mMinValueS );
                    }
                    else if( lValue > mMaxValueS )
                    {
                        lValueChanged = true;
                        SetValue( i, mMaxValueS );
                    }
                }
            }

            EmitSignal( LdObject::LIMITS_CHANGED );

            if( lValueChanged )
            {
                EmitSignal( LdObject::VALUE_CHANGED );
            }
        }
    }
    else
    {
        SetLimitsUnsigned( static_cast<uint64_t>( aMin ), static_cast<uint64_t>( aMax ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdIntegerProperty::SetLimitsUnsigned( uint64_t aMin, uint64_t aMax )
///
/// \brief  Sets limits for unsigned properties
///
/// \author David Levy
/// \date   August 2018
///
/// \exception  std::invalid_argument   if min > max
/// \exception  std::logic_error        When used on a signed property
///
/// \param  aMin    The minimum.
/// \param  aMax    The maximum.
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdIntegerProperty::SetLimitsUnsigned( uint64_t aMin, uint64_t aMax )
{
    if( aMin > aMax )
    {
        throw std::invalid_argument( "SetLimits(): Invalid min value is higher than the max value. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 )
                                     + "(" + GetDescription() + ")"
                                     + " min: " + LeddarUtils::LtStringUtils::IntToString( aMin, 10 )
                                     + +" max: " + LeddarUtils::LtStringUtils::IntToString( aMax, 10 ) );
    }

    if( mSigned )
    {
        throw std::logic_error( "Use SetLimits() for signed properties. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( ( aMin != mMinValueU ) || ( aMax != mMaxValueU ) )
    {
        mMinValueU = aMin;
        mMaxValueU = aMax;

        // Clip current values
        const size_t lCount = Count();
        bool lValueChanged = false;

        if( lCount > 0 && IsInitialized() )
        {
            for( size_t i = 0; i < lCount; ++i )
            {
                uint64_t lValue = ValueT<uint64_t>( i );

                if( lValue < mMinValueU )
                {
                    lValueChanged = true;
                    SetValueUnsigned( i, mMinValueU );
                }
                else if( lValue > mMaxValueU )
                {
                    lValueChanged = true;
                    SetValueUnsigned( i, mMaxValueU );
                }
            }
        }

        EmitSignal( LdObject::LIMITS_CHANGED );

        if( lValueChanged )
        {
            EmitSignal( LdObject::VALUE_CHANGED );
        }
    }
}


/// *****************************************************************************
/// Function: LdIntegerProperty::SetValue
///
/// \brief   Change the current value at the given index.
///
/// \param   aIndex  Index of value to change.
/// \param   aValue  The new value.
///
/// \exception std::out_of_range Index not valid, verify property count.
/// \exception std::out_of_range If value outside of limits.
/// \exception std::out_of_range If using this function for uint64_t property.
///
/// \author  Louis Perreault
///
/// \since   August 2014
/// *****************************************************************************
void
LeddarCore::LdIntegerProperty::SetValue( size_t aIndex, int64_t aValue )
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

    if( mSigned )
    {

        if( ( aValue < mMinValueS ) || ( aValue > mMaxValueS ) )
        {
            throw std::out_of_range( "Value out of range. Check min and max value. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        if( Stride() == 1 )
        {
            if( aValue > std::numeric_limits<int8_t>::max() || aValue < std::numeric_limits<int8_t>::min() )
            {
                throw std::out_of_range( "Value is too big. Increase stride/unitsize. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }

            SetValueT<int8_t>( aIndex, static_cast<int8_t>( aValue ) );
        }
        else if( Stride() == 2 )
        {
            if( aValue > std::numeric_limits<int16_t>::max() || aValue < std::numeric_limits<int16_t>::min() )
            {
                throw std::out_of_range( "Value is too big. Increase stride/unitsize. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }

            SetValueT<int16_t>( aIndex, static_cast<int16_t>( aValue ) );
        }
        else if( Stride() == 4 )
        {
            if( aValue > std::numeric_limits<int32_t>::max() || aValue < std::numeric_limits<int32_t>::min() )
            {
                throw std::out_of_range( "Value is too big. Increase stride/unitsize. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }

            SetValueT<int32_t>( aIndex, static_cast<int32_t>( aValue ) );
        }
        else if( Stride() == 8 )
        {
            SetValueT<int64_t>( aIndex, aValue );
        }
        else
        {
            throw std::logic_error( "Invalid stride." ); //Unreachable case
        }
    }
    else
    {
        if( aValue < 0 )
        {
            throw std::out_of_range( "Negative value for unsigned property. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        SetValueUnsigned( aIndex, static_cast<uint64_t>( aValue ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdIntegerProperty::ForceValue( size_t aIndex, int64_t aValue )
///
/// \brief  Force the current value at the given index.
///
/// \exception  std::out_of_range   Index not valid, verify property count.
/// \exception  std::out_of_range   If value outside of limits.
/// \exception  std::out_of_range   If using this function for uint64_t property.
///
/// \param  aIndex  Index of value to change.
/// \param  aValue  The new value.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdIntegerProperty::ForceValue( size_t aIndex, int64_t aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValue( aIndex, aValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdIntegerProperty::SetValueUnsigned( size_t aIndex, uint64_t aValue )
///
/// \brief  Sets value for unsigned properties
///
/// \exception  std::out_of_range   Index not valid, verify property count.
/// \exception  std::out_of_range   When requested value to set is too large for the property
/// \exception  std::logic_error    When using this function for a signed property.
///
/// \param  aIndex  Zero-based index of the index of the property.
/// \param  aValue  The value to set.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdIntegerProperty::SetValueUnsigned( size_t aIndex, uint64_t aValue )
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

    if( mSigned )
    {
        throw std::logic_error( "Use SetValue() for signed properties. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( ( aValue < mMinValueU ) || ( aValue > mMaxValueU ) )
    {
        throw std::out_of_range( "Value out of range. Check min and max value. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( Stride() == 1 )
    {
        if( aValue > std::numeric_limits<uint8_t>::max() )
        {
            throw std::out_of_range( "Value is too big. Increase stride/unitsize. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        SetValueT<uint8_t>( aIndex, static_cast<uint8_t>( aValue ) );
    }
    else if( Stride() == 2 )
    {
        if( aValue > std::numeric_limits<uint16_t>::max() )
        {
            throw std::out_of_range( "Value is too big. Increase stride/unitsize. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        SetValueT<uint16_t>( aIndex, static_cast<uint16_t>( aValue ) );
    }
    else if( Stride() == 4 )
    {
        if( aValue > std::numeric_limits<uint32_t>::max() )
        {
            throw std::out_of_range( "Value is too big. Increase stride/unitsize. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        SetValueT<uint32_t>( aIndex, static_cast<uint32_t>( aValue ) );
    }
    else if( Stride() == 8 )
    {
        SetValueT<uint64_t>( aIndex, aValue );
    }
    else
    {
        throw std::logic_error( "Invalid stride." ); //Unreachable case
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdIntegerProperty::ForceValueUnsigned( size_t aIndex, uint64_t aValue )
///
/// \brief  Force value unsigned
///
/// \exception  std::out_of_range   Index not valid, verify property count.
/// \exception  std::out_of_range   When requested value to set is too large for the property
/// \exception  std::logic_error    When using this function for a signed property.
///
/// \param  aIndex  Zero-based index of the index of the property.
/// \param  aValue  The value to set.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdIntegerProperty::ForceValueUnsigned( size_t aIndex, uint64_t aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValueUnsigned( aIndex, aValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn template<typename T> void LeddarCore::LdIntegerProperty::SetValueT( size_t aIndex, T aValue )
///
/// \brief  Template to set value depending on stride / unit size
///         Limits checking is done in SetValue or SetValueUnsigned
///
/// \author David Levy
/// \date   August 2018
///
/// \exception  std::logic_error    When stride doesnt correspond to T size
///
/// \tparam T   Generic type parameter (int8 to int64).
/// \param  aIndex  Zero-based index of the property to set.
/// \param  aValue  The value to set.
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void LeddarCore::LdIntegerProperty::SetValueT( size_t aIndex, T aValue )
{
    CanEdit();

    if( sizeof( T ) != Stride() )
    {
        throw std::logic_error( "Template size does not correspond to stride. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    T *lValues = reinterpret_cast<T *>( Storage() );

    if( !IsInitialized() || lValues[aIndex] != aValue )
    {
        SetInitialized( true );
        lValues[aIndex] = static_cast<T>( aValue );
        EmitSignal( LdObject::VALUE_CHANGED );
    }
}

// *****************************************************************************
// Function: LdIntegerProperty::GetStringValue
//
/// \brief   Display the value in string format
///
/// \author  Patrick Boulay
///
/// \since   February 2016
// *****************************************************************************

std::string
LeddarCore::LdIntegerProperty::GetStringValue( size_t aIndex ) const
{
    if( mSigned )
        return LeddarUtils::LtStringUtils::IntToString( Value( aIndex ) );
    else
        return LeddarUtils::LtStringUtils::IntToString( ValueT<uint64_t>( aIndex ) );
}


// *****************************************************************************
// Function: LdIntegerProperty::SetStringValue
//
/// \brief   Property writer for the value as text.
///
/// See SetStringValue with 3 arguments for more information.
/// This function is to be compliant with the virtual SetStringValue function.
///
/// \param   aIndex  Index of value to write.
/// \param   aValue  The new value.
///
/// \exception std::invalid_argument No conversion could be be performed ( from std::stoi )
/// \exception std::out_of_range Value out of range ( from std::stoi )
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

void
LeddarCore::LdIntegerProperty::SetStringValue( size_t aIndex, const std::string &aValue )
{
    SetStringValue( aIndex, aValue, 10 );
}

// *****************************************************************************
// Function: LdIntegerProperty::SetStringValue
//
/// \brief   Property writer for the value as text.
///
/// The text must be valid as a integer value in base 10. The value will only
/// be changed if the new text, once standardized is different than the
/// current text value (so changes must be seen within the number of
/// decimals configured).
///
/// \param   aIndex  Index of value to write.
/// \param   aValue  The new value.
/// \param   aBase   Base of the number.
///
/// \exception std::invalid_argument Invalid input string, no conversion could be performed.
/// \exception std::overflow_error The value exceed maximum of Int value.
/// \exception std::underflow_error The value is under the minimum of Int value.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
void
LeddarCore::LdIntegerProperty::SetStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase )
{
    CanEdit();
    std::string lCurrent = "";

    if( IsInitialized() )
        lCurrent = GetStringValue( aIndex );

    if( !IsInitialized() || lCurrent != aValue )
    {
        SetValue( aIndex, LeddarUtils::LtStringUtils::StringToInt( aValue, aBase ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdIntegerProperty::ForceStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase )
///
/// \brief   Property writer for the value as text.
///
/// The text must be valid as a integer value in provided base. The value will only
/// be changed if the new text, once standardized is different than the
/// current text value (so changes must be seen within the number of
/// decimals configured).
///
/// \param   aIndex  Index of value to write.
/// \param   aValue  The new value.
/// \param   aBase   Base of the number.
///
/// \exception std::invalid_argument Invalid input string, no conversion could be performed.
/// \exception std::overflow_error The value exceed maximum of Int value.
/// \exception std::underflow_error The value is under the minimum of Int value.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdIntegerProperty::ForceStringValue( size_t aIndex, const std::string &aValue, uint8_t aBase )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetStringValue( aIndex, aValue, aBase );
}

// *****************************************************************************
// Function: LdIntegerProperty::Value
//
/// \brief   Return the property value
///
/// \param   aIndex  Index of value.
///
/// \exception std::out_of_range Value out of range ( wrong index )
/// \exception std::out_of_range Value out of range ( if return type is not large enough )
/// \exception std::out_of_range Invalid stride
///
/// \author  David Levy
///
/// \since   August 2018
// *****************************************************************************
int64_t
LeddarCore::LdIntegerProperty::Value( size_t aIndex ) const
{
    return ValueT<int64_t>( aIndex );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn template<typename T> T LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const
///
/// \brief  Return the property value as the requested type
///
/// \author David Levy
/// \date   March 2018
///
/// \exception  std::out_of_range   Value out of range ( wrong index )
/// \exception  std::logic_error    Unreachable case.
/// \exception  std::out_of_range   Value out of range ( if return type is not large enough )
/// \exception  std::out_of_range   Invalid stride.
///
/// \tparam T   Generic type parameter.
/// \param  aIndex  Zero-based index of the value to get.
///
/// \return A T.
///
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( mSigned )
    {
        int64_t lValue = 0;

        if( Stride() == 1 )
        {
            lValue = reinterpret_cast<const int8_t *>( CStorage() )[aIndex];
        }
        else if( Stride() == 2 )
        {
            lValue = reinterpret_cast<const int16_t *>( CStorage() )[aIndex];
        }
        else if( Stride() == 4 )
        {
            lValue = reinterpret_cast<const int32_t *>( CStorage() )[aIndex];
        }
        else if( Stride() == 8 )
        {
            lValue = reinterpret_cast<const int64_t *>( CStorage() )[aIndex];
        }
        else
        {
            throw std::out_of_range( "Invalid stride" );
        }

        if( lValue < 0 && T( -1 ) > T( 0 ) ) // unsigned T
        {
            throw std::out_of_range( "Value is negative with an unsigned return type. Use ValueT<TYPE> with a signed TYPE. Property id: "
                                     + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }
        else if( lValue >= 0 && T( -1 ) > T( 0 ) ) // unsigned T
        {
            if( static_cast<uint64_t>( lValue ) > static_cast<uint64_t>( std::numeric_limits<T>::max() ) )
            {
                throw std::out_of_range( "Value is bigger than what the return type can hold. Use ValueT<TYPE> with a TYPE big enough. Property id: "
                                         + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }
        }
        else if( T( -1 ) < T( 0 ) ) // signed T
        {
            if( lValue > static_cast<int64_t>( std::numeric_limits<T>::max() ) || lValue < static_cast<int64_t>( std::numeric_limits<T>::min() ) )
            {
                throw std::out_of_range( "Value is bigger than what the return type can hold. Use ValueT<TYPE> with a TYPE big enough. Property id: "
                                         + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
            }
        }
        else
        {
            throw std::logic_error( "This case should not be reached." );
        }

        return static_cast<T>( lValue );
    }
    else
    {
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
            throw std::out_of_range( "Invalid stride" );
        }

        if( lValue > static_cast<uint64_t>( std::numeric_limits<T>::max() ) )
        {
            throw std::out_of_range( "Value is bigger than what the return type can hold. Use ValueT<TYPE> with a TYPE big enough. Property id: "
                                     + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
        }

        return static_cast<T>( lValue );
    }
}

//Template specialisation so it can be defined in the cpp file
template uint8_t LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const;
template int8_t LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const;
template uint16_t LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const;
template int16_t LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const;
template uint32_t LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const;
template int32_t LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const;
template uint64_t LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const;
template int64_t LeddarCore::LdIntegerProperty::ValueT( size_t aIndex ) const;