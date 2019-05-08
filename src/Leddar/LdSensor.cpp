// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensor.cpp
///
/// \brief   Definition of all sensors.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdSensor.h"
#include "LdConnection.h"
#include "LdIntegerProperty.h"
#include "LdPropertyIds.h"
#include "comm/LtComLeddarTechPublic.h"

using namespace LeddarDevice;

// *****************************************************************************
// Function: LdSensor::LdSensor
//
/// \brief   Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param   aConnection Connection object.
/// \param   aProperties Properties container for this sensor.
///                      If null, the sensor will declare a new instance of LdPropertiesContainer.
///
/// \author  Patrick Boulay
///
/// \since   January 2016
// *****************************************************************************

LdSensor::LdSensor( LeddarConnection::LdConnection *aConnection, LeddarCore::LdPropertiesContainer *aProperties ) :
    LdDevice( aConnection, aProperties ),
    mEchoes(),
    mStates(),
    mDataMask( 0 )
{
    InitProperties();
}

// *****************************************************************************
// Function: LdSensorVuLdSensor::~LdSensor
//
/// \brief   Destructor.
///
/// \author  David Levy
///
/// \since   June 2017
// *****************************************************************************
LdSensor::~LdSensor()
{
}

// *****************************************************************************
// Function: LdSensor::GetData
//
/// \brief   Get the data from the sensor.
///          The function SetDataMask must be call first to set the data level.
///
/// \return  True is new data was processed, otherwise false.
///
/// \author  Patrick Boulay
///
/// \since   March 2017
// *****************************************************************************
bool
LdSensor::GetData( void )
{
    bool lDataReceived = false;

    if( mDataMask == DM_NONE )
    {
        SetDataMask( DM_ALL ); //echoes | states
    }

    if( ( mDataMask & DM_ECHOES ) == DM_ECHOES )
    {
        lDataReceived = GetEchoes();
    }

    if( ( mDataMask & DM_STATES ) == DM_STATES )
    {
        GetStates();
        lDataReceived = true;
    }

    return lDataReceived;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LeddarDevice::LdSensor::ConvertDataMaskToLTDataMask( uint32_t aMask )
///
/// \brief  Convert SDK data mask to LeddarTech internal data mask
///
/// \param  aMask   The mask.
///
/// \return The data converted mask to LeddarTech data mask.
///
/// \author David Levy
/// \date   August 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t LeddarDevice::LdSensor::ConvertDataMaskToLTDataMask( uint32_t aMask )
{
    uint32_t lLTDataMask = 0;

    if( ( aMask & DM_ECHOES ) == DM_ECHOES )
        lLTDataMask |= LtComLeddarTechPublic::LT_DATA_LEVEL_ECHOES;

    if( ( aMask & DM_STATES ) == DM_STATES )
        lLTDataMask |= LtComLeddarTechPublic::LT_DATA_LEVEL_STATE;

    return lLTDataMask;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensor::InitProperties( void )
///
/// \brief   Create properties for all sensors.
///          Do not call the function from Ldsensor contructor,
///          some LdSensor children (ex: LdCarrierEnhancedModbus) will call InitProperties twice
///          on the same properties container.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LeddarDevice::LdSensor::InitProperties( void )
{
    using namespace LeddarCore;

    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_HSEGMENT, 0, 2, "Number of horizontal segments" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_VSEGMENT, 0, 2, "Number of vertical segments" ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ForceValue( 0, 1 );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->SetClean();
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_CONNECTION_TYPE, 0, 2, "Connection type" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_HFOV, LtComLeddarTechPublic::LT_COMM_ID_HFOV, 4, 0, 3, "Horizontal field of view." ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_VFOV, LtComLeddarTechPublic::LT_COMM_ID_VFOV, 4, 0, 3,
                              "Vertical field of view. Default value is 3 for module but actual value is between 0.3 and 7.5" ) );
    mProperties->GetFloatProperty( LdPropertyIds::ID_VFOV )->ForceValue( 0, 3 );
    mProperties->GetFloatProperty( LdPropertyIds::ID_VFOV )->SetClean();
    mProperties->GetFloatProperty( LdPropertyIds::ID_VFOV )->ForceValue( 0, 45 ); //Default value for Vu8 can bus and M16 modbus/canbus sensor. It is not the correct value, but we need one for ROS
    mProperties->GetFloatProperty( LdPropertyIds::ID_VFOV )->SetClean();
}