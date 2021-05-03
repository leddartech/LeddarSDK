////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorIS16.cpp
///
/// \brief  Implements the LdSensorIS16 class
///
/// Copyright (c) 2019 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdSensorIS16.h"

#if defined( BUILD_M16 ) && defined( BUILD_USB )

#include "LdPropertyIds.h"
#include "LtTimeUtils.h"
#include "comm/Legacy/M16/LtComM16.h"
#include "comm/LtComLeddarTechPublic.h"

LeddarDevice::LdSensorIS16::LdSensorIS16( LeddarConnection::LdConnection *aConnection )
    : LdSensorM16( aConnection )
{
    InitProperties();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorIS16::InitProperties( void )
///
/// \brief  Initializes the properties for this specific sensor
///
/// \author David Levy
/// \date   May 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::InitProperties( void )
{
    using namespace LeddarCore;

    // Constants
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_NO_MODIFIED_WARNING, LdPropertyIds::ID_REFRESH_RATE_LIST,
                                                   LtComM16::M16_ID_MEASUREMENT_RATE_LIST, 4, 65536, 2, "List of available measurement rates" ) );

    //Config
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_ZONE_RISING_DB,
                              LtComM16::IS16_ID_CFG_DISCRETE_OUTPUTS_RISING_DEBOUNCE, 2, "Activation delay of detection zone in number of sample", false ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_ZONE_FALLING_DB,
                              LtComM16::IS16_ID_CFG_DISCRETE_OUTPUTS_FALLING_DEBOUNCE, 2, "Deactivation delay of detection zone in number of sample", false ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_OUTPUT_NPN_PNP,
                              LtComM16::IS16_ID_CFG_DISCRETE_OUTPUTS_NPN_PNP, 1, "Bits field of electrical outputs configuration per zone: 0=NPN, 1=PNP" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_OUTPUT_INVERSION,
                              LtComM16::IS16_ID_CFG_DISCRETE_OUTPUTS_INV, 1, "Bits field of inverted outputs configuration per zone: 0=normal, 1=inverted" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_ZONE_FARS,
                              LtComM16::IS16_ID_CFG_LVLS_FAR_LIMIT, 4, 65536, 2, "For advanced mode: Far distance limit per segment and per supported zone" ) );
    mProperties->GetFloatProperty( LdPropertyIds::ID_IS16_ZONE_FARS )->SetLimits( 0, 200 );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_ZONE_NEARS,
                              LtComM16::IS16_ID_CFG_LVLS_NEAR_LIMIT, 4, 65536, 2, "For advanced mode: Near distance limit per segment and per supported zone" ) );
    mProperties->GetFloatProperty( LdPropertyIds::ID_IS16_ZONE_NEARS )->SetLimits( 0, 200 );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_ZONE_SEGMENT_ENABLES,
                              LtComM16::IS16_ID_CFG_LVLS_SEGMENTS_ENABLE, 2, "For advanced mode: Bits field of enabled segment per zone" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_ALGO_TYPE,
                              LtComM16::IS16_ID_CFG_LVLS_DETECT_ALGO_TYPE, 1, true, "Algorithm detection type per zone. See LtComM16::eLtCommIS16DectectionAlgoType" ) );
    mProperties->GetEnumProperty( LdPropertyIds::ID_IS16_ALGO_TYPE )->AddEnumPair( LtComM16::IS16_ALGO_TYPE_BOOL_RAW, "Raw" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_IS16_ALGO_TYPE )->AddEnumPair( LtComM16::IS16_ALGO_TYPE_BOOL_COUNTING, "Counting" );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_ZONE_ENABLES,
                              LtComM16::IS16_ID_CFG_LVLS_ZONES_ENABLE, 1, "Enabled detection zones" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_TEACH_MARGIN,
                              LtComM16::IS16_ID_CFG_LVLS_SECURITY_DISTANCE, 4, 65536, 2, "Security distance to add or remove from teach limit" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_ZONE_EDIT_MODE,
                              LtComM16::IS16_ID_CFG_LVLS_LAST_CONFIG_MODE, 1, true, "How to configure each zone" ) );
    mProperties->GetEnumProperty( LdPropertyIds::ID_IS16_ZONE_EDIT_MODE )->AddEnumPair( LtComM16::IS16_CONFIG_TEACH, "Teach" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_IS16_ZONE_EDIT_MODE )->AddEnumPair( LtComM16::IS16_CONFIG_QUICK, "Quick" );
    mProperties->GetEnumProperty( LdPropertyIds::ID_IS16_ZONE_EDIT_MODE )->AddEnumPair( LtComM16::IS16_CONFIG_ADVANCED, "Advanced" );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_LCD_CONTRAST,
                              LtComM16::IS16_ID_CFG_LCD_CONTRAST, 1, "LCD contrast percent", false ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_IS16_LCD_CONTRAST )->SetLimits( 0, 100 );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_LCD_BRIGHTNESS,
                              LtComM16::IS16_ID_CFG_LCD_BACKLIGHT_BRIGHTNESS, 1, "LCD brightness percent", false ) );
    mProperties->GetIntegerProperty( LdPropertyIds::ID_IS16_LCD_BRIGHTNESS )->SetLimits( 0, 100 );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_IS16_LOCK_PANEL,
                              LtComM16::IS16_ID_CFG_CONTROL_PANEL_ACCESS, "Control panel access. Locked on 1" ) );

    // Other
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_OTHER, LdProperty::F_EDITABLE | LdProperty::F_SAVE | LdProperty::F_NO_MODIFIED_WARNING,
                                                  LdPropertyIds::ID_IS16_MEASUREMENT_RATE, LtComM16::M16_ID_MEASUREMENT_RATE, 4, true,
                                                  "Target refresh rate" ) ); // TODO Connect this property to UpdateParamsForTargetRefreshRate
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_OTHER, LdProperty::F_EDITABLE | LdProperty::F_SAVE | LdProperty::F_NO_MODIFIED_WARNING,
                                                     LdPropertyIds::ID_IS16_USEFUL_RANGE, 0, 2, "Usefull range" ) ); // TODO connect this property to PointCountToRange
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorIS16::UpdateConstants( void )
///
/// \brief  Updates the constants
///
/// \author David Levy
/// \date   May 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::UpdateConstants( void )
{
    uint32_t lDistanceScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>();
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE_LIST )->SetScale( lDistanceScale );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_IS16_ZONE_NEARS )->SetScale( lDistanceScale );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_IS16_ZONE_FARS )->SetScale( lDistanceScale );
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_IS16_TEACH_MARGIN )->SetScale( lDistanceScale );

    LdSensorM16::UpdateConstants();

    auto *lRefreshRatesList  = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE_LIST );
    auto *lTargetRefreshRate = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_IS16_MEASUREMENT_RATE );

    if( lRefreshRatesList->Count() > 0 ) // IS16
    {
        lTargetRefreshRate->ClearEnum();
        for( size_t i = 0; i < lRefreshRatesList->Count(); ++i )
        {
            lTargetRefreshRate->AddEnumPair( lRefreshRatesList->RawValue( i ), LeddarUtils::LtStringUtils::IntToString( lRefreshRatesList->Value( i ) ) );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::GetConstants( void )
///
/// \brief  Gets the constants from the device
///
/// \author Patrick Boulay
/// \date   February 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::GetConstants( void )
{
    uint16_t lIds[] = { LtComM16::M16_ID_MEASUREMENT_RATE_LIST };

    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, LT_ALEN( lIds ), sizeof( lIds[0] ), lIds, sizeof( lIds[0] ) );
    mProtocolConfig->SendRequest();

    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );
    LdSensorM16::GetConstants();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorIS16::GetConfig()
///
/// \brief  Gets the configuration
///
/// \author David Lévy
/// \date   December 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::GetConfig()
{
    LdSensorM16::GetConfig();
    UpdateRefreshRateForTargetAccOvers();
    GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_IS16_MEASUREMENT_RATE )->SetClean();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::UpdateParamsForTargetRefreshRate( uint32_t aTargetRefreshRate )
///
/// \brief  Updates the parameters (accumulation, oversampling) for target refresh rate. IS16 only. You need to call SetConfig to apply it
///
/// \param  aTargetRefreshRate  Non scaled target refresh rate - Use ID_IS16_MEASUREMENT_RATE for available values
///
/// \author David Levy
/// \date   April 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::UpdateParamsForTargetRefreshRate( uint32_t aTargetRefreshRate )
{
    // First validate input value
    try
    {
        GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_IS16_MEASUREMENT_RATE )->GetEnumIndexFromValue( aTargetRefreshRate );
    }
    catch( const std::out_of_range & )
    {
        throw std::invalid_argument( "Target refresh rate invalid, check LeddarCore::LdPropertyIds::ID_REFRESH_RATE_LIST property for valid values" );
    }

    mProtocolConfig->StartRequest( LtComM16::M16_CFGSRV_REQUEST_MESUREMENT_RATE_TO_PARAMS );
    mProtocolConfig->AddElement( LtComM16::M16_ID_MEASUREMENT_RATE, 1, sizeof( uint32_t ), &aTargetRefreshRate, static_cast<uint32_t>( sizeof( uint32_t ) ) );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorIS16::UpdateRefreshRateForTargetAccOvers()
///
/// \brief  Updates the refresh rate for target accumulation and oversampling
///
/// \author David Lévy
/// \date   December 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::UpdateRefreshRateForTargetAccOvers()
{
    auto lAcc            = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP )->ValueT<uint32_t>();
    auto lOverS          = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP )->ValueT<uint32_t>();
    auto lBasePointCount = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT )->ValueT<uint32_t>();
    mProtocolConfig->StartRequest( LtComM16::M16_CFGSRV_REQUEST_PARAMS_TO_MESUREMENT_RATE );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_CFG_ACCUMULATION_EXPONENT, 1, sizeof( uint32_t ), &lAcc, static_cast<uint32_t>( sizeof( uint32_t ) ) );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_CFG_OVERSAMPLING_EXPONENT, 1, sizeof( uint32_t ), &lOverS, static_cast<uint32_t>( sizeof( uint32_t ) ) );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_CFG_BASE_SAMPLE_COUNT, 1, sizeof( uint32_t ), &lBasePointCount, static_cast<uint32_t>( sizeof( uint32_t ) ) );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorIS16::SetDefaultQuickLimits( uint8_t aZone )
///
/// \brief  Sets default limits for the quick mode
///
/// \param  aZone   The zone.
///
/// \author David Lévy
/// \date   December 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::SetDefaultQuickLimits( uint8_t aZone )
{
    mProtocolConfig->StartRequest( LtComM16::IS16_CFGSRV_REQUEST_QUICK_MODE );
    mProtocolConfig->AddElement( LtComM16::IS16_ID_LVLS_CONFIG_ZONE, 1, sizeof( uint8_t ), &aZone, sizeof( uint8_t ) );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
    // Must reload config since this call modifies it.
    GetConfig();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorIS16::Teach( uint8_t aZone, uint16_t aDuration )
///
/// \brief  Perform a teach with the parameters given in the teach command.
///
/// \param  aZone       The zone.
/// \param  aDuration   The duration, in number of frame, 0 = default
///
/// \author David Lévy
/// \date   December 2020
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::Teach( uint8_t aZone, uint16_t aDuration )
{
    uint8_t lState = LtComM16::IS16_TEACH_STATE_START;
    mProtocolConfig->StartRequest( LtComM16::IS16_CFGSRV_REQUEST_TEACH );
    mProtocolConfig->AddElement( LtComM16::IS16_ID_LVLS_TEACH_STATE, 1, sizeof( uint8_t ), &lState, sizeof( uint8_t ) );
    mProtocolConfig->AddElement( LtComM16::IS16_ID_LVLS_CONFIG_ZONE, 1, sizeof( uint8_t ), &aZone, sizeof( uint8_t ) );
    if( aDuration != 0 )
    {
        mProtocolConfig->AddElement( LtComM16::IS16_ID_LVLS_TEACH_FRAME, 1, sizeof( uint16_t ), &aDuration, sizeof( uint16_t ) );
    }
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();

    uint32_t lCount     = 0;
    uint8_t lTeachState = LtComM16::IS16_TEACH_STATE_START;
    uint16_t lIds[]     = { LtComM16::IS16_ID_LVLS_TEACH_STATE };

    do
    {
        LeddarUtils::LtTimeUtils::Wait( 100 );

        mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
        mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, LT_ALEN( lIds ), sizeof( lIds[0] ), lIds, sizeof( lIds[0] ) );
        mProtocolConfig->SendRequest();
        mProtocolConfig->ReadAnswer();

        while( mProtocolConfig->ReadElement() )
        {
            if( mProtocolConfig->GetElementId() == LtComM16::IS16_ID_LVLS_TEACH_STATE )
            {
                std::vector<uint8_t> lTeachStates( mProtocolConfig->GetElementCount(), 0 );
                mProtocolConfig->PushElementDataToBuffer( lTeachStates.data(), mProtocolConfig->GetElementCount(), sizeof( lTeachStates[0] ), sizeof( ( lTeachStates[0] ) ) );
                lTeachState = lTeachStates[aZone];
            }
        }

    } while( lTeachState == LtComM16::IS16_TEACH_STATE_TEACHING && ++lCount < 50 );

    if( lTeachState == LtComM16::IS16_TEACH_STATE_STOPPED )
    {
        GetConfig();
    }
    else
    {
        throw std::runtime_error("Teaching of the detection zone has failed");
    }
}

#endif
