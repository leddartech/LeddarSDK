// *****************************************************************************
// Module..: Leddar
//
/// \file    LdPropertiesContainer.cpp
///
/// \brief   Containter of properties
///          Important: The container have the ownership of the pointer inside the container.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdPropertiesContainer.h"

#include "LtStringUtils.h"
#include "LdBufferProperty.h"

#include <typeinfo>

// *****************************************************************************
// Function: LdPropertiesContainer::LdPropertiesContainer
///
/// \brief   Constructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdPropertiesContainer::LdPropertiesContainer() :
    mIsPropertiesOwner( true )
{

}

// *****************************************************************************
// Function: LdPropertiesContainer::LdPropertiesContainer
///
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdPropertiesContainer::~LdPropertiesContainer()
{
    if( mIsPropertiesOwner )
    {
        for( std::map<uint32_t, LeddarCore::LdProperty *>::iterator lIter = mProperties.begin(); lIter != mProperties.end(); ++lIter )
        {
            delete lIter->second;
        }
    }
}


// *****************************************************************************
// Function: LdPropertiesContainer::GetProperty
///
/// \brief   Get a property from the id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property
///
/// \exception std::runtime_error If the proeprty is not found.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdProperty *
LeddarCore::LdPropertiesContainer::GetProperty( uint32_t aId )
{
    std::map< uint32_t, LeddarCore::LdProperty *>::iterator lIter = mProperties.find( aId );

    if( lIter == mProperties.end() )
    {
        throw std::runtime_error( "Property id not found, id: " + LeddarUtils::LtStringUtils::IntToString( aId,
                                  16 ) + ". You must call AddProperty for this property first." );
    }

    return lIter->second;
}


// *****************************************************************************
// Function: LdPropertiesContainer::GetIntegerProperty
///
/// \brief   Get a property from his id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property
///
/// \exception std::runtime_error If the proeprty is not found or if it's not an Integer.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdIntegerProperty *
LeddarCore::LdPropertiesContainer::GetIntegerProperty( uint32_t aId )
{
    LeddarCore::LdIntegerProperty *lProperty = dynamic_cast<LeddarCore::LdIntegerProperty *>( GetProperty( aId ) );

    if( lProperty == nullptr )
    {
        throw std::runtime_error( "Property is not an Integer, id: " + LeddarUtils::LtStringUtils::IntToString( aId, 16 ) );
    }

    return lProperty;
}

// *****************************************************************************
// Function: LdPropertiesContainer::GetTextProperty
///
/// \brief   Get a property from his id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property
///
/// \exception std::runtime_error If the proeprty is not found.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdTextProperty *
LeddarCore::LdPropertiesContainer::GetTextProperty( uint32_t aId )
{
    LeddarCore::LdTextProperty *lProperty = dynamic_cast<LeddarCore::LdTextProperty *>( GetProperty( aId ) );

    if( lProperty == nullptr )
    {
        throw std::runtime_error( "Property is not an Text, id: " + LeddarUtils::LtStringUtils::IntToString( aId, 16 ) );
    }

    return lProperty;
}

// *****************************************************************************
// Function: LdPropertiesContainer::GetFloatProperty
///
/// \brief   Get a property from his id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property
///
/// \exception std::runtime_error If the proeprty is not found.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdFloatProperty *
LeddarCore::LdPropertiesContainer::GetFloatProperty( uint32_t aId )
{
    LeddarCore::LdFloatProperty *lProperty = dynamic_cast<LeddarCore::LdFloatProperty *>( GetProperty( aId ) );

    if( lProperty == nullptr )
    {
        throw std::runtime_error( "Property is not an Float, id: " + LeddarUtils::LtStringUtils::IntToString( aId, 16 ) );
    }

    return lProperty;
}

// *****************************************************************************
// Function: LdPropertiesContainer::GetEnumProperty
///
/// \brief   Get a property from his id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property
///
/// \exception std::runtime_error If the proeprty is not found.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdEnumProperty *
LeddarCore::LdPropertiesContainer::GetEnumProperty( uint32_t aId )
{
    LeddarCore::LdEnumProperty *lProperty = dynamic_cast<LeddarCore::LdEnumProperty *>( GetProperty( aId ) );

    if( lProperty == nullptr )
    {
        throw std::runtime_error( "Property is not an Enum, id: " + LeddarUtils::LtStringUtils::IntToString( aId, 16 ) );
    }

    return lProperty;
}

// *****************************************************************************
// Function: LdPropertiesContainer::GetBoolProperty
///
/// \brief   Get a property from his id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property
///
/// \exception std::runtime_error If the proeprty is not found.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdBoolProperty *
LeddarCore::LdPropertiesContainer::GetBoolProperty( uint32_t aId )
{
    LeddarCore::LdBoolProperty *lProperty = dynamic_cast<LeddarCore::LdBoolProperty *>( GetProperty( aId ) );

    if( lProperty == nullptr )
    {
        throw std::runtime_error( "Property is not a Bool, id: " + LeddarUtils::LtStringUtils::IntToString( aId, 16 ) );
    }

    return lProperty;
}

// *****************************************************************************
// Function: LdPropertiesContainer::GetBitProperty
///
/// \brief   Get a property from his id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property
///
/// \exception std::runtime_error If the proeprty is not found.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LeddarCore::LdBitFieldProperty *
LeddarCore::LdPropertiesContainer::GetBitProperty( uint32_t aId )
{
    LeddarCore::LdBitFieldProperty *lProperty = dynamic_cast<LeddarCore::LdBitFieldProperty *>( GetProperty( aId ) );

    if( lProperty == nullptr )
    {
        throw std::runtime_error( "Property is not an BitField, id: " + LeddarUtils::LtStringUtils::IntToString( aId, 16 ) );
    }

    return lProperty;
}

// *****************************************************************************
// Function: LdPropertiesContainer::LdBufferProperty
///
/// \brief   Get a property from his id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property
///
/// \exception std::runtime_error If the property is not found or type is wrong.
///
/// \author  David Levy
///
/// \since   November 2017
// *****************************************************************************

LeddarCore::LdBufferProperty *
LeddarCore::LdPropertiesContainer::GetBufferProperty( uint32_t aId )
{
    LeddarCore::LdBufferProperty *lProperty = dynamic_cast<LeddarCore::LdBufferProperty *>( GetProperty( aId ) );

    if( lProperty == nullptr )
    {
        throw std::runtime_error( "Property is not an buffer, id: " + LeddarUtils::LtStringUtils::IntToString( aId, 16 ) );
    }

    return lProperty;
}

// *****************************************************************************
// Function: LdPropertiesContainer::AddProperty
//
/// \brief   Add property to the properies map - Take ownership of the pointer
///
/// \exception std::invalid_argument If property pointer is not valid.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LeddarCore::LdPropertiesContainer::AddProperty( LeddarCore::LdProperty *aProperty )
{
    if( aProperty == nullptr )
    {
        throw std::invalid_argument( "Property pointer not valid." );
    }

    // Validate that the id is not already in the properties map
    if( mProperties.find( aProperty->GetId() ) != mProperties.end() )
    {
        throw std::invalid_argument( "Property id already exist, id: " + LeddarUtils::LtStringUtils::IntToString( aProperty->GetId(), 16 ) );
    }

    if( aProperty->GetDeviceId() != 0 )
    {
        for( std::map< uint32_t, LeddarCore::LdProperty *>::iterator it = mProperties.begin(); it != mProperties.end(); it++ )
        {
            if( it->second->GetDeviceId() == aProperty->GetDeviceId() )
            {
                throw std::invalid_argument( "Property device id already exist, id: " + LeddarUtils::LtStringUtils::IntToString( aProperty->GetDeviceId(), 16 ) );
            }
        }
    }

    aProperty->ConnectSignal( this, VALUE_CHANGED );

    mProperties.insert( std::make_pair( aProperty->GetId(), aProperty ) );
}

// *****************************************************************************
// Function: LdPropertiesContainer::FindProperty
///
/// \brief   Find a property from the id.
//
/// \param   aId Id of the property
///
/// \return  Pointer to the property, nullptr if not found.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

LeddarCore::LdProperty *
LeddarCore::LdPropertiesContainer::FindProperty( uint32_t aId )
{
    std::map< uint32_t, LeddarCore::LdProperty *>::iterator lIter = mProperties.find( aId );

    if( lIter == mProperties.end() )
    {
        return nullptr;
    }

    return lIter->second;
}

// *****************************************************************************
// Function: LdPropertiesContainer::FindDeviceProperty
///
/// \brief   Find a property from the device id.
//
/// \param   aDeviceId Device id of the property
///
/// \return  Pointer to the property, nullptr if not found.
///
/// \author  Patrick Boulay
///
/// \since   February 2017
// *****************************************************************************

LeddarCore::LdProperty *
LeddarCore::LdPropertiesContainer::FindDeviceProperty( uint32_t aDeviceId )
{
    for( std::map< uint32_t, LeddarCore::LdProperty *>::const_iterator lIter = mProperties.begin(); lIter != mProperties.end(); ++lIter )
    {
        if( lIter->second->GetDeviceId() == aDeviceId )
        {
            return lIter->second;
        }
    }

    return nullptr;
}

// *****************************************************************************
// Function: LdPropertiesContainer::FindPropertiesByCategories
///
/// \brief   Returns a list of properties filtered by categories.
//
/// \param   aCategory Filter categories (use | for multiple categories search)
///
/// \return  List of properties filtered
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************

std::vector<LeddarCore::LdProperty *>
LeddarCore::LdPropertiesContainer::FindPropertiesByCategories( LdProperty::eCategories aCategory )
{
    std::vector<LeddarCore::LdProperty *> lResultList;

    for( std::map< uint32_t, LeddarCore::LdProperty *>::iterator lIter = mProperties.begin(); lIter != mProperties.end(); ++lIter )
    {
        if( ( lIter->second->GetCategory() & aCategory ) != 0 )
        {
            lResultList.push_back( lIter->second );
        }
    }

    return lResultList;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::vector<LeddarCore::LdProperty *> LeddarCore::LdPropertiesContainer::FindPropertiesByFeature( uint32_t aFeature )
///
/// \brief  Searches for the properties by feature.
///
/// \param  aFeature    The feature to search for.
///
/// \return List of properties filtered by features.
///
/// \author David Levy
/// \date   September 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<LeddarCore::LdProperty *>
LeddarCore::LdPropertiesContainer::FindPropertiesByFeature( uint32_t aFeature )
{
    std::vector<LeddarCore::LdProperty *> lResultList;

    for( std::map< uint32_t, LeddarCore::LdProperty *>::iterator lIter = mProperties.begin(); lIter != mProperties.end(); ++lIter )
    {
        if( ( lIter->second->GetFeatures() & aFeature ) != 0 )
        {
            lResultList.push_back( lIter->second );
        }
    }

    return lResultList;
}

// *****************************************************************************
// Function: LdPropertiesContainer::Callback
///
/// \brief   Emit a signal when one of the property is modified.
//
/// \param   aSender Property that send the signal.
/// \param   aSignal Signal sent.
///
/// \author  Patrick Boulay
///
/// \since   October 2017
// *****************************************************************************

void
LeddarCore::LdPropertiesContainer::Callback( LdObject *aSender, const SIGNALS aSignal, void * /*aExtraData*/ )
{
    if( aSignal == VALUE_CHANGED )
    {
        // The extra info is the pointer to the modified property.
        EmitSignal( VALUE_CHANGED, aSender );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LeddarCore::LdPropertiesContainer::IsModified( LdProperty::eCategories aCategory )
///
/// \brief  Check if one of the properties in the specified category is modified.
///
/// \param  aCategory   The category.
///
/// \return True if one of the properties is modified, false if not.
///
/// \author Patrick Boulay
/// \date   September 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
LeddarCore::LdPropertiesContainer::IsModified( LdProperty::eCategories aCategory )
{
    for( std::map< uint32_t, LeddarCore::LdProperty *>::iterator lIter = mProperties.begin(); lIter != mProperties.end(); ++lIter )
    {
        if( ( lIter->second->GetCategory() & aCategory ) != 0 && lIter->second->Modified() )
        {
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdPropertiesContainer::AddProperties(LeddarCore::LdPropertiesContainer *aProperties)
///
/// \brief  Adds the properties from a container to the current container and remove the ownership from the source container.
///
/// \param [in,out] aProperties The properties to copy.
///
/// \author Patrick Boulay
/// \date   September 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdPropertiesContainer::AddProperties( LeddarCore::LdPropertiesContainer *aProperties )
{
    if( aProperties != nullptr )
    {
        const std::map< uint32_t, LeddarCore::LdProperty *> *lPropertyMap = aProperties->GetContent();

        for( std::map< uint32_t, LeddarCore::LdProperty *>::const_iterator lIter = lPropertyMap->cbegin(); lIter != lPropertyMap->cend(); ++lIter )
        {
            AddProperty( lIter->second );
        }

        aProperties->SetPropertiesOwnership( false );
    }
}