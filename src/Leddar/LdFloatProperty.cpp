// *****************************************************************************
// Module..: Leddar
//
/// \file    LdFloatProperty.cpp
///
/// \brief   Definition of functions for class LdFloatProperty.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdFloatProperty.h"
#include "LtStringUtils.h"
#include "LtScope.h"

#include <cassert>
#include <iomanip>
#include <ios>
#include <limits>
#include <math.h>
#include <sstream>
#include <string>

// *****************************************************************************
// Function: LdFloatProperty::LdFloatProperty
//
/// \brief   Constructor.
///
/// The limits are set to the maximum range for a float. If fixed point
/// format they are then updated to the maximum raw range.
///
/// \param   aFeatures  See LdProperty.
/// \param   aId        See LdProperty.
/// \param   aDeviceId  See LdProperty.
/// \param   aUnitSize  See LdProperty.
/// \param   aScale     The scale factor between the floating point and
///                     fixed point representations, i.e. fixed point =
///                     floating point * scale. If the scale is 0, the value
///                     is natively stored as a float.
/// \param   aDecimals  The number of decimals to use when transforming a
///                     value to its text form.
/// \param aCategory    See LdProperty.
/// \param aDescription See LdProperty.
///
/// \author  Louis Perreault
///
/// \since   August 2014
// *****************************************************************************
LeddarCore::LdFloatProperty::LdFloatProperty( LdProperty::eCategories aCategory,
        uint32_t aFeatures, uint32_t aId,
        uint16_t aDeviceId, uint32_t aUnitSize,
        uint32_t aScale, uint32_t aDecimals, const std::string &aDescription ) :
    LdProperty( LdProperty::TYPE_FLOAT, aCategory, aFeatures, aId, aDeviceId, aUnitSize, sizeof( int32_t ), aDescription ),
    mMinValue( -std::numeric_limits<float>::max() ),
    mMaxValue( std::numeric_limits<float>::max() ),
    mScale( aScale ),
    mDecimals( aDecimals )
{
    SetMaxLimits();
}

// *****************************************************************************
// Function: LdFloatProperty::Value
//
/// \brief   Return the current value of the property at the given index.
///
/// \param   aIndex  Index of value to get.
///
/// \return  The value.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
float
LeddarCore::LdFloatProperty::Value( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    // If scale is 0 value is a float directly. Otherwise it is a fixed-point
    // that we must transform into a float.
    if( mScale != 0 )
    {
        return static_cast<float>( RawValue( aIndex ) ) / mScale;
    }

    // cppcheck-suppress invalidPointerCast
    return reinterpret_cast<const float *>( CStorage() )[aIndex];
}

// *****************************************************************************
// Function: LdFloatProperty::DeviceValue
//
/// \brief   Return the backup value (that is the value in the device).
///
/// \param   aIndex  Index of value to get.
///
/// \return  The value.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
float
LeddarCore::LdFloatProperty::DeviceValue( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    // If scale is 0 value is a float directly. Otherwise it is a fixed-point
    // that we must transform into a float.
    if( mScale != 0 )
    {
        return static_cast<float>( RawDeviceValue( aIndex ) ) / mScale;
    }

    // cppcheck-suppress invalidPointerCast
    return reinterpret_cast<const float *>( BackupStorage() )[aIndex];
}

// *****************************************************************************
// Function: LdFloatProperty::GetStringValue
//
/// \brief   Utility function to transform a float value to text in a standard
///          way.
///
/// \param   aIndex  Index value to tranform to text
///
/// \return  The value in its textual form.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
std::string
LeddarCore::LdFloatProperty::GetStringValue( size_t aIndex ) const
{
    std::stringstream lResult;
    lResult << std::fixed << std::setprecision( mDecimals ) << Value( aIndex );
    return lResult.str();
}

// *****************************************************************************
// Function: LdFloatProperty::SetMaxLimits
//
/// \brief   Set the limits to the maximum range from the current scale and
///          unit size.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
void
LeddarCore::LdFloatProperty::SetMaxLimits( void )
{
    if( mScale != 0 )
    {
        switch( UnitSize() )
        {
            case 1:
                mMinValue = std::numeric_limits<char>::min() / static_cast<float>( mScale );
                mMaxValue = std::numeric_limits<char>::max() / static_cast<float>( mScale );
                break;

            case 2:
                mMinValue = std::numeric_limits<int16_t>::min() / static_cast<float>( mScale );
                mMaxValue = std::numeric_limits<int16_t>::max() / static_cast<float>( mScale );
                break;

            case 4:
                mMinValue = std::numeric_limits<int32_t>::min() / static_cast<float>( mScale );
                mMaxValue = std::numeric_limits<int32_t>::max() / static_cast<float>( mScale );
                break;
        }

        // Truncate to the nearest value according to the decimals
        const float lRounder = static_cast<float>( static_cast<int>( pow( 10, mDecimals ) ) );

        mMinValue = ( mMinValue * lRounder ) / lRounder;
        mMaxValue = ( mMaxValue * lRounder ) / lRounder;
    }
}

// *****************************************************************************
// Function: LdFloatProperty::SetLimits
//
/// \brief   Change the minimum and maximum allowed values.
///
/// The current value(s) will be clipped to the new limits.
///
/// \param   aMin  The minimum allowed value.
/// \param   aMax  The maximum allowed value.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
void
LeddarCore::LdFloatProperty::SetLimits( float aMin, float aMax )
{
    if( aMin > aMax )
    {
        throw std::invalid_argument( "Invalid min value is higher than the max value." );
    }

    if( ( aMin != mMinValue ) || ( aMax != mMaxValue ) )
    {
        mMinValue = aMin;
        mMaxValue = aMax;

        // Clip current values only if the property was initialized
        if( IsInitialized() )
        {
            const size_t lCount = Count();

            for( size_t i = 0; i < lCount; ++i )
            {
                const float lValue = Value( i );

                if( lValue < mMinValue )
                {
                    SetValue( i, mMinValue );
                }
                else if( lValue > mMaxValue )
                {
                    SetValue( i, mMaxValue );
                }
            }
        }

        EmitSignal( LdObject::LIMITS_CHANGED );
    }
}

// *****************************************************************************
// Function: LdFloatProperty::SetRawLimits
//
/// \brief   Change the minimum and maximum allowed values using raw values.
///
/// The current value(s) will be clipped to the new limits.
///
/// \param   aMin  The minimum allowed value.
/// \param   aMax  The maximum allowed value.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************
void
LeddarCore::LdFloatProperty::SetRawLimits( int32_t aMin, int32_t aMax )
{
    float lMinValue = mScale == 0 ? aMin : aMin / static_cast<float>( mScale );
    float lMaxValue = mScale == 0 ? aMax : aMax / static_cast<float>( mScale );

    SetLimits( lMinValue, lMaxValue );
}

// *****************************************************************************
// Function: LdFloatProperty::SetRawValue
//
/// \brief   Change the current raw value at the given index.
///
/// Limits are not checked in this mode. The native representation must be
/// fixed point or the resulting stored value will be garbage.
///
/// \param   aIndex  Index of value to change.
/// \param   aValue  The new value.
///
/// \exception std::out_of_range Invalid property count
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
void
LeddarCore::LdFloatProperty::SetRawValue( size_t aIndex, int32_t aValue )
{
    CanEdit();

    // Initialize the count to 1 on the fist SetValue if not done before.
    if( Count() == 0 && aIndex == 0 )
    {
        SetCount( 1 );
    }

    assert( mScale != 0 );

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Invalid property count." );
    }

    if( ( aValue < mMinValue * mScale ) || ( aValue > mMaxValue * mScale ) )
    {
        throw std::out_of_range( "Value outside the limits. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( !IsInitialized() || aValue != RawValue( aIndex ) )
    {
        reinterpret_cast<int32_t *>( Storage() )[aIndex] = aValue;
        SetInitialized( true );
        EmitSignal( LdObject::VALUE_CHANGED );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdFloatProperty::ForceRawValue( size_t aIndex, int32_t aValue )
///
/// \brief  Force raw value
///
/// Limits are not checked in this mode. The native representation must be
/// fixed point or the resulting stored value will be garbage.
///
/// \param   aIndex  Index of value to change.
/// \param   aValue  The new value.
///
/// \exception std::out_of_range Invalid property count
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdFloatProperty::ForceRawValue( size_t aIndex, int32_t aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetRawValue( aIndex, aValue );
}

// *****************************************************************************
// Function: LdFloatProperty::SetValue
//
/// \brief   Change the current value at the given index.
///
/// An exception will be throwned if the new value is not within the limits.
///
/// \param   aIndex  Index of value to change.
/// \param   aValue  The new value.
///
/// \exception std::out_of_range Value outside the limits or invalid property count
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
void
LeddarCore::LdFloatProperty::SetValue( size_t aIndex, float aValue )
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

    if( ( aValue < mMinValue )
            || ( aValue > mMaxValue ) )
    {
        throw std::out_of_range( "Value outside the limits. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( mScale == 0 )
    {
        if( !IsInitialized() ||  aValue != Value( aIndex ) )
        {
            // cppcheck-suppress invalidPointerCast
            float &lFloatValue = reinterpret_cast<float *>( Storage() )[aIndex];
            lFloatValue = aValue;
            SetInitialized( true );
            EmitSignal( LdObject::VALUE_CHANGED );
        }
    }
    else
    {
        std::ostringstream lNewValueStream;
        lNewValueStream << aValue;

        std::ostringstream lOldValueStream;

        if( IsInitialized() )
        {
            lOldValueStream << Value( aIndex );
        }

        if( !IsInitialized() || lNewValueStream.str() != lOldValueStream.str() )
        {
            // Must put a 0.5 offset to have rounding instead of truncating.
            const int32_t lRaw = static_cast<int32_t>( aValue * mScale + ( aValue >= 0 ? 0.5f : -0.5f ) );

            SetRawValue( aIndex, lRaw );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdFloatProperty::ForceValue( size_t aIndex, float aValue )
///
/// \brief  Force value
///
/// \param   aIndex  Index of value to change.
/// \param   aValue  The new value.
///
/// \exception std::out_of_range Value outside the limits or invalid property count
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdFloatProperty::ForceValue( size_t aIndex, float aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValue( aIndex, aValue );
}

// *****************************************************************************
// Function: LdFloatProperty::SetStringValue
//
/// \brief   Property writer for the value as text.
///
/// The text must be valid as a floating point value. The value will only
/// be changed if the new text, once standardized is different than the
/// current text value (so changes must be seen within the number of
/// decimals configured).
///
/// \param   aIndex  Index of value to write.
/// \param   aValue  The new value.
///
/// \exception std::invalid_argument No conversion could be be performed ( from std::stof )
/// \exception std::out_of_range Value out of range ( from std::stof )
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************
void
LeddarCore::LdFloatProperty::SetStringValue( size_t aIndex, const std::string &aValue )
{
    CanEdit();
    std::string lCurrent = "";

    if( IsInitialized() )
        lCurrent = GetStringValue( aIndex );

    if( !IsInitialized() || lCurrent != aValue )
    {
        float lValue;
        std::stringstream lStreamValue( aValue );
        lStreamValue.exceptions( std::ios_base::failbit | std::ios_base::badbit );
        lStreamValue >> lValue;

        // Re-normalized and verify if really different (for example
        // if the current value is 1.00 and aValue contains "1", once
        // normalized it will still be 1.00 so no differences).
        std::ostringstream lNormalizedStream;
        lNormalizedStream << lValue;

        if( lNormalizedStream.str() != lCurrent )
        {
            SetValue( aIndex, lValue );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdFloatProperty::ForceStringValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Force string value
///
/// \param   aIndex  Index of value to write.
/// \param   aValue  The new value.
///
/// \exception std::invalid_argument No conversion could be be performed ( from std::stof )
/// \exception std::out_of_range Value out of range ( from std::stof )
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdFloatProperty::ForceStringValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetStringValue( aIndex, aValue );
}