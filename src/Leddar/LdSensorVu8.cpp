// *****************************************************************************
// Module..: Leddar
//
/// \file    LdSensorVu8.cpp
///
/// \brief   Definition of LeddarVu 8 sensor class.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
//
// Copyright (c) 2016 LeddarTech Inc. All rights reserved.
// *****************************************************************************

#include "LdSensorVu8.h"
#if defined(BUILD_VU)

#include "LdPropertyIds.h"
#include "LdBitFieldProperty.h"
#include "LdBoolProperty.h"
#include "LdEnumProperty.h"
#include "LdFloatProperty.h"
#include "LdIntegerProperty.h"
#include "LdTextProperty.h"
#include "LdConnectionUniversal.h"
#include "LtExceptions.h"
#define _VU8
#include "comm/PlatformM7DefinitionsShared.h"
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include "comm/registerMap.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#undef _VU8

using namespace LeddarCore;
using namespace LeddarDevice;

// *****************************************************************************
// Function: LdSensorVu8::LdSensorVu8
//
/// \brief   Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LdSensorVu8::LdSensorVu8( LeddarConnection::LdConnection *aConnection ) :
    LdSensor( aConnection ),
    LdSensorVu( aConnection ),
    mPredictedTempAvailable( false ) //Set it back to true when firmware option is ready
{

}

// *****************************************************************************
// Function: LdSensorVu8::~LdSensorVu8
//
/// \brief   Destructor.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

LdSensorVu8::~LdSensorVu8()
{
}

// *****************************************************************************
// Function: LdSensorVu8::GetConfig
//
/// \brief   Get configuration from the device, store result in the properties.
///
/// \exception std::runtime_error If the device is not connected.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdSensorVu8::GetConfig()
{

    try
    {
        // Get comm internal buffers
        uint8_t *lInputBuffer, *lOutputBuffer;
        mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

        // ------------- Read configuration data -------------
        LdSensorVu::GetConfig();

        // ------------- Read product-specifc configuration data -------------
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_PRD_CFG_DATA ), sizeof( sProductCfgData ), 5 );
        sProductCfgData *lProdCfgInfo = reinterpret_cast< sProductCfgData * >( lOutputBuffer );

        // Crosstalk echo removal enable
        LeddarCore::LdBoolProperty *lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_XTALK_ECHO_REMOVAL_ENABLE );
        lBoolProp->SetValue( 0, lProdCfgInfo->mXtalkEchoRemovalEnable == 1 );
        lBoolProp->SetClean();

        // Crosstalk removal
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_XTALK_REMOVAL_ENABLE );
        lBoolProp->SetValue( 0, lProdCfgInfo->mXtalkRmvEnable == 1 );
        lBoolProp->SetClean();

        // ------------- Read product-specifc advanced configuration data -------------
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_PRD_ADV_CFG_DATA ) + offsetof( sProductAdvCfgData, mGrbScanDuration ),
                                    sizeof( ( ( sProductAdvCfgData * )0 )->mGrbScanDuration ), 5 );

        // Pulse frequency
        LeddarCore::LdIntegerProperty *lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PULSE_RATE );
        lIntProp->ForceValue( 0, ( int32_t )100e6 / ( ( uint16_t * )lOutputBuffer )[0] );
        lIntProp->SetClean();

        // If a settings was repaired by the GetConfig, we need to fix it on the sensor.
        if( mRepair )
        {
            SetConfig();
            mRepair = false;
        }

    }
    catch( std::exception &e )
    {
        throw LeddarException::LtConfigException( e );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu8::GetStates()
///
/// \brief  Get the states from the device.
///
/// \exception  std::runtime_error                  If the device is not connected.
/// \exception  LeddarException::LtInfoException    Thrown if predicted temperature is not available on the sensor
///
///
/// \author Vincent Simard Bilodeau
/// \date   August 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu8::GetStates()
{
    // Check connection
    if( mConnectionUniversal == nullptr )
        throw std::runtime_error( "No connection associated to the device." );

    // Get product specific states
    uint32_t lTemperature;
    mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_PRD_CMD_LIST ) + offsetof( sProductCmdList, mSensorTemp ), ( uint8_t * )&lTemperature, sizeof( lTemperature ) );
    GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_SYSTEM_TEMP )->ForceRawValue( 0, lTemperature );

    if( mPredictedTempAvailable )
    {
        try
        {
            // Read the predicted temperature
            uint32_t lPredictedTemp;
            mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_PRD_CMD_LIST ) + offsetof( sProductCmdList, mSensorTempPred ), ( uint8_t * )&lPredictedTemp, sizeof( lPredictedTemp ), 5 );
            GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_PREDICT_TEMP )->ForceRawValue( 0, lPredictedTemp );
        }
        catch( ... )
        {
            mPredictedTempAvailable = false;
            GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_PREDICT_TEMP )->ForceValue( 0, 0 );
            throw LeddarException::LtInfoException( "Error to read the predicted temperature, please update your sensor firmware." );
        }
    }

    // Get device states
    LdSensorVu::GetStates();
}

// *****************************************************************************
// Function: LdSensorVu8::SetConfig
//
/// \brief   Set configuration to the device, store result in the properties.
///
/// \exception std::runtime_error If the device is not connected.
///
/// \author  Patrick Boulay
///
/// \since   March 2016
// *****************************************************************************

void
LdSensorVu8::SetConfig()
{
    // Check connection
    if( mConnectionUniversal == nullptr )
        throw std::runtime_error( "No connection associated to the device." );

    try
    {
        mConnectionUniversal->SetWriteEnable( true );

        // Get comm internal buffer
        uint8_t *lInputBuffer, *lOutputBuffer;
        mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

        // ---------------------- Write configuration into the sensor ----------------------
        LdSensorVu::SetConfig();
        sProductCfgData *lProductCfgData = reinterpret_cast< sProductCfgData *>( lInputBuffer );

        // Crosstalk echo removal enable.
        LeddarCore::LdBoolProperty *lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_XTALK_ECHO_REMOVAL_ENABLE );
        lProductCfgData->mXtalkEchoRemovalEnable = ( lBoolProp->Value() == true ? 1 : 0 );

        // Crostalk removal enable.
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_XTALK_REMOVAL_ENABLE );
        lProductCfgData->mXtalkRmvEnable = ( lBoolProp->Value() == true ? 1 : 0 );

        // ------------- Write product-specific configuration into the sensor -------------
        mConnectionUniversal->Write( 0x2, GetBankAddress( REGMAP_PRD_CFG_DATA ), sizeof( sProductCfgData ), 5 );

        std::vector<LeddarCore::LdProperty *> lConfigProperties = mProperties->FindPropertiesByCategories( LdProperty::CAT_CONFIGURATION );

        for( size_t i = 0; i < lConfigProperties.size(); ++i )
        {
            lConfigProperties[i]->SetClean();
        }
    }
    catch( ... )
    {
        mConnectionUniversal->SetWriteEnable( false );
        throw;
    }

    mConnectionUniversal->SetWriteEnable( false );
}

// *****************************************************************************
// Function: LdSensorVu8::GetConstants
//
/// \brief   Get constants from the device, store result in the properties.
///
/// \exception std::runtime_error If the device is not connected.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

void
LdSensorVu8::GetConstants()
{
    // Check connection
    if( mConnectionUniversal == nullptr )
        throw std::runtime_error( "No connection associated to the device." );

    try
    {
        // Get constants
        LdSensorVu::GetConstants();

        // ------------- Read product-specific device information -------------
        sProductDevInfo lProductDevInfo = { 0 };
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_PRD_DEV_INFO ), ( uint8_t * )&lProductDevInfo, sizeof( sProductDevInfo ), 5 );

        // Temperature scale
        uint32_t lTempScale = 1 << lProductDevInfo.mTempSensorScaleBits;
        LeddarCore::LdIntegerProperty *lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_TEMPERATURE_SCALE );
        lIntProp->ForceValue( 0, lTempScale );
        lIntProp->SetClean();

        GetResultStates()->GetProperties()->GetFloatProperty( LdPropertyIds::ID_RS_SYSTEM_TEMP )->SetScale( lTempScale );
        GetResultStates()->GetProperties()->GetFloatProperty( LdPropertyIds::ID_RS_PREDICT_TEMP )->SetScale( lTempScale );
    }
    catch( std::exception &e )
    {
        throw LeddarException::LtConfigException( e );
    }
}

#endif