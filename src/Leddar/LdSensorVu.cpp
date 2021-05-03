////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdSensorVu.cpp
///
/// \brief  Implements the ldSensorVu class
///
/// Copyright (c) 2016 LeddarTech. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LdSensorVu.h"
#if defined(BUILD_VU)

#include "LdPropertyIds.h"

#include "LdBitFieldProperty.h"
#include "LdBoolProperty.h"
#include "LdEnumProperty.h"
#include "LdFloatProperty.h"
#include "LdIntegerProperty.h"
#include "LdTextProperty.h"

#include "LdSensorVuDefines.h"
#include "LdConnectionUniversal.h"
#include "LdResultProvider.h"
#include "LtCRCUtils.h"

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

#include "LtExceptions.h"
#include "LtStringUtils.h"
#include "LdResultEchoes.h"
#include "LtSystemUtils.h"
#include "LtTimeUtils.h"

#include <cstddef>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <iostream>

using namespace LeddarCore;
using namespace LeddarConnection;

constexpr uint8_t LICENSE_USER_SIZE = 2 * REGMAP_KEY_LENGTH;
constexpr uint8_t LICENSE_NUMBER    = 3;
constexpr uint8_t NUMBER_OF_RETRY   = 5; ///< Number of retry if CRC failed.

using namespace LeddarDevice;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdSensorVu::LdSensorVu( LeddarConnection::LdConnection *aConnection )
///
/// \brief  Constructor - Take ownership of aConnection (and the 2 pointers used to build it)
///
/// \param [in,out] aConnection If non-null, the connection.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdSensorVu::LdSensorVu( LeddarConnection::LdConnection *aConnection ) :
    LdSensor( aConnection ),
    mConnectionUniversal( nullptr ),
    mChannelCount( 0 ),
    mCalibrationOffsetBackup( nullptr ),
    mCalibrationLedBackup( nullptr ),
    mRepair( false ),
#ifdef BUILD_MODBUS
    mCarrier( nullptr ),
#endif
    mErrorFlag( false ),
    mBackupFlagAvailable( true )
{
    InitProperties();
    mConnectionUniversal = dynamic_cast<LdConnectionUniversal *>( aConnection );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdSensorVu::~LdSensorVu()
///
/// \brief  Destructor.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LdSensorVu::~LdSensorVu()
{
#ifdef BUILD_MODBUS

    if( mCarrier )
    {
        delete mCarrier;
        mCarrier = nullptr;
    }

#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::InitProperties( void )
///
/// \brief  Create properties for this specific sensor.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::InitProperties( void )
{
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_RSEGMENT, 0, 2, "Number of reference segment" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_DEVICE_NAME, 0,
                              REGMAP_PRODUCT_NAME_LENGTH,
                              LdTextProperty::TYPE_ASCII, "Device Name" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_PART_NUMBER, 0, REGMAP_PRODUCT_ID_LENGTH, LdTextProperty::TYPE_ASCII,
                              "Part Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SOFTWARE_PART_NUMBER, 0, REGMAP_PRODUCT_NAME_LENGTH,
                              LdTextProperty::TYPE_ASCII,
                              "Software Part Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_MANUFACTURER_NAME, 0, REGMAP_MFG_NAME_LENGTH, LdTextProperty::TYPE_ASCII,
                              "Manufacturer Name" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_SERIAL_NUMBER, 0, REGMAP_SERIAL_NUMBER_LENGTH, LdTextProperty::TYPE_ASCII,
                              "Serial Number" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_BUILD_DATE, 0, REGMAP_BUILD_DATE, LdTextProperty::TYPE_ASCII,
                              "Build Date" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FIRMWARE_VERSION_STR, 0, REGMAP_FIRMWATE_VERSION_LENGTH,
                              LdTextProperty::TYPE_ASCII,
                              "Firmware Version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_BOOTLOADER_VERSION, 0, REGMAP_BOOTLOADER_VERSION_LENGTH,
                              LdTextProperty::TYPE_ASCII,
                              "Bootloader Version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_ASIC_VERSION, 0, REGMAP_ASIC_VERSION_LENGTH, LdTextProperty::TYPE_ASCII,
                              "Asic Version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_FPGA_VERSION, 0, REGMAP_FPGA_VERSION_LENGTH, LdTextProperty::TYPE_ASCII,
                              "FPGA Version" ) );
    mProperties->AddProperty( new LdTextProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_GROUP_ID_NUMBER, 0, REGMAP_GROUP_ID_LENGTH, LdTextProperty::TYPE_ASCII,
                              "Group ID Number" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_OPTIONS, 0, 4, "Options" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACCUMULATION_EXP, 0, 1, true,
                              "Accumulation Exponent" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_OVERSAMPLING_EXP, 0, 1, true,
                              "Oversampling Exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_BASE_POINT_COUNT, 0, 1,
                              "Base Point Count" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_NB_SAMPLE_MAX, 0, 2, "Number Sample Max" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_REF_SEG_MASK, 0, 4, "Reference Segment Mask" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_BASE_SAMPLE_DISTANCE, 0, 4, 0, 3, "Base Sample Distance" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL, 0, 1, "Max Detection per Segment" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_DISTANCE_SCALE, 0, 4, "Distance Scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_NONE, LdPropertyIds::ID_RAW_AMP_SCALE_BITS, 0, 1, "Raw Amplitude Scale Bits" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_RAW_AMP_SCALE, 0, 4, "Raw Amplitude Scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_FILTERED_AMP_SCALE, 0, 4, "Filtered Amplitude Scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_NONE, LdPropertyIds::ID_FILTERED_AMP_SCALE_BITS, 0, 1, "Filtered Amplitude Scale Bits" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PRECISION, 0, 1, "Smoothing",
                              true ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PRECISION_ENABLE, 0,
                              "Smoothing Enable" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SEGMENT_ENABLE, 0, 4,
                              "Segment Enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_XTALK_ECHO_REMOVAL_ENABLE, 0,
                              "Crosstalk Echo Removal Enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_XTALK_REMOVAL_ENABLE, 0,
                              "Crosstalk Removal Enable" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_PULSE_RATE, 0, 4, "Pulse Frequency" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_CPU_LOAD_SCALE, 0, 4, "CPU Load Scale" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_TEMPERATURE_SCALE, 0, 4, "Temperature Scale" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SATURATION_COMP_ENABLE, 0,
                              "Saturation Compensation Enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_OVERSHOOT_MNG_ENABLE, 0,
                              "Overshoot Management Enable" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_SENSIVITY, 0, 4, 0, 2,
                              "Threshold Offset" ) );
    mProperties->AddProperty( new LdEnumProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_INTENSITY, 0, 2, true,
                              "Led Intensity %" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_AUTO_PWR_ENABLE, 0, 1,
                              "Auto Led Power Enable" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_AUTO_FRAME_AVG, 0, 2,
                              "Change Delay (Frame)" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_LED_AUTO_ECHO_AVG, 0, 1,
                              "Change Delay (Channel)" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_LEARNED_TRACE_OPTIONS, 0, 1, "Learned Trace Options" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_NONE, LdPropertyIds::ID_LED_USR_PWR_COUNT, 0, 1, "Led Power Count" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_DEMERGING_ENABLE, 0,
                              "Demerging Enable" ) );
    mProperties->AddProperty( new LdBoolProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_STATIC_NOISE_REMOVAL_ENABLE, 0,
                              "Static Noise Removal Enable" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONSTANT, LdProperty::F_SAVE, LdPropertyIds::ID_REAL_DISTANCE_OFFSET, 0, 4, 0, 3, "Real Distance Offset" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_X, 0, 4, 0, 3, "X Position" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_Y, 0, 4, 0, 3, "Y Position" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ORIGIN_Z, 0, 4, 0, 3, "Z Position" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_YAW, 0, 4, 0, 3, "Yaw Rotation" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_PITCH, 0, 4, 0, 3,
                              "Pitch Rotation" ) );
    mProperties->AddProperty( new LdFloatProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ROLL, 0, 4, 0, 3, "Roll Rotation" ) );
    mProperties->AddProperty( new LdBitFieldProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_TEMP_COMP, 0, 1,
                              "Temperature compensation. Possible value are 0 (no compensation), 1 (reference pulse), 2( compensation table) - Require integrator license to change" ) );
    mProperties->GetBitProperty( LeddarCore::LdPropertyIds::ID_TEMP_COMP )->SetExclusivityMask( 3 ); //0000 0011

    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_OTHER, LdProperty::F_EDITABLE, LdPropertyIds::ID_LICENSE, 0, REGMAP_KEY_LENGTH, "License key" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_OTHER, LdProperty::F_NONE, LdPropertyIds::ID_LICENSE_INFO, 0, 4, "License type / subtype" ) );
    mProperties->AddProperty( new LdBufferProperty( LdProperty::CAT_OTHER, LdProperty::F_EDITABLE, LdPropertyIds::ID_VOLATILE_LICENSE, 0, REGMAP_KEY_LENGTH,
                              "Temporary license key - internal use" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_OTHER, LdProperty::F_NONE, LdPropertyIds::ID_VOLATILE_LICENSE_INFO, 0, 4,
                              "Volatile license type / subtype - internal use" ) );

    GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_TIMEBASE_DELAY, 0, 4, 0, 3,
                                  "Time base delays - Require integrator licence to edit" ) );
    GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_CALIBRATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_INTENSITY_COMPENSATIONS, 0, 4, 0, 3,
                                  "Compensations - Require integrator licence to edit" ) );

    // Result State properties
    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_SYSTEM_TEMP, 0, 4, 0, 1,
            "System Temperature" ) );
    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_PREDICT_TEMP, 0, 4, 0, 1,
            "System Predicted Temperature" ) );
    GetResultStates()->GetProperties()->AddProperty( new LdFloatProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_CPU_LOAD, 0, 4, 0, 1, "CPU Load" ) );
    GetResultStates()->GetProperties()->AddProperty( new LdEnumProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_BACKUP, 0, 4,true , "Calibration Backup Flag" ) );

    GetResultStates()->GetProperties()->GetEnumProperty(LdPropertyIds::ID_RS_BACKUP)->AddEnumPair(0, "Invalid");
    GetResultStates()->GetProperties()->GetEnumProperty(LdPropertyIds::ID_RS_BACKUP)->AddEnumPair(1, "Factory backup");
    GetResultStates()->GetProperties()->GetEnumProperty(LdPropertyIds::ID_RS_BACKUP)->AddEnumPair(2, "User backup");

    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0,
            P_SPI ); //We set it as SPI here. SetCarrier function will change it to modbus if needed
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();

    
    GetResultEchoes()->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_CURRENT_LED_INTENSITY, 0, 2, "Current Led power" ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::GetConfig()
///
/// \brief  Get configuration from the device, store result in the properties.
///
/// \exception  LeddarException::LtConfigException  Thrown when a Lt Configuration error condition occurs.
/// \exception  LtConfigException                   If the configuration value is not valid.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::GetConfig()
{
    try
    {
        uint32_t lDistanceScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>();

        if( lDistanceScale == 1 )
        {
            throw LeddarException::LtConfigException( "Distance scale should not be 1. Call GetConstants first." );
        }

        uint32_t lAmplitudeScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RAW_AMP_SCALE )->ValueT<uint32_t>();

        if( lAmplitudeScale == 1 )
        {
            throw LeddarException::LtConfigException( "Amplitude scale should not be 1. Call GetConstants first." );
        }

        // ------------- Read configuration data from sensor -------------
        uint8_t *lInputBuffer, *lOutputBuffer;
        mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_CFG_DATA ), sizeof( sCfgData ), 5 );
        sCfgData *lCfgData = reinterpret_cast<sCfgData * >( lOutputBuffer );

        // Device name
        LeddarCore::LdTextProperty *lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_DEVICE_NAME );
        lTextProp->SetValue( 0, std::string( reinterpret_cast<char *>( lCfgData->mDeviceName ), REGMAP_PRODUCT_NAME_LENGTH ) );
        lTextProp->SetClean();

        // Accumulation exponent
        auto *lEnumProp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP );
        lEnumProp->SetValue( 0, lCfgData->mAccumulationExp );
        lEnumProp->SetClean();

        // Oversampling exponent
        lEnumProp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP );
        lEnumProp->SetValue( 0, lCfgData->mOversamplingExp );
        lEnumProp->SetClean();

        // Base point count
        auto *lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT );
        lIntProp->SetValue( 0, lCfgData->mBasePointCount );
        lIntProp->SetClean();

        // Segment enable
        LeddarCore::LdBitFieldProperty *lBitProp = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE );
        lBitProp->SetValue( 0, lCfgData->mSegmentEnable );
        lBitProp->SetClean();

        // Origin X
        LeddarCore::LdFloatProperty *lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_ORIGIN_X );
        lFloatProp->SetValue( 0, lCfgData->mX );
        lFloatProp->SetClean();

        // Origin Y
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_ORIGIN_Y );
        lFloatProp->SetValue( 0, lCfgData->mY );
        lFloatProp->SetClean();

        // Origin Z
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_ORIGIN_Z );
        lFloatProp->SetValue( 0, lCfgData->mZ );
        lFloatProp->SetClean();

        // YAW
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_YAW );
        lFloatProp->SetValue( 0, lCfgData->mYaw );
        lFloatProp->SetClean();

        // Pitch
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_PITCH );
        lFloatProp->SetValue( 0, lCfgData->mPitch );
        lFloatProp->SetClean();

        // Roll
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_ROLL );
        lFloatProp->SetValue( 0, lCfgData->mRoll );
        lFloatProp->SetClean();

        // Precision
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION );
        lIntProp->SetValue( 0, lCfgData->mPrecision );
        lIntProp->SetClean();

        // Precision enable
        LeddarCore::LdBoolProperty *lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_PRECISION_ENABLE );
        lBoolProp->SetValue( 0, lCfgData->mPrecisionEnable == 1 );
        lBoolProp->SetClean();

        // Saturation compensation enable
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_SATURATION_COMP_ENABLE );
        lBoolProp->SetValue( 0, lCfgData->mSatCompEnable == 1 );
        lBoolProp->SetClean();

        // Overshot managment enable
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_OVERSHOOT_MNG_ENABLE );
        lBoolProp->SetValue( 0, lCfgData->mOvershootManagementEnable == 1 );
        lBoolProp->SetClean();

        // Sensivity
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY );
        lFloatProp->SetScale( lAmplitudeScale );
        lFloatProp->SetRawValue( 0, lCfgData->mSensitivity );
        lFloatProp->SetClean();

        // LED power (in percent)
        uint8_t lLedIntensitySelected = lCfgData->mLedUserCurrentPowerPercent;

        // Auto LED power enable
        lBitProp = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_PWR_ENABLE );
        lBitProp->SetValue( 0, lCfgData->mLedUserAutoPowerEnable );
        lBitProp->SetClean();

        // Auto LED frame average
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG );
        lIntProp->SetValue( 0, lCfgData->mLedUserAutoFrameAvg );
        lIntProp->SetClean();

        // Auto LED echo average
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_ECHO_AVG );
        lIntProp->SetValue( 0, lCfgData->mLedUserAutoEchoAvg );
        lIntProp->SetClean();

        // Demerging enable
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_DEMERGING_ENABLE );
        lBoolProp->SetValue( 0, lCfgData->mDemEnable == 1 );
        lBoolProp->SetClean();

        // Static noise removal enable
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_REMOVAL_ENABLE );
        lBoolProp->SetValue( 0, lCfgData->mStNoiseRmvEnable == 1 );
        lBoolProp->SetClean();

        // ------------- Read advanced config data from sensor (part 1) -------------
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mTraceBufferType ),
                                    sizeof( uint8_t ) + sizeof( uint32_t ), 0 );

        // Field of view
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_HFOV );
        lFloatProp->SetScale( lDistanceScale );
        lFloatProp->ForceRawValue( 0, *( ( uint32_t * )( &lOutputBuffer[ 1 ] ) ) );
        lFloatProp->SetClean();

        // -------------  Read advanced config data from sensor (part 2) -------------
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mPeakFilterSumBits ),
                                    sizeof( uint8_t ), 5 );

        // Amplitude scales
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE_BITS );
        lIntProp->ForceValue( 0, *( lOutputBuffer ) );
        lIntProp->SetClean();
        uint8_t lFilteredAmpScaleBits = lIntProp->ValueT<uint8_t>();

        uint8_t lRawAmplBit = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RAW_AMP_SCALE_BITS )->ValueT<uint8_t>();
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE );
        lIntProp->ForceValue( 0, 1 << ( lRawAmplBit + lFilteredAmpScaleBits ) );
        lIntProp->SetClean();

        // -------------  Read advanced config data from sensor (part 3) -------------
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mLedUserPowerEnable ),
                                    offsetof( sAdvCfgData, mLedUserPowerLut ) - offsetof( sAdvCfgData, mLedUserPowerEnable ), 5 );

        // User led power count
        uint8_t lLedPwrCount = *( lOutputBuffer + offsetof( sAdvCfgData, mLedUsrPowerCount ) - offsetof( sAdvCfgData, mLedUserPowerEnable ) );
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_USR_PWR_COUNT );
        lIntProp->ForceValue( 0, lLedPwrCount );
        lIntProp->SetClean();

        // -------------  Read advanced config data from sensor (part 4) -------------
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mLedUserPercentLut ),
                                    offsetof( sAdvCfgData, mDemAmpThrMin ) - offsetof( sAdvCfgData, mLedUserPercentLut ), 5 );

        // User led power lookup table
        uint8_t *lLedPwrLUT = lOutputBuffer;
        LeddarCore::LdEnumProperty *lLedIntensityProp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY );
        lLedIntensityProp->SetEnumSize( lLedPwrCount );

        for( int i = 0; i < lLedPwrCount; ++i )
        {
            lLedIntensityProp->AddEnumPair( lLedPwrLUT[ i ], LeddarUtils::LtStringUtils::IntToString( lLedPwrLUT[ i ] ) );
        }


        try
        {
            lLedIntensityProp->SetValue( 0, lLedIntensitySelected );
        }
        catch( std::exception & )
        {
            // Invalid intensity, find the closest one
            size_t lClosestIndex = 0;
            int8_t lDeltaIntensity = 100;

            for( size_t i = 0; i < lLedIntensityProp->EnumSize(); ++i )
            {
                int8_t lDelta = lLedIntensitySelected - static_cast<int8_t>( lLedIntensityProp->EnumValue( i ) );

                if( lDelta < 0 )
                    lDelta = -lDelta;

                if( lDelta <= lDeltaIntensity )
                {
                    lDeltaIntensity = lDelta;
                    lClosestIndex = i;
                }
            }

            lLedIntensityProp->SetValue( 0, lLedIntensityProp->EnumValue( lClosestIndex ) );
            mRepair = true;
        }

        lLedIntensityProp->SetClean();

        // -------------  Read advanced config data from sensor (part 5) -------------
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mPeakRealDistanceOffset ),
                                    offsetof( sAdvCfgData, mPeakNbSampleForBaseLevEst ) - offsetof( sAdvCfgData, mPeakRealDistanceOffset ), 5 );

        // Real distance offset
        uint32_t *lPeakRealDistanceOffset = ( uint32_t * )( lOutputBuffer );
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_REAL_DISTANCE_OFFSET );
        lFloatProp->SetScale( lDistanceScale );
        lFloatProp->SetCount( 1 );
        lFloatProp->ForceRawValue( 0, *lPeakRealDistanceOffset );
        lFloatProp->SetClean();

        // -------------  Read advanced config data from sensor (part 6) -------------
        LeddarCore::LdBitFieldProperty *lBitFieldProp = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_TEMP_COMP );
        lBitFieldProp->SetValue( 0, 0 );
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mPeakRefDistEnable ), sizeof( uint8_t ), 5 );

        if( *lOutputBuffer != 0 )
        {
            lBitFieldProp->SetBit( 0, 0 );
        }

        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mPeakTempEnable ), sizeof( uint8_t ), 5 );

        if( *lOutputBuffer != 0 )
        {
            lBitFieldProp->SetBit( 0, 1 );
        }
        lBitFieldProp->SetClean();

        // Initialize results.
        uint32_t lTotalSegments = mProperties->GetIntegerProperty( LdPropertyIds::ID_VSEGMENT )->ValueT<uint16_t>() *
                                  mProperties->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->ValueT<uint16_t>();
        uint32_t lMaxDetections = lTotalSegments * mProperties->GetIntegerProperty( LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL )->ValueT<uint32_t>();
        uint32_t lTempeatureScale = mProperties->GetIntegerProperty( LdPropertyIds::ID_TEMPERATURE_SCALE )->ValueT<uint32_t>();
        uint32_t lCpuLoadScale = mProperties->GetIntegerProperty( LdPropertyIds::ID_CPU_LOAD_SCALE )->ValueT<uint32_t>();
        mEchoes.Init( mProperties->GetIntegerProperty( LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>(),
                      mProperties->GetIntegerProperty( LdPropertyIds::ID_FILTERED_AMP_SCALE )->ValueT<uint32_t>(),
                      lMaxDetections );
        mStates.Init( lTempeatureScale, lCpuLoadScale );
#ifdef BUILD_MODBUS

        // Get the configs on the carrier board if exists
        if( GetCarrier() != nullptr )
        {
            GetCarrier()->GetConfigSerial();
            GetCarrier()->GetConfigCAN();
        }

#endif
    }
    catch( std::exception &e )
    {
        throw LeddarException::LtConfigException( e );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::SetConfig()
///
/// \brief  Set configuration to the device, store result in the properties.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::SetConfig()
{
    try
    {
        uint8_t *lInputBuffer, *lOutputBuffer;
        mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );
        sCfgData *lCfgData = reinterpret_cast< sCfgData *>( lInputBuffer );
        memset( lCfgData, 0, sizeof( sCfgData ) );

        // ------------- Set configuration data in sensor -------------
        // Device name
        LeddarCore::LdTextProperty *lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_DEVICE_NAME );
        memcpy( lCfgData->mDeviceName, lTextProp->Value().c_str(), REGMAP_PRODUCT_NAME_LENGTH );

        // Accumulation Exponent
        auto *lEnumProp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP );
        lCfgData->mAccumulationExp = lEnumProp->ValueT<uint8_t>();

        // Oversampling exponent
        lEnumProp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP );
        lCfgData->mOversamplingExp = lEnumProp->ValueT<uint8_t>();

        // Base point count
        auto *lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT );
        lCfgData->mBasePointCount = lIntProp->ValueT<uint8_t>();

        // Segment enable
        LeddarCore::LdBitFieldProperty *lBitFieldProp = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_SEGMENT_ENABLE );
        lCfgData->mSegmentEnable = lBitFieldProp->Value();

        // Reference pulse rate
        lCfgData->mRefPulseRate = 1; //Workaround firmware bug

        // Origin X
        LeddarCore::LdFloatProperty *lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_ORIGIN_X );
        lCfgData->mX = lFloatProp->Value();

        // Origin Y
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_ORIGIN_Y );
        lCfgData->mY = lFloatProp->Value();

        // Origin Z
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_ORIGIN_Z );
        lCfgData->mZ = lFloatProp->Value();

        // YAW
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_YAW );
        lCfgData->mYaw = lFloatProp->Value();

        // Pitch
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_PITCH );
        lCfgData->mPitch = lFloatProp->Value();

        // Roll
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_ROLL );
        lCfgData->mRoll = lFloatProp->Value();

        // Precision
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION );
        lCfgData->mPrecision = lIntProp->ValueT<int8_t>();

        // Precision enable
        LeddarCore::LdBoolProperty *lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_PRECISION_ENABLE );
        lCfgData->mPrecisionEnable = lBoolProp->Value() == true ? 1 : 0;

        // Saturation compensation enable
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_SATURATION_COMP_ENABLE );
        lCfgData->mSatCompEnable = lBoolProp->Value() == true ? 1 : 0;

        // Ovrshoot management enable
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_OVERSHOOT_MNG_ENABLE );
        lCfgData->mOvershootManagementEnable = lBoolProp->Value() == true ? 1 : 0;

        // Sensivity setting
        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY );
        lCfgData->mSensitivity = lFloatProp->RawValue();

        // Current LED power level
        lEnumProp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY );
        lCfgData->mLedUserCurrentPowerPercent = static_cast<uint8_t>( lEnumProp->Value() );

        // Auto LED power enable
        LeddarCore::LdBitFieldProperty *lBitProplBitProp = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_PWR_ENABLE );
        lCfgData->mLedUserAutoPowerEnable = lBitProplBitProp->Value();

        // Auto frame average
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG );
        lCfgData->mLedUserAutoFrameAvg = lIntProp->ValueT<uint16_t>();

        // Auto echo average
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_ECHO_AVG );
        lCfgData->mLedUserAutoEchoAvg = lIntProp->ValueT<uint8_t>();

        // Demerging enable
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_DEMERGING_ENABLE );
        lCfgData->mDemEnable = lBoolProp->Value() == true ? 1 : 0;

        // Static noise removal enable
        lBoolProp = GetProperties()->GetBoolProperty( LeddarCore::LdPropertyIds::ID_STATIC_NOISE_REMOVAL_ENABLE );
        lCfgData->mStNoiseRmvEnable = lBoolProp->Value() == true ? 1 : 0;

        // Write config data into the sensor.
        mConnectionUniversal->Write( 0x2, GetBankAddress( REGMAP_CFG_DATA ), sizeof( sCfgData ), 5 );

        // -------------  Write advanced config data from sensor (part 6 of GetConfig) -------------
        //Need an integrator license to write it
        std::vector<LeddarDefines::sLicense> lLicenses = GetLicenses();

        for( size_t i = 0; i < lLicenses.size(); i++ )
        {
            if( lLicenses[i].mType == LeddarDefines::LT_INTEGRATOR || lLicenses[i].mType == LeddarDefines::LT_ADMIN )
            {
                lBitFieldProp = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_TEMP_COMP );
                *lInputBuffer = lBitFieldProp->BitState( 0, 0 );
                mConnectionUniversal->Write( 0x2, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mPeakRefDistEnable ), sizeof( uint8_t ), 5 );
                *lInputBuffer = lBitFieldProp->BitState( 0, 1 );
                mConnectionUniversal->Write( 0x2, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mPeakTempEnable ), sizeof( uint8_t ), 5 );
                break;
            }
        }

#ifdef BUILD_MODBUS

        // Set the configs on the carrier board if exists
        if( GetCarrier() != nullptr )
        {
            GetCarrier()->SetConfigSerial();
            GetCarrier()->SetConfigCAN();
        }

#endif
    }
    catch( ... )
    {
        throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::GetConstants()
///
/// \brief  Get constants from the device, store result in the properties.
///
/// \exception  LeddarException::LtConfigException  Thrown when a Lt Configuration error condition occurs.
/// \exception  LtConfigException                   If a constant value is not valid.
///
/// \author Patrick Boulay
/// \author Vincent Simard Bilodeau
/// \date   April 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::GetConstants()
{
    try
    {
        // Get communication buffer.
        uint8_t *lInputBuffer, *lOutputBuffer;
        mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

        // ---- DEVICE INFO ----
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_DEV_INFO ), sizeof( sDevInfo ), 5 );
        sDevInfo *lDevInfo = reinterpret_cast< sDevInfo * >( lOutputBuffer );

        LeddarCore::LdIntegerProperty *lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE );
        lIntProp->ForceValue( 0, lDevInfo->mDeviceType );
        lIntProp->SetClean();

        LeddarCore::LdTextProperty *lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_PART_NUMBER );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mPartNumber, REGMAP_PRODUCT_ID_LENGTH ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SOFTWARE_PART_NUMBER );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mSoftPartNumber, REGMAP_PRODUCT_NAME_LENGTH ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_MANUFACTURER_NAME );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mMfgName, REGMAP_MFG_NAME_LENGTH ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_SERIAL_NUMBER );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mSerialNumber, REGMAP_SERIAL_NUMBER_LENGTH ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_BUILD_DATE );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mBuildDate, REGMAP_BUILD_DATE ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_FIRMWARE_VERSION_STR );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mFirmwareVersion, REGMAP_FIRMWATE_VERSION_LENGTH ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_BOOTLOADER_VERSION );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mBootldVersion, REGMAP_BOOTLOADER_VERSION_LENGTH ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_ASIC_VERSION );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mASICVersion, REGMAP_ASIC_VERSION_LENGTH ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_FPGA_VERSION );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mFPGAVersion, REGMAP_FPGA_VERSION_LENGTH ) );
        lTextProp->SetClean();

        lTextProp = GetProperties()->GetTextProperty( LeddarCore::LdPropertyIds::ID_GROUP_ID_NUMBER );
        lTextProp->ForceValue( 0, std::string( lDevInfo->mGroupIdenficationNumber, REGMAP_GROUP_ID_LENGTH ) );
        lTextProp->SetClean();

        LeddarCore::LdBitFieldProperty *lBitProp = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_OPTIONS );
        lBitProp->ForceValue( 0, lDevInfo->mOptions );
        lBitProp->SetClean();

        auto *lAccumulationExp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP );        
        for( size_t i = lDevInfo->mAccExpMin; i <= lDevInfo->mAccExpMax; ++i )
        {
            lAccumulationExp->AddEnumPair( i, LeddarUtils::LtStringUtils::IntToString( 1 << i ) );
        }
        lAccumulationExp->SetClean();

        auto *lOverSExp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP );
        for( size_t i = lDevInfo->mOvrExpMin; i <= lDevInfo->mOvrExpMax; ++i )
        {
            lOverSExp->AddEnumPair( i, LeddarUtils::LtStringUtils::IntToString( 1 << i ) );
        }
        lOverSExp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT );
        lIntProp->SetLimits( lDevInfo->mBasePointMin, lDevInfo->mBasePointMax );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_HSEGMENT );
        lIntProp->ForceValue( 0, lDevInfo->mNbHonrizontalSegment );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_VSEGMENT );
        lIntProp->ForceValue( 0, lDevInfo->mNbVerticalSegment );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RSEGMENT );
        lIntProp->ForceValue( 0, lDevInfo->mNbRefSegment );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_NB_SAMPLE_MAX );
        lIntProp->ForceValue( 0, lDevInfo->mNbSampleMax );
        lIntProp->SetClean();

        lBitProp = GetProperties()->GetBitProperty( LeddarCore::LdPropertyIds::ID_REF_SEG_MASK );
        lBitProp->ForceValue( 0, lDevInfo->mRefSegMask );
        lBitProp->SetClean();

        LeddarCore::LdFloatProperty *lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_BASE_SAMPLE_DISTANCE );
        lFloatProp->SetScale( lDevInfo->mDistanceScale );
        lFloatProp->ForceRawValue( 0, lDevInfo->mBaseSplDist );
        lFloatProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_MAX_ECHOES_PER_CHANNEL );
        lIntProp->ForceValue( 0, lDevInfo->mDetectionPerSegmentCountMax );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE );
        lIntProp->ForceValue( 0, lDevInfo->mDistanceScale );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RAW_AMP_SCALE_BITS );
        lIntProp->ForceValue( 0, lDevInfo->mRawAmplitudeScaleBits );
        lIntProp->SetClean();

        uint32_t lAmplitudeScale = lDevInfo->mRawAmplitudeScale;
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RAW_AMP_SCALE );
        lIntProp->ForceValue( 0, lDevInfo->mRawAmplitudeScale );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_PRECISION );
        lIntProp->SetLimits( lDevInfo->mPrecisionMin, lDevInfo->mPrecisionMax );
        lIntProp->SetClean();

        lFloatProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY );
        lFloatProp->SetScale( lAmplitudeScale );
        lFloatProp->SetRawLimits( lDevInfo->mSensitivitytMin, lDevInfo->mSensitivitytMax );
        lFloatProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_FRAME_AVG );
        lIntProp->SetLimits( lDevInfo->mLedUserAutoFrameAvgMin, lDevInfo->mLedUserAutoFrameAvgMax );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_AUTO_ECHO_AVG );
        lIntProp->SetLimits( lDevInfo->mLedUserAutoEchoAvgMin, lDevInfo->mLedUserAutoEchoAvgMax );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LEARNED_TRACE_OPTIONS );
        lIntProp->ForceValue( 0, lDevInfo->mStNoiseRmvCalibBy );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CPU_LOAD_SCALE );
        lIntProp->ForceValue( 0, lDevInfo->mCpuLoadScale );
        lIntProp->SetClean();
        GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_CPU_LOAD )->SetScale( lDevInfo->mCpuLoadScale );

        mChannelCount = lDevInfo->mNbVerticalSegment * lDevInfo->mNbHonrizontalSegment + lDevInfo->mNbRefSegment;
#ifdef BUILD_MODBUS

        if( GetCarrier() != nullptr )
        {
            GetCarrier()->GetConstants();
        }

#endif
    }
    catch( std::exception &e )
    {
        throw LeddarException::LtConfigException( e );
    }

    UpdateConstants();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::UpdateConstants()
///
/// \brief  Updates the constants after fetching constants from the sensor.
///         Several updates are also done in GetConstants() because SetRawValue require a scale before usage
///
/// \author David Levy
/// \date   January 2019
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::UpdateConstants()
{
    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_BASE_SAMPLE_DISTANCE )->SetScale( GetProperties()->GetIntegerProperty(
                LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>() );

    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_SENSIVITY )->SetScale( GetProperties()->GetIntegerProperty(
                LeddarCore::LdPropertyIds::ID_RAW_AMP_SCALE )->ValueT<uint32_t>() );

    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_REAL_DISTANCE_OFFSET )->SetScale( GetProperties()->GetIntegerProperty(
                LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>() );

    GetResultStates()->GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_CPU_LOAD )->SetScale( GetProperties()->GetIntegerProperty(
                LeddarCore::LdPropertyIds::ID_CPU_LOAD_SCALE )->ValueT<uint32_t>() );

    GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_HFOV )->SetScale( GetProperties()->GetIntegerProperty(
                LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>() );

    GetProperties()->GetBitProperty( LdPropertyIds::ID_SEGMENT_ENABLE )->SetLimit( ( 1 << ( GetProperties()->GetIntegerProperty( LdPropertyIds::ID_HSEGMENT )->Value() + 1 ) ) - 1 );

     //In some recording, EnumPair are not saved, so we put special cases here
    auto *lAccumulationExp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP );
    if( lAccumulationExp->EnumSize() == 0 )
    {
        for( size_t i = 0; i <= 0x0a; ++i ) //0x0a magic number : taken from live sensors, should be the max possible value
        {
            lAccumulationExp->AddEnumPair( i, LeddarUtils::LtStringUtils::IntToString( 1 << i ) );
        }
        lAccumulationExp->SetClean();
    }

    auto *lOverSExp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP );
    if( lOverSExp->EnumSize() == 0 )
    {
        for( size_t i = 0; i <= 5; ++i ) //5 magic number : taken from live sensors, should be the max possible value
        {
            lOverSExp->AddEnumPair( i, LeddarUtils::LtStringUtils::IntToString( 1 << i ) );
        }
        lOverSExp->SetClean();
    }
}

// *****************************************************************************
// Function: LdSensorVu::GetCalib
//
/// \brief   Get the calibration settings on the device.
///
/// \exception LtComException If the device is not connected.
///
/// \author  Patrick Boulay
///
/// \since   June 2016
// *****************************************************************************

void
LdSensorVu::GetCalib()
{
    uint32_t lDistanceScale = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DISTANCE_SCALE )->ValueT<uint32_t>();

    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    /*const uint16_t lBufferSize = */mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    // Timebase delay
    mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mPeakCalibrationOffset ),
                                sizeof( int32_t )* mChannelCount, 5 );

    if( mCalibrationOffsetBackup == nullptr )
    {
        mCalibrationOffsetBackup = new int32_t[ mChannelCount ];
    }

    memcpy( mCalibrationOffsetBackup, lOutputBuffer, mChannelCount * sizeof( int32_t ) );
    LeddarCore::LdFloatProperty *lTimebaseDelayProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_TIMEBASE_DELAY );
    lTimebaseDelayProp->SetScale( lDistanceScale );
    lTimebaseDelayProp->SetCount( mChannelCount );

    for( uint16_t i = 0; i < mChannelCount; ++i )
    {
        lTimebaseDelayProp->SetRawValue( i, mCalibrationOffsetBackup[ i ] );
    }

    lTimebaseDelayProp->SetClean();

    // Compensations
    uint8_t lLedPwrCount = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_LED_USR_PWR_COUNT )->ValueT<uint8_t>();
    mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_ADV_CFG_DATA ) + offsetof( sAdvCfgData, mPeakCalibrationLed ), sizeof( int32_t )* lLedPwrCount, 5 );

    if( mCalibrationLedBackup == nullptr )
    {
        mCalibrationLedBackup = new int32_t[ lLedPwrCount ];
    }

    memcpy( mCalibrationLedBackup, lOutputBuffer, lLedPwrCount * sizeof( int32_t ) );
    LeddarCore::LdFloatProperty *lCompensationProp = GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_INTENSITY_COMPENSATIONS );
    lCompensationProp->SetScale( lDistanceScale );
    lCompensationProp->SetCount( lLedPwrCount );

    for( uint16_t i = 0; i < lLedPwrCount; ++i )
    {
        lCompensationProp->SetRawValue( i, mCalibrationLedBackup[ i ] );
    }

    lCompensationProp->SetClean();

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn bool LdSensorVu::GetEchoes()
///
/// \brief  Get echoes from the sensor and fill the result object.
///
/// \return True if it succeeds, false if it fails.
///
/// \author Patrick Boulay
/// \date   March 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
bool
LdSensorVu::GetEchoes()
{
    // Get echo data storage
    LdResultEchoes *lResultEchoes = GetResultEchoes();
    uint16_t lMaxEchoes = REGMAP_MAX_ECHOES_PER_CHANNEL * mChannelCount;

    // Get comm buffer
    uint8_t *lInputBuffer;
    uint8_t *lOutputBuffer;
    mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    try
    {
        uint32_t lTimeStamp = 0;
        uint16_t lEchoCount = 0;
        uint16_t lCurrentLwdPower = 0;
        uint16_t lEchoCountToRead = 0;
        //Static variables for robustness in a rare case where comm is stuck (only with a FTDI/SPI cable)
        //And we need to reset the transfer mode
        static int sStuckCounter = 0;
        static int sStuckMax = -1;

        // If last trasaction has failed, reset register locking
        // by resetting the partial blocking mode.
        if( mErrorFlag == true )
        {
            uint8_t lMode = 2;
            mConnectionUniversal->WriteRegister( GetBankAddress( REGMAP_TRN_CFG ) + offsetof( sTransactionCfg, mTransferMode ), &lMode, 1, 5 );
            mErrorFlag = false;
        }

        // Get the timestamp and the echoes number
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_CMD_LIST ) + offsetof( sCmdList, mDetectionReady ), sizeof( ( ( sCmdList * )0 )->mDetectionReady ), 1 );

        if( lOutputBuffer[ 0 ] == 1 )
        {
            mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_DETECTIONS ), offsetof( sDetections, mEchoes ), 1 );
            lTimeStamp = *( reinterpret_cast<uint32_t *>( lOutputBuffer + offsetof( sDetections, mTimestamp ) ) );
            lEchoCount = *( reinterpret_cast<uint16_t *>( lOutputBuffer + offsetof( sDetections, mNbDetection ) ) );
            lCurrentLwdPower = *( reinterpret_cast<uint16_t *>( lOutputBuffer + offsetof( sDetections, mCurrentUsrLedPower ) ) );
            lEchoCountToRead = lEchoCount;
            sStuckMax = sStuckCounter;
            sStuckCounter = 0;
        }
        else
        {
            sStuckCounter++;

            if( sStuckMax >= 0 && sStuckCounter > sStuckMax * 10 && sStuckCounter > sStuckMax + 10 )
            {
                mErrorFlag = true;
                sStuckMax = -1;
            }

            return false;
        }

        if( lEchoCount > ( lMaxEchoes ) )
        {
            return false;
        }

        if( lResultEchoes->GetTimestamp( LeddarConnection::B_GET ) != lTimeStamp )
        {
            lResultEchoes->SetTimestamp( lTimeStamp );
            uint32_t lEchoStartAddr = GetBankAddress( REGMAP_DETECTIONS ) + offsetof( sDetections, mEchoes );

            // Get echoes. If the size is over 512 bytes, do multiple read.
            while( lEchoCountToRead > 0 )
            {
                uint16_t lEchoCountToReadNow = lEchoCountToRead;

                if( sizeof( sEchoLigth )* lEchoCountToReadNow > 512 )
                {
                    lEchoCountToReadNow = 512 / sizeof( sEchoLigth );
                    lEchoCountToRead -= lEchoCountToReadNow;
                }
                else
                {
                    lEchoCountToRead -= lEchoCountToReadNow;
                }

                mConnectionUniversal->Read( 0xb, lEchoStartAddr, sizeof( sEchoLigth )* lEchoCountToReadNow, 1, 5000 );
                sEchoLigth *lDetections = reinterpret_cast<sEchoLigth *>( lOutputBuffer );
                std::vector<LdEcho> *lEchoes = lResultEchoes->GetEchoes( LeddarConnection::B_SET );
                auto lAmplitudeScale = GetProperties()->GetIntegerProperty(LeddarCore::LdPropertyIds::ID_FILTERED_AMP_SCALE)->Value();

                for( int i = 0; i < lEchoCountToReadNow; ++i )
                {
                    ( *lEchoes )[ i ].mChannelIndex = lDetections[ i ].mSegment;
                    ( *lEchoes )[ i ].mDistance = lDetections[ i ].mDistance;
                    ( *lEchoes )[ i ].mAmplitude = lDetections[ i ].mAmplitude;
                    ( *lEchoes )[ i ].mFlag = lDetections[ i ].mFlag;
                    ( *lEchoes )[ i ].mBase = 512 * lAmplitudeScale;
                }
            }

            lResultEchoes->SetEchoCount( lEchoCount );
            mEchoes.SetPropertyValue( LeddarCore::LdPropertyIds::ID_CURRENT_LED_INTENSITY, 0, lCurrentLwdPower );
        }
        else
        {
            return false;
        }
    }
    catch( ... )
    {
        mErrorFlag = true;
        throw;
    }

    ComputeCartesianCoordinates();
    lResultEchoes->Swap();
    lResultEchoes->UpdateFinished();
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::GetStates()
///
/// \brief  Get the states from the device.
///
/// \exception  LeddarException::LtException    Thrown when a Lt error condition occurs.
/// \exception  std::runtime_error              If the device is not connected.
///
/// \author Vincent Simard Bilodeau
/// \date   August 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::GetStates()
{
    // Get state data storage
    LdResultStates &aResultStates = *GetResultStates();

    //We cannot get the timestamp on its own, so we copy it from the echoes
    uint32_t lOldTimestamp = aResultStates.GetTimestamp();
    uint32_t lNewTimestamp = mEchoes.GetTimestamp();
    assert( lNewTimestamp ); //We didnt get any echoes yet, so we dont know the timestamp

    if( lOldTimestamp == lNewTimestamp )
    {
        return;
    }

    aResultStates.SetTimestamp( lNewTimestamp );

    if( mErrorFlag == true )
    {
        uint8_t lMode = 2;
        mConnectionUniversal->WriteRegister( GetBankAddress( REGMAP_TRN_CFG ) + offsetof( sTransactionCfg, mTransferMode ), &lMode, 1, 5 );
        mErrorFlag = false;
    }

    try
    {
        // Read state from device
        uint32_t lCpuLoad;
        mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_CMD_LIST ) + offsetof( sCmdList, mCpuUsage ), ( uint8_t * )&lCpuLoad, sizeof( lCpuLoad ), 5 );
        aResultStates.GetProperties()->GetFloatProperty( LeddarCore::LdPropertyIds::ID_RS_CPU_LOAD )->ForceRawValue( 0, lCpuLoad );

        if( mBackupFlagAvailable )
        {
            try
            {
                // Read backup flag
                uint32_t lBackupFlag;
                mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_CMD_LIST ) + offsetof( sCmdList, mBackupStatus ), ( uint8_t * )&lBackupFlag, sizeof( lBackupFlag ), 5 );
                aResultStates.GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_RS_BACKUP )->ForceValue( 0, lBackupFlag );
            }
            catch( ... )
            {
                mBackupFlagAvailable = false;
                aResultStates.GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_RS_BACKUP )->ForceValue( 0, 0 );
                throw LeddarException::LtException( "Error to read the calibration backup flag, please update your sensor firmware." );
            }
        }
    }
    catch( ... )
    {
        mErrorFlag = true;
        throw;
    }

    // Emit state update completed
    aResultStates.UpdateFinished();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn std::vector<LeddarDefines::sLicense> LdSensorVu::GetLicenses( void )
///
/// \brief  Get the licenses on the device.
///
/// \return Vector of permanent licenses on the device.
///
/// \author Patrick Boulay
/// \date   May 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<LeddarDefines::sLicense>
LdSensorVu::GetLicenses( void )
{
    std::vector<LeddarDefines::sLicense> lLicenses;
    LdIntegerProperty *lLicenseInfoProp = GetProperties()->GetIntegerProperty( LdPropertyIds::ID_LICENSE_INFO );
    LdBufferProperty *lLicenseProp = GetProperties()->GetBufferProperty( LdPropertyIds::ID_LICENSE );
    lLicenseInfoProp->SetCount( LICENSE_NUMBER );
    lLicenseProp->SetCount( LICENSE_NUMBER );

    // Read license and info
    uint8_t lLicenseKey[ 3 ][ REGMAP_KEY_LENGTH ];
    uint32_t lLicenseInfo[ 3 ];
    mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_LICENSE_KEYS ), ( uint8_t * )lLicenseKey, REGMAP_KEY_LENGTH * LICENSE_NUMBER, 1 );
    mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_CMD_LIST ) + offsetof( sCmdList, mLicenceInfo ), ( uint8_t * )lLicenseInfo, sizeof( uint32_t )* LICENSE_NUMBER, 1 );

    for( uint8_t i = 0; i < LICENSE_NUMBER; ++i )
    {
        LeddarDefines::sLicense lLicense;
        lLicense.mType = lLicenseInfo[ i ] & 0xFFFF;
        lLicense.mSubType = lLicenseInfo[ i ] >> 16;
        std::reverse( lLicenseKey[i], lLicenseKey[i] + REGMAP_KEY_LENGTH );
        lLicense.mLicense = LeddarUtils::LtStringUtils::ByteArrayToHexString( lLicenseKey[i], REGMAP_KEY_LENGTH );

        //Update the property
        lLicenseInfoProp->ForceValue( i, lLicenseInfo[ i ] );
        lLicenseProp->ForceValue( i, lLicenseKey[i], REGMAP_KEY_LENGTH );

        lLicenses.push_back( lLicense );
    }

    lLicenseInfoProp->SetClean();
    lLicenseProp->SetClean();

    //And update volatile license property
    LdIntegerProperty *lVolLicenseInfoProp = GetProperties()->GetIntegerProperty( LdPropertyIds::ID_VOLATILE_LICENSE_INFO );
    LdBufferProperty *lVolLicenseProp = GetProperties()->GetBufferProperty( LdPropertyIds::ID_VOLATILE_LICENSE );
    lVolLicenseInfoProp->SetCount( 0 );
    lVolLicenseProp->SetCount( 0 );

    LeddarDefines::sLicense lLicense;
    uint32_t lResultLicenseInfo;
    mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_CMD_LIST ) + offsetof( sCmdList, mLicenceInfoVolatile ), ( uint8_t * )&lResultLicenseInfo, sizeof( uint32_t ), 5 );
    lLicense.mType = lResultLicenseInfo & 0xFFFF;
    //lLicense.mSubType = lResultLicenseInfo >> 16; //Not used at the moment

    uint8_t lVolatileLicenseKey[ REGMAP_KEY_LENGTH ];
    mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_VOLATILE_LICENSE_KEYS ), ( uint8_t * )lVolatileLicenseKey, REGMAP_KEY_LENGTH, 1 );

    //Update the property
    if( lLicense.mType < LeddarDefines::LT_COUNT )
    {
        lVolLicenseInfoProp->SetCount( 1 );
        lVolLicenseInfoProp->ForceValue( 0, lResultLicenseInfo );
        lVolLicenseProp->SetCount( 1 );
        lVolLicenseProp->ForceValue( 0, lVolatileLicenseKey, REGMAP_KEY_LENGTH );
    }

    lVolLicenseInfoProp->SetClean();
    lVolLicenseProp->SetClean();

    return lLicenses;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LeddarDefines::sLicense LdSensorVu::SendLicense( const std::string &aLicense )
///
/// \brief  Send license key to the device.
///
/// \exception  std::runtime_error  Raised when a runtime error condition occurs.
///
/// \param  aLicense    License to send.
///
/// \return A LeddarDefines::sLicense.
///
/// \author Patrick Boulay
/// \date   May 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
LeddarDefines::sLicense
LdSensorVu::SendLicense( const std::string &aLicense, bool aVolatile )
{
    // Get licences from device
    if( aLicense.length() != LICENSE_USER_SIZE && aLicense.length() != 0 )
        throw std::runtime_error( "Invalid license length." );

    LeddarDefines::sLicense lResultLicense;
    uint8_t lBuffer[ 16 ];

    // Convert the user string to 16 bytes license
    for( size_t i = 0; i < aLicense.length(); i += 2 )
    {
        lBuffer[ i / 2 ] = ( uint8_t )strtoul( aLicense.substr( i, 2 ).c_str(), nullptr, 16 );
    }

    if( aVolatile )
    {
        mConnectionUniversal->SetWriteEnable( true );

        // Write the license on the device
        try
        {
            mConnectionUniversal->WriteRegister( GetBankAddress( REGMAP_VOLATILE_LICENSE_KEYS ), lBuffer, sizeof( uint8_t )* REGMAP_KEY_LENGTH );
        }
        catch( ... )
        {
            mConnectionUniversal->SetWriteEnable( false );
            throw;
        }

        mConnectionUniversal->SetWriteEnable( false );

        // Read info about the license. This info is needed to know if the license is valid
        lResultLicense.mLicense = aLicense;
        uint32_t lResultLicenseInfo;

        mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_CMD_LIST ) + offsetof( sCmdList, mLicenceInfoVolatile ), ( uint8_t * )&lResultLicenseInfo, sizeof( uint32_t ), 5 );
        lResultLicense.mType = lResultLicenseInfo & 0xFFFF;

        if( lResultLicense.mType == 0 )
        {
            throw std::runtime_error( "Invalid license." );
        }
    }
    else
    {
        std::vector<LeddarDefines::sLicense> lLicenses = GetLicenses();

        // Looking for empty slot
        uint32_t lEmptySlotIndex = 0;

        for( lEmptySlotIndex = 0; lEmptySlotIndex < lLicenses.size(); ++lEmptySlotIndex )
        {
            if( lLicenses[ lEmptySlotIndex ].mType != 0 && lLicenses[ lEmptySlotIndex ].mLicense == LeddarUtils::LtStringUtils::ToLower( aLicense ) )
            {
                throw std::runtime_error( "License already on the device." );
            }

            if( lLicenses[ lEmptySlotIndex ].mType == LeddarDefines::LT_NO || lLicenses[ lEmptySlotIndex ].mType > LeddarDefines::LT_COUNT )
            {
                break;
            }
        }

        if( lEmptySlotIndex == LICENSE_NUMBER )
        {
            throw std::runtime_error( "No empty license slot available on the device." );
        }

        mConnectionUniversal->SetWriteEnable( true );

        // Write the license on the device
        try
        {
            mConnectionUniversal->WriteRegister( GetBankAddress( REGMAP_LICENSE_KEYS ) + ( lEmptySlotIndex * REGMAP_KEY_LENGTH ), lBuffer, sizeof( uint8_t )* REGMAP_KEY_LENGTH );
        }
        catch( ... )
        {
            mConnectionUniversal->SetWriteEnable( false );
            throw;
        }

        mConnectionUniversal->SetWriteEnable( false );


        // Read info about the license. This infor is needed to know if the license is valid
        lResultLicense.mLicense = aLicense;
        uint32_t lResultLicenseInfo;

        mConnectionUniversal->ReadRegister( GetBankAddress( REGMAP_CMD_LIST ) + offsetof( sCmdList, mLicenceInfo ) + ( lEmptySlotIndex * sizeof( uint32_t ) ),
                                            ( uint8_t * )&lResultLicenseInfo,
                                            sizeof( uint32_t ), 5 );
        lResultLicense.mType = lResultLicenseInfo & 0xFFFF;
        lResultLicense.mSubType = lResultLicenseInfo >> 16;

        if( lResultLicense.mType == 0 )
        {
            throw std::runtime_error( "Invalid license." );
        }
    }

    return lResultLicense;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::RemoveLicense( const std::string &aLicense )
///
/// \brief  Remove license on the device
///
/// \param  aLicense    License to remove.
///
/// \author Patrick Boulay
/// \date   August 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::RemoveLicense( const std::string &aLicense )
{
    // Get licience from device
    std::vector<LeddarDefines::sLicense> lLicenses = GetLicenses();

    // Looking for license slot
    int8_t lSlotIndex = -1;

    for( uint8_t i = 0; i < lLicenses.size(); ++i )
    {
        if( lLicenses[ i ].mLicense == LeddarUtils::LtStringUtils::ToLower( aLicense ) )
        {
            lSlotIndex = i;
            break;
        }
    }

    // Write the empty license on the device
    if( lSlotIndex != -1 )
    {
        mConnectionUniversal->SetWriteEnable( true );

        try
        {
            uint8_t lEmptyLicense[ 16 ] = { 0 };
            mConnectionUniversal->WriteRegister( GetBankAddress( REGMAP_LICENSE_KEYS ) + ( lSlotIndex * REGMAP_KEY_LENGTH ), lEmptyLicense, sizeof( uint8_t )* REGMAP_KEY_LENGTH );
        }
        catch( ... )
        {
            mConnectionUniversal->SetWriteEnable( false );
            throw;
        }

        mConnectionUniversal->SetWriteEnable( false );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::RemoveAllLicenses( void )
///
/// \brief  Remove all licenses on the device
///
/// \author Patrick Boulay
/// \date   March 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::RemoveAllLicenses( void )
{
    mConnectionUniversal->SetWriteEnable( true );

    try
    {
        uint8_t lEmptyLicense[ 16 ] = { 0 };

        for( uint8_t i = 0; i < LICENSE_NUMBER; ++i )
        {
            mConnectionUniversal->WriteRegister( GetBankAddress( REGMAP_LICENSE_KEYS ) + ( i * REGMAP_KEY_LENGTH ), lEmptyLicense, sizeof( uint8_t )* REGMAP_KEY_LENGTH );
        }
    }
    catch( ... )
    {
        mConnectionUniversal->SetWriteEnable( false );
        throw;
    }

    mConnectionUniversal->SetWriteEnable( false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::ResetToDefaultWithoutWriteEnable( int16_t aCRCTry )
///
/// \brief  Reset parameters to default by NOT handling the write enable
///
/// \exception  std::runtime_error  Raised when a runtime error condition occurs.
///
/// \param  aCRCTry The CRC try.
///
/// \author Patrick Boulay
/// \date   April 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::ResetToDefaultWithoutWriteEnable( int16_t aCRCTry )
{
    try
    {
        // Get comm internal buffer.
        uint8_t *lInputBuffer, *lOutputBuffer;
        mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

        if( !mConnectionUniversal->IsWriteEnable() )
        {
            throw std::runtime_error( "Error to erease chip (write enable)." );
        }

        mConnectionUniversal->Write( REGMAP_CE, 0, 0, aCRCTry, 0, 0, 5000 );
        mConnectionUniversal->IsDeviceReady( 4000 );
    }
    catch( ... )
    {
        throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::ResetToDefault()
///
/// \brief  Reset parameters to default by handling the write enable
///
/// \author Patrick Boulay
/// \date   April 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::ResetToDefault()
{
    mConnectionUniversal->SetWriteEnable( true );
    LeddarUtils::LtTimeUtils::Wait( 10 );
    ResetToDefaultWithoutWriteEnable( 5 );
    Reset( LeddarDefines::RT_SOFT_RESET );
    LeddarUtils::LtTimeUtils::Wait( 10 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions )
///
/// \brief  Reset the device
///
/// \param  aType       Reset type.
/// \param  parameter2  The second parameter.
///
/// \author Patrick Boulay
/// \date   April 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::Reset( LeddarDefines::eResetType aType, LeddarDefines::eResetOptions, uint32_t )
{
    mConnectionUniversal->Reset( aType, 0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn uint32_t LdSensorVu::GetBankAddress( uint8_t aBankType )
///
/// \brief  Get bank address from the register map
///
/// \param  aBankType   Bank type.
///
/// \return The bank address.
///
/// \author Patrick Boulay
/// \date   October 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t
LdSensorVu::GetBankAddress( uint8_t aBankType )
{
    static const sRegMap gRegMap[ REGMAP_NBBANK ] = REGMAP( REGMAP_PRIMARY_KEY_PUBLIC, REGMAP_PRIMARY_KEY_TRACE,
            REGMAP_PRIMARY_KEY_INTEGRATOR, REGMAP_PRIMARY_KEY_ADMIN,
            REGMAP_PRIMARY_KEY_NO );

    return gRegMap[ aBankType ].mStartAddr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::SetTransferMode( eTransfertMode aMode )
///
/// \brief  Set the transfer mode
///
/// \param  aMode   Transfer mode.
///
/// \author Patrick Boulay
/// \date   June 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::SetTransferMode( eTransfertMode aMode )
{
    mConnectionUniversal->WriteRegister( GetBankAddress( REGMAP_TRN_CFG ) + offsetof( sTransactionCfg, mTransferMode ), ( uint8_t * )&aMode, 1, 5 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::CreateBackup( void )
///
/// \brief  Backup the calibration settings. Integrator license is required for this command.
///
/// \author Patrick Boulay
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::CreateBackup( void )
{
    mConnectionUniversal->SetWriteEnable( true );
    LeddarUtils::LtTimeUtils::Wait( 10 );

    try
    {
        uint16_t lCRCTry = 5;
        uint32_t lDataSize = 0;
        mConnectionUniversal->Write( 0x57, 0, lDataSize, lCRCTry, 0, 0, 5000 );
        mConnectionUniversal->IsDeviceReady( 4000 );
    }
    catch( ... )
    {
        mConnectionUniversal->SetWriteEnable( false );
        throw;
    }

    mConnectionUniversal->SetWriteEnable( false );
    LeddarUtils::LtTimeUtils::Wait( 1000 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LdSensorVu::DeleteBackup( void )
///
/// \brief  Delete the calibation backup. Integrator license is required for this command.
///
/// \author Patrick Boulay
/// \date   January 2018
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::DeleteBackup( void )
{
    mConnectionUniversal->SetWriteEnable( true );

    try
    {
        uint16_t lCRCTry = 5;
        uint32_t lDataSize = 0;

        mConnectionUniversal->Write( 0x5E, 0, lDataSize, lCRCTry, 0, 0, 5000 );
        mConnectionUniversal->IsDeviceReady( 4000 );
    }
    catch( ... )
    {
        mConnectionUniversal->SetWriteEnable( false );
        throw;
    }

    mConnectionUniversal->SetWriteEnable( false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorVu::UpdateFirmware( const std::string &aFileName, LeddarCore::LdIntegerProperty *aProcessPercentage, LeddarCore::LdBoolProperty *aCancel )
///
/// \brief  Updates the firmware/fpga/driver using the provided ltb file - Override default function to handle fpga data, otherwise, same behaviour
///
/// \exception  std::logic_error    Raised when a logic error condition occurs.
///
/// \param          aFileName           Path to the ltb file.
/// \param [in,out] aProcessPercentage  If non-null, the process percentage.
/// \param [in,out] aCancel             If non-null, the cancel.
///
/// \author David Lévy
/// \date   January 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorVu::UpdateFirmware( const std::string &aFileName, LeddarCore::LdIntegerProperty *aProcessPercentage, LeddarCore::LdBoolProperty *aCancel )
{
    LeddarUtils::LtFileUtils::LtLtbReader lLtbReader( aFileName );

    if( lLtbReader.GetDeviceType() != GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_DEVICE_TYPE )->Value() )
    {
        throw std::logic_error( "Provided file is not for this device" );
    }

    const std::list<std::pair<uint32_t, std::vector<uint8_t>>> lFirmwares = lLtbReader.GetFirmwares();

    auto lDSP      = std::find_if( lFirmwares.cbegin(), lFirmwares.cend(),
                              []( const std::pair<uint32_t, std::vector<uint8_t>> &a ) { return a.first == LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_GALAXY_BINARY; } );
    auto lFPGAAlgo = std::find_if( lFirmwares.cbegin(), lFirmwares.cend(),
                                   []( const std::pair<uint32_t, std::vector<uint8_t>> &a ) { return a.first == LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_FPGA_ALGO; } );
    auto lFPGAData = std::find_if( lFirmwares.cbegin(), lFirmwares.cend(),
                                   []( const std::pair<uint32_t, std::vector<uint8_t>> &a ) { return a.first == LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_FPGA_DATA; } );
    auto lFPGAEraseAlgo = std::find_if( lFirmwares.cbegin(), lFirmwares.cend(),
                                   []( const std::pair<uint32_t, std::vector<uint8_t>> &a ) { return a.first == LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_FPGA_ERASE_ALGO; } );
    auto lFPGAEraseData = std::find_if( lFirmwares.cbegin(), lFirmwares.cend(),
                                   []( const std::pair<uint32_t, std::vector<uint8_t>> &a ) { return a.first == LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_FPGA_ERASE_DATA; } );
    auto lAsicHexData = std::find_if( lFirmwares.cbegin(), lFirmwares.cend(),
                                   []( const std::pair<uint32_t, std::vector<uint8_t>> &a ) { return a.first == LeddarUtils::LtFileUtils::LtLtbReader::ID_LTB_ASIC_HEX; } );
    if( lDSP != lFirmwares.cend() )
    {
        UpdateFirmware( FT_DSP, LdFirmwareData( ( *lDSP ).second ), aProcessPercentage, aCancel );
    }
    else if( lAsicHexData != lFirmwares.cend() )
    {
        UpdateFirmware( FT_ASIC, LdFirmwareData( ( *lAsicHexData ).second ), aProcessPercentage, aCancel );
    }
    else if( lFPGAAlgo != lFirmwares.cend() && lFPGAData != lFirmwares.cend() && lFPGAEraseAlgo != lFirmwares.cend() && lFPGAEraseData != lFirmwares.cend() )
    {
        UpdateFirmware( FT_FPGA, LdFirmwareData( ( *lFPGAEraseData ).second, ( *lFPGAEraseAlgo ).second ), aProcessPercentage, aCancel );
        UpdateFirmware( FT_FPGA, LdFirmwareData( ( *lFPGAData ).second, ( *lFPGAAlgo ).second ), aProcessPercentage, aCancel );
    }
    else
    {
        throw std::logic_error( "No data in ltb file" );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorVu::UpdateFirmware( eFirmwareType aFirmwareType, const LdFirmwareData &aFirmwareData, LeddarCore::LdIntegerProperty *aProcessPercentage, LeddarCore::LdBoolProperty *aCancel )
///
/// \brief  Updates the firmware
///
/// \param          aFirmwareType       Firmware type to send. (see \ref LeddarDevice::LdSensor::eFirmwareType)
/// \param          aFirmwareData       Firmware data.
/// \param [in,out] aProcessPercentage  If non-null, Pourcentage of completion set by this function.
/// \param [in,out] aCancel             If non-null, Set to true to cancel the operation.
///
/// \author David Lévy
/// \date   January 2021
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorVu::UpdateFirmware( eFirmwareType aFirmwareType, const LdFirmwareData &aFirmwareData, LeddarCore::LdIntegerProperty *aProcessPercentage,
                                                LeddarCore::LdBoolProperty *aCancel )
{
    switch( aFirmwareType )
    {
    case LeddarDevice::LdSensor::FT_DSP:
        UpdateDSP( &aFirmwareData.mFirmwareData[0], static_cast<uint32_t>( aFirmwareData.mFirmwareData.size() ), aCancel, aProcessPercentage, nullptr );
        break;

    case LeddarDevice::LdSensor::FT_FPGA:
        if( aFirmwareData.mAlgoData.size() == 0 )
        {
            throw std::logic_error( "Missing firmware data" );
        }

        UpdateFPGA( &aFirmwareData.mAlgoData[0], static_cast<uint32_t>( aFirmwareData.mAlgoData.size() ), &aFirmwareData.mFirmwareData[0],
                    static_cast<uint32_t>( aFirmwareData.mFirmwareData.size() ), aCancel, true, aProcessPercentage, nullptr );
        break;

    case LeddarDevice::LdSensor::FT_ASIC:
    {
        IntelHEX::IntelHexMem *lHexMem =
            LeddarUtils::LtFileUtils::LoadHexFromBuffer( &aFirmwareData.mFirmwareData[0], static_cast<uint32_t>( aFirmwareData.mFirmwareData.size() ) );

        try
        {
            UpdateAsic( *lHexMem, false, aProcessPercentage );
        }
        catch( ... )
        {
            delete lHexMem;
            throw;
        }

        delete lHexMem;
        break;
    }

    default:
        throw std::logic_error( "Invalid firmware type" );
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn void LeddarDevice::LdSensorVu::UpdateDSP( const uint8_t *aData, const uint32_t &aDataSize, LeddarCore::LdBoolProperty *aCancel, LeddarCore::LdIntegerProperty *aProcessPercentage, LeddarCore::LdIntegerProperty *aState )
///
/// \brief  Updates the DSP
///
/// \param          aData               The firmware data.
/// \param          aDataSize           Size of the data.
/// \param [in,out] aCancel             Cancel update
/// \param [in,out] aProcessPercentage  Progression %
/// \param [in,out] aState              Current state
///
/// \author Patrick Boulay
/// \date   April 2016
////////////////////////////////////////////////////////////////////////////////////////////////////
void LeddarDevice::LdSensorVu::UpdateDSP( const uint8_t *aData, const uint32_t &aDataSize, LeddarCore::LdBoolProperty *aCancel, LeddarCore::LdIntegerProperty *aProcessPercentage,
                                          LeddarCore::LdIntegerProperty *aState )
{
    uint32_t lMinimumBufferSize = 100 * 1024;
    uint32_t lBufferSizeTemp    = mConnectionUniversal->GetInternalBuffersSize();

    if( lBufferSizeTemp < lMinimumBufferSize )
    {
        mConnectionUniversal->ResizeInternalBuffers( 100 * 1024 );
    }

    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );
    const uint32_t lBufferSize = 1024 * 4;
    uint32_t lSizeToSend       = ( aDataSize > lBufferSize ) ? lBufferSize : aDataSize;
    uint32_t lRamAddr          = LeddarDefines::LdSensorVuDefines::RAM_UPDATE_LOGICAL_ADDR;
    uint32_t lFlashAddr        = LeddarDefines::LdSensorVuDefines::MAIN_APP_BASE_ADDR;
    uint16_t lTotalOperation   = ( aDataSize / lBufferSize ) + ( ( aDataSize % lBufferSize ) > 0 ? 1 : 0 );
    uint8_t lAppStatus;

    try
    {
        uint16_t lAppCrc16 = CRCUTILS_CRC16_INIT_VALUE;
        uint16_t lRamCrc16 = CRCUTILS_CRC16_INIT_VALUE;

        // Hard resert
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FIUP_JUMP_IN_BOOTLOADER );

        mConnectionUniversal->Reset( LeddarDefines::RT_HARD_RESET, true );

        if( aCancel != nullptr && aCancel->Value( 0 ) )
            throw 0;

        // Set bootloader for writing
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FIUP_ENABLE_WRITE );

        uint32_t lUniqueId[4];
        GetUniqueId( lUniqueId );
        mConnectionUniversal->SetWriteEnable( true );

        // Chip erage
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FIUP_CHIP_ERASE );

        ResetToDefaultWithoutWriteEnable();

        if( aCancel != nullptr && aCancel->Value( 0 ) )
            throw 0;

        // Open firmware update session.
        OpenFirmwareUpdateSession();

        // Copy data
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FIUP_COPY_DATA );

        uint16_t lPourcentageCount = 0;
        uint16_t lTryCount         = NUMBER_OF_RETRY;

        if( aProcessPercentage != 0 )
            aProcessPercentage->ForceValue( 0, 0 );

        uint32_t lSizeSent = 0;

        while( lSizeSent != aDataSize )
        {
            // Evaluate the cumulative CRC16 of this block (one first try only)
            if( lTryCount == NUMBER_OF_RETRY )
            {
                lRamCrc16 = LeddarUtils::LtCRCUtils::Crc16( CRCUTILS_CRC16_INIT_VALUE, aData + lSizeSent, lSizeToSend );
                lAppCrc16 = LeddarUtils::LtCRCUtils::Crc16( lAppCrc16, aData + lSizeSent, lSizeToSend );
            }

            // Send data block in ram
            memcpy( lInputBuffer, &( aData[lSizeSent] ), lSizeToSend );
            mConnectionUniversal->Write( 0x2, lRamAddr, lSizeToSend );

            // Write block in flash and wait until the device is ready.
            StartFirmwareUpdateProcess( lFlashAddr, lSizeToSend, lRamCrc16 );

            // LeddarUtils::LtTimeUtils::Wait(10);
            mConnectionUniversal->IsDeviceReady( 10000 );

            // Get firmware update status
            GetFirmwareUpdateStatus( &lAppStatus );

            switch( lAppStatus )
            {
            case LeddarDefines::LdSensorVuDefines::BL_APP_UPDATE_SUCCESS:
                lTryCount = NUMBER_OF_RETRY;
                lFlashAddr += lSizeToSend;
                lSizeSent += lSizeToSend;
                lSizeToSend = ( ( aDataSize - lSizeSent ) > lBufferSize ) ? lBufferSize : ( aDataSize - lSizeSent );

                if( aProcessPercentage != 0 )
                    aProcessPercentage->ForceValue( 0, ( uint32_t )( ( (float)( ++lPourcentageCount ) / (float)lTotalOperation ) * 100. ) );

                break;

            case LeddarDefines::LdSensorVuDefines::BL_APP_UPDATE_ERROR:
                throw std::runtime_error( "RAM block to Flash writting error.\r\n" );

            case LeddarDefines::LdSensorVuDefines::BL_APP_UPDATE_CRC_ERROR:
                if( --lTryCount == 0 )
                    throw std::runtime_error( "Verify error on firmware update." );

                break;

            case LeddarDefines::LdSensorVuDefines::BL_APP_UPDATE_ERR_OVERSIZE:
                throw std::runtime_error( "Oversize to RAM available dimension." );

            default:
                throw std::runtime_error( "Unkown update status." );
            }

            if( aCancel != nullptr && aCancel->Value( 0 ) )
                throw 0;
        }

        if( aProcessPercentage != 0 )
            aProcessPercentage->ForceValue( 0, 100 );

        // Close firmware update session
        CloseFirmwareUpdateSession( &lAppStatus );

        if( lAppStatus != LeddarDefines::LdSensorVuDefines::BL_APP_UPDATE_STATUS_NONE )
        {
            throw std::runtime_error( "Close firmware update failed." );
        }

        // Disable write enable.
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FIUP_DISABLE_WRITE );

        mConnectionUniversal->SetWriteEnable( false );

        // Check data integrity
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FIUP_CHECK_DATA_INTEGRITY );

        uint16_t lCRC16 = GetAppCrc16( aDataSize );

        if( lAppCrc16 != lCRC16 )
            throw std::runtime_error( "Bad CRC error on firmware update, CRC: " + LeddarUtils::LtStringUtils::IntToString( lCRC16 ) );

        // Jump in firmware
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FIUP_SOFTWARE_RESET );

        Reset( LeddarDefines::RT_SOFT_RESET );
    }
    catch( std::exception & )
    {
        mConnectionUniversal->SetWriteEnable( false );
        throw;
    }
    catch( ... )
    {
    }

    mConnectionUniversal->SetWriteEnable( false );
}

// *****************************************************************************
// Function: LdSensorVu::GetUniqueId
//
/// \brief   Get unique ID of the device.
///
/// \param  aUniqueId   Unique ID of the device (array for 4 bytes).
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************
void LdSensorVu::GetUniqueId( uint32_t *aUniqueId )
{
    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    uint32_t lBuffer = 1;
    memcpy( lInputBuffer, &lBuffer, sizeof( lBuffer ) );
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, sizeof( uint32_t ) );
    mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, sizeof( uint32_t ) * 4 );
    memcpy( aUniqueId, ( ( uint8_t * )lOutputBuffer ), sizeof( uint32_t ) * 4 );
}

// ****************************************************************************
/// Function:   LdSensorVu::OpenFirmwareUpdateSession
///
/// \brief      Open an application firmware update session: used for RAM block
///             update method.
///
/// \pre        MCU in bootloader environment.
///
/// \author     Frédéric Parent
///
/// \since      June 8, 2016
// ****************************************************************************

void
LdSensorVu::OpenFirmwareUpdateSession()
{
    uint8_t lStatus;
    uint32_t lData = 7;

    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS,
                                 ( uint8_t * )&lData, sizeof( lData ) );
    mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, &lStatus, 1 );

    switch( lStatus )
    {
        case LeddarDefines::LdSensorVuDefines::BL_APP_UPDATE_STATUS_NONE:
            break;

        case LeddarDefines::LdSensorVuDefines::BL_APP_UPDATE_ERR_OUT_OF_MEMORY:
            throw std::runtime_error( "Out of memory for RAM block." );

        case LeddarDefines::LdSensorVuDefines::BL_APP_UPDATE_SESSION_OPENNED:
            throw std::runtime_error( "Update session is already opened." );

        case LeddarDefines::LdSensorVuDefines::BL_APP_OTHER_UPDATE_SESSION_OPENNED:
            throw std::runtime_error( "Update session is already opened." );

        default:
            throw std::runtime_error( "Start firmware update failed with unknown error." );
    }

}

// ****************************************************************************
/// Function:   LdSensorVu::StartFirmwareUpdateProcess

/// \brief      Start the application firmware update process: this write data
///             from RAM block into Flash memory. When this function is successful
///             called, application must call \ref LdConnectionUniversal::IsDeviceReady function
///             to poll the availability of the Vu device.
///
/// \pre        MCU in bootloader environment.
///
/// \param[in] addr     Address of data to write to flash
/// \param[in] dataSize Size of the data to write
/// \param[in] crc      CRC of the data
///
/// \author     Frédéric Parent
///
/// \since      June 8, 2016
// ****************************************************************************
void
LdSensorVu::StartFirmwareUpdateProcess( uint32_t addr, uint32_t dataSize, uint16_t crc )
{
    uint32_t data[4];
    data[0] = 8;
    data[1] = addr;
    data[2] = dataSize;
    data[3] = crc;
    // When send this following command, Vu device can still in "erasing/programming"
    // mode in the status register: WriteDataBlock must not being in wait ready state.
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS,
                                 ( uint8_t * )&data, sizeof( data ) );
}
// ****************************************************************************
/// Function:   LdSensorVu::GetFirmwareUpdateStatus
/// \brief      Get the application firmware update status. Before call this
///             function, ensure that device is ready by \ref LdConnectionUniversal::IsDeviceReady
///             function. This function is only valid on update by RAM block
///             method.
/// \pre        MCU in bootloader environment.
/// \param[out] *aStatus     Application firmware update returned status.
/// \author     Frédéric Parent
/// \since      June 8, 2016
// ****************************************************************************
void
LdSensorVu::GetFirmwareUpdateStatus( uint8_t *aStatus )
{
    uint32_t lData = 9;
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS,
                                 ( uint8_t * )&lData, sizeof( lData ) );
    mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, aStatus, 1 );

}

// ****************************************************************************
/// Function:   LdSensorVu::CloseFirmwareUpdateSession
/// \brief      Close application firmware update session using RAM block method.
/// \pre        MCU in bootloader environment.
/// \param[out] *aStatus     Return status.
/// \author     Frédéric Parent
/// \since      June 8, 2016
// ****************************************************************************
void
LdSensorVu::CloseFirmwareUpdateSession( uint8_t *aStatus )
{
    uint32_t lData = 10;
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS,
                                 ( uint8_t * )&lData, sizeof( lData ) );
    mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, aStatus, 1 );

}

// *****************************************************************************
// Function: LdSensorVu::GetAppCrc16
//
/// \brief   Get firmware crc
///
/// \param   aSize Size of data to calculate crc.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

uint16_t
LdSensorVu::GetAppCrc16( uint32_t aSize )
{
    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    /*const uint16_t lBufferSize = */mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    uint32_t lArg[] = { 0, aSize, 0, 0 };
    uint16_t lCrc;
    memcpy( lInputBuffer, lArg, sizeof( lArg ) );
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, sizeof( lArg ) );
    // Let the sensor calculate the CRC with a big safe delay
    LeddarUtils::LtTimeUtils::Wait( 100 );
    mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS,  sizeof( lCrc ) );
    memcpy( &lCrc, &lOutputBuffer[0], sizeof( lCrc ) );

    return lCrc;
}

// *****************************************************************************
// Function: LdSensorVu::UpdateFPGA
//
/// \brief   Update the FPGA of the device.
///
/// \param  aAlgo               Array contianing the FPGA algo.
/// \param  aAlgoSize           Array algo size.
/// \param  aData               Array contianing the FPGA data.
/// \param  aDataSize           Array data size.
/// \param  aCancel             Cancel update (can be modified by other thread)
/// \param  aVerify             Verify the the transfert.
/// \param  aProcessPercentage  Property containing the execution pourcentage.
/// \param  aState              Pointer to the state of the FPGA
///
/// \exception std::runtime_error If the verify fail.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************
void LdSensorVu::UpdateFPGA( const uint8_t *aAlgo,
                           const uint32_t &aAlgoSize,
                           const uint8_t *aData,
                           const uint32_t &aDataSize,
                           LeddarCore::LdBoolProperty  *aCancel,
                           bool aVerify,
                           LeddarCore::LdIntegerProperty *aProcessPercentage,
                           LeddarCore::LdIntegerProperty *aState )
{

    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    const uint16_t lBufferSize = mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    uint16_t lTotalOperation = ( aAlgoSize / lBufferSize )
                               + ( ( aAlgoSize  %  lBufferSize ) > 0 ? 1 : 0 )
                               + ( aDataSize / lBufferSize )
                               + ( ( aDataSize  %  lBufferSize ) > 0 ? 1 : 0 );

    try
    {

        uint16_t lFpgaCrc = LeddarUtils::LtCRCUtils::Crc16( CRCUTILS_CRC16_INIT_VALUE, aAlgo, aAlgoSize );
        lFpgaCrc = LeddarUtils::LtCRCUtils::Crc16( lFpgaCrc, aData, aDataSize );

        // Hard reset
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FPUP_JUMP_IN_BOOTLOADER );

        mConnectionUniversal->Reset( LeddarDefines::RT_HARD_RESET, true );

        if( aCancel != nullptr && aCancel->Value( 0 ) )
            throw 0;

        // Open FPGA update session
        uint32_t lUniqueId[4];
        GetUniqueId( lUniqueId );
        uint16_t lMagicNumber = LeddarUtils::LtCRCUtils::Crc16( CRCUTILS_CRC16_INIT_VALUE, lUniqueId, sizeof( lUniqueId ) );
        OpenFPGAUpdateSession( aAlgoSize, aDataSize, lFpgaCrc );

        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FPUP_ENABLE_WRITE );

        if( aCancel != nullptr && aCancel->Value( 0 ) )
            throw 0;

        // Send algo file
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FPUP_COPY_DATA );

        uint32_t lDstAddr = LeddarDefines::LdSensorVuDefines::RAM_UPDATE_LOGICAL_ADDR;
        uint32_t lSizeSent = 0;
        uint32_t lSizeToSend = ( aAlgoSize > lBufferSize ) ? lBufferSize : aAlgoSize;
        uint16_t lPourcentageCount = 0;
        int16_t lVerifyTry = 4;



        if( aProcessPercentage != 0 )
            aProcessPercentage->ForceValue( 0, 100 );

        while( lSizeSent != aAlgoSize )
        {
            // Send data block to bootloader
            memcpy( lInputBuffer, &( aAlgo[lSizeSent] ), lSizeToSend );
            mConnectionUniversal->Write( 0x2, lDstAddr, lSizeToSend );

            if( aVerify )
            {
                // Unlock bootloader to read the next data block
                UnlockBootloader( lMagicNumber );

                try
                {
                    mConnectionUniversal->Read( 0xb, lDstAddr, lSizeToSend );
                }
                catch( std::exception & )
                {
                    if( lVerifyTry-- > 0 )
                    {
                        continue;
                    }

                    throw;
                }

                lVerifyTry = 4;

                for( int i = 0; i < ( int )lSizeToSend; i++ )
                {
                    if( lOutputBuffer[i] != aAlgo[lSizeSent + i] )
                    {
                        if( --lVerifyTry > 0 )
                        {
                            std::cout << "Verify error on algo, retry to write the block" << std::endl;
                            continue;
                        }
                        else
                        {
                            throw std::runtime_error( "Verify error on algo update." );
                        }
                    }
                }
            }

            lDstAddr += lSizeToSend;
            lSizeSent += lSizeToSend;
            lSizeToSend = ( ( aAlgoSize - lSizeSent ) > lBufferSize ) ? lBufferSize : ( aAlgoSize - lSizeSent );

            if( aProcessPercentage != 0 )
                aProcessPercentage->ForceValue( 0, ( uint32_t )( ( ( float )( ++lPourcentageCount ) / ( float )lTotalOperation ) * 100. ) );

            if( aCancel != nullptr && aCancel->Value( 0 ) )
                throw 0;

            lVerifyTry = 4;
        }

        // Send data file
        lDstAddr = LeddarDefines::LdSensorVuDefines::RAM_UPDATE_LOGICAL_ADDR + aAlgoSize;
        lSizeSent = 0;
        lSizeToSend = ( aDataSize > lBufferSize ) ? lBufferSize : aDataSize;
        lVerifyTry = 4;

        while( lSizeSent != aDataSize )
        {
            // Send data block to bootloader
            memcpy( lInputBuffer, &( aData[lSizeSent] ), lSizeToSend );
            mConnectionUniversal->Write( 0x2, lDstAddr, lSizeToSend );

            if( aVerify )
            {
                // Unlock bootloader to read the next data block
                UnlockBootloader( lMagicNumber );

                // Read a data block from bootloader to compare
                try
                {
                    mConnectionUniversal->Read( 0xb, lDstAddr, lSizeToSend );
                }
                catch( std::exception & )
                {
                    if( lVerifyTry-- > 0 )
                    {
                        continue;
                    }

                    throw;
                }

                lVerifyTry = 4;

                for( int i = 0; i < ( int )lSizeToSend; i++ )
                {
                    if( lOutputBuffer[i] != aData[lSizeSent + i] )
                    {
                        if( --lVerifyTry > 0 )
                        {
                            std::cout << "Verify error on data, retry to write the block" << std::endl;
                            continue;
                        }
                        else
                        {
                            throw std::runtime_error( "Verify error on data update." );
                        }
                    }
                }
            }

            lDstAddr += lSizeToSend;
            lSizeSent += lSizeToSend;
            lSizeToSend = ( ( aDataSize - lSizeSent ) > lBufferSize ) ? lBufferSize : ( aDataSize - lSizeSent );

            if( aProcessPercentage != 0 )
                aProcessPercentage->ForceValue( 0, ( uint32_t )( ( ( float )( 3 + ++lPourcentageCount ) / ( float )lTotalOperation ) * 100. ) );

            if( aCancel != nullptr && aCancel->Value( 0 ) )
                throw 0;

            LeddarUtils::LtTimeUtils::Wait( 10 );
            lVerifyTry = 4;
        }

        if( aProcessPercentage != 0 )
            aProcessPercentage->ForceValue( 0, 100 );



        // Trigger fpga update
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FPUP_UPDATE_FPGA );

        StartFpgaUpdateProcess();

        if( !mConnectionUniversal->IsDeviceReady( 60000 ) )
        {
            throw LeddarException::LtComException( "Device not ready" );
        }


        GetFpgaUpdateStatus();

        if( aCancel != nullptr && aCancel->Value( 0 ) )
            throw 0;

        // Close flag update session
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FPUP_DISABLE_WRITE );

        CloseFPGAUpdateSession();

        // Hard reset
        if( aState != nullptr )
            aState->ForceValue( 0, LeddarDefines::LdSensorVuDefines::FPUP_RESET );

        std::cout << "Resetting the sensor..." << std::endl;
        mConnectionUniversal->Reset( LeddarDefines::RT_HARD_RESET, false );


    }
    catch( std::exception & )
    {
        throw;
    }
    catch( ... )
    {
    }
}

// *****************************************************************************
// Function: LdSensorVu::OpenFPGAUpdateSession
//
/// \brief   Open FPGA update session
///
/// \param  aAlgoSize   Array algo size.
/// \param  aDataSize   Array data size.
/// \param  aFpgaCrc    CRC.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************
void LdSensorVu::OpenFPGAUpdateSession( uint32_t aAlgoSize, uint32_t aDataSize, uint16_t aFpgaCrc )
{

    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    /*const uint16_t lBufferSize = */mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    uint32_t lArg[] = { 4, 1, aAlgoSize, aDataSize, aFpgaCrc };
    memcpy( lInputBuffer, lArg, sizeof( lArg ) );
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, sizeof( lArg ) );
    mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, 1 );
    uint8_t &lStatus = *lOutputBuffer;

    switch( lStatus )
    {
        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_STATUS_NONE:
            break;

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_TYPE_UNSUPPORTED:
            throw std::runtime_error( "FPGA update type is unsupported." );

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_SESSION_OPENNED:
            throw std::runtime_error( "Update session is already opened." );

        default:
            throw std::runtime_error( "Incorrect status: " + LeddarUtils::LtStringUtils::IntToString( lStatus ) );
    }
}

// *****************************************************************************
// Function: LdSensorVu::CloseFPGAUpdateSession
//
/// \brief   Close FPGA update session
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

void
LdSensorVu::CloseFPGAUpdateSession( void )
{

    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    /*const uint16_t lBufferSize = */mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    uint32_t lArg[] = { 4, 0, 0, 0, 0 };
    memcpy( lInputBuffer, lArg, sizeof( lArg ) );
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS,  sizeof( lArg ) );
    mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS,  1 );
    uint8_t &lStatus = lOutputBuffer[0];

    if( lStatus != LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_STATUS_NONE )
    {
        throw std::runtime_error( "FPGA update status error: " + LeddarUtils::LtStringUtils::IntToString( lStatus ) );
    }

}



// *****************************************************************************
// Function: LdSensorVu::UnlockBootloader
//
/// \brief   Unlock the bootloader
///
/// \param  aMagicNumber   Magic number
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

void
LdSensorVu::UnlockBootloader( uint32_t aMagicNumber )
{
    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    /*const uint16_t lBufferSize = */mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    uint32_t lArg[] = { 2, aMagicNumber };
    memcpy( lInputBuffer, lArg, sizeof( lArg ) );
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, sizeof( lArg ) );
}

// *****************************************************************************
// Function: LdSensorVu::StartFpgaUpdateProcess
//
/// \brief   Start the FPGA update process.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

void
LdSensorVu::StartFpgaUpdateProcess( void )
{
    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    /*const uint16_t lBufferSize = */mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    uint32_t lBuffer = 5;
    memcpy( lInputBuffer, &lBuffer, sizeof( lBuffer ) );
    // When send this following command, Vu device can still in "erasing/programming"
    // mode in the status register: WriteDataBlock must not being in wait ready state.
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, sizeof( uint32_t ), false, -1 );
}

// *****************************************************************************
// Function: LdSensorVu::GetFpgaUpdateStatus
//
/// \brief   Get FPGA update status.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************

void
LdSensorVu::GetFpgaUpdateStatus( void )
{
    // Get comm internal buffer
    uint8_t *lInputBuffer, *lOutputBuffer;
    /*const uint16_t lBufferSize = */mConnectionUniversal->InternalBuffers( lInputBuffer, lOutputBuffer );

    uint32_t lBuffer = 6;
    memcpy( lInputBuffer, &lBuffer, sizeof( lBuffer ) );
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS, sizeof( uint32_t ) );
    mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::SPECIAL_BOOT_COMMANDS,  1 );
    uint8_t lStatus = *lOutputBuffer;

    switch( lStatus )
    {
        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_SUCCESS:
            return;

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_CRC_ERROR:
            throw std::runtime_error( "Bad CRC-16." );

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_ERR_VERIFY_FAIL:
            throw std::runtime_error( "Verify failed." );

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_ERR_FIND_ALGO_FILE:
            throw std::runtime_error( "Algo file not found." );

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_ERR_FIND_DATA_FILE:
            throw std::runtime_error( "Data file not found." );

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_ERR_WRONG_VERSION:
            throw std::runtime_error( "Wrong version." );

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_ERR_ALGO_FILE_ERROR:
            throw std::runtime_error( "Algo file error." );

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_ERR_DATA_FILE_ERROR:
            throw std::runtime_error( "Data file error." );

        case LeddarDefines::LdSensorVuDefines::BL_FPGA_UPDATE_ERR_OUT_OF_MEMORY:
            throw std::runtime_error( "Device out of memory." );

        default:
            throw std::runtime_error( "Incorect status: " + LeddarUtils::LtStringUtils::IntToString( lStatus ) );
    }
}

// *****************************************************************************
// Function: LdSensorVu::UpdateAsic
//
/// \brief   Update the Asic of the device.
///
/// \param  aIntelHex   Inter file structure.
/// \param  aVerify     Verify the the transfert.
/// \param  aProcessPercentage Property containing the execution pourcentage.
///
/// \exception std::runtime_error If the verify fail.
///
/// \author  Patrick Boulay
///
/// \since   April 2016
// *****************************************************************************
void LdSensorVu::UpdateAsic( const IntelHEX::IntelHexMem &aIntelHex,
                           bool aVerify,
                           LeddarCore::LdIntegerProperty *aProcessPercentage )
{
    const uint32_t lPacketSize = 512;
    uint32_t lSize = aIntelHex.end - aIntelHex.start + 1;
    uint16_t lBuffer[2] = { static_cast<uint16_t>( lSize ), aIntelHex.start };

    // Write asic patch header on device.
    mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::ASIC_PACTH_BASE_ADDR, ( uint8_t * )lBuffer, sizeof( lBuffer ) );

    // Write data
    uint32_t lNbPacket = lSize / lPacketSize;
    uint32_t lExtraBytes = lSize % lPacketSize;

    if( lExtraBytes > 0 )
        lNbPacket++;

    uint16_t lTotalOperation = ( aVerify ? lNbPacket * 2 : lNbPacket );

    for( uint32_t i = 0; i < lNbPacket; i++ )
    {
        mConnectionUniversal->Write( 0x2, LeddarDefines::LdSensorVuDefines::ASIC_PACTH_DATA_ADDR + i * lPacketSize, ( uint8_t * )&aIntelHex.mem[aIntelHex.start + i * lPacketSize],
                                     lPacketSize );
        uint16_t lTransactionCRC16 = 0;
        mConnectionUniversal->Read( 0xb, GetBankAddress( REGMAP_TRN_CFG ) + offsetof( sTransactionCfg, mTransactionCrc ), ( uint8_t * )&lTransactionCRC16, sizeof( lTransactionCRC16 ) );

        if( aProcessPercentage != 0 )
            aProcessPercentage->ForceValue( 0, ( uint32_t )( ( ( float )i / ( float )lTotalOperation ) * 100. ) );
    }

    // Check data integrity
    if( aVerify )
    {
        for( uint32_t i = 0; i < lNbPacket; i++ )
        {
            uint8_t lReadBuffer[lPacketSize];
            mConnectionUniversal->Read( 0xb, LeddarDefines::LdSensorVuDefines::ASIC_PACTH_DATA_ADDR + i * lPacketSize, lReadBuffer, lPacketSize );

            for( uint32_t j = 0; j < lPacketSize; j++ )
            {
                if( aIntelHex.mem[aIntelHex.start + i * lPacketSize + j] != lReadBuffer[j] )
                    throw std::runtime_error( "Verify error on Asic update." );
            }

            if( aProcessPercentage != 0 )
                aProcessPercentage->ForceValue( 0, ( uint32_t )( ( ( float )( i + lPacketSize ) / ( float )lTotalOperation ) * 100. ) );
        }
    }

    if( aProcessPercentage != 0 )
        aProcessPercentage->ForceValue( 0, 100 );
}

#ifdef BUILD_MODBUS
////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn LdSensorVu::SetCarrier(LdCarrierEnhancedModbus *aCarrier)
///
/// \brief   Define the carrier for the sensor. Only available for Modbus connection.
///
/// \author David Levy
/// \date   June 2017
////////////////////////////////////////////////////////////////////////////////////////////////////
void
LdSensorVu::SetCarrier( LdCarrierEnhancedModbus *aCarrier )
{
    if( mCarrier != nullptr )
        throw std::logic_error( "Carrier already set" );

    mCarrier = aCarrier;
    mProperties->AddProperties( aCarrier->GetProperties() );

    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0, P_MODBUS_UNIVERSAL );
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();
}
#endif
#endif