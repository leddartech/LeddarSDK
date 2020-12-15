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

#include <rapidjson/document.h>
#include "rapidjson/error/en.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>

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
        for( std::map< uint32_t, LeddarCore::LdProperty *>::iterator it = mProperties.begin(); it != mProperties.end(); it++ ) //begin/end -> cbegin/cend for c++98
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
    for( std::map< uint32_t, LeddarCore::LdProperty *>::iterator lIter = mProperties.begin(); lIter != mProperties.end(); ++lIter ) //begin/end -> cbegin/cend for c++98
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

        for( std::map< uint32_t, LeddarCore::LdProperty *>::const_iterator lIter = lPropertyMap->begin(); lIter != lPropertyMap->end(); ++lIter ) //begin/end -> cbegin/cend for c++98
        {
            AddProperty( lIter->second );
        }

        aProperties->SetPropertiesOwnership( false );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarCore::LdPropertiesContainer::AddPropertiesFromFile( std::string aFilename )
///
/// \brief  Adds the properties from JSON file
///
/// \exception  std::runtime_error  Raised when a runtime error condition occurs.
///
/// \param  aFilename   Filename of the JSON file.
///
/// \author Patrick Boulay
/// \date   February 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarCore::LdPropertiesContainer::AddPropertiesFromFile( std::string aFilename )
{
    std::ifstream lInputFileStream( aFilename.c_str() );
    rapidjson::IStreamWrapper lStreamWrapper( lInputFileStream );
    rapidjson::Document lDocument;
    lDocument.ParseStream( lStreamWrapper );

    if( lDocument.HasParseError() )
    {
        throw std::runtime_error( "Error parsing: " + std::string( rapidjson::GetParseError_En( lDocument.GetParseError() ) ) );
    }

    // First element must be "properties"
    if( !lDocument.HasMember( "properties" ) )
    {
        throw std::runtime_error( "JSON format error, no element properties." );
    }

    const rapidjson::GenericArray<false, rapidjson::Value::ValueType> lPropArray = lDocument["properties"].GetArray();


    for( uint32_t i = 0; i < lPropArray.Size(); i++ )
    {
        if( lPropArray[i].IsObject() )
        {
            // Verify for valid object
            if( !lPropArray[i].HasMember( "id" ) ||
                    !lPropArray[i]["id"].IsString() ||
                    !lPropArray[i].HasMember( "size" ) ||
                    !lPropArray[i]["size"].IsInt() ||
                    !lPropArray[i].HasMember( "count" ) ||
                    !lPropArray[i]["count"].IsInt() ||
                    !lPropArray[i].HasMember( "type" ) ||
                    !lPropArray[i]["type"].IsString() ||
                    !lPropArray[i].HasMember( "category" ) ||
                    !lPropArray[i]["category"].IsString() )

            {
                continue;
            }

            uint32_t lId = static_cast<uint32_t>( LeddarUtils::LtStringUtils::StringToUInt( lPropArray[i]["deviceid"].GetString(), 16 ) );

            if( lId == 0 )
            {
                throw std::runtime_error( "Error, the device id is 0x0." );
            }

            std::string lType = lPropArray[i]["type"].GetString();

            try
            {
                LdProperty *lNewProperty = nullptr;
                std::string lCategoryStr = lPropArray[i]["category"].GetString();
                LdProperty::eCategories lCategory = LdProperty::CAT_OTHER;

                if( lCategoryStr == "CAT_OTHER" )
                    lCategory = LdProperty::CAT_OTHER;
                else if( lCategoryStr == "CAT_INFO" )
                    lCategory = LdProperty::CAT_INFO;
                else if( lCategoryStr == "CAT_CALIBRATION" )
                    lCategory = LdProperty::CAT_CALIBRATION;
                else if( lCategoryStr == "CAT_CONFIGURATION" )
                    lCategory = LdProperty::CAT_CONFIGURATION;
                else if( lCategoryStr == "CAT_CONSTANT" )
                    lCategory = LdProperty::CAT_CONSTANT;
                else
                    throw std::runtime_error( "Invalid category: " + lCategoryStr );



                // Set property editable or not
                bool lFeature = LdProperty::F_EDITABLE;

                if( lPropArray[i].HasMember( "editable" ) && lPropArray[i]["editable"].IsBool() )
                {
                    lFeature = lPropArray[i]["editable"].GetBool() ? LdProperty::F_EDITABLE : LdProperty::F_NONE;
                }

                if( lType == "bit" )
                {
                    lNewProperty = new LdBitFieldProperty( lCategory, lFeature, lId, lId, lPropArray[i]["size"].GetInt() );
                }
                else if( lType == "bool" )
                {
                    lNewProperty = new LdBoolProperty( lCategory, lFeature, lId, lId );
                }
                else if( lType == "buffer" )
                {
                    lNewProperty = new LdBufferProperty( lCategory, lFeature, lId, lId, lPropArray[i]["size"].GetInt() );
                }
                else if( lType == "enum" )
                {
                    if( !lPropArray[i].HasMember( "values" ) || !lPropArray[i]["values"].IsObject() )
                    {
                        throw std::runtime_error( "Invalid enum property values" );
                    }

                    lNewProperty = new LdEnumProperty( lCategory, lFeature, lId, lId, lPropArray[i]["size"].GetInt() );
                    LdEnumProperty *lEnumProperty = dynamic_cast<LdEnumProperty *>( lNewProperty );

                    // Add enum possible values
                    for( rapidjson::Value::ConstMemberIterator itrValues = lPropArray[i]["values"].GetObject().MemberBegin();
                            itrValues != lPropArray[i]["values"].GetObject().MemberEnd(); ++itrValues )
                    {
                        lEnumProperty->AddEnumPair( itrValues->value.GetInt(), itrValues->name.GetString() );
                    }

                }
                else if( lType == "float" )
                {
                    if( !lPropArray[i].HasMember( "scale" ) || !lPropArray[i]["scale"].IsInt() )
                    {
                        throw std::runtime_error( "Invalid property scale: " + lType );
                    }

                    uint32_t lDecimals = 3;

                    if( lPropArray[i].HasMember( "decimals" ) && lPropArray[i]["decimals"].IsInt() )
                    {
                        lDecimals = lPropArray[i]["decimals"].GetInt();
                    }

                    lNewProperty = new LdFloatProperty( lCategory, lFeature, lId, lId, lPropArray[i]["size"].GetInt(),
                                                        lPropArray[i]["scale"].GetInt(), lDecimals );
                }
                else if( lType == "int" )
                {
                    bool lSigned = false;

                    if( lPropArray[i].HasMember( "signed" ) && lPropArray[i]["signed"].IsBool() )
                    {
                        lSigned = lPropArray[i]["signed"].GetBool();
                    }

                    lNewProperty = new LdIntegerProperty( lCategory, lFeature, lId, lId, lPropArray[i]["size"].GetInt(), "", lSigned );
                    LdIntegerProperty *lIntProperty = dynamic_cast<LdIntegerProperty *>( lNewProperty );

                    if( lPropArray[i].HasMember( "limits" ) && lPropArray[i]["limits"].IsArray() )
                    {
                        lIntProperty->SetLimits( lPropArray[i]["limits"].GetArray()[0].GetInt(), lPropArray[i]["limits"].GetArray()[1].GetInt() );
                    }
                }
                else if( lType == "text" )
                {
                    lNewProperty = new LdTextProperty( lCategory, lFeature, lId, lId, lPropArray[i]["size"].GetInt() );
                }
                else
                {
                    throw std::runtime_error( "Invalid property type: " + lType );
                }


                // Set all values for each count
                if( lPropArray[i]["count"].GetInt() != 0 )
                {
                    lNewProperty->SetCount( lPropArray[i]["count"].GetInt() );
                }

                if( lPropArray[i]["value"].IsString() )
                {
                    std::string lValue = lPropArray[i]["value"].GetString();

                    if( lValue == "" && ( lType == "int" || lType == "float" ) )
                    {
                        lValue = "0";
                    }

                    // If the property has a count > 1 and only one value is specified, fill the value for all count
                    for( uint32_t i = 0; i < lNewProperty->Count(); ++i )
                    {
                        lNewProperty->ForceStringValue( i, lValue );
                    }
                }
                else //Array
                {
                    for( unsigned int j = 0; j < lPropArray[i]["value"].GetArray().Size(); ++j )
                    {
                        lNewProperty->ForceStringValue( j, lPropArray[i]["value"].GetArray()[j].GetString() );
                    }
                }

                lNewProperty->SetClean();
                AddProperty( lNewProperty );
            }
            catch( std::exception &e )
            {
                throw std::runtime_error( "Error on property id: " + LeddarUtils::LtStringUtils::IntToString( lId, 16 ) + ": " + e.what() );
            }

        }
    }

    lInputFileStream.close();
}