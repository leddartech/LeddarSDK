////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorIS16.cpp
///
/// \brief  Implements the LdSensorIS16 class
///
/// Copyright (c) 2019 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdSensorIS16.h"

#include "comm/LtComLeddarTechPublic.h"
#include "comm/Legacy/M16/LtComM16.h"
#include "LdPropertyIds.h"

#if defined(BUILD_M16) && defined(BUILD_USB)

#endif

LeddarDevice::LdSensorIS16::LdSensorIS16( LeddarConnection::LdConnection *aConnection ) :
    LdSensorM16( aConnection )
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

    //Constants
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_REFRESH_RATE_LIST,
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
    LdSensorM16::GetConstants();
    uint16_t lIds[] =
    {
        LtComM16::M16_ID_MEASUREMENT_RATE_LIST
    };

    mProtocolConfig->StartRequest( LtComLeddarTechPublic::LT_COMM_CFGSRV_REQUEST_GET );
    mProtocolConfig->AddElement( LtComLeddarTechPublic::LT_COMM_ID_ELEMENT_LIST, LT_ALEN( lIds ), sizeof( lIds[0] ), lIds, sizeof( lIds[0] ) );
    mProtocolConfig->SendRequest();

    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorM16::UpdateParamsForTargetRefreshRate( uint32_t aTargetRefreshRate )
///
/// \brief  Updates the parameters (accumulation, oversampling) for target refresh rate. IS16 only. You need to call SetConfig to apply it
///
/// \param  aTargetRefreshRate  Target refresh rate - Check available value in ID_REFRESH_RATE_LIST
///
/// \author David Levy
/// \date   April 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorIS16::UpdateParamsForTargetRefreshRate( float aTargetRefreshRate )
{
    //First validate input value
    LeddarCore::LdFloatProperty *lRefreshRates = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_REFRESH_RATE_LIST );
    uint32_t lScaledRefresh = 0;

    if( lRefreshRates->Count() > 0 ) //IS16
    {
        for( size_t i = 0; i < lRefreshRates->Count(); ++i )
        {
            if( aTargetRefreshRate == lRefreshRates->Value( i ) )
            {
                lScaledRefresh = lRefreshRates->RawValue( i );
                break;
            }
        }
    }

    if( lScaledRefresh == 0 )
    {
        throw std::invalid_argument( "Target refresh rate invalid, check LeddarCore::LdPropertyIds::ID_REFRESH_RATE_LIST property for valid values" );
    }

    mProtocolConfig->StartRequest( LtComM16::M16_CFGSRV_REQUEST_MESUREMENT_RATE_TO_PARAMS );
    mProtocolConfig->AddElement( LtComM16::M16_ID_MEASUREMENT_RATE, 1, sizeof( uint32_t ), &lScaledRefresh, static_cast<uint32_t>( sizeof( uint32_t ) ) );
    mProtocolConfig->SendRequest();
    mProtocolConfig->ReadAnswer();
    mProtocolConfig->ReadElementToProperties( GetProperties() );
}