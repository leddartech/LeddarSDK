////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdProperty.cpp
///
/// \brief  Implements the LdProperty class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdProperty.h"
#include "LtExceptions.h"
#include "LtStringUtils.h"
#include "LtScope.h"

#include <cassert>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarCore::LdProperty::LdProperty( ePropertyType aPropertyType, eCategories aCategory, uint32_t aFeatures, uint32_t aId, uint32_t aDeviceId, uint32_t aUnitSize, size_t aStride, const std::string &aDescription )
///
/// \brief  Constructor.
///
/// \exception  std::logic_error    Raised when the stride is inferior to the size.
///
/// \param  aPropertyType   Type of the property.
/// \param  aCategory       Category of the property.
/// \param  aFeatures       Combination of feature bits from the eFeatures enum.
/// \param  aId             The globally unique value identifying this property. Also used as the file id. Cannot be 0.
/// \param  aDeviceId       The value used by the device to identify this property. Can be 0 if the property is not involved in communication with
///                         the device.
/// \param  aUnitSize       The number of bytes for each value (for raw storage for interaction with files and LeddarTech protocol).
/// \param  aStride         The number of bytes for each value in the local storage.
/// \param  aDescription    Name or description of the property.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarCore::LdProperty::LdProperty( ePropertyType aPropertyType, eCategories aCategory, uint32_t aFeatures, uint32_t aId,
                                    uint32_t aDeviceId, uint32_t aUnitSize, size_t aStride, const std::string &aDescription ) :
    mCategory( aCategory ),
    mFeatures( aFeatures ),
    mId( aId ),
    mPropertyType( aPropertyType ),
    mDescription( aDescription ),
    mDeviceId( aDeviceId ),
    mInitialized( false ),
    mStride( aStride ),
    mUnitSize( aUnitSize ),
    mCheckEditable( true )
{
    assert( aId && aUnitSize );

    if( aStride < aUnitSize )
    {
        throw std::logic_error( "Property stride must superior or equal to unit size." );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarCore::LdProperty::Modified( void ) const
///
/// \brief  Indicate if the property is modified.
///     Modified means the backup values are not equal to the current values and the property has the EDITABLE bit.
///
/// \return true if modified, false if clean.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
LeddarCore::LdProperty::Modified( void ) const
{
    return ( mStorage != mBackupStorage );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdProperty::SetClean( void )
///
/// \brief  Set the backup values to be the current values (so the property is reported as not modified).
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdProperty::SetClean( void )
{
    mBackupStorage = mStorage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdProperty::SetCount( size_t aValue )
///
/// \brief  Set the array size of this property.
///
/// \param  aValue  The new count.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdProperty::SetCount( size_t aValue )
{
    mStorage.resize( aValue * mStride );
    mBackupStorage.resize( mStorage.size() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdProperty::Restore( void )
///
/// \brief  Cancel changes by writing the backup values back to the current values.
///
/// \author Patrick Boulay
/// \date   January 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdProperty::Restore( void )
{
    if( Modified() )
    {
        mStorage = mBackupStorage;
        EmitSignal( LdObject::VALUE_CHANGED );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdProperty::SetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
///
/// \brief  Set storage directly in memory
///
/// \exception  std::invalid_argument   Thrown when the size is too big.
/// \exception  std::logic_error        Raised when the size or stride is invalid.
/// \exception  std::logic_error        Raised when the property is not editable (from CanEdit()).
///
/// \param [in,out] aBuffer Buffer to copy.
/// \param          aCount  Number of element in the buffer.
/// \param          aSize   Size of each element in the buffer.
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdProperty::SetRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
{
    CanEdit();

    if( Count() != aCount )
    {
        SetCount( aCount );
    }

    if( aSize == mStride )
    {
        memcpy( static_cast<uint8_t *>( &mStorage[0] ), aBuffer, aSize * aCount );
    }
    else
    {
        if( aSize > sizeof( uint64_t ) )
        {
            throw std::invalid_argument( "Unable to SetRawStorage, invalid size: " + LeddarUtils::LtStringUtils::IntToString( aSize )
                                         + " id: " + LeddarUtils::LtStringUtils::IntToString( mId, 16 ) );
        }

        for( uint32_t i = 0; i < aCount; i++ )
        {
            uint64_t lValue( 0 );

            if( aSize == sizeof( uint8_t ) )
            {
                lValue = static_cast<uint8_t *>( aBuffer )[i];
            }
            else if( aSize == sizeof( uint16_t ) )
            {
                lValue = reinterpret_cast<uint16_t *>( aBuffer )[i];
            }
            else if( aSize == sizeof( uint32_t ) )
            {
                lValue = reinterpret_cast<uint32_t *>( aBuffer )[i];
            }
            else if( aSize == sizeof( uint64_t ) )
            {
                lValue = reinterpret_cast<uint64_t *>( aBuffer )[i];
            }
            else
            {
                throw std::logic_error( "Couldnt set storage value - Invalid size" );
            }

            if( mStride == sizeof( uint8_t ) )
            {
                static_cast<uint8_t *>( &mStorage[0] )[i] = static_cast<uint8_t>( lValue );
            }
            else if( mStride == sizeof( uint16_t ) )
            {
                reinterpret_cast<uint16_t *>( &mStorage[0] )[i] = static_cast<uint16_t>( lValue );
            }
            else if( mStride == sizeof( uint32_t ) )
            {
                reinterpret_cast<uint32_t *>( &mStorage[0] )[i] = static_cast<uint32_t>( lValue );
            }
            else if( mStride == sizeof( uint64_t ) )
            {
                reinterpret_cast<uint64_t *>( &mStorage[0] )[i] = static_cast<uint64_t>( lValue );
            }
            else
            {
                throw std::logic_error( "Couldnt set storage value - Invalid stride" );
            }
        }
    }

    SetInitialized( true );
    EmitSignal( LdObject::VALUE_CHANGED );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdProperty::ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
///
/// \brief  Set raw storage for non editable properties
///
/// \param [in,out] aBuffer Buffer to copy.
/// \param          aCount  Number of element in the buffer.
/// \param          aSize   Size of each element in the buffer.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdProperty::ForceRawStorage( uint8_t *aBuffer, size_t aCount, uint32_t aSize )
{
    LeddarUtils::LtScope<bool> lForceEdit( &mCheckEditable, true );
    mCheckEditable = false;
    SetRawStorage( aBuffer, aCount, aSize );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdProperty::SetRawValue( size_t aIndex, int32_t aValue )
///
/// \brief  Set storage directly in memory
///
/// \param  aIndex  Index of value to change.
/// \param  aValue  The new value.
///
/// \author David Levy
/// \date   February 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdProperty::SetRawValue( size_t aIndex, int32_t aValue )
{
    if( aValue != RawValue( aIndex ) || !mInitialized )
    {
        SetRawStorage( &( reinterpret_cast<uint8_t *>( &aValue ) )[UnitSize() * aIndex], 1, UnitSize() );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdProperty::VerifyInitialization( void ) const
///
/// \brief  Verify if the property is initialize with a value
///
/// \exception  std::runtime_error  Raised when the property was not initialized.
///
/// \return A const void.
///
/// \author Patrick Boulay
/// \date   November 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdProperty::VerifyInitialization( void ) const
{
    if( !mInitialized )
    {
        throw std::runtime_error( "Property not initialized. Set a value first. ID:" + LeddarUtils::LtStringUtils::IntToString( mId, 16 ) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdProperty::CanEdit( void ) const
///
/// \brief  Throw an exception if the property is not editable
///
/// \exception  std::logic_error    Raised when the property is not editable.
///
/// \author David Levy
/// \date   March 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarCore::LdProperty::CanEdit( void ) const
{
    if( mCheckEditable && ( ( GetFeatures() & F_EDITABLE ) == 0 ) )
    {
        throw std::logic_error( "Property is not editable. Id: " + LeddarUtils::LtStringUtils::IntToString( mId, 16 ) );
    }
}
