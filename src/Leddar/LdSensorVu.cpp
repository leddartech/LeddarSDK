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

#include "LdConnectionUniversal.h"
#include "LdResultProvider.h"

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

using namespace LeddarCore;
using namespace LeddarConnection;

const uint8_t LICENSE_USER_SIZE = 2 * REGMAP_KEY_LENGTH;
const uint8_t LICENSE_NUMBER = 3;

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
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_ACCUMULATION_EXP, 0, 1,
                              "Accumulation Exponent" ) );
    mProperties->AddProperty( new LdIntegerProperty( LdProperty::CAT_CONFIGURATION, LdProperty::F_EDITABLE | LdProperty::F_SAVE, LdPropertyIds::ID_OVERSAMPLING_EXP, 0, 1,
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
    GetResultStates()->GetProperties()->AddProperty( new LdIntegerProperty( LdProperty::CAT_INFO, LdProperty::F_SAVE, LdPropertyIds::ID_RS_BACKUP, 0, 4, "Calibration Backup Flag" ) );


    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->ForceValue( 0,
            P_SPI ); //We set it as SPI here. SetCarrier function will change it to modbus if needed
    mProperties->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_CONNECTION_TYPE )->SetClean();
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
        LeddarCore::LdIntegerProperty *lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP );
        lIntProp->SetValue( 0, lCfgData->mAccumulationExp );
        lIntProp->SetClean();

        // Oversampling exponent
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP );
        lIntProp->SetValue( 0, lCfgData->mOversamplingExp );
        lIntProp->SetClean();

        // Base point count
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT );
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
        lBoolProp->SetClean();

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
        LeddarCore::LdIntegerProperty *lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP );
        lCfgData->mAccumulationExp = lIntProp->ValueT<uint8_t>();

        // Oversampling exponent
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP );
        lCfgData->mOversamplingExp = lIntProp->ValueT<uint8_t>();

        // Base point count
        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_BASE_POINT_COUNT );
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
        LeddarCore::LdEnumProperty *lEnumProp = GetProperties()->GetEnumProperty( LeddarCore::LdPropertyIds::ID_LED_INTENSITY );
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

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_ACCUMULATION_EXP );
        lIntProp->SetLimits( lDevInfo->mAccExpMin, lDevInfo->mAccExpMax );
        lIntProp->SetClean();

        lIntProp = GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_OVERSAMPLING_EXP );
        lIntProp->SetLimits( lDevInfo->mOvrExpMin, lDevInfo->mOvrExpMax );
        lIntProp->SetClean();

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
        lIntProp->SetClean();

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

                for( int i = 0; i < lEchoCountToReadNow; ++i )
                {
                    ( *lEchoes )[ i ].mChannelIndex = lDetections[ i ].mSegment;
                    ( *lEchoes )[ i ].mDistance = lDetections[ i ].mDistance;
                    ( *lEchoes )[ i ].mAmplitude = lDetections[ i ].mAmplitude;
                    ( *lEchoes )[ i ].mFlag = lDetections[ i ].mFlag;
                }
            }

            lResultEchoes->SetEchoCount( lEchoCount );
            lResultEchoes->SetCurrentLedPower( lCurrentLwdPower );
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
                aResultStates.GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RS_BACKUP )->ForceValue( 0, lBackupFlag );
            }
            catch( ... )
            {
                mBackupFlagAvailable = false;
                aResultStates.GetProperties()->GetIntegerProperty( LeddarCore::LdPropertyIds::ID_RS_BACKUP )->ForceValue( 0, 0 );
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