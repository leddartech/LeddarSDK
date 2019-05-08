////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdEnumProperty.cpp
///
/// \brief  Implements the ldEnumProperty class
///
/// Copyright (c) 2018 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdEnumProperty.h"
#include "LtStringUtils.h"
#include "LtScope.h"

#include <cassert>
#include <limits>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarCore::LdEnumProperty::LdEnumProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aUnitSize, bool aStoreValue, const std::string &aDescription ) : LdProperty( LdProperty::TYPE_ENUM, aCategory, aFeatures, aId, aDeviceId, aUnitSize, aUnitSize, aDescription ), mStoreValue( aStoreValue )
///
/// \brief  Constructor.
///
/// \param  aCategory       See LdProperty.
/// \param  aFeatures       See LdProperty.
/// \param  aId             See LdProperty.
/// \param  aDeviceId       See LdProperty.
/// \param  aUnitSize       See LdProperty.
/// \param  aStoreValue     True to store the value of the enum or false to store the index.
/// \param  aDescription    See LdProperty.
///
/// \author Patrick Boulay
/// \date   February 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdEnumProperty::LdEnumProperty( LdProperty::eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint16_t aDeviceId, uint32_t aUnitSize, bool aStoreValue,
        const std::string &aDescription ) :
    LdProperty( LdProperty::TYPE_ENUM, aCategory, aFeatures, aId, aDeviceId, aUnitSize, aUnitSize, aDescription ),
    mStoreValue( aStoreValue )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn size_t LeddarCore::LdEnumProperty::ValueIndex( size_t aIndex ) const
///
/// \brief  Get the index in the enum of the current value.
///
/// \exception  std::out_of_range   Thrown when an out of range error condition occurs.
///
/// \param  aIndex  Property index of value to query.
///
/// \return The enum index.
///
/// \author Patrick Boulay
/// \date   February 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t
LeddarCore::LdEnumProperty::ValueIndex( size_t aIndex ) const
{
    const uint64_t lValue = ValueT<uint64_t>( aIndex );

    for( size_t i = 0; i < mEnumValues.size(); ++i )
    {
        if( mEnumValues[ i ].first == lValue )
        {
            return i;
        }
    }

    throw std::out_of_range( "No index associated to this value. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdEnumProperty::SetValue( size_t aIndex, uint64_t aValue )
///
/// \brief  Set the value at the current array index.
///
/// \exception  std::out_of_range   Thrown when an out of range error condition occurs.
/// \exception  out_of_range        : Value is too big for stride.
/// \exception  out_of_range        : Index not valid.
/// \exception  out_of_range        : No associated value found for this enum.
///
/// \param  aIndex  Index of value to write in the array.
/// \param  aValue  The new value to write.
///
/// \author Patrick Boulay
/// \date   February 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdEnumProperty::SetValue( size_t aIndex, uint64_t aValue )
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


    if( !IsInitialized()  || aValue != ValueT<uint64_t>( aIndex ) )
    {
        uint64_t lIndex = static_cast<uint64_t>( mEnumValues.size() - 1 );

        for( std::vector<Pair>::reverse_iterator lIter = mEnumValues.rbegin(); lIter < mEnumValues.rend(); ++lIter )
        {
            if( lIter->first == aValue )
            {
                uint64_t lValue = mStoreValue ? aValue : lIndex;

                if( Stride() == 1 )
                {
                    SetStorageValueT<uint8_t>( aIndex, lValue );
                }
                else if( Stride() == 2 )
                {
                    SetStorageValueT<uint16_t>( aIndex, lValue );
                }
                else if( Stride() == 4 )
                {
                    SetStorageValueT<uint32_t>( aIndex, lValue );
                }
                else if( Stride() == 8 )
                {
                    SetStorageValueT<uint64_t>( aIndex, lValue );
                }
                else
                {
                    throw std::out_of_range( "Invalid stride" );
                }

                SetInitialized( true );
                EmitSignal( LdObject::VALUE_CHANGED );
                return;
            }

            lIndex--;
        }

        // If we get here aValue is not valid (i.e. it is not in the enum).
        throw std::out_of_range( "No associated value found for this enum. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdEnumProperty::ForceValue( size_t aIndex, uint64_t aValue )
///
/// \brief  Force the value at the current array index.
///
/// \exception  std::out_of_range   Thrown when an out of range error condition occurs.
/// \exception  out_of_range        : Value is too big for stride.
/// \exception  out_of_range        : Index not valid.
/// \exception  out_of_range        : No associated value found for this enum.
///
/// \param  aIndex  Index of value to write in the array.
/// \param  aValue  The new value to write.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdEnumProperty::ForceValue( size_t aIndex, uint64_t aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValue( aIndex, aValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn template<typename T> void LeddarCore::LdEnumProperty::SetStorageValueT( size_t aIndex, uint64_t aValue )
///
/// \brief  Template to set the value depending on the stride of the property
///
/// \exception  std::out_of_range   Value is bigger than what stride can hold.
///
/// \tparam T   Generic type parameter.
/// \param  aIndex  Index in array of value to change.
/// \param  aValue  New value to write.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void LeddarCore::LdEnumProperty::SetStorageValueT( size_t aIndex, uint64_t aValue )
{
    CanEdit();

    if( aValue > std::numeric_limits<T>::max() )
    {
        throw std::out_of_range( "Value is too big. Increase stride/unitsize. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    T *lValues = reinterpret_cast<T *>( Storage() );

    if( lValues[aIndex] != aValue )
    {
        lValues[aIndex] = static_cast<T>( aValue );
        EmitSignal( LdObject::VALUE_CHANGED );
    }
}

//Template specialisation so it can be defined in the cpp file
template void LeddarCore::LdEnumProperty::SetStorageValueT<uint8_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdEnumProperty::SetStorageValueT<uint16_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdEnumProperty::SetStorageValueT<uint32_t>( size_t aIndex, uint64_t aValue );
template void LeddarCore::LdEnumProperty::SetStorageValueT<uint64_t>( size_t aIndex, uint64_t aValue );

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint64_t LeddarCore::LdEnumProperty::DeviceValue( size_t aIndex ) const
///
/// \brief  Device value. This value is stored in the backup storage.
///
/// \exception  std::out_of_range   Thrown when an out of range error condition occurs.
///
/// \param  aIndex  Zero-based index of the.
///
/// \return An uint64_t.
///
/// \author Patrick Boulay
/// \date   December 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t LeddarCore::LdEnumProperty::DeviceValue( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    uint64_t lValue = 0;

    if( Stride() == 1 )
    {
        lValue = reinterpret_cast<const uint8_t *>( BackupStorage() )[ aIndex ];
    }
    else if( Stride() == 2 )
    {
        lValue = reinterpret_cast<const uint16_t *>( BackupStorage() )[ aIndex ];
    }
    else if( Stride() == 4 )
    {
        lValue = reinterpret_cast<const uint32_t *>( BackupStorage() )[ aIndex ];
    }
    else if( Stride() == 8 )
    {
        lValue = reinterpret_cast<const uint64_t *>( BackupStorage() )[ aIndex ];
    }
    else
    {
        throw std::out_of_range( "Invalid stride. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    return lValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint64_t LeddarCore::LdEnumProperty::GetKeyFromValue( const std::string &aValue )
///
/// \brief  Return the key value of the string value
///
/// \exception  std::out_of_range   Thrown when an out of range error condition occurs.
///
/// \param  aValue  String value of the enum.
///
/// \return The associated key for the value.
///
/// \author Patrick Boulay
/// \date   February 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t
LeddarCore::LdEnumProperty::GetKeyFromValue( const std::string &aValue )
{
    for( std::vector<Pair>::iterator lIter = mEnumValues.begin(); lIter < mEnumValues.end(); ++lIter )
    {
        if( lIter->second == aValue )
        {
            return lIter->first;
        }
    }

    throw std::out_of_range( "No associated string value found for this enum. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdEnumProperty::AddEnumPair( uint64_t aValue, const std::string &aText )
///
/// \brief  Add a new entry in the enum.
///
/// \exception  std::invalid_argument   Thrown when an invalid argument error condition occurs.
///
/// \param  aValue  The value that is stored in the property and set in the device.
/// \param  aText   The text that is displayed to the user for aValue.
///
/// \author Patrick Boulay
/// \date   February 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdEnumProperty::AddEnumPair( uint64_t aValue, const std::string &aText )
{
    assert( mEnumValues.size() < std::numeric_limits<uint8_t>::max() - 1u );
    uint64_t lMax = 0;

    switch( UnitSize() )
    {
        case 1: lMax = std::numeric_limits<uint8_t>::max(); break;

        case 2: lMax = std::numeric_limits<uint16_t>::max(); break;

        case 4: lMax = std::numeric_limits<uint32_t>::max(); break;

        case 8: lMax = std::numeric_limits<uint64_t>::max(); break;
    }

    if( aValue > lMax )
    {
        throw std::invalid_argument( "Value is higher than the property size. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    mEnumValues.push_back( Pair( aValue, aText ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdEnumProperty::ClearEnum( void )
///
/// \brief  Clear the enum vector. Use this function carefully.
///
/// \author Patrick Boulay
/// \date   October 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdEnumProperty::ClearEnum( void )
{
    mEnumValues.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdEnumProperty::SetValueIndex( size_t aArrayIndex, size_t aEnumIndex )
///
/// \brief  Set the value via its index in the enum.
///
/// \exception  std::out_of_range   Enum index not valid.
///
/// \param  aArrayIndex Index of value to write in the array.
/// \param  aEnumIndex  Index in enum of new value.
///
/// \author Patrick Boulay
/// \date   February 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdEnumProperty::SetValueIndex( size_t aArrayIndex, size_t aEnumIndex )
{
    if( aEnumIndex >= mEnumValues.size() )
    {
        throw std::out_of_range( "Enum index not valid. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    SetValue( aArrayIndex, mEnumValues[ aEnumIndex ].first );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdEnumProperty::ForceValueIndex( size_t aArrayIndex, size_t aEnumIndex )
///
/// \brief  Force value via enum index
///
/// \exception  std::out_of_range   Enum index not valid.
///
/// \param  aArrayIndex Index of value to write in the array.
/// \param  aEnumIndex  Index in enum of new value.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdEnumProperty::ForceValueIndex( size_t aArrayIndex, size_t aEnumIndex )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetValueIndex( aArrayIndex, aEnumIndex );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::string LeddarCore::LdEnumProperty::GetStringValue( size_t aIndex ) const
///
/// \brief  Returns the string of the value.
///
/// \exception  std::out_of_range   Index not valid, verify property count.
///
/// \param  aIndex  Index of value to query.
///
/// \return Value in string format.
///
/// \author Patrick Boulay
/// \date   February 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string
LeddarCore::LdEnumProperty::GetStringValue( size_t aIndex ) const
{
    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    return mEnumValues[ ValueIndex( aIndex ) ].second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdEnumProperty::SetStringValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Property writer for the value as text.
///
/// \exception  std::out_of_range   If the string is not found in the enum.
///
/// \param  aIndex  Index of value to write.
/// \param  aValue  The new value.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdEnumProperty::SetStringValue( size_t aIndex, const std::string &aValue )
{
    uint64_t lKey = GetKeyFromValue( aValue );
    SetValue( aIndex, lKey );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdEnumProperty::ForceStringValue( size_t aIndex, const std::string &aValue )
///
/// \brief  Force string value
///
/// \param  aIndex  Index of value to write.
/// \param  aValue  The new value.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdEnumProperty::ForceStringValue( size_t aIndex, const std::string &aValue )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetStringValue( aIndex, aValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LeddarCore::LdEnumProperty::Value( size_t aIndex ) const
///
/// \brief  Return the property value
///
/// \exception  std::out_of_range   Value out of range ( from std::stoi )
///
/// \param  aIndex  Index of value.
///
/// \return An uint32_t.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t
LeddarCore::LdEnumProperty::Value( size_t aIndex ) const
{
    return ValueT<uint32_t>( aIndex );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn template<typename T> T LeddarCore::LdEnumProperty::ValueT( size_t aIndex ) const
///
/// \brief  Return the property value - Templated on the return type
///
/// \exception  std::out_of_range   Index not valid.
/// \exception  std::out_of_range   Value larger than what the return type can hold.
///
/// \tparam T   Generic type parameter.
/// \param  aIndex  Index of value.
///
/// \return A T.
///
/// \author David
/// \date   August 2018
///
/// ### tparam  T   Generic type parameter.
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T LeddarCore::LdEnumProperty::ValueT( size_t aIndex ) const
{
    VerifyInitialization();

    if( aIndex >= Count() )
    {
        throw std::out_of_range( "Index not valid, verify property count. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    uint64_t lValue = 0;

    if( Stride() == 1 )
    {
        lValue = reinterpret_cast<const uint8_t *>( CStorage() )[ aIndex ];
    }
    else if( Stride() == 2 )
    {
        lValue = reinterpret_cast<const uint16_t *>( CStorage() )[ aIndex ];
    }
    else if( Stride() == 4 )
    {
        lValue = reinterpret_cast<const uint32_t *>( CStorage() )[ aIndex ];
    }
    else if( Stride() == 8 )
    {
        lValue = reinterpret_cast<const uint64_t *>( CStorage() )[ aIndex ];
    }
    else
    {
        throw std::out_of_range( "Invalid stride. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    if( !mStoreValue )
    {
        // Index is stored, return the associated value
        lValue = mEnumValues[ static_cast<size_t>( lValue ) ].first;
    }

    if( lValue > std::numeric_limits<T>::max() )
    {
        throw std::out_of_range( "Value is bigger than what the return type can hold. Use ValueT<TYPE> with a TYPE big enough. Property id: " + LeddarUtils::LtStringUtils::IntToString( GetId(), 16 ) );
    }

    return static_cast<T>( lValue );
}
//Template specialisation so it can be defined in the cpp file
template uint8_t LeddarCore::LdEnumProperty::ValueT( size_t aIndex ) const;
template uint16_t LeddarCore::LdEnumProperty::ValueT( size_t aIndex ) const;
template uint32_t LeddarCore::LdEnumProperty::ValueT( size_t aIndex ) const;
template uint64_t LeddarCore::LdEnumProperty::ValueT( size_t aIndex ) const;
